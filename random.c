/*
 * This file contains the command processing functions for a number of random
 * commands. There is no functional grouping here, for sure.
 *
 * $Log: random.c,v $
 * Revision 1.119  1994/03/08 12:23:03  pgf
 * reworked the gocol/gotocol/getccol etc routines to provide slightly
 * different interface.  added getoff() routine.  moved region-oriented
 * functions to region.c
 *
 * Revision 1.118  1994/02/22  18:09:24  pgf
 * when choosing dos-mode for an ambiguous buffer, vote in favor of on
 * if the global mode is set, and we're running on DOS.  otherwise, choose
 * no-dos-mode.
 *
 * Revision 1.117  1994/02/22  11:03:15  pgf
 * truncated RCS log for 4.0
 *
 */

#include	"estruct.h"
#include	"edef.h"

#if HAVE_POLL
# include <poll.h>
#endif
#if HAVE_SELECT && AIX
# include <sys/select.h>
#endif

#if WATCOM
#   include <direct.h>
#endif

#if TURBO
#   include <dir.h>
#endif

#if DJGPP
#   include <dirent.h>
#endif

extern CMDFUNC f_forwchar, f_backchar, f_forwchar_to_eol, f_backchar_to_bol;

/*--------------------------------------------------------------------------*/
#if MSDOS
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
	set_b_val(bp, MDDOS, MSDOS && global_b_val(MDDOS) );

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

	/* create the buffer list buffer   */
	bp = bfind(name, BFSCRTCH);
	if (bp == NULL)
		return FALSE;

	if ((s=bclear(bp)) != TRUE) /* clear old text (?) */
		return (s);
	b_set_scratch(bp);
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

#if WATCOM || DJGPP /* for testing interrupts */
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

	offs = getoff(n,NULL);

	if (offs >= 0) {
		DOT.o = offs;
		return TRUE;
	}

	DOT.o = llength(DOT.l) - 1;
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



#if AEDIT
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
#endif


/* delay for the given number of milliseconds.  if "watchinput" is true,
	then user input will abort the delay
	FIXXXX -- the second arg is only implemented for UNIX with select() */
void
catnap(milli,watchinput)
int milli;
int watchinput;
{
#if UNIX
# if HAVE_SELECT

	struct timeval tval;
	fd_set read_bits;

	FD_ZERO(&read_bits);
	if (watchinput) {
		FD_SET(0, &read_bits);
	}
	tval.tv_sec = 0;
	tval.tv_usec = milli * 1000;	/* microseconds */
	(void)select (1, &read_bits, (fd_set*)0, (fd_set*)0, &tval);

# else
#  if HAVE_POLL

	struct pollfd pfd;
	(void)poll(&pfd, 0, milli); /* milliseconds */

#  else

	sleep(1); /* 1 second.	ugh. */

#  endif
# endif
#endif

#if VMS
	float	seconds = milli/1000.;
	lib$wait(&seconds);
#endif

#if NEWDOSCC
	delay(milli);
#endif

#if !(UNIX|VMS|NEWDOSCC)
	long i;
	for (i = 0; i < term.t_pause; i++)
		;
#endif
}

#if UNIX
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
#if USG && ! POSIX
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
#else
# if (MSDOS & NEWDOSCC) || POSIX || VMS
	cwd = getcwd(dirname, NFILEN);
# else
	cwd = getwd(dirname);
# endif
#endif
#if MSDOS
	(void)mklower(cwd);
#endif
	if (cwd == NULL) {
		cwd = dirname;
		dirname[0] = slash;
		dirname[1] = EOS;
	} else {
		s = strchr(cwd, '\n');
		if (s)
			*s = EOS;
	}

#if MSDOS && NEWDOSCC
	update_dos_drv_dir(cwd);
#endif

	return cwd;
}

#if MSDOS

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
	return drive2char(bdos(0x19, 0, 0) & 0xff);
}

/* take drive _letter_ as arg. */
int
setdrive(d)
int d;
{
	if (isalpha(d)) {
		bdos(0x0e, char2drive(d), 0);
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
#if UNIX || VMS
	if (status == FALSE) {		/* empty reply, go HOME */
#if UNIX
		(void)lengthen_path(strcpy(cdirname, "~"));
#else	/* VMS */
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

/* move to the named directory.  (Dave Lemke) */
int
set_directory(dir)
char	*dir;
{
    static char prevdir[NFILEN];
    char       exdir[NFILEN];
#if UNIX
    char       *cdpath;
    char       cdpathdir[NFILEN];
#endif
    char *exdp;
#if MSDOS
    int curd = curdrive();
#endif
    WINDOW *wp;

    for_each_window(wp)
	wp->w_flag |= WFMODE;

    exdp = strcpy(exdir, dir);

    if (doglob(exdp)) {
#if MSDOS
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
		updatelistbuffers();
		return TRUE;
	}

#if UNIX && PATHLOOK
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
				updatelistbuffers();
				return TRUE;
			}
		}
	}
#endif
    }
#if MSDOS
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
