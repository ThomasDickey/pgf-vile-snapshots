/*	INPUT:	Various input routines for MicroEMACS
 *		written by Daniel Lawrence
 *		5/9/86
 *
 * $Log: input.c,v $
 * Revision 1.60  1993/03/17 09:57:59  pgf
 * fix for calling shorten_path() correctly
 *
 * Revision 1.59  1993/03/16  10:53:21  pgf
 * see 3.36 section of CHANGES file
 *
 * Revision 1.58  1993/03/15  12:01:59  pgf
 * another fix to kbd_reply, so that search delimiters work correctly
 *
 * Revision 1.57  1993/03/11  17:55:36  pgf
 * don't return TRUE from kbd_reply if backspaced past start of buff
 *
 * Revision 1.56  1993/03/05  17:50:54  pgf
 * see CHANGES, 3.35 section
 *
 * Revision 1.55  1993/02/24  10:59:02  pgf
 * see 3.34 changes, in CHANGES file
 *
 * Revision 1.54  1993/02/15  10:37:31  pgf
 * cleanup for gcc-2.3's -Wall warnings
 *
 * Revision 1.53  1993/02/12  10:42:55  pgf
 * use insertion_cmd() routine to get the char that puts us in insert mode
 *
 * Revision 1.52  1993/02/08  14:53:35  pgf
 * see CHANGES, 3.32 section
 *
 * Revision 1.51  1993/01/16  10:35:40  foxharp
 * support for scrtch and shpipe chars in screen_string(), and use find_alt()
 * to pick up name of '#' buffer, rather than hist_lookup(1)
 *
 * Revision 1.50  1992/12/13  13:32:36  foxharp
 * got rid of extraneous assign
 *
 * Revision 1.49  1992/12/05  13:22:10  foxharp
 * make sure we escape eolchar with '\' if passed in in kbd_strings buffer,
 * since the user would have had to type the '\' to put it there themselves
 *
 * Revision 1.48  1992/12/04  09:25:34  foxharp
 * deleted unused assigns, no longer propagate pointer to block local
 * in kbd_string, and fix expansion arg to mlreply_no_bs()
 *
 * Revision 1.47  1992/12/03  00:32:59  foxharp
 * added new mlreply_no_bs, which doesn't do backslash processing
 *
 * Revision 1.46  1992/11/19  09:07:30  foxharp
 * added check on recursive replay in dotcmdplay() -- I think we should
 * never play or record a call to dotcmdplay, so we abort if we find ourselves
 * doing so.
 * also added kdone() call to finish up after ksetup()
 *
 * Revision 1.45  1992/08/20  23:40:48  foxharp
 * typo fixes -- thanks, eric
 *
 * Revision 1.44  1992/07/24  07:49:38  foxharp
 * shorten_name changes
 *
 * Revision 1.43  1992/07/18  13:13:56  foxharp
 * put all path-shortening in one place (shorten_path()), and took out some old code now
 * unnecessary
 *
 * Revision 1.42  1992/07/16  22:08:34  foxharp
 * make keyboard macros redoable -- when out of input on a dotcmd, be
 * sure to check for pending kbdmode input
 *
 * Revision 1.41  1992/07/15  23:23:12  foxharp
 * made '80i-ESC' work
 *
 * Revision 1.40  1992/07/04  14:32:08  foxharp
 * rearranged/improved the insertmode arrow key code
 *
 * Revision 1.39  1992/06/25  23:00:50  foxharp
 * changes for dos/ibmpc
 *
 * Revision 1.38  1992/05/19  08:55:44  foxharp
 * more prototype and shadowed decl fixups
 *
 * Revision 1.37  1992/05/16  12:00:31  pgf
 * prototypes/ansi/void-int stuff/microsoftC
 *
 * Revision 1.36  1992/03/19  23:21:33  pgf
 * linux portability (pathn)
 *
 * Revision 1.35  1992/03/07  10:28:52  pgf
 * don't write a null past the end of the input buffer in kbd_string
 *
 * Revision 1.34  1992/03/03  21:58:32  pgf
 * minor optimization in screen_string
 *
 * Revision 1.33  1992/03/03  09:35:52  pgf
 * added support for getting "words" out of the buffer via variables --
 * needed _nonspace character type
 *
 * Revision 1.32  1992/03/03  08:42:01  pgf
 * took out pre_colon_pos
 *
 * Revision 1.31  1992/02/17  09:05:12  pgf
 * make "RECORDED_ESC" work on machines whose natural chars are unsigned, and
 * add support for "pre_colon_pos", which is the value of DOT just before the
 * named command was run -- this lets ':' expand correctly in all cases
 *
 * Revision 1.30  1992/01/06  23:10:56  pgf
 * try no update() in get_recorded_char() -- don't know why it's
 * necessary
 *
 * Revision 1.29  1992/01/05  00:06:13  pgf
 * split mlwrite into mlwrite/mlprompt/mlforce to make errors visible more
 * often.  also normalized message appearance somewhat.
 *
 * Revision 1.28  1991/12/11  06:30:58  pgf
 * fixed backslashing, yet again -- should now be able to search for a
 * slash in a buffer
 *
 * Revision 1.27  1991/11/08  13:24:05  pgf
 * ifdefed unused function
 *
 * Revision 1.26  1991/11/01  14:38:00  pgf
 * saber cleanup
 *
 * Revision 1.25  1991/10/29  03:00:53  pgf
 * added speckey function, for '#' prefixing, and allow ESC O x in addition
 * to ESC [ x as ANSI fkeys
 *
 * Revision 1.24  1991/10/23  14:20:53  pgf
 * changes to fix interactions between dotcmdmode and kbdmode and tungetc
 *
 * Revision 1.23  1991/10/22  14:36:21  pgf
 * bug in ANSI_SPEC -- local declaration of f_insert hid global
 *
 * Revision 1.22  1991/10/22  03:08:09  pgf
 * made wkillc work in kbd_string
 *
 * Revision 1.21  1991/09/26  13:15:03  pgf
 * make backslash processing optional in kbd_string, and
 * fix type mismatch in ANSI_SPEC code (f_insert)
 *
 * Revision 1.20  1991/09/17  00:51:17  pgf
 * fixed even more backslashing bugs
 *
 * Revision 1.19  1991/09/13  03:27:06  pgf
 * bugfix for backslash changes
 *
 * Revision 1.18  1991/09/13  03:06:39  pgf
 * backslashing now works -- expansion chars and backslashes can be
 * escaped properly
 *
 * Revision 1.17  1991/09/12  13:03:16  pgf
 * kbd_string now recognizes leading eolchar corectly, but there are still
 * problems with trying to quote it, as in :s/xxx/\//g to change xxx to / .
 *
 * Revision 1.16  1991/09/12  12:27:41	pgf
 * un-record characters pushed back with tungetc
 *
 * Revision 1.15  1991/09/10  00:46:57	pgf
 * cleanup of the dotcmd stuff, to prevent catnap() for escape sequences
 * during playback
 *
 * Revision 1.14  1991/08/16  11:01:39	pgf
 * added catnap() before typahead check on esc char in ANS_SPEC, and
 * added REPLACECHAR special check on ANSI_SPEC, and
 * allow quoting of %, #, :, with \ in kbd_string, so they don't expand
 *
 * Revision 1.13  1991/08/12  15:06:21	pgf
 * added ANSI_SPEC capability -- can now use the arrow keys from
 * command or insert mode
 *
 * Revision 1.12  1991/08/12  10:24:16	pgf
 * interrupts can now interrupt keyboard recording
 *
 * Revision 1.11  1991/08/07  12:35:07	pgf
 * added RCS log messages
 *
 * revision 1.10
 * date: 1991/06/26 09:37:37;
 * removed ifdef BEFORE
 * 
 * revision 1.9
 * date: 1991/06/25 19:52:47;
 * massive data structure restructure
 * 
 * revision 1.8
 * date: 1991/06/04 09:20:31;
 * kcod2key is now a macro
 * 
 * revision 1.7
 * date: 1991/06/03 17:34:53;
 * switch from "meta" etc. to "ctla" etc.
 * 
 * revision 1.6
 * date: 1991/06/03 10:22:14;
 * took out some old ifdefs, and
 * fixed "can't escape a slash w/ a backslash" bug in searching
 * 
 * revision 1.5
 * date: 1991/02/19 18:05:36;
 * took out extraneous check
 * 
 * revision 1.4
 * date: 1990/12/03 12:02:16;
 * change 'word-under-cursor' expansion char to ':'
 * 
 * revision 1.3
 * date: 1990/11/07 14:28:41;
 * added '+' expansion character, to expand to the path-style string under the
 * cursor
 * 
 * revision 1.2
 * date: 1990/10/03 16:00:52;
 * make backspace work for everyone
 * 
 * revision 1.1
 * date: 1990/09/21 10:25:28;
 * initial vile RCS revision
 */

#include	<stdio.h>
#include	"estruct.h"
#include	"edef.h"

#define	DEFAULT_REG	-1

#define RECORDED_ESC	-2

typedef	struct	_kstack	{
	struct	_kstack	*m_link;
	int	m_save;		/* old value of 'kbdmode'		*/
	int	m_indx;		/* index identifying this macro		*/
	int	m_rept;		/* the number of times to execute the macro */
	TBUFF  *m_kbdm;		/* the macro-text to execute		*/
	TBUFF  *m_dots;		/* workspace for "." command		*/
	TBUFF  *m_DOTS;		/* save-location for 'dotcmd'		*/
	} KSTACK;

static	KSTACK *KbdStack;	/* keyboard/@-macros that are replaying */
static	TBUFF  *dotcmd;		/* dot commands, recorded	*/
static	TBUFF  *KbdMacro;	/* keyboard macro, recorded	*/
static	int	last_eolchar;	/* records last eolchar-match in 'kbd_string' */

static	int	dotcmdcnt;	/* original # of repetitions	*/
static	int	dotcmdrep;	/* number of repetitions	*/

/*
 * Returns a pointer to the buffer that we use for saving text to replay with
 * the "." command.
 */
static TBUFF *
TempDot(init)
int	init;
{
	static	TBUFF  *tmpcmd;		/* dot commands, 'til we're sure */
	if (kbdmode == PLAY) {
		if (init)
			(void)tb_init(&(KbdStack->m_dots), abortc);
		return KbdStack->m_dots;
	}
	if (init || (tmpcmd == 0))
		(void)tb_init(&tmpcmd, abortc);
	return tmpcmd;
}

/*
 * Dummy function to use when 'kbd_string()' does not handle automatic completion
 */
/*ARGSUSED*/
int
no_completion(c, buf, pos)
int	c;
char	*buf;
int	*pos;
{
	return FALSE;
}

/*
 * Ask a yes or no question in the message line. Return either TRUE, FALSE, or
 * ABORT. The ABORT status is returned if the user bumps out of the question
 * with a ^G. Used any time a confirmation is required.
 */

int
mlyesno(prompt)
char *prompt;
{
	char c; 		/* input character */

	for (;;) {
#if	NeWS
		newsimmediateon() ;
		mlprompt(,"%s [y/n]? ",prompt);
		c = tgetc(FALSE);	/* get the response */
		newsimmediateoff() ;
#else
		mlprompt("%s [y/n]? ",prompt);
		c = tgetc(FALSE);	/* get the response */
#endif

		if (c == kcod2key(abortc))		/* Bail out! */
			return(ABORT);

		if (c=='y' || c=='Y')
			return(TRUE);

		if (c=='n' || c=='N')
			return(FALSE);
	}
}

/*
 * Write a prompt into the message line, then read back a response. Keep
 * track of the physical position of the cursor. If we are in a keyboard
 * macro throw the prompt away, and return the remembered response. This
 * lets macros run at full speed. The reply is always terminated by a carriage
 * return. Handle erase, kill, and abort keys.
 */

int
mlreply(prompt, buf, bufn)
char *prompt;
char *buf;
int bufn;
{
	return kbd_string(prompt, buf, bufn, '\n', KBD_NORMAL, no_completion);
}

#ifdef NEEDED
/* as above, but don't expand special punctuation, like #, %, ~, etc. */
int
mlreply_no_exp(prompt, buf, bufn)
char *prompt;
char *buf;
int bufn;
{
	return kbd_string(prompt, buf, bufn, '\n', KBD_QUOTES, no_completion);
}
#endif

/* as above, but don't do anything to backslashes */
int
mlreply_no_bs(prompt, buf, bufn)
char *prompt;
char *buf;
int bufn;
{
	return kbd_string(prompt, buf, bufn, '\n', KBD_EXPAND, no_completion);
}

/* as above, but neither expand nor do anything to backslashes */
int
mlreply_no_opts(prompt, buf, bufn)
char *prompt;
char *buf;
int bufn;
{
	return kbd_string(prompt, buf, bufn, '\n', 0, no_completion);
}

/*	kcod2key:	translate 10-bit keycode to single key value */
/* probably defined as a macro in estruct.h */
#ifndef kcod2key
kcod2key(c)
int c;
{
	return c & (N_chars-1);
}
#endif


/* the numbered buffer names increment each time they are referenced */
void
incr_dot_kregnum()
{
	if (dotcmdmode == PLAY) {
		register int	c = tb_peek(dotcmd);
		if (isdigit(c) && c < '9')
			(void)tb_next(dotcmd);
	}
}

int tungotc = -1;

void
tungetc(c)
int c;
{
	tungotc = c;
	if (dotcmdmode == RECORD)
		tb_unput(TempDot(FALSE));

	if (kbdmode == RECORD)
		tb_unput(KbdMacro);
}

int
tpeekc()
{
	return tungotc;
}

/*
 * Record a character for "." commands
 */
static void
record_dot_char(c)
int c;
{
	if (dotcmdmode == RECORD) {
		TBUFF	*tmp = TempDot(FALSE);
		(void)tb_append(&tmp, c);
	}
}

/*
 * Record a character for kbd-macros
 */
static void
record_kbd_char(c)
int c;
{
	if (kbdmode == RECORD)
		(void)tb_append(&KbdMacro, c);
}

/* if we should preserve this input, do so */
static void
record_char(c)
int c;
{
	if (c == ESC)
		c = RECORDED_ESC;

	record_dot_char(c);
	record_kbd_char(c);
}

/* get the next character of a replayed '.' or macro */
int
get_recorded_char(eatit)
int eatit;  /* consume the character? */
{
	register int	c = -1;
	register TBUFF	*buffer;

	if (dotcmdmode == PLAY) {

		if (interrupted) {
			dotcmdmode = STOP;
			c = kcod2key(abortc);
			return c;
		} else {

			if (!tb_more(buffer = dotcmd)) {
				if (!eatit) {
					if (dotcmdrep > 1)
						return tb_get(buffer, 0);
				} else { /* at the end of last repetition?  */
					if (--dotcmdrep < 1) {
						dotcmdmode = STOP;
						(void)dotcmdbegin();
						/* immediately start recording
						 * again, just in case.
						 */
					} else {
						/* reset the macro to the
						 * beginning for the next rep.
						 */
						tb_first(buffer);
					}
				}
			}

			/* if there is some left... */
			if (tb_more(buffer)) {
				if (eatit)
					c = tb_next(buffer);
				else
					c = tb_peek(buffer);
				return c;
			}
		}
	}

	if (kbdmode == PLAY) { /* if we are playing a keyboard macro back, */

		if (interrupted) {
			finish_kbm();
			c = kcod2key(abortc);
		} else {

			if (!tb_more(buffer = KbdStack->m_kbdm)) {
				if (--(KbdStack->m_rept) >= 1)
					tb_first(buffer);
				else
					finish_kbm();
			}

			if (kbdmode == PLAY) {
				if (eatit)
					record_dot_char(c = tb_next(buffer));
				else
					c = tb_peek(buffer);
			}
		}
	}

	return c;
}

/*	tgetc:	Get a key from the terminal driver, resolve any keyboard
		macro action					*/
int 
tgetc(quoted)
int quoted;
{
	register int c;	/* fetched character */

	if (tungotc >= 0) {
		c = tungotc;
		tungotc = -1;
		record_char(c);
	} else {
		if ((c = get_recorded_char(TRUE)) == -1) {
			/* fetch a character from the terminal driver */ 
			interrupted = 0;
			if ((c = TTgetc()) == -1)
				c = kcod2key(intrc);
			if (!quoted && (c == kcod2key(intrc)))
				c = kcod2key(abortc);
			else
				record_char(c);
		}
	}

	if (!quoted && (c == RECORDED_ESC))
		c = ESC;

	/* and finally give the char back */
	return(lastkey = c);
}

/*	KBD_KEY:	Get one keystroke. The only prefix legal here
			is the SPEC prefix.  */
int
kbd_key()
{
	int    c;

	/* get a keystroke */
	c = tgetc(FALSE);

#if ANSI_SPEC
	if (insert_mode_was && last1key == -abortc) {
		int ic;
		/* then we just read the command we pushed before */
		if ((ic = insertion_cmd()) == -1) /* can we re-enter? */
			mlforce("[Can't re-enter insert mode]");
		else
			tungetc(ic);
		insertmode = insert_mode_was;
		insert_mode_was = FALSE;
	}

	if ((unsigned char)c == (unsigned char)RECORDED_ESC) {
		/* if this is being replayed... */
		/* ...then only look for esc sequences if there's input left */
		if (get_recorded_char(FALSE) != -1)
			c = ESC;
		else
			return (last1key = ESC);
	}

	if (c == ESC) {
		int nextc;

		/* if there's no recorded input, and no user typeahead */
		if ((nextc = get_recorded_char(FALSE)) == -1 && !typahead())
			catnap(50); /* give it a little extra time... */

		/* and then, if there _was_ recorded input or new typahead... */
		if (nextc != -1 || typahead()) {
			c = tgetc(FALSE);
			if (c == '[' || c == 'O') {
				/* eat ansi sequences */
				c = tgetc(FALSE);
				if (abortc != ESC || !insertmode)
					return (last1key = SPEC | c);
				if (insertmode == REPLACECHAR) {
					/* eat the sequence, but return abort */
					return abortc;
				}
				/* remember we were in insert mode */
				insert_mode_was = insertmode;
				/* save the code, but return flag to
					ins() so it can clean up */
				tungetc(SPEC | c);
				return(last1key = -abortc);
			} else {
				if (abortc != ESC)
					return (last1key = c);
				tungetc(c);
				return (last1key = ESC);
			}
		}
	}
#endif

#if	MSDOS | ST520
	if (c == 0) {			/* Apply SPEC prefix	*/
		c = tgetc(FALSE);
		return(last1key = SPEC | c);
	}
#endif

#if	AMIGA
	/* apply SPEC prefix */
	if ((unsigned)c == 155) {
		int	d;
		c = tgetc(FALSE);

		/* first try to see if it is a cursor key */
		if ((c >= 'A' && c <= 'D') || c == 'S' || c == 'T') {
			if (!insertmode)
				return(last1key = SPEC | c);
		}

		/* next, a 2 char sequence */
		d = tgetc(FALSE);
		if (d == '~') {
			if (!insertmode)
				return(last1key = SPEC | c);
		}

		/* decode a 3 char sequence */
		c = d + ' ';
		/* if a shifted function key, eat the tilde */
		if (d >= '0' && d <= '9')
			d = tgetc(FALSE);
		if (!insertmode)
			return(last1key = SPEC | c);
	}
#endif

#if  WANGPC
	if (c == 0x1F) {	/* Apply SPEC prefix	*/
		c = tgetc(FALSE);
		if (!insertmode)
			return(last1key = SPEC | c);
	}
#endif

	return (last1key = c);
}

/*	KBD_SEQ:	Get a command sequence (multiple keystrokes) from 
		the keyboard.
		Process all applicable prefix keys.
		Set lastcmd for commands which want stuttering.
*/
int
kbd_seq()
{
	int c;		/* fetched keystroke */

	/* get initial character */
	c = kbd_key();

	/* process CTLA prefix */
	if (c == cntl_a) {
		c = kbd_key();
		return (lastcmd = CTLA | c);
	}

	/* process CTLX prefix */
	if (c == cntl_x) {
		c = kbd_key();
		return (lastcmd = CTLX | c);
	}

	/* otherwise, just return it */
	return (lastcmd = c);
}


/* get a string consisting of inclchartype characters from the current
	position.  if inclchartype is 0, return everything to eol */
int
screen_string(buf,bufn,inclchartype)
char *buf;
int bufn, inclchartype;
{
	register int i = 0;
	MARK mk;

	mk = DOT;
	while ( i < bufn && !is_at_end_of_line(DOT)) {
		buf[i] = char_at(DOT);
#if !SMALLER
		if (i == 0) {
			if (inclchartype & _scrtch) {
				if (buf[0] != SCRTCH_LEFT[0])
					inclchartype &= ~_scrtch;
			}
			if (inclchartype & _shpipe) {
				if (buf[0] != SHPIPE_LEFT[0])
					inclchartype &= ~_shpipe;
			}
		}
#endif
		if (inclchartype && !istype(inclchartype, buf[i]))
			break;
		DOT.o++;
		i++;
#if !SMALLER
		if (inclchartype & _scrtch)
			if (buf[i-1] == SCRTCH_RIGHT[0])
				break;
#endif
	}

#if !SMALLER
	if (inclchartype & _scrtch)
		if (buf[i-1] != SCRTCH_RIGHT[0])
			i = 0;
#endif

	buf[bufn-1] = EOS;
	if (i < bufn)
		buf[i] = EOS;
	DOT = mk;

	return buf[0] != EOS;
}

/*
 * Returns the character that ended the last call on 'kbd_string()'
 */
int
end_string()
{
	return last_eolchar;
}

/* turn \X into X */
static void
remove_backslashes(s)
char *s;
{
	register char *cp;
	while (*s) {
		if (*s == '\\') {  /* shift left */
			for (cp = s; *cp; cp++)
				*cp = *(cp+1);
		}
		s++;
	}
}

/* count backslashes so we can tell at any point whether we have the current
 * position escaped by one.
 */
static int
countBackSlashes(buffer, len)
char *	buffer;
int	len;
{
	register int	count;

	if (len && buffer[len-1] == '\\') {
		count = 1;
		while (count+1 <= len &&
			buffer[len-1-count] == '\\')
			count++;
	} else {
		count = 0;
	}
	return count;
}

static void
showChar(c)
int	c;
{
	if (disinp) {
		int	save_expand = kbd_expand;
		kbd_expand = 1;	/* show all controls */
		kbd_putc(c);
		kbd_expand = save_expand;
	}
}

/* expand a single character (only used on interactive input) */
static int
expandChar(buf, bufn, position, c)
char *	buf;
int	bufn;
int *	position;
int	c;
{
	register int	cpos = *position;
	register char *	cp;
	register BUFFER *bp;
	char str[NFILEN];

	switch(c) {
	case '%':
		if (!*(curbp->b_fname) ||
			isspace(*(curbp->b_fname)))
			cp = curbp->b_bname;
		else {
			cp = shorten_path(strcpy(str, curbp->b_fname));
			if (!cp) cp = curbp->b_bname;
		}
		break;
	case '#':
		if ((bp = find_alt()) != 0) {
			cp = shorten_path(strcpy(str, bp->b_fname));
			if (!cp || !*cp || isspace(*cp)) {
				/* oh well, use the buffer */
				cp = bp->b_bname;
			}
		} else
			cp = NULL;
		break;
#if ! MSDOS
	/* drive letters get in the way */
	case ':':
		if (screen_string(str, sizeof(str), _pathn))
			cp = str;
		else
			cp = NULL;
		break;
#endif
	default:
		return FALSE;
	}

	if (cp != 0) {
		while (cpos < bufn-1 && (c = *cp++)) {
			buf[cpos++] = c;
			showChar(c);
		}
		buf[cpos] = EOS;
		TTflush();
	}
	*position = cpos;
	return TRUE;
}

/*
 * Returns true for the (presumably control-chars) that we use for line edits
 */
int
is_edit_char(c)
int c;
{
	return (isreturn(c)
	  ||	isbackspace(c)
	  ||	(c == wkillc)
	  ||	(c == killc));
}

/*
 * Erases the response from the screen for 'kbd_string()'
 */
void
kbd_kill_response(buf, position, c)
char *	buf;
int *	position;
int	c;
{
	int	saw_word = FALSE;
	register int	cpos = *position;

	while (cpos > 0) {
		cpos--;
		if (disinp) {
			kbd_erase();
			if (!isprint(buf[cpos]))
				kbd_erase();
		}
		if (c == wkillc) {
			if (isspace(buf[cpos])) {
				if (saw_word)
					break;
			} else {
				saw_word = TRUE;
			}
		}

		if (isbackspace(c))
			break;
	}
	if (disinp)
		TTflush();

	buf[*position = cpos] = EOS;
}

/*
 * Display the default response for 'kbd_string()', escaping backslashes if
 * necessary.
 *
 * patch: make 'dst[]' a TBUFF so we don't have to worry about overflow.
 */
int
kbd_show_response(dst, src, bufn, eolchar, options)
char	*dst;		/* string with escapes */
char	*src;		/* string w/o escapes */
int	bufn;		/* maximum # of chars we read from 'src[]' */
int	eolchar;
int	options;
{
	register int c, j, k;

	j = k = 0;
	/* add backslash escapes in front of volatile characters */
	while ((c = src[k++]) != EOS && k < bufn) {
		if (c == eolchar && eolchar != '\n')
			goto is_eolchar;
		switch(c) {
		case '%':
		case '#':
#if ! MSDOS
		case ':':
#endif
			if ((options & KBD_QUOTES)
			 && (options & KBD_EXPAND))
				dst[j++] = '\\'; /* add extra */
		default:
			break;
		case '\\':
		is_eolchar:
			if (options & KBD_QUOTES)
				dst[j++] = '\\'; /* add extra */
			break;
		}
		dst[j++] = c;
	}
	dst[j] = EOS;

	/* put out the default response, which is in the buffer */
	j = 0;
	kbd_init();
	while ((c = dst[j]) != EOS && j < NLINE-1) {
		showChar(c);
		++j;
	}
	if (disinp)
		TTflush();
	return j;
}

/* default function for 'edithistory()' */
static int
/*ARGSUSED*/
eol_history(buffer, cpos, c, eolchar)
char *	buffer;
int	cpos;
int	c;
int	eolchar;
{
	if (isprint(eolchar)) {
		if (c == eolchar)
			return TRUE;
	}
	return FALSE;
}

/*	A more generalized prompt/reply function allowing the caller
	to specify a terminator other than '\n'.  Both are accepted.
	Assumes the buffer already contains a valid (possibly
	null) string to use as the default response.
*/
int
kbd_string(prompt, extbuf, bufn, eolchar, options, complete)
char *prompt;		/* put this out first */
char *extbuf;		/* the caller's (possibly full) buffer */
int bufn;		/* the length of  " */
int eolchar;		/* char we can terminate on, in addition to '\n' */
int options;		/* KBD_EXPAND/KBD_QUOTES, etc. */
int (*complete)P((int,char *,int *));/* handles completion */
{
	return kbd_reply(prompt, extbuf, bufn, eol_history, eolchar, options, complete);
}

/*
 * Same as 'kbd_string()', except for adding the 'endfunc' parameter.
 *
 * Returns:
 *	ABORT - abort character given (usually ESC)
 *	SORTOFTRUE - backspace from empty-buffer
 *	TRUE - buffer is not empty
 *	FALSE - buffer is empty
 */
int
kbd_reply(prompt, extbuf, bufn, endfunc, eolchar, options, complete)
char *prompt;		/* put this out first */
char *extbuf;		/* the caller's (possibly full) buffer */
int bufn;		/* the length of  " */
int (*endfunc)P((char *,int,int,int));	/* parsing with 'eolchar' delimiter */
int eolchar;		/* char we can terminate on, in addition to '\n' */
int options;		/* KBD_EXPAND/KBD_QUOTES */
int (*complete)P((int,char *,int *));	/* handles completion */
{
	int	c;
	int	done;
	int	cpos;		/* current character position in string */

	register int quotef;	/* are we quoting the next char? */
	register int backslashes; /* are we quoting the next expandable char? */
	int firstch = TRUE;
	int newpos;
	char buf[NLINE];

	if (clexec) {
		int	s;
		execstr = token(execstr, extbuf, eolchar);
		if ((s = (*extbuf != EOS)) != FALSE) {
#if !SMALLER
			if ((options & KBD_LOWERC))
				(void)mklower(extbuf);
#endif
			if (complete != no_completion) {
				cpos =
				newpos = strlen(extbuf);
				if (!(*complete)(NAMEC, extbuf, &newpos))
					extbuf[cpos] = EOS;
			}
			if (!(options & KBD_NOEVAL)) {
				(void)strcpy(extbuf, tokval(extbuf));
			}
			last_eolchar = *execstr;
		}
		return s;
	}

	quotef = FALSE;

	/* prompt the user for the input string */
	if (prompt != 0)
		mlprompt(prompt);

	if (bufn > sizeof(buf)) bufn = sizeof(buf);
	cpos = kbd_show_response(buf, extbuf, bufn, eolchar, options);
	backslashes = 0; /* by definition, there is an even 
					number of backslashes */
	for (;;) {
		int	EscOrQuo = ((quotef == TRUE) || ((backslashes & 1) != 0));

		/* get a character from the user */
		c = kbd_key();

		/* If it is a <ret>, change it to a <NL> */
		if (c == '\r' && quotef == FALSE)
			c = '\n';

		/* if they hit the line terminate, wrap it up */
		/* don't allow newlines in the string -- they cause real
			problems, especially when searching for patterns
			containing them -pgf */
		/* terminate with newline, or unescaped eolchar */
		done = FALSE;
		if (c == '\n') {
			done = TRUE;
		} else if (!EscOrQuo && !is_edit_char(c)) {
			if ((*endfunc)(buf,cpos,c,eolchar)) {
				/*
				 * If this is not an exact match for 'eolchar',
				 * then the end-function was probably something
				 * like 'eol_command()', e.g., an "s" that is
				 * followed by a pattern delimiter.
				 */
				if (c != eolchar
				 && c != TESTC
				 && c != NAMEC
				 && ispunct(c))
					tungetc(c);
				done = TRUE;
			}
		}
		if (done)
			last_eolchar = c;

		if (complete != no_completion) {
			if (c == EOS) {	/* conflicts with null-terminated strings */
				kbd_alarm();
				continue;
			}
			kbd_unquery();
			if ((done && !(options & KBD_MAYBEC))
			 || (!EscOrQuo && (c == TESTC || c == NAMEC))) {
				int	ok = ((*complete)(c, buf, &cpos));

				if (ok) {
					done = TRUE;
					if (c != NAMEC)	/* cancel the unget */
						(void)tgetc(FALSE);
				} else {
					if (done) {	/* stay til matched! */
						buf[cpos] = EOS;
						kbd_unquery();
						(void)((*complete)(TESTC, buf, &cpos));
					}
					continue;
				}
			}
		}

		if (done) {
			if (options & KBD_QUOTES)
				remove_backslashes(buf); /* take out quoters */

			/* if buffer is empty, return FALSE */
			hst_append(buf, eolchar);
			return (*strcpy(extbuf, buf) == EOS) ? FALSE:TRUE;
		}

#if	NeWS	/* make sure cursor is where we think it is before output */
		TTmove(ttrow,ttcol) ;
#endif

#if	!SMALLER
		if (!EscOrQuo
		 && edithistory(buf, &cpos, &c, options, endfunc, eolchar)) {
			backslashes = countBackSlashes(buf, cpos);
			firstch = TRUE;
			continue;
		} else
#endif
		if (c == abortc && quotef == FALSE) {
			buf[cpos] = EOS;
			(void)esc(FALSE, 0);
			return ABORT;
		} else if ((isbackspace(c) ||
			c == wkillc ||
			c == killc) && quotef==FALSE) {

			if (cpos == 0) {
				buf[0] = EOS;
				if (prompt)
					mlerase();
				if (isbackspace(c)) {	/* splice calls */
					tungetc(c);
					return SORTOFTRUE;
				}
				return FALSE;
			}

		killit:
			kbd_kill_response(buf, &cpos, c);
			backslashes = countBackSlashes(buf, cpos);

		} else if ((options & KBD_EXPAND) && ((backslashes & 1 ) == 0)) {
			if (firstch == TRUE) {
				tungetc(c);
				c = killc;
				goto killit;
			}
			if (!expandChar(buf, bufn, &cpos, c))
				goto trymore;
		} else {
		trymore:
			if (c == quotec && quotef == FALSE) {
				quotef = TRUE;
				continue;	/* keep firstch==TRUE */
			} else	{
				if (firstch == TRUE) {
					/* we always clean the buf on the
						first char typed */
					tungetc(c);
					c = killc;
					goto killit;
				}
				if (c == '\\')
					backslashes++;
				else
					backslashes = 0;
				quotef = FALSE;

				if (isspecial(c)) {
					kbd_alarm();
					continue; /* keep firstch==TRUE */
				} else {
					if ((options & KBD_LOWERC)
					 && isupper(c))
						c = tolower(c);
					buf[cpos++] = c;
					buf[cpos] = EOS;
					if (disinp) {
						showChar(c);
						TTflush();
					}
				}
			}
		}
		firstch = FALSE;
	}
}

/* ARGSUSED */
int
speckey(f,n)
int f,n;
{

	tungetc( SPEC | kbd_key() );

	return TRUE;
}

/*
 * Begin recording the dot command macro.
 * Set up variables and return.
 * we use a temporary accumulator, in case this gets stopped prematurely
 */
int
dotcmdbegin()
{
	/* never record a playback */
	if (dotcmdmode != PLAY) {
		(void)TempDot(TRUE);
		dotcmdmode = RECORD;
		return TRUE;
	}
	return FALSE;
}

/*
 * Finish dot command, and copy it to the real holding area
 */
int
dotcmdfinish()
{
	if (dotcmdmode == RECORD) {
		if (tb_copy(&dotcmd, TempDot(FALSE)) != 0) {
			tb_first(dotcmd);
			dotcmdmode = STOP;
			return TRUE;
		}
	}
	return FALSE;
}


/* stop recording a dot command, 
	probably because the command is not re-doable */ 
void
dotcmdstop()
{
	if (dotcmdmode == RECORD)
		dotcmdmode = STOP;
}

/*
 * Execute a the '.' command, by putting us in PLAY mode.
 * The command argument is the number of times to loop. Quit as soon as a
 * command gets an error. Return TRUE if all ok, else FALSE.
 */
int
dotcmdplay(f, n)
int f,n;
{
	if (!f)
		n = 1;
	else if (n <= 0)
		return TRUE;

	if (dotcmdmode != STOP || tb_length(dotcmd) == 0) {
		dotcmdmode = STOP;
		return FALSE;
	}

	dotcmdcnt =
	dotcmdrep = n;		/* remember how many times to execute */
	dotcmdmode = PLAY;		/* put us in play mode */
	tb_first(dotcmd);	/*    at the beginning */

	return TRUE;
}

/*
 * Special function used in 'insert.c' to adjust cursor position between
 * iterations in command of the form "5."
 */
int
dotreplaying(f)
int f;
{
	if (dotcmdmode == PLAY) {
		if (f)
			return (dotcmdrep != dotcmdcnt);
		return (dotcmdrep > 1);
	}
	return FALSE;
}

/*
 *
 */
int
kbd_replaying()
{
	return (dotcmdmode == PLAY) || (kbdmode == PLAY);
}

/*
 * Begin recording a keyboard macro.
 */
/* ARGSUSED */
int
kbd_mac_begin(f, n)
int f,n;
{
	if (kbdmode != STOP) {
		mlforce("[Macro already active]");
		return FALSE;
	}
	mlwrite("[Start macro]");

	kbdmode = RECORD;
	kbdplayreg = DEFAULT_REG;
	return (tb_init(&KbdMacro, abortc) != 0);
}

/*
 * End keyboard macro. Check for the same limit conditions as the above
 * routine. Set up the variables and return to the caller.
 */
/* ARGSUSED */
int
kbd_mac_end(f, n)
int f,n;
{
	if (kbdmode == STOP) {
		mlforce("[Macro not active]");
		return FALSE;
	}
	if (kbdmode == RECORD) {
		mlwrite("[End macro]");
		kbdmode = STOP;
		kbdplayreg = DEFAULT_REG;
	}
	/* note that if kbd_mode == PLAY, we do nothing -- that makes
		the '^X-)' at the of the recorded buffer a no-op during
		playback */
	return TRUE;
}

/*
 * Execute a macro.
 * The command argument is the number of times to loop. Quit as soon as a
 * command gets an error. Return TRUE if all ok, else FALSE.
 */
/* ARGSUSED */
int
kbd_mac_exec(f, n)
int f,n;
{
	if (n <= 0)
		return TRUE;

	return start_kbm(n, DEFAULT_REG, KbdMacro);
}

/* ARGSUSED */
int
kbd_mac_save(f,n)
int f,n;
{
	ksetup();
	tb_first(KbdMacro);
	while (tb_more(KbdMacro))
		kinsert(tb_next(KbdMacro));
	kdone();
	mlwrite("[Keyboard macro saved in register %c.]", index2reg(ukb));
	return TRUE;
}

/*
 * Start playback of the given keyboard command-string
 */
int
start_kbm(n, macnum, ptr)
int	n;			/* # of times to repeat */
int	macnum;			/* register to execute */
TBUFF *	ptr;			/* data to interpret */
{
	register KSTACK *sp;
	TBUFF  *tp = 0;

	if (interrupted)
		return FALSE;

	if (kbdmode == RECORD && KbdStack != 0)
		return TRUE;

	if (tb_length(ptr)
	 && (sp = typealloc(KSTACK)) != 0
	 && tb_copy(&tp, ptr) != 0) {

		/* make a copy of the macro in case recursion alters it */
		tb_first(tp);

		sp->m_save = kbdmode;
		sp->m_indx = /* patch */kbdplayreg = macnum;
		sp->m_rept = n;
		sp->m_kbdm = tp;
		sp->m_link = KbdStack;

		KbdStack   = sp;
		kbdmode    = PLAY; 	/* start us in play mode */

		/* save data for "." on the same stack */
		sp->m_dots = 0;
		sp->m_DOTS = dotcmd;
		dotcmd     = 0;
		return (tb_init(&dotcmd, abortc) != 0
		  &&    tb_init(&(sp->m_dots), abortc) != 0);
	}
	return FALSE;
}

/*
 * Finish a macro begun via 'start_kbm()'
 */
void
finish_kbm()
{
	if (kbdmode == PLAY) {
		register KSTACK *sp = KbdStack;

		kbdmode = STOP;
		kbdplayreg = DEFAULT_REG;
		if (sp != 0) {
			kbdmode  = sp->m_save;
			KbdStack = sp->m_link;

			tb_free(&(sp->m_kbdm));
			tb_free(&dotcmd);
			dotcmd = sp->m_DOTS;
			tb_free(&(sp->m_dots));
			free((char *)sp);

			if (KbdStack != 0)
				kbdplayreg = KbdStack->m_indx;
		}
	}
}
