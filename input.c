/*	INPUT:	Various input routines for MicroEMACS
		written by Daniel Lawrence
		5/9/86						*/

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
	char c;			/* input character */

	for (;;) {
#if     NeWS
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
	return kbd_string(prompt, buf, bufn, '\n',EXPAND);
}

/* as above, but don't expand special punctuation, like #, %, ~, etc. */
mlreply_no_exp(prompt, buf, bufn)
char *prompt;
char *buf;
{
	return kbd_string(prompt, buf, bufn, '\n',NO_EXPAND);
}

/*	kcod2key:	translate 10-bit keycode to single key value */
kcod2key(c)
int c;
{
	return c & 0x7f;
}


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
}

tpeekc()
{
	return tungotc;
}

/*	tgetc:	Get a key from the terminal driver, resolve any keyboard
		macro action					*/
int 
tgetc()
{
	int c;	/* fetched character */

	if (dotcmdmode == PLAY) {
		
		if (interrupted) {
			dotcmdmode = STOP;
			return (kcod2key(abortc));
		}

		/* if there is some left... */
		if (dotcmdptr < dotcmdend)
			return(*dotcmdptr++);

		/* at the end of last repitition? */
		if (--dotcmdrep < 1) {
			dotcmdmode = RECORD;
			tmpcmdptr = &tmpcmdm[0];
			tmpcmdend = tmpcmdptr;
#if	VISMAC == 0
			/* force a screen update after all is done */
			update(FALSE);
#endif
		} else {

			/* reset the macro to the begining for the next rep */
			dotcmdptr = &dotcmdm[0];
			return((int)*dotcmdptr++);
		}
	} else if (kbdmode == PLAY) {
	/* if we are playing a keyboard macro back, */

		/* if there is some left... */
		if (kbdptr < kbdend)
			return((int)*kbdptr++);

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
			return((int)*kbdptr++);
		}
	}


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

	/* and finally give the char back */
	/* record it for $lastkey */
	return(lastkey = c);
}

/*	KBD_KEY:	Get one keystroke. The only prefix legal here
			is the SPEC prefix.  */
kbd_key()
{
	int    c;

	/* get a keystroke */
        c = tgetc();

#if	MSDOS | ST520
	if (c == 0) {			/* Apply SPEC prefix	*/
	        c = tgetc();
		if (insertmode) continue;
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
			if (insertmode) continue;
			return(last1key = SPEC | c);
		}

		/* next, a 2 char sequence */
		d = tgetc();
		if (d == '~') {
			if (insertmode) continue;
			return(last1key = SPEC | c);
		}

		/* decode a 3 char sequence */
		c = d + ' ';
		/* if a shifted function key, eat the tilde */
		if (d >= '0' && d <= '9')
			d = tgetc();
		if (insertmode) continue;
		return(last1key = SPEC | c);
	}
#endif

#if  WANGPC
	if (c == 0x1F) {	/* Apply SPEC prefix    */
	        c = tgetc();
		if (insertmode) continue;
		return(last1key = SPEC | c);
	}
#endif

#if  TERMCAP & are_you_sure
	/* Apply SPEC prefix    */
	if (c == '#' && !lineinput && !insertmode && !isnamedcmd) {
	        c = tgetc();
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

	/* process META prefix */
	if (c == metac) {
		c = kbd_key();
	        if (islower(c))		/* Force to upper */
        	        c = toupper(c);
#if	0	/* temporary ESC sequence fix......... */
		if (c == '[') {
			c = kbd_key();
			return (lastcmd = SPEC | c);
		}
#endif
		return (lastcmd = META | c);
	}

	/* process CTLX prefix */
	if (c == ctlxc) {
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

	setmark();
	while (s == TRUE && i < bufn) {
		buf[i] = lgetc(curwp->w_dotp, curwp->w_doto);
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
kbd_string(prompt, buf, bufn, eolchar, expand)
char *prompt;
char *buf;
int eolchar;
{
	register int cpos;	/* current character position in string */
	register int c;
	register int quotef;	/* are we quoting the next char? */
	int firstchar = TRUE;

	if (clexec)
		return nextarg(buf);

	lineinput = TRUE;

	cpos = 0;
	quotef = FALSE;


	/* prompt the user for the input string */
	mlwrite(prompt);
	/* put out the default response, which comes in already in the
		buffer */
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
		if (c == '\n' || (c == eolchar && quotef == FALSE))
		{
			if (buf[cpos] != '\0')
				buf[cpos++] = 0;

			lineinput = FALSE;
			/* if buffer is empty, return FALSE */
			if (buf[0] == 0)
				return(FALSE);

			return(TRUE);
		}

#if     NeWS    /* make sure cursor is where we think it is before output */
		TTmove(ttrow,ttcol) ;
#endif
		/* change from command form back to character form */
		c = kcod2key(c);
		
		if (c == kcod2key(abortc) && quotef == FALSE) {
			/* Abort the input? */
			if (buf[cpos] != '\0')
				buf[cpos] = 0;
			esc(FALSE, 0);
			TTflush();
			lineinput = FALSE;
			return ABORT;
		} else if (isbackspace(c) && quotef==FALSE) {
			/* rubout/erase */
			if (cpos != 0) {
				outstring("\b \b");
				--ttcol;

				if (!isprint(buf[--cpos])) {
					outstring("\b \b");
					--ttcol;
				}

				TTflush();
			} else {
				buf[0] = 0;
				mlerase();
				lineinput = FALSE;
				return FALSE;
			}

		} else if (c == kcod2key(killc) && quotef == FALSE) {
			/* C-U, kill */
		killit:
			while (cpos != 0) {
				outstring("\b \b");
				--ttcol;

				if (!isprint(buf[--cpos])) {
					outstring("\b \b");
					--ttcol;
				}
			}
			TTflush();

		} else if (expand == EXPAND && (c == '%' || c == '#')) {
			/* we prefer to expand to filenames, but a buffer name
				will do */
			char *cp = NULL;
			char *hist_lookup();
			if (firstchar == TRUE) {
				tungetc(c);
				goto killit;
			}
			if (c == '%') {
				cp = curbp->b_fname;
				if (!*cp || isspace(*cp))
					cp = curbp->b_bname;
			} else if (c == '#') {
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
			TTflush();
		} else if (c == kcod2key(quotec) && quotef == FALSE) {
			quotef = TRUE;
		} else {
			quotef = FALSE;
			if (firstchar == TRUE) {
				tungetc(c);
				goto killit;
			}
			if (cpos < bufn-1) {
				buf[cpos++] = c;

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
		firstchar = FALSE;
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
