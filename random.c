/*
 * This file contains the command processing functions for a number of random
 * commands. There is no functional grouping here, for sure.
 *
 * $Log: random.c,v $
 * Revision 1.65  1992/06/25 23:00:50  foxharp
 * changes for dos/ibmpc
 *
 * Revision 1.64  1992/05/29  09:40:53  foxharp
 * split out modes.c, fences.c, and insert.c from random.c
 *
 * Revision 1.63  1992/05/29  08:37:59  foxharp
 * getfence() changes for #ifdef matching and C comment matching
 *
 * Revision 1.62  1992/05/25  21:07:48  foxharp
 * extern func declarations moved to header
 *
 * Revision 1.61  1992/05/20  18:55:08  foxharp
 * better confirmation output from cd command
 *
 * Revision 1.60  1992/05/19  08:55:44  foxharp
 * more prototype and shadowed decl fixups
 *
 * Revision 1.59  1992/05/16  14:02:55  pgf
 * header/typedef fixups
 *
 * Revision 1.58  1992/05/16  12:00:31  pgf
 * prototypes/ansi/void-int stuff/microsoftC
 *
 * Revision 1.57  1992/05/13  09:14:50  pgf
 * when scanning for a fence character, don't go past end of line.
 * i hate do/while loops.
 *
 * Revision 1.56  1992/04/14  08:51:44  pgf
 * missing local var in DOS ifdef
 *
 * Revision 1.55  1992/03/24  22:45:09  pgf
 * allow ^D to back up past autoindented whitespace
 *
 * Revision 1.54  1992/03/24  07:45:08  pgf
 * made separate internal and external entry points for go[to]col, so that
 * users see columns starting at 1, and vile sees them starting from 0
 *
 * Revision 1.53  1992/03/19  23:23:31  pgf
 * removed extra string lib externs and include
 *
 * Revision 1.52  1992/03/05  09:19:55  pgf
 * changed some mlwrite() to mlforce(), due to new terse support
 *
 * Revision 1.51  1992/03/03  08:43:47  pgf
 * fixed off-by-one error in gotocol()
 *
 * Revision 1.50  1992/03/01  18:38:40  pgf
 * added fence matching on #if, #el, and #en, and
 * fixed compilation error #if COLOR
 *
 * Revision 1.49  1992/02/26  21:58:17  pgf
 * changes to entab/detabline, to better support shift operations
 *
 * Revision 1.48  1992/02/17  08:55:57  pgf
 * added showmode support, and
 * fixed backspace-after-wrapword bug, and
 * fixed '=' parsing in set commands, and
 * some small changes for saber cleanup
 *
 * Revision 1.47  1992/01/13  23:33:32  pgf
 * finished shiftwidth implementation -- ^D works now
 *
 * Revision 1.46  1992/01/10  08:08:46  pgf
 * added shiftwidth(), and some bug fixes to en/detabline()
 *
 * Revision 1.45  1992/01/05  00:06:13  pgf
 * split mlwrite into mlwrite/mlprompt/mlforce to make errors visible more
 * often.  also normalized message appearance somewhat.
 *
 * Revision 1.44  1992/01/03  23:31:49  pgf
 * use new ch_fname() to manipulate filenames, since b_fname is now
 * a malloc'ed sting, to avoid length limits
 *
 * Revision 1.43  1991/12/24  09:18:47  pgf
 * added current/change directory support  (Dave Lemke's changes)
 *
 * Revision 1.42  1991/11/16  18:39:50  pgf
 * pass new magic arg to regcomp, and
 * fixed bug in openup/opendown pair
 *
 * Revision 1.41  1991/11/13  20:09:27  pgf
 * X11 changes, from dave lemke
 *
 * Revision 1.40  1991/11/10  22:02:09  pgf
 * fixed bug in openup(), and made counts work right
 *
 * Revision 1.39  1991/11/08  14:13:58  pgf
 * fixed settab() and setfillcol()
 *
 * Revision 1.38  1991/11/08  13:10:26  pgf
 * lint cleanup, and
 * put in dave lemke's "autoinsert works going up" fix, and
 * made autoinsert clean up pre-existing leading whitespace
 *
 * Revision 1.37  1991/11/06  23:28:08  pgf
 * getfence() will scan for a fence if not on one to begin with.  it'll
 * scan in either direction, depending on arg to matchfence or matchfenceback
 *
 * Revision 1.36  1991/11/03  17:46:30  pgf
 * removed f,n args from all region functions -- they don't use them,
 * since they're no longer directly called by the user
 *
 * Revision 1.35  1991/11/01  14:38:00  pgf
 * saber cleanup
 *
 * Revision 1.34  1991/10/29  03:04:45  pgf
 * fixed argument mismatches
 *
 * Revision 1.33  1991/10/28  14:21:52  pgf
 * eliminated TABVAL and fillcol macros, converted gasave to per-buffer
 *
 * Revision 1.32  1991/10/27  01:51:48  pgf
 * new regex values are now settable and displayable
 *
 * Revision 1.31  1991/10/24  13:01:03  pgf
 * bug fix for append'ing in empty buffer
 *
 * Revision 1.30  1991/10/18  10:56:54  pgf
 * added code to use modified VALUE structures and lists to display settings
 * more easily (adjvalueset).  also removed some old ifdefs.
 *
 * Revision 1.29  1991/10/15  12:01:20  pgf
 * added backspacelimit support, and fewer tty chars are hard-coded now
 *
 * Revision 1.28  1991/10/08  01:09:38	pgf
 * added ^W and ^U processing to ins().  should make this depend on the tty
 * settings, and add option to control whether it's okay to backspace or
 * line or word kill back beyond the insert point
 *
 * Revision 1.27  1991/09/30  01:47:24	pgf
 * reformat listing of values a bit
 *
 * Revision 1.26  1991/09/26  13:13:07	pgf
 * created window values, and allow them to be displayed and set.  still needs
 * cleaning up
 *
 * Revision 1.25  1991/09/24  02:07:29	pgf
 * suppressed extra whitespace in listmodes
 *
 * Revision 1.24  1991/09/24  01:03:49	pgf
 * pass FALSE to gotoeol to avoid unnecessary work
 *
 * Revision 1.23  1991/09/23  01:57:20	pgf
 * n
 * don't pass null pointers to strcmp()
 *
 * Revision 1.22  1991/09/20  13:11:53	pgf
 * add local value settings to the listmodes output
 *
 * Revision 1.21  1991/09/19  13:39:53	pgf
 * added shortname synonyms to mode and value names
 *
 * Revision 1.20  1991/09/10  01:21:34	pgf
 * re-tabbed
 *
 * Revision 1.19  1991/09/10  00:43:31	pgf
 * ifdef'ed out obsolete "showm", and change calls to it to "listmodes"
 *
 * Revision 1.18  1991/08/16  11:12:26	pgf
 * added the third flavor of insertmode for replace-char, and
 * added support for the insertmode indicator on the modeline, and
 * created the catnap routine for use by fmatch, and the typahead
 * check in the ANSI_SPEC code in kbd_key
 *
 * Revision 1.17  91/08/13	12:51:29  pgf
 * make sure we pass a non-NULL arg to poll, even though the count is zero
 * 
 * Revision 1.16  1991/08/13  02:52:25	pgf
 * the fmatch() code now works if you have poll or select, and
 * is enabled by "set showmatch"
 *
 * Revision 1.15  1991/08/12  15:06:21	pgf
 * added ANSI_SPEC capability -- can now use the arrow keys from
 * command or insert mode
 *
 * Revision 1.14  1991/08/08  13:21:25	pgf
 * removed ifdef BEFORE
 *
 * Revision 1.13  1991/08/07  12:35:07	pgf
 * added RCS log messages
 *
 * revision 1.12
 * date: 1991/08/06 15:25:02;
 *	global/local values
 * and list changes
 * 
 * revision 1.11
 * date: 1991/06/26 09:35:48;
 * made count work correctly on flipchar, and
 * removed old ifdef BEFORE stuff
 * 
 * revision 1.10
 * date: 1991/06/25 19:53:09;
 * massive data structure restructure
 * 
 * revision 1.9
 * date: 1991/06/20 17:23:24;
 * fixed & --> && problem in indent_newline
 * 
 * revision 1.8
 * date: 1991/06/16 17:38:02;
 * switched to modulo tab calculations, and
 * converted entab, detab, and trim to work on regions,
 * and stripped some old ifdef NOCOUNT stuff for openup and opendown
 * and added support for local and globla tabstop and fillcol values
 * 
 * revision 1.7
 * date: 1991/06/15 09:09:27;
 * changed some forwchar calls to forwchar_to_eol, and
 * now prevent 'x', 's', 'r' from destroying the newline when used on
 * empty lines
 * 
 * revision 1.6
 * date: 1991/06/06 13:58:23;
 * added autoindent mode and "set all"
 * 
 * revision 1.5
 * date: 1991/05/31 11:19:06;
 * added showlength function, for "=" ex command, and
 * switched from stutterfunc to godotplus
 * 
 * revision 1.4
 * date: 1991/04/04 09:40:25;
 * fixed autoinsert bug
 * 
 * revision 1.3
 * date: 1991/02/12 09:49:33;
 * doindent had an unset return value, causing autoindents of 0 chars to fail
 * 
 * revision 1.2
 * date: 1990/10/03 16:01:00;
 * make backspace work for everyone
 * 
 * revision 1.1
 * date: 1990/09/21 10:25:54;
 * initial vile RCS revision
 */

#include	<stdio.h>
#include	"estruct.h"
#include	"edef.h"
#if HAVE_POLL
# include <poll.h>
#endif

extern CMDFUNC f_forwchar, f_backchar, f_forwchar_to_eol, f_backchar_to_bol;


/* generic "lister", which takes care of popping a window/buffer pair under
	the given name, and calling "func" with a couple of args to fill in
	the buffer */
int
liststuff(name,func,iarg,carg)
char *name;
void (*func)();		/* ptr to function to execute */
int iarg;
char *carg;
{
	register BUFFER *bp;
	register int	s;

	/* create the buffer list buffer   */
	bp = bfind(name, OK_CREAT, BFSCRTCH);
	if (bp == NULL)
		return FALSE;
	    
	if ((s=bclear(bp)) != TRUE) /* clear old text (?) */
		return (s);
	bp->b_flag |= BFSCRTCH;
	s = TRUE;
	if (!s || popupbuff(bp) == FALSE) {
		mlforce("[Can't list. ]");
		zotbuf(bp);
		return (FALSE);
	}
	/* call the passed in function, giving it both the integer and 
		character pointer arguments */
	(*func)(iarg,carg);
	gotobob(FALSE,1);
	{ char buf[80];
	  lsprintf(buf, "       %s   %s",prognam,version);
	  ch_fname(bp, buf);
	}
	bp->b_flag &= ~BFCHG;
	bp->b_active = TRUE;
	make_local_b_val(bp,MDVIEW);
	set_b_val(bp,MDVIEW,TRUE);
	make_local_b_val(bp,VAL_TAB);
	set_b_val(bp,VAL_TAB,8);
	make_local_b_val(bp,MDDOS);
	set_b_val(bp,MDDOS,FALSE);

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
	register long	numchars = 0;	/* # of chars in file */
	register int	numlines = 0;	/* # of lines in file */
	register long	predchars = 0;	/* # chars preceding point */
	register int	predlines = 0;	/* # lines preceding point */
	register int	curchar = '\n';	/* character under cursor */
	int ratio;
	int col;
	int savepos;			/* temp save for current offset */
	int ecol;			/* column pos/end of current line */

	/* starting at the beginning of the buffer */
	lp = lforw(curbp->b_line.l);

	/* start counting chars and lines */
	while (lp != curbp->b_line.l) {
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
		lp = lforw(lp);
	}

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
"Line %d of %d, Col %d of %d, Char %D of %D (%d%%) char is 0x%x",
		predlines+1, numlines, col+1, ecol,
		predchars+1, numchars, ratio, curchar);
	return TRUE;
}

/* ARGSUSED */
int
showlength(f,n)
int f,n;
{
	register LINE	*lp;		/* current line */
	register int	numlines = 0;	/* # of lines in file */

	/* starting at the beginning of the buffer */
	lp = lforw(curbp->b_line.l);
	while (lp != curbp->b_line.l) {
		++numlines;
		lp = lforw(lp);
	}
	mlforce("%d",numlines);
	return TRUE;
}

#if ! SMALLER
int
getcline()	/* get the current line number */
{
	register LINE	*lp;		/* current line */
	register int	numlines;	/* # of lines before point */

	/* starting at the beginning of the buffer */
	lp = lforw(curbp->b_line.l);

	/* start counting lines */
	numlines = 0;
	while (lp != curbp->b_line.l) {
		/* if we are on the current line, record it */
		if (lp == DOT.l)
			break;
		++numlines;
		lp = lforw(lp);
	}

	/* and return the resulting count */
	return(numlines + 1);
}
#endif

/*
 * Return current screen column.  Stop at first non-blank given TRUE argument.
 */
int
getccol(bflg)
int bflg;
{
	register int c, i, col;
	col = 0;
	for (i=0; i < DOT.o; ++i) {
		c = lgetc(DOT.l, i);
		if (!isspace(c) && bflg)
			break;
		col = next_column(c,col);
	}
	return col;
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

/* really set column, based on counting from 0, for internal use */
int
gocol(n)
int n;
{
	register int c; 	/* character being scanned */
	register int i; 	/* index into current line */
	register int col;	/* current cursor column   */
	register int llen;	/* length of line in bytes */

	col = 0;
	llen = llength(DOT.l);

	/* scan the line until we are at or past the target column */
	for (i = 0; i < llen; ++i) {
		/* upon reaching the target, drop out */
		if (col >= n)
			break;

		/* advance one character */
		c = lgetc(DOT.l, i);
		col = next_column(c,col);
	}

	/* set us at the new position */
	DOT.o = i;

	/* and tell whether we made it */
	return(col >= n);
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
	lchange(WFEDIT);
	return (TRUE);
}
#endif


/*
 * change all tabs in the line to the right number of spaces.
 * leadingonly says only do leading whitespace
 */
int
detabline(leadingonly)
int leadingonly;
{
	register int	s;
	register int	c;
	int	ocol;

	ocol = getccol(FALSE);

	DOT.o = 0;

	/* detab the entire current line */
	while (DOT.o < llength(DOT.l)) {
		c = char_at(DOT);
		if (leadingonly && !isspace(c))
			break;
		/* if we have a tab */
		if (c == '\t') {
			if ((s = ldelete(1L, FALSE)) != TRUE) {
				return s;
			}
			insspace( TRUE, curtabval - (DOT.o % curtabval) );
		}
		DOT.o++;
	}
	gocol(ocol);
	return TRUE;
}

/*
 * change all tabs in the region to the right number of spaces
 */
int
detab_region()
{
	return do_fl_region(detabline,FALSE);
}

/*
 * convert all appropriate spaces in the line to tab characters.
 * leadingonly says only do leading whitespace
 */
int
entabline(leadingonly)
int leadingonly;
{
	register int fspace;	/* pointer to first space if in a run */
	register int ccol;	/* current cursor column */
	register char cchar;	/* current character */
	int	ocol;

	ocol = getccol(FALSE);

	/* entab the current line */
	/* would this have been easier if it had started at
		the _end_ of the line, rather than the beginning?  -pgf */
	fspace = -1;
	ccol = 0;

	detabline(leadingonly);	/* get rid of possible existing tabs */
	DOT.o = 0;
	while (1) {
		/* see if it is time to compress */
		if ((fspace >= 0) && (nextab(fspace) <= ccol))
			if (ccol - fspace < 2)
				fspace = -1;
			else {
				backchar(TRUE, ccol - fspace);
				ldelete((long)(ccol - fspace), FALSE);
				linsert(1, '\t');	    
				fspace = -1;
			}

		if (DOT.o >= llength(DOT.l))
			break;

		/* get the current character */
		cchar = char_at(DOT);

		if (cchar == ' ') { /* a space...compress? */
			if (fspace == -1)
				fspace = ccol;
		} else {
			if (leadingonly)
				break;
			fspace = -1;
		}
		ccol++;
		DOT.o++;
	}
	gocol(ocol);
	return TRUE;
}

/*
 * convert all appropriate spaces in the region to tab characters
 */
int
entab_region()
{
	return do_fl_region(entabline,FALSE);
}

/* trim trailing whitespace from a line.  leave dot at end of line */
int
trimline()
{
	register int off, orig;
	register LINE *lp;
	    
	lp = DOT.l;
		    
	off = llength(lp)-1;
	orig = off;
	while (off >= 0) {
		if (!isspace(lgetc(lp,off)))
			break;
		off--;
	}
	    
	if (off == orig)
		return TRUE;

	DOT.o = off+1;
		    
	return ldelete((long)(orig - off),FALSE);
}

/*
 * trim trailing whitespace from a region
 */
int
trim_region()
{
	return do_fl_region(trimline,0);
}


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

	lp1 = DOT.l;
	while (llength(lp1)==0 && (lp2=lback(lp1))!=curbp->b_line.l)
		lp1 = lp2;
	lp2 = lp1;
	nld = 0;
	while ((lp2=lforw(lp2))!=curbp->b_line.l && llength(lp2)==0)
		++nld;
	if (nld == 0)
		return (TRUE);
	DOT.l = lforw(lp1);
	DOT.o = 0;
	return (ldelete(nld, FALSE));
}

#endif

/* '~' is synonymous with 'M-~<space>' */
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

	if (llength(DOT.l) == 0)
		return TRUE;

	havemotion = &f_gotoeol;
	return operdel(FALSE,1);
}

/* 'C' is synonymous with 'c$' */
/* ARGSUSED */
int
chgtoeol(f, n)
int f,n;
{
	extern CMDFUNC f_gotoeol;

	if (llength(DOT.l) == 0) {
		return ins();
	} else {
		havemotion = &f_gotoeol;
		return operchg(FALSE,1);
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
	mlforce("");
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
	char buf[NPAT]; 	/* buffer to recieve message into */

	buf[0] = 0;
	if ((status = mlreply("Message to write: ", buf, NPAT - 1)) != TRUE)
		return(status);

	/* write the message out */
	mlforce("%s",buf);
	return(TRUE);
}
#endif


void
catnap(milli)
int milli;
{
#if ! UNIX
	long i;
	for (i = 0; i < term.t_pause; i++)
		;
#else
# if HAVE_SELECT

	struct timeval tval;
	tval.tv_sec = 0;
	tval.tv_usec = milli * 1000;	/* microseconds */
	select (0, NULL, NULL, NULL, &tval);

# else
#  if HAVE_POLL

	struct pollfd pfd;
	poll(&pfd, 0, milli); /* milliseconds */

#  else

	sleep(1); /* 1 second.	ugh. */

#  endif
# endif
#endif
}

#if ! SMALLER
int
istring(f, n)	/* ask for and insert a string into the current
		   buffer at the current point */
int f, n;
{
	register char *tp;	/* pointer into string to add */
	register int status;	/* status return code */
	static char tstring[NPAT+1];	/* string to add */

	/* ask for string to insert */
	status = mlreply("String to insert: ", tstring, NPAT);
	if (status != TRUE)
		return(status);

	if (f == FALSE)
		n = 1;

	if (n < 0)
		n = - n;

	/* insert it */
	while (n--) {
		tp = &tstring[0];
		while (*tp) {
			if (*tp == '\n')
				status = lnewline();
			else
				status = linsert(1, *tp);
			++tp;
			if (status != TRUE)
				return(status);
		}
	}

	return(TRUE);
}
#endif

#if UNIX
#include	<sys/param.h>
#endif

/* return a string naming the current directory */
char *
current_directory()
{
	char *cwd, *s;
#if USG
	static char	dirname[NFILEN*2];
	FILE *f, *npopen();
	int n;
	f = npopen("/bin/pwd", "r");
	if (f == NULL) {
		fclose(f);
		return NULL;
	}
	n = fread(dirname, 1, NFILEN, f);

	dirname[n] = '\0';
	fclose(f);
	cwd = dirname;
#else
	static char	dirname[NFILEN*2];
# if MSDOS & MSC
	cwd = getcwd(dirname, NFILEN*2);
# else
	cwd = getwd(dirname);
# endif
#endif
	s = strchr(cwd, '\n');
	if (s)
		*s = '\0';
	return cwd;
}


/* ARGSUSED */
int
cd(f,n)
int f, n;
{
	int status;
	static char cdirname[NFILEN];
	status = mlreply("Change to directory: ", cdirname, NFILEN);
	if (status != TRUE) {
		/* should go HOME here */
		return status;
	}
	return set_directory(cdirname);
}

/* ARGSUSED */
int
pwd(f,n)
int f, n;
{
	mlforce("%s",current_directory());
	return TRUE;
}

/* move to the named directory.  adjust all non-absolute filenames
	to be absolute.
	(Dave Lemke)
*/
int
set_directory(dir)
char	*dir;
{
    char       exdir[NFILEN];
    BUFFER     *bp;
    char       *cwd;

    /* first fix up all the buffer file names */
    cwd = current_directory();
    if (!cwd || ! *cwd)
	return FALSE;
    for (bp = bheadp; bp; bp = bp->b_bufp) {
	/* ignore any that are already absolute or "internal" */
	if (bp->b_fname[0] == '/' ||
		bp->b_fname[0] == '\0' || 
		bp->b_fname[0] == '[' ||
		bp->b_fname[0] == '!') {
	    continue;
	}
	/* XXX file name is only 80 bytes -- could easily overflow here... */
	sprintf(exdir, "%s/%s", cwd, bp->b_fname );
	ch_fname(bp, exdir);
    }

    strcpy(exdir, dir);
    if (glob(exdir) && (chdir(exdir) == 0)) {
	pwd(TRUE,1);
	return TRUE;
    }
    mlforce("[Couldn't change to \"%s\"]", exdir);
    return FALSE;
}

