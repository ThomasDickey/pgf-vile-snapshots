/*
 * This file contains the command processing functions for a number of random
 * commands. There is no functional grouping here, for sure.
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/random.c,v 1.181 1996/06/12 18:14:19 pgf Exp $
 *
 */

#include	"estruct.h"
#include	"edef.h"
#include	"nefunc.h"

#if SYS_UNIX
#if HAVE_POLL && HAVE_POLL_H
# include <poll.h>
#endif
#endif

#include	"dirstuff.h"

#if CC_TURBO
#include <dir.h>	/* for 'chdir()' */
#endif

#if SYS_VMS
#include <starlet.h>
#include <lib$routines.h>
#endif

/*--------------------------------------------------------------------------*/

/*
 * Set default parameters for an automatically-generated, view-only buffer.
 * This is invoked after buffer creation, but usually before the buffer is
 * loaded with text.
 * the "mode" argument should be MDVIEW or MDREADONLY
 */
void
set_rdonly(BUFFER *bp, const char *name, int mode)
{
	ch_fname(bp, name);

	b_clr_changed(bp);		/* assumes text is loaded... */
	bp->b_active = TRUE;

	make_local_b_val(bp, mode);
	set_b_val(bp,mode,TRUE);

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
liststuff(
const char *name,
int appendit,
void (*func) (LIST_ARGS),	/* ptr to function to execute */
int iarg,
void *vargp)
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

	if (!appendit && (s=bclear(bp)) != TRUE) /* clear old text (?) */
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
	if (appendit) {
		(void)gotoeob(FALSE,1);
		MK = DOT;
	}
	/* call the passed in function, giving it both the integer and
		character pointer arguments */
	(*func)(iarg,vargp);
	if (alreadypopped && appendit) {
		(void)gomark(FALSE,1);
		(void)forwbline(FALSE,1);
		(void)reposition(FALSE,1);
	} else { /* if we're not appending, go to the top */
		(void)gotobob(FALSE,1);
	}
	set_rdonly(bp, non_filename(), MDVIEW);

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
showcpos(int f, int n)
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
#if debug_undo_log
	extern int do_undolog;
	if (f && n == 11)
		do_undolog = !do_undolog;
#endif
	/* count chars and lines */
	for_each_line(lp, curbp) {
		/* if we are on the current line, record it */
		if (lp == DOT.l) {
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
	DOT.o = llength(DOT.l);
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
showlength(int f, int n)
{
	/* actually, can be used to show any address-value */
	mlforce("%d", line_no(curbp, MK.l));
	return TRUE;
}

int
line_report(L_NUM before)
{
	L_NUM	after = line_count(curbp);

	if (do_report(before-after)) {
		if (before > after)
			mlwrite("[%d fewer lines]", before - after);
		else
			mlwrite("[%d more lines]", after - before);
		return TRUE;
	}
	return FALSE;
}

L_NUM
line_count(BUFFER *the_buffer)
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
line_no(		/* return the number of the given line */
BUFFER *the_buffer,
LINEPTR the_line)
{
	if (the_line != null_ptr) {
#if !SMALLER
		L_NUM	it;
		(void)bsizes(the_buffer);
		if ((it = the_line->l_number) == 0)
			it = the_buffer->b_linecount + 1;
		return it;
#else
		register LINE	*lp;		/* current line */
		register L_NUM	numlines = 0;	/* # of lines before point */

		for_each_line(lp, the_buffer) {
			/* if we are on the specified line, record it */
			if (lp == the_line)
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
getcline(void)	/* get the current line number */
{
	return line_no(curbp, DOT.l);
}
#endif

/*
 * Return the screen column in any line given an offset.
 * Assume the line is in curwp/curbp
 */
int
getcol(
MARK mark,
int actual)	/* false: effective column (expand tabs) */
		/* true: compute the actual column (depends on 'list' mode)*/
{
	register C_NUM c, i;
	register C_NUM col = 0;

	if (llength(mark.l) > 0) {
		if (actual) {
			col = offs2col(curwp, mark.l, mark.o) - nu_width(curwp);
		} else {
			for (i = w_left_margin(curwp); i < mark.o; ++i) {
				c = lgetc(mark.l, i);
				col = next_column(c,col); /* assumes curbp */
			}
		}
	}
	return col;
}

/*
 * Return current screen column.  Stop at first non-blank given TRUE argument.
 */
int
getccol(int bflg)
{
	return getcol(DOT, bflg);
}


/*
 * Set current column, based on counting from 1
 */
int
gotocol(int f, int n)
{
	if (!f || n <= 0)
		n = 1;
	return gocol(n - 1);
}

/* given a column, return the offset */
/*  if there aren't that many columns, return how too few we were */
/*	also, if non-null, return the column we _did_ reach in *rcolp */
int
getoff(
C_NUM goal,
C_NUM *rcolp)
{
	register C_NUM c;		/* character being scanned */
	register C_NUM i;		/* index into current line */
	register C_NUM col;	/* current cursor column   */
	register C_NUM llen;	/* length of line in bytes */

	col = 0;
	llen = llength(DOT.l);

	/* scan the line until we are at or past the target column */
	for (i = w_left_margin(curwp); i < llen; ++i) {
		/* upon reaching the target, drop out */
		if (col >= goal)
			break;

		/* advance one character */
		c = lgetc(DOT.l, i);
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
gocol(int n)
{
	register int offs;	/* current cursor column   */

	offs = getoff(n, (C_NUM *)0);

	if (offs >= 0) {
		DOT.o = offs;
		return TRUE;
	}

	DOT.o = llength(DOT.l) - 1;
	return FALSE;

}


#if ! SMALLER
/*
 * Twiddle the two characters on under and to the left of dot.  If dot is
 * at the end of the line twiddle the two characters before it.  This fixes
 * up a very common typo with a single stroke.  This always works within a
 * line, so "WFEDIT" is good enough.
 */
/* ARGSUSED */
int
twiddle(int f, int n)
{
	MARK		dot;
	register char	cl;
	register char	cr;

	dot = DOT;
	if (llength(dot.l) <= 1)
		return FALSE;
	if (is_at_end_of_line(dot))
		--dot.o;
	if (dot.o == 0)
		dot.o = 1;
	cr = char_at(dot);
	--dot.o;
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
 * Force zero or more blank lines at dot.  no argument leaves no blanks.
 * Lines will be deleted or added as needed to match the argument if it
 * is given.
 */
int
forceblank(int f, int n)
{
	register LINE	*lp1;
	register LINE	*lp2;
	B_COUNT nld;
	B_COUNT n_arg;
	C_NUM	nchar;
	int s = TRUE;

	if (!f || n < 0)
		n = 0;
	n_arg = n;

	lp1 = DOT.l;
	/* scan backward */
	while (firstchar(lp1) < 0 && 
			(lp2 = lback(lp1)) != buf_head(curbp))
		lp1 = lp2;
	lp2 = lp1;

	nld = 0;
	nchar = 0;

	/* scan forward */
	while ((lp2 = lforw(lp2)) != buf_head(curbp) && 
			firstchar(lp2) < 0) {
		++nld;
		if (nld > n_arg)
			nchar += llength(lp2) + 1;
	}

	DOT.l = lforw(lp1);
	DOT.o = 0;

	if (n_arg == nld) {		/* things are just right */
		/*EMPTY*/;
	} else if (n_arg < nld) {	/* delete (nld - n_arg) lines */
		DOT.l = lp2;
		DOT.o = 0;
		backchar(TRUE,nchar);
		s = ldelete((B_COUNT)nchar, FALSE);
	} else { 			/* insert (n_arg - nld) lines */
		n_arg = n_arg - nld;
		while (s && n_arg--)
			s = lnewline();
	}

	/* scan forward */
	while ((firstchar(DOT.l) < 0) &&
			(DOT.l != buf_head(curbp)))
		DOT.l = lforw(DOT.l);

	return s;
}

#endif

/* '~' is the traditional vi flip: flip a char and advance one */
int
flipchar(int f, int n)
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
forwdelchar(int f, int n)
{
	havemotion = &f_forwchar_to_eol;
	return operdel(f,n);
}

/* 'X' is synonymous with 'd<backspace>' */
int
backdelchar(int f, int n)
{
	havemotion = &f_backchar_to_bol;
	return operdel(f,n);
}

/* 'D' is synonymous with 'd$' */
/* ARGSUSED */
int
deltoeol(int f, int n)
{
	if (is_empty_line(DOT))
		return TRUE;

	havemotion = &f_gotoeol;
	return operdel(FALSE,1);
}

/* 'C' is synonymous with 'c$' */
int
chgtoeol(int f, int n)
{
	if (is_empty_line(DOT)) {
		return ins();
	} else {
		havemotion = &f_gotoeol;
		return operchg(f,n);
	}
}

/* 'Y' is synonymous with 'yy' */
int
yankline(int f, int n)
{
	havemotion = &f_godotplus;
	return(operyank(f,n));
}

/* 'S' is synonymous with 'cc' */
int
chgline(int f, int n)
{
	havemotion = &f_godotplus;
	return(operchg(f,n));
}

/* 's' is synonymous with 'c<space>' */
int
chgchar(int f, int n)
{
	havemotion = &f_forwchar_to_eol;
	return(operchg(f,n));
}



/*	This function simply clears the message line,
		mainly for macro usage			*/

/* ARGSUSED */
int
clrmes(int f, int n)
{
	mlerase();
	return(TRUE);
}

#if ! SMALLER

/*	This function writes a string on the message line
		mainly for macro usage			*/

/* ARGSUSED */
int
writemsg(int f, int n)
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
userbeep(int f, int n)
{
	TTbeep();
	return TRUE;
}
#endif /* !SMALLER */


/* delay for the given number of milliseconds.  if "watchinput" is true,
	then user input will abort the delay */
void
catnap(int milli, int watchinput)
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
	int fdcnt;
	if (watchinput) {
		pfd.fd = 0;
		pfd.events = POLLIN;
		fdcnt = 1;
	} else {
		fdcnt = 0;
	}
	(void)poll(&pfd, fdcnt, milli); /* milliseconds */

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

#if SYS_WINNT || (CC_NEWDOSCC && SYS_MSDOS) || (CC_WATCOM && (SYS_OS2 || SYS_MSDOS))
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

static char	current_dirname[NFILEN];

/* Return a string naming the current directory.  If we're unable to read the
 * directory name, return "."; otherwise we'll expect a fully-resolved path.
 */
const char *
current_directory(int force)
{
	char *s;
	static char *cwd;

	if (!force && cwd)
		return cwd;
#if HAVE_GETCWD
	cwd = getcwd(current_dirname, NFILEN);
#else
# if HAVE_GETWD
	cwd = getwd(current_dirname);
# else
	{
		FILE *f;
		int n;
		f = npopen("pwd", "r");
		if (f != NULL) {
			n = fread(current_dirname, 1, NFILEN, f);
			current_dirname[n] = EOS;
			npclose(f);
			cwd = current_dirname;
		} else
			cwd = 0;
	}
# endif
#endif
	if (cwd == NULL) {
		cwd = current_dirname;
		current_dirname[0] = '.';
		current_dirname[1] = EOS;
	} else {
		s = strchr(cwd, '\n');
		if (s)
			*s = EOS;
	}
#if OPT_MSDOS_PATH
	lengthen_path(cwd);
#if !OPT_CASELESS
	mklower(cwd);
#endif
#endif

#if SYS_MSDOS || SYS_OS2
	update_dos_drv_dir(cwd);
#endif

	TRACE(("current_directory(%s)\n", cwd))
	return cwd;
}

#if SYS_MSDOS || SYS_OS2

/* convert drive index to _letter_ */
static int
drive2char(unsigned d)
{
	if (d >= 26) {
		mlforce("[Illegal drive index %d]", d);
		d = 0;
	}
	return (d + 'A');
}

/* convert drive _letter_ to index */
static unsigned
char2drive(int d)
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
curdrive(void)
{
#if SYS_OS2
	unsigned d;
# if CC_CSETPP
	d = _getdrive();
# else
	_dos_getdrive(&d);  	/* A=1 B=2 ... */
# endif
	return drive2char((d-1) & 0xff);
#else
	return drive2char(bdos(0x19, 0, 0) & 0xff);
#endif
}

/* take drive _letter_ as arg. */
int
setdrive(int d)
{
	if (isalpha(d)) {
#if SYS_OS2
# if CC_CSETPP
		_chdrive(char2drive(d) + 1);
# else
		unsigned maxdrives;
		_dos_setdrive(char2drive(d)+1, &maxdrives); /* 1 based */
# endif
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

const char *
curr_dir_on_drive(int drive)
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
update_dos_drv_dir(const char *cwd)
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
cd(int f, int n)
{
	register int status;
	static	TBUFF	*last;
	char cdirname[NFILEN];

	status = mlreply_dir("Change to directory: ", &last, cdirname);
#if SYS_UNIX || SYS_VMS
	if (status == FALSE) {		/* empty reply, go HOME */
		(void)strcpy(cdirname, "~");
		status = TRUE;
	}
#endif
	if (status == TRUE)
		status = set_directory(cdirname);
	return status;
}

/* ARGSUSED */
int
pwd(int f, int n)
{
	mlforce("%s",current_directory(f));
	return TRUE;
}

static char prevdir[NFILEN];

static int
cd_and_pwd(const char *path)
{
#if OPT_PROCEDURES
	static int cdhooking;
#endif
#if CC_CSETPP
	if (_chdir(SL_TO_BSL(path)) == 0)
#else
	if (chdir(SL_TO_BSL(path)) == 0)
#endif
	{
#if SYS_UNIX
		const char *p = current_directory(TRUE);
		if (!is_slashc(*p)) {
			if (is_slashc(*prevdir))
				p = lengthen_path(pathcat(current_dirname, prevdir, path));
			sgarbf = TRUE;	/* some shells print to stderr */
			update(TRUE);
		}

		mlforce("%s", p);
#else
		(void)pwd(TRUE,1);
#endif
#if OPT_PROCEDURES
		if (!cdhooking && *cdhook) {
			cdhooking = TRUE;
			run_procedure(cdhook);
			cdhooking = FALSE;
		}
#endif
		updatelistbuffers();
		return TRUE;
	}
	return FALSE;
}

#if OPT_EVAL
const char *
previous_directory(void)
{
	if (*prevdir)
		return prevdir;
	else
		return current_directory(FALSE);
}
#endif

/* move to the named directory.  (Dave Lemke) */
int
set_directory(const char *dir)
{
    char       exdir[NFILEN];
#if SYS_UNIX
    const char *cdpath;
    char       cdpathdir[NFILEN];
#endif
    char *exdp;
#if SYS_MSDOS || SYS_OS2
    int curd = curdrive();
#endif

    upmode();

    TRACE(("set_directory(%s)\n", dir))
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

#if OPT_VMS_PATH
	if (!*exdp)
		strcpy(exdp, "~");
	if (!strcmp(exdp, "/"))		/* cannot really "cd /" on vms */
		lengthen_path(exdp);
	if (!is_vms_pathname(exdp,TRUE)) {
		int end = strlen(exdp);
		if (exdp[end-1] != SLASHC) {
			exdp[end++]  = SLASHC;
			exdp[end]    = EOS;
		}
		unix2vms_path(exdp,exdp);
	}
#endif

	if (cd_and_pwd(exdp))
		return TRUE;

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
			if (cd_and_pwd(pathcat(cdpathdir, cdpathdir, exdp))) {
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
ch_fname(BUFFER *bp, const char *fname)
{
	int len;
	char nfilen[NFILEN];
	char *np;
	char *holdp = NULL;

	np = strcpy(nfilen, fname);

	/* produce a full pathname, unless already absolute or "internal" */
	if (!isInternalName(np))
		(void) lengthen_path(nfilen);

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
				bp->b_fname = (char *)out_of_mem;
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
