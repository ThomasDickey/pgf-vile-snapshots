/* 
 *
 *	fences.c
 *
 * Match up various fenceposts, like (), [], {}, */ /*, #if, #el, #en
 *
 * Most code probably by Dan Lawrence or Dave Conroy for MicroEMACS
 * Extensions for vile by Paul Fox
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/fences.c,v 1.30 1995/05/09 12:12:26 pgf Exp $
 *
 */

#include	"estruct.h"
#include	"edef.h"

#if OPT_CFENCE

#define	CPP_UNKNOWN -1
#define	CPP_IF       0
#define	CPP_ELIF     1
#define	CPP_ELSE     2
#define	CPP_ENDIF    3

static	int	comment_fence P(( int ));
static	int	cpp_keyword P(( LINE *, int ));
static	int	cpp_fence P(( int, int ));
static	int	getfence P(( int, int ));
static	int	simple_fence P(( int, int, int ));

static int
cpp_keyword(lp,off)
LINE	*lp;
int	off;
{
	char	temp[NSTRING];
	register char *d = temp;
	register int  n;

	static	struct	{
		char	*name;
		int	code;
	} keyword_table[] = {
		{ "if",     CPP_IF },
		{ "ifdef",  CPP_IF },
		{ "ifndef", CPP_IF },
		{ "elif",   CPP_ELIF },
		{ "else",   CPP_ELSE },
		{ "endif",  CPP_ENDIF }
	};

	while (off < llength(lp)) {
		n = lgetc(lp,off++);
		if ((d - temp < sizeof(temp)-2) && isident(n))
			*d++ = n;
		else
			break;
	}
	*d = EOS;

	for (n = 0; n < TABLESIZE(keyword_table); n++)
		if (!strcmp(temp, keyword_table[n].name))
			return keyword_table[n].code;
	return CPP_UNKNOWN;
}

static int
cpp_fence(sdir,key)
int sdir;
int key;
{
	int count = 1;
	int i, j, this;

	/* patch: this should come from arguments */
	if (key == CPP_ENDIF)
		sdir = REVERSE;
	else
		sdir = FORWARD;

	/* set up for scan */
	if (sdir == REVERSE)
		DOT.l = lBACK(DOT.l);
	else
		DOT.l = lFORW(DOT.l);

	while (count > 0 && !is_header_line(DOT, curbp)) {
		if ((i = firstchar(l_ref(DOT.l))) >= 0
		 && lGetc(DOT.l,i) == '#'
		 && (j = nextchar(l_ref(DOT.l),i+1)) >= 0
		 && ((this = cpp_keyword(l_ref(DOT.l),j)) != CPP_UNKNOWN)) {
			int	done = FALSE;

			switch (this) {
			case CPP_IF:
				if (sdir == FORWARD) {
					count++;
				} else {
					done = ((count-- == 1) && 
						(key != this));
					if (done)
						count = 0;
				}
				break;

			case CPP_ELIF:
			case CPP_ELSE:
				done = ((sdir == FORWARD) && (count == 1));
				if (done)
					count = 0;
				break;

			case CPP_ENDIF:
				if (sdir == FORWARD) {
					done = (--count == 0);
				} else {
					count++;
				}
			}

			if ((count <= 0) || done) {
				DOT.o = i;
				break;
			}
		}

		if (sdir == REVERSE)
			DOT.l = lBACK(DOT.l);
		else
			DOT.l = lFORW(DOT.l);

		if (is_header_line(DOT,curbp) || interrupted())
			return FALSE;
	}
	if (count == 0) {
		curwp->w_flag |= WFMOVE;
		if (doingopcmd)
			regionshape = FULLLINE;
		return TRUE;
	}
	return FALSE;
}

/*	the cursor is moved to a matching fence */
int
matchfence(f,n)
int f,n;
{
	int s = getfence(0, (!f || n > 0) ? FORWARD:REVERSE);
	if (s == FALSE)
		kbd_alarm();
	return s;
}

int
matchfenceback(f,n)
int f,n;
{
	int s = getfence(0, (!f || n > 0) ? REVERSE:FORWARD);
	if (s == FALSE)
		kbd_alarm();
	return s;
}

#define PAIRED_FENCE_CH -1

int
is_user_fence(ch, sdirp)
int ch;
int *sdirp;
{
	char *fences = b_val_ptr(curbp,VAL_FENCES);
	char *chp, och;
	if (!ch)
		return 0;
	chp = strchr(fences, ch);
	if (!chp)
		return 0;
	if ((chp - fences) & 1) { 
		/* look for the left fence */
		och = chp[-1];
		if (sdirp)
			*sdirp = REVERSE;
	} else { 
		/* look for the right fence */
		och = chp[1];
		if (sdirp)
			*sdirp = FORWARD;
	}
	return och;
}

static int
getfence(ch,sdir)
int ch; /* fence type to match against */
int sdir; /* direction to scan if we're not on a fence to begin with */
{
	MARK	oldpos; 	/* original pointer */
	register int ofence = 0;	/* open fence */
	int s, i;
	int key = CPP_UNKNOWN;
	char *C_fences, *ptr;
	int fch;

	/* save the original cursor position */
	oldpos = DOT;

	/* ch may have been passed, if being used internally */
	if (!ch) {
		if ((i = firstchar(l_ref(DOT.l))) < 0)	/* offset of first nonblank */
			return FALSE;		/* line is entirely blank */

		if (DOT.o <= i && (ch = lGetc(DOT.l,i)) == '#') {
			if (lLength(DOT.l) < i+3)
				return FALSE;
		} else if ((ch = char_at(DOT)) == '/' || ch == '*') {
			/* EMPTY */;
		} else if (sdir == FORWARD) {
			/* get the current character */
			if (oldpos.o < lLength(oldpos.l)) {
				do {
					ch = char_at(oldpos);
				} while(!is_user_fence(ch, (int *)0) &&
					++oldpos.o < lLength(oldpos.l));
			}
			if (is_at_end_of_line(oldpos)) {
				return FALSE;
			}
		} else {
			/* get the current character */
			if (oldpos.o >= 0) {
				do {
					ch = char_at(oldpos);
				} while(!is_user_fence(ch, (int *)0) &&
					--oldpos.o >= 0);
			}

			if (oldpos.o < 0) {
				return FALSE;
			}
		}

		/* we've at least found a fence -- move us that far */
		DOT.o = oldpos.o;
	}

	fch = ch;

	/* is it a "special" fence char? */
	C_fences = "/*#";
	ptr = strchr(C_fences, ch);
	
	if (!ptr) {
		ofence = is_user_fence(ch, &sdir);
		if (ofence)
			fch = PAIRED_FENCE_CH;
	}

	/* setup proper matching fence */
	switch (fch) {
		case PAIRED_FENCE_CH:
			/* NOTHING */
			break;
		case '#':
			if ((i = firstchar(l_ref(DOT.l))) < 0)
				return FALSE;	/* line is entirely blank */
			if ((i = nextchar(l_ref(DOT.l),i+1)) >= 0
			 && ((key = cpp_keyword(l_ref(DOT.l),i)) != CPP_UNKNOWN))
			 	break;
			return FALSE;
		case '*':
			ch = '/';
			if (DOT.o+1 < lLength(DOT.l) &&
				(lGetc(DOT.l, DOT.o+1) == '/')) {
				sdir = REVERSE;
				forwchar(TRUE,1);
				break;
			} else if (DOT.o > 0 &&
				(lGetc(DOT.l, DOT.o-1) == '/')) {
				sdir = FORWARD;
				backchar(TRUE,1);
				if (doingopcmd)
					pre_op_dot = DOT;
				break;
			}
			return FALSE;
		case '/':
			if (DOT.o+1 < lLength(DOT.l) &&
				(lGetc(DOT.l, DOT.o+1) == '*')) {
				sdir = FORWARD;
				break;
			} else if (DOT.o > 0 &&
				(lGetc(DOT.l, DOT.o-1) == '*')) {
				sdir = REVERSE;
				break;
			}
			/* FALL THROUGH */
		default: 
			return(FALSE);
	}

	/* ops are inclusive of the endpoint */
	if (doingopcmd && sdir == REVERSE) {
		forwchar(TRUE,1);
		pre_op_dot = DOT;
		backchar(TRUE,1);
	}

	if (key != CPP_UNKNOWN) {  /* we're searching for a cpp keyword */
		s = cpp_fence(sdir, key);
	} else if (ch == '/') {
		s = comment_fence(sdir);
	} else {
		s = simple_fence(sdir, ch, ofence);
	}

	if (s == TRUE)
		return TRUE;

	/* restore the current position */
	DOT = oldpos;
	return(FALSE);
}

static int
simple_fence(sdir, ch, ofence)
int sdir;
int ch;
int ofence;
{
	int count = 1;
	int c, s = FALSE;

	/* set up for scan */
	if (sdir == REVERSE)
		backchar(FALSE, 1);
	else
		forwchar(FALSE, 1);

	while (count > 0) {
		if (is_at_end_of_line(DOT))
			c = '\n';
		else
			c = char_at(DOT);

		if (c == ch)
			++count;
		else if (c == ofence)
			--count;

		if (sdir == FORWARD)
			s = forwchar(FALSE, 1);
		else if (count == 0) {
			s = FALSE;
			break;		/* prevent backup past buffer top */
		} else
			s = backchar(FALSE, 1);

		if (s == FALSE || interrupted())
			return FALSE;
	}

	/* if count is zero, we have a match, move the sucker */
	if (count == 0) {
		if (s == TRUE) {
			if (sdir == FORWARD) {
				if (!doingopcmd || doingsweep) {
					backchar(FALSE, 1);
					sweephack = TRUE;
				}
			} else {
				forwchar(FALSE, 1);
			}
		}
		curwp->w_flag |= WFMOVE;
		return TRUE;
	}
	return FALSE;
}

static int
comment_fence(sdir)
int sdir;
{
	MARK comstartpos;
	int count = 1;
	int c, s = FALSE;

	/* set up for scan */
	if (sdir == REVERSE)
		backchar(FALSE, 1);
	else
		forwchar(FALSE, 1);

	comstartpos.l = null_ptr;

	while (count > 0) {
		if (is_at_end_of_line(DOT))
			c = '\n';
		else
			c = char_at(DOT);

		if (c == '/') {
			/* is it a comment-end? */
			if ( DOT.o > 0 && lGetc(DOT.l, DOT.o-1) == '*') {
				if ( sdir == FORWARD) {
					count = 0;
				} else {
					if (l_ref(comstartpos.l)) {
						DOT = comstartpos;
						count = 0;
					} else {
						return FALSE;
					}
				}
			}
			/* is it a comment start? */
			if ( sdir == REVERSE &&
				DOT.o < lLength(DOT.l)-1 &&
				lGetc(DOT.l, DOT.o+1) == '*') {
				/* remember where we are */
				comstartpos = DOT;
			}

		}

		if (sdir == FORWARD)
			s = forwchar(FALSE, 1);
		else
			s = backchar(FALSE, 1);

		if (s == FALSE) {
			if (l_ref(comstartpos.l)) {
				DOT = comstartpos;
				count = 0;
				break;
			}
			return FALSE;
		}

		if (interrupted())
			return FALSE;
	}

	/* if count is zero, we have a match, move the sucker */
	if (count == 0) {
		if (s == TRUE) {
			if (sdir == FORWARD) {
				if (!doingopcmd || doingsweep) {
					backchar(FALSE, 1);
					sweephack = TRUE;
				}
			} else {
				forwchar(FALSE, 1);
			}
		}
		curwp->w_flag |= WFMOVE;
		return TRUE;
	}
	return FALSE;
}

/* get the indent of the line containing the matching brace/paren. */
int
fmatchindent(c)
int c;
{
	int ind;
	    
	MK = DOT;
	    
	if (getfence(c,FORWARD) == FALSE) {
		(void)gomark(FALSE,1);
		return previndent((int *)0);
	}

	ind = indentlen(l_ref(DOT.l));

	(void)gomark(FALSE,1);
	    
	return ind;
}



/*	Close fences are matched against their partners, and if
	on screen the cursor briefly lights there		*/
void
fmatch(rch)
int rch;
{
	MARK	oldpos; 		/* original position */
	register LINE *toplp;	/* top line in current window */
	register int count; /* current fence level count */
	register char c;	/* current character in scan */
	int dir, lch;

	/* get the matching left-fence char, if it exists */
	lch = is_user_fence(rch, &dir);
	if (lch == 0 || dir != REVERSE)
		return;

	/* first get the display update out there */
	(void)update(FALSE);

	/* save the original cursor position */
	oldpos = DOT;

	/* find the top line and set up for scan */
	toplp = lBack(curwp->w_line.l);
	count = 1;
	backchar(TRUE, 2);

	/* scan back until we find it, or reach past the top of the window */
	while (count > 0 && l_ref(DOT.l) != toplp) {
		if (is_at_end_of_line(DOT))
			c = '\n';
		else
			c = char_at(DOT);
		if (c == rch)
			++count;
		if (c == lch)
			--count;
		if (backchar(FALSE, 1) != TRUE)
			break;
	}

	/* if count is zero, we have a match, display the sucker */
	if (count == 0) {
		forwchar(FALSE, 1);
		if (update(FALSE) == TRUE)
		/* the idea is to leave the cursor there for about a
			quarter of a second */
			catnap(300, FALSE);
	}

	/* restore the current position */
	DOT = oldpos;
}

#endif /* OPT_CFENCE */
