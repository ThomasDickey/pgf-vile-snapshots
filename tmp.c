/*
 *	tmp.c
 *
 * Manages temporary file (or extended memory) to cache buffer data on systems
 * where main memory is not sufficient to hold large amounts of data.
 *
 * We store the lines of all files in a cache of pages which are in-memory
 * when referenced with 'LINE *'.
 *
 * TODO:
 *	implement temp-file swapping
 *
 *	discount 'next' pointer from NCHUNK allocation
 *
 *	make REGION stuff work properly
 *
 * $Log: tmp.c,v $
 * Revision 1.1  1993/05/24 15:21:37  pgf
 * tom's 3.47 changes, part a
 *
 * Revision 1.0  1993/05/11  16:26:18  pgf
 * Initial revision
 *
 */
#include "estruct.h"
#include "edef.h"

#undef	TESTING
#define TESTING 1	/* values: 0, 1,2,3 */

#if TESTING
extern	void	WalkBack P(( void ));
extern	void	Trace P(( char *, ... ));
static	void	dump_line P(( char *, LINEPTR ));
	void	dumpBuffer P(( BUFFER * ));
static	void	invalidate_ref P(( LINE * ));
static	LINE *	ValidateDst P(( char *, LINE * ));
static	LINE *	ValidateSrc P(( char *, LINE * ));
static	void	add_to_cache P(( LINE *, LINEPTR ));
#endif

#if TESTING > 1
#define	TRACE1(s)	Trace s;
#else
#define	TRACE1(s)
#endif

#if TESTING > 2
#define	TRACE2(s)	Trace s;
#else
#define	TRACE2(s)
#endif

#if OPT_MAP_MEMORY

#define	MAX_PAGES	4	/* patch: maximum # in in-memory pages */
#define	BAD_PAGE	-1
#define	CHUNK	1024		/* patch: size of line-buffers */

#define	for_each_page(p,q)	for (p = recent_pages, q = 0; p != 0; q = p, p = p->next)

typedef	struct	free_t	{
	OFF_T		skip;	/* in-page pointer to next freespace */
	OFF_T		size;	/* size of this freespace */
	} FREE_T;

typedef	struct	buff_t	{
	struct	buff_t	*next;	/* pointer to next in-memory page */
	BLK_T		block;	/* identifies this page */
	OFF_T		space;	/* offset of first freespace in page */
	} PAGE_T;

static	PAGE_T	*recent_pages;
static	BLK_T	next_page;	/* next page # to allocate */

static	FILE	*temp_fp;	/* page-swapping file */
static	BLK_T	temp_pages;	/* highest temp-file page # written */

static	void	Oops P(( void ));
static	void	RecentPage P(( PAGE_T * ));
static	void	adjust_ptr P(( LINE *, LINE *, BUFFER * ));
static	OFF_T	SizeToSpace P(( SIZE_T ));
static	FREE_T *FirstSpace  P(( PAGE_T * ));
static	FREE_T *NextSpace   P(( FREE_T * ));
static	OFF_T	SpaceOffset P(( PAGE_T *, FREE_T * ));
static	void	ShrinkSpace P(( FREE_T *, OFF_T ));
static	void	DelinkSpace P(( PAGE_T *, FREE_T *, OFF_T ));

/*----------------------------------------------------------------------------*/
static void
Oops()
{
#if TESTING
	WalkBack();
	Trace((char *)0);
#endif
	ttclean(TRUE);
	abort();
}

/*----------------------------------------------------------------------------*/
#if TESTING
#define	MAX_CACHE	1000
static	LINE	*cache[MAX_CACHE];

#if TESTING
#define	ShowCache(tag,ptr,i,lp)	TRACE2(("%s %s #%d %p\n", tag, ptr, i, lp))
#else
#define	ShowCache(tag,ptr,i,lp)
#endif

void
dump_line(tag, p)
char	*tag;
LINEPTR	p;
{
	LINE	*q = l_ref(p);

	if (q != 0) {
		Trace("%p\t%d.%x\t%20s %d:'%.*s'\n",
			q, p.blk, p.off, tag,
			llength(q),
			lisreal(q) ? llength(q) : 1,
			lisreal(q) ? q->l_text  : "");
	} else {
		Trace("%p\t%d.%x\t%20s (null)\n",
			q, p.blk, p.off, tag);
	}
}

#define	dumpLine(p)	dump_line(#p,p)
#define	dumpMark(p)	dumpLine(p.l)

void
dumpBuffer(bp)
BUFFER	*bp;
{
	WINDOW	*wp;
	LINE	*p;
	LINEPTR	s;
	int	n;

	dumpMark(bp->b_wtraits.w_dt);
#ifdef WINMARK
	dumpMark(bp->b_wtraits.w_mk);
#endif
	dumpMark(bp->b_wtraits.w_ld);
	dumpMark(bp->b_wtraits.w_ln);

	for_each_window(wp) {
		if (wp->w_bufp == bp) {
			dumpMark(wp->w_traits.w_dt);
#ifdef WINMARK
			dumpMark(wp->w_traits.w_mk);
#endif
			dumpMark(wp->w_traits.w_ld);
			dumpMark(wp->w_traits.w_ln);
		}
	}

	if (bp->b_nmmarks != 0)
		for (n = 0; n < 26; n++)
			dumpMark(bp->b_nmmarks[n]);

	for (n = 0; n < 2; n++) {
		dumpMark(bp->b_uddot[n]);

		dumpLine(bp->b_udstks[n]);
		for (s = bp->b_udstks[n]; (p = l_ref(s)) != 0; s = p->l_nxtundo) {
			dumpLine(p->l_fp);
			dumpLine(p->l_bp);
			dumpLine(p->l_nxtundo);
		}
	}

	dumpLine(bp->b_ulinep);
	for (s = bp->b_ulinep; (p = l_ref(s)) != 0; s = p->l_nxtundo) {
		dumpLine(p->l_fp);
		dumpLine(p->l_bp);
		dumpLine(p->l_nxtundo);
	}
}

static void
invalidate_ref(lp)
LINE	*lp;
{
	if (lp == (LINE *)0
	 || lp == (LINE *)1) {
		Trace("cannot invalidate %p\n", lp);
		Oops();
	} else {
		register int	i;
		for (i = 0; i < SIZEOF(cache); i++) {
			if (cache[i] == lp) {
				cache[i] = 0;
				return;
			}
		}
	}
}

static LINE *
ValidateDst(tag, lp)
char	*tag;
LINE	*lp;
{
	register int i;

	if (lp != (LINE *)0
	 && lp != (LINE *)1) {
		for (i = 0; i < SIZEOF(cache); i++)
			if (cache[i] == lp) {
				ShowCache("<=", tag, i, lp);
				return lp;
			}
	}
	Trace("(%s) illegal value of lp:%p\n", tag, lp);
	Oops();
	return 0;
}

static LINE *
ValidateSrc(tag, lp)
char	*tag;
LINE	*lp;
{
	if (lp == (LINE *)0) {
		ShowCache("==", tag, BAD_PAGE, lp);
		return lp;
	}
	if (lp == (LINE *)1) {	/* poison-value used in 'lfree()' */
		ShowCache("::", tag, BAD_PAGE, lp);
		return lp;
	}
	return ValidateDst(tag,lp);
}

static void
add_to_cache(ref, ptr)
LINE	*ref;
LINEPTR	ptr;
{
	register int i;
	for (i = 0; i < SIZEOF(cache); i++)
		if (cache[i] == 0) {
			ShowCache("=>", "ADD", i, ref);
			cache[i] = ref;
			return;
		}
	Trace("** CACHE FULL\n");
	Oops();
}
#else
#define	invalidate_ref(lp)
#define	ValidateDst(tag,lp)	lp
#define	ValidateSrc(tag,lp)	lp
#define	add_to_cache(ref,ptr)
#endif

/*----------------------------------------------------------------------------*/

/*
 * Given the prev-pointer for the page, relink a page to the front of the
 * list of recent pages.
 */
static void
RecentPage (p)
PAGE_T	*p;
{
	if (p != 0) {
		register PAGE_T	*q = p->next;
		p->next = q->next;
		q->next = recent_pages;
		recent_pages = q;
	}
}

/*
 * Write the referenced page (and any unwritten pages up to that point)
 */
static void
WritePage()
{
	/* patch: unimplemented */
}

/*
 * Read the referenced page, making it the most recently-referenced
 */
static void
ReadPage()
{
	/* patch: unimplemented */
}

/*----------------------------------------------------------------------------*/

#define	AdjustLine(ptr)	if (l_ref(ptr) == oldp) ptr = newptr
#define	AdjustMark(m)	if (l_ref(m.l) == oldp) m.l = newptr

static void
adjust_ptr(oldp, newp, bp)
LINE	*oldp;
LINE	*newp;
BUFFER	*bp;
{
	register WINDOW	*wp;
	register int	n;
	LINEPTR	s;
	LINE	*p;
	LINEPTR	newptr;

	newptr = l_ptr(newp);

	/* adjust pointers in buffer and its windows */
	AdjustMark(bp->b_wtraits.w_dt);
#ifdef WINMARK
	AdjustMark(bp->b_wtraits.w_mk);
#endif
	AdjustMark(bp->b_wtraits.w_ld);
	AdjustMark(bp->b_wtraits.w_ln);

	for_each_window(wp) {
		if (wp->w_bufp == bp) {
			AdjustMark(wp->w_traits.w_dt);
#ifdef WINMARK
			AdjustMark(wp->w_traits.w_mk);
#endif
			AdjustMark(wp->w_traits.w_ld);
			AdjustMark(wp->w_traits.w_ln);
		}
	}

	/* adjust pointers in marks */
	if (bp->b_nmmarks != 0)
		for (n = 0; n < 26; n++)
			AdjustMark(bp->b_nmmarks[n]);

	/* adjust pointers in the undo-stacks */
	for (n = 0; n < 2; n++) {
		AdjustMark(bp->b_uddot[n]);

		AdjustLine(bp->b_udstks[n]);
		for (s = bp->b_udstks[n]; (p = l_ref(s)) != 0; s = p->l_nxtundo) {
			AdjustLine(p->l_fp);
			AdjustLine(p->l_bp);
			AdjustLine(p->l_nxtundo);
		}
	}

	AdjustLine(bp->b_ulinep);
	for (s = bp->b_ulinep; (p = l_ref(s)) != 0; s = p->l_nxtundo) {
		AdjustLine(p->l_fp);
		AdjustLine(p->l_bp);
		AdjustLine(p->l_nxtundo);
	}

	/* patch: MARK's in REGION? */
	/* patch: MARK's in struct WHBLOCK? */
}

/*----------------------------------------------------------------------------*/

/*
 * Convert a requested line-size to the total space required for storing in a
 * page-buffer.  Ensure that it is evenly divisible by the FREE_T struct so we
 * don't accumulate fragments.
 */
static	OFF_T
SizeToSpace (size)
SIZE_T	size;
{
	OFF_T	need = sizeof(LINE) + size;
	if (need & (sizeof(FREE_T)-1))
		need = (need | (sizeof(FREE_T)-1)) + 1;
	return need;
}

static	FREE_T *
FirstSpace (page)
PAGE_T	*page;
{
	return (page->space != 0) ? (FREE_T *)((char *)page + page->space) : 0;
}

static	FREE_T *
NextSpace (q)
FREE_T	*q;
{
	return (q->skip != 0) ? (FREE_T *)((char *)q + q->skip) : 0;
}

static	OFF_T
SpaceOffset (page, q)
PAGE_T	*page;
FREE_T	*q;
{
	return (q != 0) ? (OFF_T)((char *)q - (char *)page) : 0;
}

static	void
ShrinkSpace (q, size)
FREE_T	*q;
OFF_T	size;
{
	q->skip -= size;	/* closer to the next space now */
	q->size -= size;	/* ...smaller by the same amount */
}

/*----------------------------------------------------------------------------*/
static	void
DelinkSpace (page, q0, size)
PAGE_T	*page;
FREE_T	*q0;
OFF_T	size;
{
	register FREE_T	*temp;
	register FREE_T *first = FirstSpace(page);

	if (q0 == first) {
		if (q0->size == size) {
			page->space = SpaceOffset(page, NextSpace(q0));
		} else {
			page->space += size;
			*(temp = FirstSpace(page)) = *q0;
			ShrinkSpace(temp, size);
		}
	} else {
		for (temp = first; temp != 0; temp = NextSpace(temp)) {
			if (NextSpace(temp) == q0) {
				if (temp == first) {
					page->space = SpaceOffset(page, q0);
				} else {
					temp->skip += size;
				}
				break;
			}
		}
		ShrinkSpace(q0, size);
	}
}

/*----------------------------------------------------------------------------*/

/* Try to find an in-memory page with enough space to store the line.
 * If there is none, allocate a new page and return the line from that point.
 *
 * patch: must address line-truncation, in case we try to read a line that
 *	needs more than a page to store it.
 *
 * patch: must adjust allocation of lines to avoid losing stray space when
 *	we get too small a remainder for FREE_T structs.  Mostly this is
 *	done in line.c with 'roundup()' macro.
 */
LINEPTR
l_allocate(size)
SIZE_T	size;
{
	static int	count;

	LINEPTR	ptr;
	OFF_T	need	= SizeToSpace(size);
	register PAGE_T	*this, *prev;
	register FREE_T	*q;
	register FREE_T	*q0 = 0;

	TRACE1(("\n** allocate %x #%d\n", size, ++count))

	/* allocate from the first in-memory page that has enough space */
	for_each_page(this, prev) {
		for (q = FirstSpace(this); q != 0; q = NextSpace(q)) {
#if TESTING
			if (q->skip < 0 || q->size < 0) {
				TRACE1(("negative space-value (%d,%d)\n",
					q->skip, q->size))
				Oops();
			}
			if (q->skip > CHUNK || q->size > CHUNK) {
				TRACE1(("space-value too large (%d,%d)\n",
					q->skip, q->size))
				Oops();
			}
#endif
			if (q->size > need) {
				if (q0 != 0) {
					if (q0->size > q->size)
						q0 = q;
				} else
					q0 = q;
			}
		}
		if (q0 != 0)
			break;
	}

	ptr.blk = BAD_PAGE;
	ptr.off = 0;

	if (q0 == 0) {	/* allocate a new page */
		/* patch: must limit page-growth here */
		if ((this = castalloc(PAGE_T, CHUNK)) == 0)
			return ptr;

		this->next = recent_pages;
		this->block = next_page++;
		this->space = sizeof(PAGE_T);
		recent_pages = this;

		q0 = FirstSpace(this);
		q0->skip =
		q0->size = (((CHUNK - sizeof(PAGE_T)) / sizeof(FREE_T)) - 1) * sizeof(FREE_T);
		q = NextSpace(q0);
		q->skip  = 0;
		q->size  = 0;
		TRACE1(("ALLOC-PAGE %lx =>%p (%p, %p)\n", this->block, this, q0, q))
	}

	if (q0 != 0) {	/* de-link LINE from the page's freespace */
		LINE	*lp = (LINE *)q0;

		DelinkSpace(this, q0, need);

		/* setup return-data */
		ptr.blk = this->block;
		ptr.off = SpaceOffset(this, q0);

		lp->l_size = size;
		lp->l_text = (size > 0) ? (char *)(lp+1) : 0;
		lp->l_fp   =
		lp->l_bp   =
		lp->l_nxtundo = nullmark.l;
		add_to_cache(lp,ptr);
		(void)l_ptr(lp);	/* patch: trace */

		TRACE1(("ALLOC-LINE %lx.%x (%x) =>%p\n", ptr.blk, ptr.off, size, lp))
	} else
		Oops();
	return ptr;
}

/*----------------------------------------------------------------------------*/

void
l_deallocate(ptr,bp)
LINEPTR	ptr;
BUFFER	*bp;
{
	LINE	*lp = l_ref(ptr);
#if TESTING
	static	int	count;
	TRACE1(("deallocate #%d %lx.%x %p\n", ++count, ptr.blk, ptr.off, l_ref(ptr)))
#endif
	/* ad hoc fixes to ensure that we don't refer to this later... */
/*patch:	adjust_ptr(lp, (LINE *)0, bp);*/

	/* patch: unimplemented */

#if TESTING
	invalidate_ref(lp);
#endif
}

/*----------------------------------------------------------------------------*/

/*
 * Reallocate a line buffer (to increase its size).
 * patch: may be able to make line grow into adjacent space
 */
LINE *
l_reallocate(lp, size, bp)
LINEPTR	lp;
SIZE_T	size;
BUFFER	*bp;
{
	LINE	*oldp = l_ref(lp);
	LINE	*newp = l_ref(l_allocate(size));

	TRACE1(("reallocate %p to %p (%x bytes)\n", oldp, newp, size))
	if (newp != 0) {
		newp->l_size = size;
		newp->l_used = oldp->l_used;
		newp->l_text = (char *)(newp+1);
		if (oldp->l_text != 0
		 && oldp->l_used >= 1)
			memcpy(newp->l_text, oldp->l_text, oldp->l_used);

		/*
		 * Note that (unlike the virtual memory configuration), this
		 * requires that we adjust pointers that point to the old line,
		 * because we keep the LINE struct and its text together.
		 */
		(void)set_lforw(lback(oldp), newp);
		(void)set_lback(lforw(oldp), newp);
		(void)set_lforw(newp, lforw(oldp));
		(void)set_lback(newp, lback(oldp));
		adjust_ptr(oldp, newp, bp);

		/* finally, get rid of the old buffer */
		/* patch? l_deallocate(lp,bp); */
#if TESTING
		invalidate_ref(oldp);
#endif
	}
	return newp;
}

/*
 * Mark the page holding the given line as "changed", so that we remember to
 * write it out when we need to reuse its space.
 */
void
l_changed(lp)
LINEPTR	lp;
{
	/* patch: unimplemented */
}

/*
 * Ensure that the LINE referenced by the argument is in memory, return the
 * pointer to it.
 */
LINE *
l_ref (ptr)
LINEPTR	ptr;
{
	register PAGE_T	*p, *q;
	register LINE	*lp;

	if (ptr.blk == BAD_PAGE) {
		lp = (LINE *)(ptr.off);
		return ValidateSrc("ref", lp);
	}

	for_each_page(p,q) {
		if (p->block == ptr.blk) {
			lp = (LINE *)((char *)p + ptr.off);
			TRACE2(("l_ref(%lx.%x) = %p\n", ptr.blk, ptr.off, lp))
			RecentPage(q);
			return ValidateDst("ref", lp);
		}
	}
	TRACE1(("Cannot find pointer (%lx.%x)\n", ptr.blk, ptr.off))
	Oops();
	return 0;
}

LINEPTR
l_ptr (lp)
LINE *	lp;
{
	register PAGE_T *p, *q;
	LINEPTR	ptr;

	(void)ValidateSrc("ptr", lp);
	if (lp == (LINE *)0
	 || lp == (LINE *)1) {
		ptr.blk = BAD_PAGE;
		ptr.off = (OFF_T)lp;
		return ptr;
	}

	for_each_page(p,q) {
		long	off = ((long)lp - (long)p);
		if (off >= sizeof(PAGE_T) && (off < CHUNK)) {
			RecentPage(q);
			ptr.blk = p->block;
			ptr.off = (OFF_T)off;
			TRACE2(("l_ptr(%p) = %lx.%x\n", lp, ptr.blk, ptr.off))
			return ptr;
		}
	}
	TRACE1(("Cannot find line %p\n", lp))
	Oops();
	return ptr;
}

LINE *
set_lforw (dst, src)
LINE	*dst;
LINE	*src;
{
	TRACE2(("set_lforw:"))
	return l_ref((ValidateDst("set",dst)->l_fp = l_ptr(src)));
}

LINE *
set_lback (dst, src)
LINE	*dst;
LINE	*src;
{
	TRACE2(("set_lback:"))
	return l_ref((ValidateDst("set",dst)->l_bp = l_ptr(src)));
}

LINE *
lforw (lp)
LINE	*lp;
{
	TRACE2(("lforw:"))
	return l_ref(ValidateDst("get",lp)->l_fp);
}

LINE *
lback (lp)
LINE	*lp;
{
	TRACE2(("lback:"))
	return l_ref(ValidateDst("get",lp)->l_bp);
}

void
lsetclear (lp)
LINE	*lp;
{
	ValidateDst("flag", lp);
	lp->l_nxtundo = l_ptr((LINE *)0);
	lp->l.l_flag  = 0;
}
#endif


/* For memory-leak testing (only!), releases all tmp-buffer storage. */
#if NO_LEAKS
void
tmp_leaks()
{
#if OPT_MAP_MEMORY
	register PAGE_T	*p;
	while ((p = recent_pages) != 0) {
		p = p->next;
		free((char *)recent_pages);
		recent_pages = p;
	}
#endif
}
#endif
