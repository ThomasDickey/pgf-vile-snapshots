/*
 * The routines in this file provide support for the IBM-PC and other
 * compatible terminals. It goes directly to the graphics RAM to do
 * screen output. It compiles into nothing if not an IBM-PC driver
 *
 * Supported monitor cards include
 *	CGA, MONO, EGA, VGA.
 *
 * Modified by Pete Ruczynski (pjr) for auto-sensing and selection of
 * display type.
 *
 * $Log: ibmpc.c,v $
 * Revision 1.16  1993/06/18 15:57:06  pgf
 * tom's 3.49 changes
 *
 * Revision 1.15  1993/05/11  16:22:22  pgf
 * see tom's CHANGES, 3.46
 *
 * Revision 1.14  1993/05/04  17:05:14  pgf
 * see tom's CHANGES, 3.45
 *
 * Revision 1.13  1993/04/28  14:34:11  pgf
 * see CHANGES, 3.44 (tom)
 *
 * Revision 1.12  1993/04/20  12:18:32  pgf
 * see tom's 3.43 CHANGES
 *
 * Revision 1.11  1993/04/02  09:48:48  pgf
 * tom's changes for turbo
 *
 * Revision 1.10  1992/08/20  23:40:48  foxharp
 * typo fixes -- thanks, eric
 *
 * Revision 1.9  1992/07/08  08:23:57  foxharp
 * set screen attributes correctly in ibmeeop()
 *
 * Revision 1.8  1992/07/01  17:06:32  foxharp
 * pgf cleanup (in general I can't leave well enough alone...).  somewhere
 * along the way I made it work properly -- I think the problem was a missing
 * page number in ibmputc() -- but it might have been a badly calculated
 * attribute byte somewhere else...
 *
 * Revision 1.7  1992/06/25  23:00:50  foxharp
 * changes for dos/ibmpc
 *
 * Revision 1.4  1991/09/10  01:19:35  pgf
 * re-tabbed, and moved ESC and BEL to estruct.h
 *
 * Revision 1.3  1991/08/07  12:35:07  pgf
 * added RCS log messages
 *
 * revision 1.2
 * date: 1990/10/01 12:24:47;
 * changed newsize to newscreensize
 *
 * revision 1.1
 * date: 1990/09/21 10:25:27;
 * initial vile RCS revision
 */

#define	termdef	1			/* don't define "term" external */

#include        "estruct.h"
#include        "edef.h"


#if     IBMPC

#define NROW	50			/* Max Screen size.		*/
#define NCOL    80			/* Edit if you want to.         */
#define	MARGIN	8			/* size of minimum margin and	*/
#define	SCRSIZ	64			/* scroll size for extended lines */
#define	NPAUSE	200			/* # times thru update to pause */
#define	SPACE	32			/* space character		*/

#define	SCADC	0xb8000000L		/* CGA address of screen RAM	*/
#define	SCADM	0xb0000000L		/* MONO address of screen RAM	*/
#define SCADE	0xb8000000L		/* EGA address of screen RAM	*/

#define	ColorDisplay()	(dtype != CDMONO && !monochrome)
#define	AttrColor(b,f)	(((ctrans[b] & 7) << 4) | (ctrans[f] & 15))
#define	Black(n)	((n) ? 0 : 7)
#define	White(n)	((n) ? 7 : 0)
#define	AttrMono(f)	(((Black(f) & 7) << 4) | (White(f) & 15))

#define	uchar	unsigned char

static	int	dtype = -1;		/* current display type		*/

	/* scan-line resolution codes */
#define	RES_250	0
#define	RES_350	1
#define	RES_400	2

	/* character-size codes */
#define	C9x16	0	/* the default */
#define	C8x8	1
#define	C8x14  	2
#define	C8x16	4
#define	C7x9	8
#define	C7x16	16

static	struct	{
	char	*name;
	uchar	type;
	uchar	mode;
	uchar	flags;
	uchar	rows;
	uchar	cols;
	} drivers[] = {
		/* the first 4 entries are reserved as synonyms for card-types */
		{"CGA",    CDCGA,	3,	0,	25,  80},
		{"MONO",   CDMONO,	3,	0,	25,  80},
		{"EGA",    CDEGA,	3,	0,	43,  80},
		{"VGA",    CDVGA,	3,	C8x8,	50,  80},
		/* all VGA's */
		{"40x25",  CDVGA,	1,	0,	25,  40},
		{"80x25",  CDVGA,	3,	0,	25,  80},
		{"80x50",  CDVGA,	3,	C8x8,	50,  80},
		/* Paradise VGA (doesn't work yet...) */
#ifdef TOP_2000_VGA
#undef	NROW
#define	NROW	75
#undef	NCOL
#define	NCOL	132
		{"80x30",  CDVGA,	0x12,	C8x16,	30,  80},
		{"100x75", CDVGA,	0x58,	C8x8,	75,  100},
		{"132x25", CDVGA,	0x55,	C7x16,	25,  132},
		{"132x43", CDVGA,	0x54,	C7x9,	43,  132},
#endif
	};

static	long	ScreenAddress[] = {
		SCADC,	/* CDCGA: Color graphics adapter */
		SCADM,	/* CDMONO: Monochrome adapter */
		SCADE,	/* CDEGA: Enhanced graphics adapter */
		SCADE	/* CDVGA: VGA adapter */
	};

unsigned short *scptr[NROW];		/* pointer to screen lines	*/
unsigned short sline[NCOL];		/* screen line image		*/
extern union REGS rg;			/* cpu register for use of DOS calls */

static	int	original_mode	= -1,
		original_cols,		/* width of display		*/
		original_page,		/* display-page (we use 0)	*/
		original_type,		/* one of CDMONO, ... CDVGA	*/
		original_curs,		/* start/stop scan lines	*/
		monochrome	= FALSE;

static	int	egaexist = FALSE;	/* is an EGA card available?	*/

					/* Forward references.          */
extern  void	ibmmove   P((int,int));
extern  void	ibmeeol   P((void));
extern  void	ibmeeop   P((void));
extern  void	ibmbeep   P((void));
extern  void    ibmopen   P((void));
extern	void	ibmrev    P((int));
extern	int	ibmcres   P((char *));
extern	void	ibmclose  P((void));
extern	void	ibmputc   P((int));
extern	void	ibmkopen  P((void));
extern	void	ibmkclose P((void));

#if	COLOR
extern	void	ibmfcol   P((int));
extern	void	ibmbcol   P((int));

int	cfcolor = -1;		/* current forground color */
int	cbcolor = -1;		/* current background color */
int	ctrans[] = {		/* ansi to ibm color translation table */
		0,		/* black	*/
		4,		/* red		*/
		2,		/* green	*/
		14,		/* yellow	*/
		1,		/* blue		*/
		5,		/* magenta	*/
		3,		/* cyan		*/
		15		/* white	*/
	};
#endif
#if	SCROLLCODE
extern	void	ibmscroll P((int,int,int));
#endif

static	void	egaopen   P((void));

static	int	scinit    P((int));
static	int	getboard  P((void));
static	int	scblank   P((void));

int ibmtype;		/* pjr - what to do about screen resolution */

/*
 * Standard terminal interface dispatch table. Most of the fields point into
 * "termio" code.
 */
TERM    term    = {
	NROW-1,
	NROW-1,
	NCOL,
	NCOL,
	MARGIN,
	SCRSIZ,
	NPAUSE,
	ibmopen,
	ibmclose,
	ibmkopen,
	ibmkclose,
	ttgetc,
	ibmputc,
	ttflush,
	ibmmove,
	ibmeeol,
	ibmeeop,
	ibmbeep,
	ibmrev,
	ibmcres,
#if	COLOR
	ibmfcol,
	ibmbcol,
#endif
#if	SCROLLCODE
	ibmscroll
#endif
};

static void
set_display (int mode)
{
	rg.x.ax = mode;
	int86(0x10, &rg, &rg);
}

static void
set_80x25_display (void)
{
	set_display(3);
}

static void
set_8x8_chars(void)
{
	rg.h.ah = 0x11;		/* set char. generator function code */
	rg.h.al = 0x12;		/*  to 8 by 8 double dot ROM         */
	rg.h.bl = 0;		/* block 0                           */
	int86(0x10, &rg, &rg);	/* VIDEO - TEXT-MODE CHARACTER GENERATOR FUNCTIONS */
}

static void
set_cursor(int start_stop)
{
	rg.h.ah = 1;		/* set cursor size function code */
	rg.x.cx = start_stop;	/* turn cursor on code */
	int86(0x10, &rg, &rg);	/* VIDEO - SET TEXT-MODE CURSOR SHAPE */
}

static void
set_vertical_resolution(int code)
{
	rg.h.ah = 0x12;
	rg.h.al = code;
	rg.h.bl = 30;
	int86(0x10, &rg, &rg);	/* VIDEO - SELECT VERTICAL RESOLUTION */

}

/*--------------------------------------------------------------------------*/

#if	COLOR
void
ibmfcol(color)		/* set the current output color */
int color;	/* color to set */
{
	cfcolor = ctrans[color];
}

void
ibmbcol(color)		/* set the current background color */
int color;	/* color to set */
{
	cbcolor = ctrans[color];
}
#endif

void
ibmmove(row, col)
{
	rg.h.ah = 2;		/* set cursor position function code */
	rg.h.dl = col;
	rg.h.dh = row;
	rg.h.bh = 0;		/* set screen page number */
	int86(0x10, &rg, &rg);
}

/* erase to the end of the line */
void
ibmeeol()
{
	int ccol,crow;	/* current column,row for cursor */

	/* find the current cursor position */
	rg.h.ah = 3;		/* read cursor position function code */
	rg.h.bh = 0;		/* current video page */
	int86(0x10, &rg, &rg);

	ccol = rg.h.dl;		/* record current column */
	crow = rg.h.dh;		/* and row */

	scwrite(crow, ccol, term.t_ncol-ccol, NULL, gfcolor, gbcolor);

}

/* put a character at the current position in the current colors */
void
ibmputc(ch)
int ch;
{
	rg.h.ah = 14;		/* write char to screen with current attrs */
	rg.h.al = ch;
#if	COLOR
	if (ColorDisplay())
		rg.h.bl = cfcolor;
	else
#endif
	 rg.h.bl = White(TRUE);
	rg.h.bh = 0;		/* current video page */
	int86(0x10, &rg, &rg);
}

void
ibmeeop()
{
	rg.h.ah = 6;		/* scroll page up function code */
	rg.h.al = 0;		/* # lines to scroll (clear it) */
	rg.x.cx = 0;		/* upper left corner of scroll */
	rg.h.dh = term.t_nrow;  /* lower right corner of scroll */
	rg.h.dl = term.t_ncol - 1;
	rg.h.bh = scblank();
	int86(0x10, &rg, &rg);
}

void
ibmrev(state)		/* change reverse video state */
int state;	/* TRUE = reverse, FALSE = normal */
{
	/* This never gets used under the IBM-PC driver */
}

int
ibmcres(res)	/* change screen resolution */
char *res;	/* resolution to change to */
{
	char	*dst;
	register int i;		/* index */

	/* find the default configuration */
	if (!strcmp(res, "?"))
		return scinit(CDSENSE);

	/* specify a number */
	if ((i = (int)strtol(res, &dst, 0)) >= 0
	 && !*dst) {
		switch (i) {
		case 25:
		case 2:
			res = drivers[CD_25LINE].name;
			break;
		case 43:
		case 4:	/* 43 line mode */
			res = drivers[CDEGA].name;
			break;
		case 50:
		case 5:	/* 50 line mode */
			res = drivers[CDVGA].name;
		}
	}

	for (i = 0; i < SIZEOF(drivers); i++)
		if (strcmp(res, drivers[i].name) == 0)
			return scinit(i);
	return(FALSE);
}

void
spal()	/* reset the palette registers */
{
	/* nothing here now..... */
}

void
ibmbeep()
{
#if	MWC86
	putcnb(BEL);
#else
	bdos(6, BEL, 0);	/* annoying!! */
#endif
}

void
ibmopen()
{
	rg.h.ah = 0xf;
	int86(0x10,&rg, &rg);	/* VIDEO - GET DISPLAY MODE */

	original_cols = rg.h.ah;
	original_mode = rg.h.al;
	original_page = rg.h.bh;
	original_type = getboard();

	rg.h.ah = 3;
	rg.h.bh = 0;
	int86(0x10, &rg, &rg);	/* VIDEO - GET CURSOR POSITION */
	original_curs = rg.x.cx;

	if (!scinit(ibmtype))
		(void)scinit(CDSENSE);
	revexist = TRUE;
	ttopen();
}

void
ibmclose()

{
	set_display(original_mode);
	set_cursor(original_mode <= 3 ? original_curs & 0x707 : original_curs);
}

void
ibmkopen()	/* open the keyboard */
{
}

void
ibmkclose()	/* close the keyboard */
{
}

static int
scinit(n)	/* initialize the screen head pointers */
int n;		/* type of adapter to init for */
{
	union {
		long laddr;	/* long form of address */
		unsigned short *paddr;	/* pointer form of address */
	} addr;
	register int i;
	int	type;
	static	int	last = -1;

	/* if asked...find out what display is connected */
	if (n == CDSENSE)
		n = type = original_type;
	else {
		type = drivers[n].type;
		if (type == CDEGA && !egaexist)
			return(FALSE);
	}

	/* if changing resolution, we need a reset */
	if (last >= 0) {
		if (drivers[last].rows != drivers[n].rows
		 || drivers[last].cols != drivers[n].cols)
			set_80x25_display();
		else if (dtype == type) {
			(void)strcpy(sres, drivers[n].name);
			return(TRUE);
		}
	}

	/* and set up the various parameters as needed */
	if (type == CDEGA)
		egaopen();
	else if (type == CDVGA) {
		set_display(drivers[n].mode);
		set_vertical_resolution(RES_400);	/* if this needed? */
		if (drivers[n].flags & C8x8)
			set_8x8_chars();
	}

	newscreensize(drivers[n].rows, drivers[n].cols);

	/* reset the $sres environment variable */
	(void)strcpy(sres, drivers[n].name);

	if ((type == CDMONO) != (dtype == CDMONO))
		sgarbf = TRUE;
	dtype = type;
	last  = n;

	/* initialize the screen pointer array */
	if (monochrome)
		addr.laddr = ScreenAddress[CDMONO];
	else if (type == CDMONO)
		addr.laddr = ScreenAddress[CDCGA];
	else
		addr.laddr = ScreenAddress[type];

	for (i = 0; i < NROW; i++) {
		scptr[i] = addr.paddr + term.t_ncol * i;
	}
	return(TRUE);
}

/* getboard:	Determine which type of display board is attached.
 *		Current known types include:
 *
 *		CDMONO	Monochrome graphics adapter
 *		CDCGA	Color Graphics Adapter
 *		CDEGA	Extended graphics Adapter
 *		CDVGA	VGA graphics Adapter
 */

int getboard()
{
	monochrome = FALSE;
	egaexist = FALSE;

	/* check for VGA or MCGA */
	rg.x.ax = 0x1a00;
	rg.h.bl = 0x00;
	int86(0x10,&rg, &rg);	/* VIDEO - GET DISPLAY COMBINATION CODE (PS,VGA,MCGA) */

	if (rg.h.al == 0x1a) {	/* function succeeded */
		switch (rg.h.bl) {
		case 0x01:	monochrome = TRUE;
				return (CDMONO);

		case 0x02:	return (CDCGA);

		case 0x05:	monochrome = TRUE;
		case 0x04:	egaexist = TRUE;
				return (CDEGA);

		case 0x07:	monochrome = TRUE;
		case 0x08:	return (CDVGA);

		case 0x0b:	monochrome = TRUE;
		case 0x0a:
		case 0x0c:	return (CDCGA);	/* MCGA */
		}
	}

	/*
	 * Check for MONO board
	 */
	int86(0x11, &rg, &rg);	/* BIOS - GET EQUIPMENT LIST */

	/* Bits 4-5 in ax are:
	 *	00 EGA, VGA or PGA
	 *	01 40x25 color
	 *	10 80x25 color
	 *	11 80x25 monochrome
	 */
	if (((rg.x.ax & 0x30) == 0x30)) {
		monochrome = TRUE;
		return(CDMONO);
	}

	/*
	 * Test if EGA present
	 */
	rg.h.ah = 0x12;
	rg.h.bl = 0x10;
	int86(0x10,&rg, &rg);	/* VIDEO - GET EGA INFO */

	if (rg.h.bl != 0x10) {	/* function succeeded */
		egaexist = TRUE;
		return(CDEGA);
	}

	return (CDCGA);
}

/*
 * Init the computer to work with the EGA (use 43-line mode)
 */
static void
egaopen()
{
	set_80x25_display();
	set_8x8_chars();

	rg.h.ah = 0x12;		/* alternate select function code    */
	rg.h.al = 0;		/* clear AL for no good reason       */
	rg.h.bl = 0x20;		/* alt. print screen routine         */
	int86(0x10, &rg, &rg);	/* VIDEO - SELECT ALTERNATE PRTSCRN  */

	set_cursor(0x0607);
	outp(0x3d4, 10);	/* video bios bug patch */
	outp(0x3d5, 6);
}

void
scwrite(row, col, nchar, outstr, forg, bacg)	/* write a line out*/
int row, col, nchar;	/* row,col of screen to place outstr (len nchar) on */
char *outstr;	/* string to write out (must be term.t_ncol long) */
int forg;	/* foreground color of string to write */
int bacg;	/* background color */
{
	register unsigned short attr;	/* attribute byte mask to place in RAM */
	register unsigned short *lnptr;	/* pointer to the destination line */
	register int i;

	/* build the attribute byte and setup the screen pointer */
#if	COLOR
	if (ColorDisplay())
		attr = AttrColor(bacg,forg);
	else
#endif
	 attr = AttrMono(bacg<forg);
	attr <<= 8;

	if (flickcode && (dtype == CDCGA))
		lnptr = &sline[0];
	else
		lnptr = scptr[row]+col;

	if (outstr) {
		for (i = 0; i < nchar; i++) {
			*lnptr++ = (outstr[i] & 255) | attr;
		}
	} else {
		for (i = 0; i < nchar; i++) {
			*lnptr++ = (SPACE & 255) | attr;
		}
	}

	if (flickcode && (dtype == CDCGA)) {
		/* wait for vertical retrace to be off */
		while ((inp(0x3da) & 8))
			;
		/* and to be back on */
		while ((inp(0x3da) & 8) == 0)
			;
		/* and send the string out */
		movmem(&sline[0], scptr[row], nchar*sizeof(short));
	}
}

/* reads back a line into a VIDEO struct, used in line-update computation */
VIDEO *
scread(vp, row)
VIDEO *vp;
int row;
{
	register int	i;
	static	VIDEO	*mine;

	if (vp == 0) {
		if (mine == 0) {
			mine = typeallocplus(VIDEO, term.t_mcol - 4);
			if (mine == NULL)
			    ExitProgram(BAD(1));
		}
		vp = mine;
	}
	movmem(scptr[row], &sline[0], term.t_ncol*sizeof(short));
	for (i = 0; i < term.t_ncol; i++)
		vp->v_text[i] = sline[i];
	return vp;
}

/* returns attribute for blank/empty space */
static int
scblank()
{
	register int attr;
#if	COLOR
	if (ColorDisplay())
		attr = AttrColor(gbcolor,gfcolor);
	else
#endif
	 attr = AttrMono(TRUE);
	return attr;
}

#if SCROLLCODE
/*
 * Move 'n' lines starting at 'from' to 'to'
 *
 * PRETTIER_SCROLL is prettier but slower -- it scrolls a line at a time
 *	instead of all at once.
 */
void
ibmscroll(from,to,n)
int from, to, n;
{
#if PRETTIER_SCROLL_patch
	if (absol(from-to) > 1) {
		ibmscroll(from, (from < to) ? to-1 : to+1, n);
		if (from < to)
			from = to-1;
		else
			from = to+1;
	}
#endif
	rg.h.ah = 0x06;		/* scroll window up */
	rg.h.al = n;		/* number of lines to scroll */
	rg.h.bh = scblank();	/* attribute to use for line-fill */
	rg.h.ch = min(to,from);	/* upper window row */
	rg.h.cl = 0;		/* left window column */
	rg.h.dh = max(to,from);	/* lower window column */
	rg.h.dl = 0;		/* lower window column */
	int86(0x10, &rg, &rg);
}
#endif	/* SCROLLCODE */

#if	FLABEL
fnclabel(f, n)		/* label a function key */
int f,n;	/* default flag, numeric argument [unused] */
{
	/* on machines with no function keys...don't bother */
	return(TRUE);
}
#endif

#endif	/* IBMPC */
