/*	tcap:	Unix V5, V7 and BS4.2 Termcap video driver
 *		for MicroEMACS
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/tcap.c,v 1.71 1995/08/21 02:42:06 pgf Exp $
 *
 */

#define termdef 1			/* don't define "term" external */

#include	"estruct.h"
#include	"edef.h"

#if DISP_TERMCAP

#define MARGIN	8
#define SCRSIZ	64
#define NPAUSE	10			/* # times thru update to pause */


#define TCAPSLEN 768 
static	char tcapbuf[TCAPSLEN];

static char *UP, PC, *CM, *CE, *CL, *SO, *SE;
static char *TI, *TE, *KS, *KE;
static char *CS, *dl, *al, *DL, *AL, *SF, *SR;

#if OPT_VIDEO_ATTRS
static char *US;	/* underline-start */
static char *UE;	/* underline-end */
static char *ME;
static char *MD;
#endif

#if OPT_FLASH
static char *vb;	/* visible-bell */
#endif

#if OPT_COLOR
/*
 * This implementation is based on the description of SysVr4 curses found in
 * ncurses 1.8.7, which lists the following terminal capabilities:
 *
 * Full name        Terminfo  Type   Termcap Description
 * ---------------- -------   ----   ----    -----------------------------
 * back_color_erase "bce"     bool   "ut"    screen erased with background color
 * max_colors       "colors"  num    "Co"    maximum numbers of colors on screen
 * max_pairs        "pairs"   num    "pa"    maximum number of color-pairs on the screen
 * no_color_video   "ncv"     num    "NC"    video attributes that can't be used with colors
 * orig_pair        "op"      str    "op"    
 * orig_colors      "oc"      str    "oc"
 * initialize_color "initc"   str    "Ic"
 * initialize_pair  "initp"   str    "Ip"
 * set_color_pair   "scp"     str    "sp"
 * set_foreground   "setf"    str    "Sf"
 * set_background   "setb"    str    "Sb"
 * color_names      "colornm" str    "Yw"
 *
 * FIXME: In this version, we don't support color pairs, since the only
 * platform on which it's been tested is Linux, with an IBM-PC compatible
 * display.  Also, the color names are hardcoded.  The termcap must have
 * the following capabilities set:
 *	Co (hardcoded to NCOLORS)
 *	Sf (e.g., "\E[%a+c\036%dm" for Linux)
 *	Sb (e.g., "\E[%a+c\050%dm" for Linux)
 *
 * Using termcap alone, we cannot get "yellow" on IBM-PC, since that's a
 * combination of bold+(fcolor=3).
 */

#define	Num2Color(n) ctrans[(n) & (NCOLORS-1)]
#define NO_COLOR (-1)

static	char	*Sf;
static	char	*Sb;

static	int	ctrans[NCOLORS];
	/* ansi to ibm color translation table */
static	char *	initpalettestr = "0 1 2 3 4 5 6 7";
	/* black, red, green, yellow, blue, magenta, cyan, white   */
/*
 * We don't really _know_ what the default colors are set to, so the initial
 * values of the current_[fb]color are set to an illegal value to force the
 * colors to be set.
 */
static	int	current_fcolor = NO_COLOR;
static	int	current_bcolor = NO_COLOR;

#endif /* OPT_COLOR */

static struct {
    char * capname;
    int    code;
    char * seq;
} keyseqs[] = {
    /* Arrow keys */
    { "ku",	KEY_Up,		NULL },		/* up */
    { "kd",	KEY_Down,	NULL },		/* down */
    { "kr",	KEY_Right,	NULL },		/* right */
    { "kl",	KEY_Left,	NULL },		/* left */
    /* other cursor-movement */
    { "kh",	KEY_Home,	NULL },		/* home */
    { "kH",	KEY_End,	NULL },		/* end */
    /* page scroll */
    { "kN",	KEY_Next,	NULL },		/* next page */
    { "kP",	KEY_Prior,	NULL },		/* previous page */
    /* editing */
    { "kI",	KEY_Insert,	NULL },		/* Insert */
    { "kD",	KEY_Delete,	NULL },		/* Delete */
    { "@0",	KEY_Find,	NULL },		/* Find */
    { "*6",	KEY_Select,	NULL },		/* Select */
    /* command */
    { "%1",	KEY_Help,	NULL },		/* Help */
    /* function keys */
    { "k1",	KEY_F1,		NULL },		/* F1 */
    { "k2",	KEY_F2,		NULL },
    { "k3",	KEY_F3,		NULL },
    { "k4",	KEY_F4,		NULL },
    { "k5",	KEY_F5,		NULL },
    { "k6",	KEY_F6,		NULL },
    { "k7",	KEY_F7,		NULL },
    { "k8",	KEY_F8,		NULL },
    { "k9",	KEY_F9,		NULL },
    { "k;",	KEY_F10,	NULL },		/* F10 */
    { "F1",	KEY_F11,	NULL },		/* F11 */
    { "F2",	KEY_F12,	NULL },		/* F12 */
    { "F3",	KEY_F13,	NULL },		/* F13 */
    { "F4",	KEY_F14,	NULL },
    { "F5",	KEY_F15,	NULL },
    { "F6",	KEY_F16,	NULL },
    { "F7",	KEY_F17,	NULL },
    { "F8",	KEY_F18,	NULL },
    { "F9",	KEY_F19,	NULL },		/* F19 */
    { "FA",	KEY_F20,	NULL }		/* F20 */
};

static int  tcapcres P(( char * ));
static void putnpad P(( char *, int ));
static void putpad P(( char * ));
static void tcapbeep P(( void ));
static void tcapclose P(( void ));
static void tcapeeol P(( void ));
static void tcapeeop P(( void ));
static void tcapkclose P(( void ));
static void tcapkopen P(( void ));
static void tcapmove P(( int, int ));
static void tcapopen P(( void ));
static void tcapscroll_delins P(( int, int, int ));
static void tcapscroll_reg P(( int, int, int ));
static void tcapscrollregion P(( int, int ));

extern char *tgoto P((char *, int, int));
extern int tgetent P((char *, char *));
extern int tgetnum P((char * ));
extern char *tgetstr P((char *, char **));
extern char *tparam P((char *, char *, int, ...));
extern int tputs P((char *, int, void(*_f)(int) ));

#if OPT_COLOR
static void tcapfcol P(( int ));
static void tcapbcol P(( int ));
#endif

#if OPT_VIDEO_ATTRS
static void tcapattr P(( int ));
#else
static void tcaprev  P(( int ));
#endif

TERM term = {
	0,	/* these four values are set dynamically at open time */
	0,
	0,
	0,
	MARGIN,
	SCRSIZ,
	NPAUSE,
	tcapopen,
	tcapclose,
	tcapkopen,
	tcapkclose,
	ttgetc,
	ttputc,
	tttypahead,
	ttflush,
	tcapmove,
	tcapeeol,
	tcapeeop,
	tcapbeep,
#if OPT_VIDEO_ATTRS
	tcapattr,
#else
	tcaprev,
#endif
	tcapcres
#if	OPT_COLOR
	, tcapfcol
	, tcapbcol
#endif
	, NULL		/* set dynamically at open time */
};

#define	XtermPos()	keystroke() - 040

#if OPT_XTERM >= 3
# define XTERM_ENABLE_TRACKING   "\033[?1001h"	/* mouse hilite tracking */
# define XTERM_DISABLE_TRACKING  "\033[?1001l"
#else
# if OPT_XTERM >= 2
#  define XTERM_ENABLE_TRACKING   "\033[?1000h"	/* normal tracking mode */
#  define XTERM_DISABLE_TRACKING  "\033[?1000l"
# else
#  define XTERM_ENABLE_TRACKING   "\033[?9h"	/* X10 compatibility mode */
#  define XTERM_DISABLE_TRACKING  "\033[?9l"
# endif
#endif

static	int	i_am_xterm;
static	int	x_origin = 1,
		y_origin = 1;

static void
tcapopen()
{
	char *t, *p;
	char tcbuf[2048];
	char *tv_stype;
	char err_str[72];
	int i;
	static int already_open = 0;

	if (already_open) 
		return;

	if ((tv_stype = getenv("TERM")) == NULL)
	{
		puts("Environment variable TERM not defined!");
		ExitProgram(BADEXIT);
	}

	if ((tgetent(tcbuf, tv_stype)) != 1)
	{
		(void)lsprintf(err_str, "Unknown terminal type %s!", tv_stype);
		puts(err_str);
		ExitProgram(BADEXIT);
	}

	/* Get screen size from system, or else from termcap.  */
	getscreensize(&term.t_ncol, &term.t_nrow);
 
	if ((term.t_nrow <= 1) && (term.t_nrow=(short)tgetnum("li")) == -1) {
		term.t_nrow = 24;
	}

	if ((term.t_ncol <= 1) &&(term.t_ncol=(short)tgetnum("co")) == -1){
		term.t_ncol = 80;
	}

	/* are we probably an xterm?  */
	p = tcbuf;
	i_am_xterm = FALSE;
	if (strncmp(tv_stype, "xterm", sizeof("xterm") - 1) == 0) {
		i_am_xterm = TRUE;
		x_origin = 0;
		y_origin = 0;
	}
	else
		while (*p && *p != ':') {
			if (*p == 'x' 
			    && strncmp(p, "xterm", sizeof("xterm") - 1) == 0) {
				i_am_xterm = TRUE;
				x_origin = 0;
				y_origin = 0;
				break;
			}
			p++;
		}

	term.t_mrow =  term.t_nrow;
	term.t_mcol =  term.t_ncol;

	p = tcapbuf;
	t = tgetstr("pc", &p);
	if(t)
		PC = *t;

	CL = tgetstr("cl", &p);
	CM = tgetstr("cm", &p);
	CE = tgetstr("ce", &p);
	UP = tgetstr("up", &p);
	SE = tgetstr("se", &p);
	SO = tgetstr("so", &p);
	TI = tgetstr("ti", &p);
	TE = tgetstr("te", &p);
	KS = tgetstr("ks", &p);
	KE = tgetstr("ke", &p);
	if (SO != NULL)
		revexist = TRUE;

	if(CL == NULL || CM == NULL || UP == NULL)
	{
		puts("Incomplete termcap entry\n");
		ExitProgram(BADEXIT);
	}

	if (CE == NULL) 	/* will we be able to use clear to EOL? */
		eolexist = FALSE;
	CS = tgetstr("cs", &p);
	SF = tgetstr("sf", &p);
	SR = tgetstr("sr", &p);
	dl = tgetstr("dl", &p);
	al = tgetstr("al", &p);
	DL = tgetstr("DL", &p);
	AL = tgetstr("AL", &p);
        
	if (!CS || !SR) { /* some xterm's termcap entry is missing entries */
		if (i_am_xterm) {
			if (!CS) CS = "\033[%i%d;%dr";
			if (!SR) SR = "\033[M";
		}
	}

	if (CS && SR) {
		if (SF == NULL) /* assume '\n' scrolls forward */
			SF = "\n";
		term.t_scroll = tcapscroll_reg;
	} else if ((DL && AL) || (dl && al)) {
		term.t_scroll = tcapscroll_delins;
	} else {
		term.t_scroll = NULL;
	}
#if OPT_FLASH
	vb = tgetstr("vb", &p);
#endif
#if	OPT_COLOR
	Sf = tgetstr("Sf", &p);
	Sb = tgetstr("Sb", &p);
	spal(initpalettestr);
#endif
#if OPT_VIDEO_ATTRS
	ME = tgetstr("me", &p);
	MD = tgetstr("md", &p);
	US = tgetstr("us", &p);		/* underline-start */
	UE = tgetstr("ue", &p);		/* underline-end */
	if (US == 0 && UE == 0) {	/* if we don't have underline, do bold */
		US = MD;
		UE = ME;
	}
#endif

	for (i = TABLESIZE(keyseqs); i--; ) {
	    keyseqs[i].seq = tgetstr(keyseqs[i].capname, &p);
	    if (keyseqs[i].seq) {
#define DONT_MAP_DEL 1
#if DONT_MAP_DEL
		/* NetBSD, FreeBSD, etc. have the kD (delete) function key
		    defined as the DEL char.  i don't like this hack, but
		    until we (and we may never) have separate system "map"
		    and "map!" maps, we can't allow this -- DEL has different
		    semantics in insert and command mode, whereas KEY_Delete
		    has the same semantics (whatever they may be) in both. 
		    KEY_Delete is the only non-motion system map, by the
		    way -- so the rest are benign in insert or command
		    mode.  */
		if (strcmp(keyseqs[i].seq,"\177") == 0)
		    continue;
#endif
		addtosysmap(keyseqs[i].seq, (int)strlen(keyseqs[i].seq), 
					keyseqs[i].code);
	    }
	}
#if OPT_XTERM
	addtosysmap("\033[M", 3, KEY_Mouse);
#if OPT_XTERM >= 3
	addtosysmap("\033[t", 3, KEY_text);
	addtosysmap("\033[T", 3, KEY_textInvalid);
#endif
#endif
	        
	if (p >= &tcapbuf[TCAPSLEN])
	{
		puts("Terminal description too big!\n");
		ExitProgram(BADEXIT);
	}
	ttopen();
	already_open = TRUE;
}

static void
tcapclose()
{
#if OPT_VIDEO_ATTRS
	if (ME)	/* end special attributes (including color) */
		putpad(ME);
#endif
	TTmove(term.t_nrow-1, 0);	/* cf: dumbterm.c */
	tcapeeol();
#if OPT_COLOR
	current_fcolor =
	current_bcolor = NO_COLOR;
#endif
}

/*
 * We open or close the keyboard when either of the following are true:
 *	a) we're changing the xterm-mouse setting
 *	b) we're spawning a subprocess (e.g., shell or pipe command)
 */
static	int	keyboard_open = FALSE;

static void
tcapkopen()
{
#if OPT_XTERM
	if (i_am_xterm && global_g_val(GMDXTERM_MOUSE))
		putpad(XTERM_ENABLE_TRACKING);
#endif
	if (!keyboard_open) {
		keyboard_open = TRUE;
		if (TI)
			putnpad(TI, (int)strlen(TI));
		if (KS)
			putpad(KS);
	}
	(void)strcpy(sres, "NORMAL");
}

static void
tcapkclose()
{
#if OPT_XTERM
	if (i_am_xterm && global_g_val(GMDXTERM_MOUSE))
		putpad(XTERM_DISABLE_TRACKING);
#endif
	if (keyboard_open) {
		keyboard_open = FALSE;
		if (TE)
			putnpad(TE, (int)strlen(TE));
		if (KE)
			putpad(KE);
	}
}

static void
tcapmove(row, col)
register int row, col;
{
	putpad(tgoto(CM, col, row));
}

static void
tcapeeol()
{
	putpad(CE);
}

static void
tcapeeop()
{
#if	OPT_COLOR
	tcapfcol(gfcolor);
	tcapbcol(gbcolor);
#endif
	putpad(CL);
}

/*ARGSUSED*/
static int
tcapcres(res)	/* change screen resolution */
char *	res;
{
	return(TRUE);
}


/* move howmany lines starting at from to to */
static void
tcapscroll_reg(from,to,n)
int from, to, n;
{
	int i;
	if (to == from) return;
	if (to < from) {
		tcapscrollregion(to, from + n - 1);
		tcapmove(from + n - 1,0);
		for (i = from - to; i > 0; i--)
			putpad(SF);
	} else { /* from < to */
		tcapscrollregion(from, to + n - 1);
		tcapmove(from,0);
		for (i = to - from; i > 0; i--)
			putpad(SR);
	}
	tcapscrollregion(0, term.t_nrow-1);
}

/* 
OPT_PRETTIER_SCROLL is prettier but slower -- it scrolls 
		a line at a time instead of all at once.
*/

/* move howmany lines starting at from to to */
static void
tcapscroll_delins(from,to,n)
int from, to, n;
{
	int i;
	if (to == from) return;
	if (DL && AL) {
		if (to < from) {
			tcapmove(to,0);
			putpad(tgoto(DL,0,from-to));
			tcapmove(to+n,0);
			putpad(tgoto(AL,0,from-to));
		} else {
			tcapmove(from+n,0);
			putpad(tgoto(DL,0,to-from));
			tcapmove(from,0);
			putpad(tgoto(AL,0,to-from));
		}
	} else { /* must be dl and al */
#if OPT_PRETTIER_SCROLL
		if (absol(from-to) > 1) {
			tcapscroll_delins(from, (from<to) ? to-1:to+1, n);
			if (from < to)
				from = to-1;
			else
				from = to+1;    
		}
#endif
		if (to < from) {
			tcapmove(to,0);
			for (i = from - to; i > 0; i--)
				putpad(dl);
			tcapmove(to+n,0);
			for (i = from - to; i > 0; i--)
				putpad(al);
		} else {
			tcapmove(from+n,0);
			for (i = to - from; i > 0; i--)
				putpad(dl);
			tcapmove(from,0);
			for (i = to - from; i > 0; i--)
				putpad(al);
		}
	}
}

/* cs is set up just like cm, so we use tgoto... */
static void
tcapscrollregion(top,bot)
int top,bot;
{
	putpad(tgoto(CS, bot, top));
}


#if OPT_EVAL || OPT_COLOR
/* ARGSUSED */
void
spal(thePalette)	/* reset the palette registers */
char *thePalette;
{
#if OPT_COLOR
    	/* this is pretty simplistic.  big deal. */
	(void)sscanf(thePalette,"%i %i %i %i %i %i %i %i",
	    	&ctrans[0], &ctrans[1], &ctrans[2], &ctrans[3],
	    	&ctrans[4], &ctrans[5], &ctrans[6], &ctrans[7] );
#endif
}
#endif /* OPT_EVAL */

#if	OPT_COLOR
static void
show_ansi_colors P((void))
{
	char	*t;

	if ((current_fcolor >= 0)
	 && (Sf != 0)
	 && (t = tparam(Sf, (char *)0, 0, Num2Color(current_fcolor))) != 0) {
		putpad(t);
		free(t);
	}
	if ((current_bcolor >= 0)
	 && (Sb != 0)
	 && (t = tparam(Sb, (char *)0, 0, Num2Color(current_bcolor))) != 0) {
		putpad(t);
		free(t);
	}
}
#endif

#if	OPT_COLOR
static void
tcapfcol(color)
int color;
{
	if (color != current_fcolor) {
		current_fcolor = color;
		show_ansi_colors();
	}
}

static void
tcapbcol(color)
int color;
{
	if (color != current_bcolor) {
		current_bcolor = color;
		show_ansi_colors();
	}
}
#endif /* OPT_COLOR */

#if OPT_VIDEO_ATTRS
/*
 * NOTE:
 * On Linux console, the 'me' termcap setting \E[m resets _all_ attributes,
 * including color.  However, if we use 'se' instead, it doesn't clear the
 * boldface.  To compensate, we reset the colors when we put out 'me'.
 *
 * The color logic is disabled for Linux xterm, because the eeop and eeol
 * operations don't seem to propagate the colors (they're left untouched). 
 * Also, setting _any_ attribute seems to clobber the color settings.  (The
 * latter problem could be "fixed" by always emitting a complete escape sequence
 * for all attributes - boldface, reverse, color), but the former breaks the
 * screen optimization logic.
 */
static void
tcapattr(attr)
int attr;
{
	static	struct	{
		char	**start;
		char	**end;
		int	mask;
	} tbl[] = {
		{ &SO, &SE, VASEL|VAREV },
		{ &US, &UE, VAUL },
		{ &US, &UE, VAITAL },
		{ &MD, &ME, VABOLD },
	};
	static	int last;

	attr = VATTRIB(attr);	/* FIXME: color? */

	attr &= ~(VAML|VAMLFOC);

	if (attr != last) {
		register int n;
		register char *s;
		int	diff = attr ^ last;
		int	ends = FALSE;

		/* turn OFF old attributes */
		for (n = 0; n < TABLESIZE(tbl); n++) {
			if ((tbl[n].mask & diff) != 0
			 && (tbl[n].mask & attr) == 0
			 && (s = *(tbl[n].end))  != 0) {
				putpad(s);
				if (s == ME || s == UE) {
#if OPT_COLOR
					int save_fc = current_fcolor;
					int save_bc = current_bcolor;
					current_fcolor =
					current_bcolor = NO_COLOR;
					tcapfcol(save_fc);
					tcapbcol(save_bc);
#endif
					ends = TRUE;
				}
				diff &= ~(tbl[n].mask);
			}
		}

		/* turn ON new attributes */
		for (n = 0; n < TABLESIZE(tbl); n++) {
			if ((tbl[n].mask & diff)  != 0
			 && (tbl[n].mask & attr)  != 0
			 && (s = *(tbl[n].start)) != 0) {
				putpad(s);
				diff &= ~(tbl[n].mask);
			}
		}

		if (SO != 0 && SE != 0) {
			if (ends && (attr & (VAREV|VASEL))) {
				putpad(SO);
			} else if (diff) {	/* we didn't find it */
				putpad(SE);
			}
		}
		last = attr;
	}
}

#else	/* highlighting is a minimum attribute */

static void
tcaprev(state)		/* change reverse video status */
int state;		/* FALSE = normal video, TRUE = reverse video */
{
	static int revstate = -1;
	if (state == revstate)
		return;
	revstate = state;
	if (state) {
		if (SO != NULL)
			putpad(SO);
	} else {
		if (SE != NULL)
			putpad(SE);
	}
}

#endif	/* OPT_VIDEO_ATTRS */

static void
tcapbeep()
{
#if OPT_FLASH
	if (global_g_val(GMDFLASH)
	 && vb != NULL) {
		putpad(vb);
	} else
#endif
	ttputc(BEL);
}

static void
putpad(str)
char	*str;
{
	tputs(str, 1, ttputc);
}

static void
putnpad(str, n)
char	*str;
int n;
{
	tputs(str, n, ttputc);
}



#if OPT_XTERM
/* Finish decoding a mouse-click in an xterm, after the ESC and '[' chars.
 *
 * There are 3 mutually-exclusive xterm mouse-modes (selected here by values of
 * OPT_XTERM):
 *	(1) X10-compatibility (not used here)
 *		Button-press events are received.
 *	(2) normal-tracking
 *		Button-press and button-release events are received.
 *		Button-events have modifiers (e.g., shift, control, meta).
 *	(3) hilite-tracking
 *		Button-press and text-location events are received.
 *		Button-events have modifiers (e.g., shift, control, meta).
 *		Dragging with the mouse produces highlighting.
 *		The text-locations are checked by xterm to ensure validity.
 *
 * NOTE:
 *	The hilite-tracking code is here for testing and (later) use.  Because
 *	we cannot guarantee that we always are decoding escape-sequences when
 *	reading from the terminal, there is the potential for the xterm hanging
 *	when a mouse-dragging operation is begun: it waits for us to specify
 *	the screen locations that limit the highlighting.
 *
 * 	While highlighting, the xterm accepts other characters, but the display
 *	does not appear to be refreshed until highlighting is ended. So (even
 *	if we always capture the beginning of highlighting) we cannot simply
 *	loop here waiting for the end of highlighting.
 *
 *	1993/aug/6 dickey@software.org
 */

static	int	xterm_button P(( int ));

/*ARGSUSED*/
int
mouse_motion(f,n)
int f,n;
{
	return xterm_button('M');
}

#if OPT_XTERM >= 3
/*ARGSUSED*/
int
xterm_mouse_t(f,n)
int f,n;
{
	return xterm_button('t');
}

/*ARGSUSED*/
int
xterm_mouse_T(f,n)
int f,n;
{
	return xterm_button('T');
}
#endif	/* OPT_XTERM >= 3 */

static int
xterm_button(c)
int	c;
{
	WINDOW	*wp;
	int	event;
	int	button;
	int	x;
	int	y;
	int	status;
#if OPT_XTERM >= 3
	int	save_row = ttrow;
	int	save_col = ttcol;
	int	firstrow, lastrow;
	int	startx, endx, mousex;
	int	starty, endy, mousey;
	MARK	save_dot;
	char	temp[NSTRING];
	static	char	*fmt = "\033[%d;%d;%d;%d;%dT";
#endif	/* OPT_XTERM >= 3 */

	if (insertmode)
		return ABORT;

	if ((status = (global_g_val(GMDXTERM_MOUSE))) != 0) {
		beginDisplay;
		switch(c) {
		case 'M':	/* button-event */
			event	= keystroke();
			x	= XtermPos() + x_origin;
			y	= XtermPos() + y_origin;

			button	= (event & 3) + 1;
			TRACE(("M-button event:%d x:%d y:%d\n", event, x, y))
			if (button > 3) {
				endofDisplay;
				return TRUE; /* button up */
			}
			wp = row2window(y-1);
#if OPT_XTERM >= 3
			/* Tell the xterm how to highlight the selection.
			 * It won't do anything else until we do this.
			 */
			if (wp != 0) {
				firstrow = wp->w_toprow + 1;
				lastrow  = mode_row(wp) + 1;
			} else {		/* from message-line */
				firstrow = term.t_nrow ;
				lastrow  = term.t_nrow + 1;
			}
			if (y >= lastrow)	/* don't select modeline */
				y = lastrow - 1;
			(void)lsprintf(temp, fmt, 1, x, y, firstrow, lastrow);
			putpad(temp);
			TTflush();
#endif	/* OPT_XTERM >= 3 */
			/* Set the dot-location if button 1 was pressed in a
			 * window.
			 */
			if (wp != 0
			 && button == 1
			 && !reading_msg_line
			 && setcursor(y-1, x-1)) {
				/*mlerase();*/
				(void)update(TRUE);
				status = TRUE;
			} else if (button <= 3) {
#if OPT_XTERM >= 3
				/* abort the selection */
				(void)lsprintf(temp, fmt, 0, x, y, firstrow, lastrow);
				putpad(temp);
				TTflush();
#endif	/* OPT_XTERM >= 3 */
				status = ABORT;
			} else {
				status = FALSE;
			}
			break;
#if OPT_XTERM >= 3
		case 't':	/* reports valid text-location */
			x = XtermPos();
			y = XtermPos();

			TRACE(("t: x:%d y:%d\n", x, y))
			setwmark(y-1, x-1);
			yankregion();

			movecursor(save_row, save_col);
			/*mlerase();*/
			(void)update(TRUE);
			break;
		case 'T':	/* reports invalid text-location */
			/*
			 * The starting-location returned is not the location
			 * at which the mouse was pressed.  Instead, it is the
			 * top-most location of the selection.  In turn, the
			 * ending-location is the bottom-most location of the
			 * selection.  The mouse-up location is not necessarily
			 * a pointer to valid text.
			 *
			 * This case handles multi-clicking events as well as
			 * selections whose start or end location was not
			 * pointing to text.
			 */
			save_dot = DOT;
			startx = XtermPos();	/* starting-location */
			starty = XtermPos();
			endx   = XtermPos();	/* ending-location */
			endy   = XtermPos();
			mousex = XtermPos();	/* location at mouse-up */
			mousey = XtermPos();

			TRACE(("T: start(%d,%d) end(%d,%d) mouse(%d,%d)\n",
				starty, startx,
				endy,   endx,
				mousey, mousex))
			setcursor(starty - 1, startx - 1);
			setwmark (endy   - 1, endx   - 1);
			if (MK.o != 0 && !is_at_end_of_line(MK))
				MK.o += 1;
			yankregion();

			DOT = save_dot;
			movecursor(save_row, save_col);
			/*mlerase();*/
			(void)update(TRUE);
			break;
#endif /* OPT_XTERM >= 3 */
		default:
			status = FALSE;
		}
		endofDisplay;
	}
	return status;
}
#endif	/* OPT_XTERM */

#endif	/* DISP_TERMCAP */
