/*
 * The functions in this file implement commands that search in the forward
 * and backward directions.
 *  heavily modified by Paul Fox, 1990
 *
 * Aug. 1986 John M. Gamble:
 *	... a limited number of regular expressions - 'any',
 *	'character class', 'closure', 'beginning of line', and
 *	'end of line'.
 *
 *	Replacement metacharacters will have to wait for a re-write of
 *	the replaces function, and a new variation of ldelete().
 *
 *	For those curious as to my references, i made use of
 *	Kernighan & Plauger's "Software Tools."
 *	I deliberately did not look at any published grep or editor
 *	source (aside from this one) for inspiration.  I did make use of
 *	Allen Hollub's bitmap routines as published in Doctor Dobb's Journal,
 *	June, 1985 and modified them for the limited needs of character class
 *	matching.  Any inefficiences, bugs, stupid coding examples, etc.,
 *	are therefore my own responsibility.
 *
 */

#include        <stdio.h>
#include	"estruct.h"
#include        "edef.h"

#if	LATTICE
#define	void	int
#endif

int    mcmatch();
int    readpattern();
#if BEFORE
int    replaces();
#endif
int    nextch();
int    mcstr();
int    mceq();
int    cclmake();
int    biteq();
BITMAP   clearbits();
void     setbit();

int lastdirec;
LINE *boundline;
int boundoff;

scrforwsearch(f,n)
{
	return forwsearch(f,n,NULL,TRUE);
}

scrbacksearch(f,n)
{
	return backsearch(f,n,NULL,TRUE);
}

char onlyonemsg[] = "Only one occurence of pattern";
char notfoundmsg[] = "Not found";
/*
 * forwsearch -- Search forward.  Get a search string from the user, and
 *	search for the string.  If found, reset the "." to be just after
 *	the match string, and (perhaps) repaint the display.
 */

forwsearch(f, n, marking, fromscreen)
int f, n;	/* default flag / numeric argument */
{
	register int status = TRUE;
	int wrapok;
	int c;
	LINE *curdotp;
	int curoff;
	int didmark = FALSE;
	
	wrapok = marking || (curwp->w_bufp->b_mode & MDSWRAP) != 0;

	/* If n is negative, search backwards.
	 * Otherwise proceed by asking for the search string.
	 */
	if (f && n < 0)
		return(backsearch(f, -n, NULL, fromscreen));

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
					TRUE, lastkey, fromscreen)) != TRUE) {
		return status;
	}
		
	curdotp = curwp->w_dotp;
	curoff = curwp->w_doto;
	setboundry(wrapok,curwp->w_dotp,curwp->w_doto,FORWARD);
	do {
		nextch(&(curwp->w_dotp),&(curwp->w_doto), FORWARD,wrapok);
		status = thescanner(&pat[0], FORWARD, PTBEG, wrapok);
		/* if found, mark the line */
		if (status && marking) {
			/* if we were on a match when we started, then
				thescanner returns TRUE, even though it's
				on a boundary. quit if we find ourselves
				marking a line twice */
			if (lismarked(curwp->w_dotp))
				break;
			lsetmarked(curwp->w_dotp);
			/* and, so the next nextch gets to next line */
			curwp->w_doto = llength(curwp->w_dotp);
			didmark = TRUE;
		}
	} while ((marking || --n > 0) && status == TRUE);
	
	if (!marking && !status)
		nextch(&(curwp->w_dotp),&(curwp->w_doto), REVERSE,wrapok);
		
	if (marking) {  /* restore dot and offset */
		curwp->w_dotp = curdotp;
		curwp->w_doto = curoff;
	} else if (status) {
		savematch();
		if (curwp->w_dotp == curdotp && curwp->w_doto == curoff) {
			mlwrite(onlyonemsg);
			TTbeep();
		}
	}

	/* Complain if not there.  */
	if ((marking && didmark == FALSE) ||
				(!marking && status == FALSE)) {
		mlwrite(notfoundmsg);
		TTbeep();
		return FALSE;
	}
	
	return TRUE;
}

/*
 * forwhunt -- Search forward for a previously acquired search string.
 *	If found, reset the "." to be just after the match string,
 *	and (perhaps) repaint the display.
 */

forwhunt(f, n)
int f, n;	/* default flag / numeric argument */
{
	register int status = TRUE;
	int wrapok;
	LINE *curdotp;
	int curoff;
	
	wrapok = (curwp->w_bufp->b_mode & MDSWRAP) != 0;

	if (n < 0)		/* search backwards */
		return(backhunt(f, -n));

	/* Make sure a pattern exists, or that we didn't switch
	 * into MAGIC mode until after we entered the pattern.
	 */
	if (pat[0] == '\0')
	{
		mlwrite("No pattern set");
		return FALSE;
	}
	/* mlwrite("Searching ahead..."); */
#if	MAGIC
	if ((curwp->w_bufp->b_mode & MDMAGIC) && (mcpat[0].mc_type == MCNIL))
	{
		if (!mcstr())
			return FALSE;
	}
#endif

	/* Search for the pattern for as long as
	 * n is positive (n == 0 will go through once, which
	 * is just fine).
	 */
	curdotp = curwp->w_dotp;
	curoff = curwp->w_doto;
	setboundry(wrapok,curwp->w_dotp,curwp->w_doto,FORWARD);
	do {
		nextch(&(curwp->w_dotp),&(curwp->w_doto),FORWARD,wrapok);
		status = thescanner(&pat[0], FORWARD, PTBEG, wrapok);
	} while ((--n > 0) && status == TRUE);

	/* Save away the match, or complain if not there.  */
	if (status == TRUE) {
		savematch();
		if (curwp->w_dotp == curdotp && curwp->w_doto == curoff) {
			mlwrite(onlyonemsg);
			TTbeep();
		}
	} else if (status == FALSE) {
		nextch(&(curwp->w_dotp),&(curwp->w_doto),REVERSE,wrapok);
		mlwrite(notfoundmsg);
		TTbeep();
	}

	return(status);
}

/*
 * backsearch -- Reverse search.  Get a search string from the user, and
 *	search, starting at "." and proceeding toward the front of the buffer.
 *	If found "." is left pointing at the first character of the pattern
 *	(the last character that was matched).
 */
backsearch(f, n, dummy, fromscreen)
int f, n;	/* default flag / numeric argument */
{
	register int status = TRUE;
	int wrapok;
	LINE *curdotp;
	int curoff;
	
	wrapok = (curwp->w_bufp->b_mode & MDSWRAP) != 0;

	/* If n is negative, search forwards.
	 * Otherwise proceed by asking for the search string.
	 */
	if (n < 0)
		return(forwsearch(f, -n, NULL, fromscreen));

	lastdirec = 1;

	/* Ask the user for the text of a pattern.  If the
	 * response is TRUE (responses other than FALSE are
	 * possible), search for the pattern for as long as
	 * n is positive (n == 0 will go through once, which
	 * is just fine).
	 */
	if ((status = readpattern("Reverse search: ", &pat[0],
					TRUE, lastkey, fromscreen)) == TRUE) {
		curdotp = curwp->w_dotp;
		curoff = curwp->w_doto;
		setboundry(wrapok,curwp->w_dotp,curwp->w_doto,REVERSE);
		do {
			nextch(&(curwp->w_dotp),&(curwp->w_doto),REVERSE,
									wrapok);
			status = thescanner(&tap[0], REVERSE, PTBEG, wrapok);
		} while ((--n > 0) && status == TRUE);

		/* Save away the match, or complain if not there.  */
		if (status == TRUE)
			savematch();
			if (curwp->w_dotp == curdotp && curwp->w_doto == curoff) {
				mlwrite(onlyonemsg);
				TTbeep();
			}
		else if (status == FALSE) {
			nextch(&(curwp->w_dotp),&(curwp->w_doto),FORWARD,
									wrapok);
			mlwrite(notfoundmsg);
			TTbeep();
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
backhunt(f, n)
int f, n;	/* default flag / numeric argument */
{
	register int status = TRUE;
	int wrapok;
	LINE *curdotp;
	int curoff;
	
	wrapok = (curwp->w_bufp->b_mode & MDSWRAP) != 0;

	if (n < 0)
		return(forwhunt(f, -n));

	/* Make sure a pattern exists, or that we didn't switch
	 * into MAGIC mode until after we entered the pattern.
	 */
	if (tap[0] == '\0')
	{
		mlwrite("No pattern set");
		return FALSE;
	}
	/* mlwrite("Searching back..."); */
#if	MAGIC
	if ((curwp->w_bufp->b_mode & MDMAGIC) && (tapcm[0].mc_type == MCNIL))
	{
		if (!mcstr())
			return FALSE;
	}
#endif

	/* Go search for it for as long as
	 * n is positive (n == 0 will go through once, which
	 * is just fine).
	 */

	curdotp = curwp->w_dotp;
	curoff = curwp->w_doto;
	setboundry(wrapok,curwp->w_dotp,curwp->w_doto,REVERSE);
	do {
		nextch(&(curwp->w_dotp),&(curwp->w_doto),REVERSE,wrapok);
		status = thescanner(&tap[0], REVERSE, PTBEG, wrapok);
	} while ((--n > 0) && status == TRUE);

	/* Save away the match, or complain
	 * if not there.
	 */
	if (status == TRUE) {
		savematch();
		if (curwp->w_dotp == curdotp && curwp->w_doto == curoff) {
			mlwrite(onlyonemsg);
			TTbeep();
		}
	} else if (status == FALSE) {
		nextch(&(curwp->w_dotp),&(curwp->w_doto), FORWARD,wrapok);
		mlwrite(notfoundmsg);
		TTbeep();
	}

	return(status);
}

consearch(f,n)
{
	if (lastdirec == 0)
		return(forwhunt(f,n));
	else
		return(backhunt(f,n));
}

revsearch(f,n)
{
	if (lastdirec == 0)
		return(backhunt(f,n));
	else
		return(forwhunt(f,n));
}


/*
 * thescanner -- Search for a pattern in either direction.  If found,
 *	reset the "." to be at the start or just after the match string,
 *	and (perhaps) repaint the display.
 */
int	
thescanner(patrn, direct, beg_or_end, wrapok)
char	*patrn;		/* pointer into pattern */
int	direct;		/* which way to go.*/
int	beg_or_end;	/* put point at beginning or end of pattern.*/
{
	LINE *curline;			/* current line during scan */
	int curoff;			/* position within current line */
	int found;
	int (*matcher)();
	int mcmatch();
	int litmatch();

	/* If we are going in reverse, then the 'end' is actually
	 * the beginning of the pattern.  Toggle it.
	 */
	beg_or_end ^= direct;

	/*
	 * Save the old matchlen length, in case it is
	 * horribly different (closure) from the old length.
	 * This is terribly important for query-replace undo
	 * command.
	 */
	mlenold = matchlen;

	/* Setup local scan pointers to global ".".
	 */
	curline = curwp->w_dotp;
	curoff  = curwp->w_doto;
#if MAGIC
	if (magical && (curwp->w_bufp->b_mode & MDMAGIC)) {
		matcher = mcmatch;
		if (direct == FORWARD)
			patrn = (char *)mcpat;
		else
			patrn = (char *)tapcm;
	} else
#endif
	{
		matcher = litmatch;
		if (direct == FORWARD)
			patrn = pat;
		else
			patrn = tap;
	}
			
	/* Scan each character until we hit the head link record.
	 */
	do {
		if (interrupted) return ABORT;

		/* Save the current position in case we need to
		 * restore it on a match, and initialize matchlen to
		 * zero in case we are doing a search for replacement.
		 */
		matchline = curline;
		matchoff = curoff;
		matchlen = 0;
		
		if ((*matcher)(patrn, direct, &curline, &curoff)) {
			/* A SUCCESSFULL MATCH!!!
			 * reset the global "." pointers.
			 */
			if (beg_or_end == PTEND)	/* at end of string */
			{
				curwp->w_dotp = curline;
				curwp->w_doto = curoff;
			}
			else		/* at beginning of string */
			{
				curwp->w_dotp = matchline;
				curwp->w_doto = matchoff;
			}

			curwp->w_flag |= WFMOVE; /* flag that we have moved */
			return TRUE;
		}

		/* Advance the cursor.
		 */
		nextch(&curline, &curoff, direct,wrapok);
	} while (!boundry(curline, curoff, direct, wrapok));

	return FALSE;	/* We could not find a match.*/
}

#if MAGIC
/*
 * mcmatch -- Search for a meta-pattern in either direction.  Based on the
 *	recursive routine amatch() (for "anchored match") in
 *	Kernighan & Plauger's "Software Tools".
 */
mcmatch(mcptr, direct, pcwline, pcwoff)
register MC	*mcptr;	/* string to scan for */
int		direct;		/* which way to go.*/
LINE		**pcwline;	/* current line during scan */
int		*pcwoff;	/* position within current line */
{
	register int c;			/* character at current position */
	LINE *curline;			/* current line during scan */
	int curoff;			/* position within current line */
	int nchars;

	/* Set up local scan pointers to ".", and get
	 * the current character.  Then loop around
	 * the pattern pointer until success or failure.
	 */
	curline = *pcwline;
	curoff = *pcwoff;

	/* The beginning-of-line and end-of-line metacharacters
	 * do not compare against characters, they compare
	 * against positions.
	 * BOL is guaranteed to be at the start of the pattern
	 * for forward searches, and at the end of the pattern
	 * for reverse searches.  The reverse is true for EOL.
	 * So, for a start, we check for them on entry.
	 */
	if (mcptr->mc_type == BOL)
	{
		if (curoff != 0)
			return FALSE;
		mcptr++;
	}

	if (mcptr->mc_type == EOL)
	{
		if (curoff != llength(curline))
			return FALSE;
		mcptr++;
	}

	while (mcptr->mc_type != MCNIL)
	{
		c = nextch(&curline, &curoff, direct, FALSE);

		if (mcptr->mc_type & CLOSURE)
		{
			/* Try to match as many characters as possible
			 * against the current meta-character.  A
			 * newline or boundary never matches a closure.
			 */
			nchars = 0;
			while (c != '\n' && c != -1 && mceq(c, mcptr))
			{
				nchars++;
				c = nextch(&curline, &curoff, direct, FALSE);
			}

			/* We are now at the character that made us
			 * fail.  nchars is the number of successful
			 * matches.  Try to match the rest of the pattern.
			 * Shrink the closure by one for each failure.
			 * Since closure matches *zero* or more occurences
			 * of a pattern, a match may start even if the
			 * previous loop matched no characters.
			 */
			mcptr++;

			for (;;)
			{
				/* back up, since mcmatch goes forward first */
				(void)nextch(&curline, &curoff, 
					direct ^ REVERSE, FALSE);

				if (mcmatch(mcptr, direct, &curline, &curoff))
				{
					matchlen += nchars;
					goto success;
				}

				if (nchars-- == 0)
					return FALSE;
			}
		}
		else			/* Not closure.*/
		{
			/* The only way we'd get a BOL metacharacter
			 * at this point is at the end of the reversed pattern.
			 * The only way we'd get an EOL metacharacter
			 * here is at the end of a regular pattern.
			 * So if we match one or the other, and are at
			 * the appropriate position, we are guaranteed success
			 * (since the next pattern character has to be MCNIL).
			 * Before we report success, however, we back up by
			 * one character, so as to leave the cursor in the
			 * correct position.  For example, a search for ")$"
			 * will leave the cursor at the end of the line, while
			 * a search for ")<NL>" will leave the cursor at the
			 * beginning of the next line.  This follows the
			 * notion that the meta-character '$' (and likewise
			 * '^') match positions, not characters.
			 */
			if (mcptr->mc_type == BOL)
			{
				if (curoff == llength(curline))
				{
					(void)nextch(&curline, &curoff,
						   direct ^ REVERSE, FALSE);
					goto success;
				}
				else
				{
					return FALSE;
				}
			}
			if (mcptr->mc_type == EOL)
			{
				if (curoff == 0)
				{
					(void)nextch(&curline, &curoff,
						   direct ^ REVERSE, FALSE);
					goto success;
				}
				else
				{
					return FALSE;
				}
			}
				

			/* Neither BOL nor EOL, so go through
			 * the meta-character equal function.
			 */
			if (!mceq(c, mcptr))
				return FALSE;
		}

		/* Increment the length counter and
		 * advance the pattern pointer.
		 */
		matchlen++;
		mcptr++;
	}			/* End of mcptr loop.*/

	/* A SUCCESSFULL MATCH!!!
	 * Reset the "." pointers.
	 */
success:
	*pcwline = curline;
	*pcwoff  = curoff;
	

	return TRUE;
}
#endif

/*
 * litmatch -- Search for a literal match in either direction.
 */
litmatch(patptr, direct, pcwline, pcwoff)
register char	*patptr;	/* string to scan for */
int		direct;		/* which way to go.*/
LINE		**pcwline;	/* current line during scan */
int		*pcwoff;	/* position within current line */
{
	register int c;			/* character at current position */
	LINE *curline;		/* current line during scan */
	int curoff;		/* position within current line */

	/* Set up local scan pointers to ".", and get
	 * the current character.  Then loop around
	 * the pattern pointer until success or failure.
	 */
	curline = *pcwline;
	curoff = *pcwoff;

	while (*patptr != '\0') {
		c = nextch(&curline, &curoff, direct, FALSE);

		if (!eq(c, *patptr))
			return FALSE;

		/* Increment the length counter and
		 * advance the pattern pointer.
		 */
		matchlen++;
		patptr++;
	}

	*pcwline = curline;
	*pcwoff  = curoff;
	

	return TRUE;
}


/*
 * eq -- Compare two characters.  The "bc" comes from the buffer, "pc"
 *	from the pattern.  If we are not in EXACT mode, fold out the case.
 */
int	
eq(bc, pc)
register char	bc;
register char	pc;
{
	if (curwp->w_bufp->b_mode & MDEXACT)
		return (bc ^ pc) == 0;
	/* take out the bit that makes upper and lowercase different */
	return ((bc ^ pc) & ~DIFCASE) == 0;
}

/*
 * readpattern -- Read a pattern.  Stash it in apat.  If it is the
 *	search string, create the reverse pattern and the magic
 *	pattern, assuming we are in MAGIC mode (and defined that way).
 *	Apat is not updated if the user types in an empty line.  If
 *	the user typed an empty line, and there is no old pattern, it is
 *	an error.  Display the old pattern, in the style of Jeff Lomicka.
 *	There is some do-it-yourself control expansion.
 *	An alternate termination character is passed in.
 */
readpattern(prompt, apat, srch, c, fromscreen)
char	*prompt;
char	*apat;
int	srch;
int	fromscreen;
{
	int status;

	/* Read a pattern.  Either we get one,
	 * or we just get the META charater, and use the previous pattern.
	 * Then, if it's the search string, make a reversed pattern.
	 * *Then*, make the meta-pattern, if we are defined that way.
	 */
	if (fromscreen) {
	 	status = screen_string(apat, NPAT, _ident);
		if (status != TRUE)
			return status;
	} else {
	 	status = kbd_string(prompt, apat, NPAT, c, NO_EXPAND);
	}
 	if (status == TRUE) {
		if (srch) {	/* If we are doing the search string.*/
			mlenold = matchlen = strlen(apat);
			/* Reverse string copy.
			 */
			rvstrcpy(tap, apat);
#if	MAGIC
			/* Only make the meta-pattern if in magic mode,
			 * since the pattern in question might have an
			 * invalid meta combination.
			 */
			if ((curwp->w_bufp->b_mode & MDMAGIC) == 0)
				mcclear();
			else
				status = mcstr();
#endif
		}
	} else if (status == FALSE && *apat != 0) { /* Old one */
		status = TRUE;
	}

	return status;
}

/*
 * savematch -- We found the pattern?  Let's save it away.
 */

savematch()
{
	register char *ptr;	/* ptr into malloced last match string */
	register int j;		/* index */
	LINE *curline;		/* line of last match */
	int curoff;		/* offset "      "    */

	/* free any existing match string */
	if (patmatch != NULL)
		free(patmatch);

	/* attempt to allocate a new one */
	ptr = patmatch = malloc(matchlen + 1);
	if (ptr == NULL)
		return;

	/* save the match! */
	curoff = matchoff;
	curline = matchline;

	for (j = 0; j < matchlen; j++)
		*ptr++ = nextch(&curline, &curoff, FORWARD, FALSE);

	/* null terminate the match string */
	*ptr = '\0';
}

/*
 * rvstrcpy -- Reverse string copy.
 */
rvstrcpy(rvstr, str)
register char	*rvstr, *str;
{
	register int i;

	str += (i = strlen(str));

	while (i-- > 0)
		*rvstr++ = *--str;

	*rvstr = '\0';
}

rvstrncpy(rvstr, str, n)
register char	*rvstr, *str;
{
	register int i;

	i = strlen(str);
	if (n < i) i = n;

	str += i;

	while (i-- > 0)
		*rvstr++ = *--str;

	*rvstr = '\0';
}

#if BEFORE
/*
 * sreplace -- Search and replace.
 */
sreplace(f, n)
{
	return replaces(FALSE, f, n);
}

/*
 * qreplace -- search and replace with query.
 */
qreplace(f, n)
{
	register int s;
	s = replaces(TRUE, f, n);
#if	NeWS	/* user must not buffer output */
	newsimmediateoff() ;
#endif
	return(s) ;
}

/*
 * replaces -- Search for a string and replace it with another
 *	string.  Query might be enabled (according to kind).
 * f,n unused
 */
replaces(kind, f, n)
int	kind;	/* Query enabled flag */
{
	register int status;	/* success flag on pattern inputs */
	register int rlength;	/* length of replacement string */
	register int numsub;	/* number of substitutions */
	int nlflag;		/* last char of search string a <NL>? */
	int nlrepl;		/* was a replace done on the last line? */
	int c;			/* input char for query */
	char tpat[NPAT];	/* temporary to hold search pattern */
	LINE *lastline;		/* position of last replace and */
	int lastoff;		/* offset (for 'u' query option) */
	REGION region;

	/* we don't really care much about size, etc., but getregion does
		a nice job of scanning for the mark, so we use it.  Then
		we pretend that wrapping around the bottom of the file is
		okay, and use the boundary code to keep us from going
		past the region end. */
	if (curwp->w_mkp == curwp->w_dotp) {
		if (curwp->w_doto > curwp->w_mko) {
			int tmpoff;
			tmpoff = curwp->w_doto;
			curwp->w_doto = curwp->w_mko;
			curwp->w_mko = tmpoff;
		}
	} else {
		getregion(&region);
		if (region.r_linep == curwp->w_mkp)
			swapmark();
	}
	if (fulllineregions) {
		curwp->w_doto = 0;
		curwp->w_mko = llength(curwp->w_mkp);
	}
	if (curwp->w_mkp == curbp->b_linep) {
		mlwrite("BUG: mark is at b_linep");
		return FALSE;
	}

	/* Ask the user for the text of a pattern.
	 */
	if ((status = readpattern(
	    (kind == FALSE ? "Replace: " : "Query replace: "),
	    		 &pat[0], TRUE, '\n', FALSE)) != TRUE)
		return status;

	/* Ask for the replacement string.
	 */
	if ((status = readpattern("with: ", &rpat[0], FALSE, '\n', FALSE))
								== ABORT)
		return status;

	/* Find the length of the replacement string.
	 */
	rlength = strlen(&rpat[0]);

	/* Set up flags so we can make sure not to do a recursive
	 * replace on the last line.
	 */
	nlflag = (pat[matchlen - 1] == '\n');
	nlrepl = FALSE;

	if (kind)
	{
		/* Build query replace question string.
		 */
		strcpy(tpat, "Replace '");
		expandp(&pat[0], &tpat[strlen(tpat)], NPAT/3);
		strcat(tpat, "' with '");
		expandp(&rpat[0], &tpat[strlen(tpat)], NPAT/3);
		strcat(tpat, "'? ");

		/* Initialize last replaced pointers.
		 */
		lastline = NULL;
		lastoff = 0;
#if	NeWS
		newsimmediateon() ;
#endif
	}

	numsub = 0;

	while ( nlflag == FALSE || nlrepl == FALSE ) {

		if (interrupted) return ABORT;

		/* Search for the pattern.
		 * If we search with a regular expression,
		 * matchlen is reset to the true length of
		 * the matched string.
		 */
		setboundry(TRUE,curwp->w_mkp,curwp->w_mko,FORWARD);
#if	MAGIC
		if (magical && (curwp->w_bufp->b_mode & MDMAGIC) != 0) {
			if (!thescanner(&mcpat[0], FORWARD, PTBEG, TRUE))
				break;
		} else
#endif
			if (!thescanner(&pat[0], FORWARD, PTBEG, TRUE))
				break;		/* all done */

		/* Check if we are on the last line.
		 */
		nlrepl = (lforw(curwp->w_dotp) == curwp->w_bufp->b_linep);

		/* Check for query.
		 */
		if (kind)
		{
			/* Get the query.
			 */
pprompt:		mlwrite(&tpat[0], &pat[0], &rpat[0]);
qprompt:
			update(TRUE);  /* show the proposed place to change */
nprompt:
			c = kbd_key();			/* and input */
			mlwrite("");			/* and clear it */

			if (c == abortc) {
				mlwrite("[Aborted]");
				return(FALSE);
			}

			/* And respond appropriately.
			 */
			switch (c)
			{
				case tocntrl('Q'):
				case tocntrl('S'):
					goto nprompt;

				case 'y':	/* yes, substitute */
					savematch();
					break;

				case 'n':	/* no, onward */
					nextch( &(curwp->w_dotp),
						&(curwp->w_doto),FORWARD,TRUE);
					continue;

				case '!':	/* yes/stop asking */
					kind = FALSE;
					break;

				case 'u':	/* undo last and re-prompt */

					/* Restore old position.
					 */
					if (lastline == NULL)
					{
						/* There is nothing to undo.
						 */
						TTbeep();
						goto pprompt;
					}
					curwp->w_dotp = lastline;
					curwp->w_doto = lastoff;
					lastline = NULL;
					lastoff = 0;

					/* Delete the new string.
					 */
					backchar(FALSE, rlength);
					status = delins(rlength, patmatch);
					if (status != TRUE) {
						return (status);
					}

					/* Record one less substitution,
					 * backup, and reprompt.
					 */
					--numsub;
					backchar(TRUE, mlenold);
					matchline = curwp->w_dotp;
					matchoff  = curwp->w_doto;
					goto pprompt;

				default:	/* bitch and beep */
					TTbeep();

				case 'h':
				case '?':	/* help me */
					mlwrite(
			"(Y)es, (N)o, (!)Do rest, (U)ndo last, (ESC)Abort: ");
					goto qprompt;

			}	/* end of switch */
		}	/* end of "if kind" */

		/*
		 * Delete the sucker, and insert its
		 * replacement.
		 */
		status = delins(matchlen, &rpat[0]);
		if (status != TRUE) {
			return (status);
		}

		/* Save where we are if we might undo this....
		 */
		if (kind)
		{
			lastline = curwp->w_dotp;
			lastoff = curwp->w_doto;
		}

		numsub++;	/* increment # of substitutions */
	}

	/* And report the results.
	 */
	mlwrite("%d substitutions", numsub);
	return(TRUE);
}

/*
 * delins -- Delete a specified length from the current
 *	point, then insert the string.
 */
delins(dlength, instr)
int	dlength;
char	*instr;
{
	int	status;
	char	tmpc;

	/* Zap what we gotta,
	 * and insert its replacement.
	 */
	if (!(status = ldelete((long) dlength, FALSE)))
	{
		mlwrite("Error while deleting");
		return(FALSE);
	} else {
		while (tmpc = *instr)
		{
			status = (tmpc == '\n'? lnewline(): linsert(1, tmpc));

			/* Insertion error?
			 */
			if (!status)
			{
				mlwrite("Out of memory while inserting");
				break;
			}
			instr++;
		}
	}
	return (status);
}
#endif

/*
 * expandp -- Expand control key sequences for output.
 */
expandp(srcstr, deststr, maxlength)
char *srcstr;	/* string to expand */
char *deststr;	/* destination of expanded string */
int maxlength;	/* maximum chars in destination */
{
	unsigned char c;	/* current char to translate */

	/* Scan through the string.
	 */
	while ((c = *srcstr++) != 0)
	{
		if (c == '\n')		/* it's a newline */
		{
			*deststr++ = '<';
			*deststr++ = 'N';
			*deststr++ = 'L';
			*deststr++ = '>';
			maxlength -= 4;
		}
		else if (!isprint(c))	/* control character */
		{
			*deststr++ = '^';
			*deststr++ = toalpha(c);
			maxlength -= 2;
		}
		else if (c == '%')
		{
			*deststr++ = '%';
			*deststr++ = '%';
			maxlength -= 2;
		}
		else			/* any other character */
		{
			*deststr++ = c;
			maxlength--;
		}

		/* check for maxlength */
		if (maxlength < 4)
		{
			*deststr++ = '$';
			*deststr = '\0';
			return(FALSE);
		}
	}
	*deststr = '\0';
	return(TRUE);
}

/*
 * boundry -- Return information depending on whether we may search no
 *	further.  Beginning of file and end of file are the obvious
 *	cases, but we may want to add further optional boundary restrictions
 *	in future, a' la VMS EDT.  At the moment, just return TRUE or
 *	FALSE depending on if a boundary is hit (ouch).
 */
int	
boundry(curline, curoff)
LINE	*curline;
int	curoff;
{
	return (curline == boundline) && (curoff == boundoff);
}

setboundry(wrapok,lp,off,dir)
LINE *lp;
{
	if (wrapok) {
		(void)nextch(&lp,&off,dir,TRUE);
		boundline = lp;
		boundoff = off;
	} else {
		boundline = curbp->b_linep;
		boundoff = 0;
	}
}

/*
 * nextch -- retrieve the next/previous character in the buffer,
 *	and advance/retreat the point.
 *	The order in which this is done is significant, and depends
 *	upon the direction of the search.  Forward searches look at
 *	the current character and move, reverse searches move and
 *	look at the character.
 */
nextch(pcurline, pcuroff, dir, wrapok)
LINE	**pcurline;
int	*pcuroff;
int	dir;
{
	register LINE	*curline;
	register int	curoff;
	register int	c;

	/* dummy up a -1, which will never match anything, as the only
		character on the header line */
	
	curline = *pcurline;
	curoff = *pcuroff;
	if (dir == FORWARD) {
		if (curoff == ((curline == curbp->b_linep)?1:llength(curline)))
		{	/* if at EOL */
			curline = lforw(curline);	/* skip to next line */
			curoff = 0;
			c = '\n';			/* and return a <NL> */
		} else {
			c = lgetc(curline, curoff++);	/* get the char */
			if (curline == curbp->b_linep)
				c = -1;
		}
	} else	 {		/* Reverse.*/
		if (curoff == 0) {
			curline = lback(curline);
			curoff = (curline == curbp->b_linep)?1:llength(curline);
			c = '\n';
		} else {
			c = lgetc(curline, --curoff);
			if (curline == curbp->b_linep)
				c = -1;
		}
	}
	*pcurline = curline;
	*pcuroff = curoff;

	return (c);
}

#if	MAGIC
/*
 * mcstr -- Set up the 'magic' array.  The closure symbol is taken as
 *	a literal character when (1) it is the first character in the
 *	pattern, and (2) when preceded by a symbol that does not allow
 *	closure, such as a newline, beginning of line symbol, or another
 *	closure symbol.
 *
 *	Coding comment (jmg):  yes, i know i have gotos that are, strictly
 *	speaking, unnecessary.  But right now we are so cramped for
 *	code space that i will grab what i can in order to remain
 *	within the 64K limit.  C compilers actually do very little
 *	in the way of optimizing - they expect you to do that.
 */
int 
mcstr()
{
	MC	*mcptr, *rtpcm;
	char	*patptr;
 	int	mj;
 	int	pchr;
 	int	status = TRUE;
 	int	does_closure = FALSE;

	/* If we had metacharacters in the MC array previously,
	 * free up any bitmaps that may have been allocated.
	 */
	if (magical)
		mcclear();

	magical = FALSE;
	mj = 0;
	mcptr = &mcpat[0];
	patptr = &pat[0];

	while ((pchr = *patptr) && status)
	{
		switch (pchr)
		{
			case MC_CCL:
				status = cclmake(&patptr, mcptr);
				magical = TRUE;
				does_closure = TRUE;
				break;
			case MC_BOL:
				if (mj != 0)
					goto litcase;

				mcptr->mc_type = BOL;
				magical = TRUE;
				does_closure = FALSE;
				break;
			case MC_EOL:
				if (*(patptr + 1) != '\0')
					goto litcase;

				mcptr->mc_type = EOL;
				magical = TRUE;
				does_closure = FALSE;
				break;
			case MC_ANY:
				mcptr->mc_type = ANY;
				magical = TRUE;
				does_closure = TRUE;
				break;
			case MC_CLOSURE:
				/* Does the closure symbol mean closure here?
				 * If so, back up to the previous element
				 * and indicate it is enclosed.
				 */
				if (!does_closure)
					goto litcase;
				mj--;
				mcptr--;
				mcptr->mc_type |= CLOSURE;
				magical = TRUE;
				does_closure = FALSE;
				break;

			/* Note: no break between MC_ESC case and the default.
			 */
			case MC_ESC:
				if (*(patptr + 1) != '\0')
				{
					pchr = *++patptr;
					magical = TRUE;
				}
			default:
litcase:			mcptr->mc_type = LITCHAR;
				mcptr->u.lchar = pchr;
				does_closure = (pchr != '\n');
				break;
		}		/* End of switch.*/
		mcptr++;
		patptr++;
		mj++;
	}		/* End of while.*/

	/* Close off the meta-string.
	 */
	mcptr->mc_type = MCNIL;

	/* Set up the reverse array, if the status is good.  Please note the
	 * structure assignment - your compiler may not like that.
	 * If the status is not good, nil out the meta-pattern.
	 * The only way the status would be bad is from the cclmake()
	 * routine, and the bitmap for that member is guarenteed to be
	 * freed.  So we stomp a MCNIL value there, and call mcclear()
	 * to free any other bitmaps.
	 */
	if (status)
	{
		rtpcm = &tapcm[0];
		while (--mj >= 0)
		{
#if	LATTICE
			movmem(--mcptr, rtpcm++, sizeof (MC));
#endif

#if	MWC86 | AZTEC | MSC | TURBO | VMS | USG | BSD | V7
			*rtpcm++ = *--mcptr;
#endif
		}
		rtpcm->mc_type = MCNIL;
	}
	else
	{
		(--mcptr)->mc_type = MCNIL;
		mcclear();
	}

	return(status);
}

/*
 * mcclear -- Free up any CCL bitmaps, and MCNIL the MC arrays.
 */
mcclear()
{
	register MC	*mcptr;

	mcptr = &mcpat[0];

	while (mcptr->mc_type != MCNIL)
	{
		if ((mcptr->mc_type & MASKCL) == CCL ||
		    (mcptr->mc_type & MASKCL) == NCCL)
			free(mcptr->u.cclmap);
		mcptr++;
	}
	mcpat[0].mc_type = tapcm[0].mc_type = MCNIL;
}

/*
 * mceq -- meta-character equality with a character.  In Kernighan & Plauger's
 *	Software Tools, this is the function omatch(), but i felt there
 *	were too many functions with the 'match' name already.
 */
mceq(bc, mt)
int	bc;
MC	*mt;
{
	register int result;
	
	if (bc < 0)
		return FALSE;

	switch (mt->mc_type & MASKCL)
	{
		case LITCHAR:
			result = eq(bc, mt->u.lchar);
			break;

		case ANY:
			result = (bc != '\n');
			break;

		case CCL:
			if (!(result = biteq(bc, mt->u.cclmap)))
			{
				if ((curwp->w_bufp->b_mode & MDEXACT) == 0 &&
				    isalpha(bc))
				{
					result = biteq(CHCASE(bc),mt->u.cclmap);
				}
			}
			break;

		case NCCL:
			
			if (bc == '\n') {
				result = FALSE;
			} else {
				result = !biteq(bc, mt->u.cclmap);

				if ((curwp->w_bufp->b_mode & MDEXACT) == 0 &&
					    isalpha(bc))
				{
					result &= !biteq(CHCASE(bc),
								 mt->u.cclmap);
				}
			}
			break;

		default:
			mlwrite("mceq: what is %d?", mt->mc_type);
			result = FALSE;
			break;

	}	/* End of switch.*/

	return (result);
}

/*
 * cclmake -- create the bitmap for the character class.
 *	ppatptr is left pointing to the end-of-character-class character,
 *	so that a loop may automatically increment with safety.
 */
cclmake(ppatptr, mcptr)
char	**ppatptr;
MC	*mcptr;
{
	BITMAP		bmap;
	register char	*patptr;
	register int	pchr, ochr;

	if ((bmap = clearbits()) == NULL)
	{
		mlwrite("Out of memory");
		return FALSE;
	}

	mcptr->u.cclmap = bmap;
	patptr = *ppatptr;

	/*
	 * Test the initial character(s) in ccl for
	 * special cases - negate ccl, or an end ccl
	 * character as a first character.  Anything
	 * else gets set in the bitmap.
	 */
	if (*++patptr == MC_NCCL)
	{
		patptr++;
		mcptr->mc_type = NCCL;
	}
	else
		mcptr->mc_type = CCL;

	if ((ochr = *patptr) == MC_ECCL)
	{
		mlwrite("Empty character class");
		return (FALSE);
	}
	else
	{
		if (ochr == MC_ESC)
			ochr = *++patptr;

		setbit(ochr, bmap);
		patptr++;
	}

	while (ochr != '\0' && (pchr = *patptr) != MC_ECCL)
	{
		switch (pchr)
		{
			/* Range character loses its meaning
			 * if it is the last character in
			 * the class.
			 */
			case MC_RCCL:
				if (*(patptr + 1) == MC_ECCL)
					setbit(pchr, bmap);
				else
				{
					pchr = *++patptr;
					while (++ochr <= pchr)
						setbit(ochr, bmap);
				}
				break;

			/* Note: no break between case MC_ESC and the default.
			 */
			case MC_ESC:
				pchr = *++patptr;
			default:
				setbit(pchr, bmap);
				break;
		}
		patptr++;
		ochr = pchr;
	}

	*ppatptr = patptr;

	if (ochr == '\0')
	{
		mlwrite("Missing '%c'",MC_ECCL);
		free(bmap);
		return FALSE;
	}
	return TRUE;
}

/*
 * biteq -- is the character in the bitmap?
 */
biteq(bc, cclmap)
int	bc;
BITMAP	cclmap;
{
	if (bc >= HICHAR || bc < 0)
		return FALSE;

	return( (*(cclmap + (bc >> 3)) & BIT(bc & 7))? TRUE: FALSE );
}

/*
 * clearbits -- Allocate and zero out a CCL bitmap.
 */
BITMAP 
clearbits()
{

	BITMAP		cclstart, cclmap;
	register int	j;

	if ((cclmap = cclstart = (BITMAP) malloc(HIBYTE)) != NULL)
		for (j = 0; j < HIBYTE; j++)
			*cclmap++ = 0;

	return (cclstart);
}

/*
 * setbit -- Set a bit (ON only) in the bitmap.
 */
void 
setbit(bc, cclmap)
int	bc;
BITMAP	cclmap;
{
	if (bc < HICHAR && bc >= 0)
		*(cclmap + (bc >> 3)) |= BIT(bc & 7);
}
#endif
