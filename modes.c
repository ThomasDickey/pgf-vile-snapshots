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
 *	Revision 1.5  1992/12/14 09:03:25  foxharp
 *	lint cleanup, mostly malloc
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
 
/* ARGSUSED */
int
listmodes(f, n)
int f,n;
{
	register WINDOW *wp = curwp;
	register int s;

	s = liststuff("[Settings]",makemodelist,0,(char *)wp);
	/* back to the buffer whose modes we just listed */
	swbuffer(wp->w_bufp);
	return s;
}


/* list the current modes into the current buffer */
/* ARGSUSED */
void
makemodelist(dum1,ptr)
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
	listvalueset(b_valuenames, global_b_values.bv, NULL);
	bputc('\n');
	listvalueset(w_valuenames, global_w_values.wv, NULL);
#if LAZY
	lsprintf(line," lazy filename matching is %s",
					(othmode & OTH_LAZY) ? "on":"off");
	addline(blistp,line,-1);
#endif
}

/* listvalueset: print each value in the array according to type,
	along with its name, until a NULL name is encountered.  Only print
	if the value in the two arrays differs, or the second array is nil */
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
		for (wp = wheadp; wp != NULL; wp = wp->w_wndp)
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


int
adjustmode(kind, global)	/* change the editor mode status */
int kind;	/* true = set,		false = delete */
int global; /* true = global flag,	false = current buffer flag */
{
	register char *scan;		/* scanning pointer to convert prompt */
	register int i; 		/* loop index */
	register s;		/* error return on input */
#if COLOR
	register int uflag; 	/* was modename uppercase?	*/
#endif
	char prompt[50];	/* string to prompt user with */
	static char cbuf[NPAT]; 	/* buffer to receive mode name into */

	/* build the proper prompt string */
	if (global)
		strcpy(prompt,"Global value: ");
	else
		strcpy(prompt,"Local value: ");


	/* prompt the user and get an answer */

	s = mlreply(prompt, cbuf, NPAT - 1);
	if (s != TRUE)
		return(s);

	/* make it lowercase */

	scan = cbuf;
#if COLOR
	uflag = isupper(*scan);
#endif
	while (*scan != '\0' && *scan != '=') {
		if (isupper(*scan))
			*scan = tolower(*scan);
		scan++;
	}

	if (scan == cbuf) { /* no string */
		s = FALSE;
		goto errout;
	}

	if (!strcmp(cbuf,"all")) {
		return listmodes(FALSE,1);
	}

	/* colors should become another kind of value, i.e. VAL_TYPE_COLOR,
		and then this could be handled generically in adjvalueset() */
	/* test it first against the colors we know */
	for (i=0; i<NCOLORS; i++) {
		if (strcmp(cbuf, cname[i]) == 0) {
			/* finding the match, we set the color */
#if COLOR
			if (uflag)
				if (global)
					gfcolor = i;
				else if (curwp)
					set_w_val(curwp, WVAL_FCOLOR, i);
			else
				if (global)
					gbcolor = i;
				else if (curwp)
					set_w_val(curwp, WVAL_BCOLOR, i);

			if (curwp)
				curwp->w_flag |= WFCOLR;
#endif
			mlerase();
			return(TRUE);
		}
	}

	s = adjvalueset(cbuf, kind, b_valuenames,
		global ? global_b_values.bv : curbp->b_values.bv );
	if (s == TRUE)
		goto success;
	s = adjvalueset(cbuf, kind, w_valuenames,
		global ? global_w_values.wv : curwp->w_values.wv );
	if (s == TRUE)
		goto success;
	if (s == FALSE) {
	errout:
		mlforce("[Not a legal set option: \"%s\"]", cbuf);
	}
	return s;

success:
	if (global) {
		register WINDOW *wp;
		for (wp = wheadp; wp != NULL; wp = wp->w_wndp)
			wp->w_flag |= WFHARD|WFMODE;
	} else {
		curwp->w_flag |= WFHARD|WFMODE;
	}

	/* if the settings are up, redisplay them */
	if (bfind("[Settings]", NO_CREAT, BFSCRTCH))
		listmodes(FALSE,1);

	refresh(FALSE,1);
	mlerase();	/* erase the junk */

	return TRUE;
}

int
adjvalueset(cp, kind, names, values)
char *cp;
int kind;
struct VALNAMES *names;
register struct VAL *values;
{
	char respbuf[NFILEN];
	char *rp;
	char *equp;
	int rplen = 0;
	int no;
	int nval, s;

	no = 0;
	if (strncmp(cp, "no", 2) == 0) {
		kind = !kind;
		no = 2;
	}
	equp = rp = strchr(cp, '=');
	if (rp) {
		*rp = '\0';
		rp++;
		rplen = strlen(rp);
		if (!rplen)
			goto err;
	}
	while(names->name != NULL) {
		if (strncmp(cp + no, names->name, strlen(cp) - no) == 0)
			break;
		if (strncmp(cp + no, names->shortname, strlen(cp) - no) == 0)
			break;
		names++;
		values++;
	}
	if (names->name == NULL) {
	err:	if (equp)
			*equp = '=';
		return FALSE;
	}

	/* we matched a name -- get the value */
	switch(names->type) {
	case VALTYPE_BOOL:
		if (rp) {
			if (!strncmp(rp,"no",rplen) ||
					!strncmp(rp,"false",rplen))
				kind = FALSE;
			else if (!strncmp(rp,"yes",rplen) ||
					!strncmp(rp,"true",rplen))
				kind = TRUE;
			else
				goto err;
		}
		values->vp = &(values->v);
		values->vp->i = kind;
		break;

	case VALTYPE_INT:
	case VALTYPE_STRING:
	case VALTYPE_REGEX:
		if (no) goto err;
		if (!rp) {
			respbuf[0] = '\0';
			if (names->type != VALTYPE_REGEX)
				s = mlreply("New value: ", respbuf, NFILEN - 1);
			else
				s = kbd_string("New pattern: ", respbuf,
					NFILEN - 1, '\n', NO_EXPAND, FALSE);
			if (s != TRUE) {
				if (equp)
					*equp = '=';
				return s;
			}
			rp = respbuf;
			rplen = strlen(rp);
			if (!rplen)
				goto err;
		}
		values->vp = &(values->v);
		switch (names->type) {
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
		}
		break;

	default:
		mlforce("BUG: bad type %s %d",names->name,names->type);
		if (equp)
			*equp = '=';
		return FALSE;
	}
	return TRUE;
}

