/*	ESTRUCT:	Structure and preprocesser defines for
			vile.  Reshaped from the original, which
			was for MicroEMACS 3.9

			vile is by Paul Fox
			MicroEmacs was written by Dave G. Conroy
			modified by Steve Wilhite, George Jones
			substantially modified by Daniel Lawrence
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

/* non-unix flavors */
#define AMIGA	0			/* AmigaDOS			*/
#define ST520	0			/* ST520, TOS		       */
#define MSDOS	0			/* MS-DOS		       */
#define CPM	0			/* CP/M-86		       */
#define VMS	0			/* VAX/VMS		       */

/* the following overrides for sun, i386, and mips are for convenience only */
#if sun
# undef BSD
# undef USG
# define BSD	1			/* UNIX BSD 4.2	and ULTRIX	*/
# define USG	0			/* UNIX system V		*/
#endif

#if i386 || mips
# undef BSD
# undef USG
# define BSD	0			/* UNIX BSD 4.2	and ULTRIX	*/
# define USG	1			/* UNIX system V		*/
#endif

#if ODT
# undef POSIX
# undef BSD
# undef USG
# define POSIX	1
# define BSD	0			/* UNIX BSD 4.2	and ULTRIX	*/
# define USG	1			/* UNIX system V		*/
#endif

#if ULTRIX
# undef POSIX
# undef BSD
# undef USG
# define POSIX	1
# define BSD	1
# define USG	0			/* UNIX system V		*/
#endif

#define UNIX	(V7 | BSD | USG)	/* any unix		*/

#define OS	(UNIX | AMIGA | ST520 | MSDOS | CPM | VMS)
#if ! OS
  you need to choose a system #define...
#endif

/*	Porting constraints			*/
#define HAVE_MKDIR	1	/* if your system has the mkdir() system call */
#define SHORTNAMES	0	/* if your compiler insists on 7 char names */

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
#define DEBUGM	0	/* $debug triggers macro debugging		*/
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

#define TABVAL tabval
#define TABMASK tabmask

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

/* two flavors of insert mode */
#define INSERT 1
#define OVERWRITE 2

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

#define	nextab(a)	((a & ~TABMASK) + TABVAL)

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
 * There is a window structure allocated for every active display window. The
 * windows are kept in a big list, in top to bottom screen order, with the
 * listhead at "wheadp". Each window contains its own values of dot and mark.
 * The flag field contains some bits that are set by commands to guide
 * redisplay. Although this is a bit of a compromise in terms of decoupling,
 * the full blown redisplay is just too expensive to run for every input
 * character.
 */
typedef struct	WINDOW {
	struct	WINDOW *w_wndp; 	/* Next window			*/
	struct	BUFFER *w_bufp; 	/* Buffer displayed in window	*/
	struct	LINE *w_linep;		/* Top line in the window	*/
	struct	LINE *w_dotp;		/* Line containing "."		*/
	struct	LINE *w_mkp;	      /* Line containing "mark"       */
	struct	LINE *w_ldmkp;	      /* Line containing "lastdotmark"*/
	int	w_doto;		      /* Byte offset for "."	      */
	int	w_mko;		    /* Byte offset for "mark"	    */
	int	w_ldmko;		    /* Byte offset for "lastdotmark"*/
	int	w_toprow;	       /* Origin 0 top row of window   */
	int	w_ntrows;	       /* # of rows of text in window  */
	int	w_force; 	       /* If non-zero, forcing row.   */
	int	w_flag;		       /* Flags.		       */
	int	w_sideways;	       /* sideways offset */
#if	COLOR
	int	w_fcolor;		/* current forground color	*/
	int	w_bcolor;		/* current background color	*/
#endif
}	WINDOW;

#define WFFORCE 0x01			/* Window needs forced reframe	*/
#define WFMOVE	0x02			/* Movement from line to line	*/
#define WFEDIT	0x04			/* Editing within a line	*/
#define WFHARD	0x08			/* Better do a full display	*/
#define WFMODE	0x10			/* Update mode line.		*/
#define	WFCOLR	0x20			/* Needs a color change		*/
#define	WFKILLS	0x40			/* something was deleted	*/
#define	WFINS	0x80			/* something was inserted	*/

struct MARK {
	struct LINE *markp;
	int marko;
};

/*
 * Text is kept in buffers. A buffer header, described below, exists for every
 * buffer in the system. The buffers are kept in a big list, so that commands
 * that search for a buffer by name can find the buffer header. There is a
 * safe store for the dot and mark in the header, but this is only valid if
 * the buffer is not being displayed (that is, if "b_nwnd" is 0). The text for
 * the buffer is kept in a circularly linked list of lines, with a pointer to
 * the header line in "b_linep"	Buffers may be "Inactive" which means the files associated with them
 * have not been read in yet. These get read in at "use buffer" time.
 */
typedef struct	BUFFER {
	struct	BUFFER *b_bufp; 	/* Link to next BUFFER		*/
	struct	MARK *b_nmmarks;	/* named marks a-z		*/
	struct	LINE *b_linep;		/* Link to the header LINE	*/
	struct	LINE *b_dotp;		/* Link to "." LINE structure	*/
	struct	LINE *b_markp;		/* The same as the above two,	*/
	struct	LINE *b_ldmkp;	        /* The same as the above two,   */
	int	b_doto;		        /* Offset of "." in above LINE  */
	int	b_marko;		/* same but for the "mark"	*/
	int	b_ldmko;		/* same but for the "last dot mark" */
	int	b_sideways;		/* sideways offset		*/
	int	b_mode;			/* editor mode of this buffer	*/
	struct	LINE *b_udstks[2];	/* undo stack pointers		*/
	short	b_udstkindx;		/* which of above to use	*/
	struct	LINE *b_uddotps[2];	/* Link to "." before undoable op*/
	int	b_uddotos[2];		/* offset of "." before undoable op*/
	struct	LINE *b_ulinep;		/* pointer at 'Undo' line	*/
	int	b_active;		/* window activated flag	*/
	int	b_nwnd;		        /* Count of windows on buffer   */
	int	b_flag;		        /* Flags 		        */
	char	b_fname[NFILEN];	/* File name			*/
	char	b_bname[NBUFN]; 	/* Buffer name			*/
#if	CRYPT
	char	b_key[NPAT];		/* current encrypted key	*/
#endif
}	BUFFER;

#define BFINVS	0x01			/* Internal invisable buffer	*/
#define BFCHG	0x02			/* Changed since last write	*/
#define BFSCRTCH   0x04 		/* scratch -- gone on last close */

/*	mode flags	*/
/* the first set are bitmapped, and are inherited from global to per-buffer */
#define	NUMMODES	11 /* # of defined modes		*/
#define	MDWRAP	0x0001			/* word wrap			*/
#define	MDCMOD	0x0002			/* C indentation and fence match*/
#define	MDSWRAP 0x0004			/* wrap-around search mode	*/
#define	MDEXACT	0x0008			/* Exact matching for searches	*/
#define	MDVIEW	0x0010			/* read-only buffer		*/
#define MDMAGIC	0x0020			/* regular expresions in search */
#define	MDCRYPT	0x0040			/* encrytion mode active	*/
#define	MDASAVE	0x0080			/* auto-save mode		*/
#define	MDLIST	0x0100			/* "list" mode -- show tabs and EOL */
#define	MDDOS	0x0200			/* "dos" mode -- lines end in crlf */
#define	MDAIND	0x0400			/* auto-indent */

/* the next set are global, bit-mapped, but are meaningless per-buffer */
#define	NUMOTHERMODES	2 /* # of defined modes		*/
#define OTH_LAZY 0x01
#define OTH_VERS 0x02

/* the last set are global, and have values */
#define	NUMVALUEMODES	2 /* # of defined modes		*/
#define VAL_TAB 0
#define VAL_FILL 1

/*
 * The starting position of a region, and the size of the region in
 * characters, is kept in a region structure.  Used by the region commands.
 */
typedef struct	{
	struct	LINE *r_linep;		/* Origin LINE address. 	*/
	int	r_offset;		/* Origin LINE offset.		*/
	struct	LINE *r_endlinep;	/* Ending LINE address. 	*/
	int	r_endoffset;		/* Ending LINE offset.		*/
	long	r_size; 		/* Length in characters.	*/
}	REGION;

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

