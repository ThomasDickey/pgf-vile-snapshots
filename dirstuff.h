/*
 *	dirstuff.h
 *
 *	Definitions to interface to unix-like DIRECTORY(3) procedures.
 *	Include this after "estruct.h"
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/dirstuff.h,v 1.15 1994/10/03 13:24:35 pgf Exp $
 *
 */

#ifndef DIRSTUFF_H
#define DIRSTUFF_H

#if ! VMS

#define USE_LS_FOR_DIRS 0
#define OLD_STYLE_DIRS 0	/* e.g., pre-SysV.2 14-char names */

#if DIRNAMES_NOT_NULL_TERMINATED
/* rumor has it that some early readdir implementations didn't null-terminate
   the d_name array, and on those you _have_ to use d_namlen to get
   the length.  most modern dirent structs are null-terminated, however. */
#define USE_D_NAMLEN 1
#endif

#if _POSIX_VERSION || DIRENT || TURBO || WATCOM || DJGPP || OS2
# if WATCOM
#   include <direct.h>
# else
#   include <dirent.h>
# endif
# undef DIRENT
# define	DIRENT	struct dirent
#else	/* apollo & other old bsd's */
# define	DIRENT	struct direct
# define USE_D_NAMLEN 1
# if SYSNDIR
#  include <sys/ndir.h>
# else
#  if SYSDIR
#   include <sys/dir.h>
#  else
#   if NDIR
#    include <ndir.h>
#   else
#    undef USE_LS_FOR_DIRS
#    undef DIRENT
#    undef USE_D_NAMLEN
#    define USE_LS_FOR_DIRS 1
#    if NT
      struct direct {
	char *d_name;
      };
      struct _dirdesc {
	HANDLE hFindFile;
	WIN32_FIND_DATA ffd;
	int first;
      };

      typedef struct _dirdesc DIR;
      typedef struct direct DIRENT;

#     define MAXNAMLEN		255
#     define DIRECTORY_ENTRY		struct direct
#     define CLOSE_RETURN_TYPE 	int

#    else /* NT */
#     undef USE_LS_FOR_DIRS
#     define USE_LS_FOR_DIRS 1
#     undef USE_D_NAMLEN
#     define USE_LS_FOR_DIRS 1
#    endif /* NT */
#   endif
#  endif
# endif
#endif

#else /* VMS */

#include	<rms.h>
#include	<descrip.h>

#define USE_D_NAMLEN 1

typedef struct	{
	ULONG	d_ino;
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

#endif	/* VMS */

#if USE_LS_FOR_DIRS
#define	DIR	FILE
typedef	struct	{
	char	d_name[NFILEN];
	} DIRENT;
#endif

#if NT || VMS || USE_LS_FOR_DIRS
extern	DIR *	opendir P(( char * ));
extern	DIRENT *readdir P(( DIR * ));
extern	int	closedir P(( DIR * ));
#endif

#if OLD_STYLE_DIRS
	this ifdef is untested
#define USE_D_NAMLEN 1
#define	DIR	FILE
#define	DIRENT	struct direct
#define	opendir(n)	fopen(n,"r")
extern	DIRENT *readdir P(( DIR * ));
#define	closedir(dp)	fclose(dp)
#endif

#ifndef USE_D_NAMLEN
#define USE_D_NAMLEN 0
#endif

#endif /* DIRSTUFF_H */
