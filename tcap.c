/*	tcap:	Unix V5, V7 and BS4.2 Termcap video driver
 *		for MicroEMACS
 *
 * $Log: tcap.c,v $
 * Revision 1.7  1991/09/10 01:19:35  pgf
 * re-tabbed, and moved ESC and BEL to estruct.h
 *
 * Revision 1.6  1991/08/07  12:35:07  pgf
 * added RCS log messages
 *
 * revision 1.5
 * date: 1991/08/06 15:26:22;
 * sprintf changes
 * 
 * revision 1.4
 * date: 1991/06/19 01:32:21;
 * change name of howmany 'cuz of HP/UX conflict
 * sheesh
 * 
 * revision 1.3
 * date: 1991/05/31 11:25:14;
 * moved PRETTIER_SCROLL to esturct.h
 * 
 * revision 1.2
 * date: 1990/10/01 10:37:44;
 * un-#ifdef spal()
 * 
 * revision 1.1
 * date: 1990/09/21 10:26:09;
 * initial vile RCS revision
*/

#define termdef 1			/* don't define "term" external */

#include <stdio.h>
#include	"estruct.h"
#include	"edef.h"
#include <signal.h>

#if TERMCAP

#define MARGIN	8
#define SCRSIZ	64
#define NPAUSE	10			/* # times thru update to pause */

extern int	ttopen();
extern int	ttgetc();
extern int	ttputc();
extern int	tgetnum();
extern int	ttflush();
extern int	ttclose();
extern int	tcapkopen();
extern int	tcapkclose();
extern int	tcapmove();
extern int	tcapeeol();
extern int	tcapeeop();
extern int	tcapbeep();
extern int	tcaprev();
extern int	tcapcres();
extern int	tcapopen();
extern int	tput();
extern char	*tgoto();
#if	COLOR
extern	int	tcapfcol();
extern	int	tcapbcol();
#endif
#if	SCROLLCODE
extern	int	tcapscroll_reg();
extern	int	tcapscroll_delins();
#endif

#define TCAPSLEN 315
char tcapbuf[TCAPSLEN];
char *UP, PC, *CM, *CE, *CL, *SO, *SE;

#if	SCROLLCODE
char *CS, *DL, *AL, *SF, *SR;
#endif

TERM term = {
	NULL,	/* these four values are set dynamically at open time */
	NULL,
	NULL,
	NULL,
	MARGIN,
	SCRSIZ,
	NPAUSE,
	tcapopen,
	ttclose,
	tcapkopen,
	tcapkclose,
	ttgetc,
	ttputc,
	ttflush,
	tcapmove,
	tcapeeol,
	tcapeeop,
	tcapbeep,
	tcaprev,
	tcapcres
#if	COLOR
	, tcapfcol,
	tcapbcol
#endif
#if	SCROLLCODE
	, NULL		/* set dynamically at open time */
#endif
};

tcapopen()
{
	char *getenv();
	char *t, *p, *tgetstr();
	char tcbuf[1024];
	char *tv_stype;
	char err_str[72];

	if ((tv_stype = getenv("TERM")) == NULL)
	{
		puts("Environment variable TERM not defined!");
		exit(1);
	}

	if ((tgetent(tcbuf, tv_stype)) != 1)
	{
		lsprintf(err_str, "Unknown terminal type %s!", tv_stype);
		puts(err_str);
		exit(1);
	}

	/* Get screen size from system, or else from termcap.  */
	getscreensize(&term.t_ncol, &term.t_nrow);
 
	if ((term.t_nrow <= 0) && (term.t_nrow=(short)tgetnum("li")) == -1) {
		puts("termcap entry incomplete (lines)");
		exit(1);
	}
	term.t_nrow -= 1;


	if ((term.t_ncol <= 0) &&(term.t_ncol=(short)tgetnum("co")) == -1){
		puts("Termcap entry incomplete (columns)");
		exit(1);
	}

#ifdef SIGWINCH
	term.t_mrow =  200;
	term.t_mcol = 200;
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
	if (SO != NULL)
		revexist = TRUE;

	if(CL == NULL || CM == NULL || UP == NULL)
	{
		puts("Incomplete termcap entry\n");
		exit(1);
	}

	if (CE == NULL) 	/* will we be able to use clear to EOL? */
		eolexist = FALSE;
#if SCROLLCODE
	CS = tgetstr("cs", &p);
	SF = tgetstr("sf", &p);
	SR = tgetstr("sr", &p);
	DL = tgetstr("dl", &p);
	AL = tgetstr("al", &p);
        
	if (CS && SR) {
		if (SF == NULL) /* assume '\n' scrolls forward */
			SF = "\n";
		term.t_scroll = tcapscroll_reg;
	} else if (DL && AL) {
		term.t_scroll = tcapscroll_delins;
	} else {
		term.t_scroll = NULL;
	}
#endif
	        
	if (p >= &tcapbuf[TCAPSLEN])
	{
		puts("Terminal description too big!\n");
		exit(1);
	}
	ttopen();
}

tcapkopen()
{
	strcpy(sres, "NORMAL");
}

tcapkclose()
{
}

tcapmove(row, col)
register int row, col;
{
	putpad(tgoto(CM, col, row));
}

tcapeeol()
{
	putpad(CE);
}

tcapeeop()
{
	putpad(CL);
}

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

tcapcres()	/* change screen resolution */
{
	return(TRUE);
}

#if SCROLLCODE

/* move howmany lines starting at from to to */
tcapscroll_reg(from,to,n)
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
	tcapscrollregion(0, term.t_nrow);
}

/* 
PRETTIER_SCROLL is prettier but slower -- it scrolls 
		a line at a time instead of all at once.
*/

/* move howmany lines starting at from to to */
tcapscroll_delins(from,to,n)
{
	int i;
	if (to == from) return;
#if PRETTIER_SCROLL
	if (abs(from-to) > 1) {
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
			putpad(DL);
		tcapmove(to+n,0);
		for (i = from - to; i > 0; i--)
			putpad(AL);
	} else {
		tcapmove(from+n,0);
		for (i = to - from; i > 0; i--)
			putpad(DL);
		tcapmove(from,0);
		for (i = to - from; i > 0; i--)
			putpad(AL);
	}
}

/* cs is set up just like cm, so we use tgoto... */
tcapscrollregion(top,bot)
{
	putpad(tgoto(CS, bot, top));
}

#endif

spal(dummy)	/* change palette string */
{
	/*	Does nothing here	*/
}

#if	COLOR
tcapfcol()	/* no colors here, ignore this */
{
}

tcapbcol()	/* no colors here, ignore this */
{
}
#endif

tcapbeep()
{
	ttputc(BEL);
}

putpad(str)
char	*str;
{
	tputs(str, 1, ttputc);
}

putnpad(str, n)
char	*str;
{
	tputs(str, n, ttputc);
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

hello()
{
}

#endif
