/*	ESTRUCT:	Structure and preprocesser defines for
			vile.  Reshaped from the original, which
			was for MicroEMACS 3.9

			vile is by Paul Fox
			MicroEmacs was written by Dave G. Conroy
			modified by Steve Wilhite, George Jones
			substantially modified by Daniel Lawrence
*/

/*
 * $Log: estruct.h,v $
 * Revision 1.34  1991/10/18 10:56:54  pgf
 * modified VALUE structures and lists to make them more easily settable
 *
 * Revision 1.33  1991/10/15  03:10:00  pgf
 * added backspacelimit and taglength
 *
 * Revision 1.32  1991/10/10  12:33:33  pgf
 * changes to support "block malloc" of line text -- now for most files
 * there is are two mallocs and a single read, no copies.  previously there
 * were two mallocs per line, and two copies (stdio's and ours).  This change
 * implies that lines and line text should not move between buffers, without
 * checking that the text and line struct do not "belong" to the buffer.
 *
 * Revision 1.31  1991/09/26  13:08:55  pgf
 * created window values, moved list mode there
 *
 * Revision 1.30  1991/09/19  13:35:29  pgf
 * MDEXACT is now MDIGNCASE, and names are now more vi-compliant
 *
 * Revision 1.29  1991/09/10  12:29:57  pgf
 * added b_wline macro into b_wtraits
 *
 * Revision 1.28  1991/09/10  01:08:28  pgf
 * added BEL define
 *
 * Revision 1.27  1991/09/10  00:44:29  pgf
 * added ESC defines
 *
 * Revision 1.26  1991/08/16  10:58:54  pgf
 * added the third flavor of insertmode
 *
 * Revision 1.25  1991/08/13  02:48:59  pgf
 * added select and poll selectors, and alphabetized the VAL_XXX's
 *
 * Revision 1.24  1991/08/12  15:06:21  pgf
 * added ANSI_SPEC capability -- can now use the arrow keys from
 * command or insert mode
 *
 * Revision 1.23  1991/08/12  09:25:10  pgf
 * now store w_line in w_traits while buffer is offscreen, so reframe
 * isn't always necessary.  don't force reframe on redisplay.
 *
 * Revision 1.22  1991/08/07  11:51:32  pgf
 * added RCS log entries
 *
 * revision 1.21
 * date: 1991/08/06 15:07:43;
 * global/local values
 * ----------------------------
 * revision 1.20
 * date: 1991/06/28 10:52:53;
 * added config for ISC, and changed some "#if" to "#ifdef"
 * ----------------------------
 * revision 1.19
 * date: 1991/06/25 19:51:43;
 * massive data structure restructure
 * ----------------------------
 * revision 1.18
 * date: 1991/06/16 17:30:21;
 * fixed tabs to be modulo intead of mask, added ctabstop capability, added
 * LOCAL_VALUES #define to control local buffer values
 * ----------------------------
 * revision 1.17
 * date: 1991/06/06 13:57:52;
 * added auto-indent mode
 * ----------------------------
 * revision 1.16
 * date: 1991/06/04 09:20:53;
 * kcod2key is now a macro
 * ----------------------------
 * revision 1.15
 * date: 1991/06/03 17:34:35;
 * switch from "meta" etc. to "ctla" etc.
 * ----------------------------
 * revision 1.14
 * date: 1991/06/03 13:58:40;
 * made bind description list better
 * ----------------------------
 * revision 1.13
 * date: 1991/06/03 10:16:34;
 * cleanup, for release of 2.3
 * ----------------------------
 * revision 1.12
 * date: 1991/05/31 10:46:27;
 * added lspec character class for ex line specifiers
 * added end pointer and offset to the region struct
 * added #defines for the ex range allowances
 * ----------------------------
 * revision 1.11
 * date: 1991/04/22 09:00:46;
 * added ODT, POSIX defines.
 * also added iswild() support
 * ----------------------------
 * revision 1.10
 * date: 1991/04/05 13:04:55;
 * fixed "shorten" directory name
 * ----------------------------
 * revision 1.9
 * date: 1991/04/04 09:28:32;
 * line text is now separate from LINE struct
 * ----------------------------
 * revision 1.8
 * date: 1991/03/26 17:01:11;
 * new undo dot offset field
 * ----------------------------
 * revision 1.7
 * date: 1991/02/21 09:13:00;
 * added sideways offsets for horiz scrolling
 * ----------------------------
 * revision 1.6
 * date: 1990/12/16 22:23:19;
 * changed the default configuration
 * ----------------------------
 * revision 1.5
 * date: 1990/10/04 13:07:38;
 * added #define for ODT
 * ----------------------------
 * revision 1.4
 * date: 1990/10/03 16:00:49;
 * make backspace work for everyone
 * ----------------------------
 * revision 1.3
 * date: 1990/10/01 12:16:36;
 * make provisions for shortnames, and added HAVE_MKDIR define
 * ----------------------------
 * revision 1.2
 * date: 1990/09/28 14:36:22;
 * cleanup of ifdefs, response to porting problems
 * ----------------------------
 * revision 1.1
 * date: 1990/09/21 10:25:09;
 * initial vile RCS revision
 */


#ifdef	LATTICE
#undef	LATTICE		/* don't use their definitions...use ours	*/
#endif
#ifdef	MSDOS
#undef	MSDOS
#endif
#ifdef	CPM
#undef	CPM
#endif
#ifdef	AMIGA
#undef	AMIGA
#endif
#ifdef	EGA
#undef	EGA
#endif

/*	Machine/OS definitions			*/
/* sun, mips, generic 386, ODT, and Ultrix, see below... */
/* unix flavors */
#define BSD	1			/* UNIX BSD 4.2	and ULTRIX	*/
#define USG	0			/* UNIX system V		*/
#define V7	0			/* V7 UNIX or Coherent or BSD4.2*/
					/*     presumably also Minix?	*/
/* unix sub-flavors */
#define ODT	0			/* UNIX OPEN DESK TOP		*/
#define ULTRIX	0			/* UNIX ULTRIX			*/
#define POSIX	0
#define ISC	0			/* Interactive Systems */

/* non-unix flavors */
#define AMIGA	0			/* AmigaDOS			*/
#define ST520	0			/* ST520, TOS		       */
#define MSDOS	0			/* MS-DOS		       */
#define CPM	0			/* CP/M-86		       */
#define VMS	0			/* VAX/VMS		       */

/* the following overrides are for convenience only */
#if defined(sun)
# undef BSD
# undef USG
# define BSD	1
# define USG	0
#endif

#if defined(i386) || defined(mips)
# undef BSD
# undef USG
# define BSD	0
# define USG	1
#endif

#if ODT || ISC 
# undef POSIX
# undef BSD
# undef USG
# define POSIX	1
# define BSD	0
# define USG	1
#endif

#if ULTRIX 
# undef POSIX
# undef BSD
# undef USG
# define POSIX	1
# define BSD	1
# define USG	0
#endif

#define UNIX	(V7 | BSD | USG)	/* any unix		*/

#define OS	(UNIX | AMIGA | ST520 | MSDOS | CPM | VMS)
#if ! OS
  you need to choose a system #define...
#endif

/*	Porting constraints			*/
#define HAVE_MKDIR	1	/* if your system has the mkdir() system call */
#define SHORTNAMES	0	/* if your compiler insists on 7 char names */

/* has the select() or poll() call, only used for short sleeps in fmatch() */
#if BSD
# define HAVE_SELECT 1
#endif
#if POSIX || i386
# define HAVE_POLL 1
#endif

/*	Compiler definitions			*/
#define MWC86	0	/* marc williams compiler */
#define	LATTICE	0	/* Lattice 2.14 thruough 3.0 compilers */
#define	AZTEC	0	/* Aztec C 3.20e */
#define	MSC	0	/* MicroSoft C compile version 3 & 4 */
#define	TURBO	0	/* Turbo C/MSDOS */

/*	Terminal Output definitions		*/
/* choose one of the following */
#define TERMCAP 1			/* Use TERMCAP			*/
#define ANSI	0			/* ANSI escape sequences	*/
#define AT386	0			/* AT style 386 unix console	*/
#define	HP150	0			/* HP150 screen driver		*/
#define	HP110	0			/* HP110 screen driver		*/
#define	VMSVT	0			/* various VMS terminal entries	*/
#define VT52	0			/* VT52 terminal (Zenith).	*/
#define RAINBOW 0			/* Use Rainbow fast video.	*/
#define	IBMPC	0			/* IBM-PC CGA/MONO/EGA driver	*/
#define	DG10	0			/* Data General system/10	*/
#define	TIPC	0			/* TI Profesional PC driver	*/
#define	Z309	0			/* Zenith 100 PC family	driver	*/
#define	MAC	0			/* Macintosh			*/
#define	ATARI	0			/* Atari 520/1040ST screen	*/
#define NeWS	0			/* distributed */

/*   Special keyboard definitions	     */
#define WANGPC	0		/* WangPC - mostly escape sequences	*/
/* the WANGPC stuff isn't in the cmdtbl keyboard definitions: sorry -- pgf */

/*	Configuration options... pick and choose as you wish */

/* Appearance */
#define	TYPEAH	1	/* type ahead causes screen refresh to be delayed */
#define	REVSTA	1	/* Status line appears in reverse video		*/
#define	COLOR	0	/* color commands and windows			*/
#define	CLRMSG	0	/* space clears the message line with no insert	*/

/* Feature turnon/turnoff */
#define ANSI_SPEC	1 /* ANSI function/arrow keys */
#define	CTRLZ	0	/* add a ^Z at end of files under MSDOS only	*/
#define	DOSFILES 1	/* turn on code for DOS mode (lines that end in crlf) */
			/* use DOSFILES, for instance, if you edit DOS- */
			/*	created files under UNIX		*/
#define	CFENCE	1	/* do fench matching in CMODE			*/
#define	REBIND	1	/* permit rebinding of keys at run-time		*/
#define	APROP	1	/* Add code for Apropos command	(needs REBIND)	*/
#define	FILOCK	0	/* file locking under unix BSD 4.2 (uses scanf) */
#define	ISRCH	1	/* Incremental searches like ITS EMACS		*/
#define	FLABEL	0	/* function key label code [HP150]		*/
#define	CRYPT	0	/* file encryption? (not crypt(1) compatible!)	*/
#define MAGIC	1	/* include regular expression matching?		*/
#define	TAGS	1	/* tags support.  requires MAGIC		*/
#define	WORDPRO	1	/* "Advanced" word processing features		*/
#define	AEDIT	1	/* advanced editing options: e.g. en/detabbing	*/
#define	PROC	1	/* named procedures				*/
#define	FINDERR	1	/* finderr support. uses scanf()		*/
#define	GLOBALS	1	/* "global" command support.			*/
#define	PATHLOOK 1	/* look along $PATH for startup and help files	*/
#define	SCROLLCODE 1	/* code in display.c for scrolling the screen.
			   Only useful if your display can scroll
			   regions, or at least insert/delete lines. 
			   ANSI, TERMCAP, and AT386 can do this		 */
#define CVMVAS	1	/* arguments to forward/back page and half page */
			/* are in pages	instead of rows */
#define PRETTIER_SCROLL 1 /* can improve the appearance of a scrolling screen */
#define STUTTER_SEC_CMD 0 /* must the next/prev section commands (i.e.
				']]' and '[[' be stuttered?  they must be
				stuttered in real vi, I prefer them not
				to be */

/*	Code size options	*/
#define	FEWNAMES 0	/* strip some names - will no longer be bindable */
#define	SMALLER	0	/* strip out a bunch of uemacs fluff */
			/* 	(to each their own... :-)  pgf) */

/*	Debugging options	*/
#define	RAMSIZE	0	/* dynamic RAM memory usage tracking */
#define	RAMSHOW	0	/* auto dynamic RAM reporting */
#define	VMALLOC	0	/* verify malloc operation (slow!) */
#define	DEBUG	1	/* allows core dump from keyboard under UNIX */
#define	TIMING	0	/* shows user time spent on each user command */
			/* TIMING doesn't work yet... sorry  -pgf */ 
#define DEBUGM	1	/* $debug triggers macro debugging		*/
#define	VISMAC	0	/* update display during keyboard macros	*/


/* That's the end of the user selections -- the rest is static definition */
/* (i.e. you shouldn't need to touch anything below here */
/* ====================================================================== */

#if SHORTNAMES
#include "shorten/remap.h"
#endif

/*	System dependant library redefinitions, structures and includes	*/

#if	TURBO
#include      <dos.h>
#include      <mem.h>
#undef peek
#undef poke
#define       peek(a,b,c,d)   movedata(a,b,FP_SEG(c),FP_OFF(c),d)
#define       poke(a,b,c,d)   movedata(FP_SEG(c),FP_OFF(c),a,b,d)
#endif

#if	VMS
#define	atoi	xatoi
#define	abs	xabs
#define	getname	xgetname
#endif

#if	LATTICE
#define	unsigned
#endif

#if	AZTEC
#undef	fputc
#undef	fgetc
#if	MSDOS
#define	fgetc	a1getc
#else
#define	fgetc	agetc
#endif
#define	fputc	aputc
#define	int86	sysint
#define	intdos(a, b)	sysint(33, a, b)
#define	inp	inportb
#define	outp	outportb

struct XREG {
	int ax,bx,cx,dx,si,di;
};

struct HREG {
	char al,ah,bl,bh,cl,ch,dl,dh;
};

union REGS {
	struct XREG x;
	struct HREG h;
};
#endif

#if	MSDOS & MWC86
#include	<dos.h>
#define	int86(a, b, c)	intcall(b, c, a)
#define	inp	in

struct XREG {
	int ax,bx,cx,dx,si,di,ds,es,flags;
};

struct HREG {
	char al,ah,bl,bh,cl,ch,dl,dh;
	int ds,es,flags;
};

union REGS {
	struct XREG x;
	struct HREG h;
};
#endif

#if	MSDOS & MSC
#include	<dos.h>
#include	<memory.h>
#define	peek(a,b,c,d)	movedata(a,b,FP_SEG(c),FP_OFF(c),d)
#define	poke(a,b,c,d)	movedata(FP_SEG(c),FP_OFF(c),a,b,d)
#define	movmem(a, b, c)		memcpy(b, a, c)
#endif

#if	MSDOS & LATTICE
#undef	CPM
#undef	LATTICE
#include	<dos.h>
#undef	CPM
#endif

#if	VMS
#define	unlink(a)	delete(a)
#endif

/*	define some ability flags */

#if	IBMPC | Z309
#define	MEMMAP	1
#else
#define	MEMMAP	0
#endif

#if	((MSDOS) & (LATTICE | AZTEC | MSC | TURBO)) | UNIX
#define	ENVFUNC	1
#else
#define	ENVFUNC	0
#endif

#if BSD
#define strchr index
#define strrchr rindex
#endif

/*	internal constants	*/

#define	NBINDS	100			/* max # of bound prefixed keys	*/
#define NFILEN	80			/* # of bytes, file name	*/
#define NBUFN	20			/* # of bytes, buffer name	*/
#define NLINE	256			/* # of bytes, input line	*/
#define	NSTRING	128			/* # of bytes, string buffers	*/
#define NKBDM	256			/* # of strokes, keyboard macro */
#define NPAT	128			/* # of bytes, pattern		*/
#define HUGE	60000			/* Huge number			*/
#define	NLOCKS	100			/* max # of file locks active	*/
#define	NCOLORS	8			/* number of supported colors	*/
#define	KBLOCK	256			/* sizeof kill buffer chunks	*/
#define	NKREGS	36			/* number of kill buffers	*/
#define	NBLOCK	16			/* line block chunk size	*/
#define	NVSIZE	10			/* max #chars in a var name	*/

/* SPEC is just 8th bit set, for convenience in some systems (like NeWS?) */
#define SPEC	0x0080			/* special key (function keys)	*/
#define CTLA	0x0100			/* ^A flag, or'ed in		*/
#define CTLX	0x0200			/* ^X flag, or'ed in		*/

#define kcod2key(c) (c & 0x7f)		/* strip off the above prefixes */

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
#define	TMPSTOP	3			/* temporary stop, record can resume */

/* flook options */
#define FL_HERE 1
#define FL_HERE_HOME 2
#define FL_ANYWHERE 3

/* bfind options */
#define OK_CREAT TRUE
#define NO_CREAT FALSE

/* kbd_string options */
#define EXPAND TRUE
#define NO_EXPAND FALSE

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
 * PTBEG, PTEND, FORWARD, and REVERSE are all toggle-able values for
 * the scan routines.
 */
#define	PTBEG	0	/* Leave the point at the beginning on search	*/
#define	PTEND	1	/* Leave the point at the end on search		*/
#define	FORWARD	0			/* forward direction		*/
#define REVERSE	1			/* backwards direction		*/

#define FIOSUC	0			/* File I/O, success.		*/
#define FIOFNF	1			/* File I/O, file not found.	*/
#define FIOEOF	2			/* File I/O, end of file.	*/
#define FIOERR	3			/* File I/O, error.		*/
#define	FIOMEM	4			/* File I/O, out of memory	*/
#define	FIOFUN	5			/* File I/O, eod of file/bad line*/

/* three flavors of insert mode	*/
/* it's FALSE, or one of:	*/
#define INSERT 1
#define OVERWRITE 2
#define REPLACECHAR 3

/* kill register control */
#define KNEEDCLEAN   0x01		/* Kill register needs cleaning */
#define KYANK	0x02			/* Kill register resulted from yank */
#define KLINES	0x04			/* Kill register contains full lines */
#define KAPPEND  0x04			/* Kill register should be appended */

/* operator types.  Needed mainly because word movement changes depending on
	whether operator is "delete" or not.  Aargh.  */
#define OPDEL 1
#define OPOTHER 2

/* define these so C-fence matching doesn't get confused when we're editing
	the cfence code itself */
#define LBRACE '{'
#define RBRACE '}'


#if UNIX
#define	PATHCHR	':'
#else
#define	PATHCHR	';'
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

#define	nextab(a)	(((a / TABVAL) + 1) * TABVAL)

#ifdef	abs
#undef	abs
#endif

/* these are the bits that go into the _chartypes_ array */
/* the macros below test for them */
#define N_chars 128
#define _upper 1		/* upper case */
#define _lower 2		/* lower case */
#define _digit 4		/* digits */
#define _space 8		/* whitespace */
#define _bspace 16		/* backspace character (^H, DEL, and user's) */
#define _cntrl 32		/* control characterts, including DEL */
#define _print 64		/* printable */
#define _punct 128		/* punctuation */
#define _ident 256		/* is typically legal in "normal" identifier */
#define _path 512		/* is typically legal in a file's pathname */
#define _wild 2048		/* is typically a shell wildcard char */
#define _linespec 4096		/* ex-style line range: 1,$ or 13,15 or % etc.*/

/* these intentionally match the ctypes.h definitions, except that
	they force the char to 7-bit ascii first */
#define istype(sometype,c)	(_chartypes_[(c)&(N_chars-1)] & (sometype))
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
#define ispath(c)	istype(_path, c)
#define isbackspace(c)	istype(_bspace, c)
#define islinespecchar(c)	istype(_linespec, c)

/* DIFCASE represents the difference between upper
   and lower case letters, DIFCNTRL the difference between upper case and
   control characters.	They are xor-able values.  */
#define	DIFCASE		0x20
#define	DIFCNTRL	0x40
#define toupper(c)	((c)^DIFCASE)
#define tolower(c)	((c)^DIFCASE)
#define tocntrl(c)	((c)^DIFCNTRL)
#define toalpha(c)	((c)^DIFCNTRL)

#define ESC	tocntrl('[')
#define RECORDED_ESC	-2

#define BEL	tocntrl('G')	/* ascii bell character		*/

/*	Dynamic RAM tracking and reporting redefinitions	*/

#if	RAMSIZE
#define	malloc	allocate
#define	free	release
#endif

#if VMALLOC
char *vmalloc();
void vfree();
void rvverify();
char *vrealloc();
char *vcalloc();
void vdump();
# define malloc(x) vmalloc(x,__FILE__,__LINE__)
# define free(x) vfree(x,__FILE__,__LINE__)
# define realloc(x,y) vrealloc(x,y,__FILE__,__LINE__)
# define calloc(x,y) vcalloc(x,y,__FILE__,__LINE__)
# define vverify(s) rvverify(s,__FILE__,__LINE__)
#else
# define vverify(s) ;
#endif

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
typedef struct	LINE {
	struct	LINE *l_fp;		/* Link to the next line	*/
	struct	LINE *l_bp;		/* Link to the previous line	*/
	int   l_size;		      /* Allocated size 	      */
	int   l_used;		      /* Used size		      */
	char *l_text;
	union {
	    struct  LINE *l_stklnk;	/* Link for undo stack		*/
	    long	l_flag;		/* flags for undo ops		*/
	} l;
}	LINE;

/* flag values */
#define LCOPIED 1	/* original line is already on an undo stack */
#define LGMARK 2	/* line matched a global scan */

/* macros to ease the use of lines */
#define lforw(lp)	((lp)->l_fp)
#define lback(lp)	((lp)->l_bp)
#define lgetc(lp, n)	((lp)->l_text[(n)]&0xFF)
#define lputc(lp, n, c) ((lp)->l_text[(n)]=(c))
#define llength(lp)	((lp)->l_used)
#define l_nxtundo	l.l_stklnk
#define liscopied(lp)	(lp->l.l_flag & LCOPIED)
#define lismarked(lp)	(lp->l.l_flag & LGMARK)
#define lsetcopied(lp)		(lp->l.l_flag |= LCOPIED)
#define lsetnotcopied(lp)	(lp->l.l_flag &= ~LCOPIED)
#define lsetmarked(lp)		(lp->l.l_flag |= LGMARK)
#define lsetnotmarked(lp)	(lp->l.l_flag &= ~LGMARK)
#define lsetclear(lp)	(lp->l.l_flag = 0)
#define LINENOTREAL	((int)(-1))
#define LINEUNDOPATCH	((int)(-2))
#define MARKPATCH	((int)(-3))
#define lisreal(lp)	((lp)->l_used >= 0)
#define lisnotreal(lp)	   ((lp)->l_used == LINENOTREAL)
#define lislinepatch(lp)     ((lp)->l_used == LINEUNDOPATCH)
#define lismarkpatch(lp)     ((lp)->l_used == MARKPATCH)
#define lispatch(lp)	 (lislinepatch(lp) || lismarkpatch(lp))
#define lneedscopying(lp)     ((lp)->l_copied != TRUE)

/* marks are a line and an offset into that line */
typedef struct MARK {
	LINE *l;
	int o;
} MARK;

/* some macros that take marks as arguments */
#define is_at_end_of_line(m)	(m.o == llength(m.l))
#define is_empty_line(m)	(llength(m.l) == 0)
#define sameline(m1,m2)		(m1.l == m2.l)
#define samepoint(m1,m2)	((m1.l == m2.l) && (m1.o == m2.o))
#define char_at(m)		(lgetc(m.l,m.o))
#define put_char_at(m,c)	(lputc(m.l,m.o,c))
#define is_header_line(m,bp)	( m.l == bp->b_line.l)
#define is_last_line(m,bp)	( lforw(m.l) == bp->b_line.l)
#define is_first_line(m,bp)	( lback(m.l) == bp->b_line.l)

/* settable values have their names stored here, along with a synonym, and
	what type they are */
struct VALNAMES {
		char *name;
		char *shortname;
		short type;
};
/* the values of VALNAMES->type */
#define VALTYPE_INT 0
#define VALTYPE_STRING 1
#define VALTYPE_BOOL 2


/* this is to ensure values can be of any type we wish.
   more can be added if needed.  */
union V {
	int i;
	char *p;
};

struct VAL {
	union V v;
	union V *vp;
};

/* these are the boolean, integer, and pointer value'd settings that are
	associated with a window, and usually settable by a user.  There
	is a global set that is inherited into a buffer, and its windows
	in turn are inherit the buffer's set. */
#define	WMDLIST		0		/* "list" mode -- show tabs and EOL */
/* put more boolean-valued things here */
#define	MAX_BOOL_W_VALUE	0	/* max of boolean values	*/

#define WVAL_SIDEWAYS	(MAX_BOOL_W_VALUE+1)
#define WVAL_FCOLOR	(MAX_BOOL_W_VALUE+2)
#define WVAL_BCOLOR	(MAX_BOOL_W_VALUE+3)
/* put more int-valued things here */
#define	MAX_INT_W_VALUE	(MAX_BOOL_W_VALUE+3) /* max of integer-valued modes */

/* put more string-valued things here */
#define	MAX_STRING_W_VALUE (MAX_INT_W_VALUE+0) /* max of string-valued modes */
#define	MAX_W_VALUES	(MAX_STRING_W_VALUE) /* max of buffer values */

typedef struct W_VALUES {
	/* each entry is a val, and a ptr to a val */
	struct VAL wv[MAX_W_VALUES+1];
} W_VALUES;


/* these are window properties affecting window appearance _only_ */
typedef struct	W_TRAITS {
	MARK 	w_dt;			/* Line containing "."	       */
		/* i don't think "mark" needs to be here -- I think it 
			could safely live only in the buffer -pgf */
#ifdef WINMARK
	MARK 	w_mk;	        	/* Line containing "mark"      */
#endif
	MARK 	w_ld;	        	/* Line containing "lastdotmark"*/
	MARK 	w_ln;		/* Top line in the window (offset unused) */
	W_VALUES w_vals;
} W_TRAITS;

#define global_w_val(which) global_w_values.wv[which].v.i
#define set_global_w_val(which,val) global_w_val(which) = val
#define global_w_val_ptr(which) global_w_values.wv[which].v.p
#define set_global_w_val_ptr(which,val) global_w_val_ptr(which) = val

#define w_val(wp,val) (wp->w_traits.w_vals.wv[val].vp->i)
#define set_w_val(wp,which,val) w_val(wp,which) = val
#define w_val_ptr(wp,val) (wp->w_traits.w_vals.wv[val].vp->p)
#define set_w_val_ptr(wp,which,val) w_val_ptr(wp,which) = val

#define make_local_w_val(wp,which)  \
	wp->w_traits.w_vals.wv[which].vp = &(wp->w_traits.w_vals.wv[which].v)
#define make_global_w_val(wp,which)  \
	wp->w_traits.w_vals.wv[which].vp = &(global_wvalues.wv[which].v)

#define is_global_w_val(wp,which)  \
	(wp->w_traits.w_vals.wv[which].vp == &(global_w_values.wv[which].v))
#define is_local_w_val(wp,which)  \
	(wp->w_traits.w_vals.wv[which].vp == &(wp->w_traits.w_vals.wv[which].v))

#define gfcolor global_w_val(WVAL_FCOLOR)
#define gbcolor global_w_val(WVAL_BCOLOR)

/* buffer mode flags	*/
/* the indices of B_VALUES.v[] */
/* the first set are boolean */
#define	MDAIND		0		/* auto-indent */
#define	MDASAVE		1		/* auto-save mode		*/
#define	MDBACKLIMIT	2		/* backspace limited in insert mode */
#define	MDCMOD		3		/* C indentation and fence match*/
#define	MDCRYPT		4		/* encrytion mode active	*/
#define	MDDOS		5		/* "dos" mode -- lines end in crlf */
#define	MDIGNCASE	6		/* Exact matching for searches	*/
#define MDMAGIC		7		/* regular expresions in search */
#define	MDSHOWMAT	8		/* auto-indent */
#define	MDVIEW		9		/* read-only buffer		*/
#define	MDSWRAP 	10		/* wrap-around search mode	*/
#define	MDWRAP		11		/* word wrap			*/
#define	MAX_BOOL_B_VALUE	11	/* max of boolean values	*/

#define VAL_ASAVE	(MAX_BOOL_B_VALUE+1)
#define VAL_C_TAB	(MAX_BOOL_B_VALUE+2)
#define VAL_FILL	(MAX_BOOL_B_VALUE+3)
#define VAL_TAB		(MAX_BOOL_B_VALUE+4)
#define VAL_TAGLEN	(MAX_BOOL_B_VALUE+5)
#define	MAX_INT_B_VALUE	(MAX_BOOL_B_VALUE+5) /* max of integer-valued modes */

#define VAL_CSUFFIXES	(MAX_INT_B_VALUE+1)
#define VAL_CWD		(MAX_INT_B_VALUE+2)
#define VAL_TAGS	(MAX_INT_B_VALUE+3)
#define	MAX_STRING_B_VALUE (MAX_INT_B_VALUE+3) /* max of string-valued modes */

#define	MAX_B_VALUES	(MAX_STRING_B_VALUE) /* max of buffer values */

typedef struct B_VALUES {
	/* each entry is a val, and a ptr to a val */
	struct VAL bv[MAX_B_VALUES+1];
} B_VALUES;

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
	B_VALUES b_values;		/* buffer traits we inherit from */
					/*  global values		*/
	struct	W_TRAITS b_wtraits;	/* saved window traits, while we're */
					/*  not displayed		*/
	LINE 	*b_udstks[2];		/* undo stack pointers		*/
	MARK 	b_uddot[2];		/* Link to "." before undoable op*/
	short	b_udstkindx;		/* which of above to use	*/
	LINE	*b_LINEs;		/* block-malloced LINE structs */
	LINE	*b_LINEs_end;		/* end of 	"	"	" */
	LINE	*b_freeLINEs;		/* list of free " 	"	" */
	unsigned char	*b_ltext;	/* block-malloced text */
	unsigned char	*b_ltext_end;	/* end of block-malloced text */
	LINE 	*b_ulinep;		/* pointer at 'Undo' line	*/
	int	b_active;		/* window activated flag	*/
	int	b_nwnd;		        /* Count of windows on buffer   */
	int	b_flag;		        /* Flags 		        */
	char	b_fname[NFILEN];	/* File name			*/
	char	b_bname[NBUFN]; 	/* Buffer name			*/
#if	CRYPT
	char	b_key[NPAT];		/* current encrypted key	*/
#endif
}	BUFFER;

#define global_b_val(which) global_b_values.bv[which].v.i
#define set_global_b_val(which,val) global_b_val(which) = val
#define global_b_val_ptr(which) global_b_values.bv[which].v.p
#define set_global_b_val_ptr(which,val) global_b_val_ptr(which) = val

#define b_val(bp,val) (bp->b_values.bv[val].vp->i)
#define set_b_val(bp,which,val) b_val(bp,which) = val
#define b_val_ptr(bp,val) (bp->b_values.bv[val].vp->p)
#define set_b_val_ptr(bp,which,val) b_val_ptr(bp,which) = val

#define make_local_b_val(bp,which)  \
		bp->b_values.bv[which].vp = &(bp->b_values.bv[which].v)
#define make_global_b_val(bp,which)  \
		bp->b_values.bv[which].vp = &(global_b_values.bv[which].v)

#define is_global_b_val(bp,which)  \
		(bp->b_values.bv[which].vp == &(global_b_values.bv[which].v))
#define is_local_b_val(bp,which)  \
		(bp->b_values.bv[which].vp == &(bp->b_values.bv[which].v))

#define is_empty_buf(bp) (lforw(bp->b_line.l) == bp->b_line.l)
#define b_sideways b_wtraits.w_vals.w_side
#define b_dot b_wtraits.w_dt
#ifdef WINMARK
#define b_mark b_wtraits.w_mk
#endif
#define b_lastdot b_wtraits.w_ld
#define b_wline b_wtraits.w_ln

/* values for b_flag */
#define BFINVS	0x01			/* Internal invisible buffer	*/
#define BFCHG	0x02			/* Changed since last write	*/
#define BFSCRTCH   0x04 		/* scratch -- gone on last close */

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
}	WINDOW;

#define w_dot w_traits.w_dt
#ifdef WINMARK
#define w_mark w_traits.w_mk
#endif
#define w_lastdot w_traits.w_ld
#define w_line w_traits.w_ln
#define w_values w_traits.w_vals
#define w_mode w_traits.w_vals.w_mod
#define w_sideways w_traits.w_vals.w_side
#define w_fcolor w_traits.w_vals.w_fcol
#define w_bcolor w_traits.w_vals.w_bcol

#define DOT curwp->w_traits.w_dt
#ifdef WINMARK
#define MK curwp->w_traits.w_mk
#else
#define MK Mark
#endif

#define WFFORCE 0x01			/* Window needs forced reframe	*/
#define WFMOVE	0x02			/* Movement from line to line	*/
#define WFEDIT	0x04			/* Editing within a line	*/
#define WFHARD	0x08			/* Better do a full display	*/
#define WFMODE	0x10			/* Update mode line.		*/
#define	WFCOLR	0x20			/* Needs a color change		*/
#define	WFKILLS	0x40			/* something was deleted	*/
#define	WFINS	0x80			/* something was inserted	*/


/* the next set are global, bit-mapped, but are meaningless per-buffer */
#define	NUMOTHERMODES	2 /* # of defined modes		*/
#define OTH_LAZY 0x01
#define OTH_VERS 0x02

#define TABVAL curtabstopval
#define globalfillcol  global_b_val(VAL_FILL)
#define fillcol 	b_val(curbp,VAL_FILL)
/* #define gmode		global_b_val(VAL_MODES) */
#define gasave		global_b_val(VAL_ASAVE)

/*
 * The starting position of a region, and the size of the region in
 * characters, is kept in a region structure.  Used by the region commands.
 */
typedef struct	{
	MARK 	r_orig;			/* Origin LINE address. 	*/
	MARK	r_end;			/* Ending LINE address. 	*/
	long	r_size; 		/* Length in characters.	*/
}	REGION;

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
	int	(*t_open)();		/* Open terminal at the start.	*/
	int	(*t_close)();		/* Close terminal at end.	*/
	int	(*t_kopen)();		/* Open keyboard		*/
	int	(*t_kclose)();		/* close keyboard		*/
	int	(*t_getchar)(); 	/* Get character from keyboard. */
	int	(*t_putchar)(); 	/* Put character to display.	*/
	int	(*t_flush)();		/* Flush output buffers.	*/
	int	(*t_move)();		/* Move the cursor, origin 0.	*/
	int	(*t_eeol)();		/* Erase to end of line.	*/
	int	(*t_eeop)();		/* Erase to end of page.	*/
	int	(*t_beep)();		/* Beep.			*/
	int	(*t_rev)();		/* set reverse video state	*/
	int	(*t_rez)();		/* change screen resolution	*/
#if	COLOR
	int	(*t_setfor)();		/* set forground color		*/
	int	(*t_setback)();		/* set background color		*/
#endif
#if	SCROLLCODE
	int	(*t_scroll)();		/* scroll a region of the screen */
#endif
}	TERM;

/*	TEMPORARY macros for terminal I/O  (to be placed in a machine
					    dependant place later)	*/

#define	TTopen		(*term.t_open)
#define	TTclose		(*term.t_close)
#define	TTkopen		(*term.t_kopen)
#define	TTkclose	(*term.t_kclose)
#define	TTgetc		(*term.t_getchar)
#define	TTputc		(*term.t_putchar)
#define	TTflush		(*term.t_flush)
#define	TTmove		(*term.t_move)
#define	TTeeol		(*term.t_eeol)
#define	TTeeop		(*term.t_eeop)
#define	TTbeep		(*term.t_beep)
#define	TTrev		(*term.t_rev)
#define	TTrez		(*term.t_rez)
#if	COLOR
#define	TTforg		(*term.t_setfor)
#define	TTbacg		(*term.t_setback)
#endif


/* Commands are represented as CMDFUNC structures, which contain a
 *	pointer to the actual function, and flags which help to classify it.
 *	(things like is it a MOTION, can it be UNDOne)
 *
 *	These structures are generated automatically from the cmdtbl file,
 *	and can be found in the file nefunc.h
*/
typedef  struct {
	int (*c_func)();	/* function name is bound to */
	unsigned long c_flags;		/* what sort of command is it? */
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
 *	is looked up one of two ways:  single character 7-bit ascii commands
 *	(by far the majority) are simply indexed into a 128 element array of
 *	CMDFUNC pointers.  Other commands (those with ^A, ^X, or SPEC
 *	prefixes) are searched for in a binding table, made up of KBIND
 *	structures.  This structure contains the command code, and again, a
 *	pointer to the CMDFUNC structure for the command 
 *
 *	The asciitbl array, and the kbindtbl array are generated automatically
 *	from the cmdtbl file, and can be found in the file nebind.h
 */
typedef struct {
	short	k_code; 		/* Key code			*/
	CMDFUNC	*k_cmd;
}	KBIND;


/* these are the flags which can appear in the CMDFUNC structure, describing
	a command */
#define NONE	0
#define UNDO	0x01	/* command is undo-able, so clean up undo lists */
#define REDO	0x02	/* command is redo-able, record it for dotcmd */
#define MOTION	0x04	/* command causes motion, okay after operator cmds */
#define FL	0x08	/* if command causes motion, opers act on full lines */
#define ABS	0x10	/* command causes absolute (i.e. non-relative) motion */
#define GOAL	0x20	/* column goal should be retained */
#define GLOBOK	0x40	/* permitted after global command */
#define OPER	0x80	/* function is an operator, affects a region */
#define LISTED	0x100	/* internal use only -- used in describing bindings
				to only describe each once */

/* these flags are ex argument descriptors. I simply moved them over 
	from elvis.  Not all are used or honored or implemented */
#define FROM	(1<<16)		/* allow a linespec */
#define	TO	(2<<16)		/* allow a second linespec */
#define BANG	(4<<16)		/* allow a ! after the command name */
#define EXTRA	(8<<16)		/* allow extra args after command name */
#define XFILE	(16<<16)	/* expand wildcards in extra part */
#define NOSPC	(32<<16)	/* no spaces allowed in the extra part */
#define	DFLALL	(64<<16)	/* default file range is 1,$ */
#define DFLNONE	(128<<16)	/* no default file range */
#define NODFL	(256<<16)	/* do not default to the current file name */
#define EXRCOK	(512<<16)	/* can be in a .exrc file */
#define NL	(1024<<16)	/* if !exmode, then write a newline first */
#define PLUS	(2048<<16)	/* allow a line number, as in ":e +32 foo" */
#define ZERO	(4096<<16)	/* allow 0 to be given as a line number */
#define FILES	(XFILE + EXTRA)	/* multiple extra files allowed */
#define WORD1	(EXTRA + NOSPC)	/* one extra word allowed */
#define FILE1	(FILES + NOSPC)	/* 1 file allowed, defaults to current file */
#define NAMEDF	(FILE1 + NODFL)	/* 1 file allowed, defaults to "" */
#define NAMEDFS	(FILES + NODFL)	/* multiple files allowed, default is "" */
#define RANGE	(FROM + TO)	/* range of linespecs allowed */

/*	The editor holds deleted text chunks in the KILL registers. The
	kill registers are logically a stream of ascii characters, however
	due to unpredicatable size, get implemented as a linked
	list of chunks. (The d_ prefix is for "deleted" text, as k_
	was taken up by the keycode structure)
*/

typedef	struct KILL {
	struct KILL *d_next;	/* link to next chunk, NULL if last */
	char d_chunk[KBLOCK];	/* deleted text */
} KILL;

typedef struct KILLREG {
	struct KILL *kbufp;	/* current kill register chunk pointer */
	struct KILL *kbufh;	/* kill register header pointer	*/
	int   kused;		/* # of bytes used in kill last chunk	*/
	short kbflag;		/* flags describing kill register	*/
} KILLREG;

/*	When the command interpretor needs to get a variable's name,
	rather than its value, it is passed back as a VDESC variable
	description structure. The v_num field is an index into the
	appropriate variable table.
*/

typedef struct VDESC {
	int v_type;	/* type of variable */
	int v_num;	/* ordinal pointer to variable in list */
} VDESC;

/*	The !WHILE directive in the execution language needs to
	stack references to pending whiles. These are stored linked
	to each currently open procedure via a linked list of
	the following structure
*/

typedef struct WHBLOCK {
	LINE *w_begin;		/* ptr to !while statement */
	LINE *w_end;		/* ptr to the !endwhile statement*/
	int w_type;		/* block type */
	struct WHBLOCK *w_next;	/* next while */
} WHBLOCK;

#define	BTWHILE		1
#define	BTBREAK		2

/*
 * Incremental search defines.
 */
#if	ISRCH

#define	CMDBUFLEN	256	/* Length of our command buffer */

#define IS_REVERSE	tocntrl('R')	/* Search backward */
#define	IS_FORWARD	tocntrl('F')	/* Search forward */

#endif

#if	MAGIC

/*
 * Defines for the metacharacters in the regular expressions in search
 * routines.
 */

#define	MCNIL		0	/* Like the '\0' for strings.*/
#define	LITCHAR		1
#define	ANY		2
#define	CCL		3
#define	NCCL		4
#define	BOL		5
#define	EOL		6
#define	CLOSURE		256	/* An or-able value.*/
#define	MASKCL		CLOSURE - 1

#define	MC_ANY		'.'	/* 'Any' character (except newline).*/
#define	MC_CCL		'['	/* Character class.*/
#define	MC_NCCL		'^'	/* Negate character class.*/
#define	MC_RCCL		'-'	/* Range in character class.*/
#define	MC_ECCL		']'	/* End of character class.*/
#define	MC_BOL		'^'	/* Beginning of line.*/
#define	MC_EOL		'$'	/* End of line.*/
#define	MC_CLOSURE	'*'	/* Closure - does not extend past newline.*/

#define	MC_ESC		'\\'	/* Escape - suppress meta-meaning.*/

#define	BIT(n)		(1 << (n))	/* An integer with one bit set.*/
#define	CHCASE(c)	((c) ^ DIFCASE)	/* Toggle the case of a letter.*/

/* HICHAR - 1 is the largest character we will deal with.
 * HIBYTE represents the number of bytes in the bitmap.
 */

#define	HICHAR		256
#define	HIBYTE		HICHAR >> 3

typedef char	*BITMAP;

typedef	struct {
	short int	mc_type;
	union {
		int	lchar;
		BITMAP	cclmap;
	} u;
} MC;
#endif

