/*	This file is for functions dealing with execution of
 *	commands, command lines, buffers, files and startup files
 *
 *	written 1986 by Daniel Lawrence	
 *
 * $Log: exec.c,v $
 * Revision 1.14  1991/10/08 01:30:00  pgf
 * added new bp arg to lfree and lalloc
 *
 * Revision 1.13  1991/08/07  12:35:07  pgf
 * added RCS log messages
 *
 * revision 1.12
 * date: 1991/08/06 15:20:45;
 * global/local values
 * 
 * revision 1.11
 * date: 1991/06/25 19:52:27;
 * massive data structure restructure
 * 
 * revision 1.10
 * date: 1991/06/20 17:22:02;
 * changed comments -- ! is now ~ ???
 * 
 * revision 1.9
 * date: 1991/06/13 15:17:46;
 * added globbing and uniq'ifying to execfile names
 * 
 * revision 1.8
 * date: 1991/06/07 13:39:06;
 * cleanup
 * 
 * revision 1.7
 * date: 1991/06/03 15:53:46;
 * initialize fulllineregions and havemotion in dobuf()
 * 
 * revision 1.6
 * date: 1991/06/03 12:17:32;
 * ifdef'ed GLOBALS for unresolved ref
 * 
 * revision 1.5
 * date: 1991/06/03 10:20:18;
 * cleanup, and
 * namedcmd now calls execute() directly, allowing
 * removal of cfp arg to docmd
 * 
 * revision 1.4
 * date: 1991/05/31 10:53:21;
 * new namedcmd(), linespec(), and rangespec() routines to support
 * line ranges on ex commands.  mostly borrowed from kirkendall's elvis
 * 
 * revision 1.3
 * date: 1991/04/08 15:46:08;
 * fixed readin() arg count
 * 
 * revision 1.2
 * date: 1990/12/06 19:49:44;
 * turn off command display during file exec
 * 
 * revision 1.1
 * date: 1990/09/21 10:25:14;
 * initial vile RCS revision
*/

#include	<stdio.h>
#include	"estruct.h"
#include	"edef.h"

/* namedcmd:	execute a named command even if it is not bound */

namedcmd(f, n)
int f, n;
{
	char *fnp;	/* ptr to the name of the cmd to exec */
	LINE *fromline;	/* first linespec */
	LINE *toline;	/* second linespec */
	char lspec[NLINE];
	int cpos = 0;
	int s,c,isdfl,zero,flags;
	char *kbd_engl();
	CMDFUNC *cfp;		/* function to execute */
	extern CMDFUNC f_gomark;

	/* prompt the user to type a named command */
	mlwrite(": ");

	/* and now get the function name to execute */
#if	NeWS
	newsimmediateon() ;
#endif

	while(1) {
		c = tgetc();
		if (c == '\r' || c == '\n') {
			lspec[cpos] = 0;
			fnp = NULL;
			break;
		} else if (c == kcod2key(abortc)) {	/* Bell, abort */
		isabortc:
			lspec[0] = '\0';
			return FALSE;
		} else if (isbackspace(c)) {
			if (cpos != 0) {
				TTputc('\b');
				TTputc(' ');
				TTputc('\b');
				--ttcol;
				--cpos;
			} else {
				lspec[0] = '\0';
				lineinput = FALSE;
				return FALSE;
			}

		} else if (c == kcod2key(killc)) {	/* ^U, kill */
		iskillc:
			while (cpos != 0) {
				TTputc('\b');
				TTputc(' ');
				TTputc('\b');
				--cpos;
				--ttcol;
			}
		} else if (islinespecchar(c) ||
			/* special test for 'a style mark references */
				(cpos > 0 &&
				 lspec[cpos-1] == '\'' &&
				 (islower(c) || (c == '\'') )
				)
			 ) {
			lspec[cpos++] = c;
			TTputc(c);
			++ttcol;
		} else {
			int status;
			tungetc(c);
			lspec[cpos] = 0;
			status = kbd_engl_stat(&fnp);
			if (status == TRUE) {
				break;
			} else if (status == SORTOFTRUE) {
				fnp = NULL;
				continue;
			} else {
				return status;
			}
		}
		TTflush();
	}

	/* parse the accumulated lspec */
	if (rangespec(lspec,&fromline,&toline,&isdfl,&zero) != TRUE) {
		mlwrite("[Improper line range]");
		return FALSE;
	}

	/* if range given, and it wasn't "0" and the buffer's empty */
	if (!isdfl && !zero && is_empty_buf(curbp)) {
		mlwrite("[No range possible in empty buffer]", fnp);
		return FALSE;
	}

#if	NeWS
	newsimmediateoff() ;
#endif

	/* did we get a name? */
	if (fnp == NULL) {
		if (isdfl) { /* no range, no function */
			mlwrite("[No such function]");
			return FALSE;
		} else { /* range, no function */
			cfp = &f_gomark;
			fnp = "";
		}
	} else if ((cfp = engl2fnc(fnp)) == NULL) { /* bad function */
		mlwrite("[No such function %s]",fnp);
		return FALSE;
	}
	flags = cfp->c_flags;
	
	/* bad arguments? */
#ifdef EXRC_FILES
seems like we need one more check here -- is it from a .exrc file?
	    cmd not ok in .exrc 		empty file
	if (!(flags & EXRCOK) && is_empty_buf(curbp)) {
		mlwrite("[Can't use the \"%s\" command in a %s file.]", 
					cmdnames[cmdidx].name, EXRC);
		return FALSE;
	}
#endif

	/* was: if (!(flags & (ZERO | EXRCOK)) && fromline == NULL ) { */
	if (zero) {
		extern CMDFUNC f_lineputafter, f_opendown, f_insfile;
		extern CMDFUNC f_lineputbefore, f_openup;
		if (!(flags & ZERO)) {
			mlwrite("[Can't use address 0 with \"%s\" command]", fnp);
			return FALSE;
		}
		/*  we're positioned at fromline == curbp->b_linep, so commands
			must be willing to go _down_ from there.  Seems easiest
			to special case the commands that prefer going up */
		if (cfp == &f_insfile) {
			/* works okay -- acts down normally */
		} else if (cfp == &f_lineputafter) {
			cfp = &f_lineputbefore;
			fromline = lforw(fromline);
		} else if (cfp == &f_opendown) {
			cfp = &f_openup;
			fromline = lforw(fromline);
		} else {
			mlwrite("[Configuration error: ZERO]");
			return FALSE;
		}
		flags = cfp->c_flags;
		toline = fromline;
	}

	/* if we're not supposed to have a line no., and the line no. isn't
		the current line, and there's more than one line */
	if (!(flags & FROM) && fromline != curwp->w_dot.l &&
			!is_empty_buf(curbp) &&
		  (lforw(lforw(curbp->b_line.l)) != curbp->b_line.l) ) {
		mlwrite("[Can't use address with \"%s\" command.]", fnp);
		return FALSE;
	}
	/* if we're not supposed to have a second line no., and the line no. 
		isn't the same as the first line no., and there's more than
		one line */
	if (!(flags & TO) && toline != fromline &&
			!is_empty_buf(curbp) &&
		  (lforw(lforw(curbp->b_line.l)) != curbp->b_line.l) ) {
		mlwrite("[Can't use a range with \"%s\" command.]", fnp);
		return FALSE;
	}
#ifdef NEEDED
	if (!(flags & EXTRA) && *scan) {
		mlwrite("[Extra characters after \"%s\" command.]", 
						cmdnames[cmdidx].name);
		return FALSE;
	}
#endif
#ifdef NEEDED
	if ((flags & NOSPC) && !(cmd == CMD_READ && (forceit || *scan == '!'))) {
		build = scan;
#ifndef CRUNCH  /* what is this for? -pgf */
		if ((flags & PLUS) && *build == '+') {
			while (*build && !(isspace(*build))) {
				build++;
			}
			while (*build && isspace(*build)) {
				build++;
			}
		}
#endif /* not CRUNCH */
		for (; *build; build++) {
			if (isspace(*build)) {
				mlwrite("[Too many %s to \"%s\" command.]",
					(flags & XFILE) ? "filenames" : "arguments",
					cmdnames[cmdidx].name);
				return FALSE;
			}
		}
	}
#endif /* NEEDED */

	/* some commands have special default ranges */
	if (isdfl) {
		if (flags & DFLALL) {
			extern CMDFUNC f_operwrite, f_filewrite, f_operglobals,
					f_globals, f_opervglobals, f_vglobals;
			if (cfp == &f_operwrite) {
				cfp = &f_filewrite;
#if GLOBALS
			} else if (cfp == &f_operglobals) {
				cfp = &f_globals;
			} else if (cfp == &f_opervglobals) {
				cfp = &f_vglobals;
#endif
			} else {
				mlwrite("[Configuration error: DFLALL]");
				return FALSE;
			}
		} else if (flags & DFLNONE) {
			extern CMDFUNC f_operfilter, f_spawn;
			if (cfp == &f_operfilter) {
				cfp = &f_spawn;
				setmark();  /* not that it matters */
			} else {
				mlwrite("[Configuration error: DFLNONE]");
				return FALSE;
			}
			fromline = toline = NULL;
		}
	}

#ifdef NEEDED
	/* write a newline if called from visual mode */
	if ((flags & NL) && !exmode /* && !exwrote */) {
		TTputc('\n');
		/* exrefresh(); */
	}
#endif

	if (toline || fromline) {  /* assume it's an absolute motion */
				   /* we could probably do better */
		curwp->w_lastdot = curwp->w_dot;
	}
	if (toline) {
		curwp->w_dot.l = toline;
		firstnonwhite();
		setmark();
	}
	if (fromline) {
		curwp->w_dot.l = fromline;
		firstnonwhite();
		if (!toline)
			setmark();
	}

	/* and then execute the command */
	isnamedcmd = TRUE;
	havemotion = &f_gomark;
	fulllineregions = TRUE;

	s = execute(cfp,f,n);

	havemotion = NULL;
	isnamedcmd = FALSE;
	fulllineregions = FALSE;

	return s;
}

/* parse an ex-style line spec -- code culled from elvis, file ex.c, by
	Steve Kirkendall
*/
char *
linespec(s, markptr)
register char	*s;		/* start of the line specifier */
LINE		**markptr;	/* where to store the mark's value */
{
	long		num;
	LINE		*lp;	/* where the linespec takes us */
	register char	*t;
	int		status;

	setmark();
	lp = NULL;

	/* parse each ;-delimited clause of this linespec */
	do
	{
		/* skip an initial ';', if any */
		if (*s == ';')
			s++;

		/* skip leading spaces */
		while (isspace(*s))
			s++;
	
		/* dot means current position */
		if (*s == '.') {
			s++;
			lp = curwp->w_dot.l;
		} else if (*s == '$') { /* '$' means the last line */
			s++;
			status = gotoeob(TRUE,1);
			if (status) lp = curwp->w_dot.l;
		} else if (isdigit(*s)) {
			/* digit means an absolute line number */
			for (num = 0; isdigit(*s); s++) {
				num = num * 10 + *s - '0';
			}
			status = gotoline(TRUE,num);
			if (status) lp = curwp->w_dot.l;
		} else if (*s == '\'') {
			/* appostrophe means go to a set mark */
			s++;
			status = gonmmark(*s);
			if (status) lp = curwp->w_dot.l;
			s++;
		} 
#if PATTERNS
		else if (*s == '/' || *s == '?') { /* slash means do a search */
			/* put a '\0' at the end of the search pattern */
			t = parseptrn(s);
	
			/* search for the pattern */
			lp &= ~(BLKSIZE - 1);
			if (*s == '/') {
				pfetch(markline(lp));
				if (plen > 0)
					lp += plen - 1;
				lp = m_fsrch(lp, s);
			} else {
				lp = m_bsrch(lp, s);
			}
	
			/* adjust command string pointer */
			s = t;
		}
#endif
	
		/* if linespec was faulty, quit now */
		if (!lp) {
			*markptr = lp;
			swapmark();
			return s;
		}
	
		/* maybe add an offset */
		t = s;
		if (*t == '-' || *t == '+') {
			s++;
			for (num = 0; *s >= '0' && *s <= '9'; s++) {
				num = num * 10 + *s - '0';
			}
			if (num == 0)
				num = 1;
			forwline(TRUE, (*t == '+') ? num : -num);
			lp = curwp->w_dot.l;
		}
	} while (*s == ';' || *s == '+' || *s == '-');

	*markptr = lp;
	swapmark();
	return s;
}

/* parse an ex-style line range -- code culled from elvis, file ex.c, by
	Steve Kirkendall
*/
rangespec(specp,fromlinep,tolinep,isdefaultp,zerop)
char		*specp;	/* string containing a line range */
LINE		**fromlinep;	/* first linespec */
LINE		**tolinep;	/* second linespec */
int		*isdefaultp;
int		*zerop;
{
	register char	*scan;		/* used to scan thru specp */
	LINE		*fromline;	/* first linespec */
	LINE		*toline;	/* second linespec */
	int		noaddrallowed;

	*zerop = FALSE;

	/* ignore command lines that start with a double-quote */
	if (*specp == '"') {
		*fromlinep = *tolinep = curwp->w_dot.l;
		return TRUE;
	}

	/* permit extra colons at the start of the line */
	while (isspace(*specp) || *specp == ':') {
		specp++;
	}

	/* parse the line specifier */
	scan = specp;
	if (is_empty_buf(curbp)) {
		fromline = toline = NULL;
	} else if (*scan == '%') {
		/* '%' means all lines */
		fromline = lforw(curbp->b_line.l);
		toline = lback(curbp->b_line.l);
		scan++;
	} else if (*scan == '0') {
		fromline = toline = curbp->b_line.l; /* _very_ top of buffer */
		*zerop = TRUE;
		scan++;
	} else {
		scan = linespec(scan, &fromline);
		if (!fromline)
			fromline = curwp->w_dot.l;
		toline = fromline;
		if (*scan == ',') {
			scan++;
			scan = linespec(scan, &toline);
		}
		if (!toline) {
			/* faulty line spec -- fault already described */
			dbgwrite("null toline");
			return FALSE;
		}
	}

	*isdefaultp = (scan == specp);

	/* skip whitespace */
	while (isspace(*scan))
		scan++;

	if (*scan) {
		dbgwrite("crud at end %s",specp);
		return FALSE;
	}

	*fromlinep = fromline;
	*tolinep = toline;
	
	return TRUE;
}

/* old namedcmd:	execute a named command even if it is not bound */
onamedcmd(f, n)
int f, n;	/* command arguments [passed through to command executed] */
{
	register char *fnp;	/* ptr to the name of the cmd to exec */
	char *kbd_engl();
	int s;

	/* prompt the user to type a named command */
	mlwrite(": ");

	/* and now get the function name to execute */
#if	NeWS
	newsimmediateon() ;
#endif

	fnp = kbd_engl();

#if	NeWS
	newsimmediateoff() ;
#endif

	if (fnp == NULL) {
		mlwrite("[No such function]");
		return FALSE;
	}

	/* and then execute the command */
	isnamedcmd = TRUE;
	s = docmd(fnp,FALSE,f,n);
	isnamedcmd = FALSE;

	return s;
}

#if NEVER
/*	execcmd:	Execute a command line command by name alone */
execcmd(f, n)
int f, n;	/* default Flag and Numeric argument */
{
	register int status;		/* status return */
	char cmdbuf[NSTRING];		/* string holding command to execute */

	/* get the line wanted */
	cmdbuf[0] = 0;
	if ((status = mlreply("cmd: ", cmdbuf, NSTRING)) != TRUE)
		return status;

	execlevel = 0;
	return docmd(cmdbuf,TRUE,f,n);
}
#endif

/*	docmd:	take a passed string as a command line and translate
		it to be executed as a command. This function will be
		used by execute-command-line and by all source and
		startup files.

	format of the command line is:

		{# arg} <command-name> {<argument string(s)>}

*/

docmd(cline,newcle,f,n)
char *cline;	/* command line to execute */
{
	int status;		/* return status of function */
	int flags;		/* function flags */
	int oldcle;		/* old contents of clexec flag */
	char *oldestr;		/* original exec string */
	char tkn[NSTRING];	/* next token off of command line */
	CMDFUNC *cfp;
	extern CMDFUNC f_godotplus;
		
	/* if we are scanning and not executing..go back here */
	if (execlevel)
		return TRUE;

	oldestr = execstr;	/* save last ptr to string to execute */
	execstr = cline;	/* and set this one as current */

	/* first set up the default command values */
	if (newcle == TRUE) {
		f = FALSE;
		n = 1;
	}

	if ((status = macarg(tkn)) != TRUE) {	/* and grab the first token */
		execstr = oldestr;
		return status;
	}

	/* process leadin argument */
	if (toktyp(tkn) != TKCMD) {
		f = TRUE;
		strcpy(tkn, tokval(tkn));
		n = atoi(tkn);

		/* and now get the command to execute */
		if ((status = macarg(tkn)) != TRUE) {
			execstr = oldestr;
			return status;	
		}	
	}

	/* and match the token to see if it exists */
	if ((cfp = engl2fnc(tkn)) == NULL) {
		mlwrite("[No such function %s]",tkn);
		execstr = oldestr;
		return FALSE;
	}
	
	/* save the arguments and go execute the command */
	oldcle = clexec;		/* save old clexec flag */
	clexec = newcle;		/* in cline execution */
	status = execute(cfp,f,n);
	cmdstatus = status;		/* save the status */
	clexec = oldcle;		/* restore clexec flag */
	execstr = oldestr;
	return status;
}

/* token:	chop a token off a string
		return a pointer past the token
*/

char *
token(src, tok)
char *src, *tok;	/* source string, destination token string */
{
	register int quotef;	/* is the current string quoted? */

	/* first scan past any whitespace in the source string */
	while (isspace(*src))
		++src;

	/* scan through the source string */
	quotef = FALSE;
	while (*src) {
		/* process special characters */
		if (*src == '\\') {
			++src;
			if (*src == 0)
				break;
			switch (*src++) {
				case 'r':	*tok++ = '\r'; break;
				case 'n':	*tok++ = '\n'; break;
				case 't':	*tok++ = '\t';  break;
				case 'b':	*tok++ = '\b';  break;
				case 'f':	*tok++ = '\f'; break;
				default:	*tok++ = *(src-1);
			}
		} else {
			/* check for the end of the token */
			if (quotef) {
				if (*src == '"')
					break;
			} else {
				if (*src == ' ' || *src == '\t')
					break;
			}

			/* set quote mode if qoute found */
			if (*src == '"')
				quotef = TRUE;

			/* record the character */
			*tok++ = *src++;
		}
	}

	/* terminate the token and exit */
	if (*src)
		++src;
	*tok = 0;
	return src;
}

macarg(tok)	/* get a macro line argument */
char *tok;	/* buffer to place argument */
{
	int savcle;	/* buffer to store original clexec */

	savcle = clexec;	/* save execution mode */
	clexec = TRUE;		/* get the argument */
	/* grab token and advance past */
	execstr = token(execstr, tok);
	/* evaluate it */
	strcpy(tok, tokval(tok));
	clexec = savcle;	/* restore execution mode */
	return TRUE;
}

/*	nextarg:	get the next argument	*/

nextarg(buffer)
char *buffer;		/* buffer to put token into */
{
	/* grab token and advance past */
	execstr = token(execstr, buffer);
	/* evaluate it */
	strcpy(buffer, tokval(buffer));
	return TRUE;
}

/*	storemac:	Set up a macro buffer and flag to store all
			executed command lines there			*/

storemac(f, n)
int f;		/* default flag */
int n;		/* macro number to use */
{
	register struct BUFFER *bp;	/* pointer to macro buffer */
	char bname[NBUFN];		/* name of buffer to use */

	/* must have a numeric argument to this function */
	if (f == FALSE) {
		mlwrite("No macro specified");
		return FALSE;
	}

	/* range check the macro number */
	if (n < 1 || n > 40) {
		mlwrite("[Macro number out of range]");
		return FALSE;
	}

	/* construct the macro buffer name */
	strcpy(bname, "[Macro xx]");
	bname[7] = '0' + (n / 10);
	bname[8] = '0' + (n % 10);

	/* set up the new macro buffer */
	if ((bp = bfind(bname, OK_CREAT, BFINVS)) == NULL) {
		mlwrite("[Cannot create macro]");
		return FALSE;
	}

	/* and make sure it is empty */
	bclear(bp);

	/* and set the macro store pointers to it */
	mstore = TRUE;
	bstore = bp;
	return TRUE;
}

#if	PROC
/*	storeproc:	Set up a procedure buffer and flag to store all
			executed command lines there			*/

storeproc(f, n)
int f;		/* default flag */
int n;		/* macro number to use */
{
	register struct BUFFER *bp;	/* pointer to macro buffer */
	register int status;		/* return status */
	char bname[NBUFN];		/* name of buffer to use */

	/* a numeric argument means its a numbered macro */
	if (f == TRUE)
		return storemac(f, n);

	/* get the name of the procedure */
	bname[1] = 0;
        if ((status = mlreply("Procedure name: ", &bname[1], NBUFN-2)) != TRUE)
                return status;

	/* construct the macro buffer name */
	bname[0] = '[';
	strcat(bname, "]");

	/* set up the new macro buffer */
	if ((bp = bfind(bname, OK_CREAT, BFINVS)) == NULL) {
		mlwrite("[Can not create macro]");
		return FALSE;
	}

	/* and make sure it is empty */
	bclear(bp);

	/* and set the macro store pointers to it */
	mstore = TRUE;
	bstore = bp;
	return TRUE;
}

/*	execproc:	Execute a procedure				*/

execproc(f, n)
int f, n;	/* default flag and numeric arg */
{
        register BUFFER *bp;		/* ptr to buffer to execute */
        register int status;		/* status return */
        static char obufn[NBUFN+2];		/* name of buffer to execute */
        char bufn[NBUFN+2];		/* name of buffer to execute */

	/* find out what buffer the user wants to execute */
        if ((status = mlreply("Execute procedure: ", obufn, NBUFN)) != TRUE)
                return status;

	/* construct the buffer name */
	bufn[0] = '[';
	strcat(bufn, obufn);
	strcat(bufn, "]");

	/* find the pointer to that buffer */
        if ((bp=bfind(bufn, NO_CREAT, 0)) == NULL) {
		mlwrite("[No such procedure]");
                return FALSE;
        }

	/* and now execute it as asked */
	while (n-- > 0) {
		if ((status = dobuf(bp)) != TRUE)
			return status;
	}
	return TRUE;
}
#endif

#if ! SMALLER
/*	execbuf:	Execute the contents of a buffer of commands	*/

execbuf(f, n)
int f, n;	/* default flag and numeric arg */
{
        register BUFFER *bp;		/* ptr to buffer to execute */
        register int status;		/* status return */
        static char bufn[NSTRING];		/* name of buffer to execute */

	/* find out what buffer the user wants to execute */
        if ((status = mlreply("Execute buffer: ", bufn, NBUFN)) != TRUE)
                return status;

	/* find the pointer to that buffer */
        if ((bp=bfind(bufn, NO_CREAT, 0)) == NULL) {
		mlwrite("No such buffer");
                return FALSE;
        }

	/* and now execute it as asked */
	while (n-- > 0) {
		if ((status = dobuf(bp)) != TRUE)
			return status;
	}
	return TRUE;
}
#endif

/*	dobuf:	execute the contents of the buffer pointed to
		by the passed BP

	Directives start with a "~" and include:

#if SMALLER
	~endm		End a macro
#else
	~endm		End a macro
	~if (cond)	conditional execution
	~else
	~endif
	~return		Return (terminating current macro)
	~goto <label>	Jump to a label in the current macro
	~force		Force macro to continue...even if command fails
	~while (cond)	Execute a loop if the condition is true
	~endwhile
	
	Line Labels begin with a "*" as the first nonblank char, like:

	*LBL01
#endif

*/

dobuf(bp)
BUFFER *bp;	/* buffer to execute */
{
        register int status;	/* status return */
	register LINE *lp;	/* pointer to line to execute */
	register LINE *hlp;	/* pointer to line header */
	LINE *mp;		/* Macro line storage temp */
	int dirnum;		/* directive index */
	int linlen;		/* length of line to execute */
	int i;			/* index */
	int force;		/* force TRUE result? */
	WINDOW *wp;		/* ptr to windows to scan */
	WHBLOCK *whlist;	/* ptr to !WHILE list */
	char *einit;		/* initial value of eline */
	char *eline;		/* text of line to execute */
#if ! SMALLER
	WHBLOCK *scanner;	/* ptr during scan */
	register LINE *glp;	/* line to goto */
	WHBLOCK *whtemp;	/* temporary ptr to a WHBLOCK */
	char tkn[NSTRING];	/* buffer to evaluate an expresion in */
#endif

#if	DEBUGM
	char *sp;			/* temp for building debug string */
	register char *ep;	/* ptr to end of outline */
#endif
	static dobufnesting;

	if (++dobufnesting > 9) {
		dobufnesting--;
		return FALSE;
	}
		

	/* clear IF level flags/while ptr */
	execlevel = 0;
	whlist = NULL;
	fulllineregions = FALSE;
	havemotion = FALSE;

#if ! SMALLER
	scanner = NULL;
	/* scan the buffer to execute, building WHILE header blocks */
	hlp = bp->b_line.l;
	lp = hlp->l_fp;
	while (lp != hlp) {
		/* scan the current line */
		eline = lp->l_text;
		i = lp->l_used;

		/* trim leading whitespace */
		while (i-- > 0 && (*eline == ' ' || *eline == '\t'))
			++eline;

		/* if theres nothing here, don't bother */
		if (i <= 0)
			goto nxtscan;

		/* if is a while directive, make a block... */
		if (eline[0] == '!' && eline[1] == 'w' && eline[2] == 'h') {
			whtemp = (WHBLOCK *)malloc(sizeof(WHBLOCK));
			if (whtemp == NULL) {
noram:				mlwrite("%%Out of memory during while scan");
failexit:			freewhile(scanner);
				freewhile(whlist);
				mstore = FALSE;
				dobufnesting--;
				return FALSE;
			}
			whtemp->w_begin = lp;
			whtemp->w_type = BTWHILE;
			whtemp->w_next = scanner;
			scanner = whtemp;
		}

		/* if is a BREAK directive, make a block... */
		if (eline[0] == '!' && eline[1] == 'b' && eline[2] == 'r') {
			if (scanner == NULL) {
				mlwrite("%%!BREAK outside of any !WHILE loop");
				goto failexit;
			}
			whtemp = (WHBLOCK *)malloc(sizeof(WHBLOCK));
			if (whtemp == NULL)
				goto noram;
			whtemp->w_begin = lp;
			whtemp->w_type = BTBREAK;
			whtemp->w_next = scanner;
			scanner = whtemp;
		}

		/* if it is an endwhile directive, record the spot... */
		if (eline[0] == '!' && strncmp(&eline[1], "endw", 4) == 0) {
			if (scanner == NULL) {
				mlwrite("%%!ENDWHILE with no preceding !WHILE in '%s'",
					bp->b_bname);
				goto failexit;
			}
			/* move top records from the scanner list to the
			   whlist until we have moved all BREAK records
			   and one WHILE record */
			do {
				scanner->w_end = lp;
				whtemp = whlist;
				whlist = scanner;
				scanner = scanner->w_next;
				whlist->w_next = whtemp;
			} while (whlist->w_type == BTBREAK);
		}

nxtscan:	/* on to the next line */
		lp = lp->l_fp;
	}

	/* while and endwhile should match! */
	if (scanner != NULL) {
		mlwrite("%%!WHILE with no matching !ENDWHILE in '%s'",
			bp->b_bname);
		goto failexit;
	}
#endif

	/* starting at the beginning of the buffer */
	hlp = bp->b_line.l;
	lp = hlp->l_fp;
	while (lp != hlp) {
		/* allocate eline and copy macro line to it */
		linlen = lp->l_used;
		if ((einit = eline = malloc(linlen+1)) == NULL) {
			mlwrite("%%Out of Memory during macro execution");
			freewhile(whlist);
			mstore = FALSE;
			dobufnesting--;
			return FALSE;
		}
		strncpy(eline, lp->l_text, linlen);
		eline[linlen] = 0;	/* make sure it ends */

		/* trim leading whitespace */
		while (*eline == ' ' || *eline == '\t')
			++eline;

		/* dump comments and blank lines */
		if (*eline == ';' || *eline == 0)
			goto onward;

#if	DEBUGM
		/* if $debug == TRUE, every line to execute
		   gets echoed and a key needs to be pressed to continue
		   ^G will abort the command */
	
		if (macbug) {
			strcpy(outline, "<<<");
	
			/* debug macro name */
			strcat(outline, bp->b_bname);
			strcat(outline, ":");
	
			/* debug if levels */
			strcat(outline, itoa(execlevel));
			strcat(outline, ":");

			/* and lastly the line */
			strcat(outline, eline);
			strcat(outline, ">>>");
	
			/* change all '%' to ':' so mlwrite won't expect arguments */
			sp = outline;
			while (*sp)
			if (*sp++ == '%') {
				/* advance to the end */
				ep = --sp;
				while (*ep++)
					;
				/* null terminate the string one out */
				*(ep + 1) = 0;
				/* copy backwards */
				while(ep-- > sp)
					*(ep + 1) = *ep;

				/* and advance sp past the new % */
				sp += 2;					
			}
	
			/* write out the debug line */
			mlforce(outline);
			update(TRUE);
	
			/* and get the keystroke */
			if (kbd_key() == abortc) {
				mlforce("[Macro aborted]");
				freewhile(whlist);
				mstore = FALSE;
				dobufnesting--;
				return FALSE;
			}
		}
#endif

		/* Parse directives here.... */
		dirnum = -1;
		if (*eline == '~') {
			/* Find out which directive this is */
			++eline;
			for (dirnum = 0; dirnum < NUMDIRS; dirnum++)
				if (strncmp(eline, dname[dirnum],
				            strlen(dname[dirnum])) == 0)
					break;

			/* and bitch if it's illegal */
			if (dirnum == NUMDIRS) {
				mlwrite("%%Unknown Directive");
				freewhile(whlist);
				mstore = FALSE;
				dobufnesting--;
				return FALSE;
			}

			/* service only the !ENDM macro here */
			if (dirnum == DENDM) {
				mstore = FALSE;
				bstore = NULL;
				goto onward;
			}

			/* restore the original eline....*/
			--eline;
		}

		/* if macro store is on, just salt this away */
		if (mstore) {
			/* allocate the space for the line */
			linlen = strlen(eline);
			if ((mp=lalloc(linlen,bstore)) == NULL) {
				mlwrite("%%Out of memory while storing macro");
				mstore = FALSE;
				dobufnesting--;
				return FALSE;
			}
	
			/* copy the text into the new line */
			for (i=0; i<linlen; ++i)
				lputc(mp, i, eline[i]);
	
			/* attach the line to the end of the buffer */
	       		bstore->b_line.l->l_bp->l_fp = mp;
			mp->l_bp = bstore->b_line.l->l_bp;
			bstore->b_line.l->l_bp = mp;
			mp->l_fp = bstore->b_line.l;
			goto onward;
		}
	

		force = FALSE;

		/* dump comments */
		if (*eline == '*')
			goto onward;

#if ! SMALLER
		/* now, execute directives */
		if (dirnum != -1) {
			/* skip past the directive */
			while (*eline && *eline != ' ' && *eline != '\t')
				++eline;
			execstr = eline;

			switch (dirnum) {
			case DIF:	/* IF directive */
				/* grab the value of the logical exp */
				if (execlevel == 0) {
					if (macarg(tkn) != TRUE)
						goto eexec;
					if (stol(tkn) == FALSE)
						++execlevel;
				} else
					++execlevel;
				goto onward;

			case DWHILE:	/* WHILE directive */
				/* grab the value of the logical exp */
				if (execlevel == 0) {
					if (macarg(tkn) != TRUE)
						goto eexec;
					if (stol(tkn) == TRUE)
						goto onward;
				}
				/* drop down and act just like !BREAK */

			case DBREAK:	/* BREAK directive */
				if (dirnum == DBREAK && execlevel)
					goto onward;

				/* jump down to the endwhile */
				/* find the right while loop */
				whtemp = whlist;
				while (whtemp) {
					if (whtemp->w_begin == lp)
						break;
					whtemp = whtemp->w_next;
				}
			
				if (whtemp == NULL) {
					mlwrite("%%Internal While loop error");
					freewhile(whlist);
					mstore = FALSE;
					dobufnesting--;
					return FALSE;
				}
			
				/* reset the line pointer back.. */
				lp = whtemp->w_end;
				goto onward;

			case DELSE:	/* ELSE directive */
				if (execlevel == 1)
					--execlevel;
				else if (execlevel == 0 )
					++execlevel;
				goto onward;

			case DENDIF:	/* ENDIF directive */
				if (execlevel)
					--execlevel;
				goto onward;

			case DGOTO:	/* GOTO directive */
				/* .....only if we are currently executing */
				if (execlevel == 0) {

					/* grab label to jump to */
					eline = token(eline, golabel);
					linlen = strlen(golabel);
					glp = hlp->l_fp;
					while (glp != hlp) {
						if (*glp->l_text == '*' &&
						    (strncmp(&glp->l_text[1], golabel,
						            linlen) == 0)) {
							lp = glp;
							goto onward;
						}
						glp = glp->l_fp;
					}
					mlwrite("%%No such label");
					freewhile(whlist);
					mstore = FALSE;
					dobufnesting--;
					return FALSE;
				}
				goto onward;
	
			case DRETURN:	/* RETURN directive */
				if (execlevel == 0)
					goto eexec;
				goto onward;

			case DENDWHILE:	/* ENDWHILE directive */
				if (execlevel) {
					--execlevel;
					goto onward;
				} else {
					/* find the right while loop */
					whtemp = whlist;
					while (whtemp) {
						if (whtemp->w_type == BTWHILE &&
						    whtemp->w_end == lp)
							break;
						whtemp = whtemp->w_next;
					}
		
					if (whtemp == NULL) {
						mlwrite("%%Internal While loop error");
						freewhile(whlist);
						mstore = FALSE;
						dobufnesting--;
						return FALSE;
					}
		
					/* reset the line pointer back.. */
					lp = whtemp->w_begin->l_bp;
					goto onward;
				}

			case DFORCE:	/* FORCE directive */
				force = TRUE;

			}
		}
#endif

		/* execute the statement */
		status = docmd(eline,TRUE,FALSE,1);
		if (force)		/* force the status */
			status = TRUE;

		/* check for a command error */
		if (status != TRUE) {
			/* look if buffer is showing */
			wp = wheadp;
			while (wp != NULL) {
				if (wp->w_bufp == bp) {
					/* and point it */
					wp->w_dot.l = lp;
					wp->w_dot.o = 0;
					wp->w_flag |= WFHARD;
				}
				wp = wp->w_wndp;
			}
			/* in any case set the buffer . */
			bp->b_dot.l = lp;
			bp->b_dot.o = 0;
			free(einit);
			execlevel = 0;
			mstore = FALSE;
			freewhile(whlist);
			dobufnesting--;
			return status;
		}

onward:		/* on to the next line */
		free(einit);
		lp = lp->l_fp;
	}

#if ! SMALLER
eexec:	/* exit the current function */
#endif
	mstore = FALSE;
	execlevel = 0;
	freewhile(whlist);
	dobufnesting--;
        return TRUE;
}

freewhile(wp)	/* free a list of while block pointers */
WHBLOCK *wp;	/* head of structure to free */
{
	if (wp == NULL)
		return;
	if (wp->w_next)
		freewhile(wp->w_next);
	free((char *)wp);
}

#if ! SMALLER
execfile(f, n)	/* execute a series of commands in a file */
int f, n;	/* default flag and numeric arg to pass on to file */
{
	register int status;	/* return status of name query */
	static char ofname[NSTRING];	/* name of file to execute */
	char fname[NSTRING];	/* name of file to execute */
	char *fspec;		/* full file spec */

	if ((status = mlreply("File to execute: ", ofname, NSTRING -1)) != TRUE)
		return status;
	strcpy(fname,ofname);

	if ((status = glob(fname)) != TRUE)
		return status;

	/* look up the path for the file */
	fspec = flook(fname, FL_ANYWHERE);

	/* if it isn't around */
	if (fspec == NULL)
		return FALSE;

	/* otherwise, execute it */
	while (n-- > 0)
		if ((status=dofile(fspec)) != TRUE)
			return status;

	return TRUE;
}
#endif

/*	dofile:	yank a file into a buffer and execute it
		if there are no errors, delete the buffer on exit */

dofile(fname)
char *fname;	/* file name to execute */
{
	register BUFFER *bp;	/* buffer to place file to exeute */
	register int status;	/* results of various calls */
	register int odiscmd;
	char bname[NBUFN];	/* name of buffer */

	makename(bname, fname);		/* derive the name of the buffer */
	unqname(bname,FALSE);

	/* okay, we've got a unique name -- create it */
	if ((bp=bfind(bname, OK_CREAT, 0))==NULL) {
		return FALSE;
	}

	/* mark the buffer as read only */
	make_local_b_val(bp,MDVIEW);
	set_b_val(bp,MDVIEW,TRUE);

	/* and try to read in the file to execute */
	if ((status = readin(fname, FALSE, bp, TRUE)) != TRUE) {
		return status;
	}

	/* go execute it! */
	odiscmd = discmd;
	discmd = FALSE;
	status = dobuf(bp);
	discmd = odiscmd;
	if (status != TRUE)
		return status;

	/* if not displayed, remove the now unneeded buffer and exit */
	if (bp->b_nwnd == 0)
		zotbuf(bp);
	return TRUE;
}

/*	cbuf:	Execute the contents of a numbered buffer	*/

cbuf(f, n, bufnum)
int f, n;	/* default flag and numeric arg */
int bufnum;	/* number of buffer to execute */
{
        register BUFFER *bp;		/* ptr to buffer to execute */
        register int status;		/* status return */
	static char bufname[] = "[Macro xx]";

	/* make the buffer name */
	bufname[7] = '0' + (bufnum / 10);
	bufname[8] = '0' + (bufnum % 10);

	/* find the pointer to that buffer */
        if ((bp=bfind(bufname, NO_CREAT, 0)) == NULL) {
        	mlwrite("[Macro not defined]");
                return FALSE;
        }

	/* and now execute it as asked */
	while (n-- > 0)
		if ((status = dobuf(bp)) != TRUE)
			return status;
	return TRUE;
}

cbuf1(f, n)
{
	cbuf(f, n, 1);
}

cbuf2(f, n)
{
	cbuf(f, n, 2);
}

cbuf3(f, n)
{
	cbuf(f, n, 3);
}

cbuf4(f, n)
{
	cbuf(f, n, 4);
}

cbuf5(f, n)
{
	cbuf(f, n, 5);
}

cbuf6(f, n)
{
	cbuf(f, n, 6);
}

cbuf7(f, n)
{
	cbuf(f, n, 7);
}

cbuf8(f, n)
{
	cbuf(f, n, 8);
}

cbuf9(f, n)
{
	cbuf(f, n, 9);
}

cbuf10(f, n)
{
	cbuf(f, n, 10);
}

cbuf11(f, n)
{
	cbuf(f, n, 11);
}

cbuf12(f, n)
{
	cbuf(f, n, 12);
}

cbuf13(f, n)
{
	cbuf(f, n, 13);
}

cbuf14(f, n)
{
	cbuf(f, n, 14);
}

cbuf15(f, n)
{
	cbuf(f, n, 15);
}

cbuf16(f, n)
{
	cbuf(f, n, 16);
}

cbuf17(f, n)
{
	cbuf(f, n, 17);
}

cbuf18(f, n)
{
	cbuf(f, n, 18);
}

cbuf19(f, n)
{
	cbuf(f, n, 19);
}

cbuf20(f, n)
{
	cbuf(f, n, 20);
}

cbuf21(f, n)
{
	cbuf(f, n, 21);
}

cbuf22(f, n)
{
	cbuf(f, n, 22);
}

cbuf23(f, n)
{
	cbuf(f, n, 23);
}

cbuf24(f, n)
{
	cbuf(f, n, 24);
}

cbuf25(f, n)
{
	cbuf(f, n, 25);
}

cbuf26(f, n)
{
	cbuf(f, n, 26);
}

cbuf27(f, n)
{
	cbuf(f, n, 27);
}

cbuf28(f, n)
{
	cbuf(f, n, 28);
}

cbuf29(f, n)
{
	cbuf(f, n, 29);
}

cbuf30(f, n)
{
	cbuf(f, n, 30);
}

cbuf31(f, n)
{
	cbuf(f, n, 31);
}

cbuf32(f, n)
{
	cbuf(f, n, 32);
}

cbuf33(f, n)
{
	cbuf(f, n, 33);
}

cbuf34(f, n)
{
	cbuf(f, n, 34);
}

cbuf35(f, n)
{
	cbuf(f, n, 35);
}

cbuf36(f, n)
{
	cbuf(f, n, 36);
}

cbuf37(f, n)
{
	cbuf(f, n, 37);
}

cbuf38(f, n)
{
	cbuf(f, n, 38);
}

cbuf39(f, n)
{
	cbuf(f, n, 39);
}

cbuf40(f, n)
{
	cbuf(f, n, 40);
}


