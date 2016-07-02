/*
 *	glob.c
 *
 * Performs wildcard-expansion for UNIX, VMS and MS-DOS systems.
 * Written by T.E.Dickey for vile (april 1993).
 *
 * (MS-DOS code was originally taken from the winc app example of the
 * zortech compiler - pjr)
 *
 * To do:
 *	make the wildcard expansion know about escaped characters (e.g.,
 *	with backslash a la UNIX.
 *
 *	modify (ifdef-style) 'expand_leaf()' to allow ellipsis.
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/glob.c,v 1.47 1995/09/13 13:01:06 pgf Exp $
 *
 */

#include "estruct.h"	/* global structures and defines */
#include "edef.h"	/* defines 'slash' */
#if SYS_OS2
# define INCL_DOSFILEMGR
# define INCL_ERRORS
# include <os2.h>
#else
# include "dirstuff.h"	/* directory-scanning interface & definitions */
#endif

#define BAKTIK '`'	/* used in UNIX shell for pipe */
#define	isname(c)	(isalnum(c) || ((c) == '_'))
#define	isdelim(c)	((c) == '(' || ((c) == '{'))

#if SYS_MSDOS || SYS_WIN31 || SYS_OS2 || SYS_WINNT
# define UNIX_GLOBBING OPT_GLOB_ENVIRON
# if UNIX_GLOBBING
#  define DirEntryStr(p)		p->d_name
# else
#  ifdef __ZTC__
#   define DeclareFind(p)		struct FIND *p
#   define DirEntryStr(p)		p->name
#   define DirFindFirst(path,p)		(p = findfirst(path, 0))
#   define DirFindNext(p)		(p = findnext())
#  else
#   define DeclareFind(p)		struct find_t p
#   define DirEntryStr(p)		p.name
#   define DirFindFirst(path,p)		(!_dos_findfirst(path, 0, &p))
#   define DirFindNext(p)		(!_dos_findnext(&p))
#  endif
# endif
# define DirEntryLen(p)			strlen(DirEntryStr(p))
#endif	/* SYS_MSDOS */

/*
 * Make the default unix globbing code use 'echo' rather than our internal
 * globber if we do not configure the 'glob' string-mode.
 */
#if SYS_UNIX && defined(GVAL_GLOB) && !OPT_VMS_PATH
# define UNIX_GLOBBING 1
#endif

#ifndef UNIX_GLOBBING
# define UNIX_GLOBBING 0
#endif

/*
 * Verify that we don't have both boolean- and string-valued 'glob' modes.
 */
#if defined(GMDGLOB) && defined(GVAL_GLOB)
	huh??
#else
# ifdef GMDGLOB		/* boolean */
#  define globbing_active() global_g_val(GMDGLOB)
# endif
# ifdef GVAL_GLOB	/* string */
#  define globbing_active() !is_falsem(global_g_val_ptr(GVAL_GLOB))
# endif
# ifndef globbing_active
#  define globbing_active() TRUE
# endif
#endif

/*
 * Some things simply don't work on VMS: pipes and $variables
 */
#if OPT_VMS_PATH
#
# undef  UNIX_GLOBBING
# define UNIX_GLOBBING 0
#
# undef  OPT_GLOB_ENVIRON
# define OPT_GLOB_ENVIRON 0
#
# undef  OPT_GLOB_PIPE
# define OPT_GLOB_PIPE 0
#
#endif

/*--------------------------------------------------------------------------*/

static	char **	glob_expand P((char **));
static	int	string_has_wildcards P((char *));
static	int	record_a_match P((char *));
static	int	expand_pattern P((char *));
#if OPT_GLOB_ENVIRON && UNIX_GLOBBING
static	void	expand_environ P(( char * ));
#endif
#if OPT_CASELESS
static	int	cs_char P((int));
#endif

/* the expanded list is defined outside of the functions because if we
 * handle ellipsis, the generating function must be recursive.
 */
static	SIZE_T	myMax, myLen;	/* length and index of the expanded list */
static	char **	myVec;		/* the expanded list */

/*--------------------------------------------------------------------------*/
static int
string_has_wildcards (item)
char	*item;
{
#if OPT_VMS_PATH || SYS_UNIX || OPT_MSDOS_PATH
	while (*item != EOS) {
#if UNIX_GLOBBING
		if (iswild(*item))
			return TRUE;
#endif
		if (*item == GLOB_SINGLE || *item == GLOB_MULTI)
			return TRUE;
#if OPT_GLOB_ELLIPSIS || SYS_VMS
		if (!strncmp(item, GLOB_ELLIPSIS, sizeof(GLOB_ELLIPSIS)-1))
			return TRUE;
#endif
#if OPT_GLOB_RANGE && !OPT_VMS_PATH
		if (*item == GLOB_RANGE[0])
			return TRUE;
#endif
#if OPT_GLOB_ENVIRON && !OPT_VMS_PATH
		if (*item == '$' && (isname(item[1]) || isdelim(item[1])))
			return TRUE;
#endif
		item++;
	}
#endif
	return FALSE;
}

/*
 * Record a pattern-match in 'myVec[]', returning false if an error occurs
 */
static int
record_a_match(item)
char	*item;
{
	if (item != 0 && *item != EOS) {
		if ((item = strmalloc(item)) == 0)
			return no_memory("glob-match");

		if (myLen + 2 >= myMax) {
			myMax = myLen + 2;
			if (myVec == 0)
				myVec = typeallocn(char *, myMax);
			else
				myVec = typereallocn(char *, myVec, myMax);
		}
		if (myVec == 0)
			return no_memory("glob-pointers");

		myVec[myLen++] = item;
		myVec[myLen] = 0;
	}
	return TRUE;
}

#if UNIX_GLOBBING
static	char *	next_leaf P(( char * ));
static	char *	wild_leaf P(( char * ));
static	int	match_leaf P(( char *, char * ));
static	int	expand_leaf P(( char *, char * ));

/*
 * Point to the leaf following the given string (i.e., skip a slash), returns
 * null if none is found.
 */
static char *
next_leaf (path)
char	*path;
{
	if (path != 0) {
		while (*path != EOS) {
			if (is_slashc(*path))
				return path+1;
			path++;
		}
	}
	return 0;
}

/*
 * Point to the beginning (after slash, if present) of the first leaf in
 * the given pattern argument that contains a wildcard.
 */
static char *
wild_leaf (pattern)
char	*pattern;
{
	register int	j, k, ok;
	register char	c;

	/* skip leading slashes */
	for (j = 0; pattern[j] != EOS && is_slashc(pattern[j]); j++)
		;

	/* skip to the leaf with wildcards */
	while (pattern[j] != EOS) {
		int	skip = FALSE;
		for (k = j+1; (c = pattern[k]) != EOS; k++) {
			if (is_slashc(c)) {
				pattern[k] = EOS;
				ok = string_has_wildcards(pattern+j);
				pattern[k] = c;
				if (ok)
					return pattern+j;
				skip = TRUE;	/* skip this leaf */
				break;
			}
		}
		if (skip)
			j = k+1;	/* point past slash */
		else if (c == EOS)
			break;
		else
			j++;		/* leaf is empty */
	}
	return string_has_wildcards(pattern+j) ? pattern+j : 0;
}

#if OPT_CASELESS
static int
cs_char(ch)
int ch;
{
	return isupper(ch) ? tolower(ch) : ch;
}
#else
#define cs_char(ch) (ch)
#endif

/*
 * This is the heart of the wildcard matching.  We are given a directory
 * leaf and a pointer to the leaf that contains wildcards that we must
 * match against the leaf.
 */
static int
match_leaf(leaf, pattern)
char	*leaf;
char	*pattern;
{
	while (*leaf != EOS && *pattern != EOS) {
		if (*pattern == GLOB_SINGLE) {
			leaf++;
			pattern++;
		} else if (*pattern == GLOB_MULTI) {
			int	multi = FALSE;
			pattern++;
			while (*leaf != EOS) {
				if (match_leaf(leaf, pattern)) {
					multi = TRUE;
					break;
				}
				leaf++;
			}
			if (!multi && *leaf != EOS)
				return FALSE;
#if OPT_GLOB_RANGE
		} else if (*pattern == GLOB_RANGE[0]) {
			int	found	= FALSE;
			char *	first	= ++pattern;
			int	negate	= 0;

			if (*first == GLOB_NEGATE[0] || 
				(GLOB_NEGATE[1] && *first == GLOB_NEGATE[1])) {
				negate = 1;
				first = pattern++;
			}
			while (*pattern != EOS) {
				if (*pattern == GLOB_RANGE[1]) {
					pattern++;
					break;
				}
				if (*pattern == '-' && pattern != first) {
					int	lo = pattern[-1];
					int	hi = pattern[1];
					if (hi == GLOB_RANGE[1])
						hi = '~';
					if (((cs_char(lo) <= cs_char(*leaf)) && (cs_char(*leaf) <= cs_char(hi))) != negate)
						found = TRUE;
					if (pattern[1] != GLOB_RANGE[1])
						pattern++;
				} else if ((cs_char(*pattern++) == cs_char(*leaf)) != negate)
					found = TRUE;
			}
			if (!found)
				return FALSE;
			leaf++;
#endif
		} else if (cs_char(*pattern++) != cs_char(*leaf++))
			return FALSE;
	}
	return (*leaf == EOS && *pattern == EOS);
}

#if !SYS_OS2
/*
 * Recursive procedure that allows any leaf (or all!) leaves in a path to
 * have wildcards.  Except for an ellipsis, each wildcard is completed
 * within a single leaf.
 *
 * Returns false if we ran out of memory (a problem on ms-dos), and true
 * if everything went well (e.g., matches).
 */
static int
expand_leaf (path, pattern)
char	*path;		/* built-up pathname, top-level */
char	*pattern;
{
	DIR	*dp;
	DIRENT	*de;
	int	result	= TRUE;
	char	save	= 0; /* warning suppression */
	SIZE_T	len;
	char	*leaf;
	char	*wild	= wild_leaf(pattern);
	char	*next	= next_leaf(wild);
	register char	*s;

	/* Fill-in 'path[]' with the non-wild leaves that we skipped to get
	 * to 'wild'.
	 */
	if (wild == pattern) {	/* top-level, first leaf is wild */
		if (*path == EOS)
			(void)strcpy(path, ".");
		leaf = path + strlen(path) + 1;
	} else {
		len = (int)(wild - pattern) - 1;
		if (*(s = path) != EOS) {
			s += strlen(s);
			*s++ = SLASHC;
		}
		if (len != 0) {
			(void)strncpy(s, pattern, len);
			s[len] = EOS;
		}
		if (*path == EOS) {
			path[0] = SLASHC;
			path[1] = EOS;
			leaf = path + 1;
		}
#if OPT_MSDOS_PATH
		/* Force the strncpy from 'pattern' to pick up a slash just
		 * after the ':' in a drive specification.
		 */
		else if ((s = is_msdos_drive(path)) != 0 && s[0] == EOS) {
			s[0] = SLASHC;
			s[1] = EOS;
			leaf = s + 1;
		}
#endif
		else {
			leaf = path + strlen(path) + 1;
		}
	}

	if (next != 0) {
		save = next[-1];
		next[-1] = EOS;		/* restrict 'wild[]' to one leaf */
	}

	/* Scan the directory, looking for leaves that match the pattern.
	 */
	if ((dp = opendir(SL_TO_BSL(path))) != 0) {
		leaf[-1] = SLASHC;	/* connect the path to the leaf */
		while ((de = readdir(dp)) != 0) {
#if OPT_MSDOS_PATH
			(void)strcpy(leaf, de->d_name);
#if !OPT_CASELESS
			(void)mklower(leaf);
#endif
			if (strchr(pattern, '.') && !strchr(leaf, '.'))
				(void)strcat(leaf, ".");
#else
#if USE_D_NAMLEN
			len = de->d_namlen;
			(void)strncpy(leaf, de->d_name, len);
			leaf[len] = EOS;
#else
			(void)strcpy(leaf, de->d_name);
#endif
#endif
			if (!strcmp(leaf, ".")
			 || !strcmp(leaf, ".."))
				continue;
			if (!match_leaf(leaf, wild))
				continue;
			if (next != 0) {	/* there are more leaves */
				if (!string_has_wildcards(next)) {
					s = leaf + strlen(leaf);
					*s++ = SLASHC;
					(void)strcpy(s, next);
					if (ffexists(path)
					 && !record_a_match(path)) {
						result = FALSE;
						break;
					}
				} else if (is_directory(path)) {
#if SYS_MSDOS || SYS_WIN31
					s = strrchr(path, '.');
					if (s[1] == EOS)
						s[0] = EOS;
#endif
					if (!expand_leaf(path, next)) {
						result = FALSE;
						break;
					}
				}
			} else if (!record_a_match(path)) {
				result = FALSE;
				break;
			}
		}
		(void)closedir(dp);
	} else
		result = SORTOFTRUE;	/* at least we didn't run out of memory */

	if (next != 0)
		next[-1] = save;

	return result;
}

#else /* SYS_OS2 */

/*
 * Recursive procedure that allows any leaf (or all!) leaves in a path to
 * have wildcards.  Except for an ellipsis, each wildcard is completed
 * within a single leaf.
 *
 * Returns false if we ran out of memory (a problem on ms-dos), and true
 * if everything went well (e.g., matches).
 */
static int
expand_leaf (path, pattern)
char	*path;		/* built-up pathname, top-level */
char	*pattern;
{
	FILEFINDBUF3 fb;
	ULONG entries;
	HDIR hdir;
	APIRET rc;

	int	result	= TRUE;
	char	save = 0; /* warning suppression */
	SIZE_T	len;
	char	*leaf;
	char	*wild	= wild_leaf(pattern);
	char	*next	= next_leaf(wild);
	register char	*s;

	/* Fill-in 'path[]' with the non-wild leaves that we skipped to get
	 * to 'wild'.
	 */
	if (wild == pattern) {	/* top-level, first leaf is wild */
		if (*path == EOS)
			(void)strcpy(path, ".");
	} else {
		len = wild - pattern - 1;
		if (*(s = path) != EOS) {
			s += strlen(s);
			*s++ = SLASHC;
		}
		if (len != 0)
			strncpy(s, pattern, len)[len] = EOS;
	}
	leaf = path + strlen(path) + 1;

	if (next != 0) {
		save = next[-1];
		next[-1] = EOS;		/* restrict 'wild[]' to one leaf */
	}

	/* Scan the directory, looking for leaves that match the pattern.
	 */
	len = strlen(path);
	if (!is_slashc(path[len - 1]))
		(void)strcat(path, "/*.*");
	else
		(void)strcat(path, "*.*");

	hdir = HDIR_CREATE;
	entries = 1;
	rc = DosFindFirst(SL_TO_BSL(path), &hdir,
			FILE_DIRECTORY | FILE_READONLY,
			&fb, sizeof(fb), &entries, FIL_STANDARD);
	if (rc == NO_ERROR)
	{
		leaf[-1] = SLASHC;

		do
		{
			(void) mklower(strcpy(leaf, fb.achName));

			if (strcmp(leaf, ".") == 0 || strcmp(leaf, "..") == 0)
			 	continue;
			if (!match_leaf(leaf, wild))
				continue;

			if (next != 0) {	/* there are more leaves */
				if (!string_has_wildcards(next)) {
					s = leaf + strlen(leaf);
					*s++ = SLASHC;
					(void)strcpy(s, next);
					if (!record_a_match(path)) {
						result = FALSE;
						break;
					}
				} else if (is_directory(path)) {
					s = strrchr(path, '.');
					if (s[1] == EOS)
						s[0] = EOS;
					if (!expand_leaf(path, next)) {
						result = FALSE;
						break;
					}
				}
			} else if (!record_a_match(path)) {
				result = FALSE;
				break;
			}

		} while (entries = 1, 
		         DosFindNext(hdir, &fb, sizeof(fb), &entries) == NO_ERROR 
				 && entries == 1);

		DosFindClose(hdir);
	}
	else
	{
		result = SORTOFTRUE;	/* at least we didn't run out of memory */
	}

	if (next != 0)
		next[-1] = save;

	return result;
}
#endif /* SYS_OS2 */

#if ! ANSI_QSORT
static	int	compar P(( char **, char ** ));
static int
compar (a, b)
char	**a;
char	**b;
#else
static int compar (const void *a, const void *b)
#endif
{
#if OPT_CASELESS
	return stricmp(*(char **)a, *(char **)b);
#else
	return strcmp(*(char **)a, *(char **)b);
#endif
}
#endif

#if OPT_GLOB_PIPE
static	int	glob_from_pipe P(( char *));

static int
glob_from_pipe(pattern)
char	*pattern;
{
#ifdef GVAL_GLOB
	char	*cmd = global_g_val_ptr(GVAL_GLOB);
	int	single;
#else
	static	char	cmd[] = "!echo %s";
	static	int	single	= TRUE;
#endif
	FILE	*cf;
	char	tmp[NFILEN];
	int	result = FALSE;
	register SIZE_T len;
	register char *s, *d;

#ifdef GVAL_GLOB

	/*
	 * For now, assume that only 'echo' will supply the result all on one
	 * line.  Other programs (e.g., 'ls' and 'find' do the sensible thing
	 * and break up the output with newlines.
	 */
	if (!isShellOrPipe(cmd)) {
		cmd = "!echo %s";
		single = TRUE;
		d = cmd + 1;
	} else {
		char	save = EOS;
		for (d = cmd+1; *d != EOS && isspace(*d); d++)
			;
		for (s = d; *s != EOS; s++) {
			if (isspace(*s)) {
				save = *s;
				*s = EOS;
				break;
			}
		}
		single = !strcmp(pathleaf(d), "echo");
		if (save != EOS)
			*s = save;
	}
#else
	d = cmd+1;
#endif

	(void)lsprintf(tmp, d, pattern);
	if ((cf = npopen(tmp, "r")) != 0) {
		char	old[NFILEN+1];

		*(d = old) = EOS;

		while ((len = fread(tmp, sizeof(*tmp), sizeof(tmp), cf)) > 0) {
			/*
			 * Split the buffer up.  If 'single', split on all
			 * whitespace, otherwise only on newlines.
			 */
			for (s = tmp; s-tmp < len; s++) {
				if ((single && isspace(*s))
				 || (!single && (*s == '\n' || *s == EOS))) {
					*d = EOS;
					result = record_a_match(d = old);
					*d = EOS;
					if (!result)
						break;
				 } else {
				 	*d++ = *s;
				 }
			}
		}
		if (*old != EOS)
			result = record_a_match(old);
		npclose(cf);
	} else
		result = FALSE;

	return result;
}
#endif

#if OPT_GLOB_ENVIRON && UNIX_GLOBBING
/*
 * Expand environment variables in 'pattern[]'
 * It allows names of the form
 *
 *	$NAME
 *	$(NAME)
 *	${NAME}
 */
static void
expand_environ(pattern)
char	*pattern;
{
	register int	j, k;
	int	delim,
		left,
		right;
	char	*s;
	char	save[NFILEN];

	for (j = 0; pattern[j] != EOS; j++) {
		if (pattern[j] == '$') {

			k = j+1;
			if (pattern[k] == '(')		delim = ')';
			else if (pattern[k] == '{')	delim = '}';
			else				delim = EOS;

			if (delim != EOS)
				k++;
			left	=
			right	= k;

			while (pattern[k] != EOS) {
				right = k;
				if (delim != EOS) {
					if (pattern[k++] == delim)
						break;
				} else if (isname(pattern[k])) {
					k++;
				} else {
					break;
				}
			}
			if (delim == EOS)
				right = k;

			(void)strcpy(save, pattern+k);
			if (right != left) {
				pattern[right] = EOS;
				if ((s = getenv(pattern+left)) == 0)
					s = "";
			} else
				s = "";

			(void)strcpy(pattern+j, s);
			(void)strcat(pattern, save);
			j += strlen(s) - 1;
		}
	}
}
#else
#define	expand_environ(pattern)
#endif

/*
 * Notes:
 *	VMS's sys$search function (embedded in our fake 'readdir()') handles
 *	all of the VMS wildcards.
 *
 *	MS-DOS has non-UNIX functions that scan a directory and recognize DOS-style
 *	wildcards.  Use these to get the smallest executable.  However, DOS-
 *	style wildcards are crude and clumsy compared to UNIX, so we provide them as
 *	an option.  (For example, the DOS-wildcards won't match "..\*\*.bak").
 */
static int
expand_pattern (item)
char	*item;
{
	int	result = FALSE;
#if OPT_VMS_PATH
	DIR	*dp;
	DIRENT	*de;

	if ((dp = opendir(SL_TO_BSL(item))) != 0) {
		result = TRUE;
		while ((de = readdir(dp)) != 0) {
			char	temp[NFILEN];
			size_t	len = de->d_namlen;
			(void)strncpy(temp, de->d_name, len);
			temp[len] = EOS;
			if (!record_a_match(temp)) {
				result = FALSE;
				break;
			}
		}
		(void)closedir(dp);
	}

#else	/* UNIX or MSDOS, etc. */

#if OPT_GLOB_PIPE
# ifdef GVAL_GLOB
	/*
	 * The 'glob' mode value can be on/off or set to a pipe expression,
	 * e.g., "!echo %s".  This allows using the shell to expand the
	 * pattern, which is slower than vile's internal code, but may allow
	 * using specific features to which the user is accustomed.
	 *
	 * As a special case, we read from a pipe if the expression begins with
	 * a back-tick (e.g., `which script`).
	 */
	if (isShellOrPipe(global_g_val_ptr(GVAL_GLOB))
	 || *item == BAKTIK) {
		result = glob_from_pipe(item);
	} else
# else
	result = glob_from_pipe(item);
#  if UNIX_GLOBBING
	huh ??		/* thought I turned that off ... */
#  endif
# endif
#endif
#if UNIX_GLOBBING
	{
	char	builtup[NFILEN];
	char	pattern[NFILEN];
	SIZE_T	first	= myLen;

	(void)strcpy(pattern, item);
	*builtup = EOS;
#if OPT_MSDOS_PATH
#if !OPT_CASELESS
	(void)mklower(pattern);
#endif
#endif
	expand_environ(pattern);
	if (string_has_wildcards(pattern)) {
		if ((result = expand_leaf(builtup, pattern)) != FALSE
		 && (myLen-first > 1)) {
			qsort((char *)&myVec[first], myLen-first,
						sizeof(*myVec), compar);
		}
	} else
		result = record_a_match(pattern);
	}
#endif				/* UNIX-style globbing */
#if (SYS_MSDOS || SYS_WIN31 || SYS_OS2 || SYS_WINNT) && !UNIX_GLOBBING
	/* native DOS-wildcards */
	DeclareFind(p);
	char	temp[FILENAME_MAX + 1];
	char    path[FILENAME_MAX + 1];
	char *cp = pathleaf(
			strcpy(temp,
				strcpy(path, item)));

	if (DirFindFirst(path,p)) {
		result = TRUE;
		do {
			(void)strcpy(cp, DirEntryStr(p));
			if (!record_a_match(temp)) {
				result = FALSE;
				break;
			}
		} while (DirFindNext(p));
	}
#endif				/* native MS-DOS globbing */
#endif	/* OPT_VMS_PATH */
	return result;		/* true iff no errors noticed */
}

/*--------------------------------------------------------------------------*/

/*
 * Tests a list of items to see if at least one of them needs to be globbed.
 */
#if !SYS_UNIX
int
glob_needed (list_of_items)
char	**list_of_items;
{
	register int 	n;

	for (n = 0; list_of_items[n] != 0; n++)
		if (string_has_wildcards(list_of_items[n]))
			return TRUE;
	return FALSE;
}
#endif

/*
 * Expands the items in a list, returning an entirely new list (so it can be
 * freed without knowing where it came from originally).  This should only
 * return 0 if we run out of memory.
 */
static char **
glob_expand (list_of_items)
char	**list_of_items;
{
	int	len = glob_length(list_of_items);
	int	i;

	myMax = 0;
	myLen = 0;
	myVec = 0;

	for (i = 0; i < len; ++i) {
		char	*item = list_of_items[i];
		/*
		 * For UNIX, expand '~' expressions in case we've got a pattern
		 * like "~/test*.log".
		 */
#if SYS_UNIX
		char	temp[NFILEN];
		item = home_path(strcpy(temp, item));
#endif
		if (!isInternalName(item)
		 && globbing_active()
		 && string_has_wildcards(item)) {
			if (!expand_pattern(item))
				return 0;
		} else if (!record_a_match(item)) {
			return 0;
		}
	}
	return myVec;
}

/*
 * A special case of 'glob_expand()', expands a single string into a list.
 */
char **
glob_string (item)
char	*item;
{
	char	*vec[2];

	vec[0] = item;
	vec[1] = 0;

	return glob_expand(vec);
}

/*
 * Counts the number of items in a list of strings.  This is simpler (and
 * more useful) than returning the length and the list as arguments from
 * a procedure.  Note that since the standard argc/argv convention puts a
 * null pointer on the end, this function is applicable to the 'argv[]'
 * parameter of the main program as well.
 */
int
glob_length (list_of_items)
char	**list_of_items;
{
	register int	len;
	if (list_of_items != 0) {
		for (len = 0; list_of_items[len] != 0; len++)
			;
	} else
		len = 0;
	return len;
}

/*
 * Frees the strings in a list, and the list itself.  Note that this should
 * not be used for the main program's original argv, because on some systems
 * it is a part of a larger data area, as are the command strings.
 */
char **
glob_free (list_of_items)
char	**list_of_items;
{
	register int	len;
	if (list_of_items != 0) {
		for (len = 0; list_of_items[len] != 0; len++)
			free(list_of_items[len]);
		free ((char *)list_of_items);
	}
	return 0;
}


#if !SYS_UNIX
/*
 * Expand wildcards for the main program a la UNIX shell.
 */
void
expand_wild_args(argcp, argvp)
int *argcp;
char ***argvp;
{
#if SYS_VMS
	int	j, k;
	int	comma	= 0;
	int	option	= 0;
	/*
	 * VMS command-line arguments may be a comma-separated list of
	 * filenames, with an optional "/read_only" flag appended.
	 */
	for (j = 1; j < *argcp; j++) {
		char	*s = (*argvp)[j];
		int	c;
		while ((c = *s++) != EOS) {
			if (c == ',')
				comma++;
			if (c == '/')
				option++;
		}
	}
	if (comma || option) {
		char	**newvec = typeallocn(char *, comma + *argcp + 1);
		for (j = k = 0; j < *argcp; j++) {
			char	*the_arg = strmalloc((*argvp)[j]);
			char	*item;

			/* strip off VMS options */
			while ((item = strrchr(the_arg, '/')) != 0) {
				mkupper(item);
				if (!strncmp(item, "/READ_ONLY", strlen(item))) {
					set_global_b_val(MDVIEW,TRUE);
				}
				*item = EOS;
			}

			while (*the_arg != EOS) {
				item = strchr(the_arg, ',');
				if (item == 0)
					item = the_arg + strlen(the_arg);
				else
					*item++ = EOS;
				newvec[k++] = the_arg;
				the_arg = item;
			}
		}
		newvec[k] = 0;
		*argcp = k;
		*argvp = newvec;
	}
#endif
	if (glob_needed(*argvp)) {
		char	**newargs = glob_expand(*argvp);
		if (newargs != 0) {
			*argvp = newargs;
			*argcp = glob_length(newargs);
		}
	}
}
#endif

/*
 * Expand a string, permitting only one match.
 */
int
doglob (path)
char	*path;
{
	char	**expand = glob_string(path);
	int	len = glob_length(expand);

	if (len > 1) {
		if (mlyesno("Too many filenames.  Use first") != TRUE) {
			(void)glob_free(expand);
			return FALSE;
		}
	}
	if (len > 0) {
		(void)strcpy(path, expand[0]);
		(void)glob_free(expand);
	}
	return TRUE;
}
