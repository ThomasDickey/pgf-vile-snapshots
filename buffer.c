/*
 * Buffer management.
 * Some of the functions are internal,
 * and some are actually attached to user
 * keys. Like everyone else, they set hints
 * for the display system.
 */
#include        <stdio.h>
#include	"estruct.h"
#include        "edef.h"

static lastlookup = -1;

/* c is either first character of a filename, or an index into buffer list */
char *
hist_lookup(c)
{
	register BUFFER *bp;
	static char buf[NBUFN];
	buf[0] = '\0';

	if (curbp != bheadp)
		mlwrite("BUG: hist_lookup: curbp != bheadp");
		
	bp = curbp->b_bufp; /* always skip the current */
	while (bp != NULL) {
		if (!(bp->b_flag & (BFINVS|BFSCRTCH)) && (--c == 0))
			break;
		bp = bp->b_bufp;
	}
	if (bp)
		return bp->b_bname;
	return NULL;
}

hist_show()
{
	int i;
	char line[NLINE];
	BUFFER *bp;
	
	if (curbp != bheadp)
		mlwrite("BUG: hist_show: curbp != bheadp");
	
	strcpy(line,"");
	for (i = 0, bp = curbp->b_bufp; i < 10 && bp != NULL; bp = bp->b_bufp) {
		if (!(bp->b_flag & (BFINVS|BFSCRTCH))) {
			strcat(line, "  %d");
			if (bp->b_flag & BFCHG)
				strcat(line, "* ");
			else
				strcat(line, " ");
			strcat(line, bp->b_bname);
			i++;
		}
	}
	if (strcmp(line,"")) {
		mlwrite(line,1,2,3,4,5,6,7,8,9,10);
		return TRUE;
	} else {
		return FALSE;
	}
}

histbuff(f,n)
{
	register int thiscmd, c;
        register BUFFER *bp;
	char *bufn;

	if (curbp->b_bufp == NULL) {
		TTbeep();
		mlwrite("No other buffers.");
		return(FALSE);
	}

	if (f == FALSE) {
		hist_show();
		thiscmd = lastcmd;
		c = kbd_seq();
		mlerase();
		if (c == thiscmd) {
			c = 1;
		} else if (isdigit(c)) {
			c = c - '0';
		} else {
			tungetc(c);
			return FALSE;
		}
	} else {
		c = n;
	}
	
	bufn = hist_lookup(c);
	if (bufn == NULL) {
		TTbeep();
		mlwrite("No such buffer.");
		return FALSE;
	}
	/* first assume its a buffer name, then a file name */
        if ((bp=bfind(bufn, NO_CREAT, 0)) == NULL)
		return getfile(bufn,TRUE);
	else {
		return swbuffer(bp);
	}
}

altbuff(f,n)
{
	return(histbuff(TRUE,1));
}

/*
 * Attach a buffer to a window. The
 * values of dot and mark come from the buffer
 * if the use count is 0. Otherwise, they come
 * from some other window.
 */
usebuffer(f, n)
{
        register BUFFER *bp;
        register int    s;
        char            bufn[NBUFN];

	bufn[0] = 0;
        if ((s=mlreply("Use buffer: ", bufn, NBUFN)) != TRUE)
                return s;
        if ((bp=bfind(bufn, OK_CREAT, 0)) == NULL)
                return FALSE;
	return swbuffer(bp);
}

nextbuffer(f, n)	/* switch to the next buffer in the buffer list */
int f, n;	/* default flag, numeric argument */
{
	register BUFFER *bp;	/* eligable buffer to switch to*/
	register BUFFER *stopatbp;	/* eligable buffer to switch to*/
	
	stopatbp = NULL;
	while (stopatbp != bheadp) {
		/* get the last buffer in the list */
		for (bp = bheadp; bp->b_bufp != stopatbp; bp = bp->b_bufp)
			;
		/* if that one's invisible, back up and try again */
		if (bp->b_flag & BFINVS)
			stopatbp = bp;
		else
			return swbuffer(bp);
	}
	/* we're back to the top -- they were all invisible */
	return swbuffer(stopatbp);
	
}


swbuffer(bp)	/* make buffer BP current */
register BUFFER *bp;
{
        register WINDOW *wp;

	if (curbp == bp)  /* no switching to be done */
		return TRUE;

        if (curbp) {
        	/* if we'll have to take over this window, and it's the last */
		if (bp->b_nwnd == 0 && --curbp->b_nwnd == 0) {
			undispbuff(curbp,curwp);
		}
        }
	make_current(bp);
	
	/* get it already on the screen if possible */
        if (bp->b_nwnd > 0)  { /* then it's on the screen somewhere */
        	register WINDOW *wp;
        	int nw;
		for (wp = wheadp, nw= 1;
			 wp->w_bufp != bp && wp->w_wndp != NULL;
					 nw++, wp = wp->w_wndp)
			;
		if (!wp)
			mlwrite("BUG: swbuffer: wp still NULL");
		curwp = wp;
		upmode();
		return TRUE;
	}
	
	/* oh well, suck it into this window */
        curwp->w_bufp  = bp;
        curwp->w_linep = bp->b_linep;           /* For macros, ignored. */
        curwp->w_flag |= WFMODE|WFFORCE|WFHARD; /* Quite nasty.         */
        if (bp->b_nwnd++ == 0) {                /* First use.           */
                curwp->w_dotp  = bp->b_dotp;
                curwp->w_doto  = bp->b_doto;
                curwp->w_mkp = bp->b_markp;
                curwp->w_mko = bp->b_marko;
                curwp->w_ldmkp = bp->b_ldmkp;
                curwp->w_ldmko = bp->b_ldmko;
                curwp->w_sideways = bp->b_sideways;
        }
	if (bp->b_active != TRUE) {		/* buffer not active yet*/
		/* read it in and activate it */
		(void)readin(bp->b_fname, TRUE, bp, TRUE);
		curwp->w_dotp = lforw(bp->b_linep);
		curwp->w_doto = 0;
		bp->b_active = TRUE;
	}
#if     NeWS
        newsreportmodes() ;
#endif
        return TRUE;
}


undispbuff(bp,wp)
register BUFFER *bp;
register WINDOW *wp;
{
	/* get rid of it completely if it's a scratch buffer,
		or it's empty and unmodified */
	if ( (bp->b_flag & BFSCRTCH) || ( !(bp->b_flag & BFCHG) && 
				lforw(bp->b_linep) == bp->b_linep ) ) {
		(void)zotbuf(bp);
	} else {  /* otherwise just adjust it off the screen */
		bp->b_dotp  = wp->w_dotp;
		bp->b_doto  = wp->w_doto;
		bp->b_markp = wp->w_mkp;
		bp->b_marko = wp->w_mko;
		bp->b_ldmkp = wp->w_ldmkp;
		bp->b_ldmko = wp->w_ldmko;
		bp->b_sideways = wp->w_sideways;
	}
}

/* bring nbp to the top of the list, where curbp _always_ lives */
make_current(nbp)
BUFFER *nbp;
{
	register BUFFER *bp;
	
	if (nbp == bheadp) {	/* already at the head */
		curbp = bheadp;
		return;
	}
		
	/* remove nbp from the list */
	for (bp = bheadp; bp->b_bufp != nbp; bp = bp->b_bufp)
		;
	bp->b_bufp = nbp->b_bufp;
	
	/* put it at the head */
	nbp->b_bufp = bheadp;
	
	bheadp = nbp;
	curbp = nbp;
}

/*
 * Dispose of a buffer, by name.
 * Ask for the name. Look it up (don't get too
 * upset if it isn't there at all!). Get quite upset
 * if the buffer is being displayed. Clear the buffer (ask
 * if the buffer has been changed). Then free the header
 * line and the buffer header.
 */
killbuffer(f, n)
{
	register BUFFER *bp;
        register int    s;
        char bufn[NBUFN];

	bufn[0] = 0;
        if ((s=mlreply("Kill buffer: ", bufn, NBUFN)) != TRUE)
                return(s);
        if ((bp=bfind(bufn, NO_CREAT, 0)) == NULL) { /* Easy if unknown.     */
        	mlwrite("No such buffer");
                return FALSE;
        }
	if(bp->b_flag & BFINVS)		/* Deal with special buffers	*/
			return (TRUE);		/* by doing nothing.	*/
	s = zotbuf(bp);
	if (s == TRUE)
                mlwrite("Buffer %s gone", bufn);
	return s;
}

zotbuf(bp)	/* kill the buffer pointed to by bp */
register BUFFER *bp;
{
        register BUFFER *bp1;
        register BUFFER *bp2;
        register int    s;

        if (bp->b_nwnd != 0) {                  /* Error if on screen.  */
                mlwrite("Buffer is being displayed");
                return (FALSE);
        }
        if ((s=bclear(bp)) != TRUE)             /* Blow text away.      */
                return (s);
                
        lfree(bp->b_linep);             /* Release header line. */
        bp1 = NULL;                             /* Find the header.     */
        bp2 = bheadp;
        while (bp2 != bp) {
                bp1 = bp2;
                bp2 = bp2->b_bufp;
        }
        bp2 = bp2->b_bufp;                      /* Next one in chain.   */
        if (bp1 == NULL)                        /* Unlink it.           */
                bheadp = bp2;
        else
                bp1->b_bufp = bp2;
        free((char *) bp);                      /* Release buffer block */
        return (TRUE);
}

namebuffer(f,n)		/*	Rename the current buffer	*/
int f, n;		/* default Flag & Numeric arg */
{
	register BUFFER *bp;	/* pointer to scan through all buffers */
	static char bufn[NBUFN];	/* buffer to hold buffer name */
	char *prompt = "New name for buffer: ";

	/* prompt for and get the new buffer name */
ask:	if (mlreply(prompt, bufn, NBUFN) != TRUE)
		return(FALSE);

	/* and check for duplicates */
	bp = bheadp;
	while (bp != NULL) {
		if (bp != curbp) {
			/* if the names the same */
			if (strcmp(bufn, bp->b_bname) == 0) {
				prompt = "That name's been used.  New name: ";
				goto ask;  /* try again */
			}
		}
		bp = bp->b_bufp;	/* onward */
	}

	strcpy(curbp->b_bname, bufn);	/* copy buffer name to structure */
	curwp->w_flag |= WFMODE;	/* make mode line replot */
	mlerase();
	return(TRUE);
}

/* create or find a window, and stick this buffer in it.  when 
	done, we own the window and the buffer, but they are _not_
	necessarily curwp and curbp */
popupbuff(bp)
BUFFER *bp;
{
        register WINDOW *wp;

	if (!curbp) {
		curbp = bp;  /* possibly at startup time */
		curwp->w_bufp = curbp;
                ++curbp->b_nwnd;
	} else if (bp->b_nwnd == 0) {              /* Not on screen yet.   */
                if ((wp = wpopup()) == NULL)
                        return FALSE;
                if (--wp->w_bufp->b_nwnd == 0)
                	undispbuff(wp->w_bufp,wp);
                wp->w_bufp  = bp;
                ++bp->b_nwnd;
        }
        wp = wheadp;
        while (wp != NULL) {
                if (wp->w_bufp == bp) {
                        wp->w_linep = lforw(bp->b_linep);
                        wp->w_dotp  = lforw(bp->b_linep);
                        wp->w_doto  = 0;
                        wp->w_mkp = NULL;
                        wp->w_mko = 0;
                        wp->w_ldmkp = NULL;
                        wp->w_ldmko = 0;
                        wp->w_sideways = 0;
                        wp->w_flag |= WFMODE|WFHARD;
				
                }
                wp = wp->w_wndp;
        }
        return TRUE;
}

/*
	List all of the active buffers.  First update the special
	buffer that holds the list.  Next make sure at least 1
	window is displaying the buffer list, splitting the screen
	if this is what it takes.  Lastly, repaint all of the
	windows that are displaying the list.
	A numeric argument forces it to list invisable buffers as
	well.
*/
togglelistbuffers(f, n)
{
        register WINDOW *wp;
        register BUFFER *bp;
	int s;

	/* if it doesn't exist, create it */
	if ((bp = bfind("[List]", NO_CREAT, BFSCRTCH)) == NULL)
		return listbuffers(f,n);

	/* if it does exist, delete the window, which in turn 
		will kill the BFSCRTCH buffer */
        wp = wheadp;
	s = TRUE;
        while (wp != NULL && s) {
                if (wp->w_bufp == bp) {
			s = delwp(wp);
                	break;
		}
                wp = wp->w_wndp;
	}
	return s;
}

listbuffers(f, n)
{
        register BUFFER *bp;
        register int    s;

	/* create the buffer list buffer   */
	bp = bfind("[List]", OK_CREAT, BFSCRTCH);
	if (bp == NULL)
		return FALSE;
	
        if ((s=bclear(bp)) != TRUE) /* clear old text (?) */
                return (s);
	bp->b_flag |= BFSCRTCH;
        s = makelist(f,bp);
	if (!s || popupbuff(bp) == FALSE) {
		mlwrite("[Sorry, can't list. ]");
		zotbuf(bp);
                return (FALSE);
        }
        sprintf(bp->b_fname, "       %s   %s",prognam,version);
        bp->b_flag &= ~BFCHG;               /* Don't complain!      */
        bp->b_active = TRUE;

        return TRUE;
}

/*
 * This routine rebuilds the
 * text in the buffer
 * that holds the buffer list. It is called
 * by the list buffers command. Return TRUE
 * if everything works. Return FALSE if there
 * is an error (if there is no memory). Iflag
 * indicates whether to list hidden buffers.
 */
/* returns no. of lines in list */
int
makelist(iflag,blistp)
int iflag;	/* list hidden buffer flag */
BUFFER *blistp;
{
        register char   *cp1;
        register char   *cp2;
        register int    c;
        register BUFFER *bp;
        register LINE   *lp;
        register int    s;
	register int	i;
        long nbytes;		/* # of bytes in current buffer */
	int nbuf = 0;	/* no. of buffers */
        int nlines = 0;  /* no. of lines in list */
        char b[10];
        char line[NFILEN+NBUFN+30];
        static char dashes[] =
			"-------------------------------------------------";
	int footnote = FALSE;

	sprintf(line,"    %-*s %7s %-*s %s",
			NUMMODES,"Modes","Size",NBUFN,"Buffer name","Contents");
        if(addline(blistp,line, -1) == FALSE)
                return (FALSE);
        nlines++;
                
	/* put some spaces into the separator line... */
	dashes[3] = dashes[NUMMODES+4] = dashes[NUMMODES+4+8] = 
		dashes[NUMMODES+4+8+NBUFN+1] = ' ';
        if (addline(blistp,dashes, -1) == FALSE)
                return (FALSE);
        nlines++;

        bp = bheadp;                            /* For all buffers      */

	/* output the list of buffers */
        while (bp != NULL) {
		/* skip invisible buffers and ourself if iflag is false */
                if (((bp->b_flag&(BFINVS|BFSCRTCH)) != 0) && (iflag != TRUE)) {
                        bp = bp->b_bufp;
                        continue;
                }


                cp1 = &line[0];                 /* Start at left edge   */
		/* output status of ACTIVE flag (has the file been read in? */
                if (bp->b_active == TRUE) {   /* if activated       */
	                if ((bp->b_flag&BFCHG) != 0) {    /* if changed     */
	                        *cp1++ = 'm';
				footnote = TRUE;
	                } else {
	                        *cp1++ = ' ';
			}
		} else {
                        *cp1++ = 'u';
			footnote = TRUE;
		}

		sprintf(cp1,"%2d ",nbuf++);

                cp1 = &line[strlen(line)];

		/* output the mode codes */
		for (i = 0; i < NUMMODES; i++) {
			if (bp->b_mode & (1 << i))
				*cp1++ = modecode[i];
			else
				*cp1++ = '.';
		}
                *cp1++ = ' ';                   /* Gap.                 */
                nbytes = 0L;                    /* Count bytes in buf.  */
                lp = lforw(bp->b_linep);
                while (lp != bp->b_linep) {
                        nbytes += (long)llength(lp)+1L;
                        lp = lforw(lp);
                }
		sprintf(cp1,"%7ld %-*s %s",nbytes,
			NBUFN, bp->b_bname, bp->b_fname);
                if (addline(blistp,line,-1) == FALSE)
                        return (FALSE);
	        nlines++;
                bp = bp->b_bufp;
        }

	if (footnote == TRUE) {
	        if (addline(blistp,
	        	"('m' means modified, 'u' means unread)",-1)
								 == FALSE) {
			return FALSE;
		}
	        nlines++;
	}

	/* build line to report global mode settings */
	sprintf(line,"    ");
	cp1 = &line[4];

	/* output the mode codes */
	for (i = 0; i < NUMMODES; i++)
		if (gmode & (1 << i))
			*cp1++ = modecode[i];
		else
			*cp1++ = '.';
	strcpy(cp1, "    are the global modes");
	if (addline(blistp,line,-1) == FALSE)
		return(FALSE);

	sprintf(line," tabstop = %d; fillcol = %d", TABVAL, fillcol);
	if (addline(blistp,line,-1) == FALSE)
		return(FALSE);

	sprintf(line," lazy filename matching is %s",
					(othmode & OTH_LAZY) ? "on":"off");
	if (addline(blistp,line,-1) == FALSE)
		return(FALSE);

	nlines++;

        return (nlines);
}

ltoa(buf, width, num)
char   buf[];
int    width;
long   num;
{
        buf[width] = 0;                         /* End of string.       */
        while (num >= 10) {                     /* Conditional digits.  */
                buf[--width] = (int)(num%10L) + '0';
                num /= 10L;
        }
        buf[--width] = (int)num + '0';          /* Always 1 digit.      */
        while (width != 0)                      /* Pad with blanks.     */
                buf[--width] = ' ';
}

/*
 * The argument "text" points to
 * a string. Append this line to the
 * buffer. Handcraft the EOL
 * on the end. Return TRUE if it worked and
 * FALSE if you ran out of room.
 */
addline(bp,text,len)
register BUFFER *bp;
char    *text;
int len;
{
        register LINE   *lp;
        register int    i;
        register int    ntext;

        ntext = (len < 0) ? strlen(text) : len;
        if ((lp=lalloc(ntext)) == NULL)
                return (FALSE);
        for (i=0; i<ntext; ++i)
                lputc(lp, i, text[i]);
        bp->b_linep->l_bp->l_fp = lp;       /* Hook onto the end    */
        lp->l_bp = bp->b_linep->l_bp;
        bp->b_linep->l_bp = lp;
        lp->l_fp = bp->b_linep;
        if (bp->b_dotp == bp->b_linep)  /* If "." is at the end */
                bp->b_dotp = lp;            /* move it to new line  */
        return (TRUE);
}

/*
 * Look through the list of
 * buffers. Return TRUE if there
 * are any changed buffers. Buffers
 * that hold magic internal stuff are
 * not considered; who cares if the
 * list of buffer names is hacked.
 * Return FALSE if no buffers
 * have been changed.
 */
anycb()
{
        register BUFFER *bp;
	register int cnt = 0;

        bp = bheadp;
        while (bp != NULL) {
                if ((bp->b_flag&BFINVS)==0 && (bp->b_flag&BFCHG)!=0)
			cnt++;
                bp = bp->b_bufp;
        }
        return (cnt);
}

/*
 * Find a buffer, by name. Return a pointer
 * to the BUFFER structure associated with it.
 * If the buffer is not found
 * and the "cflag" is OK_CREAT, create it. The "bflag" is
 * the settings for the flags in in buffer.
 */
BUFFER  *
bfind(bname, cflag, bflag)
char   *bname;
{
        register BUFFER *bp;
        register LINE   *lp;
	register BUFFER *lastb = NULL;	/* buffer to insert after */

        bp = bheadp;
        while (bp != NULL) {
                if (strncmp(bname, bp->b_bname, NBUFN) == 0)
                        return (bp);
                lastb = bp;
                bp = bp->b_bufp;
        }
	if (cflag == NO_CREAT)	/* don't create it */
		return NULL;
	
        if ((bp=(BUFFER *)malloc(sizeof(BUFFER))) == NULL)
                return (NULL);
        if ((lp=lalloc(0)) == NULL) {
                free((char *) bp);
                return (NULL);
        }

	/* and set up the other buffer fields */
	bp->b_active = FALSE;
        bp->b_dotp  = lp;
        bp->b_doto  = 0;
        bp->b_markp = NULL;
        bp->b_marko = 0;
        bp->b_ldmkp = NULL;
        bp->b_ldmko = 0;
        bp->b_nmmarks = NULL;
        bp->b_flag  = bflag;
	bp->b_mode  = gmode & ~(MDCMOD|MDDOS); /* handled in readin() */
        bp->b_nwnd  = 0;
        bp->b_sideways = 0;
        bp->b_linep = lp;
        strcpy(bp->b_fname, "");
        strcpy(bp->b_bname, bname);
#if	CRYPT
	bp->b_key[0] = 0;
#endif
	bp->b_udstks[0] = bp->b_udstks[1] = NULL;
	bp->b_udstkindx = 0;
        bp->b_ulinep = NULL;
        lp->l_fp = lp;
        lp->l_bp = lp;
        
	/* append at the end */
	if (lastb)
	        lastb->b_bufp = bp;
	else
		bheadp = bp;
        bp->b_bufp = NULL;

        return (bp);
}

/*
 * This routine blows away all of the text
 * in a buffer. If the buffer is marked as changed
 * then we ask if it is ok to blow it away; this is
 * to save the user the grief of losing text. The
 * window chain is nearly always wrong if this gets
 * called; the caller must arrange for the updates
 * that are required. Return TRUE if everything
 * looks good.
 */
bclear(bp)
register BUFFER *bp;
{
        register LINE   *lp;
        register int    s;

        if ((bp->b_flag&(BFINVS|BFSCRTCH)) == 0 /* Not invisible or scratch */
		        && (bp->b_flag&BFCHG) != 0 ) {      /* Something changed    */
		char ques[50];
		strcpy(ques,"Discard changes to ");
		strcat(ques,bp->b_bname);
	        if (mlyesno(ques) != TRUE)
	                return FALSE;
	}
        bp->b_flag  &= ~BFCHG;                  /* Not changed          */
	freeundostacks(bp);	/* do this before removing lines */
        while ((lp=lforw(bp->b_linep)) != bp->b_linep) {
                lremove(bp,lp);
                lfree(lp);
	}
        bp->b_dotp  = bp->b_linep;              /* Fix "."              */
        bp->b_doto  = 0;
        bp->b_markp = NULL;                     /* Invalidate "mark"    */
        bp->b_marko = 0;
        bp->b_ldmkp = NULL;                     /* Invalidate "mark"    */
        bp->b_ldmko = 0;
	if (bp->b_nmmarks != NULL) { /* free the named marks */
		free((char *)(bp->b_nmmarks));
		bp->b_nmmarks = NULL;
	}
        return (TRUE);
}

unmark(f, n)	/* unmark the current buffers change flag */
int f, n;	/* unused command arguments */
{
	curbp->b_flag &= ~BFCHG;
	curwp->w_flag |= WFMODE;
	return(TRUE);
}
