/*
 * Program: A simple comment and keyword attributor for vile
 * Author : Jukka Keto, jketo@cs.joensuu.fi
 * Date   : 30.12.1994
 * 
 * Features:
 * 	- Reads the keyword file "$HOME/.vile.keywords".
 *	  Keyword file consists lines "keyword:attribute" where
 *	  keyword is any alphanumeric string [#a-zA-Z0-9_] followed
 *	  by colon ":" and attribute character; "I" for italic, 
 *	  "U" for underline, "B" for bold or "R" for reverse presentation.
 *	- Attributes the file read from stdin using vile attribute sequences
 *	  and outputs the file to stdout with keywords and comments 
 *	  attributed.
 *	- Comments are attributed with underline attribute.
 *	- Here is the macro that is needed with this filter:
 *        30 store-macro
 * 	     write-message "[Attaching C/C++ keyword/comment attributes...]"
 * 	     goto-beginning-of-file
 * 	     filter-til end-of-file "c-filt"
 * 	     goto-beginning-of-file
 * 	     attribute-cntl_a-sequences-til end-of-file
 * 	     unmark-buffer
 * 	     write-message "[Attaching C/C++ keyword/comment attributes...done]"
 *        ~endm
 *        bind-key execute-macro-30 ^X-q
 *
 * Files: ~/.vile.keywords (my examples)
 *	#if:I
 *	#ifdef:I
 *	#include:I
 *	#else:I
 *	#elsif:I
 *	#endif:I
 *	#define:I
 *	#undef:I
 *	if:B
 *	else:B
 *	endif:B
 *	for:B
 *	return:B
 *	while:B
 *	switch:B
 *	case:B
 *	do:B
 *	goto:B
 *	break:B
 *
 * Note:
 *	- I use this to get the coloring effects (in my VT220 PC emulator)
 *	  some commercial programs and epoch offer and with a color_xterm with
 *	  appropriate terminfo.
 *
 * Known Bugs (other features):
 *	- The keyword lists should be ordered for optimal operation.
 *	- Directives with empty space after # are not attributed (may
 *	  be attributed wrongly).
 *
 * To compile (need an ANSI compiler):
 *	cc -o c-filt c-filt.c
 *
 *	$Header: /usr/build/VCS/pgf-vile/RCS/c-filt.c,v 1.3 1995/01/12 21:45:21 pgf Exp $
 */

#include <stdio.h>
#include <ctype.h>

extern	char *malloc( unsigned int );
extern	char *getenv( const char * );

#define MAX_KEYWORD_LENGTH 80
#define HASH_LENGTH 256
#define MAX_LINELENGTH 256
static char *keyword_file=".vile.keywords";

typedef struct _keyword {
    char kw[MAX_KEYWORD_LENGTH+1];
    char attribute;
    int  length;
    struct _keyword *next;
} KEYWORD;

static KEYWORD *hashtable[HASH_LENGTH];
static KEYWORD identifier;
static char comment_attr = 'I';

void inithash()
{
    int i;
    for (i=0;i<HASH_LENGTH;i++) hashtable[i] = NULL; 
}

void removelist(KEYWORD *k)
{
    if (k != NULL) {
	if (k->next != NULL) removelist(k->next);
	free(k);
    }
}

void closehash()
{
    int i;
    for (i=0;i<HASH_LENGTH;i++) {
	removelist(hashtable[i]);
	hashtable[i] = NULL; /* For unseen future i do this */
    }
}

int hash_function(char *identifier) 
{
    /*
     * Build more elaborate hashing scheme. If you want one.
     */
    return ( (int) *identifier );
}

void insert_keyword(char *ident,char attribute)
{
    KEYWORD *first;
    KEYWORD *new;
    int index;
    new = first = NULL;
    index = hash_function(ident);
    first = hashtable[index];
    if ((new = (KEYWORD *)malloc(sizeof(struct _keyword))) != NULL) {
	strcpy(new->kw,ident);
	new->length = strlen(new->kw);
	new->attribute = attribute;
	new->next = first;
	hashtable[index] = new;
#ifdef DEBUG 
    fprintf(stderr,"insert_keyword: new %li, new->kw %s, new->length %i, new->attribute %c, new->next %li\n", new,
    				    new->kw, new->length, new->attribute,new->next);
#endif
    }
}


void match_identifier()
{
    KEYWORD *hash_id;
    int index, match = 0;
    index = hash_function(identifier.kw);
    hash_id = hashtable[index];
    while (hash_id != NULL && ! match) {
	if (hash_id->length == identifier.length) { /* Possible match */
	    if (strcmp(hash_id->kw,identifier.kw) == 0) {
		match = 1;
		/* Match found. Lets print out the result. */
		printf("%i%c:%s",identifier.length,
				   hash_id->attribute,
				   identifier.kw);
	    }
	}
	hash_id = hash_id->next;
    }
    if (! match)
        printf("%s",identifier.kw);
}


char *extract_identifier(char *s)
{
    register char *kwp = identifier.kw;
    identifier.kw[0] = '\0';
    identifier.length = 0;
    while ((isalpha(*s) || *s == '_' || isdigit(*s) || *s == '#') && 
           identifier.length < MAX_KEYWORD_LENGTH) {
	identifier.length += 1;
	*kwp++ = *s++;
    }
    *kwp = '\0';
    return(s);
}


void read_keywords()
{
    char filename[1024];
    char identifier[MAX_KEYWORD_LENGTH+1];
    char line[MAX_LINELENGTH+1];
    char attribute[2];
    char *home;
    int  items;
    FILE *kwfile;
    home = getenv("HOME");
    sprintf(filename,"%s/%s",(home == NULL ? "" : home),keyword_file);
    if ((kwfile = fopen(filename,"r")) != NULL) {
	fgets(line,MAX_LINELENGTH,kwfile);
	items = sscanf(line,"%[#a-zA-Z0-9_]:%[IUBR]",identifier,attribute);
	while (! feof(kwfile) ) {
#ifdef DEBUG
	    fprintf(stderr,"read_keywords: Items %i, kw = %s, attr = %s\n",items,identifier,attribute);
#endif
	    if (items == 2) 
		insert_keyword(identifier,attribute[0]);
	    fgets(line,MAX_LINELENGTH,kwfile);
	    items = sscanf(line,"%[#a-zA-Z0-9_]:%[IUBR]",identifier,attribute);
	}
	fclose(kwfile);
    }
}

int has_endofcomment(char *s)
{
    char i=0;
    while (*s) {
	if (*s == '*' && *(s+1) == '/') {
	    return(i+2);
	}
	i += 1;
	s += 1;
    }
    return(0);
}


main()
{
    char line[MAX_LINELENGTH+1];
    char *s;
    int comment,c_length,literal;
    comment = 0;
    literal = 0;
    inithash();
    read_keywords();
    while (gets(line) != NULL) {
	s = line;
	while (*s) {
	    if (*s == '/' && *(s+1) == '*') {
		c_length = has_endofcomment(s);
		if (c_length == 0) { /* Comment continues to the next line */
		    c_length = strlen(s);
		    comment += 1;
		}
		printf("%iU:%.*s",c_length,c_length,s);
		s = s + c_length ;
	    } 
	    if (*s == '/' && *(s+1) == '/') { /* C++ comments */
	        c_length = strlen(s);
		printf("%iU:%.*s",c_length,c_length,s);
		break;
	    } 
	    if (comment) {
		if ((c_length = has_endofcomment(s)) > 0) {
		    printf("%iU:%.*s",c_length,c_length,s);
		    s = s + c_length ;
		    comment -= 1;
		    if (comment < 0) comment = 0;
		} else { /* Whole line belongs to comment */
		    c_length = strlen(s);
		    printf("%iU:%.*s",c_length,c_length,s);
		    s = s + c_length;
		}
	    } else if (*s) {
	        if (*s == '\\' && *(s+1) == '\"') {/* Skip literal single character */
		    putchar(*s++);
		    putchar(*s++);
		}
		if (*s == '\"') 
		    literal = literal == 0 ? 1 : 0;
		    
		if ( (isalpha(*s) || *s == '#') && ! literal) {
		    s = extract_identifier(s);
		    match_identifier();
		} else
		    putchar(*s++);
	    } 
	}
	putchar('\n');
    }
    closehash();
}


