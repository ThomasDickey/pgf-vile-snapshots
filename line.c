/*
 * The functions in this file are a general set of line management utilities.
 * They are the only routines that touch the text. They also touch the buffer
 * and window structures, to make sure that the necessary updating gets done.
 * There are routines in this file that handle the kill register too. It isn't
 * here for any good reason.
 *
 * Note that this code only updates the dot and mark values in the window list.
 * Since all the code acts on the current window, the buffer that we are
 * editing must be being displayed, which means that "b_nwnd" is non zero,
 * which means that the dot and mark values in the buffer headers are nonsense.
 */

#include	<stdio.h>
#include	"estruct.h"
#include	"edef.h"

/*
 * This routine allocates a block of memory large enough to hold a LINE
 * containing "used" characters. The block is always rounded up a bit. Return
 * a pointer to the new block, or NULL if there isn't any memory left. Print a
 * message in the message line if no space.
 */
LINE *
lalloc(used)
register int	used;
{
	register LINE	*lp;
	register int	size;

	/* lalloc(-1) is used by undo for placeholders */
	if (used < 0)  {
		size = 0;
	} else {
		size = (used+NBLOCK-1) & ~(NBLOCK-1);
		if (size == 0)			/* Assume that an empty */
			size = NBLOCK;		/* line is for type-in. */
	}
	/* malloc 4 less, because struct LINE is 4 too big */
	if ((lp = (LINE *) malloc(sizeof(LINE) + size - 4)) == NULL) {
		mlwrite("[OUT OF MEMORY]");
		return (NULL);
	}
	lp->l_size = size;
	lp->l_used = used;
	lsetclear(lp);
	lp->l_nxtundo = NULL;
	return (lp);
}

/*
 * Delete line "lp". Fix all of the links that might point at it (they are
 * moved to offset 0 of the next line. Unlink the line from whatever buffer it
 * might be in. The buffers are updated too; the magic
 * conditions described in the above comments don't hold here.
 * Memory is not released, so line can be saved in undo stacks.
 */
lremove(bp,lp)
register BUFFER *bp;
register LINE	*lp;
{
	register WINDOW *wp;

	wp = wheadp;
	while (wp != NULL) {
		if (wp->w_linep == lp)
			wp->w_linep = lp->l_fp;
		if (wp->w_dotp	== lp) {
			wp->w_dotp  = lp->l_fp;
			wp->w_doto  = 0;
		}
		if (wp->w_mkp == lp) {
			wp->w_mkp = lp->l_fp;
			wp->w_mko = 0;
		}
#if 0
		if (wp->w_ldmkp == lp) {
			wp->w_ldmkp = lp->l_fp;
			wp->w_ldmko = 0;
		}
#endif
		wp = wp->w_wndp;
	}
	if (bp->b_nwnd == 0) {
		if (bp->b_dotp	== lp) {
			bp->b_dotp = lp->l_fp;
			bp->b_doto = 0;
		}
		if (bp->b_markp == lp) {
			bp->b_markp = lp->l_fp;
			bp->b_marko = 0;
		}
#if 0
		if (bp->b_ldmkp == lp) {
			bp->b_ldmkp = lp->l_fp;
			bp->b_ldmko = 0;
		}
#endif
	}
#if 0
	if (bp->b_nmmarks != NULL) { /* fix the named marks */
		int i;
		struct MARK *mp;
		for (i = 0; i < 26; i++) {
			mp = &(bp->b_nmmarks[i]);
			if (mp->markp == lp) {
				mp->markp = lp->l_fp;
				mp->marko = 0;
			}
		}
	}
#endif
	lp->l_bp->l_fp = lp->l_fp;
	lp->l_fp->l_bp = lp->l_bp;
}

/*
 * This routine gets called when a character is changed in place in the current
 * buffer. It updates all of the required flags in the buffer and window
 * system. The flag used is passed as an argument; if the buffer is being
 * displayed in more than 1 window we change EDIT to HARD. Set MODE if the
 * mode line needs to be updated (the "*" has to be set).
 */
lchange(flag)
register int	flag;
{
	register WINDOW *wp;

	if (curbp->b_nwnd != 1) { 		/* Ensure hard. 	*/
		flag |= WFHARD;
	}
	if ((curbp->b_flag&BFCHG) == 0) {	/* First change, so	*/
		flag |= WFMODE; 		/* update mode lines.	*/
		curbp->b_flag |= BFCHG;
	}
	wp = wheadp;
	while (wp != NULL) {
		if (wp->w_bufp == curbp)
			wp->w_flag |= flag;
		wp = wp->w_wndp;
	}
}

insspace(f, n)	/* insert spaces forward into text */
int f, n;	/* default flag and numeric argument */
{
	linsert(n, ' ');
	backchar(f, n);
}

/*
 * Insert "n" copies of the character "c" at the current location of dot. In
 * the easy case all that happens is the text is stored in the line. In the
 * hard case, the line has to be reallocated. When the window list is updated,
 * take special care; I screwed it up once. You always update dot in the
 * current window. You update mark, and a dot in another window, if it is
 * greater than the place where you did the insert. Return TRUE if all is
 * well, and FALSE on errors.
 */
linsert(n, c)
{
	register char	*cp1;
	register char	*cp2;
	register LINE	*lp1;
	register LINE	*lp2;
	register LINE	*lp3;
	register int	doto;
	register int	i;
	register WINDOW *wp;

	lchange(WFEDIT);
	lp1 = curwp->w_dotp;			/* Current line 	*/
	if (lp1 == curbp->b_linep) {		/* At the end: special	*/
		if (curwp->w_doto != 0) {
			mlwrite("bug: linsert");
			return (FALSE);
		}
		if ((lp2=lalloc(n)) == NULL)	/* Allocate new line	*/
			return (FALSE);
		copy_for_undo(lp1->l_bp); /* don't want preundodot to point
					   *	at a new line if this is the
					   *	first change */
		lp3 = lp1->l_bp;		/* Previous line	*/
		lp3->l_fp = lp2;		/* Link in		*/
		lp2->l_fp = lp1;
		lp1->l_bp = lp2;
		lp2->l_bp = lp3;
		for (i=0; i<n; ++i)
			lp2->l_text[i] = c;
		curwp->w_dotp = lp2;
		curwp->w_doto = n;
		tag_for_undo(lp2);
		return (TRUE);
	}
	doto = curwp->w_doto;			/* Save for later.	*/
	if (lp1->l_used+n > lp1->l_size) {	/* Hard: reallocate	*/
		/* for the sake of undo(), handle this as an insert and
			a delete */
		/* first, create the new image */
		if ((lp2=lalloc(lp1->l_used+n)) == NULL)
			return (FALSE);
		cp1 = &lp1->l_text[0];
		cp2 = &lp2->l_text[0];
		while (cp1 != &lp1->l_text[doto])
			*cp2++ = *cp1++;
		cp2 += n;
		while (cp1 != &lp1->l_text[lp1->l_used])
			*cp2++ = *cp1++;
		/* then insert it into the file */
		lp2->l_bp = lp1->l_bp;
		lp1->l_bp = lp2;
		lp2->l_bp->l_fp = lp2;
		lp2->l_fp = lp1;
#if NEWUNDO
		make_undo_patch(lp1,lp2,MARKPATCH);
#endif
		/* repoint the U line */
		resetuline(lp1,lp2);
		tag_for_undo(lp2);
		if (lismarked(lp1))
			lsetmarked(lp2);
		/* then remove lp1 from existence */
		lp1->l_bp->l_fp = lp1->l_fp;
		lp1->l_fp->l_bp = lp1->l_bp;
		toss_to_undo(lp1);
	} else {				/* Easy: in place	*/
		copy_for_undo(lp1);
		lp2 = lp1;			/* Pretend new line	*/
		lp2->l_used += n;
		cp2 = &lp1->l_text[lp1->l_used];
		cp1 = cp2-n;
		while (cp1 != &lp1->l_text[doto])
			*--cp2 = *--cp1;
	}
	for (i=0; i<n; ++i)			/* Add the characters	*/
		lp2->l_text[doto+i] = c;
	wp = wheadp;				/* Update windows	*/
	while (wp != NULL) {
		if (wp->w_linep == lp1)
			wp->w_linep = lp2;
		if (wp->w_dotp == lp1) {
			wp->w_dotp = lp2;
			if (wp==curwp || wp->w_doto>doto)
				wp->w_doto += n;
		}
		if (wp->w_mkp == lp1) {
			wp->w_mkp = lp2;
			if (wp->w_mko > doto)
				wp->w_mko += n;
		}
		if (wp->w_ldmkp == lp1) {
			wp->w_ldmkp = lp2;
			if (wp->w_ldmko > doto)
				wp->w_ldmko += n;
		}
		wp = wp->w_wndp;
	}
	if (curbp->b_nmmarks != NULL) { /* fix the named marks */
		struct MARK *mp;
		for (i = 0; i < 26; i++) {
			mp = &(curbp->b_nmmarks[i]);
			if (mp->markp == lp1) {
				mp->markp = lp2;
				if (mp->marko > doto)
					mp->marko += n;
			}
		}
	}
	return (TRUE);
}

/*
 * Insert a newline into the buffer at the current location of dot in the
 * current window. The funny ass-backwards way it does things is not a botch;
 * it just makes the last line in the file not a special case. Return TRUE if
 * everything works out and FALSE on error (memory allocation failure). The
 * update of dot and mark is a bit easier then in the above case, because the
 * split forces more updating.
 */
lnewline()
{
	register char	*cp1;
	register char	*cp2;
	register LINE	*lp1;
	register LINE	*lp2;
	register int	doto;
	register WINDOW *wp;

	lchange(WFHARD|WFINS);
	lp1  = curwp->w_dotp;			/* Get the address and	*/
	doto = curwp->w_doto;			/* offset of "."	*/
	if (lp1 != curbp->b_linep)
		copy_for_undo(lp1);
	if ((lp2=lalloc(doto)) == NULL) 	/* New first half line	*/
		return (FALSE);
	cp1 = &lp1->l_text[0];			/* Shuffle text around	*/
	cp2 = &lp2->l_text[0];
	while (cp1 != &lp1->l_text[doto])
		*cp2++ = *cp1++;
	cp2 = &lp1->l_text[0];
	while (cp1 != &lp1->l_text[lp1->l_used])
		*cp2++ = *cp1++;
	lp1->l_used -= doto;
	/* put lp2 in above lp1 */
	lp2->l_bp = lp1->l_bp;
	lp1->l_bp = lp2;
	lp2->l_bp->l_fp = lp2;
	lp2->l_fp = lp1;
	tag_for_undo(lp2);
	dumpuline(lp1);
	wp = wheadp;				/* Windows		*/
	while (wp != NULL) {
		if (wp->w_linep == lp1)
			wp->w_linep = lp2;
		if (wp->w_dotp == lp1) {
			if (wp->w_doto < doto)
				wp->w_dotp = lp2;
			else
				wp->w_doto -= doto;
		}
		if (wp->w_mkp == lp1) {
			if (wp->w_mko < doto)
				wp->w_mkp = lp2;
			else
				wp->w_mko -= doto;
		}
		if (wp->w_ldmkp == lp1) {
			if (wp->w_ldmko < doto)
				wp->w_ldmkp = lp2;
			else
				wp->w_ldmko -= doto;
		}
		wp = wp->w_wndp;
	}
	if (curbp->b_nmmarks != NULL) { /* fix the named marks */
		int i;
		struct MARK *mp;
		for (i = 0; i < 26; i++) {
			mp = &(curbp->b_nmmarks[i]);
			if (mp->markp == lp1) {
				if (mp->marko < doto)
					mp->markp = lp2;
				else
					mp->marko -= doto;
			}
		}
	}
	return (TRUE);
}

/*
 * This function deletes "n" bytes, starting at dot. It understands how do deal
 * with end of lines, etc. It returns TRUE if all of the characters were
 * deleted, and FALSE if they were not (because dot ran into the end of the
 * buffer. The "kflag" is TRUE if the text should be put in the kill buffer.
 */
ldelete(n, kflag)
long n; 	/* # of chars to delete */
int kflag;	/* put killed text in kill buffer flag */
{
	register char	*cp1;
	register char	*cp2;
	register LINE	*dotp;
	register LINE	*nlp;
	register int	doto;
	register int	chunk;
	register WINDOW *wp;
	register int i,s;

	while (n != 0) {
		dotp = curwp->w_dotp;
		doto = curwp->w_doto;
		if (dotp == curbp->b_linep)	/* Hit end of buffer.	*/
			return (FALSE);
		chunk = dotp->l_used-doto;	/* Size of chunk.	*/
		if (chunk > (int)n)
			chunk = (int)n;
		if (chunk == 0) {		/* End of line, merge.	*/
			lchange(WFHARD|WFKILLS);
			/* first take out any whole lines below this one */
			nlp = lforw(dotp);
			while (nlp != curbp->b_linep && llength(nlp)+1 < n) {
				if (kflag) {
					s = kinsert('\n');
					for (i = 0; i < llength(nlp) && 
								s == TRUE; i++)
						s = kinsert(lgetc(nlp,i));
					if (s != TRUE)
						return(FALSE);
				}
				lremove(curbp,nlp);
				toss_to_undo(nlp);
				n -= llength(nlp)+1;
				nlp = lforw(dotp);
			}
			if ((s = ldelnewline()) != TRUE)
				return (s);
			if (kflag && (s = kinsert('\n')) != TRUE)
				return (s);
			--n;
			continue;
		}
		lchange(WFEDIT);
		copy_for_undo(dotp);
		cp1 = &dotp->l_text[doto];	/* Scrunch text.	*/
		cp2 = cp1 + chunk;
		if (kflag) {		/* Kill?		*/
			while (cp1 != cp2) {
				if ((s = kinsert(*cp1)) != TRUE)
					return (s);
				++cp1;
			}
			cp1 = &dotp->l_text[doto];
		}
		while (cp2 != &dotp->l_text[dotp->l_used])
			*cp1++ = *cp2++;
		dotp->l_used -= chunk;
		wp = wheadp;			/* Fix windows		*/
		while (wp != NULL) {
			if (wp->w_dotp==dotp && wp->w_doto > doto) {
				wp->w_doto -= chunk;
				if (wp->w_doto < doto)
					wp->w_doto = doto;
			}
			if (wp->w_mkp==dotp && wp->w_mko > doto) {
				wp->w_mko -= chunk;
				if (wp->w_mko < doto)
					wp->w_mko = doto;
			}
			if (wp->w_ldmkp==dotp && wp->w_ldmko > doto) {
				wp->w_ldmko -= chunk;
				if (wp->w_ldmko < doto)
					wp->w_ldmko = doto;
			}
			wp = wp->w_wndp;
		}
		if (curbp->b_nmmarks != NULL) { /* fix the named marks */
			struct MARK *mp;
			for (i = 0; i < 26; i++) {
				mp = &(curbp->b_nmmarks[i]);
				if (mp->markp==dotp && mp->marko > doto) {
					mp->marko -= chunk;
					if (mp->marko < doto)
						mp->marko = doto;
				}
			}
		}
		n -= chunk;
	}
	return (TRUE);
}

/* getctext:	grab and return a string with the text of
		the current line
*/

char *getctext()

{
	register LINE *lp;	/* line to copy */
	register int size;	/* length of line to return */
	register char *sp;	/* string pointer into line */
	register char *dp;	/* string pointer into returned line */
	char rline[NSTRING];	/* line to return */

	/* find the contents of the current line and its length */
	lp = curwp->w_dotp;
	sp = lp->l_text;
	size = lp->l_used;
	if (size >= NSTRING)
		size = NSTRING - 1;

	/* copy it across */
	dp = rline;
	while (size--)
		*dp++ = *sp++;
	*dp = 0;
	return(rline);
}

#if ! SMALLER
/* putctext:	replace the current line with the passed in text	*/

putctext(iline)
char *iline;	/* contents of new line */
{
	register int status;

	/* delete the current line */
	curwp->w_doto = 0;	/* starting at the beginning of the line */
	if ((status = deltoeol(TRUE, 1)) != TRUE)
		return(status);

	/* insert the new line */
	while (*iline) {
		if (*iline == '\n') {
			if (lnewline() != TRUE)
				return(FALSE);
		} else {
			if (linsert(1, *iline) != TRUE)
				return(FALSE);
		}
		++iline;
	}
	status = lnewline();
	backline(TRUE, 1);
	return(status);
}
#endif

/*
 * Delete a newline. Join the current line with the next line. If the next line
 * is the magic header line always return TRUE; merging the last line with the
 * header line can be thought of as always being a successful operation, even
 * if nothing is done, and this makes the kill buffer work "right". Easy cases
 * can be done by shuffling data around. Hard cases require that lines be moved
 * about in memory. Return FALSE on error and TRUE if all looks ok. Called by
 * "ldelete" only.
 */
ldelnewline()
{
	register char	*cp1;
	register char	*cp2;
	register LINE	*lp1;
	register LINE	*lp2;
	register LINE	*lp3;
	register WINDOW *wp;

	lp1 = curwp->w_dotp;
	/* if the current line is empty, remove it */
	if (lp1->l_used == 0) {		/* Blank line.		*/
		lremove(curbp,lp1);
		toss_to_undo(lp1);
		return (TRUE);
	}
	lp2 = lp1->l_fp;
	/* if the next line is empty, that's "currline\n\n", so we
		remove the second \n by deleting the next line */
	/* but never delete the newline on the last non-empty line */
	if (lp2 == curbp->b_linep)
		return (TRUE);
	else if (lp2->l_used == 0) {
		/* next line blank? */
		lremove(curbp,lp2);
		toss_to_undo(lp2);
		return (TRUE);
	}
	copy_for_undo(lp1);
	/* room in line before nl for contents of line after nl */
	if (lp2->l_used <= lp1->l_size-lp1->l_used) {
		cp1 = &lp1->l_text[lp1->l_used];
		cp2 = &lp2->l_text[0];
		while (cp2 != &lp2->l_text[lp2->l_used])
			*cp1++ = *cp2++;
		/* check all windows for references to the deleted line */
		wp = wheadp;
		while (wp != NULL) {
			if (wp->w_linep == lp2)
				wp->w_linep = lp1;
			if (wp->w_dotp == lp2) {
				wp->w_dotp  = lp1;
				wp->w_doto += lp1->l_used;
			}
			if (wp->w_mkp == lp2) {
				wp->w_mkp  = lp1;
				wp->w_mko += lp1->l_used;
			}
			if (wp->w_ldmkp == lp2) {
				wp->w_ldmkp  = lp1;
				wp->w_ldmko += lp1->l_used;
			}
			wp = wp->w_wndp;
		}
		if (curbp->b_nmmarks != NULL) { /* fix the named marks */
			int i;
			struct MARK *mp;
			for (i = 0; i < 26; i++) {
				mp = &(curbp->b_nmmarks[i]);
				if (mp->markp == lp2) {
					mp->markp  = lp1;
					mp->marko += lp1->l_used;
				}
			}
		}
		lp1->l_used += lp2->l_used;
		lp1->l_fp = lp2->l_fp;
		lp2->l_fp->l_bp = lp1;
		dumpuline(lp1);
		toss_to_undo(lp2);
		return (TRUE);
	}
	/* need a new line */
	if ((lp3=lalloc(lp1->l_used+lp2->l_used)) == NULL)
		return (FALSE);
	/* copy the text */
	cp1 = &lp1->l_text[0];
	cp2 = &lp3->l_text[0];
	while (cp1 != &lp1->l_text[lp1->l_used])
		*cp2++ = *cp1++;
	cp1 = &lp2->l_text[0];
	while (cp1 != &lp2->l_text[lp2->l_used])
		*cp2++ = *cp1++;
	/* do this one step at a time, so undo can put 
			things back together right */
	toss_to_undo(lp1);

	/* now take out lp1 */
	lp1->l_bp->l_fp = lp2;
	lp2->l_bp = lp1->l_bp;
	toss_to_undo(lp2);

	/* now replace lp2 with lp3 */
	lp2->l_bp->l_fp = lp3;
	lp3->l_fp = lp2->l_fp;
	lp2->l_fp->l_bp = lp3;
	lp3->l_bp = lp2->l_bp;
	tag_for_undo(lp3);

	/* check all lwindows for references, and update */
	wp = wheadp;
	while (wp != NULL) {
		if (wp->w_linep==lp1 || wp->w_linep==lp2)
			wp->w_linep = lp3;
		if (wp->w_dotp == lp1)
			wp->w_dotp  = lp3;
		else if (wp->w_dotp == lp2) {
			wp->w_dotp  = lp3;
			wp->w_doto += lp1->l_used;
		}
		if (wp->w_mkp == lp1)
			wp->w_mkp  = lp3;
		else if (wp->w_mkp == lp2) {
			wp->w_mkp  = lp3;
			wp->w_mko += lp1->l_used;
		}
		if (wp->w_ldmkp == lp1)
			wp->w_ldmkp  = lp3;
		else if (wp->w_ldmkp == lp2) {
			wp->w_ldmkp  = lp3;
			wp->w_ldmko += lp1->l_used;
		}
		wp = wp->w_wndp;
	}
	if (curbp->b_nmmarks != NULL) { /* fix the named marks */
		int i;
		struct MARK *mp;
		for (i = 0; i < 26; i++) {
			mp = &(curbp->b_nmmarks[i]);
			if (mp->markp == lp1)
				mp->markp  = lp3;
			else if (mp->markp == lp2) {
				mp->markp  = lp3;
				mp->marko += lp1->l_used;
			}
		}
	}
	return (TRUE);
}

/*
 * Delete all of the text saved in the kill buffer. Called by commands when a
 * new kill context is being created. The kill buffer array is released, just
 * in case the buffer has grown to immense size. No errors.
 */
kdelete()
{

	if ((kregflag & KAPPEND) != 0)
		kregflag = KAPPEND;
	else
		kregflag = KNEEDCLEAN;

}

/*
 * Insert a character to the kill buffer, allocating new chunks as needed.
 * Return TRUE if all is well, and FALSE on errors.
 */

kinsert(c)
int c;		/* character to insert in the kill buffer */
{
	KILL *nchunk;	/* ptr to newly malloced chunk */
	KILLREG *kbp = &kbs[ukb];

	if ((kregflag & KNEEDCLEAN) && kbs[ukb].kbufh != NULL) {
		KILL *kp;	/* ptr to scan kill buffer chunk list */

		/* first, delete all the chunks */
		kbs[ukb].kbufp = kbs[ukb].kbufh;
		while (kbs[ukb].kbufp != NULL) {
			kp = kbs[ukb].kbufp->d_next;
			free((char *)(kbs[ukb].kbufp));
			kbs[ukb].kbufp = kp;
		}

		/* and reset all the kill buffer pointers */
		kbs[ukb].kbufh = kbs[ukb].kbufp = NULL;
		kbs[ukb].kused = KBLOCK; 	        

	}
	kregflag &= ~KNEEDCLEAN;
	kbs[ukb].kbflag = kregflag;

	/* check to see if we need a new chunk */
	if (kbp->kused >= KBLOCK || kbp->kbufh == NULL) {
		if ((nchunk = (KILL *)malloc(sizeof(KILL))) == NULL)
			return(FALSE);
		if (kbp->kbufh == NULL)	/* set head ptr if first time */
			kbp->kbufh = nchunk;
		/* point the current to this new one */
		if (kbp->kbufp != NULL)
			kbp->kbufp->d_next = nchunk;
		kbp->kbufp = nchunk;
		kbp->kbufp->d_next = NULL;
		kbp->kused = 0;
	}

	/* and now insert the character */
	kbp->kbufp->d_chunk[kbp->kused++] = c;
	return(TRUE);
}

usekreg(f,n)
{
	int c, status;

	/* take care of incrementing the buffer number, if we're replaying
		a command via 'dot' */
	incr_dot_kregnum();

	c = kbd_key();

	if (isdigit(c))
		ukb = c - '0';
	else if (islower(c))
		ukb = c - 'a' + 10;  /* named buffs are in 10 through 36 */
	else if (isupper(c)) {
		ukb = c - 'A' + 10;
		kregflag |= KAPPEND;
	} else {
		TTbeep();
		return (FALSE);
	}

	/* get the next command from the keyboard */
	c = kbd_seq();

	/* allow second chance for entering counts */
	if (f == FALSE) {
		do_num_proc(&c,&f,&n);
		do_rept_arg_proc(&c,&f,&n);
	}

	/* and execute the command */
	status = execute(c, f, n, NULL);

	ukb = 0;
	kregflag = 0;

	return(status);
	
}

/* buffers 0 through 9 are circulated automatically for full-line deletes */
/* we re-use one of them until the KLINES flag is on, then we advance */
/* to the next */
kregcirculate(killing)
{
	static lastkb; /* index of the real "0 */

	if (ukb >= 10) /* then the user specified a lettered buffer */
		return;

	/* we only allow killing into the real "0 */
	/* ignore any other buffer spec */
	if (killing) {
		if ((kbs[lastkb].kbflag & KLINES) && 
			! (kbs[lastkb].kbflag & KYANK)) {
			if (--lastkb < 0) lastkb = 9;
			kbs[lastkb].kbflag = 0;
		}
		ukb = lastkb;
	} else {
		/* let 0 pass unmolested -- it is the default */
		if (ukb == 0) {
			ukb = lastkb; 
		} else {
		/* for the others, if the current "0 has lines in it, it
		    must be `"1', else "1 is `"1'.  get it? */
			if (kbs[lastkb].kbflag & KLINES)
				ukb = (lastkb + ukb - 1) % 10;
			else
				ukb = (lastkb + ukb) % 10;
		}
	}
	
}

putbefore(f,n)
{
	return doput(f,n,FALSE,FALSE);
}

putafter(f,n)
{
	return doput(f,n,TRUE,FALSE);
}

lineputbefore(f,n)
{
	return doput(f,n,FALSE,TRUE);
}

lineputafter(f,n)
{
	return doput(f,n,TRUE,TRUE);
}


doput(f,n,after,putlines)
{
	int s, oukb, lining;
	
	if (!f)
		n = 1;
		
	oukb = ukb;
	kregcirculate(FALSE);
	if (kbs[ukb].kbufh == NULL) {
		if (ukb != 0)
			mlwrite("Nothing in register %c", 
				(oukb<10)? oukb+'0' : oukb-10+'a');
		TTbeep();
		return(FALSE);
	}
	lining = (putlines == TRUE || (kbs[ukb].kbflag & KLINES));
	if (lining) {
		if (after && curwp->w_dotp != curbp->b_linep)
			curwp->w_dotp = lforw(curwp->w_dotp);
		curwp->w_doto = 0;
	} else {
		if (after && curwp->w_doto != llength(curwp->w_dotp))
			forwchar(TRUE,1);
	}
	setmark();
	s = put(n,lining);
	if (s == TRUE)
		swapmark();
	if (curwp->w_dotp == curbp->b_linep)
		curwp->w_dotp = lback(curwp->w_dotp);
	if (lining)
		firstnonwhite(FALSE,0);
	ukb = 0;
	return (s);
}

/*
 * Put text back from the kill register.
 */
put(n,aslines)
{
	register int	c;
	register int	i;
	int wasnl, suppressnl;
	register char	*sp;	/* pointer into string to insert */
	KILL *kp;		/* pointer into kill register */
	
	if (n < 0)
		return FALSE;
		
	/* make sure there is something to put */
	if (kbs[ukb].kbufh == NULL)
		return TRUE;		/* not an error, just nothing */

	suppressnl = FALSE;
	wasnl = FALSE;

	/* for each time.... */
	while (n--) {
		kp = kbs[ukb].kbufh;
		while (kp != NULL) {
			if (kp->d_next == NULL)
				i = kbs[ukb].kused;
			else
				i = KBLOCK;
			sp = kp->d_chunk;
			while (i--) {
				if ((c = *sp++) == '\n') {
					if (lnewline() != TRUE)
						return FALSE;
					wasnl = TRUE;
				} else {
					if (curwp->w_dotp == curbp->b_linep)
						suppressnl = TRUE;
					if (linsert(1, c) != TRUE)
						return FALSE;
					wasnl = FALSE;
				}
			}
			kp = kp->d_next;
		}
		if (wasnl) {
			if (suppressnl) {
				if (ldelnewline() != TRUE)
					return FALSE;
			}
		} else {
			if (aslines && !suppressnl) {
				if (lnewline() != TRUE)
					return FALSE;
			}
		}
	}
        curwp->w_flag |= WFHARD;
	return (TRUE);
}
