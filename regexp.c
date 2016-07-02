
/*
 *	This code has been MODIFIED for use in vile (see the original
 *	copyright information down below) -- in particular:
 *	 - regexec no longer needs to scan a null terminated string
 *	 - regexec takes two extra arguments describing the first and
 *	 	just-past-last legal scan start offsets, to limit matches
 *		to beginning in that range
 *	 - inexact character matches are now handled, if the global ignorecase
 *	 	is set
 *	 - regexps are now relocatable, rather than locked down
 *
 *		pgf, 11/91
 *
 * $Log: regexp.c,v $
 * Revision 1.39  1993/10/04 10:24:09  pgf
 * see tom's 3.62 changes
 *
 * Revision 1.38  1993/09/10  16:06:49  pgf
 * tom's 3.61 changes
 *
 * Revision 1.37  1993/07/27  18:06:20  pgf
 * see tom's 3.56 CHANGES entry
 *
 * Revision 1.36  1993/07/21  17:04:17  pgf
 * changes to make \s and \S true atoms.  they both must now be followed
 * by * or + to match more than one.  also, \s no longer matches BOL or EOL
 * (thanks to alistair and eric for pointing out and fixing)
 *
 * Revision 1.35  1993/06/30  11:27:24  pgf
 * comment typo
 *
 * Revision 1.34  1993/06/18  15:57:06  pgf
 * tom's 3.49 changes
 *
 * Revision 1.33  1993/04/01  13:06:31  pgf
 * turbo C support (mostly prototypes for static)
 *
 * Revision 1.32  1993/03/25  19:50:58  pgf
 * see 3.39 section of CHANGES
 *
 * Revision 1.31  1993/02/24  10:59:02  pgf
 * see 3.34 changes, in CHANGES file
 *
 * Revision 1.30  1992/12/28  23:50:45  foxharp
 * fixes for BOL and \s etc., from eric krohn
 *
 * Revision 1.29  1992/12/14  09:03:25  foxharp
 * lint cleanup, mostly malloc
 *
 * Revision 1.28  1992/12/13  13:20:21  foxharp
 * optimization (set regstart) for \<string\> style patterns
 *
 * Revision 1.27  1992/11/19  09:17:42  foxharp
 * allow trailing backslashes in expressions, so isearch can use backslash
 *
 * Revision 1.26  1992/05/25  21:44:45  foxharp
 * moved func declarations to header
 *
 * Revision 1.25  1992/05/19  23:46:09  pgf
 * took newline out of the \W, \D, and \p matches
 *
 * Revision 1.24  1992/05/19  23:24:27  foxharp
 * fixed * and + behavior for \w, \S, \d, added \p and \P for "printable"
 *
 * Revision 1.23  1992/05/19  08:55:44  foxharp
 * more prototype and shadowed decl fixups
 *
 * Revision 1.22  1992/05/16  12:00:31  pgf
 * prototypes/ansi/void-int stuff/microsoftC
 *
 * Revision 1.21  1992/04/30  17:54:28  pgf
 * added \s, \S, \w, \W, \d, \D atoms
 *
 * Revision 1.20  1992/04/16  18:56:10  pgf
 * better fix for non-leading ^ and non-trailing $
 *
 * Revision 1.19  1992/04/14  08:34:25  pgf
 * make $ and ^ be special only at end and start of pattern, and literal
 * elsewhere
 *
 * Revision 1.18  1992/03/26  09:15:48  pgf
 * took out include of string.h
 *
 * Revision 1.17  1992/03/07  10:23:31  pgf
 * added eric krohn's diffs for \< and \> support, and removed some old ifdefs
 * for backslashed-paren code, which isn't needed with regmassage
 *
 * Revision 1.16  1992/03/01  18:41:31  pgf
 * be more restrictive setting regmust
 *
 * Revision 1.15  1992/01/05  00:06:13  pgf
 * split mlwrite into mlwrite/mlprompt/mlforce to make errors visible more
 * often.  also normalized message appearance somewhat.
 *
 * Revision 1.14  1992/01/04  00:55:56  pgf
 * tightened up regstrncmp() to keep it from running out of bounds, and
 * reversed the operands in one call to regstrncmp, to ensure the second
 * arg is the null-terminated one.
 *
 * Revision 1.13  1992/01/03  23:25:31  pgf
 * treat null (empty) lines much more carefully in lregexec()
 *
 * Revision 1.12  1991/12/20  08:12:00  pgf
 * don't go past regnoerror when checking for exact matches
 *
 * Revision 1.11  1991/11/16  18:33:12  pgf
 * added regmassage, to implement magic/non-magicness, and allow proper
 * vi backslashing in regexps.  regcomp now takes a "magic" flag
 *
 * Revision 1.10  1991/11/12  23:55:43  pgf
 * better commentary
 *
 * Revision 1.9  1991/11/12  23:44:21  pgf
 * regexec no longer needs null terminated strings -- it takes an
 * end pointer instead
 *
 * Revision 1.8  1991/11/08  13:09:15  pgf
 * fixed nested comment for saber
 *
 * Revision 1.7  1991/11/03  17:33:20  pgf
 * use new lregexec() routine to check for patterns in lines
 *
 * Revision 1.6  1991/11/01  14:38:00  pgf
 * saber cleanup
 *
 * revision 1.5
 * date: 1991/11/01 14:10:35;  author: pgf;  state: Exp;  lines: +19 -16
 * added mlen element of regexp struct, for convenience, and regmust
 * is now an offset instead of a pointer, so regexps are relocatable (i hope)
 *
 * revision 1.4
 * date: 1991/10/31 02:37:38;  author: pgf;  state: Exp;  lines: +5 -0
 * fix for "$" pattern
 *
 * revision 1.3
 * date: 1991/10/28 01:01:06;  author: pgf;  state: Exp;  lines: +24 -18
 * added start offset and end offset to regexec calls
 *
 * revision 1.2
 * date: 1991/10/27 02:23:26;  author: pgf;  state: Exp;  lines: +151 -35
 * initial rework for vile -- limit arg to regexec, switch to backslashed
 * parentheses for subexpressions, caseless matching
 *
 * revision 1.1
 * date: 1991/10/27 02:22:53;  author: pgf;  state: Exp;
 * Initial revision
 *
 */

/* #define REGDEBUG  1 */

/*
 * regcomp and regexec -- regsub and regerror are elsewhere
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
 *
 * Beware that some of this code is subtly aware of the way operator
 * precedence is structured in regular expressions.  Serious changes in
 * regular-expression syntax might require a total rethink.
 */

#include "estruct.h"
#include "edef.h"

#undef PLUS  /* vile conflict */

/*
 * The "internal use only" fields in regexp.h are present to pass info from
 * compile to execute that permits the execute phase to run lots faster on
 * simple cases.  They are:
 *
 * regstart	char that must begin a match; EOS if none obvious
 * reganch	is the match anchored (at beginning-of-line only)?
 * regmust	string (starting at program[offset]) that match must
 *				include, or NULL
 * regmlen	length of regmust string
 *
 * Regstart and reganch permit very fast decisions on suitable starting points
 * for a match, cutting down the work a lot.  Regmust permits fast rejection
 * of lines that cannot possibly match.  The regmust tests are costly enough
 * that regcomp() supplies a regmust only if the r.e. contains something
 * potentially expensive (at present, the only such thing detected is * or +
 * at the start of the r.e., which can involve a lot of backup).  Regmlen is
 * supplied because the test in regexec() needs it and regcomp() is computing
 * it anyway.
 */

/*
 * Structure for regexp "program".  This is essentially a linear encoding
 * of a nondeterministic finite-state machine (aka syntax charts or
 * "railroad normal form" in parsing technology).  Each node is an opcode
 * plus a "next" pointer, possibly plus an operand.  "Next" pointers of
 * all nodes except BRANCH implement concatenation; a "next" pointer with
 * a BRANCH on both ends of it is connecting two alternatives.  (Here we
 * have one of the subtle syntax dependencies:  an individual BRANCH (as
 * opposed to a collection of them) is never concatenated with anything
 * because of operator precedence.)  The operand of some types of node is
 * a literal string; for others, it is a node leading into a sub-FSM.  In
 * particular, the operand of a BRANCH node is the first node of the branch.
 * (NB this is *not* a tree structure:  the tail of the branch connects
 * to the thing following the set of BRANCHes.)  The opcodes are:
 */

/* definition	number	opnd?	meaning */
#define	END	0	/* no	End of program. */
#define	BOL	1	/* no	Match "" at beginning of line. */
#define	EOL	2	/* no	Match "" at end of line. */
#define	ANY	3	/* no	Match any one character. */
#define	ANYOF	4	/* str	Match any character in this string. */
#define	ANYBUT	5	/* str	Match any character not in this string. */
#define	BRANCH	6	/* node	Match this alternative, or the next... */
#define	BACK	7	/* no	Match "", "next" ptr points backward. */
#define	EXACTLY	8	/* str	Match this string. */
#define	NOTHING	9	/* no	Match empty string. */
#define	STAR	10	/* node	Match this (simple) thing 0 or more times. */
#define	PLUS	11	/* node	Match this (simple) thing 1 or more times. */
#define	BEGWORD	12	/* node	Match "" between nonword and word. */
#define	ENDWORD 13	/* node	Match "" between word and nonword. */
#define	WHITESP 14	/* node	Match single whitespace, excluding BOL and EOL */
#define	NWHITESP 15	/* node	Match single nonwhitespace, excluding BOL and EOL */
#define	ALNUM	16	/* node	Match any alphanumeric, include _ */
#define	NALNUM	17	/* node	inverse above, including BOL and EOL */
#define	DIGIT	18	/* node	Match any digit */
#define	NDIGIT	19	/* node	Match any non-digit */
#define	PRINT	20	/* node	Match any printable char (including whitesp) */
#define	NPRINT	21	/* node	Match any non-printable char */
#define	OPEN	30	/* no	Mark this point in input as start of #n. */
			/*	OPEN+1 is number 1, etc. */
#define	CLOSE	40	/* no	Analogous to OPEN. */

/*
 * Opcode notes:
 *
 * BRANCH	The set of branches constituting a single choice are hooked
 *		together with their "next" pointers, since precedence prevents
 *		anything being concatenated to any individual branch.  The
 *		"next" pointer of the last BRANCH in a choice points to the
 *		thing following the whole choice.  This is also where the
 *		final "next" pointer of each individual branch points; each
 *		branch starts with the operand node of a BRANCH node.
 *
 * BACK		Normal "next" pointers all implicitly point forward; BACK
 *		exists to make loop structures possible.
 *
 * STAR,PLUS	'?', and complex '*' and '+', are implemented as circular
 *		BRANCH structures using BACK.  Simple cases (one character
 *		per match) are implemented with STAR and PLUS for speed
 *		and to minimize recursive plunges.
 *
 * OPEN,CLOSE	...are numbered at compile time.
 */

/*
 * A node is one char of opcode followed by two chars of "next" pointer.
 * "Next" pointers are stored as two 8-bit pieces, high order first.  The
 * value is a positive offset from the opcode of the node containing it.
 * An operand, if any, simply follows the node.  (Note that much of the
 * code generation knows about this implicit relationship.)
 *
 * Using two bytes for the "next" pointer is vast overkill for most things,
 * but allows patterns to get big without disasters.
 */
#define	OP(p)	(*(p))
#define	NEXT(p)	(((*((p)+1)&0377)<<8) + (*((p)+2)&0377))
#define	OPERAND(p)	((p) + 3)

/*
 * See regmagic.h for one further detail of program structure.
 */


/*
 * Utility definitions.
 */

#define	FAIL(m)	{ regerror(m); return(NULL); }
#define	ISMULT(c)	((c) == '*' || (c) == '+' || (c) == '?')
#define	META	"^$.[()|?+*\\<>"

/*
 * Flags to be passed up and down.
 */
#define	HASWIDTH	01	/* Known never to match null string. */
#define	SIMPLE		02	/* Simple enough to be STAR/PLUS operand. */
#define	SPSTART		04	/* Starts with * or +. */
#define	WORST		0	/* Worst case. */

/*
 * Global work variables for regcomp().
 */
static char *regparse;		/* Input-scan pointer. */
static int regnpar;		/* () count. */
static char regdummy;
static char *regcode;		/* Code-emit pointer; &regdummy = don't. */
static long regsize;		/* Code size. */

/*
 *				regexp		in magic	in nomagic
 *				char		enter as	enter as
 *				-------		--------	--------
 *	0 or 1			?		\?		\?
 *	1 or more		+		\+		\+
 *	0 or more		*		*		\*
 *	beg. nest		(		\(		\(
 *	end nest		)		\(		\)
 *	beg chr class		[		[		\[
 *	beg "word"		<		\<		\<
 *	end "word"		>		\>		\>
 *	beginning		^		^		^
 *	end			$		$		$
 *	any char		.		.		\.
 *	alternation		|		\|		\|
 *	flip or literal		\		\\		\\
 *	last replace		~		~		\~
 *	words			\w		\w		\w
 *	spaces			\s		\s		\s
 *	digits			\d		\d		\d
 *	printable		\p		\p		\p
 *
 *	So:  in magic mode, we remove \ from ? + ( ) < > |
 *			   and add \ to bare ? + ( ) < > |
 *	   in nomagic mode, we remove \ from ? + ( ) < > | * [ ] . ~
 *			   and add \ to bare ? + ( ) < > | * [ ] . ~
 */

#define MAGICMETA   "?+()<>|"
#define NOMAGICMETA "?+()<>|*[] .~"

void
regmassage(old,new,magic)
char *old, *new;
int magic;
{
	char *metas =  magic ? MAGICMETA : NOMAGICMETA;
	while (*old) {
		if (*old == '\\') { /* remove \ from these metas */
			if (*(old+1) == EOS) {
				*new++ = '\\';
				break;
			}
			if (strchr(metas, *(old+1))) {
				old++; /* skip the \ */
			}
		} else if (strchr(metas, *old)) { /* add \ to these metas */
			*new++ = '\\';
		}
		*new++ = *old++;  /* the char */
	}
	*new = EOS;
}

/*
 - regcomp - compile a regular expression into internal code
 *
 * We can't allocate space until we know how big the compiled form will be,
 * but we can't compile it (and thus know how big it is) until we've got a
 * place to put the code.  So we cheat:  we compile it twice, once with code
 * generation turned off and size counting turned on, and once "for real".
 * This also means that we don't allocate space until we are sure that the
 * thing really will compile successfully, and we never have to move the
 * code and thus invalidate pointers into it.  (Note that it has to be in
 * one piece because free() must be able to free it all.)
 *
 * Beware that the optimization-preparation code in here knows about some
 * of the structure of the compiled regexp.
 */

regexp *
regcomp(origexp,magic)
char *origexp;
int magic;
{
	register regexp *r;
	register char *scan;
	register char *longest;
	register ALLOC_T len;
	int flags;
	static char *exp;
	static int explen;

	if (origexp == NULL)
		FAIL("NULL argument");

	len = strlen(origexp)+1;
	if (explen < 2*len+20) {
		if (exp)
			free(exp);
		exp = castalloc(char, 2*len+20);
		if (exp == NULL)
			FAIL("couldn't allocate exp copy");
		explen = 2*len+20;
	}

	regmassage(origexp, exp, magic);

	/* First pass: determine size, legality. */
	regparse = exp;
	regnpar = 1;
	regsize = 0;
	regcode = &regdummy;
	regc(REGEXP_MAGIC);
	if (reg(0, &flags) == NULL)
		return(NULL);

	/* Small enough for pointer-storage convention? */
	if (regsize >= 32767)		/* Probably could be 65535. */
		FAIL("regexp too big");

	/* Allocate space. */
	r = typeallocplus(regexp, regsize);
	if (r == NULL)
		FAIL("out of space");

	/* how big is it?  (vile addition) */
	r->size = sizeof(regexp) + regsize;

	/* Second pass: emit code. */
	regparse = exp;
	regnpar = 1;
	regcode = r->program;
	regc(REGEXP_MAGIC);
	if (reg(0, &flags) == NULL)
		return(NULL);

	/* Dig out information for optimizations. */
	r->regstart = EOS;	/* Worst-case defaults. */
	r->reganch = 0;
	r->regmust = -1;
	r->regmlen = 0;
	scan = r->program+1;			/* First BRANCH. */
	if (OP(regnext(scan)) == END) {		/* Only one top-level choice. */
		scan = OPERAND(scan);

		/* Starting-point info. */
		if (OP(scan) == EXACTLY)
			r->regstart = *OPERAND(scan);
		else if (OP(scan) == BEGWORD && OP(regnext(scan)) == EXACTLY)
			r->regstart = *OPERAND(regnext(scan));
		else if (OP(scan) == BOL)
			r->reganch++;

		/*
		 * If there's something expensive in the r.e., find the
		 * longest literal string that must appear and make it the
		 * regmust.  Resolve ties in favor of later strings, since
		 * the regstart check works with the beginning of the r.e.
		 * and avoiding duplication strengthens checking.  Not a
		 * strong reason, but sufficient in the absence of others.
		 */
		if (flags&SPSTART) {
			longest = NULL;
			len = 0;
			for (; scan != NULL; scan = regnext(scan))
				if (OP(scan) == EXACTLY && strlen(OPERAND(scan)) >= len) {
					longest = OPERAND(scan);
					len = strlen(OPERAND(scan));
				}
			if (longest) {
				r->regmust = longest - r->program;
				r->regmlen = len;
			}
		}
	}

#if NO_LEAKS
	if (exp != 0) { free(exp); exp = 0; explen = 0; }
#endif
	return(r);
}

/*
 - reg - regular expression, i.e. main body or parenthesized thing
 *
 * Caller must absorb opening parenthesis.
 *
 * Combining parenthesis handling with the base level of regular expression
 * is a trifle forced, but the need to tie the tails of the branches to what
 * follows makes it hard to avoid.
 */
char *
reg(paren, flagp)
int paren;			/* Parenthesized? */
int *flagp;
{
	register char *ret;
	register char *br;
	register char *ender;
	register int parno;
	int flags;

	*flagp = HASWIDTH;	/* Tentatively. */

	/* Make an OPEN node, if parenthesized. */
	if (paren) {
		if (regnpar >= NSUBEXP)
			FAIL("too many ()");
		parno = regnpar;
		regnpar++;
		ret = regnode(OPEN+parno);
	} else {
		ret = NULL;
		parno = 0;
	}

	/* Pick up the branches, linking them together. */
	br = regbranch(&flags);
	if (br == NULL)
		return(NULL);
	if (ret != NULL)
		regtail(ret, br);	/* OPEN -> first. */
	else
		ret = br;
	if (!(flags&HASWIDTH))
		*flagp &= ~HASWIDTH;
	*flagp |= flags&SPSTART;
	while (*regparse == '|') {
		regparse++;
		br = regbranch(&flags);
		if (br == NULL)
			return(NULL);
		regtail(ret, br);	/* BRANCH -> BRANCH. */
		if (!(flags&HASWIDTH))
			*flagp &= ~HASWIDTH;
		*flagp |= flags&SPSTART;
	}

	/* Make a closing node, and hook it on the end. */
	ender = regnode((paren) ? CLOSE+parno : END);
	regtail(ret, ender);

	/* Hook the tails of the branches to the closing node. */
	for (br = ret; br != NULL; br = regnext(br))
		regoptail(br, ender);

	/* Check for proper termination. */
	if (paren && *regparse++ != ')') {
		FAIL("unmatched ()");
	} else if (!paren && *regparse != EOS) {
		if (*regparse == ')') {
			FAIL("unmatched ()");
		} else
			FAIL("junk on end");	/* "Can't happen". */
		/* NOTREACHED */
	}

	return(ret);
}

/*
 - regbranch - one alternative of an | operator
 *
 * Implements the concatenation operator.
 */
char *
regbranch(flagp)
int *flagp;
{
	register char *ret;
	register char *chain;
	register char *latest;
	int flags;

	*flagp = WORST;		/* Tentatively. */

	ret = regnode(BRANCH);
	chain = NULL;
	while (*regparse != EOS && *regparse != '|' && *regparse != ')') {
		latest = regpiece(&flags, chain == NULL);
		if (latest == NULL)
			return(NULL);
		if (chain && OP(chain) == EOL) {
			regninsert(2,latest);
			OP(chain) = EXACTLY;
			*latest++ = '$';
			*latest++ = EOS;
			flags |= HASWIDTH|SIMPLE;
		}
		*flagp |= flags&HASWIDTH;
		if (chain == NULL)	/* First piece. */
			*flagp |= flags&SPSTART;
		else
			regtail(chain, latest);
		chain = latest;
	}
	if (chain == NULL)	/* Loop ran zero times. */
		(void) regnode(NOTHING);

	return(ret);
}

/*
 - regpiece - something followed by possible [*+?]
 *
 * Note that the branching code sequences used for ? and the general cases
 * of * and + are somewhat optimized:  they use the same NOTHING node as
 * both the endmarker for their branch list and the body of the last branch.
 * It might seem that this node could be dispensed with entirely, but the
 * endmarker role is not redundant.
 */
char *
regpiece(flagp, at_bop)
int *flagp;
int at_bop;
{
	register char *ret;
	register char op;
	register char *next;
	int flags;

	ret = regatom(&flags, at_bop);
	if (ret == NULL)
		return(NULL);

	op = *regparse;
	if (!ISMULT(op)) {
		*flagp = flags;
		return(ret);
	}

	if (!(flags&HASWIDTH) && op != '?')
		FAIL("*+ operand could be empty");
	*flagp = (op != '+') ? (WORST|SPSTART) : (WORST|HASWIDTH);

	if (op == '*' && (flags&SIMPLE))
		regopinsert(STAR, ret);
	else if (op == '*') {
		/* Emit x* as (x&|), where & means "self". */
		regopinsert(BRANCH, ret);			/* Either x */
		regoptail(ret, regnode(BACK));		/* and loop */
		regoptail(ret, ret);			/* back */
		regtail(ret, regnode(BRANCH));		/* or */
		regtail(ret, regnode(NOTHING));		/* null. */
	} else if (op == '+' && (flags&SIMPLE))
		regopinsert(PLUS, ret);
	else if (op == '+') {
		/* Emit x+ as x(&|), where & means "self". */
		next = regnode(BRANCH);			/* Either */
		regtail(ret, next);
		regtail(regnode(BACK), ret);		/* loop back */
		regtail(next, regnode(BRANCH));		/* or */
		regtail(ret, regnode(NOTHING));		/* null. */
	} else if (op == '?') {
		/* Emit x? as (x|) */
		regopinsert(BRANCH, ret);			/* Either x */
		regtail(ret, regnode(BRANCH));		/* or */
		next = regnode(NOTHING);		/* null. */
		regtail(ret, next);
		regoptail(ret, next);
	}
	regparse++;
	if (ISMULT(*regparse))
		FAIL("nested *?+");

	return(ret);
}

/*
 - regatom - the lowest level
 *
 * Optimization:  gobbles an entire sequence of ordinary characters so that
 * it can turn them into a single node, which is smaller to store and
 * faster to run.  Backslashed characters are exceptions, each becoming a
 * separate node; the code is simpler that way and it's not worth fixing.
 */
char *
regatom(flagp, at_bop)
int *flagp;
int at_bop;
{
	register char *ret;
	int flags;
	int len = 1;

	*flagp = WORST;		/* Tentatively. */

	switch (*regparse++) {
	case '^':
		if (!at_bop) {
			regparse--;
			goto defchar;
		}
		ret = regnode(BOL);
		break;
	case '$':
		ret = regnode(EOL);
		break;
	case '<':
		ret = regnode(BEGWORD);
		break;
	case '>':
		ret = regnode(ENDWORD);
		break;
	case '.':
		ret = regnode(ANY);
		*flagp |= HASWIDTH|SIMPLE;
		break;
	case '[': {
			register int class;
			register int classend;

			if (*regparse == '^') {	/* Complement of range. */
				ret = regnode(ANYBUT);
				regparse++;
			} else
				ret = regnode(ANYOF);
			if (*regparse == ']' || *regparse == '-')
				regc(*regparse++);
			while (*regparse != EOS && *regparse != ']') {
				if (*regparse == '-') {
					regparse++;
					if (*regparse == ']' || *regparse == EOS)
						regc('-');
					else {
						class = UCHAR_AT(regparse-2)+1;
						classend = UCHAR_AT(regparse);
						if (class > classend+1)
							FAIL("invalid [] range");
						for (; class <= classend; class++)
							regc(class);
						regparse++;
					}
				} else
					regc(*regparse++);
			}
			regc(EOS);
			if (*regparse != ']')
				FAIL("unmatched []");
			regparse++;
			*flagp |= HASWIDTH|SIMPLE;
		}
		break;
	case '(':
		ret = reg(1, &flags);
		if (ret == NULL)
			return(NULL);
		*flagp |= flags&(HASWIDTH|SPSTART);
		break;
	case EOS:
	case '|':
	case ')':
		FAIL("internal urp");	/* Supposed to be caught earlier. */

	case '?':
	case '+':
	case '*':
		FAIL("?+* follows nothing");

	case '\\':
		switch(*regparse) {
		case EOS:
#ifdef BEFORE
			FAIL("trailing \\");
#else
			/* as a special case, treat a trailing '\' char as
			 * a trailing '.'.  This makes '\' work in isearch
			 * most of the time */
			ret = regnode(ANY);
			*flagp |= HASWIDTH|SIMPLE;
			return ret;
#endif
		case 's':
			ret = regnode(WHITESP);
			*flagp |= HASWIDTH|SIMPLE;
			break;
		case 'S':
			ret = regnode(NWHITESP);
			*flagp |= HASWIDTH|SIMPLE;
			break;
		case 'w':
			ret = regnode(ALNUM);
			*flagp |= HASWIDTH|SIMPLE;
			break;
		case 'W':
			ret = regnode(NALNUM);
			*flagp |= HASWIDTH|SIMPLE;
			break;
		case 'd':
			ret = regnode(DIGIT);
			*flagp |= HASWIDTH|SIMPLE;
			break;
		case 'D':
			ret = regnode(NDIGIT);
			*flagp |= HASWIDTH|SIMPLE;
			break;
		case 'p':
			ret = regnode(PRINT);
			*flagp |= HASWIDTH|SIMPLE;
			break;
		case 'P':
			ret = regnode(NPRINT);
			*flagp |= HASWIDTH|SIMPLE;
			break;
		default:
			ret = regnode(EXACTLY);
			regc(*regparse);
			regc(EOS);
			*flagp |= HASWIDTH|SIMPLE;
			break;
		}
		regparse++;
		break;
	default: {
			register char ender;

			regparse--;
			len = strcspn(regparse, META);
			if (len <= 0)
				FAIL("internal disaster");
			ender = *(regparse+len);
			if (len > 1 && ISMULT(ender))
				len--;		/* Back off clear of ?+* operand. */
		defchar:
			*flagp |= HASWIDTH;
			if (len == 1)
				*flagp |= SIMPLE;
			ret = regnode(EXACTLY);
			while (len > 0) {
				regc(*regparse++);
				len--;
			}
			regc(EOS);
		}
		break;
	}

	return(ret);
}

/*
 - regnode - emit a node
 */
char *			/* Location. */
regnode(op)
int op;
{
	register char *ret;
	register char *ptr;

	ret = regcode;
	if (ret == &regdummy) {
		regsize += 3;
		return(ret);
	}

	ptr = ret;
	*ptr++ = op;
	*ptr++ = EOS;		/* Null "next" pointer. */
	*ptr++ = EOS;
	regcode = ptr;

	return(ret);
}

/*
 - regc - emit (if appropriate) a byte of code
 */
void
regc(b)
int b;
{
	if (regcode != &regdummy)
		*regcode++ = (char)b;
	else
		regsize++;
}

/*
 - regninsert - insert n bytes in front of already-emitted operand
 *
 * Means relocating the operand.
 */
void
regninsert(n, opnd)
register int n;
char *opnd;
{
	register char *src;
	register char *dst;
	register char *place;

	if (regcode == &regdummy) {
		regsize += n;
		return;
	}

	src = regcode;
	regcode += n;
	dst = regcode;
	while (src > opnd)
		*--dst = *--src;

	place = opnd;		/* Op node, where operand used to be. */
	while (n--)
		*place++ = EOS;
}

/*
 - regopinsert - insert an operator in front of already-emitted operand
 *
 * Means relocating the operand.
 */
void
regopinsert(op, opnd)
int op;
char *opnd;
{
	regninsert(3, opnd);
	if (regcode == &regdummy)
		return;
	*opnd = op;
}


/*
 - regtail - set the next-pointer at the end of a node chain
 */
void
regtail(p, val)
char *p;
char *val;
{
	register char *scan;
	register char *temp;
	register int offset;

	if (p == &regdummy)
		return;

	/* Find last node. */
	scan = p;
	for (;;) {
		temp = regnext(scan);
		if (temp == NULL)
			break;
		scan = temp;
	}

	if (OP(scan) == BACK)
		offset = scan - val;
	else
		offset = val - scan;
	*(scan+1) = (char)((offset>>8)&0377);
	*(scan+2) = (char)(offset&0377);
}

/*
 - regoptail - regtail on operand of first argument; nop if operandless
 */
void
regoptail(p, val)
char *p;
char *val;
{
	/* "Operandless" and "op != BRANCH" are synonymous in practice. */
	if (p == NULL || p == &regdummy || OP(p) != BRANCH)
		return;
	regtail(OPERAND(p), val);
}

/*
 * regexec and friends
 */

/*
 * Global work variables for regexec().
 */
static char *reginput;		/* String-input pointer. */
static char *regnomore;		/* String-input end pointer. */
static char *regbol;		/* Beginning of input, for ^ check. */
static char **regstartp;	/* Pointer to startp array. */
static char **regendp;		/* Ditto for endp. */

#ifdef REGDEBUG
int regnarrate = 0;
void regdump();
STATIC char *regprop();
#endif

/* this very special copy of strncmp allows for caseless operation,
 * and also for non-null terminated strings.  the A arg ends at position
 * E, which can be NULL if A really is null terminated.  B must be null-
 * terminated.  At most n characters are compared.
 */
int
regstrncmp(a,b,n,e)
char *a,*b,*e;
int n;
{
	if (e == NULL)
		e = &a[strlen(a)];
	if (ignorecase) {
		while (--n >=0 && nocase_eq(*a,*b) && a != e && *b)
			a++, b++;
	} else {
		while (--n >=0 && *a == *b && a != e && *b)
			a++, b++;
	}

	if (n < 0) return 0;
	if (a == e) return -*b;
	return *a - *b;
}

char *
regstrchr(s, c, e)
register char *s;
register int c;
register char *e;
{
	if (e == NULL)
		e = &s[strlen(s)];
	if (ignorecase) {
		while (s != e) {
			if (nocase_eq(*s,c)) return s;
			s++;
		}
	} else {
		while (s != e) {
			if (*s == c) return s;
			s++;
		}
	}
	return (char *)0;
}

/*
 - regexec - match a regexp against a string
 	prog is the compiled expression, string is the string, stringend
	points just after the string, and the match can begin at or after
	startoff, but must end before endoff
 */
int
regexec(prog, string, stringend, startoff, endoff)
register regexp *prog;
register char *string;
register char *stringend;  /* pointer to the null, if there were one */
register int startoff;
register int endoff;
{
	register char *s, *endsrch;

	/* Be paranoid... */
	if (prog == NULL || string == NULL) {
		regerror("NULL parameter");
		return(0);
	}

	/* Check validity of program. */
	if (UCHAR_AT(prog->program) != REGEXP_MAGIC) {
		regerror("corrupted program");
		return(0);
	}

	/* supply an endpoint if none given */
	if (stringend == NULL) {
		stringend = &string[strlen(string)];
	} else if (stringend < string) {
		regerror("end less than start");
		return(0);
	}


	if (endoff < 0)
		endoff = stringend - string;

	endsrch = &string[endoff];

	/* if our outer limit is the end-of-string, let us scan there,
		in case we're trying to match a lone '$' */
	if (endsrch == stringend)
		endsrch++;

	/* If there is a "must appear" string, look for it. */
	if (prog->regmust != -1) {
		s = &string[startoff];
		while ( (s = regstrchr(s, prog->program[prog->regmust],
						stringend))
					!= NULL && s < endsrch) {
			if (regstrncmp(s, &prog->program[prog->regmust],
						prog->regmlen, stringend) == 0)
				break;	/* Found it. */
			s++;
		}
		if (s >= endsrch || s == NULL)	/* Not present. */
			return(0);
	}

	/* Mark beginning of line for ^ . */
	regbol = string;

	/* Simplest case:  anchored match need be tried only once. */
	if (startoff == 0 && prog->reganch)
		return(regtry(prog, string, stringend));

	/* Messy cases:  unanchored match. */
	s = &string[startoff];
	if (prog->regstart != EOS) {
		/* We know what char it must start with. */
		while ( (s = regstrchr(s, prog->regstart, stringend)) != NULL &&
					s < endsrch) {
			if (regtry(prog, s, stringend))
				return(1);
			s++;
		}
	} else {
		/* We don't -- general case. */
		do {
			if (regtry(prog, s, stringend))
				return(1);
		} while (s++ != stringend && s < endsrch);
	}

	/* Failure. */
	return(0);
}

/*
 - regtry - try match at specific point
 */
int			/* 0 failure, 1 success */
regtry(prog, string, stringend)
regexp *prog;
char *string;
char *stringend;
{
	register int i;
	register char **sp;
	register char **ep;

	reginput = string;
	regnomore = stringend;
	regstartp = prog->startp;
	regendp = prog->endp;

	sp = prog->startp;
	ep = prog->endp;
	for (i = NSUBEXP; i > 0; i--) {
		*sp++ = NULL;
		*ep++ = NULL;
	}
	if (regmatch(prog->program + 1)) {
		prog->startp[0] = string;
		prog->endp[0] = reginput;
		prog->mlen = reginput - string;
		return(1);
	} else {
		prog->mlen = 0;  /* not indicative of anything */
		return(0);
	}
}

/*
 - regmatch - main matching routine
 *
 * Conceptually the strategy is simple:  check to see whether the current
 * node matches, call self recursively to see whether the rest matches,
 * and then act accordingly.  In practice we make some effort to avoid
 * recursion, in particular by going through "ordinary" nodes (that don't
 * need to know whether the rest of the match failed) by a loop instead of
 * by recursion.
 */
int			/* 0 failure, 1 success */
regmatch(prog)
char *prog;
{
	register char *scan;	/* Current node. */
	char *next;		/* Next node. */

	scan = prog;
#ifdef REGDEBUG
	if (scan != NULL && regnarrate)
		fprintf(stderr, "%s(\r\n", regprop(scan));
#endif
	while (scan != NULL) {
#ifdef REGDEBUG
		if (regnarrate)
			fprintf(stderr, "%s...\r\n", regprop(scan));
#endif
		next = regnext(scan);

		switch (OP(scan)) {
		case BOL:
			if (reginput != regbol)
				return(0);
			break;
		case EOL:
			if (reginput != regnomore)
				return(0);
			break;
		case BEGWORD:
			/* Match if current char isident
			 * and previous char BOL or !ident */
			if ((reginput == regnomore || !isident(*reginput))
					|| (reginput != regbol
					&& isident(reginput[-1])))
				return(0);
			break;
		case ENDWORD:
			/* Match if previous char isident
			 * and current char EOL or !ident */
			if ((reginput != regnomore && isident(*reginput))
					|| reginput == regbol
					|| !isident(reginput[-1]))
 				return(0);
 			break;
		case WHITESP:
			/* any single whitespace, but not bol or eol */
			if (reginput == regnomore)
				return 0;
			if (!isspace(*reginput))
				return 0;
			reginput++;
			break;
		case NWHITESP:
			if (reginput == regnomore)
				return 0;
			if (isspace(*reginput))
				return 0;
			reginput++;
			break;
		case ALNUM: /* includes _ */
			if (reginput == regnomore)
				return 0;
			if (!isident(*reginput))
				return 0;
			reginput++;
			break;
		case NALNUM:
			if (reginput == regnomore)
				return 0;
			if (isident(*reginput))
				return 0;
			reginput++;
			break;
		case DIGIT:
			if (reginput == regnomore)
				return 0;
			if (!isdigit(*reginput))
				return 0;
			reginput++;
			break;
		case NDIGIT:
			if (reginput == regnomore)
				return 0;
			if (isdigit(*reginput))
				return 0;
			reginput++;
			break;
		case PRINT:
			if (reginput == regnomore)
				return 0;
			if (!(isprint(*reginput) || isspace(*reginput)))
				return 0;
			reginput++;
			break;
		case NPRINT:
			if (reginput == regnomore)
				return 0;
			if (isprint(*reginput) || isspace(*reginput))
				return 0;
			reginput++;
			break;
		case ANY:
			if (reginput == regnomore)
				return(0);
			reginput++;
			break;
		case EXACTLY: {
				register int len;
				register char *opnd;

				if (reginput == regnomore)
					return(0);

				opnd = OPERAND(scan);
				/* Inline the first character, for speed. */
				if (ignorecase) {
					if (!nocase_eq(*opnd, *reginput))
						return(0);
				} else {
					if (*opnd != *reginput)
						return(0);
				}
				len = strlen(opnd);
				if (len > 1 && regstrncmp(reginput, opnd, len,
						regnomore) != 0)
					return(0);
				reginput += len;
			}
			break;
		case ANYOF:
			if (reginput == regnomore || regstrchr(OPERAND(scan),
					*reginput, (char *)0) == NULL)
				return(0);
			reginput++;
			break;
		case ANYBUT:
			if (reginput == regnomore || regstrchr(OPERAND(scan),
					*reginput, (char *)0) != NULL)
				return(0);
			reginput++;
			break;
		case NOTHING:
			break;
		case BACK:
			break;
		case OPEN+1:
		case OPEN+2:
		case OPEN+3:
		case OPEN+4:
		case OPEN+5:
		case OPEN+6:
		case OPEN+7:
		case OPEN+8:
		case OPEN+9: {
				register int no;
				register char *save;

				no = OP(scan) - OPEN;
				save = reginput;

				if (regmatch(next)) {
					/*
					 * Don't set startp if some later
					 * invocation of the same parentheses
					 * already has.
					 */
					if (regstartp[no] == NULL)
						regstartp[no] = save;
					return(1);
				} else
					return(0);
			}
			/* NOTREACHED */

		case CLOSE+1:
		case CLOSE+2:
		case CLOSE+3:
		case CLOSE+4:
		case CLOSE+5:
		case CLOSE+6:
		case CLOSE+7:
		case CLOSE+8:
		case CLOSE+9: {
				register int no;
				register char *save;

				no = OP(scan) - CLOSE;
				save = reginput;

				if (regmatch(next)) {
					/*
					 * Don't set endp if some later
					 * invocation of the same parentheses
					 * already has.
					 */
					if (regendp[no] == NULL)
						regendp[no] = save;
					return(1);
				} else
					return(0);
			}
			/* NOTREACHED */

		case BRANCH: {
				register char *save;

				if (OP(next) != BRANCH)		/* No choice. */
					next = OPERAND(scan);	/* Avoid recursion. */
				else {
					do {
						save = reginput;
						if (regmatch(OPERAND(scan)))
							return(1);
						reginput = save;
						scan = regnext(scan);
					} while (scan != NULL && OP(scan) == BRANCH);
					return(0);
					/* NOTREACHED */
				}
			}
			break;
		case STAR:
		case PLUS: {
				register char nxtch;
				register int no;
				register char *save;
				register int min;

				/*
				 * Lookahead to avoid useless match attempts
				 * when we know what character comes next.
				 */
				nxtch = EOS;
				if (OP(next) == EXACTLY)
					nxtch = *OPERAND(next);
				min = (OP(scan) == STAR) ? 0 : 1;
				save = reginput;
				no = regrepeat(OPERAND(scan));
				if (ignorecase)
				    while (no >= min) {
					/* If it could work, try it. */
					if (nxtch == EOS ||
						nocase_eq(*reginput,nxtch))
						if (regmatch(next))
							return(1);
					/* Couldn't or didn't -- back up. */
					no--;
					reginput = save + no;
				    }
				else
				    while (no >= min) {
					/* If it could work, try it. */
					if (nxtch == EOS ||
						*reginput == nxtch)
						if (regmatch(next))
							return(1);
					/* Couldn't or didn't -- back up. */
					no--;
					reginput = save + no;
				    }
				return(0);
			}
			/* NOTREACHED */

		case END:
			return(1);	/* Success! */
		default:
			regerror("memory corruption");
			return(0);
		}

		scan = next;
	}

	/*
	 * We get here only if there's trouble -- normally "case END" is
	 * the terminating point.
	 */
	regerror("corrupted pointers");
	return(0);
}

/*
 - regrepeat - repeatedly match something simple, report how many
 */
int
regrepeat(p)
char *p;
{
	register int count = 0;
	register char *scan;
	register char *opnd;

	scan = reginput;
	opnd = OPERAND(p);
	switch (OP(p)) {
	case ANY:
		count = regnomore - scan;
		scan += count;
		break;
	case EXACTLY:
		if (ignorecase)
			while (nocase_eq(*opnd,*scan)) {
				count++;
				scan++;
			}
		else
			while (*opnd == *scan) {
				count++;
				scan++;
			}
		break;
	case ANYOF:
		while (scan != regnomore && regstrchr(opnd, *scan,
						(char *)0) != NULL) {
			count++;
			scan++;
		}
		break;
	case ANYBUT:
		while (scan != regnomore && regstrchr(opnd, *scan,
						(char *)0) == NULL) {
			count++;
			scan++;
		}
		break;
	case WHITESP:
		while (scan != regnomore && isspace(*scan)) {
			count++;
			scan++;
		}
		break;
	case NWHITESP:
		while (scan != regnomore && !isspace(*scan)) {
			count++;
			scan++;
		}
		break;
	case ALNUM:
		while (scan != regnomore && isident(*scan)) {
			count++;
			scan++;
		}
		break;
	case NALNUM:
		while (scan != regnomore && !isident(*scan)) {
			count++;
			scan++;
		}
		break;
	case DIGIT:
		while (scan != regnomore && isdigit(*scan)) {
			count++;
			scan++;
		}
		break;
	case NDIGIT:
		while (scan != regnomore && !isdigit(*scan)) {
			count++;
			scan++;
		}
		break;
	case PRINT:
		while (scan != regnomore &&
				(isprint(*scan) || isspace(*scan))) {
			count++;
			scan++;
		}
		break;
	case NPRINT:
		while (scan != regnomore &&
				!(isprint(*scan) || isspace(*scan))) {
			count++;
			scan++;
		}
		break;
	default:		/* Oh dear.  Called inappropriately. */
		regerror("internal foulup");
		count = 0;	/* Best compromise. */
		break;
	}
	reginput = scan;

	return(count);
}

/*
 - regnext - dig the "next" pointer out of a node
 */
char *
regnext(p)
register char *p;
{
	register int offset;

	if (p == &regdummy)
		return(NULL);

	offset = NEXT(p);
	if (offset == 0)
		return(NULL);

	if (OP(p) == BACK)
		return(p-offset);
	else
		return(p+offset);
}

#ifdef REGDEBUG

STATIC char *regprop();

/*
 - regdump - dump a regexp onto stdout in vaguely comprehensible form
 */
void
regdump(r)
regexp *r;
{
	register char *s;
	register char op = EXACTLY;	/* Arbitrary non-END op. */
	register char *next;


	s = r->program + 1;
	while (op != END) {	/* While that wasn't END last time... */
		op = OP(s);
		printf("%2d%s", s-r->program, regprop(s));	/* Where, what. */
		next = regnext(s);
		if (next == NULL)		/* Next ptr. */
			printf("(0)");
		else
			printf("(%d)", (s-r->program)+(next-s));
		s += 3;
		if (op == ANYOF || op == ANYBUT || op == EXACTLY) {
			/* Literal string, where present. */
			while (*s != EOS) {
				putchar(*s);
				s++;
			}
			s++;
		}
		printf("\r\n");
	}

	/* Header fields of interest. */
	if (r->regstart != EOS)
		printf("start `%c' ", r->regstart);
	if (r->reganch)
		printf("anchored ");
	if (r->regmust != -1)
		printf("must have \"%s\"", &(r->program[r->regmust]));
	printf("\r\n");
}

/*
 - regprop - printable representation of opcode
 */
char *
regprop(op)
char *op;
{
	register char *p;
	static char buf[50];

	(void) strcpy(buf, ":");

	switch (OP(op)) {
	case BOL:
		p = "BOL";
		break;
	case EOL:
		p = "EOL";
		break;
	case ANY:
		p = "ANY";
		break;
	case ANYOF:
		p = "ANYOF";
		break;
	case ANYBUT:
		p = "ANYBUT";
		break;
	case BRANCH:
		p = "BRANCH";
		break;
	case EXACTLY:
		p = "EXACTLY";
		break;
	case NOTHING:
		p = "NOTHING";
		break;
	case BACK:
		p = "BACK";
		break;
	case END:
		p = "END";
		break;
	case BEGWORD:
		p = "BEGWORD";
		break;
	case ENDWORD:
		p = "ENDWORD";
		break;
	case WHITESP:
		p = "WHITESP";
		break;
	case NWHITESP:
		p = "NWHITESP";
		break;
	case ALNUM:
		p = "ALNUM";
		break;
	case NALNUM:
		p = "NALNUM";
		break;
	case DIGIT:
		p = "DIGIT";
		break;
	case NDIGIT:
		p = "NDIGIT";
		break;
	case PRINT:
		p = "PRINT";
		break;
	case NPRINT:
		p = "NPRINT";
		break;
	case OPEN+1:
	case OPEN+2:
	case OPEN+3:
	case OPEN+4:
	case OPEN+5:
	case OPEN+6:
	case OPEN+7:
	case OPEN+8:
	case OPEN+9:
		(void)sprintf(buf+strlen(buf), "OPEN%d", OP(op)-OPEN);
		p = NULL;
		break;
	case CLOSE+1:
	case CLOSE+2:
	case CLOSE+3:
	case CLOSE+4:
	case CLOSE+5:
	case CLOSE+6:
	case CLOSE+7:
	case CLOSE+8:
	case CLOSE+9:
		(void)sprintf(buf+strlen(buf), "CLOSE%d", OP(op)-CLOSE);
		p = NULL;
		break;
	case STAR:
		p = "STAR";
		break;
	case PLUS:
		p = "PLUS";
		break;
	default:
		regerror("corrupted opcode");
		break;
	}
	if (p != NULL)
		(void) strcat(buf, p);
	return(buf);
}
#endif

/*
 * The following is provided for those people who do not have strcspn() in
 * their C libraries.  They should get off their butts and do something
 * about it; at least one public-domain implementation of those (highly
 * useful) string routines has been published on Usenet.
 */
#ifdef STRCSPN
/*
 * strcspn - find length of initial segment of s1 consisting entirely
 * of characters not from s2
 */

int
strcspn(s1, s2)
char *s1;
char *s2;
{
	register char *scan1;
	register char *scan2;
	register int count;

	count = 0;
	for (scan1 = s1; *scan1 != EOS; scan1++) {
		for (scan2 = s2; *scan2 != EOS;)	/* ++ moved down. */
			if (*scan1 == *scan2++)
				return(count);
		count++;
	}
	return(count);
}
#endif

/* vile support:
 * like regexec, but takes LINE * as input instead of char *
 */
int
lregexec(prog, lp, startoff, endoff)
register regexp *prog;
register LINE *lp;
register int startoff;
register int endoff;
{
	if (endoff < startoff)
		return 0;

	if (lp->l_text) {
		return regexec(prog, lp->l_text, &(lp->l_text[llength(lp)]),
					startoff, endoff);
	} else {
		/* the prog might be ^$, or something legal on a null string */

		char *nullstr = "";
		int s;

		if (startoff > 0)
			return 0;
		s = regexec(prog, nullstr, nullstr, 0, 0);
		if (s) {
			if (prog->mlen > 0) {
				mlforce("BUG: non-zero match on null string");
				return 0;
			}
			prog->startp[0] = prog->endp[0] = NULL;
		}
		return s;
	}

}
