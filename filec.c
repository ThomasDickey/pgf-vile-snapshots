/*
 *	filec.c
 *
 * Filename prompting and completion routines
 * Written by T.E.Dickey for vile (march 1993).
 *
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/filec.c,v 1.64 1996/04/17 02:49:32 pgf Exp $
 *
 */

#include "estruct.h"
#include "edef.h"

#undef USE_QSORT
#if SYS_OS2
# define USE_QSORT 0
# define INCL_DOSFILEMGR
# define INCL_ERRORS
# include <os2.h>
# define FoundDirectory(path) ((fb.attrFile & FILE_DIRECTORY) != 0)
#endif

#ifndef USE_QSORT
#define USE_QSORT 1
#endif

#if MISSING_EXTERN_ENVIRON
extern char **environ;
#endif

#define	SLASH (EOS+1) /* less than everything but EOS */

#if OPT_VMS_PATH
#define	KBD_OPTIONS	KBD_NORMAL|KBD_UPPERC
#endif

#ifndef	KBD_OPTIONS
#define	KBD_OPTIONS	KBD_NORMAL
#endif

#ifndef FoundDirectory
# define FoundDirectory(path) is_directory(path)
#endif

static	char	**MyGlob;	/* expanded list */
static	int	in_glob;	/* index into MyGlob[] */
static	int	only_dir;	/* can only match real directories */

static void
free_expansion (void)
{
	MyGlob = glob_free(MyGlob);
	in_glob = -1;
}

#if COMPLETE_DIRS || COMPLETE_FILES

#include "dirstuff.h"

#if OPT_VMS_PATH
static	void	vms2hybrid(char *path);
#endif

/*--------------------------------------------------------------------------*/

/*
 * Test if the path has a trailing slash-delimiter (i.e., can be syntactically
 * distinguished from non-directory paths).
 */
static int
trailing_slash(const char * path)
{
	register int	len = strlen(path);
	if (len > 0) {
#if OPT_VMS_PATH
		if (is_vms_pathname(path, TRUE))
			return TRUE;
#endif
		return (is_slashc(path[len-1]));
	}
	return FALSE;
}

/*
 * Force a trailing slash on the end of the path, returns the length of the
 * resulting path.
 */
static SIZE_T
force_slash(char * path)
{
	register SIZE_T	len = strlen(path);

#if OPT_VMS_PATH
	if (!is_vms_pathname(path, -TRUE))	/* must be unix-style name */
#endif
	if (!trailing_slash(path)) {
		path[len++] = SLASHC;
		path[len] = EOS;
	}
	return len;
}

/*
 * Compare two paths lexically.
 */
static int
pathcmp(LINE * lp, char * text)
{
    register char *l, *t;
    register int lc, tc;

    if (llength(lp) <= 0)	/* (This happens on the first insertion) */
	return -1;

    l = lp->l_text;
    t = text;
    for_ever {
	lc = *l++;
	tc = *t++;
#if OPT_CASELESS
	if (isupper(lc))
	    lc = tolower(lc);
	if (isupper(tc))
	    tc = tolower(tc);
#endif
	if (lc == tc) {
	    if (tc == EOS)
		return 0;
	} else {
	    if (is_slashc(lc)) {
		lc = (*l != EOS) ? SLASH : EOS;
	    }
	    if (is_slashc(tc)) {
		tc = (*t != EOS) ? SLASH : EOS;
	    }
	    return lc - tc;
	}
    }
}

/*
 * Insert a pathname at the given line-pointer.
 * Allocate up to three extra bytes for possible trailing slash, EOS, and
 * directory scan indicator.  The latter is only used when there is a trailing
 * slash.
 */
static LINE *
makeString(BUFFER *bp, LINE * lp, char * text, SIZE_T len)
{
	register LINE	*np;
	int extra = (len != 0 && is_slashc(text[len-1])) ? 2 : 3;

	if ((np = lalloc((int)len+extra, bp)) == NULL) {
		lp = 0;
	} else {
		(void)strcpy(np->l_text, text);
		np->l_text[len+extra-1] = 0; 	/* clear scan indicator */
		llength(np) -= extra;	/* hide the null and scan indicator */
		

		set_lforw(lback(lp), np);
		set_lback(np, lback(lp));
		set_lback(lp, np);
		set_lforw(np, lp);
		lp = np;
	}
	return lp;
}

/*
 * Create a buffer to store null-terminated strings.
 *
 * The file (or directory) completion buffer is initialized at the beginning of
 * each command.  Wildcard expansion causes entries to be read for a given path
 * on demand.  Resetting the buffer in this fashion is a tradeoff between
 * efficiency (allows reuse of a pattern in NAMEC/TESTC operations) and speed
 * (directory scanning is slow).
 *
 * The tags buffer is initialized only once for a given tags-file.
 */
static BUFFER *
bs_init(const char * name)
{
	register BUFFER *bp;

	if ((bp = bfind(name, BFINVS)) != 0) {
		b_clr_flags(bp, BFSCRTCH);	/* make it nonvolatile */
		(void)bclear(bp);
		bp->b_active = TRUE;
	}
	return bp;
}

/*
 * Look for or insert a pathname string into the given buffer.  Start looking
 * at the given line if non-null.  The pathname is expected to be in
 * canonical form.
 */
static int
bs_find(
char *	fname,	/* pathname to find */
SIZE_T	len,	/* ...its length */
BUFFER *bp,	/* buffer to search */
LINEPTR *lpp)	/* in/out line pointer, for iteration */
{
	register LINE	*lp;
	int	doit	= FALSE;
#if OPT_VMS_PATH
	char	temp[NFILEN];
	if (!is_slashc(*fname))
		vms2hybrid(fname = strcpy(temp, fname));
#endif

	if (lpp == NULL || (lp = *lpp) == NULL)
		lp = buf_head(bp);
	lp = lforw(lp);

	for_ever {
		register int r = pathcmp(lp, fname);

		if (r == 0) {
			if (trailing_slash(fname)
			 && !trailing_slash(lp->l_text)) {
				/* reinsert so it is sorted properly! */
				lremove(bp, lp);
				return bs_find(fname, len,  bp, lpp);
			}
			break;
		} else if (r > 0) {
			doit = TRUE;
			break;
		}

		lp = lforw(lp);
		if (lp == buf_head(bp)) {
			doit = TRUE;
			break;
		}
	}

	if (doit) {
		lp = makeString(bp, lp, fname, len);
		b_clr_counted(bp);
	}

	if (lpp)
		*lpp = lp;
	return TRUE;
}
#endif /* COMPLETE_DIRS || COMPLETE_FILES */

#if COMPLETE_DIRS || COMPLETE_FILES

static	BUFFER	*MyBuff;	/* the buffer containing pathnames */
static	const char *MyName;	/* name of buffer for name-completion */
static	char	**MyList;	/* list, for name-completion code */
static	ALLOC_T MySize;		/* length of list, for (re)allocation */

/*
 * Returns true if the string looks like an environment variable (i.e.,
 * a '$' followed by an optional name.
 */
static int
is_environ(const char *s)
{
	if (*s++ == '$') {
		while (*s != EOS) {
			if (!isalnum(*s) && (*s != '_'))
				return FALSE;
			s++;
		}
		return TRUE;
	}
	return FALSE;
}

/*
 * Tests if the given path has been scanned during this prompt/reply operation
 *
 * If there is anything else in the list that we can do completion with, return
 * true.  This allows the case in which we scan a directory (for directory
 * completion and then look at the subdirectories.  It should not permit
 * directories which have already been scanned to be rescanned.
 */
static int
already_scanned(char * path)
{
	register LINE	*lp;
	register SIZE_T	len;
	char	fname[NFILEN];
	LINEPTR slp;

	len = force_slash(strcpy(fname, path));

	for_each_line(lp,MyBuff)
#if OPT_CASELESS
		if (stricmp(fname, lp->l_text) == 0)
#else
		if (strcmp(fname, lp->l_text) == 0)
#endif
		{
		    if (lp->l_text[llength(lp)+1])
			return TRUE;
		    else
			break; 	/* name should not occur more than once */
		}

	/* force the name in with a trailing slash */
	slp = buf_head(MyBuff);
	(void)bs_find(fname, len, MyBuff, &slp);

	/*
	 * mark name as scanned (since that is what we're about to do after
	 * returning)
	 */
	lp = slp;
	lp->l_text[llength(lp)+1] = 1;
	return FALSE;
}

#if OPT_VMS_PATH
/*
 * Convert a canonical VMS pathname to a hybrid form, in which the leaf (e.g.,
 * "name.type;ver") is left untouched, but the directory portion is in UNIX
 * form.  This alleviates a sticky problem with VMS's pathname delimiters by
 * making them all '/' characters.
 */
static void
vms2hybrid(char *path)
{
	char	leaf[NFILEN];
	char	head[NFILEN];
	char	*s = strcpy(head, path);
	char	*t;

	TRACE(("vms2hybrid '%s'\n", path))
	(void)strcpy(leaf, s = pathleaf(head));
	if ((t = is_vms_dirtype(leaf)) != 0)
		(void)strcpy(t, "/");
	*s = EOS;
	if (s == path)	/* a non-canonical name got here somehow */
		(void) strcpy(head, current_directory(FALSE));
	pathcat(path, mkupper(vms2unix_path(head, head)), leaf);
	TRACE((" -> '%s' (vms2hybrid)\n", path))
}

static void
hybrid2vms(char *path)
{
	char	leaf[NFILEN];
	char	head[NFILEN];
	char	*s = strcpy(head, path);

	TRACE(("hybrid2vms '%s'\n", path))
	(void)strcpy(leaf, s = pathleaf(head));
	*s = EOS;
	if (s == path)	/* a non-canonical name got here somehow */
		(void) vms2unix_path(head, current_directory(FALSE));
	pathcat(path, unix2vms_path(head, head), leaf);
	TRACE((" -> '%s' hybrid2vms\n", path))
}

static void
hybrid2unix(char *path)
{
	char	leaf[NFILEN];
	char	head[NFILEN];
	char	*s = strcpy(head, path);

	TRACE(("hybrid2unix '%s'\n", path))
	(void)strcpy(leaf, s = pathleaf(head));
	*s = EOS;
	if (s == path)	/* a non-canonical name got here somehow */
		(void) vms2unix_path(head, current_directory(FALSE));
	pathcat(path, head, leaf);

	/* The semicolon that indicates the version is a little tricky.  When
	 * simulating via fakevms.c on UNIX, we've got to trim the version
	 * marker at this point.  Otherwise, it'll be changed to a dollar sign
	 * the next time the string is converted from UNIX to VMS form.  A
	 * side-effect is that the name-completion echos (for UNIX only!) the
	 * version, but doesn't store it in the buffer returned to the caller.
	 */
	if ((s = strrchr(path, ';')) != 0) {
#if SYS_UNIX
		*s = EOS;	/* ...we're only simulating */
#else
		*s = '.';	/* this'll be interpreted as version-mark */
#endif
	}
	TRACE((" -> '%s' hybrid2unix\n", path))
}
#endif /* OPT_VMS_PATH */

#if USE_QSORT
/*
 * Compare two lines (containing paths) for quicksort by calling pathcmp().
 * If the paths compare equal force the one with the trailing slash to be
 * less than.  This'll make deleting the ones without slashes easier in
 * sortMyBuff().
 */
static int
qs_pathcmp(const void *lpp1, const void *lpp2)
{
    int r = pathcmp(*(LINE *const*)lpp1, (* (LINE *const*) lpp2)->l_text);

    if (r == 0) {
	const LINE *lp1 = *(LINE *const*)lpp1;

	if (llength(lp1) > 0 && is_slashc(lgetc(lp1, llength(lp1)-1)))
	    return -1;
	else		/* Don't care if the other one has slash or not... */
	    return 1;	/* ...returning 1 for two equal elements won't do  */
	    		/* any harm. */
    }
    else
	return r;
}

static void
sortMyBuff(void)
{
    L_NUM n;
    LINE **sortvec;
    register LINE *lp, *plp;
    register LINE **slp;

    b_clr_counted(MyBuff);
    n = line_count(MyBuff);
    if (n <= 0)
	return;			/* Nothing to sort */

    sortvec = typecallocn(LINE *, (ALLOC_T) n);
    if (sortvec == NULL)
	return;			/* Can't sort it .. have to get by unsorted */

    slp = sortvec;
    for_each_line(lp, MyBuff) {
	*slp++ = lp;
    }
    qsort((char *) sortvec, (SIZE_T)n, sizeof(LINE *), qs_pathcmp);

    plp = buf_head(MyBuff);
    slp = sortvec;
    while (n-- > 0) {
	lp = *slp++;
	if (pathcmp(plp, lp->l_text) == 0) {
	    lfree(lp, MyBuff);
	}
	else {
	    set_lforw(plp, lp);
	    set_lback(lp, plp);
	    plp = lp;
	}
    }
    lp = buf_head(MyBuff);
    set_lforw(plp, lp);
    set_lback(lp, plp);
    b_clr_counted(MyBuff);

    free((char *)sortvec);
}
#endif	/* USE_QSORT */

/*
 * If the given path is not in the completion-buffer, expand it, and add the
 * expanded paths to the buffer.  Because the user may be trying to get an
 * intermediate directory-name, we must 'stat()' each name, so that we can
 * provide the trailing slash in the completion.  This is slow.
 */
static void
fillMyBuff(char * name)
{
	register char	*s;
#if SYS_OS2
	FILEFINDBUF3 fb;
	ULONG entries;
	HDIR hdir;
	APIRET rc;
	int case_preserving = is_case_preserving(name);

	char	path[NFILEN + 2];
#else	/* UNIX, VMS or MSDOS */
	char	*leaf;

	DIR	*dp;
	DIRENT	*de;

	char	path[NFILEN];
#if OPT_VMS_PATH
	char	temp[NFILEN];
#endif
#endif

	TRACE(("fillMyBuff '%s'\n", name))

	/**********************************************************************/
	if (is_environ(name)) {
		LINEPTR lp;
		int n;

		/*
		 * The presence of any environment variable in the list is 
		 * sufficient indication that we've copied the environment.
		 */
		for_each_line(lp,MyBuff)
			if (is_environ(lp->l_text))
				return;

		/*
		 * Copy all of the environment-variable names, prefixed with
		 * the '$' that indicates what they are.
		 */
		for (n = 0; environ[n] != 0; n++) {
			char *d = path;

			s = environ[n];
			*d++ = name[0];
			while (((*d = *s++) != '=') && (*d != EOS)) {
				if ((d++ - path) > sizeof(path)-2)
					break;
			}
			*d = EOS;
#if SYS_MSDOS || SYS_OS2
			mklower(path);	/* glob.c will uppercase for getenv */
#endif
#if USE_QSORT
			(void)makeString(MyBuff, 
			                 buf_head(MyBuff),
					 path,
					 (SIZE_T) strlen(path));
#else	/* !USE_QSORT */
			(void)bs_find(path, (SIZE_T)strlen(path),
					MyBuff, (LINEPTR*)0);
#endif	/* USE_QSORT/!USE_QSORT */
			TRACE(("> '%s'\n", path))
		}
#if USE_QSORT
		sortMyBuff();
#endif
		return;
	}

	(void)strcpy(path, name);
#if OPT_MSDOS_PATH
	bsl_to_sl_inplace(path);
#endif
#if OPT_VMS_PATH
	(void)strcpy(temp, name);
	hybrid2vms(path);		/* convert to canonical VMS name */
#endif
	if (!is_environ(path) && !is_directory(path)) {
		*pathleaf(path) = EOS;
		if (!is_directory(path))
			return;
#if OPT_VMS_PATH
		*pathleaf(temp) = EOS;
#endif
	}

#if OPT_VMS_PATH
	if (already_scanned(temp))	/* we match the hybrid name */
		return;
#else
	if (already_scanned(path))
		return;
#endif

	/**********************************************************************/

#if SYS_OS2
	s = path + force_slash(path);
	(void)strcat(path, "*.*");

	hdir = HDIR_CREATE;
	entries = 1;
	rc = DosFindFirst(SL_TO_BSL(path), &hdir,
			FILE_DIRECTORY | FILE_READONLY,
			&fb, sizeof(fb), &entries, FIL_STANDARD);
	if (rc == NO_ERROR)
	{
		do
		{
			(void) strcpy(s, fb.achName);
			if (!case_preserving)
				(void) mklower(s);

			if (strcmp(s, ".") == 0 || strcmp(s, "..") == 0)
			 	continue;

			if (only_dir) {
				if (!FoundDirectory(path))
					continue;
				(void) force_slash(path);
			}
#if COMPLETE_DIRS
			else {
				if (global_g_val(GMDDIRC) && FoundDirectory(path))
					(void) force_slash(path);
			}
#endif
			TRACE(("> '%s'\n", path))
			(void)bs_find(path, strlen(path), MyBuff, (LINEPTR*)0);

		} while (entries = 1, 
		         DosFindNext(hdir, &fb, sizeof(fb), &entries) == NO_ERROR 
				 && entries == 1);

		DosFindClose(hdir);
	}
#else /* UNIX, VMS or MSDOS */
	/* For MS-DOS pathnames, force the use of '\' instead of '/' in the
	 * open-directory operation to allow for runtime libraries that
	 * don't allow using UNIX-style '/' pathnames.
	 */
	if ((dp = opendir(SL_TO_BSL(path))) != 0) {
		s = path;
#if !OPT_VMS_PATH
		s += force_slash(path);
#endif

		leaf = s;
		while ((de = readdir(dp)) != 0) {
#if SYS_UNIX || SYS_VMS || SYS_WINNT
# if USE_D_NAMLEN
			(void)strncpy(leaf, de->d_name, (SIZE_T)(de->d_namlen));
			leaf[de->d_namlen] = EOS;
# else
			(void)strcpy(leaf, de->d_name);
# endif
#else
# if SYS_MSDOS
			(void)mklower(strcpy(leaf, de->d_name));
# else
			huh??
# endif
#endif
#if OPT_VMS_PATH
			vms_dir2path(path);
#else
# if SYS_UNIX || SYS_MSDOS || SYS_OS2 || SYS_WINNT
			if (!strcmp(leaf, ".")
			 || !strcmp(leaf, ".."))
			 	continue;
# endif
#endif
			if (only_dir) {
				if (!FoundDirectory(path))
					continue;
				(void) force_slash(path);
			}
#if COMPLETE_DIRS
			else {
				if (global_g_val(GMDDIRC) && FoundDirectory(path))
					(void) force_slash(path);
			}
#endif
			TRACE(("> '%s'\n", path))
#if USE_QSORT
#if OPT_VMS_PATH
			vms2hybrid(s = strcpy(temp, path));
#else
			s = path;
#endif
			(void)makeString(MyBuff, 
			                 buf_head(MyBuff),
					 s,
					 (SIZE_T) strlen(s));
#else	/* !USE_QSORT */
			(void)bs_find(path, (SIZE_T)strlen(path), MyBuff,
					(LINEPTR*)0);
#endif	/* USE_QSORT/!USE_QSORT */
		}
		(void)closedir(dp);
#if USE_QSORT
		sortMyBuff();
#endif
	}
#endif	/* SYS_OS2/!SYS_OS2 */
}

/*
 * Make the list of names needed for name-completion
 */
static void
makeMyList(char *name)
{
	register int	need, n;
	register LINE *	lp;
	char *slashocc;
	int len = strlen(name);

	if (is_slashc(name[len-1]))
	    len++;

	(void)bsizes(MyBuff);
	need = MyBuff->b_linecount + 2;
	if (MySize < need) {
		MySize = need * 2;
		if (MyList == 0)
			MyList = typeallocn(char *, MySize);
		else
			MyList = typereallocn(char *, MyList, MySize);
	}

	n = 0;
	for_each_line(lp,MyBuff)
		/* exclude listings of subdirectories below
		   current directory */
		if (llength(lp) >= len 
		 && ((slashocc = strchr(lp->l_text+len, SLASHC)) == NULL
		   || slashocc[1] == EOS))
			MyList[n++] = lp->l_text;
	MyList[n] = 0;
}

#if NO_LEAKS
static void
freeMyList(void)
{
	FreeAndNull(MyList);
	MySize = 0;
}
#else
#define	freeMyList()
#endif

static void
force_output(int c, char *buf, int *pos)
{
	kbd_putc(c);
	TTflush();
	buf[*pos] = (char)c;
	*pos += 1;
	buf[*pos] = EOS;
}

/*
 * Initialize the file-completion module.  We'll only do either file- or
 * directory-completion during any given command, and they use different
 * buffers (and slightly different parsing).
 */
void
init_filec(const char *buffer_name)
{
	MyBuff = 0;
	MyName = buffer_name;
}

/*
 * Perform the name-completion/display.  Note that we must convert a copy of
 * the pathname to absolute form so that we can match against the strings that
 * are stored in the completion table.  However, the characters that might be
 * added are always applicable to the original buffer.
 *
 * We only do name-completion if asked; if we did it when the user typed a
 * return it would be too slow.
 */
int
path_completion(int c, char *buf, int *pos)
{
	int	code	= FALSE,
		action	= (c == NAMEC || c == TESTC),
		ignore	= (*buf != EOS && isInternalName(buf));

	TRACE(("path_completion('%c' %d:\"%.*s\"\n", c, *pos, *pos,buf))
#if OPT_VMS_PATH
	if (ignore && action) {		/* resolve scratch-name conflict */
		if (is_vms_pathname(buf, -TRUE))
			ignore = FALSE;
	}
#endif
	if (ignore) {
		if (action) {		/* completion-chars have no meaning */
			force_output(c, buf, pos);
		}
	} else if (action) {
		char	*s;
		char	path[NFILEN];
		int	oldlen,
			newlen;

		/* initialize only on demand */
		if (MyBuff == 0) {
			if (MyName == 0
			 || (MyBuff = bs_init(MyName)) == 0)
			 	return FALSE;
		}

		/*
		 * Copy 'buf' into 'path', making it canonical-form.
		 */
#if OPT_VMS_PATH
		if (*strcpy(path, buf) == EOS) {
			(void)strcpy(path, current_directory(FALSE));
		} else if (!is_environ(path)) {
			char	frac[NFILEN];

			if (is_vms_pathname(path, -TRUE)) {
				s = vms_pathleaf(path);
				(void)strcpy(frac, s);
				*s = EOS;
			} else {
				s = pathleaf(path);
				if (is_vms_pathname(s, -TRUE)) {
					(void)strcpy(frac, s);
					*s = EOS;
				} else {	/* e.g., path=".." */
					*frac = EOS;
				}
			}
			if (*path == EOS)
				(void)strcpy(path, current_directory(FALSE));
			else
				(void)lengthen_path(path);
			(void)strcat(path, frac);
		}
		if (is_vms_pathname(path, -TRUE)) {
			vms2hybrid(path);
			/*
			 * FIXME: This compensates for the hack in canonpath
			 */
			if (!strcmp(buf, "/")) {
				while (*pos < strlen(path))
					force_output(path[*pos], buf, pos);
			}
		}
#else
		if (is_environ(buf)) {
			(void)strcpy(path, buf);
		} else {
# if SYS_UNIX || OPT_MSDOS_PATH
			char	**expand;

			/* trim trailing "." if it is a "/." */
			if ((s = last_slash(buf)) != 0
			 && !strcmp(s+1, "."))
				kbd_kill_response(buf, pos, '\b');

			/*
			 * Expand _unique_ wildcards and environment variables.
			 * Like 'doglob()', but without the prompt.
			 */
			expand = glob_string(strcpy(path, buf));
			switch (glob_length(expand)) {
			default:
				(void)glob_free(expand);
				kbd_alarm();
				return FALSE;
			case 1:
				(void)strcpy(path, expand[0]);
			case 0:
				(void)glob_free(expand);
				break;
			}
			(void)lengthen_path(path);
#  if OPT_MSDOS_PATH
			/*
			 * Pick up slash (special case) when we've just expanded a
			 * device such as "c:" to "c:/".
			 */
			if ((newlen = strlen(path)) == 3
			 && (oldlen = strlen(buf)) == 2
			 && is_slashc(path[newlen-1])
			 && path[newlen-2] == ':') {
				force_output(SLASHC, buf, pos);
			}
#  endif
# endif
		}
#endif

		if ((s = is_appendname(buf)) == 0)
			s = buf;
		if ((*s == EOS) || trailing_slash(s))
			(void)force_slash(path);

		if ((s = is_appendname(path)) != NULL) {
			register char *d;
			for (d = path; (*d++ = *s++) != EOS; )
				;
		}
#if OPT_MSDOS_PATH
		/* if the user typed a back-slash, we need to
		 * convert it, since it's stored as '/' in the file
		 * completion buffer to avoid conflicts with the use of
		 * backslash for escaping special characters.
		 */
		bsl_to_sl_inplace(path);
#endif

		newlen =
		oldlen = strlen(path);

		fillMyBuff(path);
		makeMyList(path);

		/* patch: should also force-dot to the matched line, as in history.c */
		/* patch: how can I force buffer-update to show? */

#if OPT_CASELESS
		code = kbd_complete(TRUE, c, path, &newlen, (char *)&MyList[0], sizeof(MyList[0]));
#if 0 /* case insensitive reply correction doesn't work reliably yet */
		(void)strcpy(buf, path);
#else
		(void)strcat(buf, path+oldlen);
#endif
#else
		code = kbd_complete(FALSE, c, path, &newlen, (char *)&MyList[0], sizeof(MyList[0]));
		(void)strcat(buf, path+oldlen);
#endif
#if OPT_VMS_PATH
		if (*buf != EOS
		 && !is_vms_pathname(buf, -TRUE))
			hybrid2unix(buf);
#endif
		*pos = strlen(buf);

		/* avoid accidentally picking up directory names for files */
		if ((code == TRUE)
		 && !only_dir
		 && !trailing_slash(path)
		 && is_directory(path)) {
			force_output(SLASHC, buf, pos);
			code = FALSE;
		}
	}
	TRACE((" -> '%s' path_completion\n", buf))
	return code;
}
#else	/* no filename-completion */
#define	freeMyList()
#endif	/* filename-completion */

/******************************************************************************/
int
mlreply_file(
const char * prompt,
TBUFF **buffer,
int	flag,		/* +1 to read, -1 to write, 0 don't care */
char *	result)
{
	register int	s;
	static	TBUFF	*last;
	char	Reply[NFILEN];
	int	(*complete) (DONE_ARGS) = no_completion;
	int	had_fname = (curbp != 0
			  && curbp->b_fname != 0
			  && curbp->b_fname[0] != EOS);
	int	do_prompt = (clexec || isnamedcmd || (flag & FILEC_PROMPT));
	int	ok_expand = (flag & FILEC_EXPAND);

	flag &= ~ (FILEC_PROMPT | FILEC_EXPAND);

#if COMPLETE_FILES
	if (do_prompt && !clexec) {
		complete = shell_complete;
		init_filec(FILECOMPLETION_BufName);
	}
#endif

	/* use the current filename if none given */
	if (buffer == 0) {
		(void)tb_scopy(
			buffer = &last,
			had_fname && is_pathname(curbp->b_fname)
				? shorten_path(strcpy(Reply, curbp->b_fname),
				FALSE)
				: "");
	}

	if (do_prompt) {
		char	*t1 = tb_values(*buffer),
			*t2 = is_appendname(t1);

		if (t1 != 0)
			(void)strcpy(Reply, (t2 != 0) ? t2 : t1);
		else
			*Reply = EOS;

	        s = kbd_string(prompt, Reply, sizeof(Reply),
			'\n', KBD_OPTIONS|KBD_MAYBEC, complete);
		freeMyList();

		if (s == ABORT)
			return s;
		if (s != TRUE) {
			if ((flag == FILEC_REREAD)
			 && had_fname
			 && (!global_g_val(GMDWARNREREAD)
			  || ((s = mlyesno("Reread current buffer")) == TRUE)))
				(void)strcpy(Reply, curbp->b_fname);
			else
	                	return s;
		} else if (kbd_is_pushed_back() && isShellOrPipe(Reply)) {
			/*
			 * The first call on 'kbd_string()' split the text off
			 * the shell command.  This is needed for the logic of
			 * colon-commands, but is inappropriate for filename
			 * prompting.  Read the rest of the text into Reply.
			 */
		        s = kbd_string(prompt, Reply+1, sizeof(Reply)-1,
				'\n', KBD_OPTIONS|KBD_MAYBEC, complete);
		}
        } else if (!screen_to_bname(Reply)) {
		return FALSE;
        }
	if (flag >= FILEC_UNKNOWN && is_appendname(Reply) != 0) {
		mlforce("[file is not a legal input]");
		return FALSE;
	}

	free_expansion();
	if (ok_expand) {
		if ((MyGlob = glob_string(Reply)) == 0
		 || (s = glob_length(MyGlob)) == 0) {
			mlforce("[No files found] %s", Reply);
			return FALSE;
		}
		if (s > 1) {
			char	tmp[80];
			(void)lsprintf(tmp, "Will create %d buffers. Okay", s);
			s = mlyesno(tmp);
			mlerase();
			if (s != TRUE)
				return s;
		}
	} else if (doglob(Reply) != TRUE) {
		return FALSE;
	}

	(void)strcpy (result, Reply);
	if (flag <= FILEC_WRITE) {	/* we want to write a file */
		if (!isInternalName(Reply)
		 && !same_fname(Reply, curbp, TRUE)
		 && flook(Reply, FL_HERE|FL_READABLE)) {
			if (mlyesno("File exists, okay to overwrite") != TRUE) {
				mlwrite("File not written");
				return FALSE;
			}
		}
	}

	(void)tb_scopy(buffer, Reply);
	return TRUE;
}

/******************************************************************************/
int
mlreply_dir(
const char * prompt,
TBUFF **buffer,
char *	result)
{
	register int	s;
	char	Reply[NFILEN];
	int	(*complete) (DONE_ARGS) = no_completion;

#if COMPLETE_DIRS
	if (isnamedcmd && !clexec) {
		complete = path_completion;
		init_filec(DIRCOMPLETION_BufName);
	}
#endif
	if (clexec || isnamedcmd) {
		if (tb_values((*buffer)) != 0)
			(void)strcpy(Reply, tb_values((*buffer)));
		else
			*Reply = EOS;

		only_dir = TRUE;
		s = kbd_string(prompt, Reply, sizeof(Reply), '\n',
			KBD_OPTIONS|KBD_MAYBEC, complete);
		freeMyList();
		only_dir = FALSE;
		if (s != TRUE)
			return s;

        } else if (!screen_to_bname(Reply)) {
		return FALSE;
        }

	(void)tb_scopy(buffer, strcpy(result, Reply));
	return TRUE;
}

/******************************************************************************/

/*
 * This is called after 'mlreply_file()' to iterate over the list of files
 * that are matched by a glob-expansion.
 */
char *
filec_expand(void)
{
	if (MyGlob != 0) {
		if (MyGlob[++in_glob] != 0)
			return MyGlob[in_glob];
		free_expansion();
	}
	return 0;
}
