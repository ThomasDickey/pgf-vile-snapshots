/* these routines take care of undo operations
 * code by Paul Fox, original algorithm mostly by Julia Harper May, 89
 *
 * $Log: undo.c,v $
 * Revision 1.50  1994/03/18 18:30:38  pgf
 * fixes for OPT_MAP_MEMORY compilation
 *
 * Revision 1.49  1994/03/18  12:00:07  pgf
 * added comments on copied-marks and cookies
 *
 * Revision 1.48  1994/03/16  10:55:56  pgf
 * switch over to cookie method of marking copied lines for undo
 *
 * Revision 1.47  1994/03/08  12:28:21  pgf
 * trial version of repointstuff() which attempts to preserve offsets of
 * marks attached to lines being replaced.
 *
 * Revision 1.46  1994/02/22  11:03:15  pgf
 * truncated RCS log for 4.0
 *
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
 * The "copied" cookie in the LINE structure is unioned with the stack link
 * pointer on the undo stack, since they aren't both needed at once.  (This
 * isn't true when OPT_MAP_MEMORY is set, because it creates an illegal
 * pointer value).
 *
 * There are basically three interface routines for code making buffer changes:
 *  toss_to_undo() -- called when deleting a whole line
 *  tag_for_undo() -- called when inserting a whole line
 *  copy_for_undo() -- called when modifying a line
 * These routines should be called _before_ calling chg_buff to mark the
 * buffer as modified, since they want to record the current
 * modified/unmodified state in the undo stack, so it can be restored later.
 *
 * In addition:
 *  freeundostacks() cleans up any undo data structures for a buffer, 
 *  nounmodifiable() is called if a change is happening to a
 *	buffer that is not undoable.
 *  mayneedundo() is called before starting an operation that might call
 *	one of the toss/copy/tag routines above.
 *  dumpuline() is called if the whole-line undo (the 'U' command) line
 *	may need to be flushed due to the current change.
 *
 * The command functions are:
 *  backundo() -- undo changes going back in history  (^X-u)
 *  forwredo() -- redo changes going forward in history  (^X-r)
 *  undo() -- undo changes, toggling backwards and forwards  (u)
 *  lineundo() -- undo all changes to currently-being-modified line (U)
 *	(The lineundo() command is kind of separate, and acts independently
 *	 of the normal undo stacks -- an extra copy of the line is simply
 *	 made when the first change is made.)
 *
 *
 * Notes on how the "copied" marks work:
 *
 * Say you do a change to a line, like you go into insertmode, and type
 * three characters.  The first character causes linsert to be called,
 * which calls copy_for_undo(), which copies the line, and marks it as
 * "copied".  Then, the second and third characters also call
 * copy_for_undo(), but since the line is marked, nothing happens.  Now you
 * hit ESC.  Now enter insertmode again.  So far, nothing has happened,
 * except that the "needundocleanup" flag has been set (by execute()s call
 * to mayneedundo()), since we're in an undo-able command.  Type a
 * character.  linsert() calls copy_for_undo() which calls preundocleanup()
 * (based on the "needundocleanup" flag.  In previous versions of vile,
 * this cleanup required walking the entire buffer, to reset the "copied"
 * flag.  Now, the "copied" flag is actually a word-sized "cookie", which
 * matches the global "current_undo_cookie" when the line has bee copied. 
 * By incrementing the global "current_undo_cookie" in preundocleanup(), we
 * are effectively resetting all of the lines' "marks", since the cookie is
 * now _guaranteed_ to not match against any of them.  Which is why, when
 * the cookie wraps around to 0, we _do_ need to clean the buffer, because
 * now there's a chance that the current_undo_cookie might match a very old
 * marked line.
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
static	void	preundocleanup P(( void ));
static	void	repointstuff P(( LINEPTR, LINEPTR ));
static	int	linesmatch P(( LINE *, LINE * ));
static	void	setupuline P(( LINEPTR ));

static	short	needundocleanup;

/* this could be per-buffer, but i don't think it matters in practice */
static unsigned short current_undo_cookie = 1;

/* #define UNDOLOG 1 */
#if UNDOLOG
static void undolog P(( char *, LINEPTR ));

static void
undolog(s,lp)
char *s;
LINEPTR lp;
{
	char *t;
	if (lisreal(lp))	t = "real";
	else if (lispurestacksep(lp))t= "purestacksep";
	else if (lisstacksep(lp))t= "stacksep";
	else if (lispatch(lp))	t = "patch";
	else 			t = "unknown";

	dbgwrite("%s %s lp 0x%x",s,t,lp);
}
#else
# define undolog(s,l)
#endif

/*
 * Test if the buffer is modifiable
 */
static int
OkUndo()
{
#define SCRATCH 1
#if SCRATCH
	if (b_is_scratch(curbp))
#else
	if (b_val(curbp, MDVIEW))
#endif
		return FALSE;
	return TRUE;
}

/* push a deleted line onto the undo stack. */
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
	if (same_ptr(next, buf_head(curbp))) {
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
			FORWDOT(curbp).o = b_left_margin(curbp);
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
 * redo stack) instead of at the copy, which will have just been popped,
 * unless we fix them by popping and using the patch.
 */

void
copy_for_undo(lp)
LINEPTR lp;
{
	fast_ptr LINEPTR nlp;

	if (!OkUndo())
		return;

	if (needundocleanup)
		preundocleanup();

	if (liscopied(l_ref(lp)))
		return;

	/* take care of the normal undo stack */
	nlp = copyline(l_ref(lp));
	if (same_ptr(nlp, null_ptr))
		return;

	pushline(nlp, BACKSTK(curbp));

	make_undo_patch(lp, nlp);

	lsetcopied(l_ref(lp));

	setupuline(lp);

	FORWDOT(curbp).l = lp;
	FORWDOT(curbp).o = DOT.o;
}

/* push an unreal line onto the undo stack
 * lp should be the new line, _after_ insertion, so
 *	lforw() and lback() are right
 */
void
tag_for_undo(lp)
LINEPTR lp;
{
	fast_ptr LINEPTR nlp;

	if (!OkUndo())
		return;

	if (needundocleanup)
		preundocleanup();

	if (liscopied(l_ref(lp)))
		return;

	nlp = lalloc(LINENOTREAL, curbp);
	if (same_ptr(nlp, null_ptr))
		return;
	set_lFORW(nlp, lFORW(lp));
	set_lBACK(nlp, lBACK(lp));

	pushline(nlp, BACKSTK(curbp));

	lsetcopied(l_ref(lp));
	FORWDOT(curbp).l = lp;
	FORWDOT(curbp).o = DOT.o;
}

/* Change all PURESTACKSEPS on the stacks to STACKSEPS, so that undo won't
 * reset the BFCHG bit.  This should be called anytime a non-undable change is
 * made to a buffer.
 */
void
nounmodifiable(bp)
BUFFER *bp;
{
	register LINE *tlp;
	for (tlp = l_ref(*BACKSTK(bp)); tlp != NULL;
				tlp = l_ref(tlp->l_nxtundo)) {
		if (lispurestacksep(tlp))
			tlp->l_used = STACKSEP;
	}
	for (tlp = l_ref(*FORWSTK(bp)); tlp != NULL;
				tlp = l_ref(tlp->l_nxtundo)) {
		if (lispurestacksep(tlp))
			tlp->l_used = STACKSEP;
	}
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
		bp->b_udtail = null_ptr;
		bp->b_udlastsep = null_ptr;
		bp->b_udcount = 0;
	}

}

/* ARGSUSED */
int
undo(f,n)
int f,n;
{
	int s;
	L_NUM	before;

	if (b_val(curbp, MDVIEW))
		return(rdonly());

	before = line_count(curbp);
	if ((s = undoworker(curbp->b_udstkindx)) == TRUE) {
		line_report(before);
		curbp->b_udstkindx ^= 1;  /* flip to other stack */
		vverify("undo");
	} 	
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
	if (++current_undo_cookie == 0) {
		current_undo_cookie++;	/* never let it be zero */
		for_each_line(lp, curbp) {  /* once in while, feel the pain */
			lsetnotcopied(lp);
		}
	}

	curbp->b_udstkindx = BACK;

	if (doingopcmd)
		BACKDOT(curbp) = pre_op_dot;
	else
		BACKDOT(curbp) = DOT;

	/* be sure FORWDOT has _some_ value (may be null the first time)
	if (sameline(FORWDOT(curbp), nullmark))
		FORWDOT(curbp) = BACKDOT(curbp);
	*/
	freshstack(BACK);
	FORWDOT(curbp) = BACKDOT(curbp);

	needundocleanup = FALSE;
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

/* get a line from the specified stack.  unless force'ing, don't
	go past a false bottom stack-separator */
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
	if (b_is_changed(curbp)) {
		plp = lalloc(STACKSEP, curbp);
	} else { /* if the buffer is unmodified, use special separator */
		plp = lalloc(PURESTACKSEP, curbp);

		/* and make sure there are no _other_ special separators */
		nounmodifiable(curbp);
	}
	if (same_ptr(plp, null_ptr))
		return;
	set_lBACK(plp, BACKDOT(curbp).l);
	l_ref(plp)->l_back_offs = BACKDOT(curbp).o;
	set_lFORW(plp, FORWDOT(curbp).l);
	l_ref(plp)->l_forw_offs = FORWDOT(curbp).o;
	pushline(plp, STACK(stkindx));
	if (stkindx == BACK) {
		l_ref(plp)->l_nextsep = null_ptr;
		if (same_ptr(curbp->b_udtail, null_ptr)) {
			if (curbp->b_udcount != 0) {
				mlwrite("BUG: null tail with non-0 undo count");
				curbp->b_udcount = 0;
			}
			curbp->b_udtail = plp;
			curbp->b_udlastsep = plp;
		} else {
			if (same_ptr(curbp->b_udlastsep, null_ptr)) {
				/* then we need to find lastsep */
				int i;
				curbp->b_udlastsep = curbp->b_udtail;
				for (i = curbp->b_udcount-1; i > 0; i-- ) {
					curbp->b_udlastsep =
					 l_ref(curbp->b_udlastsep)->l_nextsep;
				}
			}
			l_ref(curbp->b_udlastsep)->l_nextsep = plp;
			curbp->b_udlastsep = plp;
		}
		/* enforce stack growth limit */
		curbp->b_udcount++;
		/* dbgwrite("bumped undocount %d", curbp->b_udcount); */
		if ( b_val(curbp, VAL_UNDOLIM) != 0 &&
				curbp->b_udcount > b_val(curbp, VAL_UNDOLIM) ) {
			LINEPTR newtail;
			LINEPTR lp;

			newtail = curbp->b_udtail;
			while (curbp->b_udcount > b_val(curbp, VAL_UNDOLIM)) {
				newtail = l_ref(newtail)->l_nextsep;
				curbp->b_udcount--;
			}

			curbp->b_udtail = newtail;
			newtail = l_ref(newtail)->l_nxtundo;
			if (!same_ptr(newtail, null_ptr)) {
				do {
					lp = newtail;
					if (same_ptr(newtail,
							curbp->b_udlastsep))
						mlwrite("BUG: tail passed lastsep");
					newtail = l_ref(newtail)->l_nxtundo;
					lfree(lp,curbp);
				} while (!same_ptr(newtail, null_ptr));
			}
			l_ref(curbp->b_udtail)->l_nxtundo = null_ptr;

		}
	}
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

static int
undoworker(stkindx)
int stkindx;
{
	fast_ptr LINEPTR lp;
	fast_ptr LINEPTR alp;
	int nopops = TRUE;

	
	while ((l_ref(lp = l_ptr(popline(STACK(stkindx), FALSE)))) != 0) {
		if (nopops)  /* first pop -- establish a new stack base */
			freshstack(1^stkindx);
		nopops = FALSE;
		if (lislinepatch(l_ref(lp))) {
			applypatch(lBACK(lp), lFORW(lp));
			lfree(lp,curbp);
			continue;
		}
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
		mlwrite("[No changes to %s]",
				stkindx == BACK ? "undo" : "redo");
		/* dbgwrite("checking undocount %d", curbp->b_udcount); */
		if (stkindx == BACK && curbp->b_udcount != 0) {
			mlwrite("BUG: nopop, non-0 undo count");
		}
		TTbeep();
		return (FALSE);
	}

#define bug_checks 1
#ifdef bug_checks
	if ((l_ref(lp = l_ptr(peekline(STACK(stkindx))))) == 0) {
		mlforce("BUG: found null after undo/redo");
		return FALSE;
	}

	if (!lisstacksep(l_ref(lp))) {
		mlforce("BUG: found non-sep after undo/redo");
		return FALSE;
	}
#endif
	
	lp = l_ptr(popline(STACK(stkindx),TRUE));
	FORWDOT(curbp).l = lFORW(lp);
	FORWDOT(curbp).o = l_ref(lp)->l_forw_offs;
	BACKDOT(curbp).l = lBACK(lp);
	BACKDOT(curbp).o = l_ref(lp)->l_back_offs;
	if (stkindx == FORW) {
		/* if we moved, update the "last dot" mark */
		if (!sameline(DOT, FORWDOT(curbp)))
			curwp->w_lastdot = DOT;
		DOT = FORWDOT(curbp);
	} else {
		/* if we moved, update the "last dot" mark */
		if (!sameline(DOT, BACKDOT(curbp)))
			curwp->w_lastdot = DOT;
		DOT = BACKDOT(curbp);
		/* dbgwrite("about to decr undocount %d", curbp->b_udcount); */
		curbp->b_udcount--;
		curbp->b_udlastsep = null_ptr;  /* it's only a hint */
		if (same_ptr(curbp->b_udtail, lp)) {
			if (curbp->b_udcount != 0) {
				mlwrite("BUG: popped tail; non-0 undo count");
				curbp->b_udcount = 0;
			}
			/* dbgwrite("clearing tail 0x%x and lastsep 0x%x", curbp->b_udtail,
						curbp->b_udlastsep); */
			curbp->b_udtail = null_ptr;
			curbp->b_udlastsep = null_ptr;
		}
	}

	b_clr_counted(curbp);	/* don't know the size! */
	if (lispurestacksep(l_ref(lp)))
		unchg_buff(curbp, 0);
	else
		chg_buff(curbp, WFHARD|WFINS|WFKILLS);

	lfree(lp,curbp);

	return TRUE;
}

static void
setupuline(lp)
LINEPTR lp;
{
	register LINE *  ulp;
	/* take care of the U line */
	if ((ulp = l_ref(curbp->b_ulinep)) == NULL
	 || !same_ptr(ulp->l_nxtundo, lp)) {
		if (ulp != NULL)
			lfree(curbp->b_ulinep, curbp);
		ulp = l_ref(curbp->b_ulinep = copyline(l_ref(lp)));
		if (ulp != NULL)
			ulp->l_nxtundo = lp;
	}
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

	DOT.l = l_ptr(lp);

	preundocleanup();

#if !OPT_MAP_MEMORY
	ntext = NULL;
	if (ulp->l_size && (ntext = malloc((ALLOC_T)(ulp->l_size))) == NULL)
		return (FALSE);
#endif

	copy_for_undo(l_ptr(lp));

#if OPT_MAP_MEMORY
	if ((lp = l_reallocate(DOT.l, ulp->l_size, curbp)) == 0)
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
		MK.o = b_left_margin(curbp);
#endif
	/* let's be defensive about this */
	for_each_window(wp) {
		if (l_ref(wp->w_dot.l) == lp)
			wp->w_dot.o = b_left_margin(curbp);
#if WINMARK
		if (wp->w_mark.l == lp)
			wp->w_mark.o = b_left_margin(curbp);
#endif
		if (l_ref(wp->w_lastdot.l) == lp)
			wp->w_lastdot.o = b_left_margin(curbp);
	}
	if (curbp->b_nmmarks != NULL) {
		/* fix the named marks */
		int i;
		struct MARK *mp;
		for (i = 0; i < 26; i++) {
			mp = &(curbp->b_nmmarks[i]);
			if (l_ref(mp->l) == lp)
				mp->o = b_left_margin(curbp);
		}
	}

	chg_buff(curbp, WFEDIT|WFKILLS|WFINS);
	
	vverify("lineundo");
	return TRUE;

}

#ifdef BEFORE
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
#if ! WINMARK
	if (same_ptr(MK.l, olp)) {
		MK.l = point;
		MK.o = b_left_margin(curbp);
	}
#endif
	/* fix anything important that points to it */
	for_each_window(wp) {
		if (same_ptr(wp->w_dot.l, olp)) {
			wp->w_dot.l = point;
			wp->w_dot.o = b_left_margin(curbp);
		}
		if (same_ptr(wp->w_line.l, olp))
			wp->w_line.l = point;
#if WINMARK
		if (same_ptr(wp->w_mark.l, olp)) {
			wp->w_mark.l = point;
			wp->w_mark.o = b_left_margin(curbp);
		}
#endif
		if (same_ptr(wp->w_lastdot.l, olp)) {
			wp->w_lastdot.l = point;
			wp->w_lastdot.o = b_left_margin(curbp);
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
					mp->l = point;
					mp->o = b_left_margin(curbp);
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
			/* we lose the ability to undo all changes
				to this line, since it's going away */
			curbp->b_ulinep = null_ptr;
		}
	}

}
#else


#define _min(a,b) ((a) < (b)) ? (a) : (b)

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
#if ! WINMARK
	if (same_ptr(MK.l, olp)) {
		MK.l = point;
		MK.o = _min(MK.o, lLength(point));
	}
#endif
	/* fix anything important that points to it */
	for_each_window(wp) {
		if (same_ptr(wp->w_dot.l, olp)) {
			wp->w_dot.l = point;
			wp->w_dot.o = _min(wp->w_dot.o, lLength(point));
		}
		if (same_ptr(wp->w_line.l, olp))
			wp->w_line.l = point;
#if WINMARK
		if (same_ptr(wp->w_mark.l, olp)) {
			wp->w_mark.l = point;
			wp->w_mark.o = _min(wp->w_mark.o, llength(point));
		}
#endif
		if (same_ptr(wp->w_lastdot.l, olp)) {
			wp->w_lastdot.l = point;
			wp->w_lastdot.o = _min(wp->w_lastdot.o, lLength(point));
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
					mp->l = point;
					mp->o = _min(mp->o, lLength(point));
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
			/* we lose the ability to undo all changes
				to this line, since it's going away */
			curbp->b_ulinep = null_ptr;
		}
	}

}
#endif

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

