/*
 *	dirstuff.h
 *
 *	Definitions to interface to unix-like DIRECTORY(3) procedures.
 *	Include this after "estruct.h"
 *
 * $Log: dirstuff.h,v $
 * Revision 1.2  1993/04/01 15:50:34  pgf
 * for sysV machines, without POSIX or BSD dirent, we now have code
 * that enumerates directories using /bin/ls.
 *
 * Revision 1.1  1993/03/25  19:50:58  pgf
 * see 3.39 section of CHANGES
 *
 */

#if VMS
#include	<rms.h>
#ifndef	$DESCRIPTOR
#include	<descrip.h>
#endif

typedef struct	{
	unsigned long d_ino;
	short	d_reclen;
	short	d_namlen;
	char	d_name[NAM$C_MAXRSS];		/* result: SYS$SEARCH */
	} DIRENT;

typedef	struct	{
	DIRENT		dd_ret;
	struct	FAB	dd_fab;
	struct	NAM	dd_nam;
	char		dd_esa[NAM$C_MAXRSS];	/* expanded: SYS$PARSE */
	} DIR;

#else

#define USE_LS_FOR_DIRS 0

#if POSIX
# include <dirent.h>
# define	DIRENT	struct dirent
#else	/* apollo & other old bsd's */
# if BSD
#  include <sys/dir.h>
#  define	DIRENT	struct direct
# else
#  undef USE_LS_FOR_DIRS
#  define USE_LS_FOR_DIRS 1
# endif
#endif

#endif	/* VMS */

#if !UNIX
extern	DIR *	opendir P(( char * ));
extern	DIRENT *readdir P(( DIR * ));
extern	int	closedir P(( DIR * ));
#endif
