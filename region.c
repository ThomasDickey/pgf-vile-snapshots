/*
 * The routines in this file
 * deal with the region, that magic space
 * between "." and mark. Some functions are
 * commands. Some functions are just for
 * internal use.
 *
 * $Log: region.c,v $
 * Revision 1.44  1994/03/29 14:51:00  pgf
 * fix buffer size passed to mlreply
 *
 * Revision 1.43  1994/03/28  17:16:51  pgf
 * typo
 *
 * Revision 1.42  1994/03/28  16:21:01  pgf
 * make sure all functions called by do_lines_in_region() are prepared
 * to handle empty lines, since they may now get them
 *
 * Revision 1.41  1994/03/18  18:30:38  pgf
 * fixes for OPT_MAP_MEMORY compilation
 *
 * Revision 1.40  1994/03/15  18:33:09  pgf
 * protect against kill_line going off the end of the line, and
 * make sure region_corner does the right thing when swapping left
 * and right.
 *
 * Revision 1.39  1994/03/11  18:27:57  pgf
 * pass correct column-to-offset converter argument to yankline.
 * add comment, so we remember to do this right the next time.
 *
 * Revision 1.38  1994/03/11  13:57:10  pgf
 * fix compiler problem
 *
 * Revision 1.37  1994/03/10  20:14:09  pgf
 * took out ifdef BEFORE code
 * changed yankline to delay the "yanking" of a newline until we're
 * sure something follows it.
 *
 * Revision 1.36  1994/03/08  18:24:05  pgf
 * oops.  bugfix for last change
 *
 * Revision 1.35  1994/03/08  14:48:38  pgf
 * fixed getregion() loops, and use line numbers if available to help
 * scan the region faster.
 * fixed bug in offset calcs for do_lines_in_region()
 *
 * Revision 1.34  1994/03/08  14:06:43  pgf
 * renamed routine, and gcc warning cleanup
 *
 * Revision 1.33  1994/03/08  12:27:24  pgf
 * new routines and much churn to accomodate rectangles.
 *
 * Revision 1.32  1994/02/28  15:08:43  pgf
 * added killregionmaybesave, which allows deleting without yanking
 *
 * Revision 1.31  1994/02/22  11:03:15  pgf
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
	if (regionshape == RECTANGLE)
	    return killrectmaybesave(TRUE);
	else
	    return killregionmaybesave(TRUE);
}

int killregionmaybesave(save)
int save;
{
	register int    status;
	REGION          region;

	if ((status = getregion(&region)) == TRUE) {
		if (save) {
			kregcirculate(TRUE);
			ksetup();		/* command, so do magic */
			if (regionshape == FULLLINE)
				kregflag |= KLINES;
		}
		DOT = region.r_orig;
		status = ldelete(region.r_size, save);
		if (save) {
			kdone();
			ukb = 0;
		}
	}
	rls_region();
	return status;
}

int
kill_line(flagp, l, r)
void	*flagp;
int 	l, r;
{
	int s;
	int save = *(int *)flagp;
	LINE *lp = l_ref(DOT.l);

	s = detabline((void *)FALSE, 0, 0);
	if (s != TRUE) return s;

	DOT.o = l;

	if (r > llength(lp))
	    r = llength(lp);

	if (r > l) {
	    s = ldelete(r - l, save);
	    if (s != TRUE) return s;
	}

	if (save)
		kinsert('\n');

	if (b_val(curbp,MDTABINSERT))
		s = entabline((void *)TRUE, 0, 0);

	DOT.o = l;
	return s;
}

int
killrectmaybesave(save)
int save;
{
	register int    s;
	MARK savedot;
	
	savedot = DOT;

	if (save) {
		kregcirculate(TRUE);
		ksetup();
		if (regionshape == FULLLINE) {
			kregflag |= KLINES;
		} else if (regionshape == RECTANGLE) {
			kregflag |= KRECT;
		}
	}
	s = do_lines_in_region(kill_line, (void *)&save, FALSE);
	DOT = savedot;
		
	if (s && do_report(klines+(kchars!=0))) {
		mlwrite("[%d line%s, %d character%s killed]",
			klines, PLURAL(klines),
			kchars, PLURAL(kchars));
	}
	if (save) {
		kdone();
		ukb = 0;
	}
	rls_region();
	return (s);
}

/*
 * open up a region -- shift the "selected" area of each line by its
 * own length.  most useful for rectangular regions. 
 * fill character is space, unless a string is passed in, in which case
 * it is used instead.
 */
/*ARGSUSED*/
int
open_hole_in_line(flagp, l, r)
void 	*flagp;
int 	l, r;
{
    	char *string = (char *)flagp;
	int len;
	int s;
	int saveo = DOT.o;
	LINE *lp = l_ref(DOT.l);

	s = detabline((void *)FALSE, 0, 0);
	if (s != TRUE) return s;

	if (llength(lp) <= l) {	/* nothing to do if no string */
		if (!string) {
		    if (b_val(curbp,MDTABINSERT))
			    s = entabline((void *)TRUE, 0, 0);
		    DOT.o = saveo;
		    return s;
		} else {
		    DOT.o = llength(lp);
		    if (l - DOT.o)
			linsert(l - DOT.o, ' ');
		}
	}
	DOT.o = l;
	if (string) {
		len = strlen(string);
		if (len < r - l)
			len = r - l;
	} else {
		len = r - l;
	}
	s =  lstrinsert(string, len );
	if (s != TRUE) return s;

	DOT.o = saveo;
	if (b_val(curbp,MDTABINSERT))
		s = entabline((void *)TRUE, 0, 0);
	return s;
}

/*
 * open up a region
 */
int
openregion()
{
	return do_lines_in_region(open_hole_in_line, (void *)NULL, FALSE);
}

/*
 * open up a region, filling it with a supplied string
 * this is pretty simplistic -- could be a lot more clever
 */
int
stringrect()
{
	int             s;
	static char     buf[NLINE];

	s = mlreply("Rectangle text: ", buf, sizeof(buf) );
	if (s != TRUE)
		return s;

/* i couldn't decide at first whether we should be inserting or
	overwriting... this chooses. */
#ifdef insert_the_string
	return do_lines_in_region(open_hole_in_line, (void *)buf, FALSE);
#else /* overwrite the string */
	return do_lines_in_region(blankline, (void *)buf, FALSE);
#endif
}

/*
 * insert a shiftwidth at the front of the line
 * don't do it if we're in cmode and the line starts with '#'
 */
/*ARGSUSED*/
int
shift_right_line(flagp, l, r)
void 	*flagp;
int 	l, r;
{
	int s, t;
	LINE *lp = l_ref(DOT.l);
	if (llength(lp) == 0 || (b_val(curbp, MDCMOD) && 
					llength(lp) > 0 && 
					char_at(DOT) == '#')) {
		return TRUE;
	}
	s = curswval;
	t = curtabval;
	DOT.o = w_left_margin(curwp);
	if (s) {  /* try to just insert tabs if possible */
		if (b_val(curbp,MDTABINSERT) && s >= t && (s % t == 0)) {
			linsert(s/t, '\t');
		} else {
			detabline((void *)TRUE, 0, 0);
			DOT.o = w_left_margin(curwp);
			linsert(s, ' ');
		}
		if (b_val(curbp,MDTABINSERT))
			entabline((void *)TRUE, 0, 0);
	}
	return firstnonwhite(FALSE,1);
}

/*
 * shift region right by a tab stop
 * if region is rectangular, "open it up"
 */
int
shiftrregion()
{
    	if (regionshape == RECTANGLE)
	    return do_lines_in_region(open_hole_in_line, (void *)NULL, FALSE);

	regionshape = FULLLINE;
	return do_lines_in_region(shift_right_line, (void *)0, FALSE);
}

/*
 * delete a shiftwidth-equivalent from the front of the line
 */
/*ARGSUSED*/
int
shift_left_line(flagp, l, r)
void    *flagp;
int	l,r;
{
	register int	i;
	register int	lim;
	register int	s;
	register LINE *linep = l_ref(DOT.l);

	if (llength(linep) == 0)
		return TRUE;

	s = curswval;

	detabline((void *)TRUE, 0, 0);

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
		entabline((void *)TRUE, 0, 0);
	return TRUE;
}

/*
 * shift region left by a tab stop
 */
int
shiftlregion()
{
    	if (regionshape == RECTANGLE)
	    return killrectmaybesave(FALSE);

	regionshape = FULLLINE;
	return do_lines_in_region(shift_left_line, (void *)0, FALSE);
}

/*
 * change all tabs in the line to the right number of spaces.
 * leadingonly says only do leading whitespace
 */
int
detabline(flagp, l, r)
void *flagp;
int l, r;
{
	register int	s;
	register int	c;
	int	ocol;
	int leadingonly = (int)flagp;
	LINE *lp = l_ref(DOT.l);

	if (llength(lp) == 0)
		return TRUE;

	ocol = getccol(FALSE);

	DOT.o = 0;

	/* detab the entire current line */
	while (DOT.o < llength(lp)) {
		c = char_at(DOT);
		if (leadingonly && !isspace(c))
			break;
		/* if we have a tab */
		if (c == '\t') {
			if ((s = ldelete(1L, FALSE)) != TRUE) {
				return s;
			}
			insspace(TRUE, curtabval - (DOT.o % curtabval) );
		}
		DOT.o++;
	}
	(void)gocol(ocol);
	return TRUE;
}


/*
 * change all tabs in the region to the right number of spaces
 */
int
detab_region()
{
	regionshape = FULLLINE;
	return do_lines_in_region(detabline,(void *)FALSE, FALSE);
}

/*
 * convert all appropriate spaces in the line to tab characters.
 * leadingonly says only do leading whitespace
 */
int
entabline(flagp, l, r)
void *flagp;
int l, r;
{
	register int fspace;	/* pointer to first space if in a run */
	register int ccol;	/* current cursor column */
	register char cchar;	/* current character */
	int	ocol;
	int leadingonly = (int)flagp;
	LINE *lp = l_ref(DOT.l);

	if (llength(lp) == 0)
		return TRUE;

	ocol = getccol(FALSE);

	/* entab the current line */
	/* would this have been easier if it had started at
		the _end_ of the line, rather than the beginning?  -pgf */
	fspace = -1;
	ccol = 0;

	detabline(flagp, 0, 0);	/* get rid of possible existing tabs */
	DOT.o = 0;
	while (1) {
		/* see if it is time to compress */
		if ((fspace >= 0) && (nextab(fspace) <= ccol))
			if (ccol - fspace < 2)
				fspace = -1;
			else {
				backchar(TRUE, ccol - fspace);
				(void)ldelete((long)(ccol - fspace), FALSE);
				linsert(1, '\t');
				fspace = -1;
			}

		if (DOT.o >= llength(lp))
			break;

		/* get the current character */
		cchar = char_at(DOT);

		if (cchar == ' ') { /* a space...compress? */
			if (fspace == -1)
				fspace = ccol;
		} else {
			if (leadingonly)
				break;
			fspace = -1;
		}
		ccol++;
		DOT.o++;
	}
	(void)gocol(ocol);
	return TRUE;
}

/*
 * convert all appropriate spaces in the region to tab characters
 */
int
entab_region()
{
	regionshape = FULLLINE;
	return do_lines_in_region(entabline,(void *)FALSE, FALSE);
}

/* trim trailing whitespace from a line.  leave dot at end of line */
/*ARGSUSED*/
int
trimline(flag, l, r)
void 	*flag;
int	l, r;
{
	register int off, orig;
	register LINE *lp;

	lp = l_ref(DOT.l);

	if (llength(lp) == 0)
		return TRUE;


	off = llength(lp)-1;
	orig = off;
	while (off >= 0) {
		if (!isspace(lgetc(lp,off)))
			break;
		off--;
	}

	if (off == orig)
		return TRUE;

	DOT.o = off+1;

	return ldelete((long)(orig - off),FALSE);
}

/*
 * trim trailing whitespace from a region
 */
int
trim_region()
{
	regionshape = FULLLINE;
	return do_lines_in_region(trimline,0, FALSE);
}

/* turn line, or part, to whitespace */
/*ARGSUSED*/
int
blankline(flagp, l, r)
void	*flagp;
int	l, r;
{
    	char *string = (char *)flagp;
	int len;
	int s;
	int saveo;
	LINE *lp = l_ref(DOT.l);

	saveo = l;

	s = detabline((void *)FALSE, 0, 0);

	if (llength(lp) <= l) {	/* nothing to do if no string */
		if (!string) {
		    if (b_val(curbp,MDTABINSERT))
			    s = entabline((void *)TRUE, 0, 0);
		    DOT.o = saveo;
		    return s;
		} else {
		    DOT.o = llength(lp);
		    if (l - DOT.o)
			linsert(l - DOT.o, ' ');
		}
	}

	DOT.o = l;

	if (llength(lp) <= r) {
	    	/* then the rect doesn't extend to the end of line */
		ldelete(llength(lp) - l, FALSE);

		/* so there's nothing beyond the rect, so insert at
			most r-l chars of the string, or nothing */
		if (string) {
		    len = strlen(string);
		    if (len > r - l)
			    len = r - l;
		} else {
		    len = 0;
		}
	} else {
	    	/* the line goes on, so delete and reinsert exactly */
		ldelete(r - l, FALSE);
	    	len = r - l;
	}

	s =  lstrinsert(string, len);
	if (s != TRUE) return s;

	/* DOT.o = saveo; */
	if (b_val(curbp,MDTABINSERT))
		s = entabline((void *)TRUE, 0, 0);

	return s;
}

/*
 * Copy all of the characters in the
 * region to the kill buffer. Don't move dot
 * at all. This is a bit like a kill region followed
 * by a yank.
 */
int
_yankchar(c)
int c;
{
	kinsert(c);
	/* FIXX check return value, longjmp back to yank_line */
	return -1;
}

int
yank_line(flagp, l, r)
void 	*flagp;
int 	l, r;
{
	int s;
	s = do_chars_in_line(_yankchar, l, r);
	if (s && (r == lLength(DOT.l) || regionshape == RECTANGLE)) {
		/* we don't necessarily want to insert the last newline
			in a region, so we delay it */
		kinsertlater('\n');
	}
	return s;
}

int
yankregion()
{
	register int    s;

	kregcirculate(TRUE);
	ksetup();
	if (regionshape == FULLLINE) {
		kregflag |= KLINES|KYANK;
	} else if (regionshape == RECTANGLE) {
		kregflag |= KRECT|KYANK;
	}
	s = do_lines_in_region(yank_line, (void *)0, TRUE);
	if (s && do_report(klines+(kchars!=0))) {
		mlwrite("[%d line%s, %d character%s yanked]",
			klines, PLURAL(klines),
			kchars, PLURAL(kchars));
	}
	kdone();
	ukb = 0;
	rls_region();
	return (s);
}


#if NEEDED
int
_blankchar(c)
int c;
{
	if (!isspace(c))
		return ' ';
	return -1;
}
#endif

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

/*
 * turn region to whitespace
 */
int
blank_region()
{
	return do_lines_in_region(blankline, (void *)NULL, FALSE);
}

int
flipregion()
{
	return do_lines_in_region(do_chars_in_line,(void *)_to_caseflip, TRUE);
}

int
lowerregion()
{
	return do_lines_in_region(do_chars_in_line,(void *)_to_lower, TRUE);
}

int
upperregion()
{
	return do_lines_in_region(do_chars_in_line,(void *)_to_upper, TRUE);
}

#if NEEDED
/* this walks a region, char by char, and invokes a funcion for
 	each.  it does _not_ know about rectangles, which is why it is
	probably obsolete -- we can do_lines_in_region/do_chars_in_line
	to get the same effect*/
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
#endif

/* finish filling in the left/right column info for a rectangular
	region */
static void region_corners P(( REGION * ));

static void
region_corners(rp)
register REGION *rp;
{
	if (regionshape != RECTANGLE)
		return;

	/* convert to columns */
	rp->r_leftcol = getcol(rp->r_orig.l, rp->r_orig.o, FALSE); 
	rp->r_rightcol = getcol(rp->r_end.l, rp->r_end.o, FALSE) + 1; 
	/* enforce geometry */
	if (rp->r_rightcol < rp->r_leftcol) {
		C_NUM tmp = rp->r_rightcol;
		/* swap and, correct for the off by one-ness of the region --
		 * regions include their start, but not their end, so if we
		 * switch left/right colums, we need to inc/decrement
		 * appropriately */
		rp->r_rightcol = rp->r_leftcol + 1;
		rp->r_leftcol = tmp ? tmp - 1 : 0;
	}
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
		if (regionshape == FULLLINE) {
			rp->r_orig.o =
			rp->r_end.o  = w_left_margin(curwp);
			rp->r_end.l  = lFORW(DOT.l);
			rp->r_size   = (B_COUNT)(Line_Length(DOT.l) - w_left_margin(curwp));
		} else {
			if (DOT.o < MK.o) {
				rp->r_orig.o = rp->r_leftcol = DOT.o;
				rp->r_end.o  = rp->r_rightcol = MK.o;
			} else {
				rp->r_orig.o = rp->r_leftcol = MK.o;
				rp->r_end.o  = rp->r_rightcol = DOT.o;
			}
			rp->r_size = rp->r_end.o - rp->r_orig.o;
			region_corners(rp);
		}
		return TRUE;
	}

	if (b_is_counted(curbp)) { /* we have valid line numbers */
		L_NUM dno, mno;
		dno = l_ref(DOT.l)->l_number;
		mno = l_ref(MK.l)->l_number;
		if (mno > dno) {
			flp = l_ref(DOT.l);
			blp = l_ref(MK.l);
			rp->r_orig = DOT;
			rp->r_end  = MK;
		} else {
			flp = l_ref(MK.l);
			blp = l_ref(DOT.l);
			rp->r_orig = MK;
			rp->r_end  = DOT;
		}
		fsize = (B_COUNT)(line_length(flp) - 
				    ((regionshape == FULLLINE) ? 
					w_left_margin(curwp) : rp->r_orig.o));
		while (flp != blp) {
			flp = lforw(flp);
			if (flp != l_ref(buf_head(curbp)))
			    fsize += line_length(flp) - w_left_margin(curwp);
			else {
			    mlwrite ("BUG: hit buf end in getregion");
			}
				
			if (flp == blp) {
				if (regionshape == FULLLINE) {
					rp->r_orig.o =
					rp->r_end.o  = w_left_margin(curwp);
					rp->r_end.l  = lFORW(rp->r_end.l);
				} else {
					fsize -= 
					    (line_length(flp) - rp->r_end.o);
					region_corners(rp);
				}
				rp->r_size = fsize;
				return TRUE;
			}
		}
	} else  {
		blp = l_ref(DOT.l);
		flp = l_ref(DOT.l);
		if (regionshape == FULLLINE) {
			bsize = fsize = 
			(B_COUNT)(line_length(blp) - w_left_margin(curwp));
		} else {
			bsize = (B_COUNT)(DOT.o - w_left_margin(curwp));
			fsize = (B_COUNT)(line_length(flp) - DOT.o);
		}
		while ((flp != l_ref(buf_head(curbp))) ||
				(lback(blp) != l_ref(buf_head(curbp)))) {
		    if (flp != l_ref(buf_head(curbp))) {
			flp = lforw(flp);
			if (flp != l_ref(buf_head(curbp)))
				fsize += line_length(flp) - w_left_margin(curwp);
			if (flp == l_ref(MK.l)) {
				rp->r_orig = DOT;
				rp->r_end  = MK;
				if (regionshape == FULLLINE) {
					rp->r_orig.o =
					rp->r_end.o  = w_left_margin(curwp);
					rp->r_end.l  = lFORW(rp->r_end.l);
				} else {
					fsize -= (line_length(flp) - MK.o);
					region_corners(rp);
				}
				rp->r_size = fsize;
				return TRUE;
			}
		    } 
		    if (lback(blp) != l_ref(buf_head(curbp))) {
			blp = lback(blp);
			bsize += line_length(blp) - w_left_margin(curwp);
			if (blp == l_ref(MK.l)) {
				rp->r_orig = MK;
				rp->r_end  = DOT;
				if (regionshape == FULLLINE) {
					rp->r_orig.o =
					rp->r_end.o  = w_left_margin(curwp);
					rp->r_end.l  = lFORW(rp->r_end.l);
				} else {
					bsize -= (MK.o - w_left_margin(curwp));
					region_corners(rp);
				}
				rp->r_size = bsize;
				return TRUE;
			}
		    }
		}
	}
	mlforce("BUG: lost mark");
	return FALSE;
}

int
get_fl_region(rp)
REGION	*rp;
{
	int	status;

	regionshape = FULLLINE;
	status = getregion(rp);
	regionshape = EXACT;

	return status;
}


/* the (*linefunc)() routine that this calls _must_ be prepared to
	to get empty lines passed to it from this routine. */
int
do_lines_in_region(linefunc,argp,convert_cols)
int (*linefunc) P((void *, int, int));
void *argp;
int convert_cols; /* if rectangle, convert columns to offsets */
{
	register LINE   *linep;
	register int    status;
	REGION          region;
	C_NUM		l, r;

	if ((status=getregion(&region)) == TRUE) {

		/* for each line in the region, ... */
		linep = l_ref(region.r_orig.l);
		while (1) {
			/* move through the region... */
			/* it's important that the linefunc get called
				for every line, even if blank, since it
				may want to keep track of newlines, for
				instance */
			DOT.l = l_ptr(linep);
			DOT.o = w_left_margin(curwp);
			if (regionshape == RECTANGLE) {
			    if (convert_cols) {
				C_NUM reached;
				l = getoff(region.r_leftcol, &reached);
				if (l < 0) l = -l + llength(linep);
				r = getoff(region.r_rightcol, &reached);
				if (r < 0) r = -r + llength(linep);
				if (reached > region.r_rightcol) /* a tab? */
					reached = region.r_rightcol;
			    } else {
				l = region.r_leftcol;
				r = region.r_rightcol;
			    }
			} else {
				l =  w_left_margin(curwp);
				r = lLength(DOT.l);
				if (sameline(region.r_orig, DOT))
					l = region.r_orig.o;
				if (sameline(region.r_end, DOT)) {
					r = region.r_end.o;
					/* if we're on the end-of-
					 * region, in col 0, we're
					 * done. we don't want to
					 * call teh line function
					 * for the empty case
					 */
					if (r == 0)
						break;
				}
			}

			/* ...and process each line */
			if ((status = (*linefunc)(argp,l,r)) != TRUE) {
				rls_region();
				return status;
			}

			if (linep == l_ref(region.r_end.l))
				break;

			linep = lforw(linep);

		}
		if (regionshape == FULLLINE) {
		    kinsertlater(-1);
		    (void)firstnonwhite(FALSE,1);
		}
	}
	rls_region();
	return status;
}


/* walk through the characters in a line, applying a function.  the
	line is marked changed if the function returns other than -1.
	if it returns other than -1, then the char is replaced with
	that value. 
    the ll and rr args are OFFSETS, so if you use this routine with
    	do_lines_in_region, tell it to CONVERT columns to offsets. */
int
do_chars_in_line(funcp, ll, rr)
void	*funcp;
int	ll, rr;		/* offsets of of chars to be processed */
{
	register int    c,nc;
	int 		changed = 0;
	register LINE *lp;
	int i;
	int (*func) P((int));

	func = *(int (*) P((int)))funcp;


	lp = l_ref(DOT.l);


	if (llength(lp) < ll)
		return TRUE;

	DOT.o = ll;
	if (rr > llength(lp))
		rr = llength(lp);

	for (i = ll; i < rr; i++) {
		c = lgetc(lp,i);
		nc = (func)(c);
		if (nc != -1) {
			copy_for_undo(l_ptr(lp));
			lputc(lp,i,nc);
			changed++;
		}
	}
	rls_region();
	if (changed)
	    chg_buff(curbp, WFHARD);
	return TRUE;
}

