/*	tcap:	Unix V5, V7 and BS4.2 Termcap video driver
 *		for MicroEMACS
 *
 * $Log: tcap.c,v $
 * Revision 1.16  1992/12/04 09:18:31  foxharp
 * allow the open routine to be called again, to emit TI, KS
 *
 * Revision 1.15  1992/05/19  08:55:44  foxharp
 * more prototype and shadowed decl fixups
 *
 * Revision 1.14  1992/05/16  12:00:31  pgf
 * prototypes/ansi/void-int stuff/microsoftC
 *
 * Revision 1.13  1992/04/10  18:47:25  pgf
 * change abs to absol to get rid of name conflicts
 *
 * Revision 1.12  1992/03/24  08:46:02  pgf
 * fixed support for AL,DL -- I hope it's really safe to use tgoto as
 * a generic parm capability expander.  I
 *
 * Revision 1.11  1992/03/24  07:46:34  pgf
 * added support for DL and AL capabilities, which are multi-line insert
 * and delete -- much better in an xterm
 *
 * Revision 1.10  1992/01/22  20:27:47  pgf
 * added TI, TE, KS, KE support, per user suggestion (sorry, forgot who)
 *
 * Revision 1.9  1991/11/01  14:38:00  pgf
 * saber cleanup
 *
 * Revision 1.8  1991/10/23  12:05:37  pgf
 * NULL initializations should have been 0 instead
 *
 * Revision 1.7  1991/09/10  01:19:35  pgf
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


#define TCAPSLEN 315
char tcapbuf[TCAPSLEN];
char *UP, PC, *CM, *CE, *CL, *SO, *SE;
char *TI, *TE, *KS, *KE;

#if	SCROLLCODE
char *CS, *dl, *al, *DL, *AL, *SF, *SR;
#endif

extern char *tgoto P((char *, int, int));
extern int tgetent P((char *, char *));
extern int tgetnum P((char * ));
extern int tputs P((char *, int, void(*_f)(int) ));

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

void
tcapopen()
{
	char *getenv();
	char *t, *p, *tgetstr();
	char tcbuf[1024];
	char *tv_stype;
	char err_str[72];
	static int already_open = 0;

	if (already_open) 
	{
		if (TI)
			putnpad(TI, strlen(TI));
		if (KS)
			putpad(KS);
		return;
	}

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
	TI = tgetstr("ti", &p);
	TE = tgetstr("te", &p);
	KS = tgetstr("ks", &p);
	KE = tgetstr("ke", &p);
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
	dl = tgetstr("dl", &p);
	al = tgetstr("al", &p);
	DL = tgetstr("DL", &p);
	AL = tgetstr("AL", &p);
        
	if (CS && SR) {
		if (SF == NULL) /* assume '\n' scrolls forward */
			SF = "\n";
		term.t_scroll = tcapscroll_reg;
	} else if ((DL && AL) || (dl && al)) {
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
	if (TI)
		putnpad(TI, strlen(TI));
	if (KS)
		putpad(KS);
	already_open = TRUE;
}

void
tcapclose()
{
	if (TE)
		putnpad(TE, strlen(TE));
	if (KE)
		putpad(KE);
}

void
tcapkopen()
{
	strcpy(sres, "NORMAL");
}

void
tcapkclose()
{
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
	putpad(CL);
}

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

int
tcapcres()	/* change screen resolution */
{
	return(TRUE);
}

#if SCROLLCODE

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
	tcapscrollregion(0, term.t_nrow);
}

/* 
PRETTIER_SCROLL is prettier but slower -- it scrolls 
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
#if PRETTIER_SCROLL
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

#endif

/* ARGSUSED */
void
spal(dummy)	/* change palette string */
char *dummy;
{
	/*	Does nothing here	*/
}

#if	COLOR
void
tcapfcol()	/* no colors here, ignore this */
{
}

void
tcapbcol()	/* no colors here, ignore this */
{
}
#endif

void
tcapbeep()
{
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


#if	FLABEL
/* ARGSUSED */
int
fnclabel(f, n)		/* label a function key */
int f,n;	/* default flag, numeric argument [unused] */
{
	/* on machines with no function keys...don't bother */
	return(TRUE);
}
#endif
#else

void
hello()
{
}

#endif
