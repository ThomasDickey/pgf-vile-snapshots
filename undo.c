
/* these routines take care of undo operations
 * code by Paul Fox, original algorithm mostly by Julia Harper May, 89
 *
 * $Log: undo.c,v $
 * Revision 1.24  1993/05/11 16:22:22  pgf
 * see tom's CHANGES, 3.46
 *
 * Revision 1.23  1993/04/20  12:18:32  pgf
 * see tom's 3.43 CHANGES
 *
 * Revision 1.22  1993/01/23  13:38:23  foxharp
 * lchange is now chg_buff
 *
 * Revision 1.21  1993/01/16  10:43:22  foxharp
 * use new macros
 *
 * Revision 1.20  1992/12/04  09:14:36  foxharp
 * deleted unused assigns
 *
 * Revision 1.19  1992/11/30  23:07:03  foxharp
 * firstchar/lastchar now return -1 for no non-white chars on line
 *
 * Revision 1.18  1992/07/28  22:02:55  foxharp
 * took out crufty ifdefs and commented-out code.  reworded some commentary.
 *
 * Revision 1.17  1992/05/16  12:00:31  pgf
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
		int fc;
		/* need to save a dot -- either the next line or 
			the previous one */
		if (lforw(lp) == curbp->b_line.l) {
			ALTDOT(curbp).l = lback(lp);
			fc =  firstchar(lback(lp));
			if (fc < 0) /* all white */
				ALTDOT(curbp).o = llength(lback(lp)) - 1;
			else
				ALTDOT(curbp).o = fc;
		} else {
			ALTDOT(curbp).l = lforw(lp);
			fc =  firstchar(lforw(lp));
			if (fc < 0) /* all white */
				ALTDOT(curbp).o = 0;
			else
				ALTDOT(curbp).o = fc;
		}
	}
	dumpuline(lp);
}

/* 
 * Push a copy of a line onto the right undo stack.  Push a patch so we can
 * later fix up any references to this line that might already be in the
 * stack.  When the undo happens, the later pops (i.e. those lines still
 * in the stack) will point at the _original_ (which will by then be on the
 * other undo stack) instead of the copy, which will have just been popped. 
 * unless we fix them by when popping the patch.
 */

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
/* lp should be the new line, _after_ insertion, so lforw() and lback() are right */
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
	set_lforw(nlp, lforw(lp));
	set_lback(nlp, lback(lp));
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
	set_lforw(plp, olp);	/* lforw() is the original line */
	set_lback(plp, nlp);	/* lback() is the copy */
	pushline(plp,CURSTK(curbp));
}

void
applypatch(newlp,oldlp)
LINE *newlp, *oldlp;
{
	register LINE *tlp;
	for (tlp = *CURSTK(curbp); tlp != NULL ; tlp = tlp->l_nxtundo) {
		if (!lispatch(tlp)) {
			if (lforw(tlp) == oldlp)
				set_lforw(tlp, newlp);
			if (lback(tlp) == oldlp)
				set_lback(tlp, newlp);
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
	set_lforw(nlp, lforw(lp));
	set_lback(nlp, lback(lp));
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
		if (lislinepatch(lp)) {
			applypatch(lback(lp), lforw(lp));
			lfree(lp,curbp);
			continue;
		}
		chg_buff(curbp, WFHARD|WFINS|WFKILLS);
		if (lforw(lback(lp)) != lforw(lp)) { /* theres something there */
			if (lforw(lforw(lback(lp))) == lforw(lp)) {
				/* then there is exactly one line there */
				/* alp is the line to remove */
				/* lp is the line we're putting in */
				alp = lforw(lback(lp));
				repointstuff(lp,alp);
				/* remove it */
				set_lforw(lback(lp), lforw(alp));
				set_lback(lforw(alp), lback(alp));
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
			set_lforw(alp, lforw(lp));
			set_lback(alp, lback(lp));
		}

		/* insert real lines into the buffer 
			throw away the markers */
		if (lisreal(lp)) {
			set_lforw(lback(lp), lp);
			set_lback(lforw(lp), lp);
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
		DOT.o = firstchar(DOT.l);
		if (DOT.o < 0) {
			DOT.o = llength(DOT.l) - 1;
			if (DOT.o < 0)
				DOT.o = 0;
		}

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

	if (lforw(ulp) != lforw(lp) ||
	    lback(ulp) != lback(lp)) {
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
	for_each_window(wp) {
		if (wp->w_dot.l == lp)
			wp->w_dot.o = 0;
#if WINMARK
		if (wp->w_mark.l == lp)
			wp->w_mark.o = 0;
#endif
		if (wp->w_lastdot.l == lp)
			wp->w_lastdot.o = 0;
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

	chg_buff(curbp, WFEDIT|WFKILLS|WFINS);
	
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
			DOT.l = lforw(olp);
		}
		DOT.o = 0;
	}
#if ! WINMARK
	if (MK.l == olp) {
		if (lisreal(nlp)) {
			MK.l = nlp;
		} else {
			MK.l = lforw(olp);
		}
		MK.o = 0;
	}
#endif
	/* fix anything important that points to it */
	for_each_window(wp) {
		if (wp->w_line.l == olp)
			if (lisreal(nlp)) {
				wp->w_line.l = nlp;
			} else {
				wp->w_line.l = lforw(olp);
			}
#if WINMARK
		if (wp->w_mark.l == olp) {
			if (lisreal(nlp)) {
				wp->w_mark.l = nlp;
			} else {
				wp->w_mark.l = lforw(olp);
			}
			wp->w_mark.o = 0;
		}
#endif
		if (wp->w_lastdot.l == olp) {
			if (lisreal(nlp)) {
				wp->w_lastdot.l = nlp;
			} else {
				wp->w_lastdot.l = lforw(olp);
			}
			wp->w_lastdot.o = 0;
		}
	}
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
	resetuline(olp,nlp);
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

