/*
 * The routines in this file implement commands that work word or a
 * paragraph at a time.  There are all sorts of word mode commands.  If I
 * do any sentence mode commands, they are likely to be put in this file. 
 */

#include	<stdio.h>
#include	"estruct.h"
#include	"edef.h"

/* Word wrap on n-spaces. Back-over whatever precedes the point on the current
 * line and stop on the first word-break or the beginning of the line. If we
 * reach the beginning of the line, jump back to the end of the word and start
 * a new line.	Otherwise, break the line at the word-break, eat it, and jump
 * back to the end of the word.
 * Returns TRUE on success, FALSE on errors.
 */
wrapword()
{
	register int cnt;	/* size of word wrapped to next line */
	register int c;		/* charector temporary */

	/* backup from the <NL> 1 char */
	if (!backchar(0, 1))
		return(FALSE);

	/* back up until we aren't in a word,
	   make sure there is a break in the line */
	cnt = 0;
	while (((c = lgetc(curwp->w_dotp, curwp->w_doto)) != ' ')
				&& (c != '\t')) {
		cnt++;
		if (!backchar(0, 1))
			return(FALSE);
		/* if we make it to the beginning, start a new line */
		if (curwp->w_doto == 0) {
			gotoeol(FALSE, 0);
			return(lnewline());
		}
	}

	/* delete the forward white space */
	if (!ldelete(1L, FALSE))
		return(FALSE);

	/* put in a end of line */
	if (!lnewline())
		return(FALSE);

	/* and past the first word */
	while (cnt-- > 0) {
		if (forwchar(FALSE, 1) == FALSE)
			return(FALSE);
	}
	return(TRUE);
}


/*
 * Move the cursor forward by the specified number of words. All of the motion
 * is done by "forwchar". Error if you try and move beyond the buffer's end.
 *
 * Returns of SORTOFTRUE result if we're doing a non-delete operation.  
 * Whitespace after a word is always included on deletes (and non-operations,
 * of course), but only on intermediate words for other operations, for
 * example.  The last word of non-delete ops does _not_ include its whitespace.
 */

forwviword(f, n)
{
	int s;

	if (n < 0)
		return (backword(f, -n));
	setchartype();
	if (forwchar(FALSE, 1) == FALSE)
		return (FALSE);
	while (n--) {
		while (((s = isnewviwordf()) == FALSE) || 
				(s == SORTOFTRUE && n != 0)) {
			if (forwchar(FALSE, 1) == FALSE)
				return (FALSE);
		}
	}
	return TRUE;
}

/*
 * Move the cursor forward by the specified number of words. All of the motion
 * is done by "forwchar". Error if you try and move beyond the buffer's end.
 */
forwword(f, n)
{
	int s;

	if (n < 0)
		return (backword(f, -n));
	setchartype();
	if (forwchar(FALSE, 1) == FALSE)
		return (FALSE);
	while (n--) {
		while (((s = isnewwordf()) == FALSE) || 
				(s == SORTOFTRUE && n != 0)) {
			if (forwchar(FALSE, 1) == FALSE)
				return (FALSE);
		}
	}
	return(TRUE);
}

/*
 * Move the cursor forward by the specified number of words. All of the motion
 * is done by "forwchar". Error if you try and move beyond the buffer's end.
 */
forwviendw(f, n)
{
	int s;
	if (n < 0)
		return (FALSE);
	if (forwchar(FALSE, 1) == FALSE)
		return (FALSE);
	setchartype();
	while (n--) {
		while ((s = isendviwordf()) == FALSE) {
			if (forwchar(FALSE, 1) == FALSE)
				return (FALSE);
		}

	}
	if (s == SORTOFTRUE)
		return TRUE;
	else
		return backchar(FALSE, 1);
}

/*
 * Move the cursor forward by the specified number of words. All of the motion
 * is done by "forwchar". Error if you try and move beyond the buffer's end.
 */
forwendw(f, n)
{
	int s;
	if (n < 0)
		return (FALSE);
	if (forwchar(FALSE, 1) == FALSE)
		return (FALSE);
	setchartype();
	while (n--) {
		while ((s = isendwordf()) == FALSE) {
			if (forwchar(FALSE, 1) == FALSE)
				return (FALSE);
		}

	}
	if (s == SORTOFTRUE)
		return TRUE;
	else
		return backchar(FALSE, 1);
}

/*
 * Move the cursor backward by "n" words. All of the details of motion are
 * performed by the "backchar" and "forwchar" routines. Error if you try to
 * move beyond the buffers.
 */
backviword(f, n)
{
	if (n < 0)
		return (forwword(f, -n));
	if (backchar(FALSE, 1) == FALSE)
		return (FALSE);
	setchartype();
	while (n--) {
		while (isnewviwordb() == FALSE) {
			if (backchar(FALSE, 1) == FALSE)
				return (FALSE);
		}
	}
	return (forwchar(FALSE, 1));
}

/*
 * Move the cursor backward by "n" words. All of the details of motion are
 * performed by the "backchar" and "forwchar" routines. Error if you try to
 * move beyond the buffers.
 */
backword(f, n)
{
	if (n < 0)
		return (forwword(f, -n));
	if (backchar(FALSE, 1) == FALSE)
		return (FALSE);
	setchartype();
	while (n--) {
		while (isnewwordb() == FALSE) {
			if (backchar(FALSE, 1) == FALSE)
				return (FALSE);
		}
	}
	return (forwchar(FALSE, 1));
}

/*
 * Return TRUE if the character at dot is a character that is considered to be
 * part of a word. The word character list is hard coded. Should be setable.
 */
inword()
{
	register int	c;

	if (curwp->w_doto == llength(curwp->w_dotp))
		return (FALSE);
	c = lgetc(curwp->w_dotp, curwp->w_doto);
	if (islower(c))
		return (TRUE);
	if (isupper(c))
		return (TRUE);
	if (isdigit(c))
		return (TRUE);
	return (FALSE);
}

join(f,n)
{
	register int s;
	register int doto;

	if (n < 0) return FALSE;
	if (n == 0) return TRUE;
	if (lforw(curwp->w_dotp) == curbp->b_linep)
		return FALSE;
	while(n--) {
		s = lastnonwhite(f,n);
		if (s == TRUE) s = forwchar(FALSE,1);
		if (s == TRUE) s = setmark();
		if (s == TRUE) s = forwline(f,1);
		if (s == TRUE) s = firstnonwhite(f,1);
		if (s == TRUE) s = killregion(f,1);
		if (s != TRUE)
			return s ;

		doto = curwp->w_doto;
		if (doto == 0)
			return  TRUE;
		if (lgetc(curwp->w_dotp,doto) == ')')
			return TRUE;
		if (lgetc(curwp->w_dotp,doto-1) == '.')
			s = linsert(2,' ');
		else
			s = linsert(1,' ');
	}

	return s;
}

formatregion(f,n)
{
	register int c;			/* current char durring scan	*/
	register int wordlen;		/* length of current word	*/
	register int clength;		/* position on line during fill	*/
	register int i;			/* index during word copy	*/
	register int newlength;		/* tentative new line length	*/
	register int finished;		/* Are we at the End-Of-Paragraph? */
	register int firstflag;		/* first word? (needs no space)	*/
	register LINE *pastline;		/* pointer to line just past EOP */
	register int dotflag;		/* was the last char a period?	*/
	char wbuf[NSTRING];		/* buffer for current word	*/
	int secondindent;
	REGION region;
	int s;
	
	if (curbp->b_mode & MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/

	if (curwp->w_mkp != curwp->w_dotp) {
		getregion(&region);
		if (region.r_linep == curwp->w_mkp)
			swapmark();
	}
	pastline = curwp->w_mkp;
	if (pastline != curbp->b_linep)
		pastline = lforw(pastline);
		
	secondindent = indentlen(curwp->w_dotp);
	
	/* go forward to get the indent for the second and following lines */
	curwp->w_dotp = lforw(curwp->w_dotp);

	if (curwp->w_dotp != pastline) {
		secondindent = indentlen(curwp->w_dotp);
	}
		
	/* and back where we should be */
	curwp->w_dotp = lback(curwp->w_dotp);
	firstnonwhite(FALSE,1);
	
	clength = indentlen(curwp->w_dotp);
	wordlen = 0;
	dotflag = FALSE;

	/* scan through lines, filling words */
	firstflag = TRUE;
	finished = FALSE;
	while (!finished) {

		if (interrupted) return ABORT;

		/* get the next character in the paragraph */
		if (curwp->w_doto == llength(curwp->w_dotp)) {
			c = ' ';
			if (lforw(curwp->w_dotp) == pastline)
				finished = TRUE;
		} else {
			c = lgetc(curwp->w_dotp, curwp->w_doto);
		}
		/* and then delete it */
		if (!finished) {
			s = ldelete(1L, FALSE);
			if (s != TRUE) return s;
		}

		/* if not a separator, just add it in */
		if (c != ' ' && c != '\t') {
			dotflag = (c == '.');		/* was it a dot */
			if (wordlen < NSTRING - 1)
				wbuf[wordlen++] = c;
		} else if (wordlen) {
			/* at a word break with a word waiting */
			/* calculate tentative new length with word added */
			newlength = clength + 1 + wordlen;
			if (newlength <= fillcol) {
				/* add word to current line */
				if (!firstflag) {
					s = linsert(1, ' '); /* the space */
					if (s != TRUE) return s;
					++clength;
				}
				firstflag = FALSE;
			} else {
		                if (lnewline() == FALSE
			                || ((i=secondindent/TABVAL)!=0 &&
			                	linsert(i, '\t')==FALSE)
			                || ((i=secondindent%TABVAL)!=0 &&
			                	linsert(i,  ' ')==FALSE)) {
		                        return FALSE;
		                }
				clength = secondindent;
			}

			/* and add the word in in either case */
			for (i=0; i<wordlen; i++) {
				s = linsert(1, wbuf[i]);
				if (s != TRUE) return s;
				++clength;
			}
			if (dotflag) {
				s = linsert(1, ' ');
				if (s != TRUE) return s;
				++clength;
			}
			wordlen = 0;
		}
	}
	return(TRUE);
}


#if	WORDCOUNT	/* who cares? -pgf */
/*	wordcount:	count the # of words in the marked region,
			along with average word sizes, # of chars, etc,
			and report on them.			*/
wordcount(f, n)
{
	register LINE *lp;	/* current line to scan */
	register int offset;	/* current char to scan */
	long size;		/* size of region left to count */
	register int ch;	/* current character to scan */
	register int wordflag;	/* are we in a word now? */
	register int lastword;	/* were we just in a word? */
	long nwords;		/* total # of words */
	long nchars;		/* total number of chars */
	int nlines;		/* total number of lines in region */
	int avgch;		/* average number of chars/word */
	int status;		/* status return code */
	REGION region;		/* region to look at */

	/* make sure we have a region to count */
	if ((status = getregion(&region)) != TRUE)
		return(status);
	lp = region.r_linep;
	offset = region.r_offset;
	size = region.r_size;

	/* count up things */
	lastword = FALSE;
	nchars = 0L;
	nwords = 0L;
	nlines = 0;
	while (size--) {

		/* get the current character */
		if (offset == llength(lp)) {	/* end of line */
			ch = '\n';
			lp = lforw(lp);
			offset = 0;
			++nlines;
		} else {
			ch = lgetc(lp, offset);
			++offset;
		}

		/* and tabulate it */
		wordflag = ((ch >= 'a' && ch <= 'z') ||
			    (ch >= 'A' && ch <= 'Z') ||
			    (ch >= '0' && ch <= '9'));
		if (wordflag == TRUE && lastword == FALSE)
			++nwords;
		lastword = wordflag;
		++nchars;
	}

	/* and report on the info */
	if (nwords > 0L)
		avgch = (int)((100L * nchars) / nwords);
	else
		avgch = 0;

	mlwrite("lines %d, words, %D chars %D  avg chars/word %f",
		nlines + 1, nwords, nchars, avgch);
	return(TRUE);
}
#endif
