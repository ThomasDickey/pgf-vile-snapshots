/* These functions perform vi's on-this-line character scanning functions.
 *	written for vile by Paul Fox, (c)1990
 *
 * $Log: csrch.c,v $
 * Revision 1.6  1992/12/04 09:12:25  foxharp
 * deleted unused assigns
 *
 * Revision 1.5  1992/05/16  12:00:31  pgf
 * prototypes/ansi/void-int stuff/microsoftC
 *
 * Revision 1.4  1991/11/01  14:38:00  pgf
 * saber cleanup
 *
 * Revision 1.3  1991/08/07  12:35:07  pgf
 * added RCS log messages
 *
 * revision 1.2
 * date: 1991/06/25 19:52:11;
 * massive data structure restructure
 * 
 * revision 1.1
 * date: 1990/09/21 10:24:56;
 * initial vile RCS revision
*/

#include "estruct.h"
#include "edef.h"

static short lstscan;
static short lstchar;
#define BACK 0
#define FORW 1
#define DIREC 1

#define F 0
#define T 2
#define TYPE 2


int
fscan(f,n,c)
int f,n,c;
{
	int i;
	int doto;

	if (!f || n <= 0)
		n = 1;

	lstchar = c;
	lstscan = FORW;

	doto = curwp->w_dot.o;

	i = doto+1;
	while(i < llength(curwp->w_dot.l)) {
		if ( c == lgetc(curwp->w_dot.l,i)) {
			doto = i;
			n--;
			if (!n) break;
		}
		i++;
	}

	if ( i == llength(curwp->w_dot.l)) {
		TTbeep();
		return(FALSE);
	}
	if (doingopcmd)
		doto++;

	curwp->w_dot.o = doto;
	curwp->w_flag |= WFMOVE;
	return(TRUE);
			
}

int
bscan(f,n,c)
int f,n,c;
{
	int i;
	int doto;

	if (f || n <= 0)
		n = 1;

	lstchar = c;
	lstscan = BACK;

	doto = curwp->w_dot.o;

	i = doto-1;
	while(i >= 0) {
		if ( c == lgetc(curwp->w_dot.l,i)) {
			doto = i;
			n--;
			if (!n) break;
		}
		i--;
	}

	if ( i < 0 ) {
		TTbeep();
		return(FALSE);
	}

	curwp->w_dot.o = doto;
	curwp->w_flag |= WFMOVE;
	return(TRUE);

}

/* f */
int
fcsrch(f,n)
int f,n;
{
	register int c;

        c = kbd_key();
	if (c == quotec)
		c = tgetc();
	else if (c == abortc)
		return FALSE;
	else
		c = kcod2key(c);

	return(fscan(f,n,c));
}

/* F */
int
bcsrch(f,n)
int f,n;
{
	register int c;

        c = kbd_key();
	if (c == quotec)
		c = tgetc();
	else if (c == abortc)
		return FALSE;
	else
		c = kcod2key(c);

	return(bscan(f,n,c));
}

/* t */
int
fcsrch_to(f,n)
int f,n;
{
	int s;
	s = fcsrch(f,n);
	if (s == TRUE)
		s = backchar(FALSE,1);
	lstscan |= T;
	return(s);
}

/* T */
int
bcsrch_to(f,n)
int f,n;
{
	int s;
	s = bcsrch(f,n);
	if (s == TRUE)
		s = forwchar(FALSE,1);
	lstscan |= T;
	return(s);
}

/* ; */
int
rep_csrch(f,n)
int f,n;
{
	int s;
	int ls = lstscan;

	if ((ls & DIREC) == FORW) {
		s = fscan(f,n,lstchar);
		if ((ls & TYPE) == T) {
			if (s == TRUE)
				s = backchar(FALSE,1);
			lstscan |= T;
		}
		return(s);
	} else {
		s = bscan(f,n,lstchar);
		if ((ls & TYPE) == T) {
			if (s == TRUE)
				s = forwchar(FALSE,1);
			lstscan |= T;
		}
		return(s);
	}
}

/* , */
int
rev_csrch(f,n)
int f,n;
{
	int s;

	lstscan ^= DIREC;
	s = rep_csrch(f,n);
	lstscan ^= DIREC;
	return(s);
}
