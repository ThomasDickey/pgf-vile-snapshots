/* 
 *
 *	insert.c
 *
 * Do various types of character insertion, including most of insert mode.
 *
 * Most code probably by Dan Lawrence or Dave Conroy for MicroEMACS
 * Extensions for vile by Paul Fox
 *
 *	$Log: insert.c,v $
 *	Revision 1.4  1992/07/15 23:23:46  foxharp
 *	made '80i-ESC' work
 *
 * Revision 1.3  1992/07/04  14:34:52  foxharp
 * added ability to call SPEC-key bound functions (motion only) during
 * insert mode, on the assumption that you don't _really_ want to insert
 * function keys into your buffer.
 *
 * Revision 1.2  1992/06/01  20:37:31  foxharp
 * added tabinsert
 * mode
 *
 * Revision 1.1  1992/05/29  09:38:33  foxharp
 * Initial revision
 *
 *
 *
 */

#include	<stdio.h>
#include	"estruct.h"
#include	"edef.h"
#if UNIX
#include	<signal.h>
#endif

/* open lines up before this one */
int
openup(f,n)
int f,n;
{
	register int s;

	if (!f) n = 1;
	if (n < 0) return (FALSE);
	if (n == 0) return ins(FALSE);

	gotobol(TRUE,1);

	/* if we are in C mode and this is a default <NL> */
	if (n == 1 && (b_val(curbp,MDCMOD) || b_val(curbp,MDAIND)) &&
						!is_header_line(DOT,curbp)) {
		s = indented_newline_above(b_val(curbp, MDCMOD));
		if (s != TRUE) return (s);

		return(ins(FALSE));
	}
	s = lnewline();
	if (s != TRUE) return s;

	s = backline(TRUE,1);		/* back to the blank line */
	if (s != TRUE) return s;

	if ( n > 1) {
		s = openlines(n-1);
		if (s != TRUE) return s;
		s = backline(TRUE, 1);	/* backup over the first one */
		if (s != TRUE) return s;
	}

	return(ins(FALSE));
}

/* open lines up after this one */
int
opendown(f,n)
int f,n;
{
	register int	s;

	if (!f) n = 1;
	if (n < 0) return (FALSE);
	if (n == 0) return ins(FALSE);

	s = openlines(n);
	if (s != TRUE)
		return (s);

	return(ins(FALSE));
}

/*
 * Open up some blank space. The basic plan is to insert a bunch of newlines,
 * and then back up over them.
 */
int
openlines(n)
int n;
{
	register int i = n;			/* Insert newlines. */
	register int s = TRUE;
	while (i-- && s==TRUE) {
		gotoeol(FALSE,1);
		s = newline(TRUE,1);
	}
	if (s == TRUE && n)			/* Then back up overtop */
		backline(TRUE, n-1);	/* of them all.		 */

	curgoal = -1;

	return s;
}

/*
 * Go into insert mode.  I guess this isn't emacs anymore...
 */
/* ARGSUSED */
int
insert(f, n)
int f,n;
{
	int s = TRUE;

	if (!f || n < 0) n = 1;

	s = ins(FALSE);

	while (s && --n)
		s = ins(TRUE);

	update(FALSE);
	return s;
}

/* ARGSUSED */
int
insertbol(f, n)
int f,n;
{
	int s = TRUE;
	firstnonwhite(f,n);

	if (!f || n < 0) n = 1;

	s = ins(FALSE);

	while (s && --n)
		s = ins(TRUE);

	return s;
}

/* ARGSUSED */
int
append(f, n)
int f,n;
{
	int s = TRUE;

	if (! is_header_line(DOT,curbp) && !is_at_end_of_line(DOT))
		forwchar(TRUE,1); /* END OF LINE HACK */

	if (!f || n < 0) n = 1;

	s = ins(FALSE);

	while (s && --n)
		s = ins(TRUE);

	return s;
}

/* ARGSUSED */
int
appendeol(f, n)
int f,n;
{
	int s = TRUE;
	if (!is_header_line(DOT,curbp))
		gotoeol(FALSE,0);

	if (!f || n < 0) n = 1;

	s = ins(FALSE);

	while (s && --n)
		s = ins(TRUE);

	return s;
}

/* ARGSUSED */
int
overwrite(f, n)
int f,n;
{
	int s = TRUE;
	insertmode = OVERWRITE;
	if (b_val(curbp, MDSHOWMODE))
		curwp->w_flag |= WFMODE;

	if (!f || n < 0) n = 1;

	s = ins(FALSE);

	while (s && --n)
		s = ins(TRUE);

	return s;
}

int
replacechar(f, n)
int f,n;
{
	register int	s;
	register int	c;

	if (!f && llength(DOT.l) == 0)
		return FALSE;

	insertmode = REPLACECHAR;  /* need to fool the SPEC prefix code */
	if (b_val(curbp, MDSHOWMODE))
		curwp->w_flag |= WFMODE;
	update(FALSE);
	c = kbd_key();
	insertmode = FALSE;
	curwp->w_flag |= WFMODE;

	if (n < 0)
		return FALSE;
	if (n == 0)
		return TRUE;
	if (c == abortc)
		return FALSE;

	ldelete((long)n,FALSE);
	if (c == quotec) {
		return(quote(f,n));
	}
	c = kcod2key(c);
	if (c == '\n' || c == '\r') {
		do {
			s = lnewline();
		} while (s==TRUE && --n);
		return s;
	} else if (isbackspace(c))
		s = TRUE;
	else
		s = linsert(n, c);
	if (s == TRUE)
		s = backchar(FALSE,1);
	return s;
}

/* grunt routine for insert mode */
int
ins(playback)
int playback;
{
	register int status;
	int f,n;
	int (*execfunc)();		/* ptr to function to execute */
	int    c;		/* command character */
	int newlineyet = FALSE; /* are we on the line we started on? */
	int startoff = DOT.o;	/* starting offset on that line */
	static char insbuff[256];
	char *iptr = insbuff;

	if (insertmode == FALSE) {
		insertmode = INSERT;
		if (b_val(curbp, MDSHOWMODE))
			curwp->w_flag |= WFMODE;
	}

	/* get the next command from the keyboard */
	while(1) {

		f = FALSE;
		n = 1;

		if (playback) {
			c = *iptr++;
		} else {
			update(FALSE);
			c = kbd_key();
			if (iptr - insbuff < 255)
				*iptr++ = c;
			else
				insbuff[255] = abortc;
		}

		if (c == abortc ) {
			 /* an unfortunate Vi-ism that ensures one 
				can always type "ESC a" if you're not sure 
				you're in insert mode. */
			if (DOT.o != 0)
				backchar(TRUE,1);
			if (autoindented >= 0) {
				trimline();
				autoindented = -1;
			}
			insertmode = FALSE;
			if (b_val(curbp, MDSHOWMODE))
				curwp->w_flag |= WFMODE;
			return (TRUE);
		} else if (c == -abortc ) {
			/* we use the negative to suppress that
				junk, for the benefit of SPEC keys */
			insertmode = FALSE;
			if (b_val(curbp, MDSHOWMODE))
				curwp->w_flag |= WFMODE;
			return (TRUE);
		}

		if (c & SPEC) {
			CMDFUNC *cfp;
			cfp = kcod2fnc(c);
			if (!cfp || ((cfp->c_flags & MOTION|REDO|UNDO)
						!= MOTION)) {
				startoff = 0;
				curgoal = getccol(FALSE);
				execute(cfp,FALSE,1);
			}
			continue;
		}

		execfunc = NULL;
		if (c == quotec) {
			execfunc = quote;
		} else {
			/*
			 * If a space was typed, fill column is defined, the
			 * argument is non- negative, wrap mode is enabled, and
			 * we are now past fill column, perform word wrap. 
			 */
			if (isspace(c) && b_val(curwp->w_bufp,MDWRAP) &&
				b_val(curbp,VAL_FILL) > 0 && n >= 0 &&
				getccol(FALSE) > b_val(curbp,VAL_FILL)) {
				wrapword(FALSE,1);
				newlineyet = TRUE;
			}

			if ( c ==  '\t') { /* tab */
				execfunc = tab;
				autoindented = -1;
			} else if (c ==  tocntrl('J') ||
				c ==  tocntrl('M')) { /* CR and NL */
				execfunc = newline;
				if (autoindented >= 0) {
					trimline();
					autoindented = -1;
				}
				newlineyet = TRUE;
			} else if ( isbackspace(c) ||
					c == tocntrl('D') || 
					c == killc ||
					c == wkillc) { /* ^U and ^W */
				/* how far can we back up? */
				register int backlimit;
				/* have we backed thru a "word" yet? */
				int saw_word = FALSE;
				if ((c == tocntrl('D') && autoindented >=0) ||
					newlineyet || !b_val(curbp,MDBACKLIMIT))
					backlimit = 0;
				else
					backlimit = startoff;
				execfunc = nullproc;
				if (c == tocntrl('D')) {
					int goal;
					int col;
					int sw = 
					 b_val(curbp, VAL_SWIDTH);
					col = getccol(FALSE);
					if (col > 0)
						goal = ((col-1)/sw)*sw;
					else
						goal = 0;
					while (col > goal &&
						DOT.o > backlimit) {
						backspace();
						col = getccol(FALSE);
					}
					if (col < goal)
						linsert(goal - col,' ');
				} else while (DOT.o > backlimit) {
					if (c == wkillc) {
						if (isspace(
							lgetc(DOT.l,DOT.o-1))) {
							if (saw_word)
								break;
						} else {
							saw_word = TRUE;
						}
					}
					backspace();
					autoindented--;
					if (isbackspace(c) ||
						c == tocntrl('D'))
						break;
				}
			} else if ( c ==  tocntrl('T')) { /* ^T */
				execfunc = shiftwidth;

#if UNIX && defined(SIGTSTP)	/* job control, ^Z */
			} else if (c == suspc) {
				execfunc = bktoshell;

#endif
			} else if (c == startc ||
					c == stopc) {  /* ^Q and ^S */
				execfunc = nullproc;
			}
		}

		if (execfunc != NULL) {
			status	 = (*execfunc)(f, n);
			if (status != TRUE) {
				insertmode = FALSE;
				if (b_val(curbp, MDSHOWMODE))
					curwp->w_flag |= WFMODE;
				return (status);
			}
			continue;
		}

		    
		/* make it a real character again */
		c = kcod2key(c);

		/* if we are in overwrite mode, not at eol,
		   and next char is not a tab or we are at a tab stop,
		   delete a char forword			*/
		if (insertmode == OVERWRITE &&
				DOT.o < llength(DOT.l) &&
				(char_at(DOT) != '\t' ||
					(DOT.o) % curtabval == curtabval-1)) {
			autoindented = -1;
			ldelete(1L, FALSE);
		}

		/* do the appropriate insertion */
		if ((c == RBRACE) && b_val(curbp, MDCMOD)) {
			status = insbrace(n, c);
		} else if (c == '#' && b_val(curbp, MDCMOD)) {
			status = inspound();
		} else {
			autoindented = -1;
			status = linsert(n, c);
		}

#if CFENCE
		/* check for CMODE fence matching */
		if ((c == RBRACE || c == ')' || c == ']') && 
						b_val(curbp, MDSHOWMAT))
			fmatch(c);
#endif

		/* check auto-save mode */
		if (b_val(curbp, MDASAVE))
			if (--curbp->b_acount <= 0) {
				/* and save the file if needed */
				upscreen(FALSE, 0);
				filesave(FALSE, 0);
				curbp->b_acount = b_val(curbp,VAL_ASAVECNT);
			}

		if (status != TRUE) {
			insertmode = FALSE;
			if (b_val(curbp, MDSHOWMODE))
				curwp->w_flag |= WFMODE;
			return (status);
		}
	}
}

int
backspace()
{
	register int	s;

	if ((s=backchar(TRUE, 1)) == TRUE)
		s = ldelete(1L, FALSE);
	return (s);
}

/*
 * Insert a newline. If we are in CMODE, do automatic
 * indentation as specified.
 */
int
newline(f, n)
int f,n;
{
	register int	s;

	if (!f)
		n = 1;
	else if (n < 0)
		return (FALSE);

#if LATER	/* already done for autoindented != 0 in ins() */
	if (b_val(curbp, MDTRIM))
		trimline();
#endif
	    
	/* if we are in C or auto-indent modes and this is a default <NL> */
	if (n == 1 && (b_val(curbp,MDCMOD) || b_val(curbp,MDAIND)) &&
						!is_header_line(DOT,curbp))
		return indented_newline(b_val(curbp, MDCMOD));

	/*
	 * If a newline was typed, fill column is defined, the argument is non-
	 * negative, wrap mode is enabled, and we are now past fill column,
	 * perform word wrap.
	 */
	if (b_val(curwp->w_bufp, MDWRAP) && b_val(curbp,VAL_FILL)> 0 &&
				getccol(FALSE) > b_val(curbp,VAL_FILL))
		wrapword(FALSE,1);

	/* insert some lines */
	while (n--) {
		if ((s=lnewline()) != TRUE)
			return (s);
		curwp->w_flag |= WFINS;
	}
	return (TRUE);
}

/* insert a newline and indentation for C */
int
indented_newline(cmode)
int cmode;
{
	register int indentwas; /* indent to reproduce */
	int bracef; /* was there a brace at the end of line? */
	    
	indentwas = previndent(&bracef);

	if (lnewline() == FALSE)
		return FALSE;
	if (cmode && bracef)
		indentwas = nextab(indentwas);
	if (doindent(indentwas) != TRUE)
		return FALSE;
	return TRUE;
}

/* insert a newline and indentation for autoindent */
int
indented_newline_above(cmode)
int cmode;
{
	register int indentwas;	/* indent to reproduce */
	int bracef; /* was there a brace at the beginning of line? */
	
	indentwas = nextindent(&bracef);
	if (lnewline() == FALSE)
		return FALSE;
	if (backline(TRUE,1) == FALSE)
		return FALSE;
	if (cmode && bracef)
		indentwas = nextab(indentwas);
	if (doindent(indentwas) != TRUE)
		return FALSE;
	return TRUE;
}
/* get the indent of the last previous non-blank line.	also, if arg
	is non-null, check if line ended in a brace */
int
previndent(bracefp)
int *bracefp;
{
	int ind;
	    
	MK = DOT;
	    
	if (backword(FALSE,1) == FALSE) {
		if (bracefp) *bracefp = FALSE;
		gomark(FALSE,1);
		return 0;
	}
	ind = indentlen(DOT.l);
	if (bracefp)
		*bracefp = (llength(DOT.l) > 0 &&
			lgetc(DOT.l,lastchar(DOT.l)) == '{');
		    
	gomark(FALSE,1);
	    
	return ind;
}

/* get the indent of the next non-blank line.	also, if arg
	is non-null, check if line starts in a brace */
int
nextindent(bracefp)
int *bracefp;
{
	int ind;
	    
	MK = DOT;
	    
	if (forwword(FALSE,1) == FALSE) {
		if (bracefp) *bracefp = FALSE;
		gomark(FALSE,1);
		return 0;
	}
	ind = indentlen(DOT.l);
	if (bracefp)
		*bracefp = (llength(DOT.l) > 0 &&
			lgetc(DOT.l,firstchar(DOT.l)) == '}');
		    
	gomark(FALSE,1);
	    
	return ind;
}

int
doindent(ind)
int ind;
{
	int i;
	/* if no indent was asked for, we're done */
	if (ind <= 0)
		return TRUE;
	autoindented = 0;
	/* first clean up existing leading whitespace */
	i = firstchar(DOT.l);
	if (i)
		ldelete((long)i,FALSE);
	if ((i=ind/curtabval)!=0) {
		autoindented += i;
		if (tab(TRUE,i) == FALSE)
			return FALSE;
	}
	if ((i=ind%curtabval) != 0) {
		autoindented += i;
		if (linsert(i,	' ') == FALSE)
			return FALSE;
	}
	if (!autoindented)
		autoindented = -1;
	    
	return TRUE;
}

/* return the column indent of the specified line */
int
indentlen(lp)
LINE *lp;
{
	register int ind, i, c;
	ind = 0;
	for (i=0; i<llength(lp); ++i) {
		c = lgetc(lp, i);
		if (!isspace(c))
			break;
		if (c == '\t')
			ind = nextab(ind);
		else
			++ind;
	}
	return ind;
}


int
insbrace(n, c)	/* insert a brace into the text here...we are in CMODE */
int n;	/* repeat count */
int c;	/* brace to insert (always { for now) */
{

#if ! CFENCE
	/* wouldn't want to back up from here, but fences might take us 
		forward */
	/* if we are at the beginning of the line, no go */
	if (DOT.o == 0)
		return(linsert(n,c));
#endif

	if (autoindented >= 0) {
		trimline();
	}
	else {
		return linsert(n,c);
	}
#if ! CFENCE /* no fences?	then put brace one tab in from previous line */
	doindent(((previndent(NULL)-1) / curtabval) * curtabval);
#else /* line up brace with the line containing its match */
	doindent(fmatchindent());
#endif
	autoindented = -1;

	/* and insert the required brace(s) */
	return(linsert(n, c));
}

int
inspound()	/* insert a # into the text here...we are in CMODE */
{

	/* if we are at the beginning of the line, no go */
	if (DOT.o == 0)
		return(linsert(1,'#'));

	if (autoindented > 0) { /* must all be whitespace before us */
		DOT.o = 0;
		ldelete((long)autoindented,FALSE);
	}
	autoindented = -1;

	/* and insert the required pound */
	return(linsert(1, '#'));
}

/* insert a tab into the file */
/* ARGSUSED */
int
tab(f, n)
int f,n;
{
	int ccol;
	if (!f) n = 1;
	if (n <= 0)
		return FALSE;

	if (b_val(curbp,MDTABINSERT))
		return linsert(n, '\t');

	ccol = getccol(FALSE);
	return linsert((nextab(ccol) - ccol) + (n-1)*curtabval,' ');
}

int
shiftwidth()
{
	int s;
	int fc;
	fc = firstchar(DOT.l);
	if (fc < DOT.o) {
		s = linsert(b_val(curbp, VAL_SWIDTH), ' ');
		/* should entab mult ^T inserts */
		return s;
	}
	detabline(TRUE);
	s = b_val(curbp, VAL_SWIDTH) - 
		(getccol(FALSE) % b_val(curbp,VAL_SWIDTH));
	if (s)
		s = linsert(s, ' ');
	if (b_val(curbp,MDTABINSERT))
                entabline(TRUE);
	if (autoindented >= 0) {
		autoindented = firstchar(DOT.l);
	}
	return TRUE;
}

/*
 * Quote the next character, and insert it into the buffer. All the characters
 * are taken literally, with the exception of the newline, which always has
 * its line splitting meaning. The character is always read, even if it is
 * inserted 0 times, for regularity.
 */
int
quote(f, n)
int f,n;
{
	register int	s;
	register int	c;

	c = tgetc();
	if (!f)
		n = 1;
	if (n < 0)
		return FALSE;
	if (n == 0)
		return TRUE;
	if (c == '\n') {
		do {
			s = lnewline();
		} while (s==TRUE && --n);
		return s;
	}
	return linsert(n, c);
}

