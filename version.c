/*
 * version & usage-messages for vile
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/version.c,v 1.25 1995/04/25 02:31:21 pgf Exp $
 *
 */

#include	"estruct.h"	/* global structures and defines */
#include	"edef.h"	/* global definitions */
#include	"patchlev.h"

static	char	version_string[NSTRING];

void
print_usage P((void))
{
	static	char	*options[] = {
	"-h             to get help on startup",
	"-gNNN          or simply +NNN to go to line NNN",
	"-sstring       or +/string to search for \"string\"",
#if OPT_TAGS
	"-ttagname      to look up a tag",
#endif
	"-v             to view files as read-only",
#if OPT_ENCRYPT
	"-kcryptkey     for encrypted files",
#endif
#if DISP_X11
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
#if DISP_IBMPC || DISP_BORLAND
	"-2             25-line mode",
	"-4             43-line mode",
	"-5             50-line mode",
#if SYS_OS2
	"-6		60-line mode",
#endif
	"(see help file for more screen resolutions)",
#endif
	"-V             for version info",
	"use @filename to run filename as commands",
	" (this will suppress .vilerc)" };
	register int	j;

	(void)fprintf(stderr, "usage: %s [-flags] [@cmdfile] files...\n", 
		prog_arg);
	for (j = 0; j < TABLESIZE(options); j++)
		(void)fprintf(stderr, "\t%s\n", options[j]);
	ExitProgram(BADEXIT);
}

char *
getversion()
{

	if (*version_string)
		return version_string;
#if SYS_UNIX || SYS_VMS
	/*
	 * We really would like to have the date at which this program was
	 * linked, but a.out doesn't have that in general.  COFF files do. 
	 * Getting the executable's modification-time is a reasonable
	 * compromise.
	 */
	(void) lsprintf(version_string, "%s %s%s for %s", 
				prognam, version, PATCHLEVEL, opersys);
	{
		char *s;
		if ((s = flook(prog_arg, 
				(FL_EXECDIR|FL_PATH)|FL_EXECABLE)) != NULL) {
			time_t mtime = file_modified(s);
			if (mtime != 0) {
				(void)strcat(version_string, ", installed ");
				(void)strcat(version_string, ctime(&mtime));
				/* trim the newline */
				version_string[strlen(version_string)-1] = EOS;
			}
		}
	}
#else
# if SYS_MSDOS || SYS_OS2
#  if defined(__DATE__) && !SMALLER
	(void)lsprintf(version_string,"%s %s%s for %s, built %s %s with %s", 
		prognam, version, PATCHLEVEL, opersys, __DATE__, __TIME__,
#   if CC_WATCOM
		"Watcom C/386"
#   endif
#   if CC_DJGPP
		"DJGPP"
#   endif
#   if CC_TURBO
		"TurboC/BorlandC++"
#   endif
#   if CC_CSETPP
		"IBM C Set ++"
#   endif
	);
#  endif
# endif /* SYS_MSDOS || SYS_OS2 */
#endif /* not SYS_UNIX or SYS_VMS */
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


/*
 * Returns the special string consisting of program name + version, used to
 * fill in the filename-field for scratch buffers that are not associated with
 * an external file.
 */
char *
non_filename()
{
	static	char	buf[80];
	if (buf[0] == EOS)
		(void)lsprintf(buf, "       %s   %s%s",
				prognam, version, PATCHLEVEL);
	return buf;
}

