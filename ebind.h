THIS FILE IS NO LONGER USED -- IT IS INCLUDED FOR HISTORICAL PURPOSES
SEE THE mktbls PROGRAM, AND THE FILE cmdtbl
/*	EBIND:		Initial default key to function bindings for
			MicroEMACS 3.7
*/

/*
 * Command table.
 * This table  is *roughly* in ASCII order, left to right across the
 * characters of the command. This explains the funny location of the
 * control-X commands.
 */
KEYTAB  keytab[NBINDS] = {

	{' ',		forwchar},
	{'"',		usekreg},
	{'$',		gotoeol},
#if	CFENCE
	{'%',			getfence},
#endif
	{'*',		togglelistbuffers},
	{'+',		forwbline},
	{',',		rev_csrch},
	{'-',		backbline},
	{'.',		dotcmdplay},
	{'/',		forwsearch},
	{'0',		gotobol},
	{':',		namedcmd},
	{';',		rep_csrch},
	{'<',		operlshift},
	{'>',		operrshift},
	{'?',		backsearch},
	{'A',		appendeol},
	{'B',		backword},
	{'C',		chgtoeol},
	{'D',		deltoeol},
	{'E',		forwendw},
	{'F',		bcsrch},
	{'G',		gotoline},
	{'H',		gotobos},
	{'I',		insertbol},
	{'J',		join},
	{'K',		unarg},
	{'L',		gotoeos},
	{'M',		gotomos},
	{'N',		revsearch},
	{'O',		openup},
	{'P',		putbefore},
	{'Q',		quit},
	{'R',		overwrite},
	{'S',		chgline},
	{'T',		bcsrch_to},
	{'U',		lineundo},
	{'V',		enlargewind},
	{'W',		forwword},
	{'X',		backdelchar},
	{'Y',		yankline},
	{'Z',		quickexit},
	{'[',		gotobosec},
	{'\'',		golinenmmark},
	{']',		gotoeosec},
	{'^',		firstnonwhite},
	{'_',		histbuff},
	{'`',		goexactnmmark},
	{'a',		append},
	{'b',		backviword},
	{'c',		operchg},
	{'d',		operdel},
	{'e',		forwviendw},
	{'f',		fcsrch},
	{'g',		operqreplace},
	{'h',		backchar},
	{'i',		insert},
	{'j',		forwline},
	{'k',		backline},
	{'l',		forwchar},
	{'m',		setnmmark},
	{'n',		consearch},
	{'o',		opendown},
	{'p',		putafter},
	{'q',		opersreplace},
	{'r',		replacechar},
	{'s',		chgchar},
	{'t',		fcsrch_to},
	{'u',		undo},
	{'v',		shrinkwind},
	{'w',		forwviword},
	{'x',		forwdelchar},
	{'y',		operyank},
	{'z',		poswind},
	{LBRACE,	gotobop},
	{'|',		gotocol},
	{RBRACE,	gotoeop},
	{'~',		operflip},
	{tocntrl('['),		esc},
#if TAGS
	{tocntrl(']'),		gototag},
#endif
	{tocntrl('^'),		altbuff},
	{tocntrl('A'),		meta},
	{tocntrl('B'),		backpage},
	/* ctrl C */
	{tocntrl('D'),		forwhpage},
	{tocntrl('E'),		mvdnwind},
	{tocntrl('F'),		forwpage},
	{tocntrl('G'),		showcpos},
	{tocntrl('H'),		backchar},
	/* ctrl I */
	{tocntrl('J'),		forwline},
	{tocntrl('K'),		delwind},
	{tocntrl('L'),		refresh},
	{tocntrl('M'),		forwbline},
	{tocntrl('N'),		nextwind},
	{tocntrl('O'),		onlywind},
	{tocntrl('P'),		prevwind},
	{tocntrl('Q'),		nullproc},
	{tocntrl('R'),		insfile},
	{tocntrl('S'),		nullproc},
	{tocntrl('T'),		splitwind},
	{tocntrl('U'),		backhpage},
	{tocntrl('V'),		quote},
	{tocntrl('W'),		operwrite},
	{tocntrl('X'),		cex},
	{tocntrl('Y'),		mvupwind},
#if BSD
	{tocntrl('Z'),		bktoshell},
#endif

	{META|tocntrl('D'),		scrnextdw},
	{META|tocntrl('E'),		mvdnnxtwind},
	{META|tocntrl('U'),		scrnextup},
	{META|tocntrl('Y'),		mvupnxtwind},
	{META|'H',		help},
	{META|'*',		listbuffers},
	{CTLX|'g',		operlineqreplace},
	{CTLX|'q',		operlinesreplace},

#if	AEDIT
	{META|' ',		detab},
	{META|tocntrl('I'),		entab},
	{META|'T',		trim},
	{META|'O',		deblank},
#endif
#if	WORDPRO
	{META|'F',		fillpara},
	{META|'J',		fillpara},
#endif
	{META|'U',		operupper},
	{META|'L',		operlower},
#if	CRYPT
	{CTLX|'X',		setkey},
#endif
#if	ISRCH
	{CTLX|'?',		risearch},
	{CTLX|'/',		fisearch},
#endif
	{CTLX|'(',		ctlxlp},
	{CTLX|')',		ctlxrp},
	{CTLX|'0',		delwind},
	{CTLX|'1',		onlywind},
	{CTLX|'2',		splitwind},
	{CTLX|'=',		showcpos},
	{CTLX|'c',		operlinechg},
	{CTLX|'d',		operlinedel},
	{CTLX|'e',		ctlxe},
	{CTLX|'f',		setfillcol},
	{CTLX|'h',		help},
	{CTLX|'P',		lineputbefore},
	{CTLX|'p',		lineputafter},
	{CTLX|'t',		settab},
	{CTLX|'y',		operlineyank},
	{CTLX|tocntrl('C'),		quit},
	{CTLX|tocntrl('X'),		finderr},
	{CTLX|'!',		pipecmd},
#if TERMCAP
	{SPEC|'0',		cbuf1},
	{SPEC|'1',		cbuf2},
	{SPEC|'2',		cbuf3},
	{SPEC|'3',		cbuf4},
	{SPEC|'4',		cbuf5},
	{SPEC|'5',		cbuf6},
	{SPEC|'6',		cbuf7},
	{SPEC|'7',		cbuf8},
	{SPEC|'8',		cbuf9},
	{SPEC|'9',		cbuf10},
#endif

#ifdef BEFORE
	{CTLX|'!',		spawn},
	{CTLX|'#',		filter},
	{CTLX|'@',		pipecmd},
#if ! SMALLER
	{CTLX|'a',		setvar},
#endif
	{CTLX|'b',		usebuffer},
	{CTLX|'g',		enlargewind},
	{CTLX|'i',		killbuffer},
	{CTLX|'m',		setmode},
	{CTLX|'n',		filename},
	{CTLX|'w',		resize},
	{CTLX|'x',		nextbuffer},
	{CTLX|'s',		shrinkwind},
	{CTLX|'z',		enlargewind},
	{CTLX|'^',		enlargewind},
	{CTLX|tocntrl('B'),		listbuffers},
	{CTLX|tocntrl('F'),		filefind},
	{CTLX|tocntrl('I'),		insfile},
	{CTLX|tocntrl('L'),		lowerregion},
	{CTLX|tocntrl('M'),		delmode},
	{CTLX|tocntrl('N'),		mvdnwind},
	{CTLX|tocntrl('P'),		mvupwind},
	{CTLX|tocntrl('R'),		fileread},
	{CTLX|tocntrl('S'),		filesave},
	{CTLX|tocntrl('U'),		upperregion},
	{CTLX|tocntrl('V'),		viewfile},
	{CTLX|tocntrl('W'),		filewrite},
	{CTLX|tocntrl('Z'),		shrinkwind},
	/* {tocntrl('C'),		insspace}, */
	{tocntrl('T'),		twiddle},
	{META|' ',		setmark},
	{META|'.',		setmark},
	{META|'!',		reposition},
	{META|'C',		capword},
	{META|'K',		bindkey},
	{META|'L',		lowerword},
	{META|'M',		setgmode},
	{META|'R',		sreplace},
	{META|'U',		upperword},
	{META|tocntrl('K'),		unbindkey},
	{META|tocntrl('L'),		reposition},
	{META|tocntrl('M'),		delgmode},
	{META|tocntrl('N'),		namebuffer},
	{META|tocntrl('R'),		qreplace},
	{META|tocntrl('S'),		newsize},
	{META|tocntrl('T'),		newwidth},
	{META|tocntrl('V'),		scrnextdw},
	{META|tocntrl('Z'),		scrnextup},
#if	WORDPRO
	{META|tocntrl('C'),		wordcount},
	/* {META|tocntrl('W'),		killpara}, */
#endif
#if	PROC
	{META|tocntrl('E'),		execproc},
#endif
#if	APROP
	{META|'A',		apro},
#endif
	/* META|'D',		delfword, */
	/* META|0x7F,              delbword, */
#if	WORDPRO
	{META|'N',		gotoeop},
	{META|'P',		gotobop},
	{META|'Q',		fillpara},
#endif

#endif /* BEFORE */

#if	NeWS
	{SPEC|0,		setcursor},	/* mouse support */
	{SPEC|1,		newsadjustmode},/* quiet mode setting */

/* top fkeys */
	{SPEC|96,		filefind},
	{SPEC|97,		fileread},
	{SPEC|98,		insfile},
	{SPEC|99,		viewfile},
	{SPEC|100,		filesave},
	{SPEC|101,		filewrite},
	{SPEC|102,		filename},
	{SPEC|103,		yankregion},
	{SPEC|104,		unarg},

/* right function keys */
	{SPEC|80,		backpage},
	{SPEC|81,		gotobob},
	{SPEC|82,		risearch},
	{SPEC|83,		forwpage},
	{SPEC|84,		gotoeob},
	{SPEC|85,		fisearch},
	{SPEC|86,		gotobol},
	{SPEC|87,		backline},
	{SPEC|88,		gotoeol},
	{SPEC|89,		backchar},
	{SPEC|90,		setmark},
	{SPEC|91,		forwchar},
	{SPEC|92,		backword},
	{SPEC|93,		forwline},
	{SPEC|94,		forwword},
#endif

#if	MSDOS & (HP150 == 0) & (WANGPC == 0) & (HP110 == 0)
	{SPEC|tocntrl('_'),		forwhunt},
	{SPEC|tocntrl('S'),		backhunt},
	{SPEC|71,		gotobob},
	{SPEC|72,		backline},
	{SPEC|73,		backpage},
	{SPEC|75,		backchar},
	{SPEC|77,		forwchar},
	{SPEC|79,		gotoeob},
	{SPEC|80,		forwline},
	{SPEC|81,		forwpage},
	{SPEC|82,		insspace},
	{SPEC|83,		forwdelchar},
	{SPEC|115,		backword},
	{SPEC|116,		forwword},
#if	WORDPRO
	{SPEC|132,		gotobop},
	{SPEC|118,		gotoeop},
#endif
	{SPEC|84,		cbuf1},
	{SPEC|85,		cbuf2},
	{SPEC|86,		cbuf3},
	{SPEC|87,		cbuf4},
	{SPEC|88,		cbuf5},
	{SPEC|89,		cbuf6},
	{SPEC|90,		cbuf7},
	{SPEC|91,		cbuf8},
	{SPEC|92,		cbuf9},
	{SPEC|93,		cbuf10},
#endif

#if	HP150
	{SPEC|32,		backline},
	{SPEC|33,		forwline},
	{SPEC|35,		backchar},
	{SPEC|34,		forwchar},
	{SPEC|44,		gotobob},
	{SPEC|46,		forwpage},
	{SPEC|47,		backpage},
	{SPEC|82,		nextwind},
	{SPEC|68,		openline},
	{SPEC|69,		deltoeol},
	{SPEC|65,		forwdelchar},
	{SPEC|64,		ctlxe},
	{SPEC|67,		refresh},
	{SPEC|66,		reposition},
	{SPEC|83,		help},
	{SPEC|81,		deskey},
#endif

#if	HP110
	{SPEC|0x4b,		backchar},
	{SPEC|0x4d,		forwchar},
	{SPEC|0x48,		backline},
	{SPEC|0x50,		forwline},
	{SPEC|0x43,		help},
	{SPEC|0x73,		backword},
	{SPEC|0x74,		forwword},
	{SPEC|0x49,		backpage},
	{SPEC|0x51,		forwpage},
	{SPEC|84,		cbuf1},
	{SPEC|85,		cbuf2},
	{SPEC|86,		cbuf3},
	{SPEC|87,		cbuf4},
	{SPEC|88,		cbuf5},
	{SPEC|89,		cbuf6},
	{SPEC|90,		cbuf7},
	{SPEC|91,		cbuf8},
#endif

#if	AMIGA
	{SPEC|'?',		help},
	{SPEC|'A',		backline},
	{SPEC|'B',		forwline},
	{SPEC|'C',		forwchar},
	{SPEC|'D',		backchar},
	{SPEC|'T',		backpage},
	{SPEC|'S',		forwpage},
	{SPEC|'a',		backword},
	{SPEC|'`',		forwword},
	{SPEC|'P',		cbuf1},
	{SPEC|'Q',		cbuf2},
	{SPEC|'R',		cbuf3},
	{SPEC|'S',		cbuf4},
	{SPEC|'T',		cbuf5},
	{SPEC|'U',		cbuf6},
	{SPEC|'V',		cbuf7},
	{SPEC|'W',		cbuf8},
	{SPEC|'X',		cbuf9},
	{SPEC|'Y',		cbuf10},
	{127,			forwdelchar},
#endif

#if	ST520
	{SPEC|'b',		help},
	{SPEC|'H',		backline},
	{SPEC|'P',		forwline},
	{SPEC|'M',		forwchar},
	{SPEC|'K',		backchar},
	{SPEC|'t',		setmark},
	{SPEC|'a',		put},
	{SPEC|'R',		insspace},
	{SPEC|'G',		gotobob},
	{127,			forwdelchar},
	{SPEC|84,		cbuf1},
	{SPEC|85,		cbuf2},
	{SPEC|86,		cbuf3},
	{SPEC|87,		cbuf4},
	{SPEC|88,		cbuf5},
	{SPEC|89,		cbuf6},
	{SPEC|90,		cbuf7},
	{SPEC|91,		cbuf8},
	{SPEC|92,		cbuf9},
	{SPEC|93,		cbuf10},
#endif

#if  WANGPC
	SPEC|0xE0,              quit,           /* Cancel */
	SPEC|0xE1,              help,           /* Help */
	SPEC|0xF1,              help,           /* ^Help */
	SPEC|0xE3,              esc,          /* Print */
	SPEC|0xF3,              esc,          /* ^Print */
	SPEC|0xC0,              backline,       /* North */
	SPEC|0xD0,              gotobob,        /* ^North */
	SPEC|0xC1,              forwchar,       /* East */
	SPEC|0xD1,              gotoeol,        /* ^East */
	SPEC|0xC2,              forwline,       /* South */
	SPEC|0xD2,              gotobop,        /* ^South */
	SPEC|0xC3,              backchar,       /* West */
	SPEC|0xD3,              gotobol,        /* ^West */
	SPEC|0xC4,              esc,          /* Home */
	SPEC|0xD4,              gotobob,        /* ^Home */
	SPEC|0xC5,              filesave,       /* Execute */
	SPEC|0xD5,              esc,          /* ^Execute */
	SPEC|0xC6,              insfile,        /* Insert */
	SPEC|0xD6,              esc,          /* ^Insert */
	SPEC|0xC7,              forwdelchar,        /* Delete */
	SPEC|0xD7,              killregion,     /* ^Delete */
	SPEC|0xC8,              backpage,       /* Previous */
	SPEC|0xD8,              prevwind,       /* ^Previous */
	SPEC|0xC9,              forwpage,       /* Next */
	SPEC|0xD9,              nextwind,       /* ^Next */
	SPEC|0xCB,              esc,          /* Erase */
	SPEC|0xDB,              esc,          /* ^Erase */
	SPEC|0xDC,              esc,          /* ^Tab */
	SPEC|0xCD,              esc,          /* BackTab */
	SPEC|0xDD,              esc,          /* ^BackTab */
	SPEC|0x80,              esc,          /* Indent */
	SPEC|0x90,              esc,          /* ^Indent */
	SPEC|0x81,              esc,          /* Page */
	SPEC|0x91,              esc,          /* ^Page */
	SPEC|0x82,              esc,          /* Center */
	SPEC|0x92,              esc,          /* ^Center */
	SPEC|0x83,              esc,          /* DecTab */
	SPEC|0x93,              esc,          /* ^DecTab */
	SPEC|0x84,              esc,          /* Format */
	SPEC|0x94,              esc,          /* ^Format */
	SPEC|0x85,              esc,          /* Merge */
	SPEC|0x95,              esc,          /* ^Merge */
	SPEC|0x86,              setmark,        /* Note */
	SPEC|0x96,              esc,          /* ^Note */
	SPEC|0x87,              esc,          /* Stop */
	SPEC|0x97,              esc,          /* ^Stop */
	SPEC|0x88,              forwsearch,     /* Srch */
	SPEC|0x98,              backsearch,     /* ^Srch */
	SPEC|0x89,              sreplace,       /* Replac */
	SPEC|0x99,              qreplace,       /* ^Replac */
	SPEC|0x8A,              esc,          /* Copy */
	SPEC|0x9A,              esc,          /* ^Copy */
	SPEC|0x8B,              esc,          /* Move */
	SPEC|0x9B,              esc,          /* ^Move */
	SPEC|0x8C,              namedcmd,       /* Command */
	SPEC|0x9C,              spawn,          /* ^Command */
	SPEC|0x8D,              esc,          /* ^ */
	SPEC|0x9D,              esc,          /* ^^ */
	SPEC|0x8E,              esc,          /* Blank */
	SPEC|0x9E,              esc,	      /* ^Blank */
	SPEC|0x8F,              gotoline,       /* GoTo */
	SPEC|0x9F,              usebuffer,      /* ^GoTo */
#endif
 

	/* special internal bindings */
	SPEC|META|'W',		wrapword,	/* called on word wrap */
	SPEC|META|'C',		nullproc,	/*  every command input */
	SPEC|META|'R',		nullproc,	/*  on file read */
	{0,			NULL}
};

#if RAINBOW

#include "rainbow.h"

/*
 * Mapping table from the LK201 function keys to the internal EMACS character.
 */

short lk_map[][2] = {
	Up_Key,                         tocntrl('P'),
	Down_Key,                       tocntrl('N'),
	Left_Key,                       tocntrl('B'),
	Right_Key,                      tocntrl('F'),
	Shift+Left_Key,                 META+'B',
	Shift+Right_Key,                META+'F',
	Control+Left_Key,               tocntrl('A'),
	Control+Right_Key,              tocntrl('E'),
	Prev_Scr_Key,                   META+'V',
	Next_Scr_Key,                   tocntrl('V'),
	Shift+Up_Key,                   META+'<',
	Shift+Down_Key,                 META+'>',
	Cancel_Key,                     tocntrl('G'),
	Find_Key,                       tocntrl('S'),
	Shift+Find_Key,                 tocntrl('R'),
	Insert_Key,                     tocntrl('Y'),
	Options_Key,                    tocntrl('D'),
	Shift+Options_Key,              META+'D',
	Remove_Key,                     tocntrl('W'),
	Shift+Remove_Key,               META+'W',
	Select_Key,                     tocntrl('@'),
	Shift+Select_Key,               CTLX+tocntrl('X'),
	Interrupt_Key,                  tocntrl('U'),
	Keypad_PF2,                     META+'L',
	Keypad_PF3,                     META+'C',
	Keypad_PF4,                     META+'U',
	Shift+Keypad_PF2,               CTLX+tocntrl('L'),
	Shift+Keypad_PF4,               CTLX+tocntrl('U'),
	Keypad_1,                       CTLX+'1',
	Keypad_2,                       CTLX+'2',
	Do_Key,                         CTLX+'E',
	Keypad_4,                       CTLX+tocntrl('B'),
	Keypad_5,                       CTLX+'B',
	Keypad_6,                       CTLX+'K',
	Resume_Key,                     META+'!',
	Control+Next_Scr_Key,           CTLX+'N',
	Control+Prev_Scr_Key,           CTLX+'P',
	Control+Up_Key,                 CTLX+tocntrl('P'),
	Control+Down_Key,               CTLX+tocntrl('N'),
	Help_Key,                       CTLX+'=',
	Shift+Do_Key,                   CTLX+'(',
	Control+Do_Key,                 CTLX+')',
	Keypad_0,                       CTLX+'Z',
	Shift+Keypad_0,                 CTLX+tocntrl('Z'),
	Main_Scr_Key,                   tocntrl('C'),
	Keypad_Enter,                   CTLX+'!',
	Exit_Key,                       CTLX+tocntrl('C'),
	Shift+Exit_Key,                 tocntrl('Z')
};

#define lk_map_size     (sizeof(lk_map)/2)
#endif

