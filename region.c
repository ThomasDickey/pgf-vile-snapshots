/*
 * The routines in this file
 * deal with the region, that magic space
 * between "." and mark. Some functions are
 * commands. Some functions are just for
 * internal use.
 *
 * $Log: region.c,v $
 * Revision 1.19  1992/12/04 09:20:58  foxharp
 * deleted unused assigns
 *
 * Revision 1.18  1992/12/02  09:13:16  foxharp
 * changes for "c-shiftwidth"
 *
 * Revision 1.17  1992/11/19  09:18:45  foxharp
 * name change of kdelete() to ksetup(), and close off with kdone()
 *
 * Revision 1.16  1992/06/01  20:42:15  foxharp
 * honor "tabinsert" mode in {en,de}tabline()
 *
 * Revision 1.15  1992/05/16  12:00:31  pgf
 * prototypes/ansi/void-int stuff/microsoftC
 *
 * Revision 1.14  1992/02/26  21:57:20  pgf
 * shift operators now go by shiftwidth, not tabstop
 *
 * Revision 1.13  1992/01/10  08:09:23  pgf
 * added arg to do_fl_region, which is passed on to the function it calls
 * vi pointer
 *
 * Revision 1.12  1992/01/05  00:06:13  pgf
 * split mlwrite into mlwrite/mlprompt/mlforce to make errors visible more
 * often.  also normalized message appearance somewhat.
 *
 * Revision 1.11  1991/11/27  10:09:09  pgf
 * don't do left shifts on lines starting with # when we're in C mode.
 * i'll bet someone's gonna complain about that.  let's give it a try
 * anyway.
 *
 * Revision 1.10  1991/11/13  20:09:27  pgf
 * X11 changes, from dave lemke
 *
 * Revision 1.9  1991/11/08  13:15:02  pgf
 * yankregion now reports how much was yanked (proposed by dave lemke)
 *
 * Revision 1.8  1991/11/03  17:46:30  pgf
 * removed f,n args from all region functions -- they don't use them,
 * since they're no longer directly called by the user
 *
 * Revision 1.7  1991/11/01  14:38:00  pgf
 * saber cleanup
 *
 * Revision 1.6  1991/10/28  14:26:15  pgf
 * eliminated TABVAL macro -- now use curtabval directly
 *
 * Revision 1.5  1991/08/07  12:35:07  pgf
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
int
killregion()
{
        register int    s;
        REGION          region;

        if ((s=getregion(&region)) != TRUE)
                return s;
	kregcirculate(TRUE);
        ksetup();                      /* command, so do magic */
	DOT = region.r_orig;
	if (fulllineregions)
		kregflag |= KLINES;
	s = ldelete(region.r_size, TRUE);
	kdone();
	ukb = 0;
        return s;
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
        register int    s;
        REGION          region;

        if ((s=getregion(&region)) != TRUE)
                return s;
	kregcirculate(TRUE);
        ksetup();
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
	if (klines)
		mlwrite("[ %d line%c yanked]", klines,
					klines == 1 ? ' ':'s');
	else
		mlwrite("[ %d character%c yanked]", kchars, 
					kchars == 1 ? ' ':'s');
	kdone();
	ukb = 0;
        return TRUE;
}

/*
 * insert a shiftwidth at the front of the line
 * don't do it if we're in cmode and the line starts with '#'
 */
int
shift_right_line()
{
	int s, t;
	if (b_val(curbp, MDCMOD) &&
		llength(DOT.l) > 0 && char_at(DOT) == '#')
		return TRUE;
	s = curswval;
	t = curtabval;
	DOT.o = 0;
	if (s) {  /* try to just insert tabs if possible */
		if (b_val(curbp,MDTABINSERT) && s >= t && (s % t == 0)) {
			linsert(s/t, '\t');
		} else {
			detabline(TRUE);
			DOT.o = 0;
			linsert(s, ' ');
		}
		if (b_val(curbp,MDTABINSERT))
			entabline(TRUE);
	}
	firstnonwhite(FALSE,1);
	return TRUE;
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
int
shift_left_line()
{
	register int	i;
	register int	lim;
	register int	s;
	register LINE *linep = DOT.l;

	i = 0;

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
		DOT.o = 0;
		if ((s = ldelete((long)i,FALSE)) != TRUE)
			return s;
	}

	DOT.o = 0;
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
int
getregion(rp)
register REGION *rp;
{
        register LINE   *flp;
        register LINE   *blp;
        long fsize;
        long bsize;

        if (MK.l == NULL) {
                mlforce("BUG: getregion: no mark set in this window");
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
        mlforce("BUG: lost mark");
        return FALSE;
}

int
do_fl_region(lineprocfunc,arg)
int (*lineprocfunc)();
int arg;
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
		if ((s = (*lineprocfunc)(arg)) != TRUE)
			return s;

        }
	firstnonwhite(FALSE,1);
        return TRUE;
}

