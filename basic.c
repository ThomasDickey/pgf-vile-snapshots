/*
 * The routines in this file move the cursor around on the screen. They
 * compute a new value for the cursor, then adjust ".". The display code
 * always updates the cursor location, so only moves between lines, or
 * functions that adjust the top line in the window and invalidate the
 * framing, are hard.
 */
#include        <stdio.h>
#include	"estruct.h"
#include        "edef.h"

/*
 * Move the cursor to the
 * beginning of the current line.
 * Trivial.
 */
gotobol(f, n)
{
        curwp->w_doto  = 0;
        return (TRUE);
}

/*
 * Move the cursor backwards by "n" characters. If "n" is less than zero call
 * "forwchar" to actually do the move. Otherwise compute the new cursor
 * location. Error if you try and move out of the buffer. Set the flag if the
 * line pointer for dot changes.
 */
backchar(f, n)
register int    n;
{
        register LINE   *lp;

	if (f == FALSE) n = 1;
        if (n < 0)
                return (forwchar(f, -n));
        while (n--) {
                if (curwp->w_doto == 0) {
                        if ((lp=lback(curwp->w_dotp)) == curbp->b_linep)
                                return (FALSE);
                        curwp->w_dotp  = lp;
                        curwp->w_doto  = llength(lp);
                        curwp->w_flag |= WFMOVE;
                } else
                        curwp->w_doto--;
        }
        return (TRUE);
}

/*
 * Move the cursor to the end of the current line. Trivial. No errors.
 */
gotoeol(f, n)
{
	if (f == TRUE) {
		if (n > 0)
			 --n;
		else if (n < 0)
			 ++n;
		forwline(f,n);
	}
        curwp->w_doto  = llength(curwp->w_dotp);
	curgoal = HUGE;
        return (TRUE);
}

/*
 * Move the cursor forwards by "n" characters. If "n" is less than zero call
 * "backchar" to actually do the move. Otherwise compute the new cursor
 * location, and move ".". Error if you try and move off the end of the
 * buffer. Set the flag if the line pointer for dot changes.
 */
forwchar(f, n)
register int    n;
{
	if (f == FALSE) n = 1;
        if (n < 0)
                return (backchar(f, -n));
        while (n--) {
                if (curwp->w_doto == llength(curwp->w_dotp)) {
                        if (curwp->w_dotp == curbp->b_linep ||
					lforw(curwp->w_dotp) == curbp->b_linep)
                                return (FALSE);
                        curwp->w_dotp  = lforw(curwp->w_dotp);
                        curwp->w_doto  = 0;
                        curwp->w_flag |= WFMOVE;
                } else
                        curwp->w_doto++;
        }
        return (TRUE);
}

gotoline(f, n)		/* move to a particular line.
			   argument (n) must be a positive integer for
			   this to actually do anything		*/
{
	register int status;	/* status return */

	/* get an argument if one doesnt exist */
	if (f == FALSE) {
		return(gotoeob(f,n));
	}

	if (n < 1)		/* if a bogus argument...then leave */
		return(FALSE);

	/* first, we go to the start of the buffer */
        curwp->w_dotp  = lforw(curbp->b_linep);
        curwp->w_doto  = 0;
	status = forwline(f, n-1);
	if (status == TRUE)
		firstnonwhite(f,n);
	return(status);
}

/*
 * Goto the beginning of the buffer. Massive adjustment of dot. This is
 * considered to be hard motion; it really isn't if the original value of dot
 * is the same as the new value of dot.
 */
gotobob(f, n)
{
        curwp->w_dotp  = lforw(curbp->b_linep);
        curwp->w_doto  = 0;
        curwp->w_flag |= WFMOVE;
        return (TRUE);
}

/*
 * Move to the end of the buffer. Dot is always put at the end of the file
 * (ZJ). The standard screen code does most of the hard parts of update.
 */
gotoeob(f, n)
{
        curwp->w_dotp  = lback(curbp->b_linep);
	firstnonwhite(FALSE,1);
        curwp->w_flag |= WFMOVE;
        return (TRUE);
}

gotobos(f,n)
{
	int s = TRUE;
	if (f == FALSE || n <= 0) n = 1;
	curwp->w_dotp = curwp->w_linep;
	while (--n) {
		if ((s = forwline(FALSE,1)) != TRUE)
			break;
	}
	firstnonwhite(f,n);
	return (s);
}

gotomos(f,n)
{
	return gotobos(TRUE,curwp->w_ntrows/2);
}

gotoeos(f,n)
{
	return gotobos(TRUE,curwp->w_ntrows-(f==TRUE? n-1:0));
}

/*
 * Move forward by full lines. If the number of lines to move is less than
 * zero, call the backward line function to actually do it. The last command
 * controls how the goal column is set. No errors are
 * possible.
 */
forwline(f, n)
{
        register LINE   *dlp;

	if (f == FALSE) n = 1;
        if (n < 0)
                return (backline(f, -n));

	/* if we are on the last line as we start....fail the command */
	if (curwp->w_dotp == curbp->b_linep)
		return(FALSE);

	/* if the last command was not not a line move,
	   reset the goal column */
        if (curgoal < 0)
                curgoal = getccol(FALSE);

	/* and move the point down */
        dlp = curwp->w_dotp;
        while (n-- && dlp!=curbp->b_linep)
                dlp = lforw(dlp);

	/* reseting the current position */
        curwp->w_dotp  = (dlp == curbp->b_linep) ? lback(dlp) : dlp;
        curwp->w_doto  = getgoal(dlp);
        curwp->w_flag |= WFMOVE;
        return (TRUE);
}

firstnonwhite(f,n)
{
	int c;
        curwp->w_doto  = 0;
	while ( curwp->w_doto != llength(curwp->w_dotp) && 
			isspace(lgetc(curwp->w_dotp, curwp->w_doto)) )
		curwp->w_doto++;
	return (TRUE);
}

lastnonwhite(f,n)
{
	int c;
        curwp->w_doto  = llength(curwp->w_dotp)-1;
	while ( curwp->w_doto != 0 && 
	    ((c = lgetc(curwp->w_dotp, curwp->w_doto)) == ' ' || c == '\t'))
		curwp->w_doto--;
	return (TRUE);

}

/* like forwline, but got to first non-white char position */
forwbline(f,n)
{
	int s;

	if (f == FALSE) n = 1;
	if ((s = forwline(f,n)) != TRUE)
		return (s);
	firstnonwhite(f,n);
	return(TRUE);
}

/* like backline, but got to first non-white char position */
backbline(f,n)
{
	int s;

	if (f == FALSE) n = 1;
	if ((s = backline(f,n)) != TRUE)
		return (s);
	firstnonwhite(f,n);
	return(TRUE);
}

/*
 * This function is like "forwline", but goes backwards. The scheme is exactly
 * the same. Check for arguments that are less than zero and call your
 * alternate. Figure out the new line and call "movedot" to perform the
 * motion. No errors are possible.
 */
backline(f, n)
{
        register LINE   *dlp;

	if (f == FALSE) n = 1;
        if (n < 0)
                return (forwline(f, -n));


	/* if we are on the last line as we start....fail the command */
	if (lback(curwp->w_dotp) == curbp->b_linep)
		return(FALSE);

	/* if the last command was not note a line move,
	   reset the goal column */
        if (curgoal < 0)
                curgoal = getccol(FALSE);

	/* and move the point up */
        dlp = curwp->w_dotp;
        while (n-- && lback(dlp)!=curbp->b_linep)
                dlp = lback(dlp);

	/* reseting the current position */
        curwp->w_dotp  = dlp;
        curwp->w_doto  = getgoal(dlp);
        curwp->w_flag |= WFMOVE;
        return (TRUE);
}

#if	WORDPRO

gotobop(f,n)
{
	return(backlinebeg(f,n,"\n.","ILPQb"));
}
gotoeop(f,n)
{
	return(forwlinebeg(f,n,"\n.","ILPQb"));
}
gotobosec(f,n)
{
#if STUTTER_SEC_CMD
	int this1key;
	if (!clexec) {
		this1key = last1key;
		kbd_seq();
		if (this1key != last1key) {
			TTbeep();
			return(FALSE);
		}
	}
#endif
	return(backlinebeg(f,n,"{\f.","SHN"));
}
gotoeosec(f,n)
{
#if STUTTER_SEC_CMD
	int this1key;
	if (!clexec) {
		this1key = last1key;
		kbd_seq();
		if (this1key != last1key) {
			TTbeep();
			return(FALSE);
		}
	}
#endif
	return(forwlinebeg(f,n,"{\f.","SHN"));
}

backlinebeg(f, n, s1, s2)
char *s1, *s2;
{
	register int suc;	/* success of last backchar */
	LINE *odotp;

	if (f == FALSE) n = 1;
	if (n < 0)	/* the other way...*/
		return(gotoeop(f, -n));

	odotp = curwp->w_dotp;
	while (n-- > 0) {	/* for each one asked for */

		/* first scan back until we are in a word */
		suc = backchar(FALSE, 1);
		while (!inword() && suc)
			suc = backchar(FALSE, 1);

		while (lback(curwp->w_dotp) != curbp->b_linep) {
			if (issecbeg(s1,s2) == TRUE)
				break;
			curwp->w_dotp = lback(curwp->w_dotp);
		}
	}
	/* if doing an operation and we moved */
	if (doingopcmd && odotp != curwp->w_dotp) {
		curwp->w_dotp = lforw(curwp->w_dotp);
		curwp->w_doto = 0;
	} else {
		firstnonwhite(f,n);
	}
	curwp->w_flag |= WFMOVE;	/* force screen update */
	return(TRUE);
}

forwlinebeg(f, n, s1, s2)
char *s1, *s2;
{
	register int suc;	/* success of last backchar */
	LINE *odotp;

	if (f == FALSE) n = 1;
	if (n < 0)	/* the other way...*/
		return(gotobop(f, -n));

	odotp = curwp->w_dotp;
	while (n-- > 0) {	/* for each one asked for */

		/* first scan forward until we are in a word */
		suc = forwchar(FALSE, 1);
		while (!inword() && suc)
			suc = forwchar(FALSE, 1);
		curwp->w_doto = 0;	/* and go to the B-O-Line */
		if (suc)	/* of next line if not at EOF */
			curwp->w_dotp = lforw(curwp->w_dotp);

		while (curwp->w_dotp != curbp->b_linep) {
			if (issecbeg(s1,s2) == TRUE)
				break;
			curwp->w_dotp = lforw(curwp->w_dotp);
		}
	}
	/* if doing an operation and we moved */
	if (doingopcmd && odotp != curwp->w_dotp) {
		curwp->w_dotp = lback(curwp->w_dotp);
		curwp->w_doto = llength(curwp->w_dotp)-1;
	} else {
		firstnonwhite(f,n);
	}
	curwp->w_flag |= WFMOVE;	/* force screen update */
	return(TRUE);
}

/* a new "section" of some sort starts at the beginning of the line,
	with either a character from s1 or a "." followed by a character
	from s2 */
issecbeg(s1,s2)
char *s1,*s2;
{
	register char *cp1, *cp2;
	register int l, c1, c2;

	l = llength(curwp->w_dotp);
	for(cp1 = s1; *cp1 != 0; cp1++) {
		if ( l == 0) {
			if (*cp1 == '\n')
				return TRUE;
			else
				continue;
		}
		c1 = lgetc(curwp->w_dotp, 0);
		if (c1 == '.' && *cp1 == '.' && s2) {
			for(cp2 = s2; *cp2 != 0; cp2++) {
				if ( l <= 1) {
					if (*cp2 == '\n')
						return TRUE;
					else
						continue;
				} 
				c2 = lgetc(curwp->w_dotp, 1);
				if ( *cp2 == c2 )
					return TRUE;
			}
			
		} else if ( *cp1 == c1 ) {
			return TRUE;
		}
	}
	return FALSE;
}
#endif

/*
 * This routine, given a pointer to a LINE, and the current cursor goal
 * column, return the best choice for the offset. The offset is returned.
 * Used by "C-N" and "C-P".
 */
getgoal(dlp)
register LINE   *dlp;
{
        register int    c;
        register int    col;
        register int    newcol;
        register int    dbo;

        col = 0;
        dbo = 0;
        while (dbo != llength(dlp)) {
                c = lgetc(dlp, dbo);
                newcol = col;
		if (((curwp->w_bufp->b_mode&MDLIST) == 0) && c == '\t')
                        newcol |= TABMASK;
                else if (!isprint(c))
                        ++newcol;
                ++newcol;
                if (newcol > curgoal)
                        break;
                col = newcol;
                ++dbo;
        }
        return (dbo);
}

/*
 * Scroll forward by a specified number of lines, or by a full page if no
 * argument.  The "2" in the arithmetic on the window size is
 * the overlap; this value is the default overlap value in ITS EMACS. Because
 * this zaps the top line in the display window, we have to do a hard update.
 */
forwpage(f, n)
register int    n;
{
        register LINE   *lp;

        if (f == FALSE) {
                n = curwp->w_ntrows - 2;        /* Default scroll.      */
                if (n <= 0)                     /* Forget the overlap   */
                        n = 1;                  /* if tiny window.      */
        } else if (n < 0)
                return (backpage(f, -n));
#if     CVMVAS
        else                                    /* Convert from pages   */
                n *= curwp->w_ntrows;           /* to lines.            */
#endif
        lp = curwp->w_linep;
        while (n-- && lp!=curbp->b_linep)
                lp = lforw(lp);
        curwp->w_linep = lp;
        curwp->w_dotp  = lp;
        curwp->w_doto  = 0;
        curwp->w_flag |= WFHARD;
        return (TRUE);
}

/*
 * This command is like "forwpage", but it goes backwards. The "2", like
 * above, is the overlap between the two windows. The value is from the ITS
 * EMACS manual. We do a hard update for exactly the same
 * reason.
 */
backpage(f, n)
register int    n;
{
        register LINE   *lp;

        if (f == FALSE) {
                n = curwp->w_ntrows - 2;        /* Default scroll.      */
                if (n <= 0)                     /* Don't blow up if the */
                        n = 1;                  /* window is tiny.      */
        } else if (n < 0)
                return (forwpage(f, -n));
#if     CVMVAS
        else                                    /* Convert from pages   */
                n *= curwp->w_ntrows;           /* to lines.            */
#endif
        lp = curwp->w_linep;
        while (n-- && lback(lp)!=curbp->b_linep)
                lp = lback(lp);
        curwp->w_linep = lp;
        curwp->w_dotp  = lp;
        curwp->w_doto  = 0;
        curwp->w_flag |= WFHARD;
        return (TRUE);
}

/*
 * Scroll forward by a specified number of lines, or by a full page if no
 * argument. The "2" in the arithmetic on the window size is
 * the overlap; this value is the default overlap value in ITS EMACS. Because
 * this zaps the top line in the display window, we have to do a hard update.
 */
forwhpage(f, n)
register int    n;
{
        register LINE   *llp, *dlp;

        if (f == FALSE) {
                n = curwp->w_ntrows / 2;        /* Default scroll.      */
                if (n <= 0)                     /* Forget the overlap   */
                        n = 1;                  /* if tiny window.      */
        } else if (n < 0)
                return (backhpage(f, -n));
#if     CVMVAS
        else                                    /* Convert from pages   */
                n *= curwp->w_ntrows/2;           /* to lines.            */
#endif
        llp = curwp->w_linep;
        dlp = curwp->w_dotp;
        while (n-- && lforw(dlp) != curbp->b_linep) {
                llp = lforw(llp);
                dlp = lforw(dlp);
	}
        curwp->w_linep = llp;
        curwp->w_dotp  = dlp;
	firstnonwhite(f,n);
        curwp->w_flag |= WFHARD|WFKILLS;
        return (TRUE);
}

/*
 * This command is like "forwpage", but it goes backwards. The "2", like
 * above, is the overlap between the two windows. The value is from the ITS
 * EMACS manual. We do a hard update for exactly the same
 * reason.
 */
backhpage(f, n)
register int    n;
{
        register LINE   *llp, *dlp;

        if (f == FALSE) {
                n = curwp->w_ntrows / 2;        /* Default scroll.      */
                if (n <= 0)                     /* Don't blow up if the */
                        n = 1;                  /* window is tiny.      */
        } else if (n < 0)
                return (forwhpage(f, -n));
#if     CVMVAS
        else                                    /* Convert from pages   */
                n *= curwp->w_ntrows/2;           /* to lines.            */
#endif
        llp = curwp->w_linep;
        dlp = curwp->w_dotp;
        while (n-- && lback(dlp)!=curbp->b_linep) {
                llp = lback(llp);
                dlp = lback(dlp);
	}
        curwp->w_linep = llp;
        curwp->w_dotp  = dlp;
	firstnonwhite(f,n);
        curwp->w_flag |= WFHARD|WFINS;
        return (TRUE);
}



/*
 * Set the named mark in the current window to the value of "." in the window.
 * No errors are possible.
 */
setnmmark(f,n)
{
	char *s;
	int c,i;

	c = kbd_key();
	if (c < 'a' || c > 'z') {
		TTbeep();
		mlwrite("[Invalid mark name]");
		return FALSE;
	}
		
	if (curbp->b_nmmarks == NULL) {
		curbp->b_nmmarks = 
			(struct MARK *)malloc(26*sizeof(struct MARK));
		if (curbp->b_nmmarks == NULL) {
			mlwrite("[OUT OF MEMORY]");
			return(FALSE);
		}
		for (i = 0; i < 26; i++) {
			curbp->b_nmmarks[i].markp = NULL;
			curbp->b_nmmarks[i].marko = 0;
		}
	}
		
        curbp->b_nmmarks[c-'a'].markp = curwp->w_dotp;
        curbp->b_nmmarks[c-'a'].marko = curwp->w_doto;
        s = "[Mark X set]";
	s[6] = c;
        mlwrite(s);
        return (TRUE);
}

golinenmmark(f,n)
{
	int status;
	status = goexactnmmark(f,n);
	if (status != TRUE)
		return(status);
	firstnonwhite(f,n);
	return(TRUE);
}

goexactnmmark(f,n)
{
	register int c;
	register LINE **markpp;
	register int *markop;
	LINE *tmarkp;
	int tmarko;
	int this1key;
	int useldmark;
	int found;

	this1key = last1key;
	c = kbd_seq();
	useldmark = (last1key == this1key);
	c = kcod2key(c);

	if ((c < 'a' || c > 'z') && !useldmark) {
		TTbeep();
		mlwrite("[Invalid mark name]");
		return FALSE;
	}

	markpp = NULL;

	if (useldmark) { /* it's a stutter */
		markpp = &(curwp->w_ldmkp);
		markop = &(curwp->w_ldmko);
	} else if (curbp->b_nmmarks != NULL) {
		markpp = &(curbp->b_nmmarks[c-'a'].markp);
 		markop = &(curbp->b_nmmarks[c-'a'].marko);
	}
		
	found = FALSE;
	if (markpp != NULL && *markpp != NULL) {
		register LINE *lp;
		for (lp = lforw(curbp->b_linep);
				lp != curbp->b_linep; lp = lforw(lp)) {
			if (*markpp == lp) {
				found = TRUE;
				break;
			}
		}
	}
	if (!found) {
		TTbeep();
		mlwrite("[Mark not set]");
		return (FALSE);
	}
		
	if (useldmark) {
		tmarkp = curwp->w_dotp;
		tmarko = curwp->w_doto;
	}

	curwp->w_dotp = *markpp;
	curwp->w_doto = *markop;

	if (useldmark) {
		*markpp = tmarkp;
		*markop = tmarko;
	}

        curwp->w_flag |= WFMOVE;
        return (TRUE);
}

/*
 * Set the mark in the current window to the value of "." in the window. No
 * errors are possible.
 */
setmark()
{
        curwp->w_mkp = curwp->w_dotp;
        curwp->w_mko = curwp->w_doto;
        return (TRUE);
}

gomark()
{
        curwp->w_dotp = curwp->w_mkp;
        curwp->w_doto = curwp->w_mko;
        curwp->w_flag |= WFMOVE;
        return (TRUE);
}

atmark()
{
        return ((curwp->w_dotp == curwp->w_mkp) && 
		(curwp->w_doto == curwp->w_mko));
}

/*
 * Swap the values of "." and "mark" in the current window. This is pretty
 * easy, bacause all of the hard work gets done by the standard routine
 * that moves the mark about. The only possible error is "no mark".
 */
swapmark()
{
        register LINE   *odotp;
        register int    odoto;

        if (curwp->w_mkp == NULL) {
                mlwrite("No mark in this window");
                return (FALSE);
        }
        odotp = curwp->w_dotp;
        odoto = curwp->w_doto;
        curwp->w_dotp  = curwp->w_mkp;
        curwp->w_doto  = curwp->w_mko;
        curwp->w_mkp = odotp;
        curwp->w_mko = odoto;
        curwp->w_flag |= WFMOVE;
        return (TRUE);
}



#if	NeWS
/* SETCURSOR
 *
 * Mouse support function.  Put the cursor to the requested location.
 * The cursor will be put just after the last character of the line if 
 * requested past the line.  The coordinates are expected in the command
 * stream.
 *   In the case of multiple windows, the window indicated by the mouse
 * is located and made the current window.
 */
setcursor()
{
int	row, col, pasteol ;
register LINE	*dlp;
WINDOW *wp0 ;		/* current window on entry */

	row = tgetc() ;
	col = tgetc() ;

/* find the window we are pointing to */
	wp0 = curwp ;
	while ( row < curwp->w_toprow ||
		row > curwp->w_ntrows + curwp->w_toprow ) {
		nextwind(FALSE,0) ;
		if ( curwp == wp0 ) break ;	/* full circle */
	}

/* move to the right row */
	row -= curwp->w_toprow ;
	dlp = curwp->w_linep ;			/* get pointer to 1st line */
	while ( row-- && (dlp != curbp->b_linep) ) dlp = lforw(dlp) ;
	curwp->w_dotp = dlp ;			/* set dot line pointer */

	/* now move the dot over until near the requested column */
	curgoal = col ;		/* a global for this ?? */
	curwp->w_doto = getgoal(dlp) ;
	curwp->w_flag |= WFMOVE;
	return (TRUE);
}
#endif
