/*
 *	tbuff.c
 *
 *	Manage dynamic temporary buffers.
 *	Note that some temp-buffs are never freed, for speed
 *
 *	To do:	add 'tb_ins()' and 'tb_del()' to support cursor-level command
 *		editing.
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/tbuff.c,v 1.20 1994/11/29 17:04:43 pgf Exp $
 *
 */

#include "estruct.h"
#include "edef.h"

#define	NCHUNK	NLINE

/*******(testing)************************************************************/
#if NO_LEAKS
typedef	struct	_tb_list	{
	struct	_tb_list	*link;
	TBUFF			*buff;
	} TB_LIST;

static	TB_LIST	*all_tbuffs;

#define	AllocatedBuffer(q)	tb_remember(q);
#define	FreedBuffer(q)		tb_forget(q);

static	void	tb_remember P(( TBUFF * ));
static	void	tb_forget P(( TBUFF * ));

static
void	tb_remember(p)
	TBUFF	*p;
{
	register TB_LIST *q = typealloc(TB_LIST);
	q->buff = p;
	q->link = all_tbuffs;
	all_tbuffs = q;
}

static
void	tb_forget(p)
	TBUFF	*p;
{
	register TB_LIST *q, *r;

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

void	tb_leaks()
{
	while (all_tbuffs != 0) {
		TBUFF	*q = all_tbuffs->buff;
		tb_free(&q);
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
TBUFF *	tb_alloc(p, n)
	TBUFF	**p;
	ALLOC_T	n;
{
	register TBUFF *q = *p;
	if (q == 0) {
		q = *p = typealloc(TBUFF);
		q->tb_data = typeallocn(char, q->tb_size = n);
		q->tb_used = 0;
		q->tb_last = 0;
		q->tb_endc = abortc;
		AllocatedBuffer(q)
	} else if (n >= q->tb_size) {
		q->tb_data = typereallocn(char, q->tb_data, q->tb_size = (n*2));
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
	q->tb_last = 0;
	q->tb_endc = c;
	return (*p = q);
}

/*
 * deallocate a temp-buff
 */
void	tb_free(p)
	TBUFF	**p;
{
	register TBUFF *q = *p;

	if (q != 0) {
		free(q->tb_data);
		free((char *)q);
		FreedBuffer(q)
	}
	*p = 0;
}

/*******(storage)************************************************************/

/*
 * put a character c at the nth position of the temp-buff
 */
TBUFF *	tb_put(p, n, c)
	TBUFF	**p;
	ALLOC_T	n;
	int	c;
{
	register TBUFF *q;

	if ((q = tb_alloc(p, n+1)) != 0) {
		q->tb_data[n] = c;
		q->tb_used = n+1;
	}
	return q;
}

#if NEEDED
/*
 * stuff the nth character into the temp-buff -- assumes space already there
 *  it's sort of the opposite of tb_peek
 */
void	tb_stuff(p, c)
	TBUFF	*p;
	int	c;
{
	if (p->tb_last < p->tb_used)
		p->tb_data[p->tb_last] = c;
	else
		p->tb_endc = c;
}
#endif
/*
 * append a character to the temp-buff
 */
TBUFF *	tb_append(p, c)
	TBUFF	**p;
	int	c;
{
	register TBUFF *q = *p;
	register ALLOC_T n = (q != 0) ? q->tb_used : 0;
	
	return tb_put(p, n, c);
}

/*
 * Copy one temp-buff to another
 */
TBUFF *	tb_copy(d, s)
	TBUFF	**d;
	TBUFF	*s;
{
	register TBUFF *p;

	if (s != 0) {
		if ((p = tb_init(d, s->tb_endc)) != 0)
			p = tb_bappend(d, s->tb_data, s->tb_used);
	} else
		p = tb_init(d, abortc);
	return p;
}

/*
 * append a binary data to the temp-buff
 */
TBUFF *	tb_bappend(p, s, len)
	TBUFF	**p;
	char	*s;
	ALLOC_T len;
{
	while ((len-- != 0) && tb_append(p, (int)(*s++)) != 0)
		;
	return *p;
}
/*
 * append a string to the temp-buff
 */
TBUFF *	tb_sappend(p, s)
	TBUFF	**p;
	char	*s;
{
	if (!s)
		return *p;
	while (*s && tb_append(p, (int)(*s++)) != 0)
		;
	return *p;
}

/*
 * copy a string to the temp-buff, including a null
 */
TBUFF *	tb_scopy(p, s)
	TBUFF	**p;
	char	*s;
{
	(void) tb_init(p, EOS);
	(void) tb_sappend(p, s);
	return tb_append(p, EOS);
}

/*******(retrieval)************************************************************/

#if DISP_X11
/*
 * get the nth character from the temp-buff
 */
int	tb_get(p, n)
	TBUFF	*p;
	ALLOC_T	n;
{
	register int	c = abortc;

	if (p != 0)
		c = (n < p->tb_used) ? p->tb_data[n] : p->tb_endc;

	return char2int(c);
}
#endif

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

#if NEEDED
/*
 * Reset the iteration-count
 */
void	tb_first(p)
	TBUFF	*p;
{
	if (p != 0)
		p->tb_last = 0;
}
#endif

#if DISP_X11
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
#endif

#if NEEDED
/*
 * undo a tb_next
 */
void	tb_unnext(p)
	TBUFF	*p;
{
	if (p == 0)
		return;
	if (p->tb_last > 0)
		p->tb_last--;
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
#endif  /* NEEDED */

/*******(bulk-data)************************************************************/

/*
 * returns a pointer to data, assumes it is one long string
 */
char *	tb_values(p)
	TBUFF	*p;
{
	return (p != 0) ? p->tb_data : 0;
}

/*
 * returns the length of the data
 */
ALLOC_T tb_length(p)
	TBUFF	*p;
{
	return (p != 0) ? p->tb_used : 0;
}
