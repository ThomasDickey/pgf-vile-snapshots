
/* these routines take care of undo operations */
/* code by Paul Fox, original algorithm mostly by Julia Harper May, 89 */

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
#define CURDOTP(bp) (bp->b_uddotps[bp->b_udstkindx])
#define ALTDOTP(bp) (bp->b_uddotps[1^(bp->b_udstkindx)])
#define CURDOTO(bp) (bp->b_uddotos[bp->b_udstkindx])
#define ALTDOTO(bp) (bp->b_uddotos[1^(bp->b_udstkindx)])
#define SWITCHSTKS(bp) (bp->b_udstkindx = 1 ^ bp->b_udstkindx)

short needundocleanup;
LINE *copyline();

/* push the line onto the right undo stack. */
toss_to_undo(lp)
LINE *lp;
{
	if (needundocleanup)
		preundocleanup();
	pushline(lp,CURSTK(curbp));
	if ((ALTDOTP(curbp) == NULL) || (ALTDOTP(curbp) == lp)) {
		/* need to save a dot -- either the next line or 
			the previous one */
		if (lp->l_fp == curbp->b_linep) {
			ALTDOTP(curbp) = lp->l_bp;
			ALTDOTO(curbp) = firstchar(lp->l_bp);
		} else {
			ALTDOTP(curbp) = lp->l_fp;
			ALTDOTO(curbp) = firstchar(lp->l_fp);
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
copy_for_undo(lp)
LINE *lp;
{
	register LINE *nlp;

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

	if (ALTDOTP(curbp) == NULL) {
		ALTDOTP(curbp) = lp;
		ALTDOTO(curbp) = curwp->w_doto;
	}
	return (TRUE);
}

/* push an unreal line onto the right undo stack */
/* lp should be the new line, _after_ insertion, so l_fp and l_bp are right */
tag_for_undo(lp)
LINE *lp;
{
	register LINE *nlp;

	if (needundocleanup)
		preundocleanup();

	if (liscopied(lp))
		return(TRUE);

	nlp = lalloc(-1);
	if (nlp == NULL)
		return(FALSE);
	llength(nlp) = LINENOTREAL;
	nlp->l_fp = lp->l_fp;
	nlp->l_bp = lp->l_bp;
	pushline(nlp,CURSTK(curbp));
	lsetcopied(lp);
	if (ALTDOTP(curbp) == NULL) {
		    ALTDOTP(curbp) = lp;
		    ALTDOTO(curbp) = curwp->w_doto;
	}
	return (TRUE);
}

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

make_undo_patch(olp,nlp,type)
LINE *olp,*nlp;
{
	register LINE *plp;
	/* push on a tag that matches up the copy with the original */
	plp = lalloc(-1);
	if (plp == NULL)
		return(FALSE);
	llength(plp) = type;
	plp->l_fp = olp;	/* l_fp is the original line */
	plp->l_bp = nlp;	/* l_bp is the copy */
	pushline(plp,CURSTK(curbp));
}

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
	int i;
	register LINE *nlp;
	
	nlp = lalloc(lp->l_used);
	if (nlp == NULL)
		return(NULL);
	/* copy the text and forward and back pointers.  everything else 
		matches already */
	nlp->l_fp = lp->l_fp;
	nlp->l_bp = lp->l_bp;
	/* copy the rest */
	memcpy(nlp->l_text, lp->l_text, llength(lp));
	return nlp;
}


/* before any undoable command (except undo itself), clean the undo list */
/* clean the copied flag on the line we're the copy of */
freeundostacks(bp)
register BUFFER *bp;
{
	register LINE *lp;
	int i;

	for (i = 0; i <= 1; i++, SWITCHSTKS(bp)) {
		while ((lp = popline(CURSTK(bp))) != NULL) {
			lfree(lp);
		}
	}

	/* clear the flags in the buffer */
	/* there may be a way to clean these less drastically, by
		using the information on the stacks above, but I
		couldn't figure it out.  -pgf  */
	lp = lforw(bp->b_linep);
	while (lp != bp->b_linep) {
		lsetnotcopied(lp);
		lp = lforw(lp);
	}

}

undo(f,n)
{
	LINE *lp, *alp;
	int nopops = TRUE;
	
	if (curbp->b_mode & MDVIEW)
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
			lfree(lp);
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
				mlwrite("Bug! no stacked line for an insert");
				/* cleanup ? naw, a bugs a bug */
				return(FALSE);
			}
		} else { /* there is no line where we're going */
			/* create an "unreal" tag line to push */
			alp = lalloc(-1);
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
			lfree(lp);
		}

		pushline(alp,ALTSTK(curbp));
	}
	
	if (nopops) {
		TTbeep();
		return (FALSE);
	}


	curwp->w_dotp = CURDOTP(curbp);
	curwp->w_doto = CURDOTO(curbp);
	if (curwp->w_doto >= llength(curwp->w_dotp))
		curwp->w_doto = llength(curwp->w_dotp) - 1;
	else if (curwp->w_doto < firstchar(curwp->w_dotp))
		curwp->w_doto = firstchar(curwp->w_dotp);

	SWITCHSTKS(curbp);
	
	vverify("undo");
	
	return TRUE;
}

mayneedundo()
{
	needundocleanup = TRUE;
}

preundocleanup()
{
	freeundostacks(curbp);
	CURDOTP(curbp) = curwp->w_dotp;
	CURDOTO(curbp) = curwp->w_doto;
	ALTDOTP(curbp) = NULL;
	ALTDOTO(curbp) = curwp->w_doto;
	needundocleanup = FALSE;
}

lineundo(f,n)
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

	curwp->w_dotp = lp;
	preundocleanup();


	ntext = malloc(ulp->l_size);
	if (ntext == NULL)
		return (FALSE);

	copy_for_undo(lp);

	memcpy(ntext, ulp->l_text, llength(ulp));
	free(lp->l_text);
	lp->l_text = ntext;
	lp->l_used = ulp->l_used;
	lp->l_size = ulp->l_size;

	/* let's be defensive about this */
	wp = wheadp;
	while (wp != NULL) {
		if (wp->w_dotp == lp)
			wp->w_doto = 0;
		if (wp->w_mkp == lp)
			wp->w_mko = 0;
		if (wp->w_ldmkp == lp)
			wp->w_ldmko = 0;
		wp = wp->w_wndp;
	}
	if (CURDOTP(curbp) == lp)
		CURDOTO(curbp) = 0;
	if (curbp->b_nmmarks != NULL) {
		/* fix the named marks */
		int i;
		struct MARK *mp;
		for (i = 0; i < 26; i++) {
			mp = &(curbp->b_nmmarks[i]);
			if (mp->markp == lp)
				mp->marko = 0;
		}
	}

	curwp->w_flag |= WFEDIT;
	
	vverify("lineundo");
	return TRUE;

}

repointstuff(nlp,olp)
register LINE *nlp,*olp;
{
	register WINDOW *wp;

	/* fix anything important that points to it */
	wp = wheadp;
	while (wp != NULL) {
		if (wp->w_linep == olp)
			if (lisreal(nlp)) {
				wp->w_linep = nlp;
			} else {
				wp->w_linep = olp->l_fp;
			}
		if (wp->w_mkp == olp) {
			if (lisreal(nlp)) {
				wp->w_mkp = nlp;
			} else {
				wp->w_mkp = olp->l_fp;
			}
			wp->w_mko = 0;
		}
		if (wp->w_ldmkp == olp) {
			if (lisreal(nlp)) {
				wp->w_ldmkp = nlp;
			} else {
				wp->w_ldmkp = olp->l_fp;
			}
			wp->w_ldmko = 0;
		}
		wp = wp->w_wndp;
	}
#if 0
no code for ALTDOTO, but this was ifdef'ed out before I put that in...  pgf
	if (ALTDOTP(curbp) == olp) {
		if (lisreal(nlp)) {
			ALTDOTP(curbp) = nlp;
		} else {
		    mlwrite("Bug: preundodot points at newly inserted line!");
		}
	}
#endif
	if (CURDOTP(curbp) == olp) {
		if (lisreal(nlp)) {
			CURDOTP(curbp) = nlp;
		} else {
		    mlwrite("Bug: preundodot points at newly inserted line!");
		}
	}
	if (curbp->b_nmmarks != NULL) {
		/* fix the named marks */
		int i;
		struct MARK *mp;
		for (i = 0; i < 26; i++) {
			mp = &(curbp->b_nmmarks[i]);
			if (mp->markp == olp) {
				if (lisreal(nlp)) {
					mp->markp = nlp;
					mp->marko = 0;
				} else {
				mlwrite("Sorry, lost the mark.");
				}
			}
		}
	}
#if !NEWUNDO
	resetuline(olp,nlp);
#endif
}

linesmatch(lp1,lp2)
register LINE *lp1,*lp2;
{
	int i;
	if (llength(lp1) != llength(lp2))
		return FALSE;
	return !memcmp(lp1->l_text, lp2->l_text, llength(lp1));
}

dumpuline(lp)
LINE *lp;
{
	if ((curbp->b_ulinep != NULL) &&
		    (curbp->b_ulinep->l_nxtundo == lp)) {
		lfree(curbp->b_ulinep);
		curbp->b_ulinep = NULL;
	}
}

setupuline(lp)
LINE *lp;
{
	/* take care of the U line */
	if ((curbp->b_ulinep == NULL) || (curbp->b_ulinep->l_nxtundo != lp)) {
		if (curbp->b_ulinep != NULL)
			lfree(curbp->b_ulinep);
		curbp->b_ulinep = copyline(lp);
		if (curbp->b_ulinep != NULL)
			curbp->b_ulinep->l_nxtundo = lp;
	}
}

resetuline(olp,nlp)
register LINE *olp,*nlp;
{
	if (curbp->b_ulinep != NULL && curbp->b_ulinep->l_nxtundo == olp) {
		if (lisreal(nlp)) {
			curbp->b_ulinep->l_nxtundo = nlp;
		} else {
			mlwrite("Bug: b_ulinep pointed at inserted line!");
		}
	}
}

firstchar(lp)
LINE *lp;
{
	int off = 0;
	while ( off != llength(lp) && isspace(lgetc(lp, off)) )
		off++;
	return off;
}
