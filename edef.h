/*	EDEF:		Global variable definitions for vile
			

			written for MicroEMACS 3.9 by Dave G. Conroy
			modified by Steve Wilhite, George Jones
			greatly modified by Daniel Lawrence
			modified even more than that by Paul Fox.  honest.
*/

/*
 * $Log: edef.h,v $
 * Revision 1.62  1992/07/24 18:20:08  foxharp
 * v. 3.24
 *
 * Revision 1.61  1992/07/15  08:53:12  foxharp
 * added "slash", for UNIX vs. DOS path separators
 *
 * Revision 1.60  1992/07/13  20:08:17  foxharp
 * "terse" is now a boolean mode rather than a variable, and
 * added "tagsrelative" mode
 *
 * Revision 1.59  1992/07/13  09:25:32  foxharp
 * added "usefullpaths", which tells us all filenames are absolute
 *
 * Revision 1.58  1992/07/08  08:48:46  foxharp
 * v. 3.23
 *
 * Revision 1.57  1992/07/07  08:41:33  foxharp
 * v. 3.22
 *
 * Revision 1.56  1992/07/07  08:35:40  foxharp
 * v. 3.21
 *
 * Revision 1.55  1992/07/04  14:31:06  foxharp
 * insert_mode_was is now a global
 *
 * Revision 1.54  1992/06/25  23:00:50  foxharp
 * changes for dos/ibmpc
 *
 * Revision 1.53  1992/06/14  12:40:30  foxharp
 * working on v. 3.20
 *
 * Revision 1.52  1992/06/12  22:23:42  foxharp
 * changes for separate 'comments' r.e. for formatregion
 *
 * Revision 1.51  1992/06/03  22:19:49  foxharp
 * v. 3.19
 *
 * Revision 1.50  1992/06/01  20:35:59  foxharp
 * added "tabinsert" support
 *
 * Revision 1.49  1992/05/27  08:32:57  foxharp
 * v 3.18
 *
 * Revision 1.48  1992/05/25  22:07:45  foxharp
 * v 3.17
 *
 * Revision 1.47  1992/05/25  21:09:01  foxharp
 * func. decls moved to proto.h
 *
 * Revision 1.46  1992/05/16  12:00:31  pgf
 * prototypes/ansi/void-int stuff/microsoftC
 *
 * Revision 1.45  1992/04/29  07:31:56  pgf
 * v 3.16
 *
 * Revision 1.44  1992/04/10  19:54:00  pgf
 * v 3.15
 *
 * Revision 1.43  1992/04/03  07:23:08  pgf
 * v. 3.14
 *
 * Revision 1.42  1992/03/24  07:35:39  pgf
 * since we include string.h, we also put the extern decls for index and rindex
 * here if we need them
 *
 * Revision 1.41  1992/03/23  08:40:19  pgf
 * v 3.13
 *
 * Revision 1.40  1992/03/19  23:42:34  pgf
 * version 3.12
 *
 * Revision 1.39  1992/03/19  23:16:11  pgf
 * vers. 3.11
 *
 * Revision 1.38  1992/03/13  19:49:30  pgf
 * version three point ten
 *
 * Revision 1.37  1992/03/05  09:17:21  pgf
 * added support for new "terse" variable, to control unnecessary messages
 *
 * Revision 1.36  1992/03/03  08:38:20  pgf
 * use string.h instead of our own decl's
 *
 * Revision 1.35  1992/02/17  09:18:32  pgf
 * version 3.9
 *
 * Revision 1.34  1992/02/17  08:59:33  pgf
 * added "showmode", and
 * macros, and dotcmd, kill registers all now hold unsigned chars
 *
 * Revision 1.33  1992/01/10  08:22:58  pgf
 * v. 3.85
 *
 * Revision 1.32  1992/01/10  07:11:20  pgf
 * added shiftwidth
 *
 * Revision 1.31  1992/01/03  23:30:35  pgf
 * added pre_op_dot as a global -- it's the current position at the start
 * of an operator command
 *
 * Revision 1.30  1991/12/30  23:15:04  pgf
 * version 3.8
 *
 * Revision 1.29  1991/12/04  09:23:48  pgf
 * now version three seven
 *
 * Revision 1.28  1991/11/08  13:17:40  pgf
 * lint cleanup (deleted unused's), and added klines/kchars
 *
 * Revision 1.27  1991/11/04  14:23:36  pgf
 * got rid of unused matchlen, matchpos, and mlenold
 *
 * Revision 1.26  1991/10/29  03:06:26  pgf
 * changes for replaying named registers
 *
 * Revision 1.25  1991/10/28  14:23:31  pgf
 * renamed curtabstopval to curtabval
 *
 * Revision 1.24  1991/10/28  01:11:56  pgf
 * version three point six
 *
 * Revision 1.23  1991/10/27  16:09:38  pgf
 * added gregexp, the compiled pattern
 *
 * Revision 1.22  1991/10/26  00:17:07  pgf
 * paragraph, sentence, sectino, and suffix regex values
 *
 * Revision 1.21  1991/10/24  13:05:52  pgf
 * conversion to new regex package -- much faster
 *
 * Revision 1.20  1991/10/22  03:07:34  pgf
 * bumped to version three 5ive
 *
 * Revision 1.19  1991/10/20  23:05:44  pgf
 * declared realloc()
 *
 * Revision 1.18  1991/10/18  10:56:54  pgf
 * modified VALUE structures and lists to make them more easily settable
 *
 * Revision 1.17  1991/10/15  03:08:57  pgf
 * added backspacelimit and taglength
 *
 * Revision 1.16  1991/09/26  13:07:45  pgf
 * moved LIST mode to window vals, and created window vals
 *
 * Revision 1.15  1991/09/19  13:34:30  pgf
 * added short synonyms for mode and value names, and made names more vi-compliant
 *
 * Revision 1.14  1991/08/13  02:47:23  pgf
 * alphabetized VAL_XXX's, and added "showmatch"
 *
 * Revision 1.13  1991/08/07  11:51:32  pgf
 * added RCS log entries
 *
 * revision 1.12
 * date: 1991/08/06 15:08:00;
 * global/local values
 * ----------------------------
 * revision 1.11
 * date: 1991/06/25 19:51:57;
 * massive data structure restructure
 * ----------------------------
 * revision 1.10
 * date: 1991/06/16 17:32:01;
 * added ctabstop, switched over to "values" array (global and local) to
 * hold things like tabstops and fillcol
 * ----------------------------
 * revision 1.9
 * date: 1991/06/06 13:58:06;
 * added auto-indent mode
 * ----------------------------
 * revision 1.8
 * date: 1991/06/03 18:07:34;
 * changed version number
 * to version three
 * ----------------------------
 * revision 1.7
 * date: 1991/06/03 17:34:49;
 * switch from "meta" etc. to "ctla" etc.
 * ----------------------------
 * revision 1.6
 * date: 1991/06/03 10:36:32;
 * added exmode flag
 * ----------------------------
 * revision 1.5
 * date: 1991/05/31 10:36:44;
 * bumped version no. to 2.3, and
 * changed plinesdone flag to more generic "calledbefore"
 * ----------------------------
 * revision 1.4
 * date: 1991/02/21 10:02:45;
 * don't need lbound for display anymore
 * ----------------------------
 * revision 1.3
 * date: 1990/10/03 16:04:07;
 * up'ed version number to 2.2
 * ----------------------------
 * revision 1.2
 * date: 1990/10/03 16:00:47;
 * make backspace work for everyone
 * ----------------------------
 * revision 1.1
 * date: 1990/09/21 10:25:05;
 * initial vile RCS revision
 */

/* I know this declaration stuff is really ugly, and I probably won't ever
 *	do it again.  promise.  but it _does_ make it easy to add/change
 *	globals.  Too bad about "comma".    -pgf
 */
#ifdef realdef
# define comma ,
# define decl_init(thing,value) thing = value
# define decl_uninit(thing) thing
#else
# define decl_init(thing,value) extern thing
# define decl_uninit(thing) extern thing
#endif

decl_init( char prognam[], "vile");
decl_init( char version[], "version three point twenty-four");

decl_init( char slash, '/'); 		/* so DOS can use '\' as path separator */

decl_init( int autoindented , -1);	/* how many chars (not cols) indented */
decl_uninit( int isnamedcmd );		/* are we typing a command name */
decl_uninit( int calledbefore );	/* called before during this command? */
decl_uninit( short _chartypes_[N_chars] );	/* character types	*/
decl_uninit( int interrupted );		/* interrupt signal?		*/
decl_uninit( int insertmode );		/* are we inserting or overwriting? */
decl_uninit( int insert_mode_was );	/* were we (and will we be?)	*/
					/*	inserting or overwriting? */
decl_uninit( int lineinput );		/* are we inserting linestyle? */
decl_uninit( int lastkey );		/* last keystoke (tgetc)	*/
decl_uninit( int last1key );		/* last keystoke (kbd_key)	*/
decl_uninit( int lastcmd );		/* last command	(kbd_seq)	*/
decl_uninit( short fulllineregions );   /* regions should be full lines */
decl_uninit( short doingopcmd );        /* operator command in progress */
decl_uninit( MARK pre_op_dot );		/* current pos. before operator cmd */
decl_uninit( short opcmd );             /* what sort of operator?	*/
decl_uninit( CMDFUNC *havemotion );	/* so we can use "oper" routines
					   internally */
decl_uninit( unsigned char kbdm[KBLOCK] );	/* Macro                        */
decl_uninit( unsigned char dotcmdm[KBLOCK] );	/* dot commands			*/
decl_uninit( unsigned char tmpcmdm[KBLOCK] );	/* dot commands, 'til we're sure */
decl_uninit( int currow );              /* Cursor row                   */
decl_uninit( int curcol );              /* Cursor column                */
decl_uninit( WINDOW *curwp );           /* Current window               */
decl_uninit( BUFFER *curbp );           /* Current buffer               */
decl_uninit( WINDOW *wheadp );          /* Head of list of windows      */
decl_uninit( BUFFER *bheadp );          /* Head of list of buffers      */

decl_uninit( char sres[NBUFN] );		/* current screen resolution	*/

decl_uninit( char pat[NPAT] );          /* Search pattern		*/
decl_uninit( char rpat[NPAT] );		/* replacement pattern		*/

decl_uninit( regexp *gregexp );		/* compiled version of pat */

/* patmatch holds the string that satisfied the search command.  */
decl_uninit( char *patmatch );

decl_uninit( int ignorecase );

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

decl_init( int curgoal, -1 );           /* column goal			*/
decl_uninit( char *execstr );		/* pointer to string to execute	*/
decl_uninit( char golabel[NPAT] );	/* current line to go to	*/
decl_uninit( int execlevel );		/* execution IF level		*/
decl_init( int	eolexist, TRUE );	/* does clear to EOL exist	*/
decl_uninit( int revexist );		/* does reverse video exist?	*/
decl_uninit( int flickcode );		/* do flicker supression?	*/
decl_uninit( int curtabval );		/* current tab width		*/

decl_init( MARK nullmark, { NULL comma 0 } );
#if ! WINMARK
decl_uninit( MARK Mark );		/* the worker mark */
#endif

/* these get their initial values in main.c, in global_val_init() */
decl_uninit( B_VALUES global_b_values );

#ifndef realdef
extern struct VALNAMES b_valuenames[];
extern struct VALNAMES w_valuenames[];
#else
/* THE FOLLOWING MODE NAME TABLES MUST CORRESPOND EXACTLY WITH THE #DEFINES
	IN ESTRUCT.H */
struct VALNAMES b_valuenames[] = {
	{ "autoindent"	comma "ai" comma VALTYPE_BOOL } comma
	{ "autosave"	comma "as" comma VALTYPE_BOOL } comma
	{ "backspacelimit"	comma "bl" comma VALTYPE_BOOL } comma
	{ "cmode"	comma "X"  comma VALTYPE_BOOL } comma
	{ "crypt"	comma "X"  comma VALTYPE_BOOL } comma
	{ "dos"		comma "X"  comma VALTYPE_BOOL } comma
	{ "ignorecase"	comma "ic" comma VALTYPE_BOOL } comma
	{ "magic"	comma "X"  comma VALTYPE_BOOL } comma
	{ "showmatch"	comma "sm" comma VALTYPE_BOOL } comma
	{ "showmode"	comma "smd" comma VALTYPE_BOOL } comma
	{ "tabinsert"	comma "ti" comma VALTYPE_BOOL } comma
	{ "tagsrelative"comma "tr" comma VALTYPE_BOOL } comma
	{ "terse"	comma "X"  comma VALTYPE_BOOL } comma
	{ "view"	comma "X"  comma VALTYPE_BOOL } comma
	{ "wrapscan"	comma "ws" comma VALTYPE_BOOL } comma
	{ "wrapwords"	comma "ww" comma VALTYPE_BOOL } comma

	{ "autosavecnt"	comma "ascnt" comma VALTYPE_INT } comma
	{ "c-tabstop"	comma "cts" comma VALTYPE_INT } comma
	{ "fillcol"	comma "fc" comma VALTYPE_INT } comma
	{ "shiftwidth"	comma "sw" comma VALTYPE_INT } comma
	{ "tabstop"	comma "ts" comma VALTYPE_INT } comma
	{ "taglength"	comma "tl" comma VALTYPE_INT } comma

	{ "cwd"		comma "X"  comma VALTYPE_STRING } comma
	{ "tags"	comma "tag" comma VALTYPE_STRING } comma

	{ "c-suffixes"	comma "csuf" comma VALTYPE_REGEX } comma
	{ "comments"	comma "X" comma VALTYPE_REGEX } comma
	{ "paragraphs"	comma "X" comma VALTYPE_REGEX } comma
	{ "sections"	comma "X" comma VALTYPE_REGEX } comma
	{ "sentences"	comma "X" comma VALTYPE_REGEX } comma

	{ NULL		comma NULL comma VALTYPE_INT }
};

struct VALNAMES w_valuenames[] = {
	{ "list"	comma "li" comma VALTYPE_BOOL } comma

	{ "sideways"	comma "side" comma VALTYPE_INT } comma
	{ "fcolor"	comma "X"  comma VALTYPE_INT } comma
	{ "bcolor"	comma "X"  comma VALTYPE_INT } comma
	{ NULL		comma NULL comma VALTYPE_INT }
};
#endif


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
decl_uninit( int taboff	);		/* tab offset for display	*/

decl_init( int cntl_a, tocntrl('A') );	/* current meta character	*/
decl_init( int cntl_x, tocntrl('X') );	/* current control X prefix char */
decl_init( int reptc, 'K' );		/* current universal repeat char */
decl_init( int abortc, tocntrl('[') );	/* ESC: current abort command char */
decl_init( int quotec, tocntrl('V') );	/* quote char during mlreply()	*/
decl_init( int killc, tocntrl('U') );	/* current line kill char	*/
decl_init( int wkillc, tocntrl('W') );	/* current word kill char	*/
decl_init( int intrc, tocntrl('C') );	/* current interrupt char	*/
decl_init( int suspc, tocntrl('Z') );	/* current suspend char	*/
decl_init( int startc, tocntrl('Q') );	/* current output start char	*/
decl_init( int stopc, tocntrl('S') );	/* current output stop char	*/
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

/* these get their initial values in main.c, in global_val_init() */
decl_uninit( W_VALUES global_w_values );

decl_uninit( KILLREG kbs[NKREGS] );	/* all chars, 1 thru 9, and default */
decl_uninit( short ukb );		/* index of current kbuffs */
decl_uninit( int kregflag );		/* info for pending kill into reg */
decl_uninit( int kchars );		/* how much did we kill? */
decl_uninit( int klines );

decl_uninit( WINDOW *swindow );		/* saved window pointer		*/
#if CRYPT
decl_init( int cryptflag, FALSE );		/* currently encrypting?	*/
#endif
decl_uninit( unsigned char *tmpcmdptr );	/* current position in dot cmd buf */
decl_init( unsigned char *tmpcmdend, &tmpcmdm[0] );/* ptr to end of the dot cmd */
decl_uninit( unsigned char *dotcmdptr );	/* current position in dot cmd buf */
decl_init( unsigned char *dotcmdend, &dotcmdm[0] );/* ptr to end of the dot command */
decl_init( int dotcmdmode, RECORD );	/* current dot command mode	*/
decl_uninit( int dotcmdrep );		/* number of repetitions	*/
decl_uninit( unsigned char *kbdptr );		/* current position in keyboard buf */
decl_init( unsigned char *kbdend, &kbdm[0] );	/* volatile to end of the keyboard */
decl_uninit( unsigned char *kbdlim);		/* perm. ptr to end of the keyboard */
decl_init( int	kbdmode, STOP );	/* current keyboard macro mode	*/
decl_init( int	kbdplayreg, -1 );	/* register currently playing back */
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

/* per-character output function called by dofmt() */
decl_uninit( void (*dfoutfn)() );

#if IBMPC
#if __ZTC__
extern int set43;
#endif	/* __ZTC__ */

extern int ibmtype;
#endif	/* IBMPC */
