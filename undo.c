/* these routines take care of undo operations
 * code by Paul Fox, original algorithm mostly by Julia Harper May, 89
 *
 * $Log: undo.c,v $
 * Revision 1.28  1993/06/22 10:28:23  pgf
 * implemented infinite undo
 *
 * Revision 1.27  1993/06/18  15:57:06  pgf
 * tom's 3.49 changes
 *
 * Revision 1.26  1993/06/02  14:28:47  pgf
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

/*
 * There are two stacks and two "dots" saved for undo operations.
 *
 * The first stack, the "backstack", or the "undo stack", is used to move
 * back in history -- that is, as changes are made, they go on this stack,
 * and as we undo those changes, they come off of this stack.  The second
 * stack, the "forward stack" or "redo stack", is used when going forward
 * in history, i.e., when undoing an undo.
 *
 * The "backdot" is the last value of DOT _before_ a change occurred, and
 * therefore must be restored _after_ undoing that change.  The "forwdot"
 * is the first value of DOT _after_ a change occurred, and therefore must
 * be restored _after_ re-doing that change.
 *
 * Distinct sets of changes (i.e.  single user-operations) are separated on
 * the stacks by "stack separator" (natch) entries.  The lforw and lback
 * pointers of these entries are used to save the values of the forward and
 * backward dots.
 *
 * The undo strategy is this:
 *
 * 1) For any deleted line, push it onto the undo stack.
 *
 * 2) On any change to a line, make a copy of it, push the copy to the undo
 * stack, and mark the original as having been copied.  Do not copy/push
 * lines that are already marked as having been copied.  Push a tag
 * matching up the copy with the original.  Later, when undoing, and the
 * copy is being put into the file, we can go down through the undo stack,
 * find all references to the original line, and make them point at the
 * copy instead.  We could do this immediately, instead of pushing the
 * "patch" and waiting for the undo, but it seems better to pay the price
 * of that stack walk at undo time, rather than at the time of the change. 
 * This patching wouldn't be necessary at all if we used line no's as the
 * pointers, instead of real pointers.
 *
 * On the actual undo, we pop these things (lines or tags) one by one. 
 * There should be either a) no lines where it goes (it was a deleted line)
 * and we can just put it back, or b) exactly one line where it goes (it
 * was a changed/copied line) and it can be replaced, or if this is a tag
 * we're popping (not a real line), then the line it points at was a
 * fresh insert (and should be deleted now).  That makes it easy to undo
 * the changes one by one.  Of course, we need to build a different,
 * inverse stack (the "forward", or "redo" stack) as we go, so that undo
 * can itself be undone.
 *
 * The "copied" flag in the LINE structure is unioned with the stack link
 * pointer on the undo stack, since they aren't both needed at once.  (This
 * isn't true when OPT_MAP_MEMORY is set, because it creates an illegal
 * pointer value).
 *
 */

#define FORW 0
#define BACK 1

/* shorthand for the two stacks and the two dots */
#define BACKSTK(bp) (&(bp->b_udstks[BACK]))
#define FORWSTK(bp) (&(bp->b_udstks[FORW]))
#define BACKDOT(bp) (bp->b_uddot[BACK])
#define FORWDOT(bp) (bp->b_uddot[FORW])

/* these let us refer to the current and other stack in relative terms */
#define STACK(i)	(&(curbp->b_udstks[i]))
#define OTHERSTACK(i)	(&(curbp->b_udstks[1^i]))

static	int	OkUndo P(( void ));
static	LINEPTR	copyline P(( LINE * ));
static	void	make_undo_patch P(( LINEPTR, LINEPTR ));
static	void	freshstack P(( int ));
static	void	applypatch P(( LINEPTR, LINEPTR ));
static	void	pushline P(( LINEPTR, LINEPTR * ));
static	LINE *	popline P(( LINEPTR *, int ));
static	LINE *	peekline P(( LINEPTR * ));
static	int	undoworker P(( int ));
static	void	fixupdot P(( LINEPTR ));
static	void	preundocleanup P(( void ));
static	void	repointstuff P(( LINEPTR, LINEPTR ));
static	int	linesmatch P(( LINE *, LINE * ));

static	short	needundocleanup;

/* #define UNDOLOG 1 */
#if UNDOLOG
void undolog P(( char *, LINEPTR ));

void
undolog(s,lp)
char *s;
LINEPTR lp;
{
	char *t;
	if (lisreal(lp))	t = "real";
	else if (lisstacksep(lp))t= "stacksep";
	else if (lispatch(lp))	t = "patch";
	else 			t = "unknown";

	dbgwrite("%s %s lp 0x%x",s,t,lp);
}
#else
# define undolog(s,l)
#endif

/* Test if the buffer is modifiable; if so setup for undo and return true.
 */
static int
OkUndo()
{
#define SCRATCH 1
#if SCRATCH
	if (curbp->b_flag & BFSCRTCH)
#else
	if (b_val(curbp, MDVIEW))
#endif
		return FALSE;
	return TRUE;
}

/* push the line onto the undo stack. */
void
toss_to_undo(lp)
LINEPTR lp;
{
	fast_ptr LINEPTR next;
	fast_ptr LINEPTR prev;
	int fc;

	if (!OkUndo())
		return;

	if (needundocleanup)
		preundocleanup();

	pushline(lp, BACKSTK(curbp));

	next = lFORW(lp);

	/* need to save a dot -- either the next line or 
		the previous one */
	if (same_ptr(next, curbp->b_line.l)) {
		prev = lBACK(lp);
		FORWDOT(curbp).l = prev;
		fc =  firstchar(l_ref(prev));
		if (fc < 0) /* all white */
			FORWDOT(curbp).o = llength(l_ref(prev)) - 1;
		else
			FORWDOT(curbp).o = fc;
	} else {
		FORWDOT(curbp).l = next;
		fc =  firstchar(l_ref(next));
		if (fc < 0) /* all white */
			FORWDOT(curbp).o = 0;
		else
			FORWDOT(curbp).o = fc;
	}

	dumpuline(lp);
}

/* 
 * Push a copy of a line onto the undo stack.  Push a patch so we can
 * later fix up any references to this line that might already be in the
 * stack.  When the undo happens, the later pops (i.e. those lines still
 * in the stack) will point at the _original_ (which will by then be on the
 * redo stack) instead of the copy, which will have just been popped. 
 * unless we fix them by when popping the patch.
 */

int
copy_for_undo(lp)
LINEPTR lp;
{
	fast_ptr LINEPTR nlp;
	register LINE *  ulp;

	if (!OkUndo())
		return TRUE;

	if (needundocleanup)
		preundocleanup();

	if (liscopied(l_ref(lp)))
		return(TRUE);

	/* take care of the normal undo stack */
	nlp = copyline(l_ref(lp));
	if (same_ptr(nlp, null_ptr))
		return(FALSE);

	pushline(nlp, BACKSTK(curbp));

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

	FORWDOT(curbp).l = lp;
	FORWDOT(curbp).o = curwp->w_dot.o;
	return (TRUE);
}

/* push an unreal line onto the undo stack
 * lp should be the new line, _after_ insertion, so
 *	lforw() and lback() are right
 */
int
tag_for_undo(lp)
LINEPTR lp;
{
	fast_ptr LINEPTR nlp;

	if (!OkUndo())
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

	pushline(nlp, BACKSTK(curbp));

	lsetcopied(l_ref(lp));
	FORWDOT(curbp).l = lp;
	FORWDOT(curbp).o = curwp->w_dot.o;
	return (TRUE);
}

static void
pushline(lp,stk)
LINEPTR lp;
LINEPTR *stk;
{
	l_ref(lp)->l_nxtundo = *stk;
	*stk = lp;
	undolog("pushing", lp);
}

static LINE *
popline(stkp,force)
LINEPTR *stkp;
int force;
{
	LINE *lp;

	lp = l_ref(*stkp);

	if (lp == NULL || (!force && lisstacksep(lp))) {
		undolog("popping null",lp);
		return NULL;
	}

	*stkp = lp->l_nxtundo;
	lp->l_nxtundo = null_ptr;
	undolog("popped", lp);
	return (lp);
}

static LINE *
peekline(stkp)
LINEPTR *stkp;
{
	return l_ref(*stkp);
}

static void
freshstack(stkindx)
int stkindx;
{
	fast_ptr LINEPTR plp;
	/* push on a stack delimiter, so we know where this undo ends */
	plp = lalloc(STACKSEP, curbp);
	if (same_ptr(plp, null_ptr))
		return;
	set_lBACK(plp, BACKDOT(curbp).l);
	set_lFORW(plp, FORWDOT(curbp).l);
	pushline(plp, STACK(stkindx));
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
	pushline(plp, BACKSTK(curbp));
}

static void
applypatch(newlp,oldlp)
LINEPTR newlp;
LINEPTR oldlp;
{
	register LINE *tlp;
	for (tlp = l_ref(*BACKSTK(curbp)); tlp != NULL;
					tlp = l_ref(tlp->l_nxtundo)) {
		if (!lispatch(tlp)) {
			if (lforw(tlp) == l_ref(oldlp))
				set_lForw(tlp, newlp);
			if (lback(tlp) == l_ref(oldlp))
				set_lBack(tlp, newlp);
		} else { /* it's a patch */
			if (lforw(tlp) == l_ref(oldlp)) {
				set_lForw(tlp, newlp);
			}
			if (lback(tlp) == l_ref(oldlp)) {
					/* set_lBack(tlp, newlp); */
				mlforce("BUG? copy is an old line");
				break;
			}
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
freeundostacks(bp,both)
register BUFFER *bp;
int both;
{
	register LINE *lp;

	while ((lp = popline(FORWSTK(bp),TRUE)) != NULL) {
		lfree(l_ptr(lp),bp);
	}
	if (both) {
		while ((lp = popline(BACKSTK(bp),TRUE)) != NULL)
			lfree(l_ptr(lp),bp);
	}

}

/* ARGSUSED */
int
undo(f,n)
int f,n;
{
	int s;

	if (b_val(curbp, MDVIEW))
		return(rdonly());

	s = undoworker(curbp->b_udstkindx);
	if (!s)
		return s;

	curbp->b_udstkindx ^= 1;  /* flip to other stack */

	vverify("undo");
	
	return s;
}

int
backundo(f,n)
int f,n;
{
	int s = TRUE;

	if (b_val(curbp, MDVIEW))
		return(rdonly());

	if (!f || n < 1) n = 1;

	while (n--) {
		s = undoworker(BACK);
		if (!s)
			return s;
	}

	curbp->b_udstkindx = FORW;  /* flip to other stack */

	vverify("undo");
	
	return s;
}

int
forwredo(f,n)
int f,n;
{
	int s = TRUE;

	if (b_val(curbp, MDVIEW))
		return(rdonly());

	if (!f || n < 1) n = 1;

	while (n--) {
		s = undoworker(FORW);
		if (!s)
			return s;
	}

	curbp->b_udstkindx = BACK;  /* flip to other stack */

	vverify("undo");
	
	return s;
}

int
undoworker(stkindx)
int stkindx;
{
	fast_ptr LINEPTR lp;
	fast_ptr LINEPTR alp;
	int nopops = TRUE;

	
	while ((l_ref(lp = l_ptr(popline(STACK(stkindx), FALSE)))) != 0) {
		if (nopops)
			freshstack(1^stkindx);
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

		pushline(alp, OTHERSTACK(stkindx));
	}

	if (nopops) {
		TTbeep();
		return (FALSE);
	}

#define bug_checks 1
#ifdef bug_checks
	if ((l_ref(lp = l_ptr(peekline(STACK(stkindx))))) == 0) {
		mlforce("BUG: found null after undo/redo");
		return FALSE;
	}

	if (!lisstacksep(lp)) {
		mlforce("BUG: found non-sep after undo/redo");
		return FALSE;
	}
#endif
	
	(void)popline(STACK(stkindx),TRUE);
	FORWDOT(curbp).l = lforw(lp);
	BACKDOT(curbp).l = lback(lp);
	if (stkindx == FORW) {
		fixupdot(lforw(lp));
	} else {
		fixupdot(lback(lp));
	}
	lfree(lp,curbp);

	return TRUE;
}

static void
fixupdot(dotl)
LINEPTR dotl;
{
	/* it's an absolute move -- remember where we are */
	MARK odot;
	odot = DOT;

	DOT.l = dotl;
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

void
mayneedundo()
{
	needundocleanup = TRUE;
}

static void
preundocleanup()
{
	register LINE *lp;

	freeundostacks(curbp,FALSE);

	/* clear the flags in the buffer */
	/* there may be a way to clean these less drastically, by
		using the information on the stacks above, but I
		couldn't figure it out.  -pgf  */
	for_each_line(lp, curbp) {
		lsetnotcopied(lp);
	}

	curbp->b_udstkindx = BACK;

	BACKDOT(curbp) = DOT;
	if (sameline(FORWDOT(curbp), nullmark))
		FORWDOT(curbp) = DOT;
	freshstack(BACK);
	FORWDOT(curbp) = DOT;

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
	    	/* then the last change affected more than one line,
			and we can't use the saved U-line */
		dumpuline(curbp->b_ulinep);
		TTbeep();
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
#ifdef BEFORE
/* need to FIXXX  -- this BACKDOT is really now on the stack in the stacksep */
	if (l_ref(BACKDOT(curbp).l) == lp)
		BACKDOT(curbp).o = 0;
#endif
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
#ifdef BEFORE
	if (same_ptr(BACKDOT(curbp).l, olp)) {
		if (usenew) {
			BACKDOT(curbp).l = point;
		} else {
		    mlforce("BUG: preundodot points at newly inserted line!");
		}
	}
#endif
	if (curbp->b_nmmarks != NULL) {
		/* fix the named marks */
		int i;
		struct MARK *mp;
		for (i = 0; i < 26; i++) {
			mp = &(curbp->b_nmmarks[i]);
			if (same_ptr(mp->l, olp)) {
				if (usenew) {
					mp->l = point;
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
			ulp->l_nxtundo = point;
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
