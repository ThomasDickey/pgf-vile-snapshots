/*	EPATH:	This file contains certain info needed to locate the
		MicroEMACS files on a system dependant basis.

									*/

/*	possible names and paths of help files under different OSs	*/

/*
 * $Log: epath.h,v $
 * Revision 1.3  1991/08/07 11:51:32  pgf
 * added RCS log entries
 *
 * revision 1.2
 * date: 1991/06/26 09:43:03;
 * added trailing slash on some file search paths
 * ----------------------------
 * revision 1.1
 * date: 1990/09/21 10:25:08;
 * initial vile RCS revision
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

#if	MSDOS
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

#if	V7 | BSD | USG
{
	".vilerc",
	"vile.hlp",
	"/usr/local/",
	"/usr/lib/",
	"/usr/local/bin/",
	"/usr/local/lib/",
	""
};
#endif

#if	VMS
{
	"vile.rc",
	"vile.hlp",
	"",
	"sys$sysdevice:[vmstools]"
};
#endif

#define	NPNAMES	(sizeof(pathname)/sizeof(char *))
