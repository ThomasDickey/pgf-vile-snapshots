/* these routines take care of undo operations
 * code by Paul Fox, original algorithm mostly by Julia Harper May, 89
 *
 * $Log: undo.c,v $
 * Revision 1.26  1993/06/02 14:28:47  pgf
 * see tom's 3.48 CHANGES
 *
 * Revision 1.25  1993/05/24  15:21:37  pgf
 * tom's 3.47 changes, part a
 *
 * Revision 1.24  1993/05/11  16:22:22  pgf
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
	(This isn't true when OPT_MAP_MEMORY is set, because it creates an
	illegal pointer value).

*/


#define CURSTK(bp) (&(bp->b_udstks[bp->b_udstkindx]))
#define ALTSTK(bp) (&(bp->b_udstks[1^(bp->b_udstkindx)]))

#define CURDOT(bp) bp->b_uddot[bp->b_udstkindx]
#define ALTDOT(bp) bp->b_uddot[1^(bp->b_udstkindx)]
#define SWITCHSTKS(bp) (bp->b_udstkindx = 1 ^ bp->b_udstkindx)

static	LINEPTR	copyline P(( LINE * ));
static	void	make_undo_patch P(( LINEPTR, LINEPTR ));
static	void	applypatch P(( LINEPTR, LINEPTR ));
static	void	pushline P(( LINEPTR, LINEPTR * ));
static	LINE *	popline P(( LINEPTR * ));
static	void	preundocleanup P(( void ));
static	void	repointstuff P(( LINEPTR, LINEPTR ));
static	int	linesmatch P(( LINE *, LINE * ));

static	short	needundocleanup;

/* push the line onto the right undo stack. */
void
toss_to_undo(lp)
LINEPTR lp;
{
	fast_ptr LINEPTR next;
	fast_ptr LINEPTR prev;

	if (curbp->b_flag & BFSCRTCH)
		return;
	if (needundocleanup)
		preundocleanup();
	pushline(lp, CURSTK(curbp));
	if (same_ptr(ALTDOT(curbp).l, null_ptr)
	 || same_ptr(ALTDOT(curbp).l, lp)) {
		int fc;
		next = lFORW(lp);

		/* need to save a dot -- either the next line or 
			the previous one */
		if (same_ptr(next, curbp->b_line.l)) {
			prev = lBACK(lp);
			ALTDOT(curbp).l = prev;
			fc =  firstchar(l_ref(prev));
			if (fc < 0) /* all white */
				ALTDOT(curbp).o = llength(l_ref(prev)) - 1;
			else
				ALTDOT(curbp).o = fc;
		} else {
			ALTDOT(curbp).l = next;
			fc =  firstchar(l_ref(next));
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
LINEPTR lp;
{
	fast_ptr LINEPTR nlp;
	register LINE *  ulp;

	if (curbp->b_flag & BFSCRTCH)
		return TRUE;
	if (needundocleanup)
		preundocleanup();

	if (liscopied(l_ref(lp)))
		return(TRUE);

	/* take care of the normal undo stack */
	nlp = copyline(l_ref(lp));
	if (same_ptr(nlp, null_ptr))
		return(FALSE);
	pushline(nlp, CURSTK(curbp));

	make_undo_patch(lp, nlp);

	lsetcopied(l_ref(lp));

	/* take care of the U line */
	if ((ulp = l_ref(curbp->b_ulinep)) == NULL
	 || !same_ptr(ulp->l_nxtundo, lp)) {
		if (ulp != NULL)
			lfree(curbp->b_ulinep, curbp);
		ulp = l_ref(curbp->b_ulinep = copyline(l_ref(lp)));
		if (ulp != NULL)
			ulp->l_nxtundo = lp;
	}

	if (same_ptr(ALTDOT(curbp).l, null_ptr)) {
		ALTDOT(curbp).l = lp;
		ALTDOT(curbp).o = curwp->w_dot.o;
	}
	return (TRUE);
}

/* push an unreal line onto the right undo stack */
/* lp should be the new line, _after_ insertion, so lforw() and lback() are right */
int
tag_for_undo(lp)
LINEPTR lp;
{
	fast_ptr LINEPTR nlp;

	if (curbp->b_flag & BFSCRTCH)
		return TRUE;
	if (needundocleanup)
		preundocleanup();

	if (liscopied(l_ref(lp)))
		return(TRUE);

	nlp = lalloc(LINENOTREAL, curbp);
	if (same_ptr(nlp, null_ptr))
		return(FALSE);
	set_lFORW(nlp, lFORW(lp));
	set_lBACK(nlp, lBACK(lp));
	pushline(nlp, CURSTK(curbp));
	lsetcopied(l_ref(lp));
	if (l_ref(ALTDOT(curbp).l) == NULL) {
		ALTDOT(curbp).l = lp;
		ALTDOT(curbp).o = curwp->w_dot.o;
	}
	return (TRUE);
}

static void
pushline(lp,stk)
LINEPTR lp;
LINEPTR *stk;
{
	l_ref(lp)->l_nxtundo = *stk;
	*stk = lp;
}

static LINE *
popline(stk)
LINEPTR *stk;
{
	LINE *lp;
	lp = l_ref(*stk);
	if (lp != NULL) {
		*stk = lp->l_nxtundo;
		lp->l_nxtundo = null_ptr;
	}
	return (lp);
}

static void
make_undo_patch(olp,nlp)
LINEPTR olp;
LINEPTR nlp;
{
	fast_ptr LINEPTR plp;
	/* push on a tag that matches up the copy with the original */
	plp = lalloc(LINEUNDOPATCH, curbp);
	if (same_ptr(plp, null_ptr))
		return;
	set_lFORW(plp, olp);	/* lforw() is the original line */
	set_lBACK(plp, nlp);	/* lback() is the copy */
	pushline(plp, CURSTK(curbp));
}

static void
applypatch(newlp,oldlp)
LINEPTR newlp;
LINEPTR oldlp;
{
	register LINE *tlp;
	for (tlp = l_ref(*CURSTK(curbp)); tlp != NULL ; tlp = l_ref(tlp->l_nxtundo)) {
		if (!lispatch(tlp)) {
			if (lforw(tlp) == l_ref(oldlp))
				set_lForw(tlp, newlp);
			if (lback(tlp) == l_ref(oldlp))
				set_lBack(tlp, newlp);
		}
	}
}

static LINEPTR
copyline(lp)
register LINE *lp;
{
	register LINE *nlp;
	
	nlp = l_ref(lalloc(lp->l_used,curbp));
	if (nlp == NULL)
		return null_ptr;
	/* copy the text and forward and back pointers.  everything else 
		matches already */
	set_lforw(nlp, lforw(lp));
	set_lback(nlp, lback(lp));
	/* copy the rest */
	if (lp->l_text && nlp->l_text)
		(void)memcpy(nlp->l_text, lp->l_text, (SIZE_T)lp->l_used);
	return l_ptr(nlp);
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
			lfree(l_ptr(lp),bp);
		}
	}

	/* clear the flags in the buffer */
	/* there may be a way to clean these less drastically, by
		using the information on the stacks above, but I
		couldn't figure it out.  -pgf  */
	lp = lForw(bp->b_line.l);
	while (lp != l_ref(bp->b_line.l)) {
		lsetnotcopied(lp);
		lp = lforw(lp);
	}

}

/* ARGSUSED */
int
undo(f,n)
int f,n;
{
	fast_ptr LINEPTR lp;
	fast_ptr LINEPTR alp;
	int nopops = TRUE;
	
	if (b_val(curbp, MDVIEW))
		return(rdonly());

	while ((l_ref(lp = l_ptr(popline(CURSTK(curbp))))) != 0) {
		nopops = FALSE;
		if (lislinepatch(l_ref(lp))) {
			applypatch(lBACK(lp), lFORW(lp));
			lfree(lp,curbp);
			continue;
		}
		chg_buff(curbp, WFHARD|WFINS|WFKILLS);
		if (lforw(lBack(lp)) != lForw(lp)) { /* theres something there */
			if (lforw(lforw(lBack(lp))) == lForw(lp)) {
				/* then there is exactly one line there */
				/* alp is the line to remove */
				/* lp is the line we're putting in */
				alp = lFORW(lBACK(lp));
				repointstuff(lp,alp);
				/* remove it */
				set_lforw(lBack(lp), lForw(alp));
				set_lback(lForw(alp), lBack(alp));
			} else { /* there is more than one line there */
				mlforce("BUG: no stacked line for an insert");
				/* cleanup ? naw, a bugs a bug */
				return(FALSE);
			}
		} else { /* there is no line where we're going */
			/* create an "unreal" tag line to push */
			alp = lalloc(LINENOTREAL, curbp);
			if (same_ptr(alp, null_ptr))
				return(FALSE);
			set_lFORW(alp, lFORW(lp));
			set_lBACK(alp, lBACK(lp));
		}

		/* insert real lines into the buffer 
			throw away the markers */
		if (lisreal(l_ref(lp))) {
			set_lFORW(lBACK(lp), lp);
			set_lBACK(lFORW(lp), lp);
		} else {
			lfree(lp, curbp);
		}

		pushline(alp, ALTSTK(curbp));
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
		DOT.o = firstchar(l_ref(DOT.l));
		if (DOT.o < 0) {
			DOT.o = lLength(DOT.l) - 1;
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

static void
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
#if !OPT_MAP_MEMORY
	register char *ntext;
#endif

	ulp = l_ref(curbp->b_ulinep);
	if (ulp == NULL) {
		TTbeep();
		return FALSE;
	}

	lp = l_ref(ulp->l_nxtundo);

	if (lforw(ulp) != lforw(lp) ||
	    lback(ulp) != lback(lp)) {
	    	/* then the change affected more than one line */
		dumpuline(curbp->b_ulinep);
		return FALSE;
	}

	/* avoid losing our undo stacks needlessly */
	if (linesmatch(ulp,lp) == TRUE) 
		return TRUE;

	curwp->w_dot.l = l_ptr(lp);
	preundocleanup();

#if !OPT_MAP_MEMORY
	ntext = NULL;
	if (ulp->l_size && (ntext = malloc(ulp->l_size)) == NULL)
		return (FALSE);
#endif

	copy_for_undo(l_ptr(lp));

#if OPT_MAP_MEMORY
	if ((lp = l_reallocate(curwp->w_dot.l, ulp->l_size, curbp)) == 0)
		return (FALSE);
	lp->l_used = ulp->l_used;
	(void)memcpy(lp->l_text, ulp->l_text, (SIZE_T)llength(ulp));
#else
	if (ntext && lp->l_text) {
		(void)memcpy(ntext, ulp->l_text, (SIZE_T)llength(ulp));
		ltextfree(lp,curbp);
	}

	lp->l_text = ntext;
	lp->l_used = ulp->l_used;
	lp->l_size = ulp->l_size;
#endif

#if ! WINMARK
	if (l_ref(MK.l) == lp)
		MK.o = 0;
#endif
	/* let's be defensive about this */
	for_each_window(wp) {
		if (l_ref(wp->w_dot.l) == lp)
			wp->w_dot.o = 0;
#if WINMARK
		if (wp->w_mark.l == lp)
			wp->w_mark.o = 0;
#endif
		if (l_ref(wp->w_lastdot.l) == lp)
			wp->w_lastdot.o = 0;
	}
	if (l_ref(CURDOT(curbp).l) == lp)
		CURDOT(curbp).o = 0;
	if (curbp->b_nmmarks != NULL) {
		/* fix the named marks */
		int i;
		struct MARK *mp;
		for (i = 0; i < 26; i++) {
			mp = &(curbp->b_nmmarks[i]);
			if (l_ref(mp->l) == lp)
				mp->o = 0;
		}
	}

	chg_buff(curbp, WFEDIT|WFKILLS|WFINS);
	
	vverify("lineundo");
	return TRUE;

}

static void
repointstuff(nlp,olp)
fast_ptr LINEPTR nlp;
fast_ptr LINEPTR olp;
{
	register WINDOW *wp;
	int	usenew = lisreal(l_ref(nlp));
	fast_ptr LINEPTR point;
	register LINE *  ulp;

	point = usenew ? nlp : lFORW(olp);
	if (same_ptr(DOT.l, olp)) {
		DOT.l = point;
		DOT.o = 0;
	}
#if ! WINMARK
	if (same_ptr(MK.l, olp)) {
		MK.l = point;
		MK.o = 0;
	}
#endif
	/* fix anything important that points to it */
	for_each_window(wp) {
		if (same_ptr(wp->w_line.l, olp))
			wp->w_line.l = point;
#if WINMARK
		if (same_ptr(wp->w_mark.l, olp)) {
			wp->w_mark.l = point;
			wp->w_mark.o = 0;
		}
#endif
		if (same_ptr(wp->w_lastdot.l, olp)) {
			wp->w_lastdot.l = point;
			wp->w_lastdot.o = 0;
		}
	}
	if (same_ptr(CURDOT(curbp).l, olp)) {
		if (usenew) {
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
			if (same_ptr(mp->l, olp)) {
				if (usenew) {
					mp->l = nlp;
					mp->o = 0;
				} else {
					mlforce("[Lost mark]");
				}
			}
		}
	}

	/* reset the uline */
	if ((ulp = l_ref(curbp->b_ulinep)) != NULL
	 && same_ptr(ulp->l_nxtundo, olp)) {
		if (usenew) {
			ulp->l_nxtundo = nlp;
		} else {
			mlforce("BUG: b_ulinep pointed at inserted line!");
		}
	}

}

static int
linesmatch(lp1,lp2)
register LINE *lp1,*lp2;
{
	if (llength(lp1) != llength(lp2))
		return FALSE;
	if (llength(lp1) == 0)
		return TRUE;
	return !memcmp(lp1->l_text, lp2->l_text, (SIZE_T)llength(lp1));
}

void
dumpuline(lp)
LINEPTR lp;
{
	register LINE *ulp = l_ref(curbp->b_ulinep);

	if ((ulp != NULL) && same_ptr(ulp->l_nxtundo, lp)) {
		lfree(curbp->b_ulinep, curbp);
		curbp->b_ulinep = null_ptr;
	}
}
