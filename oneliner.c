/*
 *	A few functions that used to operate on single whole lines, mostly
 *	here to support the globals() function.  They now work on regions.
 *	Written (except for delins()) for vile by Paul Fox, (c)1990
 *
 * $Log: oneliner.c,v $
 * Revision 1.13  1991/10/29 14:35:29  pgf
 * implemented the & commands: substagain
 *
 * Revision 1.12  1991/10/27  01:50:16  pgf
 * switched from regex to regexp, and
 * used Spencer's regsub routine as the basis for a new delins()
 * that allows for replacement metachars
 *
 * Revision 1.11  1991/10/24  13:05:52  pgf
 * conversion to new regex package -- much faster
 *
 * Revision 1.10  1991/10/23  14:17:36  pgf
 * initialize nth_occur in all cases
 *
 * Revision 1.9  1991/09/26  13:11:19  pgf
 * LIST mode moved to the window
 *
 * Revision 1.8  1991/09/16  23:48:30  pgf
 * added "nth occurrence" support to s/repl/pat
 *
 * Revision 1.7  1991/09/10  00:55:24  pgf
 * don't re-popup the buffer everytime -- only on first call
 *
 * Revision 1.6  1991/08/07  12:35:07  pgf
 * added RCS log messages
 *
 * revision 1.5
 * date: 1991/08/06 15:24:19;
 *  global/local values
 * 
 * revision 1.4
 * date: 1991/06/27 18:33:47;
 * made screen oriented substitutes always act globally across line
 * 
 * revision 1.3
 * date: 1991/06/25 19:53:07;
 * massive data structure restructure
 * 
 * revision 1.2
 * date: 1991/05/31 11:14:50;
 * turned these into region operators.
 * they're not one-liners anymore
 * 
 * revision 1.1
 * date: 1990/09/21 10:25:51;
 * initial vile RCS revision
 */

#include	"estruct.h"
#include	"edef.h"
#include	<stdio.h>

#define PLIST	0x01

/*
 * put lines in a popup window
 */
pregion(f, n, flag)
{
	register WINDOW *wp;
	register BUFFER *bp;
	register int	s;
	REGION		region;
	static char bname[] = "[p-lines]";
	register LINE *linep;
	int cb;

	fulllineregions = TRUE;
	        
	if ((s=getregion(&region,NULL)) != TRUE)
		return (s);

	linep = region.r_orig.l;		 /* Current line.	 */
	        
	/* first check if we are already here */
	bp = bfind(bname, OK_CREAT, 0);
	if (bp == NULL)
		return FALSE;

	if (!calledbefore) {		/* fresh start */
		/* bring p-lines up */
		if (popupbuff(bp) != TRUE)
			return FALSE;
	        
		bclear(bp);
		make_local_b_val(bp,WMDLIST);
		set_b_val(bp, WMDLIST, ((flag & PLIST) != 0) );
		calledbefore = TRUE;
	}
        
	do {
		addline(bp,linep->l_text,llength(linep));
		linep = lforw(linep);
	} while (linep != region.r_end.l);

	bp->b_flag &= ~BFCHG;
        
	strcpy(bp->b_bname,bname);
	strcpy(bp->b_fname, "");

	make_local_b_val(bp,MDVIEW);
	set_b_val(bp,MDVIEW,TRUE);

	make_local_b_val(bp,VAL_TAB);
	set_b_val(bp,VAL_TAB,tabstop_val(curbp));

	bp->b_active = TRUE;
	for (wp=wheadp; wp!=NULL; wp=wp->w_wndp) {
		if (wp->w_bufp == bp) {
			wp->w_flag |= WFMODE|WFFORCE;
			wp->w_traits.w_vals = bp->b_wtraits.w_vals;
		}
	}
	return TRUE;
}

llineregion(f,n)
{
	return pregion(f,n,PLIST);
}

plineregion(f,n)
{
	return pregion(f,n,0);
}

substregion(f,n)
{
	return substreg1(f,n,TRUE);
}

subst_again_region(f,n)
{
	return substreg1(f,n,FALSE);
}

substreg1(f,n,needpats)
{
	int c, s;
	static int printit, globally, nth_occur;
	REGION region;
	LINE *oline;

	fulllineregions = TRUE;
	        
	if ((s=getregion(&region,NULL)) != TRUE)
		return (s);

	if (calledbefore == FALSE && needpats) {
		c = '\n';
		if (isnamedcmd) {
			c = tpeekc();
			if (c < 0) {
				c = '\n';
			} else {
				if (ispunct(c)) {
					(void)kbd_key();
				}
			}
		} else {
			/* if it's a screen region, assume they want .../g */
			globally = TRUE;
		}
		if ((s = readpattern("substitute pattern: ", &pat[0], TRUE, c,
				FALSE)) != TRUE) {
			if (s != ABORT)
				mlwrite("No pattern.");
			return FALSE;
		}
		if ((s = readpattern("replacement string: ", &rpat[0], FALSE, c,
				FALSE)) != TRUE) {
			if (s == ABORT)
				return FALSE;
			/* else the pattern is null, which is okay... */
		}
		nth_occur = -1;
		if (lastkey == c) { /* the user may have something to add */
			char buf[3];
			char *bp = buf;
			buf[0] = 0;
			mlreply(
	"(g)lobally or ([1-9])th occurrence on line and/or (p)rint result: ",
				buf, sizeof buf);
			printit = globally = FALSE;
			nth_occur = 0;
			while (*bp) {
				if (*bp == 'p' && !printit) {
					printit = TRUE;
				} else if (*bp == 'g' &&
						!globally && !nth_occur) {
					globally = TRUE;
				} else if (isdigit(*bp) &&
						!nth_occur && !globally) {
					nth_occur = *bp - '0';
					globally = TRUE;
				} else if (!isspace(*bp)) {
					mlwrite("Unknown action %s",buf);
					return FALSE;
				}
				bp++;
			}
			if (!nth_occur)
				nth_occur = -1;
		}
	}


	DOT.l = region.r_orig.l;	    /* Current line.	    */

	ignorecase = b_val(curwp->w_bufp, MDIGNCASE);

	do {
		oline = DOT.l;
		if ((s = substline(nth_occur, printit, globally)) != TRUE)
			return s;
		DOT.l = lforw(oline);
	} while (!sameline(DOT, region.r_end));
	calledbefore = TRUE;
	return TRUE;
}

substline(nth_occur, printit, globally)
int nth_occur, printit, globally;
{
	MARK tdot;
	int foundit;
	register int s;
	register int which_occur = 0;
	foundit = FALSE;
	tdot.l = DOT.l;
	tdot.o = llength(DOT.l);
	setboundry(TRUE, tdot, FORWARD);
	DOT.o = 0;
	do {
		s = thescanner(gregexp, FORWARD, FALSE);
		if (s != TRUE)
			break;
		        
		/* found the pattern */
		foundit = TRUE;
		which_occur++;
		if (nth_occur == -1 || which_occur == nth_occur) {
			s = delins(matchlen, &rpat[0]);
			if (s != TRUE)
				return s;
			if (nth_occur > 0)
				break;
		} else {
			s = forwchar(TRUE,matchlen);
			if (s != TRUE)
				return s;
		}
	} while (sameline(tdot,DOT) /* !boundry(DOT) */ && globally);
	if (foundit && printit) {
		register WINDOW *wp = curwp;
		setmark();
		s = plineregion(FALSE,1);
		if (s != TRUE) return s;
		/* back to our buffer */
		swbuffer(wp->w_bufp);
	}
	return TRUE;
}

/*
 * delins, modified by pgf from regsub
 *
 *	Copyright (c) 1986 by University of Toronto.
 *	Written by Henry Spencer.  Not derived from licensed software.
 *
 *	Permission is granted to anyone to use this software for any
 *	purpose on any computer system, and to redistribute it freely,
 *	subject to the following restrictions:
 *
 *	1. The author is not responsible for the consequences of use of
 *		this software, no matter how awful, even if they arise
 *		from defects in it.
 *
 *	2. The origin of this software must not be misrepresented, either
 *		by explicit claim or by omission.
 *
 *	3. Altered versions must be plainly marked as such, and must not
 *		be misrepresented as being the original software.
 */

#ifndef CHARBITS
#define	UCHARAT(p)	((int)*(unsigned char *)(p))
#else
#define	UCHARAT(p)	((int)*(p)&CHARBITS)
#endif

/*
 - delins - perform substitutions after a regexp match
 */
delins(dlength, source)
int dlength;
char *source;
{
	register char *src;
	register char c;
	register int no;
	register int len;
	register char *buf;
	extern char *strncpy();

	if (gregexp == NULL || source == NULL) {
		mlwrite("BUG: NULL parm to delins");
		return;
	}
	if (UCHARAT(gregexp->program) != REGEXP_MAGIC) {
		regerror("damaged regexp fed to delins");
		return;
	}

	if ((buf = (char *)malloc(dlength+1)) == NULL) {
		mlwrite("Out of memory in delins");
		return FALSE;
	}

	strncpy(buf, gregexp->startp[0], dlength);
	buf[dlength] = '\0';

	if (ldelete((long) dlength, FALSE) != TRUE) {
		mlwrite("Error while deleting");
		free(buf);
		return FALSE;
	}
	src = source;
	while ((c = *src++) != '\0') {
		if (c == '&')
			no = 0;
		else if (c == '\\' && '0' <= *src && *src <= '9')
			no = *src++ - '0';
		else {
			if ((c == '\n'? lnewline(): linsert(1, c)) != TRUE) {
			nomem:
				mlwrite("Out of memory while inserting");
				free(buf);
				return FALSE;
			}
			continue;
		}

		if (gregexp->startp[no] != NULL && gregexp->endp[no] != NULL) {
			char *cp;
			int len;
			len = (gregexp->endp[no] - gregexp->startp[no]);
			cp = (gregexp->startp[no] - gregexp->startp[0]) + buf;
			while (len--) {
				if ((*cp == '\n' ? lnewline() : linsert(1, *cp)) != TRUE) {
					goto nomem;
				}
				if (!*cp) {
					mlwrite("BUG: mangled replace");
					free(buf);
					return FALSE;
				}
				cp++;
			}
		}
	}
	free(buf);
	return TRUE;

}
