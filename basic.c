/*
 * The routines in this file move the cursor around on the screen. They
 * compute a new value for the cursor, then adjust ".". The display code
 * always updates the cursor location, so only moves between lines, or
 * functions that adjust the top line in the window and invalidate the
 * framing, are hard.
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/basic.c,v 1.92 1996/04/17 02:49:32 pgf Exp $
 *
 */

#include	"estruct.h"
#include	"edef.h"

#define	RegexpLen(exp) ((exp->mlen) ? (int)(exp->mlen) : 1)

static	int	getnmmarkname ( int *cp );
static	void	skipblanksb (void);
static	void	skipblanksf (void);

/* utility routine for 'forwpage()' and 'backpage()' */
static int
full_pages(int f, int n)
{
	if (f == FALSE) {
		n = curwp->w_ntrows - 2;	/* Default scroll.	*/
		if (n <= 0)			/* Don't blow up if the */
			n = 1;			/* window is tiny.	*/
	}
#if	OPT_CVMVAS
	else if (n > 0)				/* Convert from pages	*/
		n *= curwp->w_ntrows;		/* to lines.		*/
#endif
	return n;
}

/* utility routine for 'forwhpage()' and 'backhpage()' */
static int
half_pages(int f, int n)
{
	if (f == FALSE) {
		n = curwp->w_ntrows / 2;	/* Default scroll.	*/
		if (n <= 0)			/* Forget the overlap	*/
			n = 1;			/* if tiny window.	*/
	}
#if	OPT_CVMVAS
	else if (n > 0)				/* Convert from pages	*/
		n *= curwp->w_ntrows/2;		/* to lines.		*/
#endif
	return n;
}

/*
 * Implements the vi "0" command.
 *
 * Move the cursor to the beginning of the current line.
 */
/* ARGSUSED */
int
gotobol(int f, int n)
{
	DOT.o  = w_left_margin(curwp);
	return mvleftwind(TRUE, -w_val(curwp,WVAL_SIDEWAYS));
}

/*
 * Move the cursor backwards by "n" characters. If "n" is less than zero call
 * "forwchar" to actually do the move. Otherwise compute the new cursor
 * location. Error if you try and move out of the buffer. Set the flag if the
 * line pointer for dot changes.
 */
int
backchar(int f, int n)
{
	register LINE	*lp;

	if (f == FALSE) n = 1;
	if (n < 0)
		return (forwchar(f, -n));
	while (n--) {
		if (DOT.o == w_left_margin(curwp)) {
			if ((lp=lback(DOT.l)) == buf_head(curbp))
				return (FALSE);
			DOT.l  = lp;
			DOT.o  = llength(lp);
			curwp->w_flag |= WFMOVE;
		} else
			DOT.o--;
	}
	return (TRUE);
}

/*
 * Implements the vi "h" command.
 *
 * Move the cursor backwards by "n" characters. Stop at beginning of line.
 */
int
backchar_to_bol(int f, int n)
{

	if (f == FALSE) n = 1;
	if (n < 0)
		return forwchar_to_eol(f, -n);
	while (n--) {
		if (DOT.o == w_left_margin(curwp))
			return doingopcmd;
		else
			DOT.o--;
	}
	return TRUE;
}

/*
 * Implements the vi "$" command.
 *
 * Move the cursor to the end of the current line.  Trivial.
 */
int
gotoeol(int f, int n)
{
	if (f == TRUE) {
		if (n > 0)
			--n;
		else if (n < 0)
			++n;
		if (forwline(f,n) != TRUE)
			return FALSE;
	}
	DOT.o  = llength(DOT.l);
	curgoal = HUGE;
	return (TRUE);
}

/*
 * Move the cursor forwards by "n" characters. If "n" is less than zero call
 * "backchar" to actually do the move. Otherwise compute the new cursor
 * location, and move ".". Error if you try and move off the end of the
 * buffer. Set the flag if the line pointer for dot changes.
 */
int
forwchar(int f, int n)
{
	if (f == FALSE) n = 1;
	if (n < 0)
		return (backchar(f, -n));
	while (n--) {
		/* if an explicit arg was given, allow us to land
			on the newline, else skip it */
		if (is_at_end_of_line(DOT) || 
				(f == FALSE && !insertmode &&
				llength(DOT.l) && DOT.o == llength(DOT.l) - 1)
								) {
			if (is_header_line(DOT, curbp) ||
					is_last_line(DOT,curbp))
				return (FALSE);
			DOT.l  = lforw(DOT.l);
			DOT.o  = w_left_margin(curwp);
			curwp->w_flag |= WFMOVE;
		} else
			DOT.o++;
	}
	return (TRUE);
}


/*
 * Implements the vi "l" command.
 *
 * Move the cursor forwards by "n" characters. Don't go past end-of-line
 *
 * If the flag 'doingopcmd' is set, implements a vi "l"-like motion for
 * internal use.  The end-of-line test is off-by-one from the true "l" command
 * to allow for substitutions at the end of a line.
 */
int
forwchar_to_eol(int f, int n)
{
	int nwas = n;
	int lim;
	if (f == FALSE) n = 1;
	if (n < 0) return backchar_to_bol(f, -n);
	if (n == 0) return TRUE;

	/* normally, we're confined to the text on the line itself.  if
	  we're doing an opcmd, then we're allowed to move to the newline
	  as well, to take care of the internal cases:  's', 'x', and '~'. */
	if (doingopcmd || insertmode)
		lim = llength(DOT.l);
	else
		lim = llength(DOT.l) - 1;
	do {
		if (DOT.o >= lim)
			return n != nwas; /* return ok if we moved at all */
		else
			DOT.o++;
	} while (--n);
	return TRUE;
}

/*
 * Implements the vi "G" command.
 *
 * Move to a particular line (the argument).  Count from bottom of file if
 * argument is negative.
 */
int
gotoline(int f, int n)
{
	register int status;	/* status return */

	MARK odot;

	if (f == FALSE) {
		return(gotoeob(f,n));
	}

	if (n == 0)		/* if a bogus argument...then leave */
		return(FALSE);

	odot = DOT;

	DOT.o  = w_left_margin(curwp);
	if (n < 0) {
		DOT.l  = lback(buf_head(curbp));
		status = backline(f, -n - 1 );
	} else {
		DOT.l  = lforw(buf_head(curbp));
		status = forwline(f, n-1);
	}
	if (status != TRUE) {
		DOT = odot;
		return status;
	}
	(void)firstnonwhite(FALSE,1);
	curwp->w_flag |= WFMOVE;
	return TRUE;
}
/*
 * Goto the beginning of the buffer. Massive adjustment of dot. This is
 * considered to be hard motion; it really isn't if the original value of dot
 * is the same as the new value of dot.
 */
/* ARGSUSED */
int
gotobob(int f, int n)
{
	DOT.l  = lforw(buf_head(curbp));
	DOT.o  = w_left_margin(curwp);
	curwp->w_flag |= WFMOVE;
	return (TRUE);
}

/*
 * Move to the end of the buffer. Dot is always put at the end of the file.
 */
/* ARGSUSED */
int
gotoeob(int f, int n)
{
	DOT.l  = lback(buf_head(curbp));
	curwp->w_flag |= WFMOVE;
	return firstnonwhite(FALSE,1);
}

/*
 * Implements the vi "H" command.
 *
 * Move to first (or nth) line in window
 */
int
gotobos(int f, int n)
{
	int	nn = curwp->w_ntrows;
	if (!f || n <= 0)
		n = 1;

	DOT.l = curwp->w_line.l;
	while (--n) {
		if (is_last_line(DOT,curbp))
			break;
		nn -= line_height(curwp, DOT.l);
		DOT.l = lforw(DOT.l);
	}

	if (nn <= 0)		/* we went past the end of window */
		curwp->w_flag |= WFMOVE;
	return firstnonwhite(FALSE,1);
}

/*
 * Implements the vi "M" command.
 *
 * Move to the middle of lines displayed in window
 */
/* ARGSUSED */
int
gotomos(int f, int n)
{
	register LINEPTR lp, head;
	int	half = (curwp->w_ntrows+1) / 2;

	head = buf_head(curbp);
	for (n = 0, lp = curwp->w_line.l; lp != head; lp = lforw(lp)) {
		if (n < half)
			DOT.l = lp;
		if ((n += line_height(curwp, lp)) >= curwp->w_ntrows)
			break;
	}
	if (n < curwp->w_ntrows) {	/* then we hit eof before eos */
		half = (n+1) / 2;	/* go back up */
		for (n = 0, lp = curwp->w_line.l; lp != head; lp = lforw(lp)) {
			DOT.l = lp;
			if ((n += line_height(curwp, lp)) >= half)
				break;
		}
	}

	return firstnonwhite(FALSE,1);
}

/*
 * Implements the vi "L" command.
 *
 * Move to the last (or nth last) line in window
 */
int
gotoeos(int f, int n)
{
	int nn;
	if (f == FALSE || n <= 0)
		n = 1;

	/* first get to the end */
	DOT.l = curwp->w_line.l;
	nn = curwp->w_ntrows;
	while ((nn -= line_height(curwp,DOT.l)) > 0) {
		if (is_last_line(DOT,curbp))
			break;
		DOT.l = lforw(DOT.l);
	}
#ifdef WMDLINEWRAP
	/* adjust if we pointed to a line-fragment */
	if (w_val(curwp,WMDLINEWRAP)
	 && nn < 0
	 && DOT.l != curwp->w_line.l)
		DOT.l = lback(DOT.l);
#endif
	/* and then go back up */
	/* (we're either at eos or eof) */
	while (--n) {
		if (sameline(DOT, curwp->w_line))
			break;
		DOT.l = lback(DOT.l);
	}
	return firstnonwhite(FALSE,1);
}

/*
 * Implements the vi "j" command.
 *
 * Move forward by full lines. If the number of lines to move is less than
 * zero, call the backward line function to actually do it. The last command
 * controls how the goal column is set.
 */
int
forwline(int f, int n)
{
	register LINE	*dlp;

	if (f == FALSE) n = 1;
	if (n < 0) return (backline(f, -n));
	if (n == 0) return TRUE;

	/* if the last command was not a line move,
	   reset the goal column */
	if (curgoal < 0)
		curgoal = getccol(FALSE);

	/* and move the point down */
	dlp = DOT.l;
	do {
		register LINE *nlp = lforw(dlp);
		if (nlp == buf_head(curbp)) {
			return FALSE;
		}
		dlp = nlp;
	} while (--n);

	/* resetting the current position */
	DOT.l  = dlp;
	DOT.o  = getgoal(dlp);
	curwp->w_flag |= WFMOVE;
	return TRUE;
}
/*
 * Implements the vi "^" command.
 *
 * Move to the first nonwhite character on the current line.  No errors are
 * returned.
 */
/* ARGSUSED */
int
firstnonwhite(int f, int n)
{
	DOT.o  = firstchar(DOT.l);
	if (DOT.o < w_left_margin(curwp)) {
		if (llength(DOT.l) <= w_left_margin(curwp))
			DOT.o = w_left_margin(curwp);
		else
			DOT.o = llength(DOT.l) - 1;
	}
	return TRUE;
}

/* ARGSUSED */
#if !SMALLER
int
lastnonwhite(int f, int n)
{
	DOT.o  = lastchar(DOT.l);
	if (DOT.o < w_left_margin(curwp))
		DOT.o = w_left_margin(curwp);
	return TRUE;
}
#endif

/* return the offset of the first non-white character on the line,
	or -1 if there are no non-white characters on the line */
int
firstchar(LINE *lp)
{
	int off = w_left_margin(curwp);
	while ( off < llength(lp) && isblank(lgetc(lp, off)) )
		off++;
	if (off == llength(lp))
		return -1;
	return off;
}

/* return the offset of the next non-white character on the line,
	or -1 if there are no more non-white characters on the line */
int
nextchar(LINE *lp, int off)
{
	while (off < llength(lp)) {
		if (!isspace(lgetc(lp,off)))
			return off;
		off++;
	}
	return -1;
}

/* return the offset of the last non-white character on the line
	or -1 if there are no non-white characters on the line */
int
lastchar(LINE *lp)
{
	int off = llength(lp)-1;
	while ( off >= 0 && isspace(lgetc(lp, off)) )
		off--;
	return off;
}

/*
 * Implements the vi "^M" command.
 *
 * Like 'forwline()', but goes to the first non-white character position.
 */
int
forwbline(int f, int n)
{
	int s;

	if (f == FALSE) n = 1;
	if ((s = forwline(f,n)) != TRUE)
		return (s);
	return firstnonwhite(FALSE,1);
}

/*
 * Implements the vi "-" command.
 *
 * Like 'backline()', but goes to the first non-white character position.
 */
int
backbline(int f, int n)
{
	int s;

	if (f == FALSE) n = 1;
	if ((s = backline(f,n)) != TRUE)
		return (s);
	return firstnonwhite(FALSE,1);
}

/*
 * Implements the vi "k" command.
 *
 * This function is like "forwline", but goes backwards.
 */
int
backline(int f, int n)
{
	register LINE	*dlp;

	if (f == FALSE) n = 1;
	if (n < 0)
		return (forwline(f, -n));

	/* if we are on the first line as we start....fail the command */
	if (is_first_line(DOT, curbp))
		return(FALSE);

	/* if the last command was not note a line move,
	   reset the goal column */
	if (curgoal < 0)
		curgoal = getccol(FALSE);

	/* and move the point up */
	dlp = DOT.l;
	while (n-- && lback(dlp) != buf_head(curbp))
		dlp = lback(dlp);

	/* reseting the current position */
	DOT.l  = dlp;
	DOT.o  = getgoal(dlp);
	curwp->w_flag |= WFMOVE;
	return (TRUE);
}

/*
 * Go to the beginning of the current paragraph.
 */
int
gotobop(int f, int n)
{
	MARK odot;
	int was_on_empty;
	int fc;

	if (!f) n = 1;

	was_on_empty = is_empty_line(DOT);
	odot = DOT;

	fc = firstchar(DOT.l);
	if (doingopcmd &&
		((fc >= 0 && DOT.o <= fc) || fc < 0) &&
		!is_first_line(DOT,curbp)) {
		backchar(TRUE,DOT.o+1);
		pre_op_dot = DOT;
	}
	while (n) {
		if (findpat(TRUE, 1, b_val_rexp(curbp,VAL_PARAGRAPHS)->reg,
							REVERSE) != TRUE) {
			(void)gotobob(f,n);
		} else if (is_empty_line(DOT)) {
			/* special case -- if we found an empty line,
				and it's adjacent to where we started,
				skip all adjacent empty lines, and try again */
			if ( (was_on_empty && lforw(DOT.l) == odot.l) ||
				(n > 0 && llength(lforw(DOT.l)) == 0) ) {
				/* then we haven't really found what we
					wanted.  keep going */
				skipblanksb();
				continue;
			}
		}
		n--;
	}
	if (doingopcmd) {
		fc = firstchar(DOT.l);
		if (!sameline(DOT,odot) &&
			(pre_op_dot.o > lastchar(pre_op_dot.l)) &&
			((fc >= 0 && DOT.o <= fc) || fc < 0)) {
			regionshape = FULLLINE;
		}
	}
	return TRUE;
}

/*
 * Go to the end of the current paragraph.
 */
int
gotoeop(int f, int n)
{
	MARK odot;
	int was_at_bol;
	int was_on_empty;
	int fc;

	if (!f) n = 1;

	fc = firstchar(DOT.l);
	was_on_empty = is_empty_line(DOT);
	was_at_bol = ((fc >= 0 && DOT.o <= fc) || fc < 0);
	odot = DOT;

	while (n) {
		if (findpat(TRUE, 1, b_val_rexp(curbp,VAL_PARAGRAPHS)->reg,
						FORWARD) != TRUE) {
			DOT = curbp->b_line;
		} else if (is_empty_line(DOT)) {
			/* special case -- if we found an empty line. */
			/* either as the very next line, or at the end of
				our search */
			if ( (was_on_empty && lback(DOT.l) == odot.l) ||
				(n > 0 && llength(lback(DOT.l)) == 0) ) {
				/* then we haven't really found what we
					wanted.  keep going */
				skipblanksf();
				continue;
			}
		}
		n--;
	}
	if (doingopcmd) {
		/* if we're now at the beginning of a line and we can back up,
		  do so to avoid eating the newline and leading whitespace */
		fc = firstchar(DOT.l);
		if (((fc >= 0 && DOT.o <= fc) || fc < 0) &&
			!is_first_line(DOT,curbp) &&
			!sameline(DOT,odot) ) {
			backchar(TRUE,DOT.o+1);
		}
		/* if we started at the start of line, eat the whole line */
		if (!sameline(DOT,odot) && was_at_bol)
			regionshape = FULLLINE;
	}
	return TRUE;
}

static void
skipblanksf(void)
{
	while (lforw(DOT.l) != buf_head(curbp) && is_empty_line(DOT))
		DOT.l = lforw(DOT.l);
}

static void
skipblanksb(void)
{
	while (lback(DOT.l) != buf_head(curbp) && is_empty_line(DOT))
		DOT.l = lback(DOT.l);
}

#if OPT_STUTTER_SEC_CMD
getstutter(void)
{
	int thiskey;
	if (!clexec) {
		thiskey = lastkey;
		kbd_seq();
		if (thiskey != lastkey) {
			return FALSE;
		}
	}
	return TRUE;
}
#endif

/*
 * Go to the beginning of the current section (or paragraph if no section
 * marker found).
 */
int
gotobosec(int f, int n)
{
#if OPT_STUTTER_SEC_CMD
	if (!getstutter())
		return FALSE;
#endif
	if (findpat(f, n, b_val_rexp(curbp,VAL_SECTIONS)->reg,
							REVERSE) != TRUE) {
		(void)gotobob(f,n);
	}
	return TRUE;
}

/*
 * Go to the end of the current section (or paragraph if no section marker
 * found).
 */
int
gotoeosec(int f, int n)
{
#if OPT_STUTTER_SEC_CMD
	if (!getstutter())
		return FALSE;
#endif
	if (findpat(f, n, b_val_rexp(curbp,VAL_SECTIONS)->reg,
							FORWARD) != TRUE) {
		DOT = curbp->b_line;
	}
	return TRUE;
}

/*
 * Go to the beginning of the current sentence. If we skip into an empty line
 * (from a non-empty line), return at that point -- that's what vi does.
 */
int
gotobosent(int f, int n)
{
	MARK savepos;
	int looped = 0;
	int extra;
	int empty = is_empty_line(DOT);
	register regexp *exp;
	register int s = TRUE;

	savepos = DOT;
	exp = b_val_rexp(curbp,VAL_SENTENCES)->reg;

	while (s && (is_at_end_of_line(DOT) || isspace(char_at(DOT)))) {
		s = backchar(TRUE,1);
		if (is_at_end_of_line(DOT) && !empty)
			return TRUE;
	}
 top:
	extra = 0;
	if (findpat(f, n, exp, REVERSE) != TRUE) {
		return gotobob(f,n);
	}
	s = forwchar(TRUE, RegexpLen(exp));
	while (s && (is_at_end_of_line(DOT) || isspace(char_at(DOT)))) {
		s = forwchar(TRUE,1);
		extra++;
	}
	if (n == 1 && samepoint(savepos,DOT)) { /* try again */
		if (looped > 10)
			return FALSE;
		s = backchar(TRUE, RegexpLen(exp) + extra + looped);
		while (s && is_at_end_of_line(DOT)) {
			if (!empty && is_empty_line(DOT))
				return TRUE;
			s = backchar(TRUE,1);
		}
		looped++;
		goto top;

	}
	return TRUE;
}

/*
 * Go to the end of the current sentence.  Like gotobosent(), if we skip into
 * an empty line, return at that point.
 */
int
gotoeosent(int f, int n)
{
	register regexp *exp;
	register int s;
	int empty = is_empty_line(DOT);

	exp = b_val_rexp(curbp,VAL_SENTENCES)->reg;
	/* if we're on the end of a sentence now, don't bother scanning
		further, or we'll miss the immediately following sentence */
	if (!(lregexec(exp, DOT.l, DOT.o, llength(DOT.l)) &&
				exp->startp[0] - DOT.l->l_text == DOT.o)) {
		if (findpat(f, n, exp, FORWARD) != TRUE) {
			DOT = curbp->b_line;
			return TRUE;
		} else {
			if (!empty && is_at_end_of_line(DOT))
				return TRUE;
		}
	}
	s = forwchar(TRUE, RegexpLen(exp));
	while (s && (is_at_end_of_line(DOT) || isspace(char_at(DOT)))) {
		s = forwchar(TRUE,1);
	}
	return TRUE;
}


/*
 * This routine, given a pointer to a LINE, and the current cursor goal
 * column, return the best choice for the offset. The offset is returned.
 * Used by "C-N" and "C-P".
 */
int
getgoal(LINE *dlp)
{
	register int	c;
	register int	col;
	register int	newcol;
	register int	dbo;

	col = 0;
	dbo = w_left_margin(curwp);
	while (dbo < llength(dlp)) {
		c = lgetc(dlp, dbo);
		newcol = next_column(c,col);
		if (newcol > curgoal)
			break;
		col = newcol;
		++dbo;
	}
	return (dbo);
}

/* return the next column index, given the current char and column */
int
next_column(int c, int col)
{
	if (c == '\t')
		return nextab(col);
	else if (!isprint(c))
		return col+2;
	else
		return col+1;
}

/*
 * Implements the vi "^F" command.
 *
 * Scroll forward by a specified number of lines, or by a full page if no
 * argument.
 */
int
forwpage(int f, int n)
{
	register LINEPTR lp;
	int	status;

	if ((n = full_pages(f,n)) < 0)
		return backpage(f, -n);

	if ((status = (lforw(DOT.l) != buf_head(curbp))) == TRUE) {
		lp = curwp->w_line.l;
		n -= line_height(curwp,lp);
		while (lp != buf_head(curbp)) {
			lp = lforw(lp);
			if ((n -= line_height(curwp,lp)) < 0)
				break;
		}
		if (n < 0)
			curwp->w_line.l = lp;
		DOT.l  = lp;
		(void)firstnonwhite(FALSE,1);
		curwp->w_flag |= WFHARD|WFMODE;
	}
	return status;
}

/*
 * Implements the vi "^B" command.
 *
 * This command is like "forwpage", but it goes backwards.
 */
int
backpage(int f, int n)
{
	register LINEPTR lp;
	int	status;

	if ((n = full_pages(f,n)) < 0)
		return forwpage(f, -n);

	lp = curwp->w_line.l;
	if (lback(lp) != buf_head(curbp)) {
		while ((n -= line_height(curwp,lp)) >= 0
		  &&   lback(lp) != buf_head(curbp))
			lp = lback(lp);
		curwp->w_line.l = lp;
		(void)gotoeos(FALSE,1);
		curwp->w_flag |= WFHARD|WFMODE;
		status = TRUE;
	} else if (DOT.l != lp) {
		DOT.l = lp;
		curwp->w_flag |= WFHARD|WFMODE;
		status = TRUE;
	} else {
		status = FALSE;
	}
	return status;
}

/*
 * Implements the vi "^D" command.
 *
 * Scroll forward by a half-page.  If a repeat count is given, interpret that
 * as the number of half-pages to scroll.
 *
 * Unlike vi, the OPT_CVMVAS option causes the repeat-count to be interpreted as
 * half-page, rather than lines.
 */
int
forwhpage(int f, int n)
{
	register LINEPTR  llp, dlp;
	int	status;

	if ((n = half_pages(f,n)) < 0)
		return backhpage(f, -n);

	llp = curwp->w_line.l;
	dlp = DOT.l;
	if ((status = (lforw(dlp) != buf_head(curbp))) == TRUE) {
		n -= line_height(curwp,dlp);
		while (lforw(dlp) != buf_head(curbp)) {
			llp = lforw(llp);
			dlp = lforw(dlp);
			if ((n -= line_height(curwp,dlp)) < 0)
				break;
		}
		curwp->w_line.l = llp;
		DOT.l  = dlp;
		curwp->w_flag |= WFHARD|WFKILLS;
	}
	(void)firstnonwhite(FALSE,1);
	return status;
}

/*
 * Implements the vi "^U" command.
 *
 * This command is like "forwpage", but it goes backwards.  It returns false
 * only if the cursor is on the first line of the buffer.
 *
 * Unlike vi, the OPT_CVMVAS option causes the repeat-count to be interpreted as
 * half-pages, rather than lines.
 */
int
backhpage(int f, int n)
{
	register LINEPTR llp, dlp;
	int	status;

	if ((n = half_pages(f,n)) < 0)
		return forwhpage(f, -n);

	llp = curwp->w_line.l;
	dlp = DOT.l;
	if ((status = (lback(dlp) != buf_head(curbp))) == TRUE) {
		n -= line_height(curwp,dlp);
		while (lback(dlp) != buf_head(curbp)) {
			llp = lback(llp);
			dlp = lback(dlp);
			if ((n -= line_height(curwp,dlp)) < 0)
				break;
		}
		curwp->w_line.l = llp;
		DOT.l  = dlp;
		curwp->w_flag |= WFHARD|WFINS;
	}
	(void)firstnonwhite(FALSE,1);
	return status;
}

/*
 * Implements the vi "m" command.
 *
 * Set the named mark in the current window to the value of "." in the window.
 */
/* ARGSUSED */
int
setnmmark(int f, int n)
{
	int c,i;

	if (clexec || isnamedcmd) {
		int status;
		static char cbuf[2];
		if ((status=mlreply("Set mark: ", cbuf, 2)) != TRUE)
			return status;
		c = cbuf[0];
	} else {
		c = keystroke();
		if (ABORTED(c))
			return ABORT;
	}

	if (c < 'a' || c > 'z') {
		mlforce("[Invalid mark name]");
		return FALSE;
	}

	if (curbp->b_nmmarks == NULL) {
		curbp->b_nmmarks = typeallocn(struct MARK,26);
		if (curbp->b_nmmarks == NULL)
			return no_memory("named-marks");
		for (i = 0; i < 26; i++) {
			curbp->b_nmmarks[i] = nullmark;
		}
	}

	curbp->b_nmmarks[c-'a'] = DOT;
	mlwrite("[Mark %c set]",c);
	return TRUE;
}

/* ARGSUSED */
int
golinenmmark(int f, int n)
{
	int c;
	register int s;

	s = getnmmarkname(&c);
	if (s != TRUE)
		return s;
	s = gonmmark(c);
	if (s != TRUE)
		return s;

	return firstnonwhite(FALSE,1);

}

/* ARGSUSED */
int
goexactnmmark(int f, int n)
{
	int c;
	register int s;

	s = getnmmarkname(&c);
	if (s != TRUE)
		return s;

	return gonmmark(c);
}

/* ARGSUSED */
int
gorectnmmark(int f, int n)
{
	int c;
	register int s;

	s = getnmmarkname(&c);
	if (s != TRUE)
		return s;

	regionshape = RECTANGLE;
	return gonmmark(c);
}

/* get the name of the mark to use.  interactively, "last dot" is
	represented by stuttering the goto-mark command.  from
	the command line, it's always named ' or `.  I suppose
	this is questionable. */
static int
getnmmarkname(int *cp)
{
	int c;
	int thiskey;
	int useldmark;

	if (clexec || isnamedcmd) {
		int status;
		static char cbuf[2];
		if ((status=mlreply("Goto mark: ", cbuf, 2)) != TRUE)
			return status;
		c = cbuf[0];
		useldmark = (c == '\'' || c == '`');
	} else {
		thiskey = lastkey;
		c = keystroke();
		if (ABORTED(c))
			return ABORT;
		useldmark = (lastkey == thiskey);  /* usually '' or `` */
	}

	if (useldmark)
		c = '\'';

	*cp = c;
	return TRUE;
}

int
gonmmark(int c)
{
	register MARK *markp;
	MARK tmark;
	int found;

	if (!islower(c) && c != '\'') {
		mlforce("[Invalid mark name]");
		return FALSE;
	}

	markp = NULL;

	if (c == '\'') { /* use the 'last dot' mark */
		markp = &(curwp->w_lastdot);
	} else if (curbp->b_nmmarks != NULL) {
		markp = &(curbp->b_nmmarks[c-'a']);
	}

	found = FALSE;
	/* if we have any named marks, and the one we want isn't null */
	if (markp != NULL && !samepoint((*markp), nullmark)) {
		register LINE *lp;
		for_each_line(lp, curbp) {
			if ((*markp).l == lp) {
				found = TRUE;
				break;
			}
		}
	}
	if (!found) {
		mlforce("[Mark not set]");
		return (FALSE);
	}

	/* save current dot */
	tmark = DOT;

	/* move to the selected mark */
	DOT = *markp;

	if (!doingopcmd)	/* reset last-dot-mark to old dot */
		curwp->w_lastdot = tmark;

	curwp->w_flag |= WFMOVE;
	return (TRUE);
}

/*
 * Set the mark in the current window to the value of "." in the window. No
 * errors are possible.
 */
int
setmark(void)
{
	MK = DOT;
	return (TRUE);
}

/* ARGSUSED */
int
gomark(int f, int n)
{
	DOT = MK;
	curwp->w_flag |= WFMOVE;
	return (TRUE);
}

/* this odd routine puts us at the internal mark, plus an offset of lines */
/*  n == 1 leaves us at mark, n == 2 one line down, etc. */
/*  this is for the use of stuttered commands, and line oriented regions */
int
godotplus(int f, int n)
{
	int s;
	if (!f || n == 1) {
		return firstnonwhite(FALSE,1);
	}
	if (n < 1)
		return (FALSE);
	s = forwline(TRUE,n-1);
	if (s && is_header_line(DOT, curbp))
		s = backline(FALSE,1);
	if (s == TRUE)
		(void)firstnonwhite(FALSE,1);
	return s;
}

/*
 * Swap the values of "." and "mark" in the current window. This is pretty
 * easy, because all of the hard work gets done by the standard routine
 * that moves the mark about. The only possible error is "no mark".
 */
void
swapmark(void)
{
	MARK odot;

	if (samepoint(MK, nullmark)) {
		mlforce("BUG: No mark ");
		return;
	}
	odot = DOT;
	DOT = MK;
	MK = odot;
	curwp->w_flag |= WFMOVE;
	return;
}

#if OPT_MOUSE
/*
 * Given row & column from the screen, set the MK value.
 * The resulting position will not be past end-of-buffer unless the buffer
 * is empty.
 */
int
setwmark(int row, int col)
{
	MARK	save;
	register LINEPTR dlp;

	save = DOT;
	if (row == mode_row(curwp)) {
		(void) gotoeos(FALSE,1);
		DOT.l = lforw(DOT.l);
		DOT.o = w_left_margin(curwp);
	} else {	/* move to the right row */
		row -= curwp->w_toprow;
		dlp = curwp->w_line.l;	/* get pointer to 1st line */
		while ((row -= line_height(curwp,dlp)) >= 0
		  &&   dlp != buf_head(curbp))
			dlp = lforw(dlp);
		DOT.l = dlp;			/* set dot line pointer */

		/* now move the dot over until near the requested column */
#ifdef WMDLINEWRAP
		if (w_val(curwp,WMDLINEWRAP))
			col += term.t_ncol * (row+line_height(curwp,dlp));
#endif
		DOT.o = col2offs(curwp, dlp, col);

#if dont_allow_mouse_to_select_newline
		/* don't allow the cursor to be set past end of line unless we
		 * are in insert mode
		 */
		if (DOT.o >= llength(dlp) && DOT.o > w_left_margin(curwp) && 
					!insertmode)
			DOT.o--;
#endif
	}
	if (is_header_line(DOT, curwp->w_bufp)) {
		DOT.l = lback(DOT.l);
		DOT.o = llength(DOT.l);
	}
	MK  = DOT;
	DOT = save;
	return TRUE;
}

/*
 * Given row & column from the screen, set the curwp and DOT values.
 */
int
setcursor (int row, int col)
{
	register WINDOW *wp0 = curwp;
	register WINDOW *wp1;
	MARK saveMK;

	if ((wp1 = row2window(row)) == 0)
		return FALSE;
	if (doingsweep && curwp != wp1)
		return FALSE;
	saveMK = MK;
	if (set_curwp(wp1)
	 && setwmark(row, col)) {
		if (insertmode != FALSE
		 && b_val(wp1->w_bufp, MDVIEW)
		 && b_val(wp1->w_bufp, MDSHOWMODE)) {
			if (b_val(wp0->w_bufp, MDSHOWMODE))
				wp0->w_flag |= WFMODE;
			if (b_val(wp1->w_bufp, MDSHOWMODE))
				wp1->w_flag |= WFMODE;
			insertmode = FALSE;
		}
		DOT = MK;
		if (wp0 == wp1)
			MK = saveMK;
		curwp->w_flag |= WFMOVE;
		return TRUE;
	}

	return FALSE;
}
#endif
