/*
 *	tbuff.c
 *
 *	Manage dynamic temporary buffers.
 *	Note that some temp-buffs are never freed, for speed
 *
 *	To do:	add 'tb_ins()' and 'tb_del()' to support cursor-level command
 *		editing.
 *
 * $Log: tbuff.c,v $
 * Revision 1.1  1993/02/08 14:51:46  pgf
 * Initial revision
 *
 */

#include "estruct.h"
#include "edef.h"

#define	NCHUNK	NLINE
#define	UCHAR	unsigned char
#define	UINT	unsigned

/*******(initialization)*****************************************************/

/*
 * ensure that the given temp-buff has as much space as specified
 */
TBUFF *	tb_alloc(p, n)
	TBUFF	**p;
	UINT	n;
{
	register TBUFF *q = *p;
	if (q == 0) {
		q = *p = typealloc(TBUFF);
		q->tb_data = typeallocn(UCHAR, q->tb_size = n);
		q->tb_used = 0;
		q->tb_endc = abortc;
	} else if (n >= q->tb_size) {
		q->tb_data = typereallocn(UCHAR, q->tb_data, q->tb_size = (n*2));
	}
	return q;
}

/*
 * (re)initialize a temp-buff
 */
TBUFF *	tb_init(p, c)
	TBUFF	**p;
	int	c;		/* code to return if no-more-data */
{
	register TBUFF *q = *p;
	if (q == 0)
		q = tb_alloc(p, NCHUNK);
	q->tb_used = 0;
	q->tb_endc = c;
	return q;
}

/*
 * deallocate a temp-buff
 */
void	tb_free(p)
	TBUFF	**p;
{
	register TBUFF *q = *p;

	if (q != 0) {
		free((char *)(q->tb_data));
		free((char *)q);
	}
	*p = 0;
}

/*******(storage)************************************************************/

/*
 * put a character c at the nth position of the temp-buff
 */
TBUFF *	tb_put(p, n, c)
	TBUFF	**p;
	UINT	n;
	int	c;
{
	register TBUFF *q;

	if (q = tb_alloc(p, n+1)) {
		q->tb_data[n] = c;
		q->tb_used = n+1;
	}
	return q;
}

/*
 * append a character to the temp-buff
 */
TBUFF *	tb_append(p, c)
	TBUFF	**p;
	int	c;
{
	register TBUFF *q = *p;
	register UINT	n = (q != 0) ? q->tb_used : 0;
	
	return tb_put(p, n, c);
}

/*******(retrieval)************************************************************/

/*
 * get the nth character from the temp-buff
 */
int	tb_get(p, n)
	TBUFF	*p;
	UINT	n;
{
	register int	c = abortc;

	if (p != 0)
		c = (n < p->tb_used) ? p->tb_data[n] : p->tb_endc;

	return (c >= 128) ? (c-256) : c;	/* sign-extend */
}

/*
 * undo the last 'tb_put'
 */
void	tb_unput(p)
	TBUFF	*p;
{
	if (p != 0
	 && p->tb_used != 0)
		p->tb_used -= 1;
}

/*******(iterators)************************************************************/

/*
 * Reset the iteration-count
 */
void	tb_first(p)
	TBUFF	*p;
{
	if (p != 0)
		p->tb_last = 0;
}

/*
 * Returns true iff the iteration-count has not gone past the end of temp-buff.
 */
int	tb_more(p)
	TBUFF	*p;
{
	return (p != 0) ? (p->tb_last < p->tb_used) : FALSE;
}

/*
 * get the next character from the temp-buff
 */
int	tb_next(p)
	TBUFF	*p;
{
	if (p != 0)
		return tb_get(p, p->tb_last++);
	return abortc;
}

/*
 * get the next character from the temp-buff w/o incrementing index
 */
int	tb_peek(p)
	TBUFF	*p;
{
	if (p != 0)
		return tb_get(p, p->tb_last);
	return abortc;
}
