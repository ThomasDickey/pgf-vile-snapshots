/*
 *	tmp.c
 *
 * Manages tempory file (or extended memory) to cache buffer data on systems
 * where main memory is not sufficient to hold large amounts of data.
 *
 * $Log: tmp.c,v $
 * Revision 1.0  1993/05/11 16:26:18  pgf
 * Initial revision
 *
 */
#include "estruct.h"

#if OPT_MAP_MEMORY
#undef lsync
#undef set_lforw
#undef set_lback
#undef lforw
#undef lback

/*
 * Ensure that the LINE referenced by the argument is in memory, return the
 * pointer to it.
 */
LINE *lsync (lp)
LINEPTR	lp;
{
	union	{
		LINEPTR	in;
		LINE *	out;
	} recast;
	recast.in = lp;
	return recast.out;
}

LINE *
set_lforw (dst, src)
LINE	*dst;
LINE	*src;
{
	return (dst->l_fp = src);
}

LINE *
set_lback (dst, src)
LINE	*dst;
LINE	*src;
{
	return (dst->l_bp = src);
}

LINE *
lforw (lp)
LINE	*lp;
{
	return lp->l_fp;
}

LINE *
lback (lp)
LINE	*lp;
{
	return lp->l_bp;
}
#endif

