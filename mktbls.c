/* This standalone utility program constructs the function, key and command
 *	binding tables for vile.  The input is a data file containing the
 *	desired default relationships among the three entities.  Output
 *	is nebind.h, nefunc.h, and nename.h, all of which are then included
 *	in main.c
 *	This code written by Paul Fox, (c)1990
 *
 *	See the file "cmdtbls" for input data formats, and "estruct.h" for
 *	the output structures.
 *
 * $Log: mktbls.c,v $
 * Revision 1.11  1993/01/23 14:44:08  foxharp
 * local copies of isspace and isprint, since not everyone has them
 * in a standard library.
 *
 * Revision 1.10  1993/01/23  13:38:23  foxharp
 * tom's changes, for lint, cleanup, variables, and functions
 *
 * Revision 1.9  1992/07/17  19:12:44  foxharp
 * explicit int return on func
 *
 * Revision 1.8  1992/07/08  08:35:09  foxharp
 * don't report duplicate keymaps if the pre-processor conditionals are
 * both present, and they don't match -- in that case, they're unlikely
 * to both compile in at the same time.
 *
 * also, be sure to emit backslash escapes when printing ' and \ character
 * constants, i.e. '\'' and '\\'
 *
 * Revision 1.7  1992/05/29  08:34:01  foxharp
 * prototypes, gcc -W cleanups
 *
 * Revision 1.6  1991/11/07  03:29:12  pgf
 * lint cleanup
 *
 * Revision 1.5  1991/11/01  14:38:00  pgf
 * saber cleanup
 *
 * Revision 1.4  1991/08/07  12:35:07  pgf
 * added RCS log messages
 *
 * revision 1.3
 * date: 1991/06/03 17:34:57;
 * switch from "meta" etc. to "ctla" etc.
 * 
 * revision 1.2
 * date: 1991/06/03 10:26:03;
 * commentary change
 * 
 * revision 1.1
 * date: 1990/09/21 10:25:40;
 * initial vile RCS revision
 */

#include <stdio.h>
#include <string.h>

#define	MAX_BIND	4	/* total # of binding-types */
#define	MAX_PARSE	5	/* maximum # of tokens on line */
#define	LEN_BUFFER	50	/* nominal buffer-length */
#define	MAX_BUFFER	(LEN_BUFFER*10)
#define	LEN_CHRSET	128	/* total # of chars in set (ascii) */

#define	DIFCNTRL	0x40
#define tocntrl(c)	((c)^DIFCNTRL)
#define toalpha(c)	((c)^DIFCNTRL)
#define	DIFCASE		0x20
#define toupper(c)	((c)^DIFCASE)
#define tolower(c)	((c)^DIFCASE)

#define	Fprintf	(void)fprintf
#define	Sprintf	(void)sprintf

#define	SaveEndif(head)	InsertOnEnd(&head, "#endif")

extern char *malloc();


typedef	struct stringl {
	char *s1;	/* stores primary-data */
	char *s2;	/* stores secondary-data */
	char *s3;	/* stores ifdef-flags */
	char *s4;	/* stores comment, if any */
	struct stringl *nst;
} LIST;

static	LIST	*all_names,
		*all_funcs,
		*all_FUNCs,
		*all_envars,
		*all_ufuncs;

	/* definitions for sections of cmdtbl */
#define	SECT_CMDS 0
#define	SECT_FUNC 1
#define	SECT_VARS 2
#define	SECT_BUFF 3
#define	SECT_WIND 4

	/* definitions for [MAX_BIND] indices */
#define ASCIIBIND 0
#define CTLXBIND 1
#define CTLABIND 2
#define SPECBIND 3

static	char *bindings  [MAX_BIND][LEN_CHRSET];
static	char *conditions[MAX_BIND][LEN_CHRSET];
static	char *tblname   [MAX_BIND] = {"asciitbl", "ctlxtbl", "metatbl", "spectbl" };
static	char *prefname  [MAX_BIND] = {"",         "CTLX|",   "CTLA|",   "SPEC|" };

static	int l = 0;
static	FILE *nebind, *nefunc, *nename, *cmdtbl;
static	FILE *nevars;



/******************************************************************************/
static isspace(c)
int c;
{
	return c == ' ' || c == '\t' || c == '\n';
}
static isprint(c)
int c;
{
	return c >= ' ' && c < '\0177';
}
/******************************************************************************/
static
char *	StrAlloc(s)
	char	*s;
{
	return strcpy(malloc((unsigned)strlen(s)+1), s);
}

static
LIST *	ListAlloc()
{
	return (LIST *)malloc(sizeof(LIST));
}

/******************************************************************************/
static
void	badfmt(s)
	char *s;
{
	Fprintf(stderr,"\"cmdtbl\", line %d: bad format:",l);
	Fprintf(stderr,"	%s\n",s);
	exit(1);
}

static
void	badfmt2(s,col)
	char *s;
	int col;
{
	char	temp[BUFSIZ];
	Sprintf(temp, "%s (column %d)", s, col);
	badfmt(temp);
}

/******************************************************************************/
static
void	WriteLines(fp, list, count)
	FILE	*fp;
	char	**list;
	int	count;
{
	while (count-- > 0)
		Fprintf(fp, "%s\n", *list++);
}
#define	write_lines(fp,list) WriteLines(fp, list, sizeof(list)/sizeof(list[0]))

/******************************************************************************/
static
FILE *	OpenHeader(name, argv)
	char	*name;
	char	**argv;
{
	register FILE *fp;
	static char *progcreat =
"/* %s: this header file was produced automatically by\n\
 * the %s program, based on input from the file %s\n */\n";

	if (!(fp = fopen(name, "w"))) {
		Fprintf(stderr,"mktbls: couldn't open header file %s\n", name);
		exit(1);
	}
	Fprintf(fp, progcreat, name, argv[0], argv[1]);
	return fp;
}

/******************************************************************************/
static
void	InsertSorted(headp, name, func, cond, note)
	LIST	**headp;
	char	*name;
	char	*func;
	char	*cond;
	char	*note;
{
	register LIST *n, *p, *q;
	register int  r;

	n = ListAlloc();
	n->s1 = StrAlloc(name);
	n->s2 = StrAlloc(func);
	n->s3 = StrAlloc(cond);
	n->s4 = StrAlloc(note);

	for (p = *headp, q = 0; p != 0; q = p, p = p->nst) {
		if ((r = strcmp(n->s1, p->s1)) < 0)
			break;
		else if (r == 0)
			badfmt("duplicate name");
	}
	n->nst = p;
	if (q == 0)
		*headp = n;
	else
		q->nst = n;
}

static
void	InsertOnEnd(headp, name)
	LIST	**headp;
	char	*name;
{
	register LIST *n, *p, *q;

	n = ListAlloc();
	n->s1 = StrAlloc(name);
	n->s2 = "";
	n->s3 = "";
	n->s4 = "";

	for (p = *headp, q = 0; p != 0; q = p, p = p->nst)
		;

	n->nst = 0;
	if (q == 0)
		*headp = n;
	else
		q->nst = n;
}

/******************************************************************************/
static
char *	append(dst, src)
	char	*dst;
	char	*src;
{
	(void)strcat(dst, src);
	return (dst + strlen(dst));
}

static
char *	formcond(c1,c2)
	char *c1, *c2;
{
	static char cond[LEN_BUFFER];
	if (c1[0] && c2[0])
		Sprintf(cond,"#if (%s) & (%s)\n",c1,c2);
	else if (c1[0] || c2[0])
		Sprintf(cond,"#if (%s%s)\n",c1,c2);
	else
		cond[0] = '\0';
	return cond;
}

static
int	LastCol(buffer)
	char	*buffer;
{
	register int	col = 0,
			c;
	while (c = *buffer++) {
		if (isprint(c))
			col++;
		else if (c == '\t')
			col = (col | 7) + 1;
	}
	return col;
}

static
char *	PadTo(col, buffer)
	int col;
	char	*buffer;
{
	int	any	= 0,
		len	= strlen(buffer),
		now,
		with;

	for (;;) {
		if ((now = LastCol(buffer)) >= col) {
			if (any)
				break;
			else
				with = ' ';
		} else if (col-now > 1)
			with = '\t';
		else
			with = ' ';

		buffer[len++] = with;
		buffer[len]   = '\0';
		any++;
	}
	return buffer;
}

static
int	two_conds(btype,c,cond)
	int btype,c;
	char *cond;
{
	/* return true if both bindings have different
	 conditions associated with them */
	return (cond[0] != 0 &&
		conditions[btype][c] != NULL &&
		strcmp(cond, conditions[btype][c]) != 0);
}

/******************************************************************************/
static
void	save_bindings(s,func,cond)
	char *s, *func, *cond;
{
	int btype, c;

	btype = ASCIIBIND;

	if (*s == '^' && *(s+1) == 'A'&& *(s+2) == '-') {
		btype = CTLABIND;
		s += 3;
	} else if (*s == 'F' && *(s+1) == 'N' && *(s+2) == '-') {
		btype = SPECBIND;
		s += 3;
	} else if (*s == '^' && *(s+1) == 'X'&& *(s+2) == '-') {
		btype = CTLXBIND;
		s += 3;
	}

	if (*s == '\\') { /* try for an octal value */
		c = 0;
		while (*++s < '8' && *s >= '0')
			c = (c*8) + *s - '0';
		if (c >= LEN_CHRSET)
			badfmt("octal character too big");
		if (bindings[btype][c] != NULL && !two_conds(btype,c,cond))
			badfmt("duplicate key binding");
		bindings[btype][c] = StrAlloc(func);
	} else if (*s == '^' && (c = *(s+1)) != '\0') { /* a control char? */
		if (c > 'a' &&  c < 'z')
			c = toupper(c);
		c = tocntrl(c);
		if (bindings[btype][c] != NULL && !two_conds(btype,c,cond))
			badfmt("duplicate key binding");
		bindings[btype][c] = StrAlloc(func);
		s += 2;
	} else if (c = *s) {
		if (bindings[btype][c] != NULL && !two_conds(btype,c,cond))
			badfmt("duplicate key binding");
		bindings[btype][c] = StrAlloc(func);
		s++;
	} else {
		badfmt("getting binding");
	}
	if (cond[0]) {
		conditions[btype][c] = StrAlloc(cond);
	} else {
		conditions[btype][c] = NULL;
	}

	if (*s != '\0')
		badfmt("got extra characters");
}

static
void	dump_bindings()
{
	char	temp[MAX_BUFFER];
	char	old_cond[LEN_BUFFER], *new_cond;
	char *sctl;
	int i, c, btype;

	btype = ASCIIBIND;

	Fprintf(nebind,"\nCMDFUNC *%s[%d] = {\n",tblname[btype], LEN_CHRSET);
	for (i = 0; i < LEN_CHRSET; i++) {
		if (conditions[btype][i]) {
			Fprintf(nebind,"%s", conditions[btype][i]);
		}
		if (i < ' ' || i > '~' ) {
			sctl = "ctrl-";
			c = toalpha(i);
		} else {
			sctl = "";
			c = i;
		}

		if (bindings[btype][i])
			Sprintf(temp, "\t&f_%s,", bindings[btype][i]);
		else
			Sprintf(temp, "\tNULL,");

		Fprintf(nebind, "%s/* %s%c */\n", PadTo(32, temp), sctl, c);

		if (conditions[btype][i]) {
			Fprintf(nebind,"#else\n\tNULL,\n#endif\n");
		}

	}
	Fprintf(nebind,"};\n");

	Fprintf(nebind,"\nKBIND kbindtbl[NBINDS] = {\n");
	for (btype = 1; btype <= 3; btype++) {
		*old_cond = '\0';
		for (i = 0; i < LEN_CHRSET; i++) {
			if (bindings[btype][i]) {
				if (!(new_cond = conditions[btype][i]))
					new_cond = "";
				if (strcmp(old_cond, new_cond)) {
					if (*old_cond)
						Fprintf(nebind,"#endif\n");
					Fprintf(nebind,"%s",new_cond);
					(void)strcpy(old_cond,new_cond);
				}
				if (i < ' ') {
					Sprintf(temp,
					"\t{ %stocntrl('%c'),",
						prefname[btype],
						toalpha(i));
				} else {
					Sprintf(temp,
					"\t{ %s'%s%c',",
						prefname[btype],
						(i == '\'' || i == '\\') ?
							"\\":"",
						i);
				}
				Fprintf(nebind, "%s&f_%s },\n",
					PadTo(32, temp), bindings[btype][i]);
			}
		}
		if (*old_cond)
			Fprintf(nebind,"#endif\n");
	}
	Fprintf(nebind,"	{ 0, NULL }\n");
	Fprintf(nebind,"};\n");
}

/******************************************************************************/
static
void	start_evar_h(argv)
	char	**argv;
{
	static	char	*head[] = {
		"",
		"#if !SMALLER",
		"",
		"/*\tstructure to hold user variables and their definitions\t*/",
		"",
		"typedef struct UVAR {",
		"\tchar u_name[NVSIZE + 1];\t\t/* name of user variable */",
		"\tchar *u_value;\t\t\t\t/* value (string) */",
		"} UVAR;",
		"",
		"/*\tcurrent user variables (This structure will probably change)\t*/",
		"",
		"#define\tMAXVARS\t\t10 \t/* was 255 */",
		"",
		"UVAR uv[MAXVARS];\t/* user variables */",
		};

	if (!nevars) {
		nevars = OpenHeader("nevars.h", argv);
		write_lines(nevars, head);
	}
}

static
void	finish_evar_h()
{
	if (nevars)
		Fprintf(nevars, "\n#endif\n");
}

/******************************************************************************/
static
void	init_envars()
{
	static	char	*head[] = {
		"",
		"/*\tlist of recognized environment variables\t*/",
		"",
		"char *envars[] = {"
		};
	static	int	done;

	if (!done++)
		write_lines(nevars, head);
}

static
void	save_envars(vec)
	char	**vec;
{
	InsertSorted(&all_envars, vec[1], vec[2], vec[3], vec[0]);
}

static
void	dump_envars()
{
	static	char *middle[] = {
		"};",
		"",
		"#define\tNEVARS\tSIZEOF(envars)",
		"",
		"/* \tand its preprocesor definitions\t\t*/",
		""
		};
	char	temp[MAX_BUFFER];
	register LIST *p;
	register int count;

	for (p = all_envars, count = 0; p != 0; p = p->nst) {
		if (!count++)
			init_envars();
		if (p->s3[0])
			Fprintf(nevars, "#if %s\n", p->s3);
		Sprintf(temp, "\t\"%s\",", p->s1);
		if (p->s4[0]) {
			(void)PadTo(24, temp);
			Sprintf(temp+strlen(temp), "/* %s */", p->s4);
		}
		Fprintf(nevars, "%s\n", temp);
		if (p->s3[0])
			Fprintf(nevars, "#endif\n");
	}
	for (p = all_envars, count = 0; p != 0; p = p->nst) {
		if (!count)
			write_lines(nevars, middle);
		Sprintf(temp, "#define\tEV%s", p->s2);
		Fprintf(nevars, "%s%d\n", PadTo(24, temp), count++);
	}
}

/******************************************************************************/
static
void	save_funcs(func, flags, cond, old_cond)
	char	*func;
	char	*flags;
	char	*cond;
	char	*old_cond;
{
	char	temp[MAX_BUFFER];
	register char	*s;

	if (strcmp(cond, old_cond)) {
		if (*old_cond) {
			SaveEndif(all_funcs);
			SaveEndif(all_FUNCs);
		}
		if (*cond) {
			Sprintf(temp, "#if %s", cond);
			InsertOnEnd(&all_funcs, temp);
			InsertOnEnd(&all_FUNCs, temp);
		}
		(void)strcpy(old_cond, cond);
	}
	Sprintf(temp, "extern int %s P(( int, int ));", func);
	InsertOnEnd(&all_funcs, temp);

	s = append(strcpy(temp, "\tCMDFUNC f_"), func);
	(void)PadTo(32, temp);
	s = append(append(append(s, "= { "), func), ",");
	(void)PadTo(56, temp);
	(void)append(append(s, flags), " };");
	InsertOnEnd(&all_FUNCs, temp);
}

static
void	dump_funcs(head)
	LIST	*head;
{
	register LIST *p;
	for (p = head; p != 0; p = p->nst)
		Fprintf(nefunc, "%s\n", p->s1);
}

/******************************************************************************/
static
void	save_names(name,func,cond)
	char *name, *func, *cond;
{
	char tmpline[80];

	Sprintf(tmpline,"\t{ \"%s\",\t&f_%s },\n", name, func);
	InsertSorted(&all_names, name, func, cond, "");
}

static
void	dump_names()
{
	register LIST *m;
	char	temp[MAX_BUFFER];
	char	old_s3[LEN_BUFFER];

	Fprintf(nename,"\n/* if you maintain this by hand, keep it in */\n");
	Fprintf(nename,"/* alphabetical order!!!! */\n\n");
	Fprintf(nename,"NTAB nametbl[] = {\n");
	*old_s3 = '\0';

	for (m = all_names; m != NULL; m = m->nst) {
		if (strcmp(old_s3, m->s3)) {
			if (*old_s3)
				Fprintf(nename,"#endif\n");
			if (m->s3[0])
				Fprintf(nename,"%s",m->s3);
			(void)strcpy(old_s3, m->s3);
		}
		Sprintf(temp, "\t{ \"%s\",", m->s1);
		Fprintf(nename, "%s&f_%s },\n", PadTo(40, temp), m->s2);
	}
	if (*old_s3)
		Fprintf(nename,"#endif\n");
	Fprintf(nename,"	{ NULL, NULL }\n};\n");
}

/******************************************************************************/
static
void	init_ufuncs()
{
	static	char	*head[] = {
		"",
		"/*\tlist of recognized user functions\t*/",
		"",
		"typedef struct UFUNC {",
		"\tchar *f_name;\t/* name of function */",
		"\tint f_type;\t/* 1 = monamic, 2 = dynamic */",
		"} UFUNC;",
		"",
		"#define\tNILNAMIC\t0",
		"#define\tMONAMIC\t\t1",
		"#define\tDYNAMIC\t\t2",
		"#define\tTRINAMIC\t3",
		"",
		"UFUNC funcs[] = {",
		};
	static	int	done;

	if (!done++)
		write_lines(nevars, head);
}

static
void	save_ufuncs(vec)
	char	**vec;
{
	InsertSorted(&all_ufuncs, vec[1], vec[2], vec[3], vec[0]);
}

static
void	dump_ufuncs()
{
	static	char	*middle[] = {
		"};",
		"",
		"#define\tNFUNCS\tSIZEOF(funcs)",
		"",
		"/* \tand its preprocesor definitions\t\t*/",
		"",
		};
	char	temp[MAX_BUFFER];
	register LIST *p;
	register int	count;

	for (p = all_ufuncs, count = 0; p != 0; p = p->nst) {
		if (!count++)
			init_ufuncs();
		Sprintf(temp, "\t\"%s\",", p->s1);
		(void)PadTo(15, temp);
		Sprintf(temp+strlen(temp), "%s,", p->s3);
		if (p->s4[0]) {
			(void)PadTo(32, temp);
			Sprintf(temp+strlen(temp), "/* %s */", p->s4);
		}
		Fprintf(nevars, "%s\n", temp);
	}
	for (p = all_ufuncs, count = 0; p != 0; p = p->nst) {
		if (!count)
			write_lines(nevars, middle);
		Sprintf(temp, "#define\tUF%s", p->s2);
		Fprintf(nevars, "%s%d\n", PadTo(24, temp), count++);
	}
}

/******************************************************************************/
	/* returns the number of non-comment tokens parsed, with a list of
	 * tokens (0=comment) as a side-effect.  Note that quotes are removed
	 * from the token, so we have to have them only in the first token! */
static
int	Parse(input, vec)
	char	*input;
	char	**vec;
{
	char	*base = input;
	register int	expecting = 1,
			count = 0,
			quote = 0,
			c;

	for (c = 0; c < MAX_BIND; c++)
		vec[c] = "";
	for (c = strlen(input); c > 0 && isspace(input[c-1]); c--)
		input[c-1] = '\0';

	while (c = *input++) {
		if (quote) {
			if (c == quote) {
				quote = 0;
				if (*input && !isspace(*input))
					badfmt2("expected blank", input-base);
				input[-1] = '\0';
			}
		} else {
			if ((c == '"') || (c == '\'')) {
				quote = c;
			} else if (isspace(c)) {
				input[-1] = '\0';
				expecting = 1;
			} else if ((c == '#')) {
				while (isspace(*input))
					input++;
				vec[0] = input;
				break;
			}
			if (expecting && !isspace(c)) {
				if (count+1 >= MAX_PARSE)
					break;
				vec[++count] = input - (c != quote);
				expecting = 0;
			}
		}
	}
	return count;
}

/******************************************************************************/
int
main(argc, argv)
int	argc;
char    *argv[];
{
	char *vec[MAX_PARSE];
	char line[MAX_BUFFER];
	char func[LEN_BUFFER];
	char flags[LEN_BUFFER];
	char old_fcond[LEN_BUFFER],	fcond[LEN_BUFFER];
	int section = SECT_CMDS;
	int r;

	if (argc != 2) {
		Fprintf(stderr, "usage: mktbls cmd-file\n");
		exit(1);
	}

	if ((cmdtbl = fopen(argv[1],"r")) == NULL ) {
		Fprintf(stderr,"mktbls: couldn't open cmd-file\n");
		exit(1);
	}

	nebind = OpenHeader("nebind.h", argv);
	nefunc = OpenHeader("nefunc.h", argv);
	nename = OpenHeader("nename.h", argv);

	*old_fcond = '\0';

	/* process each input line */
	while (fgets(line, sizeof(line), cmdtbl) != NULL) {
		l++;
		switch (line[0]) {
		case '#':		/* comment */
		case '\n':		/* empty-list */
			break;

		case '.':		/* a new section */
			switch (line[1]) {
			case 'c':
				section = SECT_CMDS;
				break;
			case 'e':
				section = SECT_VARS;
				start_evar_h(argv);
				break;
			case 'f':
				section = SECT_FUNC;
				start_evar_h(argv);
				break;
			case 'b':
				section = SECT_BUFF;
				break;
			case 'w':
				section = SECT_WIND;
				break;
			default:
				badfmt("unknown section");
			}
			break;

		case '\t':		/* a new function */
			if (section != SECT_CMDS)
				badfmt("did not expect a tab");

			switch (line[1]) {
			case '"':	/* then it's an english name */

				r = Parse(line, vec);
				if (r < 1 || r > 2)
					badfmt("looking for english name");

				save_names(vec[1], func, formcond(fcond,vec[2]));
				break;

			case '\'':	/* then it's a key */
				r = Parse(line, vec);
				if (r < 1 || r > 2)
					badfmt("looking for key binding");

				save_bindings(vec[1], func, formcond(fcond,vec[2]));
				break;

			default:
				badfmt("bad line");
			}
			break;

		default:		/* cache information about funcs */
			switch (section) {
			case SECT_CMDS:
				r = Parse(line, vec);
				if (r < 2 || r > 3)
					badfmt("looking for new function");
				if (r != 3)
					fcond[0] = '\0';

				save_funcs(
					strcpy(func,  vec[1]),
					strcpy(flags, vec[2]),
					strcpy(fcond, vec[3]),
					old_fcond);
				break;

			case SECT_VARS:
				r = Parse(line, vec);
				if (r < 2 || r > 3)
					badfmt("looking for char *envars[]");
				save_envars(vec);
				break;

			case SECT_FUNC:
				r = Parse(line, vec);
				if (r < 2 || r > 3)
					badfmt("looking for UFUNC func[]");
				save_ufuncs(vec);
				break;

			case SECT_BUFF:
			case SECT_WIND:
			default:	badfmt("section not implemented");
			}
		}
	}

	if (*old_fcond) {
		SaveEndif(all_funcs);
		SaveEndif(all_FUNCs);
	}

	dump_names();
	dump_bindings();
	dump_funcs(all_funcs);
	dump_funcs(all_FUNCs);

	dump_envars();
	dump_ufuncs();
	finish_evar_h();

	return 0;
}
