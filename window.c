/*
 * Window management. Some of the functions are internal, and some are
 * attached to keys that the user actually types.
 */

#include        <stdio.h>
#include        "estruct.h"
#include	"edef.h"

#if	MEGAMAX & ST520
overlay	"window"
#endif

/*
 * Reposition dot's line to line "n" of the window. If the argument is
 * positive, it is that line. If it is negative it is that line from the
 * bottom. If it is 0 the window is centered around dot (this is what 
 * the standard redisplay code does). Defaults to 0.
 */
reposition(f, n)
{
    if (f == FALSE)	/* default to 0 to center screen */
	n = 0;
    curwp->w_force = n;
    curwp->w_flag |= WFFORCE;
    return (TRUE);
}

/*
 * Refresh the screen. With no argument, it just does the refresh. With an
 * argument it recenters "." in the current window.
 */
refresh(f, n)
{
#if	NeWS	/* see if the window has changed size */
    newsrefresh() ;
#endif	

	if (f == FALSE) {
		sgarbf = TRUE;
	} else {
	        curwp->w_force = 0;             /* Center dot. */
	        curwp->w_flag |= WFFORCE;
	}

#if     NeWS
	newsreportmodes() ;
#endif
	return (TRUE);
}

/*
 * The command make the next window (next => down the screen) the current
 * window. There are no real errors, although the command does nothing if
 * there is only 1 window on the screen.
 *
 * with an argument this command finds the <n>th window from the top
 *
 */
nextwind(f, n)
int f, n;	/* default flag and numeric argument */
{
	register WINDOW *wp;
	register int nwindows;		/* total number of windows */

	if (f) {

		/* first count the # of windows */
		wp = wheadp;
		nwindows = 1;
		while (wp->w_wndp != NULL) {
			nwindows++;
			wp = wp->w_wndp;
		}

		/* if the argument is negative, it is the nth window
		   from the bottom of the screen			*/
		if (n < 0)
			n = nwindows + n + 1;

		/* if an argument, give them that window from the top */
		if (n > 0 && n <= nwindows) {
			wp = wheadp;
			while (--n)
				wp = wp->w_wndp;
		} else {
			mlwrite("Window number out of range");
			return(FALSE);
		}
	} else {
		if ((wp = curwp->w_wndp) == NULL)
			wp = wheadp;
	}
	curwp = wp;
	make_current(curwp->w_bufp);
	upmode();
	return (TRUE);
}

poswind(f,n)
{
	register int c;
	register int rows;
	int s;

	c = kbd_key();
	if (c == abortc)
		return FALSE;

	if (c == '+' || c == '\r' || c == 'H') {
		rows = 1;
	} else if (c == '.' || c == 'M') {
		rows = 0;
	} else if (c == '-' || c == 'L') {
		rows = -1;
	} else {
		TTbeep();
		return FALSE;
	}

	if (f == TRUE) {
		s = gotoline(f,n);
		if (s != TRUE)
			return(s);
	}
	return(reposition(TRUE,rows));
}

/*
 * This command makes the previous window (previous => up the screen) the
 * current window. There arn't any errors, although the command does not do a
 * lot if there is 1 window.
 */
prevwind(f, n)
{
	register WINDOW *wp1;
	register WINDOW *wp2;

	/* if we have an argument, we mean the nth window from the bottom */
	if (f)
		return(nextwind(f, -n));

	wp1 = wheadp;
	wp2 = curwp;

	if (wp1 == wp2)
		wp2 = NULL;

	while (wp1->w_wndp != wp2)
		wp1 = wp1->w_wndp;

	curwp = wp1;
	make_current(curwp->w_bufp);
	upmode();
	return (TRUE);
}

/*
 * This command moves the current window down by "arg" lines. Recompute the
 * top line in the window. The move up and move down code is almost completely
 * the same; most of the work has to do with reframing the window, and picking
 * a new dot. We share the code by having "move down" just be an interface to
 * "move up". Magic.
 */
mvdnwind(f, n)
int n;
{
	return (mvupwind(f, -n));
}

/*
 * Move the current window up by "arg" lines. Recompute the new top line of
 * the window. Look to see if "." is still on the screen. If it is, you win.
 * If it isn't, then move "." to center it in the new framing of the window
 * (this command does not really move "." (except as above); it moves the 
 * frame).
 */
mvupwind(f, n)
int n;
{
    register LINE *lp;
    register int i;
    int was_n = n;

    lp = curwp->w_linep;

    if (n < 0) {
        while (n++ && lforw(lp) != curbp->b_linep)
            lp = lforw(lp);
    } else {
        while (n-- && lback(lp)!=curbp->b_linep)
            lp = lback(lp);
    }

    curwp->w_linep = lp;
    curwp->w_flag |= WFHARD|WFMODE;

	/* is it still in the window */
    for (i = 0; i < curwp->w_ntrows; ++i) {
        if (lp == curwp->w_dotp)
            return (TRUE);
        if (lforw(lp) == curbp->b_linep)
            break;
        lp = lforw(lp);
    }
	/* now lp is either just past the window bottom, or
		it's the last line of the file */

	/* preserve the current column */
    if (curgoal < 0)
        curgoal = getccol(FALSE);

#if BEFORE
    lp = curwp->w_linep;
    i  = curwp->w_ntrows/2;

    while (i-- && lforw(lp) != curbp->b_linep)
        lp = lforw(lp);

    curwp->w_dotp  = lp;
    curwp->w_doto  = 0;
#else
    if (was_n < 0)
	    curwp->w_dotp  = curwp->w_linep;
    else
	    curwp->w_dotp  = lback(lp);
    curwp->w_doto  = getgoal(curwp->w_dotp);
#endif
    return (TRUE);
}

mvdnnxtwind(f, n)
{
	nextwind(FALSE, 1);
	mvdnwind(f, n);
	prevwind(FALSE, 1);
}

mvupnxtwind(f, n)
{
	nextwind(FALSE, 1);
	mvupwind(f, n);
	prevwind(FALSE, 1);
}


/*
 * This command makes the current window the only window on the screen.
 * Try to set the framing so that "." does not have to move on the
 * display. Some care has to be taken to keep the values of dot and mark in
 * the buffer structures right if the distruction of a window makes a buffer
 * become undisplayed.
 */
onlywind(f, n)
{
        register WINDOW *wp;
        register LINE   *lp;
        register int    i;

        wp = wheadp;
        while (wp != NULL) {
		register WINDOW *nwp;
		nwp = wp->w_wndp;
        	if (wp != curwp) {
	                if (--wp->w_bufp->b_nwnd == 0)
	                        undispbuff(wp->w_bufp,wp);
	                free((char *) wp);
	        }
                wp = nwp;
        }
        wheadp = curwp;
        wheadp->w_wndp = NULL;
        lp = curwp->w_linep;
        i  = curwp->w_toprow;
        while (i!=0 && lback(lp)!=curbp->b_linep) {
                --i;
                lp = lback(lp);
        }
        curwp->w_toprow = 0;
        curwp->w_ntrows = term.t_nrow-1;
        curwp->w_linep  = lp;
        curwp->w_flag  |= WFMODE|WFHARD;
        return (TRUE);
}

/*
 * Delete the current window, placing its space in the window above,
 * or, if it is the top window, the window below.
 */

delwind(f,n)
int f, n;	/* arguments are ignored for this command */
{
	return delwp(curwp);
}

delwp(thewp)
WINDOW *thewp;
{
	register WINDOW *wp;	/* window to recieve deleted space */
	register LINE *lp;	/* line pointer */
	register int i;

	/* if there is only one window, don't delete it */
	if (wheadp->w_wndp == NULL) {
		mlwrite("Cannot delete the only window");
		return(FALSE);
	}

#if BEFORE
	/* find window before thewp in linked list */
	wp = wheadp;
	lwp = NULL;
	while (wp != NULL) {
		if (wp == thewp)
			break;
		lwp = wp;
		wp = wp->w_wndp;
	}
	/* find recieving window and give up our space */
	wp = wheadp;
	if (thewp->w_toprow == 0) {
		/* find the next window down */
		target = thewp->w_ntrows + 1;
		while (wp != NULL) {
			if (wp->w_toprow == target)
				break;
			wp = wp->w_wndp;
		}
		if (wp == NULL)
			return(FALSE);
                lp = wp->w_linep;
                for (i=target; i > 0 && lback(lp)!=wp->w_bufp->b_linep; --i)
                        lp = lback(lp);
                wp->w_linep  = lp;
		wp->w_toprow = 0;
		wp->w_ntrows += target;
	} else {
		/* find the next window up */
		target = thewp->w_toprow - 1;
		while (wp != NULL) {
			if ((wp->w_toprow + wp->w_ntrows) == target)
				break;
			wp = wp->w_wndp;
		}
		if (wp == NULL)
			return(FALSE);
		wp->w_ntrows += 1 + thewp->w_ntrows;
	}

	/* get rid of the current window */
	if (--thewp->w_bufp->b_nwnd == 0) {
		undispbuff(thewp->w_bufp,thewp);
	}
	if (lwp == NULL)
		wheadp = thewp->w_wndp;
	else
		lwp->w_wndp = thewp->w_wndp;
	free((char *)thewp);
	if (thewp == curwp) {
		curwp = wp;
		curwp->w_flag |= WFHARD;
		make_current(curwp->w_bufp);
	}
	upmode();
	return(TRUE);
#else
	/* find recieving window and give up our space */
	if (thewp == wheadp) { /* there's nothing before */
		/* find the next window down */
		wp = thewp->w_wndp;
                lp = wp->w_linep;
                /* the prev. window (thewp) has wp->w_toprow rows in it */
                for (i = wp->w_toprow;
                		 i > 0 && lback(lp) != wp->w_bufp->b_linep; --i)
                        lp = lback(lp);
                wp->w_linep  = lp;
		wp->w_ntrows += wp->w_toprow;  /* add in the new rows */
		wp->w_toprow = 0;	/* and we're at the top of the screen */
		wheadp = wp;	/* and at the top of the list as well */
	} else {
		/* find window before thewp in linked list */
		for (wp = wheadp; wp->w_wndp != thewp; wp = wp->w_wndp)
			;
		/* add thewp's rows to the next window up */
		wp->w_ntrows += thewp->w_ntrows+1;
		
		wp->w_wndp = thewp->w_wndp; /* make their next window ours */
	}

	/* get rid of the current window */
	if (--thewp->w_bufp->b_nwnd == 0)
		undispbuff(thewp->w_bufp,thewp);
	free((char *)thewp);
	if (thewp == curwp) {
		curwp = wp;
		curwp->w_flag |= WFHARD;
		make_current(curwp->w_bufp);
	}
	upmode();
	return(TRUE);
#endif
}

/*
	Split the current window.  A window smaller than 3 lines cannot be
	split.  An argument of 1 forces the cursor into the upper window, an
	argument of two forces the cursor to the lower window.  The only other
	error that is possible is a "malloc" failure allocating the structure
	for the new window.
 */
splitwind(f, n)
{
        register WINDOW *wp;
        register LINE   *lp;
        register int    ntru;
        register int    ntrl;
        register int    ntrd;
        register WINDOW *wp1;
        register WINDOW *wp2;
	register int i;

        if (curwp->w_ntrows < 3) {
                mlwrite("Cannot split a %d line window", curwp->w_ntrows);
                return (FALSE);
        }
        if ((wp = (WINDOW *) malloc(sizeof(WINDOW))) == NULL) {
                mlwrite("[OUT OF MEMORY]");
                return (FALSE);
        }
        ++curbp->b_nwnd;                        /* Displayed twice.     */
        wp->w_bufp  = curbp;
        wp->w_dotp  = curwp->w_dotp;
        wp->w_doto  = curwp->w_doto;
        wp->w_mkp = curwp->w_mkp;
        wp->w_mko = curwp->w_mko;
        wp->w_linep = curwp->w_linep;
        wp->w_ldmkp = curwp->w_ldmkp;
        wp->w_ldmko = curwp->w_ldmko;
        wp->w_flag  = 0;
        wp->w_force = 0;
#if	COLOR
	/* set the colors of the new window */
	wp->w_fcolor = gfcolor;
	wp->w_bcolor = gbcolor;
#endif
        ntru = (curwp->w_ntrows-1) / 2;         /* Upper size           */
        ntrl = (curwp->w_ntrows-1) - ntru;      /* Lower size           */
        lp = curwp->w_linep;
        ntrd = 0;
        while (lp != curwp->w_dotp) {
                ++ntrd;
                lp = lforw(lp);
        }
	/* ntrd is now the row containing dot */
        if (((f == FALSE) && (ntrd <= ntru)) || ((f == TRUE) && (n == 1))) {
                /* Old is upper window. */
	        /* Adjust the top line if necessary */
                if (ntrd == ntru)               /* Hit mode line.       */
			if (ntrl > 1) {
				ntru++;
				ntrl--;
			} else
	                        curwp->w_linep = lforw(curwp->w_linep);
                curwp->w_ntrows = ntru; /* new size */
		/* insert new window after curwp in window list */
                wp->w_wndp = curwp->w_wndp;
                curwp->w_wndp = wp;
		/* set new window's position and size */
                wp->w_toprow = curwp->w_toprow+ntru+1;
                wp->w_ntrows = ntrl;
		/* try to keep lower from reframing */
		for (i = ntru+1; i > 0 &&
				 wp->w_linep != wp->w_bufp->b_linep; i--) {
			wp->w_linep = lforw(wp->w_linep);
		}
		wp->w_dotp = wp->w_linep;
		wp->w_doto = 0;
        } else {
		/* Old is lower window  */
                wp1 = NULL;
                wp2 = wheadp;
                while (wp2 != curwp) {
                        wp1 = wp2;
                        wp2 = wp2->w_wndp;
                }
                if (wp1 == NULL)
                        wheadp = wp;
                else
                        wp1->w_wndp = wp;
                wp->w_wndp   = curwp;
                wp->w_toprow = curwp->w_toprow;
                wp->w_ntrows = ntru;
                ++ntru;                         /* Mode line.           */
                curwp->w_toprow += ntru;
                curwp->w_ntrows  = ntrl;
		wp->w_dotp = wp->w_linep;
		/* move upper window dot to bottom line of upper */
		for (i = ntru-2; i > 0 && wp->w_dotp!=wp->w_bufp->b_linep; i--)
			wp->w_dotp = lforw(wp->w_dotp);
		wp->w_doto = 0;
		/* adjust lower window topline */
                while (ntru--)
                        curwp->w_linep = lforw(curwp->w_linep);
        }
        curwp->w_flag |= WFMODE|WFHARD;
        wp->w_flag |= WFMODE|WFHARD;
        return (TRUE);
}

/*
 * Enlarge the current window. Find the window that loses space. Make sure it
 * is big enough. If so, hack the window descriptions, and ask redisplay to do
 * all the hard work. You don't just set "force reframe" because dot would
 * move.
 */
enlargewind(f, n)
{
        register WINDOW *adjwp;
        register LINE   *lp;
        register int    i;

        if (n < 0)
                return (shrinkwind(f, -n));
        if (wheadp->w_wndp == NULL) {
                mlwrite("Only one window");
                return (FALSE);
        }
        if ((adjwp=curwp->w_wndp) == NULL) {
                adjwp = wheadp;
                while (adjwp->w_wndp != curwp)
                        adjwp = adjwp->w_wndp;
        }
        if (adjwp->w_ntrows <= n) {
                mlwrite("Impossible change");
                return (FALSE);
        }
        if (curwp->w_wndp == adjwp) {           /* Shrink below.        */
                lp = adjwp->w_linep;
                for (i=0; i<n && lp!=adjwp->w_bufp->b_linep; ++i)
                        lp = lforw(lp);
                adjwp->w_linep  = lp;
                adjwp->w_toprow += n;
        } else {                                /* Shrink above.        */
                lp = curwp->w_linep;
                for (i=0; i<n && lback(lp)!=curbp->b_linep; ++i)
                        lp = lback(lp);
                curwp->w_linep  = lp;
                curwp->w_toprow -= n;
        }
        curwp->w_ntrows += n;
        adjwp->w_ntrows -= n;
        curwp->w_flag |= WFMODE|WFHARD|WFINS;
        adjwp->w_flag |= WFMODE|WFHARD|WFKILLS;
        return (TRUE);
}

/*
 * Shrink the current window. Find the window that gains space. Hack at the
 * window descriptions. Ask the redisplay to do all the hard work.
 */
shrinkwind(f, n)
{
        register WINDOW *adjwp;
        register LINE   *lp;
        register int    i;

        if (n < 0)
                return (enlargewind(f, -n));
        if (wheadp->w_wndp == NULL) {
                mlwrite("Only one window");
                return (FALSE);
        }
        if ((adjwp=curwp->w_wndp) == NULL) {
                adjwp = wheadp;
                while (adjwp->w_wndp != curwp)
                        adjwp = adjwp->w_wndp;
        }
        if (curwp->w_ntrows <= n) {
                mlwrite("Impossible change");
                return (FALSE);
        }
        if (curwp->w_wndp == adjwp) {           /* Grow below.          */
                lp = adjwp->w_linep;
                for (i=0; i<n && lback(lp)!=adjwp->w_bufp->b_linep; ++i)
                        lp = lback(lp);
                adjwp->w_linep  = lp;
                adjwp->w_toprow -= n;
        } else {                                /* Grow above.          */
                lp = curwp->w_linep;
                for (i=0; i<n && lp!=curbp->b_linep; ++i)
                        lp = lforw(lp);
                curwp->w_linep  = lp;
                curwp->w_toprow += n;
        }
        curwp->w_ntrows -= n;
        adjwp->w_ntrows += n;
        curwp->w_flag |= WFMODE|WFHARD|WFKILLS;
        adjwp->w_flag |= WFMODE|WFHARD|WFINS;
        return (TRUE);
}

#if !SMALLER

/*	Resize the current window to the requested size	*/
resize(f, n)
int f, n;	/* default flag and numeric argument */
{
	int clines;	/* current # of lines in window */
	
	/* must have a non-default argument, else ignore call */
	if (f == FALSE)
		return(TRUE);

	/* find out what to do */
	clines = curwp->w_ntrows;

	/* already the right size? */
	if (clines == n)
		return(TRUE);

	return(enlargewind(TRUE, n - clines));
}
#endif

/*
 * Pick a window for a pop-up. Split the screen if there is only one window.
 * Pick the uppermost window that isn't the current window. An LRU algorithm
 * might be better. Return a pointer, or NULL on error.
 */
WINDOW  *
wpopup()
{
        register WINDOW *wp;

        if (wheadp->w_wndp == NULL              /* Only 1 window        */
        	&& splitwind(FALSE, 0) == FALSE) /* and it won't split   */
                return (NULL);
        wp = wheadp;                            /* Find window to use   */
        while (wp!=NULL && wp==curwp)
                wp = wp->w_wndp;
        return (wp);
}

scrnextup(f, n)		/* scroll the next window up (back) a page */
{
	nextwind(FALSE, 1);
	backhpage(f, n);
	prevwind(FALSE, 1);
}

scrnextdw(f, n)		/* scroll the next window down (forward) a page */
{
	nextwind(FALSE, 1);
	forwhpage(f, n);
	prevwind(FALSE, 1);
}

#if ! SMALLER
savewnd(f, n)		/* save ptr to current window */
{
	swindow = curwp;
	return(TRUE);
}

restwnd(f, n)		/* restore the saved screen */
{
	register WINDOW *wp;

	/* find the window */
	wp = wheadp;
	while (wp != NULL) {
		if (wp == swindow) {
			curwp = wp;
			make_current(curwp->w_bufp);
			upmode();
			return (TRUE);
		}
		wp = wp->w_wndp;
	}

	mlwrite("[No such window exists]");
	return(FALSE);
}
#endif

newlength(n)	/* resize the screen, re-writing the screen */
int n;	/* numeric argument */
{
	WINDOW *wp;	/* current window being examined */
	WINDOW *nextwp;	/* next window to scan */
	WINDOW *lastwp;	/* last window scanned */
	int lastline;	/* screen line of last line of current window */

	/* make sure it's in range */
	if (n < 3 || n > term.t_mrow + 1) {
		mlwrite("Screen size out of range");
		return(FALSE);
	}

	if (term.t_nrow == n - 1)
		return(TRUE);
	else if (term.t_nrow < n - 1) {

		/* go to the last window */
		wp = wheadp;
		while (wp->w_wndp != NULL)
			wp = wp->w_wndp;

		/* and enlarge it as needed */
		wp->w_ntrows = n - wp->w_toprow - 2;
		wp->w_flag |= WFHARD|WFMODE;

	} else {

		/* rebuild the window structure */
		nextwp = wheadp;
		wp = NULL;
		lastwp = NULL;
		while (nextwp != NULL) {
			wp = nextwp;
			nextwp = wp->w_wndp;
	
			/* get rid of it if it is too low */
			if (wp->w_toprow > n - 2) {

				if (--wp->w_bufp->b_nwnd == 0) {
					undispbuff(wp->w_bufp,wp);
				}
	
				/* update curwp and lastwp if needed */
				if (wp == curwp)
					curwp = wheadp;
					make_current(curwp->w_bufp);
				if (lastwp != NULL)
					lastwp->w_wndp = NULL;

				/* free the structure */
				free((char *)wp);
				wp = NULL;

			} else {
				/* need to change this window size? */
				lastline = wp->w_toprow + wp->w_ntrows - 1;
				if (lastline >= n - 2) {
					wp->w_ntrows = n - wp->w_toprow - 2;
					wp->w_flag |= WFHARD|WFMODE;
				}
			}
	
			lastwp = wp;
		}
	}

	/* screen is garbage */
	term.t_nrow = n - 1;
	sgarbf = TRUE;
	return(TRUE);
}

newwidth(n)	/* resize the screen, re-writing the screen */
int n;	/* numeric argument */
{
	register WINDOW *wp;

	/* make sure it's in range */
	if (n < 10 || n > term.t_mcol) {
#if	NeWS		/* serious error for NeWS, halt */
		fprintf(stderr, "Screen width out of range\n") ;
		newsclose() ;
		exit(1) ;
#else
		mlwrite("Screen width out of range");
		return(FALSE);
#endif
	}

	/* otherwise, just re-width it (no big deal) */
	term.t_ncol = n;
	term.t_margin = n / 10;
	term.t_scrsiz = n - (term.t_margin * 2);

	/* force all windows to redraw */
	wp = wheadp;
	while (wp) {
		wp->w_flag |= WFHARD | WFMOVE | WFMODE;
		wp = wp->w_wndp;
	}
	sgarbf = TRUE;

	return(TRUE);
}

#if ! SMALLER
int getwpos()	/* get screen offset of current line in current window */
{
	register int sline;	/* screen line from top of window */
	register LINE *lp;	/* scannile line pointer */

	/* search down the line we want */
	lp = curwp->w_linep;
	sline = 1;
	while (lp != curwp->w_dotp) {
		++sline;
		lp = lforw(lp);
	}

	/* and return the value */
	return(sline);
}
#endif

/*
 * Initialize all of the windows.
 */
winit()
{
        register WINDOW *wp;

        wp = (WINDOW *) malloc(sizeof(WINDOW)); /* First window         */
        if (wp==NULL )
                exit(1);
        wheadp = wp;
        curwp  = wp;
        wp->w_wndp  = NULL;                     /* Initialize window    */
        wp->w_doto  = 0;
        wp->w_mkp = NULL;
        wp->w_mko = 0;
        wp->w_ldmkp = NULL;
        wp->w_ldmko = 0;
        wp->w_toprow = 0;
#if	COLOR
	/* initalize colors to global defaults */
	wp->w_fcolor = gfcolor;
	wp->w_bcolor = gbcolor;
#endif
        wp->w_ntrows = term.t_nrow-1;           /* "-1" for mode line.  */
        wp->w_force = 0;
        wp->w_flag  = WFMODE|WFHARD;            /* Full.                */
}


