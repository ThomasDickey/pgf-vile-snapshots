/*
 * debugging support -- tom dickey.
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/trace.h,v 1.2 1994/07/11 22:56:20 pgf Exp $
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

extern	void	fail_alloc P(( char *, char * ));
extern	void	Trace P(( char *, ... ));
extern	void	Elapsed P(( char * ));
extern	void	WalkBack P(( void ));
extern	void	show_alloc P(( void ));

#ifdef fast_ptr
extern	void	lp_dump P(( char *, LINE * ));
extern	void	bp_dump P(( BUFFER * ));
#endif

#endif	/* _trace_h */
