/*
 * version & usage-messages for vile
 *
 * $Log: version.c,v $
 * Revision 1.11  1994/04/22 14:34:15  pgf
 * changed BAD and GOOD to BADEXIT and GOODEXIT
 *
 * Revision 1.10  1994/04/20  19:54:50  pgf
 * changes to support 'BORLAND' console i/o screen driver
 *
 * Revision 1.9  1994/04/18  14:26:27  pgf
 * merge of OS2 port patches, and changes to tungetc operation
 *
 * Revision 1.8  1994/02/25  12:08:54  pgf
 * removed rows/columns options
 *
 * Revision 1.7  1994/02/23  05:31:03  pgf
 * x options up to date
 *
 * Revision 1.6  1994/02/22  11:03:15  pgf
 * truncated RCS log for 4.0
 *
 *
 */

#include	"estruct.h"	/* global structures and defines */
#include	"edef.h"	/* global definitions */

#if UNIX || VMS
#include <sys/stat.h>		/* ...for 'struct stat' */
#include <time.h>		/* ...for 'ctime()' */
#endif

extern char *pathname[];	/* startup file path/name array */

#if UNIX || VMS
static	char	built_at[40];
#endif

void
print_usage P((void))
{
	static	char	*options[] = {
	"-h             to get help on startup",
	"-gNNN          or simply +NNN to go to line NNN",
	"-sstring       or +/string to search for \"string\"",
#if TAGS
	"-ttagname      to look up a tag",
#endif
	"-v             to view files as read-only",
#if CRYPT
	"-kcryptkey     for encrypted files",
#endif
#if X11
	"-name name     to change program name for X resources",
	"-title name	to set name in title bar",
	"-fg color      to change foreground color",
	"-bg color      to change background color",
	"-fn fontname   to change font",
	"-display displayname to change the default display",
	"-rv            for reverse video",
	"-geometry CxR	to set initial size to R rows and C columns",
	"-xrm Resource  to change an xvile resource",
	"-leftbar	Put scrollbar(s) on left",
	"-rightbar	Put scrollbar(s) on right (default)",
#endif
#if IBMPC || BORLAND
	"-2             25-line mode",
	"-4             43-line mode",
	"-5             50-line mode",
#if OS2
	"-6		60-line mode",
#endif
	"(see help file for more screen resolutions)",
#endif
	"-V             for version info",
	"use @filename to run filename as commands",
	" (this will suppress .vilerc)" };
	register int	j;

	(void)fprintf(stderr, "usage: %s [-flags] [@cmdfile] files...\n", prog_arg);
	for (j = 0; j < SIZEOF(options); j++)
		(void)fprintf(stderr, "\t%s\n", options[j]);
	ExitProgram(BADEXIT);
}

#if UNIX || VMS
void
makeversion()
{
	/*
	 * Remember the directory from which we were run, to use in finding the
	 * help-file.
	 */
	char	temp[NFILEN];
	char	*s = strmalloc(lengthen_path(strcpy(temp, prog_arg))),
		*t = pathleaf(s);
	if (t != s) {
#if UNIX	/* 't' points past slash */
		t[-1] = EOS;
#else		/* 't' points to ']' */
		*t = EOS;
#endif
		pathname[2] = s;
	}

	/*
	 * We really would like to have the date at which this program was
	 * linked, but a.out doesn't have that in general.  COFF files do. 
	 * Getting the executable's modification-time is a reasonable
	 * compromise.
	 */
	*built_at = EOS;
	if ((s = flook(prog_arg, FL_ANYWHERE)) != NULL) {
		struct	stat	sb;
		if (stat(s, &sb) >= 0) {
			(void)lsprintf(built_at, ", installed %s",
				ctime(&sb.st_mtime));
			built_at[strlen(built_at)-1] = EOS;
		}
	}
}
#endif

/* ARGSUSED */
int
showversion(f,n)
int f,n;
{
#if UNIX || VMS
	mlforce("%s%s", version, built_at);
#endif

#if MSDOS || OS2
# if defined(__DATE__) && !SMALLER
	mlforce("%s, built %s %s with %s", version, __DATE__, __TIME__,
#  if WATCOM
	"Watcom C/386"
#  endif
#  if DJGPP
	"DJGPP"
#  endif
#  if TURBO
	"TurboC/BorlandC++"
#  endif
	);
# endif
#else
	mlforce(version);
#endif

	return TRUE;
}
