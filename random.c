/*
 * This file contains the command processing functions for a number of random
 * commands. There is no functional grouping here, for sure.
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/random.c,v 1.145 1994/11/29 14:47:13 pgf Exp $
 *
 */

#include	"estruct.h"
#include	"edef.h"

#if HAVE_POLL && HAVE_POLL_H
# include <poll.h>
#endif

#if CC_WATCOM
#   include <direct.h>
#endif

#if CC_TURBO
#   include <dir.h>
#endif

#if CC_DJGPP
#   include <dirent.h>
#endif

#if SYS_OS2
#   include <dos.h>  /* for _dos_getdrive */
#   include <dir.h>
#endif

extern CMDFUNC f_forwchar, f_backchar, f_forwchar_to_eol, f_backchar_to_bol;

/*--------------------------------------------------------------------------*/
#if SYS_MSDOS || SYS_OS2 || SYS_WINNT
static	int	drive2char P(( int ));
static	int	char2drive P(( int ));
#endif

/*--------------------------------------------------------------------------*/

/*
 * Set default parameters for an automatically-generated, view-only buffer.
 * This is invoked after buffer creation, but usually before the buffer is
 * loaded with text.
 */
void
set_rdonly(bp, name)
BUFFER	*bp;
char	*name;
{
	ch_fname(bp, name);

	b_clr_changed(bp);		/* assumes text is loaded... */
	bp->b_active = TRUE;

	make_local_b_val(bp,MDVIEW);
	set_b_val(bp,MDVIEW,TRUE);

	make_local_b_val(bp,VAL_TAB);
	set_b_val(bp,VAL_TAB,8);

	make_local_b_val(bp,MDDOS);
	set_b_val(bp, MDDOS, CRLF_LINES);

	make_local_b_val(bp,MDCMOD);
	set_b_val(bp,MDCMOD,FALSE);
}

/* generic "lister", which takes care of popping a window/buffer pair under
	the given name, and calling "func" with a couple of args to fill in
	the buffer */
int
liststuff(name,func,iarg,carg)
char *name;
void (*func) P(( int, char *));	/* ptr to function to execute */
int iarg;
char *carg;
{
	register BUFFER *bp;
	register int	s;
	WINDOW  *wp;
	int alreadypopped;
	BUFFER *ocurbp = curbp;

	/* create the buffer list buffer   */
	bp = bfind(name, BFSCRTCH);
	if (bp == NULL)
		return FALSE;

	if ((s=bclear(bp)) != TRUE) /* clear old text (?) */
		return (s);
	b_set_scratch(bp);
	alreadypopped = (bp->b_nwnd != 0);
	if (popupbuff(bp) == FALSE) {
		(void)zotbuf(bp);
		return (FALSE);
	}

	if ((wp = bp2any_wp(bp)) != NULL) {
		make_local_w_val(wp,WMDNUMBER);
		set_w_val(wp,WMDNUMBER,FALSE);
	}
	/* call the passed in function, giving it both the integer and
		character pointer arguments */
	(*func)(iarg,carg);
	(void)gotobob(FALSE,1);
	set_rdonly(bp, non_filename());

	if (alreadypopped) /* don't switch to the popup if it wasn't there */
		swbuffer(ocurbp);
	else
		shrinkwrap(); /* only resize if it's fresh */

	return TRUE;
}

/*
 * Display the current position of the cursor, lines and columns, in the file,
 * the character that is under the cursor (in hex), and the fraction of the
 * text that is before the cursor. The displayed column is not the current
 * column, but the column that would be used on an infinite width display.
 */
/* ARGSUSED */
int
showcpos(f, n)
int f,n;
{
	register LINE	*lp;		/* current line */
	register B_COUNT numchars = 0;	/* # of chars in file */
	register L_NUM	 numlines = 0;	/* # of lines in file */
	register B_COUNT predchars = 0;	/* # chars preceding point */
	register L_NUM	 predlines = 0;	/* # lines preceding point */
	register int	curchar = '\n';	/* character under cursor */
	long ratio;
	C_NUM col;
	C_NUM savepos;			/* temp save for current offset */
	C_NUM ecol;			/* column pos/end of current line */

#if CC_WATCOM || CC_DJGPP /* for testing interrupts */
	if (f && n == 11) {
		mlwrite("DOS interrupt test.  hit control-C or control-BREAK");
		while (!interrupted())
			;
		mlwrite("whew.  got interrupted");
		return ABORT;
	}
#endif
	/* count chars and lines */
	for_each_line(lp, curbp) {
		/* if we are on the current line, record it */
		if (lp == l_ref(DOT.l)) {
			predlines = numlines;
			predchars = numchars + DOT.o;
			if (DOT.o == llength(lp))
				curchar = '\n';
			else
				curchar = char_at(DOT);
		}
		/* on to the next line */
		++numlines;
		numchars += llength(lp) + 1;
	}

	if (!b_val(curbp,MDNEWLINE))
		numchars--;

	/* if at end of file, record it */
	if (is_header_line(DOT,curbp)) {
		predlines = numlines;
		predchars = numchars;
	}

	/* Get real column and end-of-line column. */
	col = getccol(FALSE);
	savepos = DOT.o;
	DOT.o = lLength(DOT.l);
	ecol = getccol(FALSE);
	DOT.o = savepos;

	ratio = 0;		/* Ratio before dot. */
	if (numchars != 0)
		ratio = (100L*predchars) / numchars;

	/* summarize and report the info */
	mlforce(
"Line %d of %d, Col %d of %d, Char %D of %D (%D%%) char is 0x%x or 0%o",
		predlines+1, numlines, col+1, ecol,
		predchars+1, numchars, ratio, curchar, curchar);
	return TRUE;
}

/* ARGSUSED */
int
showlength(f,n)
int f,n;
{
	/* actually, can be used to show any address-value */
	mlforce("%d", line_no(curbp, MK.l));
	return TRUE;
}

void
line_report(before)
L_NUM	before;
{
	L_NUM	after = line_count(curbp);

	if (do_report(before-after)) {
		if (before > after)
			mlforce("[%d fewer lines]", before - after);
		else
			mlforce("[%d more lines]", after - before);
	}
}

L_NUM
line_count(the_buffer)
BUFFER *the_buffer;
{
    if (the_buffer == (BUFFER *) 0)
	return 0;
    else {
#if !SMALLER
	(void)bsizes(the_buffer);
	return the_buffer->b_linecount;
#else
	register LINE	*lp;		/* current line */
	register L_NUM	numlines = 0;	/* # of lines in file */

	for_each_line(lp, the_buffer)
		++numlines;

	return numlines;
#endif
    }
}

L_NUM
line_no(the_buffer, the_line)	/* return the number of the given line */
BUFFER *the_buffer;
LINEPTR the_line;
{
	if (!same_ptr(the_line, null_ptr)) {
#if !SMALLER
		L_NUM	it;
		(void)bsizes(the_buffer);
		if ((it = l_ref(the_line)->l_number) == 0)
			it = the_buffer->b_linecount + 1;
		return it;
#else
		register LINE	*lp;		/* current line */
		register L_NUM	numlines = 0;	/* # of lines before point */

		for_each_line(lp, the_buffer) {
			/* if we are on the specified line, record it */
			if (lp == l_ref(the_line))
				break;
			++numlines;
		}

		/* and return the resulting count */
		return(numlines + 1);
#endif
	}
	return 0;
}

#if OPT_EVAL
L_NUM
getcline()	/* get the current line number */
{
	return line_no(curbp, DOT.l);
}
#endif

/*
 * Return the screen column in any line given an offset.
 * Assume the line is in curwp/curbp
 * Stop at first non-blank given TRUE argument.
 */
int
getcol(lp,offs,bflg)
LINEPTR lp;
C_NUM offs;
int bflg;
{
	register C_NUM c, i, col;
	col = 0;
	if (lLength(lp))
		for (i = w_left_margin(curwp); i < offs; ++i) {
			c = lGetc(lp, i);
			if (!isspace(c) && bflg)
				break;
			col = next_column(c,col);	/* assumes curbp */
		}
	return col;
}

/*
 * Return current screen column.  Stop at first non-blank given TRUE argument.
 */
int
getccol(bflg)
int bflg;
{
	return getcol(DOT.l, DOT.o, bflg);
}


/*
 * Set current column, based on counting from 1
 */
int
gotocol(f,n)
int f,n;
{
	if (!f || n <= 0)
		n = 1;
	return gocol(n - 1);
}

/* given a column, return the offset */
/*  if there aren't that many columns, return how too few we were */
/*	also, if non-null, return the column we _did_ reach in *rcolp */
int
getoff(goal,rcolp)
C_NUM goal;
C_NUM *rcolp;
{
	register C_NUM c;		/* character being scanned */
	register C_NUM i;		/* index into current line */
	register C_NUM col;	/* current cursor column   */
	register C_NUM llen;	/* length of line in bytes */

	col = 0;
	llen = lLength(DOT.l);

	/* scan the line until we are at or past the target column */
	for (i = w_left_margin(curwp); i < llen; ++i) {
		/* upon reaching the target, drop out */
		if (col >= goal)
			break;

		/* advance one character */
		c = lGetc(DOT.l, i);
		col = next_column(c,col);
	}

	if (rcolp)
		*rcolp = col;

	/* and tell whether we made it */
	if (col >= goal)
		return i;	/* we made it */
	else
		return col - goal; /* else how far short (in spaces) we were */
}

/* really set column, based on counting from 0, for internal use */
int
gocol(n)
int n;
{
	register int offs;	/* current cursor column   */

	offs = getoff(n, (C_NUM *)0);

	if (offs >= 0) {
		DOT.o = offs;
		return TRUE;
	}

	DOT.o = lLength(DOT.l) - 1;
	return FALSE;

}


#if ! SMALLER
/*
 * Twiddle the two characters on either side of dot. If dot is at the end of
 * the line twiddle the two characters before it. Return with an error if dot
 * is at the beginning of line; it seems to be a bit pointless to make this
 * work. This fixes up a very common typo with a single stroke.
 * This always works within a line, so "WFEDIT" is good enough.
 */
/* ARGSUSED */
int
twiddle(f, n)
int f,n;
{
	MARK		dot;
	register int	cl;
	register int	cr;

	dot = DOT;
	if (is_empty_line(dot))
		return (FALSE);
	--dot.o;
	cr = char_at(dot);
	if (--dot.o < 0)
		return (FALSE);
	cl = char_at(dot);
	copy_for_undo(dot.l);
	put_char_at(dot, cr);
	++dot.o;
	put_char_at(dot, cl);
	chg_buff(curbp, WFEDIT);
	return (TRUE);
}
#endif



#if OPT_AEDIT
/*
 * Delete blank lines around dot. What this command does depends if dot is
 * sitting on a blank line. If dot is sitting on a blank line, this command
 * deletes all the blank lines above and below the current line. If it is
 * sitting on a non blank line then it deletes all of the blank lines after
 * the line. Any argument is ignored.
 */
/* ARGSUSED */
int
deblank(f, n)
int f,n;
{
	register LINE	*lp1;
	register LINE	*lp2;
	long nld;

	lp1 = l_ref(DOT.l);
	while (llength(lp1)==0 && (lp2=lback(lp1))!=l_ref(buf_head(curbp)))
		lp1 = lp2;
	lp2 = lp1;
	nld = 0;
	while ((lp2=lforw(lp2))!=l_ref(buf_head(curbp)) && llength(lp2)==0)
		++nld;
	if (nld == 0)
		return (TRUE);
	DOT.l = l_ptr(lforw(lp1));
	DOT.o = 0;
	return (ldelete(nld, FALSE));
}

#endif

/* '~' is the traditional vi flip: flip a char and advance one */
int
flipchar(f, n)
int f,n;
{
	int s;

	havemotion = &f_forwchar_to_eol;
	s = operflip(f,n);
	if (s != TRUE)
		return s;
	return forwchar_to_eol(f,n);
}

/* 'x' is synonymous with 'd<space>' */
int
forwdelchar(f, n)
int f,n;
{

	havemotion = &f_forwchar_to_eol;
	return operdel(f,n);
}

/* 'X' is synonymous with 'd<backspace>' */
int
backdelchar(f, n)
int f,n;
{
	havemotion = &f_backchar_to_bol;
	return operdel(f,n);
}

/* 'D' is synonymous with 'd$' */
/* ARGSUSED */
int
deltoeol(f, n)
int f,n;
{
	extern CMDFUNC f_gotoeol;

	if (lLength(DOT.l) == 0)
		return TRUE;

	havemotion = &f_gotoeol;
	return operdel(FALSE,1);
}

/* 'C' is synonymous with 'c$' */
int
chgtoeol(f, n)
int f,n;
{
	extern CMDFUNC f_gotoeol;

	if (lLength(DOT.l) == 0) {
		return ins();
	} else {
		havemotion = &f_gotoeol;
		return operchg(f,n);
	}
}

/* 'Y' is synonymous with 'yy' */
int
yankline(f, n)
int f,n;
{
	extern CMDFUNC f_godotplus;
	havemotion = &f_godotplus;
	return(operyank(f,n));
}

/* 'S' is synonymous with 'cc' */
int
chgline(f, n)
int f,n;
{
	extern CMDFUNC f_godotplus;
	havemotion = &f_godotplus;
	return(operchg(f,n));
}

/* 's' is synonymous with 'c<space>' */
int
chgchar(f, n)
int f,n;
{
	havemotion = &f_forwchar_to_eol;
	return(operchg(f,n));
}



/*	This function simply clears the message line,
		mainly for macro usage			*/

/* ARGSUSED */
int
clrmes(f, n)
int f, n;	/* arguments ignored */
{
	mlerase();
	return(TRUE);
}

#if ! SMALLER

/*	This function writes a string on the message line
		mainly for macro usage			*/

/* ARGSUSED */
int
writemsg(f, n)
int f, n;	/* arguments ignored */
{
	register int status;
	char buf[NPAT];		/* buffer to receive message into */

	buf[0] = EOS;
	if ((status = mlreply("Message to write: ", buf, sizeof(buf))) != TRUE)
		return(status);

	/* write the message out */
	mlforce("%s",buf);
	return(TRUE);
}

/* ARGSUSED */
int
userbeep(f, n)
int f, n;
{
	TTbeep();
	return TRUE;
}
#endif /* !SMALLER */


/* delay for the given number of milliseconds.  if "watchinput" is true,
	then user input will abort the delay */
void
catnap(milli,watchinput)
int milli;
int watchinput;
{
    if (milli == 0)
    	return;
#if DISP_X11
    if (watchinput)
	x_typahead(milli);
    else
#endif
    {
#if SYS_UNIX
# if HAVE_SELECT

	struct timeval tval;
	fd_set read_bits;
#  if HAVE_SIGPROCMASK
	sigset_t newset, oldset;
	sigemptyset(&newset);
	sigaddset(&newset, SIGALRM);
	sigprocmask(SIG_BLOCK, &newset, &oldset);
#  endif

	FD_ZERO(&read_bits);
	if (watchinput) {
		FD_SET(0, &read_bits);
	}
	tval.tv_sec = milli / 1000;
	tval.tv_usec = (milli % 1000) * 1000;	/* microseconds */
	(void)select (1, &read_bits, (fd_set*)0, (fd_set*)0, &tval);

#  if HAVE_SIGPROCMASK
	sigprocmask(SIG_SETMASK, &oldset, (sigset_t*)0);
#  endif

# else
#  if HAVE_POLL && HAVE_POLL_H

	struct pollfd pfd;
	(void)poll(&pfd, 0, milli); /* milliseconds */

#  else

	int seconds = (milli + 999) / 1000;
	if (watchinput)  {
		while(seconds > 0) {
			sleep(1);
			if (TTtypahead())
				return;
			seconds -= 1;
		}
	} else {
		sleep(seconds);
	}

#  endif
# endif
#define have_slept 1
#endif

#if SYS_VMS
	float	seconds = milli/1000.;
	if (watchinput)  {
		float tenth = .1;
		while(seconds > 0.1) {
			lib$wait(&tenth);
			if (TTtypahead())
				return;
			seconds -= tenth;
		}
	}
	lib$wait(&seconds);
#define have_slept 1
#endif

#if SYS_WINNT || (CC_NEWDOSCC && SYS_MSDOS)
#if SYS_WINNT
#define delay(a) Sleep(a)
#endif
	if (watchinput)  {
		while(milli > 100) {
			delay(100);
			if (TTtypahead())
				return;
			milli -= 100;
		}
	}
	delay(milli);
#define have_slept 1
#endif

#ifndef have_slept
	long i;
	for (i = 0; i < term.t_pause; i++)
		;
#endif
    }
}

#if SYS_UNIX
#include	<sys/param.h>
#endif

/* return a string naming the current directory */
char *
current_directory(force)
int force;
{
	char *s;
	static char	dirname[NFILEN];
	static char *cwd;

	if (!force && cwd)
		return cwd;
#if HAVE_GETCWD
	cwd = getcwd(dirname, NFILEN);
#else
# if HAVE_GETWD
	cwd = getwd(dirname);
# else
	{
	FILE *f, *npopen();
	int n;
	f = npopen("pwd", "r");
	if (f == NULL) {
		npclose(f);
		return NULL;
	}
	n = fread(dirname, 1, NFILEN, f);

	dirname[n] = EOS;
	npclose(f);
	cwd = dirname;
	}
# endif
#endif
#if SYS_MSDOS || SYS_OS2 || SYS_WINNT
	(void)mklower(cwd);
#endif
	if (cwd == NULL) {
		cwd = dirname;
		dirname[0] = SLASHC;
		dirname[1] = EOS;
	} else {
		s = strchr(cwd, '\n');
		if (s)
			*s = EOS;
	}

#if SYS_MSDOS || SYS_OS2
	update_dos_drv_dir(cwd);
#endif

	return cwd;
}

#if SYS_MSDOS || SYS_OS2

/* convert drive index to _letter_ */
static int
drive2char(d)
int	d;
{
	if (d < 0 || d >= 26) {
		mlforce("[Illegal drive index %d]", d);
		d = 0;
	}
	return (d + 'A');
}

/* convert drive _letter_ to index */
static int
char2drive(d)
int	d;
{
	if (isalpha(d)) {
		if (islower(d))
			d = toupper(d);
	} else {
		mlforce("[Not a drive '%c']", d);
		d = curdrive();
	}
	return (d - 'A');
}

/* returns drive _letter_ */
int
curdrive()
{
#if SYS_OS2
	int d;
	_dos_getdrive(&d);  	/* A=1 B=2 ... */
	return drive2char((d-1) & 0xff);
#else
	return drive2char(bdos(0x19, 0, 0) & 0xff);
#endif
}

/* take drive _letter_ as arg. */
int
setdrive(d)
int d;
{
	if (isalpha(d)) {
#if SYS_OS2
		int maxdrives;
		_dos_setdrive(char2drive(d)+1, &maxdrives); /* 1 based */
#else
		bdos(0x0e, char2drive(d), 0); /* 0 based */
#endif
		return TRUE;
	}
	mlforce("[Bad drive specifier]");
	return FALSE;
}


static int curd;		/* current drive-letter */
static char *cwds[26];		/* list of current dirs on each drive */

char *
curr_dir_on_drive(drive)
int drive;
{
	int	n = char2drive(drive);

	if (n != 0) {
		if (curd == 0)
			curd = curdrive();

		if (cwds[n])
			return cwds[n];
		else {
			cwds[n] = castalloc(char,NFILEN);

			if (cwds[n]) {
				if (setdrive(drive) == TRUE) {
					(void)strcpy(cwds[n], current_directory(TRUE));
					(void)setdrive(curd);
					(void)current_directory(TRUE);
					return cwds[n];
				}
			}
		}
	}
	return current_directory(FALSE);
}

void
update_dos_drv_dir(cwd)
char *cwd;
{
	char	*s;

	if ((s = is_msdos_drive(cwd)) != 0) {
		int n = char2drive(*cwd);

		if (!cwds[n])
			cwds[n] = castalloc(char,NFILEN);

		if (cwds[n])
			(void)strcpy(cwds[n], s);
	}
}
#endif


/* ARGSUSED */
int
cd(f,n)
int f, n;
{
	register int status;
	static	TBUFF	*last;
	char cdirname[NFILEN];

	status = mlreply_dir("Change to directory: ", &last, cdirname);
#if SYS_UNIX || SYS_VMS
	if (status == FALSE) {		/* empty reply, go HOME */
#if SYS_UNIX
		(void)lengthen_path(strcpy(cdirname, "~"));
#else	/* SYS_VMS */
		(void)strcpy(cdirname, "sys$login");
#endif
	} else
#endif
	if (status != TRUE)
		return status;

	return set_directory(cdirname);
}

/* ARGSUSED */
int
pwd(f,n)
int f, n;
{
	mlforce("%s",current_directory(f));
	return TRUE;
}

static char prevdir[NFILEN];

#if OPT_EVAL
char *
previous_directory()
{
	if (*prevdir)
		return prevdir;
	else
		return current_directory(FALSE);
}
#endif

/* move to the named directory.  (Dave Lemke) */
int
set_directory(dir)
char	*dir;
{
    char       exdir[NFILEN];
#if SYS_UNIX
    char       *cdpath;
    char       cdpathdir[NFILEN];
#endif
    char *exdp;
#if SYS_MSDOS || SYS_OS2
    int curd = curdrive();
#endif
    WINDOW *wp;
#if OPT_PROCEDURES
    static int cdhooking;
#endif

    for_each_window(wp)
	wp->w_flag |= WFMODE;

    exdp = strcpy(exdir, dir);

    if (doglob(exdp)) {
#if SYS_MSDOS || SYS_OS2
	char	*s;
	if ((s = is_msdos_drive(exdp)) != 0) {
		if (setdrive(*exdp) == TRUE) {
			exdp = s;	/* skip device-part */
			if (!*exdp) {
				return pwd(TRUE,1);
			}
		} else {
			return FALSE;
		}
	}
#endif
	/*
	** "cd -" switches to the previous directory.
	*/
	if (!strcmp(exdp, "-"))
	{
		if (*prevdir)
			(void) strcpy(exdp, prevdir);
		else
		{
		    mlforce("[No previous directory");
		    return FALSE;
		}
	}

	/* Save current directory for subsequent "cd -". */
	(void) strcpy(prevdir, current_directory(FALSE));

	if (chdir(exdp) == 0) {
		(void)pwd(TRUE,1);
#if OPT_PROCEDURES
	{ 
	    if (!cdhooking && *cdhook) {
		    cdhooking = TRUE;
		    run_procedure(cdhook);
		    cdhooking = FALSE;
	    }
	}
#endif
		updatelistbuffers();
		return TRUE;
	}

#if SYS_UNIX && OPT_PATHLOOKUP
	/*
	** chdir failed.  If the directory name doesn't begin with any of
	** "/", "./", or "../", get the CDPATH environment variable and check
	** if the specified directory name is a subdirectory of a
	** directory in CDPATH.
	*/
	if (!is_pathname(exdp)
	 && (cdpath = getenv("CDPATH")) != 0) {
		/* For each colon-separated component in CDPATH */
		while ((cdpath = parse_pathlist(cdpath, cdpathdir)) != 0) {
			if (chdir(pathcat(cdpathdir, cdpathdir, exdp)) == 0) {
				(void)pwd(TRUE,1);
#if OPT_PROCEDURES
				{ 
				    if (!cdhooking && *cdhook) {
					    cdhooking = TRUE;
					    run_procedure(cdhook);
					    cdhooking = FALSE;
				    }
				}
#endif
				updatelistbuffers();
				return TRUE;
			}
		}
	}
#endif
    }
#if SYS_MSDOS || SYS_OS2
    setdrive(curd);
    current_directory(TRUE);
#endif
    mlforce("[Couldn't change to \"%s\"]", exdir);
    return FALSE;
}

void
ch_fname(bp,fname)
BUFFER *bp;
char *fname;
{
	int len;
	char nfilen[NFILEN];
	char *np;
	char *holdp = NULL;

	np = fname;

	/* produce a full pathname, unless already absolute or "internal" */
	if (!isInternalName(np))
		np = lengthen_path(strcpy(nfilen, np));

	len = strlen(np)+1;

	if (bp->b_fname == 0 || strcmp(bp->b_fname, np)) {

		if (bp->b_fname && bp->b_fnlen < len ) {
			/* don't free it yet -- it _may_ have been passed in as
			 * the current file-name
			 */
			holdp = bp->b_fname;
			bp->b_fname = NULL;
		}

		if (!bp->b_fname) {
			bp->b_fname = strmalloc(np);
			if (!bp->b_fname) {
				bp->b_fname = out_of_mem;
				bp->b_fnlen = strlen(bp->b_fname);
				return;
			}
			bp->b_fnlen = len;
		}

		/* it'll fit, leave len untouched */
		(void)strcpy(bp->b_fname, np);

		if (holdp != out_of_mem)
			FreeIfNeeded(holdp);
		updatelistbuffers();
	}
#ifdef	MDCHK_MODTIME
	(void)get_modtime(bp, &(bp->b_modtime));
	bp->b_modtime_at_warn = 0;
#endif
}
