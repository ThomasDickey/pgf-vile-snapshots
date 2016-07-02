/*
 * Uses Borland console i/o routines.
 * (Created for OS/2 by Charles Moschel 29-MAR-94.)
 * (modified to be more generic, not os/2 specific, pgf, april '94)
 *
 * Supported monitor cards include
 *	CGA, MONO, EGA, VGA.
 *
 * Note: Visual flashes are not yet supported.
 *
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/borland.c,v 1.17 1996/04/14 23:37:50 pgf Exp $
 *
 */


#define	termdef	1			/* don't define "term" external */

#include        "estruct.h"
#include        "edef.h"

#if SYS_OS2
#define INCL_VIO
#define INCL_NOPMAPI
#include <os2.h>
#undef OPT_COLOR
#endif /* SYS_OS2 */


#if !DISP_BORLAND || DISP_IBMPC
#error misconfigured:  DISP_BORLAND should be defined if using borland.c
#error (and DISP_IBMPC should not be defined)
#endif

#define NROW	50			/* Max Screen size.		*/
#define NCOL    80			/* Edit if you want to.         */
#define	MARGIN	8			/* size of minimum margin and	*/
#define	SCRSIZ	64			/* scroll size for extended lines */
#define	NPAUSE	200			/* # times thru update to pause */
#define	SPACE	32			/* space character		*/

/* We assume that most users have a color display.  */

#include <conio.h>

static  void	borflush  (void);
static  void	bormove   (int,int);
static  void	boreeol   (void);
static  void	boreeop   (void);
static  void	borbeep   (void);
static  void    boropen   (void);
static	void	borrev    (int);
static	int	borcres   (char *);
static	void	borclose  (void);
static	void	borputc   (int);
static	void	borkopen  (void);
static	void	borkclose (void);

#if OPT_COLOR
static	void	borfcol   (int);
static	void	borbcol   (int);
static	void	borspal   (char *);
#endif

#if OPT_ICURSOR
static	void	boricursor(int);
#endif

int	cfcolor = -1;		/* current forground color */
int	cbcolor = -1;		/* current background color */
int	ctrans[NCOLORS];
/* ansi to ibm color translation table */
char *initpalettestr = "0 4 2 14 1 5 3 7";  /* 15 is too bright */
/* black, red, green, yellow, blue, magenta, cyan, white   */

extern	void	borscroll (int,int,int);

static	int	scinit    (int);


static	char	linebuf[128];
static	int	bufpos = 0;

static struct {
	char  *seq;
	int   code;
} keyseqs[] = {
	/* Arrow keys */
	{"\0\110",     KEY_Up},
	{"\0\120",     KEY_Down},
	{"\0\115",     KEY_Right},
	{"\0\113",     KEY_Left},
	/* page scroll */
	{"\0\121",     KEY_Next},
	{"\0\111",     KEY_Prior},
	{"\0\107",     KEY_Home},
	{"\0\117",     KEY_End},
	/* editing */
        {"\0R",        KEY_Insert},
	{"\0\123",     KEY_Delete},
	/* function keys */
        {"\0;",        KEY_F1},
	{"\0<",        KEY_F2},
	{"\0=",        KEY_F3},
	{"\0>",        KEY_F4},
	{"\0?",        KEY_F5},
	{"\0@",        KEY_F6},
	{"\0A",        KEY_F7},
	{"\0B",        KEY_F8},
	{"\0C",        KEY_F9},
        {"\0D",        KEY_F10},
};

int ibmtype;

static int borlastchar = -1;
static int bortttypahead(void);
static int borttgetc(void);

/*
 * Standard terminal interface dispatch table. Most of the fields point into
 * "termio" code.
 */
TERM    term    = {
	NROW,
	NROW,
	NCOL,
	NCOL,
	MARGIN,
	SCRSIZ,
	NPAUSE,
	boropen,
	borclose,
	borkopen,
	borkclose,
	borttgetc,
	borputc,
	bortttypahead,
	borflush,
	bormove,
	boreeol,
	boreeop,
	borbeep,
	borrev,
	borcres,
#if OPT_COLOR
	borfcol,
	borbcol,
	borspal,
#else
	null_t_setfor,
	null_t_setback,
	null_t_setpal,
#endif
	borscroll,
	null_t_pflush,
#if OPT_ICURSOR
	boricursor,
#else
	null_t_icursor,
#endif
	null_t_title,
};

#ifdef OPT_ICURSOR
static void
boricursor(int cmode)
{
	switch (cmode) {
	case -1: _setcursortype( _NOCURSOR);		break;
	case  0: _setcursortype( _NORMALCURSOR);	break;
	case  1: _setcursortype( _SOLIDCURSOR);		break;
	} 
}
#endif

#if OPT_COLOR
static void
borfcol(int color)		/* set the current output color */
{
	cfcolor = ctrans[color];
	textcolor(cfcolor & 15);
}

static void
borbcol(int color)		/* set the current background color */
{
	cbcolor = ctrans[color];
	textbackground(cbcolor & 7);
}

static void
borspal(char *thePalette)	/* reset the palette registers */
{
	borflush();
    	/* this is pretty simplistic.  big deal. */
	(void)sscanf(thePalette,"%i %i %i %i %i %i %i %i",
	    	&ctrans[0], &ctrans[1], &ctrans[2], &ctrans[3],
	    	&ctrans[4], &ctrans[5], &ctrans[6], &ctrans[7] );
}
#endif

static void
borflush(void)
{
	if (bufpos) {
		linebuf[bufpos] = '\0';
		cputs(linebuf);
		bufpos = 0;
	}
}

static void
bormove(int row, int col)
{
	borflush();
	gotoxy(col+1, row+1);
}

/* erase to the end of the line */
static void
boreeol(void)
{
	borflush();
	clreol();		/* pointer to the destination line */
}

/* put a character at the current position in the current colors */
static void
borputc(int ch)
{
	linebuf[bufpos++] = ch;
}


static void
boreeop(void)
{
	int x, y, i;
	struct text_info t;

	borflush();
	x = wherex();
	y = wherey();
	gettextinfo(&t);
	clreol();
	for (i = x + 1; i <= t.screenheight; i++) {
		gotoxy(1, i);
		clreol();
	}
	gotoxy(x, y);
}

static void
borrev(int reverse)		/* change reverse video state */
{
	borflush();
	if (reverse) {
	    textbackground(cfcolor & 7);
	    textcolor(cbcolor & 15);
	} else {
	    textbackground(cbcolor & 7);
	    textcolor(cfcolor & 15);
	}
}

static int
borcres(char *res)	/* change screen resolution */
{
	char	*dst;
	register int i;		/* index */
	int	status = FALSE;

	strcpy(current_res_name, res);
	borflush();
	/* find the default configuration */
	if (!strcmp(res, "?")) {
		status = scinit(-1);
	} else {	/* specify a number */
		if ((i = (int)strtol(res, &dst, 0)) >= 0 && !*dst)
		{
		/* only allow valid row selections */
		/* Are these all valid under dos?  */
		if (i==2)  status=scinit(25); 
		if (i==4)  status=scinit(43);
		if (i==5)  status=scinit(50);
		if (i==6)  status=scinit(60);

		if (i>6 && i<28) 
			status=scinit(25);
			
		if (i>=28 && i<43) 
			status=scinit(28);

		if (i>=43 && i<50) 
			status=scinit(43);

		if (i>=50 && i<60)
			status=scinit(50);

		if (i>=60)
			status=scinit(60);

		}
	}
	sgarbf = TRUE;
	return status;
}


static void
borbeep(void)
{
	putch('\a');
}


static void
boropen(void)
{
	int i;

	set_palette(initpalettestr);
#if OPT_COLOR
	borfcol(gfcolor);
	borbcol(gbcolor);
#endif
	if (!borcres(current_res_name))
		(void)scinit(-1);
	ttopen();
	for (i = TABLESIZE(keyseqs) - 1; i >= 0; i--)
		addtosysmap(keyseqs[i].seq, 2, keyseqs[i].code);
}


static void
borclose(void)
{
	int	current_type = ibmtype;

	borflush();
#ifdef OPT_ICURSOR
	_setcursortype(_NORMALCURSOR);
#endif
	ibmtype = current_type;	/* ...so subsequent TTopen restores us */

	bottomleft();
	TTeeol();
}

static void
borkopen(void)	/* open the keyboard */
{
	/* ms_install(); */
}

static void
borkclose(void)	/* close the keyboard */
{
	/* ms_deinstall(); */
}

static static
int borttgetc(void)
{
	return (borlastchar = ttgetc());
}


/* bortttypahead:  Check to see if any characters are already in the
 * keyboard buffer.In Borland C OS/2 1.5, kbhit doesn't return non-zero for
 * the 2nd part of an extended character, but 1st part is still 0 so use
 * that as indicator as well (why it's saved in borttgetc).
*/
static int
bortttypahead(void)
{

	return (kbhit() != 0 || borlastchar == 0);

}

#if SYS_OS2  		/* all modes are available under OS/2 */
static int
scinit(int rows)	/* initialize the screen head pointers */
{

	/* and set up the various parameters as needed */

	if (rows == -1)
	{
		struct text_info ti;
		gettextinfo(&ti);
		rows = ti.screenheight;
	}

	switch (rows) {

/* these are enum's, and thus cannot easily be checked, ie. #ifdef C80X21 */
		case 21:	/* color C80X21 */
				textmode(C80X21);
				newscreensize(21, term.t_ncol);
				(void)strcpy(sres, "C80X21");
				break;

		default:
		case 25:	/* Color graphics adapter */
				textmode(C80);
				newscreensize(25, term.t_ncol);
				(void)strcpy(sres, "C80");
				break;

		case 28:	/* Enhanced graphics adapter */
				textmode(C80X28);
				newscreensize(28, term.t_ncol);
				(void)strcpy(sres, "C80X28");
				break;

		case 43:	/* Enhanced graphics adapter */
				textmode(C80X43);
				newscreensize(43, term.t_ncol);
				(void)strcpy(sres, "C80X43");
				break;

		case 50:	/* VGA adapter */
				textmode(C80X50);
				newscreensize(50, term.t_ncol);
				(void)strcpy(sres, "C80X50");
				break;

		case 60:	/* Enhanced graphics adapter */
				textmode(C80X60);
				newscreensize(60, term.t_ncol);
				(void)strcpy(sres, "C80X60");
				break;


	}

	ibmtype = rows;

	return(TRUE);
}

#else /* SYS_OS2 */

static int
scinit(int rows)	/* initialize the screen head pointers */
{

	/* and set up the various parameters as needed */

	struct text_info ti;
	int oldrows;

	gettextinfo(&ti);
	oldrows = ti.screenheight;
	if (rows == -1)
		rows = oldrows;

	switch (rows) {

/* DOS has only BW40, C40, BW80, C80, MONO, and C4350 */

		default:
		case 25:	/* Color graphics adapter */
				if (oldrows != 25)
					textmode(C80);
				newscreensize(25, term.t_ncol);
				(void)strcpy(sres, "C80");
				break;

		case 43:
		case 50:
		case 60:
				if (rows != oldrows)
					textmode(C4350);
				gettextinfo(&ti);
				rows = ti.screenheight;
				newscreensize(rows, term.t_ncol);
				sprintf(sres, "C80X%d", rows);
				break;

	}

	ibmtype = rows;

	return(TRUE);
}

#endif /* SYS_OS2 */

/*
 * Move 'n' lines starting at 'from' to 'to'
 *
 * OPT_PRETTIER_SCROLL is prettier but slower -- it scrolls a line at a time
 *	instead of all at once.
 */

/* move howmany lines starting at from to to */
static void
borscroll(int from, int to,int n)
{
	int i;
	struct text_info t;

	borflush();
	if (to == from) return;
#if OPT_PRETTIER_SCROLL
	if (absol(from-to) > 1) {
		borscroll(from, (from<to) ? to-1:to+1, n);
		if (from < to)
			from = to-1;
		else
			from = to+1;    
	}
#endif
	gettextinfo(&t);
	if (to < from) {
		window(1, to + 1, t.screenwidth, from + n);
		gotoxy(1, 1);
		for (i = from - to; i > 0; i--)
			delline();
		gotoxy(1, n + 1);
		for (i = from - to; i > 0; i--)
			insline();
	} else {
		window(1, from + 1, t.screenwidth, to + n);
		gotoxy(1, n + 1);
		for (i = to - from; i > 0; i--)
			delline();
		gotoxy(1, 1);
		for (i = to - from; i > 0; i--)
			insline();
	}
	window(1, 1, t.screenwidth, t.screenheight);
}

/*--------------------------------------------------------------------------*/

