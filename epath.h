/*	EPATH:	This file contains certain info needed to locate the
		certain needed files on a system dependent basis.

									*/

/*	possible names and paths of help files under different OSs	*/

/*
 * $Header: /usr/build/VCS/pgf-vile/RCS/epath.h,v 1.16 1994/11/29 04:02:03 pgf Exp $
 */

/* first two entries are default startup and help files, the rest are
	possible places to look for them */

char *pathname[] =

#if	SYS_AMIGA
{
	".vilerc",
	"vile.hlp",
	"",
	"sys:c/",
	"sys:t/",
	"sys:s/",
	":c/",
	":t/",
	":s/"
};
#endif

#if	SYS_ST520
{
	"vile.rc",
	"vile.hlp",
	"\\",
	"\\bin\\",
	"\\util\\",
	""
};
#endif

#if	SYS_MSDOS || SYS_WIN31 || SYS_OS2 || SYS_WINNT
{
	"vile.rc",
	"vile.hlp",
	"\\sys\\public\\",
	"\\usr\\bin\\",
	"\\bin\\",
	"\\",
	""
};
#endif

#if	SYS_UNIX
{
	".vilerc",
	"vile.hlp",
	".",		/* replaced at runtime with path-head of argv[0] */
	"/usr/local/",
	"/usr/lib/",
	"/usr/local/bin/",
	"/usr/local/lib/",
#ifdef HELP_LOC
#ifndef lint	/* makefile gives inconsistent quoting for lint, compiler */
	HELP_LOC,
#endif	/* lint */
#endif
	""
};
#endif

#if	SYS_VMS
{
	"vile.rc",
	"vile.hlp",
	".",		/* replaced at runtime with path-head of argv[0] */
	"sys$login:",
	"",
	"sys$sysdevice:[vmstools]"
};
#endif

#define	NPNAMES	(sizeof(pathname)/sizeof(char *))
