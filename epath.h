/*	EPATH:	This file contains certain info needed to locate the
		MicroEMACS files on a system dependant basis.

									*/

/*	possible names and paths of help files under different OSs	*/

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
	"/bin",
	"/sys/public",
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
	"/usr/local/bin",
	"/usr/local/lib",
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
