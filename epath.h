/*	EPATH:	This file contains certain info needed to locate the
		certain needed files on a system dependent basis.

									*/

/*	possible names and paths of help files under different OSs	*/

/*
 * $Header: /usr/build/VCS/pgf-vile/RCS/epath.h,v 1.15 1994/07/11 22:56:20 pgf Exp $
 */

/* first two entries are default startup and help files, the rest are
	possible places to look for them */

char *pathname[] =

#if	AMIGA
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

#if	ST520
{
	"vile.rc",
	"vile.hlp",
	"\\",
	"\\bin\\",
	"\\util\\",
	""
};
#endif

#if	FINDER
{
	"vile.rc",
	"vile.hlp",
	"/bin/",
	"/sys/public/",
	""
};
#endif

#if	MSDOS || WIN31 || OS2 || NT
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

#if	UNIX
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

#if	VMS
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
