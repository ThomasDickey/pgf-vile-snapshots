/*	EPATH:	This file contains certain info needed to locate the
		certain needed files on a system dependent basis.

									*/

/*	possible names and paths of help files under different OSs	*/

/*
 * $Header: /usr/build/VCS/pgf-vile/RCS/epath.h,v 1.18 1995/02/28 16:55:40 pgf Exp $
 */

/* first two entries are default startup and help files, the rest are
	possible places to look for them */

char *pathname[] =

#if	SYS_AMIGA
{
	".vilerc",
	"vile.hlp",
	NULL,
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
	NULL,
	"\\",
	"\\bin\\",
	"\\util\\"
};
#endif

#if	SYS_MSDOS || SYS_WIN31 || SYS_OS2 || SYS_WINNT
{
	"vile.rc",
	"vile.hlp",
	NULL,
	"\\sys\\public\\",
	"\\usr\\bin\\",
	"\\bin\\",
	"\\"
};
#endif

#if	SYS_UNIX
{
	".vilerc",
	"vile.hlp",
	".",		/* replaced at runtime with path-head of argv[0] */
#ifdef HELP_LOC
#ifndef lint	/* makefile gives inconsistent quoting for lint, compiler */
	HELP_LOC,
#endif	/* lint */
#endif
	"/usr/local/lib/",
	"/usr/local/",
	"/usr/lib/"
};
#endif

#if	SYS_VMS
{
	"vile.rc",
	"vile.hlp",
	NULL,		/* replaced at runtime with path-head of argv[0] */
	"sys$login:",
	"",
	"sys$sysdevice:[vmstools]"
};
#endif

#define	NPNAMES	(sizeof(pathname)/sizeof(char *))
