/*	EVAR.H:	Environment and user variable definitions
		for MicroEMACS (and now vile)

		written 1986 by Daniel Lawrence
*/

/*
 * $Log: evar.h,v $
 * Revision 1.7  1991/12/24 09:18:47  pgf
 * added current/change directory support  (Dave Lemke's changes)
 *
 * Revision 1.6  1991/11/13  20:09:27  pgf
 * X11 changes, from dave lemke
 *
 * Revision 1.5  1991/11/04  14:18:36  pgf
 * not so many user vars (255 -> 10) and
 * made defines and string lists match for the internal "env" vars
 *
 * Revision 1.4  1991/10/28  14:20:29  pgf
 * removed obsoletes, renumbered
 *
 * Revision 1.3  1991/08/07  11:51:32  pgf
 * added RCS log entries
 *
 * revision 1.2	locked by: pgf;
 * date: 1990/10/01 11:05:29;
 * change name to vile
 * ----------------------------
 * revision 1.1
 * date: 1990/09/21 10:25:13;
 * initial vile RCS revision
 */

#if !SMALLER

/*	structure to hold user variables and their definitions	*/

typedef struct UVAR {
	char u_name[NVSIZE + 1];		/* name of user variable */
	char *u_value;				/* value (string) */
} UVAR;

/*	current user variables (This structure will probably change)	*/

#define	MAXVARS		10 	/* was 255 */

UVAR uv[MAXVARS];	/* user variables */

/*	list of recognized environment variables	*/

char *envars[] = {
	"pagelen",		/* number of lines used by editor */
	"curcol",		/* current column pos of cursor */
	"curline",		/* current line in file */
	"ram",			/* ram in use by malloc */
	"flicker",		/* flicker supression */
	"curwidth",		/* current screen width */
	"cbufname",		/* current buffer name */
	"cfname",		/* current file name */
	"sres",			/* current screen resolution */
	"debug",		/* macro debugging */
	"status",		/* returns the status of the last command */
	"palette",		/* current palette string */
	"lastkey",		/* last keyboard char struck */
	"curchar",		/* current character under the cursor */
	"discmd",		/* display commands on command line */
	"version",		/* current version number */
	"progname",		/* returns current prog name - "vile" */
	"seed",			/* current random number seed */
	"disinp",		/* display command line input characters */
	"wline",		/* # of lines in current window */
	"cwline",		/* current screen line in window */
	"target",		/* target for line moves */
	"search",		/* search pattern */
	"replace",		/* replacement pattern */
	"match",		/* last matched magic pattern */
	"kill",			/* kill buffer (read only) */
	"cmode",		/* mode of current buffer */
	"tpause",		/* length to pause for paren matching */
	"pending",		/* type ahead pending flag */
	"lwidth",		/* width of current line */
	"line",			/* text of current line */
	"directory",		/* current directory */
#if X11
	"font",
#endif
};

#define	NEVARS	sizeof(envars) / sizeof(char *)

/* 	and its preprocesor definitions		*/

#define	EVPAGELEN	0
#define	EVCURCOL	1
#define	EVCURLINE	2
#define	EVRAM		3
#define	EVFLICKER	4
#define	EVCURWIDTH	5
#define	EVCBUFNAME	6
#define	EVCFNAME	7
#define	EVSRES		8
#define	EVDEBUG		9
#define	EVSTATUS	10
#define	EVPALETTE	11
#define	EVLASTKEY	12
#define	EVCURCHAR	13
#define	EVDISCMD	14
#define	EVVERSION	15
#define	EVPROGNAME	16
#define	EVSEED		17
#define	EVDISINP	18
#define	EVWLINE		19
#define EVCWLINE	20
#define	EVTARGET	21
#define	EVSEARCH	22
#define	EVREPLACE	23
#define	EVMATCH		24
#define	EVKILL		25
#define	EVCMODE		26
#define	EVTPAUSE	27
#define	EVPENDING	28
#define	EVLWIDTH	29
#define	EVLINE		30
#define	EVDIR		31
#define	EVFONT		32

/*	list of recognized user functions	*/

typedef struct UFUNC {
	char *f_name;	/* name of function */
	int f_type;	/* 1 = monamic, 2 = dynamic */
} UFUNC;

#define	NILNAMIC	0
#define	MONAMIC		1
#define	DYNAMIC		2
#define	TRINAMIC	3

UFUNC funcs[] = {
	"add", DYNAMIC,		/* add two numbers together */
	"sub", DYNAMIC,		/* subtraction */
	"tim", DYNAMIC,		/* multiplication */
	"div", DYNAMIC,		/* division */
	"mod", DYNAMIC,		/* mod */
	"neg", MONAMIC,		/* negate */
	"cat", DYNAMIC,		/* concatinate string */
	"lef", DYNAMIC,		/* left string(string, len) */
	"rig", DYNAMIC,		/* right string(string, pos) */
	"mid", TRINAMIC,	/* mid string(string, pos, len) */
	"not", MONAMIC,		/* logical not */
	"equ", DYNAMIC,		/* logical equality check */
	"les", DYNAMIC,		/* logical less than */
	"gre", DYNAMIC,		/* logical greater than */
	"seq", DYNAMIC,		/* string logical equality check */
	"sle", DYNAMIC,		/* string logical less than */
	"sgr", DYNAMIC,		/* string logical greater than */
	"ind", MONAMIC,		/* evaluate indirect value */
	"and", DYNAMIC,		/* logical and */
	"or",  DYNAMIC,		/* logical or */
	"len", MONAMIC,		/* string length */
	"upp", MONAMIC,		/* uppercase string */
	"low", MONAMIC,		/* lower case string */
	"tru", MONAMIC,		/* Truth of the universe logical test */
	"asc", MONAMIC,		/* char to integer conversion */
	"chr", MONAMIC,		/* integer to char conversion */
	"gtk", NILNAMIC,	/* get 1 charater */
	"rnd", MONAMIC,		/* get a random number */
	"abs", MONAMIC,		/* absolute value of a number */
	"sin", DYNAMIC,		/* find the index of one string in another */
	"env", MONAMIC,		/* retrieve a system environment var */
	"bin", MONAMIC,		/* loopup what function name is bound to a key */
};

#define	NFUNCS	sizeof(funcs) / sizeof(UFUNC)

/* 	and its preprocesor definitions		*/

#define	UFADD		0
#define	UFSUB		1
#define	UFTIMES		2
#define	UFDIV		3
#define	UFMOD		4
#define	UFNEG		5
#define	UFCAT		6
#define	UFLEFT		7
#define	UFRIGHT		8
#define	UFMID		9
#define	UFNOT		10
#define	UFEQUAL		11
#define	UFLESS		12
#define	UFGREATER	13
#define	UFSEQUAL	14
#define	UFSLESS		15
#define	UFSGREAT	16
#define	UFIND		17
#define	UFAND		18
#define	UFOR		19
#define	UFLENGTH	20
#define	UFUPPER		21
#define	UFLOWER		22
#define	UFTRUTH		23
#define	UFASCII		24
#define	UFCHR		25
#define	UFGTKEY		26
#define	UFRND		27
#define	UFABS		28
#define	UFSINDEX	29
#define	UFENV		30
#define	UFBIND		31

#endif
