/*
 * version & usage-messages for vile
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/version.c,v 1.15 1994/07/22 01:45:44 pgf Exp $
 *
 */

#include	"estruct.h"	/* global structures and defines */
#include	"edef.h"	/* global definitions */

#if UNIX || VMS
#include <sys/stat.h>		/* ...for 'struct stat' */
#endif

extern char *pathname[];	/* startup file path/name array */

static	char	version_string[NSTRING];

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

char *
getversion()
{
	if (*version_string)
		return version_string;
#if UNIX || VMS
	/*
	 * Remember the directory from which we were run, to use in finding the
	 * help-file.
	 */
	{
		char	temp[NFILEN];
		char	*s, *t;
		s = strmalloc(lengthen_path(strcpy(temp, prog_arg)));
		t = pathleaf(s);
		if (t != s) {
# if UNIX	/* 't' points past slash */
			t[-1] = EOS;
# else		/* 't' points to ']' */
			*t = EOS;
# endif
			pathname[2] = s;
		}
	}

	/*
	 * We really would like to have the date at which this program was
	 * linked, but a.out doesn't have that in general.  COFF files do. 
	 * Getting the executable's modification-time is a reasonable
	 * compromise.
	 */
	lsprintf(version_string, "%s %s for %s", prognam, version, opersys);
	{
		char *s;
		if ((s = flook(prog_arg, FL_ANYWHERE)) != NULL) {
			struct	stat	sb;
			if (stat(s, &sb) >= 0) {
				(void)strcat(version_string, ", installed ");
				(void)strcat(version_string, ctime(&sb.st_mtime));
				/* trim the newline */
				version_string[strlen(version_string)-1] = EOS;
			}
		}
	}
#else
# if MSDOS || OS2
#  if defined(__DATE__) && !SMALLER
	(void)lsprintf(version_string,"%s %s for %s, built %s %s with %s", 
		prognam, version, opersys, __DATE__, __TIME__,
#   if WATCOM
		"Watcom C/386"
#   endif
#   if DJGPP
		"DJGPP"
#   endif
#   if TURBO
		"TurboC/BorlandC++"
#   endif
	);
#  endif
# endif /* MSDOS || OS2 */
#endif /* not UNIX or VMS */
	return version_string;
}

/* ARGSUSED */
int
showversion(f,n)
int f,n;
{
	mlforce(getversion());
	return TRUE;
}
