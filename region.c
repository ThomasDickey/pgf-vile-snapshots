/*
 * The routines in this file
 * deal with the region, that magic space
 * between "." and mark. Some functions are
 * commands. Some functions are just for
 * internal use.
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

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
        if ((s=getregion(&region)) != TRUE)
                return (s);
	kregcirculate(TRUE);
        kdelete();                      /* command, so do magic */
        curwp->w_dotp = region.r_linep;
        curwp->w_doto = region.r_offset;
	if (fulllineregions)
		kregflag |= KLINES;
	s = ldelete(region.r_size, TRUE);
	ukb = 0;
        return (s);
}

/*
 * Copy all of the characters in the
 * region to the kill buffer. Don't move dot
 * at all. This is a bit like a kill region followed
 * by a yank.
 */
yankregion(f, n)
{
        register LINE   *linep;
        register int    loffs;
        register int    s;
        REGION          region;

        if ((s=getregion(&region)) != TRUE)
                return (s);
	kregcirculate(TRUE);
        kdelete();
        linep = region.r_linep;                 /* Current line.        */
        loffs = region.r_offset;                /* Current offset.      */
	if (fulllineregions)
		kregflag |= KLINES|KYANK;
        while (region.r_size--) {
                if (loffs == llength(linep)) {  /* End of line.         */
                        if ((s=kinsert('\n')) != TRUE) {
				ukb = 0;
                                return (s);
			}
                        linep = lforw(linep);
                        loffs = 0;
                } else {                        /* Middle of line.      */
                        if ((s=kinsert(lgetc(linep, loffs))) != TRUE) {
				ukb = 0;
                                return (s);
			}
                        ++loffs;
                }
        }
	mlwrite("[region yanked]");
	ukb = 0;
        return (TRUE);
}

/*
 * shift region left by a tab stop
 */
shiftrregion(f, n)
{
        register LINE   *linep;
        register int    loffs;
        register int    s;
        REGION          region;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
        if ((s=getregion(&region)) != TRUE)
                return (s);
        linep = region.r_linep;
        loffs = region.r_offset;
	if (loffs != 0) {  /* possibly on first time through */
		region.r_size -= llength(linep)-loffs+1;
		loffs = 0;
		linep = lforw(linep);
	}
	s = TRUE;
	while (region.r_size > 0) {
		/* adjust r_size now, while line length is right */
		region.r_size -= llength(linep)+1;
		if (llength(linep) != 0) {
			curwp->w_dotp = linep;
			curwp->w_doto = 0;
			if ((s = linsert(1,'\t')) != TRUE)
				return (s);
		}
		linep = lforw(linep);
        }
	firstnonwhite(f,n);
	return (TRUE);
}

/*
 * shift region left by a tab stop
 */
shiftlregion(f, n)
{
        register LINE   *linep;
        register int    loffs;
        register int    c;
        register int    s;
	register int	i;
        REGION          region;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
        if ((s=getregion(&region)) != TRUE)
                return (s);
        linep = region.r_linep;
        loffs = region.r_offset;
	if (loffs != 0) {  /* possibly on first time through */
		region.r_size -= llength(linep)-loffs+1;
		loffs = 0;
		linep = lforw(linep);
	}
	s = TRUE;
	while (region.r_size > 0) {
		/* adjust r_size now, while line length is right */
		region.r_size -= llength(linep)+1;
		if (llength(linep) != 0) {
			curwp->w_dotp = linep;
			curwp->w_doto = 0;
			if ((c = lgetc(linep,0)) == '\t') { /* delete the tab */
				i = 1;
			} else if (c == ' ') {
				i = 1; 
				/* after this, i'th char is _not_ a space, 
					or 0-7 are spaces  */
				while ((c = lgetc(linep,i)) == ' ' && i < TABVAL)
					i++;
				if (i != TABVAL && c == '\t') /* ith char is tab */
					i++;
			} else {
				i = 0;
			}
			
			if ( i!=0 && (s = ldelete((long)i,FALSE)) != TRUE)
				return (s);
		}
		linep = lforw(linep);
        }
	firstnonwhite(f,n);
        return (TRUE);
}

_to_lower(c)
{
	if (isupper(c))
		return(c ^ DIFCASE);
	return -1;
}

_to_upper(c)
{
	if (islower(c))
		return(c ^ DIFCASE);
	return -1;
}

_to_caseflip(c)
{
	if (isalpha(c))
		return(c ^ DIFCASE);
	return -1;
}

flipregion(f, n)
{
	return charprocregion(f,n,_to_caseflip);
}

lowerregion(f, n)
{
	return charprocregion(f,n,_to_lower);
}

upperregion(f, n)
{
	return charprocregion(f,n,_to_upper);
}

charprocregion(f, n, func)
int (*func)();
{
        register LINE   *linep;
        register int    loffs;
        register int    c,nc;
        register int    s;
        REGION          region;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
        if ((s=getregion(&region)) != TRUE)
                return (s);
        lchange(WFHARD);
        linep = region.r_linep;
        loffs = region.r_offset;
        while (region.r_size--) {
                if (loffs == llength(linep)) {
                        linep = lforw(linep);
                        loffs = 0;
                } else {
                        c = lgetc(linep, loffs);
			nc = (func)(c);
			if (nc != -1) {
				copy_for_undo(linep);
                                lputc(linep, loffs, nc);
			}
                        ++loffs;
                }
        }
        return (TRUE);
}

/*
 * This routine figures out the
 * bounds of the region in the current window, and
 * fills in the fields of the "REGION" structure pointed
 * to by "rp". Because the dot and mark are usually very
 * close together, we scan outward from dot looking for
 * mark. This should save time. Return a standard code.
 * Callers of this routine should be prepared to get
 * an "ABORT" status; we might make this have the
 * conform thing later.
 */
getregion(rp)
register REGION *rp;
{
        register LINE   *flp;
        register LINE   *blp;
        long fsize;
        long bsize;

        if (curwp->w_mkp == NULL) {
                mlwrite("No mark set in this window");
                return (FALSE);
        }
        if (curwp->w_dotp == curwp->w_mkp) {
                rp->r_linep = curwp->w_dotp;
		if (fulllineregions) {
                        rp->r_offset = 0;
                        rp->r_size = (long)llength(curwp->w_dotp)+1;
			return (TRUE);
		}
                if (curwp->w_doto < curwp->w_mko) {
                        rp->r_offset = curwp->w_doto;
                        rp->r_size = (long)(curwp->w_mko-curwp->w_doto);
                } else {
                        rp->r_offset = curwp->w_mko;
                        rp->r_size = (long)(curwp->w_doto-curwp->w_mko);
                }
                return (TRUE);
        }
        blp = curwp->w_dotp;
        flp = curwp->w_dotp;
	if (fulllineregions) {
		bsize = (long)(llength(blp)+1);
		fsize = (long)(llength(flp)+1);
	} else {
		bsize = (long)curwp->w_doto;
		fsize = (long)(llength(flp)-curwp->w_doto+1);
	}
        while (flp!=curbp->b_linep || lback(blp)!=curbp->b_linep) {
                if (flp != curbp->b_linep) {
                        flp = lforw(flp);
                        if (flp == curwp->w_mkp) {
                                rp->r_linep = curwp->w_dotp;
                                rp->r_offset = curwp->w_doto;
                                rp->r_size = fsize+curwp->w_mko;
				if (fulllineregions) {
					rp->r_offset = 0;
					rp->r_size = fsize+
						    llength(curwp->w_mkp)+1;
				} else {
					rp->r_offset = curwp->w_doto;
					rp->r_size = fsize+curwp->w_mko;
				}
                                return (TRUE);
                        }
                        fsize += llength(flp)+1;
                }
                if (lback(blp) != curbp->b_linep) {
                        blp = lback(blp);
                        bsize += llength(blp)+1;
                        if (blp == curwp->w_mkp) {
                                rp->r_linep = blp;
				if (fulllineregions) {
					rp->r_offset = 0;
					rp->r_size = bsize;
				} else {
					rp->r_offset = curwp->w_mko;
					rp->r_size = bsize - curwp->w_mko;
				}
                                return (TRUE);
                        }
                }
        }
        mlwrite("Bug: lost mark");
        return (FALSE);
}

