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
 *	Revision 1.46  1994/02/11 15:20:08  pgf
 *	implemented 0^D (patch from kev) and ^^D
 *
 * Revision 1.45  1994/02/11  14:11:25  pgf
 * call kbd_key when doing insertions, so we can interpret function keys.
 * preserve goal when doing function keys.
 *
 * Revision 1.44  1994/02/07  12:46:41  pgf
 * allow backspacing over autoindent, whether the backspacelimit mode
 * is set or not
 *
 * Revision 1.43  1994/02/07  12:27:16  pgf
 * treat brackets like braces and parentheses in cmode
 *
 * Revision 1.42  1994/02/03  19:35:12  pgf
 * tom's changes for 3.65
 *
 * Revision 1.41  1994/02/02  18:41:07  pgf
 * added new openup_no_aindent
 *
 * Revision 1.40  1994/02/02  16:44:04  pgf
 * new mode, meta-insert-bindings, which controls whether bindings to meta
 * keys are honored during insert mode or not, and
 * new commands ^A-i and ^A-o which suppress autoindent and cmode during
 * an insert, for the benefit of mouse pasting.
 *
 * Revision 1.39  1994/02/01  19:14:55  pgf
 * fixed core dump resulting from calculating indentlen and checking for '#'
 * char at start of empty line
 *
 * Revision 1.38  1994/01/31  19:52:25  pgf
 * upcased the 'dot_argument" macro to "DOT_ARGUMENT"
 *
 * Revision 1.37  1994/01/31  18:11:03  pgf
 * change kbd_key() to tgetc()
 *
 * Revision 1.36  1993/12/08  17:19:17  pgf
 * don't copy the indent of lines beginning with '#' in cmode.  this
 * causes us to return to the previous indent level after typing a "#if"
 * or "#else" line, etc.
 *
 * Revision 1.35  1993/10/11  17:39:56  pgf
 * pass char to match to fmatchindent()
 *
 * Revision 1.34  1993/09/16  11:06:43  pgf
 * make parentheses act like braces in c-mode -- for indentation purposes, in
 * languages like scheme (and lisp?)
 *
 * Revision 1.33  1993/09/10  16:06:49  pgf
 * tom's 3.61 changes
 *
 * Revision 1.32  1993/09/03  09:11:54  pgf
 * tom's 3.60 changes
 *
 * Revision 1.31  1993/08/13  16:32:50  pgf
 * tom's 3.58 changes
 *
 * Revision 1.30  1993/08/05  14:29:12  pgf
 * tom's 3.57 changes
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
 */

#include	"estruct.h"
#include	"edef.h"

#define	DOT_ARGUMENT	((dotcmdmode == PLAY) && dotcmdarg)

#define	BackspaceLimit() (b_val(curbp,MDBACKLIMIT) && autoindented <= 0)\
			? DOT.o\
			: w_left_margin(curwp)

static	int	wrap_at_col P(( void ));
static	void	advance_one_char P(( void ));
static	int	ins_n_times P(( int, int, int ));
static	int	ins_anytime P(( int, int, int, int * ));

static	int	savedmode;  /* value of insertmode maintained through subfuncs */

static int allow_aindent = TRUE;
static int skipindent;

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
	register int status = TRUE;
	register int i;
	int	flag	= FALSE;

	if (!f || n < 0)
		n = 1;

	for (i = 0; i < n; i++) {
		if ((status = ins_anytime((i != 0), i, n, &flag)) != TRUE)
			break;
		if (advance && !flag)
			advance_one_char();
	}
	return status;
}

/* open lines up before this one */
int
openup(f,n)
int f,n;
{
	register int s;

	if (!f) n = 1;
	if (n < 0) return (FALSE);
	if (n == 0) return ins();

	(void)gotobol(TRUE,1);

	/* if we are in C mode and this is a default <NL> */
	if (allow_aindent && n == 1 && 
		(b_val(curbp,MDCMOD) || b_val(curbp,MDAIND)) &&
				    !is_header_line(DOT,curbp)) {
		s = indented_newline_above();
		if (s != TRUE) return (s);

		return(ins());
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

	return(ins());
}

/*
 * as above, but override all autoindenting and cmode-ing
 */
int
openup_no_aindent(f, n)
int f,n;
{
    	int s;
	int oallow = allow_aindent;
	allow_aindent = FALSE;
	s = openup(f,n);
	allow_aindent = oallow;
	return s;
}

/* open lines up after this one */
int
opendown(f,n)
int f,n;
{
	register int	s;

	if (!f) n = 1;
	if (n < 0) return (FALSE);
	if (n == 0) return ins();

	s = openlines(n);
	if (s != TRUE)
		return (s);

	return(ins());
}

/*
 * as above, but override all autoindenting and cmode-ing
 */
int
opendown_no_aindent(f, n)
int f,n;
{
    	int s;
	int oallow = allow_aindent;
	allow_aindent = FALSE;
	s = opendown(f,n);
	allow_aindent = oallow;
	return s;
}

/*
 * Open up some blank space. The basic plan is to insert a bunch of newlines,
 * and then back up over them.
 *
 * This interprets the repeat-count for the 'o' and 'O' commands.  Unlike vi
 * (which does not use the repeat-count), this specifies the number of blank
 * lines to create before proceeding with inserting the string-argument of the
 * command.
 */
int
openlines(n)
int n;
{
	register int i = n;			/* Insert newlines. */
	register int s = TRUE;
	while (i-- && s==TRUE) {
		(void)gotoeol(FALSE,1);
		s = newline(TRUE,1);
	}
	if (s == TRUE && n)			/* Then back up overtop */
		backline(TRUE, n-1);		/* of them all.		 */

	curgoal = -1;

	return s;
}

/*
 * Implements the vi 'i' command.
 */
int
insert(f, n)
int f,n;
{
	return ins_n_times(f,n,TRUE);
}

/*
 * as above, but override all autoindenting and cmode-ing
 */
int
insert_no_aindent(f, n)
int f,n;
{
    	int s;
	int oallow = allow_aindent;
	allow_aindent = FALSE;
	s = ins_n_times(f,n,TRUE);
	allow_aindent = oallow;
	return s;
}

/*
 * Implements the vi 'I' command.
 */
int
insertbol(f, n)
int f,n;
{
	if (!DOT_ARGUMENT || (dotcmdrep == dotcmdcnt))
		(void)firstnonwhite(FALSE,1);
	return ins_n_times(f,n,TRUE);
}

/*
 * Implements the vi 'a' command.
 */
int
append(f, n)
int f,n;
{
	advance_one_char();

	return ins_n_times(f,n, !DOT_ARGUMENT);
}

/*
 * Implements the vi 'A' command.
 */
int
appendeol(f, n)
int f,n;
{
	if (!is_header_line(DOT,curbp))
		(void)gotoeol(FALSE,0);

	return ins_n_times(f,n,TRUE);
}

/*
 * Implements the vi 'R' command.
 *
 * This takes an optional repeat-count and a string-argument.  The repeat-count
 * (default 1) specifies the number of times that the string argument is
 * inserted.  The length of the string-argument itself determines the number of
 * characters (beginning with the cursor position) to delete before beginning
 * the insertion.
 */
int
overwritechars(f, n)
int f,n;
{
	insertmode = OVERWRITE;
	if (b_val(curbp, MDSHOWMODE))
		curwp->w_flag |= WFMODE;

	return ins_n_times(f,n, TRUE);
}

/*
 * Implements the vi 'r' command.
 *
 * This takes an optional repeat-count and a single-character argument.  The
 * repeat-count (default 1) specifies the number of characters beginning with
 * the cursor position that are replaced by the argument.  Newline is treated
 * differently from the other characters (only one newline is inserted).
 *
 * Unlike vi, the number of characters replaced can be longer than a line.
 * Also, vile allows quoted characters.
 */
int
replacechar(f, n)
int f,n;
{
	register int	s = TRUE;
	register int	t = FALSE;
	register int	c;

	if (!f && lLength(DOT.l) == 0)
		return FALSE;

	insertmode = REPLACECHAR;  /* need to fool the SPEC prefix code */
	if (b_val(curbp, MDSHOWMODE))
		curwp->w_flag |= WFMODE;
	if (dotcmdmode != PLAY)
		(void)update(FALSE);
	c = tgetc(FALSE);
	insertmode = FALSE;
	curwp->w_flag |= WFMODE;

	if (!f || !n)
		n = 1;
	if (n < 0 || c == abortc)
		s = FALSE;
	else {
		int	vi_fix = (!DOT_ARGUMENT || (dotcmdrep <= 1));

		(void)ldelete((long)n, FALSE);
		if (c == quotec) {
			t = s = quote(f,n);
		} else {
			if (isreturn(c)) {
				if (vi_fix)
					s = lnewline();
			} else {
				if (isbackspace(c)) {	/* vi beeps here */
					s = TRUE;	/* replaced with nothing */
				} else {
					t = s = linsert(n, c);
				}
			}
		}
		if ((t == TRUE) && (DOT.o > w_left_margin(curwp)) && vi_fix)
			s = backchar(FALSE,1);
	}
	return s;
}

/*
 * This routine performs the principal decoding for insert mode (i.e.., the
 * i,I,a,A,R commands).  It is invoked via 'ins_n_times()', which loops over
 * the repeat-count for direct commands.  One complicating factor is that
 * 'ins_n_times()' (actually its callers) is called once for each repetition in
 * a '.' command.  At this level we compute the effective loop counter (the '.'
 * and the direct commands), because we have to handle the vi-compatibilty case
 * of inserting a newline.
 *
 * We stop repeating insert after the first newline in the insertion-string
 * (that's what vi does).  If a user types
 *
 *	3iABC<nl>foo<esc>
 *
 * then we want to insert
 *
 *	ABCABCABC<nl>foo
 */
static last_insert_char;

static int
ins_anytime(playback, cur_count, max_count, splice)
int playback;
int cur_count;
int max_count;
int *splice;
{
#if OPT_MOUSE
	WINDOW	*wp0 = curwp;
#endif
	register int status;
	int	c;		/* command character */
	int backsp_limit;
	static TBUFF *insbuff;
	int osavedmode;
	int is_func_prefix = FALSE;

	if (DOT_ARGUMENT) {
		max_count = cur_count + dotcmdcnt;
		cur_count += dotcmdcnt - dotcmdrep;
	}

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

	backsp_limit = BackspaceLimit();

	last_insert_char = '\0';

	while(1) {

		/*
		 * Read another character from the insertion-string.
		 */
		c = abortc;
		if (playback) {
			if (*splice && !tb_more(insbuff))
				playback = FALSE;
			else
				c = tb_next(insbuff);
		}
		if (!playback) {
			if (dotcmdmode != PLAY)
				(void)update(FALSE);
			if (!tb_append(&insbuff, c = kbd_key())) {
				status = FALSE;
				break;
			}
		}

		if (is_func_prefix) {
		    	/* if we're allowed to honor meta-character bindings,
				then see if it's bound to something, and
				insert it if not */
			CMDFUNC *cfp = kcod2fnc(SPEC|c);
			if (cfp) {
				map_check(c);
				backsp_limit = w_left_margin(curwp);
				if (curgoal < 0)
					curgoal = getccol(FALSE);
				(void)execute(cfp,FALSE,1);
			}
			is_func_prefix = FALSE;
			continue;
		}
		if (isreturn(c)) {
			if ((cur_count+1) < max_count) {
				if (DOT_ARGUMENT) {
					while (tb_more(dotcmd))
						(void)kbd_key();
				}
				*splice = TRUE;
				status = TRUE;
				break;
			} else if (DOT_ARGUMENT) {
				*splice = TRUE;
			}
		}
#if OPT_MOUSE
		/*
		 * Prevent user from starting insertion into a modifiable
		 * buffer, then clicking on a readonly buffer to continue
		 * inserting.  This assumes that 'setcursor()' handles entry
		 * into the readonly buffer.
		 */
		if (curwp != wp0) {
			if (b_val(curbp, MDVIEW) || !insertmode) {
				tungetc(c);
				status = FALSE;
				break;
			}
			wp0 = curwp;
			backsp_limit = BackspaceLimit();
		}
#endif

		/*
		 * Decode the character
		 */
		if (c == abortc) {
			 /* an unfortunate Vi-ism that ensures one
				can always type "ESC a" if you're not sure
				you're in insert mode. */
			if (DOT.o > w_left_margin(curwp))
				backchar(TRUE,1);
			if (autoindented >= 0) {
				trimline(FALSE);
				autoindented = -1;
			}
			if (cur_count+1 == max_count)
				*splice = TRUE;
			status = TRUE;
			break;
		} else /* the only way to get through kbd_key() and
			* tgetc() and end up with c == poundc and a pushed-
			* back character (tungotc) is if this was the
			* result of a function-key press.
			* since this breaks on repeated inserts (we end up
			* inserting the #c instead of executing the
			* function), we allow for 'altpoundc', which is
			* presumably something the user will never want to
			* actually insert -- we never try to insert it, we
			* always execute instead.  */
			if ((c == poundc && tungotc >= 0) ||
		    		(c == altpoundc && altpoundc != poundc)) {
		    	is_func_prefix = TRUE;
			continue;
		} else if ((c & HIGHBIT) && b_val(curbp, MDMETAINSBIND)) {
		    	/* if we're allowed to honor meta-character bindings,
				then see if it's bound to something, and
				insert it if not */
			CMDFUNC *cfp = kcod2fnc(c);
			if (cfp) {
				map_check(c);
				backsp_limit = w_left_margin(curwp);
				if (curgoal < 0)
					curgoal = getccol(FALSE);
				(void)execute(cfp,FALSE,1);
				continue;
			}
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
			curgoal = -1;
		}

		if (status != TRUE)
			break;

#if CFENCE
		/* check for CMODE fence matching */
		if ((c == RBRACE || c == RPAREN || c == RBRACK ) &&
						b_val(curbp, MDSHOWMAT))
			fmatch(c);
#endif

		/* check auto-save mode */
		if (b_val(curbp, MDASAVE)) {
			if (--curbp->b_acount <= 0) {
				/* and save the file if needed */
				(void)update(TRUE);
				filesave(FALSE, 0);
				curbp->b_acount = b_val(curbp,VAL_ASAVECNT);
			}
		}
	}

	insertmode = FALSE;
	if (b_val(curbp, MDSHOWMODE))
		curwp->w_flag |= WFMODE;
	savedmode = osavedmode;
	return (status);
}

/* grunt routine for insert mode */
int
ins()
{
	int	flag;
	return ins_anytime(FALSE,1,1,&flag);
}

static int
isallspace(ln,lb,ub)
LINEPTR ln;
int lb;
int ub;
{
	while (lb <= ub) {
	    if (!isspace(lGetc(ln,ub)))
		return 0;
	    ub--;
	}
	return 1;
}

int
inschar(c,backsp_limit_p)
int c;
int *backsp_limit_p;
{
	CmdFunc execfunc;	/* ptr to function to execute */

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
			*backsp_limit_p = w_left_margin(curwp);
		}

		if ( c == '\t') { /* tab */
			execfunc = tab;
			autoindented = -1;
		} else if (isreturn(c)) {
			execfunc = newline;
			if (autoindented >= 0) {
				trimline(FALSE);
				autoindented = -1;
			}
			*backsp_limit_p = w_left_margin(curwp);
		} else if ( isbackspace(c) ||
				c == tocntrl('D') ||
				c == killc ||
				c == wkillc) { /* ^U and ^W */
			execfunc = nullproc;
			/* all this says -- "is this a regular ^D for
				backing up a shiftwidth?".  otherwise,
				we treat it as ^U, below */
			if (c == tocntrl('D')
			 && !(DOT.o > *backsp_limit_p
			      && ((lGetc(DOT.l,DOT.o-1) == '0'
				  && last_insert_char == '0')
				  || (lGetc(DOT.l,DOT.o-1) == '^'
				  && last_insert_char == '^'))
			      && isallspace(DOT.l,w_left_margin(curwp),
			                          DOT.o-2))) {
				int goal, col, sw;

				sw = curswval;
				if (autoindented >=0)
					*backsp_limit_p = w_left_margin(curwp);
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
				/* have we backed thru a "word" yet? */
				int saw_word = FALSE;

				/* was it '^^D'?  then set the flag
					that tells us to skip a line
					when calculating the autoindent
					on the next newline */
				if (c == tocntrl('D') && 
					last_insert_char == '^')
					skipindent = 1;

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
					if (c != wkillc && c != killc
					    && c != tocntrl('D'))
						break;
				}
			}
		} else if ( c ==  tocntrl('T')) { /* ^T */
			execfunc = shiftwidth;
		}

		last_insert_char = c;

	}

	if (execfunc != NULL)
		return (*execfunc)(FALSE, 1);

	/* make it a real character again */
	c = kcod2key(c);

	/* if we are in overwrite mode, not at eol,
	   and next char is not a tab or we are at a tab stop,
	   delete a char forword			*/
	if ((insertmode == OVERWRITE)
	 && (!DOT_ARGUMENT || (dotcmdrep <= 1))
	 && (DOT.o < lLength(DOT.l))
	 && (char_at(DOT) != '\t' || DOT.o % curtabval == curtabval-1)) {
		autoindented = -1;
		(void)ldelete(1L, FALSE);
	}

	/* do the appropriate insertion */
	if (allow_aindent && b_val(curbp, MDCMOD)) {
	    if (((c == RBRACE) || (c == RPAREN) || (c == RBRACK))) {
		    return insbrace(1, c);
	    } else if (c == '#') {
		    return inspound();
	    }
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

	backsp_limit = BackspaceLimit();

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


	/* if we are in C or auto-indent modes and this is a default <NL> */
	if (allow_aindent && n == 1 && 
		(b_val(curbp,MDCMOD) || b_val(curbp,MDAIND)) &&
						!is_header_line(DOT,curbp))
		return indented_newline();

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
indented_newline()
{
	int cmode = allow_aindent && b_val(curbp, MDCMOD);
	register int indentwas; /* indent to reproduce */
	int bracef; /* was there a brace at the end of line? */

	indentwas = previndent(&bracef);
	skipindent = 0;

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
indented_newline_above()
{
	int cmode = allow_aindent && b_val(curbp, MDCMOD);
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
	int cmode = allow_aindent && b_val(curbp, MDCMOD);

	if (bracefp) *bracefp = FALSE;

	MK = DOT;

	/* backword() will leave us either on this line, if there's something
		non-blank here, or on the nearest previous non-blank line. */
	/* (at start of buffer, may leave us on empty line) */
	do {
	    if (backword(FALSE,1) == FALSE || llength(DOT.l) == 0) {
		    gomark(FALSE,1);
		    return 0;
	    }
	    DOT.o = 0;
	/* if the line starts with a #, then don't copy its indent */
	} while ((skipindent-- > 0) || (cmode && lGetc(DOT.l, 0) == '#'));

	ind = indentlen(l_ref(DOT.l));
	if (bracefp) {
		int lc = lastchar(l_ref(DOT.l));
		*bracefp = (lc >= 0 &&
			(lGetc(DOT.l,lc) == LBRACE ||
			 lGetc(DOT.l,lc) == LPAREN ||
			 lGetc(DOT.l,lc) == LBRACK ||
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
		*bracefp = ((lGetc(DOT.l,fc) == RBRACE) ||
				(lGetc(DOT.l,fc) == RPAREN) ||
				(lGetc(DOT.l,fc) == RBRACK));
	}

	DOT = MK;

	return ind;
}

int
doindent(ind)
int ind;
{
	register int i;

	/* first clean up existing leading whitespace */
	DOT.o = w_left_margin(curwp);
	i = firstchar(l_ref(DOT.l));
	if (i > 0)
		(void)ldelete((long)i,FALSE);

	autoindented = 0;
	/* if no indent was asked for, we're done */
	if (ind > 0) {
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


/* insert a brace or paren into the text here... we are in CMODE */
int
insbrace(n, c)
int n;	/* repeat count */
int c;	/* brace/paren to insert (always '}' or ')' for now) */
{

#if ! CFENCE
	/* wouldn't want to back up from here, but fences might take us
		forward */
	/* if we are at the beginning of the line, no go */
	if (DOT.o <= w_left_margin(curwp))
		return(linsert(n,c));
#endif

	if (autoindented >= 0) {
		trimline(FALSE);
	} else {
		return linsert(n,c);
	}
	skipindent = 0;
#if ! CFENCE /* no fences?	then put brace one tab in from previous line */
	doindent(((previndent(NULL)-1) / curtabval) * curtabval);
#else /* line up brace with the line containing its match */
	doindent(fmatchindent(c));
#endif
	autoindented = -1;

	/* and insert the required brace(s) */
	return(linsert(n, c));
}

int
inspound()	/* insert a # into the text here...we are in CMODE */
{

	/* if we are at the beginning of the line, no go */
	if (DOT.o <= w_left_margin(curwp))
		return(linsert(1,'#'));

	if (autoindented > 0) { /* must all be whitespace before us */
		DOT.o = w_left_margin(curwp);
		(void)ldelete((long)autoindented,FALSE);
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
	if (fc >= w_left_margin(curwp) && fc < DOT.o) {
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

#if OPT_EVAL
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
#endif
