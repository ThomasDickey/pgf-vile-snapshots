/*
 *	filec.c
 *
 *	Filename prompting and completion routines
 *
 * $Log: filec.c,v $
 * Revision 1.3  1993/03/16 16:04:01  pgf
 * fix 'parentheses suggested' warnings
 *
 * Revision 1.2  1993/03/16  10:53:21  pgf
 * see 3.36 section of CHANGES file
 *
 * Revision 1.1  1993/03/05  18:45:59  pgf
 * fixes for null pointers and filenames
 *
 * Revision 1.0  1993/03/04  15:06:38  pgf
 * Initial revision
 *
 */

#include "estruct.h"
#include "edef.h"

#define	SLASH (EOS+1) /* less than everything but EOS */

#if UNIX || defined(MDTAGSLOOK)

#include <sys/stat.h>

#if POSIX
#include <dirent.h>
#define	DIRENT	struct dirent
#else	/* apollo & other old bsd's */
#include <sys/dir.h>
#define	DIRENT	struct direct
#endif

/*
 * Test if the given path is a directory
 */
static int
is_directory(path)
char *	path;
{
	struct	stat	sb;

	return ((*path != EOS)
	  &&	(stat(path, &sb) >= 0)
	  &&	((sb.st_mode & S_IFMT) == S_IFDIR));
}

/*
 * Test if the path has a trailing slash-delimiter
 */
static int
trailing_slash(path)
char *	path;
{
	register int	len = strlen(path);
	return (len > 1
	  &&	slashc(path[len-1]));
}

/*
 * Force a trailing slash on the end of the path, returns the length of the
 * resulting path.
 */
static int
force_slash(path)
char *	path;
{
	register int	len = strlen(path);

	if (!trailing_slash(path)) {
		path[len++] = slash;
		path[len] = EOS;
	}
	return len;
}

/*
 * Test if the path has a trailing SLASH-delimiter
 */
static int
trailing_SLASH(path)
char *	path;
{
	register int	len = strlen(path);
	return (len > 1
	  &&	path[len-1] == SLASH);
}

/*
 * Force a trailing SLASH on the end of the path, returns the length of the
 * resulting path.
 */
static int
force_SLASH(path)
char *	path;
{
	register int	len = strlen(path);

	if (!trailing_SLASH(path)) {
		path[len++] = SLASH;
		path[len] = EOS;
	}
	return len;
}

/*
 * Because the slash-delimiter is not necessarily lexically before any of the
 * other characters in a path, provide a conversion that makes it so.  Then,
 * using 'strcmp()', we will get lexically-sorted paths.
 */
static void
conv_path(dst, src, len)
char *	dst;
char *	src;
int	len;
{
	register int	c;

	while ((len-- > 0) && (c = *src++)) {
		if (slashc(c))
			c = SLASH;
		*dst++ = c;
	}
	*dst = EOS;
}

/*
 * Compare two paths lexically.  The text-string is normally not a full-path,
 * so we must find an appropriate place along the lp-string to start the
 * comparison.
 */
static int
pathcmp(lp, text)
LINE *	lp;
char *	text;
{
	char	ref[NFILEN],
		tst[NFILEN];
	int	reflen = llength(lp),	/* length, less null */
		tstlen = strlen(text);
	register int	j, k;

	if (reflen <= 0)	/* cannot compare */
		return 1;

	conv_path(ref, lp->l_text, reflen);
	conv_path(tst, text,       tstlen);

	/* If we stored a trailing slash on the ref-value, then it was known to
	 * be a directory.  Append a slash to the tst-value in that case to
	 * force a match if it is otherwise the same.
	 */
	if (trailing_SLASH(ref))
		(void)force_SLASH(tst);
	else if (trailing_SLASH(tst))
		(void)force_SLASH(ref);

	/* count the slashes embedded in text-string */
	for (j = k = 0; tst[j] != EOS; j++)
		if (tst[j] == SLASH)
			k++;

	/* skip back so we have the same number of slashes in lp-string */
	j = strlen(ref);
	if (k > 0) {
		for (; j >= 0; j--)
			if (ref[j] == SLASH)
				if (--k <= 0)
					break;
	}

	if ((k == 0) && (j > 0)) {
		/* skip back to include the leading leaf */
		if (tst[0] != SLASH) {
			j--;
			while ((j >= 0) && (ref[j] != SLASH))
				j--;
			if (ref[j] == SLASH)
				j++;	/* back to the beginning of leaf */
		}
	} else
		j = 0;	/* cannot get there */

#if APOLLO
	if (j == 1)		/* we have leading "//" on apollo */
		j = 0;
#endif
	return strcmp(ref+j, tst);
}

/*
 * Insert a pathname at the given line-pointer.
 * Allocate 2 extra bytes for EOS and (possible) trailing slash.
 */
static LINE *
makeString(bp, lp, text)
BUFFER *bp;
LINE *	lp;
char *	text;
{
	register LINE	*np;
	register int	len = strlen(text);

	if ((np=lalloc(len+2,bp)) == NULL) {
		lp = 0;
	} else {
		(void)strcpy(np->l_text, text);
		llength(np) -= 2;	/* hide the null */

		lp->l_bp->l_fp = np;
		np->l_bp = lp->l_bp;
		lp->l_bp = np;
		np->l_fp = lp;
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
bs_init(name, first)
char *	name;
int	first;
{
	register BUFFER *bp;

	if ((bp = bfind(name, OK_CREAT, BFINVS)) != 0) {
		bp->b_flag &= ~BFSCRTCH;	/* make it nonvolatile */
		if (first == -TRUE) {
			(void)bclear(bp);
			bp->b_active = TRUE;
		}
#ifdef GMDTAGSLOOK	/* looktags keeps buffer in-place */
		else if (first == TRUE) {	/* check if editing has broken null-endings */
			register LINE	*lp;
			for_each_line(lp,bp) {
				register int	n = llength(lp);

				if ((n >= lp->l_size)
				 || (lp->l_text[n] != EOS)
				 || (n+1 >= lp->l_size
				  && !trailing_slash(lp->l_text))) {
					char	temp[NLINE];
					LINE	*np;
					(void)strncpy(temp, lp->l_text, sizeof(temp));
					if (np = makeString(bp, lp, temp)) {
						lremove(bp, lp);
						lp = lback(np);
					}
				}
			}
		}
#endif	/* GMDTAGSLOOK */
	}
	return bp;
}

/*
 * Look for or insert a pathname string into the given buffer.  Start looking
 * at the given line if non-null.
 */
int
bs_find(text, len,  bp, iflag, lpp)
char *	text;	/* pathname to find */
int	len;	/* ...its length */
BUFFER *bp;	/* buffer to search */
int	iflag;	/* true to insert if not found, -true if it is directory */
LINE **	lpp;	/* in/out line pointer, for iteration */
{
	register LINE	*lp;
	int	doit	= FALSE;
	char	fname[NFILEN];

	/*
	 * If we are only looking up a name (for "looktags"), keep the name as
	 * we are given it.  If told that we might insert the name, convert it
	 * to absolute form.  In the special case of inserting a directory
	 * name, append a slash on the end so that we can see this in the name
	 * completion.
	 */
	strncpy(fname, text, len)[len] = EOS;
	if (iflag) {
		/* always store full paths */
		(void)lengthen_path(fname);
		if (iflag == -TRUE)
			(void)force_slash(fname);
	}

	if (lpp == NULL || (lp = *lpp) == NULL)
		lp = bp->b_line.l;
	lp = lforw(lp);

	for (;;) {
		register int r = pathcmp(lp, fname);

		if (r == 0) {
			if (iflag == -TRUE
			 &&  trailing_slash(fname)
			 && !trailing_slash(lp->l_text)) {
				/* reinsert so it is sorted properly! */
				lremove(bp, lp);
				return bs_find(text, len,  bp, iflag, lpp);
			}
			break;
		} else if (iflag && (r > 0)) {
			doit = TRUE;
			break;
		}

		lp = lforw(lp);
		if (lp == bp->b_line.l) {
		 	if (!iflag)
				return FALSE;
			doit = TRUE;
			break;
		}
	}

	if (doit)
		lp = makeString(bp, lp, fname);

	if (lpp)
		*lpp = lp;
	return TRUE;
}
#endif /* UNIX || defined(MDTAGSLOOK) */

#if UNIX
static	BUFFER	*MyBuff;	/* the buffer containing pathnames */
static	char	*MyName;	/* name of buffer for name-completion */
static	char	**MyList;	/* list, for name-completion code */
static	unsigned MySize;	/* length of list, for (re)allocation */
static	int	only_dir;	/* can only match real directories */

/*
 * Tests if the given path has been scanned during this prompt/reply operation
 *
 * If there is anything else in the list that we can do completion with, return
 * true.  This allows the case in which we scan a directory (for directory
 * completion and then look at the subdirectories.  Note that it may result in
 * re-scanning a directory that has no subdirectories, but this happens only
 * during directory completion, which is slow anyway.
 */
static int
already_scanned(path)
char *	path;
{
	register LINE	*lp;
	register int	len;
	char	fname[NFILEN];

	len = force_slash(strcpy(fname, path));

	for_each_line(lp,MyBuff)
		if (!strcmp(fname, lp->l_text)) {
			LINE	*np = lforw(lp);

			if (llength(np) > 0
			 && !strncmp(path, np->l_text, strlen(path)))
				return TRUE;
		}

	/* force the name in with a trailing slash */
	(void)bs_find(fname, len, MyBuff, -TRUE, (LINE **)0);
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

	char	path[NFILEN],
		leaf[NFILEN];

	int	iflag;

	(void)strcpy(path, name);
	if (is_directory(path)) {
		*leaf = EOS;
	} else {
		if (((s = strrchr(path, slash)) != 0) && (*++s != EOS)) {
			(void)strcpy(leaf, s);
			*--s = EOS;
		}
		if (!is_directory(path))
			return;
	}

	if (already_scanned(path))
		return;

	if ((dp = opendir(path)) != 0) {
		s = path + force_slash(path);

		while ((de = readdir(dp)) != 0) {
			strncpy(s, de->d_name, (int)(de->d_namlen))[de->d_namlen] = EOS;
			if (!strcmp(s, ".")
			 || !strcmp(s, ".."))
			 	continue;
			if (only_dir) {
				if (!is_directory(path))
					continue;
				iflag = -TRUE;
			} else {
				iflag = (global_g_val(GMDDIRC) && is_directory(path))
					? -TRUE
					: TRUE;
			}
			(void)bs_find(path, (int)strlen(path), MyBuff, iflag, (LINE **)0);
		}
		(void)closedir(dp);
	}
}

/*
 * Make the list of names needed for name-completion
 */
static void
makeMyList()
{
	register int	need, n;
	register LINE *	lp;

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
		if (only_dir || !trailing_slash(lp->l_text))
			MyList[n++] = lp->l_text;
	MyList[n] = 0;
}

#if NO_LEAKS
static void
freeMyList()
{
	if (MyList != 0) {
		free((char *)MyList);
		MyList = 0;
		MySize = 0;
	}
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
	int	code	= FALSE;

	if ((c == NAMEC || c == TESTC) && !isInternalName(buf)) {
		char	*s;
		char	path[NFILEN];
		int	oldlen,
			newlen;

		/* initialize only on demand */
		if (MyBuff == 0) {
			if ((MyName == 0)
			 || (MyBuff = bs_init(MyName, -TRUE)) == 0)
			 	return FALSE;
		}

		/* trim trailing "." if it is a "/." */
		if ((s = strrchr(buf, slash)) != 0
		 && !strcmp(s+1, "."))
			kbd_kill_response(buf, pos, '\b');

		(void)lengthen_path(strcpy(path, buf));
		if (!(s = is_appendname(buf)))
			s = buf;
		if ((*s == EOS) || trailing_slash(s))
			(void)force_slash(path);

		if ((s = is_appendname(path)) != 0) {
			register char *d;
			for (d = path; (*d++ = *s++) != 0; )
				;
		}

		newlen =
		oldlen = strlen(path);

		fillMyBuff(path);
		makeMyList();

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
			kbd_putc(slash);
			TTflush();
			buf[*pos] = slash;
			*pos += 1;
			buf[*pos] = EOS;
			code = FALSE;
		}
	}
	return code;
}
#endif	/* UNIX */

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
	int	had_fname = (curbp != 0 && curbp->b_fname != 0);
	int	do_prompt = (clexec || isnamedcmd || (flag & FILEC_PROMPT));

	flag &= ~ FILEC_PROMPT;

#ifdef GMDTAGSLOOK	/* looktags overrides file completion */
	if (flag >= FILEC_UNKNOWN && global_g_val(GMDTAGSLOOK))
		;
	else
#endif
#if UNIX
	if (isnamedcmd && !clexec) {
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
				? shorten_path(strcpy(Reply, curbp->b_fname))
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
			'\n', KBD_NORMAL|KBD_MAYBEC, complete);
		freeMyList();

		if (s != TRUE) {
			if ((flag == FILEC_REREAD)
			 && had_fname
			 && (mlyesno("Reread current buffer")))
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
	if ((s = glob(Reply)) != TRUE)
		return FALSE;

	(void)strcpy (result, Reply);
#ifdef GMDTAGSLOOK
	if (flag >= FILEC_UNKNOWN && global_g_val(GMDTAGSLOOK)) {
		LINE	*lp = 0;
		BUFFER	*bp;
		int	count = 0;

		while (flook(result, FL_HERE) == NULL) {
			if (((bp = look_tags(count++)) == 0)
			|| !bs_find(Reply, (int)strlen(Reply), bp, FALSE, &lp)) {
				/* give up, and try what they asked for */
				(void)strcpy(result, Reply);
				break;
			}
			(void)strcpy(result, lp->l_text);
		}
	}
#endif
	if (flag <= FILEC_WRITE) {	/* we want to write a file */
		if (strcmp(Reply, curbp->b_fname) != 0
		 && !isShellOrPipe(Reply)
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
#if UNIX
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
			KBD_NORMAL|KBD_MAYBEC, complete);
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
