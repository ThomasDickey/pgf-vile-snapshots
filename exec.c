/*	This file is for functions dealing with execution of
 *	commands, command lines, buffers, files and startup files
 *
 *	written 1986 by Daniel Lawrence	
 *
 * $Log: exec.c,v $
 * Revision 1.35  1992/12/14 09:03:25  foxharp
 * lint cleanup, mostly malloc
 *
 * Revision 1.34  1992/12/04  09:12:25  foxharp
 * deleted unused assigns
 *
 * Revision 1.33  1992/08/20  23:40:48  foxharp
 * typo fixes -- thanks, eric
 *
 * Revision 1.32  1992/07/16  22:06:27  foxharp
 * same change for kbdmode
 *
 * Revision 1.31  1992/07/15  23:24:12  foxharp
 * dotcmdplay() is now undoable, so don't undo individual
 * commands when "dotcmdmode == PLAY"
 *
 * Revision 1.30  1992/07/07  08:37:48  foxharp
 * set macro buffer's "dot" to first line when done storing, rather than
 * leaving it at the last line.  less confusing when you bring one up for
 * editing.
 *
 * Revision 1.29  1992/06/25  22:44:06  foxharp
 * better string quoting parse, from pjr
 *
 * Revision 1.28  1992/05/19  08:55:44  foxharp
 * more prototype and shadowed decl fixups
 *
 * Revision 1.27  1992/05/16  12:00:31  pgf
 * prototypes/ansi/void-int stuff/microsoftC
 *
 * Revision 1.26  1992/04/26  13:42:33  pgf
 * added names to "no such ..." messages
 *
 * Revision 1.25  1992/03/13  08:11:04  pgf
 * allow ":0" to work by honoring ZERO flag for gomark
 *
 * Revision 1.24  1992/03/03  08:41:15  pgf
 * took out pre_colon_pos -- now we don't move dot unless the command _wants_
 * a line no. or range
 *
 * Revision 1.23  1992/02/17  09:01:50  pgf
 * save "dot before the named colon command" so that ':' can be expanded to
 * a filename correctly, as in ":e:".
 * also, fixed bug in buffer exec code, and
 * took out unused vars for saber
 *
 * Revision 1.22  1992/01/22  20:26:50  pgf
 * made directive leading char consistent ('~', instead uemacs' '!')
 *
 * Revision 1.21  1992/01/06  23:09:05  pgf
 * switch to buffer on failure in dobuf()
 *
 * Revision 1.20  1992/01/05  00:06:13  pgf
 * split mlwrite into mlwrite/mlprompt/mlforce to make errors visible more
 * often.  also normalized message appearance somewhat.
 *
 * Revision 1.19  1991/11/13  20:09:27  pgf
 * X11 changes, from dave lemke
 *
 * Revision 1.18  1991/11/08  13:16:15  pgf
 * added dave lemke's ":+NN" and ":-NN" commands, and made ":+++" and
 * ":----" work as well
 *
 * Revision 1.17  1991/11/07  03:58:31  pgf
 * lint cleanup
 *
 * Revision 1.16  1991/11/03  17:36:59  pgf
 * make 0 arg work in empty buffer, as in ":0r file"
 *
 * Revision 1.15  1991/11/01  14:38:00  pgf
 * saber cleanup
 *
 * Revision 1.14  1991/10/08  01:30:00  pgf
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


extern CMDFUNC f_gomark;

/* namedcmd:	execute a named command even if it is not bound */

int
namedcmd(f, n)
int f, n;
{
	char *fnp;	/* ptr to the name of the cmd to exec */
	LINE *fromline;	/* first linespec */
	LINE *toline;	/* second linespec */
	char lspec[NLINE];
	int cpos = 0;
	int s,c,isdfl,zero;
	long flags;
	CMDFUNC *cfp;		/* function to execute */

	lspec[0] = '\0';
	fnp = NULL;

	/* prompt the user to type a named command */
	mlprompt(": ");

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
		mlforce("[Improper line range]");
		return FALSE;
	}

	/* if range given, and it wasn't "0" and the buffer's empty */
	if (!isdfl && !zero && is_empty_buf(curbp)) {
		mlforce("[No range possible in empty buffer]", fnp);
		return FALSE;
	}

#if	NeWS
	newsimmediateoff() ;
#endif

	/* did we get a name? */
	if (fnp == NULL) {
		if (isdfl) { /* no range, no function */
			mlforce("[No such function]");
			return FALSE;
		} else { /* range, no function */
			cfp = &f_gomark;
			fnp = "";
		}
	} else if ((cfp = engl2fnc(fnp)) == NULL) { /* bad function */
		mlforce("[No such function \"%s\"]",fnp);
		return FALSE;
	}
	flags = cfp->c_flags;
	
	/* bad arguments? */
#ifdef EXRC_FILES
seems like we need one more check here -- is it from a .exrc file?
	    cmd not ok in .exrc 		empty file
	if (!(flags & EXRCOK) && is_empty_buf(curbp)) {
		mlforce("[Can't use the \"%s\" command in a %s file.]", 
					cmdnames[cmdidx].name, EXRC);
		return FALSE;
	}
#endif

	/* was: if (!(flags & (ZERO | EXRCOK)) && fromline == NULL ) { */
	if (zero) {
		extern CMDFUNC f_lineputafter, f_opendown, f_insfile;
		extern CMDFUNC f_lineputbefore, f_openup;
		if (!(flags & ZERO)) {
			mlforce("[Can't use address 0 with \"%s\" command]", fnp);
			return FALSE;
		}
		/*  we're positioned at fromline == curbp->b_linep, so commands
			must be willing to go _down_ from there.  Seems easiest
			to special case the commands that prefer going up */
		if (cfp == &f_insfile) {
			/* EMPTY */ /* works okay, or acts down normally */ ;
		} else if (cfp == &f_lineputafter) {
			cfp = &f_lineputbefore;
			fromline = lforw(fromline);
		} else if (cfp == &f_opendown) {
			cfp = &f_openup;
			fromline = lforw(fromline);
		} else if (cfp == &f_gomark) {
			fromline = lforw(fromline);
		} else {
			mlforce("[Configuration error: ZERO]");
			return FALSE;
		}
		flags = cfp->c_flags;
		toline = fromline;
	}

	/* if we're not supposed to have a line no., and the line no. isn't
		the current line, and there's more than one line */
	if (!(flags & FROM) && fromline != DOT.l &&
			!is_empty_buf(curbp) &&
		  (lforw(lforw(curbp->b_line.l)) != curbp->b_line.l) ) {
		mlforce("[Can't use address with \"%s\" command.]", fnp);
		return FALSE;
	}
	/* if we're not supposed to have a second line no., and the line no. 
		isn't the same as the first line no., and there's more than
		one line */
	if (!(flags & TO) && toline != fromline &&
			!is_empty_buf(curbp) &&
		  (lforw(lforw(curbp->b_line.l)) != curbp->b_line.l) ) {
		mlforce("[Can't use a range with \"%s\" command.]", fnp);
		return FALSE;
	}
#ifdef NEEDED
	if (!(flags & EXTRA) && *scan) {
		mlforce("[Extra characters after \"%s\" command.]", 
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
				mlforce("[Too many %s to \"%s\" command.]",
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
				mlforce("[Configuration error: DFLALL]");
				return FALSE;
			}
		} else if (flags & DFLNONE) {
			extern CMDFUNC f_operfilter, f_spawn;
			if (cfp == &f_operfilter) {
				cfp = &f_spawn;
				setmark();  /* not that it matters */
			} else {
				mlforce("[Configuration error: DFLNONE]");
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
		curwp->w_lastdot = DOT;
	}
	if (toline && (flags & TO)) {
		DOT.l = toline;
		firstnonwhite(f,n);
		setmark();
	}
	if (fromline && (flags & FROM)) {
		DOT.l = fromline;
		firstnonwhite(f,n);
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
	int		num;
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
			lp = DOT.l;
		} else if (*s == '$') { /* '$' means the last line */
			s++;
			status = gotoeob(TRUE,1);
			if (status) lp = DOT.l;
		} else if (isdigit(*s)) {
			/* digit means an absolute line number */
			for (num = 0; isdigit(*s); s++) {
				num = num * 10 + *s - '0';
			}
			status = gotoline(TRUE,num);
			if (status) lp = DOT.l;
		} else if (*s == '\'') {
			/* apostrophe means go to a set mark */
			s++;
			status = gonmmark(*s);
			if (status) lp = DOT.l;
			s++;
		} else if (*s == '+') {
			s++;
			for (num = 0; isdigit(*s); s++)
				num = num * 10 + *s - '0';
			if (num == 0)
				num++;
			while (*s == '+')
				s++, num++;
			status = forwline(TRUE,num);
			if (status)
				lp = DOT.l;
                } else if (*s == '-') {
			s++;
			for (num = 0; isdigit(*s); s++)
					num = num * 10 + *s - '0';
			if (num == 0)
				num++;
			while (*s == '-')
				s++, num++;
			status = forwline(TRUE,-num);
			if (status)
				lp = DOT.l;
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
			lp = DOT.l;
		}
	} while (*s == ';' || *s == '+' || *s == '-');

	*markptr = lp;
	swapmark();
	return s;
}

/* parse an ex-style line range -- code culled from elvis, file ex.c, by
	Steve Kirkendall
*/
int
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

	*zerop = FALSE;

	/* ignore command lines that start with a double-quote */
	if (*specp == '"') {
		*fromlinep = *tolinep = DOT.l;
		return TRUE;
	}

	/* permit extra colons at the start of the line */
	while (isspace(*specp) || *specp == ':') {
		specp++;
	}

	/* parse the line specifier */
	scan = specp;
	if (*scan == '0') {
		fromline = toline = curbp->b_line.l; /* _very_ top of buffer */
		*zerop = TRUE;
		scan++;
	} else if (*scan == '%') {
		/* '%' means all lines */
		fromline = lforw(curbp->b_line.l);
		toline = lback(curbp->b_line.l);
		scan++;
	} else {
		scan = linespec(scan, &fromline);
		if (!fromline)
			fromline = DOT.l;
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

	if (is_empty_buf(curbp))
		fromline = toline = NULL;

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
int
onamedcmd(f, n)
int f, n;	/* command arguments [passed through to command executed] */
{
	register char *fnp;	/* ptr to the name of the cmd to exec */
	char *kbd_engl();
	int s;

	/* prompt the user to type a named command */
	mlprompt(": ");

	/* and now get the function name to execute */
#if	NeWS
	newsimmediateon() ;
#endif

	fnp = kbd_engl();

#if	NeWS
	newsimmediateoff() ;
#endif

	if (fnp == NULL) {
		mlforce("[No such function]");
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
int
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

int
docmd(cline,newcle,f,n)
char *cline;	/* command line to execute */
int newcle;
int f,n;
{
	int status;		/* return status of function */
	int oldcle;		/* old contents of clexec flag */
	char *oldestr;		/* original exec string */
	char tkn[NSTRING];	/* next token off of command line */
	CMDFUNC *cfp;
		
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

	/* process leading argument */
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
		mlforce("[No such function \"%s\"]",tkn);
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

/*
 * This is the general command execution routine. It takes care of checking
 * flags, globals, etc, to be sure we're not doing something dumb.
 * Return the status of command.
 */

int
execute(execfunc, f, n)
CMDFUNC *execfunc;		/* ptr to function to execute */
int f,n;
{
	register int status, flags;
	MARK odot;

	if (execfunc == NULL) {
		TTbeep();
#if REBIND
		mlforce("[Key not bound]");	/* complain		*/
#else
		mlforce("[Not a command]");	/* complain		*/
#endif
		return FALSE;
	}

	flags = execfunc->c_flags;

	/* commands following operators can't be redone or undone */
	if ( !doingopcmd) {
		/* don't record non-redoable cmds */
		if ((flags & REDO) == 0)
			dotcmdstop();
		if (flags & UNDO) {
			/* undoable command can't be permitted when read-only */
			if (b_val(curbp,MDVIEW))
				return rdonly();
			if (dotcmdmode != PLAY && kbdmode != PLAY)
				mayneedundo();
		}
	}

	/* if motion is absolute, remember where we are */
	if (flags & ABS) {
		odot = DOT;
	}

	status = (execfunc->c_func)(f, n);
	if ((flags & GOAL) == 0) { /* goal should not be retained */
		curgoal = -1;
	}
#if VMALLOC
	if (flags & UNDO)	/* verify malloc arena after line changers */
		vverify("main");
#endif

	/* if motion was absolute, and it wasn't just on behalf of an
		operator, and we moved, update the "last dot" mark */
	if ((flags & ABS) && !doingopcmd && !sameline(DOT, odot)) {
		curwp->w_lastdot = odot;
	}


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


	quotef = FALSE;
	/* scan through the source string */
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

int
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

int
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

int
storemac(f, n)
int f;		/* default flag */
int n;		/* macro number to use */
{
	register struct BUFFER *bp;	/* pointer to macro buffer */
	char bname[NBUFN];		/* name of buffer to use */

	/* must have a numeric argument to this function */
	if (f == FALSE) {
		mlforce("[No macro specified]");
		return FALSE;
	}

	/* range check the macro number */
	if (n < 1 || n > 40) {
		mlforce("[Macro number out of range]");
		return FALSE;
	}

	/* construct the macro buffer name */
	strcpy(bname, "[Macro xx]");
	bname[7] = '0' + (n / 10);
	bname[8] = '0' + (n % 10);

	/* set up the new macro buffer */
	if ((bp = bfind(bname, OK_CREAT, BFINVS)) == NULL) {
		mlforce("[Cannot create macro]");
		return FALSE;
	}

	/* and make sure it is empty */
	bclear(bp);
	bp->b_flag &= ~BFCHG;
	bp->b_active = TRUE;
	make_local_b_val(bp,MDVIEW);
	set_b_val(bp,MDVIEW,TRUE);
	make_local_b_val(bp,VAL_TAB);
	set_b_val(bp,VAL_TAB,8);
	make_local_b_val(bp,MDDOS);
	set_b_val(bp,MDDOS,FALSE);

	/* and set the macro store pointers to it */
	mstore = TRUE;
	bstore = bp;
	return TRUE;
}

#if	PROC
/*	storeproc:	Set up a procedure buffer and flag to store all
			executed command lines there			*/

int
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
		mlforce("[Cannot create macro]");
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

int
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
	strcpy(&bufn[1], obufn);
	strcat(bufn, "]");

	/* find the pointer to that buffer */
        if ((bp=bfind(bufn, NO_CREAT, 0)) == NULL) {
		mlforce("[No such procedure \"%s\"]",bufn);
                return FALSE;
        }

	if (!f)
		n = 1;

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

int
execbuf(f, n)
int f, n;	/* default flag and numeric arg */
{
        register BUFFER *bp;		/* ptr to buffer to execute */
        register int status;		/* status return */
        static char bufn[NSTRING];		/* name of buffer to execute */

	if (!f)
		n = 1;

	/* find out what buffer the user wants to execute */
        if ((status = mlreply("Execute buffer: ", bufn, NBUFN)) != TRUE)
                return status;

	/* find the pointer to that buffer */
        if ((bp=bfind(bufn, NO_CREAT, 0)) == NULL) {
		mlforce("[No such buffer \"%s\"]",bufn);
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

void
freewhile(wp)	/* free a list of while block pointers */
WHBLOCK *wp;	/* head of structure to free */
{
	if (wp == NULL)
		return;
	if (wp->w_next)
		freewhile(wp->w_next);
	free((char *)wp);
}

#define DIRECTIVE_CHAR '~'

int
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
	WHBLOCK *whlist;	/* ptr to WHILE list */
	char *einit;		/* initial value of eline */
	char *eline;		/* text of line to execute */
#if ! SMALLER
	WHBLOCK *scanpt;	/* ptr during scan */
	register LINE *glp;	/* line to goto */
	WHBLOCK *whtemp;	/* temporary ptr to a WHBLOCK */
	char tkn[NSTRING];	/* buffer to evaluate an expression in */
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
	scanpt = NULL;
	/* scan the buffer to execute, building WHILE header blocks */
	hlp = bp->b_line.l;
	lp = hlp->l_fp;
	bp->b_dot.o = 0;
	while (lp != hlp) {
		bp->b_dot.l = lp;
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
		if (eline[0] == DIRECTIVE_CHAR && eline[1] == 'w' && eline[2] == 'h') {
			whtemp = typealloc(WHBLOCK);
			if (whtemp == NULL) {
noram:				mlforce("[Out of memory during while scan]");
failexit:			freewhile(scanpt);
				freewhile(whlist);
				mstore = FALSE;
				dobufnesting--;
				return FALSE;
			}
			whtemp->w_begin = lp;
			whtemp->w_type = BTWHILE;
			whtemp->w_next = scanpt;
			scanpt = whtemp;
		}

		/* if is a BREAK directive, make a block... */
		if (eline[0] == DIRECTIVE_CHAR && eline[1] == 'b' && eline[2] == 'r') {
			if (scanpt == NULL) {
				mlforce("[BREAK outside of any WHILE loop]");
				goto failexit;
			}
			whtemp = typealloc(WHBLOCK);
			if (whtemp == NULL)
				goto noram;
			whtemp->w_begin = lp;
			whtemp->w_type = BTBREAK;
			whtemp->w_next = scanpt;
			scanpt = whtemp;
		}

		/* if it is an endwhile directive, record the spot... */
		if (eline[0] == DIRECTIVE_CHAR && strncmp(&eline[1], "endw", 4) == 0) {
			if (scanpt == NULL) {
				mlforce("[ENDWHILE with no preceding WHILE in '%s']",
					bp->b_bname);
				goto failexit;
			}
			/* move top records from the scanpt list to the
			   whlist until we have moved all BREAK records
			   and one WHILE record */
			do {
				scanpt->w_end = lp;
				whtemp = whlist;
				whlist = scanpt;
				scanpt = scanpt->w_next;
				whlist->w_next = whtemp;
			} while (whlist->w_type == BTBREAK);
		}

nxtscan:	/* on to the next line */
		lp = lp->l_fp;
	}

	/* while and endwhile should match! */
	if (scanpt != NULL) {
		mlforce("[WHILE with no matching ENDWHILE in '%s']",
			bp->b_bname);
		goto failexit;
	}
#endif

	/* starting at the beginning of the buffer */
	hlp = bp->b_line.l;
	lp = hlp->l_fp;
	while (lp != hlp) {
		bp->b_dot.l = lp;
		/* allocate eline and copy macro line to it */
		linlen = lp->l_used;
		if ((einit = eline = castalloc(char, linlen+1)) == NULL) {
			mlforce("[Out of Memory during macro execution]");
			freewhile(whlist);
			mstore = FALSE;
			dobufnesting--;
			return FALSE;
		}

		/* pjr - must check for empty line before copy */
		if (linlen)
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
			strcat(outline, l_itoa(execlevel));
			strcat(outline, ":");

			/* and lastly the line */
			strcat(outline, eline);
			strcat(outline, ">>>");
	
			/* write out the debug line */
			mlforce("%s",outline);
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
		if (*eline == DIRECTIVE_CHAR) {
			/* Find out which directive this is */
			++eline;
			for (dirnum = 0; dirnum < NUMDIRS; dirnum++)
				if (strncmp(eline, dname[dirnum],
				            strlen(dname[dirnum])) == 0)
					break;

			/* and bitch if it's illegal */
			if (dirnum == NUMDIRS) {
				mlforce("[Unknown directive \"%s\"]", eline);
				freewhile(whlist);
				mstore = FALSE;
				dobufnesting--;
				return FALSE;
			}

			/* service only the ENDM macro here */
			if (dirnum == DENDM) {
				mstore = FALSE;
				bstore->b_dot.l = lforw(bstore->b_line.l);
				bstore->b_dot.o = 0;
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
			if ((mp=lalloc((int)linlen,bstore)) == NULL) {
				mlforce("[Out of memory while storing macro]");
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
				/* drop down and act just like BREAK */

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
					mlforce("[WHILE loop error]");
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
					(void) token(eline, golabel);
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
					mlforce("[No such label \"%s\"]", golabel);
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
						mlforce("[Internal While loop error]");
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
			/* in any case set the buffer's dot */
			bp->b_dot.l = lp;
			bp->b_dot.o = 0;
			bp->b_wline.l = lforw(bp->b_line.l);
			free(einit);
			execlevel = 0;
			mstore = FALSE;
			freewhile(whlist);
			dobufnesting--;
			swbuffer(bp);
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


#if ! SMALLER
/* ARGSUSED */
int
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

int
dofile(fname)
char *fname;	/* file name to execute */
{
	register BUFFER *bp;	/* buffer to place file to execute */
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

int
cbuf(f, n, bufnum)
int f, n;	/* default flag and numeric arg */
int bufnum;	/* number of buffer to execute */
{
        register BUFFER *bp;		/* ptr to buffer to execute */
        register int status;		/* status return */
	static char bufname[] = "[Macro xx]";

	if (!f) n = 1;

	/* make the buffer name */
	bufname[7] = '0' + (bufnum / 10);
	bufname[8] = '0' + (bufnum % 10);

	/* find the pointer to that buffer */
        if ((bp=bfind(bufname, NO_CREAT, 0)) == NULL) {
        	mlforce("[Macro not defined]");
                return FALSE;
        }

	/* and now execute it as asked */
	while (n-- > 0)
		if ((status = dobuf(bp)) != TRUE)
			return status;
	return TRUE;
}

int cbuf1(f, n) int f,n; { return cbuf(f, n, 1); }

int cbuf2(f, n) int f,n; { return cbuf(f, n, 2); }

int cbuf3(f, n) int f,n; { return cbuf(f, n, 3); }

int cbuf4(f, n) int f,n; { return cbuf(f, n, 4); }

int cbuf5(f, n) int f,n; { return cbuf(f, n, 5); }

int cbuf6(f, n) int f,n; { return cbuf(f, n, 6); }

int cbuf7(f, n) int f,n; { return cbuf(f, n, 7); }

int cbuf8(f, n) int f,n; { return cbuf(f, n, 8); }

int cbuf9(f, n) int f,n; { return cbuf(f, n, 9); }

int cbuf10(f, n) int f,n; { return cbuf(f, n, 10); }

int cbuf11(f, n) int f,n; { return cbuf(f, n, 11); }

int cbuf12(f, n) int f,n; { return cbuf(f, n, 12); }

int cbuf13(f, n) int f,n; { return cbuf(f, n, 13); }

int cbuf14(f, n) int f,n; { return cbuf(f, n, 14); }

int cbuf15(f, n) int f,n; { return cbuf(f, n, 15); }

int cbuf16(f, n) int f,n; { return cbuf(f, n, 16); }

int cbuf17(f, n) int f,n; { return cbuf(f, n, 17); }

int cbuf18(f, n) int f,n; { return cbuf(f, n, 18); }

int cbuf19(f, n) int f,n; { return cbuf(f, n, 19); }

int cbuf20(f, n) int f,n; { return cbuf(f, n, 20); }

int cbuf21(f, n) int f,n; { return cbuf(f, n, 21); }

int cbuf22(f, n) int f,n; { return cbuf(f, n, 22); }

int cbuf23(f, n) int f,n; { return cbuf(f, n, 23); }

int cbuf24(f, n) int f,n; { return cbuf(f, n, 24); }

int cbuf25(f, n) int f,n; { return cbuf(f, n, 25); }

int cbuf26(f, n) int f,n; { return cbuf(f, n, 26); }

int cbuf27(f, n) int f,n; { return cbuf(f, n, 27); }

int cbuf28(f, n) int f,n; { return cbuf(f, n, 28); }

int cbuf29(f, n) int f,n; { return cbuf(f, n, 29); }

int cbuf30(f, n) int f,n; { return cbuf(f, n, 30); }

int cbuf31(f, n) int f,n; { return cbuf(f, n, 31); }

int cbuf32(f, n) int f,n; { return cbuf(f, n, 32); }

int cbuf33(f, n) int f,n; { return cbuf(f, n, 33); }

int cbuf34(f, n) int f,n; { return cbuf(f, n, 34); }

int cbuf35(f, n) int f,n; { return cbuf(f, n, 35); }

int cbuf36(f, n) int f,n; { return cbuf(f, n, 36); }

int cbuf37(f, n) int f,n; { return cbuf(f, n, 37); }

int cbuf38(f, n) int f,n; { return cbuf(f, n, 38); }

int cbuf39(f, n) int f,n; { return cbuf(f, n, 39); }

int cbuf40(f, n) int f,n; { return cbuf(f, n, 40); }


