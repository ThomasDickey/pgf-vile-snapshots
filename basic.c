/*
 * The routines in this file move the cursor around on the screen. They
 * compute a new value for the cursor, then adjust ".". The display code
 * always updates the cursor location, so only moves between lines, or
 * functions that adjust the top line in the window and invalidate the
 * framing, are hard.
 *
 * $Log: basic.c,v $
 * Revision 1.49  1993/04/29 19:14:28  pgf
 * allow goto-named-mark command to be used from command line
 *
 * Revision 1.48  1993/04/28  17:11:22  pgf
 * got rid of NeWS ifdefs
 *
 * Revision 1.47  1993/04/20  12:18:32  pgf
 * see tom's 3.43 CHANGES
 *
 * Revision 1.46  1993/04/01  12:53:33  pgf
 * removed redundant includes and declarations
 *
 * Revision 1.45  1993/03/31  19:30:50  pgf
 * added firstnonwhite() calls to godotplus, so we can publish it as "whole-
 * lines"
 *
 * Revision 1.44  1993/03/18  17:42:20  pgf
 * see 3.38 section of CHANGES
 *
 * Revision 1.43  1993/03/16  10:53:21  pgf
 * see 3.36 section of CHANGES file
 *
 * Revision 1.42  1992/12/14  09:03:25  foxharp
 * lint cleanup, mostly malloc
 *
 * Revision 1.41  1992/12/05  13:12:16  foxharp
 * fix paragraph problem -- i didn't fix all the firstchar() calls before
 *
 * Revision 1.40  1992/12/04  09:08:45  foxharp
 * deleted unused assigns
 *
 * Revision 1.39  1992/11/30  23:06:03  foxharp
 * firstchar/lastchar now return -1 for no non-white chars in line
 *
 * Revision 1.38  1992/08/20  23:40:48  foxharp
 * typo fixes -- thanks, eric
 *
 * Revision 1.37  1992/08/04  20:09:03  foxharp
 * prototype fixups, for xvile
 *
 * Revision 1.36  1992/05/16  12:00:31  pgf
 * prototypes/ansi/void-int stuff/microsoftC
 *
 * Revision 1.35  1992/03/22  10:54:41  pgf
 * fixed bad bug in gotoline
 *
 * Revision 1.34  1992/03/19  23:30:35  pgf
 * gotoline can now take neg. argument, to count from bottom of
 * buffer. (for finderr)
 *
 * Revision 1.33  1992/03/19  23:05:50  pgf
 * forwpage now sets WFMODE
 *
 * Revision 1.32  1992/03/13  08:12:53  pgf
 * new paragraph behavior wrt blank lines, once again
 *
 * Revision 1.31  1992/03/03  21:57:01  pgf
 * fixed loop at buffer top/bottom in gotoeosent
 *
 * Revision 1.30  1992/02/17  08:49:47  pgf
 * took out unused var for saber
 *
 * Revision 1.29  1992/01/22  17:15:25  pgf
 * minor change to blank-line-skip for backward paragraph motions
 *
 * Revision 1.28  1992/01/22  16:58:20  pgf
 * paragraph motions now treat consecutive blank lines as a single paragraph
 * delimeter (note that this is independent of what the regexp says)
 *
 * Revision 1.27  1992/01/10  08:10:15  pgf
 * don't bother with list mode in next_column(), since this should _always_
 * give the "unlist-moded" column
 *
 * Revision 1.26  1992/01/05  00:06:13  pgf
 * split mlwrite into mlwrite/mlprompt/mlforce to make errors visible more
 * often.  also normalized message appearance somewhat.
 *
 * Revision 1.25  1992/01/03  23:35:47  pgf
 * screen motions (goto[emb]os()) now unconditionally return TRUE, to
 * eliminate oddness when buffer doesn't fill window
 *
 * paragraph and section motions no longer fail at the bottom of the
 * buffer (operators wouldn't work)
 *
 * paragraph motions are treated more vi-like.  in particular, whether
 * a para motion is character or line oriented is determined by where
 * the motion starts and finishes.  thanks to Eric Krohn for this tip.
 * (forward, the motion is line oriented if it moves off the current line
 * and it started at the beginning of the line.  otherwise it's character
 * oriented.  backward, it's similar, but it's line oriented if it starts
 * at the beginning _or_ end of a line.)
 *
 * Revision 1.24  1991/12/24  18:32:28  pgf
 * don't reset lastdot mark if we're moving to a mark and it's on behalf
 * of an opcmd
 *
 * Revision 1.23  1991/11/13  20:09:27  pgf
 * X11 changes, from dave lemke
 *
 * Revision 1.22  1991/11/10  22:28:17  pgf
 * the goto{end,begin}of{para,sec,sentence} motions now return TRUE, whether
 * they really went to a para,sec, or sentence, or went to the beginning or
 * end of the buffer because there were no more paras, secs, or sentences.
 * this makes operators work right.
 *
 * Revision 1.21  1991/11/08  13:08:04  pgf
 * moved firstchar() here, created lastchar(), and
 * eliminated unused atmark()
 *
 * Revision 1.20  1991/11/03  17:33:20  pgf
 * use new lregexec() routine to check for patterns in lines
 *
 * Revision 1.19  1991/11/01  14:38:00  pgf
 * saber cleanup
 *
 * Revision 1.18  1991/11/01  14:10:35  pgf
 * matchlen is now part of a regexp, not global
 *
 * Revision 1.17  1991/10/28  00:57:31  pgf
 * cleaned the sentence motions some more
 *
 * Revision 1.16  1991/10/27  16:09:06  pgf
 * improved the sentence motions
 *
 * Revision 1.15  1991/10/26  00:12:34  pgf
 * section, paragraph, and new sentence motions are all regex based
 *
 * Revision 1.14  1991/09/27  02:48:16  pgf
 * remove unused automatics
 *
 * Revision 1.13  1991/09/26  13:05:45  pgf
 * undid forw/backline optimization, since it causes flags to not be set,
 * and moved LIST mode to window
 *
 * Revision 1.12  1991/09/24  01:04:33  pgf
 * forwline and backline now do nothing if passed 0 arg
 *
 * Revision 1.11  1991/09/19  12:22:57  pgf
 * paragraphs now end at nroff-style section boundaries as well
 *
 * Revision 1.10  1991/08/07  12:35:07  pgf
 * added RCS log messages
 *
 * revision 1.9
 * date: 1991/08/06 15:10:26;
 * bug fix in forwline, and
 * global/local values
 * 
 * revision 1.8
 * date: 1991/06/25 19:52:01;
 * massive data structure restructure
 * 
 * revision 1.7
 * date: 1991/06/20 17:22:42;
 * fixed write-to-const-string problem in setnmmark
 * 
 * revision 1.6
 * date: 1991/06/16 17:33:32;
 * added next_column() routine, along with converting to modulo tab processing
 * 
 * revision 1.5
 * date: 1991/06/15 09:08:44;
 * added new forwchar_to_eol, and backchar_to_bol
 * 
 * revision 1.4
 * date: 1991/06/03 10:17:45;
 * prompt for mark name in setnmmark if isnamedcmd
 * 
 * revision 1.3
 * date: 1991/05/31 10:29:06;
 * fixed "last dot" mark code, and
 * added godotplus() for the operators
 * 
 * revision 1.2
 * date: 1990/09/25 11:37:50;
 * took out old ifdef BEFORE code
 * 
 * revision 1.1
 * date: 1990/09/21 10:24:42;
 * initial vile RCS revision
 */

#include	"estruct.h"
#include        "edef.h"

/*
 * Move the cursor to the
 * beginning of the current line.
 * Trivial.
 */
/* ARGSUSED */
int
gotobol(f, n)
int f,n;
{
        curwp->w_dot.o  = 0;
        return (TRUE);
}

/*
 * Move the cursor backwards by "n" characters. If "n" is less than zero call
 * "forwchar" to actually do the move. Otherwise compute the new cursor
 * location. Error if you try and move out of the buffer. Set the flag if the
 * line pointer for dot changes.
 */
int
backchar(f, n)
int f;
register int n;
{
        register LINE   *lp;

	if (f == FALSE) n = 1;
        if (n < 0)
                return (forwchar(f, -n));
        while (n--) {
                if (curwp->w_dot.o == 0) {
                        if ((lp=lback(curwp->w_dot.l)) == curbp->b_line.l)
                                return (FALSE);
                        curwp->w_dot.l  = lp;
                        curwp->w_dot.o  = llength(lp);
                        curwp->w_flag |= WFMOVE;
                } else
                        curwp->w_dot.o--;
        }
        return (TRUE);
}

/*
 * Move the cursor backwards by "n" characters. Stop at beginning of line.
 */
int
backchar_to_bol(f, n)
int f;
register int    n;
{

	if (f == FALSE) n = 1;
        if (n < 0)
                return forwchar_to_eol(f, -n);
        while (n--) {
                if (curwp->w_dot.o == 0)
			return TRUE;
                else
                        curwp->w_dot.o--;
        }
        return TRUE;
}

/*
 * Move the cursor to the end of the current line. Trivial. No errors.
 */
int
gotoeol(f, n)
int f,n;
{
	if (f == TRUE) {
		if (n > 0)
			 --n;
		else if (n < 0)
			 ++n;
		forwline(f,n);
	}
        curwp->w_dot.o  = llength(curwp->w_dot.l);
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
forwchar(f, n)
int f;
register int    n;
{
	if (f == FALSE) n = 1;
        if (n < 0)
                return (backchar(f, -n));
        while (n--) {
                if (is_at_end_of_line(curwp->w_dot)) {
                        if (is_header_line(curwp->w_dot, curbp) ||
					is_last_line(curwp->w_dot,curbp))
                                return (FALSE);
                        curwp->w_dot.l  = lforw(curwp->w_dot.l);
                        curwp->w_dot.o  = 0;
                        curwp->w_flag |= WFMOVE;
                } else
                        curwp->w_dot.o++;
        }
        return (TRUE);
}

/*
 * Move the cursor forwards by "n" characters. Don't go past end-of-line
 */
int
forwchar_to_eol(f, n)
int f;
register int    n;
{
	if (f == FALSE) n = 1;
        if (n < 0)
                return backchar_to_bol(f, -n);
        while (n--) {
                if (is_at_end_of_line(curwp->w_dot))
			return TRUE;
                else
                        curwp->w_dot.o++;
        }
        return TRUE;
}

/* move to a particular line. */
/* count from bottom of file if negative */
int
gotoline(f, n)
int f,n;
{
	register int status;	/* status return */

	/* get an argument if one doesnt exist */
	if (f == FALSE) {
		return(gotoeob(f,n));
	}

	if (n == 0)		/* if a bogus argument...then leave */
		return(FALSE);

	curwp->w_dot.o  = 0;
	if (n < 0) {
		curwp->w_dot.l  = lback(curbp->b_line.l);
		status = backline(f, -n - 1 );
	} else {
		curwp->w_dot.l  = lforw(curbp->b_line.l);
		status = forwline(f, n-1);
	}
	if (status == TRUE)
		firstnonwhite(f,n);
	return(status);
}

/*
 * Goto the beginning of the buffer. Massive adjustment of dot. This is
 * considered to be hard motion; it really isn't if the original value of dot
 * is the same as the new value of dot.
 */
/* ARGSUSED */
int
gotobob(f, n)
int f,n;
{
        curwp->w_dot.l  = lforw(curbp->b_line.l);
        curwp->w_dot.o  = 0;
        curwp->w_flag |= WFMOVE;
        return (TRUE);
}

/*
 * Move to the end of the buffer. Dot is always put at the end of the file.
 */
/* ARGSUSED */
int
gotoeob(f, n)
int f,n;
{
        curwp->w_dot.l  = lback(curbp->b_line.l);
	firstnonwhite(FALSE,1);
        curwp->w_flag |= WFMOVE;
        return TRUE;
}

int
gotobos(f,n)
int f,n;
{
	if (f == FALSE || n <= 0) n = 1;
	curwp->w_dot.l = curwp->w_line.l;
	while (--n) {
		if (forwline(FALSE,1) != TRUE)
			break;
	}
	firstnonwhite(f,n);
	return TRUE;
}

/* ARGSUSED */
int
gotomos(f,n)
int f,n;
{
	return gotobos(TRUE,curwp->w_ntrows/2);
}

int
gotoeos(f,n)
int f,n;
{
	return gotobos(TRUE,curwp->w_ntrows-(f==TRUE? n-1:0));
}

/*
 * Move forward by full lines. If the number of lines to move is less than
 * zero, call the backward line function to actually do it. The last command
 * controls how the goal column is set. No errors are
 * possible.
 */
int
forwline(f, n)
int f,n;
{
        register LINE   *dlp;

	if (f == FALSE) n = 1;
        if (n < 0)
                return (backline(f, -n));

	/* if we are on the last line as we start....fail the command */
	if (is_last_line(curwp->w_dot, curbp))
		return(FALSE);

	/* if the last command was not a line move,
	   reset the goal column */
        if (curgoal < 0)
                curgoal = getccol(FALSE);

	/* and move the point down */
        dlp = curwp->w_dot.l;
        while (n-- && dlp!=curbp->b_line.l)
                dlp = lforw(dlp);

	/* resetting the current position */
        curwp->w_dot.l  = (dlp == curbp->b_line.l) ? lback(dlp) : dlp;
        curwp->w_dot.o  = getgoal(dlp);
        curwp->w_flag |= WFMOVE;
        return (TRUE);
}

/* ARGSUSED */
int
firstnonwhite(f,n)
int f,n;
{
        DOT.o  = firstchar(DOT.l);
	if (DOT.o < 0) {
		if (llength(DOT.l) == 0)
			DOT.o = 0;
		else
			DOT.o = llength(DOT.l) - 1;
	}
	return TRUE;
}

/* ARGSUSED */
int
lastnonwhite(f,n)
int f,n;
{
        DOT.o  = lastchar(DOT.l);
	if (DOT.o < 0)
		DOT.o = 0;
	return TRUE;
}

/* return the offset of the first non-white character on the line,
	or -1 if there are no non-white characters on the line */
int
firstchar(lp)
LINE *lp;
{
	int off = 0;
	while ( off != llength(lp) && isspace(lgetc(lp, off)) )
		off++;
	if (off == llength(lp))
		return -1;
	return off;
}

/* return the offset of the next non-white character on the line,
	or -1 if there are no more non-white characters on the line */
int
nextchar(lp,off)
LINE *lp;
int off;
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
lastchar(lp)
LINE *lp;
{
	int off = llength(lp)-1;
	while ( off >= 0 && isspace(lgetc(lp, off)) )
		off--;
	return off;
}

/* like forwline, but got to first non-white char position */
int
forwbline(f,n)
int f,n;
{
	int s;

	if (f == FALSE) n = 1;
	if ((s = forwline(f,n)) != TRUE)
		return (s);
	firstnonwhite(f,n);
	return(TRUE);
}

/* like backline, but got to first non-white char position */
int
backbline(f,n)
int f,n;
{
	int s;

	if (f == FALSE) n = 1;
	if ((s = backline(f,n)) != TRUE)
		return (s);
	firstnonwhite(f,n);
	return(TRUE);
}

/*
 * This function is like "forwline", but goes backwards. The scheme is exactly
 * the same. Check for arguments that are less than zero and call your
 * alternate. Figure out the new line and call "movedot" to perform the
 * motion. No errors are possible.
 */
int
backline(f, n)
int f,n;
{
        register LINE   *dlp;

	if (f == FALSE) n = 1;
        if (n < 0)
                return (forwline(f, -n));

	/* if we are on the first line as we start....fail the command */
	if (is_first_line(curwp->w_dot, curbp))
		return(FALSE);

	/* if the last command was not note a line move,
	   reset the goal column */
        if (curgoal < 0)
                curgoal = getccol(FALSE);

	/* and move the point up */
        dlp = curwp->w_dot.l;
        while (n-- && lback(dlp) != curbp->b_line.l)
                dlp = lback(dlp);

	/* reseting the current position */
        curwp->w_dot.l  = dlp;
        curwp->w_dot.o  = getgoal(dlp);
        curwp->w_flag |= WFMOVE;
        return (TRUE);
}

#if	WORDPRO


int
gotobop(f,n)
int f,n;
{
	MARK odot;
	int was_on_empty;
	int fc;

	if (!f) n = 1;

	was_on_empty = (llength(DOT.l) == 0);
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
			gotobob(f,n);
		} else if (llength(DOT.l) == 0) {
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
			fulllineregions = TRUE;
		}
	}
	return TRUE;
}

int
gotoeop(f,n)
int f,n;
{
	MARK odot;
	int was_at_bol;
	int was_on_empty;
	int fc;

	if (!f) n = 1;

	fc = firstchar(DOT.l);
	was_on_empty = (llength(DOT.l) == 0);
	was_at_bol = ((fc >= 0 && DOT.o <= fc) || fc < 0);
	odot = DOT;

	while (n) {
		if (findpat(TRUE, 1, b_val_rexp(curbp,VAL_PARAGRAPHS)->reg, 
						FORWARD) != TRUE) {
			DOT = curbp->b_line;
		} else if (llength(DOT.l) == 0) {
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
			fulllineregions = TRUE;
	}
	return TRUE;
}

void
skipblanksf()
{
	while (lforw(DOT.l) != curbp->b_line.l && llength(DOT.l) == 0)
		DOT.l = lforw(DOT.l);
}

void
skipblanksb()
{
	while (lback(DOT.l) != curbp->b_line.l && llength(DOT.l) == 0)
		DOT.l = lback(DOT.l);
}

#if STUTTER_SEC_CMD
getstutter()
{
	int this1key;
	if (!clexec) {
		this1key = last1key;
		kbd_seq();
		if (this1key != last1key) {
			TTbeep();
			return FALSE;
		}
	}
	return TRUE;
}
#endif

int
gotobosec(f,n)
int f,n;
{
#if STUTTER_SEC_CMD
	if (!getstutter())
		return FALSE;
#endif
	if (findpat(f, n, b_val_rexp(curbp,VAL_SECTIONS)->reg,
							REVERSE) != TRUE) {
		gotobob(f,n);
	}
	return TRUE;
}

int
gotoeosec(f,n)
int f,n;
{
#if STUTTER_SEC_CMD
	if (!getstutter())
		return FALSE;
#endif
	if (findpat(f, n, b_val_rexp(curbp,VAL_SECTIONS)->reg,
							FORWARD) != TRUE) {
		DOT = curbp->b_line;
	}
	return TRUE;
}

int
gotobosent(f,n)
int f,n;
{
	MARK savepos;
	int looped = 0;
	int extra;
	register regexp *exp;
	register int s;

	savepos = DOT;
	exp = b_val_rexp(curbp,VAL_SENTENCES)->reg;
 top:
	extra = 0;
	if (findpat(f, n, exp, REVERSE) != TRUE) {
		gotobob(f,n);
		return TRUE;
	}
	forwchar(TRUE, exp->mlen?exp->mlen:1);
	while (is_at_end_of_line(DOT) || isspace(char_at(DOT))) {
		forwchar(TRUE,1);
		extra++;
	}
	if (n == 1 && samepoint(savepos,DOT)) { /* try again */
		if (looped > 10)
			return FALSE;
		s = backchar(TRUE, (exp->mlen?exp->mlen:1) + extra + looped);
		while (s && is_at_end_of_line(DOT))
			s = backchar(TRUE,1);
		looped++;
		goto top;

	}
	return TRUE;
}

int
gotoeosent(f,n)
int f,n;
{
	register regexp *exp;
	register int s;

	exp = b_val_rexp(curbp,VAL_SENTENCES)->reg;
	/* if we're on the end of a sentence now, don't bother scanning
		further, or we'll miss the immediately following sentence */
	if (!(lregexec(exp, DOT.l, DOT.o, llength(DOT.l)) &&
				exp->startp[0] - DOT.l->l_text == DOT.o)) {
		if (findpat(f, n, exp, FORWARD) != TRUE) {
			DOT = curbp->b_line;
			return TRUE;
		}
	}
	s = forwchar(TRUE, exp->mlen?exp->mlen:1);
	while (s && (is_at_end_of_line(DOT) || isspace(char_at(DOT)))) {
		s = forwchar(TRUE,1);
	}
	return TRUE;
}

#endif /* WORDPRO */

/*
 * This routine, given a pointer to a LINE, and the current cursor goal
 * column, return the best choice for the offset. The offset is returned.
 * Used by "C-N" and "C-P".
 */
int
getgoal(dlp)
register LINE   *dlp;
{
        register int    c;
        register int    col;
        register int    newcol;
        register int    dbo;

        col = 0;
        dbo = 0;
        while (dbo != llength(dlp)) {
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
next_column(c,col)
int c, col;
{
	if (c == '\t')
                return nextab(col);
        else if (!isprint(c))
                return col+2;
	else
                return col+1;
}

/*
 * Scroll forward by a specified number of lines, or by a full page if no
 * argument.  The "2" in the arithmetic on the window size is
 * the overlap; this value is the default overlap value in ITS EMACS. Because
 * this zaps the top line in the display window, we have to do a hard update.
 */
int
forwpage(f, n)
int f;
register int    n;
{
        register LINE   *lp;

        if (f == FALSE) {
                n = curwp->w_ntrows - 2;        /* Default scroll.      */
                if (n <= 0)                     /* Forget the overlap   */
                        n = 1;                  /* if tiny window.      */
        } else if (n < 0)
                return (backpage(f, -n));
#if     CVMVAS
        else                                    /* Convert from pages   */
                n *= curwp->w_ntrows;           /* to lines.            */
#endif
        lp = curwp->w_line.l;
        while (n-- && lp!=curbp->b_line.l)
                lp = lforw(lp);
        curwp->w_line.l = lp;
        curwp->w_dot.l  = lp;
        curwp->w_dot.o  = 0;
        curwp->w_flag |= WFHARD|WFMODE;
        return (TRUE);
}

/*
 * This command is like "forwpage", but it goes backwards. The "2", like
 * above, is the overlap between the two windows. The value is from the ITS
 * EMACS manual. We do a hard update for exactly the same
 * reason.
 */
int
backpage(f, n)
int f;
register int    n;
{
        register LINE   *lp;

        if (f == FALSE) {
                n = curwp->w_ntrows - 2;        /* Default scroll.      */
                if (n <= 0)                     /* Don't blow up if the */
                        n = 1;                  /* window is tiny.      */
        } else if (n < 0)
                return (forwpage(f, -n));
#if     CVMVAS
        else                                    /* Convert from pages   */
                n *= curwp->w_ntrows;           /* to lines.            */
#endif
        lp = curwp->w_line.l;
        while (n-- && lback(lp)!=curbp->b_line.l)
                lp = lback(lp);
        curwp->w_line.l = lp;
        curwp->w_dot.l  = lp;
        curwp->w_dot.o  = 0;
        curwp->w_flag |= WFHARD;
        return (TRUE);
}

/*
 * Scroll forward by a specified number of lines, or by a full page if no
 * argument. The "2" in the arithmetic on the window size is
 * the overlap; this value is the default overlap value in ITS EMACS. Because
 * this zaps the top line in the display window, we have to do a hard update.
 */
int
forwhpage(f, n)
int f;
register int    n;
{
        register LINE   *llp, *dlp;

        if (f == FALSE) {
                n = curwp->w_ntrows / 2;        /* Default scroll.      */
                if (n <= 0)                     /* Forget the overlap   */
                        n = 1;                  /* if tiny window.      */
        } else if (n < 0)
                return (backhpage(f, -n));
#if     CVMVAS
        else                                    /* Convert from pages   */
                n *= curwp->w_ntrows/2;           /* to lines.            */
#endif
        llp = curwp->w_line.l;
        dlp = curwp->w_dot.l;
        while (n-- && lforw(dlp) != curbp->b_line.l) {
                llp = lforw(llp);
                dlp = lforw(dlp);
	}
        curwp->w_line.l = llp;
        curwp->w_dot.l  = dlp;
	firstnonwhite(f,n);
        curwp->w_flag |= WFHARD|WFKILLS;
        return (TRUE);
}

/*
 * This command is like "forwpage", but it goes backwards. The "2", like
 * above, is the overlap between the two windows. The value is from the ITS
 * EMACS manual. We do a hard update for exactly the same
 * reason.
 */
int
backhpage(f, n)
int f;
register int    n;
{
        register LINE   *llp, *dlp;

        if (f == FALSE) {
                n = curwp->w_ntrows / 2;        /* Default scroll.      */
                if (n <= 0)                     /* Don't blow up if the */
                        n = 1;                  /* window is tiny.      */
        } else if (n < 0)
                return (forwhpage(f, -n));
#if     CVMVAS
        else                                    /* Convert from pages   */
                n *= curwp->w_ntrows/2;           /* to lines.            */
#endif
        llp = curwp->w_line.l;
        dlp = curwp->w_dot.l;
        while (n-- && lback(dlp)!=curbp->b_line.l) {
                llp = lback(llp);
                dlp = lback(dlp);
	}
        curwp->w_line.l = llp;
        curwp->w_dot.l  = dlp;
	firstnonwhite(f,n);
        curwp->w_flag |= WFHARD|WFINS;
        return (TRUE);
}



/*
 * Set the named mark in the current window to the value of "." in the window.
 */
/* ARGSUSED */
int
setnmmark(f,n)
int f,n;
{
	int c,i;

	if (clexec || isnamedcmd) {
		int stat;
		static char cbuf[2];
	        if ((stat=mlreply("Set mark: ", cbuf, 2)) != TRUE)
	                return stat;
		c = cbuf[0];
        } else {
		c = kbd_key();
        }
	if (c < 'a' || c > 'z') {
		TTbeep();
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
golinenmmark(f,n)
int f,n;
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
goexactnmmark(f,n)
int f,n;
{
	int c;
	register int s;

	s = getnmmarkname(&c);
	if (s != TRUE)
		return s;

	return gonmmark(c);
}

/* get the name of the mark to use.  interactively, "last dot" is
	represented by stuttering the goto-mark command.  from
	the command line, it's always named ' or `.  I suppose
	this is questionable. */
int
getnmmarkname(cp)
int *cp;
{
	int c;
	int this1key;
	int useldmark;

	if (clexec || isnamedcmd) {
		int stat;
		static char cbuf[2];
	        if ((stat=mlreply("Goto mark: ", cbuf, 2)) != TRUE)
	                return stat;
		c = cbuf[0];
		useldmark = (c == '\'' || c == '`');
        } else {
		this1key = last1key;
		c = kbd_key();
		useldmark = (last1key == this1key);  /* usually '' or `` */
        }

	if (useldmark)
		c = '\'';

	*cp = c;
	return TRUE;
}

int
gonmmark(c)
int c;
{
	register MARK *markp;
	MARK tmark;
	int found;

	if (!islower(c) && c != '\'') {
		TTbeep();
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
		for (lp = lforw(curbp->b_line.l);
				lp != curbp->b_line.l; lp = lforw(lp)) {
			if ((*markp).l == lp) {
				found = TRUE;
				break;
			}
		}
	}
	if (!found) {
		TTbeep();
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
setmark()
{
	MK = DOT;
        return (TRUE);
}

/* ARGSUSED */
int
gomark(f,n)
int f,n;
{
	DOT = MK;
        curwp->w_flag |= WFMOVE;
        return (TRUE);
}

/* this odd routine puts us at the internal mark, plus an offset of lines */
/*  n == 1 leaves us at mark, n == 2 one line down, etc. */
/*  this is for the use of stuttered commands, and line oriented regions */
int
godotplus(f,n)
int f,n;
{
	int s;
	if (!f || n == 1) {
		firstnonwhite(f,n);
	        return (TRUE);
	}
	if (n < 1)
	        return (FALSE);
	s = forwline(TRUE,n-1);
	if (s && is_header_line(DOT, curbp))
		s = backline(FALSE,1);
	if (s == TRUE)
		firstnonwhite(f,n);
	return s;
}

/*
 * Swap the values of "." and "mark" in the current window. This is pretty
 * easy, because all of the hard work gets done by the standard routine
 * that moves the mark about. The only possible error is "no mark".
 */
void
swapmark()
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


#if X11
void
setcursor(row, col)
int row, col;
{
    register LINE *dlp;
    WINDOW     *wp0;		/* current window on entry */

/* find the window we are pointing to */
    wp0 = curwp;
    while (row < curwp->w_toprow ||
	    row > curwp->w_ntrows + curwp->w_toprow) {
	nextwind(FALSE, 0);
	if (curwp == wp0)
	    break;		/* full circle */
    }

/* move to the right row */
    row -= curwp->w_toprow;
    dlp = curwp->w_line.l;	/* get pointer to 1st line */
    while (row-- && (dlp != curbp->b_line.l))
	dlp = lforw(dlp);
    DOT.l = dlp;	/* set dot line pointer */

    /* now move the dot over until near the requested column */
    curgoal = col + w_val(curwp, WVAL_SIDEWAYS);
    DOT.o = getgoal(dlp);
    /* don't allow the cursor to be set past end of line unless in
     * insert mode
     */
    if (DOT.o >= llength(dlp) && DOT.o > 0 && !insertmode)
    	DOT.o--;
    curwp->w_flag |= WFMOVE;
    return;
}
#endif
