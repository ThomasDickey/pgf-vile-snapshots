
/* these routines take care of undo operations
 * code by Paul Fox, original algorithm mostly by Julia Harper May, 89
 *
 * $Log: undo.c,v $
 * Revision 1.17  1992/05/16 12:00:31  pgf
 * prototypes/ansi/void-int stuff/microsoftC
 *
 * Revision 1.16  1992/01/05  00:06:13  pgf
 * split mlwrite into mlwrite/mlprompt/mlforce to make errors visible more
 * often.  also normalized message appearance somewhat.
 *
 * Revision 1.15  1991/11/08  13:06:42  pgf
 * moved firstchar() to basic.c
 *
 * Revision 1.14  1991/11/02  19:40:06  pgf
 * fixed bad free in lineundo()
 *
 * Revision 1.13  1991/11/01  14:38:00  pgf
 * saber cleanup
 *
 * Revision 1.12  1991/10/08  01:30:00  pgf
 * added new bp arg to lfree and lalloc
 *
 * Revision 1.11  1991/09/30  01:47:24  pgf
 * satisfy gcc's objection to ' in ifdefs
 *
 * Revision 1.10  1991/09/10  13:04:34  pgf
 * don't let DOT.o go negative after an undo
 *
 * Revision 1.9  1991/08/07  12:35:07  pgf
 * added RCS log messages
 *
 * revision 1.8
 * date: 1991/08/06 15:26:42;
 * no undo on scratch buffers, and
 * allow for null l_text pointers
 * 
 * revision 1.7
 * date: 1991/07/23 11:11:42;
 * undo is an absolute motion -- maintain "lastdot" properly
 * 
 * revision 1.6
 * date: 1991/06/25 19:53:36;
 * massive data structure restructure
 * 
 * revision 1.5
 * date: 1991/06/03 10:27:18;
 * took out #if INLINE code
 * 
 * revision 1.4
 * date: 1991/04/04 09:44:09;
 * undo line makes use of the separated line text
 * 
 * revision 1.3
 * date: 1991/04/04 09:28:41;
 * line text is now separate from LINE struct
 * 
 * revision 1.2
 * date: 1991/03/26 17:01:36;
 * preserve offset across undo
 * 
 * revision 1.1
 * date: 1990/09/21 10:26:14;
 * initial vile RCS revision
 */

#include "estruct.h"
#include "edef.h"

#ifndef NULL
#define NULL 0
#endif


/* the undo strategy is this:
	1) For any deleted line, push it onto the undo list.
	2) On any change to a line, make a copy of it, push the copy to
		the undo list, and mark the original as having been copied.
		Do not copy/push lines that are marked as having been copied.
		Push a tag matching up the copy with the original.  Later,
		when the copy has been put into the file, we can
		go back through the undo stack, find lines there pointing
		at the original, and make them point at the copy.  ugh.
		This wouldn't be necessary if we used line no's as the pointers,
		instead of real pointers.

	On the actual undo, we pop these things one by one.  There should
	either be no lines where it goes (it was deleted), or exactly
	one line where it goes (it was changed/copied).  That makes it
	easy to undo the changes one by one.  Of course, we need to build
	a different, inverse stack as we go, so that undo can be undone.

	The "copied" flag in the LINE structure is unioned with the stack
	link pointer on the undo stack, since they aren't both needed at once.

*/


#define CURSTK(bp) (&(bp->b_udstks[bp->b_udstkindx]))
#define ALTSTK(bp) (&(bp->b_udstks[1^(bp->b_udstkindx)]))
/*
#define CURDOTP(bp) (bp->b_uddot[bp->b_udstkindx].l)
#define ALTDOTP(bp) (bp->b_uddot[1^(bp->b_udstkindx)].l)
#define CURDOTO(bp) (bp->b_uddot[bp->b_udstkindx].o)
#define ALTDOTO(bp) (bp->b_uddot[1^(bp->b_udstkindx)].o)
*/
#define CURDOT(bp) bp->b_uddot[bp->b_udstkindx]
#define ALTDOT(bp) bp->b_uddot[1^(bp->b_udstkindx)]
#define SWITCHSTKS(bp) (bp->b_udstkindx = 1 ^ bp->b_udstkindx)

short needundocleanup;
LINE *copyline();
void make_undo_patch();

/* push the line onto the right undo stack. */
void
toss_to_undo(lp)
LINE *lp;
{
	if (curbp->b_flag & BFSCRTCH)
		return;
	if (needundocleanup)
		preundocleanup();
	pushline(lp,CURSTK(curbp));
	if ((ALTDOT(curbp).l == NULL) || (ALTDOT(curbp).l == lp)) {
		/* need to save a dot -- either the next line or 
			the previous one */
		if (lp->l_fp == curbp->b_line.l) {
			ALTDOT(curbp).l = lp->l_bp;
			ALTDOT(curbp).o = firstchar(lp->l_bp);
		} else {
			ALTDOT(curbp).l = lp->l_fp;
			ALTDOT(curbp).o = firstchar(lp->l_fp);
		}
	}
	dumpuline(lp);
}

/* push a copy of a line onto the right undo stack */
/* push a patch so we can later fix up any references to this line that */
/* might already be in the stack.  */
/* This unforutunate breach of stak protocol is because we'd rather push the */
/* _copy_ than the origianal. When the undo happens, the later pops will  */
/* point at the _original_ (which will by then be on the other undo stack)  */
/* unless we fix them now. */
int
copy_for_undo(lp)
LINE *lp;
{
	register LINE *nlp;

	if (curbp->b_flag & BFSCRTCH)
		return TRUE;
	if (needundocleanup)
		preundocleanup();

	if (liscopied(lp))
		return(TRUE);

	/* take care of the normal undo stack */
	nlp = copyline(lp);
	if (nlp == NULL)
		return(FALSE);
	pushline(nlp,CURSTK(curbp));

	make_undo_patch(lp,nlp,LINEUNDOPATCH);

	lsetcopied(lp);

	setupuline(lp);

	if (ALTDOT(curbp).l == NULL) {
		ALTDOT(curbp).l = lp;
		ALTDOT(curbp).o = curwp->w_dot.o;
	}
	return (TRUE);
}

/* push an unreal line onto the right undo stack */
/* lp should be the new line, _after_ insertion, so l_fp and l_bp are right */
int
tag_for_undo(lp)
LINE *lp;
{
	register LINE *nlp;

	if (curbp->b_flag & BFSCRTCH)
		return TRUE;
	if (needundocleanup)
		preundocleanup();

	if (liscopied(lp))
		return(TRUE);

	nlp = lalloc(-1,curbp);
	if (nlp == NULL)
		return(FALSE);
	llength(nlp) = LINENOTREAL;
	nlp->l_fp = lp->l_fp;
	nlp->l_bp = lp->l_bp;
	pushline(nlp,CURSTK(curbp));
	lsetcopied(lp);
	if (ALTDOT(curbp).l == NULL) {
		    ALTDOT(curbp).l = lp;
		    ALTDOT(curbp).o = curwp->w_dot.o;
	}
	return (TRUE);
}

void
pushline(lp,stk)
LINE *lp,**stk;
{
	lp->l_nxtundo = *stk;
	*stk = lp;
}

LINE *
popline(stk)
LINE **stk;
{
	LINE *lp;
	lp = *stk;
	if (lp != NULL) {
		*stk = lp->l_nxtundo;
		lp->l_nxtundo = NULL;
	}
	return (lp);
}

void
make_undo_patch(olp,nlp,type)
LINE *olp,*nlp;
int type;
{
	register LINE *plp;
	/* push on a tag that matches up the copy with the original */
	plp = lalloc(-1,curbp);
	if (plp == NULL)
		return;
	llength(plp) = type;
	plp->l_fp = olp;	/* l_fp is the original line */
	plp->l_bp = nlp;	/* l_bp is the copy */
	pushline(plp,CURSTK(curbp));
}

void
patchstk(newlp,oldlp)
LINE *newlp, *oldlp;
{
	register LINE *tlp;
	for (tlp = *CURSTK(curbp); tlp != NULL ; tlp = tlp->l_nxtundo) {
		if (!lispatch(tlp)) {
			if (tlp->l_fp == oldlp)
				tlp->l_fp = newlp;
			if (tlp->l_bp == oldlp)
				tlp->l_bp = newlp;
		}
	}
}

LINE *
copyline(lp)
register LINE *lp;
{
	register LINE *nlp;
	
	nlp = lalloc(lp->l_used,curbp);
	if (nlp == NULL)
		return(NULL);
	/* copy the text and forward and back pointers.  everything else 
		matches already */
	nlp->l_fp = lp->l_fp;
	nlp->l_bp = lp->l_bp;
	/* copy the rest */
	if (lp->l_text && nlp->l_text)
		memcpy(nlp->l_text, lp->l_text, lp->l_used);
	return nlp;
}


/* before any undoable command (except undo itself), clean the undo list */
/* clean the copied flag on the line we're the copy of */
void
freeundostacks(bp)
register BUFFER *bp;
{
	register LINE *lp;
	int i;

	for (i = 0; i <= 1; i++, SWITCHSTKS(bp)) {
		while ((lp = popline(CURSTK(bp))) != NULL) {
			lfree(lp,bp);
		}
	}

	/* clear the flags in the buffer */
	/* there may be a way to clean these less drastically, by
		using the information on the stacks above, but I
		couldn't figure it out.  -pgf  */
	lp = lforw(bp->b_line.l);
	while (lp != bp->b_line.l) {
		lsetnotcopied(lp);
		lp = lforw(lp);
	}

}

/* ARGSUSED */
int
undo(f,n)
int f,n;
{
	LINE *lp, *alp;
	int nopops = TRUE;
	
	if (b_val(curbp, MDVIEW))
		return(rdonly());

	while ((lp = popline(CURSTK(curbp))) != NULL) {
		nopops = FALSE;
#if NEWUNDO
		if (lismarkpatch(lp)) {
			register LINE *tlp;
			resetuline(lp->l_bp,lp->l_fp);
			tlp = lp->l_fp;
			lp->l_fp = lp->l_bp;
			lp->l_bp = tlp;
			pushline(lp,ALTSTK(curbp));
			continue;
		}
#endif
		if (lislinepatch(lp)) {
			patchstk(lp->l_bp, lp->l_fp);
			lfree(lp,curbp);
			continue;
		}
		lchange(WFHARD|WFINS|WFKILLS);
		if (lp->l_bp->l_fp != lp->l_fp) { /* theres something there */
			if (lp->l_bp->l_fp->l_fp == lp->l_fp) {
				/* then there is exactly one line there */
				/* alp is the line to remove */
				/* lp is the line we're putting in */
				alp = lp->l_bp->l_fp;
				repointstuff(lp,alp);
				/* remove it */
				lp->l_bp->l_fp = alp->l_fp;
				alp->l_fp->l_bp = alp->l_bp;
			} else { /* there is more than one line there */
				mlforce("BUG: no stacked line for an insert");
				/* cleanup ? naw, a bugs a bug */
				return(FALSE);
			}
		} else { /* there is no line where we're going */
			/* create an "unreal" tag line to push */
			alp = lalloc(-1,curbp);
			if (alp == NULL)
				return(FALSE);
			llength(alp) = LINENOTREAL;
			alp->l_fp = lp->l_fp;
			alp->l_bp = lp->l_bp;
		}

		/* insert real lines into the buffer 
			throw away the markers */
		if (lisreal(lp)) {
			lp->l_bp->l_fp = lp;
			lp->l_fp->l_bp = lp;
		} else {
			lfree(lp,curbp);
		}

		pushline(alp,ALTSTK(curbp));
	}
	
	if (nopops) {
		TTbeep();
		return (FALSE);
	}


	{
		/* it's an absolute move -- remember where we are */
		MARK odot;
		odot = DOT;

		DOT = CURDOT(curbp);
		if (DOT.o >= llength(DOT.l))
			DOT.o = llength(DOT.l);
		else if (DOT.o < firstchar(DOT.l))
			DOT.o = firstchar(DOT.l);

		/* if we moved, update the "last dot" mark */
		if (!sameline(DOT, odot)) {
			curwp->w_lastdot = odot;
		}
	}

	SWITCHSTKS(curbp);
	
	vverify("undo");
	
	return TRUE;
}

void
mayneedundo()
{
	needundocleanup = TRUE;
}

void
preundocleanup()
{
	freeundostacks(curbp);
	CURDOT(curbp) = curwp->w_dot;
	ALTDOT(curbp) = nullmark;
	needundocleanup = FALSE;
}

/* ARGSUSED */
int
lineundo(f,n)
int f,n;
{
	register LINE *ulp;	/* the Undo line */
	register LINE *lp;	/* the line we may replace */
	register WINDOW *wp;
	register char *ntext;

	ulp = curbp->b_ulinep;
	if (ulp == NULL) {
		TTbeep();
		return FALSE;
	}

	lp = ulp->l_nxtundo;

	if (ulp->l_fp != lp->l_fp ||
	    ulp->l_bp != lp->l_bp) {
	    	/* then the change affected more than one line */
		dumpuline(ulp);
		return FALSE;
	}

	/* avoid losing our undo stacks needlessly */
	if (linesmatch(ulp,lp) == TRUE) 
		return TRUE;

	curwp->w_dot.l = lp;
	preundocleanup();


	
	ntext = NULL;
	if (ulp->l_size && (ntext = malloc(ulp->l_size)) == NULL)
		return (FALSE);

	copy_for_undo(lp);

	if (ntext && lp->l_text) {
		memcpy(ntext, ulp->l_text, llength(ulp));
		ltextfree(lp,curbp);
	}
	lp->l_text = ntext;
	lp->l_used = ulp->l_used;
	lp->l_size = ulp->l_size;

#if ! WINMARK
	if (MK.l == lp)
		MK.o = 0;
#endif
	/* let's be defensive about this */
	wp = wheadp;
	while (wp != NULL) {
		if (wp->w_dot.l == lp)
			wp->w_dot.o = 0;
#if WINMARK
		if (wp->w_mark.l == lp)
			wp->w_mark.o = 0;
#endif
		if (wp->w_lastdot.l == lp)
			wp->w_lastdot.o = 0;
		wp = wp->w_wndp;
	}
	if (CURDOT(curbp).l == lp)
		CURDOT(curbp).o = 0;
	if (curbp->b_nmmarks != NULL) {
		/* fix the named marks */
		int i;
		struct MARK *mp;
		for (i = 0; i < 26; i++) {
			mp = &(curbp->b_nmmarks[i]);
			if (mp->l == lp)
				mp->o = 0;
		}
	}

	curwp->w_flag |= WFEDIT;
	
	vverify("lineundo");
	return TRUE;

}

void
repointstuff(nlp,olp)
register LINE *nlp,*olp;
{
	register WINDOW *wp;

	if (DOT.l == olp) {
		if (lisreal(nlp)) {
			DOT.l = nlp;
		} else {
			DOT.l = olp->l_fp;
		}
		DOT.o = 0;
	}
#if ! WINMARK
	if (MK.l == olp) {
		if (lisreal(nlp)) {
			MK.l = nlp;
		} else {
			MK.l = olp->l_fp;
		}
		MK.o = 0;
	}
#endif
	/* fix anything important that points to it */
	wp = wheadp;
	while (wp != NULL) {
		if (wp->w_line.l == olp)
			if (lisreal(nlp)) {
				wp->w_line.l = nlp;
			} else {
				wp->w_line.l = olp->l_fp;
			}
#if WINMARK
		if (wp->w_mark.l == olp) {
			if (lisreal(nlp)) {
				wp->w_mark.l = nlp;
			} else {
				wp->w_mark.l = olp->l_fp;
			}
			wp->w_mark.o = 0;
		}
#endif
		if (wp->w_lastdot.l == olp) {
			if (lisreal(nlp)) {
				wp->w_lastdot.l = nlp;
			} else {
				wp->w_lastdot.l = olp->l_fp;
			}
			wp->w_lastdot.o = 0;
		}
		wp = wp->w_wndp;
	}
#if 0
no code for ALTDOT, but this was ifdefed out before I put that in...  pgf
	if (ALTDOT(curbp).l == olp) {
		if (lisreal(nlp)) {
			ALTDOT(curbp).l = nlp;
		} else {
		    mlforce("BUG: preundodot points at newly inserted line!");
		}
	}
#endif
	if (CURDOT(curbp).l == olp) {
		if (lisreal(nlp)) {
			CURDOT(curbp).l = nlp;
		} else {
		    mlforce("BUG: preundodot points at newly inserted line!");
		}
	}
	if (curbp->b_nmmarks != NULL) {
		/* fix the named marks */
		int i;
		struct MARK *mp;
		for (i = 0; i < 26; i++) {
			mp = &(curbp->b_nmmarks[i]);
			if (mp->l == olp) {
				if (lisreal(nlp)) {
					mp->l = nlp;
					mp->o = 0;
				} else {
					mlforce("[Lost mark]");
				}
			}
		}
	}
#if !NEWUNDO
	resetuline(olp,nlp);
#endif
}

int
linesmatch(lp1,lp2)
register LINE *lp1,*lp2;
{
	if (llength(lp1) != llength(lp2))
		return FALSE;
	if (llength(lp1) == 0)
		return TRUE;
	return !memcmp(lp1->l_text, lp2->l_text, llength(lp1));
}

void
dumpuline(lp)
LINE *lp;
{
	if ((curbp->b_ulinep != NULL) &&
		    (curbp->b_ulinep->l_nxtundo == lp)) {
		lfree(curbp->b_ulinep,curbp);
		curbp->b_ulinep = NULL;
	}
}

void
setupuline(lp)
LINE *lp;
{
	/* take care of the U line */
	if ((curbp->b_ulinep == NULL) || (curbp->b_ulinep->l_nxtundo != lp)) {
		if (curbp->b_ulinep != NULL)
			lfree(curbp->b_ulinep,curbp);
		curbp->b_ulinep = copyline(lp);
		if (curbp->b_ulinep != NULL)
			curbp->b_ulinep->l_nxtundo = lp;
	}
}

void
resetuline(olp,nlp)
register LINE *olp,*nlp;
{
	if (curbp->b_ulinep != NULL && curbp->b_ulinep->l_nxtundo == olp) {
		if (lisreal(nlp)) {
			curbp->b_ulinep->l_nxtundo = nlp;
		} else {
			mlforce("BUG: b_ulinep pointed at inserted line!");
		}
	}
}

