/*
 * The functions in this file handle redisplay. There are two halves, the
 * ones that update the virtual display screen, and the ones that make the
 * physical display screen the same as the virtual display screen. These
 * functions use hints that are left in the windows by the commands.
 *
 */


#include        <stdio.h>
#include        <varargs.h>
#include	"estruct.h"
#include        "edef.h"
#if UNIX
#include <signal.h>
#include <termio.h>
#if ODT
#include <sys/ptem.h>
#endif
#endif

typedef struct  VIDEO {
        int	v_flag;                 /* Flags */
#if	COLOR
	int	v_fcolor;		/* current forground color */
	int	v_bcolor;		/* current background color */
	int	v_rfcolor;		/* requested forground color */
	int	v_rbcolor;		/* requested background color */
#endif
	/* allocate 4 bytes here, and malloc 4 bytes less than we need, 
		to keep malloc from rounding up. */
        char    v_text[4];              /* Screen data. */
}       VIDEO;

#define VFCHG   0x0001                  /* Changed flag			*/
#define	VFEXT	0x0002			/* extended (beyond column 80)	*/
#define	VFREV	0x0004			/* reverse video status		*/
#define	VFREQ	0x0008			/* reverse video request	*/
#define	VFCOL	0x0010			/* color change requested	*/

VIDEO   **vscreen;                      /* Virtual screen. */
#if	! MEMMAP
VIDEO   **pscreen;                      /* Physical screen. */
#endif


int displaying = FALSE;
#ifdef SIGWINCH
/* for window size changes */
int chg_width, chg_height;
#endif

/*
 * Initialize the data structures used by the display code. The edge vectors
 * used to access the screens are set up. The operating system's terminal I/O
 * channel is set up. All the other things get initialized at compile time.
 * The original window has "WFCHG" set, so that it will get completely
 * redrawn on the first call to "update".
 */
vtinit()
{
    register int i;
    register VIDEO *vp;

    TTopen();		/* open the screen */
    TTkopen();		/* open the keyboard */
    TTrev(FALSE);
    vscreen = (VIDEO **) malloc(term.t_mrow*sizeof(VIDEO *));

    if (vscreen == NULL)
        exit(1);

#if	! MEMMAP
    pscreen = (VIDEO **) malloc(term.t_mrow*sizeof(VIDEO *));

    if (pscreen == NULL)
        exit(1);
#endif

    for (i = 0; i < term.t_mrow; ++i) {
    	/* struct VIDEO already has 4 of the bytes */
        vp = (VIDEO *) malloc(sizeof(struct VIDEO) + term.t_mcol - 4);

        if (vp == NULL)
            exit(1);

	vp->v_flag = 0;
#if	COLOR
	vp->v_rfcolor = 7;
	vp->v_rbcolor = 0;
#endif
        vscreen[i] = vp;
#if	! MEMMAP
    	/* struct VIDEO already has 4 of the bytes */
        vp = (VIDEO *) malloc(sizeof(struct VIDEO) + term.t_mcol - 4);

        if (vp == NULL)
            exit(1);

	vp->v_flag = 0;
        pscreen[i] = vp;
#endif
        }
}

/*
 * Clean up the virtual terminal system, in anticipation for a return to the
 * operating system. Move down to the last line and advance, to make room
 * for the system prompt. Shut down the channel to the
 * terminal.
 */
vttidy(f)
{
	ttclean(f);	/* does it all now */
}

/*
 * Set the virtual cursor to the specified row and column on the virtual
 * screen. There is no checking for nonsense values.
 */
vtmove(row, col)
{
    vtrow = row;
    vtcol = col;
}

/* Write a character to the virtual screen. The virtual row and
   column are updated. If we are not yet on left edge, don't print
   it yet. If the line is too long put a ">" in the last column.
   This routine only puts printing characters into the virtual
   terminal buffers. Only column overflow is checked.
*/

vtputc(c,list)
int c,list;
{
	register VIDEO *vp;	/* ptr to line being updated */

	vp = vscreen[vtrow];

	if (c == '\t' && !list) {
		do {
			vtputc(' ',FALSE);
		} while (((vtcol + taboff)&TABMASK) != 0);
	} else if (c == '\n' && !list) {
		return;
	} else if (vtcol >= term.t_ncol) {
		++vtcol;
		vp->v_text[term.t_ncol - 1] = '>';
	} else if (!isprint(c)) {
		vtputc('^',FALSE);
		vtputc(toalpha(c),FALSE);
	} else {
		if (vtcol >= 0)
			vp->v_text[vtcol] = c;
		++vtcol;
	}
}

vtgetc(col)
{
	return vscreen[vtrow]->v_text[col];
}

vtputsn(s,n)
char *s;
{
	int c;
	while (n-- && (c = *s++) != 0)
		vtputc(c,FALSE);
}

/*
 * Erase from the end of the software cursor to the end of the line on which
 * the software cursor is located.
 */
vteeol()
{
    register VIDEO      *vp;

    vp = vscreen[vtrow];
    while (vtcol < term.t_ncol)
        vp->v_text[vtcol++] = ' ';
}

/* upscreen:	user routine to force a screen update
		always finishes complete update		*/
upscreen(f, n)
{
	update(TRUE);
	return(TRUE);
}

int scrflags;
/*
 * Make sure that the display is right. This is a three part process. First,
 * scan through all of the windows looking for dirty ones. Check the framing,
 * and refresh the screen. Second, make sure that "currow" and "curcol" are
 * correct for the current window. Third, make the virtual and physical
 * screens the same.
 */
update(force)
int force;	/* force update past type ahead? */
{
	register WINDOW *wp;

#if	TYPEAH
	if (force == FALSE && typahead())
		return(SORTOFTRUE);
#endif
#if	VISMAC == 0
	if (force == FALSE && (kbdmode == PLAY || dotcmdmode == PLAY))
		return(TRUE);
#endif

	displaying = TRUE;

	/* first, propagate mode line changes to all instances of
		a buffer displayed in more than one window */
	wp = wheadp;
	while (wp != NULL) {
		if (wp->w_flag & WFMODE) {
			if (wp->w_bufp->b_nwnd > 1) {
				/* make sure all previous windows have this */
				register WINDOW *owp;
				owp = wheadp;
				while (owp != NULL) {
					if (owp->w_bufp == wp->w_bufp)
						owp->w_flag |= WFMODE;
					owp = owp->w_wndp;
				}
			}
		}
		wp = wp->w_wndp;
	}

	/* update any windows that need refreshing */
	wp = wheadp;
	while (wp != NULL) {
		if (wp->w_flag) {
			/* if the window has changed, service it */
			reframe(wp);	/* check the framing */
			if (wp->w_flag & (WFKILLS|WFINS)) {
				scrflags |= (wp->w_flag & (WFINS|WFKILLS));
				wp->w_flag &= ~(WFKILLS|WFINS);
			}
			if ((wp->w_flag & ~WFMODE) == WFEDIT)
				updone(wp);	/* update EDITed line */
			else if (wp->w_flag & ~WFMOVE)
				updall(wp);	/* update all lines */
			if (scrflags || (wp->w_flag & WFMODE))
				modeline(wp);	/* update modeline */
			wp->w_flag = 0;
			wp->w_force = 0;
		}
		/* on to the next window */
		wp = wp->w_wndp;
	}

	/* recalc the current hardware cursor location */
	updpos();

#if	MEMMAP
	/* update the cursor and flush the buffers */
	movecursor(currow, curcol - lbound);
#endif

	/* check for lines to de-extend */
	upddex();

#if	NeWS
	newsupd(force) ;
#else
	/* if screen is garbage, re-plot it */
	if (sgarbf)
		updgar();

	/* update the virtual screen to the physical screen */
	updupd(force);
#endif

	/* update the cursor and flush the buffers */
	movecursor(currow, curcol - lbound);
	TTflush();
	displaying = FALSE;
#if SIGWINCH
	while (chg_width || chg_height)
		newscreensize(chg_height,chg_width);
#endif
	return(TRUE);
}

/*	reframe:	check to see if the cursor is on in the window
			and re-frame it if needed or wanted		*/
reframe(wp)
WINDOW *wp;
{
	register LINE *lp;
	register int i;

	/* if not a requested reframe, check for a needed one */
	if ((wp->w_flag & WFFORCE) == 0) {
#if SCROLLCODE
		/* loop from one line above the window to one line after */
		lp = lback(wp->w_linep);
		for (i = -1; i <= wp->w_ntrows; i++)
#else
		/* loop through the window */
		lp = wp->w_linep;
		for (i = 0; i < wp->w_ntrows; i++)
#endif
		{

			/* if the line is in the window, no reframe */
			if (lp == wp->w_dotp) {
#if SCROLLCODE
				/* if not _quite_ in, we'll reframe gently */
				if ( i < 0 || i == wp->w_ntrows) {
					/* if the terminal can't help, then
						we're simply outside */
					if (term.t_scroll == NULL)
						i = wp->w_force;
					break;
				}
#endif
				return(TRUE);
			}

			/* if we are at the end of the file, reframe */
			if (i >= 0 && lp == wp->w_bufp->b_linep)
				break;

			/* on to the next line */
			lp = lforw(lp);
		}
	}

#if SCROLLCODE
	if (i == -1) {	/* we're just above the window */
		i = 1;	/* put dot at first line */
		scrflags |= WFINS;
	} else if (i == wp->w_ntrows) { /* we're just below the window */
		i = -1;	/* put dot at last line */
		scrflags |= WFKILLS;
	} else /* put dot where requested */
#endif
		i = wp->w_force;  /* (is 0, unless reposition() was called) */

	wp->w_flag |= WFMODE;
	
	/* w_force specifies which line of the window dot should end up on */
	/* 	positive --> lines from the top				*/
	/* 	negative --> lines from the bottom			*/
	/* 	zero --> middle of window				*/
	
	/* enforce some maximums */
	if (i > 0) {
		if (--i >= wp->w_ntrows)
			i = wp->w_ntrows - 1;
	} else if (i < 0) {	/* negative update???? */
		i += wp->w_ntrows;
		if (i < 0)
			i = 0;
	} else
		i = wp->w_ntrows / 2;

	/* backup to new line at top of window */
	lp = wp->w_dotp;
	while (i != 0 && lback(lp) != wp->w_bufp->b_linep) {
		--i;
		lp = lback(lp);
	}

	if (lp == wp->w_bufp->b_linep)
		lp = lback(lp);
		
	/* and reset the current line-at-top-of-window */
	wp->w_linep = lp;
	wp->w_flag |= WFHARD;
	wp->w_flag &= ~WFFORCE;
	return(TRUE);
}

/*	updone:	update the current line	to the virtual screen		*/

updone(wp)
WINDOW *wp;	/* window to update current line in */
{
	register LINE *lp;	/* line to update */
	register int sline;	/* physical screen line to update */

	/* search down the line we want */
	lp = wp->w_linep;
	sline = wp->w_toprow;
	while (lp != wp->w_dotp) {
		++sline;
		lp = lforw(lp);
	}

	l_to_vline(wp,lp,sline);
	vteeol();
}

/*	updall:	update all the lines in a window on the virtual screen */

updall(wp)
WINDOW *wp;	/* window to update lines in */
{
	register LINE *lp;	/* line to update */
	register int sline;	/* physical screen line to update */

	/* search down the lines, updating them */
	lp = wp->w_linep;
	sline = wp->w_toprow;
	while (sline < wp->w_toprow + wp->w_ntrows) {
		l_to_vline(wp,lp,sline);
		vteeol();
		if (lp != wp->w_bufp->b_linep)
			lp = lforw(lp);
		++sline;
	}

}

/* line to virtual screen line */
l_to_vline(wp,lp,sline)
WINDOW *wp;	/* window to update lines in */
LINE *lp;
{
	int i,c;

	/* and update the virtual line */
	vscreen[sline]->v_flag |= VFCHG;
	vscreen[sline]->v_flag &= ~VFREQ;
	vtmove(sline, 0);
	if (lp != wp->w_bufp->b_linep) {
		for (i = 0; i < llength(lp); ++i)
			vtputc(lgetc(lp, i), wp->w_bufp->b_mode & MDLIST);
		vtputc('\n', wp->w_bufp->b_mode & MDLIST);
	} else {
		vtputc('~');
	}
#if	COLOR
	vscreen[sline]->v_rfcolor = wp->w_fcolor;
	vscreen[sline]->v_rbcolor = wp->w_bcolor;
#endif
}

/*	updpos:	update the position of the hardware cursor and handle extended
		lines. This is the only update for simple moves.	*/

updpos()
{
	register LINE *lp;
	register int c;
	register int i;

	/* find the current row */
	lp = curwp->w_linep;
	currow = curwp->w_toprow;
	while (lp != curwp->w_dotp) {
		++currow;
		lp = lforw(lp);
		if (lp == curwp->w_linep) {
			mlwrite("Bug:  lost dot updpos().  setting at top");
			curwp->w_linep = curwp->w_dotp  = lforw(curbp->b_linep);
			currow = curwp->w_toprow;
		}
	}

	/* find the current column */
	curcol = 0;
	i = 0;
	while (i < curwp->w_doto) {
		c = lgetc(lp, i++);
		if (((curwp->w_bufp->b_mode&MDLIST) == 0) && c == '\t')
			curcol |= TABMASK;
		else
			if (!isprint(c))
				++curcol;

		++curcol;
	}

	/* if extended, flag so and update the virtual line image */
	if (curcol >=  term.t_ncol - 1) {
		vscreen[currow]->v_flag |= (VFEXT | VFCHG);
		updext(curwp);
	} else
		lbound = 0;
}

/*	upddex:	de-extend any line that derserves it		*/

upddex()
{
	register WINDOW *wp;
	register LINE *lp;
	register int i,j;

	wp = wheadp;

	while (wp != NULL) {
		lp = wp->w_linep;
		i = wp->w_toprow;

		while (i < wp->w_toprow + wp->w_ntrows) {
			if (vscreen[i]->v_flag & VFEXT) {
				if ((wp != curwp) || (lp != wp->w_dotp) ||
				   (curcol < term.t_ncol - 1)) {
					vtmove(i, 0);
					for (j = 0; j < llength(lp); ++j)
						vtputc(lgetc(lp, j),
						wp->w_bufp->b_mode&MDLIST);
					vtputc('\n', wp->w_bufp->b_mode&MDLIST);
					vteeol();

					/* this line no longer is extended */
					vscreen[i]->v_flag &= ~VFEXT;
					vscreen[i]->v_flag |= VFCHG;
				}
			}
			lp = lforw(lp);
			++i;
		}
		/* and onward to the next window */
		wp = wp->w_wndp;
	}
}

/*	updgar:	if the screen is garbage, clear the physical screen and
		the virtual screen and force a full update		*/

updgar()
{
	register char *txt;
	register int i,j;

	for (i = 0; i < term.t_nrow; ++i) {
		vscreen[i]->v_flag |= VFCHG;
#if	REVSTA
		vscreen[i]->v_flag &= ~VFREV;
#endif
#if	COLOR
		vscreen[i]->v_fcolor = gfcolor;
		vscreen[i]->v_bcolor = gbcolor;
#endif
#if	! MEMMAP
		txt = pscreen[i]->v_text;
		for (j = 0; j < term.t_ncol; ++j)
			txt[j] = ' ';
#endif
	}

	movecursor(0, 0);		 /* Erase the screen. */
	(*term.t_eeop)();
	sgarbf = FALSE;			 /* Erase-page clears */
	mpresf = FALSE;			 /* the message area. */
#if	COLOR
	mlerase();			/* needs to be cleared if colored */
#endif
}

/*	updupd:	update the physical screen from the virtual screen	*/

updupd(force)
int force;	/* forced update flag */
{
	register VIDEO *vp1;
	register int i;
#if SCROLLCODE
	if (scrflags & WFKILLS)
		scrolls(FALSE);
	if (scrflags & WFINS)
		scrolls(TRUE);
	scrflags = 0;
#endif

	for (i = 0; i < term.t_nrow; ++i) {
		vp1 = vscreen[i];

		/* for each line that needs to be updated*/
		if ((vp1->v_flag & VFCHG) != 0) {
#if	TYPEAH
			if (force == FALSE && typahead())
				return(TRUE);
#endif
#if	MEMMAP
			updateline(i, vp1);
#else
			updateline(i, vp1, pscreen[i]);
#endif
		}
	}
	return(TRUE);
}

#if SCROLLCODE
/* optimize out scrolls (line breaks, and newlines) */
/* arg. chooses between looking for inserts or deletes */
int	
scrolls(inserts)	/* returns true if it does something */
{
	struct	VIDEO *vpv ;	/* virtual screen image */
	struct	VIDEO *vpp ;	/* physical screen image */
	int	i, j, k ;
	int	rows, cols ;
	int	first, match, count, ptarget, vtarget, end ;
	int	longmatch, longcount;
	int	from, to;

	if (!term.t_scroll) /* no way to scroll */
		return FALSE;

	rows = term.t_nrow ;
	cols = term.t_ncol ;

	first = -1 ;
	for (i = 0; i < rows; i++) {	/* find first wrong line */
		if (!texttest(i,i)) {
			first = i;
			break;
		}
	}

	if (first < 0)
		return FALSE;		/* no text changes */

	vpv = vscreen[first] ;
	vpp = pscreen[first] ;

	if (inserts) {
		/* determine types of potential scrolls */
		end = endofline(vpv->v_text,cols) ;
		if ( end == 0 )
			ptarget = first ;		/* newlines */
		else if ( strncmp(vpp->v_text, vpv->v_text, end) == 0 )
			ptarget = first + 1 ;	/* broken line newlines */
		else 
			ptarget = first ;
	} else {
		vtarget = first + 1 ;
	}

	/* find the matching shifted area */
	match = -1 ;
	longmatch = -1;
	longcount = 0;
	from = inserts ? ptarget : vtarget;
	for (i = from+1; i < rows; i++) {
		if (inserts ? texttest(i,from) : texttest(from,i) ) {
			match = i ;
			count = 1 ;
			for (j=match+1, k=from+1; j<rows && k<rows; j++, k++) {
				if (inserts ? texttest(j,k) : texttest(k,j))
					count++ ;
				else
					break ;
			}
			if (longcount < count) {
				longcount = count;
				longmatch = match;
			}
		}
	}
	match = longmatch;
	count = longcount;

	if (!inserts) {
		/* full kill case? */
		if (match > 0 && texttest(first, match-1)) {
			vtarget-- ;
			match-- ;
			count++ ;
		}
	}

	/* do the scroll */
	if (match>0 && count>2) {		 /* got a scroll */
		/* move the count lines starting at ptarget to match */
		/* mlwrite("scrolls: move the %d lines starting at %d to %d",
						count,ptarget,match);
		*/
		if (inserts) {
			from = ptarget;
			to = match;
		} else {
			from = match;
			to = vtarget;
		}
		scrscroll(from, to, count) ;
		for (i = 0; i < count; i++) {
			vpp = pscreen[to+i] ;
			vpv = vscreen[to+i];
			strncpy(vpp->v_text, vpv->v_text, cols) ;
		}
		if (inserts) {
			from = ptarget;
			to = match;
		} else {
			from = vtarget+count;
			to = match+count;
		}
		for (i = from; i < to; i++) {
			char *txt;
			txt = pscreen[i]->v_text;
			for (j = 0; j < term.t_ncol; ++j)
				txt[j] = ' ';
			vscreen[i]->v_flag |= VFCHG;
		}
		return(TRUE) ;
	}
	return(FALSE) ;
}

/* move the "count" lines starting at "from" to "to" */
scrscroll(from, to, count)
{
	ttrow = ttcol = -1;
	(*term.t_scroll)(from,to,count);
}

texttest(vrow,prow)		/* return TRUE on text match */
int	vrow, prow ;		/* virtual, physical rows */
{
struct	VIDEO *vpv = vscreen[vrow] ;	/* virtual screen image */
struct	VIDEO *vpp = pscreen[prow]  ;	/* physical screen image */

	return (!memcmp(vpv->v_text, vpp->v_text, term.t_ncol)) ;
}

/* return the index of the first blank of trailing whitespace */
int	
endofline(s,n) 
char 	*s ;
{
int	i ;
	for (i = n - 1; i >= 0; i--)
		if (s[i] != ' ') return(i+1) ;
	return(0) ;
}

#endif /* SCROLLCODE */


/*	updext: update the extended line which the cursor is currently
		on at a column greater than the terminal width. The line
		will be scrolled right or left to let the user see where
		the cursor is
								*/

updext(wp)
WINDOW *wp;	/* window to update lines in */
{
	register int rcursor;	/* real cursor location */
	register LINE *lp;	/* pointer to current line */
	register int j;		/* index into line */

	/* calculate what column the real cursor will end up in */
	rcursor = ((curcol - term.t_ncol) % term.t_scrsiz) + term.t_margin;
	taboff = lbound = curcol - rcursor + 1;

	/* scan through the line outputing characters to the virtual screen */
	/* once we reach the left edge					*/
	vtmove(currow, -lbound);	/* start scanning offscreen */
	lp = curwp->w_dotp;		/* line to output */
	for (j = 0; j < llength(lp); ++j)
		vtputc(lgetc(lp, j), wp->w_bufp->b_mode&MDLIST);
	vtputc('\n', wp->w_bufp->b_mode&MDLIST);

	/* truncate the virtual line, restore tab offset */
	vteeol();
	taboff = 0;

	/* and put a '<' in column 1 */
	vscreen[currow]->v_text[0] = '<';
}


#if	NeWS
newsupd(force)	/* update the physical screen from the virtual screen */
int force;	/* forced update flag */
{
register int i ;
struct	VIDEO *vpv ;	/* virtual screen image */
struct	VIDEO *vpp ;	/* physical screen image */
int	bad, badcol, rows ;

	rows = term.t_nrow ;

	if (force == FALSE && typahead()) return ;
	if (sgarbf) {
		fastupdate() ;
		return ;
	}

	/* if enough lines are bad try to optimize scrolls/kills */	
	for (bad = 0, i = 0; i < rows; ++i)
		if (!texttest(i,i)) {
			bad++ ;
			if (bad > 3) {
				if (!scrolls()) kills() ;
				break ;
			}
		}

	/* count bad lines, if enough need fixed redo whole screen */
	for (bad = 0, badcol = 0, i = 0; i < rows; ++i) {
		vpv = vscreen[i] ;
		vpv->v_flag &= ~(VFCHG|VFCOL) ;		/* clear flag */
		if (!texttest(i,i)) {
			vpv->v_flag |= VFCHG ;
			bad++ ;
		}
		if (!colortest(i)) {
			vpv->v_flag |= VFCOL ;
			badcol++  ;
		}
	}
	if (bad == 0 && badcol > 0) {	/* pure color update */
		colorupdate() ;
		return ;
	}
	if (bad > (3*rows)/4) {		/* full update */
		fastupdate() ;
		return ;
	}

	/* Fix the bad lines one by one */
	for (i = 0; i < rows; ++i)
		if (vscreen[i]->v_flag & (VFCHG|VFCOL)) updateline(i) ;
}


/* optimize out scrolls (line breaks, and newlines) */
int	scrolls()	/* returns true if it does something */
{
struct	VIDEO *vpv ;	/* virtual screen image */
struct	VIDEO *vpp ;	/* physical screen image */
int	i, j, k ;
int	rows, cols ;
int	first, match, count, ptarget, end ;

	rows = term.t_nrow ;
	cols = term.t_ncol ;

	first = -1 ;
	for (i = 0; i < rows; i++)	/* find first wrong line */
		if (!texttest(i,i)) {first = i ; break ;}

	if (first < 0) return(FALSE) ;		/* no text changes */

	vpv = vscreen[first] ;
	vpp = pscreen[first] ;

	/* determine types of potential scrolls */
	end = endofline(vpv->v_text,cols) ;
	if ( end == 0 )
		ptarget = first ;		/* newlines */
	else if ( strncmp(vpp->v_text, vpv->v_text, end) == 0 )
		ptarget = first + 1 ;	/* broken line newlines */
	else return(FALSE) ;			/* no scrolls */

	/* find the matching shifted area */
	match = -1 ;
	for (i = ptarget+1; i < rows; i++) {
		if (texttest(i, ptarget)) {
			match = i ;
			count = 1 ;
			for (j=match+1, k=ptarget+1; j<rows && k<rows; j++, k++) {
				if (texttest(j,k))
					count++ ;
				else
					break ;
			}
			break ;
		}
	}

	/* do the scroll */
	if (match>0 && count>2) {		 /* got a scroll */
		newsscroll(ptarget, match, count) ;
		for (i = 0; i < count; i++) {
			vpv = vscreen[match+i] ; vpp = pscreen[match+i] ;
			strncpy(vpp->v_text, vpv->v_text, cols) ;
		}
		return(TRUE) ;
	}
	return(FALSE) ;
}


/* optimize out line kills (full and with a partial kill) */
int	kills()		/* returns true if it does something */
{
struct	VIDEO *vpv ;	/* virtual screen image */
struct	VIDEO *vpp ;	/* physical screen image */
int	i, j, k ;
int	rows, cols ;
int	first, match, count, vtarget, end ;

	rows = term.t_nrow ;
	cols = term.t_ncol ;

	first = -1 ;
	for (i = 0; i < rows; i++)	/* find first wrong line */
		if (!texttest(i,i)) {first = i ; break ;}

	if (first < 0) return(FALSE) ;		/* no text changes */

	vpv = vscreen[first] ;
	vpp = pscreen[first] ;

	vtarget = first + 1 ;

	/* find the matching shifted area */
	match = -1 ;
	for (i = vtarget+1; i < rows; i++) {
		if (texttest(vtarget, i)) {
			match = i ;
			count = 1 ;
			for (j=match+1, k=vtarget+1; j<rows && k<rows; j++, k++) {
				if (texttest(k,j))
					count++ ;
				else
					break ;
			}
			break ;
		}
	}
	if (texttest(first, match-1)) {		/* full kill case */
		vtarget-- ;
		match-- ;
		count++ ;
	}

	/* do the scroll */
	if (match>0 && count>2) {		/* got a scroll */
		newsscroll(match, vtarget, count) ;
		for (i = 0; i < count; i++) {
			vpv = vscreen[vtarget+i] ; vpp = pscreen[vtarget+i] ;
			strncpy(vpp->v_text, vpv->v_text, cols) ;
		}
		return(TRUE) ;
	}
	return(FALSE) ;
}


texttest(vrow,prow)		/* return TRUE on text match */
int	vrow, prow ;		/* virtual, physical rows */
{
struct	VIDEO *vpv = vscreen[vrow] ;	/* virtual screen image */
struct	VIDEO *vpp = pscreen[prow]  ;	/* physical screen image */

	vpp->v_text[term.t_ncol] = 0 ;
	vpv->v_text[term.t_ncol] = 0 ;
	return (!strncmp(vpv->v_text, vpp->v_text, term.t_ncol)) ;
}

colortest(row)			/* TRUE on color match */
int	row ;	
{
struct	VIDEO *vpv = vscreen[row] ;	/* virtual screen image */

	return (vpv->v_fcolor == vpv->v_rfcolor &&
		vpv->v_bcolor == vpv->v_rbcolor) ;
}


updateline(row)
int	row ;		/* row of screen to update */
{
struct	VIDEO *vpv = vscreen[row] ;	/* virtual screen image */
struct	VIDEO *vpp = pscreen[row]  ;	/* physical screen image */
int	end ;

	end = endofline(vpv->v_text, term.t_ncol) ;
	strncpy(vpp->v_text, vpv->v_text, term.t_ncol) ;
	vpv->v_text[end] = 0 ;
	newsputline(row, vpv->v_text, vpv->v_rfcolor, vpv->v_rbcolor) ;
	vpv->v_text[end] = ' ' ;
	vpv->v_fcolor = vpv->v_rfcolor;
	vpv->v_bcolor = vpv->v_rbcolor;
	vpv->v_flag &= ~(VFCHG|VFCOL);	/* clear flag */
}


colorupdate()
{
struct	VIDEO *vpv ;	/* virtual screen image */
int	row ;
	
	for (row=0; row<term.t_nrow; row++) {	/* send the row colors */
		vpv = vscreen[row] ;
		if (vpv->v_flag & VFCOL) {
			newssetrowcolors(row, vpv->v_rfcolor, vpv->v_rbcolor) ;
			vpv->v_fcolor = vpv->v_rfcolor;
			vpv->v_bcolor = vpv->v_rbcolor;
		}
		vpv->v_flag &= ~VFCOL ;
	}

	newslocalupdate() ;	/* ask for a screen refresh */
}


fastupdate()		/* redo the entire screen fast */
{
int	row ;
register char	*cp, *first ;
struct	VIDEO *vpv ;	/* virtual screen image */
struct	VIDEO *vpp ;	/* physical screen image */

	/* send the row colors */
	for (row=0; row<term.t_nrow; row++) {
		vpv = vscreen[row] ;
		if (!colortest(row)) {
			newssetrowcolors(row, vpv->v_rfcolor, vpv->v_rbcolor) ;
			vpv->v_fcolor = vpv->v_rfcolor;
			vpv->v_bcolor = vpv->v_rbcolor;
		}
		vpv->v_flag &= ~VFCOL ;
	}

	/* virtual -> physical buffer */
	for (row=0; row<term.t_nrow; row++) {
		vpv = vscreen[row] ; vpp = pscreen[row] ;
		memcpy(vpp->v_text, vpv->v_text, term.t_ncol);
		vpp->v_text[term.t_ncol] = 0 ;
		vpv->v_text[term.t_ncol] = 0 ;
		vpv->v_flag &= ~VFCHG;
	}
	/* send the stuff */
	newscls() ;
	for (row=0; row<term.t_nrow; row++) {
		first = pscreen[row]->v_text ;
		/* don't send trailing blanks */
		cp = &first[endofline(first,term.t_ncol)] ;
		if (cp > first) {
			*cp = 0 ;
			newsfastputline(row, first) ;
			*cp = ' ' ;
		}
	}
	sgarbf = FALSE;
}
 

/* return the index of the first blank of trailing whitespace */
int	endofline(s,n) 
char 	*s ;
{
int	i ;
	for (i = n - 1; i >= 0; i--)
		if (s[i] != ' ') return(i+1) ;
	return(0) ;
}
#else

/*
 * Update a single line. This does not know how to use insert or delete
 * character sequences; we are using VT52 functionality. Update the physical
 * row and column variables. It does try an exploit erase to end of line. The
 * RAINBOW version of this routine uses fast video.
 */
#if	MEMMAP
/*	UPDATELINE specific code for the IBM-PC and other compatables */

updateline(row, vp1)

int row;		/* row of screen to update */
struct VIDEO *vp1;	/* virtual screen image */

{
#if	COLOR
	scwrite(row, vp1->v_text, vp1->v_rfcolor, vp1->v_rbcolor);
	vp1->v_fcolor = vp1->v_rfcolor;
	vp1->v_bcolor = vp1->v_rbcolor;
#else
	if (vp1->v_flag & VFREQ)
		scwrite(row, vp1->v_text, 0, 7);
	else
		scwrite(row, vp1->v_text, 7, 0);
#endif
	vp1->v_flag &= ~(VFCHG | VFCOL);	/* flag this line as changed */

}

#else

updateline(row, vp1, vp2)

int row;		/* row of screen to update */
struct VIDEO *vp1;	/* virtual screen image */
struct VIDEO *vp2;	/* physical screen image */

{
#if RAINBOW
/*	UPDATELINE specific code for the DEC rainbow 100 micro	*/

    register char *cp1;
    register char *cp2;
    register int nch;

    /* since we don't know how to make the rainbow do this, turn it off */
    flags &= (~VFREV & ~VFREQ);

    cp1 = &vp1->v_text[0];                    /* Use fast video. */
    cp2 = &vp2->v_text[0];
    putline(row+1, 1, cp1);
    nch = term.t_ncol;

    do
        {
        *cp2 = *cp1;
        ++cp2;
        ++cp1;
        }
    while (--nch);
    *flags &= ~VFCHG;
#else
/*	UPDATELINE code for all other versions		*/

	register char *cp1;
	register char *cp2;
	register char *cp3;
	register char *cp4;
	register char *cp5;
	register int nbflag;	/* non-blanks to the right flag? */
	int rev;		/* reverse video flag */
	int req;		/* reverse video request flag */


	/* set up pointers to virtual and physical lines */
	cp1 = &vp1->v_text[0];
	cp2 = &vp2->v_text[0];

#if	COLOR
	TTforg(vp1->v_rfcolor);
	TTbacg(vp1->v_rbcolor);
#endif

#if	REVSTA | COLOR
	/* if we need to change the reverse video status of the
	   current line, we need to re-write the entire line     */
	rev = (vp1->v_flag & VFREV) == VFREV;
	req = (vp1->v_flag & VFREQ) == VFREQ;
	if ((rev != req)
#if	COLOR
	    || (vp1->v_fcolor != vp1->v_rfcolor) || (vp1->v_bcolor != vp1->v_rbcolor)
#endif
#if	HP150
	/* the HP150 has some reverse video problems */
	    || req || rev
#endif
			) {
		movecursor(row, 0);	/* Go to start of line. */
		/* set rev video if needed */
		if (rev != req)
			(*term.t_rev)(req);

		/* scan through the line and dump it to the screen and
		   the virtual screen array				*/
		cp3 = &vp1->v_text[term.t_ncol];
		while (cp1 < cp3) {
			TTputc(*cp1);
			++ttcol;
			*cp2++ = *cp1++;
		}
		/* turn rev video off */
		if (rev != req)
			(*term.t_rev)(FALSE);

		/* update the needed flags */
		vp1->v_flag &= ~VFCHG;
		if (req)
			vp1->v_flag |= VFREV;
		else
			vp1->v_flag &= ~VFREV;
#if	COLOR
		vp1->v_fcolor = vp1->v_rfcolor;
		vp1->v_bcolor = vp1->v_rbcolor;
#endif
		return(TRUE);
	}
#endif

	/* advance past any common chars at the left */
	while (cp1 != &vp1->v_text[term.t_ncol] && cp1[0] == cp2[0]) {
		++cp1;
		++cp2;
	}

/* This can still happen, even though we only call this routine on changed
 * lines. A hard update is always done when a line splits, a massive
 * change is done, or a buffer is displayed twice. This optimizes out most
 * of the excess updating. A lot of computes are used, but these tend to
 * be hard operations that do a lot of update, so I don't really care.
 */
	/* if both lines are the same, no update needs to be done */
	if (cp1 == &vp1->v_text[term.t_ncol]) {
 		vp1->v_flag &= ~VFCHG;		/* flag this line is changed */
		return(TRUE);
	}

	/* find out if there is a match on the right */
	nbflag = FALSE;
	cp3 = &vp1->v_text[term.t_ncol];
	cp4 = &vp2->v_text[term.t_ncol];

	while (cp3[-1] == cp4[-1]) {
		--cp3;
		--cp4;
		if (cp3[0] != ' ')		/* Note if any nonblank */
			nbflag = TRUE;		/* in right match. */
	}

	cp5 = cp3;

	/* Erase to EOL ? */
	if (nbflag == FALSE && eolexist == TRUE && (req != TRUE)) {
		while (cp5!=cp1 && cp5[-1]==' ')
			--cp5;

		if (cp3-cp5 <= 3)		/* Use only if erase is */
			cp5 = cp3;		/* fewer characters. */
	}

	movecursor(row, cp1 - &vp1->v_text[0]);	/* Go to start of line. */
#if	REVSTA
	TTrev(rev);
#endif

	while (cp1 != cp5) {		/* Ordinary. */
		TTputc(*cp1);
		++ttcol;
		*cp2++ = *cp1++;
	}

	if (cp5 != cp3) {		/* Erase. */
		TTeeol();
		while (cp1 != cp3)
			*cp2++ = *cp1++;
	}
#if	REVSTA
	TTrev(FALSE);
#endif
	vp1->v_flag &= ~VFCHG;		/* flag this line as updated */
	return(TRUE);
#endif
}
#endif

#endif  /* NeWS */

/*
 * Redisplay the mode line for the window pointed to by the "wp". This is the
 * only routine that has any idea of how the modeline is formatted. You can
 * change the modeline format by hacking at this routine. Called by "update"
 * any time there is a dirty window.
 */
modeline(wp)
WINDOW *wp;
{
	register int n;
	register BUFFER *bp;
	register lchar;		/* character to draw line in buffer with */

	n = wp->w_toprow+wp->w_ntrows;      	/* Location. */
	vscreen[n]->v_flag |= VFCHG | VFREQ | VFCOL;/* Redraw next time. */

#if	NeWS
	vscreen[n]->v_rfcolor = 0;
	vscreen[n]->v_rbcolor = 7;
	if (wp == curwp) {			/* mark the current buffer */
		lchar = '^' ;
	} else {
		lchar = ' ' ;
	}
	vtmove(n, 0);                      	/* Seek to right line. */
#else

#if	COLOR
	vscreen[n]->v_rfcolor = 0;			/* black on */
	vscreen[n]->v_rbcolor = 7;			/* white.....*/
#endif
	vtmove(n, 0);                       	/* Seek to right line. */
	if (wp == curwp) {				/* mark the current buffer */
		lchar = '=';
	} else {
#if	REVSTA
		if (revexist)
			lchar = ' ';
		else
#endif
			lchar = '-';
#endif
	}
	bp = wp->w_bufp;

	vtputc(lchar,FALSE);
	vtputc(' ',FALSE);
	vtputsn(bp->b_bname, NBUFN);
	if (bp->b_mode&MDVIEW)
		vtputsn(" [view only]", 20);
	if (bp->b_mode&MDDOS)
		vtputsn(" [dos-style]", 20);
	if (bp->b_flag&BFCHG)
		vtputsn(" [modified]", 20);
	/* don't print a filename if they're the same, 
		or the filename is null */
	if (strcmp(bp->b_fname,bp->b_bname)) {
		if (bp->b_fname[0] != '\0') {
			if (isspace(bp->b_fname[0])) {
				/* some of the internally generated buffers
					put other info. in filename slot */
				vtputsn(bp->b_fname, NFILEN);
			} else {
				if (ispunct(bp->b_fname[0]))
					vtputsn(" is \"", 20);
				else
					vtputsn(" is file \"", 20);
				vtputsn(bp->b_fname, NFILEN);
				vtputsn("\"", 20);
			}
		}
	}
	vtputc(' ',FALSE);


	/* Pad to full width, then go back and overwrite right-end info */
	n = term.t_ncol;
	while (vtcol < n)
		vtputc(lchar,FALSE);
		
	{ /* determine if top line, bottom line, or both are visible */
		LINE *lp = wp->w_linep;
		int rows = wp->w_ntrows;
		char *msg = NULL;
		
		vtcol = n - 7;  /* strlen(" top ") plus a couple */
		while (rows--) {
			lp = lforw(lp);
			if (lp == wp->w_bufp->b_linep) {
				msg = " bot ";
				break;
			}
		}
		if (lback(wp->w_linep) == wp->w_bufp->b_linep) {
			if (msg) {
				if (wp->w_linep == wp->w_bufp->b_linep)
					msg = " emp ";
				else 
					msg = " all ";
			} else {
				msg = " top ";
			}
		}
		if (!msg)
			msg = " mid ";
		vtputsn(msg,20);
	}

	if (vtgetc(80) == lchar) {
		vtcol = 80;
		vtputc('|',FALSE);
	}
}

upmode()	/* update all the mode lines */
{
	register WINDOW *wp;

#if     NeWS            /* tell workstation the current modes */
        newsreportmodes() ;
#endif
	wp = wheadp;
	while (wp != NULL) {
		wp->w_flag |= WFMODE;
		wp = wp->w_wndp;
	}
}

/*
 * Send a command to the terminal to move the hardware cursor to row "row"
 * and column "col". The row and column arguments are origin 0. Optimize out
 * random calls. Update "ttrow" and "ttcol".
 */
movecursor(row, col)
{
#if	! NeWS		/* "line buffered" */
	if (row!=ttrow || col!=ttcol)
#endif
        {
	        ttrow = row;
	        ttcol = col;
	        TTmove(row, col);
        }
}



#if	NeWS		/* buffer the message line stuff, newsputc is slow */
#define NEWSBUFSIZ	256
#undef	TTputc
#undef	TTflush
#define TTputc(c)	bufputc(c)
#define TTflush()	bufputc((char)0)

bufputc(c)
char	c ;
{
	static		bufindex = 0 ;
	static char	outbuf[NEWSBUFSIZ] ;

	if (c == NULL || bufindex >= NEWSBUFSIZ || bufindex >= term.t_ncol) {
		outbuf[bufindex] = NULL ;
		newsputline(term.t_nrow, outbuf, 7, 0) ;
		movecursor(term.t_nrow, strlen(outbuf)) ;
		newsflush() ;
		bufindex = 0 ;
	}
	else outbuf[bufindex++] = c ;
}
#endif


/*
 * Erase the message line. This is a special routine because the message line
 * is not considered to be part of the virtual screen. It always works
 * immediately; the terminal buffer is flushed via a call to the flusher.
 */
mlerase()
{
    int i;
    
    if (mpresf == FALSE)
		return;
    movecursor(term.t_nrow, 0);
    if (discmd == FALSE)
    	return;

#if	COLOR
     TTforg(7);
     TTbacg(0);
#endif
    if (eolexist == TRUE)
	    TTeeol();
    else {
        for (i = 0; i < term.t_ncol - 1; i++)
            TTputc(' ');
        movecursor(term.t_nrow, 1);	/* force the move! */
        movecursor(term.t_nrow, 0);
    }
    TTflush();
    mpresf = FALSE;
}




#ifndef va_dcl	 /* then try these out */

typedef char *va_list;
#define va_dcl int va_alist;
#define va_start(list) list = (char *) &va_alist
#define va_end(list)
#define va_arg(list, mode) ((mode *)(list += sizeof(mode)))[-1]

#endif

/*
 * Write a message into the message line. Keep track of the physical cursor
 * position. A small class of printf like format items is handled.
 * Set the "message line" flag TRUE.
 */

/* VARARGS */
mlwrite(fmt, va_alist)
char *fmt;	/* format string for output */
va_dcl
{
	register int c;		/* current char in format string */
	register va_list ap;	/* ptr to current data field */

	/* if we are not currently echoing on the command line, abort this */
	if (dotcmdmode == PLAY || discmd == FALSE) {
		movecursor(term.t_nrow, 0);
		return;
	}

#if	COLOR
	/* set up the proper colors for the command line */
	TTforg(7);
	TTbacg(0);
#endif

	/* if we can not erase to end-of-line, do it manually */
	if (eolexist == FALSE) {
		mlerase();
		TTflush();
	}

	va_start(ap);

	movecursor(term.t_nrow, 0);
	while ((c = *fmt) != 0 && ttcol < term.t_ncol-1) {
		if (c != '%') {
			mlputc(c);
		} else {
			c = *++fmt;
			switch (c) {
				case '\0':
					break;
				case 'c':
					mlputc(va_arg(ap,int));
					break;

				case 'd':
					mlputi(va_arg(ap,int), 10);
					break;

				case 'o':
					mlputi(va_arg(ap,int), 8);
					break;

				case 'x':
					mlputi(va_arg(ap,int), 16);
					break;

				case 'D':
					mlputli(va_arg(ap,long), 10);
					break;

				case 's':
					mlputs(va_arg(ap,char *));
					break;

				case 'S': {
					int wid = va_arg(ap,int);
					mlputsn(va_arg(ap,char *),wid);
					break;
					}

				case 'f':
					mlputf(va_arg(ap,int));
					break;

				default:
					mlputc(c);
			}
		}
		fmt++;
	}

	va_end(ap);

	/* if we can, erase to the end of screen */
	if (eolexist == TRUE)
		TTeeol();
	TTflush();
	mpresf = TRUE;
}

/*	Force a string out to the message line regardless of the
	current $discmd setting. This is needed when $debug is TRUE
	and for the write-message and clear-message-line commands
*/

mlforce(s)
char *s;	/* string to force out */
{
	register oldcmd;	/* original command display flag */

	oldcmd = discmd;	/* save the discmd value */
	discmd = TRUE;		/* and turn display on */
	mlwrite(s);		/* write the string out */
	discmd = oldcmd;	/* and restore the original setting */
}

/*
 * Write out a character. Update the physical cursor position. This assumes that
 * the character has width "1"; if this is not the case
 * things will get screwed up a little.
 */
mlputc(c)
char c;
{
	if (c == '\r') ttcol = 0;
	if (ttcol < term.t_ncol-1) {
	        TTputc(c);
	        ++ttcol;
	}
}

/*
 * Write out a string. Update the physical cursor position. This assumes that
 * the characters in the string all have width "1"; if this is not the case
 * things will get screwed up a little.
 */
mlputs(s)
char *s;
{
	register int c;

	while ((c = *s++) != 0) {
	        mlputc(c);
	}
}

/* as above, but takes a count for s's length */
mlputsn(s,n)
char *s;
{
	register int c;
	while ((c = *s++) != 0 && n-- != 0) {
	        mlputc(c);
	}
}

/*
 * Write out an integer, in the specified radix. Update the physical cursor
 * position.
 */
mlputi(i, r)
{
    register int q;
    static char hexdigits[] = "0123456789ABCDEF";

    if (i < 0) {
        i = -i;
        mlputc('-');
    }

    q = i/r;

    if (q != 0)
        mlputi(q, r);

    mlputc(hexdigits[i%r]);
}

/*
 * do the same except as a long integer.
 */
mlputli(l, r)
long l;
{
    register long q;

    if (l < 0) {
        l = -l;
        mlputc('-');
    }

    q = l/r;

    if (q != 0)
        mlputli(q, r);

    mlputc((int)(l%r)+'0');
}

/*
 *	write out a scaled integer with two decimal places
 */

mlputf(s)
int s;	/* scaled integer to output */
{
	register int i;	/* integer portion of number */
	register int f;	/* fractional portion of number */

	/* break it up */
	i = s / 100;
	f = s % 100;

	/* send out the integer portion */
	mlputi(i, 10);
	mlputc('.');
	mlputc((f / 10) + '0');
	mlputc((f % 10) + '0');
}	

#if RAINBOW

putline(row, col, buf)
int row, col;
char buf[];
{
    int n;

    n = strlen(buf);
    if (col + n - 1 > term.t_ncol)
        n = term.t_ncol - col + 1;
    Put_Data(row, col, n, buf);
}
#endif

/* Get terminal size from system.
   Store number of lines into *heightp and width into *widthp.
   If zero or a negative number is stored, the value is not valid.  */

getscreensize (widthp, heightp)
int *widthp, *heightp;
{
#ifdef TIOCGWINSZ
	struct winsize size;
	*widthp = 0;
	*heightp = 0;
	if (ioctl (0, TIOCGWINSZ, &size) < 0)
		return;
	*widthp = size.ws_col;
	*heightp = size.ws_row;
#else
	*widthp = 0;
	*heightp = 0;
#endif
}

#ifdef SIGWINCH
sizesignal ()
{
	int w, h;
	extern int errno;
	int old_errno = errno;

	getscreensize (&w, &h);

	if ((h && h-1 != term.t_nrow) || (w && w != term.t_ncol))
		newscreensize(h, w);

	signal (SIGWINCH, sizesignal);
	errno = old_errno;
}

newscreensize (h, w)
int h, w;
{
	/* do the change later */
	if (displaying) {
		chg_width = w;
		chg_height = h;
		return;
	}
	chg_width = chg_height = 0;
	if (h - 1 < term.t_mrow)
		newlength(h);
	if (w < term.t_mcol)
		newwidth(w);

	update(TRUE);
	return TRUE;
}

#endif
