/*	ESTRUCT:	Structure and preprocessor defines for
			vile.  Reshaped from the original, which
			was for MicroEMACS 3.9

			vile is by Paul Fox
			MicroEmacs was written by Dave G. Conroy
			modified by Steve Wilhite, George Jones
			substantially modified by Daniel Lawrence
*/

/*
 * $Header: /usr/build/VCS/pgf-vile/RCS/estruct.h,v 1.253 1995/05/08 03:06:17 pgf Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef os_chosen

/* All variants of UNIX should now be handled by the configure script */

#ifdef VMS		/* predefined by VAX/VMS compiler */
# define scrn_chosen
# define DISP_VMSVT  1
#endif

/* non-unix flavors */
#undef SYS_MSDOS
#undef SYS_WIN31
#undef SYS_CPM
#undef SYS_AMIGA

#define SYS_AMIGA	0			/* AmigaDOS			*/
#define SYS_ST520	0			/* ST520, TOS			*/
#define SYS_MSDOS	0			/* MS-DOS			*/
#define SYS_WIN31	0			/* Windows 3.1			*/
#define SYS_OS2		0			/* son of DOS			*/
#define SYS_CPM		0			/* CP/M-86			*/
#define SYS_WINNT	0			/* Windows/NT			*/

/*	Compiler definitions			*/
#define CC_MSC		0	/* MicroSoft C compile version 3 & 4 & 5 & 6 */
#define CC_ZTC		0	/* Zortech C compiler */
#define CC_TURBO	0	/* Turbo C/MSDOS or Borland C++ */
#define CC_WATCOM	0	/* WATCOM C/386 version 9.0 or above */
#define CC_DJGPP	0	/* DJ's GCC version 1.09 */
#define CC_CSETPP	0	/* IBM C Set ++ 2.x */

#ifdef __TURBOC__	/* predefined in Turbo C, both DOS and Windows */
# undef CC_TURBO
# undef SYS_MSDOS
# undef SYS_WIN31
# ifdef _Windows	/* predefined in TurboC for Windows */
#  define SYS_WIN31  1
# else
#  define SYS_MSDOS  1
# endif
# define CC_TURBO  1
#endif

#ifdef __WATCOMC__
#undef SYS_MSDOS
#undef CC_WATCOM
#define SYS_MSDOS  1
#define CC_WATCOM 1
#endif

#ifdef __IBMC__
# if __IBMC__ >= 200	/* IBM C Set ++ version 2.x */
#  undef  CC_CSETPP
#  define CC_CSETPP 1
# endif
#endif

#ifdef __OS2__
/* assume compiler already chosen */
#undef SYS_MSDOS
#undef SYS_OS2
#define SYS_OS2    1
#endif

#ifdef __GO32__  	/* DJ's GCC version 1.09 */
#undef CC_DJGPP
#undef SYS_MSDOS
#define SYS_MSDOS  1
#define CC_DJGPP   1
#endif

#if SYS_WINNT
#undef SYS_MSDOS
#undef SYS_WINNT
#define SYS_WINNT	1
#endif

/* As of version 3.51 of vile, CC_NEWDOSCC should be correct for Turbo,
 * Watcom, and the DJ gcc (GO32) compilers.  I'm betting that it's also
 * probably correct for MSC (Microsoft C) and ZTC (Zortech), but I'm not
 * sure of those.  (It implies a lot of ANSI and POSIX behavior.)
 */
#if CC_TURBO || CC_WATCOM || CC_MSC || CC_DJGPP || CC_ZTC || SYS_WINNT || CC_CSETPP
# define CC_NEWDOSCC 1
# define HAVE_GETCWD 1
# define off_t long		/* missing in TurboC 3.1 */
#else
# define CC_NEWDOSCC 0
#endif

#if CC_CSETPP
# define HAVE_UTIME		1
# define HAVE_SYS_UTIME_H	1
# define CPP_SUBS_BEFORE_QUOTE	1
# define HAVE_LOSING_SWITCH_WITH_STRUCTURE_OFFSET	1
#endif

#endif /* os_chosen */

/*
 * Porting constraints: supply the normally assumed values that we get from
 * the "configure" script, for systems on which we cannot run "configure"
 * (e.g., VMS, OS2, MSDOS).
 */
#ifndef HAVE_ACCESS
# define HAVE_ACCESS    1	/* if your system has the access() system call */
#endif

#ifndef HAVE_MKDIR
# define HAVE_MKDIR	1	/* if your system has the mkdir() system call */
#endif

#ifndef HAVE_UTIME
# define HAVE_UTIME	0	/* if your system has the utime() system call */
#endif

#ifndef HAVE_SETJMP_H
# define HAVE_SETJMP_H  1	/* if your system has <setjmp.h> */
#endif

#ifndef HAVE_SIGNAL_H
# define HAVE_SIGNAL_H  1	/* if your system has <signal.h> */
#endif

#if !(defined(HAVE_STRCHR) || defined(HAVE_STRRCHR))
# define HAVE_STRCHR    1
# define HAVE_STRRCHR   1
#endif

#ifndef HAVE_STDLIB_H
# define HAVE_STDLIB_H  1	/* if your system has <stdlib.h> */
#endif

#ifndef HAVE_STRING_H
# define HAVE_STRING_H  1	/* if your system has <string.h> */
#endif

/* Some code uses these as values in expressions, so we always want them
 * defined, just in case we run into a substandard preprocessor.
 */
#ifndef DISP_X11
# define DISP_X11 0
#endif
#ifndef SYS_MSDOS
# define SYS_MSDOS 0
#endif
#ifndef SYS_OS2
# define SYS_OS2 0
#endif
#ifndef SYS_WIN31
# define SYS_WIN31 0
#endif
#ifndef SYS_WINNT
# define SYS_WINNT 	0
#endif
#ifdef VMS		/* predefined by VAX/VMS compiler (VAX C V3.2-044) */
# define SYS_VMS    1
# define HAVE_GETCWD 1
# if defined(__DECC) && !defined(__alpha)
#  define HAVE_ACCESS 0	/* 'access()' is reported to not work properly */
# endif
#else
# define SYS_VMS    0
#endif
#ifdef apollo
# define SYS_APOLLO 1	/* FIXME: still more ifdefs to autoconf */
#endif
#ifdef sun
# define SYS_SUNOS 1	/* FIXME: need to tweak lint ifdefs */
#endif

#define IBM_KBD 	(SYS_MSDOS || SYS_OS2 || SYS_WINNT)
#define IBM_VIDEO 	(SYS_MSDOS || SYS_OS2 || SYS_WINNT)
#define CRLF_LINES 	(SYS_MSDOS || SYS_OS2 || SYS_WINNT)

#if SYS_WINNT
#include <windows.h>
#endif 
#include <stdio.h>
#include <sys/types.h>
#if SYS_VMS
#include <sys/stat.h>	/* defines off_t */
#endif
#if HAVE_LIBC_H
#include <libc.h>
#endif
#if HAVE_FCNTL_H
#ifndef O_RDONLY	/* prevent multiple inclusion on lame systems */
#include <fcntl.h>	/* 'open()' */
#endif
#endif

#if HAVE_SYS_TIME_H && ! SYSTEM_LOOKS_LIKE_SCO
/* on SCO, sys/time.h conflicts with select.h, and we don't need it */
#include <sys/time.h>
#ifdef TIME_WITH_SYS_TIME
# include <time.h>
#endif
#else
#include <time.h>
#endif

#if HAVE_SYS_RESOURCE_H && ! SYSTEM_LOOKS_LIKE_SCO
/* On SunOS, struct rusage is referred to in <sys/wait.h>.  struct rusage
   is defined in <sys/resource.h>.   NeXT may be similar.  On SCO,
   resource.h needs time.h, which we excluded above.  */
#include <sys/resource.h>
#endif

#if HAVE_SYS_WAIT_H
#include <sys/wait.h>	/* 'wait()' */
#endif

/* definitions for testing apollo version with gcc warnings */
#if SYS_APOLLO
# ifdef __GNUC__		/* only tested for SR10.3 with gcc 1.36 */
#  ifndef _APOLLO_SOURCE	/* ...defined in gcc 2.4.5 */
#  define _APOLLO_SOURCE	/* appease gcc by forcing extern-defs */
#  endif
#  define __attribute(s)
#  define APOLLO_STDLIB 1
# endif
# if defined(L_tmpnam)		/* SR10.3, CC 6.8 defines in <stdio.h> */
#  define APOLLO_STDLIB 1
# endif
#endif

#ifndef APOLLO_STDLIB
# define APOLLO_STDLIB 0
#endif

#ifndef SIGT
/* choose between void and int signal handler return type.
  "typedefs?  we don't need no steenking typedefs..." */
# if CC_NEWDOSCC
#  define SIGT void
#  define SIGRET
# else
#  define SIGT int
#  define SIGRET return 0
# endif
#endif /* SIGT */

#if HAVE_SIGNAL_H
#include	<signal.h>
# if SYS_APOLLO
#  if APOLLO_STDLIB && !defined(lint)	/* SR10.3, CC 6.8 */
#   define ACTUAL_SIG_ARGS int signo, ...
#   define ACTUAL_SIG_DECL /* empty */
#   define DEFINE_SIG_ARGS ACTUAL_SIG_ARGS
#  endif
# endif
#endif

#ifndef ACTUAL_SIG_ARGS
# if __STDC__
#  define ACTUAL_SIG_ARGS int signo
#  define ACTUAL_SIG_DECL /* empty */
#  define DEFINE_SIG_ARGS int
# else
#  define ACTUAL_SIG_ARGS signo
#  define ACTUAL_SIG_DECL int signo;
#  define DEFINE_SIG_ARGS
# endif
#endif

#if defined(__GNUC__)
# undef  SIG_DFL
# undef  SIG_IGN
# define SIG_DFL	(SIGT (*)(DEFINE_SIG_ARGS))0
# define SIG_IGN	(SIGT (*)(DEFINE_SIG_ARGS))1
#endif

#if HAVE_SETJMP_H
#include	<setjmp.h>
#endif

/* argument for 'exit()' or '_exit()' */
#if SYS_VMS
#include	<stsdef.h>
#define GOODEXIT	(STS$M_INHIB_MSG | STS$K_SUCCESS)
#define BADEXIT		(STS$M_INHIB_MSG | STS$K_ERROR)
#else
#define GOODEXIT	0
#define BADEXIT		1
#endif

/* has the select() or poll() call, only used for short sleeps in fmatch() */
#if HAVE_SELECT
#undef HAVE_POLL
#endif

#if SYS_UNIX || SYS_MSDOS || SYS_WIN31 || SYS_VMS || SYS_OS2 || SYS_WINNT
#define	ENVFUNC	1
#else
#define	ENVFUNC	0
#endif

/* ====================================================================== */
#ifndef scrn_chosen
/*	Terminal Output definitions		*/
/* choose ONLY one of the following */
#define DISP_TERMCAP	SYS_UNIX	/* Use TERMCAP			*/
#define DISP_ANSI	0		/* ANSI escape sequences	*/
#define DISP_AT386	0		/* AT style 386 unix console	*/
#define	DISP_HP150	0		/* HP150 screen driver		*/
#define	DISP_HP110	0		/* HP110 screen driver		*/
#define	DISP_VMSVT	SYS_VMS		/* various VMS terminal entries	*/
#define DISP_VT52	0		/* VT52 terminal (Zenith).	*/
#define	DISP_BORLAND	0		/* Borland console I/O routines */
#define	DISP_IBMPC	(SYS_MSDOS && !DISP_BORLAND && !DISP_ANSI) /* IBM-PC CGA/MONO/EGA driver */
#define	DISP_ZIBMPC	0		/* Zortech lib IBM-PC CGA/MONO/EGA driver	*/
#define	DISP_DG10	0		/* Data General system/10	*/
#define	DISP_TIPC	0		/* TI Professional PC driver	*/
#define	DISP_Z309	0		/* Zenith 100 PC family	driver	*/
#define	DISP_MAC	0		/* Macintosh			*/
#define	DISP_ATARI	0		/* Atari 520/1040ST screen	*/
#define	DISP_X11	0		/* X Window System */
#define DISP_NTCONS	0		/* Windows/NT console		*/
#define DISP_VIO	SYS_OS2		/* OS/2 VIO (text mode)		*/

/*   Special keyboard definitions	     */
#define DISP_WANGPC	0		/* funny escape sequences -- see ibmpc.c */
#endif

/* ====================================================================== */
/*	Configuration options... pick and choose as you wish */

/*	Code size options	*/
#define	FEWNAMES 0	/* strip some names - will no longer be bindable */
#define	SMALLER	0	/* strip some fluff -- not a lot smaller, but some */
#define OPT_MAP_MEMORY 0	/* tiny systems can page out data */

/* various color stuff */
/* termcap color stuff conditional on linux for now, 
	since it's only been tested there. */
#define	OPT_COLOR (DISP_ANSI || IBM_VIDEO || \
			(DISP_TERMCAP && defined(linux)))

/* Feature turnon/turnoff */
#define	OPT_BSD_FILOCK	0	/* file locking under unix BSD 4.2  */
#define	OPT_DOSFILES	1	/* turn on code for DOS mode (lines that
				   end in crlf).  use DOSFILES, for instance,
				   if you edit DOS-created files under UNIX   */
#define	OPT_REVSTA	1	/* Status line appears in reverse video       */
#define	OPT_CFENCE	1	/* do fence matching in CMODE		      */
#define	OPT_LCKFILES	0	/* create lock files (file.lck style) 	      */
#define	OPT_ENCRYPT	0	/* file encryption (not crypt(1) compatible!) */
#define	OPT_TAGS	1	/* tags support  			      */
#define	OPT_WORDCOUNT	0	/* "count-words" command"		      */
#define	OPT_PROCEDURES	1	/* named procedures			      */
#define	OPT_KSH_HISTORY	0	/* ksh-style history commands		      */
#define	OPT_PATHLOOKUP	1	/* search $PATH for startup and help files    */
#define	OPT_SCROLLCODE	1	/* code in display.c for scrolling the screen.
				   Only useful if your display can scroll
				   regions, or at least insert/delete lines.
				   ANSI, TERMCAP, IBMPC, VMSVT and AT386 can 
				   do this */
#define OPT_CVMVAS	1	/* arguments to forward/back page and half page
				   are in pages	instead of rows (in vi,
				   they're rows, and the argument is "sticky",
				   i.e. it's remembered */
#define OPT_PRETTIER_SCROLL 0	/* can improve the appearance of a scrolling
				   screen, but it will in general be slower */
#define OPT_STUTTER_SEC_CMD 0	/* must the next/prev section commands (i.e.
				   ']]' and '[[' be stuttered?  they must be
				   stuttered in real vi, I prefer them not
				   to be */
#define OPT_ICURSOR	0	/* use an insertion cursor if possible */

/* the "working..." message -- we must have the alarm() syscall, and
   system calls must be restartable after an interrupt by default or be
   made restartable with sigaction() */
#define OPT_WORKING (!SMALLER && HAVE_ALARM && HAVE_RESTARTABLE_PIPEREAD)

#define OPT_SCROLLBARS XTOOLKIT			/* scrollbars */
#define OPT_VMS_PATH    (SYS_VMS)  /* vax/vms path parsing (testing/porting)*/

/* systems with MSDOS-like filename syntax */
#define OPT_MSDOS_PATH  (SYS_MSDOS || SYS_WIN31 || SYS_OS2 || SYS_WINNT)

/* individual features that are (normally) controlled by SMALLER */
#define OPT_AEDIT       !SMALLER		/* advanced editing options: e.g. en/detabbing	*/
#define OPT_B_LIMITS    !SMALLER		/* left-margin */
#define OPT_ENUM_MODES  !SMALLER		/* fixed-string modes */
#define OPT_EVAL        !SMALLER		/* expression-evaluation */
#define OPT_FINDERR     !SMALLER		/* finderr support. */
#define OPT_FORMAT      !SMALLER		/* region formatting support. */
#define OPT_FLASH       !SMALLER || DISP_IBMPC	/* visible-bell */
#define OPT_HISTORY     !SMALLER		/* command-history */
#define OPT_ISRCH       !SMALLER		/* Incremental searches */
#define OPT_ISO_8859    !SMALLER		/* ISO 8859 characters */
#define OPT_LINEWRAP    !SMALLER		/* line-wrap mode */
#define OPT_MLFORMAT    !SMALLER		/* modeline-format */
#define OPT_MS_MOUSE    !SMALLER && DISP_IBMPC && CC_TURBO 	/* MsDos-mouse */
#define OPT_ONLINEHELP  !SMALLER		/* short per-command help */
#define OPT_POPUPCHOICE !SMALLER		/* popup-choices mode */
#define OPT_POPUP_MSGS  !SMALLER		/* popup-msgs mode */
#define OPT_REBIND      !SMALLER		/* permit rebinding of keys at run-time	*/
#define OPT_FILEBACK    !SMALLER && !SYS_VMS	/* file backup style */
#define OPT_TERMCHRS    !SMALLER		/* set/show-terminal */
#define OPT_UPBUFF      !SMALLER		/* animated buffer-update */
#define OPT_WIDE_CTYPES !SMALLER		/* extra char-types tests */
#define OPT_HILITEMATCH !SMALLER		/* highlight all matches of a search */

/* "show" commands for the optional features */
#define OPT_SHOW_EVAL   !SMALLER && OPT_EVAL	/* "show-variables" */
#define OPT_SHOW_MAPS   !SMALLER 		/* display mapping for ":map" */
#define OPT_SHOW_REGS   !SMALLER		/* "show-registers" */
#define OPT_SHOW_TAGS   !SMALLER && OPT_TAGS	/* ":tags" displays tag-stack */

/* selections and attributed regions */
#define OPT_VIDEO_ATTRS !SMALLER
#define OPT_SELECTIONS  OPT_VIDEO_ATTRS

/* OPT_PSCREEN permits a direct interface to the pscreen data structure
 * in display.c. This allows us to avoid storing the screen data on the
 * screen interface side.
 */
#define OPT_PSCREEN  (XTOOLKIT && OPT_VIDEO_ATTRS)

#if	DISP_TERMCAP && !SMALLER
/* the setting "xterm-mouse" is always available, i.e.  the modetbl entry
 * is not conditional.  but all of the code that supports xterm-mouse _is_
 * ifdefed.  this makes it easier for users to be able to put "set
 * xterm-mouse" in their .vilerc which is shared between vile and xvile. 
 */
#define	OPT_XTERM	2	/* mouse-clicking support */
#else
#define	OPT_XTERM	0	/* vile doesn't recognize xterm mouse */
#endif

	/* any mouse capability */
#define OPT_MOUSE       (DISP_X11 || OPT_XTERM || OPT_MS_MOUSE || SYS_WINNT)

/*
 * If selections will be used to communicate between vile and other
 * applications, OWN_SELECTION must be defined to call the procedure
 * for establishing ownership of the selection.
 */
#if OPT_SELECTIONS && XTOOLKIT /* or others? */
#define OWN_SELECTION() own_selection()
#else
#define OWN_SELECTION()
#endif

/*
 * Special characters used in globbing
 */
#define	GLOB_MULTI	'*'
#define	GLOB_SINGLE	'?'
#define	GLOB_ELLIPSIS	"..."
#define	GLOB_RANGE	"[]"

/*
 * Configuration options for globbing
 */
#define	OPT_GLOB_ENVIRON	ENVFUNC && !SMALLER
#define	OPT_GLOB_ELLIPSIS	SYS_VMS || SYS_UNIX || SYS_OS2 || (SYS_MSDOS && !SMALLER)
#define	OPT_GLOB_PIPE		SYS_UNIX
#define	OPT_GLOB_RANGE		SYS_UNIX || SYS_OS2 || (SYS_MSDOS && !SMALLER)

/*	Debugging options	*/
#define	OPT_RAMSIZE		0  /* dynamic RAM memory usage tracking */
#define OPT_DEBUGMACROS		0  /* $debug triggers macro debugging	*/
#define	OPT_VISIBLE_MACROS	0  /* update display during keyboard macros*/
#define	OPT_VRFY_MALLOC		0  /* verify malloc operation (slow!) */
#define OPT_TRACE		0  /* turn on debug/trace (link with trace.o) */

/* That's the end of the user selections -- the rest is static definition */
/* (i.e. you shouldn't need to touch anything below here */
/* ====================================================================== */

#include <errno.h>
#if SYS_VMS
#include <perror.h>	/* defines 'sys_errlist[]' */
#endif
#if SYS_UNIX
# ifndef HAVE_EXTERN_ERRNO
extern	int	errno;	/* some systems don't define this in <errno.h> */
# endif
extern	int	sys_nerr;
# ifndef HAVE_EXTERN_SYS_ERRLIST
extern	char *	sys_errlist[];
# endif
#endif
#define	set_errno(code)	errno = code

	/* bit-mask definitions */
#define	lBIT(n)	(1L<<(n))
#define	iBIT(n) (1 <<(n))

/* use 'size_t' if we have it ... for unix systems, the configuration script
   will define size_t if it wasn't available in sys/types.h. */
#if SYS_UNIX || SYS_VMS || CC_NEWDOSCC
# if defined(lint) && (SYS_SUNOS||SYS_APOLLO)
#  define SIZE_T  size_t	/* note: SunOs 4.1.3 defines 'size_t' as 'int'*/
#  define ALLOC_T unsigned
#  if SYS_APOLLO && !APOLLO_STDLIB
#   undef SIZE_T
#   define SIZE_T int
#  endif
# else
#  define SIZE_T  size_t
#  define ALLOC_T size_t
# endif
#else
# define SIZE_T  int
# define ALLOC_T unsigned
#endif

#if SYS_APOLLO
# if !defined(__STDCPP__) && !defined(__GNUC__)
#  define ANSI_PROTOS 0 /* SR10.2 does not like protos w/o variable names */
# endif
#endif

#ifndef ANSI_PROTOS
#  if defined(__STDC__) || SYS_VMS || CC_NEWDOSCC || defined(__CLCC__)
#    define ANSI_PROTOS 1
#  else
#    define ANSI_PROTOS 0
#  endif
#endif

#if HAVE_STDARG_H
# define ANSI_VARARGS  1
#endif

#ifndef ANSI_VARARGS
# if defined(__STDC__) || SYS_VMS || CC_NEWDOSCC || defined(__CLCC__)
#  define ANSI_VARARGS 1	/* look in <stdarg.h> */
# else
#  define ANSI_VARARGS 0	/* look in <varargs.h> */
# endif
#endif

#if ANSI_PROTOS
# define P(a) a
#else
# define P(a) ()
#endif

#ifndef HAVE_GETHOSTNAME
#define HAVE_GETHOSTNAME 0
#endif

#if !(HAVE_STRCHR && HAVE_STRRCHR)
#define USE_INDEX 1
#endif

#ifdef USE_INDEX
#define strchr index
#define strrchr rindex
#if MISSING_EXTERN_RINDEX
extern char *index P((const char *, int));
extern char *rindex P((const char *, int));
#endif
#endif /* USE_INDEX */

#if STDC_HEADERS || HAVE_STRING_H
# include <string.h>
  /* An ANSI string.h and pre-ANSI memory.h might conflict.  */
# if !STDC_HEADERS && HAVE_MEMORY_H
#  include <memory.h>
# endif /* not STDC_HEADERS and HAVE_MEMORY_H */
#else /* not STDC_HEADERS and not HAVE_STRING_H */
# if HAVE_STRINGS_H
#  include <strings.h>
  /* memory.h and strings.h conflict on some systems */
  /* FIXME: should probably define memcpy and company in terms of bcopy,
     et al here */
# endif
#endif /* not STDC_HEADERS and not HAVE_STRING_H */

#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

/*	System dependent library redefinitions, structures and includes	*/

#if CC_NEWDOSCC && ! CC_CSETPP
#include <dos.h>
#endif

#if CC_NEWDOSCC && ! CC_DJGPP && ! CC_CSETPP
#undef peek
#undef poke
#define	peek(a,b,c,d)	movedata(a,b,FP_SEG(c),FP_OFF(c),d)
#define	poke(a,b,c,d)	movedata(FP_SEG(c),FP_OFF(c),a,b,d)
#define	movmem(a, b, c)		memcpy(b, a, c)
#endif

#if  CC_WATCOM
#include      <string.h>
#endif

#if  CC_WATCOM || CC_DJGPP
#define	movmem(a, b, c)		memcpy(b, a, c)
#endif

#if CC_MSC || CC_ZTC
#include <memory.h>
#endif


/* on MS-DOS we have to open files in binary mode to see the ^Z characters. */

#if SYS_MSDOS || SYS_WIN31 || SYS_OS2 || SYS_WINNT
#define FOPEN_READ	"rb"
#define FOPEN_WRITE	"wb"
#define FOPEN_APPEND	"ab"
#define FOPEN_UPDATE	"w+b"
#else
#define FOPEN_READ	"r"
#define FOPEN_WRITE	"w"
#define FOPEN_APPEND	"a"
#define FOPEN_UPDATE	"w+"
#endif


#if ! OPT_MSDOS_PATH /* DOS path / to \ conversions */
# define SL_TO_BSL(s)	(s)
# define bsl_to_sl(s)	(s)
# define sl_to_bsl_inplace(s)
# define bsl_to_sl_inplace(s)
#else
# define SLASHC slashc
# define is_slashc(c) (c == '\\' || c == '/')
# define SL_TO_BSL(s)	sl_to_bsl(s)
#endif

#if SYS_ST520
# define SLASHC '\\'
# define is_slashc(c) (c == '\\')
#endif

#ifndef SLASHC
# define SLASHC '/'
# define is_slashc(c) (c == '/')
#endif


#if SYS_VMS
#define	unlink(a)	delete(a)
#endif

/*	define some ability flags */

	/* intermediate config-controls for filec.c (needed in nemode.h) */
#if !SMALLER && !OPT_MAP_MEMORY
#define COMPLETE_FILES  (SYS_UNIX || SYS_MSDOS || SYS_VMS || SYS_OS2 || SYS_WINNT)
#define	COMPLETE_DIRS   (SYS_UNIX || SYS_MSDOS || SYS_OS2 || SYS_WINNT)
#else
#define COMPLETE_FILES  0
#define COMPLETE_DIRS   0
#endif

	/* semaphore may be needed to prevent interrupt of display-code */
#if defined(SIGWINCH) || OPT_WORKING
# define beginDisplay displaying++
# define endofDisplay displaying--
#else
# define beginDisplay
# define endofDisplay
#endif

#if OPT_WORKING
#define ShowWorking() (!global_b_val(MDTERSE) && global_g_val(GMDWORKING))
#else
#define ShowWorking() (!global_b_val(MDTERSE))
#endif

/* how to signal our process group: pass the 0 to 'getpgrp()' if we can,
 * since it's safer --- the machines where we can't are probably POSIX
 * machines with ANSI C.
 */
#if GETPGRP_HAS_ARG || MISSING_EXTERN_GETPGRP
# define GETPGRPCALL getpgrp(0)
#else
# define GETPGRPCALL getpgrp()
#endif

#if HAVE_KILLPG
# define signal_pg(sig) killpg( GETPGRPCALL, sig)
#else
# define signal_pg(sig)   kill(-GETPGRPCALL, sig)
#endif

#if	DISP_IBMPC || DISP_Z309
#define	MEMMAP	1
#else
#define	MEMMAP	0
#endif

#define UCHAR	unsigned char
#define UINT	unsigned int
#define USHORT	unsigned short
#define ULONG	unsigned long

/*	internal constants	*/

#if SYS_MSDOS || SYS_WIN31
#define	BITS_PER_INT	16
#endif

#ifndef	BITS_PER_INT
#define	BITS_PER_INT	32
#endif

#ifdef  MAXPATHLEN			/* usually in <sys/param.h>	*/
#define NFILEN	MAXPATHLEN		/* # of bytes, file name	*/
#else
#define NFILEN	256			/* # of bytes, file name	*/
#endif
#define NBUFN	20			/* # of bytes, buffer name	*/
#define NLINE	256			/* # of bytes, input line	*/
#define	NSTRING	128			/* # of bytes, string buffers	*/
#define NPAT	128			/* # of bytes, pattern		*/
#define HUGE	(1<<(BITS_PER_INT-2))	/* Huge number			*/
#define	NLOCKS	100			/* max # of file locks active	*/
#if DISP_X11
#define	NCOLORS	16			/* number of supported colors	*/
#else
#define	NCOLORS	8			/* number of supported colors	*/
#endif
#define	KBLOCK	256			/* sizeof kill buffer chunks	*/
#if !OPT_SELECTIONS
#define	NKREGS	36			/* number of kill buffers	*/
#else
#define NKREGS	37			/* When selections are enabled, we
					 * allocate an extra kill buffer for
					 * the current selection.
					 */
#define SEL_KREG (NKREGS-1)
#endif
#define	NBLOCK	16			/* line block chunk size	*/
#define MINWLNS	3			/* min # lines, window/screen	*/
#define MAXROWS	200			/* max # lines per screen	*/
#define MAXCOLS	200			/* max # cols per screen	*/

#define C_BLACK 0
#define C_WHITE (NCOLORS-1)

#define N_chars 256		/* must be a power-of-2		*/
#define HIGHBIT	0x0080		/* the meta bit			*/
#define CTLA	0x0100		/* ^A flag, or'ed in		*/
#define CTLX	0x0200		/* ^X flag, or'ed in		*/
#define SPEC	0x0400		/* special key (function keys)	*/
#define NOREMAP	0x0800		/* unremappable */
#define YESREMAP 0x1000		/* override noremap */
#define REMAPFLAGS (NOREMAP|YESREMAP)

#define kcod2key(c)	(c & (N_chars-1)) /* strip off the above prefixes */
#define	isspecial(c)	(c & ~(N_chars-1))

#define	char2int(c)	((int)(c & 0xff)) /* mask off sign-extension, etc. */

#define	PLURAL(n)	((n!=1)?"s":"")

#define	EOS     '\0'

#ifdef	FALSE
#undef	FALSE
#endif
#ifdef	TRUE
#undef	TRUE
#endif

#define FALSE	0			/* False, no, bad, etc. 	*/
#define TRUE	1			/* True, yes, good, etc.	*/
#define ABORT	2			/* Death, ESC, abort, etc.	*/
#define	FAILED	3			/* not-quite fatal false return	*/
#define	SORTOFTRUE	4		/* really!	*/

#define	STOP	0			/* keyboard macro not in use	*/
#define	PLAY	1			/*	"     "	  playing	*/
#define	RECORD	2			/*	"     "   recording	*/

#define QUOTED	TRUE
#define NOQUOTED	FALSE

#define DOMAP	TRUE
#define NODOMAP	FALSE

/* values for regionshape */
typedef enum {
	EXACT,
	FULLLINE,
	RECTANGLE
} REGIONSHAPE;

/* flook options */
#define FL_EXECABLE  iBIT(0)	/* same as X_OK */
#define FL_WRITEABLE iBIT(1)	/* same as W_OK */
#define FL_READABLE  iBIT(2)	/* same as R_OK */
#define FL_HERE      iBIT(3)	/* look in current directory */
#define FL_HOME      iBIT(4)	/* look in home directory */
#define FL_EXECDIR   iBIT(5)	/* look in execution directory */
#define FL_TABLE     iBIT(6)	/* look in table */
#define FL_PATH      iBIT(7)	/* look along execution-path */

#define FL_ANYWHERE  (FL_HERE|FL_HOME|FL_EXECDIR|FL_TABLE|FL_PATH)

/* indices into pathname[] array (epath.h) */
#define PATH_STARTUP_NAME	0
#define PATH_HELPFILE_NAME	1
#define PATH_EXECDIR		2
#define PATH_TABLEDIRS		3

/* definitions for name-completion */
#define	NAMEC		name_cmpl /* char for forcing name-completion */
#define	TESTC		test_cmpl /* char for testing name-completion */

/* kbd_string options */
#define KBD_EXPAND	iBIT(0)	/* do we want to expand %, #, : */
#define KBD_QUOTES	iBIT(1)	/* do we add and delete '\' chars for the caller */
#define KBD_LOWERC	iBIT(2)	/* do we force input to lowercase */
#define KBD_UPPERC	iBIT(3)	/* do we force input to uppercase */
#define KBD_NOEVAL	iBIT(4)	/* disable 'tokval()' (e.g., from buffer) */
#define KBD_MAYBEC	iBIT(5)	/* may be completed -- or not */
#define KBD_NULLOK	iBIT(6)	/* may be empty -- or not */
#define KBD_EXPCMD	iBIT(7)	/* expand %, #, : only in shell-command */
#define KBD_SHPIPE	iBIT(8)	/* expand, assuming shell-command */
#define KBD_NOMAP	iBIT(9) /* don't permit mapping via kbd_key() */

/* default option for 'mlreply' (used in modes.c also) */
#if !(SYS_MSDOS || SYS_WIN31 || SYS_OS2 || SYS_WINNT)
#define	KBD_NORMAL	KBD_EXPAND|KBD_QUOTES
#else
#define	KBD_NORMAL	KBD_EXPAND
#endif

/* reserve space for ram-usage option */
#if OPT_RAMSIZE
#define	LastMsgCol	(term.t_ncol - 10)
#else
#define	LastMsgCol	(term.t_ncol - 1)
#endif

/*	Directive definitions	*/

#if ! SMALLER

#define	DIF		0
#define DELSE		1
#define DENDIF		2
#define DGOTO		3
#define DRETURN		4
#define DENDM		5
#define DWHILE		6
#define	DENDWHILE	7
#define	DBREAK		8
#define DFORCE		9

#define NUMDIRS		10

#else

#define DENDM		0
#define NUMDIRS		1

#endif

/*
 * directions for the scan routines.
 */
#define	FORWARD	0			/* forward direction		*/
#define REVERSE	1			/* backwards direction		*/

#define FIOSUC  0			/* File I/O, success.		*/
#define FIOFNF  1			/* File I/O, file not found.	*/
#define FIOEOF  2			/* File I/O, end of file.	*/
#define FIOERR  3			/* File I/O, error.		*/
#define FIOMEM  4			/* File I/O, out of memory	*/
#define FIOABRT 5			/* File I/O, aborted		*/
	/* nonfatal codes */
#define FIOFUN  -1			/* File I/O, eod of file/bad line*/

/* three flavors of insert mode	*/
/* it's FALSE, or one of:	*/
#define INSERT 1
#define OVERWRITE 2
#define REPLACECHAR 3

/* kill register control -- values for kbflag */
#define KNEEDCLEAN   iBIT(0)		/* Kill register needs cleaning */
#define KYANK        iBIT(1)		/* Kill register resulted from yank */
#define KLINES       iBIT(2)		/* Kill register contains full lines */
#define KRECT        iBIT(3)		/* Kill register contains rectangle */
#define KAPPEND      iBIT(4)		/* Kill register should be appended */

/* operator types.  Needed mainly because word movement changes depending on
	whether operator is "delete" or not.  Aargh.  */
#define OPDEL 1
#define OPOTHER 2

/* define these so C-fence matching doesn't get confused when we're editing
	the cfence code itself */
#define LBRACE '{'
#define RBRACE '}'
#define LPAREN '('
#define RPAREN ')'
#define LBRACK '['
#define RBRACK ']'


/* separator used when scanning PATH environment variable */
#if SYS_VMS
#define	PATHCHR	','
#endif

#if OPT_MSDOS_PATH
#define	PATHCHR	';'
#endif

#ifndef PATHCHR				/* e.g., UNIX */
#define	PATHCHR	':'
#endif

/* how big is the ascii rep. of an int? */
#define	INTWIDTH	sizeof(int) * 3

/*	Macro argument token types					*/

#define	TKNUL	0			/* end-of-string		*/
#define	TKARG	1			/* interactive argument		*/
#define	TKBUF	2			/* buffer argument		*/
#define	TKVAR	3			/* user variables		*/
#define	TKENV	4			/* environment variables	*/
#define	TKFUN	5			/* function....			*/
#define	TKDIR	6			/* directive			*/
#define	TKLBL	7			/* line label			*/
#define	TKLIT	8			/* numeric literal		*/
#define	TKSTR	9			/* quoted string literal	*/
#define	TKCMD	10			/* command name			*/

/*	Internal defined functions					*/

#define	nextab(a)	(((a / curtabval) + 1) * curtabval)
#define	nextsw(a)	(((a / curswval) + 1) * curswval)

/* these are the bits that go into the _chartypes_ array */
/* the macros below test for them */
#if OPT_WIDE_CTYPES
#define chrBIT(n) lBIT(n)
#else
#define chrBIT(n) iBIT(n)
#endif

#define _upper    chrBIT(0)		/* upper case */
#define _lower    chrBIT(1)		/* lower case */
#define _digit    chrBIT(2)		/* digits */
#define _space    chrBIT(3)		/* whitespace */
#define _bspace   chrBIT(4)		/* backspace character (^H, DEL, and user's) */
#define _cntrl    chrBIT(5)		/* control characters, including DEL */
#define _print    chrBIT(6)		/* printable */
#define _punct    chrBIT(7)		/* punctuation */
#define _ident    chrBIT(8)		/* is typically legal in "normal" identifier */
#define _pathn    chrBIT(9)		/* is typically legal in a file's pathname */
#define _wild     chrBIT(10)		/* is typically a shell wildcard char */
#define _linespec chrBIT(11)		/* ex-style line range: 1,$ or 13,15 or % etc.*/
#define _fence    chrBIT(12)		/* a fence, i.e. (, ), [, ], {, } */
#define _nonspace chrBIT(13)		/* non-whitespace */
#define _qident   chrBIT(14)		/* is typically legal in "qualified" identifier */

#if OPT_WIDE_CTYPES
#define _scrtch   chrBIT(15)		/* legal in scratch-buffer names */
#define _shpipe   chrBIT(16)		/* legal in shell/pipe-buffer names */

#define	screen_to_bname(buf)\
	screen_string(buf,sizeof(buf),(CHARTYPE)(_pathn|_scrtch|_shpipe))
typedef	long CHARTYPE;
#else
#define	screen_to_bname(buf)\
	screen_string(buf,sizeof(buf),(CHARTYPE)(_pathn))
typedef short CHARTYPE;
#endif

#if SYS_WINNT
/* I dont like the warnings, so... */
#undef islower
#undef isupper
#undef isdigit
#undef isspace
#undef iscntrl
#undef isprint
#undef ispunct
#undef isalpha
#undef isalnum
#endif /* SYS_WINNT */

/* these intentionally match the ctypes.h definitions, except that
	they force the char to valid range first */
#define istype(sometype,c) ((_chartypes_[(c)&(N_chars-1)] & (sometype))!=0)
#define islower(c)	istype(_lower, c)
#define isupper(c)	istype(_upper, c)
#define isdigit(c)	istype(_digit, c)
#define isspace(c)	istype(_space, c)
#define iscntrl(c)	istype(_cntrl, c)
#define isprint(c)	istype(_print, c)
#define ispunct(c)	istype(_punct, c)
#define iswild(c)	istype(_wild, c)
#define isalpha(c)	istype(_lower|_upper, c)
#define isalnum(c)	istype(_lower|_upper|_digit, c)
#define isident(c)	istype(_ident, c)
#define ispath(c)	istype(_pathn, c)
#define isbackspace(c)	(istype(_bspace, c) || (c) == backspc)
#define islinespecchar(c)	istype(_linespec, c)
#define isfence(c)	istype(_fence, c)

/* macro for cases where return & newline are equivalent */
#define	isreturn(c)	((c == '\r') || (c == '\n'))

/* macro for whitespace (non-return) */
#define	isblank(c)      ((c == '\t') || (c == ' '))

/* DIFCASE represents the difference between upper
   and lower case letters, DIFCNTRL the difference between upper case and
   control characters.	They are xor-able values.  */
#define	DIFCASE		0x20
#define	DIFCNTRL	0x40
#define toupper(c)	((c)^DIFCASE)
#define tolower(c)	((c)^DIFCASE)
#define tocntrl(c)	((c)^DIFCNTRL)
#define toalpha(c)	((c)^DIFCNTRL)

#define nocase_eq(bc,pc)	((bc) == (pc) || \
			(isalpha(bc) && (((bc) ^ DIFCASE) == (pc))))

#define ESC		tocntrl('[')
#define BEL		tocntrl('G')	/* ascii bell character		*/
#define CONTROL_A	tocntrl('A')	/* for cntl_a attribute sequences */

#if !(SYS_MSDOS && CC_DJGPP)
/* some systems need a routine to check for interrupts.  most don't, and
 * the routine overhead can be expensive in some places
 */
# define interrupted() (am_interrupted != 0)
#endif

#define ABORTED(c) ((c) == abortc || (c) == intrc || interrupted())

/*
 * Definitions etc. for regexp(3) routines.
 *
 *	the regexp code is:
 *	Copyright (c) 1986 by University of Toronto.
 *	Written by Henry Spencer.  Not derived from licensed software.
 *
 */
#define NSUBEXP  10
typedef struct regexp {
	char *startp[NSUBEXP];
	char *endp[NSUBEXP];
	SIZE_T mlen;		/* convenience:  endp[0] - startp[0] */
	char regstart;		/* Internal use only. */
	char reganch;		/* Internal use only. */
	int regmust;		/* Internal use only. */
	int regmlen;		/* Internal use only. */
	SIZE_T size;		/* vile addition -- how big is this */
	char program[1];	/* Unwarranted chumminess with compiler. */
} regexp;

/*
 * The first byte of the regexp internal "program" is actually this magic
 * number; the start node begins in the second byte.
 */
#define	REGEXP_MAGIC	0234

#ifndef CHARBITS
#define	UCHAR_AT(p)	((int)*(UCHAR *)(p))
#else
#define	UCHAR_AT(p)	((int)*(p)&CHARBITS)
#endif

/* end of regexp stuff */

/*
 * Definitions for 'tbuff.c' (temporary/dynamic char-buffers)
 */
typedef	struct	_tbuff	{
	char *	tb_data;	/* the buffer-data */
	ALLOC_T	tb_size;	/* allocated size */
	ALLOC_T	tb_used;	/* total used in */
	ALLOC_T	tb_last;	/* last put/get index */
	int	tb_endc;
	} TBUFF;

/*
 * Definitions for 'itbuff.c' (temporary/dynamic int-buffers)
 */
typedef	struct	_itbuff	{
	int *	itb_data;	/* the buffer-data */
	ALLOC_T	itb_size;	/* allocated size */
	ALLOC_T	itb_used;	/* total used in */
	ALLOC_T	itb_last;	/* last put/get index */
	int	itb_endc;
	} ITBUFF;

/*
 * Primitive types
 */
typedef	int		L_NUM;		/* line-number */
typedef	int		C_NUM;		/* column-number */
typedef	struct {
    unsigned short flgs;
    unsigned short cook;
} L_FLAG;		/* LINE-flags */

typedef	ULONG		CMDFLAGS;	/* CMDFUNC flags */
typedef	long		B_COUNT;	/* byte-count */

/*
 * All text is kept in circularly linked lists of "LINE" structures. These
 * begin at the header line. This line is pointed to by the "BUFFER".
 * Each line contains:
 *  number of bytes in the line (the "used" size), 
 *  the size of the text array,
 *  the text.
 * The end of line is not stored as a byte; it's implied. Future
 * additions may include update hints, and a list of marks into the line.
 *
 * Lines are additionally sometimes stacked in undo lists.
 */
#if OPT_MAP_MEMORY
typedef	long	BLK_T;
typedef	int	OFF_T;
typedef	struct	{ BLK_T blk; OFF_T off; } LINEPTR;
#else
typedef	struct	LINE*	LINEPTR;
#endif

typedef struct	LINE {
	LINEPTR l_fp;			/* Link to the next line	*/
	LINEPTR l_bp;			/* Link to the previous line	*/
	union {
		SIZE_T	l_sze;		/* Allocated size 		*/
		C_NUM	l_fo;		/* forward undo dot offs (undo only) */
	} l_s_fo;
	union {
		L_NUM	l_nmbr;		/* line-# iff b_numlines > 0	*/
		C_NUM	l_bo;		/* backward undo dot offs (undo only) */
	} l_n_bo;
	int	l_used;			/* Used size (may be negative)	*/
	union {
	    char *l_txt;		/* The data for this line	*/
	    LINEPTR l_nxt;		/* if an undo stack separator,	*/
	} lt;				/*  a pointer to the next one	*/
#if OPT_MAP_MEMORY
	struct
#else
	union
#endif
	{
	    LINEPTR	l_stklnk;	/* Link for undo stack		*/
	    L_FLAG	l_flg;		/* flags for undo ops		*/
	} l;
}	LINE;

#define l_size		l_s_fo.l_sze
#define l_forw_offs	l_s_fo.l_fo
#define l_number	l_n_bo.l_nmbr
#define l_back_offs	l_n_bo.l_bo
#define l_text		lt.l_txt
#define l_nextsep	lt.l_nxt

#define l_undo_cookie	l_flg.cook
#define l_flag		l_flg.flgs

/* LINE.l_flag values */
#define LCOPIED  lBIT(0)	/* original line is already on an undo stack */
#define LGMARK   lBIT(1)	/* line matched a global scan */
#define LTRIMMED lBIT(2)	/* line doesn't have newline to display */

/* macros to ease the use of lines */
#define	for_each_line(lp,bp) for (lp = lForw(buf_head(bp)); \
					lp != l_ref(buf_head(bp)); \
					lp = lforw(lp))

#define l_nxtundo		l.l_stklnk

	/*
	 * Special values used in LINE.l_used
	 */
#define LINENOTREAL	((int)(-1)) /* for undo, marks an inserted line */
#define LINEUNDOPATCH	((int)(-2)) /* provides stack patching value for undo */
/* #define MARKPATCH	((int)(-3)) *//*	unused */
#define STACKSEP	((int)(-4)) /* delimit set of changes on undo stack */
#define PURESTACKSEP	((int)(-5)) /* as above, but buffer unmodified before */
					/* this change */

	/*
	 * If we are configured with mapped-data, references to LINE pointers
	 * are translated by functions to/from LINEPTR structs (see file tmp.c).
	 *
	 * LINEPTR is a structure holding the block- and offset-value in the
	 * mapped-data file.  Each LINEPTR corresponds to a LINE struct.
	 *
	 * Basically, l_ref and l_ptr map into functions that translate between
	 * 'LINE *' and LINEPTR.  If you forget to use one of these functions,
	 * then the C-compiler will nag you about it if you turn on
	 * OPT_MAP_MEMORY, since you cannot really assign or compare between
	 * these two types.  (To be honest, my compiler doesn't catch all of
	 * the comparison cases; I also linted the code). 
	 */
#if OPT_MAP_MEMORY
#define fast_ptr	/* can't do it */
#define	same_ptr(a,b)	(((a).blk == (b).blk) && ((a).off == (b).off))
#define	null_ptr	nullmark.l
#define	rls_region()	l_region((REGION *)0)
#else
#define fast_ptr	register
#define	same_ptr(a,b)	((a) == (b))
#define	null_ptr	(LINE *)0
#define	l_ref(lp)	(lp)
#define	l_ptr(lp)	(lp)
#define set_lforw(a,b)	lforw(a) = (b)
#define set_lback(a,b)	lback(a) = (b)
#define lforw(lp)	(lp)->l_fp
#define lback(lp)	(lp)->l_bp
#define	rls_region()	/* unused */
#endif

	/*
	 * Macros for referencing fields in the LINE struct.
	 */
#define lgetc(lp, n)		char2int((lp)->l_text[(n)])
#define lputc(lp, n, c) 	((lp)->l_text[(n)]=(c))
#define llength(lp)		((lp)->l_used)

#define liscopied(lp)		((lp)->l.l_undo_cookie == current_undo_cookie)
#define lsetcopied(lp)		((lp)->l.l_undo_cookie = current_undo_cookie)
#define lsetnotcopied(lp)	((lp)->l.l_undo_cookie = 0)

#define lismarked(lp)		((lp)->l.l_flag & LGMARK)
#define lsetmarked(lp)		((lp)->l.l_flag |= LGMARK)
#define lsetnotmarked(lp)	((lp)->l.l_flag &= ~LGMARK)
#define lflipmark(lp)		((lp)->l.l_flag ^= LGMARK)

#define listrimmed(lp)		((lp)->l.l_flag & LTRIMMED)
#define lsettrimmed(lp)		((lp)->l.l_flag |= LTRIMMED)
#define lsetnottrimmed(lp)	((lp)->l.l_flag &= ~LTRIMMED)
#if !OPT_MAP_MEMORY
#define lsetclear(lp)		((lp)->l.l_flag = (lp)->l.l_undo_cookie = 0)
#endif

#define lisreal(lp)		((lp)->l_used >= 0)
#define lisnotreal(lp)		((lp)->l_used == LINENOTREAL)
#define lislinepatch(lp)	((lp)->l_used == LINEUNDOPATCH)
/* #define lismarkpatch(lp)	((lp)->l_used == MARKPATCH) */
#define lispatch(lp)		(lislinepatch(lp) /* || lismarkpatch(lp) */ )
#define lisstacksep(lp)		((lp)->l_used == STACKSEP || \
					(lp)->l_used == PURESTACKSEP)
#define lispurestacksep(lp)	((lp)->l_used == PURESTACKSEP)

	/*
	 * Corresponding (mixed-case : mixed-type) names for LINEPTR references
	 *
	 * I introduced the mixed-case macros so that the l_ref/l_ptr
	 * translations wouldn't be _too_ intrusive (and also to limit the
	 * amount of text that I changed).  They all serve the same function.
	 * (dickey@software.org)
	 */
#define lGetc(lp, n)		lgetc(l_ref(lp), n)
#define lPutc(lp, n, c)		lputc(l_ref(lp), n, c)
#define lLength(lp)		llength(l_ref(lp))

#if OPT_MAP_MEMORY

#define	lForw(lp)		lforw_p2r(lp)
#define	lBack(lp)		lback_p2r(lp)

#define	lFORW(lp)		lforw_p2p(lp)
#define	lBACK(lp)		lback_p2p(lp)

#define	set_lForw(dst,src)	set_lforw_p2r(dst, src)
#define	set_lBack(dst,src)	set_lback_p2r(dst, src)

#define	set_lFORW(dst,src)	set_lforw_p2p(dst, src)
#define	set_lBACK(dst,src)	set_lback_p2p(dst, src)

#else	/* 'LINEPTR' == 'LINE*' */

#define	lForw(lp)		lforw(l_ref(lp))
#define	lBack(lp)		lback(l_ref(lp))

#define	lFORW(lp)		l_ptr(lForw(lp))
#define	lBACK(lp)		l_ptr(lBack(lp))

#define	set_lForw(dst,src)	set_lforw(dst, src)
#define	set_lBack(dst,src)	set_lback(dst, src)

#define	set_lFORW(dst,src)	set_lforw(dst, src)
#define	set_lBACK(dst,src)	set_lback(dst, src)

#endif


/* marks are a line and an offset into that line */
typedef struct MARK {
	LINEPTR l;
	C_NUM o;
} MARK;

/* some macros that take marks as arguments */
#define is_at_end_of_line(m)	(m.o == lLength(m.l))
#define is_empty_line(m)	(lLength(m.l) == 0)
#define sameline(m1,m2)		(same_ptr(m1.l, m2.l))
#define samepoint(m1,m2)	(sameline(m1,m2) && (m1.o == m2.o))
#define char_at(m)		(lGetc(m.l,m.o))
#define put_char_at(m,c)	(lPutc(m.l,m.o,c))
#define is_header_line(m,bp)	(same_ptr(m.l, buf_head(bp)))
#define is_last_line(m,bp)	(lForw(m.l) == l_ref(buf_head(bp)))
#define is_first_line(m,bp)	(lBack(m.l) == l_ref(buf_head(bp)))

/*
 * The starting position of a region, and the size of the region in
 * characters, is kept in a region structure.  Used by the region commands.
 */
typedef struct	{
	MARK 	r_orig;			/* Origin LINE address. 	*/
	MARK	r_end;			/* Ending LINE address. 	*/
	C_NUM	r_leftcol;		/* Leftmost column. 		*/
	C_NUM	r_rightcol;		/* Rightmost column. 		*/
	B_COUNT	r_size; 		/* Length in characters.	*/
#if OPT_SELECTIONS
	unsigned short	r_attr_id;	/* id of corresponding display  */
#endif
}	REGION;

#if OPT_COLOR || DISP_X11 || OPT_HILITEMATCH
typedef unsigned short VIDEO_ATTR;	/* assumption: short is at least 16 bits */
#else
typedef unsigned char VIDEO_ATTR;
#endif

#define VACURS	0x01			/* cursor -- this is intentionally
					 * the same as VADIRTY. It should
					 * not be used anywhere other than
					 * in specific places in the low
					 * level drivers (e.g, x11.c).
					 */
#define VAMLFOC	0x02			/* modeline w/ focus		*/
#define VAML	0x04			/* standard mode line (no focus)*/
#define VASEL	0x08			/* selection			*/
#define	VAREV	0x10			/* reverse video		*/
#define	VAUL	0x20			/* underline			*/
#define	VAITAL	0x40			/* italics			*/
#define	VABOLD	0x80			/* bold				*/
#define VAOWNER 0x0f00			/* owner mask			*/
#define VACOLOR 0xf000			/* color mask			*/
#define VCOLORNUM(attr) (((attr) & VACOLOR) >> 12)
#define VCOLORATTR(num) ((num) << 12)

/* who owns an attributed region -- so we can delete them independently */
#define VOWNER(attr)	((attr) & VAOWNER)
#define VOWN_MATCHES	0x0100
#define VOWN_OPERS	0x0200
#define VOWN_SELECT	0x0300
#define VOWN_CTLA	0x0400

/* The VATTRIB macro masks out those bits which should not be considered
 * for comparison purposes
 */

#if OPT_PSCREEN
#define VADIRTY	0x01			/* cell needs to be written out */
#define VATTRIB(attr) ((attr) & ~(VAOWNER|VADIRTY))
#else
#define VADIRTY 0x0			/* nop for all others */
#define VATTRIB(attr) ((attr) & ~(VAOWNER))
#endif

/* grow (or initially allocate) a vector of newsize types, pointed to by
 * ptr.  this is used primarily for resizing the screen
 * the studious will note this is a lot like realloc.   but realloc
 * doesn't guarantee to preserve contents if if fails, and this also
 * zeroes the new space.
 */
#define GROW(ptr, type, oldsize, newsize) \
{ \
	int tmpold = oldsize; \
	type *tmpp; \
	tmpp = typeallocn(type, newsize); \
	if (tmpp == NULL) \
		return FALSE; \
 \
	if (ptr) { \
		(void) memcpy((char *)tmpp, (char *)ptr, tmpold * sizeof(type)); \
		free((char *)ptr); \
	} else { \
		tmpold = 0; \
	} \
	ptr = tmpp; \
	(void) memset ((char *)(ptr+tmpold), 0, (newsize - tmpold) * sizeof(type)); \
}

/*
 * An attributed region is attached to a buffer and indicates how the
 * region should be displayed; eg. inverse video, underlined, etc.
 */

typedef struct _aregion {
	struct _aregion	*ar_next;
	REGION		ar_region;
	VIDEO_ATTR	ar_vattr;
	REGIONSHAPE	ar_shape;
}	AREGION;


/* settable values have their names stored here, along with a synonym, and
	what type they are */
struct VALNAMES {
		char *name;
		char *shortname;
		short type;
		short winflags;
};
/* the values of VALNAMES->type */
#define VALTYPE_INT 0
#define VALTYPE_STRING 1
#define VALTYPE_BOOL 2
#define VALTYPE_REGEX 3
#define VALTYPE_COLOR 4

typedef	struct {
	char *pat;
	regexp *reg;
} REGEXVAL;

/* this is to ensure values can be of any type we wish.
   more can be added if needed.  */
union V {
	int i;
	char *p;
	REGEXVAL *r;
};

struct VAL {
	union V v;
	union V *vp;
};

typedef	struct	{
	struct VALNAMES *names;
	struct VAL      *local;
	struct VAL      *global;
} VALARGS;

	/*
	 * Values are either local or global. We distinguish the two cases
	 * by whether the value-pointer points into the VAL-struct or not.
	 */
#define is_local_val(lv,which)          (lv[which].vp == &(lv[which].v))
#define make_local_val(lv,which)        (lv[which].vp = &(lv[which].v))
#define make_global_val(lv,gv,which)    (lv[which].vp = &(gv[which].v))

/* these are masks for the WINDOW.w_flag hint */
#define WFFORCE 0x01			/* Window needs forced reframe	*/
#define WFMOVE	0x02			/* Movement from line to line	*/
#define WFEDIT	0x04			/* Editing within a line	*/
#define WFHARD	0x08			/* Better do a full display	*/
#define WFMODE	0x10			/* Update mode line.		*/
#define WFCOLR	0x20			/* Needs a color change		*/
#define WFKILLS	0x40			/* something was deleted	*/
#define WFINS	0x80			/* something was inserted	*/
#define WFSTAT	0x100			/* Update mode line (info only).*/
#define WFSBAR	0x200			/* Update scroll bar(s) */

/* define indices for GLOBAL, BUFFER, WINDOW modes */
#include "nemode.h"

/* macros for setting GLOBAL modes */

#define global_g_val(which) global_g_values.gv[which].v.i
#define set_global_g_val(which,val) global_g_val(which) = val
#define global_g_val_ptr(which) global_g_values.gv[which].v.p
#define set_global_g_val_ptr(which,val) global_g_val_ptr(which) = val
#define global_g_val_rexp(which) global_g_values.gv[which].v.r
#define set_global_g_val_rexp(which,val) global_g_val_rexp(which) = val

/* these are window properties affecting window appearance _only_ */
typedef struct	{
	MARK 	w_dt;		/* Line containing "."	       */
		/* i don't think "mark" needs to be here -- I think it 
			could safely live only in the buffer -pgf */
#ifdef WINMARK
	MARK 	w_mk;	        /* Line containing "mark"      */
#endif
	MARK 	w_ld;	        /* Line containing "lastdotmark"*/
	MARK 	w_tld;	        /* Line which may become "lastdotmark"*/
	MARK 	w_ln;		/* Top line in the window (offset unused) */
#if OPT_MOUSE
	int	insmode;	
#endif
	W_VALUES w_vals;
} W_TRAITS;

#define global_w_val(which) global_w_values.wv[which].v.i
#define set_global_w_val(which,val) global_w_val(which) = val
#define global_w_val_ptr(which) global_w_values.wv[which].v.p
#define set_global_w_val_ptr(which,val) global_w_val_ptr(which) = val

#define w_val(wp,val) (wp->w_values.wv[val].vp->i)
#define set_w_val(wp,which,val) w_val(wp,which) = val
#define w_val_ptr(wp,val) (wp->w_values.wv[val].vp->p)
#define set_w_val_ptr(wp,which,val) w_val_ptr(wp,which) = val

#define make_local_w_val(wp,which)  \
	make_local_val(wp->w_values.wv, which)
#define make_global_w_val(wp,which)  \
	make_global_val(wp->w_values.wv, global_wvalues.wv, which)

#define is_local_w_val(wp,which)  \
	is_local_val(wp->w_values.wv,which)

#if OPT_COLOR
#define gfcolor global_w_val(WVAL_FCOLOR)
#define gbcolor global_w_val(WVAL_BCOLOR)
#else
#define gfcolor C_WHITE
#define gbcolor C_BLACK
#endif

/*
 * Text is kept in buffers. A buffer header, described below, exists for every
 * buffer in the system. The buffers are kept in a big list, so that commands
 * that search for a buffer by name can find the buffer header. There is a
 * safe store for the dot and mark in the header, but this is only valid if
 * the buffer is not being displayed (that is, if "b_nwnd" is 0). The text for
 * the buffer is kept in a circularly linked list of lines, with a pointer to
 * the header line in "b_line"	Buffers may be "Inactive" which means the files associated with them
 * have not been read in yet. These get read in at "use buffer" time.
 */

typedef struct	BUFFER {
	MARK 	b_line;		/* Link to the header LINE (offset unused) */
	struct	BUFFER *b_bufp; 	/* Link to next BUFFER		*/
	MARK 	*b_nmmarks;		/* named marks a-z		*/
#if OPT_SELECTIONS
	AREGION	*b_attribs;		/* attributed regions		*/
#endif
	B_VALUES b_values;		/* buffer traits we inherit from */
					/*  global values		*/
	W_TRAITS b_wtraits;		/* saved window traits, while we're */
					/*  not displayed		*/
	B_COUNT	b_bytecount;		/* # of chars			*/
	L_NUM	b_linecount;		/* no. lines as of last read/write */
	LINEPTR b_udstks[2];		/* undo stack pointers		*/
	MARK 	b_uddot[2];		/* Link to "." before undoable op*/
	short	b_udstkindx;		/* which of above to use	*/
	LINEPTR b_udtail;		/* tail of undo backstack	*/
	LINEPTR b_udlastsep;		/* last stack separator pushed	*/
	int	b_udcount;		/* how many undo's we can do	*/
#if !OPT_MAP_MEMORY
	LINEPTR	b_LINEs;		/* block-malloced LINE structs */
	LINEPTR	b_LINEs_end;		/* end of 	"	"	" */
	LINEPTR	b_freeLINEs;		/* list of free " 	"	" */
	UCHAR	*b_ltext;		/* block-malloced text */
	UCHAR	*b_ltext_end;		/* end of block-malloced text */
#endif
	LINEPTR	b_ulinep;		/* pointer at 'Undo' line	*/
	int	b_active;		/* window activated flag	*/
	int	b_nwnd;		        /* Count of windows on buffer   */
	int	b_flag;		        /* Flags 		        */
	short	b_acount;		/* auto-save count	        */
	char	*b_fname;		/* File name			*/
	int	b_fnlen;		/* length of filename		*/
	char	b_bname[NBUFN]; 	/* Buffer name			*/
#if	OPT_ENCRYPT
	char	b_key[NPAT];		/* current encrypted key	*/
#endif
#ifdef	MDCHK_MODTIME
	time_t	b_modtime;		/* file's last-modification time */
	time_t	b_modtime_at_warn;	/* file's modtime when user warned */
#endif
#if	OPT_UPBUFF
	int	(*b_upbuff) P((struct BUFFER *)); /* call to recompute  */
#endif
#if	OPT_B_LIMITS
	int	b_lim_left;		/* extra left-margin (cf:show-reg) */
#endif
	struct	BUFFER *b_relink; 	/* Link to next BUFFER (sorting) */
	int	b_created;
	int	b_last_used;
#if OPT_HILITEMATCH
	short	b_highlight;
#endif
}	BUFFER;

/*
 * Special symbols for scratch-buffer names.
 */
#define	SCRTCH_LEFT  "["
#define	SCRTCH_RIGHT "]"
#define	SHPIPE_LEFT  "!"

/* warning:  code in file.c and fileio.c knows how long the shell, pipe, and
	append prefixes are (e.g. fn += 2 when appending) */
#define	isShellOrPipe(s)  ((s)[0] == SHPIPE_LEFT[0])
#define	isInternalName(s) (isShellOrPipe(s) || is_internalname(s))
#define	isAppendToName(s) ((s)[0] == '>' && (s)[1] == '>')

/* shift-commands can be repeated when typed on :-command */
#define isRepeatable(c)   ((c) == '<' || (c) == '>')

/*
 * Macros for manipulating buffer-struct members.
 */
#define	for_each_buffer(bp) for (bp = bheadp; bp; bp = bp->b_bufp)

#define global_b_val(which) global_b_values.bv[which].v.i
#define set_global_b_val(which,val) global_b_val(which) = val
#define global_b_val_ptr(which) global_b_values.bv[which].v.p
#define set_global_b_val_ptr(which,val) global_b_val_ptr(which) = val
#define global_b_val_rexp(which) global_b_values.bv[which].v.r
#define set_global_b_val_rexp(which,val) global_b_val_rexp(which) = val

#define b_val(bp,val) (bp->b_values.bv[val].vp->i)
#define set_b_val(bp,which,val) b_val(bp,which) = val
#define b_val_ptr(bp,val) (bp->b_values.bv[val].vp->p)
#define set_b_val_ptr(bp,which,val) b_val_ptr(bp,which) = val
#define b_val_rexp(bp,val) (bp->b_values.bv[val].vp->r)
#define set_b_val_rexp(bp,which,val) b_val_rexp(bp,which) = val

#define window_b_val(wp,val) \
 	((wp != 0 && wp->w_bufp != 0) \
 		? b_val(wp->w_bufp,val) \
		: global_b_val(val))

#define make_local_b_val(bp,which)  \
		make_local_val(bp->b_values.bv, which)
#define make_global_b_val(bp,which)  \
		make_global_val(bp->b_values.bv, global_b_values.bv, which)

#define is_local_b_val(bp,which)  \
	is_local_val(bp->b_values.bv,which)

#define is_empty_buf(bp) (lForw(buf_head(bp)) == l_ref(buf_head(bp)))

#define b_dot     b_wtraits.w_dt
#ifdef WINMARK
#define b_mark    b_wtraits.w_mk
#endif
#define b_lastdot b_wtraits.w_ld
#define b_tentative_lastdot b_wtraits.w_tld
#define b_wline   b_wtraits.w_ln

/* buffer-name may not necessarily have trailing null */
#define eql_bname(bp,name) !strncmp(bp->b_bname, name, NBUFN)

/* values for b_flag */
#define BFINVS     0x01			/* Internal invisible buffer	*/
#define BFCHG      0x02			/* Changed since last write	*/
#define BFSCRTCH   0x04			/* scratch -- gone on last close */
#define BFARGS     0x08			/* set for ":args" buffers */
#define BFIMPLY    0x010		/* set for implied-# buffers */
#define BFSIZES    0x020		/* set if byte/line counts current */
#define BFUPBUFF   0x040		/* set if buffer should be updated */

/* macros for manipulating b_flag */
#define b_is_implied(bp)        ((bp)->b_flag & (BFIMPLY))
#define b_is_argument(bp)       ((bp)->b_flag & (BFARGS))
#define b_is_changed(bp)        ((bp)->b_flag & (BFCHG))
#define b_is_invisible(bp)      ((bp)->b_flag & (BFINVS))
#define b_is_scratch(bp)        ((bp)->b_flag & (BFSCRTCH))
#define b_is_temporary(bp)      ((bp)->b_flag & (BFINVS|BFSCRTCH))
#define b_is_counted(bp)        ((bp)->b_flag & (BFSIZES))
#define b_is_obsolete(bp)       ((bp)->b_flag & (BFUPBUFF))

#define b_set_flags(bp,flags)   (bp)->b_flag |= (flags)
#define b_set_changed(bp)       b_set_flags(bp, BFCHG)
#define b_set_counted(bp)       b_set_flags(bp, BFSIZES)
#define b_set_invisible(bp)     b_set_flags(bp, BFINVS)
#define b_set_obsolete(bp)      b_set_flags(bp, BFUPBUFF)
#define b_set_scratch(bp)       b_set_flags(bp, BFSCRTCH)

#define b_clr_flags(bp,flags)   (bp)->b_flag &= ~(flags)
#define b_clr_changed(bp)       b_clr_flags(bp, BFCHG)
#define b_clr_counted(bp)       b_clr_flags(bp, BFSIZES)
#define b_clr_obsolete(bp)      b_clr_flags(bp, BFUPBUFF)

#if OPT_HILITEMATCH
#define b_match_attrs_dirty(bp)	(bp)->b_highlight |= HILITE_DIRTY
#else
#define b_match_attrs_dirty(bp)
#endif

#if OPT_B_LIMITS
#define b_left_margin(bp)       bp->b_lim_left
#define b_set_left_margin(bp,n) b_left_margin(bp) = n
#else
#define b_left_margin(bp)       0
#define b_set_left_margin(bp,n)
#endif

#if OPT_HILITEMATCH
#define HILITE_ON	1
#define HILITE_DIRTY	2
#endif

/* macro for iterating over the marks associated with the current buffer */

#if OPT_VIDEO_ATTRS
#define do_mark_iterate(mp, statement)			\
    do {						\
	struct MARK *mp;				\
	int	     dmi_idx;				\
	AREGION     *dmi_ap = curbp->b_attribs;		\
	if (curbp->b_nmmarks != NULL)			\
	    for (dmi_idx=0; dmi_idx < 26; dmi_idx++) {	\
		mp = &(curbp->b_nmmarks[dmi_idx]);	\
		statement				\
	    }						\
	if (dmi_ap != NULL) {				\
	    while (dmi_ap != NULL) {			\
		mp = &dmi_ap->ar_region.r_orig;		\
		statement				\
		mp = &dmi_ap->ar_region.r_end;		\
		statement				\
		dmi_ap = dmi_ap->ar_next;		\
	    }						\
	    sel_reassert_ownership(curbp);		\
	}						\
    } while (0)
#else /* OPT_VIDEO_ATTRS */
#define do_mark_iterate(mp, statement)			\
    do							\
	if (curbp->b_nmmarks != NULL) {			\
	    struct MARK *mp;				\
	    int dmi_idx;				\
	    for (dmi_idx=0; dmi_idx < 26; dmi_idx++) {	\
		mp = &(curbp->b_nmmarks[dmi_idx]);	\
		statement				\
	    }						\
	}						\
    while (0)
#endif /* OPT_VIDEO_ATTRS */

/*
 * There is a window structure allocated for every active display window. The
 * windows are kept in a big list, in top to bottom screen order, with the
 * listhead at "wheadp". Each window contains its own values of dot and mark.
 * The flag field contains some bits that are set by commands to guide
 * redisplay. Although this is a bit of a compromise in terms of decoupling,
 * the full blown redisplay is just too expensive to run for every input
 * character.
 */

typedef struct	WINDOW {
	W_TRAITS w_traits;		/* features of the window we should */
					/*  remember between displays */
	struct	WINDOW *w_wndp; 	/* Next window			*/
	BUFFER  *w_bufp; 		/* Buffer displayed in window	*/
	int	w_toprow;	        /* Origin 0 top row of window   */
	int	w_ntrows;	        /* # of rows of text in window  */
	int	w_force; 	        /* If non-zero, forcing row.    */
	int	w_flag;		        /* Flags.		        */
	ULONG	w_split_hist;		/* how to recombine deleted windows */
#ifdef WMDRULER
	int	w_ruler_line;
	int	w_ruler_col;
#endif
}	WINDOW;

#define	for_each_window(wp) for (wp = wheadp; wp; wp = wp->w_wndp)

#define w_dot     w_traits.w_dt
#ifdef WINMARK
#define w_mark    w_traits.w_mk
#endif
#define w_lastdot w_traits.w_ld
#define w_tentative_lastdot w_traits.w_tld
#define w_line    w_traits.w_ln
#define w_values  w_traits.w_vals

#define mode_row(wp)	((wp)->w_toprow + (wp)->w_ntrows)
#define	buf_head(bp)	((bp)->b_line.l)
#define	win_head(wp)	buf_head((wp)->w_bufp)

#define DOT curwp->w_dot
#if OPT_MOUSE
#define insertmode (curwp->w_traits.insmode)
#endif /* OPT_MOUSE */
#ifdef WINMARK
#define MK curwp->w_mark
#else
#define MK Mark
#endif

	/* we use left-margin for protecting the prefix-area of [Registers]
	 * from cut/paste selection.
	 */
#define w_left_margin(wp) b_left_margin(wp->w_bufp)

/*
 * The editor communicates with the display using a high level interface. A
 * "TERM" structure holds useful variables, and indirect pointers to routines
 * that do useful operations. The low level get and put routines are here too.
 * This lets a terminal, in addition to having non standard commands, have
 * funny get and put character code too. The calls might get changed to
 * "termp->t_field" style in the future, to make it possible to run more than
 * one terminal type.
 */
typedef struct	{
	int	t_mrow;			/* max number of rows allowable */
	int	t_nrow; 		/* current number of rows used	*/
	int	t_mcol; 		/* max Number of columns.	*/
	int	t_ncol; 		/* current Number of columns.	*/
	int	t_margin;		/* min margin for extended lines*/
	int	t_scrsiz;		/* size of scroll region "	*/
	int	t_pause;		/* # times thru update to pause */
	void	(*t_open) P((void));	/* Open terminal at the start.	*/
	void	(*t_close) P((void));	/* Close terminal at end.	*/
	void	(*t_kopen) P((void));	/* Open keyboard		*/
	void	(*t_kclose) P((void));	/* close keyboard		*/
	int	(*t_getchar) P((void)); /* Get character from keyboard. */
	void	(*t_putchar) P((int)); 	/* Put character to display.	*/
	int	(*t_typahead) P((void));/* character ready?		*/
	void	(*t_flush) P((void));	/* Flush output buffers.	*/
	void	(*t_move) P((int,int));	/* Move the cursor, origin 0.	*/
	void	(*t_eeol) P((void));	/* Erase to end of line.	*/
	void	(*t_eeop) P((void));	/* Erase to end of page.	*/
	void	(*t_beep) P((void));	/* Beep.			*/
	void	(*t_rev) P((int));	/* set reverse video state	*/
	int	(*t_rez) P((char *));	/* change screen resolution	*/
#if	OPT_COLOR
	void	(*t_setfor) P((int));	/* set foreground color		*/
	void	(*t_setback) P((int));	/* set background color		*/
#endif
	void	(*t_scroll) P((int,int,int)); /* scroll a region of the screen */
#if 	OPT_PSCREEN
	void	(*t_pflush) P((void));	/* really flush */
#endif	/* OPT_PSCREEN */
#if	OPT_ICURSOR
	void	(*t_icursor) P((int));  /* set cursor shape for insertion */
#endif
}	TERM;

/*	TEMPORARY macros for terminal I/O  (to be placed in a machine
					    dependent place later)	*/

#define	TTopen		(*term.t_open)
#define	TTclose		(*term.t_close)
#define	TTkopen		(*term.t_kopen)
#define	TTkclose	(*term.t_kclose)
#define	TTgetc		(*term.t_getchar)
#define	TTputc		(*term.t_putchar)
#define	TTtypahead	(*term.t_typahead)
#define	TTflush		(*term.t_flush)
#define	TTmove		(*term.t_move)
#define	TTeeol		(*term.t_eeol)
#define	TTeeop		(*term.t_eeop)
#define	TTbeep		(*term.t_beep)
#define	TTrev		(*term.t_rev)
#define	TTrez		(*term.t_rez)
#if	OPT_COLOR
#define	TTforg		(*term.t_setfor)
#define	TTbacg		(*term.t_setback)
#endif
#define	TTscroll	(*term.t_scroll)
#if	OPT_PSCREEN
#define	TTpflush	(*term.t_pflush)
#endif
#if	OPT_ICURSOR
#define	TTicursor	(*term.t_icursor)
#endif

typedef struct  VIDEO {
        int	v_flag;                 /* Flags */
#if	OPT_COLOR
	int	v_fcolor;		/* current forground color */
	int	v_bcolor;		/* current background color */
	int	v_rfcolor;		/* requested forground color */
	int	v_rbcolor;		/* requested background color */
#endif
#if	OPT_VIDEO_ATTRS
	VIDEO_ATTR *v_attrs;		/* screen data attributes */
#endif
	/* allocate 4 bytes here, and malloc 4 bytes less than we need,
		to keep malloc from rounding up. */
        char    v_text[4];              /* Screen data. */
}       VIDEO;

#define VideoText(vp) (vp)->v_text
#define VideoAttr(vp) (vp)->v_attrs

#if OPT_COLOR
#define CurFcolor(vp) (vp)->v_fcolor
#define CurBcolor(vp) (vp)->v_bcolor
#define ReqFcolor(vp) (vp)->v_rfcolor
#define ReqBcolor(vp) (vp)->v_rbcolor
#else
#define CurFcolor(vp) gfcolor
#define CurBcolor(vp) gbcolor
#define ReqFcolor(vp) gfcolor
#define ReqBcolor(vp) gbcolor
#endif

#define VFCHG   0x0001                  /* Changed flag			*/
#define	VFEXT	0x0002			/* extended (beyond column 80)	*/
#define	VFREV	0x0004			/* reverse video status		*/
#define	VFREQ	0x0008			/* reverse video request	*/
#define	VFCOL	0x0010			/* color change requested	*/

#if DISP_IBMPC
/*
 * these need to go into edef.h eventually!
 */
#define	CDCGA	0			/* color graphics card		*/
#define	CDMONO	1			/* monochrome text card		*/
#define	CDEGA	2			/* EGA color adapter		*/
#define	CDVGA	3			/* VGA color adapter		*/
#define	CDSENSE	-1			/* detect the card type		*/

#if OPT_COLOR
#define	CD_25LINE	CDCGA
#else
#define	CD_25LINE	CDMONO
#endif

#endif


/* Commands are represented as CMDFUNC structures, which contain a
 *	pointer to the actual function, and flags which help to classify it.
 *	(things like is it a MOTION, can it be UNDOne)
 *
 *	These structures are generated automatically from the cmdtbl file,
 *	and can be found in the file nefunc.h
 */
#if ANSI_PROTOS
# define CMD_ARGS int f, int n
# define CMD_DECL
#else
# define CMD_ARGS f, n
# define CMD_DECL int f,n;
#endif

typedef	int	(*CmdFunc) P(( int, int ));

typedef	struct {
	CmdFunc  c_func;	/* function name is bound to */
	CMDFLAGS c_flags;	/* what sort of command is it? */
#if OPT_ONLINEHELP
	char	*c_help;	/* short help message for the command */
#endif
}	CMDFUNC;

/* when referencing a command by name (e.g ":e file") it is looked up in
 *	the nametbl, which is an array of NTAB structures, containing the
 *	name, and a pointer to the CMDFUNC structure.  There can be several
 *	entries pointing at a single CMDFUNC, since a command might have
 *	several synonymous names.
 *
 *	The nametbl array is generated automatically from the cmdtbl file,
 *	and can be found in the file nename.h
 */
typedef struct {
	char *n_name;
	CMDFUNC	*n_cmd;
}	NTAB;

/* when a command is referenced by bound key (like h,j,k,l, or "dd"), it
 *	is looked up one of two ways: single character 7-bit ascii commands (by
 *	far the majority) are simply indexed into an array of CMDFUNC pointers. 
 *	Other commands (those with ^A, ^X, or SPEC prefixes) are searched for
 *	in a binding table, made up of KBIND structures.  This structure
 *	contains the command code, and again, a pointer to the CMDFUNC
 *	structure for the command
 *
 *	The asciitbl array, and the kbindtbl array are generated automatically
 *	from the cmdtbl file, and can be found in the file nebind.h
 */
typedef struct  k_bind {
	short	k_code; 		/* Key code			*/
	CMDFUNC	*k_cmd;
#if OPT_REBIND
	struct  k_bind *k_link;
#endif
}	KBIND;


/* These are the flags which can appear in the CMDFUNC structure, describing a
 * command.
 */
#define NONE    0L
#define cmdBIT(n) lBIT(n)	/* ...to simplify typing */
#define UNDO    cmdBIT(0)	/* command is undo-able, so clean up undo lists */
#define REDO    cmdBIT(1)	/* command is redo-able, record it for dotcmd */
#define MOTION  cmdBIT(2)	/* command causes motion, okay after operator cmds */
#define FL      cmdBIT(3)	/* if command causes motion, opers act on full lines */
#define ABSM    cmdBIT(4)	/* command causes absolute (i.e. non-relative) motion */
#define GOAL    cmdBIT(5)	/* column goal should be retained */
#define GLOBOK  cmdBIT(6)	/* permitted after global command */
#define OPER    cmdBIT(7)	/* function is an operator, affects a region */
#define LISTED  cmdBIT(8)	/* internal use only -- used in describing
				 * bindings to only describe each once */
#define NOMOVE  cmdBIT(9)	/* dot doesn't move (although address may be used) */
#define VIEWOK  cmdBIT(10)	/* command is okay in view mode, even though it
				 * _may_ be undoable (macros and maps) */
#define RECT    cmdBIT(11)	/* motion causes rectangular operation */

/* These flags are 'ex' argument descriptors, adapted from elvis.  Not all are
 * used or honored or implemented.
 */
#define argBIT(n) cmdBIT(n+11)	/* ...to simplify adding bits */
#define FROM    argBIT(1)	/* allow a linespec */
#define TO      argBIT(2)	/* allow a second linespec */
#define BANG    argBIT(3)	/* allow a ! after the command name */
#define EXTRA   argBIT(4)	/* allow extra args after command name */
#define XFILE   argBIT(5)	/* expand wildcards in extra part */
#define NOSPC   argBIT(6)	/* no spaces allowed in the extra part */
#define DFLALL  argBIT(7)	/* default file range is 1,$ */
#define DFLNONE argBIT(9)	/* no default file range */
#define NODFL   argBIT(10)	/* do not default to the current file name */
#define EXRCOK  argBIT(11)	/* can be in a .exrc file */
#define NL      argBIT(12)	/* if !exmode, then write a newline first */
#define PLUS    argBIT(13)	/* allow a line number, as in ":e +32 foo" */
#define ZERO    argBIT(14)	/* allow 0 to be given as a line number */
#define OPTREG  argBIT(15)	/* allow optional register-name */
#define FILES   (XFILE | EXTRA)	/* multiple extra files allowed */
#define WORD1   (EXTRA | NOSPC)	/* one extra word allowed */
#define FILE1   (FILES | NOSPC)	/* 1 file allowed, defaults to current file */
#define NAMEDF  (FILE1 | NODFL)	/* 1 file allowed, defaults to "" */
#define NAMEDFS (FILES | NODFL)	/* multiple files allowed, default is "" */
#define RANGE   (FROM  | TO)	/* range of linespecs allowed */

#define SPECIAL_BANG_ARG -42	/* arg passed as 'n' to functions which
 					were invoked by their "xxx!" name */

/* definitions for 'mlreply_file()' and other filename-completion */
#define	FILEC_REREAD   4
#define	FILEC_READ     3
#define	FILEC_UNKNOWN  2
#define	FILEC_WRITE    1

#define	FILEC_PROMPT   8	/* always prompt (never from screen) */
#define	FILEC_EXPAND   16	/* allow glob-expansion to multiple files */

#ifndef P_tmpdir		/* not all systems define this */
#if SYS_MSDOS || SYS_OS2 || SYS_WINNT
#define P_tmpdir ""
#endif
#if SYS_UNIX
#define P_tmpdir "/usr/tmp"
#endif
#if SYS_VMS
#define P_tmpdir "sys$scratch:"
#endif
#endif	/* P_tmpdir */

#undef TMPDIR

#if OPT_EVAL
#define TMPDIR gtenv("directory")
#else
#define TMPDIR P_tmpdir		/* defined in <stdio.h> */
#endif	/* !SMALLER */

/*	The editor holds deleted text chunks in the KILL registers. The
	kill registers are logically a stream of ascii characters, however
	due to unpredictable size, are implemented as a linked
	list of chunks. (The d_ prefix is for "deleted" text, as k_
	was taken up by the keycode structure)
*/

typedef	struct KILL {
	struct KILL *d_next;	/* link to next chunk, NULL if last */
	UCHAR d_chunk[KBLOCK];	/* deleted text */
} KILL;

typedef struct KILLREG {
	struct KILL *kbufp;	/* current kill register chunk pointer */
	struct KILL *kbufh;	/* kill register header pointer	*/
	unsigned kused;		/* # of bytes used in kill last chunk	*/
	C_NUM kbwidth;		/* width of chunk, if rectangle */
	short kbflag;		/* flags describing kill register	*/
} KILLREG;

#define	KbSize(i,p)	((p->d_next != 0) ? KBLOCK : kbs[i].kused)

/*	The !WHILE directive in the execution language needs to
	stack references to pending whiles. These are stored linked
	to each currently open procedure via a linked list of
	the following structure
*/

typedef struct WHBLOCK {
	LINEPTR	w_begin;	/* ptr to !while statement */
	LINEPTR	w_end;		/* ptr to the !endwhile statement*/
	int w_type;		/* block type */
	struct WHBLOCK *w_next;	/* next while */
} WHBLOCK;

#define	BTWHILE		1
#define	BTBREAK		2

/*
 * Incremental search defines.
 */
#if	OPT_ISRCH

#define	CMDBUFLEN	256	/* Length of our command buffer */

#define IS_REVERSE	tocntrl('R')	/* Search backward */
#define	IS_FORWARD	tocntrl('F')	/* Search forward */

#endif

#ifndef NULL
# define NULL 0
#endif

/*
 * General purpose includes
 */

#if ANSI_VARARGS
# include <stdarg.h>
# ifdef lint
#  undef  va_arg
#  define va_arg(ptr,cast) (cast)(ptr-(char *)0)
# endif
#else
# include <varargs.h>
# ifdef lint
#  undef  va_dcl
#  define va_dcl char * va_alist;
#  undef  va_start
#  define va_start(list) list = (char *) &va_alist
#  undef  va_arg
#  define va_arg(ptr,cast) (cast)(ptr-(char *)0)
# else
#  ifndef va_dcl	 /* then try these out */
    typedef char *va_list;
#   define va_dcl int va_alist;
#   define va_start(list) list = (char *) &va_alist
#   define va_end(list)
#   define va_arg(list, mode) ((mode *)(list += sizeof(mode)))[-1]
#  endif
# endif
#endif /* ANSI_VARARGS */

#if DISP_X11 && SYS_APOLLO
#define SYSV_STRINGS	/* <strings.h> conflicts with <string.h> */
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#if HAVE_STDDEF_H
#include <stddef.h>
#endif

#if (HAVE_STDLIB_H || SYS_VMS || CC_NEWDOSCC)
#include <stdlib.h>
#else
# if ! OPT_VRFY_MALLOC
#if MISSING_EXTERN_MALLOC
extern	char *	malloc	P(( unsigned int ));
#endif
#if MISSING_EXTERN_REALLOC
extern	char *	realloc	P(( char *, unsigned int ));
#endif
# endif
extern void exit P((int));
extern void _exit P((int));
#endif	/* HAVE_STDLIB_H */

/* array/table size */
#define	TABLESIZE(v)	(sizeof(v)/sizeof(v[0]))

#ifndef	offsetof	/* <stddef.h> */
#define	offsetof(type, member)	((size_t)&(((type*)0)->member))
#endif

/* structure-allocate, for linting */
#ifdef	lint
#define	castalloc(cast,nbytes)		((cast *)0)
#define	castrealloc(cast,ptr,nbytes)	((ptr)+(nbytes))
#define	typecalloc(cast)		((cast *)0)
#define	typecallocn(cast,ntypes)	(((cast *)0)+(ntypes))
#define	typealloc(cast)			((cast *)0)
#define	typeallocn(cast,ntypes)		(((cast *)0)+(ntypes))
#define	typereallocn(cast,ptr,ntypes)	((ptr)+(ntypes))
#define	typeallocplus(cast,extra)	(((cast *)0)+(extra))
#else
#define	castalloc(cast,nbytes)		(cast *)malloc(nbytes)
#define	castrealloc(cast,ptr,nbytes)	(cast *)realloc((char *)(ptr),(nbytes))
#define	typecalloc(cast)		(cast *)calloc(sizeof(cast),1)
#define	typecallocn(cast,ntypes)	(cast *)calloc(sizeof(cast),ntypes)
#define	typealloc(cast)			(cast *)malloc(sizeof(cast))
#define	typeallocn(cast,ntypes)		(cast *)malloc((ntypes)*sizeof(cast))
#define	typereallocn(cast,ptr,ntypes)	(cast *)realloc((char *)(ptr),\
							(ntypes)*sizeof(cast))
#define	typeallocplus(cast,extra)	(cast *)malloc((extra)+sizeof(cast))
#endif

#define	FreeAndNull(p)	if ((p) != 0) { free((char *)p); p = 0; }
#define	FreeIfNeeded(p)	if ((p) != 0) free((char *)(p))

/* extra level for cleanup of temp-file */
#if OPT_MAP_MEMORY
#define	ExitProgram(code)	exit_program(code)
#else
#define	ExitProgram(code)	exit(code)
#endif

#if HAVE_SELECT
# if HAVE_SELECT_H
# include <select.h>
# endif
# if HAVE_SYS_SELECT_H
# include <sys/select.h>
# endif
#endif

#if HAVE_UTIME_H
# include <utime.h>
#endif

#if HAVE_SYS_UTIME_H
# include <sys/utime.h>
#endif

/*
 * Comparison-function for 'qsort()'
 */
#if __STDC__ || defined(CC_TURBO) || CC_WATCOM || defined(__CLCC__)
#if defined(apollo) && !(defined(__STDCPP__) || defined(__GNUC__))
#define	ANSI_QSORT 0	/* cc 6.7 */
#else
#define	ANSI_QSORT 1
#endif
#else
#define	ANSI_QSORT 0
#endif

/*
 * Local prototypes
 */

#include "neproto.h"
#include "proto.h" 

/*
 * the list of generic function key bindings
 */
#include "nefkeys.h" 

/*
 * Debugging/memory-leak testing
 */

#ifndef	DOALLOC		/* record info for 'show_alloc()' */
#define	DOALLOC		0
#endif
#ifndef	DBMALLOC	/* test malloc/free/strcpy/memcpy, etc. */
#define	DBMALLOC	0
#endif
#ifndef	NO_LEAKS	/* free permanent memory, analyze leaks */
#define	NO_LEAKS	0
#endif

#undef TRACE

#if	DBMALLOC
#undef strchr
#undef strrchr
#undef memcpy
#undef memccpy
#undef malloc
#undef realloc
#undef free
#include "dbmalloc.h"		/* renamed from dbmalloc's convention */
#define show_alloc() malloc_dump(fileno(stderr))
#define strmalloc strdup
#else
#if	NO_LEAKS || DOALLOC || OPT_TRACE
#include "trace.h"
#endif
#endif	/* DBMALLOC */

/* Normally defined in "trace.h" */
#ifndef TRACE
#define TRACE(p) /* nothing */
#endif

/*	Dynamic RAM tracking and reporting redefinitions	*/
#if	OPT_RAMSIZE
#undef	realloc
#define	realloc	reallocate
#undef	calloc
#define	calloc(n,m)	allocate((n)*(m))
#undef	malloc
#define	malloc	allocate
#undef	free
#define	free	release
#endif

#if OPT_VRFY_MALLOC
extern	char *vmalloc P(( SIZE_T, char *, int ));
extern	void vfree P(( char *, char *, int ));
extern	void rvverify P(( char *, char *, int ));
extern	char *vrealloc P(( char *, SIZE_T, char *, int ));
extern	char *vcalloc P(( int, SIZE_T, char *, int ));
extern	void vdump P(( char * ));
# define malloc(x) vmalloc(x,__FILE__,__LINE__)
# define free(x) vfree(x,__FILE__,__LINE__)
# define realloc(x,y) vrealloc(x,y,__FILE__,__LINE__)
# define calloc(x,y) vcalloc(x,y,__FILE__,__LINE__)
# define vverify(s) rvverify(s,__FILE__,__LINE__)
#else
# define vverify(s) ;
#endif

/* for debugging VMS pathnames on UNIX... */
#if SYS_UNIX && OPT_VMS_PATH
#include "fakevms.h"
#endif
