/*
 * The routines in this file
 * deal with the region, that magic space
 * between "." and mark. Some functions are
 * commands. Some functions are just for
 * internal use.
 *
 * $Log: region.c,v $
 * Revision 1.31  1994/02/22 11:03:15  pgf
 * truncated RCS log for 4.0
 *
 */

#include	"estruct.h"
#include        "edef.h"

#if	MEGAMAX & ST520
overlay	"region"
#endif

/*
 * Kill the region. Ask "getregion"
 * to figure out the bounds of the region.
 * Move "." to the start, and kill the characters.
 */
int
killregion()
{
	register int    status;
	REGION          region;

	if ((status = getregion(&region)) == TRUE) {
		kregcirculate(TRUE);
		ksetup();		/* command, so do magic */
		DOT = region.r_orig;
		if (fulllineregions)
			kregflag |= KLINES;
		status = ldelete(region.r_size, TRUE);
		kdone();
		ukb = 0;
	}
	rls_region();
	return status;
}

/*
 * Copy all of the characters in the
 * region to the kill buffer. Don't move dot
 * at all. This is a bit like a kill region followed
 * by a yank.
 */
int
yankregion()
{
	MARK		m;
	register int    status;
	REGION          region;

	if ((status = getregion(&region)) == TRUE) {
		kregcirculate(TRUE);
		ksetup();
		m = region.r_orig;
		if (fulllineregions)
			kregflag |= KLINES|KYANK;
		while (region.r_size-- > 0) {
			if (m.o >= lLength(m.l)) { /* On/past end of line. */
				if ((status = kinsert('\n')) != TRUE) {
					ukb = 0;
					rls_region();
					return status;
				}
				m.l = lFORW(m.l);
				m.o = w_left_margin(curwp);
#if OPT_B_LIMITS
				if (same_ptr(m.l, buf_head(curbp)))
					break;
#endif
			} else {                    /* Middle of line.      */
				if ((status = kinsert(char_at(m))) != TRUE) {
					ukb = 0;
					rls_region();
					return status;
				}
				++m.o;
			}
		}
		if (do_report(klines+(kchars!=0)))
			mlwrite("[%d line%s, %d character%s yanked]",
				klines, PLURAL(klines),
				kchars, PLURAL(kchars));
		kdone();
		ukb = 0;
	}
	rls_region();
	return (status);
}

/*
 * insert a shiftwidth at the front of the line
 * don't do it if we're in cmode and the line starts with '#'
 */
/*ARGSUSED*/
int
shift_right_line(flag)
int	flag;
{
	int s, t;
	if (b_val(curbp, MDCMOD) &&
		lLength(DOT.l) > 0 && char_at(DOT) == '#')
		return TRUE;
	s = curswval;
	t = curtabval;
	DOT.o = w_left_margin(curwp);
	if (s) {  /* try to just insert tabs if possible */
		if (b_val(curbp,MDTABINSERT) && s >= t && (s % t == 0)) {
			linsert(s/t, '\t');
		} else {
			detabline(TRUE);
			DOT.o = w_left_margin(curwp);
			linsert(s, ' ');
		}
		if (b_val(curbp,MDTABINSERT))
			entabline(TRUE);
	}
	return firstnonwhite(FALSE,1);
}

/*
 * shift region right by a tab stop
 */
int
shiftrregion()
{
	return do_fl_region(shift_right_line, 0);
}

/*
 * delete a shiftwidth-equivalent from the front of the line
 */
/*ARGSUSED*/
int
shift_left_line(flag)
int	flag;
{
	register int	i;
	register int	lim;
	register int	s;
	register LINE *linep = l_ref(DOT.l);

	s = curswval;

	detabline(TRUE);

	/* examine the line to the end, or the first shiftwidth, whichever
		comes first */
	lim = (s < llength(linep)) ? s : llength(linep);


	i = 0;
	/* count the leading spaces */
	while (lgetc(linep,i) == ' ' && i < lim)
		i++;

	if (i != 0) { /* did we find space/tabs to kill? */
		DOT.o = w_left_margin(curwp);
		if ((s = ldelete((long)i,FALSE)) != TRUE)
			return s;
	}

	DOT.o = w_left_margin(curwp);
	if (b_val(curbp,MDTABINSERT))
		entabline(TRUE);
	return TRUE;
}

/*
 * shift region left by a tab stop
 */
int
shiftlregion()
{
	return do_fl_region(shift_left_line, 0);
}

int
_to_lower(c)
int c;
{
	if (isupper(c))
		return c ^ DIFCASE;
	return -1;
}

int
_to_upper(c)
int c;
{
	if (islower(c))
		return c ^ DIFCASE;
	return -1;
}

int
_to_caseflip(c)
int c;
{
	if (isalpha(c))
		return c ^ DIFCASE;
	return -1;
}

int
flipregion()
{
	return charprocreg(_to_caseflip);
}

int
lowerregion()
{
	return charprocreg(_to_lower);
}

int
upperregion()
{
	return charprocreg(_to_upper);
}

int
charprocreg(func)
int (*func) P((int));
{
	MARK		m;
	register int    c,nc;
	register int    status;
	REGION          region;
	int 		changed = 0;

	if ((status = getregion(&region)) == TRUE) {
		m = region.r_orig;
		while (region.r_size-- > 0) {
			if (is_at_end_of_line(m)) {
				m.l = lFORW(m.l);
				m.o = w_left_margin(curwp);
			} else {
				c = char_at(m);
				nc = (func)(c);
				if (nc != -1) {
					copy_for_undo(m.l);
					put_char_at(m, nc);
					changed++;
				}
				++m.o;
			}
		}
	}
	rls_region();
	if (changed)
	    chg_buff(curbp, WFHARD);
	return (status);
}

/*
 * This routine figures out the
 * bounds of the region in the current window, and
 * fills in the fields of the "REGION" structure pointed
 * to by "rp". Because the dot and mark are usually very
 * close together, we scan outward from dot looking for
 * mark. This should save time. Return a standard code.
 */
int
getregion(rp)
register REGION *rp;
{
	register LINE   *flp;
	register LINE   *blp;
	B_COUNT fsize;
	B_COUNT bsize;

#if OPT_MAP_MEMORY
	rp->r_orig.l = null_ptr;
	rp->r_end.l  = null_ptr;
	l_region(rp);
#endif
	if (l_ref(MK.l) == NULL) {
		mlforce("BUG: getregion: no mark set in this window");
		return FALSE;
	}

#define line_length(lp) (llength(lp)+1)	/* length counting newline */
#define Line_Length(lp) (lLength(lp)+1)	/* length counting newline */
	if (sameline(DOT, MK)) {
		rp->r_orig =
		rp->r_end  = DOT;
		if (fulllineregions) {
			rp->r_orig.o =
			rp->r_end.o  = w_left_margin(curwp);
			rp->r_end.l  = lFORW(DOT.l);
			rp->r_size   = (B_COUNT)(Line_Length(DOT.l) - w_left_margin(curwp));
		} else {
			if (DOT.o < MK.o) {
				rp->r_orig.o = DOT.o;
				rp->r_end.o  = MK.o;
			} else {
				rp->r_orig.o = MK.o;
				rp->r_end.o  = DOT.o;
			}
			rp->r_size = rp->r_end.o - rp->r_orig.o;
		}
		return TRUE;
	}

	blp = l_ref(DOT.l);
	flp = l_ref(DOT.l);
	if (fulllineregions) {
		bsize = (B_COUNT)(line_length(blp) - w_left_margin(curwp));
		fsize = (B_COUNT)(line_length(flp) - w_left_margin(curwp));
	} else {
		bsize = (B_COUNT)(DOT.o - w_left_margin(curwp));
		fsize = (B_COUNT)(line_length(flp) - DOT.o);
	}
	while (1) {
		if (flp != l_ref(buf_head(curbp))) {
			flp = lforw(flp);
			if (flp != l_ref(buf_head(curbp)))
				fsize += line_length(flp) - w_left_margin(curwp);
			if (flp == l_ref(MK.l)) {
				rp->r_orig = DOT;
				rp->r_end  = MK;
				if (fulllineregions) {
					rp->r_orig.o =
					rp->r_end.o  = w_left_margin(curwp);
					rp->r_end.l  = lFORW(rp->r_end.l);
				} else {
					fsize -= (line_length(flp) - MK.o);
				}
				rp->r_size = fsize;
				return TRUE;
			}
		} else if (lback(blp) != l_ref(buf_head(curbp))) {
			blp = lback(blp);
			bsize += line_length(blp) - w_left_margin(curwp);
			if (blp == l_ref(MK.l)) {
				rp->r_orig = MK;
				rp->r_end  = DOT;
				if (fulllineregions) {
					rp->r_orig.o =
					rp->r_end.o  = w_left_margin(curwp);
					rp->r_end.l  = lFORW(rp->r_end.l);
				} else {
					bsize -= (MK.o - w_left_margin(curwp));
				}
				rp->r_size = bsize;
				return TRUE;
			}
		} else
			break;
	}
	mlforce("BUG: lost mark");
	return FALSE;
}

int
get_fl_region(rp)
REGION	*rp;
{
	int	status;

	fulllineregions = TRUE;
	status = getregion(rp);
	fulllineregions = FALSE;

	return status;
}

int
do_fl_region(lineprocfunc,arg)
int (*lineprocfunc) P((int));
int arg;
{
	register LINE   *linep;
	register int    status;
	REGION          region;

	fulllineregions = TRUE;
	if ((status=getregion(&region)) == TRUE) {

		/* for each line in the region, ... */
		for ( linep = l_ref(region.r_orig.l);
			linep != l_ref(region.r_end.l); linep = lforw(linep) ) {

			/* nothing on the line? */
			if (llength(linep) <= w_left_margin(curwp))
				continue;

			/* move through the region... */
			DOT.l = l_ptr(linep);
			DOT.o = w_left_margin(curwp);

			/* ...and process each line */
			if ((status = (*lineprocfunc)(arg)) != TRUE) {
				rls_region();
				return status;
			}

		}
		(void)firstnonwhite(FALSE,1);
	}
	rls_region();
	return status;
}
