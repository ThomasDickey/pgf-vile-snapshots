/*	tcap:	Unix V5, V7 and BS4.2 Termcap video driver
 *		for MicroEMACS
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/tcap.c,v 1.60 1995/01/05 15:03:23 pgf Exp $
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
char tcapbuf[TCAPSLEN];
char *UP, PC, *CM, *CE, *CL, *SO, *SE;
char *TI, *TE, *KS, *KE;

char *CS, *dl, *al, *DL, *AL, *SF, *SR;

#if OPT_VIDEO_ATTRS
static char *US;	/* underline-start */
static char *UE;	/* underline-end */
static char *ME;
static char *MD;
#endif

#if OPT_FLASH
char *vb;	/* visible-bell */
#endif

#if OPT_COLOR
static	int	ctrans[NCOLORS];
	/* ansi to ibm color translation table */
static	char *	initpalettestr = "0 1 2 11 4 5 6 7";
	/* black, red, green, yellow, blue, magenta, cyan, white   */
/*
 * We don't really _know_ what the default colors are set to, so the initial
 * values of the current_[fb]color are set to an illegal value to force the
 * colors to be set.
 */
static	int	current_fcolor = -1;
static	int	current_bcolor = -1;

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

extern char *tgoto P((char *, int, int));
extern int tgetent P((char *, char *));
extern int tgetnum P((char * ));
extern char *tgetstr P((char *, char **));
extern int tputs P((char *, int, void(*_f)(int) ));

#if OPT_COLOR
extern void tcapfcol P(( int ));
extern void tcapbcol P(( int ));
#endif

#if OPT_VIDEO_ATTRS
extern void tcapattr P(( int ));
#else
extern void tcaprev  P(( int ));
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

void
tcapopen()
{
	char *t, *p;
	char tcbuf[2048];
	char *tv_stype;
	char err_str[72];
	int i;
	static int already_open = 0;

	if (already_open) 
	{
		if (TI)
			putnpad(TI, (int)strlen(TI));
		if (KS)
			putpad(KS);
		return;
	}

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
	if (strncmp(tv_stype, "xterm", sizeof("xterm") - 1) == 0)
		i_am_xterm = TRUE;
	else
		while (*p && *p != ':') {
			if (*p == 'x' 
			    && strncmp(p, "xterm", sizeof("xterm") - 1) == 0) {
				i_am_xterm = TRUE;
				break;
			}
			p++;
		}

#if BEFORE
#ifdef SIGWINCH
	term.t_mcol = 200;
	term.t_mrow = 200;
#else
	term.t_mrow =  term.t_nrow;
	term.t_mcol =  term.t_ncol;
#endif
#else
	term.t_mrow =  term.t_nrow;
	term.t_mcol =  term.t_ncol;
#endif
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
	spal(initpalettestr);
#endif
#if OPT_VIDEO_ATTRS
	ME = tgetstr("me", &p);
	MD = tgetstr("md", &p);
#if !(IBM_VIDEO && OPT_COLOR)
	US = tgetstr("us", &p);		/* underline-start */
	UE = tgetstr("ue", &p);		/* underline-end */
	if (US == 0 && UE == 0) {	/* if we don't have underline, do bold */
		US = MD;
		UE = ME;
	}
#endif
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
#endif
	        
	if (p >= &tcapbuf[TCAPSLEN])
	{
		puts("Terminal description too big!\n");
		ExitProgram(BADEXIT);
	}
	ttopen();
	if (TI)
		putnpad(TI, (int)strlen(TI));
	if (KS)
		putpad(KS);
	already_open = TRUE;
}

void
tcapclose()
{
#if OPT_VIDEO_ATTRS
	if (ME)	/* end special attributes (including color) */
		putpad(ME);
#endif
	tcapmove(term.t_nrow-1, 0);
	tcapeeol();
#if OPT_COLOR
	current_fcolor =
	current_bcolor = -1;
#endif

	if (TE)
		putnpad(TE, (int)strlen(TE));
	if (KE)
		putpad(KE);
}

void
tcapkopen()
{
#if OPT_XTERM
	if (i_am_xterm && global_g_val(GMDXTERM_MOUSE))
		putpad(XTERM_ENABLE_TRACKING);
#endif
	(void)strcpy(sres, "NORMAL");
}

void
tcapkclose()
{
#if OPT_XTERM
	if (i_am_xterm && global_g_val(GMDXTERM_MOUSE))
		putpad(XTERM_DISABLE_TRACKING);
#endif
}

void
tcapmove(row, col)
register int row, col;
{
	putpad(tgoto(CM, col, row));
}

void
tcapeeol()
{
	putpad(CE);
}

void
tcapeeop()
{
#if	OPT_COLOR
	tcapfcol(gfcolor);
	tcapbcol(gbcolor);
#endif
	putpad(CL);
}

/*ARGSUSED*/
int
tcapcres(res)	/* change screen resolution */
char *	res;
{
	return(TRUE);
}


/* move howmany lines starting at from to to */
void
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
void
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
void
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

#if	OPT_COLOR && IBM_VIDEO
static void
show_ansi_colors P((void))
{
	char	str[20], *s, c = '[';

	s = lsprintf(str, "%c", ESC);
	if (current_fcolor >= 0) {
		s = lsprintf(s, "%c%d;%d",
			c,
			(ctrans[current_fcolor] > 7),		/* bold? */
			30 + (ctrans[current_fcolor] & 7));	/* foreground */
		c = ';';
	}
	if (current_bcolor >= 0) {
		s = lsprintf(s, "%c%d",
			c,
			40 + (ctrans[current_bcolor] & 7));	/* background */
	}
	s = lsprintf(s, "m");
	putpad(str);
}
#endif

#if	OPT_COLOR
void
tcapfcol(color)
int color;
{
	if (color != current_fcolor) {
#if IBM_VIDEO
		current_fcolor = color;
# ifdef linux
		if (i_am_xterm)
			return;
# endif
		show_ansi_colors();
#endif
	}
}

void
tcapbcol(color)
int color;
{
	if (color != current_bcolor) {
#if IBM_VIDEO
		current_bcolor = color;
# ifdef linux
		if (i_am_xterm)
			return;
# endif
		show_ansi_colors();
#endif
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
 * latter problem could be "fixed" by alway emitting a complete escape sequence
 * for all attributes - boldface, reverse, color), but the former breaks the
 * screen optimization logic.
 */
void
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
#if !(IBM_VIDEO && OPT_COLOR)
		{ &US, &UE, VAITAL },
		{ &MD, &ME, VABOLD },
#else
		{ &MD, &ME, VAITAL|VABOLD }, /* treat italics like bold */
#endif
	};
	static	int last;

	attr = VATTRIB(attr);	/* FIXME: color? */

	attr &= ~(VAML|VAMLFOC);

	if (attr != last) {
		register int n;
		register char *s;
		int	diff = attr ^ last;
		int	ends = FALSE;

		for (n = 0; n < TABLESIZE(tbl); n++) {
			if (tbl[n].mask & diff) {
				if (tbl[n].mask & attr) {
					if ((s = *(tbl[n].start)) != 0) {
						putpad(s);
					}
				} else if ((s = *(tbl[n].end)) != 0) {
					putpad(s);
					if (s == ME || s == UE) {
#ifdef linux
						int save_fc = current_fcolor;
						int save_bc = current_bcolor;
						current_fcolor = -1;
						current_bcolor = -1;
						tcapfcol(save_fc);
						tcapbcol(save_bc);
#endif
						ends = TRUE;
					}
				}
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

void
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

void
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

void
putpad(str)
char	*str;
{
	tputs(str, 1, ttputc);
}

void
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

int
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

	if ((status = (i_am_xterm && global_g_val(GMDXTERM_MOUSE))) != 0) {
		beginDisplay;
		switch(c) {
		case 'M':	/* button-event */
			event	= keystroke();
			x	= XtermPos();
			y	= XtermPos();

			button	= (event & 3) + 1;
			if (button > 3)
				return TRUE; /* button up */
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
				mlerase();
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

			setwmark(y-1, x-1);
			yankregion();

			movecursor(save_row, save_col);
			mlerase();
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

			setcursor(starty - 1, startx - 1);
			setwmark (endy   - 1, endx   - 1);
			if (MK.o != 0 && !is_at_end_of_line(MK))
				MK.o += 1;
			yankregion();

			DOT = save_dot;
			movecursor(save_row, save_col);
			mlerase();
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
