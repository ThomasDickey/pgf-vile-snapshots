/*
 *	dirstuff.h
 *
 *	Definitions to interface to unix-like DIRECTORY(3) procedures.
 *	Include this after "estruct.h"
 *
 * $Log: dirstuff.h,v $
 * Revision 1.4  1993/04/20 12:18:32  pgf
 * see tom's 3.43 CHANGES
 *
 * Revision 1.3  1993/04/02  10:57:41  pgf
 * cleanup of ls-based directory enumeration, and support (unused as yet?)
 * for old-style directories
 *
 * Revision 1.2  1993/04/01  15:50:34  pgf
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
#define OLD_STYLE_DIRS 0	/* e.g., pre-SysV.2 14-char names */

#if POSIX || TURBO
# include <dirent.h>
# define	DIRENT	struct dirent
#else	/* apollo & other old bsd's */
# if BERK
#  include <sys/dir.h>
#  define	DIRENT	struct direct
# else
#  undef USE_LS_FOR_DIRS
#  define USE_LS_FOR_DIRS 1
# endif
#endif

#endif	/* VMS */

#if USE_LS_FOR_DIRS
#define	DIR	FILE
typedef	struct	{
	short	d_namlen;
	char	d_name[NFILEN];
	} DIRENT;
#endif

#if VMS || USE_LS_FOR_DIRS
extern	DIR *	opendir P(( char * ));
extern	DIRENT *readdir P(( DIR * ));
extern	int	closedir P(( DIR * ));
#endif

#if OLD_STYLE_DIRS
#define	DIR	FILE
#define	DIRENT	struct direct
#define	opendir(n)	fopen(n,"r")
extern	DIRENT *readdir P(( DIR * ));
#define	closedir(dp)	fclose(dp)
#endif
