/*
 * The routines in this file provide support for the IBM-PC and other
 * compatible terminals. It goes directly to the graphics RAM to do
 * screen output. It compiles into nothing if not an IBM-PC driver
 * Supported monitor cards include CGA, MONO and EGA.
 * Modified by Pete Ruczynski (pjr) for auto-sensing and selection of
 * display type.
 *
 * $Log: ibmpc.c,v $
 * Revision 1.8  1992/07/01 17:06:32  foxharp
 * pgf cleanup (in general I can't leave well enough alone...).  somewhere
 * along the way I made it work properly -- I think the problem was a missing
 * page number in ibmputc() -- but it might have been a badly calculated
 * attribute byte somewhere else...
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

#include        <stdio.h>
#include        "estruct.h"
#include        "edef.h"


#if     IBMPC

#define NROW	50			/* Max Screen size.		*/
#define NCOL    80                      /* Edit if you want to.         */
#define	MARGIN	8			/* size of minimim margin and	*/
#define	SCRSIZ	64			/* scroll size for extended lines */
#define	NPAUSE	200			/* # times thru update to pause */
#define	SPACE	32			/* space character		*/

#define	SCADC	0xb8000000L		/* CGA address of screen RAM	*/
#define	SCADM	0xb0000000L		/* MONO address of screen RAM	*/
#define SCADE	0xb8000000L		/* EGA address of screen RAM	*/

#define MONOCRSR 0x0B0D			/* monochrome cursor	    */
#define CGACRSR 0x0607			/* CGA cursor		    */
#define EGACRSR 0x0709			/* EGA cursor		    */

#define NDRIVE	4			/* number of screen drivers	upped to 4 (vga) */


int dtype = -1;				/* current display type		*/
char drvname[][8] = {			/* screen resolution names	*/
	"CGA", "MONO", "EGA", "VGA"
};
long scadd;				/* address of screen ram	*/
short *scptr[NROW];			/* pointer to screen lines	*/
unsigned short sline[NCOL];		/* screen line image		*/
int egaexist = FALSE;			/* is an EGA card available?	*/
union REGS rg;				/* cpu register for use of DOS calls */

extern  void     ttopen();               /* Forward references.          */
extern  int     ttgetc();
extern  void     ttputc();
extern  void     ttflush();
extern  void     ttclose();
extern  int     ibmmove();
extern  int     ibmeeol();
extern  int     ibmeeop();
extern  int     ibmbeep();
extern  int     ibmopen();
extern	int     ibmrev();
extern	int	    ibmcres();
extern	int	    ibmclose();
extern	int	    ibmputc();
extern	int	    ibmkopen();
extern	int	    ibmkclose();

#if	COLOR
extern	int	ibmfcol();
extern	int	ibmbcol();

int	cfcolor = -1;		/* current forground color */
int	cbcolor = -1;		/* current background color */
int	ctrans[] =		/* ansi to ibm color translation table */
	{0, 4, 2, 6, 1, 5, 3, 7};
#endif

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
	NULL
};

#if	COLOR
ibmfcol(color)		/* set the current output color */
int color;	/* color to set */
{
        cfcolor = ctrans[color];
}

ibmbcol(color)		/* set the current background color */
int color;	/* color to set */
{
        cbcolor = ctrans[color];
}
#endif

ibmmove(row, col)
{
	rg.h.ah = 2;		/* set cursor position function code */
	rg.h.dl = col;
	rg.h.dh = row;
	rg.h.bh = 0;		/* set screen page number */
	int86(0x10, &rg, &rg);
}

/* erase to the end of the line */
ibmeeol()
{
	unsigned short attr;	/* attribute byte mask to place in RAM */
	unsigned short *lnptr;	/* pointer to the destination line */
	int i;
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
ibmputc(ch)
int ch;
{
	rg.h.ah = 14;		/* write char to screen with current attrs */
	rg.h.al = ch;
#if	COLOR
	if (dtype != CDMONO)
		rg.h.bl = cfcolor;
	else
#endif
	 rg.h.bl = 0x07;
	rg.h.bh = 0;		/* current video page */
	int86(0x10, &rg, &rg);

}

ibmeeop()
{
	int attr;		/* attribute to fill screen with */

	rg.h.ah = 6;		/* scroll page up function code */
	rg.h.al = 0;		/* # lines to scroll (clear it) */
	rg.x.cx = 0;		/* upper left corner of scroll */
	rg.h.dh = term.t_nrow;  /* lower right corner of scroll */
	rg.h.dl = term.t_ncol - 1;
#if	COLOR
	if (dtype != CDMONO)
		attr = ((ctrans[gbcolor] & 15) << 4) | (ctrans[gfcolor] & 15);
	else
#endif
	 attr = 0;
	rg.h.bh = attr;
	int86(0x10, &rg, &rg);
}


ibmrev(state)		/* change reverse video state */
int state;	/* TRUE = reverse, FALSE = normal */
{
	/* This never gets used under the IBM-PC driver */
}


ibmcres(res)	/* change screen resolution */
int res;	/* resolution to change to */
{
	int i;		/* index */

	for (i = 0; i < NDRIVE; i++)
		if (strcmp(drvname[res], drvname[i]) == 0) {
			scinit(i);
			return(TRUE);
		}
	return(FALSE);
}

void
spal()	/* reset the pallette registers */
{
	/* nothin here now..... */
}

ibmbeep()
{
#if	MWC86
	putcnb(BEL);
#else
	bdos(6, BEL, 0);	/* annoying!! */
#endif
}

ibmopen()
{
	scinit(ibmtype);
	revexist = TRUE;
        ttopen();
}

ibmclose()

{
#if	COLOR
	ibmfcol(7);
	ibmbcol(0);
#endif
	if (dtype == CDEGA) /* if we had the EGA open... close it */
		egaclose();
	else if (dtype == CDVGA) /* if we had the VGA open... close it */
		vgaclose();
}

ibmkopen()	/* open the keyboard */
{
}

ibmkclose()	/* close the keyboard */
{
}

scinit(type)	/* initialize the screen head pointers */
int type;	/* type of adapter to init for */
{
	union {
		long laddr;	/* long form of address */
		short *paddr;	/* pointer form of address */
	} addr;
	int i;

	/* if asked...find out what display is connected */
	if (type == CDSENSE)
		type = getboard();
	else
		/* otherwise set up params as requested */
		type = setboard(type);

	/* if we have nothing to do....don't do it */
	if (dtype == type)
		return(TRUE);

	/* if we try to switch to EGA and there is none, don't */
	if ((type != CDVGA) && (type == CDEGA && egaexist != TRUE))
		return(FALSE);

	if (dtype == CDEGA) /* if we had the EGA open... close it */
		egaclose();
	else if (dtype == CDVGA) /* if we had the VGA open... close it */
		vgaclose();

	/* and set up the various parameters as needed */
	switch (type) {
		case CDMONO:	/* Monochrome adapter */
				scadd = SCADM;
				newscreensize(25, term.t_ncol);
				break;

		case CDCGA:	/* Color graphics adapter */
				scadd = SCADC;
				newscreensize(25, term.t_ncol);
				break;

		case CDEGA:	/* Enhanced graphics adapter */
				scadd = SCADE;
				egaopen();
				newscreensize(43, term.t_ncol);
				break;

		case CDVGA:	/* VGA adapter */
				scadd = SCADE;
				vgaopen();
				newscreensize(50, term.t_ncol);
				break;
	}

	/* reset the $sres environment variable */
	strcpy(sres, drvname[type]);
	dtype = type;

	/* initialize the screen pointer array */
	addr.laddr = scadd;
	for (i = 0; i < NROW; i++) {
		scptr[i] = addr.paddr + NCOL * i;
	}
	return(TRUE);
}

/* getboard:	Determine which type of display board is attached.
		Current known types include:

		CDMONO	Monochrome graphics adapter
		CDCGA	Color Graphics Adapter
		CDEGA	Extended graphics Adapter
		CDVGA	VGA graphics Adapter
*/

/* getbaord:	Detect the current display adapter
		if MONO		set to MONO
		   CGA		set to CGA	EGAexist = FALSE
		   EGA		set to CGA	EGAexist = TRUE
*/

int getboard()

{
	int type;	/* board type to return */

	type = CDCGA;
	egaexist = FALSE;

	/* check for VGA or MCGA */
	rg.h.ah = 0x1a;
	rg.h.bl = 0x00;
	int86(0x10,&rg, &rg);

	if (rg.h.al == 0x1a) { /* its a VGA or MCGA */
		if (rg.h.bl < 0x0a)
			type = CDVGA;
		else
			type = CDCGA;

		return(type);
	}
	
	/* check for MONO board first */
	int86(0x11, &rg, &rg);
	if (((rg.x.ax & 0x30) == 0x30))
		return(CDMONO);

	/* test if EGA present */
	rg.h.ah = 0x12;
	rg.h.bl = 0x10;
	int86(0x10,&rg, &rg);

	if (rg.h.bl != 0x10) { /* its an EGA */
		egaexist = TRUE;
		return(CDEGA);
	}

	return(type);

}

/*
 * setboard() - added by pjr
 *
 * This allows the user to set the monitor type by hand by using a set
 * variable in the rc file rathher than trying to figure out automatically
 * which board is present, anyhow people may prefer 25 lines!
 */
int setboard(type)
int type;	/* board requested and type to return */
{

	switch (type)
	{
	case CDMONO:	/* 25 lines */
		if (dtype == CDMONO)
			type = CDMONO;
		else
			type = CDCGA;

		egaexist = FALSE;
		break;
	case CDCGA:	/* 25 lines */
		egaexist = FALSE;
		break;
	case CDEGA:	/* 43 lines */
		egaexist = TRUE;
		break;
	case CDVGA:	/* 50 lines */
		egaexist = FALSE;
		break;
	default:
		egaexist = FALSE;
	} /* end of switch */
	
	return(type);
}

egaopen()	/* init the computer to work with the EGA */

{
	/* put the beast into EGA 43 row mode */
	rg.x.ax = 3;
	int86(16, &rg, &rg);

	rg.h.ah = 17;		/* set char. generator function code */
	rg.h.al = 18;		/*  to 8 by 8 double dot ROM         */
	rg.h.bl = 0;		/* block 0                           */
	int86(16, &rg, &rg);

	rg.h.ah = 18;		/* alternate select function code    */
	rg.h.al = 0;		/* clear AL for no good reason       */
	rg.h.bl = 32;		/* alt. print screen routine         */
	int86(16, &rg, &rg);

	rg.h.ah = 1;		/* set cursor size function code */
	rg.x.cx = 0x0607;	/* turn cursor on code */
	int86(0x10, &rg, &rg);

	outp(0x3d4, 10);	/* video bios bug patch */
	outp(0x3d5, 6);
}

egaclose()

{
	/* put the beast into 80 column mode */
	rg.x.ax = 3;
	int86(16, &rg, &rg);
}

vgaopen()	/* init the computer to work with the VGA */
{
	/* put the beast into VGA 50 row mode */
	rg.x.ax = 0x1202;
	rg.h.bl = 30;
	int86(16, &rg, &rg);

	rg.x.ax = 3;
	int86(16, &rg, &rg);

	rg.x.ax = 0x1112;	/*  to 8 by 8 double dot ROM         */
	rg.h.bl = 0;		/* block 0                           */
	int86(16, &rg, &rg);
}

vgaclose()
{
	/* put the beast back into normal 80 column mode */
	rg.x.ax = 3;
	int86(16, &rg, &rg);
}

scwrite(row, col, nchar, outstr, forg, bacg)	/* write a line out*/
int row, col, nchar;	/* row,col of screen to place outstr (len nchar) on */
char *outstr;	/* string to write out (must be term.t_ncol long) */
int forg;	/* forground color of string to write */
int bacg;	/* background color */
{
	unsigned short attr;	/* attribute byte mask to place in RAM */
	unsigned short *lnptr;	/* pointer to the destination line */
	int i;

	/* build the attribute byte and setup the screen pointer */
#if	COLOR
	if (dtype != CDMONO)
		attr = ((ctrans[bacg] & 15) << 4) | (ctrans[forg] & 15);
	else
#endif
	 attr = ((bacg & 15) << 4) | (forg & 15);
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

#if	FLABEL
fnclabel(f, n)		/* label a function key */
int f,n;	/* default flag, numeric argument [unused] */
{
	/* on machines with no function keys...don't bother */
	return(TRUE);
}
#endif

#else
ibmhello()
{
}
#endif
