/*	This file is for functions having to do with key bindings,
	descriptions, help commands and startup file.

	written 11-feb-86 by Daniel Lawrence
								*/

#include	<stdio.h>
#include	"estruct.h"
#include	"edef.h"
#include	"epath.h"

/* dummy prefix binding functions */
extern CMDFUNC f_meta, f_cex, f_unarg, f_esc;

/* give me some help!!!! bring up a buffer and read the help file into it */
help(f, n)
{
	register WINDOW *wp;	/* scaning pointer to windows */
	register BUFFER *bp;	/* buffer pointer to help */
	char *fname;		/* ptr to file returned by flook() */

	/* first check if we are already here */
	bp = bfind("[Help]", OK_CREAT, BFSCRTCH);
	if (bp == NULL)
		return FALSE;

	if (bp->b_active == FALSE) { /* never been used */
		fname = flook(pathname[1], FL_ANYWHERE);
		if (fname == NULL) {
			mlwrite("[Sorry, can't find the help information]");
			zotbuf(bp);
			return(FALSE);
		}
		/* and read the stuff in */
		if (readin(fname, 0, bp, TRUE) == FALSE ||
				popupbuff(bp) == FALSE) {
			zotbuf(bp);
			return(FALSE);
		}
		strcpy(bp->b_bname,"[Help]");
	        sprintf(bp->b_fname, "       %s   %s",prognam,version);
		bp->b_mode |= MDVIEW;
		bp->b_mode &= ~MDEXACT;
		bp->b_flag |= BFSCRTCH;
	}
	return swbuffer(bp);
}

#if REBIND

deskey(f, n)	/* describe the command for a certain key */
{
	register int c;		/* key to describe */
	register char *ptr;	/* string pointer to scan output strings */
	char outseq[NSTRING];	/* output buffer for command sequence */

	/* prompt the user to type us a key to describe */
	mlwrite(": describe-key ");

	/* get the command sequence to describe
	   change it to something we can print as well */

	/* check to see if we are executing a command line */
	if (clexec) {
		char tok[NSTRING];
		macarg(tok);	/* get the next token */
		c = prc2kcod(tok);
	} else {
		c = kbd_seq();
	}
	kcod2prc(c, &outseq[0]);

	/* and dump it out */
	ostring(outseq);
	ostring(" ");

	/* find the right ->function */
	if ((ptr = fnc2engl(kcod2fnc(c))) == NULL)
		ptr = "Not Bound";

	/* output the command sequence */
	ostring(ptr);
}

/* bindkey:	add a new key to the key binding table		*/

bindkey(f, n)
int f, n;	/* command arguments [IGNORED] */
{
	register int c;		/* command key to bind */
	register CMDFUNC *kcmd;	/* ptr to the requested function to bind to */
	register KBIND *kbp;	/* pointer into a binding table */
	register int found;	/* matched command flag */
	char outseq[80];	/* output buffer for keystroke sequence */
	char *fnp;
	char *kbd_engl();

	/* prompt the user to type in a key to bind */
	mlwrite(": bind-to-key ");

	/* get the function name to bind it to */
#if	NeWS
	newsimmediateon() ;
	fnp = kbd_engl();
	newsimmediateoff() ;
#else
	fnp = kbd_engl();
#endif

	if (fnp == NULL || (kcmd = engl2fnc(fnp)) == NULL) {
		mlwrite("[No such function]");
		return(FALSE);
	}
	ostring(" ");

	/* get the command sequence to bind */
	if (clexec) {
		char tok[NSTRING];
		macarg(tok);	/* get the next token */
		c = prc2kcod(tok);
	} else {
		/* perhaps we only want a single key, not a sequence */
		/* 	(see more comments below) */
		if ((kcmd == &f_meta) || (kcmd == &f_cex) ||
	            (kcmd == &f_unarg) || (kcmd == &f_esc))
			c = kbd_key();
		else
			c = kbd_seq();
	}

	/* change it to something we can print as well */
	kcod2prc(c, &outseq[0]);

	/* and dump it out */
	ostring(outseq);

	/* if the function is a prefix key, i.e. we're changing the definition
		of a prefix key, then they typed a dummy function name, which
		has been translated into a dummy function pointer */
	if (kcmd == &f_meta || kcmd == &f_cex ||
	    kcmd == &f_unarg || kcmd == &f_esc) {
	    	register CMDFUNC **cfp;
		/* search for an existing binding for the prefix key */
		cfp = asciitbl;
		found = FALSE;
		for (cfp = asciitbl; cfp < &asciitbl[128]; cfp++) {
			if (*cfp == kcmd) {
				unbindchar(cfp - asciitbl);
				break;
			}
		}

		/* reset the appropriate global prefix variable */
		if (kcmd == &f_meta)
			metac = c;
		if (kcmd == &f_cex)
			ctlxc = c;
		if (kcmd == &f_unarg)
			reptc = c;
		if (kcmd == &f_esc)
			abortc = c;
	}
	
	if ((c & (META|SPEC|CTLX)) == 0) {
		asciitbl[c] = kcmd;
	} else {
		for(kbp = kbindtbl; kbp->k_cmd && kbp->k_code != c; kbp++)
			;
		if (kbp->k_cmd) { /* found it, change it in place */
			kbp->k_cmd = kcmd;
		} else {
			if (kbp >= &kbindtbl[NBINDS-1]) {
				mlwrite("Prefixed binding table full");
				return(FALSE);
			}
			kbp->k_code = c;	/* add keycode */
			kbp->k_cmd = kcmd; /* and func pointer */
			++kbp;		/* and make sure the next is null */
			kbp->k_code = 0;
			kbp->k_cmd = NULL;
		}
	}

	return TRUE;
}

/* unbindkey:	delete a key from the key binding table	*/

unbindkey(f, n)
int f, n;	/* command arguments [IGNORED] */
{
	register int c;		/* command key to unbind */
	char outseq[80];	/* output buffer for keystroke sequence */

	/* prompt the user to type in a key to unbind */
	mlwrite(": unbind-key ");

	/* get the command sequence to unbind */
	if (clexec) {
		char tok[NSTRING];
		macarg(tok);	/* get the next token */
		c = prc2kcod(tok);
	} else {
		c = kbd_seq();
	}

	/* change it to something we can print as well */
	kcod2prc(c, &outseq[0]);

	/* and dump it out */
	ostring(outseq);

	/* if it isn't bound, bitch */
	if (unbindchar(c) == FALSE) {
		mlwrite("[Key not bound]");
		return(FALSE);
	}
	return(TRUE);
}

unbindchar(c)
int c;		/* command key to unbind */
{
	register KBIND *kbp;	/* pointer into the command table */
	register KBIND *skbp;	/* saved pointer into the command table */
	register int found;	/* matched command flag */

	if ((c & (META|SPEC|CTLX)) == 0) {
		asciitbl[c] = NULL;
	} else {
		/* search the table to see if the key exists */
		for (kbp = kbindtbl; kbp->k_cmd && kbp->k_code != c; kbp++)
			;

		/* if it isn't bound, bitch */
		if (kbp->k_cmd == NULL)
			return(FALSE);

		/* save the pointer and scan to the end of the table */
		skbp = kbp;
		while (kbp->k_cmd != NULL)
			++kbp;
		--kbp;		/* backup to the last legit entry */

		/* copy the last entry to the current one */
		skbp->k_code = kbp->k_code;
		skbp->k_cmd  = kbp->k_cmd;

		/* null out the last one */
		kbp->k_code = 0;
		kbp->k_cmd = NULL;
	}
	return TRUE;
}

/* describe bindings bring up a fake buffer and list the key bindings
		   into it with view mode			*/
desbind(f, n)
{
        register BUFFER *bp;
        register int    s;

	/* create the buffer list buffer   */
	bp = bfind("[Binding List]", OK_CREAT, BFSCRTCH);
	if (bp == NULL)
		return FALSE;
	
	if (bp->b_active == FALSE) { /* never been used */
	        if ((s=bclear(bp)) != TRUE) /* clear old text (?) */
	                return (s);
	        s = buildlist(TRUE,"",bp);
		if (s != TRUE || popupbuff(bp) == FALSE) {
			mlwrite("[Sorry, can't list. ]");
			zotbuf(bp);
	                return (s);
	        }
	        strcpy(bp->b_fname, "");
		bp->b_mode |= MDVIEW;
		bp->b_flag |= BFSCRTCH;
	        bp->b_flag &= ~BFCHG;        /* Don't complain!      */
	        bp->b_active = TRUE;
	}
        return (TRUE);
}

#if	APROP
apro(f, n)	/* Apropos (List functions that match a substring) */
{
	static char mstring[NSTRING] = 0;	/* string to match cmd names to */
        register BUFFER *bp;
        register int    s;


	s = mlreply("Apropos string: ", mstring, NSTRING - 1);
	if (s != TRUE)
		return(s);

	/* create the buffer list buffer   */
	bp = bfind("[Binding List]", OK_CREAT, BFSCRTCH);
	if (bp == NULL)
		return FALSE;
	
	if (bp->b_active == FALSE) { /* never been used */
	        if ((s=bclear(bp)) != TRUE) /* clear old text (?) */
	                return (s);
	        s = buildlist(FALSE,mstring,bp);
		if (s != TRUE || popupbuff(bp) == FALSE) {
			mlwrite("[Sorry, can't list. ]");
			zotbuf(bp);
	                return (s);
	        }
	        strcpy(bp->b_fname, "");
		bp->b_mode |= MDVIEW;
		bp->b_flag |= BFSCRTCH;
	        bp->b_flag &= ~BFCHG;        /* Don't complain!      */
	        bp->b_active = TRUE;
	}
        return (TRUE);
}
#endif

/* build a binding list (limited or full) */
buildlist(type, mstring, bp)
int type;		/* true = full list,   false = partial list */
char *mstring;		/* match string if a partial list */
register BUFFER *bp;	/* buffer to put binding list into */
{
#if	ST520 & LATTICE
#define	register		
#endif
	register WINDOW *wp;	/* scanning pointer to windows */
	register KBIND *kbp;	/* pointer into a key binding table */
	register CMDFUNC **cfp;	/* pointer into the ascii table */
	register NTAB *nptr;	/* pointer into the name table */
	char *strp;		/* pointer int string to send */
	int cpos;		/* current position to use in outseq */
	char outseq[81];	/* output buffer for keystroke sequence */
	int i;


	/* let us know this is in progress */
	mlwrite("[Building binding list]");

	/* build the contents of this window, inserting it line by line */
	for (nptr = nametbl; nptr->n_cmd != NULL; ++nptr) {

		/* add in the command name */
		strcpy(outseq, fnc2engl(nptr->n_cmd));
		cpos = strlen(outseq);
		
#if	APROP
		/* if we are executing an apropos command
		   and current string doesn't include the search string */
		if (type == FALSE && strinc(outseq, mstring) == FALSE)
		    	continue;
#endif
		for(cfp = asciitbl, i = 0; cfp < &asciitbl[128]; cfp++, i++) {
			if (*cfp == nptr->n_cmd) {
				while (cpos < 25) outseq[cpos++] = ' ';
				kcod2prc(i, &outseq[cpos]);
				addline(bp,outseq,-1);
				cpos = 0;	/* and clear the line */
			}
		}
		for(kbp = kbindtbl; kbp->k_cmd; kbp++) {
			if (kbp->k_cmd == nptr->n_cmd) {
				while (cpos < 25)
					outseq[cpos++] = ' ';
				kcod2prc(kbp->k_code, &outseq[cpos]);
				addline(bp,outseq,-1);
				cpos = 0;	/* and clear the line */
			}
		}
		/* if no key was bound, we need to dump it anyway */
		if (cpos > 0) {
			outseq[cpos] = 0;
			addline(bp,outseq,-1);
		}
	}

	mlwrite("");	/* clear the mode line */
	return(TRUE);
}

#if	APROP
strinc(source, sub)	/* does source include sub? */
char *source;	/* string to search in */
char *sub;	/* substring to look for */
{
	char *sp;	/* ptr into source */
	char *nxtsp;	/* next ptr into source */
	char *tp;	/* ptr into substring */

	/* for each character in the source string */
	sp = source;
	while (*sp) {
		tp = sub;
		nxtsp = sp;

		/* is the substring here? */
		while (*tp) {
			if (*nxtsp++ != *tp)
				break;
			else
				tp++;
		}

		/* yes, return a success */
		if (*tp == 0)
			return(TRUE);

		/* no, onward */
		sp++;
	}
	return(FALSE);
}
#endif

#endif /* REBIND */


/* execute the startup file */

startup(sfname)
char *sfname;	/* name of startup file  */
{
	char *fname;	/* resulting file name to execute */

	/* look up the startup file */
	fname = flook(sfname, FL_HERE_HOME);

	/* if it isn't around, don't sweat it */
	if (fname == NULL) {
		mlwrite("[Can't find startup file %s]",sfname);
		return(TRUE);
	}

	/* otherwise, execute the sucker */
	return(dofile(fname));
}

/*	Look up the existence of a file along the normal or PATH
	environment variable. Look first in the HOME directory if
	asked and possible
*/

char *
flook(fname, hflag)
char *fname;	/* base file name to search for */
int hflag;	/* Look in the HOME environment variable first? */
{
	register char *home;	/* path to home directory */
	register char *path;	/* environmental PATH variable */
	register char *sp;	/* pointer into path spec */
	register int i;		/* index */
	static char fspec[NSTRING];	/* full path spec to search */
	char *getenv();

	/* tak care of special cases */
	if (!fname || !fname[0] || isspace(fname[0]))
		return NULL;
	else if (fname[0] == '!')
		return fname;
		
	/* always try the current directory first */
	if (ffropen(fname) == FIOSUC) {
		ffclose();
		return(fname);
	}

	if (hflag == FL_HERE)
		return NULL;

#if	ENVFUNC

	if (hflag) {
		home = getenv("HOME");
		if (home != NULL) {
			/* build home dir file spec */
			strcpy(fspec, home);
			strcat(fspec, "/");
			strcat(fspec, fname);

			/* and try it out */
			if (ffropen(fspec) == FIOSUC) {
				ffclose();
				return(fspec);
			}
		}
	}

	if (hflag == FL_HERE_HOME)
		return NULL;

#if PATHLOOK
	/* get the PATH variable */
	path = getenv("PATH");
	if (path != NULL)
		while (*path) {

			/* build next possible file spec */
			sp = fspec;
			while (*path && (*path != PATHCHR))
				*sp++ = *path++;
			*sp++ = '/';
			*sp = 0;
			strcat(fspec, fname);

			/* and try it out */
			if (ffropen(fspec) == FIOSUC) {
				ffclose();
				return(fspec);
			}

			if (*path == PATHCHR)
				++path;
		}
#endif
#endif

	/* look it up via the old table method */
	for (i=2; i < NPNAMES; i++) {
		strcpy(fspec, pathname[i]);
		strcat(fspec, fname);

		/* and try it out */
		if (ffropen(fspec) == FIOSUC) {
			ffclose();
			return(fspec);
		}
	}


	return NULL;	/* no such luck */
}

/* translate a 10-bit keycode to its printable name (like "M-j")  */
kcod2prc(c, seq)
int c;		/* sequence to translate */
char *seq;	/* destination string for sequence */
{
	char *ptr;	/* pointer into current position in sequence */

	ptr = seq;

	/* apply meta sequence if needed */
	if (c & META) {
		if (isprint(metac)) {
			*ptr++ = c;
		} else {
			/*
			*ptr++ = '^';
			*ptr++ = toalpha(kcod2key(c));
			*/
			*ptr++ = 'M';
		}
		*ptr++ = '-';
	}

	/* apply ^X sequence if needed */
	if (c & CTLX) {
		*ptr++ = '^';
		*ptr++ = 'X';
		*ptr++ = '-';
	}

	/* apply SPEC sequence if needed */
	if (c & SPEC) {
		*ptr++ = 'F';
		*ptr++ = 'N';
		*ptr++ = '-';
	}
	
	c = kcod2key(c);

	/* apply control sequence if needed */
	if (iscntrl(c)) {
		*ptr++ = '^';
		c = toalpha(c);
	}

	/* and output the final sequence */

	if (c == ' ') {
		*ptr++ = '<';
		*ptr++ = 's';
		*ptr++ = 'p';
		*ptr++ = '>';
	} else if (c == '\t') {
		*ptr++ = '<';
		*ptr++ = 't';
		*ptr++ = 'a';
		*ptr++ = 'b';
		*ptr++ = '>';
	} else {
		*ptr++ = c;
	}
	*ptr = 0;	/* terminate the string */
}


/* kcod2fnc:  translate a 10-bit keycode to a function pointer */
/*	(look a key binding up in the binding table)		*/
CMDFUNC *
kcod2fnc(c)
int c;	/* key to find what is bound to it */
{
	register KBIND *kbp;

	if ((c & (META|SPEC|CTLX)) == 0) {
		return asciitbl[c];
	} else {
		for (kbp = kbindtbl; kbp->k_cmd && kbp->k_code != c; kbp++)
			;
		return kbp->k_cmd;
	}
}


/* fnc2engl: translate a function pointer to the english name for 
		that function
*/

char *
fnc2engl(cfp)
CMDFUNC *cfp;	/* ptr to the requested function to bind to */
{
	register NTAB *nptr;	/* pointer into the name table */

	/* skim through the table, looking for a match */
	for (nptr = nametbl; nptr->n_cmd; nptr++) {
		if (nptr->n_cmd == cfp) {
			return(nptr->n_name);
		}
	}
	return NULL;
}

#if NEEDED
/* translate a function pointer to its associated flags */
fnc2flags(func)
CMDFUNC *cfp;	/* ptr to the requested function to bind to */
{
	register NTAB *nptr;	/* pointer into the name binding table */

	/* skim through the table, looking for a match */
	nptr = nametbl;
	while (nptr->n_cmd != NULL) {
		if (nptr->n_cmd == cfp) {
			return nptr->n_flags;
		}
		++nptr;
	}
	return NONE;
}
#endif


/* engl2fnc: match name to a function in the names table
	translate english name to function pointer
 		 return any match or NULL if none
 */
CMDFUNC *
engl2fnc(fname)
char *fname;	/* name to attempt to match */
{
	register NTAB *nptr;	/* pointer to entry in name binding table */

	/* scan through the table, returning any match */
	nptr = nametbl;
	while (nptr->n_cmd != NULL) {
		if (strcmp(fname, nptr->n_name) == 0) {
			return nptr->n_cmd;
		}
		++nptr;
	}
	return NULL;
}

/* prc2kcod: translate printable code to 10 bit keycode */
int 
prc2kcod(k)
char *k;		/* name of key to translate to Command key form */
{
	register int c;	/* key sequence to return */

	/* parse it up */
	c = 0;

	/* first, the META prefix */
	if (*k == 'M' && *(k+1) == '-') {
		c = META;
		k += 2;
	}

	/* next the function prefix */
	if (*k == 'F' && *(k+1) == 'N' && *(k+2) == '-') {
		c |= SPEC;
		k += 3;
	}

	/* control-x as well... (but not with FN) */
	if (*k == '^' && *(k+1) == 'X' && *(k+2) == '-' && !(c & SPEC)) {
		c |= CTLX;
		k += 3;
	}

	/* a control char? */
	if (*k == '^' && *(k+1) != 0) {
		++k;
		c |= *k;
		if (islower(c)) c = toupper(c);
		c = tocntrl(c);
	} else if (!isprint(*k)) {
		*k = toalpha(c);
	} else {
		c |= *k;
	}
	return c;
}

#if ! SMALLER

/* translate printable code (like "M-r") to english command name */
char *
prc2engl(skey)	/* string key name to binding name.... */
char *skey;	/* name of keey to get binding for */
{
	char *bindname;
	unsigned int prc2kcod();

	bindname = fnc2engl(kcod2fnc(prc2kcod(skey)));
	if (bindname == NULL)
		bindname = "ERROR";

	return bindname;
}
#endif

/* get an english command name from the user. Command completion means
 * that pressing a <SPACE> will attempt to complete an unfinished command
 * name if it is unique.
 */
char *
kbd_engl()
{
#if	ST520 & LATTICE
#define register		
#endif
	register int cpos;	/* current column on screen output */
	register int c;
	register char *sp;	/* pointer to string for output */
	register NTAB *nbp;	/* first ptr to entry in name binding table */
	register NTAB *cnbp;	/* current ptr to entry in name binding table */
	register NTAB *lnbp;	/* last ptr to entry in name binding table */
	static char buf[NSTRING]; /* buffer to hold tentative command name */

	/* starting at the beginning of the string buffer */
	cpos = 0;

	/* if we are executing a command line just get the next arg and return */
	if (clexec) {
		if (macarg(buf) != TRUE)
			return NULL;
		return buf;
	}

	lineinput = TRUE;

	/* build a name string from the keyboard */
	while (TRUE) {
		c = tgetc();

		/* if we are at the end, just match it */
		if (c == '\r') {
			buf[cpos] = 0;

			/* and match it off */
			lineinput = FALSE;
			return buf;

		} else if (c == kcod2key(abortc)) {	/* Bell, abort */
			buf[0] = '\0';
			lineinput = FALSE;
			return buf;

		} else if (c == 0x7F || c == '\b') {	/* rubout/erase */
			if (cpos != 0) {
				TTputc('\b');
				TTputc(' ');
				TTputc('\b');
				--ttcol;
				--cpos;
				TTflush();
			} else {
				buf[0] = '\0';
				lineinput = FALSE;
				return buf;
			}

		} else if (c == kcod2key(killc)) {	/* ^U, kill */
			while (cpos != 0) {
				TTputc('\b');
				TTputc(' ');
				TTputc('\b');
				--cpos;
				--ttcol;
			}

			TTflush();

/* the following mess causes the command to terminate if:
	we've got a space
		-or-
	we're in the first few chars and we're switching from punctuation
	to alphanumerics, or vice-versa.  oh yeah -- '!' is considered
	alphanumeric today.
	All this allows things like:
		: e#
		: !ls
		: q!
	to work properly.
	If we pass this "if" with c != ' ', then c is ungotten below,
	so it can be picked up by the commands argument getter later.
*/
		} else if (c == ' ' || (cpos > 0 && cpos < 3 &&
			     ((!ispunct(c) &&  ispunct(buf[cpos-1])) ||
	   ((c != '!' && ispunct(c)) &&
	   	 (buf[cpos-1] == '!' || !ispunct(buf[cpos-1]))) )
			      		)
			) {
/* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
	/* attempt a completion */
	buf[cpos] = 0;		/* terminate it for us */
	nbp = nametbl;	/* scan for matches */
	while (nbp->n_cmd != NULL) {
		if (strncmp(buf, nbp->n_name, strlen(buf)) == 0) {
			/* a possible match! exact? no more than one? */
			if (strcmp(buf, nbp->n_name) == 0 || /* exact? */
				(nbp + 1)->n_cmd == NULL ||
				strncmp(buf, (nbp+1)->n_name, strlen(buf)) != 0)
			{
				/* exact or only one like it.  print it */
				sp = nbp->n_name + cpos;
				while (*sp)
					TTputc(*sp++);
				TTflush();
				if (c != ' ')  /* put it back */
					tungetc(c);
				lineinput = FALSE;
				return nbp->n_name;
			} else {
/* << << << << << << << << << << << << << << << << << */
	/* try for a partial match against the list */

	/* first scan down until we no longer match the current input */
	lnbp = (nbp + 1);
	while ((lnbp+1)->n_cmd != NULL) {
		if (strncmp(buf, (lnbp+1)->n_name, strlen(buf)) != 0)
			break;
		++lnbp;
	}

	/* and now, attempt to partial complete the string, char at a time */
	while (TRUE) {
		/* add the next char in */
		buf[cpos] = nbp->n_name[cpos];

		/* scan through the candidates */
		cnbp = nbp + 1;
		while (cnbp <= lnbp) {
			if (cnbp->n_name[cpos] != buf[cpos])
				goto onward;
			++cnbp;
		}

		/* add the character */
		TTputc(buf[cpos++]);
	}
/* << << << << << << << << << << << << << << << << << */
			}
		}
		++nbp;
	}

	/* no match.....beep and onward */
	TTbeep();
onward:;
	TTflush();
/* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
		} else {
			if (cpos < NSTRING-1 && isprint(c)) {
				buf[cpos++] = c;
				TTputc(c);
			}

			++ttcol;
			TTflush();
		}
	}
}

