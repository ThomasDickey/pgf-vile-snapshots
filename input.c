/*	INPUT:	Various input routines for vile
 *		written by Daniel Lawrence	5/9/86
 *		variously munged/massaged/relayered/slashed/burned
 *			since then. -pgf
 *
 *	TTgetc()	raw 8-bit key from terminal driver.
 *
 *	sysmapped_c()	single "keystroke" -- may have SPEC bit, if it was
 *			a sytem-mapped function key.  calls TTgetc().  these
 *			system-mapped keys will never map to a multi-char
 *			sequence.  the routine does have storage, to hold
 *			keystrokes gathered "in error".
 *
 *	tgetc()		fresh, pushed back, or recorded output of result of 
 *			sysmapped_c() (i.e. dotcmd and the keyboard macros
 *			are recordedand played back at this level).  this is
 *			only called from mapgetc() in map.c
 *
 *	mapped_c()	(map.c) worker routine which will return a mapped
 *			or non-mapped character from the mapping engine.
 *			determines correct map, and uses its own pushback 
 *			buffers on top of calls to tgetc() (see mapgetc/
 *			mapungetc).
 *
 *	mapped_keystroke() applies user-specified maps to user's input.
 *			correct map used depending on mode (insert, command,
 *			message-line).
 *
 *	keystroke()	returns pushback from mappings, the results of 
 *			previous calls to mapped_keystroke().
 *
 *	keystroke8()	as above, but masks off any "wideness", i.e. SPEC bits.
 *
 *	keystroke_raw() as above, but recording is forced even if
 *			sysmapped_c() returns intrc. (old "tgetc(TRUE)")
 *
 *	kbd_seq()	the vile prefix keys (^X,^A,#) are checked for, and
 *			appropriate kbd_key() pairs are turned into CTLA|c,
 *			CTLX|c, SPEC|c.
 *	
 *
 *	TTtypahead() 	  true if a key is avail from TTgetc().
 *	sysmapped_c_avail() "  if a key is avail from sysmapped_c() or below.
 *	tgetc_avail()     true if a key is avail from tgetc() or below.
 *	keystroke_avail() true if a key is avail from keystroke() or below.
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/input.c,v 1.129 1995/02/08 13:29:39 pgf Exp $
 *
 */

#include	"estruct.h"
#include	"edef.h"

#define	DEFAULT_REG	-1

#define INFINITE_LOOP_COUNT 1200

typedef	struct	_kstack	{
	struct	_kstack	*m_link;
	int	m_save;		/* old value of 'kbdmode'		*/
	int	m_indx;		/* index identifying this macro		*/
	int	m_rept;		/* the number of times to execute the macro */
	ITBUFF  *m_kbdm;		/* the macro-text to execute		*/
	ITBUFF  *m_dots;		/* workspace for "." command		*/
#ifdef GMDDOTMACRO
	ITBUFF  *m_DOTS;		/* save-area for "." command		*/
	int	m_RPT0;		/* saves 'dotcmdcnt'			*/
	int	m_RPT1;		/* saves 'dotcmdrep'			*/
#endif
	} KSTACK;

/*--------------------------------------------------------------------------*/
static	ITBUFF *	TempDot P(( int ));
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
static	ITBUFF  *KbdMacro;	/* keyboard macro, recorded	*/
static	int	last_eolchar;	/* records last eolchar-match in 'kbd_string' */

/*--------------------------------------------------------------------------*/

/*
 * Returns a pointer to the buffer that we use for saving text to replay with
 * the "." command.
 */
static ITBUFF *
TempDot(init)
int	init;
{
	static	ITBUFF  *tmpcmd;	/* dot commands, 'til we're sure */

	if (kbdmode == PLAY) {
		if (init)
			(void)itb_init(&(KbdStack->m_dots), abortc);
		return KbdStack->m_dots;
	}
	if (init || (tmpcmd == 0))
		(void)itb_init(&tmpcmd, abortc);
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

	/* in case this is right after a shell escape */
	update(TRUE);

	for (;;) {
		mlforce("%s [y/n]? ",prompt);
		c = keystroke();	/* get the response */

		if (ABORTED(c))		/* Bail out! */
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

	update(TRUE);

	for (;;) {
		mlforce("%s ",prompt);
		*cp = keystroke();	/* get the response */

		if (ABORTED(*cp))	/* Bail out! */
			return(ABORT);

		if (strchr(respchars,*cp))
			return TRUE;

		kbd_alarm();
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
		c = keystroke();
		if (ABORTED(c))
			return ABORT;
	}

	if (c == '@' && at_dft != -1) {
		c = at_dft;
	} else if (reg2index(c) < 0) {
		mlwarn("[Invalid register name]");
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
		register int	c = itb_peek(dotcmd);
		if (isdigit(c) && c < '9')
			itb_stuff(dotcmd, ++c);
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
		ITBUFF	*tmp = TempDot(FALSE);
		(void)itb_append(&tmp, c);
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
		(void)itb_append(&KbdMacro, c);
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
	register ITBUFF	*buffer;

	if (dotcmdmode == PLAY) {

		if (interrupted()) {
			dotcmdmode = STOP;
			return intrc;
		} else {

			if (!itb_more(buffer = dotcmd)) {
				if (!eatit) {
					if (dotcmdrep > 1)
						return itb_get(buffer, 0);
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
						itb_first(buffer);
					}
				}
			}

			/* if there is some left... */
			if (itb_more(buffer)) {
				if (eatit)
					c = itb_next(buffer);
				else
					c = itb_peek(buffer);
				return c;
			}
		}
	}

	if (kbdmode == PLAY) { /* if we are playing a keyboard macro back, */

		if (interrupted()) {
			while (kbdmode == PLAY)
				finish_kbm();
			return intrc;
		} else {

			if (!itb_more(buffer = KbdStack->m_kbdm)) {
				if (--(KbdStack->m_rept) >= 1)
					itb_first(buffer);
				else
					finish_kbm();
			}

			if (kbdmode == PLAY) {
				buffer = KbdStack->m_kbdm;
				if (eatit)
					record_dot_char(c = itb_next(buffer));
				else
					c = itb_peek(buffer);
			}
		}
	}

	return c;
}

static ITBUFF *tungottenchars = NULL;

void
unkeystroke(c)
int c;
{
	mapungetc(c|NOREMAP);
}

int
mapped_keystroke()
{
	return lastkey = mapped_c(DOMAP,NOQUOTED);
}

int
keystroke()
{
	return lastkey = mapped_c(NODOMAP,NOQUOTED);
}

int
keystroke8()
{
	int c;
	for (;;) {
		c = mapped_c(NODOMAP,NOQUOTED);
		if ((c & ~0xff) == 0)
			return lastkey = c;
		kbd_alarm();
	}
}

int
keystroke_raw8()
{
	int c;
	for (;;) {
		c = mapped_c(NODOMAP,QUOTED);
		if ((c & ~0xff) == 0)
			return lastkey = c;
		kbd_alarm();
	}
}

int
mapped_keystroke_raw()
{
	return lastkey = mapped_c(DOMAP,QUOTED);
}


int
keystroke_avail()
{
	return mapped_c_avail();
}

int
tgetc_avail()
{
	return tungotcnt > 0 ||
		get_recorded_char(FALSE) != -1 || 
		sysmapped_c_avail();
}

int
tgetc(quoted)
int quoted;
{
	register int c;	/* fetched character */

	if (tungotcnt > 0) {
		c = itb_get(tungottenchars, (ALLOC_T) --tungotcnt);
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
					c = sysmapped_c();
				} while (c == -1);
			}
			doing_kbd_read = FALSE;
			if (quoted || (c != kcod2key(intrc)))
				record_char(c);
		}
	}

	/* and finally give the char back */
	return c;
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

	c = mapped_keystroke();

	if (c == cntl_a) {
		prefix = CTLA;
		c = mapped_keystroke();
	} else if (c == cntl_x) {
		prefix = CTLX;
		c = mapped_keystroke();
	} else if (c == poundc) {
		prefix = SPEC;
		c = mapped_keystroke();
	}

	c |= prefix;

	/* otherwise, just return it */
	return (lastcmd = c);
}

/* get a string consisting of inclchartype characters from the current
	position.  if inclchartype is 0, return everything to eol */
#if ANSI_PROTOS
int screen_string (char *buf, int bufn, CHARTYPE inclchartype )
#else
int
screen_string(buf,bufn,inclchartype)
char *buf;
int bufn;
CHARTYPE inclchartype;
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
#if OPT_VMS_PATH
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
	} else if (shell && (c == '!')
#ifdef only_expand_first_bang
			/* without this check, do as vi does -- expand '!'
			   to previous command anywhere it's typed */
			&& cpos <= (buf[0] == '!')
#endif
		) {
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
	int dontmap = (options & KBD_NOMAP);
	int firstch = TRUE;
	int newpos;
	char buf[NLINE];

	last_eolchar = EOS;	/* ...in case we don't set it elsewhere */

	if (clexec) {
		execstr = token(execstr, extbuf, eolchar);
		status = (*extbuf != EOS);
		if (status) { /* i.e. we got some input */
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
		if (quotef)
			c = keystroke_raw8();
		else if (dontmap)
			/* this looks wrong, but isn't.  no mapping will happen
			anyway, since we're on the command line.  we want SPEC
			keys to be expanded to #c, but no further.  this does
			that */
			c = mapped_keystroke_raw();
		else
			c = keystroke();

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
						(void)keystroke();
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
			(void)strncpy0(extbuf, buf, (SIZE_T)bufn);
			status = (*extbuf != EOS);

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
		if (ABORTED(c) && quotef == FALSE && !dontmap) {
			buf[cpos] = EOS;
			status = esc_func(FALSE, 1);
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
					unkeystroke(c);
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
				unkeystroke(c);
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
					if (!keystroke_avail())
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
#if OPT_POPUPCHOICE
	popdown_completions();
#endif
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
	ITBUFF	*tmp;
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
	(void)itb_sappend(&tmp, temp);
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
		ITBUFF	*tmp = TempDot(FALSE);
		if (itb_length(tmp) == 0	/* keep the old value */
		 || itb_copy(&dotcmd, tmp) != 0) {
			itb_first(dotcmd);
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

	if (dotcmdmode != STOP || itb_length(dotcmd) == 0) {
		dotcmdmode = STOP;
		dotcmdarg = FALSE;
		return FALSE;
	}

	if (n == 0) n = 1;

	dotcmdcnt = dotcmdrep = n;  /* remember how many times to execute */
	dotcmdmode = PLAY;	/* put us in play mode */
	itb_first(dotcmd);	/*    at the beginning */

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
		 && (dotcmd->itb_last+1 >= dotcmd->itb_used)) {
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
	return (itb_init(&KbdMacro, abortc) != 0);
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
	itb_first(KbdMacro);
	while (itb_more(KbdMacro))
		if (!kinsert(itb_next(KbdMacro)))
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
				while (kbdmode == PLAY)
					finish_kbm();
				mlwarn("[Error: currently executing %s%c]",
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
ITBUFF *	ptr;			/* data to interpret */
{
	register KSTACK *sp;
	ITBUFF  *tp = 0;

	if (interrupted())
		return FALSE;

	if (kbdmode == RECORD && KbdStack != 0)
		return TRUE;

	if (itb_length(ptr)
	 && (sp = typealloc(KSTACK)) != 0
	 && itb_copy(&tp, ptr) != 0) {

		/* make a copy of the macro in case recursion alters it */
		itb_first(tp);

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
		return (itb_init(&dotcmd, abortc) != 0
		  &&    itb_init(&(sp->m_dots), abortc) != 0);
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

			itb_free(&(sp->m_kbdm));
			itb_free(&(sp->m_dots));
#ifdef GMDDOTMACRO
			itb_free(&dotcmd);
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
