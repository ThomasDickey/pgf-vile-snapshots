/*
 * debugging support -- tom dickey.
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/trace.h,v 1.3 1994/10/03 13:24:35 pgf Exp $
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
extern	void	show_alloc P((void));
#endif

#if	DOALLOC
extern	char *	doalloc P((char *,unsigned));
extern	void	dofree P((char *));
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

extern	void	fail_alloc P(( char *, char * ));
extern	void	Trace P(( char *, ... )) __attribute__ ((format(printf,1,2)));
extern	void	Elapsed P(( char * ));
extern	void	WalkBack P(( void ));
extern	void	show_alloc P(( void ));

#ifdef fast_ptr
extern	void	lp_dump P(( char *, LINE * ));
extern	void	bp_dump P(( BUFFER * ));
#endif

#undef TRACE
#define TRACE(p) Trace p;

#endif	/* _trace_h */
