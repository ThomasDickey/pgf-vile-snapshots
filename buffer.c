/*
 * Buffer management.
 * Some of the functions are internal,
 * and some are actually attached to user
 * keys. Like everyone else, they set hints
 * for the display system.
 *
 * $Log: buffer.c,v $
 * Revision 1.32  1992/03/05 09:19:55  pgf
 * changed some mlwrite() to mlforce(), due to new terse support
 *
 * Revision 1.31  1992/01/05  00:06:13  pgf
 * split mlwrite into mlwrite/mlprompt/mlforce to make errors visible more
 * often.  also normalized message appearance somewhat.
 *
 * Revision 1.30  1992/01/03  23:33:50  pgf
 * use new ch_fname() routine to manipulate filenames
 *
 * Revision 1.29  1991/11/12  23:50:10  pgf
 * no longer allocate text for the header line -- the scanner doesn't
 * need it
 *
 * Revision 1.28  1991/11/12  23:43:23  pgf
 * pass stringend param to regexp, for end of string
 *
 * Revision 1.27  1991/11/08  13:08:44  pgf
 * ifdefed unused func
 *
 * Revision 1.26  1991/11/03  17:48:11  pgf
 * pass -1 to regexec to indicate no srchlim specified -- 0 was ambiguous
 *
 * Revision 1.25  1991/11/01  14:38:00  pgf
 * saber cleanup
 *
 * Revision 1.24  1991/10/28  14:21:05  pgf
 * renamed curtabstopval to curtabval, initialized new b_acount field
 * in BUFFER struct
 *
 * Revision 1.23  1991/10/28  01:01:06  pgf
 * added start offset and end offset to regexec calls
 *
 * Revision 1.22  1991/10/26  00:10:35  pgf
 * use regexec for c suffixes
 *
 * Revision 1.21  1991/10/23  14:19:53  pgf
 * allocate some text for a buffer's header line, and put a -1 there as the
 * first and only character.  this helps the search engine.
 *
 * Revision 1.20  1991/10/22  14:08:23  pgf
 * took out old ifdef BEFORE code
 *
 * Revision 1.19  1991/10/10  12:33:33  pgf
 * changes to support "block malloc" of line text -- now for most files
 * there is are two mallocs and a single read, no copies.  previously there
 * were two mallocs per line, and two copies (stdio's and ours).  This change
 * implies that lines and line text should not move between buffers, without
 * checking that the text and line struct do not "belong" to the buffer.
 *
 * Revision 1.18  1991/09/30  01:47:24  pgf
 * don't allow recursive zotbuf() via delwp()
 *
 * Revision 1.17  1991/09/26  13:17:11  pgf
 * can now kill a displayed buffer, and
 * we initialize window values at creation time
 *
 * Revision 1.16  1991/09/20  13:11:53  pgf
 * protect against null curtabstopval after make_current() returns
 *
 * Revision 1.15  1991/09/10  12:30:45  pgf
 * initialize b_wline when creating buffer
 *
 * Revision 1.14  1991/09/10  01:52:05  pgf
 * fixed bug caused by re-tabbing
 *
 * Revision 1.13  1991/09/10  00:50:23  pgf
 * re-tab, and elim. unused local in swbuffer
 *
 * Revision 1.12  1991/08/16  11:03:50	pgf
 * since we now switch to popped up buffers, the numbering in the buffer
 * list was off by one -- now we number from 1
 *
 * Revision 1.11  1991/08/12  09:25:10	pgf
 * now store w_line in w_traits while buffer is offscreen, so reframe
 * isn't always necessary.  don't force reframe on redisplay.
 *
 * Revision 1.10  1991/08/08  13:15:25	pgf
 * removed some ifdef BEFORE
 *
 * Revision 1.9  1991/08/07  12:35:07  pgf
 * added RCS log messages
 *
 * revision 1.8
 * date: 1991/08/06 15:11:31;
 * global/local values
 * and printf/list stuff
 * 
 * revision 1.7
 * date: 1991/06/25 19:52:06;
 * massive data structure restructure
 * 
 * revision 1.6
 * date: 1991/06/16 17:34:23;
 * added support for local values of tabstops and fillcol
 * 
 * revision 1.5
 * date: 1991/04/08 15:45:31;
 * fixed readin() arg count
 * 
 * revision 1.4
 * date: 1991/04/04 09:28:02;
 * line text is now separate from LINE struct
 * 
 * revision 1.3
 * date: 1991/02/21 09:15:35;
 * added horizontal scroll support
 * 
 * revision 1.2
 * date: 1990/12/06 19:47:56;
 * fixed list-buffer window re-use problem, and startup core dumps
 * if popupbuff was called in startup file
 * 
 * revision 1.1
 * date: 1990/09/21 10:24:46;
 * initial vile RCS revision
 */
#include	<stdio.h>
#include	"estruct.h"
#include	"edef.h"


/* c is either first character of a filename, or an index into buffer list */
char *
hist_lookup(c)
int c;
{
	register BUFFER *bp;
	static char buf[NBUFN];
	buf[0] = '\0';

	if (curbp != bheadp)
		mlforce("BUG: hist_lookup: curbp != bheadp");
	        
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
		mlforce("BUG: hist_show: curbp != bheadp");
        
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
		mlforce(line,1,2,3,4,5,6,7,8,9,10);
		return TRUE;
	} else {
		return FALSE;
	}
}

histbuff(f,n)
int f,n;
{
	register int thiscmd, c;
	register BUFFER *bp;
	char *bufn;

	if (curbp->b_bufp == NULL) {
		TTbeep();
		mlforce("[No other buffers.]");
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
		mlforce("[No such buffer.]");
		return FALSE;
	}
	/* first assume its a buffer name, then a file name */
	if ((bp=bfind(bufn, NO_CREAT, 0)) == NULL)
		return getfile(bufn,TRUE);
	else {
		return swbuffer(bp);
	}
}

/* switch back to the most recent buffer */
/* ARGSUSED */
altbuff(f,n)
int f,n;
{
	return histbuff(TRUE,1);
}

/*
 * Attach a buffer to a window. The
 * values of dot and mark come from the buffer
 * if the use count is 0. Otherwise, they come
 * from some other window.
 */
/* ARGSUSED */
usebuffer(f, n)
int f,n;
{
	register BUFFER *bp;
	register int	s;
	char		bufn[NBUFN];

	bufn[0] = 0;
	if ((s=mlreply("Use buffer: ", bufn, NBUFN)) != TRUE)
		return s;
	if ((bp=bfind(bufn, OK_CREAT, 0)) == NULL)
		return FALSE;
	return swbuffer(bp);
}

/* ARGSUSED */
nextbuffer(f, n)	/* switch to the next buffer in the buffer list */
int f, n;	/* default flag, numeric argument */
{
	register BUFFER *bp;	/* eligable buffer to switch to*/
	register BUFFER *stopatbp;	/* eligable buffer to switch to*/
        
	stopatbp = NULL;
	while (stopatbp != bheadp) {
		/* get the last buffer in the list */
		bp = bheadp;
		while(bp->b_bufp != stopatbp)
			bp = bp->b_bufp;
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

	if (!bp) {
		mlforce("BUG:  swbuffer passed null bp");
		return FALSE;
	}

	if (curbp == bp)  /* no switching to be done */
		return TRUE;

	if (curbp) {
		/* if we'll have to take over this window, and it's the last */
		if (bp->b_nwnd == 0 && --curbp->b_nwnd == 0) {
			undispbuff(curbp,curwp);
		}
	}
	make_current(bp);	/* sets curbp */

	/* get it already on the screen if possible */
	if (bp->b_nwnd > 0)  { /* then it's on the screen somewhere */
		register WINDOW *wp;
		wp = wheadp;
		while (wp->w_bufp != bp && wp->w_wndp != NULL)
			wp = wp->w_wndp;
		if (!wp)
			mlforce("BUG: swbuffer: wp still NULL");
		curwp = wp;
		upmode();
		return TRUE;
	}
        
	/* oh well, suck it into this window */
	curwp->w_bufp  = bp;
	curwp->w_flag |= WFMODE|WFHARD; /* Quite nasty. 	*/
	if (bp->b_nwnd++ == 0) {		/* First use.		*/
		curwp->w_traits = bp->b_wtraits;
	}
	if (bp->b_active != TRUE) {		/* buffer not active yet*/
		/* read it in and activate it */
		(void)readin(bp->b_fname, TRUE, bp, TRUE);
		curwp->w_dot.l = lforw(bp->b_line.l);
		curwp->w_dot.o = 0;
		bp->b_active = TRUE;
	}
#if	NeWS
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
	if ( (bp->b_flag & BFSCRTCH) || 
		( !(bp->b_flag & BFCHG) && is_empty_buf(bp)) ) {
		(void)zotbuf(bp);
	} else {  /* otherwise just adjust it off the screen */
		bp->b_wtraits  = wp->w_traits;
	}
}

/* bring nbp to the top of the list, where curbp _always_ lives */
make_current(nbp)
BUFFER *nbp;
{
	register BUFFER *bp;
        
	if (nbp == bheadp) {	/* already at the head */
		curbp = bheadp;
		curtabval = tabstop_val(curbp);
		return;
	}
	        
	/* remove nbp from the list */
	bp = bheadp; while(bp->b_bufp != nbp)
		bp = bp->b_bufp;
	bp->b_bufp = nbp->b_bufp;
        
	/* put it at the head */
	nbp->b_bufp = bheadp;
        
	bheadp = nbp;
	curbp = nbp;

	curtabval = tabstop_val(curbp);
}

tabstop_val(bp)
register BUFFER *bp;
{
	if (is_local_b_val(bp,MDCMOD))
		return b_val(bp,
			b_val(bp, MDCMOD) ? VAL_C_TAB : VAL_TAB);
	else
		return b_val(bp,
			(b_val(bp, MDCMOD) && has_C_suffix(bp))
					 ? VAL_C_TAB : VAL_TAB);
}

has_C_suffix(bp)
register BUFFER *bp;
{
	int s;
	ignorecase = FALSE;
	s =  regexec(b_val_rexp(bp,VAL_CSUFFIXES)->reg,
			bp->b_fname, NULL, 0, -1);
	return s;
}

/*
 * Dispose of a buffer, by name.
 * Ask for the name. Look it up (don't get too
 * upset if it isn't there at all!). Get quite upset
 * if the buffer is being displayed. Clear the buffer (ask
 * if the buffer has been changed). Then free the header
 * line and the buffer header.
 */
/* ARGSUSED */
killbuffer(f, n)
int f,n;
{
	register BUFFER *bp;
	register int	s;
	char bufn[NBUFN];
	register BUFFER *bp1;

	bufn[0] = 0;
	if ((s=mlreply("Kill buffer: ", bufn, NBUFN)) != TRUE)
		return(s);
	if ((bp=bfind(bufn, NO_CREAT, 0)) == NULL) { /* Easy if unknown.     */
		mlforce("[No such buffer]");
		return FALSE;
	}
	if(bp->b_flag & BFINVS) 	/* Deal with special buffers	*/
			return (TRUE);		/* by doing nothing.	*/
	if (curbp == bp) {
		if (histbuff(TRUE,1) != TRUE) {
			mlforce("[Can't kill that buffer]");
			return FALSE;
		}
	}
	if (bp->b_nwnd > 0)  { /* then it's on the screen somewhere */
		register WINDOW *wp;
		for (wp = wheadp; wp != NULL; wp = wp->w_wndp) {
			if (wp->w_bufp == bp) {
				delwp(wp);
			}
		}
		for (bp1 = bheadp; bp1; bp1 = bp1->b_bufp) {
			if (bp1 == bp)
				break;
		}
		if (bp1 == NULL)  /* delwp must have zotted us */
			return TRUE;
	}
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
	register int	s;
	register int	didswitch = FALSE;

#define no_del 
#ifdef no_del
	if (bp->b_nwnd != 0) {			/* Error if on screen.	*/
		mlforce("[Buffer is being displayed]");
		return (FALSE);
	}
#else
	if (curbp == bp) {
		didswitch = TRUE;
		if (histbuff(TRUE,1) != TRUE) {
			mlforce("[Can't kill that buffer]");
			return FALSE;
		}
	}
	if (bp->b_nwnd > 0)  { /* then it's on the screen somewhere */
		register WINDOW *wp;
		for (wp = wheadp; wp != NULL; wp = wp->w_wndp) {
			if (wp->w_bufp == bp) {
				delwp(wp);
			}
		}
		for (bp1 = bheadp; bp1; bp1 = bp1->b_bufp) {
			if (bp1 == bp)
				break;
		}
		if (bp1 == NULL)  /* delwp must have zotted us */
			return TRUE;
	}

		
#endif
	/* Blow text away.	*/
	if ((s=bclear(bp)) != TRUE) {
		/* the user must have answered no */
		if (didswitch)
			swbuffer(bp);
		return (s);
	}
	        
	lfree(bp->b_line.l,bp);	 /* Release header line. */
	bp1 = NULL;				/* Find the header.	*/
	bp2 = bheadp;
	while (bp2 != bp) {
		bp1 = bp2;
		bp2 = bp2->b_bufp;
	}
	bp2 = bp2->b_bufp;			/* Next one in chain.	*/
	if (bp1 == NULL)			/* Unlink it.		*/
		bheadp = bp2;
	else
		bp1->b_bufp = bp2;
	free((char *) bp);			/* Release buffer block */
	return (TRUE);
}

/* ARGSUSED */
namebuffer(f,n) 	/*	Rename the current buffer	*/
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
	} else if (bp->b_nwnd == 0) {		   /* Not on screen yet.   */
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
			wp->w_line.l = lforw(bp->b_line.l);
			wp->w_dot.l  = lforw(bp->b_line.l);
			wp->w_dot.o  = 0;
#ifdef WINMARK
			wp->w_mark = nullmark;
#endif
			wp->w_lastdot = nullmark;
			wp->w_values = global_w_values;
			wp->w_flag |= WFMODE|WFHARD;
			        
		}
		wp = wp->w_wndp;
	}
	swbuffer(bp);
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

#define BUFFER_LIST_NAME "[Buffer List]"

togglelistbuffers(f, n)
int f,n;
{
	register WINDOW *wp;
	register BUFFER *bp;
	int s;

	/* if it doesn't exist, create it */
	if ((bp = bfind(BUFFER_LIST_NAME, NO_CREAT, BFSCRTCH)) == NULL)
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

/* ARGSUSED */
listbuffers(f, n)
int f,n;
{
	int makebufflist();
	return liststuff(BUFFER_LIST_NAME, makebufflist, f, NULL);
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
/* ARGSUSED */
makebufflist(iflag,dummy)
int iflag;	/* list hidden buffer flag */
char *dummy;
{
	register BUFFER *bp;
	int nbuf = 1;	/* no. of buffers */
	long nbytes;		/* # of bytes in current buffer */
	int footnote = FALSE;

	bprintf("     %7s %*s %s\n", "Size",NBUFN,"Buffer name","Contents");
	bprintf("     %7p %*p %30p\n", '-',NBUFN,'-','-');
	        
	bp = bheadp;				/* For all buffers	*/

	/* output the list of buffers */
	while (bp != NULL) {
		/* skip invisible buffers and ourself if iflag is false */
		if (((bp->b_flag&(BFINVS|BFSCRTCH)) != 0) && (iflag != TRUE)) {
			bp = bp->b_bufp;
			continue;
		}
		/* output status of ACTIVE flag (has the file been read in? */
		if (bp->b_active == TRUE) {   /* if activated	    */
			if ((bp->b_flag&BFCHG) != 0) {	  /* if changed     */
				bputc('m');
				footnote = TRUE;
			} else {
				bputc(' ');
			}
		} else {
			bputc('u');
			footnote = TRUE;
		}

		bprintf(" %2d ",nbuf++);

		{	/* Count bytes in buf.	*/
			register LINE	*lp;
			nbytes = 0L;
			lp = lforw(bp->b_line.l);
			while (lp != bp->b_line.l) {
				nbytes += (long)llength(lp)+1L;
				lp = lforw(lp);
			}
		}
		bprintf("%7ld %*s %s\n",nbytes, NBUFN, 
						bp->b_bname, bp->b_fname);
		bp = bp->b_bufp;
	}

	if (footnote == TRUE)
		bprintf("('m' means modified, 'u' means unread)\n");
}

#ifdef NEEDED
ltoa(buf, width, num)
char   buf[];
int    width;
long   num;
{
	buf[width] = 0; 			/* End of string.	*/
	while (num >= 10) {			/* Conditional digits.	*/
		buf[--width] = (int)(num%10L) + '0';
		num /= 10L;
	}
	buf[--width] = (int)num + '0';		/* Always 1 digit.	*/
	while (width != 0)			/* Pad with blanks.	*/
		buf[--width] = ' ';
}
#endif

/*
 * The argument "text" points to
 * a string. Append this line to the
 * buffer. Handcraft the EOL
 * on the end. Return TRUE if it worked and
 * FALSE if you ran out of room.
 */
addline(bp,text,len)
register BUFFER *bp;
char	*text;
int len;
{
	register LINE	*lp;
	register int	i;
	register int	ntext;

	ntext = (len < 0) ? strlen(text) : len;
	if ((lp=lalloc(ntext,bp)) == NULL)
		return (FALSE);
	for (i=0; i<ntext; ++i)
		lputc(lp, i, text[i]);
	bp->b_line.l->l_bp->l_fp = lp;	     /* Hook onto the end    */
	lp->l_bp = bp->b_line.l->l_bp;
	bp->b_line.l->l_bp = lp;
	lp->l_fp = bp->b_line.l;
	if (sameline(bp->b_dot, bp->b_line))  /* If "." is at the end */
		bp->b_dot.l = lp;	     /* move it to new line  */
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
BUFFER	*
bfind(bname, cflag, bflag)
int cflag, bflag;
char   *bname;
{
	register BUFFER *bp;
	register LINE	*lp;
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

	/* these affect lalloc(), below */
	bp->b_LINEs = NULL;
	bp->b_LINEs_end = NULL;
	bp->b_freeLINEs = NULL;
	bp->b_ltext = NULL;
	bp->b_ltext_end = NULL;

	if ((lp=lalloc(0,bp)) == NULL) {
		free((char *) bp);
		return (NULL);
	}

	/* and set up the other buffer fields */
	bp->b_active = FALSE;
	bp->b_dot.l  = lp;
	bp->b_dot.o  = 0;
#if WINMARK
	bp->b_mark = nullmark;
#endif
	bp->b_lastdot = nullmark;
	bp->b_nmmarks = NULL;
	bp->b_flag  = bflag;
	bp->b_values = global_b_values;
	bp->b_wtraits.w_vals = global_w_values;
	bp->b_nwnd  = 0;
	bp->b_wline.l = lp;
	bp->b_wline.o = 0;  /* unused */
	bp->b_line.l = lp;
	bp->b_line.o = 0;
	bp->b_acount = b_val(bp, VAL_ASAVECNT);
	bp->b_fname = NULL;
	ch_fname(bp, "");
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
	register LINE	*lp;

	if ((bp->b_flag&(BFINVS|BFSCRTCH)) == 0 /* Not invisible or scratch */
			&& (bp->b_flag&BFCHG) != 0 ) {	    /* Something changed    */
		char ques[50];
		strcpy(ques,"Discard changes to ");
		strcat(ques,bp->b_bname);
		if (mlyesno(ques) != TRUE)
			return FALSE;
	}
	bp->b_flag  &= ~BFCHG;			/* Not changed		*/
	freeundostacks(bp);	/* do this before removing lines */
	while ((lp=lforw(bp->b_line.l)) != bp->b_line.l) {
		lremove(bp,lp);
		lfree(lp,bp);
	}
	if (bp->b_ltext) {
		free(bp->b_ltext);
		bp->b_ltext = NULL;
		bp->b_ltext_end = NULL;
	}
	if (bp->b_LINEs) {
		free(bp->b_LINEs);
		bp->b_LINEs = NULL;
		bp->b_LINEs_end = NULL;
	}
	bp->b_freeLINEs = NULL;
	bp->b_dot  = bp->b_line;		/* Fix "."		*/
#if WINMARK
	bp->b_mark = nullmark;			/* Invalidate "mark"	*/
#endif
	bp->b_lastdot = nullmark;		/* Invalidate "mark"	*/
	if (bp->b_nmmarks != NULL) { /* free the named marks */
		free((char *)(bp->b_nmmarks));
		bp->b_nmmarks = NULL;
	}
	return (TRUE);
}

/* ARGSUSED */
unmark(f, n)	/* unmark the current buffers change flag */
int f, n;	/* unused command arguments */
{
	curbp->b_flag &= ~BFCHG;
	curwp->w_flag |= WFMODE;
	return(TRUE);
}
