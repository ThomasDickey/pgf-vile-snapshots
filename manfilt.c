/*
 * manfilt.c		-- replace backspace sequences with attribute
 *			   information for vile
 *
 * Author: Kevin A. Buettner
 * Creation: 4/17/94
 * 
 * This program filters backspace sequences often found in manual pages
 * for vile/xvile.  Backspace sequences representing italicized or bold
 * text are fixed up by removing the backspaces, underlines, and duplicate
 * characters (leaving just the text as it should appear on the screen).
 * Attributed text is so indicated by inserting a Cntrl-A sequence in front
 * of the text to be attributed.  These Cntrl-A sequences take the following
 * form:
 *   	^A<Count><Attr>:
 *
 * <Count> is a sequence of digits representing the number of characters
 * following the ':' to be attributed.
 *
 * <Attr> is a sequence of characters which indicates how to attribute the
 * characters following the ':'.  The following characters are presently
 * recognized by vile:
 *    	
 *	'I'	-- italic
 *	'B'	-- bold
 *	'U'	-- underline
 *	'R'	-- reverse video
 *
 * Examples:
 *	Before					After
 *	------					-----
 *	_^Hi_^Ht_^Ha_^Hl_^Hi_^Hc		^A6I:italic
 *
 *	b^Hbo^Hol^Hld^Hd			^A4B:bold
 *
 *	_^HB^HB_^Ho^Ho_^Ht^Ht_^Hh^Hh		^A4IB:Both
 *
 * On many system, bold sequences are actually quite a bit longer.  On
 * some systems, the repeated character is repeated as many as four times.
 * Thus the letter "B" would be represented as B^HB^HB^HB.
 *
 * For comparison, see the description of the BSD 'col' program (for
 * information about the types of escape sequences that might be emitted by
 * nroff).
 *
 * vile will choose some appropriate fallback (such as underlining) if
 * italics are not available.
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/manfilt.c,v 1.14 1995/11/21 02:29:34 pgf Exp $
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef ANSI_PROTOS
# if defined(__STDC__)
#  define ANSI_PROTOS 1
# else
#  define ANSI_PROTOS 0
# endif
#endif

#ifndef HAVE_STDLIB_H
# define HAVE_STDLIB_H 0
#endif

#ifndef ANSI_PROTOS
# define ANSI_PROTOS 0
#endif

#if ANSI_PROTOS
#define P(param) param
#else
#define P(param) ()
#endif

#include <sys/types.h>		/* sometimes needed to get size_t */

#if HAVE_STDLIB_H
#include <stdlib.h>
#else
# if !defined(HAVE_CONFIG_H) || MISSING_EXTERN_CALLOC
extern	char *	calloc	P(( size_t, size_t ));
# endif
# if !defined(HAVE_CONFIG_H) || MISSING_EXTERN_REALLOC
extern	char *	realloc	P(( char *, size_t ));
# endif
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <stdio.h>
#include <ctype.h>

#if MISSING_EXTERN__FILBUF
extern	int	_filbuf	P(( FILE * ));
#endif

#ifdef lint
#define	typecallocn(cast,ntypes)	(((cast *)0)+(ntypes))
#define	typereallocn(cast,ptr,ntypes)	((ptr)+(ntypes))
#else
#define	typecallocn(cast,ntypes)	(cast *)calloc(sizeof(cast),ntypes)
#define	typereallocn(cast,ptr,ntypes)	(cast *)realloc((char *)(ptr),\
							(ntypes)*sizeof(cast))
#endif

#define backspace() \
		if (cur_line != 0 \
		 && cur_line->l_this > 0) \
		    cur_line->l_this -= 1;

#define MAX_LINES 200

	/* SGR codes that we also use as mask values */
#define ATR_NORMAL 0
#define ATR_BOLD   1
#define ATR_UNDER  4

	/* character names */
#define ESCAPE    '\033'
#define CNTL_A    '\001'
#define SHIFT_OUT '\016'
#define SHIFT_IN  '\017'

#define SPACE     ' '
#define UNDERLINE '_'

#define CS_NORMAL    0
#define CS_ALTERNATE 1
/*
 * Each column of a line is represented by a linked list of the characters that
 * are printed to that position.  When storing items in this list, we keep a
 * canonical order to simplify analysis when dumping the line.
 */
typedef	struct	CharCell {
	struct	CharCell *link;
	char	c_ident;	/* CS_NORMAL/CS_ALTERNATE */
	char	c_level;	/* 0=base, 1=up halfline, -1=down halfline */
	char	c_value;	/* the actual value */
	} CHARCELL;

typedef struct	LineData {
	struct	LineData *l_next;
	struct	LineData *l_prev;
	size_t	l_last;		/* the number of cells allocated in l_cells[] */
	size_t	l_used;		/* the number of cells used in l_cells[] */
	size_t	l_this;		/* the current cell within the line */
	CHARCELL *l_cell;
	} LINEDATA;

extern int main P(( int argc, char **argv ));

static CHARCELL * allocate_cell P(( void ));
static LINEDATA * allocate_line P(( void ));
static int ansi_escape P(( FILE *ifp, int level, int ident ));
static int cell_code P(( LINEDATA *line, int col));
static int half_down P(( int level ));
static int half_up P(( int level ));
static void ManFilter P(( FILE *ifp ));
static void extend_line P(( void ));
static void failed P(( char *s ));
static void flush_line P(( void ));
static void next_line P(( void ));
static void prev_line P(( void ));
static void put_cell P(( int c, int level, int ident ));

static LINEDATA *all_lines;
static LINEDATA *cur_line;
static long	total_lines;

static void
failed(s)
char *s;
{
	perror(s);
	exit(1);
}

/*
 * Allocate a CHARCELL struct
 */
static CHARCELL *
allocate_cell()
{
	CHARCELL *p = typecallocn(CHARCELL,1);
	if (p == 0)
		failed("allocate_cell");
	return p;
}

/*
 * Allocate a LINEDATA struct
 */
static LINEDATA *
allocate_line()
{
	LINEDATA *p = typecallocn(LINEDATA,1);
	if (p == 0)
		failed("allocate_line");

	if (all_lines == 0)
		all_lines = p;

	if (total_lines++ > MAX_LINES)
		flush_line();

	return p;
}

/*
 * (Re)allocate the l_cell[] array for the current line
 */
static void
extend_line()
{
	size_t have = cur_line->l_last;
	size_t want = cur_line->l_this;
	if (want >= have) {
		CHARCELL *c = cur_line->l_cell;
		want += 80;
		if (c == 0) {
			c = typecallocn(CHARCELL,want);
		} else {
			c = typereallocn(CHARCELL,c,want);
		}
		if (c == 0)
			failed("extend_line");
		while (have < want) {
			c[have].c_value = SPACE;
			c[have].c_level = 0;
			c[have].c_ident = CS_NORMAL;
			have++;
		}
		cur_line->l_last = want;
		cur_line->l_cell = c;
	}
}

/*
 * Store a character at the current position, updating the current position.
 * We expect (but do not require) that an underscore precedes a nonblank
 * character that will overstrike it.  (Some programs produce the underscore
 * second, rather than first).
 */
static void
put_cell(c, level, ident)
int c;
int level;
int ident;
{
	int	col;
	int	len;
	CHARCELL *p, *q;

	if (cur_line == 0)
		cur_line = allocate_line();

	len = cur_line->l_used;
	col = cur_line->l_this++;
	extend_line();

	p = &(cur_line->l_cell[col]);

	if (len > col) {	/* some type of overstrike */
		if (c == UNDERLINE) {
			while ((q = p->link) != 0)
				p = q;
			q = allocate_cell();
			p->link = q;
			p = q;
		} else {
			if ((c != SPACE)
			 || (p->c_value == UNDERLINE)) {
				q = allocate_cell();
				*q = *p;
				p->link = q;
			} else if (c == SPACE)
				return;
		}
	}

	p->c_value = c;
	p->c_level = level;
	p->c_ident = ident;

	if (cur_line->l_used < cur_line->l_this)
		cur_line->l_used = cur_line->l_this;
}

/*
 * Interpret equivalent overstrike/underline for an ANSI escape sequence.
 */
static int
ansi_escape(ifp, level, ident)
FILE *ifp;
int level;
int ident;
{
	int	code = ATR_NORMAL;
	int	c;

	while ((c = fgetc(ifp)) != EOF) {
		if (isalpha(c))
			break;
		else if (isdigit(c))
			code = (code * 10) + (c - '0');
		else
			code = 0;
	}

	if (c == 'm') {
		if (code != ATR_BOLD && code != ATR_UNDER)
			code = ATR_NORMAL;
	} else {
		code = ATR_NORMAL;
	}
	return code;
}

/*
 * Set the current pointer to the previous line, allocating it if necessary
 */
static void
prev_line()
{
	LINEDATA *old_line;

	if (cur_line == 0)
		cur_line = allocate_line();

	if (cur_line->l_prev == 0) {
		cur_line->l_prev = allocate_line();
		if (cur_line == all_lines)
			all_lines = cur_line->l_prev;
	}
	old_line = cur_line;
	cur_line = cur_line->l_prev;
	cur_line->l_next = old_line;
	cur_line->l_this = old_line->l_this;
}

/*
 * Set the current pointer to the next line, allocating it if necessary
 */
static void
next_line()
{
	LINEDATA *old_line;

	if (cur_line == 0)
		cur_line = allocate_line();

	if (cur_line->l_next == 0)
		cur_line->l_next = allocate_line();

	old_line = cur_line;
	cur_line = cur_line->l_next;
	cur_line->l_prev = old_line;
	cur_line->l_this = old_line->l_this;
}

/*
 * If we've got a blank line to write onto, fake half-lines that way. 
 * Otherwise, eat them.  We assume that half-line controls occur in pairs.
 */
static int
half_up(level)
int level;
{
	prev_line();
	if (cur_line->l_this < cur_line->l_used) {
		next_line();
		return level+1;
	}
	return 0;
}

static int
half_down(level)
int level;
{
	if (level == 0) {
		next_line();
		return 0;
	}
	return level-1;
}

static int
cell_code(line,col)
LINEDATA *line;
int col;
{
	CHARCELL *p = &(line->l_cell[col]);
	CHARCELL *q;
	int code = ATR_NORMAL;
	while ((q = p->link) != 0) {
		if (q->c_value == UNDERLINE
		 && q->c_value != p->c_value) {
			code |= ATR_UNDER;
		} else
			code |= ATR_BOLD;
		p = q;
	}
	return code;
}

/*
 * Write the oldest line from memory to standard output and deallocate its
 * storage.
 */
static void
flush_line()
{
	int	col;
	int	ref_code;
	int	tst_code;
	int	counter;
	LINEDATA *l = all_lines;
	CHARCELL *p;

	if (l != 0) {
		all_lines = l->l_next;
		if (cur_line == l)
			cur_line = all_lines;

		ref_code = ATR_NORMAL;
		counter  = 0;
		for (col = 0; col < l->l_used; col++) {
			if (--counter <= 0) {
				tst_code = cell_code(l,col);
				if (tst_code != ref_code) {
					ref_code = tst_code;
					for (counter = 1; counter+col < l->l_used; counter++) {
						tst_code = cell_code(l, col+counter);
						if (tst_code != ref_code)
							break;
					}
					if (ref_code != ATR_NORMAL) {
						printf("%c%d", CNTL_A, counter);
						if (ref_code & ATR_BOLD)
							putchar('B');
						if (ref_code & ATR_UNDER)
							putchar('I');
						putchar(':');
					}
				}
			}
			putchar(l->l_cell[col].c_value);

			while ((p = l->l_cell[col].link) != 0) {
				l->l_cell[col].link = p->link;
				free((char *)p);
			}
		}
		putchar('\n');

		free((char *)l->l_cell);
		free((char *)l);
	}
}

/*
 * Filter an entire file, writing the result to the standard output.
 */
static void
ManFilter(ifp)
FILE *ifp;
{
	int	c;
	int	level = 0;
	int	ident = CS_NORMAL;
	int	esc_mode = ATR_NORMAL;

	while ((c = fgetc(ifp)) != EOF) {
		switch (c) {
		case '\b':
			backspace();
			break;

		case '\r':
			if (cur_line != 0)
				cur_line->l_this = 0;
			break;

		case '\n':
			next_line();
			cur_line->l_this = 0;
			break;

		case '\t':
			do {
				put_cell(SPACE, level, ident);
			} while (cur_line->l_this & 7);
			break;

		case '\v':
			prev_line();
			break;

		case SHIFT_IN:
			ident = CS_NORMAL;
			break;

		case SHIFT_OUT:
			ident = CS_ALTERNATE;
			break;

		case ESCAPE:
			switch (fgetc(ifp)) {
			case '[':
				esc_mode = ansi_escape(ifp, ident, level);
				break;
			case '\007':
			case '7':
				prev_line();
				break;
			case '\010':
			case '8':
				level = half_up(level);
				break;
			case '\011':
			case '9':
				level = half_down(level);
				break;
			default: /* ignore everything else */
				break;
			}
			break;

		default: /* ignore other nonprinting characters */
			if (isprint(c)) {
				put_cell(c, level, ident);
				if (c != SPACE) {
					if (esc_mode & ATR_BOLD) {
						backspace();
						put_cell(c, level, ident);
					}
					if (esc_mode & ATR_UNDER) {
						backspace();
						put_cell('_', level, ident);
					}
				}
			}
			break;
		}
	}

	while (all_lines != 0)
		flush_line();

	total_lines = 0;
}

int main(argc, argv)
	int argc;
	char **argv;
{
	int n;

	if (argc > 1) {
		for (n = 1; n < argc; n++) {
			FILE *fp = fopen(argv[n], "r");
			if (fp == 0)
				failed(argv[n]);
			ManFilter(fp);
			(void)fclose(fp);
		}
	} else {
		ManFilter(stdin);
	}
	exit(0);	/* successful exit */
	/*NOTREACHED*/
}
