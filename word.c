/*
 * The routines in this file implement commands that work word or a
 * paragraph at a time.  There are all sorts of word mode commands.  If I
 * do any sentence mode commands, they are likely to be put in this file. 
 *
 * $Log: word.c,v $
 * Revision 1.14  1991/11/08 13:02:46  pgf
 * ifdefed unneeded funcs
 *
 * Revision 1.13  1991/11/03  17:33:20  pgf
 * use new lregexec() routine to check for patterns in lines
 *
 * Revision 1.12  1991/11/01  14:37:22  pgf
 * saber cleanup
 *
 * Revision 1.11  1991/10/28  14:26:45  pgf
 * eliminated TABVAL and fillcol macros -- now use curtabval and the VAL_FILL
 * directly
 *
 * Revision 1.10  1991/10/28  01:01:06  pgf
 * added start offset and end offset to regexec calls
 *
 * Revision 1.9  1991/10/27  01:57:45  pgf
 * changed usage of issecbegin() in formatregion to use regexec instead
 *
 * Revision 1.8  1991/08/09  13:17:52  pgf
 * formatregion now restarts with each fresh paragraph, so you can
 * format an entire file at once, without collapsing it all into a
 * single paragraph
 *
 * Revision 1.7  1991/08/07  12:35:07  pgf
 * added RCS log messages
 *
 * revision 1.6
 * date: 1991/08/06 15:27:52;
 * removed old rdonly check
 * 
 * revision 1.5
 * date: 1991/06/28 10:54:14;
 * suppress trailing space after paragraph reformat
 * 
 * revision 1.4
 * date: 1991/06/25 19:53:45;
 * massive data structure restructure
 * 
 * revision 1.3
 * date: 1991/06/06 13:58:09;
 * added auto-indent mode
 * 
 * revision 1.2
 * date: 1991/03/26 17:02:20;
 * formatting now knows about ! and ? as well as .
 * 
 * revision 1.1
 * date: 1990/09/21 10:26:25;
 * initial vile RCS revision
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
/* ARGSUSED */
wrapword(f,n)
int f,n;
{
	register int cnt;	/* size of word wrapped to next line */
	register int c;		/* charector temporary */

	/* backup from the <NL> 1 char */
	if (!backchar(0, 1))
		return(FALSE);

	/* back up until we aren't in a word,
	   make sure there is a break in the line */
	cnt = 0;
	while (c = char_at(DOT), !isspace(c)) {
		cnt++;
		if (!backchar(0, 1))
			return(FALSE);
		/* if we make it to the beginning, start a new line */
		if (DOT.o == 0) {
			gotoeol(FALSE, 0);
			return(lnewline());
		}
	}

	/* delete the forward white space */
	if (!ldelete(1L, FALSE))
		return(FALSE);

	/* put in a end of line */
	if (!newline(TRUE,1))
		return FALSE;

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
int f,n;
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
int f,n;
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
int f,n;
{
	int s;
	if (!f)
		n = 1;
	else if (n < 0)
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
int f,n;
{
	int s;
	if (!f)
		n = 1;
	else if (n < 0)
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
int f,n;
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
int f,n;
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

#ifdef NEEDED
/*
 * Return TRUE if the character at dot is a character that is considered to be
 * part of a word. The word character list is hard coded. Should be setable.
 */
inword()
{
	register int	c;

	if (is_at_end_of_line(DOT))
		return (FALSE);
	c = char_at(DOT);
	if (islower(c))
		return (TRUE);
	if (isupper(c))
		return (TRUE);
	if (isdigit(c))
		return (TRUE);
	return (FALSE);
}
#endif

join(f,n)
int f,n;
{
	register int s;
	register int doto;

	if (!f) {
		n = 1;
	} else {
		if (n < 0) return FALSE;
		if (n == 0) return TRUE;
	}
	if (is_last_line(DOT, curbp))
		return FALSE;
	while(n--) {
		s = lastnonwhite(f,n);
		if (s == TRUE) s = forwchar(FALSE,1);
		if (s == TRUE) s = setmark();
		if (s == TRUE) s = forwline(f,1);
		if (s == TRUE) s = firstnonwhite(f,1);
		if (s == TRUE) s = killregion();
		if (s != TRUE)
			return s ;

		doto = DOT.o;
		if (doto == 0)
			return  TRUE;
		if (lgetc(DOT.l, doto) == ')')
			return TRUE;
		if (lgetc(DOT.l, doto-1) == '.')
			s = linsert(2,' ');
		else
			s = linsert(1,' ');
	}

	return s;
}

formatregion()
{
	register int c;			/* current char durring scan	*/
	register int wordlen;		/* length of current word	*/
	register int clength;		/* position on line during fill	*/
	register int i;			/* index during word copy	*/
	register int newlength;		/* tentative new line length	*/
	register int finished;		/* Are we at the End-Of-Paragraph? */
	register int firstflag;		/* first word? (needs no space)	*/
	register LINE *pastline;		/* pointer to line just past EOP */
	register int sentence;		/* was the last char a period?	*/
	char wbuf[NSTRING];		/* buffer for current word	*/
	int secondindent;
	REGION region;
	regexp *exp;
	int s;
	
	if (!sameline(MK, DOT)) {
		getregion(&region);
		if (sameline(region.r_orig, MK))
			swapmark();
	}
	pastline = MK.l;
	if (pastline != curbp->b_line.l)
		pastline = lforw(pastline);

	exp = b_val_rexp(curbp,VAL_PARAGRAPHS)->reg;
 	finished = FALSE;
 	while (finished != TRUE) {  /* i.e. is FALSE or SORTOFTRUE */
		while (lregexec(exp, DOT.l, 0, llength(DOT.l)) ) {
			DOT.l = lforw(DOT.l);
			if (DOT.l == pastline) {
				setmark();
				return TRUE;
			}
		}

		secondindent = indentlen(DOT.l);
		
		/* go forward to get the indent for the second
			and following lines */
		DOT.l = lforw(DOT.l);

		if (DOT.l != pastline) {
			secondindent = indentlen(DOT.l);
		}
			
		/* and back where we should be */
		DOT.l = lback(DOT.l);
		firstnonwhite(FALSE,1);
		
		clength = indentlen(DOT.l);
		wordlen = 0;
		sentence = FALSE;

		/* scan through lines, filling words */
		firstflag = TRUE;
		finished = FALSE;
		while (finished == FALSE) { /* i.e. is not TRUE  */
					    /* or SORTOFTRUE */
			if (interrupted) return ABORT;

			/* get the next character */
			if (is_at_end_of_line(DOT)) {
				c = ' ';
				DOT.l = lforw(DOT.l);
				if (DOT.l == pastline) {
					finished = TRUE;
				} else if (lregexec(exp, DOT.l,
					0, llength(DOT.l))) {
					/* we're at a section break */
					finished = SORTOFTRUE;
				}
				DOT.l = lback(DOT.l);
			} else {
				c = char_at(DOT);
			}
			/* and then delete it */
			if (finished == FALSE) {
				s = ldelete(1L, FALSE);
				if (s != TRUE) return s;
			}

			/* if not a separator, just add it in */
			if (c != ' ' && c != '\t') {
				/* was it the end of a "sentence"? */
				sentence = (c == '.' || c == '?' || c == '!');
				if (wordlen < NSTRING - 1)
					wbuf[wordlen++] = c;
			} else if (wordlen) {
				/* at a word break with a word waiting */
				/* calculate tentative new length
							with word added */
				newlength = clength + 1 + wordlen;
				if (newlength <= b_val(curbp,VAL_FILL)) {
					/* add word to current line */
					if (!firstflag) {
						/* the space */
						s = linsert(1, ' ');
						if (s != TRUE) return s;
						++clength;
					}
					firstflag = FALSE;
				} else {
			                if (lnewline() == FALSE ||
					((i=secondindent/curtabval)!=0 &&
			                	   linsert(i, '\t')==FALSE) ||
					((i=secondindent%curtabval)!=0 &&
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
				if (finished == FALSE && sentence) {
					s = linsert(1, ' ');
					if (s != TRUE) return s;
					++clength;
				}
				wordlen = 0;
			}
		}
		DOT.l = lforw(DOT.l);
	}
	setmark();
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
