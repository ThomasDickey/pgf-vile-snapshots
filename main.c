/*
 *	This used to be MicroEMACS 3.9
 *			written by Dave G. Conroy.
 *			substantially modified by Daniel M. Lawrence
 *
 *	Turned into "VI Like Emacs", a.k.a. vile, by Paul Fox
 *
 *	(C)opyright 1987 by Daniel M. Lawrence
 *	MicroEMACS 3.9 can be copied and distributed freely for any
 *	non-commercial purposes. MicroEMACS 3.9 can only be incorporated
 *	into commercial software with the permission of the current author.
 *
 *	The same goes for vile.  -pgf
 *
 *
 * $Log: main.c,v $
 * Revision 1.105  1993/04/09 13:36:47  pgf
 * made the dummy functions more real, so that prototypes work
 *
 * Revision 1.104  1993/04/08  11:09:27  pgf
 * implemented horizscroll mode
 *
 * Revision 1.103  1993/04/01  14:43:16  pgf
 * typo -- missing semicolon
 *
 * Revision 1.102  1993/04/01  13:07:50  pgf
 * see tom's 3.40 CHANGES
 *
 * Revision 1.101  1993/04/01  12:05:46  pgf
 * add setjmp/longjmp to tgetc() and catchintr(), so that ^C gets
 * through on BSD-style signals
 *
 * Revision 1.100  1993/03/25  19:50:58  pgf
 * see 3.39 section of CHANGES
 *
 * Revision 1.99  1993/03/17  11:35:04  pgf
 * cleaned up confusion in alt-tabstop mode
 *
 * Revision 1.98  1993/03/16  10:53:21  pgf
 * see 3.36 section of CHANGES file
 *
 * Revision 1.97  1993/03/05  18:46:39  pgf
 * fix for tab cursor positioning in insert mode, and mode to control
 * positioning style
 *
 * Revision 1.96  1993/03/05  17:50:54  pgf
 * see CHANGES, 3.35 section
 *
 * Revision 1.95  1993/02/24  10:59:02  pgf
 * see 3.34 changes, in CHANGES file
 *
 * Revision 1.94  1993/02/15  10:37:31  pgf
 * cleanup for gcc-2.3's -Wall warnings
 *
 * Revision 1.93  1993/02/08  14:53:35  pgf
 * see CHANGES, 3.32 section
 *
 * Revision 1.92  1993/01/23  13:38:23  foxharp
 * writeall is now in file.c,
 * use new exit code macros
 *
 * Revision 1.91  1993/01/16  10:37:36  foxharp
 * macro-ization of special filenames, and re-ordering of bval initializations
 *
 * Revision 1.90  1992/12/23  09:20:50  foxharp
 * added .C, .i to cmode suffixes, for C++ and cpp output files
 *
 * Revision 1.89  1992/12/14  09:03:25  foxharp
 * lint cleanup, mostly malloc
 *
 * Revision 1.88  1992/12/04  09:24:12  foxharp
 * deleted unused assigns
 *
 * Revision 1.87  1992/12/02  09:13:16  foxharp
 * changes for "c-shiftwidth"
 *
 * Revision 1.86  1992/11/19  09:11:45  foxharp
 * eric krohn's changes for xvile "foreground", "background", and "name"
 * arguments, and
 * the "_qident" class of characters, useful for C++ "qualified" identifiers
 *
 * Revision 1.85  1992/11/19  08:48:48  foxharp
 * comment fixup
 *
 * Revision 1.84  1992/08/20  23:40:48  foxharp
 * typo fixes -- thanks, eric
 *
 * Revision 1.83  1992/08/19  22:59:05  foxharp
 * catch SIGINT and critical errors under DOS, glob command line args,
 * and clean up debug log ifdefs
 *
 * Revision 1.82  1992/08/18  22:39:59  foxharp
 * prevent double zotting of VILEINIT buffer by delaying setting the
 * BFSCRTCH flag (which probably isn't necessary anyway)
 *
 * Revision 1.81  1992/08/11  23:29:53  foxharp
 * fixed core from vileinit buf getting killed too early -- no longer
 * a scratch buffer
 *
 * Revision 1.80  1992/08/07  21:41:09  foxharp
 * cosmetic ifdef cleanup
 *
 * Revision 1.79  1992/07/15  08:56:28  foxharp
 * find our basename, so we can come up in view mode if named "view".
 *
 * Revision 1.78  1992/07/13  22:14:37  foxharp
 * initialize TERSE and TAGSRELATIVE modes
 *
 * Revision 1.77  1992/07/13  09:26:56  foxharp
 * use canonpath() on args before creating their buffers
 *
 * Revision 1.76  1992/07/04  14:37:49  foxharp
 * allow the cursor to rest on the 'newline', in the case where we're in
 * the middle of insert mode, and are only out here due to using arrow
 * keys.  otherwise, there's no way to append to end of line with arrow
 * keys -- you're blocked at the last character.
 *
 * Revision 1.75  1992/07/01  17:01:42  foxharp
 * make sure startstat is always set properly, and
 * commented the usage of the FF logfile
 *
 * Revision 1.74  1992/06/26  22:22:23  foxharp
 * moved reset of curbp (after makecurrent()) up higher -- vtinit may
 * call update().
 * some small fixes to the dos arg. expander.
 * took out all the freshmem stuff -- it wasn't doing anything.
 *
 * Revision 1.73  1992/06/25  23:00:50  foxharp
 * changes for dos/ibmpc
 *
 * Revision 1.72  1992/06/22  08:36:14  foxharp
 * bug in section r.e.
 *
 * Revision 1.71  1992/06/12  22:23:42  foxharp
 * changes for separate 'comments' r.e. for formatregion
 *
 * Revision 1.70  1992/06/08  08:56:05  foxharp
 * added version no. to usage message
 *
 * Revision 1.69  1992/06/04  19:44:29  foxharp
 * added -V for version info
 *
 * Revision 1.68  1992/06/01  20:38:12  foxharp
 * writeall() no longer calls pressreturn() if successful, and
 * added initialization for "tabinsert" mode
 *
 * Revision 1.67  1992/05/31  22:11:11  foxharp
 * paragraph regexp augmented to support reformatting of comments
 *
 * Revision 1.66  1992/05/19  08:55:44  foxharp
 * more prototype and shadowed decl fixups
 *
 * Revision 1.65  1992/05/16  12:00:31  pgf
 * prototypes/ansi/void-int stuff/microsoftC
 *
 * Revision 1.64  1992/05/13  09:12:20  pgf
 * force initial update in X11 -- not sure why -- seems to wait for event
 * otherwise
 *
 * Revision 1.63  1992/04/30  17:53:37  pgf
 * use new \s atom in paragraph and section regexps
 *
 * Revision 1.62  1992/04/14  08:42:42  pgf
 * don't handle SIGWINCH in X11 case
 *
 * Revision 1.61  1992/04/10  18:49:23  pgf
 * mark VILEINIT buf as scratch, so it goes away quickly
 *
 * Revision 1.60  1992/04/09  08:32:35  pgf
 * new vilerc processing was happening too late, after the first buffer
 * was already in.  this kept some modes from "sticking"
 *
 * Revision 1.59  1992/04/02  23:00:28  pgf
 * fixed empty buffer bug, just introduced
 *
 * Revision 1.58  1992/03/24  07:44:05  pgf
 * added support for VILEINIT variable for initialization
 *
 * Revision 1.57  1992/03/19  23:22:46  pgf
 * SIGT for signals, linux port
 *
 * Revision 1.56  1992/03/19  23:09:22  pgf
 * usage cleanup
 *
 * Revision 1.55  1992/03/07  10:36:29  pgf
 * fix missing goto-line argument problem.  "vile + file.c" now goes to end of
 * file, as it should
 *
 * Revision 1.54  1992/03/05  09:19:55  pgf
 * changed some mlwrite() to mlforce(), due to new terse support
 *
 * Revision 1.53  1992/03/03  21:59:02  pgf
 * added '`' to the _wild character set
 *
 * Revision 1.52  1992/03/03  09:35:52  pgf
 * added support for getting "words" out of the buffer via variables --
 * needed _nonspace character type
 *
 * Revision 1.51  1992/02/17  08:58:12  pgf
 * added "showmode" support, and kill registers now hold unsigned chars
 *
 * Revision 1.50  1992/01/14  20:24:54  pgf
 * don't ask for pressreturn() confirmation in quickexit() (ZZ command)
 *
 * Revision 1.49  1992/01/10  08:08:28  pgf
 * added initialization of shiftwidth
 *
 * Revision 1.48  1992/01/05  00:06:13  pgf
 * split mlwrite into mlwrite/mlprompt/mlforce to make errors visible more
 * often.  also normalized message appearance somewhat.
 *
 * Revision 1.47  1992/01/03  23:31:49  pgf
 * use new ch_fname() to manipulate filenames, since b_fname is now
 * a malloc'ed sting, to avoid length limits
 *
 * Revision 1.46  1991/11/27  10:09:09  pgf
 * slight rearrangement of init code, to prevent null filenames in makecurrent
 *
 * Revision 1.45  1991/11/16  18:38:58  pgf
 * changes for better magic mode -- the regexps for sections, paras etc. had
 * to change
 *
 * Revision 1.44  1991/11/13  20:09:27  pgf
 * X11 changes, from dave lemke
 *
 * Revision 1.43  1991/11/07  02:00:32  pgf
 * lint cleanup
 *
 * Revision 1.42  1991/11/06  23:26:27  pgf
 * recomp the search pattern if set from command line, and
 * added _fence to chartypes
 *
 * Revision 1.41  1991/11/01  14:38:00  pgf
 * saber cleanup
 *
 * Revision 1.40  1991/10/29  14:35:29  pgf
 * implemented the & commands: substagain
 *
 * Revision 1.39  1991/10/29  03:02:55  pgf
 * rename ctlx? routines to be ...kbd_macro... names, and allowed
 * for replaying of named registers
 *
 * Revision 1.38  1991/10/28  14:25:44  pgf
 * VAL_ASAVE --> VAL_ASAVECNT
 *
 * Revision 1.37  1991/10/27  01:45:10  pgf
 * set new regex values in global_val_init
 *
 * Revision 1.36  1991/10/24  13:05:52  pgf
 * conversion to new regex package -- much faster
 *
 * Revision 1.35  1991/10/23  14:20:53  pgf
 * changes to fix interactions between dotcmdmode and kbdmode and tungetc
 *
 * Revision 1.34  1991/10/22  03:08:45  pgf
 * call cmdlinetag() for command-line tags
 *
 * Revision 1.33  1991/10/20  23:07:56  pgf
 * pass taglength value to tags()
 *
 * Revision 1.32  1991/10/18  10:56:54  pgf
 * modified VALUE structures and lists to make them more easily settable
 *
 * Revision 1.31  1991/10/15  03:10:49  pgf
 * added backspacelimit and taglength
 *
 * Revision 1.30  1991/09/26  13:13:54  pgf
 * initialize window values, and
 * make sure file writing errors in writeall() are made visible
 *
 * Revision 1.29  1991/09/19  13:39:17  pgf
 * MDEXACT is now MDIGNCASE, and initialize new VAL_TAGS value to "tags"
 *
 * Revision 1.28  1991/09/17  13:02:57  pgf
 * added write-all and brethren
 *
 * Revision 1.27  1991/09/10  00:46:07	pgf
 * cleanup of the dotcmd stuff
 *
 * Revision 1.26  1991/08/13  02:50:13	pgf
 * initialize showmatch b_val
 *
 * Revision 1.25  1991/08/12  11:22:52	pgf
 * added 'vi +/searchstring file.c' invocation
 *
 * Revision 1.24  1991/08/12  10:23:43	pgf
 * esc() no longer kills keyboard recording
 *
 * Revision 1.23  1991/08/08  23:28:49	pgf
 * moved init of VAL_FILL to after vtinit, since it depends on term.t_ncol
 *
 * Revision 1.22  1991/08/08  13:19:58	pgf
 * removed ifdef BEFORE
 *
 * Revision 1.21  1991/08/07  12:35:07	pgf
 * added RCS log messages
 *
 * revision 1.20
 * date: 1991/08/06 15:55:24;
 * fixed dos mode on empty buffer
 * 
 * revision 1.19
 * date: 1991/08/06 15:23:42;
 *  global/local values
 * 
 * revision 1.18
 * date: 1991/07/23 11:12:19;
 * don't reset lastdot if absolute motion is on behalf of an operator
 * 
 * revision 1.17
 * date: 1991/07/19 17:16:03;
 * ignore SIG_QUIT unless DEBUG
 * 
 * revision 1.16
 * date: 1991/06/28 10:53:42;
 * undo last change -- was breaking some ops
 * 
 * revision 1.15
 * date: 1991/06/27 19:45:16;
 * moved back from eol and eob to execute()
 * 
 * revision 1.14
 * date: 1991/06/26 09:38:15;
 * removed an ifdef BEFORE
 * 
 * revision 1.13
 * date: 1991/06/25 19:52:59;
 * massive data structure restructure
 * 
 * revision 1.12
 * date: 1991/06/16 17:36:17;
 * support for local vs. global fillcol value
 * 
 * revision 1.11
 * date: 1991/06/07 22:00:18;
 * changed header
 * 
 * revision 1.10
 * date: 1991/06/07 13:22:23;
 * don't move "last dot" mark if ABS command doesn't change dot
 * 
 * revision 1.9
 * date: 1991/06/03 17:34:55;
 * switch from "meta" etc. to "ctla" etc.
 * 
 * revision 1.8
 * date: 1991/06/03 10:25:16;
 * command loop is now a separate routine, and
 * if (doingopcmd) stuff is now in operators()
 * 
 * revision 1.7
 * date: 1991/05/31 12:54:25;
 * put _wild chartypes back in -- dropped by mistake
 * 
 * revision 1.6
 * date: 1991/05/31 11:12:19;
 * changed args to execute(), and
 * added linespec character class, and
 * added unimplemented ex functions
 * 
 * revision 1.5
 * date: 1991/04/22 09:00:12;
 * added iswild type to chartypes
 * 
 * revision 1.4
 * date: 1991/04/04 09:37:48;
 * new arg. to unqname
 * 
 * revision 1.3
 * date: 1991/02/19 17:27:03;
 * set up the reverse pattern if -s is given
 * 
 * revision 1.2
 * date: 1990/10/03 16:00:57;
 * make backspace work for everyone
 * 
 * revision 1.1
 * date: 1990/09/21 10:25:35;
 * initial vile RCS revision
 */

/* Make global definitions not external */
#define realdef
#include	"estruct.h"	/* global structures and defines */
#include	"edef.h"	/* global definitions */

extern char *pathname[];	/* startup file path/name array */

/* for MSDOS, increase the default stack space */

#if	MSDOS & LATTICE
unsigned _stack = 32767;
#endif

#if	ATARI & LATTICE & 0
int _mneed = 256000;		/* reset memory pool size */
#endif

#if	MSDOS & AZTEC
int _STKSIZ = 32767/16; 	/* stack size in paragraphs */
int _STKRED = 1024;		/* stack checking limit */
int _HEAPSIZ = 4096/16; 	/* (in paragraphs) */
int _STKLOW = 0;		/* default is stack above heap (small only) */
#endif

#if	MSDOS & TURBO
unsigned _stklen = 32768;
#endif

int
main(argc, argv)
int	argc;
char	*argv[];
{
	int    c;			/* command character */
	register BUFFER *bp;		/* temp buffer pointer */
	register int	gotafile = FALSE;/* filename arg present? */
	register int	carg;		/* current arg to scan */
	register int	ranstartup = FALSE;/* startup executed flag */
	int startstat = TRUE;		/* result of running startup */
	BUFFER *firstbp = NULL; 	/* ptr to first buffer in cmd line */
	int gotoflag = FALSE;		/* do we need to goto line at start? */
	int gline = FALSE;		/* if so, what line? */
	int helpflag = FALSE;		/* do we need help at start? */
	int searchflag = FALSE; 	/* Do we need to search at start? */
	char bname[NBUFN];		/* buffer name of file to read */
	char *msg;
#if TAGS
	int didtag = FALSE;		/* look up a tag to start? */
	char *tname = NULL;
#endif
#if	CRYPT
	char ekey[NPAT];		/* startup encryption key */
#endif

#if MSDOS
	slash = '\\';  /* getswitchar() == '/' ? '\\' : '/'; */
	expand_wild_args(&argc, &argv);
#else
	slash = '/';
#endif

	prog_arg = argv[0];	/* this contains our only clue to exec-path */

	start_debug_log(argc,argv);

	charinit();		/* character types -- we need these pretty
					early  */
	global_val_init();	/* global buffer values */

	if (strcmp(pathleaf(prog_arg), "view") == 0)
		set_global_b_val(MDVIEW,TRUE);

#if IBMPC	/* pjr */
	ibmtype = CDSENSE;
#endif	/* IBMPC */

#if X11
	x_preparse_args(&argc, &argv);
#endif
	/* Parse the command line */
	for (carg = 1; carg < argc; ++carg) {
#if X11
		if (argv[carg][0] == '=') {
                	x_set_geometry(argv[carg]);
                        continue;
                }
#endif


		/* Process Switches */
		if (argv[carg][0] == '-') {
			switch (argv[carg][1]) {
#if	NeWS
			case 'l':	/* -l for screen lines */
			case 'L':
				term.t_nrow = atoi(&argv[carg][2]);
				break;
#endif
#if X11
			case 'd':
				if (argv[carg + 1])
					x_set_dpy(argv[++carg]);
				else
					goto usage;
				break;
			case 'r':
			case 'R':
				x_set_rv();
				break;
			case 'f':
			case 'F':
				if (strcmp(&argv[carg][1], "foreground") == 0)
					x_setforeground(argv[++carg]);
				else if (strcmp(&argv[carg][1], "fg") == 0)
					x_setforeground(argv[++carg]);
				else if (argv[carg + 1])
					x_setfont(argv[++carg]);
				else
					goto usage;

				break;
			case 'b':
				if (strcmp(&argv[carg][1], "background") == 0)
					x_setbackground(argv[++carg]);
				else if (strcmp(&argv[carg][1], "bg") == 0)
					x_setbackground(argv[++carg]);
				else
					goto usage;

				break;
			case 'n':
				if (strcmp(&argv[carg][1], "name") == 0)
					x_setname(argv[++carg]);
				else
					goto usage;

				break;
#endif
			case 'e':	/* -e for Edit file */
			case 'E':
				set_global_b_val(MDVIEW,FALSE);
				break;
			case 'g':	/* -g for initial goto */
			case 'G':
				gotoflag = TRUE;
				if (argv[carg][2]) {
					gline = atoi(&argv[carg][2]);
				} else {
					if (++carg < argc)
						gline = atoi(&argv[carg][0]);
					else
						goto usage;
				}
				break;
			case 'h':	/* -h for initial help */
			case 'H':
				helpflag = TRUE;
				break;
#if	CRYPT
			case 'k':	/* -k<key> for code key */
			case 'K':
				cryptflag = TRUE;
				if (argv[carg][2]) {
					strcpy(ekey, &argv[carg][2]);
				} else {
					if (++carg < argc)
						strcpy(ekey, &argv[carg][0]);
					else
						goto usage;
				}
				set_global_b_val(MDCRYPT,TRUE);
				crypt((char *)NULL, 0);
				crypt(ekey, strlen(ekey));
				break;
#endif
			case 's':  /* -s for initial search string */
			case 'S':
		dosearch:
				searchflag = TRUE;
				if (argv[carg][2]) {
					strncpy(pat,&argv[carg][2],NPAT);
				} else {
					if (++carg < argc)
						strncpy(pat,&argv[carg][0],NPAT);
					else
						goto usage;
				}
				gregexp = regcomp(pat, global_b_val(MDMAGIC));
				break;
#if TAGS
			case 't':  /* -t for initial tag lookup */
			case 'T':
				if (argv[carg][2]) {
					tname = &argv[carg][2];
				} else {
					if (++carg < argc)
						tname = &argv[carg][0];
					else
						goto usage;
				}
				break;
#endif
			case 'V':
#ifndef DOS
				printf("vile %s\n", version);
				exit(GOOD);
#endif
			case 'v':	/* -v for View File */
				set_global_b_val(MDVIEW,TRUE);
				break;
#if IBMPC
#if __ZTC__
			/*
			 * Note that ibmtype is now only used to detect
			 * whether a command line option was given, ie if
			 * it is not equal to CDSENSE then a command line
			 * option was given
			 */
			case '2':	/* 25 line mode */
				ibmtype = CDMONO;
				set43 = FALSE;
  				break;

			case '4':	/* 43 line mode */
				ibmtype = CDEGA;
				set43 = TRUE;
				break;

			case '5':	/* 50 line mode */
				ibmtype = CDVGA;
				set43 = TRUE;
				break;
#else
			case '2':	/* 25 line mode */
#if COLOR
				ibmtype = CDCGA;
#else
				ibmtype = CDMONO;
#endif
				break;
			case '4':	/* 43 line mode */
				ibmtype = CDEGA;
				break;
			case '5':	/* 50 line mode */
				ibmtype = CDVGA;
				break;

#endif	/* __ZTC__ */
#endif	/* IBMPC */

			case '?':
			default:	/* unknown switch */
			usage:
	fprintf(stderr, "usage: %s [-flags] [@cmdfile] files...\n",argv[0]);
	fprintf(stderr, "	-h to get help on startup\n");
	fprintf(stderr, "	-gNNN or simply +NNN to go to line NNN\n");
	fprintf(stderr, "	-sstring or +/string to search for \"string\"\n");
#if TAGS
	fprintf(stderr, "	-ttagname to look up a tag\n");
#endif
	fprintf(stderr, "	-v to view files as read-only\n");
	/* fprintf(stderr, "	-e to edit (as opposed to view) files\n"); */
#if CRYPT
	fprintf(stderr, "	-kcryptkey for encrypted files\n");
#endif
#if X11
	fprintf(stderr, "	-name name to change program name for X resources\n");
	fprintf(stderr, "	-fg color to change foreground color\n");
	fprintf(stderr, "	-bg color to change background color\n");
	fprintf(stderr, "	-f fontname to change font\n");
	fprintf(stderr, "	-d displayname to change the default display\n");
	fprintf(stderr, "	-r for reverse video\n");
#endif
#if ! DOS
	fprintf(stderr, "	-V for version info\n");
#endif
	fprintf(stderr, "	use @filename to run filename as commands\n");
	fprintf(stderr, "	 (this will suppress .vilerc)\n");
	fprintf(stderr, "	This is vile %s\n",version);
				exit(BAD(1));
			}

		} else if (argv[carg][0]== '+') { /* alternate form of -g */
			if (argv[carg][1] == '/')
				goto dosearch;
			gotoflag = TRUE;
			gline = atoi(&argv[carg][1]);
		} else if (argv[carg][0]== '@') {
			/* Process Startup macroes */
			if ((startstat = startup(&argv[carg][1])) == TRUE)
				ranstartup = TRUE; /* don't execute .vilerc */
		} else {

			/* Process an input file */

			/* set up a buffer for this file */
#if MSDOS
			(void)glob(argv[carg]);
#endif
			makename(bname, argv[carg]);
			unqname(bname,FALSE);

			bp = bfind(bname, OK_CREAT, BFARGS);
			ch_fname(bp, argv[carg]);
			make_current(bp); /* pull it to the front */
			if (!gotafile) {
				firstbp = bp;
				gotafile = TRUE;
			}

		}
	}

	/* we made some calls to make_current() above, to shuffle the
		list order.  this set curbp, which isn't actually kosher */
	curbp = NULL;

	/* initialize the editor */
#if UNIX
	signal(SIGINT,catchintr);
	signal(SIGHUP,imdying);
#ifdef SIGBUS
	signal(SIGBUS,imdying);
#endif
#ifdef SIGSYS
	signal(SIGSYS,imdying);
#endif
	signal(SIGSEGV,imdying);
	signal(SIGTERM,imdying);
#if DEBUG
	signal(SIGQUIT,imdying);
#else
	signal(SIGQUIT,SIG_IGN);
#endif
	signal(SIGPIPE,SIG_IGN);
#if defined(SIGWINCH) && ! X11
	signal(SIGWINCH,sizesignal);
#endif
#else
# if MSDOS
	signal(SIGINT,catchintr);
	_harderr(dos_crit_handler);
# endif
#endif
	vtinit();		/* Display */
	winit();		/* windows */
	varinit();		/* user variables */
        
	/* this comes out to 70 on an 80 (or greater) column display */
	{	register int fill;
		fill = (7 * term.t_ncol) / 8;  /* must be done after vtinit() */
		if (fill > 70) fill = 70;
		set_global_b_val(VAL_FILL, fill);
	}

	/* pull in an unnamed buffer, if we were given none to work with */
	if (!gotafile) {
		bp = bfind(ScratchName(unnamed), OK_CREAT, 0);
		bp->b_active = TRUE;
#if DOSFILES
		make_local_b_val(bp,MDDOS);
		set_b_val(bp, MDDOS, FALSE );
#endif
		swbuffer(bp);
	}

	/* if invoked with no other startup files,
	   run the system startup file here */
	if (!ranstartup) {
		char *vileinit = getenv("VILEINIT");
		if (vileinit != NULL) {
			int odiscmd;
			BUFFER *vbp, *obp;
			int oflags = 0;

			/* mark as modified, to prevent undispbuff() from
				 clobbering */
			obp = curbp;
			if (obp) {
				oflags = obp->b_flag;
				obp->b_flag |= BFCHG;
			}

			if ((vbp=bfind(ScratchName(vileinit), OK_CREAT, 0))==NULL)
				exit(BAD(1));
			/* mark the buffer as read only */
			make_local_b_val(vbp,MDVIEW);
			set_b_val(vbp,MDVIEW,TRUE);

			vbp->b_active = TRUE;

			swbuffer(vbp);
			bprintf("%s", vileinit);
			vbp->b_flag &= ~BFCHG;

			/* go execute it! */
			odiscmd = discmd;
			discmd = FALSE;
			startstat = dobuf(vbp);
			discmd = odiscmd;
			if (obp) {
				swbuffer(obp);
				obp->b_flag = oflags;
			}
			/* remove the now unneeded buffer */
			vbp->b_flag |= BFSCRTCH;  /* make sure it will go */
			zotbuf(vbp);
		} else {
			char *fname;
			/* if .vilerc is one of the input files....
					don't clobber it */
#if MSDOS
			/* search PATH for vilerc under dos */
	 		fname = flook(pathname[0], FL_ANYWHERE); /* pjr - find it! */
#else
			fname = pathname[0];
#endif
			if (gotafile && 
				strcmp(pathname[0], firstbp->b_bname) == 0) {
				c = firstbp->b_bname[0];
				firstbp->b_bname[0] = SCRTCH_LEFT[0];
				startstat = startup(fname);
				firstbp->b_bname[0] = c;
			} else {
				if (fname)
					startstat = startup(fname);
				else
					startstat = TRUE;
			}
		}
		/* ranstartup = TRUE; (unneeded) */
	}


	/* if there are any files to read, read the first one! */
	if (gotafile) {
		nextbuffer(FALSE,0);
	}
#if TAGS
	else if (tname) {
		cmdlinetag(tname);
		didtag = TRUE;
	}
#endif
	msg = "";
	if (helpflag) {
		if (help(TRUE,1) != TRUE) {
			msg = 
	"[Problem with help information. Type \":quit\" to exit if you wish]";
		}
	} else {
		msg = "[Use ^A-h, ^X-h, or :help to get help]";
	}

	/* Deal with startup gotos and searches */
	if (gotoflag + searchflag
#if TAGS
		 + (tname?1:0) 
#endif
		> 1) {
#if TAGS
		msg = "[Search, goto and tag are used one at a time]";
#else
		msg = "[Cannot search and goto at the same time]";
#endif
	} else if (gotoflag) {
		if (gotoline(gline != 0, gline) == FALSE)
			msg = "[Invalid goto argument]";
	} else if (searchflag) {
		forwhunt(FALSE, 0);
#if TAGS
	} else if (tname && !didtag) {
		cmdlinetag(tname);
#endif
	}

#if X11
	update(TRUE);
#else
	update(FALSE);
#endif
	if (startstat == TRUE)  /* else there's probably an error message */
		mlforce(msg);

	/* process commands */
	loop();

	/* NOTREACHED */
	return BAD(1);
}

#if MSDOS
/*
 * This is a replacement for the usual main routine.  It takes argc and
 * argv, and does wild card expansion on any arguments containing a '*' or
 * '?', and then calls main() with a new argc and argv.  Any errors result
 * in calling main() with the original arguments intact.  Arguments
 * containing wild cards that expand to 0 filenames are deleted.  Arguments
 * without wild cards are passed straight through.
 *
 * Arguments which are preceded by a " or ', are passed straight through. 
 * (cck)
 *
 * (taken from the winc app example of the zortech compiler - pjr)
 */

void
expand_wild_args(argcp, argvp)
int *argcp;
char ***argvp;
{
#ifdef FRESHMEM
	char          **freshmem = NULL;
	unsigned int    freshmemcount = 0;
	unsigned int    freshmemmax = 0;
#endif
	int oargc;
	char **oargv;

#ifdef __ZTC__
	struct FIND    *p;
#else
	struct find_t   p;
	int             j = 0;
#endif

	int             i, nargc, path_size, nargvmax;
	char          **nargv, path[FILENAME_MAX + 1], *cp, *end_path;

	oargc = *argcp;
	oargv = *argvp;

	nargc = 0;
	nargvmax = 2;		/* dimension of nargv[]		 */
	if ((nargv = typeallocn(char *, nargvmax)) == NULL) {
		mlforce("[OUT OF MEMORY]");
		return;
	}

	for (i = 0; i < oargc; ++i) {
		if (nargc + 2 >= nargvmax) {
			nargvmax = nargc + 2;
			if ((nargv = typereallocn( char *, nargv, nargvmax))
						== NULL) {
				mlforce("[OUT OF MEMORY]");
				return;
			}
		}
		cp = oargv[i];	/* cck */

		/* if have expandable names */
		if (!(cp[0] == '"' || cp[0] == '\'') && 
			(strchr(cp, '*') || strchr(cp, '?'))) {
			end_path = cp + strlen(cp);

			while (end_path >= cp && *end_path != '\\'
			       && *end_path != '/' && *end_path != ':')
				--end_path;

			path_size = 0;

			if (end_path >= cp) {	/* if got a path */
				path_size = end_path - cp + 1;
				memcpy(path, cp, path_size);
			}
			path[path_size] = 0;
#ifdef __ZTC__
			p = findfirst(cp, 0);
			while (p) {
				if ((cp = castalloc(char,
					path_size+strlen(p->name)+1)) == NULL)
#else
			j = _dos_findfirst(cp, 0, &p);
			while (!j) {
				if ((cp = castalloc(char,
					path_size+strlen(p.name)+1)) == NULL)
#endif
				{
					mlforce("[OUT OF MEMORY]");
					return;
				}
				strcpy(cp, path);

#ifdef __ZTC__
				strcat(cp, p->name);
#else
				strcat(cp, p.name);
#endif

				if (nargc + 2 >= nargvmax) {
					nargvmax = nargc + 2;
					if ((nargv = (char **) realloc(
						(char *)nargv,
						nargvmax * sizeof(char *))
						) == NULL) {
						mlforce("[OUT OF MEMORY]");
						return;
					}
				}
				nargv[nargc++] = cp;

#ifdef FRESHMEM
				if (freshmemcount >= freshmemmax) {
					freshmemmax += 4;

					if ((freshmem = (char **) realloc(
						freshmem, 
						freshmemmax * sizeof(char *))
							) == NULL) {
						mlforce("[OUT OF MEMORY]");
						return;
					}
				}
				freshmem[freshmemcount++] = cp;
#endif
#ifdef __ZTC__
				p = findnext();
			}
#else
				j = _dos_findnext(&p);
			}
#endif
		} else {
			nargv[nargc++] = oargv[i];
		}
	}

	nargv[nargc] = NULL;
	*argcp = nargc;
	*argvp = nargv;
}
#endif

void do_num_proc();
void do_rept_arg_proc();

/* this is nothing but the main command loop */
void
loop()
{
	int s,c,f,n;
	while(1) {
		extern int insert_mode_was;

		/* vi doesn't let the cursor rest on the newline itself.  This
			takes care of that. */
		/* if we're inserting, or will be inserting again, then
			suppress.  this happens if we're using arrow keys
			during insert */
		if (is_at_end_of_line(DOT) && !is_empty_line(DOT) &&
				!insertmode && !insert_mode_was)
			backchar(TRUE,1);

		/* same goes for end-of-file -- I'm actually not sure if
			this can ever happen, but I _am_ sure that it's
			a lot safer not to let it... */
		if (is_header_line(DOT,curbp) && !is_empty_buf(curbp))
			backline(TRUE,1);

		/* start recording for '.' command */
		dotcmdbegin();

		/* Fix up the screen	*/
		s = update(FALSE);

		/* get the next command from the keyboard */
		c = kbd_seq();

		/* if there is something on the command line, clear it */
		if (mpresf != FALSE) {
			mlerase();
			if (s != SORTOFTRUE) /* did nothing due to typeahead */
				update(FALSE);
		}

		f = FALSE;
		n = 1;

		do_num_proc(&c,&f,&n);
		do_rept_arg_proc(&c,&f,&n);

		kregflag = 0;
	        
		/* flag the first time through for some commands -- e.g. subst
			must know to not prompt for strings again, and pregion
			must only restart the p-lines buffer once for each
			command. */
		calledbefore = FALSE;

		/* and execute the command */
		execute(kcod2fnc(c), f, n);
	        
		/* stop recording for '.' command */
		dotcmdfinish();
	}
}

char *
strmalloc(s)
char *s;
{
	register char *ns = castalloc(char,strlen(s)+1);
	if (ns)
		return strcpy(ns,s);
	else
		return NULL;

}


void
global_val_init()
{
	register int i;
	/* set up so the global value pointers point at the global
		values.  we never actually use the global pointers
		directly, but when buffers get a copy of the
		global_b_values structure, the pointers will still point
		back at the global values, which is what we want */
	for (i = 0; i <= MAX_G_VALUES; i++)
		copy_val(global_g_values.gv, global_g_values.gv, i);

	for (i = 0; i <= MAX_B_VALUES; i++)
		copy_val(global_b_values.bv, global_b_values.bv, i);

	for (i = 0; i <= MAX_W_VALUES; i++)
		copy_val(global_w_values.wv, global_w_values.wv, i);


	set_global_g_val(GMDABUFF,TRUE); 	/* auto-buffer */
	set_global_g_val(GMDALTTABPOS,FALSE); 	/* emacs-style tab positioning */
	set_global_g_val(GMDDIRC,FALSE); 	/* directory-completion */
#ifdef GMDTAGSLOOK
	set_global_g_val(GMDTAGSLOOK,FALSE); 	/* tags-look */
#endif
	set_global_g_val(GMDIMPLYBUFF,FALSE); 	/* imply-buffer */

	set_global_b_val(MDAIND,FALSE); 	/* auto-indent */
	set_global_b_val(MDASAVE,FALSE);	/* auto-save */
	set_global_b_val(MDBACKLIMIT,TRUE); 	/* limit backspacing to insert point */
	set_global_b_val(MDCMOD,FALSE); 	/* C mode */
	set_global_b_val(MDCRYPT,FALSE);	/* crypt */
	set_global_b_val(MDIGNCASE,FALSE); 	/* exact matches */
	set_global_b_val(MDDOS,FALSE);		/* dos mode */
	set_global_b_val(MDMAGIC,TRUE); 	/* magic searches */
	set_global_b_val(MDSHOWMAT,FALSE);	/* show-match */
	set_global_b_val(MDSHOWMODE,TRUE);	/* show-mode */
	set_global_b_val(MDSWRAP,TRUE); 	/* scan wrap */
	set_global_b_val(MDTABINSERT,TRUE);	/* allow tab insertion */
	set_global_b_val(MDTAGSRELTIV,FALSE);	/* path relative tag lookups */
	set_global_b_val(MDTERSE,FALSE);	/* terse messaging */
	set_global_b_val(MDVIEW,FALSE); 	/* view-only */
	set_global_b_val(MDWRAP,FALSE); 	/* wrap */

	set_global_b_val(VAL_ASAVECNT, 256);	/* autosave count */
	set_global_b_val(VAL_C_SWIDTH, 8); 	/* C file shiftwidth */
	set_global_b_val(VAL_C_TAB, 8); 	/* C file tab stop */
	set_global_b_val(VAL_SWIDTH, 8); 	/* shiftwidth */
	set_global_b_val(VAL_TAB, 8);		/* tab stop */
	set_global_b_val(VAL_TAGLEN, 0);	/* significant tag length */

	set_global_b_val_ptr(VAL_TAGS, strmalloc("tags")); /* tags filename */

	/* suffixes for C mode */
	set_global_g_val_rexp(GVAL_CSUFFIXES,
		new_regexval(
			"\\.[Cchis]$",
			TRUE));

	/* where do paragraphs start? */
	set_global_b_val_rexp(VAL_PARAGRAPHS,
		new_regexval(
			"^\\.[ILPQ]P\\s\\|^\\.P\\s\\|^\\.LI\\s\\|\
^\\.[plinb]p\\s\\|^\\.\\?\\s$",
			TRUE));

	/* where do comments start and end, for formatting them */
	set_global_b_val_rexp(VAL_COMMENTS,
		new_regexval(
			"^\\s/\\?[#*>]\\+/\\?\\s$",
			TRUE));

	/* where do sections start? */
	set_global_b_val_rexp(VAL_SECTIONS,
		new_regexval(
			"^[{\014]\\|^\\.[NS]H\\s\\|^\\.HU\\?\\s\\|\
^\\.[us]h\\s\\|^+c\\s",
			TRUE));

	/* where do sentences start? */
	set_global_b_val_rexp(VAL_SENTENCES,
		new_regexval(
	"[.!?][])\"']* \\?$\\|[.!?][])\"']*  \\|^\\.[ILPQ]P\\s\\|\
^\\.P\\s\\|^\\.LI\\s\\|^\\.[plinb]p\\s\\|^\\.\\?\\s$",
			TRUE));

	set_global_w_val(WMDLIST,FALSE); /* list-mode */
	set_global_w_val(WVAL_SIDEWAYS,0); /* list-mode */
	set_global_w_val(WVAL_FCOLOR,7); /* foreground color */
	set_global_w_val(WVAL_BCOLOR,0); /* background color */
	set_global_w_val(WMDNUMBER,FALSE);	/* number */
	set_global_w_val(WMDHORSCROLL,TRUE);	/* horizontal scrolling */


}

#if UNIX || MSDOS
/* ARGSUSED */
ACTUAL_SIGNAL(catchintr)
{
	interrupted = TRUE;
#if USG || MSDOS
	signal(SIGINT,catchintr);
#endif
	if (doing_kbd_read)
		longjmp(read_jmp_buf, signo);
	SIGRET;
}
#endif

#if MSDOS
void
dos_crit_handler()
{
	_hardresume(_HARDERR_FAIL);
}
#endif

/* do number processing if needed */
void
do_num_proc(cp,fp,np)
int *cp, *fp, *np;
{
	register int c, f, n;
	register int	mflag;

	c = *cp;
	f = *fp;
	n = *np;

	if (iscntrl(c) || (c & (CTLA|CTLX|SPEC)))
		return;
	if ( isdigit(c) && c != '0' ) {
		f = TRUE;		/* there is a # arg */
		n = 0;			/* start with a zero default */
		mflag = 1;		/* current minus flag */
		while (isdigit(c) || (c == '-')) {
			if (c == '-') {
				/* already hit a minus or digit? */
				if ((mflag == -1) || (n != 0))
					break;
				mflag = -1;
			} else {
				n = n * 10 + (c - '0');
			}
			if ((n == 0) && (mflag == -1))	/* lonely - */
				mlwrite("arg:");
			else
				mlwrite("arg: %d",n * mflag);

			c = kbd_seq();	/* get the next key */
		}
		n = n * mflag;	/* figure in the sign */
	}
	*cp = c;
	*fp = f;
	*np = n;
}

/* do ^U-style repeat argument processing -- vile binds this to 'K' */
void
do_rept_arg_proc(cp,fp,np)
int *cp, *fp, *np;
{
	register int c, f, n;
	register int	mflag;
	c = *cp;

	if (c != reptc) 
		return;

	f = TRUE;
	n = 4;				/* with argument of 4 */
	mflag = 0;			/* that can be discarded. */
	mlwrite("arg: 4");
	while (isdigit(c=kbd_seq()) || c==reptc || c=='-'){
		if (c == reptc)
			if ((n > 0) == ((n*4) > 0))
				n = n*4;
			else
				n = 1;
		/*
		 * If dash, and start of argument string, set arg.
		 * to -1.  Otherwise, insert it.
		 */
		else if (c == '-') {
			if (mflag)
				break;
			n = 0;
			mflag = -1;
		}
		/*
		 * If first digit entered, replace previous argument
		 * with digit and set sign.  Otherwise, append to arg.
		 */
		else {
			if (!mflag) {
				n = 0;
				mflag = 1;
			}
			n = 10*n + c - '0';
		}
		mlwrite("arg: %d", (mflag >=0) ? n : (n ? -n : -1));
	}
	/*
	 * Make arguments preceded by a minus sign negative and change
	 * the special argument "^U -" to an effective "^U -1".
	 */
	if (mflag == -1) {
		if (n == 0)
			n++;
		n = -n;
	}

	*cp = c;
	*fp = f;
	*np = n;
}

/* the vi ZZ command -- write all, then quit */
int
zzquit(f,n)
int f,n;
{
	int thiscmd;
	int cnt;

	thiscmd = lastcmd;
	cnt = anycb();
	if (cnt) {
		mlprompt("Will write %d buffer%c  %s ",
			cnt, cnt > 1 ? 's':'.',
			clexec ? "" : "Repeat command to continue.");
		if (!clexec && !isnamedcmd) {
			if (thiscmd != kbd_seq())
				return FALSE;
		}

		if (writeall(TRUE,1) != TRUE)
			return FALSE;

	} else if (!clexec && !isnamedcmd) {
		/* consume the next char. anyway */
		if (thiscmd != kbd_seq())
			return FALSE;
	}
	quithard(f, n);
	return TRUE;
}

/*
 * Fancy quit command, as implemented by Norm. If the any buffer has
 * changed do a write on that buffer and exit, otherwise simply exit.
 */
int
quickexit(f, n)
int f,n;
{
	if (writeall(TRUE,1) == TRUE)
		quithard(f, n); 	/* conditionally quit	*/
	return TRUE;
}

/* Force quit by giving argument */
/* ARGSUSED */
int
quithard(f,n)
int f,n;
{
    return quit(TRUE,1);
}

/*
 * Quit command. If an argument, always quit. Otherwise confirm if a buffer
 * has been changed and not written out.
 */
/* ARGSUSED */
int
quit(f, n)
int f,n;
{
	int cnt;
        
	if (f == FALSE && (cnt = anycb()) != 0) {
		if (cnt == 1)
			mlforce(
			"There is an unwritten modified buffer.  Write it, or use :q!");
		else
			mlforce(
			"There are %d unwritten modified buffers.  Write them, or use :q!",
				cnt);
		return FALSE;
	}
	vttidy(TRUE);
#if	FILOCK
	if (lockrel() != TRUE) {
		exit(BAD(1));
		/* NOTREACHED */
	}
#endif
#if UNIX
	signal(SIGHUP,SIG_DFL);	/* I don't care anymore */
#endif
#if NO_LEAKS
	{
		register int i;
		register struct VALNAMES *v;

		/* free all of the global data structures */
		kbs_leaks();
		tb_leaks();
		wp_leaks();
		bp_leaks();
		vt_leaks();
		ev_leaks();

		for (i = 0, v=g_valuenames; v[i].name != 0; i++)
			free_val(v+i, &global_g_values.gv[i]);

		for (i = 0, v=b_valuenames; v[i].name != 0; i++)
			free_val(v+i, &global_b_values.bv[i]);

		for (i = 0, v=w_valuenames; v[i].name != 0; i++)
			free_val(v+i, &global_w_values.wv[i]);

		if (gregexp != 0)	free((char *)gregexp);
		if (patmatch != 0)	free(patmatch);

		/* whatever is left over must be a leak */
		show_alloc();
	}
#endif
	exit(GOOD);
	/* NOTREACHED */
	return FALSE;
}

/* ARGSUSED */
int
writequit(f,n)
int f,n;
{
	int s;
	s = filesave(FALSE,n);
	if (s != TRUE)
		return s;
	return quit(FALSE,n);
}

/*
 * Abort.
 * Beep the beeper. Kill off any keyboard macro, etc., that is in progress.
 * Sometimes called as a routine, to do general aborting of stuff.
 */
/* ARGSUSED */
int
esc(f, n)
int f,n;
{
	TTbeep();
	dotcmdmode = STOP;
	fulllineregions = FALSE;
	doingopcmd = FALSE;
	opcmd = 0;
	mlforce("[Aborted]");
	return ABORT;
}

/* tell the user that this command is illegal while we are in
   VIEW (read-only) mode				*/

int
rdonly()
{
	TTbeep();
	mlforce("[No changes are allowed while in \"view\" mode]");
	return FALSE;
}

/* ARGSUSED */
int
showversion(f,n)
int f,n;
{
	mlforce(version);
	return TRUE;
}

/* ARGSUSED */
int
unimpl(f,n)
int f,n;
{
	TTbeep();
	mlforce("[Sorry, that vi command is unimplemented in vile ]");
	return FALSE;
}

int opercopy(f,n) int f,n; { return unimpl(f,n); }
int opermove(f,n) int f,n; { return unimpl(f,n); }
int opertransf(f,n) int f,n; { return unimpl(f,n); }

int operglobals(f,n) int f,n; { return unimpl(f,n); }
int opervglobals(f,n) int f,n; { return unimpl(f,n); }

int map(f,n) int f,n; { return unimpl(f,n); }
int unmap(f,n) int f,n; { return unimpl(f,n); }

int source(f,n) int f,n; { return unimpl(f,n); }

int visual(f,n) int f,n; { return unimpl(f,n); }
int ex(f,n) int f,n; { return unimpl(f,n); }

/* ARGSUSED */
int
nullproc(f,n)	/* user function that does (almost) NOTHING */
int f,n;
{
	return TRUE;
}

/* ARGSUSED */
int
cntl_af(f,n)	/* dummy function for binding to control-a prefix */
int f,n;
{
	return TRUE;
}

/* ARGSUSED */
int
cntl_xf(f,n)	/* dummy function for binding to control-x prefix */
int f,n;
{
	return TRUE;
}

/* ARGSUSED */
int
unarg(f,n) /* dummy function for binding to universal-argument */
int f,n;
{
	return TRUE;
}

/* initialize our version of the "chartypes" stuff normally in ctypes.h */
void
charinit()
{
	register int c;

	/* legal in pathnames */
	_chartypes_['.'] = 
		_chartypes_['_'] = 
		_chartypes_['-'] =
		_chartypes_['*'] = 
		_chartypes_['/'] = _pathn;

	/* legal in "identifiers" */
	_chartypes_['_'] |= _ident|_qident;
	_chartypes_[':'] |= _qident;

	/* whitespace */
	_chartypes_[' '] =
		_chartypes_['\t'] = 
		_chartypes_['\r'] =
		_chartypes_['\n'] = 
		_chartypes_['\f'] = _space;

	/* control characters */
	for (c = 0; c < ' '; c++)
		_chartypes_[c] |= _cntrl;
	_chartypes_[127] |= _cntrl;

	/* lowercase */
	for (c = 'a'; c <= 'z'; c++)
		_chartypes_[c] |= _lower|_pathn|_ident|_qident;

	/* uppercase */
	for (c = 'A'; c <= 'Z'; c++)
		_chartypes_[c] |= _upper|_pathn|_ident|_qident;

	/* digits */
	for (c = '0'; c <= '9'; c++)
		_chartypes_[c] |= _digit|_pathn|_ident|_qident|_linespec;

	/* punctuation */
	for (c = '!'; c <= '/'; c++)
		_chartypes_[c] |= _punct;
	for (c = ':'; c <= '@'; c++)
		_chartypes_[c] |= _punct;
	for (c = '['; c <= '`'; c++)
		_chartypes_[c] |= _punct;
	for (c = '{'; c <= '~'; c++)
		_chartypes_[c] |= _punct;

	/* printable */
	for (c = ' '; c <= '~'; c++)
		_chartypes_[c] |= _print;

	/* backspacers: ^H, rubout, and the user's backspace char */
	/* we'll add the user's char later */
	_chartypes_['\b'] |= _bspace;
	_chartypes_[127] |= _bspace;

	/* wildcard chars for most shells */
	_chartypes_['*'] |= _wild;
	_chartypes_['?'] |= _wild;
#if !VMS
	_chartypes_['~'] |= _wild;
	_chartypes_['['] |= _wild;
	_chartypes_[']'] |= _wild;
	_chartypes_['$'] |= _wild;
	_chartypes_['{'] |= _wild;
	_chartypes_['}'] |= _wild;
	_chartypes_['`'] |= _wild;
#endif

	/* ex mode line specifiers */
	_chartypes_[','] |= _linespec;
	_chartypes_['%'] |= _linespec;
	_chartypes_['-'] |= _linespec;
	_chartypes_['+'] |= _linespec;
	_chartypes_['.'] |= _linespec;
	_chartypes_['$'] |= _linespec;
	_chartypes_['\''] |= _linespec;

	/* fences */
	_chartypes_['('] |= _fence;
	_chartypes_[')'] |= _fence;
	_chartypes_['['] |= _fence;
	_chartypes_[']'] |= _fence;
	_chartypes_['{'] |= _fence;
	_chartypes_['}'] |= _fence;

#if VMS
	_chartypes_['['] |= _pathn;	/* actually, "<", ">" too */
	_chartypes_[']'] |= _pathn;
	_chartypes_['$'] |= _pathn;
	_chartypes_[':'] |= _pathn;
	_chartypes_[';'] |= _pathn;
#endif

#if !SMALLER
	/* scratch-buffer-names (usually superset of _pathn) */
	_chartypes_[(unsigned)SCRTCH_LEFT[0]]  |= _scrtch;
	_chartypes_[(unsigned)SCRTCH_RIGHT[0]] |= _scrtch;
	_chartypes_[' '] |= _scrtch;	/* ...to handle "[Buffer List]" */
#endif

	for (c = 0; c < N_chars; c++) {
#if !SMALLER
		if (isspace(c) || isprint(c))
			_chartypes_[c] |= _shpipe;
		if (ispath(c))
			_chartypes_[c] |= _scrtch;
#endif
		if ((_chartypes_[c] & _space) == 0)
			_chartypes_[c] |= _nonspace;
	}
}


/*****		Compiler specific Library functions	****/

#if	MWC86 & MSDOS
movmem(source, dest, size)
char *source;	/* mem location to move memory from */
char *dest;	/* memory location to move text to */
int size;	/* number of bytes to move */
{
	register int i;

	for (i=0; i < size; i++)
		*dest++ = *source++;
}
#endif

#if	(AZTEC || LATTICE || ZTC) && MSDOS
/*	strncpy:	copy a string...with length restrictions
			ALWAYS null terminate
Hmmmm...
I don't know much about DOS, but I do know that strncpy shouldn't ALWAYS
	null terminate.  -pgf
*/

char *strncpy(dst, src, maxlen)
char *dst;	/* destination of copied string */
char *src;	/* source */
int maxlen;	/* maximum length */
{
	char *dptr;	/* ptr into dst */

	dptr = dst;
	while (*src && (maxlen-- > 0))
		*dptr++ = *src++;
	*dptr = 0;
	return dst;
}
#endif

#if	RAMSIZE & LATTICE & MSDOS
/*	These routines will allow me to track memory usage by placing
	a layer on top of the standard system malloc() and free() calls.
	with this code defined, the environment variable, $RAM, will
	report on the number of bytes allocated via malloc.

	with SHOWRAM defined, the number is also posted on the
	end of the bottom mode line and is updated whenever it is changed.
*/

#undef	malloc
#undef	free

char *allocate(nbytes)	/* allocate nbytes and track */
unsigned nbytes;	/* # of bytes to allocate */
{
	char *mp;	/* ptr returned from malloc */

	mp = malloc(nbytes);
	if (mp) {
		envram += nbytes;
#if	RAMSHOW
		dspram();
#endif
	}

	return mp;
}

release(mp)	/* release malloced memory and track */
char *mp;	/* chunk of RAM to release */
{
	unsigned *lp;	/* ptr to the long containing the block size */

	if (mp) {
		lp = ((unsigned *)mp) - 1;

		/* update amount of ram currently malloced */
		envram -= (long)*lp - 2;
		free(mp);
#if	RAMSHOW
		dspram();
#endif
	}
}

#if	RAMSHOW
dspram()	/* display the amount of RAM currently malloced */
{
	char mbuf[20];
	char *sp;

	TTmove(term.t_nrow - 1, 70);
#if	COLOR
	TTforg(7);
	TTbacg(0);
#endif
	lsprintf(mbuf, "[%ld]", envram);
	sp = &mbuf[0];
	while (*sp)
		TTputc(*sp++);
	TTmove(term.t_nrow, 0);
	movecursor(term.t_nrow, 0);
}
#endif
#endif

#if MALLOCDEBUG
mallocdbg(f,n)
{
	int lvl;
	lvl = malloc_debug(n);
	mlwrite("malloc debug level was %d",lvl);
	if (!f) {
		malloc_debug(lvl);
	} else if (n > 2) {
		malloc_verify();
	}
	return TRUE;
}
#endif


/*
 *	the log file is left open, unbuffered.  thus any code can do 

 	extern FILE *FF;
	fprintf(FF, "...", ...);
	
 *	to log events without disturbing the screen
 */

#ifdef DEBUGLOG
/* suppress the declaration so that the link will fail if someone uses it */
FILE *FF;
#endif

void
start_debug_log(ac,av)	/* ARGSUSED */
int ac;
char **av;
{
#ifdef DEBUGLOG
	int i;
	FF = fopen("vilelog", "w");
	setbuf(FF,NULL);
	for (i = 0; i < ac; i++)
		fprintf(FF,"arg %d: %s\n",i,av[i]);
#endif
}
