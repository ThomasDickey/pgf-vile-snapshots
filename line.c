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
 *
 * $Log: line.c,v $
 * Revision 1.24  1992/07/04 14:36:17  foxharp
 * added temporary line-poisoner, to catch core dump on buffer/line reuse.
 *
 * Revision 1.23  1992/05/16  12:00:31  pgf
 * prototypes/ansi/void-int stuff/microsoftC
 *
 * Revision 1.22  1992/03/24  07:37:35  pgf
 * usekreg now works better as a namedcmd
 *
 * Revision 1.21  1992/03/07  10:28:52  pgf
 * don't bother copying and marking lines that aren't really being split,
 * in lnewline -- fixes problem "p"utting and undoing a blank line at the
 * end of file
 *
 * Revision 1.20  1992/03/03  09:35:52  pgf
 * added support for getting "words" out of the buffer via variables --
 * needed _nonspace character type
 *
 * Revision 1.19  1992/02/17  09:03:22  pgf
 * kill registers now hold unsigned chars
 *
 * Revision 1.18  1992/01/22  18:38:34  pgf
 * check for empty buffer in lnewline() was insufficient.  amazing what
 * you find when you go looking...
 *
 * Revision 1.17  1992/01/05  00:06:13  pgf
 * split mlwrite into mlwrite/mlprompt/mlforce to make errors visible more
 * often.  also normalized message appearance somewhat.
 *
 * Revision 1.16  1992/01/04  14:14:12  pgf
 * attempt to keep all newly inserted lines in a fresh buffer visible, rather
 * than have them "hidden" above the window, though the window might be empty
 *
 * Revision 1.15  1991/11/08  13:24:33  pgf
 * added klines and kchars counters to kinsert()
 *
 * Revision 1.14  1991/11/01  14:38:00  pgf
 * saber cleanup
 *
 * Revision 1.13  1991/10/29  03:02:04  pgf
 * fixups to usekreg, and added execkreg and loadkreg
 *
 * Revision 1.12  1991/10/24  12:59:18  pgf
 * new lgrow() routine, to add more space in a line
 *
 * Revision 1.11  1991/10/10  12:33:33  pgf
 * changes to support "block malloc" of line text -- now for most files
 * there is are two mallocs and a single read, no copies.  previously there
 * were two mallocs per line, and two copies (stdio's and ours).  This change
 * implies that lines and line text should not move between buffers, without
 * checking that the text and line struct do not "belong" to the buffer.
 *
 * Revision 1.10  1991/09/24  01:04:10  pgf
 * do lnewline() correctly in empty buffer
 *
 * Revision 1.9  1991/08/07  12:35:07  pgf
 * added RCS log messages
 *
 * revision 1.8
 * date: 1991/08/06 15:22:27;
 * allow null l_text pointers for empty lines
 * 
 * revision 1.7
 * date: 1991/07/19 17:14:49;
 * fixed missing "copy_for_undo" bug introduced a while ago
 * 
 * revision 1.6
 * date: 1991/06/25 19:52:54;
 * massive data structure restructure
 * 
 * revision 1.5
 * date: 1991/06/16 17:35:32;
 * fixed bug -- wasn't assigning new size to line struct when re-allocing
 * for a line merge
 * 
 * revision 1.4
 * date: 1991/06/03 10:24:26;
 * took out old #ifdef INLINE stuff, and
 * added comments for usekreg
 * 
 * revision 1.3
 * date: 1991/05/31 11:11:16;
 * change args to execute()
 * 
 * revision 1.2
 * date: 1991/04/04 09:28:37;
 * line text is now separate from LINE struct
 * 
 * revision 1.1
 * date: 1990/09/21 10:25:32;
 * initial vile RCS revision
 */

#define poison(p,s) memset(p, 0xdf, s)
/* #define poison(p,s)  */

#include	<stdio.h>
#include	"estruct.h"
#include	"edef.h"

#define roundup(n) ((n+NBLOCK-1) & ~(NBLOCK-1))
/*
 * This routine allocates a block of memory large enough to hold a LINE
 * containing "used" characters. The block is always rounded up a bit. Return
 * a pointer to the new block, or NULL if there isn't any memory left. Print a
 * message in the message line if no space.
 */
LINE *
lalloc(used,bp)
register int	used;
BUFFER *bp;
{
	register LINE	*lp;
	register int	size;

	/* lalloc(-1) is used by undo for placeholders */
	if (used < 0)  {
		size = 0;
	} else {
		size = roundup(used);
	}
	/* see if the buffer LINE block has any */
	if ((lp = bp->b_freeLINEs) != NULL) {
		bp->b_freeLINEs = lp->l_fp;
	} else if ((lp = (LINE *) malloc(sizeof(LINE))) == NULL) {
		mlforce("[OUT OF MEMORY]");
		return NULL;
	}
	lp->l_text = NULL;
	if (size && (lp->l_text = malloc(size)) == NULL) {
		mlforce("[OUT OF MEMORY]");
		poison(lp, sizeof(*lp));
		free((char *)lp);
		return NULL;
	}
	lp->l_size = size;
	lp->l_used = used;
	lsetclear(lp);
	lp->l_nxtundo = NULL;
	return (lp);
}

int
lgrow(lp,howmuch,bp)
register LINE	*lp;
register int	howmuch;
BUFFER *bp;
{
	register int	size;
	char *ntext;

	size = roundup(lp->l_size + howmuch);
	ntext = (char *)malloc(size);
	if (ntext == NULL) {
		mlforce("[OUT OF MEMORY]");
		return FALSE;
	}
	memcpy(ntext, lp->l_text, lp->l_used);
	ltextfree(lp,bp);
	lp->l_text = ntext;
	lp->l_size = size;
	return TRUE;
}



void
lfree(lp,bp)
register LINE *lp;
register BUFFER *bp;
{
	ltextfree(lp,bp);

	/* if the buffer doesn't have its own block of LINEs, or this
		one isn't in that range, free it */
	if (!bp->b_LINEs || lp < bp->b_LINEs || lp >= bp->b_LINEs_end) {
		poison(lp, sizeof(*lp));
		free((char *)lp);
	} else {
		/* keep track of freed buffer LINEs here */
		lp->l_fp = bp->b_freeLINEs;
		bp->b_freeLINEs = lp;
		lp->l_text = (char *)1; /* catch references hard */
	}
}

void
ltextfree(lp,bp)
register LINE *lp;
register BUFFER *bp;
{
	register unsigned char *ltextp;

	ltextp = (unsigned char *)lp->l_text;
	if (ltextp) {
		if (bp->b_ltext) { /* could it be in the big range? */
			if (ltextp < bp->b_ltext || ltextp >= bp->b_ltext_end) {
				poison(ltextp, lp->l_size);
				free((char *)ltextp);
			} /* else {
			could keep track of freed big range text here;
			} */
		} else {
			poison(ltextp, lp->l_size);
			free((char *)ltextp);
		}
		lp->l_text = NULL;
	} /* else nothing to free */
}

/*
 * Delete line "lp". Fix all of the links that might point at it (they are
 * moved to offset 0 of the next line. Unlink the line from whatever buffer it
 * might be in. The buffers are updated too; the magic
 * conditions described in the above comments don't hold here.
 * Memory is not released, so line can be saved in undo stacks.
 */
void
lremove(bp,lp)
register BUFFER *bp;
register LINE	*lp;
{
	register WINDOW *wp;

#if !WINMARK
	if (MK.l == lp) {
		MK.l = lp->l_fp;
		MK.o = 0;
	}
#endif
	wp = wheadp;
	while (wp != NULL) {
		if (wp->w_line.l == lp)
			wp->w_line.l = lp->l_fp;
		if (wp->w_dot.l	== lp) {
			wp->w_dot.l  = lp->l_fp;
			wp->w_dot.o  = 0;
		}
#if WINMARK
		if (wp->w_mark.l == lp) {
			wp->w_mark.l = lp->l_fp;
			wp->w_mark.o = 0;
		}
#endif
#if 0
		if (wp->w_lastdot.l == lp) {
			wp->w_lastdot.l = lp->l_fp;
			wp->w_lastdot.o = 0;
		}
#endif
		wp = wp->w_wndp;
	}
	if (bp->b_nwnd == 0) {
		if (bp->b_dot.l	== lp) {
			bp->b_dot.l = lp->l_fp;
			bp->b_dot.o = 0;
		}
#if WINMARK
		if (bp->b_mark.l == lp) {
			bp->b_mark.l = lp->l_fp;
			bp->b_mark.o = 0;
		}
#endif
#if 0
		if (bp->b_lastdot.l == lp) {
			bp->b_lastdot.l = lp->l_fp;
			bp->b_lastdot.o = 0;
		}
#endif
	}
#if 0
	if (bp->b_nmmarks != NULL) { /* fix the named marks */
		int i;
		struct MARK *mp;
		for (i = 0; i < 26; i++) {
			mp = &(bp->b_nmmarks[i]);
			if (mp->p == lp) {
				mp->p = lp->l_fp;
				mp->o = 0;
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
void
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

int
insspace(f, n)	/* insert spaces forward into text */
int f, n;	/* default flag and numeric argument */
{
	linsert(n, ' ');
	return backchar(f, n);
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
int
linsert(n, c)
int n, c;
{
	register char	*cp1;
	register char	*cp2;
	register LINE	*lp1;
	register LINE	*lp2;
	register LINE	*lp3;
	register int	doto;
	register int	i;
	register WINDOW *wp;
	register char	*ntext;
	int nsize;

	lchange(WFEDIT);
	lp1 = curwp->w_dot.l;			/* Current line 	*/
	if (lp1 == curbp->b_line.l) {		/* At the end: special	*/
		if (curwp->w_dot.o != 0) {
			mlforce("BUG: linsert");
			return (FALSE);
		}
		if ((lp2=lalloc(n,curbp)) == NULL) /* Allocate new line	*/
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
		curwp->w_dot.l = lp2;
		curwp->w_dot.o = n;
		tag_for_undo(lp2);
		return (TRUE);
	}
	doto = curwp->w_dot.o;			/* Save for later.	*/
	if (lp1->l_used+n > lp1->l_size) {	/* Hard: reallocate	*/
		copy_for_undo(lp1);
		/* first, create the new image */
		if ((ntext=malloc(nsize = roundup(lp1->l_used+n))) == NULL)
			return (FALSE);
		if (lp1->l_text) /* possibly NULL if l_size == 0 */
			memcpy(&ntext[0],      &lp1->l_text[0],    doto);
		memset(&ntext[doto],   c, n);
		if (lp1->l_text) {
			memcpy(&ntext[doto+n], &lp1->l_text[doto],
							lp1->l_used-doto );
			ltextfree(lp1,curbp);
		}
		lp1->l_text = ntext;
		lp1->l_size = nsize;
		lp1->l_used += n;
	} else {				/* Easy: in place	*/
		copy_for_undo(lp1);
		/* don't used memcpy:  overlapping regions.... */
		lp1->l_used += n;
		cp2 = &lp1->l_text[lp1->l_used];
		cp1 = cp2-n;
		while (cp1 != &lp1->l_text[doto])
			*--cp2 = *--cp1;
		for (i=0; i<n; ++i)		/* Add the characters	*/
			lp1->l_text[doto+i] = c;
	}
#if ! WINMARK
	if (MK.l == lp1) {
		if (MK.o > doto)
			MK.o += n;
	}
#endif
	wp = wheadp;				/* Update windows	*/
	while (wp != NULL) {
		if (wp->w_dot.l == lp1) {
			if (wp==curwp || wp->w_dot.o>doto)
				wp->w_dot.o += n;
		}
#if WINMARK
		if (wp->w_mark.l == lp1) {
			if (wp->w_mark.o > doto)
				wp->w_mark.o += n;
		}
#endif
		if (wp->w_lastdot.l == lp1) {
			if (wp->w_lastdot.o > doto)
				wp->w_lastdot.o += n;
		}
		wp = wp->w_wndp;
	}
	if (curbp->b_nmmarks != NULL) { /* fix the named marks */
		struct MARK *mp;
		for (i = 0; i < 26; i++) {
			mp = &(curbp->b_nmmarks[i]);
			if (mp->l == lp1) {
				if (mp->o > doto)
					mp->o += n;
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
int
lnewline()
{
	register char	*cp1;
	register char	*cp2;
	register LINE	*lp1;
	register LINE	*lp2;
	register int	doto;
	register WINDOW *wp;

	lchange(WFHARD|WFINS);
	lp1  = curwp->w_dot.l;			/* Get the address and	*/
	doto = curwp->w_dot.o;			/* offset of "."	*/
	if (lp1 == curbp->b_line.l && lforw(lp1) == lp1) {
		/* empty buffer -- just  create empty line */
		if ((lp2=lalloc(doto,curbp)) == NULL)
			return (FALSE);
		/* put lp2 in below lp1 */
		lp2->l_fp = lp1->l_fp;
		lp1->l_fp = lp2;
		lp2->l_fp->l_bp = lp2;
		lp2->l_bp = lp1;
		tag_for_undo(lp2);
		wp = wheadp;
		while (wp != NULL) {
			if (wp->w_line.l == lp1)
				wp->w_line.l = lp2;
			if (wp->w_dot.l == lp1)
				wp->w_dot.l = lp2;
			wp = wp->w_wndp;
		}
		return TRUE;
	}
	if ((lp2=lalloc(doto,curbp)) == NULL) 	/* New first half line	*/
		return (FALSE);
	if (doto > 0) {
		copy_for_undo(lp1);
		cp1 = &lp1->l_text[0];		/* Shuffle text around	*/
		cp2 = &lp2->l_text[0];
		while (cp1 != &lp1->l_text[doto])
			*cp2++ = *cp1++;
		cp2 = &lp1->l_text[0];
		while (cp1 != &lp1->l_text[lp1->l_used])
			*cp2++ = *cp1++;
		lp1->l_used -= doto;
	}
	/* put lp2 in above lp1 */
	lp2->l_bp = lp1->l_bp;
	lp1->l_bp = lp2;
	lp2->l_bp->l_fp = lp2;
	lp2->l_fp = lp1;
	tag_for_undo(lp2);
	dumpuline(lp1);
#if ! WINMARK
	if (MK.l == lp1) {
		if (MK.o < doto)
			MK.l = lp2;
		else
			MK.o -= doto;
	}
#endif
	wp = wheadp;				/* Windows		*/
	while (wp != NULL) {
		if (wp->w_line.l == lp1)
			wp->w_line.l = lp2;
		if (wp->w_dot.l == lp1) {
			if (wp->w_dot.o < doto)
				wp->w_dot.l = lp2;
			else
				wp->w_dot.o -= doto;
		}
#if WINMARK
		if (wp->w_mark.l == lp1) {
			if (wp->w_mark.o < doto)
				wp->w_mark.l = lp2;
			else
				wp->w_mark.o -= doto;
		}
#endif
		if (wp->w_lastdot.l == lp1) {
			if (wp->w_lastdot.o < doto)
				wp->w_lastdot.l = lp2;
			else
				wp->w_lastdot.o -= doto;
		}
		wp = wp->w_wndp;
	}
	if (curbp->b_nmmarks != NULL) { /* fix the named marks */
		int i;
		struct MARK *mp;
		for (i = 0; i < 26; i++) {
			mp = &(curbp->b_nmmarks[i]);
			if (mp->l == lp1) {
				if (mp->o < doto)
					mp->l = lp2;
				else
					mp->o -= doto;
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
int
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
		dotp = DOT.l;
		doto = DOT.o;
		if (dotp == curbp->b_line.l)	/* Hit end of buffer.	*/
			return (FALSE);
		chunk = dotp->l_used-doto;	/* Size of chunk.	*/
		if (chunk > (int)n)
			chunk = (int)n;
		if (chunk == 0) {		/* End of line, merge.	*/
			lchange(WFHARD|WFKILLS);
			/* first take out any whole lines below this one */
			nlp = lforw(dotp);
			while (nlp != curbp->b_line.l && llength(nlp)+1 < n) {
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
#if ! WINMARK
		if (MK.l && MK.o > doto) {
			MK.o -= chunk;
			if (MK.o < doto)
				MK.o = doto;
		}
#endif
		wp = wheadp;			/* Fix windows		*/
		while (wp != NULL) {
			if (wp->w_dot.l==dotp && wp->w_dot.o > doto) {
				wp->w_dot.o -= chunk;
				if (wp->w_dot.o < doto)
					wp->w_dot.o = doto;
			}
#if WINMARK
			if (wp->w_mark.l==dotp && wp->w_mark.o > doto) {
				wp->w_mark.o -= chunk;
				if (wp->w_mark.o < doto)
					wp->w_mark.o = doto;
			}
#endif
			if (wp->w_lastdot.l==dotp && wp->w_lastdot.o > doto) {
				wp->w_lastdot.o -= chunk;
				if (wp->w_lastdot.o < doto)
					wp->w_lastdot.o = doto;
			}
			wp = wp->w_wndp;
		}
		if (curbp->b_nmmarks != NULL) { /* fix the named marks */
			struct MARK *mp;
			for (i = 0; i < 26; i++) {
				mp = &(curbp->b_nmmarks[i]);
				if (mp->l==dotp && mp->o > doto) {
					mp->o -= chunk;
					if (mp->o < doto)
						mp->o = doto;
				}
			}
		}
		n -= chunk;
	}
	return (TRUE);
}

/* getctext:	grab and return a string with text from
		the current line, consisting of chars of type "type"
*/

char *getctext(type)
int type;
{
	static char rline[NSTRING];	/* line to return */

	screen_string(rline, NSTRING, type);
	return rline;
}

#if ! SMALLER
/* putctext:	replace the current line with the passed in text	*/

int
putctext(iline)
char *iline;	/* contents of new line */
{
	register int status;

	/* delete the current line */
	curwp->w_dot.o = 0;	/* starting at the beginning of the line */
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
int
ldelnewline()
{
	register char	*cp1;
	register char	*cp2;
	register LINE	*lp1;
	register LINE	*lp2;
	register WINDOW *wp;

	lp1 = curwp->w_dot.l;
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
	if (lp2 == curbp->b_line.l)
		return (TRUE);
	else if (lp2->l_used == 0) {
		/* next line blank? */
		lremove(curbp,lp2);
		toss_to_undo(lp2);
		return (TRUE);
	}
	copy_for_undo(lp1);
	/* no room in line above, make room */
	if (lp2->l_used > lp1->l_size-lp1->l_used) {
		char *ntext;
		int nsize;
		/* first, create the new image */
		if ((ntext=malloc(nsize = roundup(lp1->l_used + lp2->l_used)))
								 == NULL)
			return (FALSE);
		if (lp1->l_text) { /* possibly NULL if l_size == 0 */
			memcpy(&ntext[0], &lp1->l_text[0], lp1->l_used);
			ltextfree(lp1,curbp);
		}
		lp1->l_text = ntext;
		lp1->l_size = nsize;
	}
	cp1 = &lp1->l_text[lp1->l_used];
	cp2 = &lp2->l_text[0];
	while (cp2 != &lp2->l_text[lp2->l_used])
		*cp1++ = *cp2++;
#if ! WINMARK
	if (MK.l == lp2) {
		MK.l  = lp1;
		MK.o += lp1->l_used;
	}
#endif
	/* check all windows for references to the deleted line */
	wp = wheadp;
	while (wp != NULL) {
		if (wp->w_line.l == lp2)
			wp->w_line.l = lp1;
		if (wp->w_dot.l == lp2) {
			wp->w_dot.l  = lp1;
			wp->w_dot.o += lp1->l_used;
		}
#if WINMARK
		if (wp->w_mark.l == lp2) {
			wp->w_mark.l  = lp1;
			wp->w_mark.o += lp1->l_used;
		}
#endif
		if (wp->w_lastdot.l == lp2) {
			wp->w_lastdot.l  = lp1;
			wp->w_lastdot.o += lp1->l_used;
		}
		wp = wp->w_wndp;
	}
	if (curbp->b_nmmarks != NULL) { /* fix the named marks */
		int i;
		struct MARK *mp;
		for (i = 0; i < 26; i++) {
			mp = &(curbp->b_nmmarks[i]);
			if (mp->l == lp2) {
				mp->l  = lp1;
				mp->o += lp1->l_used;
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

/*
 * Delete all of the text saved in the kill buffer. Called by commands when a
 * new kill context is being created. The kill buffer array is released, just
 * in case the buffer has grown to immense size. No errors.
 */
void
kdelete()
{

	if ((kregflag & KAPPEND) != 0)
		kregflag = KAPPEND;
	else
		kregflag = KNEEDCLEAN;
	kchars = klines = 0;

}

/*
 * Insert a character to the kill buffer, allocating new chunks as needed.
 * Return TRUE if all is well, and FALSE on errors.
 */

int
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
	kchars++;
	if (c == '\n')
		klines++;
	return(TRUE);
}

/* select one of the named registers for use with the following command */
/*  this could actually be handled as a command prefix, in kbdseq(), much
	the way ^X-cmd and META-cmd are done, except that we need to be
	able to accept any of
		 3"adw	"a3dw	"ad3w
	to delete 3 words into register a.  So this routine gives us an
	easy way to handle the second case.  (The third case is handled in
	operators(), the first in main())
*/
int
usekreg(f,n)
int f,n;
{
	int c, status;
	CMDFUNC *cfp;			/* function to execute */
	char tok[NSTRING];		/* command incoming */

	/* take care of incrementing the buffer number, if we're replaying
		a command via 'dot' */
	incr_dot_kregnum();

	if (clexec || isnamedcmd) {
		int stat;
		static char cbuf[2];
	        if ((stat=mlreply("Use named register: ", cbuf, 2)) != TRUE)
	                return stat;
		c = cbuf[0];
        } else {
		c = kbd_key();
        }

	if (isdigit(c))
		ukb = c - '0';
	else if (islower(c))
		ukb = c - 'a' + 10;  /* named buffs are in 10 through 36 */
	else if (isupper(c)) {
		ukb = c - 'A' + 10;
	} else {
		TTbeep();
		return (FALSE);
	}

	if (kbdmode == PLAY && kbdplayreg == ukb) {
		mlforce("[Error: currently executing register %c]",c);
		kbdmode = STOP;
		return FALSE;
	}
	
	if (isupper(c))
		kregflag |= KAPPEND;

	if (clexec) {
		macarg(tok);	/* get the next token */
		cfp = engl2fnc(tok);
	} else if (isnamedcmd) {
		return namedcmd(f,n);
	} else {
		/* get the next command from the keyboard */
		c = kbd_seq();

		/* allow second chance for entering counts */
		if (f == FALSE) {
			do_num_proc(&c,&f,&n);
			do_rept_arg_proc(&c,&f,&n);
		}
		cfp = kcod2fnc(c);
	}

	/* and execute the command */
	status = execute(cfp, f, n);

	ukb = 0;
	kregflag = 0;

	return(status);
	
}

/* buffers 0 through 9 are circulated automatically for full-line deletes */
/* we re-use one of them until the KLINES flag is on, then we advance */
/* to the next */
void
kregcirculate(killing)
int killing;
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

int
putbefore(f,n)
int f,n;
{
	return doput(f,n,FALSE,FALSE);
}

int
putafter(f,n)
int f,n;
{
	return doput(f,n,TRUE,FALSE);
}

int
lineputbefore(f,n)
int f,n;
{
	return doput(f,n,FALSE,TRUE);
}

int
lineputafter(f,n)
int f,n;
{
	return doput(f,n,TRUE,TRUE);
}


int
doput(f,n,after,putlines)
int f,n,after,putlines;
{
	int s, oukb, lining;
	
	if (!f)
		n = 1;
		
	oukb = ukb;
	kregcirculate(FALSE);
	if (kbs[ukb].kbufh == NULL) {
		if (ukb != 0)
			mlforce("[Nothing in register %c]", 
				(oukb<10)? oukb+'0' : oukb-10+'a');
		TTbeep();
		return(FALSE);
	}
	lining = (putlines == TRUE || (kbs[ukb].kbflag & KLINES));
	if (lining) {
		if (after && !is_header_line(curwp->w_dot, curbp))
			curwp->w_dot.l = lforw(curwp->w_dot.l);
		curwp->w_dot.o = 0;
	} else {
		if (after && !is_at_end_of_line(curwp->w_dot))
			forwchar(TRUE,1);
	}
	setmark();
	s = put(n,lining);
	if (s == TRUE)
		swapmark();
	if (is_header_line(curwp->w_dot, curbp))
		curwp->w_dot.l = lback(curwp->w_dot.l);
	if (lining)
		firstnonwhite(FALSE,0);
	ukb = 0;
	return (s);
}

/*
 * Put text back from the kill register.
 */
int
put(n,aslines)
int n,aslines;
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
			sp = (char *)kp->d_chunk;
			while (i--) {
				if ((c = *sp++) == '\n') {
					if (lnewline() != TRUE)
						return FALSE;
					wasnl = TRUE;
				} else {
					if (is_header_line(curwp->w_dot,curbp))
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

/* ARGSUSED */
int
execkreg(f,n)
int f,n;
{
	int c, i;
	KILL *kp;		/* pointer into kill register */
	static lastreg = -1;

	if (kbdmode != STOP) {
		mlforce("[Error: already executing macro]");
		kbdmode = STOP;
		return FALSE;
	}

	if (!f)
		n = 1;
	else if (n <= 0)
		return TRUE;

	if (clexec || isnamedcmd) {
		int stat;
		static char cbuf[2];
	        if ((stat=mlreply("Execute register: ", cbuf, 2)) != TRUE)
	                return stat;
		c = cbuf[0];
        } else {
		c = kbd_key();
        }

	if (c == '@' && lastreg != -1) {
		c = lastreg;
	} else if (c < 'a' || c > 'z') {
		TTbeep();
		mlforce("[Invalid register name]");
		return FALSE;
	}

	lastreg = c;

	c = c - 'a' + 10;

	/* make sure there is something to execure */
	kp = kbs[c].kbufh;
	if (kp == NULL)
		return TRUE;		/* not an error, just nothing */

	if (kp->d_next == NULL) {
		i = kbs[c].kused;
	} else {
		mlforce("Warning:  only first %d characters will execute",
								KBLOCK);
		i = KBLOCK;
	}
	kbdrep = n;		/* remember how many times to execute */
	kbdmode = PLAY; 	/* start us in play mode */
	kbdplayreg = c;  	/* which buffer is playing */
	kbdptr = kp->d_chunk;	/*    at the beginning */
	kbdend = &kp->d_chunk[i];
	dotcmdmode = STOP;
	return TRUE;
}

/* ARGSUSED */
int
loadkreg(f,n)
int f,n;
{
	int s;
	char respbuf[NFILEN];

	kdelete();
	s = kbd_string("Load register with: ", respbuf,
					NFILEN - 1, '\n', NO_EXPAND, FALSE);
	if (s != TRUE)
		return FALSE;
	for (s = 0; s < NFILEN; s++) {
		if (!respbuf[s])
			break;
		kinsert(respbuf[s]);
	}
	return TRUE;
}
