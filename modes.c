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
 *	Revision 1.8  1993/02/08 14:53:35  pgf
 *	see CHANGES, 3.32 section
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
 *
 *
 */

#include	<stdio.h>
#include	"estruct.h"
#include	"edef.h"
 
#define MODES_LIST_NAME  ScratchName(Settings)

#if COLOR
#define	need_color_change(wp) if (wp) wp->w_flag |= WFCOLR
#else
#define	need_color_change(wp)
#endif

/* listvalueset: print each value in the array according to type,
	along with its name, until a NULL name is encountered.  Only print
	if the value in the two arrays differs, or the second array is nil */
static
int
listvalueset(names, values, globvalues)
struct VALNAMES *names;
struct VAL *values, *globvalues;
{
	register int j, perline;
	perline = 3;
	j = 0;
	while(names->name != NULL) {
		switch(names->type) {
		case VALTYPE_BOOL:
			if (!globvalues || values->vp->i != globvalues->v.i) {
				bprintf("%s%s%*P", values->vp->i ? "":"no",
					names->name, 26, ' ');
				j++;
			}
			break;
		case VALTYPE_COLOR:
			/* show the index-value */
		case VALTYPE_INT:
			if (!globvalues || values->vp->i != globvalues->v.i) {
				bprintf("%s=%d%*P", names->name,
					values->vp->i, 26, ' ');
				j++;
			}
			break;
		case VALTYPE_STRING:
			perline = 1;
			if (j && j >= perline) {
				bputc('\n');
				j = 0;
			}
			if (!globvalues || ( values->vp->p && globvalues->v.p &&
				(strcmp( values->vp->p, globvalues->v.p)))) {
				bprintf("%s=%s%*P", names->name,
					values->vp->p ? values->vp->p : "",
					26, ' ');
				j++;
			}
			break;
		case VALTYPE_REGEX:
			if (!globvalues || (values->vp->r->pat && globvalues->v.r->pat &&
				(strcmp( values->vp->r->pat, globvalues->v.r->pat)))) {
				bprintf("%s=%s%*P", names->name,
					values->vp->r->pat ? values->vp->r->pat : "",
					26, ' ');
				j++;
			}
			break;
		default:
			mlforce("BUG: bad type %s %d",names->name,names->type);
			return FALSE;
		}
		if (j && j >= perline) {
			bputc('\n');
			j = 0;
		}
		names++;
		values++;
		if (globvalues) globvalues++;
	}
	if (j % 3 != 0)
		bputc('\n');
	return TRUE;
}

/* list the current modes into the current buffer */
/* ARGSUSED */
static
void	makemodelist(dum1,ptr)
	int dum1;
	char *ptr;
{
	register WINDOW *localwp = (WINDOW *)ptr;  /* alignment okay */
	register BUFFER *localbp = localwp->w_bufp;
	bprintf("--- \"%s\" settings, if different than globals %*P\n",
			localbp->b_bname, term.t_ncol-1, '-');
	listvalueset(b_valuenames, localbp->b_values.bv, global_b_values.bv);
	bputc('\n');
	listvalueset(w_valuenames, localwp->w_values.wv, global_w_values.wv);
	bprintf("--- Global settings %*P\n", term.t_ncol-1, '-');
	listvalueset(b_valuenames, global_b_values.bv, (struct VAL *)0);
	bputc('\n');
	listvalueset(w_valuenames, global_w_values.wv, (struct VAL *)0);
#if LAZY
	lsprintf(line," lazy filename matching is %s",
					(othmode & OTH_LAZY) ? "on":"off");
	addline(blistp,line,-1);
#endif
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

static
int
adjvalueset(cp, kind, names, values)
char *cp;
int kind;
struct VALNAMES *names;
register struct VAL *values;
{
	char prompt[NLINE];
	char respbuf[NFILEN];
	int no = !strncmp(cp, "no", 2);
	char *rp = no ? cp+2 : cp;
	int nval, s;
	register int i;

	if (no)
		kind = !kind;

	while (names->name != NULL) {
		if (!strcmp(rp, names->name)
		 || !strcmp(rp, names->shortname))
			break;
		names++;
		values++;
	}
	if (names->name == NULL)
		return FALSE;

	if (no && (names->type != VALTYPE_BOOL))
		return FALSE;

	/* get a value if we need one */
	if ((end_string() == '=')
	 || (names->type != VALTYPE_BOOL)) {
		int	regex = (names->type == VALTYPE_REGEX);
		int	opts = regex ? 0 : KBD_NORMAL;

		respbuf[0] = EOS;
		(void)lsprintf(prompt, "New %s %s: ",
			cp,
			regex ? "pattern" : "value");

		s = kbd_string(prompt, respbuf, sizeof(respbuf) - 1, '\n', opts, no_completion);
		if (s != TRUE)
			return s;
		if (!strlen(rp = respbuf))
			return FALSE; 	
		if (names->type == VALTYPE_BOOL) {
			if (!strcmp(rp, "yes") || !strcmp(rp, "true"))
			 	kind = TRUE;
			else if (!strcmp(rp, "no") || !strcmp(rp, "false"))
				kind = FALSE;
			else
				return FALSE;
		}
	}

	/* we matched a name -- get the value */
	values->vp = &(values->v);

	switch(names->type) {
	case VALTYPE_BOOL:
		values->vp->i = kind;
		break;

	case VALTYPE_COLOR:
		for (i=0; i<NCOLORS; i++) {
			if (strcmp(rp, cname[i]) == 0) {
				values->vp->i = i;
				need_color_change(curwp);
				return TRUE;
			}
		}
		if (!isdigit(*rp))
			break;
		need_color_change(curwp);
		/* fall-thru with number */

	case VALTYPE_INT:
		nval = 0;
		while (isdigit(*rp))
			nval = (nval * 10) + (*rp++ - '0');
		values->vp->i = nval;
		break;

	case VALTYPE_STRING:
		if (values->vp->p)
			free(values->vp->p);
		values->vp->p = strmalloc(rp);
		break;

	case VALTYPE_REGEX:
		if (!values->vp->r) {
			values->vp->r = typealloc(struct regexval);
		} else {
			if (values->vp->r->pat)
				free(values->vp->r->pat);
			if (values->vp->r->reg)
				free((char *)values->vp->r->reg);
		}
		values->vp->r->pat = strmalloc(rp);
		values->vp->r->reg = regcomp(values->vp->r->pat, TRUE);
		break;

	default:
		mlforce("BUG: bad type %s %d",names->name,names->type);
		return FALSE;
	}
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
 * This procedure is invoked from 'kbd_string()' to setup the mode-name completion
 * and query displays.
 */
static int
mode_complete(c, buf, pos)
int	c;
char	*buf;
int	*pos;
{
	return kbd_complete(c, buf, pos, (char *)&all_modes[0], sizeof(all_modes[0]));
}

/*
 * Process a single mode-setting
 */
static int
do_a_mode(kind, global)
int	kind;
int	global;
{
	static char cbuf[NLINE]; 	/* buffer to receive mode name into */

	/* prompt the user and get an answer */
	if (!kbd_string(
		global	? "Global value: "
			: "Local value: ",
		cbuf, sizeof(cbuf)-1,
		'=', KBD_NORMAL|KBD_LOWERC, mode_complete)
	|| (*cbuf == EOS))
		return FALSE;

	if (!strcmp(cbuf,"all"))
		return listmodes(FALSE,1);

	if (adjvalueset(cbuf, kind, b_valuenames,
		global ? global_b_values.bv : curbp->b_values.bv )
	 || adjvalueset(cbuf, kind, w_valuenames,
		global ? global_w_values.wv : curwp->w_values.wv ))
		return TRUE;

	mlforce("[Not a legal set option: \"%s\"]", cbuf);
	return FALSE;
}

/*
 * Process the list of mode-settings
 */
static int
adjustmode(kind, global)	/* change the editor mode status */
int kind;	/* true = set,		false = delete */
int global; /* true = global flag,	false = current buffer flag */
{
	int autobuff = global_b_val(MDABUFF);
	int count = 0;

	do {
		if (do_a_mode(kind, global))
			count++;
	} while (end_string() == ' ');

	if (count > 0) {
		if (global) {
			register WINDOW *wp;
			for_each_window(wp)
				wp->w_flag |= WFHARD|WFMODE;
		} else {
			curwp->w_flag |= WFHARD|WFMODE;
		}
	}

	/* if the settings are up, redisplay them */
	if (bfind(MODES_LIST_NAME, NO_CREAT, BFSCRTCH))
		(void)listmodes(FALSE,1);

	if (autobuff != global_b_val(MDABUFF)) sortlistbuffers();

	(void)refresh(FALSE,1);
	mlerase();	/* erase the junk */

	return TRUE;
}

/* ARGSUSED */
int
setmode(f, n)	/* prompt and set an editor mode */
int f, n;	/* default and argument */
{
	return adjustmode(TRUE, FALSE);
}

/* ARGSUSED */
int
delmode(f, n)	/* prompt and delete an editor mode */
int f, n;	/* default and argument */
{
	return adjustmode(FALSE, FALSE);
}

/* ARGSUSED */
int
setgmode(f, n)	/* prompt and set a global editor mode */
int f, n;	/* default and argument */
{
	return adjustmode(TRUE, TRUE);
}

/* ARGSUSED */
int
delgmode(f, n)	/* prompt and delete a global editor mode */
int f, n;	/* default and argument */
{
	return adjustmode(FALSE, TRUE);
}
