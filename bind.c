/*	This file is for functions having to do with key bindings,
 *	descriptions, help commands and startup file.
 *
 *	written 11-feb-86 by Daniel Lawrence
 *
 * $Log: bind.c,v $
 * Revision 1.25  1992/12/04 09:08:45  foxharp
 * deleted unused assigns
 *
 * Revision 1.24  1992/08/20  23:40:48  foxharp
 * typo fixes -- thanks, eric
 *
 * Revision 1.23  1992/06/04  19:45:14  foxharp
 * cast strlen() to int for new ANSI promotion semantics :-(
 *
 * Revision 1.22  1992/05/19  08:55:44  foxharp
 * more prototype and shadowed decl fixups
 *
 * Revision 1.21  1992/05/16  12:00:31  pgf
 * prototypes/ansi/void-int stuff/microsoftC
 *
 * Revision 1.20  1992/03/13  08:44:53  pgf
 * honor \n like \r in kbd_engl_stat
 *
 * Revision 1.19  1992/03/05  09:19:55  pgf
 * changed some mlwrite() to mlforce(), due to new terse support
 *
 * Revision 1.18  1992/01/05  00:06:13  pgf
 * split mlwrite into mlwrite/mlprompt/mlforce to make errors visible more
 * often.  also normalized message appearance somewhat.
 *
 * Revision 1.17  1992/01/03  23:31:49  pgf
 * use new ch_fname() to manipulate filenames, since b_fname is now
 * a malloc'ed sting, to avoid length limits
 *
 * Revision 1.16  1991/11/01  14:38:00  pgf
 * saber cleanup
 *
 * Revision 1.15  1991/10/22  14:08:23  pgf
 * took out old ifdef BEFORE code
 *
 * Revision 1.14  1991/09/27  02:49:01  pgf
 * removed scalar init of static array
 *
 * Revision 1.13  1991/09/19  13:33:48  pgf
 * MDEXACT changed to MDIGNCASE
 *
 * Revision 1.12  1991/08/12  15:05:14  pgf
 * added fnc2key function, for getting back into insert mode
 *
 * Revision 1.11  1991/08/07  12:35:07  pgf
 * added RCS log messages
 *
 * revision 1.10
 * date: 1991/08/06 15:10:56;
 * global/local values
 * and new printf/list stuff
 * 
 * revision 1.9
 * date: 1991/06/27 19:45:08;
 * fixed prompts
 * 
 * revision 1.8
 * date: 1991/06/03 17:34:51;
 * switch from "meta" etc. to "ctla" etc.
 * 
 * revision 1.7
 * date: 1991/06/03 13:58:22;
 * made bind description list better
 * 
 * revision 1.6
 * date: 1991/06/03 10:18:31;
 * fix apropos bug, and a bind nit
 * 
 * revision 1.5
 * date: 1991/05/31 10:31:34;
 * new kbd_engl_stat() routine, which returns more status, for use in the
 * new namedcmd() code
 * 
 * revision 1.4
 * date: 1990/12/06 19:49:07;
 * always rebuild Binding List buffer on request
 * 
 * revision 1.3
 * date: 1990/10/03 16:00:30;
 * make backspace work for everyone
 * 
 * revision 1.2
 * date: 1990/09/28 14:34:57;
 * changed prc2kcod decl to int
 * 
 * revision 1.1
 * date: 1990/09/21 10:24:44;
 * initial vile RCS revision
*/

#include	<stdio.h>
#include	"estruct.h"
#include	"edef.h"
#include	"epath.h"

/* dummy prefix binding functions */
extern CMDFUNC f_cntl_af, f_cntl_xf, f_unarg, f_esc;

/* give me some help!!!! bring up a buffer and read the help file into it */
/* ARGSUSED */
int
help(f, n)
int f,n;
{
	register BUFFER *bp;	/* buffer pointer to help */
	char *fname;		/* ptr to file returned by flook() */

	/* first check if we are already here */
	bp = bfind("[Help]", OK_CREAT, BFSCRTCH);
	if (bp == NULL)
		return FALSE;

	if (bp->b_active == FALSE) { /* never been used */
		fname = flook(pathname[1], FL_ANYWHERE);
		if (fname == NULL) {
			mlforce("[Sorry, can't find the help information]");
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
		{
			char buf[80];
	        	lsprintf(buf, "       %s   %s",prognam,version);
			ch_fname(bp, buf);
		}
		make_local_b_val(bp,MDVIEW);	/* make it readonly, */
		set_b_val(bp,MDVIEW,TRUE);
		make_local_b_val(bp,MDIGNCASE); /* easy to search, */
		set_b_val(bp,MDIGNCASE,TRUE);
		make_local_b_val(bp,VAL_TAB);	/* and tabbed by 8 */
		set_b_val(bp,VAL_TAB,8);
		bp->b_flag |= BFSCRTCH;
	}
	return swbuffer(bp);
}

#if REBIND

/* ARGSUSED */
int
deskey(f, n)	/* describe the command for a certain key */
int f,n;
{
	register int c;		/* key to describe */
	register char *ptr;	/* string pointer to scan output strings */
	char outseq[NSTRING];	/* output buffer for command sequence */
	char *kcod2prc();

	/* prompt the user to type us a key to describe */
	mlprompt("Describe the function bound to this key sequence: ");

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
	return TRUE;
}

/* bindkey:	add a new key to the key binding table		*/

/* ARGSUSED */
int
bindkey(f, n)
int f, n;	/* command arguments [IGNORED] */
{
	register int c;		/* command key to bind */
	register CMDFUNC *kcmd;	/* ptr to the requested function to bind to */
	register KBIND *kbp;	/* pointer into a binding table */
	char outseq[80];	/* output buffer for keystroke sequence */
	char *fnp;
	char *kbd_engl();
	char *kcod2prc();

	/* prompt the user to type in a key to bind */
	mlprompt("Bind function with english name: ");

	/* get the function name to bind it to */
#if	NeWS
	newsimmediateon() ;
#endif
	fnp = kbd_engl();
#if	NeWS
	newsimmediateoff() ;
#endif

	if (fnp == NULL || (kcmd = engl2fnc(fnp)) == NULL) {
		mlforce("[No such function]");
		return(FALSE);
	}
	mlprompt("...to keyboard sequence (type it exactly): ");

	/* get the command sequence to bind */
	if (clexec) {
		char tok[NSTRING];
		macarg(tok);	/* get the next token */
		c = prc2kcod(tok);
	} else {
		/* perhaps we only want a single key, not a sequence */
		/* 	(see more comments below) */
		if ((kcmd == &f_cntl_af) || (kcmd == &f_cntl_xf) ||
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
	if (kcmd == &f_cntl_af || kcmd == &f_cntl_xf ||
	    kcmd == &f_unarg || kcmd == &f_esc) {
	    	register CMDFUNC **cfp;
		/* search for an existing binding for the prefix key */
		for (cfp = asciitbl; cfp < &asciitbl[128]; cfp++) {
			if (*cfp == kcmd) {
				(void)unbindchar(cfp - asciitbl);
				break;
			}
		}

		/* reset the appropriate global prefix variable */
		if (kcmd == &f_cntl_af)
			cntl_a = c;
		if (kcmd == &f_cntl_xf)
			cntl_x = c;
		if (kcmd == &f_unarg)
			reptc = c;
		if (kcmd == &f_esc)
			abortc = c;
	}
	
	if ((c & (CTLA|SPEC|CTLX)) == 0) {
		asciitbl[c] = kcmd;
	} else {
		kbp = kbindtbl;
		while (kbp->k_cmd && kbp->k_code != c)
			kbp++;
		if (kbp->k_cmd) { /* found it, change it in place */
			kbp->k_cmd = kcmd;
		} else {
			if (kbp >= &kbindtbl[NBINDS-1]) {
				mlforce("[Prefixed binding table full]");
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

/* ARGSUSED */
int
unbindkey(f, n)
int f, n;	/* command arguments [IGNORED] */
{
	register int c;		/* command key to unbind */
	char outseq[80];	/* output buffer for keystroke sequence */
	char *kcod2prc();

	/* prompt the user to type in a key to unbind */
	mlprompt("Unbind this key sequence: ");

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
		mlforce("[Key not bound]");
		return(FALSE);
	}
	return(TRUE);
}

int
unbindchar(c)
int c;		/* command key to unbind */
{
	register KBIND *kbp;	/* pointer into the command table */
	register KBIND *skbp;	/* saved pointer into the command table */

	if ((c & (CTLA|SPEC|CTLX)) == 0) {
		asciitbl[c] = NULL;
	} else {
		/* search the table to see if the key exists */
		kbp = kbindtbl;
		while (kbp->k_cmd && kbp->k_code != c)
			kbp++;

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
/* ARGSUSED */
int
desbind(f, n)
int f,n;
{
        return liststuff("[Binding List]",makebindlist,1,NULL);
}

#if	APROP
/* ARGSUSED */
int
apro(f, n)	/* Apropos (List functions that match a substring) */
int f,n;
{
	static char mstring[NSTRING];	/* string to match cmd names to */
        register int    s;


	s = mlreply("Apropos string: ", mstring, NSTRING - 1);
	if (s != TRUE)
		return(s);

        return liststuff("[Binding List]",makebindlist,1,mstring);
}
#endif

/* build a binding list (limited or full) */
/* ARGSUSED */
void
makebindlist(dummy, mstring)
int dummy;
char *mstring;		/* match string if partial list, NULL to list all */
{
#if	ST520 & LATTICE
#define	register		
#endif
	register KBIND *kbp;	/* pointer into a key binding table */
	register CMDFUNC **cfp;	/* pointer into the ascii table */
	register NTAB *nptr,*nptr2;	/* pointer into the name table */
	int cpos;		/* current position to use in outseq */
	char outseq[81];	/* output buffer for keystroke sequence */
	int i,pass;
	char *kcod2prc();


	/* let us know this is in progress */
	mlwrite("[Building binding list]");

	/* build the contents of this window, inserting it line by line */
	for (pass = 0; pass < 2; pass++) {
	    for (nptr = nametbl; nptr->n_name != NULL; ++nptr) {

		/* if we've already described this one, move on */
		if (nptr->n_cmd->c_flags & LISTED)
			continue;

		/* try to avoid alphabetizing by the real short names */
		if (pass == 0 && (int)strlen(nptr->n_name) <= 2)
			continue;

		/* add in the command name */
		strcpy(outseq,"\"");
		strcat(outseq, nptr->n_name);
		strcat(outseq,"\"");
		cpos = strlen(outseq);
		while (cpos < 32)
			outseq[cpos++] = ' ';
		outseq[cpos] = 0;
		
#if	APROP
		/* if we are executing an apropos command
		   and current string doesn't include the search string */
		if (mstring && (strinc(outseq, mstring) == FALSE))
		    	continue;
#endif
		/* look in the simple ascii binding table first */
		for(cfp = asciitbl, i = 0; cfp < &asciitbl[128]; cfp++, i++) {
			if (*cfp == nptr->n_cmd) {
				cpos = kcod2prc(i, &outseq[strlen(outseq)]) -
					outseq;
				while(cpos & 7)
					outseq[cpos++] = ' ';
				outseq[cpos] = '\0';
			}
		}
		/* then look in the multi-key table */
		for(kbp = kbindtbl; kbp->k_cmd; kbp++) {
			if (kbp->k_cmd == nptr->n_cmd) {
				cpos = 
				kcod2prc(kbp->k_code, &outseq[strlen(outseq)]) -
					outseq;
				while(cpos & 7)
					outseq[cpos++] = ' ';
				outseq[cpos] = '\0';
			}
		}
		/* dump the line */
		addline(curbp,outseq,-1);

		cpos = 0;

		/* then look for synonyms */
		for (nptr2 = nametbl; nptr2->n_name != NULL; ++nptr2) {
			/* if it's the one we're on, skip */
			if (nptr2 == nptr)
				continue;
			/* if it's already been listed, skip */
			if (nptr2->n_cmd->c_flags & LISTED)
				continue;
			/* if it's not a synonym, skip */
			if (nptr2->n_cmd != nptr->n_cmd)
				continue;
			while (cpos < 8)
				outseq[cpos++] = ' ';
			outseq[cpos] = '\0';
			strcat(outseq,"\"");
			strcat(outseq,nptr2->n_name);
			strcat(outseq,"\"");
			addline(curbp,outseq,-1);
			cpos = 0;	/* and clear the line */

		}

		nptr->n_cmd->c_flags |= LISTED; /* mark it as already listed */
	    }
	}

	for (nptr = nametbl; nptr->n_name != NULL; ++nptr)
		nptr->n_cmd->c_flags &= ~LISTED; /* mark it as unlisted */

	mlwrite("");	/* clear the message line */
}

#if	APROP
int
strinc(sourc, sub)	/* does source include sub? */
char *sourc;	/* string to search in */
char *sub;	/* substring to look for */
{
	char *sp;	/* ptr into source */
	char *nxtsp;	/* next ptr into source */
	char *tp;	/* ptr into substring */

	/* for each character in the source string */
	sp = sourc;
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

int
startup(sfname)
char *sfname;	/* name of startup file  */
{
	char *fname;	/* resulting file name to execute */

	/* look up the startup file */
	fname = flook(sfname, FL_HERE_HOME);

	/* if it isn't around, don't sweat it */
	if (fname == NULL) {
		mlforce("[Can't find startup file %s]",sfname);
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
char *
kcod2prc(c, seq)
int c;		/* sequence to translate */
char *seq;	/* destination string for sequence */
{
	char *ptr;	/* pointer into current position in sequence */

	ptr = seq;

	/* apply cntl_a sequence if needed */
	if (c & CTLA) {
		*ptr++ = '^';
		*ptr++ = 'A';
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
	return ptr;
}


/* kcod2fnc:  translate a 10-bit keycode to a function pointer */
/*	(look a key binding up in the binding table)		*/
CMDFUNC *
kcod2fnc(c)
int c;	/* key to find what is bound to it */
{
	register KBIND *kbp;

	if ((c & (CTLA|SPEC|CTLX)) == 0) {
		return asciitbl[c];
	} else {
		kbp = kbindtbl;
		while (kbp->k_cmd && kbp->k_code != c)
			kbp++;
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

/* fnc2key: translate a function pointer to a simple key that is bound
		to that function
*/

int
fnc2key(cfp)
CMDFUNC *cfp;	/* ptr to the requested function to bind to */
{
	register int i;

	for(i = 0; i < 128; i++) {
		if (cfp == asciitbl[i])
			return i;
	}
	return -1;
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

	/* first, the CTLA prefix */
	if (*k == '^' && *(k+1) == toalpha(cntl_a) && *(k+2) == '-') {
		c = CTLA;
		k += 3;
	}

	/* next the function prefix */
	if (*k == 'F' && *(k+1) == 'N' && *(k+2) == '-') {
		c |= SPEC;
		k += 3;
	}

	/* control-x as well... (but not with FN) */
	if (*k == '^' && *(k+1) == toalpha(cntl_x) && 
				*(k+2) == '-' && !(c & SPEC)) {
		c |= CTLX;
		k += 3;
	}

	/* a control char? */
	if (*k == '^' && *(k+1) != 0) {
		++k;
		c |= *k;
		if (islower(c)) c = toupper(c);
		c = tocntrl(c);
	} else if (!strcmp(k,"<sp>")) {
		c |= ' ';
	} else if (!strcmp(k,"<tab>")) {
		c |= '\t';
	} else {
		c |= *k;
	}
	return c;
}

#if ! SMALLER

/* translate printable code (like "M-r") to english command name */
char *
prc2engl(skey)	/* string key name to binding name.... */
char *skey;	/* name of key to get binding for */
{
	char *bindname;

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
	int status;
	char *buf;


	status = kbd_engl_stat(&buf);
	if (status == SORTOFTRUE) /* something was tungetc'ed */
		(void)tgetc();
	if (status == TRUE)
		return buf;
	return NULL;
}

/* *bufp only valid if return is TRUE */
int
kbd_engl_stat(bufp)
char **bufp;
{
#if	ST520 & LATTICE
#define register		
#endif
	register int c;
	register int cpos;	/* current column on screen output */
	register char *sp;	/* pointer to string for output */
	register NTAB *nbp;	/* first ptr to entry in name binding table */
	register NTAB *cnbp;	/* current ptr to entry in name binding table */
	register NTAB *lnbp;	/* last ptr to entry in name binding table */
	static char buf[NLINE]; /* buffer to hold tentative command name */

	cpos = 0;

	*bufp = NULL;

	/* if we are executing a command line just get the next arg and return */
	if (clexec) {
		if (macarg(buf) != TRUE) {
			return FALSE;
		}
		*bufp = buf;
		return TRUE;
	}

	lineinput = TRUE;

	/* build a name string from the keyboard */
	while (TRUE) {
		c = tgetc();

		if (cpos == 0) {
			if (isbackspace(c) ||
			    c == kcod2key(abortc) ||
			    c == kcod2key(killc) ||
			    c == '\r' ||
			    c == '\n' ||
			    islinespecchar(c) ) {
				tungetc(c);
				return SORTOFTRUE;
			}
		}

		if (c == '\r' || c == '\n') {
			buf[cpos] = 0;
			lineinput = FALSE;
			*bufp = buf;
			return TRUE;

		} else if (c == kcod2key(abortc)) {	/* Bell, abort */
			buf[0] = '\0';
			lineinput = FALSE;
			return FALSE;

		} else if (isbackspace(c)) {
			TTputc('\b');
			TTputc(' ');
			TTputc('\b');
			--ttcol;
			--cpos;
			TTflush();

		} else if (c == kcod2key(killc)) {	/* ^U, kill */
			while (cpos != 0) {
				TTputc('\b');
				TTputc(' ');
				TTputc('\b');
				--cpos;
				--ttcol;
			}
			TTflush();
			tungetc(c);
			return SORTOFTRUE;

		} /* else... */
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
		else if (c == ' ' || (cpos > 0 && cpos < 3 &&
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
				/* return nbp->n_name; */
				strncpy(buf,nbp->n_name,NLINE);
				*bufp = buf;
				return TRUE;
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
			if (cpos < NLINE-1 && isprint(c)) {
				buf[cpos++] = c;
				TTputc(c);
			}

			++ttcol;
			TTflush();
		}
	}
}

