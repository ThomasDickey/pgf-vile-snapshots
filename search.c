/*
 * The functions in this file implement commands that search in the forward
 * and backward directions.
 *  heavily modified by Paul Fox, 1990
 *
 * $Log: search.c,v $
 * Revision 1.60  1994/03/29 14:51:27  pgf
 * fix for alternate search delimiters
 *
 * Revision 1.59  1994/02/22  11:03:15  pgf
 * truncated RCS log for 4.0
 *
 *
 * original written Aug. 1986 by John M. Gamble, but I (pgf) have since
 * replaced his regex stuff with Henry Spencer's regexp package.
 *
 */

#include	"estruct.h"
#include        "edef.h"

#if	LATTICE
#define	void	int
#endif

int lastdirec;

MARK scanboundpos;

char onlyonemsg[] = "Only one occurrence of pattern";
char notfoundmsg[] = "Not found";
char hitendmsg[] = "Search reached %s without matching pattern";

void
not_found_msg(wrapok, dir)
int wrapok, dir;
{
	if (wrapok || global_b_val(MDTERSE))
		mlforce (notfoundmsg);
	else
		mlforce (hitendmsg, dir == FORWARD ? "BOTTOM":"TOP");
}

int
scrforwsearch(f,n)
int f,n;
{
	return fsearch(f,n,FALSE,TRUE);
}

int
scrbacksearch(f,n)
int f,n;
{
	return rsearch(f,n,FALSE,TRUE);
}

/*
 * forwsearch -- Search forward.  Get a search string from the user, and
 *	search for the string.  If found, reset the "." to be just after
 *	the match string, and (perhaps) repaint the display.
 */

int
forwsearch(f, n)
int f, n;
{
	register int status;
	hst_init('/');
	status = fsearch(f, n, FALSE, FALSE);
	hst_flush();
	return status;
}

/* extra args -- marking if called from globals, and should mark lines, and
	fromscreen, if the searchpattern is on the screen, so we don't need to
	ask for it.  */
int
fsearch(f, n, marking, fromscreen)
int f, n;
int marking, fromscreen;
{
	register int status;
	int wrapok;
	MARK curpos;
	int didmark = FALSE;

	if (f && n < 0)
		return rsearch(f, -n, FALSE, FALSE);

	wrapok = marking || b_val(curwp->w_bufp, MDSWRAP);

	lastdirec = 0;

	/* Ask the user for the text of a pattern.  If the
	 * response is TRUE (responses other than FALSE are
	 * possible), search for the pattern for as long as
	 * n is positive (n == 0 will go through once, which
	 * is just fine).
	 *
	 * If "marking", then we were called to do line marking for the
	 *  global command.
	 */
	if (!marking && (status = readpattern("Search: ", &pat[0],
				&gregexp, lastkey, fromscreen)) != TRUE) {
		return status;
	}

	ignorecase = b_val(curwp->w_bufp, MDIGNCASE);

	curpos = DOT;
	scanboundry(wrapok,curpos,FORWARD);
	do {
		nextch(&(DOT), FORWARD);
		status = scanner(gregexp, FORWARD, wrapok);
		if (status == ABORT) {
			mlforce("[Aborted]");
			DOT = curpos;
			return status;
		}
		/* if found, mark the line */
		if (status && marking) {
			/* if we were on a match when we started, then
				scanner returns TRUE, even though it's
				on a boundary. quit if we find ourselves
				marking a line twice */
			if (lismarked(l_ref(DOT.l)))
				break;
			lsetmarked(l_ref(DOT.l));
			/* and, so the next nextch gets to next line */
			DOT.o = lLength(DOT.l);
			didmark = TRUE;
		}
	} while ((marking || --n > 0) && status == TRUE);

	if (!marking && !status)
		nextch(&(DOT),REVERSE);

	if (marking) {  /* restore dot and offset */
		DOT = curpos;
	} else if (status) {
		savematch(DOT,gregexp->mlen);
		if (samepoint(DOT,curpos)) {
			mlwrite(onlyonemsg);
			TTbeep();
		}
	}

	/* Complain if not there.  */
	if ((marking && didmark == FALSE) ||
				(!marking && status == FALSE)) {
		not_found_msg(wrapok,FORWARD);
		return FALSE;
	}

	return TRUE;
}

/*
 * forwhunt -- Search forward for a previously acquired search string.
 *	If found, reset the "." to be just after the match string,
 *	and (perhaps) repaint the display.
 */

int
forwhunt(f, n)
int f, n;	/* default flag / numeric argument */
{
	register int status;
	int wrapok;
	MARK curpos;

	wrapok = b_val(curwp->w_bufp, MDSWRAP);

	if (n < 0)		/* search backwards */
		return(backhunt(f, -n));

	/* Make sure a pattern exists */
	if (pat[0] == EOS)
	{
		mlforce("[No pattern set]");
		return FALSE;
	}

	ignorecase = b_val(curwp->w_bufp, MDIGNCASE);

	/* Search for the pattern for as long as
	 * n is positive (n == 0 will go through once, which
	 * is just fine).
	 */
	curpos = DOT;
	scanboundry(wrapok,DOT,FORWARD);
	do {
		nextch(&(DOT),FORWARD);
		status = scanner(gregexp, FORWARD, wrapok);
	} while ((--n > 0) && status == TRUE);

	/* Save away the match, or complain if not there.  */
	if (status == TRUE) {
		savematch(DOT,gregexp->mlen);
		if (samepoint(DOT,curpos)) {
			mlwrite(onlyonemsg);
			TTbeep();
		}
	} else if (status == FALSE) {
		nextch(&(DOT),REVERSE);
		not_found_msg(wrapok,FORWARD);
	} else if (status == ABORT) {
		mlforce("[Aborted]");
		DOT = curpos;
		return status;
	}

	return status;
}

/*
 * backsearch -- Reverse search.  Get a search string from the user, and
 *	search, starting at "." and proceeding toward the front of the buffer.
 *	If found "." is left pointing at the first character of the pattern
 *	(the last character that was matched).
 */
int
backsearch(f, n)
int f, n;
{
	register int status;
	hst_init('?');
	status = rsearch(f, n, FALSE, FALSE);
	hst_flush();
	return status;
}

/* ARGSUSED */
int
rsearch(f, n, dummy, fromscreen)
int f, n;	/* default flag / numeric argument */
int dummy, fromscreen;
{
	register int status;
	int wrapok;
	MARK curpos;

	if (n < 0)
		return fsearch(f, -n, FALSE, fromscreen);

	wrapok = b_val(curwp->w_bufp, MDSWRAP);

	lastdirec = 1;

	/* Ask the user for the text of a pattern.  If the
	 * response is TRUE (responses other than FALSE are
	 * possible), search for the pattern for as long as
	 * n is positive (n == 0 will go through once, which
	 * is just fine).
	 */
	if ((status = readpattern("Reverse search: ", &pat[0],
				&gregexp, lastkey, fromscreen)) == TRUE) {
		ignorecase = b_val(curwp->w_bufp, MDIGNCASE);

		curpos = DOT;
		scanboundry(wrapok,DOT,REVERSE);
		do {
			nextch(&(DOT),REVERSE);
			status = scanner(gregexp, REVERSE, wrapok);
		} while ((--n > 0) && status == TRUE);

		/* Save away the match, or complain if not there.  */
		if (status == TRUE)
			savematch(DOT,gregexp->mlen);
			if (samepoint(DOT,curpos)) {
				mlwrite(onlyonemsg);
				TTbeep();
			}
		else if (status == FALSE) {
			nextch(&(DOT),FORWARD);
			not_found_msg(wrapok,REVERSE);
		} else if (status == ABORT) {
			mlforce("[Aborted]");
			DOT = curpos;
			return status;
		}
	}
	return(status);
}

/*
 * backhunt -- Reverse search for a previously acquired search string,
 *	starting at "." and proceeding toward the front of the buffer.
 *	If found "." is left pointing at the first character of the pattern
 *	(the last character that was matched).
 */
int
backhunt(f, n)
int f, n;	/* default flag / numeric argument */
{
	register int status;
	int wrapok;
	MARK curpos;

	wrapok = b_val(curwp->w_bufp, MDSWRAP);

	if (n < 0)		/* search forwards */
		return(forwhunt(f, -n));

	/* Make sure a pattern exists */
	if (pat[0] == EOS) {
		mlforce("[No pattern set]");
		return FALSE;
	}

	/* Go search for it for as long as
	 * n is positive (n == 0 will go through once, which
	 * is just fine).
	 */

	ignorecase = b_val(curwp->w_bufp, MDIGNCASE);

	curpos = DOT;
	scanboundry(wrapok,DOT,REVERSE);
	do {
		nextch(&(DOT),REVERSE);
		status = scanner(gregexp, REVERSE, wrapok);
	} while ((--n > 0) && status == TRUE);

	/* Save away the match, or complain
	 * if not there.
	 */
	if (status == TRUE) {
		savematch(DOT,gregexp->mlen);
		if (samepoint(DOT, curpos)) {
			mlwrite(onlyonemsg);
			TTbeep();
		}
	} else if (status == FALSE) {
		nextch(&(DOT),FORWARD);
		not_found_msg(wrapok,REVERSE);
	} else if (status == ABORT) {
		mlforce("[Aborted]");
		DOT = curpos;
		return status;
	}

	return(status);
}

/* continue searching in the same direction as previous */
int
consearch(f,n)
int f,n;
{
	if (lastdirec == 0)
		return(forwhunt(f,n));
	else
		return(backhunt(f,n));
}

/* continue searching in the opposite direction as previous */
int
revsearch(f,n)
int f,n;
{
	if (lastdirec == 0)
		return(backhunt(f,n));
	else
		return(forwhunt(f,n));
}


/*
 * scanner -- Search for a pattern in either direction.  If found,
 *	reset the "." to be at the start or just after the match string
 */
int
scanner(exp, direct, wrapok)
regexp	*exp;	/* the compiled expression */
int	direct;	/* which way to go.*/
int	wrapok;	/* ok to wrap around bottom of buffer? */
{
	MARK curpos;
	int found, wrapped;

	if (!exp) {
		mlforce("BUG: null exp");
		return FALSE;
	}

	/* Setup local scan pointers to global ".".
	 */
	curpos = DOT;

	wrapped = 0;

	/* Scan each character until we hit the scan boundary */
	for(;;) {
		register int startoff, srchlim;

		if (interrupted()) return ABORT;

		if (sameline(curpos, scanboundpos)) {
			if (direct == FORWARD) {
				if (wrapped) {
					startoff = curpos.o;
					srchlim = scanboundpos.o;
				} else {
					startoff = curpos.o;
					srchlim = (scanboundpos.o > curpos.o) ?
					  scanboundpos.o : lLength(curpos.l);
				}
			} else {
				if (wrapped) {
					startoff = scanboundpos.o;
					srchlim = lLength(curpos.l);
				} else {
					startoff = 0;
					srchlim = scanboundpos.o + 1;
				}
			}
		} else {
			if (direct == FORWARD) {
				startoff = curpos.o;
				srchlim = lLength(curpos.l);
			} else {
				startoff = 0;
				srchlim = curpos.o+1;
			}
		}
		found = lregexec(exp, l_ref(curpos.l), startoff, srchlim);
		if (found) {
			if (direct == REVERSE) { /* find the last one */
				char *got = exp->startp[0];
				while (lregexec(exp, l_ref(curpos.l),
						(int)(got+1-l_ref(curpos.l)->l_text),
						srchlim)) {
					got = exp->startp[0];
				}
				if (!lregexec(exp, l_ref(curpos.l),
						(int)(got-l_ref(curpos.l)->l_text),
						srchlim)) {
					mlforce("BUG: prev. match no good");
					return FALSE;
				}
			}
			DOT.l = curpos.l;
			DOT.o = (C_NUM)(exp->startp[0] - l_ref(curpos.l)->l_text);
			curwp->w_flag |= WFMOVE; /* flag that we have moved */
			return TRUE;
		} else {
			if (sameline(curpos,scanboundpos) &&
							(!wrapok || wrapped))
				break;
		}
		if (direct == FORWARD) {
			curpos.l = lFORW(curpos.l);
		} else {
			curpos.l = lBACK(curpos.l);
		}
		if (is_header_line(curpos, curbp)) {
			wrapped++;
			if (sameline(curpos,scanboundpos) &&
						(!wrapok || wrapped) )
				break;
			if (direct == FORWARD) {
				curpos.l = lFORW(curpos.l);
			} else {
				curpos.l = lBACK(curpos.l);
			}
		}
		if (direct == FORWARD) {
			curpos.o = 0;
		} else {
			if ((curpos.o = lLength(curpos.l)-1) < 0)
				curpos.o = 0;
		}

	}

	return FALSE;	/* We could not find a match.*/
}

void
regerror(s)
char *s;
{
	mlforce("[Bad pattern: %s ]",s);
}


/*
 * eq -- Compare two characters.  The "bc" comes from the buffer, "pc"
 *	from the pattern.  If we are in IGNCASE mode, fold out the case.
 */
#if OPT_EVAL || UNUSED
int
eq(bc, pc)
register int	bc;
register int	pc;
{
	if (bc == pc)
		return TRUE;
	if (!b_val(curwp->w_bufp, MDIGNCASE))
		return FALSE;
	return nocase_eq(bc,pc);
}
#endif

/* ARGSUSED */
int
scrsearchpat(f,n)
int f,n;
{
	int s;
	s =  readpattern("", pat, &gregexp, EOS, TRUE);
	mlwrite("Search pattern is now %s", pat);
	lastdirec = 0;
	return s;
}

/*
 * readpattern -- Read a pattern.  Stash it in apat.  If it is the
 *	search string, re_comp() it.
 *	Apat is not updated if the user types in an empty line.  If
 *	the user typed an empty line, and there is no old pattern, it is
 *	an error.  Display the old pattern, in the style of Jeff Lomicka.
 *	There is some do-it-yourself control expansion.
 *	An alternate termination character is passed in.
 */
int
readpattern(prompt, apat, srchexpp, c, fromscreen)
char	*prompt;
char	*apat;
regexp	**srchexpp;
int	c;
int	fromscreen;
{
	int status;

	if (did_tungetc()) /* cf: execute_named_command */
		tgetc(TRUE);

	/* Read a pattern.  Either we get one,
	 * or we don't, and use the previous pattern.
	 * Then, if it's the search string, make a reversed pattern.
	 */
	if (fromscreen) {
		status = screen_string(apat, NPAT, _ident);
		if (status != TRUE)
			return status;
	} else {
		/* don't expand #, %, :, and never process backslashes
			since they're handled by regexp directly for the
			search pattern, and in delins() for the replacement
			pattern */
		hst_glue(c);
	 	status = kbd_string(prompt, apat, NPAT, c, 0, no_completion);
	}
 	if (status == TRUE) {
		if (srchexpp) {	/* If doing the search string, compile it */
			FreeIfNeeded(*srchexpp);
			*srchexpp = regcomp(pat, b_val(curbp, MDMAGIC));
			if (!*srchexpp)
				return FALSE;
		}
	} else if (status == FALSE && *apat != EOS) { /* Old one */
		status = TRUE;
	}

	return status;
}

/*
 * savematch -- We found the pattern?  Let's save it away.
 */

void
savematch(curpos,matchlen)
MARK curpos;		/* last match */
SIZE_T matchlen;
{
	static	ALLOC_T	patlen = 0;	/* length of last malloc */

	/* free any existing match string */
	if (patmatch == NULL || patlen < matchlen) {
		FreeIfNeeded(patmatch);
		/* attempt to allocate a new one */
		patmatch = castalloc(char, patlen = matchlen + 20);
		if (patmatch == NULL)
			return;
	}

	(void)strncpy(patmatch, &(l_ref(curpos.l)->l_text[curpos.o]), matchlen);

	/* null terminate the match string */
	patmatch[matchlen] = EOS;
}

void
scanboundry(wrapok,dot,dir)
int wrapok;
MARK dot;
int dir;
{
	if (wrapok) {
		nextch(&dot,dir);
		scanboundpos = dot;
	} else {
		scanboundpos = curbp->b_line;
	}
}

/*
 * nextch -- advance/retreat the given mark
 *  will wrap, and doesn't set motion flags
 */
void
nextch(pdot, dir)
MARK	*pdot;
int	dir;
{
	register LINE	*curline;
	register int	curoff;

	curline = l_ref(pdot->l);
	curoff = pdot->o;
	if (dir == FORWARD) {
		if (curoff == llength(curline)) {
			curline = lforw(curline);	/* skip to next line */
			curoff = 0;
		} else {
			curoff++;
		}
	} else {
		if (curoff == 0) {
			curline = lback(curline);
			curoff = llength(curline);
		} else {
			curoff--;
		}
	}
	pdot->l = l_ptr(curline);
	pdot->o = curoff;
}


/* simple finder -- give it a compiled regex, a direction, and it takes you
	there if it can.  no wrapping allowed  */
int
findpat(f,n,exp,direc)
int f, n;
regexp *exp;
int direc;
{
	int s;
	MARK savepos;

	if (!exp)
		return FALSE;

	if (!f) n = 1;

	s = TRUE;
	scanboundpos = curbp->b_line; /* was scanboundry(FALSE,savepos,0); */
	while (s == TRUE && n--) {
		savepos = DOT;
		s = (direc == FORWARD) ? forwchar(TRUE,1) : backchar(TRUE,1);
		if (s == TRUE)
			s = scanner(exp, direc, FALSE);
	}
	if (s != TRUE)
		DOT = savepos;

	return s;
}

