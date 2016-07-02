/*
 * debugging support -- tom dickey.
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/trace.h,v 1.5 1996/02/05 02:19:21 pgf Exp $
 *
 */
#ifndef	_trace_h
#define	_trace_h

#ifndef	NO_LEAKS
#define NO_LEAKS 0
#endif

#ifndef DOALLOC
#define DOALLOC 0
#endif

#if	NO_LEAKS
extern	void	show_alloc (void);
#endif

#if	DOALLOC
extern	char *	doalloc (char *oldp, unsigned amount);
extern	char *	do_calloc (unsigned nmemb, unsigned size);
extern	void	dofree (char *oldp);
#undef	calloc
#define	calloc(n,m)	do_calloc(n, m)
#undef	malloc
#define	malloc(n)	doalloc((char *)0,n)
#undef	realloc
#define	realloc(p,n)	doalloc(p,n)
#undef	free
#define	free(p)		dofree(p)
#endif	/* DOALLOC */

#if !(defined(__GNUC__) || defined(__attribute__))
#define __attribute__(p)
#endif

extern	void	fail_alloc (char *msg, char *ptr);
extern	void	Trace ( char *fmt, ... ) __attribute__ ((format(printf,1,2)));
extern	void	Elapsed ( char * msg);
extern	void	WalkBack (void);
extern	void	show_alloc (void);

#ifdef fast_ptr
extern	void	lp_dump (char *msg, LINE *lp);
extern	void	bp_dump ( BUFFER *bp );
#endif

#undef TRACE
#define TRACE(p) Trace p;

#endif	/* _trace_h */
