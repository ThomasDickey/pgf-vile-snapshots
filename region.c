/*
 * The routines in this file
 * deal with the region, that magic space
 * between "." and mark. Some functions are
 * commands. Some functions are just for
 * internal use.
 *
 * $Log: region.c,v $
 * Revision 1.5  1991/08/07 12:35:07  pgf
 * added RCS log messages
 *
 * revision 1.4
 * date: 1991/06/25 19:53:18;
 * massive data structure restructure
 * 
 * revision 1.3
 * date: 1991/06/16 17:40:10;
 * fixed bug in shiftlregion due to not stopping at end of line when scanning
 * for spaces, and
 * added do_fl_region() wrapper routine to do full-line region processing, and
 * rewrote shift[lr]region to use it
 * 
 * revision 1.2
 * date: 1991/05/31 11:21:54;
 * added endline pointer and offset to region struct and getregion() code
 * 
 * revision 1.1
 * date: 1990/09/21 10:25:58;
 * initial vile RCS revision
 */
#include        <stdio.h>
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
killregion(f, n)
{
        register int    s;
        REGION          region;

        if ((s=getregion(&region)) != TRUE)
                return s;
	kregcirculate(TRUE);
        kdelete();                      /* command, so do magic */
	DOT = region.r_orig;
	if (fulllineregions)
		kregflag |= KLINES;
	s = ldelete(region.r_size, TRUE);
	ukb = 0;
        return s;
}

/*
 * Copy all of the characters in the
 * region to the kill buffer. Don't move dot
 * at all. This is a bit like a kill region followed
 * by a yank.
 */
yankregion(f, n)
{
	MARK		m;
        register int    s;
        REGION          region;

        if ((s=getregion(&region)) != TRUE)
                return s;
	kregcirculate(TRUE);
        kdelete();
	m = region.r_orig;
	if (fulllineregions)
		kregflag |= KLINES|KYANK;
        while (region.r_size--) {
                if (is_at_end_of_line(m)) {  /* End of line.         */
                        if ((s=kinsert('\n')) != TRUE) {
				ukb = 0;
                                return s;
			}
                        m.l = lforw(m.l);
                        m.o = 0;
                } else {                        /* Middle of line.      */
                        if ((s=kinsert(char_at(m))) != TRUE) {
				ukb = 0;
                                return s;
			}
                        ++m.o;
                }
        }
	mlwrite("[region yanked]");
	ukb = 0;
        return TRUE;
}

/*
 * insert a tab at the front of the line
 */
shift_right_line()
{
	return tab(FALSE,1);
}

/*
 * shift region right by a tab stop
 */
shiftrregion(f, n)
{
	return do_fl_region(shift_right_line);
}

/*
 * delete a tab-equivalent from the front of the line
 */
shift_left_line()
{
        register int    c;
	register int	i;
	register int	lim;
	register int	s;
	register LINE *linep = DOT.l;

	i = 0;

	/* examine the line to the end, or the first tabstop, whichever
		comes first */
	lim = (TABVAL < llength(linep)) ? TABVAL:llength(linep);

	/* count the leading spaces */
	while ((c = lgetc(linep,i)) == ' ' && i < lim)
		i++;

	/* the i'th char is _not_ a space, or else we hit lim */
	if (c == '\t' && i < lim) /* ith char is tab? */
		i++;

	if (i != 0) { /* did we find space/tabs to kill? */
		if ((s = ldelete((long)i,FALSE)) != TRUE)
			return s;
	}
	return TRUE;
}

/*
 * shift region left by a tab stop
 */
shiftlregion(f, n)
{
	return do_fl_region(shift_left_line);
}

_to_lower(c)
{
	if (isupper(c))
		return c ^ DIFCASE;
	return -1;
}

_to_upper(c)
{
	if (islower(c))
		return c ^ DIFCASE;
	return -1;
}

_to_caseflip(c)
{
	if (isalpha(c))
		return c ^ DIFCASE;
	return -1;
}

flipregion(f, n)
{
	return charprocreg(f,n,_to_caseflip);
}

lowerregion(f, n)
{
	return charprocreg(f,n,_to_lower);
}

upperregion(f, n)
{
	return charprocreg(f,n,_to_upper);
}

charprocreg(f, n, func)
int (*func)();
{
	MARK		m;
        register int    c,nc;
        register int    s;
        REGION          region;

        if ((s=getregion(&region)) != TRUE)
                return s;
        lchange(WFHARD);
	m = region.r_orig;
        while (region.r_size--) {
                if (is_at_end_of_line(m)) {
                        m.l = lforw(m.l);
                        m.o = 0;
                } else {
                        c = char_at(m);
			nc = (func)(c);
			if (nc != -1) {
				copy_for_undo(m.l);
                                put_char_at(m, nc);
			}
                        ++m.o;
                }
        }
        return TRUE;
}

/*
 * This routine figures out the
 * bounds of the region in the current window, and
 * fills in the fields of the "REGION" structure pointed
 * to by "rp". Because the dot and mark are usually very
 * close together, we scan outward from dot looking for
 * mark. This should save time. Return a standard code.
 */
getregion(rp)
register REGION *rp;
{
        register LINE   *flp;
        register LINE   *blp;
        long fsize;
        long bsize;

        if (MK.l == NULL) {
                mlwrite("Bug: getregion: no mark set in this window");
                return FALSE;
        }
        if (sameline(DOT, MK)) {
		if (fulllineregions) {
	                rp->r_orig.l = DOT.l;
                        rp->r_orig.o = 0;
	                rp->r_end.l = lforw(DOT.l);
                        rp->r_end.o = 0;
                        rp->r_size = (long)llength(DOT.l)+1;
			return TRUE;
		}
                rp->r_orig.l = rp->r_end.l = DOT.l;
                if (DOT.o < MK.o) {
                        rp->r_orig.o = DOT.o;
                        rp->r_end.o = MK.o;
                        rp->r_size = (long)(MK.o-DOT.o);
                } else {
                        rp->r_orig.o = MK.o;
                        rp->r_end.o = DOT.o;
                        rp->r_size = (long)(DOT.o-MK.o);
                }
                return TRUE;
        }
        blp = DOT.l;
        flp = DOT.l;
	if (fulllineregions) {
		bsize = (long)(llength(blp)+1);
		fsize = (long)(llength(flp)+1);
	} else {
		bsize = (long)DOT.o;
		fsize = (long)(llength(flp) - DOT.o + 1);
	}
        while ((flp!=curbp->b_line.l) || (lback(blp) != curbp->b_line.l)) {
                if (flp != curbp->b_line.l) {
                        flp = lforw(flp);
                        if (flp == MK.l) {
                                rp->r_orig = DOT;
				if (fulllineregions) {
					rp->r_orig.o = 0;
					rp->r_size = fsize + llength(MK.l)+1;
	                                rp->r_end.l = lforw(flp);
					rp->r_end.o = 0;
	                                return TRUE;
				}
				rp->r_size = fsize + MK.o;
                                rp->r_end = MK;
                                return TRUE;
                        }
                        fsize += llength(flp)+1;
                }
                if (lback(blp) != curbp->b_line.l) {
                        blp = lback(blp);
                        bsize += llength(blp)+1;
                        if (blp == MK.l) {
                                rp->r_orig = MK;
				if (fulllineregions) {
					rp->r_orig.o = 0;
					rp->r_size = bsize;
	                                rp->r_end.l = lforw(DOT.l);
					rp->r_end.o = 0;
	                                return TRUE;
				}
                                rp->r_end = DOT;
				rp->r_size = bsize - MK.o;
                                return TRUE;
                        }
                }
        }
        mlwrite("Bug: lost mark");
        return FALSE;
}

do_fl_region(lineprocfunc)
int (*lineprocfunc)();
{
        register LINE   *linep;
        register int    s;
        REGION          region;

	fulllineregions = TRUE;
        if ((s=getregion(&region)) != TRUE)
                return s;

	/* for each line in the region, ... */
	for ( linep = region.r_orig.l;
			linep != region.r_end.l; linep = lforw(linep) ) {

		/* nothing on the line? */
		if (llength(linep) == 0)
			continue;

		/* move through the region... */
		DOT.l = linep;
		DOT.o = 0;

		/* ...and process each line */
		if ((s = (*lineprocfunc)()) != TRUE)
			return s;

        }
	firstnonwhite(FALSE,1);
        return TRUE;
}

