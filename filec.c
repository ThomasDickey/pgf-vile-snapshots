/*
 *	filec.c
 *
 * Filename prompting and completion routines
 * Written by T.E.Dickey for vile (march 1993).
 *
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/filec.c,v 1.35 1994/08/09 20:07:03 pgf Exp $
 *
 */

#include "estruct.h"
#include "edef.h"

#define	SLASH (EOS+1) /* less than everything but EOS */

#if VMS
#define	KBD_OPTIONS	KBD_NORMAL|KBD_UPPERC
#endif

#ifdef problemwithlowercasenames  /* ? */
#if MSDOS
#define	KBD_OPTIONS	KBD_NORMAL|KBD_LOWERC
#endif
#endif

#ifndef	KBD_OPTIONS
#define	KBD_OPTIONS	KBD_NORMAL
#endif

static	char	**MyGlob;	/* expanded list */
static	int	in_glob;	/* index into MyGlob[] */
static	int	only_dir;	/* can only match real directories */

static void
free_expansion P(( void ))
{
	MyGlob = glob_free(MyGlob);
	in_glob = -1;
}

#if COMPLETE_DIRS || COMPLETE_FILES

#include "dirstuff.h"

static	int	trailing_slash P((char *));
static	SIZE_T	force_slash P((char *));
static	int	pathcmp P((LINE *, char *));
static	LINE *	makeString P((BUFFER *, LINE *, char *, SIZE_T));

/*--------------------------------------------------------------------------*/

/*
 * Test if the path has a trailing slash-delimiter (i.e., can be syntactically
 * distinguished from non-directory paths).
 */
static int
trailing_slash(path)
char *	path;
{
	register int	len = strlen(path);
	if (len > 0) {
#if VMS
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
force_slash(path)
char *	path;
{
	register SIZE_T	len = strlen(path);

#if VMS
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
pathcmp(lp, text)
LINE *  lp;
char *  text;
{
    register char *l, *t;
    register int lc, tc;

    if (llength(lp) <= 0)	/* Can this happen? */
	return -1;

    l = lp->l_text;
    t = text;
    for (;;) {
	lc = *l++;
	tc = *t++;
	if (lc == tc) {
	    if (tc == EOS)
		return 0;
	}
	else {
	    if (is_slashc(lc)) {
		if (*l != EOS)
		    lc = SLASH;
		else
		    lc = EOS;
	    }
	    if (is_slashc(tc)) {
		if (*t != EOS)
		    tc = SLASH;
		else
		    tc = EOS;
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
makeString(bp, lp, text, len)
BUFFER *bp;
LINE *	lp;
char *	text;
SIZE_T	len;
{
	register LINE	*np;
	int extra = (len > 0 && is_slashc(text[len-1])) ? 2 : 3;

	if ((np = l_ref(lalloc((int)len+extra, bp))) == NULL) {
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
BUFFER *
bs_init(name)
char *	name;
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
int
bs_find(fname, len,  bp, lpp)
char *	fname;	/* pathname to find */
SIZE_T	len;	/* ...its length */
BUFFER *bp;	/* buffer to search */
LINEPTR *lpp;	/* in/out line pointer, for iteration */
{
	register LINE	*lp;
	int	doit	= FALSE;

	if (lpp == NULL || (lp = l_ref(*lpp)) == NULL)
		lp = l_ref(buf_head(bp));
	lp = lforw(lp);

	for (;;) {
		register int r = pathcmp(lp, fname);

		if (r == 0) {
			if (trailing_slash(fname)
			 && !trailing_slash(lp->l_text)) {
				/* reinsert so it is sorted properly! */
				lremove(bp, l_ptr(lp));
				return bs_find(fname, len,  bp, lpp);
			}
			break;
		} else if (r > 0) {
			doit = TRUE;
			break;
		}

		lp = lforw(lp);
		if (lp == l_ref(buf_head(bp))) {
			doit = TRUE;
			break;
		}
	}

	if (doit) {
		lp = makeString(bp, lp, fname, len);
		b_clr_counted(bp);
	}

	if (lpp)
		*lpp = l_ptr(lp);
	return TRUE;
}
#endif /* COMPLETE_DIRS || COMPLETE_FILES */

#if COMPLETE_DIRS || COMPLETE_FILES

static	int	already_scanned P((char *));
static	void	fillMyBuff P((char *));
static	void	makeMyList P((char *));
#if NO_LEAKS
static	void	freeMyList P((void));
#endif
static	int	path_completion P(( int, char *, int * ));

static	BUFFER	*MyBuff;	/* the buffer containing pathnames */
static	char	*MyName;	/* name of buffer for name-completion */
static	char	**MyList;	/* list, for name-completion code */
static	ALLOC_T MySize;		/* length of list, for (re)allocation */

/*
 * Tests if the given path has been scanned during this prompt/reply operation
 *
 * If there is anything else in the list that we can do completion with, return
 * true.  This allows the case in which we scan a directory (for directory
 * completion and then look at the subdirectories.  It should not permit
 * directories which have already been scanned to be rescanned.
 */
static int
already_scanned(path)
char *	path;
{
	register LINE	*lp;
	register SIZE_T	len;
	char	fname[NFILEN];
	LINEPTR slp;

	len = force_slash(strcpy(fname, path));

	for_each_line(lp,MyBuff)
		if (strcmp(fname, lp->l_text) == 0) {
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
	lp = l_ref(slp);
	lp->l_text[llength(lp)+1] = 1;
	return FALSE;
}

/*
 * If the given path is not in the completion-buffer, expand it, and add the
 * expanded paths to the buffer.  Because the user may be trying to get an
 * intermediate directory-name, we must 'stat()' each name, so that we can
 * provide the trailing slash in the completion.  This is slow.
 */
static void
fillMyBuff(name)
char *	name;
{
	register char	*s;

	DIR	*dp;
	DIRENT	*de;

	char	path[NFILEN];

	(void)strcpy(path, name);
	if (!is_directory(path)) {
		*pathleaf(path) = EOS;
		if (!is_directory(path))
			return;
	}

	if (already_scanned(path))
		return;

	if ((dp = opendir(path)) != 0) {
		s = path;
#if VMS
		if (!is_vms_pathname(path, -TRUE))
#endif
		s += force_slash(path);

		while ((de = readdir(dp)) != 0) {
#if UNIX || VMS || OS2 || NT
# if USE_D_NAMLEN
			strncpy(s, de->d_name, (SIZE_T)(de->d_namlen))[de->d_namlen] = EOS;
# else
			(void)strcpy(s, de->d_name);
# endif
#else
# if MSDOS
			(void)mklower(strcpy(s, de->d_name));
# else
			huh??
# endif
#endif
#if UNIX || MSDOS || OS2 || NT
			if (!strcmp(s, ".")
			 || !strcmp(s, ".."))
			 	continue;
#endif
#if VMS
			/* convert ".dir" files to "]" form */
			if ((s = strchr(path, ']')) != 0
			 && (s[1] != EOS)) {
				register char *t;
				if ((t = strchr(s, '.')) != 0
				 && !strcmp(t, ".DIR;1")) {
					*s = '.';
					*t++ = ']';
					*t = EOS;
				}
			}
			s = path;
#endif
			if (only_dir) {
				if (!is_directory(path))
					continue;
				(void) force_slash(path);
			}
#if COMPLETE_DIRS
			else {
				if (global_g_val(GMDDIRC) && is_directory(path))
					(void) force_slash(path);
			}
#endif
			(void)bs_find(path, (SIZE_T)strlen(path), MyBuff,
					(LINEPTR*)0);
		}
		(void)closedir(dp);
	}
}

/*
 * Make the list of names needed for name-completion
 */
static void
makeMyList(name)
char *name;
{
	register int	need, n;
	register LINE *	lp;
	char *slashocc;
	int len = strlen(name);

	if (is_slashc(name[len-1]))
	    len++;

	bsizes(MyBuff);
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
freeMyList()
{
	FreeAndNull(MyList);
	MySize = 0;
}
#else
#define	freeMyList()
#endif

/*
 * Perform the name-completion/display.  Note that we must convert a copy of
 * the pathname to absolute form so that we can match against the strings that
 * are stored in the completion table.  However, the characters that might be
 * added are always applicable to the original buffer.
 *
 * We only do name-completion if asked; if we did it when the user typed a
 * return it would be too slow.
 */
static int
path_completion(c, buf, pos)
int	c;
char	*buf;
int	*pos;
{
	int	code	= FALSE,
		action	= (c == NAMEC || c == TESTC),
		ignore	= (*buf != EOS && isInternalName(buf));

#if VMS
	if (ignore && action) {		/* resolve scratch-name conflict */
		if (is_vms_pathname(buf, -TRUE))
			ignore = FALSE;
	}
#endif
	if (ignore) {
		if (action) {
			kbd_putc(c);	/* completion-chars have no meaning */
			TTflush();
			buf[*pos] = c;
			*pos += 1;
			buf[*pos] = EOS;
		}
	} else if (action) {
		char	*s;
		char	path[NFILEN];
		int	oldlen,
			newlen;

		/* initialize only on demand */
		if (MyBuff == 0) {
			if ((MyName == 0)
			 || (MyBuff = bs_init(MyName)) == 0)
			 	return FALSE;
		}

		/*
		 * Copy 'buf' into 'path', making it canonical-form.
		 */
#if UNIX || MSDOS || OS2 || NT
		/* trim trailing "." if it is a "/." */
		if ((s = last_slash(buf)) != 0
		 && !strcmp(s+1, "."))
			kbd_kill_response(buf, pos, '\b');

		(void)lengthen_path(strcpy(path, buf));
#endif

#if VMS
		if (*strcpy(path, buf) == EOS) {
			(void)strcpy(path, current_directory(FALSE));
		} else {
			char	frac[NFILEN];
			*frac = EOS;
			if (is_vms_pathname(path, -TRUE)) {
				s = vms_pathleaf(path);
				(void)strcpy(frac, s);
				*s = EOS;
			}
			if (*path == EOS)
				(void)strcpy(path, current_directory(FALSE));
			else
				(void)lengthen_path(path);
			(void)strcat(path, frac);
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

		newlen =
		oldlen = strlen(path);

		fillMyBuff(path);
		makeMyList(path);

		/* patch: should also force-dot to the matched line, as in history.c */
		/* patch: how can I force buffer-update to show? */

		code = kbd_complete(c, path, &newlen, (char *)&MyList[0], sizeof(MyList[0]));
		(void)strcat(buf, path+oldlen);
		*pos = strlen(buf);

		/* avoid accidentally picking up directory names for files */
		if ((code == TRUE)
		 && !only_dir
		 && !trailing_slash(path)
		 && is_directory(path)) {
			kbd_putc(SLASHC);
			TTflush();
			buf[*pos] = SLASHC;
			*pos += 1;
			buf[*pos] = EOS;
			code = FALSE;
		}
	}
	return code;
}
#else	/* no filename-completion */
#define	freeMyList()
#endif	/* filename-completion */

/******************************************************************************/
int
mlreply_file(prompt, buf, flag, result)
char *	prompt;
TBUFF **buf;
int	flag;		/* +1 to read, -1 to write, 0 don't care */
char *	result;
{
	register int	s;
	static	TBUFF	*last;
	char	Reply[NFILEN];
	int	(*complete) P(( int, char *, int *)) = no_completion;
	int	had_fname = (curbp != 0
			  && curbp->b_fname != 0
			  && curbp->b_fname[0] != EOS);
	int	do_prompt = (clexec || isnamedcmd || (flag & FILEC_PROMPT));
	int	ok_expand = (flag & FILEC_EXPAND);

	flag &= ~ (FILEC_PROMPT | FILEC_EXPAND);

#if COMPLETE_FILES
	if (do_prompt && !clexec) {
		complete = path_completion;
		MyBuff = 0;
		MyName = ScratchName(FileCompletion);
	}
#endif

	/* use the current filename if none given */
	if (buf == 0) {
		(void)tb_scopy(
			buf = &last,
			had_fname && is_pathname(curbp->b_fname)
				? shorten_path(strcpy(Reply, curbp->b_fname),
				FALSE)
				: "");
	}

	if (do_prompt) {
		char	*t1 = tb_values(*buf),
			*t2 = is_appendname(t1);

		if (t1 != 0)
			(void)strcpy(Reply, (t2 != 0) ? t2 : t1);
		else
			*Reply = EOS;

	        s = kbd_string(prompt, Reply, NFILEN,
			'\n', KBD_OPTIONS|KBD_MAYBEC, complete);
		freeMyList();

		if (s == ABORT)
			return s;
		if (s != TRUE) {
			if ((flag == FILEC_REREAD)
			 && had_fname
			 && ((s = mlyesno("Reread current buffer")) == TRUE))
				(void)strcpy(Reply, curbp->b_fname);
			else
	                	return s;
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
			(void)lsprintf(tmp, "Will read %d files. Okay", s);
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
		 && flook(Reply, FL_HERE)) {
			if (mlyesno("File exists, okay to overwrite") != TRUE) {
				mlwrite("File not written");
				return FALSE;
			}
		}
	}

	(void)tb_scopy(buf, Reply);
	return TRUE;
}

/******************************************************************************/
int
mlreply_dir(prompt, buf, result)
char *	prompt;
TBUFF **buf;
char *	result;
{
	register int	s;
	char	Reply[NFILEN];
	int	(*complete) P(( int, char *, int *)) = no_completion;

#if COMPLETE_DIRS
	if (isnamedcmd && !clexec) {
		complete = path_completion;
		MyBuff = 0;
		MyName = ScratchName(DirCompletion);
	}
#endif
	if (clexec || isnamedcmd) {
		if (tb_values((*buf)) != 0)
			(void)strcpy(Reply, tb_values((*buf)));
		else
			*Reply = EOS;

		only_dir = TRUE;
	        s = kbd_string(prompt, Reply, NFILEN, '\n',
			KBD_OPTIONS|KBD_MAYBEC, complete);
		freeMyList();
		only_dir = FALSE;
		if (s != TRUE)
			return s;

        } else if (!screen_to_bname(Reply)) {
		return FALSE;
        }

	(void)tb_scopy(buf, strcpy(result, Reply));
	return TRUE;
}

/******************************************************************************/

/*
 * This is called after 'mlreply_file()' to iterate over the list of files
 * that are matched by a glob-expansion.
 */
char *
filec_expand()
{
	if (MyGlob != 0) {
		if (MyGlob[++in_glob] != 0)
			return MyGlob[in_glob];
		free_expansion();
	}
	return 0;
}
