/* These functions perform vi's on-this-line character scanning functions.
 *	written for vile by Paul Fox, (c)1990
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/csrch.c,v 1.17 1994/07/11 22:56:20 pgf Exp $
 *
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

	doto = DOT.o;

	i = doto+1;
	while(i < lLength(DOT.l)) {
		if ( c == lGetc(DOT.l,i)) {
			doto = i;
			n--;
			if (!n) break;
		}
		i++;
	}

	if ( i == lLength(DOT.l)) {
		return(FALSE);
	}
	if (doingopcmd)
		doto++;

	DOT.o = doto;
	curwp->w_flag |= WFMOVE;
	return(TRUE);
			
}

int
bscan(f,n,c)
int f,n,c;
{
	int i;
	int doto;

	if (!f || n <= 0)
		n = 1;

	lstchar = c;
	lstscan = BACK;

	doto = DOT.o;

	i = doto-1;
	while(i >= w_left_margin(curwp)) {
		if ( c == lGetc(DOT.l,i)) {
			doto = i;
			n--;
			if (!n) break;
		}
		i--;
	}

	if ( i < w_left_margin(curwp) ) {
		return(FALSE);
	}

	DOT.o = doto;
	curwp->w_flag |= WFMOVE;
	return(TRUE);

}

int
get_csrch_char(cp)
int *cp;
{
	int c;

	if (clexec || isnamedcmd) {
		int status;
		static char cbuf[2];
		if ((status=mlreply("Scan for: ", cbuf, 2)) != TRUE)
			return status;
		c = cbuf[0];
	} else {
		c = tgetc(FALSE);
		if (c == quotec)
			c = tgetc(TRUE);
		else if (c == abortc)
			return FALSE;
	}

	*cp = c;
	return TRUE;
}

/* f */
int
fcsrch(f,n)
int f,n;
{
	int c, s;

	s = get_csrch_char(&c);
	if (s != TRUE)
		return s;

	return(fscan(f,n,c));
}

/* F */
int
bcsrch(f,n)
int f,n;
{
	int c, s;

	s = get_csrch_char(&c);
	if (s != TRUE)
		return s;

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
