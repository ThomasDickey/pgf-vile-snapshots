/* Find the next error in mentioned in the shell output window.
 * written for vile: Copyright (c) 1990, 1995 by Paul Fox
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/finderr.c,v 1.44 1995/02/24 00:35:23 pgf Exp $
 *
 */

#include "estruct.h"
#include "edef.h"

#if OPT_FINDERR

static	char febuff[NBUFN+1];	/* name of buffer to find errors in */
static	unsigned newfebuff;	/* is the name new since last time? */

void
set_febuff(name)
char	*name;
{
	(void)strncpy0(febuff, name, (SIZE_T)(NBUFN+1));
	newfebuff = TRUE;
}

/* edits the file and goes to the line pointed at by the next compiler
        error in the "[output]" window.  It unfortunately doesn't mark
        the lines for you, so adding lines to the file throws off the
        later numbering.  Solutions to this seem messy at the moment */

/* ARGSUSED */
int
finderr(f,n)
int f,n;
{
	register BUFFER *sbp;
	register int s;
	struct LINE *dotp;
	int moveddot = FALSE;
	char	*text;

	int errline;
	char errfile[NFILEN];
	char ferrfile[NFILEN];
	char verb[32];

	static int oerrline = -1;
	static char oerrfile[256];

#if SYS_APOLLO
	static	char	lint_fmt1[] = "%32[^( \t] \t%[^(](%d)";
	static	char	lint_fmt2[] = "%32[^(]( arg %d ) \t%32[^( \t](%d) :: %[^(](%d))";
	int	num1, num2;
	char	nofile[NFILEN];
#endif

#define DIRLEVELS 20
	static int l = 0;
	static char *dirs[DIRLEVELS];

	/* look up the right buffer */
	if ((sbp = find_b_name(febuff)) == NULL) {
		mlforce("[No buffer to search for errors.]");
		return(FALSE);
	}
 
	if (newfebuff) {
		oerrline = -1;
		oerrfile[0] = EOS;
		while (l)
			free(dirs[l--]);
	}
	newfebuff = FALSE;

	dotp = getdot(sbp);

	for(;;) {
		/* to use this line, we need both the filename and the line
			number in the expected places, and a different line
			than last time */
		/* accepts error forms */
		/*  (could probably be reduced/simplified -- maybe even 
			regexps could be used. */
		/*
			"file.c", line 223: error....
			or
			file.c: 223: error....
			or
			******** Line 187 of "filec.c"
		*/
		if (lisreal(dotp)) {
			static	TBUFF	*tmp;
#if SYS_SUNOS
			char *t;
#endif

			if (tb_init(&tmp, EOS) != 0
			 && tb_bappend(&tmp, dotp->l_text,
			 	(ALLOC_T)llength(dotp)) != 0
			 && tb_append(&tmp, EOS) != 0)
			 	text = tb_values(tmp);
			else
				break;	/* out of memory */

			if ( (
			/* "file.c", line 223: error.... */
			      sscanf(text,
				"\"%[^\" \t]\", line %d:",
				errfile, &errline) == 2

			/* file.c: 223: error....	*/
			  ||  sscanf(text,
				"%[^: \t]: %d:",
				errfile, &errline) == 2

#if SYS_APOLLO		 	/* C compiler */
			  ||  sscanf(text,
				"%32[*] Line %d of \"%[^\" \t]\"",
				verb, &errline, errfile) == 3
				/* sys5 lint */
			  ||  (!strncmp(text, "    ", 4)
			    && ((sscanf(text+4, lint_fmt1,
			    	verb, errfile, &errline) == 3)
			     || sscanf(text+4, lint_fmt2,
			     	verb, &num1, nofile, &num2,
						errfile, &errline) == 6))
			  	/* C++ compiler */
			  ||  sscanf(text,
			  	"CC: \"%[^\"]\", line %d",
						errfile, &errline) == 2
#endif

#if defined(__hpux)			/* HP/UX C compiler */
/* 	compiler-name: "filename", line line-number ...	*/
			  ||  sscanf(text,
			  	"%*s: \"%[^\"]\", line %d",
						errfile, &errline) == 2
#endif
/* 	ultrix, sgi, osf1 (alpha only?)  use:			*/
/* 	compiler-name: Error: filename, line line-number ...	*/
			  ||  sscanf(text,
			  	"%*s %*s %[^, \t], line %d",
						errfile, &errline) == 2
#if SYS_SUNOS
			  ||  sscanf(text,
			  	"%[^:( \t](%d):",  /* ) */
				errfile, &errline) == 2
			  ||  ((t = strstr(text, "  ::  ")) != 0
			    && sscanf(t,
			  	"  ::  %[^( \t](%d)", /* ) */
				errfile, &errline) == 2)
#endif
#if CC_TURBO
			  ||  sscanf(text,
			  	"Error %[^ ] %d:",
				errfile, &errline) == 2
			  ||  sscanf(text,
			  	"Warning %[^ ] %d:",
				errfile, &errline) == 2
#endif
				) && (oerrline != errline ||
					strcmp(errfile,oerrfile))) {
				break;
			} else if (sscanf(text,
			"%*[^ \t:`]: %32[^ \t`] directory `", verb) == 1 ) {
				if (!strcmp(verb, "Entering")) {
					if (l < DIRLEVELS) {
						char *e,*d;
						/* find the start of path */
						d = strchr(text, '`');
						if (!d)
							continue;
						d += 1;
						/* find the term. quote */
						e = strchr(d, '\'');
						if (!e)
							continue;
						/* put a null in its place */
						*e = EOS;
						/* make a copy */
						dirs[++l] = strmalloc(d);
						/* put the quote back */
						*e = '\'';
					}
				} else if (!strcmp(verb, "Leaving")) {
					if (l > 0)
						free(dirs[l--]);
				}
			}
		}

		if (lforw(dotp) == l_ref(buf_head(sbp))) {
			mlwarn("[No more errors in %s buffer]", febuff);
			/* start over at the top of file */
			putdotback(sbp, lForw(buf_head(sbp)));
			while (l)
				free(dirs[l--]);
			return FALSE;
		}
		dotp = lforw(dotp);
		moveddot = TRUE;
	}
	/* put the new dot back, before possible changes to contents
				of current window from getfile() */
	if (moveddot)
		putdotback(sbp,dotp);

	(void)pathcat(ferrfile, dirs[l], errfile);

	if (!eql_bname(curbp, ferrfile) &&
		strcmp(ferrfile,curbp->b_fname)) {
		/* if we must change windows */
		struct WINDOW *wp;
		for_each_window(wp) {
			if (eql_bname(wp->w_bufp, ferrfile)
			 || !strcmp(wp->w_bufp->b_fname,ferrfile))
				break;
		}
		if (wp) {
			curwp = wp;
			make_current(curwp->w_bufp);
			upmode();
		} else {
			s = getfile(ferrfile,TRUE);
			if (s != TRUE)
				return s;
		}
	}

	mlforce("Error: %*S", dotp->l_used, dotp->l_text);

	/* it's an absolute move */
	curwp->w_lastdot = DOT;
	s = gotoline(TRUE, -(curbp->b_linecount - errline + 1));

	oerrline = errline;
	(void)strcpy(oerrfile,errfile);

	return s;
}

struct LINE *
getdot(bp)
struct BUFFER *bp;
{
	register WINDOW *wp;
	if (bp->b_nwnd) {
		/* scan for windows holding that buffer,
					pull dot from the first */
		for_each_window(wp) {
			if (wp->w_bufp == bp) {
				return l_ref(wp->w_dot.l);
			}
		}
	}
	return l_ref(bp->b_dot.l);
}

void
putdotback(bp,dotp)
struct BUFFER *bp;
struct LINE *dotp;
{
	register WINDOW *wp;

	if (bp->b_nwnd) {
		for_each_window(wp) {
			if (wp->w_bufp == bp) {
				wp->w_dot.l = l_ptr(dotp);
				wp->w_dot.o = 0;
				wp->w_flag |= WFMOVE;
			}
		}
		return;
	}
	/* then the buffer isn't displayed */
	bp->b_dot.l = l_ptr(dotp);
	bp->b_dot.o = 0;
}

/*
 * Ask for a new finderr buffer name
 */
/* ARGSUSED */
int
finderrbuf(f, n)
int f,n;
{
	register int    s;
	char name[NFILEN+1];
	BUFFER *bp;

	(void)strcpy(name, febuff);
	if ((s = mlreply("Buffer to scan for \"errors\": ", name, sizeof(name))) == ABORT)
		return s;
	if (s == FALSE) {
		set_febuff(OUTPUT_BufName);
	} else {
		if ((bp = find_any_buffer(name)) == 0)
			return FALSE;
		set_febuff(get_bname(bp));
	}
	return TRUE;
}
#endif
