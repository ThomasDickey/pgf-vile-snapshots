/*	INPUT:	Various input routines for MicroEMACS
 *		written by Daniel Lawrence
 *		5/9/86
 *
 * $Log: input.c,v $
 * Revision 1.100  1994/03/24 12:04:37  pgf
 * arrow key fix from tom
 *
 * Revision 1.99  1994/02/22  11:03:15  pgf
 * truncated RCS log for 4.0
 *
 */

#include	"estruct.h"
#include	"edef.h"

#define	SQUARE_LEFT	'['

#define	DEFAULT_REG	-1

typedef	struct	_kstack	{
	struct	_kstack	*m_link;
	int	m_save;		/* old value of 'kbdmode'		*/
	int	m_indx;		/* index identifying this macro		*/
	int	m_rept;		/* the number of times to execute the macro */
	TBUFF  *m_kbdm;		/* the macro-text to execute		*/
	TBUFF  *m_dots;		/* workspace for "." command		*/
#ifdef GMDDOTMACRO
	TBUFF  *m_DOTS;		/* save-area for "." command		*/
	int	m_RPT0;		/* saves 'dotcmdcnt'			*/
	int	m_RPT1;		/* saves 'dotcmdrep'			*/
#endif
	} KSTACK;

/*--------------------------------------------------------------------------*/
static	TBUFF *	TempDot P(( int ));
static	void	record_dot_char P(( int ));
static	void	record_kbd_char P(( int ));
static	void	record_char P(( int ));
static	void	remove_backslashes P(( char * ));
static	int	countBackSlashes P(( char *, int ));
static	void	showChar P(( int ));
static	void	show1Char P(( int ));
static	void	eraseChar P(( int ));
static	int	expandChar P(( char *, int, int *, int, int ));
static	int	eol_history P(( char *, int, int, int ));
#ifdef GMDDOTMACRO
static	void	dot_replays_macro P(( int ));
#endif

static	KSTACK *KbdStack;	/* keyboard/@-macros that are replaying */
static	TBUFF  *KbdMacro;	/* keyboard macro, recorded	*/
static	int	last_eolchar;	/* records last eolchar-match in 'kbd_string' */

/*--------------------------------------------------------------------------*/

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
 * with an abortc. Used any time a confirmation is required.
 */

int
mlyesno(prompt)
char *prompt;
{
	char c; 		/* input character */

	for (;;) {
		mlprompt("%s [y/n]? ",prompt);
		c = tgetc(FALSE);	/* get the response */

		if (c == abortc)		/* Bail out! */
			return(ABORT);

		if (c=='y' || c=='Y')
			return(TRUE);

		if (c=='n' || c=='N')
			return(FALSE);
	}
}

/*
 * Ask a simple question in the message line. Return the single char response,
 *  if it was one of the valid responses.
 */

int
mlquickask(prompt,respchars,cp)
char *prompt;
char *respchars;
int *cp;
{

	for (;;) {
		mlprompt("%s ",prompt);
		*cp = tgetc(FALSE);	/* get the response */

		if (*cp == abortc)	/* Bail out! */
			return(ABORT);

		if (strchr(respchars,*cp))
			return TRUE;

		TTbeep();
	}
}

/*
 * Prompt for a named-buffer (i.e., "register")
 */
int
mlreply_reg(prompt, cbuf, retp, at_dft)
char	*prompt;
char	*cbuf;		/* 2-char buffer for register+eol */
int	*retp;		/* => the register-name */
int	at_dft;		/* default-value (e.g., for "@@" command) */
{
	register int status;
	register int c;

	if (clexec || isnamedcmd) {
		if ((status = mlreply(prompt, cbuf, 2)) != TRUE)
			return status;
		c = cbuf[0];
	} else {
		c = tgetc(FALSE);
	}

	if (c == '@' && at_dft != -1) {
		c = at_dft;
	} else if (reg2index(c) < 0) {
		TTbeep();
		mlforce("[Invalid register name]");
		return FALSE;
	}

	*retp = c;
	return TRUE;
}

/*
 * Prompt for a register-name and/or line-count (e.g., for the ":yank" and
 * ":put" commands).  The register-name, if given, is first.
 */
int
mlreply_reg_count(state, retp, next)
int	state;		/* negative=register, positive=count, zero=either */
int	*retp;		/* returns the register-index or line-count */
int	*next;		/* returns 0/1=register, 2=count */
{
	register int status;
	char	prompt[80];
	char	expect[80];
	char	buffer[10];
	int	length;

	*expect = EOS;
	if (state <= 0)
		(void)strcat(expect, " register");
	if (state == 0)
		(void)strcat(expect, " or");
	if (state >= 0) {
		(void)strcat(expect, " line-count");
		length = sizeof(buffer);
	} else
		length = 2;

	(void)lsprintf(prompt, "Specify%s: ", expect);
	*buffer = EOS;
	status = kbd_string(prompt, buffer, length, ' ', 0, no_completion);

	if (status == TRUE) {
		if (state <= 0
		 && isalpha(buffer[0])
		 && buffer[1] == EOS
		 && (*retp = reg2index(*buffer)) >= 0) {
			*next = isupper(*buffer) ? 1 : 0;
		} else if (state >= 0
		 && string_to_number(buffer, retp)
		 && *retp) {
			*next = 2;
		} else {
			mlforce("[Expected%s]", expect);
			kbd_alarm();
			status = ABORT;
		}
	}
	return status;
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
	return kbd_string(prompt, buf, bufn, '\n', KBD_EXPAND|KBD_SHPIPE, no_completion);
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


/* the numbered buffer names increment each time they are referenced */
void
incr_dot_kregnum()
{
	if (dotcmdmode == PLAY) {
		register int	c = tb_peek(dotcmd);
		if (isdigit(c) && c < '9')
			tb_stuff(dotcmd, ++c);
	}
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
	if (dotcmdmode != PLAY && kbdmode == RECORD)
		(void)tb_append(&KbdMacro, c);
}

/* if we should preserve this input, do so */
static void
record_char(c)
int c;
{
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

		if (interrupted()) {
			dotcmdmode = STOP;
			return abortc;
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

		if (interrupted()) {
			while (kbdmode == PLAY)
				finish_kbm();
			return abortc;
		} else {

			if (!tb_more(buffer = KbdStack->m_kbdm)) {
				if (--(KbdStack->m_rept) >= 1)
					tb_first(buffer);
				else
					finish_kbm();
			}

			if (kbdmode == PLAY) {
				buffer = KbdStack->m_kbdm;
				if (eatit)
					record_dot_char(c = tb_next(buffer));
				else
					c = tb_peek(buffer);
			}
		}
	}

	return c;
}

void
tungetc(c)
int c;
{

	tungotc = c;
	if (dotcmdmode == RECORD) {
		tb_unput(TempDot(FALSE));
		if (kbdmode == RECORD)
			tb_unput(KbdMacro);
	} else if (dotcmdmode != PLAY && kbdmode == RECORD)
		tb_unput(KbdMacro);
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
			not_interrupted();
			if (setjmp(read_jmp_buf)) {
				c = kcod2key(intrc);
			} else {
				doing_kbd_read = TRUE;
				do { /* if it's sysV style signals,
					 we want to try again, since this
					 must not have been SIGINT, but
					 was probably SIGWINCH */
					c = TTgetc();
				} while (c == -1);
			}
			doing_kbd_read = FALSE;
			if (!quoted && (c == kcod2key(intrc)))
				c = abortc;
			else
				record_char(c);
		}
		c = char2int(c);
	}

	/* and finally give the char back */
	return lastkey = c;
}

/*	KBD_KEY:	Get one keystroke.  system function keys are
	translated to the internal representation of '#' followed by a
	single character.  be careful here.  insert mode function key/arrow
	key processing depends on the behavior that the only way to get a
	poundc from kbd_key with a tungotc having been pushed back is for a
	real function key to have been pressed */
int
kbd_key()
{
	int    c;
	int pound = insertmode ? altpoundc : poundc;

#if OPT_XTERM && !X11
kbd_key_loop:
#endif
	/* get a keystroke */
	c = tgetc(FALSE);

#if ANSI_SPEC

	if (c == ESC) {
		int nextc;

		/* if there's no recorded input, and no user typeahead */
		if ((nextc = get_recorded_char(FALSE)) == -1 && !typahead()) {
			/* give it a little extra time... */
			catnap(global_g_val(GVAL_TIMEOUTVAL),TRUE);
		}

		/* and then, if there _was_ recorded input or new typahead... */
		if (nextc != -1 || typahead()) {
			c = tgetc(FALSE);
			if (c == SQUARE_LEFT || c == 'O') {
#if OPT_XTERM && !X11
				int	d = c;
#endif
				/* eat ansi sequences */
				c = tgetc(FALSE);
#if OPT_XTERM && !X11
				if (d == SQUARE_LEFT
				 && (d = xterm_button(c)) != FALSE) {
					if (insertmode || (d != TRUE))
						return abortc;
					goto kbd_key_loop;
				}
#endif
				if (abortc != ESC || !insertmode) {
				    	tungetc(c);
					return pound;
				}
				if (insertmode == REPLACECHAR) {
					/* eat the sequence, but return abort */
					return abortc;
				}
				tungetc(c);
				return pound;
			} else {
				if (abortc != ESC)
					return (c);
				tungetc(c);
				return (ESC);
			}
		}
	}
#endif
#if	MSDOS | ST520
	if (c == 0) {			/* Apply SPEC prefix	*/
		c = tgetc(FALSE);
		tungetc(c);
		return pound;
	}
#endif

#if	AMIGA
	/* apply SPEC prefix */
	if ((unsigned)c == 155) {
		int	d;
		c = tgetc(FALSE);

		/* first try to see if it is a cursor key */
		if ((c >= 'A' && c <= 'D') || c == 'S' || c == 'T') {
			tungetc(c);
			return pound;
		}

		/* next, a 2 char sequence */
		d = tgetc(FALSE);
		if (d == '~') {
			tungetc(c);
			return pound;
		}

		/* decode a 3 char sequence */
		c = d + ' ';
		/* if a shifted function key, eat the tilde */
		if (d >= '0' && d <= '9')
			d = tgetc(FALSE);
		tungetc(c);
		return pound;
	}
#endif

#if  WANGPC
	if (c == 0x1F) {	/* Apply SPEC prefix	*/
		c = tgetc(FALSE);
		tungetc(c);
		return pound;
	}
#endif

	return (c);
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

	int prefix = 0;	/* accumulate prefix */

	c = kbd_key();

	if (c == cntl_a) {
		prefix = CTLA;
		c = kbd_key();
	} else if (c == cntl_x) {
		prefix = CTLX;
		c = kbd_key();
	}

	if (c == poundc || c == altpoundc) {
		prefix |= SPEC;
		c = kbd_key();
	}

	c |= prefix;

	/* otherwise, just return it */
	return (lastcmd = c);
}


/* Read a character from the input, allowing an escape-sequence if we've got
 * one stacked up.  Use this function when we want to get the 16-bit keycode
 * for a single keystroke.
 */
int
kbd_escape_seq ()
{
	register int	c = kbd_key();
	if (c == poundc && did_tungetc()) {
		c = SPEC | kbd_key();
	}
	return c;
}

/* Translate a 16-bit keycode to a string that will replay into the same code,
 * i.e., when routed via 'kbd_escape_seq()'.  This differs from 'kcod2str()',
 * which produces results that could be routed via 'kbd_seq()'.
 */
int
kcod2escape_seq (c, ptr)
int	c;
char *	ptr;
{
	char	*base = ptr;

	/* ...just for completeness */
	if (c & CTLA) *ptr++ = cntl_a;
	if (c & CTLX) *ptr++ = cntl_x;

	/* ...this is why we're here */
	if (c & SPEC) {
		c = char2int(c);
#if ANSI_SPEC
		*ptr++ = ESC;
		*ptr++ = SQUARE_LEFT;

#endif
#if	MSDOS | ST520
		*ptr++ = 0;
#endif

#if	AMIGA
		/* FIXME: untested 22-mar-94 dickey@software.org */
		*ptr++ = 155;

		/* first try to see if it is a cursor key */
		if ((c < 'A' || c > 'D') && c != 'S' && c != 'T') {
			int	d;

			if (c != '~') {
				/* decode a 3 char sequence */
				d = c - ' ';
				/* if a shifted function key, eat the tilde */
				if (d >= '0' && d <= '9') {
					*ptr++ = c;
					c = '~';
				}
			}
		}
#endif

#if  WANGPC
		*ptr++ = 0x1F;
#endif
	}
	*ptr++ = c;
	return (int)(ptr - base);
}

/* get a string consisting of inclchartype characters from the current
	position.  if inclchartype is 0, return everything to eol */
#if ANSI_PROTOS
int screen_string (char *buf, int bufn, CMASK inclchartype )
#else
int
screen_string(buf,bufn,inclchartype)
char *buf;
int bufn;
CMASK inclchartype;
#endif
{
	register int i = 0;
	MARK mk;

	mk = DOT;
	while ( i < (bufn-1) && !is_at_end_of_line(DOT)) {
		buf[i] = char_at(DOT);
#if OPT_WIDE_CTYPES
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
		/* guard against things like "[Buffer List]" on VMS */
		if (inclchartype & _pathn) {
			if (!ispath(buf[i]) && (inclchartype == _pathn))
				break;
		}
#endif
		if (inclchartype && !istype(inclchartype, buf[i]))
			break;
		DOT.o++;
		i++;
#if OPT_WIDE_CTYPES
		if (inclchartype & _scrtch) {
			if ((i < bufn)
			 && (inclchartype & _pathn)
			 && ispath(char_at(DOT)))
				continue;
			if (buf[i-1] == SCRTCH_RIGHT[0])
				break;
		}
#endif
	}

#if OPT_WIDE_CTYPES
#if VMS
	if (inclchartype & _pathn) {
		;	/* override conflict with "[]" */
	} else
#endif
	if (inclchartype & _scrtch) {
		if (buf[i-1] != SCRTCH_RIGHT[0])
			i = 0;
	}
#endif

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

/*
 * Returns an appropriate delimiter for /-commands, based on the end of the
 * last reply.  That is, in a command such as
 *
 *	:s/first/last/
 *
 * we will get prompts for
 *
 *	:s/	/-delimiter saved in 'end_string()'
 *	first/
 *	last/
 *
 * If a newline is used at any stage, subsequent delimiters are forced to a
 * newline.
 */
int
kbd_delimiter()
{
	register int	c = '\n';

	if (isnamedcmd) {
		register int	d = end_string();
		if (ispunct(d))
			c = d;
	}
	return c;
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

static void
show1Char(c)
int	c;
{
	if (disinp) {
		showChar(c);
		TTflush();
	}
}

static void
eraseChar(c)
int	c;
{
	if (disinp) {
		kbd_erase();
		if (!isprint(c)) {
			kbd_erase();		/* e.g. back up over ^H */
		    	if (c & HIGHBIT) {
			    kbd_erase();	/* e.g. back up over \200 */
			    kbd_erase();
			}
		}
	}
}

/* expand a single character (only used on interactive input) */
static int
expandChar(buf, bufn, position, c, options)
char *	buf;
int	bufn;
int *	position;
int	c;
int	options;
{
	register int	cpos = *position;
	register char *	cp;
	register BUFFER *bp;
	char str[NFILEN];
	int  shell = ((options & KBD_EXPCMD) && isShellOrPipe(buf))
		   || (options & KBD_SHPIPE);

	/* Are we allowed to expand anything? */
	if (!((options & KBD_EXPAND) || shell))
	 	return FALSE;

	/* Is this a character that we know about? */
	if (strchr(global_g_val_ptr(GVAL_EXPAND_CHARS),c) == 0)
		return FALSE;

	if (c == '%' || c == '#') {
		bp = (c == '%') ? curbp : find_alt();
		if (bp == 0 || b_is_invisible(bp)) {
			kbd_alarm();	/* complain a little */
			return FALSE;	/* ...and let the user type it as-is */
		}
		cp = bp->b_fname;
		if (isInternalName(cp))
			cp = get_bname(bp);
		else if (!global_g_val(GMDEXPAND_PATH))
			cp = shorten_path(strcpy(str, cp), FALSE);
	} else if (c == ':') {
		if (screen_string(str, sizeof(str), _pathn))
			cp = str;
		else
			cp = NULL;
	} else if ((c == '!') && shell) {
		cp = tb_values(save_shell[!isShellOrPipe(buf)]);
		if (cp != NULL && isShellOrPipe(cp))
			cp++;	/* skip the '!' */
	} else {
		return FALSE;
	}

	if (cp != NULL) {
		while (cpos < bufn-1 && ((c = *cp++) != EOS)) {
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
	register int	cpos = *position;

	while (cpos > 0) {
		cpos--;
		eraseChar(buf[cpos]);
		if (c == wkillc) {
			if (!isspace(buf[cpos])) {
				if (cpos > 0 && isspace(buf[cpos-1]))
					break;
			}
		}

		if (c != killc && c != wkillc)
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

	/* add backslash escapes in front of volatile characters */
	for (j = k = 0; k < bufn; k++) {
		if ((c = src[k]) == EOS)
			break;
		if ((c == '\\') || (c == eolchar && eolchar != '\n')) {
			if (options & KBD_QUOTES)
				dst[j++] = '\\'; /* add extra */
		} else if (strchr(global_g_val_ptr(GVAL_EXPAND_CHARS),c) != 0) {
			if ((options & KBD_QUOTES)
			 && (options & KBD_EXPAND))
				dst[j++] = '\\'; /* add extra */
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

/*
 * Store a one-level push-back of the shell command text. This allows simple
 * prompt/substitution of shell commands, while keeping the "!" and text
 * separate for the command decoder.
 */
static	int	pushed_back;
static	int	pushback_flg;
static	char *	pushback_ptr;

void kbd_pushback(buffer, skip)
char	*buffer;
int	skip;
{
	static	TBUFF	*PushBack;
	if (macroize(&PushBack, buffer+1, buffer)) {
		pushed_back  = TRUE;
		pushback_flg = clexec;
		pushback_ptr = execstr;
		clexec       = TRUE;
		execstr      = tb_values(PushBack);
		buffer[skip] = EOS;
	}
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
	int	status;

	register int quotef;	/* are we quoting the next char? */
	register int backslashes; /* are we quoting the next expandable char? */
	int firstch = TRUE;
	int newpos;
	char buf[NLINE];

	last_eolchar = EOS;	/* ...in case we don't set it elsewhere */

	if (clexec) {
		execstr = token(execstr, extbuf, eolchar);
		if ((status = (*extbuf != EOS)) != FALSE) {
#if !SMALLER
			if ((options & KBD_LOWERC))
				(void)mklower(extbuf);
			else if ((options & KBD_UPPERC))
				(void)mkupper(extbuf);
#endif
			if (!(options & KBD_NOEVAL)) {
				(void)strncpy(extbuf,
					tokval(strcpy(buf, extbuf)),
					(SIZE_T)bufn);
			}
			if (complete != no_completion) {
				cpos =
				newpos = strlen(extbuf);
				if (!(*complete)(NAMEC, extbuf, &newpos))
					extbuf[cpos] = EOS;
			}
			last_eolchar = *execstr;
			extbuf[bufn-1] = EOS;
		}
		if (pushed_back && (*execstr == EOS)) {
			pushed_back = FALSE;
			clexec  = pushback_flg;
			execstr = pushback_ptr;
#if	OPT_HISTORY
			if (!pushback_flg)
				hst_append(extbuf,EOS);
#endif
		}
		return status;
	}

	quotef = FALSE;
	reading_msg_line = TRUE;

	/* prompt the user for the input string */
	if (prompt != 0)
		mlprompt("%s", prompt);

	if (bufn > sizeof(buf)) bufn = sizeof(buf);
	cpos = kbd_show_response(buf, extbuf, bufn, eolchar, options);
	backslashes = 0; /* by definition, there is an even 
					number of backslashes */
	for (;;) {
		int	EscOrQuo = ((quotef == TRUE) || ((backslashes & 1) != 0));

		/*
		 * Get a character from the user. If not quoted, treat escape
		 * sequences as a single (16-bit) special character.
		 */
		c = quotef ? tgetc(TRUE) : kbd_escape_seq();

		/* if we echoed ^V, erase it now */
		if (quotef) {
			firstch = FALSE;
			eraseChar(quotec);
			TTflush();
		}

		/* If it is a <ret>, change it to a <NL> */
		if (c == '\r' && quotef == FALSE)
			c = '\n';

		/*
		 * If they hit the line terminate (i.e., newline or unescaped
		 * eolchar), wrap it up.
		 *
		 * Don't allow newlines in the string -- they cause real
		 * problems, especially when searching for patterns
		 * containing them -pgf
		 */
		done = FALSE;
		if (c == '\n') {
			done = TRUE;
		} else if (!EscOrQuo && !is_edit_char(c)) {
			if ((*endfunc)(buf,cpos,c,eolchar)) {
				done = TRUE;
			}
		}

		if (complete != no_completion) {
			if (c == EOS) {	/* conflicts with null-terminated strings */
				kbd_alarm();
				continue;
			}
			kbd_unquery();
			if (done && (options & KBD_NULLOK) && cpos == 0)
				;
			else if ((done && !(options & KBD_MAYBEC))
			 || (!EscOrQuo
			  && !((options & KBD_EXPCMD) && isShellOrPipe(buf))
			  && (c == TESTC || c == NAMEC))) {
				int	ok = ((*complete)(c, buf, &cpos));

				if (ok) {
					done = TRUE;
					if (c != NAMEC) /* cancel the unget */
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
			last_eolchar = c;
			if (options & KBD_QUOTES)
				remove_backslashes(buf); /* take out quoters */

			/* if buffer is empty, return FALSE */
			hst_append(buf, eolchar);
			status = (*strncpy(extbuf, buf, (SIZE_T)bufn) != EOS);

			/* If this is a shell command, push-back the actual
			 * text to separate the "!" from the command.  Note
			 * that the history command tries to do this already,
			 * but cannot for some special cases, e.g., the user
			 * types
			 *	:execute-named-command !ls -l
			 * which is equivalent to
			 *	:!ls -l
			 */
			if (status == TRUE	/* ...we have some text */
			 && (options & KBD_EXPCMD)
#if	OPT_HISTORY
			 && (strlen(buf) == cpos) /* history didn't split it */
#endif
			 && isShellOrPipe(buf)) {
				kbd_pushback(extbuf, 1);
			}
			break;
		}


#if	OPT_HISTORY
		if (!EscOrQuo
		 && edithistory(buf, &cpos, &c, options, endfunc, eolchar)) {
			backslashes = countBackSlashes(buf, cpos);
			firstch = TRUE;
			continue;
		} else
#endif
		if (c == abortc && quotef == FALSE) {
			buf[cpos] = EOS;
			status = esc(FALSE, 1);
			break;
		} else if ((isbackspace(c) ||
			c == wkillc ||
			c == killc) && quotef==FALSE) {

			if (prompt == 0 && c == killc)
				cpos = 0;

			if (cpos == 0) {
				buf[0] = EOS;
				if (prompt)
					mlerase();
				if (isbackspace(c)) {	/* splice calls */
					tungetc(c);
					status = SORTOFTRUE;
					break;
				}
				status = FALSE;
				break;
			}

		killit:
			kbd_kill_response(buf, &cpos, c);
			backslashes = countBackSlashes(buf, cpos);

		} else if (c == quotec && quotef == FALSE) {
			quotef = TRUE;
			show1Char(c);
			continue;	/* keep firstch==TRUE */

		} else {
			if (firstch == TRUE) {
				/* clean the buffer on the first char typed */
				tungetc(c);
				c = killc;
				goto killit;
			}

			if (EscOrQuo
			 || !expandChar(buf, bufn, &cpos, c, options)) {
				if (c == '\\')
					backslashes++;
				else
					backslashes = 0;
				quotef = FALSE;

				if (isspecial(c)
				 || (cpos >= bufn-1)) {
					if (!typahead())
						kbd_alarm();
					continue; /* keep firstch==TRUE */
				} else {
#if !SMALLER
					if ((options & KBD_LOWERC)
					 && isupper(c))
						c = tolower(c);
					else if ((options & KBD_UPPERC)
					 && islower(c))
						c = toupper(c);
#endif
					buf[cpos++] = c;
					buf[cpos] = EOS;
					show1Char(c);
				}
			}
		}
		firstch = FALSE;
	}
	reading_msg_line = FALSE;
	return status;
}

/*
 * Make the "." replay the keyboard macro
 */
#ifdef GMDDOTMACRO
static void
dot_replays_macro(macnum)
int	macnum;
{
	extern	CMDFUNC	f_kbd_mac_exec;
	char	temp[NSTRING];
	TBUFF	*tmp;
	int	c;

	if (macnum == DEFAULT_REG) {
		if ((c = fnc2kcod(&f_kbd_mac_exec)) == -1)
			return;
		(void)kcod2str(c, temp);
	} else {
		(void)lsprintf(temp, "@%c", index2reg(macnum));
	}
	dotcmdbegin();
	tmp = TempDot(FALSE);
	(void)tb_sappend(&tmp, temp);
	dotcmdfinish();
	dotcmdbegin();
}
#endif

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
		TBUFF	*tmp = TempDot(FALSE);
		if (tb_length(tmp) == 0	/* keep the old value */
		 || tb_copy(&dotcmd, tmp) != 0) {
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
 * Execute the '.' command, by putting us in PLAY mode.
 * The command argument is the number of times to loop. Quit as soon as a
 * command gets an error. Return TRUE if all ok, else FALSE.
 */
int
dotcmdplay(f, n)
int f,n;
{
	if (!f)
		n = dotcmdarg ? dotcmdcnt:1;
	else if (n < 0)
		return TRUE;

	if (f)	/* we definitely have an argument */
		dotcmdarg = TRUE;
	/* else
		leave dotcmdarg alone; */

	if (dotcmdmode != STOP || tb_length(dotcmd) == 0) {
		dotcmdmode = STOP;
		dotcmdarg = FALSE;
		return FALSE;
	}

	if (n == 0) n = 1;

	dotcmdcnt = dotcmdrep = n;  /* remember how many times to execute */
	dotcmdmode = PLAY;	/* put us in play mode */
	tb_first(dotcmd);	/*    at the beginning */

	if (ukb != 0) /* save our kreg, if one was specified */
		dotcmdkreg = ukb;
	else /* use our old one, if it wasn't */
		ukb = dotcmdkreg;

	return TRUE;
}

/*
 * Test if we are replaying either '.' command, or keyboard macro.
 */
int
kbd_replaying(match)
int	match;
{
	if (dotcmdmode == PLAY) {
		/*
		 * Force a false-return if we are in insert-mode and have
		 * only one character to display.
		 */
		if (match
		 && insertmode == INSERT
		 && b_val(curbp, MDSHOWMAT)
		 && KbdStack == 0
		 && (dotcmd->tb_last+1 >= dotcmd->tb_used)) {
			return FALSE;
		}
		return TRUE;
	}
	return (kbdmode == PLAY);
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
#ifdef GMDDOTMACRO
		dot_replays_macro(DEFAULT_REG);
#endif
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
		if (!kinsert(tb_next(KbdMacro)))
			break;
	kdone();
	mlwrite("[Keyboard macro saved in register %c.]", index2reg(ukb));
	return TRUE;
}

/*
 * Test if the given macro has already been started.
 */
int
kbm_started(macnum, force)
int	macnum;
int	force;
{
	if (force || (kbdmode == PLAY)) {
		register KSTACK *sp;
		for (sp = KbdStack; sp != 0; sp = sp->m_link) {
			if (sp->m_indx == macnum) {
				TTbeep();
				while (kbdmode == PLAY)
					finish_kbm();
				mlforce("[Error: currently executing %s%c]",
					macnum == DEFAULT_REG
						? "macro" : "register ",
					index2reg(macnum));
				return TRUE;
			}
		}
	}
	return FALSE;
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

	if (interrupted())
		return FALSE;

	if (kbdmode == RECORD && KbdStack != 0)
		return TRUE;

	if (tb_length(ptr)
	 && (sp = typealloc(KSTACK)) != 0
	 && tb_copy(&tp, ptr) != 0) {

		/* make a copy of the macro in case recursion alters it */
		tb_first(tp);

		sp->m_save = kbdmode;
		sp->m_indx = macnum;
		sp->m_rept = n;
		sp->m_kbdm = tp;
		sp->m_link = KbdStack;

		KbdStack   = sp;
		kbdmode    = PLAY; 	/* start us in play mode */

		/* save data for "." on the same stack */
		sp->m_dots = 0;
		if (dotcmdmode == PLAY) {
#ifdef GMDDOTMACRO
			sp->m_DOTS = dotcmd;
			sp->m_RPT0 = dotcmdcnt;
			sp->m_RPT1 = dotcmdrep;
#endif
			dotcmd     = 0;
			dotcmdmode = RECORD;
		}
#ifdef GMDDOTMACRO
		  else {
			sp->m_DOTS = 0;
		  }
#endif
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
		if (sp != 0) {
			kbdmode  = sp->m_save;
			KbdStack = sp->m_link;

			tb_free(&(sp->m_kbdm));
			tb_free(&(sp->m_dots));
#ifdef GMDDOTMACRO
			tb_free(&dotcmd);
			if (sp->m_DOTS != 0) {
				dotcmd     = sp->m_DOTS;
				dotcmdcnt  = sp->m_RPT0;
				dotcmdrep  = sp->m_RPT1;
				dotcmdmode = PLAY;
			}
			dot_replays_macro(sp->m_indx);
#endif
			free((char *)sp);
		}
	}
}
