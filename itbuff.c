/*
 *	tbuff.c
 *
 *	Manage dynamic temporary buffers.
 *	Note that some temp-buffs are never freed, for speed
 *
 *	To do:	add 'itb_ins()' and 'itb_del()' to support cursor-level command
 *		editing.
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/itbuff.c,v 1.6 1995/04/22 03:22:53 pgf Exp $
 *
 */

#include "estruct.h"
#include "edef.h"

#define	NCHUNK	NLINE

static	ALLOC_T	itb_seek P(( ITBUFF *, ALLOC_T, int ));
static	ITBUFF *itb_put P(( ITBUFF **, ALLOC_T, int ));

/*******(testing)************************************************************/
#if NO_LEAKS
typedef	struct	_itb_list	{
	struct	_itb_list	*link;
	ITBUFF			*buff;
	} ITB_LIST;

static	ITB_LIST	*all_tbuffs;

#define	AllocatedBuffer(q)	itb_remember(q);
#define	FreedBuffer(q)		itb_forget(q);

static	void	itb_remember P(( ITBUFF * ));
static	void	itb_forget P(( ITBUFF * ));

static
void	itb_remember(p)
	ITBUFF	*p;
{
	register ITB_LIST *q = typealloc(ITB_LIST);
	q->buff = p;
	q->link = all_tbuffs;
	all_tbuffs = q;
}

static
void	itb_forget(p)
	ITBUFF	*p;
{
	register ITB_LIST *q, *r;

	for (q = all_tbuffs, r = 0; q != 0; r = q, q = q->link)
		if (q->buff == p) {
			if (r != 0)
				r->link = q->link;
			else
				all_tbuffs = q->link;
			free((char *)q);
			break;
		}
}

void	itb_leaks()
{
	while (all_tbuffs != 0) {
		ITBUFF	*q = all_tbuffs->buff;
		itb_free(&q);
		FreedBuffer(q);
	}
}

#else
#define	AllocatedBuffer(q)
#define	FreedBuffer(q)
#endif

/*******(initialization)*****************************************************/

/*
 * ensure that the given temp-buff has as much space as specified
 */
ITBUFF *	itb_alloc(p, n)
	ITBUFF	**p;
	ALLOC_T	n;
{
	register ITBUFF *q = *p;
	if (q == 0) {
		q = *p = typealloc(ITBUFF);
		q->itb_data = typeallocn(int, q->itb_size = n);
		q->itb_used = 0;
		q->itb_last = 0;
		q->itb_endc = abortc;
		AllocatedBuffer(q)
	} else if (n >= q->itb_size) {
		q->itb_data = typereallocn(int, q->itb_data, 
						q->itb_size = (n*2));
	}
	return q;
}

/*
 * (re)initialize a temp-buff
 */
ITBUFF *	itb_init(p, c)
	ITBUFF	**p;
	int	c;		/* code to return if no-more-data */
{
	register ITBUFF *q = *p;
	if (q == 0)
		q = itb_alloc(p, NCHUNK);
	q->itb_used = 0;
	q->itb_last = 0;
	q->itb_endc = c;
	return (*p = q);
}

/*
 * deallocate a temp-buff
 */
void	itb_free(p)
	ITBUFF	**p;
{
	register ITBUFF *q = *p;

	if (q != 0) {
		free((char *)(q->itb_data));
		free((char *)q);
		FreedBuffer(q)
	}
	*p = 0;
}

/*******(storage)************************************************************/

/*
 * put a character c at the nth position of the temp-buff.  make it the last.
 */
static ITBUFF *
itb_put(p, n, c)
	ITBUFF	**p;
	ALLOC_T	n;
	int	c;
{
	register ITBUFF *q;

	if ((q = itb_alloc(p, n+1)) != 0) {
		q->itb_data[n] = c;
		q->itb_used = n+1;
	}
	return q;
}

/*
 * stuff the nth character into the temp-buff -- assumes space already there
 *  it's sort of the opposite of itb_peek
 */
void	itb_stuff(p, c)
	ITBUFF	*p;
	int	c;
{
	if (p->itb_last < p->itb_used)
		p->itb_data[p->itb_last] = c;
	else
		p->itb_endc = c;
}
/*
 * append a character to the temp-buff
 */
ITBUFF *	itb_append(p, c)
	ITBUFF	**p;
	int	c;
{
	register ITBUFF *q = *p;
	register ALLOC_T n = (q != 0) ? q->itb_used : 0;
	
	return itb_put(p, n, c);
}

/*
 * Copy one temp-buff to another
 */
ITBUFF *	itb_copy(d, s)
	ITBUFF	**d;
	ITBUFF	*s;
{
	ITBUFF *p;

	if (s != 0) {
		if ((p = itb_init(d, s->itb_endc)) != 0) {
			int	*ptr = s->itb_data;
			ALLOC_T len = s->itb_used;
			while ((len-- != 0) && itb_append(&p, *ptr++) != 0)
				;
		}
	} else
		p = itb_init(d, abortc);
	return p;
}

/*
 * append a binary data to the temp-buff
 */
ITBUFF *	itb_bappend(p, s, len)
	ITBUFF	**p;
	char	*s;
	ALLOC_T len;
{
	while ((len-- != 0) && itb_append(p, (int)(*s++)) != 0)
		;
	return *p;
}

#if NEEDED
/*
 * append a string to the temp-buff
 */
ITBUFF *	itb_sappend(p, s)
	ITBUFF	**p;
	char	*s;
{
	if (!s)
		return *p;
	while (*s && itb_append(p, (int)(*s++)) != 0)
		;
	return *p;
}

void	itb_delete(p, cnt)
	ITBUFF	*p;
	ALLOC_T	cnt;
{
	int *from, *to, *used;

	to = &p->itb_data[p->itb_last];
	from = to + cnt;

	used = &p->itb_data[p->itb_used];

	if (from >= used) {
		p->itb_used = p->itb_last;
		return;
	}

	while (from < used) {
		*to++ = *from++;
	}

	p->itb_used -= cnt;

}

ITBUFF *	itb_insert(p, c)
	ITBUFF	**p;
	int	c;
{
	register ITBUFF *q = *p;
	int *last, *to;

	/* force enough room for another character */
	itb_put(p, q->itb_used, 0 /* any value */ );

	/* copy up */
	to = &q->itb_data[q->itb_used-1];
	last = &q->itb_data[q->itb_last];
	while (to > last) {
		*to = *(to-1);
		to--;
	}

	/* put in the new one */
	itb_stuff(q, c);

	return *p;
}
#endif  /* NEEDED */


/*******(retrieval)************************************************************/

/*
 * get the nth character from the temp-buff
 */
int	itb_get(p, n)
	ITBUFF	*p;
	ALLOC_T	n;
{
	register int	c = abortc;

	if (p != 0)
		c = (n < p->itb_used) ? p->itb_data[n] : p->itb_endc;

	return c;
}

#if NEEDED
/*
 * undo the last 'itb_put'
 */
void	itb_unput(p)
	ITBUFF	*p;
{
	if (p != 0
	 && p->itb_used != 0)
		p->itb_used -= 1;
}
#endif

/*******(iterators)************************************************************/

/*
 * Reset the iteration-count
 */
static ALLOC_T
itb_seek(p, seekto, whence)
	ITBUFF *p;
	ALLOC_T seekto;
	int whence;
{
	ALLOC_T olast;

	if (p == 0)
		return 0;

	olast = p->itb_last;

	if (whence == 0)
		p->itb_last = seekto;
	else if (whence == 1)
		p->itb_last += seekto;
	else if (whence == 2)
		p->itb_last = p->itb_used - seekto;
	return olast;
}

void	itb_first(p)
	ITBUFF	*p;
{
	(void)itb_seek(p, 0, 0);
}

/*
 * Returns true iff the iteration-count has not gone past the end of temp-buff.
 */
int	itb_more(p)
	ITBUFF	*p;
{
	return (p != 0) ? (p->itb_last < p->itb_used) : FALSE;
}

/*
 * get the next character from the temp-buff
 */
int	itb_next(p)
	ITBUFF	*p;
{
	if (p != 0)
		return itb_get(p, p->itb_last++);
	return abortc;
}

/*
 * get the last character from the temp-buff, and shorten
 * (opposite of itb_append)
 */
int	itb_last(p)
	ITBUFF	*p;
{
	int c;
	if (p != 0 && p->itb_used > 0) {
		c = itb_get(p, p->itb_used-1);
		p->itb_used--;
		return c;
	}
	return abortc;
}

#if NEEDED
/*
 * undo a itb_next
 */
void	itb_unnext(p)
	ITBUFF	*p;
{
	if (p == 0)
		return;
	if (p->itb_last > 0)
		p->itb_last--;
}
#endif

/*
 * get the next character from the temp-buff w/o incrementing index
 */
int	itb_peek(p)
	ITBUFF	*p;
{
	if (p != 0)
		return itb_get(p, p->itb_last);
	return abortc;
}

/*******(bulk-data)************************************************************/

/*
 * returns the length of the data
 */
ALLOC_T itb_length(p)
	ITBUFF	*p;
{
	return (p != 0) ? p->itb_used : 0;
}
