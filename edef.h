/*	EDEF:		Global variable definitions for
			MicroEMACS 3.9

			written by Dave G. Conroy
			modified by Steve Wilhite, George Jones
			greatly modified by Daniel Lawrence
*/

/* some global function declarations */

char *flook();
char *getctext();
char *fnc2engl();
char *tokval();
#if ! SMALLER
char *gtenv();
char *gtfun();
char *gtusr();
char *itoa();
char *ltos();
char *mklower();
char *mkupper();
#endif
#if ! VMALLOC
char *malloc();
#endif
char *strcat();
char *strcpy();
char *strncpy();
char *token();
char *prc2engl();
CMDFUNC *engl2fnc();
CMDFUNC *kcod2fnc();
int prc2kcod();
BUFFER  *bfind();               /* Lookup a buffer by name      */
WINDOW  *wpopup();              /* Pop up window creation       */
LINE    *lalloc();              /* Allocate a line              */

/* I know this declaration stuff is really ugly, and I probably won't ever
 *	do it again.  promise.  but it _does_ make it easy to add/change
 *	globals.  Too bad about "comma".    -pgf
 */
#ifdef maindef
# define comma ,
# define decl_init(thing,value) thing = value
# define decl_uninit(thing) thing
#else
# define decl_init(thing,value) extern thing
# define decl_uninit(thing) extern thing
#endif

decl_init( char prognam[], "vile");
decl_init( char version[], "version two point two");

decl_init( int tabval, 8);		/* which column tabs are on,
						 always power of two */
decl_init( int tabmask, 7);		/* one less than above. I know it's
						wasteful */
decl_init( int autoindented , -1);	/* how many chars (not cols) indented */
decl_uninit( int isnamedcmd );		/* are we typing a command name */
decl_uninit( short _chartypes_[N_chars] );	/* character types	*/
decl_uninit( int interrupted );		/* interrupt signal?		*/
decl_uninit( int insertmode );		/* are we inserting or overwriting? */
decl_uninit( int lineinput );		/* are we inserting linestyle? */
decl_uninit( int lastkey );		/* last keystoke (tgetc)	*/
decl_uninit( int last1key );		/* last keystoke (kbd_key)	*/
decl_uninit( int lastcmd );		/* last command	(kbd_seq)	*/
decl_uninit( short fulllineregions );   /* regions should be full lines */
decl_uninit( short doingopcmd );        /* operator command in progress */
decl_uninit( short opcmd );             /* what sort of operator?	*/
decl_uninit( CMDFUNC *havemotion );	/* so we can use "oper" routines
					   internally */
decl_uninit( short plinesdone );        /* have we already used pline()? */
decl_uninit( int fillcol );             /* Fill column                  */
decl_uninit( short kbdm[NKBDM] );	/* Macro                        */
decl_uninit( short dotcmdm[NKBDM] );	/* dot commands			*/
decl_uninit( short tmpcmdm[NKBDM] );	/* dot commands, 'til we're sure */
decl_uninit( int currow );              /* Cursor row                   */
decl_uninit( int curcol );              /* Cursor column                */
decl_uninit( WINDOW *curwp );           /* Current window               */
decl_uninit( BUFFER *curbp );           /* Current buffer               */
decl_uninit( WINDOW *wheadp );          /* Head of list of windows      */
decl_uninit( BUFFER *bheadp );          /* Head of list of buffers      */

decl_uninit( char sres[NBUFN] );		/* current screen resolution	*/

decl_uninit( char pat[NPAT] );          /* Search pattern		*/
decl_uninit( char tap[NPAT] );		/* Reversed pattern array.	*/
decl_uninit( char rpat[NPAT] );		/* replacement pattern		*/

/* The variable matchlen holds the length of the matched
 * string - used by the replace functions.
 * The variable patmatch holds the string that satisfies
 * the search command.
 * The variables matchline and matchoff hold the line and
 * offset position of the start of match.
 */
decl_uninit( int matchlen );
decl_uninit( int mlenold );
decl_uninit( char *patmatch );
decl_uninit( LINE *matchline );
decl_uninit( int matchoff );

#if	MAGIC
/*
 * The variable magical determines if there are actual
 * metacharacters in the string - if not, then we don't
 * have to use the slower MAGIC mode search functions.
 */
decl_uninit( short int magical );
decl_uninit( MC	mcpat[NPAT] );		/* the magic pattern		*/
decl_uninit( MC	tapcm[NPAT] );		/* the reversed magic pattern	*/

#endif

/* directive name table:
	This holds the names of all the directives....	*/

#if ! SMALLER
decl_init(char *dname[],
	 { "if" comma "else" comma "endif" comma
	"goto" comma "return" comma "endm" comma
	"while" comma "endwhile" comma "break" comma
	"force" }
	 );
#else
decl_init(char *dname[],
	 { "endm" }
	);
#endif


#if	DEBUGM
/*	vars needed for macro debugging output	*/
/* global string to hold debug line text */
decl_uninit( char outline[NSTRING] );
#endif

#if	NeWS
decl_uninit( int inhibit_update );	/* prevents output to terminal */
#endif

/* initialized global definitions */

decl_init( int curgoal, -1 );           /* column goal			*/
decl_uninit( char *execstr );		/* pointer to string to execute	*/
decl_uninit( char golabel[NPAT] );	/* current line to go to	*/
decl_uninit( int execlevel );		/* execution IF level		*/
decl_init( int	eolexist, TRUE );	/* does clear to EOL exist	*/
decl_uninit( int revexist );		/* does reverse video exist?	*/
decl_uninit( int flickcode );		/* do flicker supression?	*/

/* THE FOLLOWING MODE NAME TABLES MUST CORRESPOND EXACTLY WITH THE #DEFINES
	IN ESTRUCT.H */
decl_init( char	*othermodes[] , {
	"lazy" comma
	"versionctrl" } );

decl_init( int othmode, 0);   /* "other" global modes	*/

decl_init( char	*valuemodes[] , {
	"tabstop" comma
	"fillcol" } );

decl_init( char	*modename[] , {		/* names of modes		*/
	"wrap" comma
	"cmode" comma
	"swrap" comma
	"exact" comma
	"view" comma
	"magic" comma
	"crypt" comma
	"asave" comma
	"list" comma
	"dos" } );

decl_init( char	modecode[], "wcsevmyald" );/* letters to represent modes	*/

decl_init( int gmode, MDSWRAP|MDEXACT|MDMAGIC );/* global editor mode	*/

decl_init( int gfcolor, 7 );		/* global forgrnd color (white)	*/
decl_uninit( int gbcolor );		/* global backgrnd color (black)*/
decl_init( int gasave, 256 );		/* global ASAVE size		*/
decl_init( int gacount, 256 );		/* count until next ASAVE	*/
decl_init( int sgarbf, TRUE );          /* TRUE if screen is garbage	*/
decl_uninit( int mpresf );              /* TRUE if message in last line */
decl_uninit( int clexec	);		/* command line execution flag	*/
decl_uninit( int mstore	);		/* storing text to macro flag	*/
decl_init( int discmd, TRUE );		/* display command flag		*/
decl_init( int disinp, TRUE );		/* display input characters	*/
decl_uninit( struct BUFFER *bstore );	/* buffer to store macro text to*/
decl_uninit( int vtrow );               /* Row location of SW cursor	*/
decl_uninit( int vtcol );               /* Column location of SW cursor */
decl_init( int ttrow, HUGE );           /* Row location of HW cursor	*/
decl_init( int ttcol, HUGE );           /* Column location of HW cursor */
decl_uninit( int lbound	);		/* leftmost column of current line
					   being displayed */
decl_uninit( int taboff	);		/* tab offset for display	*/

decl_init( int metac, tocntrl('A') );	/* current meta character	*/
decl_init( int ctlxc, tocntrl('X') );	/* current control X prefix char */
decl_init( int reptc, 'K' );		/* current universal repeat char */
decl_init( int abortc, tocntrl('[') );	/* ESC: current abort command char */
decl_init( int quotec, tocntrl('V') );	/* quote char during mlreply()	*/
decl_init( int killc, tocntrl('U') );	/* current line kill char	*/
decl_init( int intrc, tocntrl('C') );	/* current interrupt char	*/
decl_init( int backspc, '\b');		/* current backspace char	*/

#if	NeWS
decl_init( char	*cname[], {		/* names of colors		*/
	"WHITE" comma "RED" comma "GREEN" comma "YELLOW" comma "BLUE" comma
	"MAGENTA" comma "CYAN" comma "BLACK"} );
#else
decl_init( char	*cname[], {		/* names of colors		*/
	"BLACK" comma "RED" comma "GREEN" comma "YELLOW" comma "BLUE" comma
	"MAGENTA" comma "CYAN" comma "WHITE"} );
#endif

decl_uninit( KILLREG kbs[NKREGS] );	/* all chars, 1 thru 9, and default */
decl_uninit( short ukb );		/* index of current kbuffs */
decl_uninit( int kregflag );		/* info for pending kill into reg */
decl_uninit( WINDOW *swindow );		/* saved window pointer		*/
decl_uninit( int cryptflag );		/* currently encrypting?	*/
decl_uninit( short *tmpcmdptr );	/* current position in dot cmd buf */
decl_init( short *tmpcmdend, &tmpcmdm[0] );/* ptr to end of the dot cmd */
decl_uninit( short *dotcmdptr );	/* current position in dot cmd buf */
decl_init( short *dotcmdend, &dotcmdm[0] );/* ptr to end of the dot command */
decl_init( int dotcmdmode, RECORD );	/* current dot command mode	*/
decl_uninit( int dotcmdrep );		/* number of repetitions	*/
decl_uninit( short *kbdptr );		/* current position in keyboard buf */
decl_init( short *kbdend, &kbdm[0] );	/* ptr to end of the keyboard */
decl_init( int	kbdmode, STOP );	/* current keyboard macro mode	*/
decl_uninit( int kbdrep );		/* number of repetitions	*/
decl_uninit( int seed );		/* random number seed		*/
decl_uninit( long envram );		/* # of bytes current used malloc */
decl_uninit( int macbug );		/* macro debuging flag		*/
decl_init( char	errorm[], "ERROR" );	/* error literal		*/
decl_init( char	truem[], "TRUE" );	/* true literal			*/
decl_init( char	falsem[], "FALSE" );	/* false litereal		*/
decl_init( int	cmdstatus, TRUE );	/* last command status		*/
decl_uninit( char palstr[49] );		/* palette string		*/
decl_uninit( char *fline );		/* dynamic return line		*/
decl_uninit( int flen );		/* current length of fline	*/

#if FINDERR
decl_init( char febuff[NBUFN], "" );	/* name of buffer to find errors in */
decl_uninit( unsigned newfebuff );	/* is the name new since last time? */
#endif

/* defined in nebind.h and nename.h */
extern NTAB nametbl[];
extern CMDFUNC *asciitbl[];
extern KBIND kbindtbl[];

/* terminal table defined only in TERM.C */

#ifndef	termdef
extern  TERM    term;                   /* Terminal information.        */
#endif


