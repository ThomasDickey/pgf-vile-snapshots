/* 
 *
 *	fences.c
 *
 * Match up various fenceposts, like (), [], {}, */ /*, #if, #el, #en
 *
 * Most code probably by Dan Lawrence or Dave Conroy for MicroEMACS
 * Extensions for vile by Paul Fox
 *
 *	$Log: fences.c,v $
 *	Revision 1.4  1992/08/20 23:40:48  foxharp
 *	typo fixes -- thanks, eric
 *
 * Revision 1.3  1992/06/08  08:56:05  foxharp
 * fixed infinite loop if simple fence not found, and
 * suppressed beeping in input mode if fence not found
 * ,.
 * 
 *
 * Revision 1.2  1992/06/03  08:37:23  foxharp
 * removed nested comment
 *
 * Revision 1.1  1992/05/29  09:38:33  foxharp
 * Initial revision
 *
 *
 *
 */


#include	<stdio.h>
#include	"estruct.h"
#include	"edef.h"

#if CFENCE
/*	the cursor is moved to a matching fence */
int
matchfence(f,n)
int f,n;
{
	int s = getfence(0, (!f || n > 0) ? FORWARD:REVERSE);
	if (s == FALSE)
		TTbeep();
	return s;
}

int
matchfenceback(f,n)
int f,n;
{
	int s = getfence(0, (!f || n > 0) ? REVERSE:FORWARD);
	if (s == FALSE)
		TTbeep();
	return s;
}

int
getfence(ch,sdir)
int ch; /* fence type to match against */
int sdir; /* direction to scan if we're not on a fence to begin with */
{
	MARK	oldpos; 	/* original pointer */
	register int ofence = 0;	/* open fence */
	int s, i;
	char *otherkey = NULL, *key = NULL;
	char *lookingforkey = NULL;

	/* save the original cursor position */
	oldpos = DOT;

	/* ch may have been passed, if being used internally */
	if (!ch) {
		if (DOT.o == 0 && (ch = char_at(DOT)) == '#') {
			if (llength(DOT.l) < 3)
				return FALSE;
		} else if ((ch = char_at(DOT)) == '/' || ch == '*') {
			/* EMPTY */;
		} else if (sdir == FORWARD) {
			/* get the current character */
			if (oldpos.o < llength(oldpos.l)) {
				do {
					ch = char_at(oldpos);
				} while(!isfence(ch) &&
					++oldpos.o < llength(oldpos.l));
			}
			if (is_at_end_of_line(oldpos)) {
				return FALSE;
			}
		} else {
			/* get the current character */
			if (oldpos.o >= 0) {
				do {
					ch = char_at(oldpos);
				} while(!isfence(ch) && --oldpos.o >= 0);
			}

			if (oldpos.o < 0) {
				return FALSE;
			}
		}

		/* we've at least found a fence -- move us that far */
		DOT.o = oldpos.o;
	}

	/* setup proper matching fence */
	switch (ch) {
		case '(':
			ofence = ')';
			sdir = FORWARD;
			break;
		case ')':
			ofence = '(';
			sdir = REVERSE;
			break;
		case LBRACE:
			ofence = RBRACE;
			sdir = FORWARD;
			break;
		case RBRACE:
			ofence = LBRACE;
			sdir = REVERSE;
			break;
		case '[':
			ofence = ']';
			sdir = FORWARD;
			break;
		case ']':
			ofence = '[';
			sdir = REVERSE;
			break;
		case '#':
			for (i = 1; i < llength(DOT.l) &&
				isspace(lgetc(DOT.l,i)); i++)
				;
			if (i + 2 <= llength(DOT.l)) {
				if (!strncmp("if",
						&DOT.l->l_text[i], 2)) {
					sdir = FORWARD;
					key = "if";
					otherkey = "en";
					lookingforkey = "el";
					break;
				} else if (!strncmp("el",
						&DOT.l->l_text[i], 2)) {
					if (sdir == FORWARD) {
						key = "if";
						lookingforkey =
							otherkey = "en";
					} else {
						key = "en";
						lookingforkey = 
							otherkey = "if";
					}
					break;
				} else if (!strncmp("en",
						&DOT.l->l_text[i], 2)) {
					sdir = REVERSE;
					key = "en";
					lookingforkey = otherkey = "if";
					break;
				}
			}
			return FALSE;
		case '*':
			ch = '/';
			if (DOT.o+1 < llength(DOT.l) &&
				(lgetc(DOT.l, DOT.o+1) == '/')) {
				sdir = REVERSE;
				forwchar(TRUE,1);
				break;
			} else if (DOT.o > 0 &&
				(lgetc(DOT.l, DOT.o-1) == '/')) {
				sdir = FORWARD;
				backchar(TRUE,1);
				if (doingopcmd)
					pre_op_dot = DOT;
				break;
			}
			return FALSE;
		case '/':
			if (DOT.o+1 < llength(DOT.l) &&
				(lgetc(DOT.l, DOT.o+1) == '*')) {
				sdir = FORWARD;
				break;
			} else if (DOT.o > 0 &&
				(lgetc(DOT.l, DOT.o-1) == '*')) {
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

	if (lookingforkey != NULL) {  /* we're searching for a cpp keyword */
		s = cpp_fence(sdir,key,otherkey,lookingforkey);
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

int
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
		else
			s = backchar(FALSE, 1);

		if (s == FALSE || interrupted)
			return FALSE;
	}

	/* if count is zero, we have a match, move the sucker */
	if (count == 0) {
		if (s == TRUE) {
			if (sdir == FORWARD) {
				if (!doingopcmd)
					backchar(FALSE, 1);
			} else {
				forwchar(FALSE, 1);
			}
		}
		curwp->w_flag |= WFMOVE;
		return TRUE;
	}
	return FALSE;
}

int
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

	comstartpos.l = NULL;

	while (count > 0) {
		if (is_at_end_of_line(DOT))
			c = '\n';
		else
			c = char_at(DOT);

		if (c == '/') {
			/* is it a comment-end? */
			if ( DOT.o > 0 && lgetc(DOT.l, DOT.o-1) == '*') {
				if ( sdir == FORWARD) {
					count = 0;
				} else {
					if (comstartpos.l) {
						DOT = comstartpos;
						count = 0;
					} else {
						return FALSE;
					}
				}
			}
			/* is it a comment start? */
			if ( sdir == REVERSE &&
				DOT.o < llength(DOT.l)-1 &&
				lgetc(DOT.l, DOT.o+1) == '*') {
				/* remember where we are */
				comstartpos = DOT;
			}

		}

		if (sdir == FORWARD)
			s = forwchar(FALSE, 1);
		else
			s = backchar(FALSE, 1);

		if (s == FALSE) {
			if (comstartpos.l) {
				DOT = comstartpos;
				count = 0;
				break;
			}
			return FALSE;
		}

		if (interrupted)
			return FALSE;
	}

	/* if count is zero, we have a match, move the sucker */
	if (count == 0) {
		if (s == TRUE) {
			if (sdir == FORWARD) {
				if (!doingopcmd && s)
					backchar(FALSE, 1);
			} else {
				forwchar(FALSE, 1);
			}
		}
		curwp->w_flag |= WFMOVE;
		return(TRUE);
	}
	return FALSE;
}

int
cpp_fence(sdir,key,otherkey,lookingforkey)
int sdir;
char *key, *otherkey, *lookingforkey;
{
	int count = 1;
	int i;

	/* set up for scan */
	if (sdir == REVERSE)
		DOT.l = lback(DOT.l);
	else
		DOT.l = lforw(DOT.l);
	while (count > 0 && !is_header_line(DOT, curbp)) {
		for (i = 1; i < llength(DOT.l) && 
			isspace(lgetc(DOT.l,i)); i++)
			;
		if (llength(DOT.l) >= i + 2 && char_at(DOT) == '#') {

			if (!strncmp(key, &DOT.l->l_text[i], 2))
				++count;
			else if (!strncmp(otherkey,
						&DOT.l->l_text[i], 2))
				--count;
			if (count == 1 && 
				strcmp(otherkey, lookingforkey) && 
				!strncmp(lookingforkey,
						&DOT.l->l_text[i], 2)) {
				
				count = 0;
				break;
			}
		}

		if (count == 0)
			break;

		if (sdir == REVERSE)
			DOT.l = lback(DOT.l);
		else
			DOT.l = lforw(DOT.l);

		if (is_header_line(DOT,curbp) || interrupted)
			return FALSE;
	}
	if (count == 0) {
		curwp->w_flag |= WFMOVE;
		if (doingopcmd)
			fulllineregions = TRUE;
		return TRUE;
	}
	return FALSE;
}

/* get the indent of the line containing the matching brace. */
int
fmatchindent()
{
	int ind;
	    
	MK = DOT;
	    
	if (getfence(RBRACE,FORWARD) == FALSE) {
		gomark(FALSE,1);
		return previndent(NULL);
	}

	ind = indentlen(DOT.l);

	gomark(FALSE,1);
	    
	return ind;
}



/*	Close fences are matched against their partners, and if
	on screen the cursor briefly lights there		*/
int
fmatch(ch)
int ch;	/* fence type to match against */
{
	MARK	oldpos; 		/* original position */
	register LINE *toplp;	/* top line in current window */
	register int count; /* current fence level count */
	register char opench;	/* open fence */
	register char c;	/* current character in scan */

	/* first get the display update out there */
	update(FALSE);

	/* save the original cursor position */
	oldpos = DOT;

	/* setup proper open fence for passed close fence */
	if (ch == ')')
		opench = '(';
	else if (ch == RBRACE)
		opench = LBRACE;
	else
		opench = '[';

	/* find the top line and set up for scan */
	toplp = lback(curwp->w_line.l);
	count = 1;
	backchar(TRUE, 2);

	/* scan back until we find it, or reach past the top of the window */
	while (count > 0 && DOT.l != toplp) {
		if (is_at_end_of_line(DOT))
			c = '\n';
		else
			c = char_at(DOT);
		if (c == ch)
			++count;
		if (c == opench)
			--count;
		if (backchar(FALSE, 1) != TRUE)
			break;
	}

	/* if count is zero, we have a match, display the sucker */
	/* there is a real machine dependent timing problem here we have
	   yet to solve......... */
	if (count == 0) {
		forwchar(FALSE, 1);
		update(FALSE);
		/* the idea is to leave the cursor there for about a
			quarter of a second */
		catnap(250);
	}

	/* restore the current position */
	DOT = oldpos;

	return(TRUE);
}

#endif /* CFENCE */
