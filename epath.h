/*	EPATH:	This file contains certain info needed to locate the
		MicroEMACS files on a system dependent basis.

									*/

/*	possible names and paths of help files under different OSs	*/

/*
 * $Log: epath.h,v $
 * Revision 1.14  1994/06/30 14:08:29  pgf
 * tom's patch, and pgf fixups
 *
 * Revision 1.13  1994/06/17  01:29:44  pgf
 * windows/nt changes, from joe greer
 *
 * Revision 1.12  1994/04/18  14:26:27  pgf
 * merge of OS2 port patches, and changes to tungetc operation
 *
 * Revision 1.11  1994/02/22  11:03:15  pgf
 * truncated RCS log for 4.0
 *
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
