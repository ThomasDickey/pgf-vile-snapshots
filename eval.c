/*	EVAL.C:	Expression evaluation functions for
		MicroEMACS

	written 1986 by Daniel Lawrence
 *
 * $Log: eval.c,v $
 * Revision 1.49  1993/04/01 13:06:31  pgf
 * turbo C support (mostly prototypes for static)
 *
 * Revision 1.48  1993/03/29  11:18:15  pgf
 * $pagewid was returning length instead of width
 *
 * Revision 1.47  1993/03/16  10:53:21  pgf
 * see 3.36 section of CHANGES file
 *
 * Revision 1.46  1993/03/05  17:50:54  pgf
 * see CHANGES, 3.35 section
 *
 * Revision 1.45  1993/02/24  10:59:02  pgf
 * see 3.34 changes, in CHANGES file
 *
 * Revision 1.44  1993/02/16  20:52:49  pgf
 * eliminate putenv call, which isn't always available.  real vi doesn't
 * push the "shell" variable to the environment anyway.
 *
 * Revision 1.43  1993/02/08  14:53:35  pgf
 * see CHANGES, 3.32 section
 *
 * Revision 1.42  1993/01/23  13:38:23  foxharp
 * now include nevars.h, which is auto-created by mktbls
 *
 * Revision 1.41  1993/01/16  10:30:58  foxharp
 * makevarslist() and listvars()
 *
 * Revision 1.40  1992/12/14  09:03:25  foxharp
 * lint cleanup, mostly malloc
 *
 * Revision 1.39  1992/11/19  09:03:56  foxharp
 * fix identification of single character tokens that happen to be prefixes
 * for special constructs, like '~', '*', etc.
 *
 * Revision 1.38  1992/08/20  23:40:48  foxharp
 * typo fixes -- thanks, eric
 *
 * Revision 1.37  1992/07/24  18:22:51  foxharp
 * deleted local atoi() routine -- now we use the system's copy
 *
 * Revision 1.36  1992/07/13  20:09:57  foxharp
 * "terse" is no longer a variable
 *
 * Revision 1.35  1992/07/13  09:26:16  foxharp
 * added "force" argument to current_directory()
 *
 * Revision 1.34  1992/06/25  23:00:50  foxharp
 * changes for dos/ibmpc
 *
 * Revision 1.33  1992/05/19  08:55:44  foxharp
 * more prototype and shadowed decl fixups
 *
 * Revision 1.32  1992/05/16  12:00:31  pgf
 * prototypes/ansi/void-int stuff/microsoftC
 *
 * Revision 1.30  1992/04/10  18:48:17  pgf
 * change abs to absol to get rid of name conflicts
 *
 * Revision 1.29  1992/03/24  09:02:18  pgf
 * need to glob() filenames for &rd and &wr
 *
 * Revision 1.28  1992/03/24  07:36:23  pgf
 * added &rd and &wr functions, for file access, and fixed off-by-one in $curcol
 *
 * Revision 1.27  1992/03/19  23:19:55  pgf
 * linux prototyped portability
 *
 * Revision 1.26  1992/03/19  23:07:47  pgf
 * variable access cleanup/rearrangement
 *
 * Revision 1.25  1992/03/05  09:17:21  pgf
 * added support for new "terse" variable, to control unnecessary messages
 *
 * Revision 1.24  1992/03/03  09:35:52  pgf
 * added support for getting "words" out of the buffer via variables --
 * needed _nonspace character type
 *
 * Revision 1.23  1992/02/17  09:01:16  pgf
 * took out unused vars for saber
 *
 * Revision 1.22  1992/01/05  00:06:13  pgf
 * split mlwrite into mlwrite/mlprompt/mlforce to make errors visible more
 * often.  also normalized message appearance somewhat.
 *
 * Revision 1.21  1992/01/03  23:31:49  pgf
 * use new ch_fname() to manipulate filenames, since b_fname is now
 * a malloc'ed sting, to avoid length limits
 *
 * Revision 1.20  1991/12/24  09:18:47  pgf
 * added current/change directory support  (Dave Lemke's changes)
 *
 * Revision 1.19  1991/11/27  10:09:09  pgf
 * bug fix, from pete
 *
 * Revision 1.18  1991/11/13  20:09:27  pgf
 * X11 changes, from dave lemke
 *
 * Revision 1.17  1991/11/08  13:19:01  pgf
 * lint cleanup
 *
 * Revision 1.16  1991/11/04  14:18:09  pgf
 * use lsprintf in itoa
 *
 * Revision 1.15  1991/11/03  17:36:13  pgf
 * picky saber change
 *
 * Revision 1.14  1991/11/01  14:38:00  pgf
 * saber cleanup
 *
 * Revision 1.13  1991/10/28  14:25:06  pgf
 * eliminated some variables that are now buffer-values
 *
 * Revision 1.12  1991/10/24  13:05:52  pgf
 * conversion to new regex package -- much faster
 *
 * Revision 1.11  1991/10/22  14:10:07  pgf
 * more portable #if --> #ifdef
 *
 * Revision 1.10  1991/09/26  13:12:04  pgf
 * new arg. to kbd_string to enable backslash processing
 *
 * Revision 1.9  1991/09/16  23:46:55  pgf
 * more hardening
 *
 * Revision 1.8  1991/09/13  03:27:06  pgf
 * attempt to harden against bad variable names (like lone %)
 *
 * Revision 1.7  1991/08/07  12:35:07  pgf
 * added RCS log messages
 *
 * revision 1.6
 * date: 1991/08/06 15:13:27;
 * global/local values
 * 
 * revision 1.5
 * date: 1991/06/25 19:52:23;
 * massive data structure restructure
 * 
 * revision 1.4
 * date: 1991/06/03 10:19:11;
 * newscreensize() is now named newlength()
 * 
 * revision 1.3
 * date: 1990/10/01 11:05:54;
 * progname --> prognam
 * 
 * revision 1.2
 * date: 1990/09/25 11:38:13;
 * took out old ifdef BEFORE code
 * 
 * revision 1.1
 * date: 1990/09/21 10:25:11;
 * initial vile RCS revision
 */

#include	"estruct.h"
#include	"edef.h"
#include	"nevars.h"

#if ! SMALLER
#if ENVFUNC
static	char	*GetEnv P(( char * ));
#endif
static	void	makevarslist P(( int, char *));
static	char *	gtfun P(( char * ));
static	void	findvar P(( char *, VDESC * ));
#endif
static	int	vars_complete P(( int, char *, int * ));

/*--------------------------------------------------------------------------*/

#if	ENVFUNC && !SMALLER
static
char	*GetEnv(s)
	char	*s;
{
	register char *v = getenv(s);
	return v ? v : "";
}
#else
#define	GetEnv(s)	""
#endif

/* points to current "shell" environment variable, either in malloc'ed
	if user set it, or in environment ($SHELL) otherwise */
static char *shell;

void
varinit()		/* initialize the user variable list */
{
#if ! SMALLER
	register int i;

	for (i=0; i < MAXVARS; i++)
		uv[i].u_name[0] = 0;
#endif
}

#if ! SMALLER
/* list the current vars into the current buffer */
/* ARGSUSED */
static void
makevarslist(dum1,ptr)
int dum1;
char *ptr;
{
	register int i, j;

	bprintf("--- Environment variables %*P\n", term.t_ncol-1, '-');
	bprintf("%s", ptr);
	for (i = j = 0; i < MAXVARS; i++) {
		if (uv[i].u_name[0] != 0) {
			if (!j++)
				bprintf("--- User variables %*P\n", term.t_ncol-1, '-');
			bprintf("%%%s = %s\n", uv[i].u_name, uv[i].u_value);
		}
	}
}
#endif

/* ARGSUSED */
int
listvars(f, n)
int f,n;
{
#if ! SMALLER
	char *values;
	register WINDOW *wp = curwp;
	register int s;
	register unsigned t;
	register char *v, *vv;
	static	char *fmt = "$%s = %.*s\n";

	/* collect data for environment-variables, since some depend on window */
	for (s = t = 0; s < NEVARS; s++) {
		vv = gtenv(envars[s]);
		if (vv != errorm)
			t += strlen(envars[s]) + strlen(fmt) + strlen(vv);
	}
	if (!(values = malloc(t)))
		return FALSE;

	for (s = 0, v = values; s < NEVARS; s++) {
		vv = gtenv(envars[s]);
		if (vv != errorm) {
			t = strlen(vv);
			if (t == 0) {
				t = 1;
				vv = "";
			} else if (vv[t-1] == '\n')
				t--;	/* suppress trailing newline */
			(void)sprintf(v, fmt, envars[s], t, vv);
			v += strlen(v);
		}
	}
	s = liststuff(ScratchName(Variables), makevarslist, 0, (char *)values);
	free(values);

	/* back to the buffer whose modes we just listed */
	swbuffer(wp->w_bufp);
	return s;
#endif
}

#if ! SMALLER
static
char *gtfun(fname)	/* evaluate a function */
char *fname;		/* name of function to evaluate */
{
	register int fnum;		/* index to function to eval */
	char arg1[NSTRING];		/* value of first argument */
	char arg2[NSTRING];		/* value of second argument */
	char arg3[NSTRING];		/* value of third argument */
	static char result[2 * NSTRING];	/* string result */

	if (!fname[0])
		return(errorm);

	/* look the function up in the function table */
	fname[3] = 0;	/* only first 3 chars significant */
	(void)mklower(fname);	/* and let it be upper or lower case */
	for (fnum = 0; fnum < NFUNCS; fnum++)
		if (strcmp(fname, funcs[fnum].f_name) == 0)
			break;

	/* return errorm on a bad reference */
	if (fnum == NFUNCS)
		return(errorm);

	/* if needed, retrieve the first argument */
	if (funcs[fnum].f_type >= MONAMIC) {
		if (macarg(arg1) != TRUE)
			return(errorm);

		/* if needed, retrieve the second argument */
		if (funcs[fnum].f_type >= DYNAMIC) {
			if (macarg(arg2) != TRUE)
				return(errorm);
	
			/* if needed, retrieve the third argument */
			if (funcs[fnum].f_type >= TRINAMIC)
				if (macarg(arg3) != TRUE)
					return(errorm);
		}
	}
		

	/* and now evaluate it! */
	switch (fnum) {
		case UFADD:	return(l_itoa(atoi(arg1) + atoi(arg2)));
		case UFSUB:	return(l_itoa(atoi(arg1) - atoi(arg2)));
		case UFTIMES:	return(l_itoa(atoi(arg1) * atoi(arg2)));
		case UFDIV:	return(l_itoa(atoi(arg1) / atoi(arg2)));
		case UFMOD:	return(l_itoa(atoi(arg1) % atoi(arg2)));
		case UFNEG:	return(l_itoa(-atoi(arg1)));
		case UFCAT:	strcpy(result, arg1);
				return(strcat(result, arg2));
		case UFLEFT:	return(strncpy(result, arg1, atoi(arg2)));
		case UFRIGHT:	return(strcpy(result, &arg1[atoi(arg2)-1]));
		case UFMID:	return(strncpy(result, &arg1[atoi(arg2)-1],
					atoi(arg3)));
		case UFNOT:	return(ltos(stol(arg1) == FALSE));
		case UFEQUAL:	return(ltos(atoi(arg1) == atoi(arg2)));
		case UFLESS:	return(ltos(atoi(arg1) < atoi(arg2)));
		case UFGREATER:	return(ltos(atoi(arg1) > atoi(arg2)));
		case UFSEQUAL:	return(ltos(strcmp(arg1, arg2) == 0));
		case UFSLESS:	return(ltos(strcmp(arg1, arg2) < 0));
		case UFSGREAT:	return(ltos(strcmp(arg1, arg2) > 0));
		case UFIND:	return(tokval(arg1));
		case UFAND:	return(ltos(stol(arg1) && stol(arg2)));
		case UFOR:	return(ltos(stol(arg1) || stol(arg2)));
		case UFLENGTH:	return(l_itoa((int)strlen(arg1)));
		case UFUPPER:	return(mkupper(arg1));
		case UFLOWER:	return(mklower(arg1));
				/* patch: why 42? */
		case UFTRUTH:	return(ltos(atoi(arg1) == 42));
		case UFASCII:	return(l_itoa((int)arg1[0]));
		case UFCHR:	result[0] = atoi(arg1);
				result[1] = 0;
				return(result);
		case UFGTKEY:	result[0] = tgetc(TRUE);
				result[1] = 0;
				return(result);
		case UFRND:	return(l_itoa((ernd() % absol(atoi(arg1))) + 1));
		case UFABS:	return(l_itoa(absol(atoi(arg1))));
		case UFSINDEX:	return(l_itoa(sindex(arg1, arg2)));
		case UFENV:	return(GetEnv(arg1));
		case UFBIND:	return(prc2engl(arg1));
		case UFREADABLE:glob(arg1);
				return(ltos(flook(arg1, FL_HERE) != NULL));
		case UFWRITABLE:glob(arg1);
				return(ltos(!ffronly(arg1)));
	}
	return errorm;
}

char *gtusr(vname)	/* look up a user var's value */
char *vname;		/* name of user variable to fetch */
{

	register int vnum;	/* ordinal number of user var */

	if (!vname[0])
		return (errorm);

	/* scan the list looking for the user var name */
	for (vnum = 0; vnum < MAXVARS; vnum++)
		if (strcmp(vname, uv[vnum].u_name) == 0)
			break;

	/* return errorm on a bad reference */
	if (vnum == MAXVARS)
		return(errorm);

	return(uv[vnum].u_value);
}

char *gtenv(vname)
char *vname;		/* name of environment variable to retrieve */
{
	register int vnum;	/* ordinal number of var referenced */
#if X11
	char	*x_current_fontname(); 
#endif

	if (!vname[0])
		return (errorm);

	/* scan the list, looking for the referenced name */
	for (vnum = 0; vnum < NEVARS; vnum++)
		if (strcmp(vname, envars[vnum]) == 0)
			break;

	/* return errorm on a bad reference */
	if (vnum == NEVARS)
		return(errorm);

	/* otherwise, fetch the appropriate value */
	switch (vnum) {
		case EVPAGELEN:	return(l_itoa(term.t_nrow + 1));
		case EVCURCOL:	return(l_itoa(getccol(FALSE) + 1));
		case EVCURLINE: return(l_itoa(getcline()));
		case EVRAM:	return(l_itoa((int)(envram / 1024l)));
		case EVFLICKER:	return(ltos(flickcode));
		case EVCURWIDTH:return(l_itoa(term.t_ncol));
		case EVCBUFNAME:return(curbp->b_bname);
		case EVCFNAME:	return(curbp->b_fname);
		case EVSRES:	return(sres);
		case EVDEBUG:	return(ltos(macbug));
		case EVSTATUS:	return(ltos(cmdstatus));
		case EVPALETTE:	return(palstr);
		case EVLASTKEY: return(l_itoa(lastkey));
		case EVCURCHAR:
			return(is_at_end_of_line(DOT) ? l_itoa('\n') :
				l_itoa(char_at(DOT)));
		case EVDISCMD:	return(ltos(discmd));
		case EVVERSION:	return(version);
		case EVPROGNAME:return(prognam);
		case EVSEED:	return(l_itoa(seed));
		case EVDISINP:	return(ltos(disinp));
		case EVWLINE:	return(l_itoa(curwp->w_ntrows));
		case EVCWLINE:	return(l_itoa(getwpos()));
		case EVSEARCH:	return(pat);
		case EVREPLACE:	return(rpat);
		case EVMATCH:	return((patmatch == NULL)? "": patmatch);
		case EVKILL:	return(getkill());
		case EVTPAUSE:	return(l_itoa(term.t_pause));
		case EVPENDING:
#if	TYPEAH
				return(ltos(typahead()));
#else
				return(falsem);
#endif
		case EVLLENGTH:	return(l_itoa(llength(DOT.l)));
		case EVLINE:	return(getctext(0));
		case EVWORD:	return(getctext(_nonspace));
		case EVIDENTIF:	return(getctext(_ident));
		case EVQIDENTIF:return(getctext(_qident));
		case EVPATHNAME:return(getctext(_pathn));
		case EVDIR:	return(current_directory(FALSE));
#if X11
		case EVFONT:	return(x_current_fontname());
#endif
		case EVSHELL:	if (!shell) {
					shell = GetEnv("SHELL");
					if (!shell)  /* only search once */
						shell = "/bin/sh";
					shell = strmalloc(shell);
				}
				return shell;
	}
	return errorm;
}

char *getkill()		/* return some of the contents of the kill buffer */
{
	register int size;	/* max number of chars to return */
	static char value[NSTRING];	/* temp buffer for value */

	if (kbs[0].kbufh == NULL)
		/* no kill buffer....just a null string */
		value[0] = 0;
	else {
		/* copy in the contents... */
		if (kbs[0].kused < NSTRING)
			size = kbs[0].kused;
		else
			size = NSTRING - 1;
		strncpy(value, (char *)(kbs[0].kbufh->d_chunk), size);
	}

	/* and return the constructed value */
	return(value);
}

static void
findvar(var, vd)	/* find a variables type and name */
char *var;	/* name of var to get */
VDESC *vd;	/* structure to hold type and ptr */
{
	register int vnum;	/* subscript in variable arrays */
	register int vtype;	/* type to return */

fvar:
	vtype = vnum = -1;
	if (!var[1]) {
		vd->v_type = vtype;
		return;
	}
	switch (var[0]) {

		case '$': /* check for legal enviromnent var */
			for (vnum = 0; vnum < NEVARS; vnum++)
				if (strcmp(&var[1], envars[vnum]) == 0) {
					vtype = TKENV;
					break;
				}
			break;

		case '%': /* check for existing legal user variable */
			for (vnum = 0; vnum < MAXVARS; vnum++)
				if (strcmp(&var[1], uv[vnum].u_name) == 0) {
					vtype = TKVAR;
					break;
				}
			if (vnum < MAXVARS)
				break;

			/* create a new one??? */
			for (vnum = 0; vnum < MAXVARS; vnum++)
				if (uv[vnum].u_name[0] == 0) {
					vtype = TKVAR;
					strcpy(uv[vnum].u_name, &var[1]);
					break;
				}
			break;

		case '&':	/* indirect operator? */
			var[4] = 0;
			if (strcmp(&var[1], "ind") == 0) {
				/* grab token, and eval it */
				execstr = token(execstr, var, EOS);
				strcpy(var, tokval(var));
				goto fvar;
			}
	}

	/* return the results */
	vd->v_num = vnum;
	vd->v_type = vtype;
}

/*
 * Handles name-completion for variables.
 */
static int
vars_complete(c, buf, pos)
int	c;
char	*buf;
int	*pos;
{
	if (buf[0] == '$') {
		int	cpos = *pos;
		*pos -= 1;	/* account for leading '$', not in tables */
		if (kbd_complete(c, buf+1, pos, (char *)&envars[0], sizeof(envars[0])))
			*pos += 1;
		else {
			*pos = cpos;
			return FALSE;
		}
	}
	return TRUE;
}

int 
setvar(f, n)		/* set a variable */
int f;		/* default flag */
int n;		/* numeric arg (can overide prompted value) */
{
	register int status;	/* status return */
	VDESC vd;		/* variable num/type */
	static char var[NLINE+3];	/* name of variable to fetch */
	char prompt[NLINE];
	char value[NLINE];	/* value to set variable to */

	/* first get the variable to set.. */
	status = kbd_string("Variable to set: ",
		var, NLINE,
		'=', KBD_NOEVAL|KBD_LOWERC, vars_complete);
	if (status != TRUE)
		return(status);

	/* check the legality and find the var */
	findvar(var, &vd);
	
	/* if its not legal....bitch */
	if (vd.v_type == -1) {
		mlforce("[No such variable as '%s']", var);
		return(FALSE);
	}

	/* get the value for that variable */
	if (f == TRUE)
		strcpy(value, l_itoa(n));
	else {
		value[0] = EOS;
		(void)lsprintf(prompt, "Value of %s: ", var);
		status = mlreply(prompt, value, sizeof(value));
		if (status != TRUE)
			return(status);
	}

	/* and set the appropriate value */
	status = svar(&vd, value);

#if	DEBUGM
	/* if $debug == TRUE, every assignment will echo a statment to
	   that effect here. */
	
	if (macbug) {
		strcpy(outline, "(((");

		/* assignment status */
		strcat(outline, ltos(status));
		strcat(outline, ":");

		/* variable name */
		strcat(outline, var);
		strcat(outline, ":");

		/* and lastly the value we tried to assign */
		strcat(outline, value);
		strcat(outline, ")))");


		/* write out the debug line */
		mlforce("%s",outline);
		update(TRUE);

		/* and get the keystroke to hold the output */
		if (kbd_key() == abortc) {
			mlforce("[Macro aborted]");
			status = FALSE;
		}
	}
#endif

	/* and return it */
	return(status);
}

int svar(var, value)		/* set a variable */
VDESC *var;	/* variable to set */
char *value;	/* value to set to */
{
	register int vnum;	/* ordinal number of var referenced */
	register int vtype;	/* type of variable to set */
	register int status;	/* status return */
	register int c;		/* translated character */
	register char * sp;	/* scratch string pointer */

	/* simplify the vd structure (we are gonna look at it a lot) */
	vnum = var->v_num;
	vtype = var->v_type;

	/* and set the appropriate value */
	status = TRUE;
	switch (vtype) {
	case TKVAR: /* set a user variable */
		if (uv[vnum].u_value != NULL)
			free(uv[vnum].u_value);
		sp = castalloc(char, strlen(value) + 1);
		if (sp == NULL)
			return(FALSE);
		strcpy(sp, value);
		uv[vnum].u_value = sp;
		break;

	case TKENV: /* set an environment variable */
		status = TRUE;	/* by default */
		switch (vnum) {
		case EVCURCOL:	status = gotocol(TRUE,atoi(value));
				break;
		case EVCURLINE:	status = gotoline(TRUE, atoi(value));
				break;
		case EVFLICKER:	flickcode = stol(value);
				break;
		case EVCURWIDTH:status = newwidth(TRUE, atoi(value));
				break;
		case EVPAGELEN:	status = newlength(TRUE,atoi(value));
				break;
		case EVCBUFNAME:strcpy(curbp->b_bname, value);
				curwp->w_flag |= WFMODE;
				break;
		case EVCFNAME:	ch_fname(curbp, value);
				curwp->w_flag |= WFMODE;
				break;
		case EVSRES:	status = TTrez(atoi(value));
				break;
		case EVDEBUG:	macbug = stol(value);
				break;
		case EVSTATUS:	cmdstatus = stol(value);
				break;
		case EVPALETTE:	strncpy(palstr, value, 48);
				spal(palstr);
				break;
		case EVLASTKEY:	lastkey = atoi(value);
				break;
		case EVCURCHAR:	ldelete(1L, FALSE);	/* delete 1 char */
				c = atoi(value);
				if (c == '\n')
					lnewline();
				else
					linsert(1, c);
				backchar(FALSE, 1);
				break;
		case EVDISCMD:	discmd = stol(value);
				break;
		case EVSEED:	seed = atoi(value);
				break;
		case EVDISINP:	disinp = stol(value);
				break;
		case EVWLINE:	status = resize(TRUE, atoi(value));
				break;
		case EVCWLINE:	status = forwline(TRUE,
						atoi(value) - getwpos());
				break;
		case EVSEARCH:	strcpy(pat, value);
				if (gregexp) free((char *)gregexp);
				gregexp = regcomp(pat, b_val(curbp, MDMAGIC));
				break;
		case EVREPLACE:	strcpy(rpat, value);
				break;
		case EVTPAUSE:	term.t_pause = atoi(value);
				break;
		case EVPROGNAME:
		case EVVERSION:
		case EVMATCH:
		case EVKILL:
		case EVPENDING:
		case EVLLENGTH:
		case EVRAM:	break;
		case EVLINE:	(void)putctext(value);
				break;
		case EVDIR:	status = set_directory(value);
				break;
#if X11
		case EVFONT:	status = x_setfont(value);
				break;
#endif
		case EVSHELL:	if (shell)
					free(shell);
				shell = strmalloc(value);
				break;
		}
		break;
	}
	return(status);
}
#endif


#if ! SMALLER

/*	l_itoa:	integer to ascii string.......... This is too
		inconsistent to use the system's	*/

char *l_itoa(i)
int i;	/* integer to translate to a string */
{
	static char result[INTWIDTH+1];	/* resulting string */
	lsprintf(result,"%d",i);
	return result;
}
#endif

int toktyp(tokn)	/* find the type of a passed token */
char *tokn;	/* token to analyze */
{

	/* no blanks!!! */
	if (tokn[0] == '\0')
		return(TKNUL);

	/* a numeric literal? */
	if (isdigit(tokn[0]))
		return(TKLIT);

	if (tokn[0] == '"')
		return(TKSTR);

	/* if it's any other single char, it must be itself */
	if (tokn[1] == '\0')
		return(TKCMD);

#if ! SMALLER
	switch (tokn[0]) {
		case '"':	return(TKSTR);

		case '~':	return(TKDIR);
		case '@':	return(TKARG);
		case '#':	return(TKBUF);
		case '$':	return(TKENV);
		case '%':	return(TKVAR);
		case '&':	return(TKFUN);
		case '*':	return(TKLBL);

		default:	return(TKCMD);
	}
#else
	return(TKCMD);
#endif
}

char *
tokval(tokn)	/* find the value of a token */
char *tokn;		/* token to evaluate */
{
#if ! SMALLER
	register int status;	/* error return */
	register BUFFER *bp;	/* temp buffer pointer */
	register int blen;	/* length of buffer argument */
	register int distmp;	/* temporary discmd flag */
	int	oclexec;
	static char buf[NSTRING];/* string buffer for some returns */

	switch (toktyp(tokn)) {
		case TKNUL:	return("");

		case TKARG:	/* interactive argument */
				oclexec = clexec;

				/* clexec = FALSE; * in cline execution - added pjr */

				strcpy(tokn, tokval(&tokn[1]));
				distmp = discmd;	/* echo it always! */
				discmd = TRUE;
				clexec = FALSE;
				buf[0] = 0;
				status = kbd_string(tokn, buf, sizeof(buf), '\n', KBD_EXPAND|KBD_QUOTES,no_completion);
				discmd = distmp;
				clexec = oclexec;

				if (status == ABORT)
					return(errorm);
				return(buf);

		case TKBUF:	/* buffer contents fetch */

				/* grab the right buffer */
				strcpy(tokn, tokval(&tokn[1]));
				bp = bfind(tokn, NO_CREAT, 0);
				if (bp == NULL)
					return(errorm);
		
				/* if the buffer is displayed, get the window
				   vars instead of the buffer vars */
				if (bp->b_nwnd > 0) {
					curbp->b_dot = curwp->w_dot;
				}

				/* make sure we are not at the end */
				if (is_header_line(bp->b_dot,bp))
					return(errorm);
		
				/* grab the line as an argument */
				blen = llength(bp->b_dot.l) - bp->b_dot.o;
				if (blen > NSTRING)
					blen = NSTRING;
				strncpy(buf, bp->b_dot.l->l_text + bp->b_dot.o,
					blen);
				buf[blen] = 0;
		
				/* and step the buffer's line ptr
					ahead a line */
				bp->b_dot.l = lforw(bp->b_dot.l);
				bp->b_dot.o = 0;

				/* if displayed buffer, reset window ptr vars*/
				if (bp->b_nwnd > 0) {
					curwp->w_dot.l = curbp->b_dot.l;
					curwp->w_dot.o = 0;
					curwp->w_flag |= WFMOVE;
				}

				/* and return the spoils */
				return(buf);		

		case TKVAR:	return(gtusr(tokn+1));
		case TKENV:	return(gtenv(tokn+1));
		case TKFUN:	return(gtfun(tokn+1));
		case TKDIR:	return(errorm);
		case TKLBL:	return(l_itoa(gtlbl(tokn)));
		case TKLIT:	return(tokn);
		case TKSTR:	return(tokn+1);
		case TKCMD:	return(tokn);
	}
	return errorm;
#else
	return tokn;
#endif
}

#if ! SMALLER

/* ARGSUSED */
int
gtlbl(tokn)	/* find the line number of the given label */
char *tokn;	/* label name to find */
{
	return(1);
}

int stol(val)	/* convert a string to a numeric logical */
char *val;	/* value to check for stol */
{
	/* check for logical values */
	if (val[0] == 'F' || val[0] == 'f')
		return(FALSE);
	if (val[0] == 'T' || val[0] == 't')
		return(TRUE);

	/* check for numeric truth (!= 0) */
	return((atoi(val) != 0));
}

char *ltos(val)		/* numeric logical to string logical */
int val;	/* value to translate */
{
	if (val)
		return(truem);
	else
		return(falsem);
}

char *mkupper(str)	/* make a string upper case */
char *str;		/* string to upper case */
{
	char *sp;

	sp = str;
	while (*sp) {
		if (islower(*sp))
			*sp += 'A' - 'a';
		++sp;
	}
	return(str);
}
#endif

#if ! SMALLER || MSDOS

char *mklower(str)	/* make a string lower case */
char *str;		/* string to lower case */
{
	char *sp;

	sp = str;
	while (*sp) {
		if (isupper(*sp))
			*sp += 'a' - 'A';
		++sp;
	}
	return(str);
}
#endif

int absol(x)	/* take the absolute value of an integer */
int x;
{
	return(x < 0 ? -x : x);
}

#if ! SMALLER
int ernd()	/* returns a random integer */
{
	seed = absol(seed * 1721 + 10007);
	return(seed);
}

int sindex(sourc, pattern)	/* find pattern within source */
char *sourc;	/* source string to search */
char *pattern;	/* string to look for */
{
	char *sp;	/* ptr to current position to scan */
	char *csp;	/* ptr to source string during comparison */
	char *cp;	/* ptr to place to check for equality */

	/* scanning through the source string */
	sp = sourc;
	while (*sp) {
		/* scan through the pattern */
		cp = pattern;
		csp = sp;
		while (*cp) {
			if (!eq(*cp, *csp))
				break;
			++cp;
			++csp;
		}

		/* was it a match? */
		if (*cp == 0)
			return((int)(sp - sourc) + 1);
		++sp;
	}

	/* no match at all.. */
	return(0);
}

#endif

#if NO_LEAKS
void ev_leaks()
{
	if (shell) {
		free(shell);
		shell = 0;
	}
}
#endif
