/*
 * The routines in this file provide support for the IBM-PC family
 * of machines. It goes directly to the graphics RAM to do screen output.
 *
 * Supported monitor cards include
 *	CGA, MONO, EGA, VGA.
 *
 * Modified by Pete Ruczynski (pjr) for auto-sensing and selection of
 * display type.
 *
 * $Log: ibmpc.c,v $
 * Revision 1.48  1994/03/16 19:20:15  pgf
 * don't OR in the upper bit to the ctrans value.
 *
 * Revision 1.47  1994/03/16  19:01:44  pgf
 * implement palette string variable -- whitespace separated list of
 * seven "industry standard" color constants
 *
 * Revision 1.46  1994/03/11  12:03:40  pgf
 * cleaned up video driver selection -- now ibmcres is simply a string
 * based interface to scinit, which takes a driver number.  we search
 * the table top to bottom, in case there's more than one entry by the
 * same name, for different screen resolutions.
 *
 * Revision 1.45  1994/03/08  12:09:39  pgf
 * changed 'fulllineregions' to 'regionshape'.
 *
 * Revision 1.44  1994/03/07  11:24:14  pgf
 * took out the omnibook specific modes
 *
 * Revision 1.43  1994/02/25  13:09:00  pgf
 * fixes for watcom compiler and get_vga_bios_info
 *
 * Revision 1.42  1994/02/22  11:55:59  pgf
 * CSENSE should map directly to the original display type/mode
 *
 * Revision 1.41  1994/02/22  11:03:15  pgf
 * truncated RCS log for 4.0
 *
 */

#define	termdef	1			/* don't define "term" external */

#include        "estruct.h"
#include        "edef.h"


#if DJGPP
#include <pc.h>
#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define outp(p,v) outportb(p,v)
#define inp(p) inportb(p)
#define far
#include <io.h> /* for setmode() */
#include <fcntl.h> /* for O_BINARY */
#include <dpmi.h> /* for the register struct */
#include <go32.h>
#endif


#define NROW	60			/* Max Screen size.		*/
#define NCOL    80			/* Edit if you want to.         */
#define	MARGIN	8			/* size of minimum margin and	*/
#define	SCRSIZ	64			/* scroll size for extended lines */
#define	NPAUSE	200			/* # times thru update to pause */
#define	SPACE	32			/* space character		*/

#if WATCOM
#define	SCADC	(0xb800 << 4)		/* CGA address of screen RAM	*/
#define	SCADM	(0xb000 << 4)		/* MONO address of screen RAM	*/
#define SCADE	(0xb800 << 4)		/* EGA address of screen RAM	*/
#endif

#if DJGPP
#define FAR_POINTER(s,o) (0xe0000000 + s*16 + o)
#define FP_SEG(a)	((unsigned long)(a) >> 4L)
#define FP_OFF(a)	((unsigned long)(a) & 0x0fL)
#define	SCADC	0xb800			/* CGA address of screen RAM	*/
#define	SCADM	0xb000			/* MONO address of screen RAM	*/
#define SCADE	0xb800			/* EGA address of screen RAM	*/
#endif

#ifndef SCADC
#define	SCADC	0xb8000000L		/* CGA address of screen RAM	*/
#define	SCADM	0xb0000000L		/* MONO address of screen RAM	*/
#define SCADE	0xb8000000L		/* EGA address of screen RAM	*/
#endif

#ifndef FAR_POINTER
#define FAR_POINTER(s,o) (s)
#endif

#ifdef __WATCOMC__
#define	INTX86(a,b,c)		int386(a, b, c)
#define	INTX86X(a,b,c,d)	int386x(a, b, c, d)
#define	_AX_		eax
#define	_BX_		ebx
#define	_CX_		ecx
#define	_DX_		edx
#define	_DI_		edi
#else
#define	INTX86(a,b,c)	int86(a, b, c)
#define	INTX86X(a,b,c,d)	int86x(a, b, c, d)
#define	_AX_		ax
#define	_BX_		bx
#define	_CX_		cx
#define	_DX_		dx
#define	_DI_		di
#endif

#define	ColorDisplay()	(dtype != CDMONO && !monochrome)
#define	AttrColor(b,f)	(((ctrans[b] & 7) << 4) | (ctrans[f] & 15))
#define	Black(n)	((n) ? 0 : 7)
#define	White(n)	((n) ? 7 : 0)
#define	AttrMono(f)	(((Black(f) & 7) << 4) | (White(f) & 15))

#if OPT_MS_MOUSE
	static	int	ms_crsr2inx  P(( int, int ));
	static	void	ms_deinstall P(( void ));
	static	void	ms_hidecrsr  P(( void ));
	static	void	ms_install   P(( void ));
	static	void	ms_setvrange P(( int, int ));
	static	void	ms_showcrsr  P(( void ));
#else
# define ms_deinstall()
# define ms_install()
#endif

static	int	dtype = -1;	/* current display type		*/
#if DJGPP
#define PACKED __attribute__ ((packed))
#else
#define PACKED
#endif

	/* mode-independent VGA-BIOS status information */
typedef	struct {
	UCHAR	video_modes[3] PACKED;	/* 00 Supported video-modes */
	UCHAR	reserved[4] PACKED;	/* 03 */
	UCHAR	text_scanlines PACKED;	/* 07 Number of pixel rows in text mode */
	UCHAR	num_charsets PACKED;	/* 08 Number of character sets that can be displayed */
	UCHAR	num_loadable PACKED;	/* 09 Number of character sets loadable into video RAM */
	UCHAR	capability PACKED;	/* 0A VGA-BIOS capability info */
	UCHAR	more_info PACKED;	/* 0B More VGA-BIOS capability info */
	UCHAR	reserved2[4] PACKED;	/* 0C */
	} static_VGA_info PACKED;

	/* mode-dependent VGA-BIOS status information */
typedef	struct {
	static_VGA_info *static_info PACKED; /* 00 Address of table containing static info */
	UCHAR	code_number PACKED;	/* 04 Code number of current video mode */
	USHORT	num_columns PACKED;	/* 05 Number of displayed screen or pixel cols */
	USHORT	page_length PACKED;	/* 07 Length of display page in video RAM */
	USHORT	curr_page PACKED;	/* 09 Starting address of current display-page in video RAM */
	UCHAR	crsr_pos[8][2] PACKED;	/* 0B Cursor positions in display-pages in col/row order */
	UCHAR	crsr_end PACKED;	/* 1B Ending row of cursor (pixel) row */
	UCHAR	crsr_start PACKED;	/* 1C Starting row of cursor (pixel) row */
	UCHAR	curr_page_num PACKED;	/* 1D Number of current display page */
	USHORT	port_crt PACKED;	/* 1E Port address of the CRT controller address register */
	UCHAR	curr_crtc PACKED;	/* 20 Current contents of CRTC control registers */
	UCHAR	curr_color_sel PACKED;	/* 21 Current color selection register contents */
	UCHAR	num_rows PACKED;	/* 22 Number of screen rows displayed */
	USHORT	char_height PACKED;	/* 23 Height of characters in pixel rows */
	UCHAR	code_active PACKED;	/* 25 Code number of active video adapter */
	UCHAR	code_inactive PACKED;	/* 26 Code number of inactive video adapter */
	USHORT	num_colors PACKED;	/* 27 Number of displayable colors (0=monochrome) */
	UCHAR	num_pages PACKED;	/* 29 Number of screen pages */
	UCHAR	num_pixel_rows PACKED;	/* 2A Number of displayed pixel rows (RES_200, etc.) */
	UCHAR	num_cset0 PACKED;	/* 2B Number of char-table used with chars whose 3rd attr bit is 0 */
	UCHAR	num_cset1 PACKED;	/* 2C Number of char-table used with chars whose 3rd attr bit is 1 */
	UCHAR	misc_info PACKED;	/* 2D Miscellaneous information */
	UCHAR	reserved[3] PACKED;	/* 2E */
	UCHAR	size_of_ram PACKED;	/* 31 Size of available video RAM (0=64k, 1=128k, 2=192k, 3=256k) */
	UCHAR	reserved2[14] PACKED;	/* 32 */
	} dynamic_VGA_info PACKED;	/* length == 64 bytes */

	/* scan-line resolution codes */
#define	RES_200	0
#define	RES_350	1
#define	RES_400	2
#define RES_480 3

	/* character-size codes */
#define	C8x8	0x12
#define	C8x14	0x11
#define	C8x16	0x14

	/* character-size in pixels, for mouse-positioning */
static	int	chr_wide = 8;
static	int	chr_high = 8;

typedef	struct	{
	char	*name;
	UCHAR	type;
	UCHAR	mode;
	UCHAR	vchr;	/* code for setting character-height */
	UCHAR	rows;
	UCHAR	cols;
	UCHAR	vres;	/* required scan-lines, RES_200, ... */
	} DRIVERS;

#define ORIGTYPE  0	/* store original info in this slot.
			   (these values should all (?) get replaced 
			   at open time) */

/* order this table so that more higher resolution entries for the same
 *  name come first.  remember -- synonyms take only 10 bytes, plus the
 *  name itself.
 */
static	DRIVERS drivers[] = {
		{"default",ORIGTYPE,    3,      C8x8,   25,  80, RES_200},
		{"2",      CDVGA,	3,	C8x16,	25,  80, RES_400},
		{"25",     CDVGA,	3,	C8x16,	25,  80, RES_400},
		{"2",      CDCGA,	3,	C8x8,	25,  80, RES_200},
		{"25",     CDCGA,	3,	C8x8,	25,  80, RES_200},
		{"CGA",    CDCGA,	3,	C8x8,	25,  80, RES_200},
		{"MONO",   CDMONO,	3,	C8x8,	25,  80, RES_200},
		{"80x25",  CDVGA,	3,	C8x16,	25,  80, RES_400},
		{"80x28",  CDVGA,	3,	C8x14,  28,  80, RES_400},
		{"EGA",    CDEGA,	3,	C8x8,	43,  80, RES_350},
		{"4",      CDEGA,	3,	C8x8,	43,  80, RES_350},
		{"43",     CDEGA,	3,	C8x8,	43,  80, RES_350},
		{"80x43",  CDVGA,	3,	C8x8,   43,  80, RES_350},
		{"5",      CDVGA,	3,	C8x8,	50,  80, RES_400},
		{"50",     CDVGA,	3,	C8x8,	50,  80, RES_400},
		{"VGA",    CDVGA,	3,	C8x8,	50,  80, RES_400},
		{"80x50",  CDVGA,	3,	C8x8,	50,  80, RES_400},
		{"80x14",  CDVGA,	3,	C8x14,  14,  80, RES_200},
		{"40x12",  CDVGA,	1,	C8x16,	12,  40, RES_200},
		{"40x21",  CDVGA,	1,	C8x16,	21,  40, RES_350},
		{"40x25",  CDVGA,	1,	C8x16,	25,  40, RES_400},
		{"40x28",  CDVGA,	1,	C8x14,  28,  40, RES_400},
		{"40x50",  CDVGA,	1,	C8x8,   50,  40, RES_400},

	};

static	long	ScreenAddress[] = {
		SCADC,	/* CDCGA: Color graphics adapter */
		SCADM,	/* CDMONO: Monochrome adapter */
		SCADE,	/* CDEGA: Enhanced graphics adapter */
		SCADE	/* CDVGA: VGA adapter */
	};

USHORT *scptr[NROW];			/* pointer to screen lines	*/
USHORT *s2ptr[NROW];			/* pointer to page-1 lines	*/
USHORT sline[NCOL];			/* screen line image		*/
extern union REGS rg;			/* cpu register for use of DOS calls */

static	int	ibm_opened,
		original_page,	/* display-page (we use 0)	*/
		allowed_vres,	/* possible scan-lines, 1 bit per value */
		original_curs,	/* start/stop scan lines	*/
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
int	ctrans[NCOLORS];
/* ansi to ibm color translation table */
char *initpalettestr = "0 4 2 14 1 5 3 15";
/* black, red, green, yellow, blue, magenta, cyan, white   */
#endif
#if	SCROLLCODE
extern	void	ibmscroll P((int,int,int));
#endif

static	int	scinit    P((int));
static	int	getboard  P((void));
static	int	scblank   P((void));
static	VIDEO * videoAlloc P(( VIDEO ** ));

#ifdef MUCK_WITH_KBD_RATE
static	void	maxkbdrate   P((void));
#endif

static int current_ibmtype;

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

#if DJGPP
_go32_dpmi_seginfo vgainfo;
#endif

static int
get_vga_bios_info(dynamic_VGA_info *buffer)
{
# if DJGPP
	_go32_dpmi_registers regs;
	vgainfo.size = (sizeof (dynamic_VGA_info)+15) / 16;
	if (_go32_dpmi_allocate_dos_memory(&vgainfo) != 0) {
		fprintf(stderr,"Couldn't allocate vgainfo memory\n");
		exit(1);
	}

	regs.x.ax = 0x1b00;
	regs.x.bx = 0;
	regs.x.ss = regs.x.sp = 0;
	regs.x.es = vgainfo.rm_segment;
	regs.x.di = 0;
	_go32_dpmi_simulate_int(0x10, &regs);

	dosmemget( vgainfo.rm_segment*16, sizeof (dynamic_VGA_info), buffer);

	_go32_dpmi_free_dos_memory(&vgainfo);

	return (regs.h.al == 0x1b);
#else
	struct SREGS segs;

#if WATCOM
	segread(&segs);
#else
	segs.es   = FP_SEG(buffer);
#endif
	rg.x._DI_ = FP_OFF(buffer);
	rg.x._AX_ = 0x1b00;
	rg.x._BX_ = 0;
	INTX86X(0x10, &rg, &rg, &segs); /* Get VGA-BIOS status */

	return (rg.h.al == 0x1b);
#endif
}

static void
set_display (int mode)
{
	rg.h.ah = 0;
	rg.h.al = mode;
	INTX86(0x10, &rg, &rg);
}

static void
set_page (int page)
{
	rg.h.ah = 5;
	rg.h.al = page;
	INTX86(0x10, &rg, &rg);
}

#ifdef MUCK_WITH_KBD_RATE
/*  set the keyboard rate to max */
static void
maxkbdrate (void)
{
	rg.h.ah = 0x3;
	rg.h.al = 0x5;
	rg.h.bh = 0x0;
	rg.h.bl = 0x0;
	INTX86(0x16, &rg, &rg);
}
#endif

static void
set_char_size(int code)
{
	switch (code) {
	case C8x8:	chr_wide = 8;	chr_high = 8;	break;
	case C8x14:	chr_wide = 8;	chr_high = 14;	break;
	case C8x16:	chr_wide = 8;	chr_high = 16;	break;
	default:	return;		/* cannot set this one! */
	}
	rg.h.ah = 0x11;		/* set char. generator function code	*/
	rg.h.al = code;		/*  to specified ROM			*/
	rg.h.bl = 0;		/* Character table 0			*/
	INTX86(0x10, &rg, &rg);	/* VIDEO - TEXT-MODE CHARACTER GENERATOR FUNCTIONS */
}

static void
set_cursor(int start_stop)
{
	rg.h.ah = 1;		/* set cursor size function code */
	rg.x._CX_ = (drivers[ORIGTYPE].mode <= 3) ?
		start_stop & 0x707 : start_stop;
	INTX86(0x10, &rg, &rg);	/* VIDEO - SET TEXT-MODE CURSOR SHAPE */
}

static int
get_cursor(void)
{
	rg.h.ah = 3;
	rg.h.bh = 0;
	INTX86(0x10, &rg, &rg);	/* VIDEO - GET CURSOR POSITION */
	return rg.x._CX_;
}

static void
set_vertical_resolution(int code)
{
	rg.h.ah = 0x12;
	rg.h.al = code;
	rg.h.bl = 0x30;
	INTX86(0x10, &rg, &rg);	/* VIDEO - SELECT VERTICAL RESOLUTION */
	if (code == RES_200)
		delay(50);	/* patch: timing problem? */
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
int row, col;
{
	rg.h.ah = 2;		/* set cursor position function code */
	rg.h.dl = col;
	rg.h.dh = row;
	rg.h.bh = 0;		/* set screen page number */
	INTX86(0x10, &rg, &rg);
}

/* erase to the end of the line */
void
ibmeeol()
{
	int ccol,crow;	/* current column,row for cursor */

	/* find the current cursor position */
	rg.h.ah = 3;		/* read cursor position function code */
	rg.h.bh = 0;		/* current video page */
	INTX86(0x10, &rg, &rg);

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
	INTX86(0x10, &rg, &rg);
}

void
ibmeeop()
{
	rg.h.ah = 6;		/* scroll page up function code */
	rg.h.al = 0;		/* # lines to scroll (clear it) */
	rg.x._CX_ = 0;		/* upper left corner of scroll */
	rg.h.dh = term.t_nrow;  /* lower right corner of scroll */
	rg.h.dl = term.t_ncol - 1;
	rg.h.bh = scblank();
	INTX86(0x10, &rg, &rg);
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
	register int i;		/* index */
	int	status = FALSE;

	if (!res || !strcmp(res, "?")) 	/* find the default configuration */
		res = "default";

	/* try for a name match on all drivers, until we find it
		and succeed or fall off the bottom */
	for (i = 0; i < SIZEOF(drivers); i++) {
		if (strcmp(res, drivers[i].name) == 0) {
			if ((status = scinit(i)) == TRUE) {
				break;
			}
		}
	}
	return status;
}

void
spal(palstr)	/* reset the palette registers */
char *palstr;
{
#if COLOR
    	/* this is pretty simplistic.  big deal. */
	(void)sscanf(palstr,"%i %i %i %i %i %i %i %i",
	    	&ctrans[0], &ctrans[1], &ctrans[2], &ctrans[3],
	    	&ctrans[4], &ctrans[5], &ctrans[6], &ctrans[7] );
#endif

}

#if OPT_MS_MOUSE || OPT_FLASH
extern VIDEO **vscreen;	/* patch: edef.h */

static	int	vp2attr P(( VIDEO *, int ));
static	void	move2nd P(( int, int ));
static	void	copy2nd P(( int ));

static int
vp2attr(vp, inverted)
VIDEO	*vp;
int	inverted;
{
	int	attr;
#if	COLOR
	if (ColorDisplay())
		attr = AttrColor(
			inverted ? vp->v_rfcolor : vp->v_rbcolor,
			inverted ? vp->v_rbcolor : vp->v_rfcolor);
	else
#endif
	 attr = AttrMono(!inverted);
	return (attr << 8);
}

static void
move2nd(row, col)
int	row;
int	col;
{
	rg.h.ah = 0x02;
	rg.h.bh = 1;
	rg.h.dh = row;
	rg.h.dl = col;
	INTX86(0x10, &rg, &rg);
}

static void
copy2nd(inverted)
int	inverted;
{
	WINDOW	*wp;
	VIDEO	*vp;
	USHORT *lp;
	char *tt;
	register int	row, col, attr;

	for_each_window(wp) {
		for (row = wp->w_toprow; row <= mode_row(wp); row++) {
			vp = vscreen[row];
			lp = s2ptr[row];
			tt = vp->v_text;
			attr = vp2attr(vp, inverted ^ (row == mode_row(wp)));
			for (col = 0; col < term.t_ncol; col++) {
				lp[col] = (attr | tt[col]);
			}
		}
	}

	/* fill in the message line */
	lp = s2ptr[term.t_nrow];
	attr = scblank() << 8;
	for (col = 0; col < term.t_ncol; col++)
		lp[col] = attr | SPACE;
	move2nd(ttrow, ttcol);
}
#endif

/*
 * Reading back from display memory does not work properly when using a mouse,
 * because the portion of the display line beginning with the mouse cursor is
 * altered (by the mouse driver, apparently).  Flash the display by building an
 * inverted copy of the screen in the second display-page and toggling
 * momentarily to that copy.
 *
 * Note: there's no window from which to copy the message line.
 */
#if OPT_FLASH
void
flash_display P((void))
{
	copy2nd(TRUE);
	set_page(1);
	catnap(100,FALSE); /* if we don't wait, we cannot see the flash */
	set_page(0);
}
#endif	/* OPT_FLASH */

void
ibmbeep()
{
#if	OPT_FLASH
	if (global_g_val(GMDFLASH) && ibm_opened) {
		flash_display();
		return;
	}
#endif
#if	MWC86
	putcnb(BEL);
#else
	bdos(6, BEL, 0);	/* annoying!! */
#endif
}

void
ibmopen()
{
	register DRIVERS *driver = &drivers[ORIGTYPE];

	if (sizeof(dynamic_VGA_info) != 64) {
		printf("DOS vile build error -- dynamic_VGA_info struct"
			" must be packed (ibmpc.c)\n");
		exit(1);
	}

	rg.h.ah = 0xf;
	INTX86(0x10,&rg, &rg);	/* VIDEO - GET DISPLAY MODE */

	driver->vchr  = C8x8;
	driver->vres  = RES_200;
	driver->rows  = 25;
	driver->cols  = rg.h.ah;
	driver->mode  = rg.h.al;
	original_page = rg.h.bh;
	driver->type  = getboard();
	allowed_vres  = (1<<RES_200);	/* assume only 200 scan-lines */

	if (driver->type == CDVGA) {	/* we can determine original rows */
		dynamic_VGA_info buffer;

		if (get_vga_bios_info(&buffer)) {
#undef TESTIT
#ifdef TESTIT
			printf ("call succeeded\n");
			printf ("static_info is 0x%x\n", buffer.static_info);
			printf ("code_number is 0x%x\n", buffer.code_number);
			printf ("num_columns is 0x%x\n", buffer.num_columns);
			printf ("page_length is 0x%x\n", buffer.page_length);
			printf ("curr_page is 0x%x\n", buffer.curr_page);
			printf ("num_rows is 0x%x\n", buffer.num_rows);
#endif
			switch (buffer.char_height) {
			case 8:		driver->vchr = C8x8;	break;
			case 14:	driver->vchr = C8x14;	break;
			case 16:	driver->vchr = C8x16;	break;
			}
			driver->rows = buffer.num_rows;
#if DJGPP
			{ u_long staticinfop;
			static_VGA_info static_info;
			staticinfop = ((u_long)buffer.static_info & 0xffffL);
			staticinfop += (((u_long)buffer.static_info >> 12) & 
						0xffff0L);
			dosmemget( staticinfop, sizeof(static_info),
					&static_info);
			allowed_vres = static_info.text_scanlines;
			}
#elif WATCOM
			{
			static_VGA_info __far *staticinfop;
			staticinfop = MK_FP (
			((unsigned long)(buffer.static_info) >> 16) & 0xffffL,
			((unsigned long)(buffer.static_info)      ) & 0xffffL
				);
			allowed_vres = staticinfop->text_scanlines;
			}
#else
			allowed_vres = buffer.static_info->text_scanlines;
#endif
			driver->vres = buffer.num_pixel_rows;
		}
	} else if (driver->type == CDEGA) {
		allowed_vres |= (1<<RES_350);
	}

	original_curs = get_cursor();

#ifdef PVGA
	set_display(10);	/* set graphic 640x350 mode */
	rg.x._AX_ = 0x007F;      
	rg.h.bh = 0x01;		/* set non-VGA mode */
	INTX86(0x10,&rg, &rg);
	set_display(7);		/* set Hercule mode */
	current_res_name = "25";
#endif

#if	COLOR
	spal(initpalettestr);
#endif
	if (!ibmcres(current_res_name))
		(void)scinit(ORIGTYPE);
	revexist = TRUE;
	ttopen();

#ifdef MUCK_WITH_KBD_RATE
	maxkbdrate();   /* set the keyboard rate to max */
#endif
	ibm_opened = TRUE;	/* now safe to use 'flash', etc. */
}

void
ibmclose()
{
	int	ctype = current_ibmtype;

	if (current_ibmtype != ORIGTYPE)
		scinit(ORIGTYPE);
	if (original_page != 0)
		set_page(original_page);
	set_cursor(original_curs);

	current_ibmtype = ctype; /* ...so subsequent TTopen restores us */

	dtype = CDMONO;		/* ...force monochrome */
	movecursor(term.t_nrow, 0);
}

void
ibmkopen()	/* open the keyboard */
{
	ms_install();
#if DJGPP
	setmode(0,O_BINARY);
#endif
}

void
ibmkclose()	/* close the keyboard */
{
#if DJGPP
	setmode(0,O_TEXT);
#endif
	ms_deinstall();
}

static int
scinit(newtype)	/* initialize the screen head pointers */
int newtype;		/* type of adapter to init for */
{
	dynamic_VGA_info buffer;
	union {
		long laddr;		/* long form of address */
		USHORT *paddr;		/* pointer form of address */
	} addr;
	long pagesize;
	register DRIVERS *driver;
	register int i;
	int	     type, rows, cols;

	driver = &drivers[newtype];

	/* check to see if we're allow this many scan-lines */
	if ((allowed_vres & (1 << (driver->vres))) == 0)
		return FALSE;

	type = driver->type;
	rows = driver->rows;
	cols = driver->cols;

	/* and set up the various parameters as needed */
	set_vertical_resolution(driver->vres);

	set_display(driver->mode);
	set_vertical_resolution(driver->vres);
	set_char_size(driver->vchr);

	/* reset the original cursor -- it gets changed above somewhere */
	set_cursor(original_curs);

	/*
	 * Install an alternative hardcopy routine which prints as many lines
	 * as are displayed on the screen. The normal BIOS routine always
	 * prints 25 lines.
	 */
	if (rows > 25) {
		rg.h.ah = 0x12;		/* alternate select function code    */
		rg.h.bl = 0x20;		/* alt. print screen routine         */
		INTX86(0x10, &rg, &rg);	/* VIDEO - SELECT ALTERNATE PRTSCRN  */
	}

	if (driver->type == CDEGA) {
		outp(0x3d4, 10);	/* video bios bug patch */
		outp(0x3d5, 6);
	}

	/* reset the $sres environment variable */
	(void)strcpy(sres, driver->name);

	if ((type == CDMONO) != (dtype == CDMONO))
		sgarbf = TRUE;
	dtype = type;
	current_ibmtype = newtype;

	/* initialize the screen pointer array */
	if (monochrome)
		addr.laddr = FAR_POINTER(ScreenAddress[CDMONO],0x0000);
	else if (type == CDMONO)
		addr.laddr = FAR_POINTER(ScreenAddress[CDCGA],0x0000);
	else
		addr.laddr = FAR_POINTER(ScreenAddress[type],0x0000);

	for (i = 0; i < rows; i++)
		scptr[i] = addr.paddr + (cols * i);

#if OPT_FLASH || OPT_MS_MOUSE
	/* Build row-indices for display page #1, to use it in screen flashing
	 * or for mouse highlighting.
	 */
	if ((type == CDVGA) && get_vga_bios_info(&buffer)) {
		/*
		 * Setting page-1 seems to make Window 3.1 "aware" that we're
		 * going to write to that page.  Otherwise, the "flash" mode
		 * doesn't write to the correct address.
		 */
		set_page(1);
		get_vga_bios_info(&buffer);
		pagesize = (long)buffer.page_length; /* also, page-1 offset */
		set_page(0);
	} else {
		/*
		 * This was tested for all of the VGA combinations running
		 * with MSDOS, but isn't general -- dickey@software.org
		 */
		pagesize = (rows * cols);
		switch (cols) {
		case 40:	pagesize += ((4*cols) - 32);	break;
		case 80:	pagesize += ((2*cols) - 32);	break;
		}
		pagesize <<= 1;
	}
	addr.laddr += pagesize;
	for (i = 0; i < rows; i++)
		s2ptr[i] = addr.paddr + (cols * i);
#endif
	/* changing the screen-size forces an update, so we do this last */
	newscreensize(rows, cols);
#if OPT_MS_MOUSE
	if (ms_exists()) {
		ms_deinstall();
		ms_install();
	}
#endif
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
	rg.x._AX_ = 0x1a00;
	rg.h.bl = 0x00;
	INTX86(0x10,&rg, &rg);	/* VIDEO - GET DISPLAY COMBINATION CODE (PS,VGA,MCGA) */

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
	INTX86(0x11, &rg, &rg);	/* BIOS - GET EQUIPMENT LIST */

	/* Bits 4-5 in ax are:
	 *	00 EGA, VGA or PGA
	 *	01 40x25 color
	 *	10 80x25 color
	 *	11 80x25 monochrome
	 */
	if (((rg.x._AX_ & 0x30) == 0x30)) {
		monochrome = TRUE;
		return(CDMONO);
	}

	/*
	 * Test if EGA present
	 */
	rg.h.ah = 0x12;
	rg.h.bl = 0x10;
	INTX86(0x10,&rg, &rg);	/* VIDEO - GET EGA INFO */

	if (rg.h.bl != 0x10) {	/* function succeeded */
		egaexist = TRUE;
		return(CDEGA);
	}

	return (CDCGA);
}

void
scwrite(row, col, nchar, outstr, forg, bacg)	/* write a line out*/
int row, col, nchar;	/* row,col of screen to place outstr (len nchar) on */
char *outstr;	/* string to write out (must be term.t_ncol long) */
int forg;	/* foreground color of string to write */
int bacg;	/* background color */
{
	register USHORT attr;	/* attribute byte mask to place in RAM */
	register USHORT *lnptr;	/* pointer to the destination line */
	register int i;

	if (row > term.t_nrow)
		return;

	/* build the attribute byte and setup the screen pointer */
#if	COLOR
	if (ColorDisplay())
		attr = AttrColor(bacg,forg);
	else
#endif
	attr = AttrMono(bacg<forg);
	attr <<= 8;

	if (flickcode && (dtype == CDCGA))
		lnptr = sline;
	else
		lnptr = scptr[row]+col;

	if (outstr) {
		for (i = 0; i < nchar; i++) {
			*lnptr++ = (outstr[i+col] & 255) | attr;
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
		movmem(sline, scptr[row]+col, nchar*sizeof(short));
	}
}

static VIDEO *
videoAlloc(vpp)
VIDEO **vpp;
{
	if (*vpp == 0) {
		*vpp = typeallocplus(VIDEO, term.t_mcol - 4);
		if (*vpp == NULL)
		ExitProgram(BAD(1));
	}
	return *vpp;
}

/* reads back a line into a VIDEO struct, used in line-update computation */
VIDEO *
scread(vp, row)
VIDEO *vp;
int row;
{
	register int	i;

	if (row > term.t_nrow)
		return 0;

	if (vp == 0) {
		static	VIDEO	*mine;
		vp = videoAlloc(&mine);
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
	INTX86(0x10, &rg, &rg);
}
#endif	/* SCROLLCODE */

#if	FLABEL
fnclabel(f, n)		/* label a function key */
int f,n;		/* default flag, numeric argument [unused] */
{
	/* on machines with no function keys...don't bother */
	return(TRUE);
}
#endif

/*--------------------------------------------------------------------------*/

#if OPT_MS_MOUSE

/* Define translations between mouse (pixels) and chars (row/col).
 * patch: why does 8 by 8 work, not chr_high by chr_wide?
 */
#define MS_CHAR_WIDE	((term.t_ncol == 80) ? 8 : 16)
#define MS_CHAR_HIGH    8

#define	pixels2col(x)   ((x)/MS_CHAR_WIDE)
#define pixels2row(y)   ((y)/MS_CHAR_HIGH)

#define col2pixels(x)   ((x)*MS_CHAR_WIDE)
#define row2pixels(y)   ((y)*MS_CHAR_HIGH)

/* Define a macro for calling mouse services */
#define MouseCall INTX86(0x33, &rg, &rg)

#define MS_MOVEMENT	iBIT(0)	/* mouse cursor movement */
#define	MS_BTN1_PRESS   iBIT(1)	/* left button */
#define MS_BTN1_RELEASE iBIT(2)
#define MS_BTN2_PRESS   iBIT(3)	/* right button */
#define MS_BTN2_RELEASE iBIT(4)
#define MS_BTN3_PRESS   iBIT(5)	/* center button */
#define MS_BTN3_RELEASE iBIT(6)

#define BLINK 0x8000

	/* These have to be "far", otherwise TurboC doesn't force the
	 * segment register to be specified from 'ms_event_handler()'
	 */
int	far	button_pending;	/* 0=none, 1=pressed, 2=released */
int	far	button_number;	/* 1=left, 2=right, 3=center */
int	far	button_press_x;
int	far	button_press_y;
int	far	button_relsd_x;
int	far	button_relsd_y;
int		rodent_exists;
int		rodent_cursor_display;

int
ms_exists ()
{
	return rodent_exists;
}

void
ms_processing ()
{
	WINDOW	*wp;
	int	copied = FALSE,
		invert, normal,
		first, first_x = ttcol, first_y = ttrow,
		last,  last_x  = ttcol,  last_y = ttrow,
		that,  that_x,  that_y,
		attr, delta;
	USHORT *s2page = s2ptr[0];

	ms_showcrsr();
	while (button_pending) {
		if (button_pending == 2) {
			button_pending = 0;
			if (copied) {
				set_page(0);
				ms_setvrange(0, term.t_nrow);
				mlerase();
				setwmark(pixels2row(last_y),
					 pixels2col(last_x));
				if (!same_ptr(DOT.l,MK.l)
				 || DOT.o != MK.o) {
					regionshape = EXACT;
					(void)yankregion();
					(void)update(TRUE);
				}
				movecursor(pixels2row(first_y),
					 pixels2col(first_x));

			}
		} else {	/* selection */

			disable();
			that_x = button_relsd_x;
			that_y = button_relsd_y;
			enable();

			if (!copied) {
				int x = pixels2col(button_press_x);
				int y = pixels2row(button_press_y);
				wp = row2window(y);
				/* Set the dot-location if button 1 was pressed in a
				 * window.
				 */
				if (wp != 0
				 && ttrow != term.t_nrow
				 && setcursor(y, x)) {
					(void)update(TRUE);
					ms_setvrange(wp->w_toprow,
						     mode_row(wp) - 1);
				} else {	/* cannot reposition */
					kbd_alarm();
					while (button_pending != 2)
						;
					continue;
				}
				first_x = last_x = col2pixels(ttcol);
				first_y = last_y = row2pixels(ttrow);
				first = ms_crsr2inx(first_x, first_y);

				copy2nd(FALSE);
				set_page(1);
				copied = TRUE;

				invert = vp2attr(vscreen[ttrow],TRUE);
				normal = vp2attr(vscreen[ttrow],FALSE);
			}

			that  = ms_crsr2inx(that_x, that_y);
			last  = ms_crsr2inx(last_x, last_y);
			delta = (last < that) ? 1 : -1;

			if (that != last) {
				register int j;
				if (((last < that) && (last <= first))
				 || ((last > that) && (last >= first))) {
					attr = normal;
				} else {
					attr = invert;
				}
#define HiLite(n,attr) s2page[n] = attr | (s2page[n] & 0xff)

				for (j = last; j != that; j += delta) {
					if (j == first) {
						if (attr == normal) {
							attr = invert;
						} else {
							attr = normal;
						}
					}
					HiLite(j,attr);
				}
				HiLite(that,invert|BLINK);
			}
			last_x = that_x;
			last_y = that_y;
		}
	}
}

/* translate cursor position (pixels) to array-index of the text-position */
static int
ms_crsr2inx(x, y)
int	x;
int	y;
{
	return pixels2col(x) + (pixels2row(y) * term.t_ncol);
}

static void
ms_deinstall(void)
{
	ms_hidecrsr();
	rg.x._AX_ = 0;	/* reset the mouse */
	MouseCall;
	rodent_exists = FALSE;
}

	/* This event-handler cannot do I/O; tracing it can be tricky...
	 */
void far
ms_event_handler P((void))
{
	UINT	ms_event  = _AX;
/*	UINT	ms_button = _BX;*/
	UINT	ms_horz   = _CX;
	UINT	ms_vert   = _DX;

	if (ms_event & MS_BTN1_PRESS) {
		button_pending = 1;
		button_number  = 1;
		button_press_x = button_relsd_x = ms_horz;
		button_press_y = button_relsd_y = ms_vert;
	} else {	/* movement or release */
		if (ms_event & MS_BTN1_RELEASE)
			button_pending = 2;
		button_relsd_x = ms_horz;
		button_relsd_y = ms_vert;
	}
	return;
}

static void
ms_hidecrsr()
{
	/* Hides the mouse cursor if it is displayed */
	if (rodent_cursor_display) {
		rodent_cursor_display = FALSE;
		rg.x._AX_ = 0x02;
		MouseCall;
	}
} /* End of ms_hidecrsr() */

static void
ms_install()
{
	if (rodent_exists)
		return;

	/* If a mouse is installed, initializes the mouse and
	 * sets rodent_exists to 1. If no mouse is installed,
	 * sets rodent_exists to 0.
	 */
	rg.x._AX_ = 0;
	MouseCall;
	rodent_exists = rg.x._AX_;
	rodent_cursor_display = FALSE; /* safest assumption */
	if (ms_exists()) {
		struct SREGS segs;
		rg.x._AX_ = 0xc;
		rg.x._CX_ = MS_MOVEMENT | MS_BTN1_PRESS | MS_BTN1_RELEASE;
		rg.x._DX_ = FP_OFF(ms_event_handler);
		segs.es = FP_SEG(ms_event_handler);
		INTX86X(0x33, &rg, &rg, &segs);
	}
}

static void
ms_setvrange(upperrow, lowerrow)
int upperrow;
int lowerrow;
{
	/* Restricts vertical cursor movement to the screen region
	 * between upperrow and lowerrow.  If the cursor is outside the range,
	 * it is moved inside.
	 */
	rg.x._AX_ = 0x08;
	rg.x._CX_ = row2pixels(upperrow);
	rg.x._DX_ = row2pixels(lowerrow);
	MouseCall;
} /* End of ms_setvrange() */


static void
ms_showcrsr(void)
{
	/* Displays the mouse cursor */
	int counter;

	/* Call Int 33H Function 2AH to get the value of the display counter */
	rg.x._AX_ = 0x2A;
	MouseCall;
	counter = rg.x._AX_;

	/* Call Int 33H Function 01H as many times as needed to display */
	/* the mouse cursor */
	while (counter-- > 0) {
		rg.x._AX_ = 0x01;
		MouseCall;
	}
	rodent_cursor_display = TRUE;
} /* End of ms_showcrsr() */
#endif /* OPT_MS_MOUSE */
