/*
 * The routines in this file move the cursor around on the screen. They
 * compute a new value for the cursor, then adjust ".". The display code
 * always updates the cursor location, so only moves between lines, or
 * functions that adjust the top line in the window and invalidate the
 * framing, are hard.
 *
 * $Log: basic.c,v $
 * Revision 1.13  1991/09/26 13:05:45  pgf
 * undid forw/backline optimization, since it causes flags to not be set,
 * and moved LIST mode to window
 *
 * Revision 1.12  1991/09/24  01:04:33  pgf
 * forwline and backline now do nothing if passed 0 arg
 *
 * Revision 1.11  1991/09/19  12:22:57  pgf
 * paragraphs now end at nroff-style section boundaries as well
 *
 * Revision 1.10  1991/08/07  12:35:07  pgf
 * added RCS log messages
 *
 * revision 1.9
 * date: 1991/08/06 15:10:26;
 * bug fix in forwline, and
 * global/local values
 * 
 * revision 1.8
 * date: 1991/06/25 19:52:01;
 * massive data structure restructure
 * 
 * revision 1.7
 * date: 1991/06/20 17:22:42;
 * fixed write-to-const-string problem in setnmmark
 * 
 * revision 1.6
 * date: 1991/06/16 17:33:32;
 * added next_column() routine, along with converting to modulo tab processing
 * 
 * revision 1.5
 * date: 1991/06/15 09:08:44;
 * added new forwchar_to_eol, and backchar_to_bol
 * 
 * revision 1.4
 * date: 1991/06/03 10:17:45;
 * prompt for mark name in setnmmark if isnamedcmd
 * 
 * revision 1.3
 * date: 1991/05/31 10:29:06;
 * fixed "last dot" mark code, and
 * added godotplus() for the operators
 * 
 * revision 1.2
 * date: 1990/09/25 11:37:50;
 * took out old ifdef BEFORE code
 * 
 * revision 1.1
 * date: 1990/09/21 10:24:42;
 * initial vile RCS revision
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
        curwp->w_dot.o  = 0;
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
                if (curwp->w_dot.o == 0) {
                        if ((lp=lback(curwp->w_dot.l)) == curbp->b_line.l)
                                return (FALSE);
                        curwp->w_dot.l  = lp;
                        curwp->w_dot.o  = llength(lp);
                        curwp->w_flag |= WFMOVE;
                } else
                        curwp->w_dot.o--;
        }
        return (TRUE);
}

/*
 * Move the cursor backwards by "n" characters. Stop at beginning of line.
 */
backchar_to_bol(f, n)
register int    n;
{
        register LINE   *lp;

	if (f == FALSE) n = 1;
        if (n < 0)
                return forwchar_to_eol(f, -n);
        while (n--) {
                if (curwp->w_dot.o == 0)
			return TRUE;
                else
                        curwp->w_dot.o--;
        }
        return TRUE;
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
        curwp->w_dot.o  = llength(curwp->w_dot.l);
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
                if (is_at_end_of_line(curwp->w_dot)) {
                        if (is_header_line(curwp->w_dot, curbp) ||
					is_last_line(curwp->w_dot,curbp))
                                return (FALSE);
                        curwp->w_dot.l  = lforw(curwp->w_dot.l);
                        curwp->w_dot.o  = 0;
                        curwp->w_flag |= WFMOVE;
                } else
                        curwp->w_dot.o++;
        }
        return (TRUE);
}

/*
 * Move the cursor forwards by "n" characters. Don't go past end-of-line
 */
forwchar_to_eol(f, n)
register int    n;
{
	if (f == FALSE) n = 1;
        if (n < 0)
                return backchar_to_bol(f, -n);
        while (n--) {
                if (is_at_end_of_line(curwp->w_dot))
			return TRUE;
                else
                        curwp->w_dot.o++;
        }
        return TRUE;
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
        curwp->w_dot.l  = lforw(curbp->b_line.l);
        curwp->w_dot.o  = 0;
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
        curwp->w_dot.l  = lforw(curbp->b_line.l);
        curwp->w_dot.o  = 0;
        curwp->w_flag |= WFMOVE;
        return (TRUE);
}

/*
 * Move to the end of the buffer. Dot is always put at the end of the file
 * (ZJ). The standard screen code does most of the hard parts of update.
 */
gotoeob(f, n)
{
        curwp->w_dot.l  = lback(curbp->b_line.l);
	firstnonwhite(FALSE,1);
        curwp->w_flag |= WFMOVE;
        return (TRUE);
}

gotobos(f,n)
{
	int s = TRUE;
	if (f == FALSE || n <= 0) n = 1;
	curwp->w_dot.l = curwp->w_line.l;
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
	if (is_last_line(curwp->w_dot, curbp))
		return(FALSE);

	/* if the last command was not a line move,
	   reset the goal column */
        if (curgoal < 0)
                curgoal = getccol(FALSE);

	/* and move the point down */
        dlp = curwp->w_dot.l;
        while (n-- && dlp!=curbp->b_line.l)
                dlp = lforw(dlp);

	/* resetting the current position */
        curwp->w_dot.l  = (dlp == curbp->b_line.l) ? lback(dlp) : dlp;
        curwp->w_dot.o  = getgoal(dlp);
        curwp->w_flag |= WFMOVE;
        return (TRUE);
}

firstnonwhite(f,n)
{
	int c;
        curwp->w_dot.o  = 0;
	while ( !is_at_end_of_line(curwp->w_dot) &&
			isspace(char_at(curwp->w_dot)) )
		curwp->w_dot.o++;
	return TRUE;
}

lastnonwhite(f,n)
{
        curwp->w_dot.o  = llength(curwp->w_dot.l)-1;
	while ( curwp->w_dot.o != 0 && 
			isspace(char_at(curwp->w_dot)))
		curwp->w_dot.o--;
	return TRUE;

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

	/* if we are on the first line as we start....fail the command */
	if (is_first_line(curwp->w_dot, curbp))
		return(FALSE);

	/* if the last command was not note a line move,
	   reset the goal column */
        if (curgoal < 0)
                curgoal = getccol(FALSE);

	/* and move the point up */
        dlp = curwp->w_dot.l;
        while (n-- && lback(dlp)!=curbp->b_line.l)
                dlp = lback(dlp);

	/* reseting the current position */
        curwp->w_dot.l  = dlp;
        curwp->w_dot.o  = getgoal(dlp);
        curwp->w_flag |= WFMOVE;
        return (TRUE);
}

#if	WORDPRO

gotobop(f,n)
{
	return(backlinebeg(f,n,"\n.","ILPQbSHN"));
}
gotoeop(f,n)
{
	return(forwlinebeg(f,n,"\n.","ILPQbSHN"));
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

	odotp = curwp->w_dot.l;
	while (n-- > 0) {	/* for each one asked for */

		/* first scan back until we are in a word */
		suc = backchar(FALSE, 1);
		while (!inword() && suc)
			suc = backchar(FALSE, 1);

		while (!is_first_line(curwp->w_dot, curbp)) {
			if (issecbeg(s1,s2) == TRUE)
				break;
			curwp->w_dot.l = lback(curwp->w_dot.l);
		}
	}
	/* if doing an operation and we moved */
	if (doingopcmd && odotp != curwp->w_dot.l) {
		curwp->w_dot.l = lforw(curwp->w_dot.l);
		curwp->w_dot.o = 0;
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

	odotp = curwp->w_dot.l;
	while (n-- > 0) {	/* for each one asked for */

		/* first scan forward until we are in a word */
		suc = forwchar(FALSE, 1);
		while (!inword() && suc)
			suc = forwchar(FALSE, 1);
		curwp->w_dot.o = 0;	/* and go to the B-O-Line */
		if (suc)	/* of next line if not at EOF */
			curwp->w_dot.l = lforw(curwp->w_dot.l);

		while (!is_header_line(curwp->w_dot, curbp)) {
			if (issecbeg(s1,s2) == TRUE)
				break;
			curwp->w_dot.l = lforw(curwp->w_dot.l);
		}
	}
	/* if doing an operation and we moved */
	if (doingopcmd && odotp != curwp->w_dot.l) {
		curwp->w_dot.l = lback(curwp->w_dot.l);
		curwp->w_dot.o = llength(curwp->w_dot.l)-1;
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

	l = llength(curwp->w_dot.l);
	for(cp1 = s1; *cp1 != 0; cp1++) {
		if ( l == 0) {
			if (*cp1 == '\n')
				return TRUE;
			else
				continue;
		}
		c1 = lgetc(curwp->w_dot.l, 0);
		if (c1 == '.' && *cp1 == '.' && s2) {
			for(cp2 = s2; *cp2 != 0; cp2++) {
				if ( l <= 1) {
					if (*cp2 == '\n')
						return TRUE;
					else
						continue;
				} 
				c2 = lgetc(curwp->w_dot.l, 1);
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
		newcol = next_column(c,col);
                if (newcol > curgoal)
                        break;
                col = newcol;
                ++dbo;
        }
        return (dbo);
}

/* return the next column index, given the current char and column */
next_column(c,col)
{
	if (c == '\t' && (!w_val(curwp,WMDLIST)))
                return nextab(col);
        else if (!isprint(c))
                return col+2;
	else
                return col+1;
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
        lp = curwp->w_line.l;
        while (n-- && lp!=curbp->b_line.l)
                lp = lforw(lp);
        curwp->w_line.l = lp;
        curwp->w_dot.l  = lp;
        curwp->w_dot.o  = 0;
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
        lp = curwp->w_line.l;
        while (n-- && lback(lp)!=curbp->b_line.l)
                lp = lback(lp);
        curwp->w_line.l = lp;
        curwp->w_dot.l  = lp;
        curwp->w_dot.o  = 0;
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
        llp = curwp->w_line.l;
        dlp = curwp->w_dot.l;
        while (n-- && lforw(dlp) != curbp->b_line.l) {
                llp = lforw(llp);
                dlp = lforw(dlp);
	}
        curwp->w_line.l = llp;
        curwp->w_dot.l  = dlp;
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
        llp = curwp->w_line.l;
        dlp = curwp->w_dot.l;
        while (n-- && lback(dlp)!=curbp->b_line.l) {
                llp = lback(llp);
                dlp = lback(dlp);
	}
        curwp->w_line.l = llp;
        curwp->w_dot.l  = dlp;
	firstnonwhite(f,n);
        curwp->w_flag |= WFHARD|WFINS;
        return (TRUE);
}



/*
 * Set the named mark in the current window to the value of "." in the window.
 */
setnmmark(f,n)
{
	int c,i;

	if (clexec || isnamedcmd) {
		int stat;
		static char cbuf[2];
	        if ((stat=mlreply("Set mark: ", cbuf, 2)) != TRUE)
	                return stat;
		c = cbuf[0];
        } else {
		c = kbd_key();
        }
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
			return FALSE;
		}
		for (i = 0; i < 26; i++) {
			curbp->b_nmmarks[i] = nullmark;
		}
	}
		
        curbp->b_nmmarks[c-'a'] = DOT;
        mlwrite("[Mark %c set]",c);
        return TRUE;
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
	int c;
	int this1key;
	int useldmark;

	this1key = last1key;
	c = kbd_seq();
	useldmark = (last1key == this1key);  /* '' or `` */
	c = kcod2key(c);

	if (useldmark)
		c = '\'';

	return gonmmark(c);
}

gonmmark(c)
{
	register MARK *markp;
	MARK tmark;
	int found;

	if (!islower(c) && c != '\'') {
		TTbeep();
		mlwrite("[Invalid mark name]");
		return FALSE;
	}

	markp = NULL;

	if (c == '\'') { /* use the 'last dot' mark */
		markp = &(curwp->w_lastdot);
	} else if (curbp->b_nmmarks != NULL) {
		markp = &(curbp->b_nmmarks[c-'a']);
	}
		
	found = FALSE;
	/* if we have any named marks, and the one we want isn't null */
	if (markp != NULL && !samepoint((*markp), nullmark)) {
		register LINE *lp;
		for (lp = lforw(curbp->b_line.l);
				lp != curbp->b_line.l; lp = lforw(lp)) {
			if ((*markp).l == lp) {
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
	
	/* save current dot */
	tmark = DOT;

	/* move to the selected mark */
	DOT = *markp;

	/* reset last-dot-mark to old dot */
	curwp->w_lastdot = tmark;

        curwp->w_flag |= WFMOVE;
        return (TRUE);
}

/*
 * Set the mark in the current window to the value of "." in the window. No
 * errors are possible.
 */
setmark()
{
	MK = DOT;
        return (TRUE);
}

gomark(f,n)
{
	DOT = MK;
        curwp->w_flag |= WFMOVE;
        return (TRUE);
}

/* this odd routine puts us at the internal mark, plus an offset of lines */
/*  n == 1 leaves us at mark, n == 2 one line down, etc. */
/*  this is for the use of stuttered commands, and line oriented regions */
godotplus(f,n)
{
	int s;
	if (!f || n == 1)
	        return (TRUE);
	if (n < 1)
	        return (FALSE);
	s = forwline(TRUE,n-1);
	if (s && is_header_line(DOT, curbp))
		s = backline(FALSE,1);
	return s;
}

atmark()
{
	return samepoint(MK,DOT);
}

/*
 * Swap the values of "." and "mark" in the current window. This is pretty
 * easy, bacause all of the hard work gets done by the standard routine
 * that moves the mark about. The only possible error is "no mark".
 */
swapmark()
{
	MARK odot;

        if (samepoint(MK, nullmark)) {
                mlwrite("No mark in this window");
                return (FALSE);
        }
	odot = DOT;
	DOT = MK;
	MK = odot;
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
	dlp = curwp->w_line.l ;			/* get pointer to 1st line */
	while ( row-- && (dlp != curbp->b_line.l) ) dlp = lforw(dlp) ;
	curwp->w_dot.l = dlp ;			/* set dot line pointer */

	/* now move the dot over until near the requested column */
	curgoal = col ;		/* a global for this ?? */
	curwp->w_dot.o = getgoal(dlp) ;
	curwp->w_flag |= WFMOVE;
	return (TRUE);
}
#endif
