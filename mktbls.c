#include <stdio.h>

/* This standalone utility program constructs the function, key and command
	binding tables for vile.  The input is a data file containing the
	desired default relationships among the three entities.  Output
	is nebind.h, nefunc.h, and nename.h, all of which are then included
	in main.c
	This code written by Paul Fox, (c)1990
	
	See the file "cmdtbls" for data formats.
	
*/

char *progcreat = "/* %s: this header file was produced automatically by\n\
 * the %s program, based on input from the file %s\n */\n";

#define	DIFCNTRL	0x40
#define tocntrl(c)	((c)^DIFCNTRL)
#define toalpha(c)	((c)^DIFCNTRL)
#define	DIFCASE		0x20
#define toupper(c)	((c)^DIFCASE)
#define tolower(c)	((c)^DIFCASE)

int l = 0;
FILE *nebind, *nefunc, *nename, *cmdtbl;

char *malloc();
char *formcond();

struct stringl {
	char *s1;
	char *s2;
	char *s3;
	struct stringl *nst;
};
	
main(argc, argv)
char    *argv[];
{
	char line[100];
	char func[50];
	char flags[50];
	char english[50];
	char fcond[50];
	char ncond[50];
	char key[50];
	char bcond[50];
	int r;
	
	if (argc != 2) {
		fprintf(stderr, "usage: mktbls cmd-file\n");
		exit(1);
	}
	
	if ((cmdtbl = fopen(argv[1],"r")) == NULL ) {
		fprintf(stderr,"mktbls: couldn't open cmd-file\n");
		exit(1);
	}
	
	if ( (nebind = fopen("nebind.h","w")) == NULL ||
		(nefunc = fopen("nefunc.h","w")) == NULL ||
		(nename = fopen("nename.h","w")) == NULL ) {
		fprintf(stderr,"mktbls: couldn't open header files\n");
		exit(1);
	}
	
	fprintf(nebind,progcreat,"nebind.h",argv[0],argv[1]);
	fprintf(nefunc,progcreat,"nefunc.h",argv[0],argv[1]);
	fprintf(nename,progcreat,"nename.h",argv[0],argv[1]);
	
	
	/* process each input line */
	while (fgets(line,100,cmdtbl) != NULL) {
		l++;
		if (line[0] == '#') {  /* comment */
			continue;
		} else if (line[0] == '\n') { /* empty line */
			continue;
		} else if (line[0] != '\t') { /* then it's a new func */
			/* we can spill information about funcs right away */
			r = sscanf(line,"%s %s %s",func,flags,fcond);
			if (r < 2 || r > 3)
				badfmt("looking for new function");
			if (r != 3)
				fcond[0] = '\0';
				
			if (fcond[0])
				fprintf(nefunc,"#if %s\n",fcond);
			fprintf(nefunc,"extern int %s();\n", func);
			fprintf(nefunc,"\tCMDFUNC f_%s = { %s,\t%s };\n",
				func, func, flags);
			if (fcond[0])
				fprintf(nefunc,"#endif\n");
				
		} else if (line[1] == '"') { /* then it's an english name */
		
			r = sscanf(line, " \"%[^\"]\"	%s", english,ncond);
			if (r < 1 || r > 2)
				badfmt("looking for english name");
			if (r != 2)
				ncond[0] = '\0';
				
			savenames(english, func, formcond(fcond,ncond));
				
		} else if (line[1] == '\'') { /* then it's a key */
			r = sscanf(&line[2], "%[^']' %s", key, ncond);
			if (r < 1 || r > 2)
				badfmt("looking for key binding");
			if (r != 2)
				ncond[0] = '\0';
				
			savebindings(key, func, formcond(fcond,ncond));
			
		} else {
			badfmt("bad line");
		}
	}
	
	dumpnames();
	dumpbindings();
	
	exit(0);
}

char *
formcond(c1,c2)
char *c1, *c2;
{
	static char cond[50];
	if (c1[0] && c2[0])
		sprintf(cond,"#if (%s) & (%s)\n",c1,c2);
	else if (c1[0] || c2[0])
		sprintf(cond,"#if (%s%s)\n",c1,c2);
	else
		cond[0] = '\0';
	return cond;
}

badfmt(s)
{
	fprintf(stderr,"\"cmdtbl\", line %d: bad format:",l);
	fprintf(stderr,"	%s\n",s);
	exit(1);
}

#define ASCIIBIND 0
#define CTLXBIND 1
#define METABIND 2
#define SPECBIND 3
char *bindings[4][128];
char *conditions[4][128];
char *tblname[] = {"asciitbl", "ctlxtbl", "metatbl", "spectbl" };
char *prefname[] = {"", "CTLX|", "META|", "SPEC|" };

/* prc2kcod: translate printable code to C-language keycode */
savebindings(s,func,cond)
char *s, *func, *cond;
{
	int btype, c;
	
	btype = ASCIIBIND;
	
	if (*s == 'M' && *(s+1) == '-') {
		s += 2;
		btype = METABIND;
	} else if (*s == 'F' && *(s+1) == 'N' && *(s+2) == '-') {
		btype = SPECBIND;
		s += 3;
	} else if (*s == '^' && *(s+1) == 'X'&& *(s+2) == '-') {
		btype = CTLXBIND;
		s += 3;
	}
	
	if (*s == '\\') { /* try for an octal value */
		c = 0;
		while (*++s < '8' && *s >= '0') c = (c*8) + *s - '0';
		if (c > 127)
			badfmt("octal character");
		if (bindings[btype][c] != NULL)
			badfmt("duplicate key binding");
		bindings[btype][c] = malloc(strlen(func)+1);
		strcpy(bindings[btype][c], func);
	} else if (*s == '^' && (c = *(s+1)) != '\0') { /* a control char? */
		if (c > 'a' &&  c < 'z')
			c = toupper(c);
		c = tocntrl(c);
		if (bindings[btype][c] != NULL)
			badfmt("duplicate key binding");
		bindings[btype][c] = malloc(strlen(func)+1);
		strcpy(bindings[btype][c], func);
		s += 2;
	} else if (c = *s) {
		if (bindings[btype][c] != NULL)
			badfmt("duplicate key binding");
		bindings[btype][c] = malloc(strlen(func)+1);
		strcpy(bindings[btype][c], func);
		s++;
	} else {
		badfmt("getting binding");
	}
	if (cond[0]) {
		conditions[btype][c] = malloc(strlen(cond)+1);
		strcpy(conditions[btype][c], cond);
	} else {
		conditions[btype][c] = NULL;
	}
	
	if (*s != '\0')
		badfmt("got extra characters");
	
}

dumpbindings()
{
	char **bindptr;
	char *sctl;
	int i, c, btype;
	
	btype = ASCIIBIND;
	
	fprintf(nebind,"\nCMDFUNC *%s[128] = {\n",tblname[btype]);
	for (i = 0; i < 128; i++) {
		if (conditions[btype][i]) {
			fprintf(nebind,"%s", conditions[btype][i]);
		}
		if (i < ' ' || i > '~' ) {
			sctl = "ctrl-";
			c = toalpha(i);
		} else {
			sctl = "";
			c = i;
		}
			
		if (bindings[btype][i])
			fprintf(nebind,"	&f_%s,	/* %s%c */\n",
				bindings[btype][i], sctl, c);
		else
			fprintf(nebind,"	NULL,	/* %s%c */\n", sctl, c);
		if (conditions[btype][i]) {
			fprintf(nebind,"#else\n	NULL,\n#endif\n");
		}
			
	}
	fprintf(nebind,"};\n");
	
	
	fprintf(nebind,"\nKBIND kbindtbl[NBINDS] = {\n");
	for (btype = 1; btype <= 3; btype++) {
		for (i = 0; i < 128; i++) {
			if (bindings[btype][i]) {
				if (conditions[btype][i]) {
					fprintf(nebind,"%s",
						conditions[btype][i]);
				}
				if (i < ' ')
					fprintf(nebind,
					"	{ %stocntrl('%c'), &f_%s },\n",
						prefname[btype],
						toalpha(i),bindings[btype][i]);
				else
					fprintf(nebind,
					"	{ %s'%c', &f_%s },\n",
						prefname[btype],
						i, bindings[btype][i]);
				if (conditions[btype][i]) {
					fprintf(nebind,"#endif\n");
				}
			}
		}
	}
	fprintf(nebind,"	{ 0, NULL }\n");
	fprintf(nebind,"};\n");
}

struct stringl lastname = {"\177\177\177\177\177\177", "", "", NULL};
struct stringl lnamecond = {"", "", "", NULL};
struct stringl firstname = {"", "", "", &lastname};
struct stringl fnamecond = {"", "", "", &lnamecond};

savenames(name,func,cond)
char *name, *func, *cond;
{
	char tmpline[80];
	struct stringl *n, *m;
	int r;
	
	n = (struct stringl *)malloc(sizeof (struct stringl));
	
	/* strtolower(name); */
	
	n->s1 = (char *)malloc(strlen(name)+1);
	strcpy(n->s1, name);
	
	sprintf(tmpline,"\t{ \"%s\",\t&f_%s },\n",
		name, func);
	n->s2 = (char *)malloc(strlen(func)+1);
	strcpy(n->s2, func);
	
	n->s3 = (char *)malloc(strlen(cond)+1);
	strcpy(n->s3, cond);
	
	for (m = &firstname; m->nst != NULL; m = m->nst) {
		if ((r = strcmp(n->s1, m->nst->s1)) < 0) { /* insert it here */
			n->nst = m->nst;
			m->nst = n;
			break;
		} else if (r == 0) {
			badfmt("duplicate english name");
		}
	}
}

dumpnames()
{
	struct stringl *m;
	
	fprintf(nename,"\n/* if you maintain this by hand, keep it in */\n");
	fprintf(nename,"/* alphabetical order!!!! */\n\n");
	fprintf(nename,"NTAB nametbl[] = {\n");
	for (m = firstname.nst; m->nst != NULL; m = m->nst) {
		if (m->s3[0])
			fprintf(nename,"%s",m->s3);
		fprintf(nename,"\t{ \"%s\",\t&f_%s },\n", m->s1, m->s2);
		if (m->s3[0])
			fprintf(nename,"#endif\n");
	}
	fprintf(nename,"	{ NULL, NULL }\n};\n");
}

strtolower(s)
char *s;
{
	while (*s) {
		if (*s >= 'A' && *s <= 'Z')
			*s = tolower(*s);
		s++;
	}
}
