/*	This file is for functions dealing with execution of
 *	commands, command lines, buffers, files and startup files
 *
 *	written 1986 by Daniel Lawrence
 *
 * $Log: exec.c,v $
 * Revision 1.83  1994/03/08 12:09:11  pgf
 * changed 'fulllineregions' to 'regionshape'.
 *
 * Revision 1.82  1994/02/22  11:03:15  pgf
 * truncated RCS log for 4.0
 *
 */

#include	"estruct.h"
#include	"edef.h"


extern CMDFUNC f_gomark;

static	int	eol_range P(( char *, int, int, int ));
static	int	end_of_cmd P(( void ));
#if !SMALLER
static	int	execute_named_command P(( int, int ));
#endif
static	int	rangespec P(( char *, LINEPTR *, LINEPTR *, CMDFLAGS * ));
static	char *	linespec P(( char *, LINEPTR * ));
static	void	freewhile P(( WHBLOCK * ));

/* directive name table:
	This holds the names of all the directives....	*/

static	char	*dname[] = {
#if ! SMALLER
	"if",    "else",     "endif",
	"goto",  "return",   "endm",
	"while", "endwhile", "break",
	"force"
#else
	 "endm"
#endif
	};

/*----------------------------------------------------------------------------*/

/*ARGSUSED*/
static int
eol_range(buffer, cpos, c, eolchar)
char *	buffer;
int	cpos;
int	c;
int	eolchar;
{
	if (is_edit_char(c))
		return FALSE;

	if (isspecial(c)	/* sorry, cannot scroll with arrow keys */
	 || iscntrl(c))
		return TRUE;

	if (islinespecchar(c)
	 || /* special test for 'a style mark references */
		(cpos > 0
		&& buffer[cpos-1] == '\''
		&& (islower(c) || (c == '\'') ) ) )
		return FALSE;
	return TRUE;
}

/*
 * Returns true iff the user ended the last prompt with a carriage-return.
 */
static int
end_of_cmd()
{
	register int c = end_string();
	return isreturn(c);
}

/* returns true iff we are in a named-command and if the user ended the last
 * prompt with a carriage-return.
 */
int
end_named_cmd()
{
	if (isnamedcmd) {
		return end_of_cmd();
	}
	return FALSE;
}

/* returns true iff we are in a named-command and if the user did not end the
 * last prompt with a carriage-return.
 */
int
more_named_cmd()
{
	if (isnamedcmd) {
		return !end_of_cmd();
	}
	return FALSE;
}

/* namedcmd:	execute a named command even if it is not bound */
#if SMALLER
#define execute_named_command namedcmd
#else
static	/* next procedure is local */
#endif

int
execute_named_command(f, n)
int f, n;
{
	register int	status;
	register CMDFUNC *cfp;	/* function to execute */
	register char *fnp;	/* ptr to the name of the cmd to exec */

	LINEPTR fromline;	/* first linespec */
	LINEPTR toline;		/* second linespec */
	MARK	save_DOT;
	MARK	last_DOT;
	char lspec[NLINE];	/* text of line spec */
	char cspec[NLINE];	/* text of command spec */
	int cmode = 0;
	int c;
	int repeat_cmd = EOS;
	int maybe_reg, maybe_num;
	CMDFLAGS lflag, flags;

	lspec[0] = EOS;
	last_DOT = DOT;

	/* prompt the user to type a named command */
	mlprompt(": ");

	/* and now get the function name to execute */
	while(1) {
		if (cmode == 0) {	/* looking for range-spec, if any */
			status = kbd_reply(
				(char *)0,	/* no-prompt => splice */
				lspec,		/* in/out buffer */
				sizeof(lspec),
				eol_range,
				EOS,		/* may be a conflict */
				0,		/* no expansion, etc. */
				no_completion);
			c = end_string();
			if (status != TRUE && status != FALSE) {
				if (status == SORTOFTRUE) {
					(void)tgetc(FALSE); /* cancel the tungetc */
					return FALSE;
				}
				return status;
			} else if (isreturn(c) && (status == FALSE)) {
				return FALSE;
			} else {
				tungetc(c);	/* ...so we can splice */
			}
			cmode = 1;
			repeat_cmd = EOS;
		} else {			/* looking for command-name */
			fnp = NULL;
			status = kbd_engl_stat((char *)0, cspec);
			if (status == TRUE) {
				fnp = cspec;
#if !SMALLER
				if (isRepeatable(*fnp) && clexec) {
					repeat_cmd = *fnp;
					cmode++;
					hst_glue(EOS);
				} else
#endif
				 if (isRepeatable(*fnp) && (*fnp == end_string())) {
					tungetc(*fnp);
					repeat_cmd = *fnp;
					cmode++;
					hst_glue(EOS);
				} else {
					c = fnp[strlen(fnp)-1];
					if (ispunct(c)) {
						c = end_string();
						if (c != NAMEC
						 && c != ' '
						 && !isreturn(c)) {
							tungetc(c);
							/* e.g., !-command */
						 }
					} else {	/* e.g., ":e#" */
						c = end_string();
						if (ispunct(c)
						 && strchr(global_g_val_ptr(GVAL_EXPAND_CHARS),c) != 0)
							tungetc(c);
					}
					break;
				}
			} else if (status == SORTOFTRUE) {
				hst_remove((cmode > 1) ? "*" : "");
				if (cmode > 1)
					(void)tgetc(FALSE); /* eat the delete */
				if (--cmode <= 1) {
					cmode = 0;
					hst_remove(lspec);
				}
			} else if ((status == ABORT) || (lastkey == killc)) {
				return status;
			} else {	/* status == FALSE (killc/wkillc) */
				if (cmode > 1) {
					fnp = cspec;
					cmode--;
					break;
				} else {
					if (lspec[0] == EOS) {
						return status;
					} else {
						break;	/* range-only */
					}
				}
			}
		}
	}

	/* reconstruct command if user edited it */
	if (repeat_cmd != EOS && fnp != 0 && *fnp == EOS) {
		fnp[0] = repeat_cmd;
		fnp[1] = EOS;
	}

	/* infer repeat count from repeated-command */
	if (cmode > 1) {
		if (!f) {
			f = TRUE;
			n = cmode;
		} else {
			mlforce("[Redundant repeat-count]");
			return FALSE;
		}
	}

	/* parse the accumulated lspec */
	if (rangespec(lspec, &fromline, &toline, &lflag) != TRUE) {
		mlforce("[Improper line range]");
		return FALSE;
	}

	/* if range given, and it wasn't "0" and the buffer's empty */
	if (!(lflag & (DFLALL|ZERO)) && is_empty_buf(curbp)) {
		mlforce("[No range possible in empty buffer]", fnp);
		return FALSE;
	}

	/* did we get a name? */
	if (fnp == NULL) {
		if ((lflag & DFLALL)) { /* no range, no function */
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

	/* was: if (!(flags & (ZERO | EXRCOK)) && fromline == NULL ) */
	if ((lflag & ZERO)) {
		extern CMDFUNC f_lineputafter, f_opendown, f_insfile;
		extern CMDFUNC f_lineputbefore, f_openup;
		if (!(flags & ZERO)) {
			mlforce("[Can't use address 0 with \"%s\" command]", fnp);
			return FALSE;
		}
		if (same_ptr(fromline,null_ptr))
			fromline = buf_head(curbp); /* buffer is empty */

		/*  we're positioned at fromline == curbp->b_linep, so commands
			must be willing to go _down_ from there.  Seems easiest
			to special case the commands that prefer going up */
		if (cfp == &f_insfile) {
			/* EMPTY */ /* works okay, or acts down normally */ ;
		} else if (cfp == &f_lineputafter) {
			cfp = &f_lineputbefore;
			fromline = lFORW(fromline);
		} else if (cfp == &f_opendown) {
			cfp = &f_openup;
			fromline = lFORW(fromline);
		} else if (cfp == &f_gomark) {
			fromline = lFORW(fromline);
		} else {
			mlforce("[Configuration error: ZERO]");
			return FALSE;
		}
		flags = cfp->c_flags;
		toline = fromline;
	}

	/* if we're not supposed to have a line no., and the line no. isn't
		the current line, and there's more than one line */
	if (!(flags & FROM) && !same_ptr(fromline, DOT.l) &&
			!is_empty_buf(curbp) &&
		  (lforw(lForw(buf_head(curbp))) != l_ref(buf_head(curbp))) ) {
		mlforce("[Can't use address with \"%s\" command.]", fnp);
		return FALSE;
	}
	/* if we're not supposed to have a second line no., and the line no.
		isn't the same as the first line no., and there's more than
		one line */
	if (!(flags & TO) && !same_ptr(toline, fromline) &&
			!is_empty_buf(curbp) &&
		  (lforw(lForw(buf_head(curbp))) != l_ref(buf_head(curbp))) ) {
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
	if (lflag & DFLALL) {
		if (flags & DFLALL) {
			extern CMDFUNC f_showlength;
			extern CMDFUNC f_operwrite, f_filewrite, f_operglobals,
					f_globals, f_opervglobals, f_vglobals;
			if (cfp == &f_showlength) {
				fromline = lFORW(buf_head(curbp));
				toline   = lBACK(buf_head(curbp));
			} else if (cfp == &f_operwrite) {
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
			lflag |= (FROM|TO); /* avoid prompt for line-count */
		} else if (flags & DFLNONE) {
			extern CMDFUNC f_operfilter, f_spawn;
			if (cfp == &f_operfilter) {
				cfp = &f_spawn;
				(void)setmark();  /* not that it matters */
			} else {
				mlforce("[Configuration error: DFLNONE]");
				return FALSE;
			}
			fromline = toline = null_ptr;
			lflag |= (FROM|TO); /* avoid prompt for line-count */
		} else
			lflag &= ~DFLALL;
	}

#ifdef NEEDED
	/* write a newline if called from visual mode */
	if ((flags & NL) && !exmode /* && !exwrote */) {
		TTputc('\n');
		/* exrefresh(); */
	}
#endif

	/* Process argument(s) for named-buffer & line-count, if present.  The
	 * line-count is expected only if no "to" address specification was
	 * given.
	 */
	if ((*fnp != EOS) && !end_of_cmd() && !ispunct(end_string())) {
		maybe_reg = ((flags & OPTREG) == OPTREG);
		maybe_num = ((flags & TO) == TO)
			&& !((lflag & TO) == TO);
	} else {
		maybe_reg =
		maybe_num = FALSE;
	}

	if (maybe_reg || maybe_num) {
		int	num,
			this = (maybe_num && maybe_reg)
				? 0
				: (maybe_num ? 1 : -1),
			last = maybe_num ? 2 : 1;

		while (!end_of_cmd() && (this < last)) {
			status = mlreply_reg_count(this, &num, &this);
			if (status == ABORT)
				return status;
			else if (status != TRUE)
				break;
			if (this == 2) {
				swapmark();
				DOT.l = fromline;
				(void)forwline(TRUE, num-1);
				toline = DOT.l;
				swapmark();
			} else {
				ukb = num;
				kregflag = (this == 1) ? KAPPEND : 0;
				this = 1;
				/* patch: need to handle recursion */
			}
		}
	}

	if (flags & NOMOVE)
		save_DOT = DOT;
	else if (!same_ptr(toline, null_ptr) || !same_ptr(fromline, null_ptr)) {
		/* assume it's an absolute motion */
		/* we could probably do better */
		curwp->w_lastdot = DOT;
	}
	if (!same_ptr(toline, null_ptr) && (flags & TO)) {
		DOT.l = toline;
		(void)firstnonwhite(FALSE,1);
		(void)setmark();
	}
	if (!same_ptr(fromline, null_ptr) && (flags & FROM)) {
		DOT.l = fromline;
		(void)firstnonwhite(FALSE,1);
		if (same_ptr(toline, null_ptr))
			(void)setmark();
	}
	if (!sameline(last_DOT, DOT))
		curwp->w_flag |= WFMOVE;

	/* and then execute the command */
	isnamedcmd = TRUE;
	havemotion = &f_gomark;
	regionshape = FULLLINE;

	status = execute(cfp,f,n);

	havemotion = NULL;
	isnamedcmd = FALSE;
	regionshape = EXACT;

	if (flags & NOMOVE) {	/* don't use this if the command modifies! */
		DOT = save_DOT;
		curwp->w_flag &= ~WFMOVE;
	}

	return status;
}

#if !SMALLER
/* intercept calls on 'namedcmd()' to allow logging of all commands, even
 * those that have errors in them.
 */
int
namedcmd(f,n)
int f,n;
{
	int status;
	hst_init(EOS);
	status = execute_named_command(f,n);
	hst_flush();
	return status;
}
#endif

/* parse an ex-style line spec -- code culled from elvis, file ex.c, by
	Steve Kirkendall
*/
static char *
linespec(s, markptr)
register char	*s;		/* start of the line specifier */
LINEPTR		*markptr;	/* where to store the mark's value */
{
	int		num;
	LINE		*lp;	/* where the linespec takes us */
	register char	*t;
	int		status;

	(void)setmark();
	lp = NULL;

	/* parse each ;-delimited clause of this linespec */
	do {
		/* skip an initial ';', if any */
		if (*s == ';')
			s++;

		/* skip leading spaces */
		while (isspace(*s))
			s++;

		/* dot means current position */
		if (*s == '.') {
			s++;
			lp = l_ref(DOT.l);
		} else if (*s == '$') { /* '$' means the last line */
			s++;
			status = gotoeob(TRUE,1);
			if (status) lp = l_ref(DOT.l);
		} else if (isdigit(*s)) {
			/* digit means an absolute line number */
			for (num = 0; isdigit(*s); s++) {
				num = num * 10 + *s - '0';
			}
			status = gotoline(TRUE,num);
			if (status) lp = l_ref(DOT.l);
		} else if (*s == '\'') {
			/* apostrophe means go to a set mark */
			s++;
			status = gonmmark(*s);
			if (status) lp = l_ref(DOT.l);
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
				lp = l_ref(DOT.l);
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
				lp = l_ref(DOT.l);
		}
#if PATTERNS
		else if (*s == '/' || *s == '?') { /* slash means do a search */
			/* put a null at the end of the search pattern */
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
			*markptr = l_ptr(lp);
			swapmark();
			return s;
		}

		/* maybe add an offset */
		t = s;
		if (*t == '-' || *t == '+') {
			s++;
			for (num = 0; isdigit(*s); s++) {
				num = num * 10 + *s - '0';
			}
			if (num == 0)
				num = 1;
			forwline(TRUE, (*t == '+') ? num : -num);
			lp = l_ref(DOT.l);
		}
	} while (*s == ';' || *s == '+' || *s == '-');

	*markptr = l_ptr(lp);
	swapmark();
	return s;
}

/* parse an ex-style line range -- code culled from elvis, file ex.c, by
	Steve Kirkendall
*/
static int
rangespec(specp, fromlinep, tolinep, flagp)
char		*specp;		/* string containing a line range */
LINEPTR		*fromlinep;	/* first linespec */
LINEPTR		*tolinep;	/* second linespec */
CMDFLAGS	*flagp;
{
	register char	*scan;		/* used to scan thru specp */
	LINEPTR		fromline;	/* first linespec */
	LINEPTR		toline;		/* second linespec */

	*flagp = 0;

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
		fromline = toline = buf_head(curbp); /* _very_ top of buffer */
		*flagp |= (FROM|ZERO);
		scan++;
	} else if (*scan == '%') {
		/* '%' means all lines */
		fromline = lFORW(buf_head(curbp));
		toline = lBACK(buf_head(curbp));
		scan++;
		*flagp |= (FROM|TO);
	} else {
		scan = linespec(scan, &fromline);
		*flagp |= FROM;
		if (same_ptr(fromline, null_ptr))
			fromline = DOT.l;
		toline = fromline;
		if (*scan == ',') {
			scan++;
			scan = linespec(scan, &toline);
			*flagp |= TO;
		}
		if (same_ptr(toline,null_ptr)) {
			/* faulty line spec */
			return FALSE;
		}
	}

	if (is_empty_buf(curbp))
		fromline = toline = null_ptr;

	if (scan == specp)
		*flagp |= DFLALL;

	/* skip whitespace */
	while (isspace(*scan))
		scan++;

	if (*scan) {
		/* dbgwrite("crud at end %s (%s)",specp, scan); */
		return FALSE;
	}

	*fromlinep = fromline;
	*tolinep = toline;

	return TRUE;
}

#ifdef UNUSED
/* old namedcmd:	execute a named command even if it is not bound */
int
onamedcmd(f, n)
int f, n;	/* command arguments [passed through to command executed] */
{
	register char *fnp;	/* ptr to the name of the cmd to exec */
	int s;
	char cmd[NLINE];

	/* get the function name to execute */

	fnp = kbd_engl(": ", cmd);


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
#endif

#if NEVER
/*	execcmd:	Execute a command line command by name alone */
int
execcmd(f, n)
int f, n;	/* default Flag and Numeric argument */
{
	register int status;		/* status return */
	char cmdbuf[NSTRING];		/* string holding command to execute */

	/* get the line wanted */
	cmdbuf[0] = EOS;
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

	do {
		if ((status = macarg(tkn)) != TRUE) {	/* and grab the first token */
			execstr = oldestr;
			return status;
		}
		if (*tkn == ':') {	/* allow leading ':' on line */
			register int j;
			for (j = 0; (tkn[j] = tkn[j+1]) != EOS; j++)
				;
		}
	} while (!*tkn);

	/* process leading argument */
	if (toktyp(tkn) != TKCMD) {
		f = TRUE;
		n = atoi(strcpy(tkn, tokval(tkn)));

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
	/* flag the first time through for some commands -- e.g. subst
		must know to not prompt for strings again, and pregion
		must only restart the p-lines buffer once for each
		command. */
	calledbefore = FALSE;
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
	register int status;
	register long flags;
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
		/* don't record non-redoable cmds, */
		/* but if we're in insertmode, it's okay, since we must
			be executing a function key, like an arrow key,
			that the user will want to have replayed later */
		if (!insertmode && (flags & REDO) == 0)
			dotcmdstop();
		if (flags & UNDO) {
			/* undoable command can't be permitted when read-only */
			if (!(flags & VIEWOK)) {
				if (b_val(curbp,MDVIEW))
					return rdonly();
#ifdef MDCHK_MODTIME
				if (!b_is_changed(curbp) &&
					!ask_shouldchange(curbp))
					return FALSE;
#endif
			}
			if (!kbd_replaying(FALSE))
				mayneedundo();
		}
	}

	if (dotcmdmode != PLAY) {
		extern CMDFUNC f_dotcmdplay;
		if (execfunc != &f_dotcmdplay) {
			/* reset dotcmdkreg on any command where ukb is
			 * unspecified.  usekreg() does it on the one's
			 * where it is specified.  */
			if (ukb == 0)
			    dotcmdkreg = 0;

			/* override the saved dot-cmd argument, if this
				is a new redoable command */
			if (flags & REDO)
			    dotcmdarg = FALSE;
		}
	} else {
		/* if we _are_ playing, re-use the previously kreg */
		if (dotcmdkreg != 0)
			ukb = dotcmdkreg;
	}

	/* if motion is absolute, remember where we are */
	if (flags & ABSM) {
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
	if ((flags & ABSM) && !doingopcmd && !sameline(DOT, odot)) {
		curwp->w_lastdot = odot;
	}


	return status;
}

/* token:	chop a token off a string
		return a pointer past the token
*/

char *
token(src, tok, eolchar)
char *src, *tok;	/* source string, destination token string */
int eolchar;
{
	register int quotef = EOS;	/* nonzero iff the current string quoted */
	register int c, i, d;

	/* first scan past any whitespace in the source string */
	while (isspace(*src))
		++src;

	/* scan through the source string */
	while ((c = *src) != EOS) {
		/* process special characters */
		if (c == '\\') {
			src++;
			if (*src == EOS)
				break;
			switch (c = *src++) {
				case 'r':	*tok++ = '\r'; break;
				case 'n':	*tok++ = '\n'; break;
				case 't':	*tok++ = '\t';  break;
				case 'b':	*tok++ = '\b';  break;
				case 'f':	*tok++ = '\f'; break;
				case 'e':	*tok++ = ESC; break;

				case 'x':
				case 'X':
					i = 2; /* allow \xNN hex */
					c = 0;
					while (isalnum(*src) && i--) {
						if (isdigit(*src)) {
							d = *src - '0';
						} else if (islower(*src)) {
							d = *src - 'a' + 10;
						} else {
							d = *src - 'A' + 10;
						}
						if (d > 15)
							break;
						c = (c * 16) + d;
						src++;
					}
					*tok++ = c;
					break;

				default:
					if (c >= '0' && c <= '7') {
						i = 2; /* allow \NNN octal */
						c -= '0';
						while (isdigit(*src)
						  && *src < '8'
						  && i--) {
							c = (c * 8) + (*src++ - '0');
						}
					}
					*tok++ = c;
			}
		} else {
			/* check for the end of the token */
			if (quotef != EOS) {
				if (c == quotef) {
					src++;
					break;
				}
			} else {
				if (c == eolchar) {
					if (!isspace(c))
						src++;
					break;
				} else if (c == '"') {
					quotef = c;
					/* note that leading quote
						is included */
				} else if (isspace(c)) {
					break;
				}
			}

			*tok++ = *src++;	/* record the character */
		}
	}

	*tok = EOS;
	return src;
}

/*
 * Convert the string 'src' into a string that we can read back with 'token()'. 
 * If it is a shell-command, this will be a single-token.  Repeated shift
 * commands are multiple tokens.
 */
int
macroize(p, src, ref)
TBUFF	**p;
char *	src;
char *	ref;
{
	register int	c;
	int	multi	= !isShellOrPipe(ref);	/* shift command? */
	int	count	= 0;

	if (tb_init(p, abortc) != 0) {
		(void)tb_append(p, '"');
		while ((c = *src++) != EOS) {
			if (multi && count++)
				(void)tb_sappend(p, "\" \"");
			if (c == '\\' || c == '"')
				(void)tb_append(p, '\\');
			(void)tb_append(p, c);
		}
		(void)tb_append(p, '"');
		return (tb_append(p, EOS) != 0);
	}
	return FALSE;
}

int
macarg(tok)	/* get a macro line argument */
char *tok;	/* buffer to place argument */
{
	int savcle;	/* buffer to store original clexec */

	savcle = clexec;	/* save execution mode */
	clexec = TRUE;		/* get the argument */
	/* grab token and advance past */
	execstr = token(execstr, tok, EOS);
	/* evaluate it */
	(void)strcpy(tok, tokval(tok));
	clexec = savcle;	/* restore execution mode */
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
	(void)lsprintf(bname, ScratchName(Macro %d), n);

	/* set up the new macro buffer */
	if ((bp = bfind(bname, BFINVS)) == NULL) {
		mlforce("[Cannot create macro]");
		return FALSE;
	}

	/* and make sure it is empty */
	if (!bclear(bp))
		return FALSE;

	set_rdonly(bp, bp->b_fname);

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
	char bname[NBUFN+1];		/* name of buffer to use */

	/* a numeric argument means its a numbered macro */
	if (f == TRUE)
		return storemac(f, n);

	/* get the name of the procedure */
	bname[1] = EOS;
	if ((status = mlreply("Procedure name: ", bname+1, (int)sizeof(bname)-2)) != TRUE)
		return status;

	/* construct the macro buffer name */
	bname[0] = SCRTCH_LEFT[0];
	(void)strcat(bname, SCRTCH_RIGHT);

	/* set up the new macro buffer */
	if ((bp = bfind(bname, BFINVS)) == NULL) {
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
	static char obufn[NBUFN-1];	/* name of buffer to execute */
	char bufn[NBUFN+1];		/* name of buffer to execute */
	register int odiscmd;

	/* find out what buffer the user wants to execute */
	if ((status = mlreply("Execute procedure: ", obufn, sizeof(obufn))) != TRUE)
		return status;

	/* construct the buffer name */
	bufn[0] = SCRTCH_LEFT[0];
	(void)strcat(strcpy(&bufn[1], obufn), SCRTCH_RIGHT);

	/* find the pointer to that buffer */
	if ((bp = find_b_name(bufn)) == NULL) {
		mlforce("[No such procedure \"%s\"]",bufn);
		return FALSE;
	}

	if (!f)
		n = 1;

	odiscmd = discmd;
	discmd = FALSE;
	status = TRUE;

	/* and now execute it as asked */
	while (n-- > 0 && status == TRUE)
		status = dobuf(bp);

	discmd = odiscmd;
	return status;
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
	static char bufn[NBUFN+1];	/* name of buffer to execute */
	register int odiscmd;

	if (!f)
		n = 1;

	/* find out what buffer the user wants to execute */
	if ((status = mlreply("Execute buffer: ", bufn, sizeof(bufn))) != TRUE)
		return status;

	/* find the pointer to that buffer */
	if ((bp = find_b_name(bufn)) == NULL) {
		mlforce("[No such buffer \"%s\"]",bufn);
		return FALSE;
	}

	odiscmd = discmd;
	discmd = FALSE;
	status = TRUE;
	/* and now execute it as asked */
	while (n-- > 0 && status == TRUE)
		status = dobuf(bp);

	discmd = odiscmd;

	return status;
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

static void
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
	fast_ptr LINEPTR lp;	/* pointer to line to execute */
	fast_ptr LINEPTR hlp;	/* pointer to line header */
	int dirnum;		/* directive index */
	SIZE_T linlen;		/* length of line to execute */
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

	static int dobufnesting;
	static BUFFER *dobuferrbp;

	if (dobufnesting == 0) {
		dobuferrbp = NULL;
	}

	if (++dobufnesting > 9) {
		dobufnesting--;
		return FALSE;
	}


	/* clear IF level flags/while ptr */
	execlevel = 0;
	whlist = NULL;
	regionshape = EXACT;
	havemotion = NULL;

#if ! SMALLER
	scanpt = NULL;
	/* scan the buffer to execute, building WHILE header blocks */
	hlp = buf_head(bp);
	lp = lFORW(hlp);
	bp->b_dot.o = 0;
	while (!same_ptr(lp, hlp)) {
		int i;			/* index */

		bp->b_dot.l = lp;
		/* scan the current line */
		eline = l_ref(lp)->l_text;
		i = l_ref(lp)->l_used;

		/* trim leading whitespace */
		while (i-- > 0 && isblank(*eline))
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
					get_bname(bp));
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
		lp = lFORW(lp);
	}

	/* while and endwhile should match! */
	if (scanpt != NULL) {
		mlforce("[WHILE with no matching ENDWHILE in '%s']",
			get_bname(bp));
		goto failexit;
	}
#endif

	/* starting at the beginning of the buffer */
	hlp = buf_head(bp);
	lp = lFORW(hlp);
	while (!same_ptr(lp, hlp)) {
		bp->b_dot.l = lp;
		/* allocate eline and copy macro line to it */
		linlen = l_ref(lp)->l_used;
		if ((einit = eline = castalloc(char, linlen+1)) == NULL) {
			mlforce("[Out of Memory during macro execution]");
			freewhile(whlist);
			mstore = FALSE;
			dobufnesting--;
			return FALSE;
		}

		/* pjr - must check for empty line before copy */
		if (linlen)
			(void)strncpy(eline, l_ref(lp)->l_text, linlen);
		eline[linlen] = EOS;	/* make sure it ends */

		/* trim leading whitespace */
		while (isblank(*eline))
			++eline;

		/* dump comments and blank lines */
		if (*eline == ';' || *eline == EOS)
			goto onward;

#if	DEBUGM
		/* if $debug == TRUE, every line to execute
		   gets echoed and a key needs to be pressed to continue
		   ^G will abort the command */

		if (macbug) {
			char	outline[NLINE];
			(void)strcpy(outline, "<<<");

			/* debug macro name */
			(void)strcat(outline, get_bname(bp));
			(void)strcat(outline, ":");

			/* debug if levels */
			(void)strcat(outline, l_itoa(execlevel));
			(void)strcat(outline, ":");

			/* and lastly the line */
			(void)strcat(outline, eline);
			(void)strcat(outline, ">>>");

			/* write out the debug line */
			mlforce("%s",outline);
			(void)update(TRUE);

			/* and get the keystroke */
			if (tgetc() == abortc) {
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
				bstore->b_dot.l = lFORW(buf_head(bstore));
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
			if (addline(bstore, eline, -1) == FALSE) {
				mlforce("[Out of memory while storing macro]");
				mstore = FALSE;
				dobufnesting--;
				return FALSE;
			}
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
			while (*eline && !isblank(*eline))
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
					if (same_ptr(whtemp->w_begin, lp))
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
					(void) token(eline, golabel, EOS);
					linlen = strlen(golabel);
					glp = lForw(hlp);
					while (glp != l_ref(hlp)) {
						if (glp->l_text[0] == '*' &&
						    (strncmp(&glp->l_text[1], golabel,
							    linlen) == 0)) {
							lp = l_ptr(glp);
							goto onward;
						}
						glp = lforw(glp);
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
						    same_ptr(whtemp->w_end, lp))
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
					lp = lBACK(whtemp->w_begin);
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
			for_each_window(wp) {
				if (wp->w_bufp == bp) {
					/* and point it */
					wp->w_dot.l = lp;
					wp->w_dot.o = 0;
					wp->w_flag |= WFHARD;
				}
			}
			/* in any case set the buffer's dot */
			bp->b_dot.l = lp;
			bp->b_dot.o = 0;
			bp->b_wline.l = lFORW(buf_head(bp));
			free(einit);
			execlevel = 0;
			mstore = FALSE;
			freewhile(whlist);
			dobufnesting--;
			if (dobuferrbp == NULL) {
				dobuferrbp = bp;
				(void)swbuffer(bp);
				TTbeep();
			}
			return status;
		}

onward:		/* on to the next line */
		free(einit);
		lp = lFORW(lp);
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
	char fname[NFILEN];	/* name of file to execute */
	char *fspec;		/* full file spec */
	static	TBUFF	*last;

	if ((status = mlreply_file("File to execute: ", &last, FILEC_READ, fname)) != TRUE)
		return status;

	/* look up the path for the file */
	fspec = flook(fname, FL_ANYWHERE);

	/* if it isn't around */
	if (fspec == NULL)
		return no_such_file(fname);

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
	char bname[NBUFN+1];	/* name of buffer */

	makename(bname, fname);	/* derive the name of the buffer */
	(void)unqname(bname, FALSE);

	/* okay, we've got a unique name -- create it */
	if ((bp=bfind(bname, 0))==NULL) {
		return FALSE;
	}

	/* and try to read in the file to execute */
	if ((status = readin(fname, FALSE, bp, TRUE)) == TRUE) {

		/* go execute it! */
		odiscmd = discmd;
		discmd = FALSE;
		status = dobuf(bp);
		discmd = odiscmd;

		/*
		 * If no errors occurred, and if the buffer isn't displayed,
		 * remove it.
		 */
		if (status != TRUE)
			(void)swbuffer(bp);
		else if (bp->b_nwnd == 0)
			(void)zotbuf(bp);
	}
	return status;
}

static	int	cbuf P(( int, int, int ));

/*	cbuf:	Execute the contents of a numbered buffer	*/

static int
cbuf(f, n, bufnum)
int f, n;	/* default flag and numeric arg */
int bufnum;	/* number of buffer to execute */
{
	register BUFFER *bp;		/* ptr to buffer to execute */
	register int status;		/* status return */
	static char bufname[NBUFN];
	register int odiscmd;

	if (!f) n = 1;

	/* make the buffer name */
	(void)lsprintf(bufname, ScratchName(Macro %d), bufnum);

	/* find the pointer to that buffer */
	if ((bp = find_b_name(bufname)) == NULL) {
		mlforce("[Macro not defined]");
		return FALSE;
	}

	odiscmd = discmd;
	discmd = FALSE;
	status = TRUE;
	/* and now execute it as asked */
	while (n-- > 0 && status == TRUE)
		status = dobuf(bp);

	discmd = odiscmd;
	return status;

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

#if !SMALLER
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

#endif /* !SMALLER */
