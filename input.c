/*	INPUT:	Various input routines for MicroEMACS
 *		written by Daniel Lawrence
 *		5/9/86
 *
 * $Log: input.c,v $
 * Revision 1.23  1991/10/22 14:36:21  pgf
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

/*
 * Ask a yes or no question in the message line. Return either TRUE, FALSE, or
 * ABORT. The ABORT status is returned if the user bumps out of the question
 * with a ^G. Used any time a confirmation is required.
 */

mlyesno(prompt)
char *prompt;
{
	char c; 		/* input character */

	for (;;) {
#if	NeWS
		newsimmediateon() ;
		mlwrite(,"%s [y/n]? ",prompt);
		c = tgetc();		/* get the responce */
		newsimmediateoff() ;
#else
		mlwrite("%s [y/n]? ",prompt);
		c = tgetc();		/* get the responce */
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

mlreply(prompt, buf, bufn)
char *prompt;
char *buf;
{
	return kbd_string(prompt, buf, bufn, '\n',EXPAND,TRUE);
}

/* as above, but don't expand special punctuation, like #, %, ~, etc. */
mlreply_no_exp(prompt, buf, bufn)
char *prompt;
char *buf;
{
	return kbd_string(prompt, buf, bufn, '\n',NO_EXPAND,TRUE);
}

/*	kcod2key:	translate 10-bit keycode to single key value */
/* probably defined as a macro in estruct.h */
#ifndef kcod2key
kcod2key(c)
int c;
{
	return c & 0x7f;
}
#endif


/* the numbered buffer names increment each time they are referenced */
incr_dot_kregnum()
{
	if (dotcmdmode == PLAY) {
		if (isdigit(*dotcmdptr) && *dotcmdptr < '9')
			(*dotcmdptr)++;
	}
}

int tungotc = -1;

tungetc(c)
{
	tungotc = c;
	if (dotcmdmode == RECORD) {
		if (tmpcmdend > tmpcmdm) {
			tmpcmdend--;
			tmpcmdptr--;
		}
	}
	if (kbdmode == RECORD) {
		if (kbdend > kbdm)
			kbdend--;
			kbdptr--;
	}
}

tpeekc()
{
	return tungotc;
}

/* get the next character of a replayed '.' or macro */
get_recorded_char(eatit)
int eatit;  /* consume the character? */
{
	if (dotcmdmode == PLAY) {
	        
		if (interrupted) {
			dotcmdmode = STOP;
			return (kcod2key(abortc));
		}

		/* if there is some left... */
		if (dotcmdptr < dotcmdend) {
			if (eatit)
				return *dotcmdptr++;
			else
				return *dotcmdptr;
		}

		if (!eatit) {
			if (dotcmdrep <= 1)
				return -1;
			else
				return dotcmdm[0];
		}

		/* at the end of last repitition? */
		if (--dotcmdrep < 1) {
			dotcmdmode = STOP;
			dotcmdbegin(); /* immediately start recording
					   again, just in case */
#if	VISMAC == 0
			/* force a screen update after all is done */
			update(FALSE);
#endif
			return -1;
		} else {

			/* reset the macro to the begining for the next rep */
			dotcmdptr = &dotcmdm[0];
			if (eatit)
				return (int)*dotcmdptr++;
			else
				return (int)*dotcmdptr;
		}
	} else if (kbdmode == PLAY) {
	/* if we are playing a keyboard macro back, */

		if (interrupted) {
			kbdmode = STOP;
			return (kcod2key(abortc));
		}

		/* if there is some left... */
		if (kbdptr < kbdend) {
			if (eatit)
				return (int)*kbdptr++;
			else
				return (int)*kbdptr;
		}

		/* at the end of last repitition? */
		if (--kbdrep < 1) {
			kbdmode = STOP;
#if	VISMAC == 0
			/* force a screen update after all is done */
			update(FALSE);
#endif
		} else {

			/* reset the macro to the begining for the next rep */
			kbdptr = &kbdm[0];
			if (eatit)
				return (int)*kbdptr++;
			else
				return (int)*kbdptr;
		}
	}


	return -1;  /* no stored chars */
}

/* if we should preserve this input, do so */
record_char(c)
int c;
{
	if (c == ESC)
		c = RECORDED_ESC;

	/* save it if we need to */
	if (dotcmdmode == RECORD) {
		*tmpcmdptr++ = c;
		tmpcmdend = tmpcmdptr;

		/* don't overrun the buffer  */
		/* (we're recording, so must be using tmp) */
		if (tmpcmdptr == &tmpcmdm[NKBDM - 1]) {
			dotcmdmode = STOP;
			TTbeep();
		}
	}

	/* save it if we need to */
	if (kbdmode == RECORD) {

		*kbdptr++ = c;
		kbdend = kbdptr;

		/* don't overrun the buffer */
		if (kbdptr == &kbdm[NKBDM - 1]) {
			kbdmode = STOP;
			TTbeep();
		}
	}
}
/*	tgetc:	Get a key from the terminal driver, resolve any keyboard
		macro action					*/
int 
tgetc()
{
	int c;	/* fetched character */

	c = get_recorded_char(TRUE);
	if (c != -1)
		return lastkey = c;

	if (tungotc >= 0) {
		c = tungotc;
		tungotc = -1;
	} else { /* fetch a character from the terminal driver */
		interrupted = 0;
		c = TTgetc();
		if (c == -1 || c == kcod2key(intrc)) {
			c = kcod2key(abortc);
			return lastkey = c;
		}
	}

	record_char(c);

	/* and finally give the char back */
	return(lastkey = c);
}

/*	KBD_KEY:	Get one keystroke. The only prefix legal here
			is the SPEC prefix.  */
kbd_key()
{
	int    c;
#ifdef ANSI_SPEC
	static insert_mode_was;
#endif

	/* get a keystroke */
	c = tgetc();

#ifdef ANSI_SPEC
	if (insert_mode_was && last1key == -abortc) {
		/* then we just read the command we pushed before */
		extern CMDFUNC f_insert;
		static back_to_ins_char = -1;
		if (back_to_ins_char == -1) /* try to initialize it.. */
			back_to_ins_char = fnc2key(&f_insert);
		if (back_to_ins_char == -1) /* ... but couldn't */
			mlwrite("Can't re-enter insert mode");
		else
			tungetc(back_to_ins_char);
		insertmode = insert_mode_was;
		insert_mode_was = FALSE;
	}

	if (c == RECORDED_ESC) { /* if this is being replayed... */
		/* ...then only look for esc sequences if there's input left */
		if (get_recorded_char(FALSE) != -1)
			c = ESC;
		else
			return (last1key = ESC);
	}

	if (c == ESC) {
		int nextc;

		if (abortc != ESC) {  /* then just eat the sequence */
			c = tgetc();
			if (c == '[') {
				c = tgetc();
				return(last1key = SPEC | c);
			}
			return(last1key = c);
		}

		/* else abortc must be ESC.  big surprise. */

		/* if there's no recorded input, and no user typeahead */
		if ((nextc = get_recorded_char(FALSE)) == -1 && !typahead())
			catnap(50); /* give it a little extra time... */

		/* and then, if there _was_ recorded input or new typahead... */
		if (nextc != -1 || typahead()) {
			if ((c = tgetc()) == '[') {  /* eat ansi sequences */
				c = tgetc();
				if (insertmode == REPLACECHAR) {
					/* eat the sequence, but return abort */
					return abortc;
				}
				if (insertmode) {
					/* remember we were in insert mode */
					insert_mode_was = insertmode;
					/* save the code, but return flag to
						ins() so it can clean up */
					tungetc(SPEC | c);
					return(last1key = -abortc);
				} else { /* just return the function code */
					return(last1key = SPEC | c);
				}
			} else {
				tungetc(c);
				return (last1key = ESC);
			}
		}
	}
#endif

#if	MSDOS | ST520
	if (c == 0) {			/* Apply SPEC prefix	*/
		c = tgetc();
		if (!insertmode)
			return(last1key = SPEC | c);
	}
#endif

#if	AMIGA
	/* apply SPEC prefix */
	if ((unsigned)c == 155) {
		int	d;
		c = tgetc();

		/* first try to see if it is a cursor key */
		if ((c >= 'A' && c <= 'D') || c == 'S' || c == 'T') {
			if (!insertmode)
				return(last1key = SPEC | c);
		}

		/* next, a 2 char sequence */
		d = tgetc();
		if (d == '~') {
			if (!insertmode)
				return(last1key = SPEC | c);
		}

		/* decode a 3 char sequence */
		c = d + ' ';
		/* if a shifted function key, eat the tilde */
		if (d >= '0' && d <= '9')
			d = tgetc();
		if (!insertmode)
			return(last1key = SPEC | c);
	}
#endif

#if  WANGPC
	if (c == 0x1F) {	/* Apply SPEC prefix	*/
		c = tgetc();
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

screen_string(buf,bufn,inclchartype)
char *buf;
{
	register int i = 0;
	register int s = TRUE;

	MK = DOT;
	while (s == TRUE && i < bufn && !is_at_end_of_line(DOT)) {
		buf[i] = char_at(DOT);
		if (!istype(inclchartype, buf[i]))
			break;
		s = forwchar(FALSE, 1);
		i++;
	}
	buf[i] = '\0';
	swapmark();

	return buf[0] != '\0';
}

/*	A more generalized prompt/reply function allowing the caller
	to specify a terminator other than '\n'.  Both are accepted.
	Assumes the buffer already contains a valid (possibly
	null) string to use as the default response.
*/
kbd_string(prompt, extbuf, bufn, eolchar, expand, dobackslashes)
char *prompt;
char *extbuf;
int eolchar;
{
	register int cpos, extcpos;/* current character position in string */
	register int c;
	register int quotef;	/* are we quoting the next char? */
	register int backslashes; /* are we quoting the next expandable char? */
	int firstchar = TRUE;
	char buf[256];

	if (clexec)
		return nextarg(extbuf);

	lineinput = TRUE;

	cpos = extcpos = 0;
	quotef = FALSE;
	backslashes = 0;


	/* prompt the user for the input string */
	mlwrite(prompt);

	/* add backslash escapes in front of volatile characters */
	while((c = extbuf[extcpos]) != '\0' && extcpos < bufn-1) {
		switch(extbuf[extcpos]) {
		case '%':
		case '#':
		case ':':
			if (dobackslashes && expand == EXPAND)
				buf[cpos++] = '\\'; /* add extra */
		default:
			break;
		case '\\':
			if (dobackslashes)
				buf[cpos++] = '\\'; /* add extra */
			break;
		}
		buf[cpos] = extbuf[extcpos];

		++extcpos;
		++cpos;
	}
	buf[cpos] = '\0';

	/* put out the default response, which comes in already in the buffer */
	cpos = 0;
	while((c = buf[cpos]) != '\0' && cpos < bufn-1) {
		if (!isprint(c)) {
			if (disinp)
				TTputc('^');
			++ttcol;
			c = toalpha(c);
		}

		if (disinp)
			TTputc(c);


		++ttcol;
		++cpos;
	}
	TTflush();

	backslashes = 0; /* by definition, there is an even 
					number of backslashes */
	for (;;) {
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
		if ((c == '\n') || (c == eolchar && (
			(quotef == FALSE && (backslashes & 1) == 0))))
		{
			/* if (buf[cpos] != '\0')
				buf[cpos++] = '\0'; */

			lineinput = FALSE;
			if (dobackslashes)
				remove_backslashes(buf); /* take out quoters */

			strcpy(extbuf,buf);

			/* if buffer is empty, return FALSE */
			return (extbuf[0] == 0) ? FALSE:TRUE;
		}

#if	NeWS	/* make sure cursor is where we think it is before output */
		TTmove(ttrow,ttcol) ;
#endif
		/* change from command form back to character form */
		c = kcod2key(c);
	        
		if (c == kcod2key(abortc) && quotef == FALSE) {
			buf[cpos] = 0;
			esc(FALSE, 0);
			TTflush();
			lineinput = FALSE;
			return ABORT;
		} else if ((isbackspace(c) ||
			c == kcod2key(wkillc) ||
			c == kcod2key(killc)) && quotef==FALSE) {

			/* have we backed thru a "word" yet? */
			int saw_word = FALSE;

			/* rubout/erase */
			if (cpos == 0) {
				buf[0] = '\0';
				mlerase();
				lineinput = FALSE;
				return FALSE;
			}

		killit:
			while (cpos > 0) {
				if (c == kcod2key(wkillc)) {
					if (isspace(buf[cpos-1])) {
						if (saw_word)
							break;
					} else {
						saw_word = TRUE;
					}
				}
				outstring("\b \b");
				--ttcol;

				if (!isprint(buf[--cpos])) {
					outstring("\b \b");
					--ttcol;
				}
				if (backslashes && buf[cpos-1] == '\\') {
					backslashes = 1;
					while (backslashes+1 <= cpos &&
						buf[cpos-1-backslashes] == '\\')
						backslashes++;
				}

				buf[cpos] = '\0';
				if (isbackspace(c))
					break;
			}
			TTflush();

		} else if (expand == EXPAND && ((backslashes & 1 ) == 0)) {
			/* we prefer to expand to filenames, but a buffer name
				will do */
			char *cp = NULL;
			char *hist_lookup();
			if (firstchar == TRUE) {
				tungetc(c);
				c = killc;
				goto killit;
			}
			switch(c) {
			case '%':
				cp = curbp->b_fname;
				if (!*cp || isspace(*cp))
					cp = curbp->b_bname;
				break;
			case '#':
				cp = hist_lookup(1);  /* returns buffer name */
				if (cp) {
					/* but we want a file */
					BUFFER *bp;
					bp = bfind(cp,NO_CREAT,0);
					cp = bp->b_fname;
					if (!*cp || isspace(*cp)) {
						/* oh well, use the buffer */
						cp = bp->b_bname;
					}
				}
				break;
			case ':':
				{
				char str[80];
				if (screen_string(str, 80, _path))
					cp = str;
				break;
				}
			default:
				goto trymore;
			}
		        
			if (!cp) {
				TTbeep();
				continue;
			}
			while (cpos < bufn-1 && (c = *cp++)) {
				buf[cpos++] = c;
				if (!isprint(c)) {
					outstring("^");
					++ttcol;
					c = toalpha(c);
				}
				if (disinp)
					TTputc(c);
				++ttcol;
			}
			buf[cpos] = '\0';
			TTflush();
		} else {
		trymore:
			if (c == kcod2key(quotec) && quotef == FALSE) {
				quotef = TRUE;
			} else	{
				if (dobackslashes && c == '\\')
					backslashes++;
				else
					backslashes = 0;
				quotef = FALSE;
				if (firstchar == TRUE) {
					tungetc(c);
					c = killc;
					goto killit;
				}
				if (cpos < bufn-1) {
					buf[cpos++] = c;
					buf[cpos] = '\0';

					if (!isprint(c)) {
						outstring("^");
						++ttcol;
						c = toalpha(c);
					}

					if (disinp)
						TTputc(c);

					++ttcol;
					TTflush();
				}
			}
		}
		firstchar = FALSE;
	}
}

/* turn \X into X */
remove_backslashes(s)
char *s;
{
	char *cp;
	while (*s) {
		if (*s == '\\') {  /* shift left */
			for (cp = s; *cp; cp++)
				*cp = *(cp+1);
		}
		s++;
	}
}

outstring(s)	/* output a string of input characters */
char *s;	/* string to output */
{
	if (disinp)
		while (*s)
			TTputc(*s++);
}

ostring(s)	/* output a string of output characters */
char *s;	/* string to output */
{
	if (discmd)
		while (*s)
			TTputc(*s++);
}
