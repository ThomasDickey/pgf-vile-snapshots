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
 *	Revision 1.30  1993/08/05 14:29:12  pgf
 *	tom's 3.57 changes
 *
 * Revision 1.29  1993/06/30  17:43:14  pgf
 * added call to map_check() when we're about to execute a SPEC binding,
 * to make sure :maps work
 *
 * Revision 1.28  1993/06/28  20:10:27  pgf
 * new variable
 *
 * Revision 1.27  1993/06/28  15:07:35  pgf
 * when killing chars, check for c == killc or wkillc _before_ checking it
 * against backspace, in case DEL is someones killc or wkillc.
 *
 * Revision 1.26  1993/06/28  13:32:04  pgf
 * fixed insstring() to INSERT instead of OVERWRITE (cut/paste error)
 *
 * Revision 1.25  1993/06/02  14:28:47  pgf
 * see tom's 3.48 CHANGES
 *
 * Revision 1.24  1993/05/24  15:21:37  pgf
 * tom's 3.47 changes, part a
 *
 * Revision 1.23  1993/05/05  10:31:30  pgf
 * cleaned up handling of SPEC keys from withing insert mode.  now, any
 * function bound to a SPECkey (i.e. any FN-? thing) can be executed
 * either from inside or outside insert mode.
 *
 * Revision 1.22  1993/05/03  14:22:55  pgf
 * fixed botch of backspace limiting introduced when i created inschar()
 *
 * Revision 1.21  1993/04/28  17:04:09  pgf
 * cosmetics in inschar()
 *
 * Revision 1.20  1993/04/28  09:49:29  pgf
 * fixed indentation if openup used in cmode, with a single word on the
 * current line -- indentation was coming from the _next_ line, instead.
 *
 * Revision 1.19  1993/04/20  12:18:32  pgf
 * see tom's 3.43 CHANGES
 *
 * Revision 1.18  1993/04/08  15:01:05  pgf
 * broke up ins(), to create inschar(), usable by insstring and ovrwstring
 *
 * Revision 1.17  1993/04/01  13:06:31  pgf
 * turbo C support (mostly prototypes for static)
 *
 * Revision 1.16  1993/03/18  17:42:20  pgf
 * see 3.38 section of CHANGES
 *
 * Revision 1.15  1993/03/17  10:01:18  pgf
 * overwrite() renamed to overwritechars()
 *
 * Revision 1.14  1993/03/16  10:53:21  pgf
 * see 3.36 section of CHANGES file
 *
 * Revision 1.13  1993/02/15  10:37:31  pgf
 * cleanup for gcc-2.3's -Wall warnings
 *
 * Revision 1.12  1993/02/08  14:53:35  pgf
 * see CHANGES, 3.32 section
 *
 * Revision 1.11  1993/01/23  14:27:23  foxharp
 * protect against backline() failing in openup(), if we're at top of buf
 *
 * Revision 1.10  1993/01/16  10:36:33  foxharp
 * use isreturn() macro
 *
 * Revision 1.9  1992/12/04  09:25:03  foxharp
 * deleted unused assigns
 *
 * Revision 1.8  1992/12/02  09:13:16  foxharp
 * changes for "c-shiftwidth"
 *
 * Revision 1.7  1992/11/30  23:06:03  foxharp
 * firstchar/lastchar now return -1 for no non-white chars in line
 *
 * Revision 1.6  1992/11/19  09:08:49  foxharp
 * experiment -- cause further indent on lines following lines that end
 * in a ':' when in c-mode, so we indent after labels, case xxx:, and default:
 *
 * Revision 1.5  1992/08/06  23:53:51  foxharp
 * autoindent now goes by shiftwidth, instead of tabstop
 *
 * Revision 1.4  1992/07/15  23:23:46  foxharp
 * made '80i-ESC' work
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

#include	"estruct.h"
#include	"edef.h"

static	int	wrap_at_col P(( void ));
static	void	advance_one_char P(( void ));
static	int	ins_n_times P(( int, int, int ));

static savedmode;  /* value of insertmode maintained through subfuncs */

/*--------------------------------------------------------------------------*/

/* returns nonzero iff wrap-margin or wrap-words is active */
static int
wrap_at_col()
{
	register int	n;

	if ((n = b_val(curbp, VAL_WRAPMARGIN)) > 0
	 && (n = col_limit(curwp) - n) > 0)
		return n;

	if (b_val(curbp, MDWRAP)
	 && (n = b_val(curbp, VAL_FILL)) > 0)
	 	return n;

	return getccol(FALSE)+1;
}

/* advance one character past the current position, for 'append()' */
static void
advance_one_char()
{
	if (! is_header_line(DOT,curbp) && !is_at_end_of_line(DOT))
		forwchar(TRUE,1); /* END OF LINE HACK */
}

/* common logic for i,I,a,A commands */
static int
ins_n_times(f,n,advance)
int f,n,advance;
{
	register int s;

	if (!f || n < 0)
		n = 1;

	s = ins(FALSE);

	while ((s == TRUE) && --n) {
		if (advance)
			advance_one_char();
		s = ins(TRUE);
	}

	return s;
}

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

	if (!is_first_line(DOT,curbp))
		backline(TRUE,1);		/* back to the blank line */

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
int
insert(f, n)
int f,n;
{
	int	s,
		t = dotreplaying(FALSE);

	s = ins_n_times(f,n,n>0);
	if (t)
		advance_one_char();
	(void)update(FALSE);

	return s;
}

int
insertbol(f, n)
int f,n;
{
	int	s,
		t = dotreplaying(FALSE);

	if (dotreplaying(TRUE))
		return insert(f,n);

	(void)firstnonwhite(f,n);
	s = ins_n_times(f,n,n>0);
	if (t)
		advance_one_char();

	return s;
}

int
append(f, n)
int f,n;
{
	advance_one_char();

	return ins_n_times(f,n,TRUE);
}

int
appendeol(f, n)
int f,n;
{
	if (!is_header_line(DOT,curbp))
		gotoeol(FALSE,0);

	return ins_n_times(f,n,FALSE);
}

int
overwritechars(f, n)
int f,n;
{
	int	s,
		t = dotreplaying(FALSE);

	insertmode = OVERWRITE;
	if (b_val(curbp, MDSHOWMODE))
		curwp->w_flag |= WFMODE;

	s = ins_n_times(f,n,FALSE);
	if (t)
		advance_one_char();

	return s;
}

int
replacechar(f, n)
int f,n;
{
	register int	s;
	register int	c;

	if (!f && lLength(DOT.l) == 0)
		return FALSE;

	insertmode = REPLACECHAR;  /* need to fool the SPEC prefix code */
	if (b_val(curbp, MDSHOWMODE))
		curwp->w_flag |= WFMODE;
	(void)update(FALSE);
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
	if (isreturn(c)) {
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
	int    c;		/* command character */
	int backsp_limit;
	static TBUFF *insbuff;
	int osavedmode;

	if (playback && (insbuff != 0))
		tb_first(insbuff);
	else if (!tb_init(&insbuff, abortc))
		return FALSE;

	if (insertmode == FALSE) {
		insertmode = INSERT;
		if (b_val(curbp, MDSHOWMODE))
			curwp->w_flag |= WFMODE;
	}
	osavedmode = savedmode;
	savedmode = insertmode;

	if (b_val(curbp,MDBACKLIMIT))
		backsp_limit = DOT.o;	/* starting offset */
	else
		backsp_limit = 0;	/* no limit on backspaces */

	/* get the next command from the keyboard */
	while(1) {

		if (playback) {
			c = tb_next(insbuff);
		} else {
			(void)update(FALSE);
			if (!tb_append(&insbuff, c = kbd_key())) {
				savedmode = osavedmode;
				return FALSE;
			}
		}

		if (c == abortc ) {
			 /* an unfortunate Vi-ism that ensures one 
				can always type "ESC a" if you're not sure 
				you're in insert mode. */
			if (DOT.o != 0)
				backchar(TRUE,1);
			if (autoindented >= 0) {
				trimline(FALSE);
				autoindented = -1;
			}
			status = TRUE;
			goto leaveinsert;
		} 

		if (c & SPEC) {
			CMDFUNC *cfp;
			int oins;
			cfp = kcod2fnc(c);
			if (cfp) {
				map_check(c);
				oins = insertmode;
				insertmode = FALSE;
				backsp_limit = 0;
				curgoal = getccol(FALSE);
				(void)execute(cfp,FALSE,1);
				insertmode = oins;
			}
			continue;
		}

		if (c == startc || c == stopc) {  /* ^Q and ^S */
			continue;
		}
#if UNIX && defined(SIGTSTP)	/* job control, ^Z */
		else if (c == suspc) {
			status = bktoshell(FALSE,1);
		}
#endif
		else {
			status = inschar(c,&backsp_limit);
		}

		if (status != TRUE) {
 leaveinsert:
			insertmode = FALSE;
			if (b_val(curbp, MDSHOWMODE))
				curwp->w_flag |= WFMODE;
			savedmode = osavedmode;
			return (status);
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
				(void)update(TRUE);
				filesave(FALSE, 0);
				curbp->b_acount = b_val(curbp,VAL_ASAVECNT);
			}
	}
}

int
inschar(c,backsp_limit_p)
int c;
int *backsp_limit_p;
{
	int (*execfunc) P((int, int));	/* ptr to function to execute */

	execfunc = NULL;
	if (c == quotec) {
		execfunc = quote;
	} else {
		/*
		 * If a space was typed, fill column is defined, the
		 * argument is non- negative, wrap mode is enabled, and
		 * we are now past fill column, perform word wrap. 
		 */
		if (isspace(c) && getccol(FALSE) > wrap_at_col()) {
			wrapword(FALSE,1);
			*backsp_limit_p = 0;
		}

		if ( c == '\t') { /* tab */
			execfunc = tab;
			autoindented = -1;
		} else if (c ==  tocntrl('J') ||
			c ==  tocntrl('M')) { /* CR and NL */
			execfunc = newline;
			if (autoindented >= 0) {
				trimline(FALSE);
				autoindented = -1;
			}
			*backsp_limit_p = 0;
		} else if ( isbackspace(c) ||
				c == tocntrl('D') || 
				c == killc ||
				c == wkillc) { /* ^U and ^W */
			/* have we backed thru a "word" yet? */
			int saw_word = FALSE;
			execfunc = nullproc;
			if (c == tocntrl('D')) {
				int goal, col, sw;

				sw = curswval;
				if (autoindented >=0)
					*backsp_limit_p = 0;
				col = getccol(FALSE);
				if (col > 0)
					goal = ((col-1)/sw)*sw;
				else
					goal = 0;
				while (col > goal &&
					DOT.o > *backsp_limit_p) {
					backspace();
					col = getccol(FALSE);
				}
				if (col < goal)
					linsert(goal - col,' ');
			} else {
				while (DOT.o > *backsp_limit_p) {
					if (c == wkillc) {
						if (isspace( lGetc(DOT.l,
								DOT.o-1))) {
							if (saw_word)
								break;
						} else {
							saw_word = TRUE;
						}
					}
					backspace();
					autoindented--;
					if (c != wkillc && c != killc)
						break;
				}
			}
		} else if ( c ==  tocntrl('T')) { /* ^T */
			execfunc = shiftwidth;
		}

	}

	if (execfunc != NULL)
		return (*execfunc)(FALSE, 1);
		
	/* make it a real character again */
	c = kcod2key(c);

	/* if we are in overwrite mode, not at eol,
	   and next char is not a tab or we are at a tab stop,
	   delete a char forword			*/
	if (insertmode == OVERWRITE && DOT.o < lLength(DOT.l) &&
			(char_at(DOT) != '\t' ||
				(DOT.o) % curtabval == curtabval-1)) {
		autoindented = -1;
		ldelete(1L, FALSE);
	}

	/* do the appropriate insertion */
	if ((c == RBRACE) && b_val(curbp, MDCMOD)) {
		return insbrace(1, c);
	} else if (c == '#' && b_val(curbp, MDCMOD)) {
		return inspound();
	}

	autoindented = -1;
	return linsert(1, c);

}

#if ! SMALLER
int
insstring(f, n)
int f, n;
{
	return istring(f,n,INSERT);
}

int
overwstring(f, n)
int f, n;
{
	return istring(f,n,OVERWRITE);
}

/* ask for and insert or overwrite a string into the current */
/* buffer at the current point */
int
istring(f,n,mode)
int f,n;
int mode;
{
	register char *tp;	/* pointer into string to add */
	register int status;	/* status return code */
	int backsp_limit;
	static char tstring[NPAT+1];	/* string to add */

	/* ask for string to insert */
	status = mlreply("String to insert: ", tstring, NPAT);
	if (status != TRUE)
		return(status);


	if (f == FALSE)
		n = 1;

	if (n < 0)
		n = - n;

	insertmode = mode;

	if (b_val(curbp,MDBACKLIMIT))
		backsp_limit = DOT.o;	/* starting offset */
	else
		backsp_limit = 0;	/* no limit on backspaces */

	/* insert it */
	while (n--) {
		tp = &tstring[0];
		while (*tp) {
			status = inschar(*tp++,&backsp_limit);
			if (status != TRUE) {
				insertmode = FALSE;
				return(status);
			}
		}
	}

	insertmode = FALSE;
	return(TRUE);
}
#endif

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
		trimline(FALSE);
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
	if (getccol(FALSE) > wrap_at_col())
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
		indentwas = nextsw(indentwas);
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
		indentwas = nextsw(indentwas);
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
	    
	/* backword() will leave us either on this line, if there's something
		non-blank here, or on the nearest previous non-blank line. */
	if (backword(FALSE,1) == FALSE) {
		if (bracefp) *bracefp = FALSE;
		gomark(FALSE,1);
		return 0;
	}
	ind = indentlen(l_ref(DOT.l));
	if (bracefp) {
		int lc = lastchar(l_ref(DOT.l));
		*bracefp = (lc >= 0 &&
			(lGetc(DOT.l,lc) == LBRACE ||
			 lGetc(DOT.l,lc) == ':') );
	}
		    
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
	int fc;
	    
	MK = DOT;
	    
	/* we want the indent of this line if it's non-blank, or the indent
		of the next non-blank line otherwise */
	fc = firstchar(l_ref(DOT.l));
	if (fc < 0 && forwword(FALSE,1) == FALSE) {
		if (bracefp) *bracefp = FALSE;
		DOT = MK;
		return 0;
	}
	ind = indentlen(l_ref(DOT.l));
	if (bracefp) {
		*bracefp = (lGetc(DOT.l,fc) == RBRACE);
	}
		    
	DOT = MK;
	    
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
	i = firstchar(l_ref(DOT.l));
	if (i > 0)
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
		trimline(FALSE);
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

/*ARGSUSED*/
int
shiftwidth(f, n)
int f,n;
{
	int s;
	int fc;
	fc = firstchar(l_ref(DOT.l));
	if (fc >= 0 && fc < DOT.o) {
		s = linsert(curswval, ' ');
		/* should entab mult ^T inserts */
		return s;
	}
	detabline(TRUE);
	s = curswval - (getccol(FALSE) % curswval);
	if (s)
		linsert(s, ' ');
	if (b_val(curbp,MDTABINSERT))
                entabline(TRUE);
	if (autoindented >= 0) {
		fc = firstchar(l_ref(DOT.l));
		if (fc >= 0)
			autoindented = fc;
		else /* all white */
			autoindented = lLength(DOT.l);
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

	c = tgetc(TRUE);
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

char *
current_modename()
{
	switch (savedmode) {
		default:
			return "command";
		case INSERT:
			return "insert";
		case OVERWRITE:
			return "overwrite";
		case REPLACECHAR:
			return "replace";
	}
}
