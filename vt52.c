/*
 * The routines in this file
 * provide support for VT52 style terminals
 * over a serial line. The serial I/O services are
 * provided by routines in "termio.c". It compiles
 * into nothing if not a VT52 style device. The
 * bell on the VT52 is terrible, so the "beep"
 * routine is conditionalized on defining BEL.
 *
 * $Log: vt52.c,v $
 * Revision 1.6  1993/06/18 15:57:06  pgf
 * tom's 3.49 changes
 *
 * Revision 1.5  1993/04/01  12:57:22  pgf
 * removed redundant includes and declarations
 *
 * Revision 1.4  1991/11/16  18:25:37  pgf
 * ifdef change for UNIX
 *
 * Revision 1.3  1991/09/10  01:19:35  pgf
 * re-tabbed, and moved ESC and BEL to estruct.h
 *
 * Revision 1.2  1991/08/07  12:35:07  pgf
 * added RCS log messages
 *
 * revision 1.1
 * date: 1990/09/21 10:26:20;
 * initial vile RCS revision
 */

#define termdef 1			/* don't define "term" external */

#include	"estruct.h"
#include	"edef.h"

#if	VT52

#define NROW	24			/* Screen size. 		*/
#define NCOL	80			/* Edit if you want to. 	*/
#define MARGIN	8			/* size of minimim margin and	*/
#define SCRSIZ	64			/* scroll size for extended lines */
#define NPAUSE	100			/* # times thru update to pause */
#define BIAS	0x20			/* Origin 0 coordinate bias.	*/

extern	int	ttopen();		/* Forward references.		*/
extern	int	ttgetc();
extern	int	ttputc();
extern	int	ttflush();
extern	int	ttclose();
extern	int	vt52move();
extern	int	vt52eeol();
extern	int	vt52eeop();
extern	int	vt52beep();
extern	int	vt52open();
extern	int	vt52rev();
extern	int	vt52cres();
extern	int	vt52kopen();
extern	int	vt52kclose();

#if	COLOR
extern	int	vt52fcol();
extern	int	vt52bcol();
#endif

/*
 * Dispatch table. All the
 * hard fields just point into the
 * terminal I/O code.
 */
TERM	term	= {
	NROW-1,
	NROW-1,
	NCOL,
	NCOL,
	MARGIN,
	SCRSIZ,
	NPAUSE,
	&vt52open,
	&ttclose,
	&vt52kopen,
	&vt52kclose,
	&ttgetc,
	&ttputc,
	&ttflush,
	&vt52move,
	&vt52eeol,
	&vt52eeop,
	&vt52beep,
	&vt52rev,
	&vt52cres
#if	COLOR
	, &vt52fcol,
	&vt52bcol
#endif
};

vt52move(row, col)
{
	ttputc(ESC);
	ttputc('Y');
	ttputc(row+BIAS);
	ttputc(col+BIAS);
}

vt52eeol()
{
	ttputc(ESC);
	ttputc('K');
}

vt52eeop()
{
	ttputc(ESC);
	ttputc('J');
}

vt52rev(status) /* set the reverse video state */

int status;	/* TRUE = reverse video, FALSE = normal video */

{
	/* can't do this here, so we won't */
}

vt52cres()	/* change screen resolution - (not here though) */

{
	return(TRUE);
}

spal()		/* change palette string */

{
	/*	Does nothing here	*/
}

#if	COLOR
vt52fcol()	/* set the forground color [NOT IMPLIMENTED] */
{
}

vt52bcol()	/* set the background color [NOT IMPLIMENTED] */
{
}
#endif

vt52beep()
{
#ifdef	BEL
	ttputc(BEL);
	ttflush();
#endif
}

vt52open()
{
#if	UNIX
	register char *cp;

	if ((cp = getenv("TERM")) == NULL) {
		puts("Shell variable TERM not defined!");
		ExitProgram(1);
	}
	if (strcmp(cp, "vt52") != 0 && strcmp(cp, "z19") != 0) {
		puts("Terminal type not 'vt52'or 'z19' !");
		ExitProgram(1);
	}
#endif
	ttopen();
}

vt52kopen()

{
}

vt52kclose()

{
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

vt52hello()

{
}

#endif
