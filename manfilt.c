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
 * This program also bolds lines which begin with two uppercase letters
 * causing the headings on many manual pages to be turned bold.
 *
 * vile will choose some appropriate fallback (such as underlining) if
 * italics are not available.
 *
 */

#include <stdio.h>

/* Initial amount of space to allocate for the input line. */
#define INITIALSIZE 1024

/* The input and output lines */
static char *line_in;
static char *line_out;

/* Sizes of input and output lines.  We've carefully calculated the size of
 * the output line in terms of the input line so that overflow is impossible.
 */
static int line_in_size = INITIALSIZE;
static int line_out_size = INITIALSIZE * 4 / 3;

/* The name of this program (for use in fatal error messages) */
static char *progname;

/* 
 * The following makes the filter procedure easier to write in that we don't
 * need to do a bounds check when looking back one character.
 */
#define LINE_IN (line_in+1)

/* Prototypes */
static void filter(FILE *);
static int  getline(FILE *);
static void putline(void);
static void fatal_error(const char *);


/* Print an error message and exit */
static void
fatal_error(const char *message)
{
    fprintf(stderr, "%s: Fatal error: %s\n", progname, message);
    exit(1);	/* unsuccessful exit */
}

void
main(int argc, char **argv)
{
    progname = argv[0];
    
    line_in = (char *) malloc(line_in_size);
    line_out = (char *) malloc(line_out_size);

    if (line_in == NULL || line_out == NULL)
	fatal_error("Insufficient memory");

    if (argc <= 1)
	filter(stdin);
    else {
	int i;
	for (i=1; i< argc; i++) {
	    FILE *fp = fopen(argv[i], "r");
	    if (fp == NULL)
		fprintf(stderr, 
		        "Error opening '%s' for read access\n", argv[i]);
	    else
		filter(fp);
	}
    }
    exit(0);	/* successful exit */
}

static void
filter(FILE *fp)
{
    char *inp, *outp;
    int c;
    int underline;
    int bold;

    while (getline(fp)) {
	inp = LINE_IN;
	outp = line_out;

	/*
	 * Make the entire line bold if the first two characters are
	 * uppercase. It is assumed that there won't be any backspace
	 * sequences on such lines.
	 */

	if ('A' <= *inp && *inp <= 'Z' && 'A' <= *(inp+1) && *(inp+1) <= 'Z') {
	    char *p, *q;
	    int count = strlen(inp) - 1;
	    *outp++ = '\001';		/* Control A */
	    p = outp;
	    while (count) {		/* Output count (in reverse order) */
		*outp++ = (count % 10) + '0';
		count /= 10;
	    }
	    q = outp-1;
	    while (p < q) {		/* Put count in right order */
		c = *p;
		*p++ = *q;
		*q-- = c;
	    }
	    *outp++ = 'B';		/* Bold */
	    *outp++ = ':';
	    while ((*outp++ = *inp++));	/* copy line */
	    putline();
	    continue;			/* go onto next line */
	}

	/*
	 * Scan the line for backspace sequences
	 */

	while ((c = *inp)) {
	    if (c != '\b') {
		/* Not backspace; copy character */
		*outp++ = c;
		inp++;
	    }
	    else {
		int count = 0;
		char *countp;
		bold = underline = 0;

		/* Figure out if sequence is underline, bold, or both */
		if (*(inp-1) == '_') {
		    underline = 1;
		    if (*(inp+1) && *(inp+2) == '\b' && *(inp+1) == *(inp+3))
			bold = 1;
		}
		else if (*(inp-1) == *(inp+1))
		    bold = 1;

		/* Nuke character preceding backspace if we have neither
		 * bold nor underline.
		 */
		if (!bold && !underline) {
		    if (outp > line_out)
			outp--;
		    inp++;
		    continue;
		}

		inp--;			/* rescan start of sequence */
		*(outp-1) = '\001';	/* control-A */
		countp = outp++;	/* leave room for one digit */
		if (underline)
		    *outp++ = 'I';
		if (bold)
		    *outp++ = 'B';
		*outp++ = ':';
		for (;;) {
		    if (underline) {
			if (*inp == '_' && *(inp+1) == '\b')
			    inp += 2;
			else
			    break;
		    }
		    if (bold) {
			if (*inp && *(inp+1) == '\b' && *inp == *(inp+2)) {
			    *outp++ = *inp;
			    inp += 3;
			    while (*inp == '\b' && *(inp-1) == *(inp+1))
				inp += 2;
			}
			else {
			    if (underline)
				inp -= 2;    /* went too far; back up */
			    break;
			}
		    }
		    else
			*outp++ = *inp++;
		    count++;
		}
		if (count < 10)
		    *countp = '0' + count;
		else {
		    int shift;
		    char *p = outp;
		    if (count < 100)
			shift = 1;
		    else if (count < 1000)
			shift = 2;
		    else if (count < 10000)
			shift = 3;
		    else
			shift = 4;
		    do {
			p--;
			*(p+shift) = *p;
		    } while (p != countp);
		    for (p = countp+shift; p >= countp; p--) {
			*p = (count % 10) + '0';
			count /= 10;
		    }
		    outp += shift;
		}
	    }
	}
	*outp = '\0';
	putline();
    }
}

static int
getline(FILE *fp)
{
    register char *lim = line_in + line_in_size;
    register char *p = LINE_IN;
    register int c;

    *(p-1) = 0;

    c = getc(fp);

    while (c != -1) {
	*p++ = c;
	if (p >= lim) {
	    int offset = p - line_in;
	    line_in_size *= 2;
	    line_in = (char *) realloc(line_in, line_in_size);
	    if (line_in == NULL)
		fatal_error("Insufficient memory");
	    p = line_in + offset;
	    lim = line_in + line_in_size;
	    line_out_size *= 2;
	    line_out = (char *) realloc(line_out, line_out_size);
	    if (line_out == NULL)
		fatal_error("Insufficient memory");
	}
	if (c == '\n')
	    break;
	c = getc(fp);
    }
    
    *p = '\0';

    return (p != LINE_IN);
}

static void
putline()
{
    fputs(line_out, stdout);
}
