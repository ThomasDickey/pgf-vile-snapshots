/* 
 *
 *	modes.c
 *
 * Maintain and list the editor modes and value sets.
 *
 * Original code probably by Dan Lawrence or Dave Conroy for MicroEMACS.
 * Major extensions for vile by Paul Fox, 1991
 *
 *	$Log: modes.c,v $
 *	Revision 1.23  1993/08/18 15:10:36  pgf
 *	don't let the OPT_XTERM code do anything under X11
 *
 * Revision 1.22  1993/08/13  16:32:50  pgf
 * tom's 3.58 changes
 *
 * Revision 1.21  1993/08/05  14:35:57  pgf
 * changed name of setmode to avoid library conflict with djgpp compiler
 * (also changed delmode, setgmode, and delgmode to be consistent)
 *
 * Revision 1.20  1993/07/15  10:37:58  pgf
 * see 3.55 CHANGES
 *
 * Revision 1.19  1993/07/01  16:15:54  pgf
 * tom's 3.51 changes
 *
 * Revision 1.18  1993/06/28  16:58:34  pgf
 * gave adjustmode() a real return value
 *
 * Revision 1.17  1993/06/18  15:57:06  pgf
 * tom's 3.49 changes
 *
 * Revision 1.16  1993/04/28  17:11:22  pgf
 * got rid of NeWS ifdefs
 *
 * Revision 1.15  1993/04/21  14:37:52  pgf
 * special cases for validating glob strings
 *
 * Revision 1.14  1993/04/20  12:18:32  pgf
 * see tom's 3.43 CHANGES
 *
 * Revision 1.13  1993/04/01  13:06:31  pgf
 * turbo C support (mostly prototypes for static)
 *
 * Revision 1.12  1993/03/16  10:53:21  pgf
 * see 3.36 section of CHANGES file
 *
 * Revision 1.11  1993/03/05  18:49:39  pgf
 * change "global" modes to "universal", to be less ambigous about what
 * we're talking about
 *
 * Revision 1.10  1993/03/05  17:50:54  pgf
 * see CHANGES, 3.35 section
 *
 * Revision 1.9  1993/02/24  10:59:02  pgf
 * see 3.34 changes, in CHANGES file
 *
 * Revision 1.8  1993/02/08  14:53:35  pgf
 * see CHANGES, 3.32 section
 *
 * Revision 1.7  1993/01/16  10:38:56  foxharp
 * reordering to support some new static routines
 *
 * Revision 1.6  1992/12/23  09:21:56  foxharp
 * macro-ized the "[Settings]" name
 *
 * Revision 1.5  1992/12/14  09:03:25  foxharp
 * lint cleanup, mostly malloc
 *
 * Revision 1.4  1992/12/04  09:14:36  foxharp
 * deleted unused assigns
 *
 * Revision 1.3  1992/08/20  23:40:48  foxharp
 * typo fixes -- thanks, eric
 *
 * Revision 1.2  1992/07/13  20:03:54  foxharp
 * the "terse" variable is now a boolean mode
 *
 * Revision 1.1  1992/05/29  09:38:33  foxharp
 * Initial revision
 *
 */

#include	"estruct.h"
#include	"edef.h"
 
#define MODES_LIST_NAME  ScratchName(Settings)
#define	NonNull(s)	((s == 0) ? "" : s)
#define	ONE_COL	26
#define	NCOLS	3

/*--------------------------------------------------------------------------*/

static	int	same_val P(( struct VALNAMES *, struct VAL *, struct VAL * ));
static	int	size_val P(( struct VALNAMES *, struct VAL * ));
static	int	listvalueset P(( char *, int, struct VALNAMES *, struct VAL *, struct VAL * ));
static	void	makemodelist P(( int, char * ));
static	int	string_to_bool P(( char *, int * ));
static	int	string_to_number P(( char *, int * ));
#if defined(GMD_GLOB) || defined(GVAL_GLOB)
static	int	legal_glob_mode P(( char * ));
#endif
static	int	mode_complete P(( int, char *, int * ));
static	int	mode_eol P(( char *, int, int, int ));
static	int	do_a_mode P(( int, int ));
static	int	adjustmode P(( int, int ));

#if COLOR
static	char	*cname[] = {	/* names of colors */
	"black", "red",     "green", "yellow",
	"blue",  "magenta", "cyan",  "white"
	};
#endif

/*--------------------------------------------------------------------------*/

static int
same_val(names, tst, ref)
struct VALNAMES *names;
struct VAL *tst, *ref;
{
	if (ref == 0)	/* can't test, not really true */
		return -TRUE;

	switch (names->type) {
	case VALTYPE_BOOL:
	case VALTYPE_COLOR:
	case VALTYPE_INT:
		return	(tst->vp->i == ref->vp->i);
	case VALTYPE_STRING:
		return	(tst->vp->p != 0)
		  &&	(ref->vp->p != 0)
		  &&	!strcmp(tst->vp->p, ref->vp->p);
	case VALTYPE_REGEX:
		return	(tst->vp->r->pat != 0)
		  &&	(ref->vp->r->pat != 0)
		  &&	!strcmp(tst->vp->r->pat, ref->vp->r->pat);
	default:
		mlforce("BUG: bad type %s %d", names->name, names->type);
	}

	return FALSE;
}

/*
 * Returns the formatted length of a string value.
 */
static int
size_val(names, values)
struct VALNAMES *names;
struct VAL *values;
{
	register int	n = strlen(names->name) + 3;
	register char	*s = 0;

	switch (names->type) {
#if	COLOR && !SMALLER	/* will show the color name too */
	case VALTYPE_COLOR:
		n += 4;
		s = cname[values->vp->i];
		break;
#endif
	case VALTYPE_STRING:
		s = values->vp->p;
		break;
	case VALTYPE_REGEX:
		s = values->vp->r->pat;
		break;
	}
	return	n + strlen(NonNull(s));
}

/*
 * Returns a mode-value formatted as a string
 */
char *
string_mode_val(names, values)
register struct VALNAMES *names;
register struct VAL *values;
{
	switch(names->type) {
	case VALTYPE_BOOL:
		return values->vp->i ? truem : falsem;
	case VALTYPE_COLOR:
#if COLOR && !SMALLER
		{
		static	char	temp[20];
		(void)lsprintf(temp, "%d (%s)",
			values->vp->i,
			cname[values->vp->i]);
		return temp;
		}
#endif				/* else, fall-thru to use int-code */
	case VALTYPE_INT:
		return l_itoa(values->vp->i);
	case VALTYPE_STRING:
		return NonNull(values->vp->p);
	case VALTYPE_REGEX:
		return NonNull(values->vp->r->pat);
	}
	return errorm;
}

/* listvalueset: print each value in the array according to type,
	along with its name, until a NULL name is encountered.  Only print
	if the value in the two arrays differs, or the second array is nil */
static int
listvalueset(which, nflag, names, values, globvalues)
char *which;
int nflag;
struct VALNAMES *names;
struct VAL *values, *globvalues;
{
	int	show[MAX_G_VALUES+MAX_B_VALUES+MAX_W_VALUES];
	int	any	= 0,
		passes	= 1,
		perline,
		percol,
		total;
	register int j, pass;

	/*
	 * First, make a list of values we want to show.
	 * Store:
	 *	0 - don't show
	 *	1 - show in first pass
	 *	2 - show in second pass (too long)
	 */
	for (j = 0; names[j].name != 0; j++) {
		show[j] = 0;
		if (same_val(names+j, values+j, globvalues ? globvalues+j : 0) != TRUE) {
			if (!any++) {
				if (nflag)
					bputc('\n');
				bprintf("%s:\n", which);
			}
			switch (names[j].type) {
			case VALTYPE_STRING:
			case VALTYPE_REGEX:
				if (size_val(names+j, values+j) > ONE_COL) {
					show[j] += 1;
					passes = 2;
				}
				/* fall-thru */
			default:
				show[j] += 1;
			}
		}
	}
	total = j;

	if (any) {
		if (!passes)
			passes = 1;
	} else
		return nflag;

	/*
	 * Now, go back and display the values
	 */
	for (pass = 1; pass <= passes; pass++) {
		register int	line, col, k;
		int	offsets[NCOLS+1];

		offsets[0] = 0;
		if (pass == 1) {
			for (j = percol = 0; j < total; j++) {
				if (show[j] == pass)
					percol++;
			}
			for (j = 1; j < NCOLS; j++) {
				offsets[j]
					= (percol + NCOLS - j) / NCOLS
					+ offsets[j-1];
			}
			perline = NCOLS;
		} else {	/* these are too wide for ONE_COL */
			offsets[1] = total;
			perline = 1;
		}
		offsets[NCOLS] = total;

		line = 0;
		col  = 0;
		for (;;) {
			k = line + offsets[col];
			for (j = 0; j < total; j++) {
				if (show[j] == pass) {
					if (k-- <= 0)
						break;
				}
			}
			if (k >= 0)	/* no more cells to display */
				break;

			if (col == 0)
				bputc(' ');
			if (names[j].type == VALTYPE_BOOL) {
				bprintf("%s%s%*P",
					values[j].vp->i ? "" : "no",
					names[j].name,
					ONE_COL, ' ');
			} else {
				bprintf("%s=%s%*P",
					names[j].name,
					string_mode_val(names+j, values+j),
					ONE_COL, ' ');
			}
			if (++col >= perline) {
				col = 0;
				bputc('\n');
				if (++line >= offsets[1])
					break;
			} else if (line+offsets[col] >= offsets[col+1])
				break;
		}
		if ((col != 0) || (pass != passes))
			bputc('\n');
	}
	return TRUE;
}

#ifdef lint
/*ARGSUSED*/ WINDOW *ptr2WINDOW(p) char *p; { return 0; }
#else
#define	ptr2WINDOW(p)	(WINDOW *)p
#endif

/* list the current modes into the current buffer */
/* ARGSUSED */
static
void	makemodelist(dum1,ptr)
	int dum1;
	char *ptr;
{
	static	char	gg[] = "Universal",
			bb[] = "Buffer",
			ww[] = "Window";
	int	nflag;

	register WINDOW *localwp = ptr2WINDOW(ptr);  /* alignment okay */
	register BUFFER *localbp = localwp->w_bufp;

	bprintf("--- \"%s\" settings, if different than globals %*P\n",
			localbp->b_bname, term.t_ncol-1, '-');
	if (!(nflag = listvalueset(bb, FALSE, b_valuenames, localbp->b_values.bv, global_b_values.bv))
	 && !(nflag = listvalueset(ww, nflag, w_valuenames, localwp->w_values.wv, global_w_values.wv)))
	 	bputc('\n');
	bputc('\n');

	bprintf("--- Global settings %*P\n", term.t_ncol-1, '-');
	nflag = listvalueset(gg, nflag, g_valuenames, global_g_values.gv, (struct VAL *)0);
	nflag = listvalueset(bb, nflag, b_valuenames, global_b_values.bv, (struct VAL *)0);
	(void)  listvalueset(ww, nflag, w_valuenames, global_w_values.wv, (struct VAL *)0);
}

/*
 * Set tab size
 */
int
settab(f, n)
int f,n;
{
	register WINDOW *wp;
	int val;
	char *whichtabs;
	if (b_val(curbp, MDCMOD)) {
		val = VAL_C_TAB;
		whichtabs = "C-t";
	} else {
		val = VAL_TAB;
		whichtabs = "T";
	}
	if (f && n >= 1) {
		make_local_b_val(curbp,val);
		set_b_val(curbp,val,n);
		curtabval = n;
		for_each_window(wp)
			if (wp->w_bufp == curbp) wp->w_flag |= WFHARD;
	} else if (f) {
		mlforce("[Illegal tabstop value]");
		TTbeep();
		return FALSE;
	}
	if (!global_b_val(MDTERSE) || !f)
		mlwrite("[%sabs are %d columns apart, using %s value.]", whichtabs,
			curtabval, is_global_b_val(curbp,val)?"global":"local" );
	return TRUE;
}

/*
 * Set fill column to n.
 */
int
setfillcol(f, n)
int f,n;
{
	if (f && n >= 1) {
		make_local_b_val(curbp,VAL_FILL);
		set_b_val(curbp,VAL_FILL,n);
	} else if (f) {
		mlforce("[Illegal fill-column value]");
		TTbeep();
		return FALSE;
	}
	if (!global_b_val(MDTERSE) || !f)
		mlwrite("[Fill column is %d, and is %s]", b_val(curbp,VAL_FILL),
			is_global_b_val(curbp,VAL_FILL) ? "global" : "local" );
	return(TRUE);
}

/*
 * Allocate/set a new REGEXVAL struct
 */
REGEXVAL *
new_regexval(pattern, magic)
char	*pattern;
int	magic;
{
	register REGEXVAL *rp;

	if ((rp = typealloc(REGEXVAL)) != 0) {
		rp->pat = strmalloc(pattern);
		rp->reg = regcomp(rp->pat, magic);
	}
	return rp;
}

/*
 * Copy the struct-pointer for the given VAL-struct, to make local or global
 * mode value.
 */
void
copy_val (dst, src, which)
struct VAL *dst;
struct VAL *src;
int	which;
{
	if (dst[which].vp != &(src[which].v)) {
		dst[which].vp = &(src[which].v);
		src[which].refs += 1;
	}
}

/*
 * Release storage of a REGEXVAL struct
 */
void
free_regexval(rp)
register REGEXVAL *rp;
{
	if (rp != 0) {
		FreeAndNull(rp->pat);
		FreeAndNull(rp->reg);
		free((char *)rp);
	}
}

/*
 * Release storage of a VAL struct
 */
void
free_val(names, values)
struct VALNAMES *names;
struct VAL *values;
{
	if (values->refs > 0) {
		values->refs -= 1;
		if (values->refs == 0) {
			switch (names->type) {
			case VALTYPE_STRING:
				FreeAndNull(values->v.p);
				break;
			case VALTYPE_REGEX:
				free_regexval(values->v.r);
				break;
			default:	/* nothing to free */
				break;
			}
		}
	}
}

/*
 * These are special routines designed to save the values of local modes and to
 * restore them.  The 'recompute_buffer()' procedure assumes that global modes
 * do not change during the recomputation process (so there is no point in
 * trying to convert any of those values to local ones).
 */
#if	OPT_UPBUFF
void
save_vals(maximum, gbl, dst, src)
int maximum;
struct VAL *gbl;
struct VAL *dst;
struct VAL *src;
{
	register int	n;
	for (n = 0; n < maximum; n++) {
		dst[n] = src[n];
		if (src[n].vp == &src[n].v) {	/* local value? */
			dst[n].vp = &dst[n].v;
			copy_val(src, gbl, n);
		}
	}
}

void
restore_vals(maximum, dst, src)
int maximum;
struct VAL *dst;
struct VAL *src;
{
	register int	n;
	for (n = 0; n < maximum; n++) {
		if (dst[n].vp == &dst[n].v) {
			src[n] = dst[n];
			src[n].vp = &src[n].v;
		}
	}
}
#endif /* OPT_UPBUFF */

#if NO_LEAKS
/*
 * free storage used by local mode-values, called only when we are freeing
 * all other storage associated with a buffer or window.
 */
void
free_local_vals(names, local, global)
struct VALNAMES *names;
struct VAL *local;
struct VAL *global;
{
	register int	j;

	for (j = 0; names[j].name != 0; j++) {
		if (local[j].vp == &(local[j].v)) {
			free_val(names+j, local+j);
		}
	}
}
#endif

/*
 * Convert a string to boolean, checking for errors
 */
static int
string_to_bool(base, np)
char	*base;
int	*np;
{
	if (is_truem(base))
		*np = TRUE;
	else if (is_falsem(base))
		*np = FALSE;
	else {
		mlforce("[Not a boolean: '%s']", base);
		return FALSE;
	}
	return TRUE;
}

/*
 * Convert a string to number, checking for errors
 */
static int
string_to_number(base, np)
char	*base;
int	*np;
{
	register char *s = base;
	register int nval = 0;
	while (isdigit(*s))
		nval = (nval * 10) + (*s++ - '0');

	if (*s != EOS) {
		mlforce("[Not a number: '%s']", base);
		return FALSE;
	}
	*np = nval;
	return TRUE;
}

/*
 * Validate a 'glob' mode-value.  It is either a boolean, or it must be a
 * pipe-expression with exactly one "%s" embedded (no other % characters,
 * unless escaped).  That way, we can use the string to format the pipe
 * command.
 */
#if defined(GMD_GLOB) || defined(GVAL_GLOB)
static int
legal_glob_mode(base)
char	*base;
{
#ifdef GVAL_GLOB	/* string */
	if (isShellOrPipe(base)) {
		register char	*s = base;
		int	count = 0;
		while (*s != EOS) {
			if (*s == '%') {
				if (*++s != '%') {
					if (*s == 's')
						count++;
					else
						count = 2;
				}
			}
			if (*s != EOS)
				s++;
		}
		if (count == 1)
			return TRUE;
	}
#endif
	if (!strcmp(base, "off")
	 || !strcmp(base, "on"))
	 	return TRUE;

	mlforce("[Illegal value for glob: '%s']", base);
	return FALSE;
}
#endif

/*
 * Lookup the mode named with 'cp[]' and adjust its value.
 */
int
adjvalueset(cp, kind, names, global, values)
char *cp;
int kind;
struct VALNAMES *names;
int global;
register struct VAL *values;
{
	struct VAL oldvalue;
	char prompt[NLINE];
	char respbuf[NFILEN];
	int no = !strncmp(cp, "no", 2);
	char *rp = no ? cp+2 : cp;
	int nval, s;
#if COLOR
	register int i;
#endif

	if (no)
		kind = !kind;

	if (no && (names->type != VALTYPE_BOOL))
		return FALSE;		/* this shouldn't happen */

	/* prevent major-mode changes for scratch-buffers */
	if ((global != TRUE)
	 && (names->winflags & WFMODE)
	 && b_is_scratch(curbp)) {
		mlforce("[Cannot change mode \"%s\" of scratch buffer]",
			names->name);
		return FALSE;
	}

	/* get a value if we need one */
	if ((end_string() == '=')
	 || (names->type != VALTYPE_BOOL)) {
		int	regex = (names->type == VALTYPE_REGEX);
		int	opts = regex ? 0 : KBD_NORMAL;
		int	eolchar = (names->type == VALTYPE_REGEX
				|| names->type == VALTYPE_STRING) ? '\n' : ' ';

		respbuf[0] = EOS;
		(void)lsprintf(prompt, "New %s %s: ",
			cp,
			regex ? "pattern" : "value");

		s = kbd_string(prompt, respbuf, sizeof(respbuf) - 1, eolchar, opts, no_completion);
		if (s != TRUE)
			return s;
		if (!strlen(rp = respbuf))
			return FALSE; 	
		if (names->type == VALTYPE_BOOL) {
			if (!string_to_bool(rp, &kind))
				return FALSE;
		}
#if defined(GMD_GLOB) || defined(GVAL_GLOB)
		if (!strcmp(names->name, "glob")
		 && !legal_glob_mode(rp))
		 	return FALSE;
#endif
	}
#if !SMALLER
	else
		hst_glue(' ');
#endif

	oldvalue = *values;	/* save, to simplify no-change testing */
	if (oldvalue.vp == &(values->v))
		oldvalue.vp = &(oldvalue.v);
	else
		oldvalue.refs += 1;	/* fake a reference */

	values->vp = &(values->v);	/* make sure we point to result! */
	values->refs = 1;		/* we just created an object */

	/* we matched a name -- get the value */
	switch(names->type) {
	case VALTYPE_BOOL:
		values->vp->i = kind;
		break;

#if COLOR
	case VALTYPE_COLOR:
		nval = -1;
		(void)mklower(rp);
		for (i = 0; i < NCOLORS; i++) {
			if (strcmp(rp, cname[i]) == 0) {
				nval = i;
				break;
			}
		}
		if ((nval < 0) && !string_to_number(rp, &nval))
			return FALSE;
		if (nval >= NCOLORS) {
			mlforce("[Not a legal color-index: %d]", nval);
			return FALSE;
		}
		values->vp->i = nval;
		break;
#endif /* COLOR */

	case VALTYPE_INT:
		if (!string_to_number(rp, &nval))
			return FALSE;
		values->vp->i = nval;
		break;

	case VALTYPE_STRING:
		values->vp->p = strmalloc(rp);
		break;

	case VALTYPE_REGEX:
		values->vp->r = new_regexval(rp, TRUE);
		break;

	default:
		mlforce("BUG: bad type %s %d", names->name, names->type);
		return FALSE;
	}

	if (!same_val(names, values, &oldvalue)) {
		if (names->winflags) {
			if (global == TRUE) {
				register WINDOW *wp;
				for_each_window(wp) {
	 				if ((wp->w_bufp == NULL)
					 || !b_is_scratch(wp->w_bufp)
					 || !(names->winflags & WFMODE))
						wp->w_flag |= names->winflags;
				}
			} else {
				curwp->w_flag |= names->winflags;
			}
		}
	}

	free_val(names, &oldvalue);
	return TRUE;
}

/* ARGSUSED */
int
listmodes(f, n)
int f,n;
{
	register WINDOW *wp = curwp;
	register int s;

	s = liststuff(MODES_LIST_NAME, makemodelist,0,(char *)wp);
	/* back to the buffer whose modes we just listed */
	(void)swbuffer(wp->w_bufp);
	return s;
}

/*
 * The 'mode_complete()' and 'mode_eol()' functions are invoked from
 * 'kbd_reply()' to setup the mode-name completion and query displays.
 */
static int
mode_complete(c, buf, pos)
int	c;
char	*buf;
int	*pos;
{
	return kbd_complete(c, buf, pos, (char *)&all_modes[0], sizeof(all_modes[0]));
}

static int
/*ARGSUSED*/
mode_eol(buffer, cpos, c, eolchar)
char *	buffer;
int	cpos;
int	c;
int	eolchar;
{
	return (c == ' ' || c == eolchar);
}

int
find_mode(mode, global, namesp, valuep)
char	*mode;
int	global;
struct VALNAMES **namesp;
struct VAL **valuep;
{
	register char *rp = !strncmp(mode, "no", 2) ? mode+2 : mode;
	register struct VALNAMES *nn;
	register struct VAL *vv;
	register int	class;

	for (class = 0; class < 3; class++) {
		switch (class) {
		default: /* universal modes */
			nn = g_valuenames;
			vv = (global != FALSE) ? global_g_values.gv : (struct VAL *)0;
			break;
		case 1:	/* buffer modes */
			nn = b_valuenames;
			vv = (global == TRUE) ? global_b_values.bv : curbp->b_values.bv;
			break;
		case 2:	/* window modes */
			nn = w_valuenames;
			vv = (global == TRUE) ? global_w_values.wv : curwp->w_values.wv;
			break;
		}

		if (vv != 0) {
			while (nn->name != NULL) {
				if (!strcmp(rp, nn->name)
				 || !strcmp(rp, nn->shortname)) {
					*namesp = nn;
					*valuep = vv;
					return TRUE;
				}
				nn++;
				vv++;
			}
		}
	}
	return FALSE;
}

/*
 * Process a single mode-setting
 */
static int
do_a_mode(kind, global)
int	kind;
int	global;
{
	struct VALNAMES	*nn;
	struct VAL	*vv;
	register int	s;
	static char cbuf[NLINE]; 	/* buffer to receive mode name into */

	/* prompt the user and get an answer */
	if ((s = kbd_reply(
		global	? "Global value: "
			: "Local value: ",
		cbuf, sizeof(cbuf)-1,
		mode_eol, '=', KBD_NORMAL|KBD_LOWERC, mode_complete)) != TRUE)
		return ((s == FALSE) ? SORTOFTRUE : s);

	if (!strcmp(cbuf,"all")) {
		hst_glue(' ');
		return listmodes(FALSE,1);
	}

	if ((s = find_mode(cbuf, global, &nn, &vv)) != TRUE) {
#if OPT_EVAL
		return set_variable(cbuf);
#else
		mlforce("[Not a legal set option: \"%s\"]", cbuf);
#endif
	} else if ((s = adjvalueset(cbuf, kind, nn, global, vv)) != 0) {
		if (s == TRUE)
			mlerase();	/* erase the junk */
		return s;
	}

	return FALSE;
}

/*
 * Process the list of mode-settings
 */
static int
adjustmode(kind, global)	/* change the editor mode status */
int kind;	/* true = set,		false = delete */
int global;	/* true = global flag,	false = current buffer flag */
{
	int s;
	int autobuff = global_g_val(GMDABUFF);
	int anything = 0;
#if	OPT_XTERM && !X11
	int xterm_mouse = global_g_val(GMDXTERM_MOUSE);
#endif

	while (((s = do_a_mode(kind, global)) == TRUE) && (end_string() == ' '))
		anything++;
	if ((s == SORTOFTRUE) && anything) /* fix for trailing whitespace */
		return TRUE;

	/* if the settings are up, redisplay them */
	if (bfind(MODES_LIST_NAME, NO_CREAT, BFSCRTCH))
		(void)listmodes(FALSE,1);

	if (autobuff != global_g_val(GMDABUFF)) sortlistbuffers();

#if	OPT_XTERM && !X11
	if (xterm_mouse != global_g_val(GMDXTERM_MOUSE)) {
		set_global_g_val(GMDXTERM_MOUSE,TRUE);
		if (xterm_mouse)	TTkclose();
		else			TTkopen();
		set_global_g_val(GMDXTERM_MOUSE,!xterm_mouse);
	}
#endif

	return s;
}

/* ARGSUSED */
int
setlocmode(f, n)	/* prompt and set an editor mode */
int f, n;	/* default and argument */
{
	return adjustmode(TRUE, FALSE);
}

/* ARGSUSED */
int
dellocmode(f, n)	/* prompt and delete an editor mode */
int f, n;	/* default and argument */
{
	return adjustmode(FALSE, FALSE);
}

/* ARGSUSED */
int
setglobmode(f, n)	/* prompt and set a global editor mode */
int f, n;	/* default and argument */
{
	return adjustmode(TRUE, TRUE);
}

/* ARGSUSED */
int
delglobmode(f, n)	/* prompt and delete a global editor mode */
int f, n;	/* default and argument */
{
	return adjustmode(FALSE, TRUE);
}
