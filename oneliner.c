/*
 *	A few functions that used to operate on single whole lines, mostly
 *	here to support the globals() function.  They now work on regions.
 *	Written (except for delins()) for vile by Paul Fox, (c)1990
 *
 * $Log: oneliner.c,v $
 * Revision 1.28  1992/03/13 08:12:11  pgf
 * attempt to use args passed after :& command, as in ":&g"
 *
 * Revision 1.27  1992/03/03  08:42:59  pgf
 * skip a char when doing /g if the pattern can or does match nothing
 *
 * Revision 1.26  1992/03/01  18:41:31  pgf
 * /g finds non-overlapping matches, and in addition, must skip a character
 * if a pattern can find nothing
 *
 * Revision 1.25  1992/02/26  21:56:36  pgf
 * fixed inf. loop caused by s/$/string/g
 *
 * Revision 1.24  1992/01/22  16:58:20  pgf
 * substline() now always returns true except on hard failure -- this makes
 * substreg work properly, instead of quitting on the first line not containing
 * a pattern
 *
 * Revision 1.23  1992/01/05  00:06:13  pgf
 * split mlwrite into mlwrite/mlprompt/mlforce to make errors visible more
 * often.  also normalized message appearance somewhat.
 *
 * Revision 1.22  1992/01/03  23:31:49  pgf
 * use new ch_fname() to manipulate filenames, since b_fname is now
 * a malloc'ed sting, to avoid length limits
 *
 * Revision 1.21  1991/12/22  12:03:09  pgf
 * bug fix for global subst, and allow aborts at the "g, p, or n" prompt
 *
 * Revision 1.20  1991/11/16  18:37:27  pgf
 * use memcpy instead of strncpy
 *
 * Revision 1.19  1991/11/04  14:29:22  pgf
 * extra caution in delins
 *
 * Revision 1.18  1991/11/03  17:46:30  pgf
 * removed f,n args from all region functions -- they don't use them,
 * since they're no longer directly called by the user
 *
 * Revision 1.17  1991/11/01  14:38:00  pgf
 * saber cleanup
 *
 * Revision 1.16  1991/11/01  14:10:35  pgf
 * regexps are now relocatable, so the subst commands can keep a copy
 * of their pattern around
 *
 * Revision 1.15  1991/10/31  02:37:02  pgf
 * implement vi-like substagain
 *
 * Revision 1.14  1991/10/30  14:55:43  pgf
 * fixed boundary position in substline, renamed thescanner to scanner
 *
 * Revision 1.13  1991/10/29  14:35:29  pgf
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
pregion(flag)
int flag;
{
	register WINDOW *wp;
	register BUFFER *bp;
	register int	s;
	REGION		region;
	static char bname[] = "[p-lines]";
	register LINE *linep;

	fulllineregions = TRUE;
	        
	if ((s=getregion(&region)) != TRUE)
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
	ch_fname(bp, "");

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

llineregion()
{
	return pregion(PLIST);
}

plineregion()
{
	return pregion(0);
}

static regexp *substexp;

substregion()
{
	return substreg1(TRUE,TRUE);
}

subst_again_region()
{
	return substreg1(FALSE,TRUE);
}

/* traditional vi & command */
/* ARGSUSED */
subst_again(f,n)
int f,n;
{
	int s;
	MARK curpos;

	curpos = DOT;

	/* the region spans just the line */
	MK.l = DOT.l;
	DOT.o = 0;
	MK.o = llength(MK.l);
	s = substreg1(FALSE,FALSE);
	if (s != TRUE) {
		mlforce("[No match.]");
		DOT = curpos;
		return s;
	}
	swapmark();
	return TRUE;
}

substreg1(needpats, use_opts)
int needpats, use_opts;
{
	int c, s;
	static int printit, globally, nth_occur;
	REGION region;
	LINE *oline;

	fulllineregions = TRUE;
	        
	if ((s=getregion(&region)) != TRUE)
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
		if ((s = readpattern("substitute pattern: ", &pat[0],
					&gregexp, c, FALSE)) != TRUE) {
			if (s != ABORT)
				mlforce("[No pattern.]");
			return FALSE;
		}

		if (gregexp) {
			if (substexp)
				free(substexp);
			substexp = (regexp *)malloc(gregexp->size);
			memcpy(substexp, gregexp, gregexp->size);
		}

		if ((s = readpattern("replacement string: ", &rpat[0], NULL, c,
				FALSE)) != TRUE) {
			if (s == ABORT)
				return FALSE;
			/* else the pattern is null, which is okay... */
		}
		nth_occur = -1;
		printit = globally = FALSE;
		if (lastkey == c) { /* the user may have something to add */
			char buf[3];
			char *bp;
		getopts:
			bp = buf;
			buf[0] = 0;
			s = mlreply(
	"(g)lobally or ([1-9])th occurrence on line and/or (p)rint result: ",
				buf, sizeof buf);
			if (s == ABORT)
				return FALSE;
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
					mlforce("[Unknown action %s]",buf);
					return FALSE;
				}
				bp++;
			}
			if (!nth_occur)
				nth_occur = -1;
		}
	} else {
		if (!use_opts) {
			nth_occur = -1;
			printit = globally = FALSE;
		} else {
			if (isnamedcmd && lastkey != '\r') {
				tungetc(lastkey);
				nth_occur = -1;
				printit = globally = FALSE;
				goto getopts;
			}
		}
	}

	DOT.l = region.r_orig.l;	    /* Current line.	    */

	do {
		oline = DOT.l;
		if ((s = substline( substexp, nth_occur, printit, globally))
								!= TRUE)
			return s;
		DOT.l = lforw(oline);
	} while (!sameline(DOT, region.r_end));
	calledbefore = TRUE;
	return TRUE;
}

substline(exp, nth_occur, printit, globally)
regexp *exp;
int nth_occur, printit, globally;
{
	MARK eol;
	int foundit;
	register int s;
	register int which_occur = 0;
	int matched_at_eol = FALSE;
	extern MARK scanboundpos;

	/* if the "magic number" hasn't been set yet... */
	if (!exp || UCHARAT(exp->program) != REGEXP_MAGIC) {
		mlforce("[No pattern set yet]");
		return FALSE;
	}

	ignorecase = b_val(curwp->w_bufp, MDIGNCASE);

	foundit = FALSE;
	eol.l = DOT.l;
	eol.o = llength(DOT.l);
	scanboundpos = eol;
	DOT.o = 0;
	do {
		s = scanner(exp, FORWARD, FALSE);
		if (s != TRUE)
			break;
		        
		/* found the pattern */
		foundit = TRUE;
		which_occur++;
		if (nth_occur == -1 || which_occur == nth_occur) {
			setmark();
			/* only allow one match at the end of line, to
				prevent loop with s/$/x/g  */
			if (MK.o == llength(DOT.l)) {
				if (matched_at_eol)
					break;
				matched_at_eol = TRUE;
			}
			s = delins(exp, &rpat[0]);
			if (s != TRUE)
				return s;
			if ((exp->mlen == 0 || exp->regmlen == 0) &&
					forwchar(TRUE,1) == FALSE)
				break;
			if (nth_occur > 0)
				break;
		} else { /* non-overlapping matches */
			s = forwchar(TRUE, exp->mlen);
			if (s != TRUE)
				return s;
		}
	} while (globally && sameline(eol,DOT));
	if (foundit && printit) {
		register WINDOW *wp = curwp;
		setmark();
		s = plineregion();
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

/*
 - delins - perform substitutions after a regexp match
 */
delins(exp, source)
regexp *exp;
char *source;
{
	register char *src;
	register int dlength;
	register char c;
	register int no;
	static char *buf = NULL;
	static buflen = -1;

	if (exp == NULL || source == NULL) {
		mlforce("BUG: NULL parm to delins");
		return FALSE;
	}
	if (UCHARAT(exp->program) != REGEXP_MAGIC) {
		regerror("damaged regexp fed to delins");
		return FALSE;
	}

	dlength = exp->mlen;

	if (buf == NULL || dlength + 1 > buflen) {
		if (buf)
			free(buf);
		if ((buf = (char *)malloc(dlength+1)) == NULL) {
			mlforce("[Out of memory in delins]");
			return FALSE;
		}
		buflen = dlength + 1;
	}

	memcpy(buf, exp->startp[0], dlength);
	buf[dlength] = '\0';

	if (ldelete((long) dlength, FALSE) != TRUE) {
		mlforce("[Error while deleting]");
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
				mlforce("[Out of memory while inserting]");
				return FALSE;
			}
			continue;
		}

		if (exp->startp[no] != NULL && exp->endp[no] != NULL) {
			char *cp;
			int len;
			len = (exp->endp[no] - exp->startp[no]);
			cp = (exp->startp[no] - exp->startp[0]) + buf;
			while (len--) {
				if ((*cp == '\n' ? lnewline() : linsert(1, *cp)) != TRUE) {
					goto nomem;
				}
				if (!*cp) {
					mlforce("BUG: mangled replace");
					return FALSE;
				}
				cp++;
			}
		}
	}
	return TRUE;
}