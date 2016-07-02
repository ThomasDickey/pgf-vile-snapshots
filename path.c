/*	PATH.C	
 *		The routines in this file handle the conversion of pathname
 *		strings.
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/path.c,v 1.59 1995/10/19 20:02:25 pgf Exp $
 *
 *
 */

#include	"estruct.h"
#include        "edef.h"

#if SYS_UNIX
#include <sys/types.h>
#include <pwd.h>
#endif

#if SYS_VMS
#include <starlet.h>
#include <file.h>
#endif

#include <sys/stat.h>

#include "dirstuff.h"

#if (SYS_WIN31 && CC_TURBO) || SYS_WINNT
# include <direct.h>
# define curdrive() (_getdrive() + ('A' - 1))
# define curr_dir_on_drive(d) _getdcwd(d, temp, sizeof(temp))
#endif

#if HAVE_SYMLINK && !OPT_VMS_PATH
#define USE_RESOLVE_DIR 1
#else
#define USE_RESOLVE_DIR 0
#endif

#if USE_RESOLVE_DIR
#if HAVE_SYS_ITIMER_H && SYSTEM_LOOKS_LIKE_SCO
#include <sys/itimer.h>
#endif
static	char * resolve_directory P(( char *, char ** ));
#endif

static	char * canonpath P(( char * ));

#if OPT_CASELESS
static	int case_correct_path P((char *old_file, char *new_file));
#endif

/*
 * Fake directory-routines for system where we cannot otherwise figure out how
 * to read the directory-file.
 */
#if USE_LS_FOR_DIRS
DIR *
opendir (path)
char	*path;
{
	static	char	fmt[] = "/bin/ls %s";
	char	lscmd[NFILEN+sizeof(fmt)];

	(void)lsprintf(lscmd, fmt, path);
	return npopen(lscmd, "r");
}

DIRENT *
readdir (dp)
DIR	*dp;
{
	static	DIRENT	dummy;

	if ((fgets(dummy.d_name, NFILEN, dp)) != NULL) {
		/* zap the newline */
		dummy.d_name[strlen(dummy.d_name)-1] = EOS;
		return &dummy;
	}
	return 0;
}

int
closedir (dp)
DIR	*dp;
{
	(void)npclose(dp);
	return 0;
}
#endif

/*
 * Use this routine to fake compatibility with unix directory routines.
 */
#if OLD_STYLE_DIRS
DIRENT *
readdir(dp)
DIR	*dp;
{
	static	DIRENT	dbfr;
	return (fread(&dbfr, sizeof(dbfr), 1, dp)\
				? &dbfr\
				: (DIRENT *)0);
}
#endif

#if OPT_MSDOS_PATH
/*
 * If the pathname begins with an MSDOS-drive, return the pointer past it.
 * Otherwise, return null.
 */
char *
is_msdos_drive(path)
char	*path;
{
#if OPT_UNC_PATH
	if (is_slashc(path[0]) && is_slashc(path[1])) {
#if !OPT_CASELESS
		(void)mklower(path);	/* MS-DOS isn't case-sensitive */
#endif
		return path+1;
	}
#endif
	if (isalpha(path[0]) && path[1] == ':') {
#if !OPT_CASELESS
		(void)mklower(path);	/* MS-DOS isn't case-sensitive */
#endif
		return path+2;
	}
	return NULL;
}
#endif

#if OPT_VMS_PATH
#define VMSPATH_END_NODE   1
#define VMSPATH_END_DEV    2
#define VMSPATH_BEGIN_DIR  3
#define VMSPATH_NEXT_DIR   4
#define VMSPATH_END_DIR    5
#define	VMSPATH_BEGIN_FILE 6
#define VMSPATH_BEGIN_TYP  7
#define VMSPATH_BEGIN_VER  8

/*
 * Returns true if the string is delimited in a manner compatible with VMS
 * pathnames.  To be consistent with the use of 'is_pathname()', insist that
 * at least the "[]" characters be given.
 *
 * Complete syntax:
 *	node::device:[dir1.dir2]filename.type;version
 *	    ^1     ^2^3   ^4  ^5^6      ^7   ^8
 */
int
is_vms_pathname(path, option)
char	*path;
int	option;		/* true:directory, false:file, -true:don't care */
{
	char	*base	= path;
	int	this	= 0,
		next	= -1;

	if (*path == EOS)	/* this can happen with null buffer-name */
		return FALSE;

	while (ispath(*path)) {
		switch (*path) {
		case '[':
			if (this >= VMSPATH_BEGIN_FILE)
				return FALSE;
			next = VMSPATH_BEGIN_DIR;
			break;
		case ']':
			if (this < VMSPATH_BEGIN_DIR)
				return FALSE;
			if (path != base	/* rooted logical? */
			 && path[1] == '['
			 && path[-1] == '.')
				path++;
			else
				next = VMSPATH_END_DIR;
			break;
		case '.':
			if (this >= VMSPATH_BEGIN_TYP)
				return FALSE;
			next = (this >= VMSPATH_END_DIR)
				? VMSPATH_BEGIN_TYP
				: (this >= VMSPATH_BEGIN_DIR
					? VMSPATH_NEXT_DIR
					: VMSPATH_BEGIN_TYP);
			break;
		case ';':
			next = VMSPATH_BEGIN_VER;
			break;
		case ':':
			if (path[1] == ':') {
				path++;	/* eat "::" */
				if (this >= VMSPATH_END_NODE)
					return FALSE;
				next = VMSPATH_END_NODE;
			} else
				next = VMSPATH_END_DEV;
			break;
		case '!':
		case '/':
		case '~':
			return FALSE;	/* a DEC-shell name */
		default:
			if (!ispath(*path))
				return FALSE;
			next = (this == VMSPATH_END_DIR)
				? VMSPATH_BEGIN_FILE
				: this;
			break;
		}
		if (next < this)
			break;
		this = next;
		path++;
	}

	if ((*path != EOS)
	 || (this  <  next))
		return FALSE;

	if (this == 0)
		this = VMSPATH_BEGIN_FILE;

	return (option == TRUE  && (this == VMSPATH_END_DIR))	/* dir? */
	  ||   (option == TRUE  && (this == VMSPATH_END_DEV))	/* dev? */
	  ||   (option == FALSE && (this >= VMSPATH_BEGIN_FILE))/* file? */
	  ||   (option == -TRUE && (this >= VMSPATH_END_DIR	/* anything? */
				 || this <  VMSPATH_BEGIN_DIR));
}
#endif

#if OPT_VMS_PATH
/*
 * Returns a pointer to the argument's last path-leaf (i.e., filename).
 */
char *
vms_pathleaf(path)
char	*path;
{
	register char	*s;
	for (s = path + strlen(path);
		s > path && !strchr(":]", s[-1]);
			s--)
		;
	return s;
}
#endif

/*
 * Returns a pointer to the argument's last path-leaf (i.e., filename).
 */

#if CC_WATCOM

char *
pathleaf(path)
char	*path;
{
	register char	*s = last_slash(path);
	if (s == 0) {
#if OPT_MSDOS_PATH
		if (!(s = is_msdos_drive(path)))
#endif
		s = path;
	} else
		s++;
	return s;
}

#else

#if !OPT_VMS_PATH
#define	unix_pathleaf	pathleaf
#endif

char *
unix_pathleaf(path)
char	*path;
{
	register char	*s = last_slash(path);
	if (s == 0) {
#if OPT_MSDOS_PATH
		if (!(s = is_msdos_drive(path)))
#endif
		s = path;
	} else
		s++;
	return s;
}
#endif /* WATCOMC */


#if OPT_VMS_PATH
char *pathleaf(path)
char	*path;
{
	if (is_vms_pathname(path, -TRUE))
		return vms_pathleaf(path);
	return unix_pathleaf(path);
}
#endif

/*
 * Concatenates a directory and leaf name to form a full pathname
 */
char *
pathcat (dst, path, leaf)
char	*dst;
char	*path;
char	*leaf;
{
	char	save_path[NFILEN];
	char	save_leaf[NFILEN];
	register char	*s = dst;

	if (path == 0 || *path == EOS)
		return strcpy(dst, leaf);

	path = strcpy(save_path, path);		/* in case path is in dst */
	leaf = strcpy(save_leaf, leaf);		/* in case leaf is in dst */

	(void)strcpy(s, path);
	s += strlen(s) - 1;

#if OPT_VMS_PATH
	if (!is_vms_pathname(dst, TRUE))	/* could be DecShell */
#endif
	if (!is_slashc(*s)) {
		*(++s) = SLASHC;
	}

	(void)strcpy(s+1, leaf);

#if OPT_VMS_PATH
	if (is_vms_pathname(path, -TRUE)
	 && is_vms_pathname(leaf, -TRUE)
	 && !is_vms_pathname(dst, -TRUE))
		(void)strcpy(dst, leaf);
#endif
	return dst;
}

/*
 * Tests to see if the string contains a slash-delimiter.  If so, return the
 * last one (so we can locate the path-leaf).
 */
char *
last_slash(fn)
char *fn;
{
	register char	*s;

	for (s = fn + strlen(fn) - 1; s >= fn; s--)
		if (is_slashc(*s))
			return s;
	return 0;
}

/*
 * If a pathname begins with "~", lookup the name in the password-file.  Cache
 * the names that we lookup, because searching the password-file can be slow,
 * and users really don't move that often.
 */
#if SYS_UNIX
typedef	struct	_upath {
	struct	_upath *next;
	char	*name;
	char	*path;
	} UPATH;

static	UPATH	*user_paths;

static	char *	save_user P(( char *, char * ));

static char *
save_user(name, path)
char	*name;
char	*path;
{
	register UPATH *q;

	if (name != NULL
	 && path != NULL
	 && (q = typealloc(UPATH)) != NULL) {
		if ((q->name = strmalloc(name)) != NULL
		 && (q->path = strmalloc(path)) != NULL) {
			q->next = user_paths;
			user_paths = q;
			return q->path;
		} else {
			FreeIfNeeded(q->name);
			FreeIfNeeded(q->path);
			free((char *)q);
		}
	}
	return NULL;
}

static	char *	find_user P(( char * ));
static char *
find_user(name)
char	*name;
{
	register struct	passwd *p;
	register UPATH	*q;

	if (name != NULL) {
		for (q = user_paths; q != NULL; q = q->next) {
			if (!strcmp(q->name, name)) {
				return q->path;
			}
		}

		/* not-found, do a lookup */
		if (*name != EOS)
			p = getpwnam(name);
		else {
			p = getpwuid((int)getuid());
			if (p == 0) {
				char *env = getenv("HOME");
				if (env != 0)
					return save_user(name, env);
			}
		}

		if (p != NULL)
			return save_user(name, p->pw_dir);
#if NEEDED
	} else {	/* lookup all users (for globbing) */
		(void)setpwent();
		while ((p = getpwent()) != NULL)
			(void)save_user(p->pw_name, p->pw_dir);
		(void)endpwent();
#endif
	}
	return NULL;
}

char *
home_path(path)
char	*path;
{
	if (*path == '~') {
		char	temp[NFILEN];
		char	*s, *d;

		/* parse out the user-name portion */
		for (s = path+1, d = temp; (*d = *s) != EOS; d++, s++) {
			if (is_slashc(*d)) {
				*d = EOS;
				s++;
				break;
			}
		}

#if OPT_VMS_PATH
		(void)mklower(temp);
#endif
		if ((d = find_user(temp)) != NULL)
			(void)pathcat(path, d, s);
	}
	return path;
}
#endif

#if USE_RESOLVE_DIR
/*
 * Some of this code was "borrowed" from the GNU C library (getcwd.c).  It has
 * been largely rewritten and bears little resemblance to what it started out
 * as.
 *
 * The purpose of this code is to generalize getcwd.  The idea is to pass it as
 * input some path name.  This pathname can be relative, absolute, whatever. 
 * It may have elements which reference symbolic links.  The output from this
 * function will be the absolute pathname representing the same file. 
 * Actually, it only returns the directory.  If the thing you pass it is a
 * directory, you'll get that directory back (canonicalized).  If you pass it a
 * path to an ordinary file, you'll get back the canonicalized directory which
 * contains that file.
 *
 * The way that this code works is similar to the classic implementation of
 * getcwd (or getwd).  The difference is that once it finds a directory, it
 * will cache it.  If that directory is referenced again, finding it will be
 * very fast.  The callee of this function should not free up the pointer which
 * is returned.  This will be done automatically by the caching code.  The
 * value returned will exist at least up until the next call It should not be
 * relied on any longer than this.  Care should be taken not to corrupt the
 * value returned.
 *
 * FIXME: there should be some way to reset the cache in case directories are
 * renamed.
 */

#define CPN_CACHE_SIZE 64
#define CPN_CACHE_MASK (CPN_CACHE_SIZE-1)

#if !defined(HAVE_SETITIMER) || !defined(HAVE_SIGACTION)
#define TimedStat stat
#else /* !defined(HAVE_SETITIMER) */

#define TimedStat stat_with_timeout

static	SIGT	StatHandler		P(( DEFINE_SIG_ARGS ));
static	int	stat_with_timeout	P(( const char *, struct stat * ));

static	jmp_buf stat_jmp_buf;		/* for setjmp/longjmp on timeout */

static SIGT
StatHandler(ACTUAL_SIG_ARGS)
{
	longjmp(stat_jmp_buf, signo);
	SIGRET;
}

static int
stat_with_timeout(path, statbuf)
	const char *path;
	struct stat *statbuf;
{
	struct sigaction newact;
	struct sigaction oldact;
	sigset_t newset;
	sigset_t oldset;
	struct itimerval timeout;
	struct itimerval oldtimerval;
	int retval, stat_errno;

	newact.sa_handler = StatHandler;
	newact.sa_flags = 0;
	if (sigemptyset(&newact.sa_mask) < 0
	 || sigfillset(&newset) < 0
	 || sigdelset(&newset, SIGPROF) < 0
	 || sigprocmask(SIG_BLOCK, &newset, &oldset) < 0)
		return -1;

	if (sigaction(SIGPROF, &newact, &oldact) < 0) {
		sigprocmask(SIG_SETMASK, &oldset, (sigset_t *)0);
		return -1;
	}

	timeout.it_interval.tv_sec  = 0;
	timeout.it_interval.tv_usec = 0;
	timeout.it_value.tv_sec     = 0;
	timeout.it_value.tv_usec    = 75000;

	(void)setitimer(ITIMER_PROF, &timeout, &oldtimerval);

	/*
	 * POSIX says that 'stat()' won't return an error if it's interrupted,
	 * so we force an error by making a longjmp from the timeout handler,
	 * and forcing the error return status.
	 */
	if (setjmp(stat_jmp_buf)) {
		retval = -1;
		stat_errno = EINTR;
	} else {
		retval = stat(path, statbuf);
		stat_errno = errno;
	}

	timeout.it_value.tv_usec = 0;
	(void)setitimer(ITIMER_PROF, &timeout, (struct itimerval *)0);

	(void)sigaction(SIGPROF, &oldact, (struct sigaction *)0);
	(void)sigprocmask(SIG_SETMASK, &oldset, (sigset_t *)0);
	(void)setitimer(ITIMER_PROF, &oldtimerval, (struct itimerval *)0);

	errno = stat_errno;
	return retval;
}
#endif /* !defined(HAVE_SETITIMER) */

static char *
resolve_directory(path_name, file_namep)
	char *path_name;
	char **file_namep;
{
	dev_t rootdev, thisdev;
	ino_t rootino, thisino;
	struct stat  st;

	static char   rootdir[] = { SLASHC, EOS };

	static TBUFF *last_leaf;
	static TBUFF *last_path;
	static TBUFF *last_temp;

	char         *temp_name;
	char         *tnp;	/* temp name pointer */
	char         *temp_path; /* the path that we've determined */

	ALLOC_T       len;	/* temporary for length computations */

	static struct cpn_cache {
		dev_t ce_dev;
		ino_t ce_ino;
		TBUFF *ce_dirname;
	} cache_entries[CPN_CACHE_SIZE];

	struct cpn_cache *cachep;

	tb_free(&last_leaf);
	*file_namep = NULL;
	len = strlen(path_name);

	if (!tb_alloc(&last_temp, len + 1))
		return NULL;
	tnp = (temp_name = tb_values(last_temp)) + len;

	if (!tb_alloc(&last_path, len + 1))
		return NULL;
	*(temp_path = tb_values(last_path)) = EOS;

	(void)strcpy(temp_name, path_name);

	/*
	 * Test if the given pathname is an actual directory, or not.  If it's
	 * a symbolic link, we'll have to determine if it points to a directory
	 * before deciding how to split it.
	 */
	if (lstat(temp_name, &st) < 0)
		st.st_mode = S_IFREG;	/* assume we're making a file... */

	if (!S_ISDIR(st.st_mode)) {
		int levels = 0;
		char target[NFILEN];

		/* loop until no more links */
		while ((st.st_mode & S_IFMT) == S_IFLNK) {
			int got;

			if (levels++ > 4	/* FIXME */
			 || (got = readlink(temp_name,
			 		target, sizeof(target)-1)) < 0) {
				return NULL;
			}
			target[got] = EOS;

			if (tb_alloc(&last_temp, strlen(temp_name)+got+1) == 0)
				return NULL;

			temp_name = tb_values(last_temp);

			if (!is_slashc(target[0])) {
				tnp = pathleaf(temp_name);
				if (tnp != temp_name && !is_slashc(tnp[-1]))
					*tnp++ = SLASHC;
				(void)strcpy(tnp, target);
			} else {
				(void)strcpy(temp_name, target);
			}
			if (lstat(temp_name, &st) < 0)
				break;
		}

		/*
		 * If we didn't resolve any symbolic links, we can find the
		 * filename leaf in the original 'path_name' argument.
		 */
		tnp = last_slash(temp_name);
		if (tnp == NULL) {
			tnp = temp_name;
			if (tb_scopy(&last_leaf, tnp) == 0)
				return NULL;
			*tnp++ = '.';
			*tnp = EOS;
		} else if (tb_scopy(&last_leaf, tnp + 1) == 0) {
			return NULL;
		}
		if (tnp == temp_name && is_slashc(*tnp)) /* initial slash */
			tnp++;
		*tnp = EOS;

		/*
		 * If the parent of the given path_name isn't a directory, give
		 * up...
		 */
		if (TimedStat(temp_name, &st) < 0 || !S_ISDIR(st.st_mode))
			return NULL;
	}

	/*
	 * Now, 'temp_name[]' contains a null-terminated directory-path, and
	 * 'tnp' points to the null.  If we've set file_namep, we've allocated
	 * a pointer since it may be pointing within the temp_name string --
	 * which may be overwritten. 
	 */
	*file_namep = tb_values(last_leaf);

	thisdev = st.st_dev;
	thisino = st.st_ino;

	cachep =  &cache_entries[(thisdev ^ thisino) & CPN_CACHE_MASK];
	if (tb_values(cachep->ce_dirname) != 0
	 && cachep->ce_ino == thisino
	 && cachep->ce_dev == thisdev) {
		return tb_values(cachep->ce_dirname);
	} else {
		cachep->ce_ino = thisino;
		cachep->ce_dev = thisdev;
		tb_free(&(cachep->ce_dirname));	/* will reset iff ok */
	}

	if (TimedStat(rootdir, &st) < 0)
		return NULL;

	rootdev = st.st_dev;
	rootino = st.st_ino;

	while ((thisdev != rootdev)
	   ||  (thisino != rootino)) {
		register DIR *dp;
		register DIRENT *de;
		dev_t dotdev;
		ino_t dotino;
		char  mount_point;
		SIZE_T namelen = 0;

		len = tnp - temp_name;
		if (tb_alloc(&last_temp, 4 + len) == 0)
			return NULL;

		tnp = (temp_name = tb_values(last_temp)) + len;
		*tnp++ = SLASHC;
		*tnp++ = '.';
		*tnp++ = '.';
		*tnp   = EOS;

		/* Figure out if this directory is a mount point.  */
		if (TimedStat(temp_name, &st) < 0)
			return NULL;

		dotdev = st.st_dev;
		dotino = st.st_ino;
		mount_point = (dotdev != thisdev);

		/* Search for the last directory.  */
		if ((dp = opendir(temp_name)) != 0) {
			int	found = FALSE;

			while ((de = readdir(dp)) != NULL) {
#if USE_D_NAMLEN
				namelen = de->d_namlen;
#else
				namelen = strlen(de->d_name);
#endif
				/* Ignore "." and ".." */
				if (de->d_name[0] == '.'
				 && (namelen == 1
				  || (namelen == 2 && de->d_name[1] == '.')))
					continue;

				if (mount_point || de->d_ino == thisino) {
					len = tnp - temp_name;
					if (tb_alloc(&last_temp, len + namelen + 1) == 0)
						break;

					temp_name = tb_values(last_temp);
					tnp = temp_name + len;

					*tnp = SLASHC;
					(void)strncpy(tnp+1, de->d_name, namelen);
					tnp[namelen+1] = EOS;

					if (TimedStat(temp_name, &st) == 0
					 && st.st_dev == thisdev
					 && st.st_ino == thisino) {
						found = TRUE;
						break;
					}
				}
			}

			if (found) {
				/*
				 * Push the latest directory-leaf before the
				 * string already in 'temp_path[]'.
				 */
				len = strlen(temp_path) + 1;
				if (tb_alloc(&last_path, len + namelen + 1) == 0) {
					(void) closedir(dp);
					return NULL;
				}
				temp_path = tb_values(last_path);
				while (len-- != 0)
					temp_path[namelen+1+len] = temp_path[len];
				temp_path[0] = SLASHC;
				memcpy(temp_path+1, de->d_name, namelen);
			}
			(void) closedir(dp);
			if (!found)
				return NULL;
		}

		thisdev = dotdev;
		thisino = dotino;
	}

	if (tb_scopy(&(cachep->ce_dirname),
		*temp_path ? temp_path : rootdir) == 0)
		return NULL;

	return tb_values(cachep->ce_dirname);
}
#endif	/* USE_RESOLVE_DIR */

#if OPT_CASELESS

/*
 * The function case_correct_path is intended to determine the true
 * case of all pathname components of a syntactically canonicalized
 * pathname for operating systems on which OPT_CASELESS applies. 
 */

#if SYS_WINNT
static int
case_correct_path(old_file, new_file)
char *old_file;
char *new_file;
{
	WIN32_FIND_DATA fd;
	HANDLE h;
	int len;
	char *next, *current, *end, *sofar;
	char tmp_file[MAX_PATH];

	/* Handle old_file == new_file safely. */
	(void)strcpy(tmp_file, old_file);
	old_file = tmp_file;

	if (is_slashc(old_file[0]) && is_slashc(old_file[1])) {

		/* Handle UNC filenames. */
		current = old_file + 2;
		next = strchr(current, SLASHC);
		if (next)
			next = strchr(next + 1, SLASHC);

		/* Canonicalize the system name and share name. */
		if (next)
			len = next - old_file + 1;
		else
			len = strlen(old_file);
		(void)memcpy(new_file, old_file, len);
		new_file[len] = '\0';
		(void)mklower(new_file);
		if (!next)
			return 0;
		sofar = new_file + len;
		current = next + 1;
	}
	else {

		/* Canonicalize a leading drive letter, if any. */
		if (old_file[0] && old_file[1] == ':') {
			new_file[0] = old_file[0];
			new_file[1] = old_file[1];
			if (islower(new_file[0]))
				new_file[0] = toupper(new_file[0]);
			current = old_file + 2;
			sofar = new_file + 2;
		}
		else {
			current = old_file;
			sofar = new_file;
		}

		/* Skip a leading slash, if any. */
		if (is_slashc(*current)) {
			current++;
			*sofar++ = SLASHC;
		}
	}

	/* Canonicalize each pathname prefix. */
	end = old_file + strlen(old_file);
	while (current < end) {
		next = strchr(current, SLASHC);
		if (!next)
			next = end;
		len = next - current;
		(void)memcpy(sofar, current, len);
		sofar[len] = '\0';
		h = FindFirstFile(new_file, &fd);
		if (h != INVALID_HANDLE_VALUE) {
			FindClose(h);
			(void)strcpy(sofar, fd.cFileName);
			sofar += strlen(sofar);
		}
		else
			sofar += len;
		if (next != end)
			*sofar++ = SLASHC;
		current = next + 1;
	}
	return 0;
}

#else /* !SYS_WINNT */

static int
case_correct_path(old_file, new_file)
char *old_file;
char *new_file;
{
	if (old_file != new_file)
		(void)strcpy(new_file, old_file);
	return 0;
}

#endif /* !SYS_WINNT */

#endif /* OPT_CASELESS */

/* canonicalize a pathname, to eliminate extraneous /./, /../, and ////
	sequences.  only guaranteed to work for absolute pathnames */
static char *
canonpath(ss)
char *ss;
{
#if !USE_RESOLVE_DIR
	char *p, *pp;
#endif
	char *s;

	TRACE(("canonpath '%s'\n", ss))
	if ((s = is_appendname(ss)) != 0)
		return (canonpath(s) != 0) ? ss : 0;

	s = ss;

	if (!*s)
		return s;

#if OPT_MSDOS_PATH
#if !OPT_CASELESS
	(void)mklower(ss);	/* MS-DOS is case-independent */
#endif
	if (is_slashc(*ss))
		*ss = SLASHC;
	/* pretend the drive designator isn't there */
	if ((s = is_msdos_drive(ss)) == 0)
		s = ss;
#endif

#if SYS_UNIX
	(void)home_path(s);
#endif

#if OPT_VMS_PATH
	/*
	 * If the code in 'lengthen_path()', as well as the scattered calls on
	 * 'fgetname()' are correct, the path given to this procedure should
	 * be a fully-resolved VMS pathname.  The logic in filec.c will allow a
	 * unix-style name, so we'll fall-thru if we find one.
	 */
	if (is_vms_pathname(s, -TRUE)) {
		return mkupper(ss);
	}
#endif

#if SYS_UNIX || OPT_MSDOS_PATH || OPT_VMS_PATH
	if (!is_slashc(*s)) {
		mlforce("BUG: canonpath '%s'", s);
		return ss;
	}
	*s = SLASHC;

	/*
	 * If the system supports symbolic links (most UNIX systems do), we
	 * cannot do dead reckoning to resolve the pathname.
	 */
#if USE_RESOLVE_DIR
	{
		char temp[NFILEN];
		char *leaf;
		char *head = resolve_directory(s, &leaf);
		if (head != 0) {
			if (leaf != 0)
				(void)strcpy(s, pathcat(temp, head, leaf));
			else
				(void)strcpy(s, head);
		}
	}

#else	/* !USE_RESOLVE_DIR */
	p = pp = s;

#if SYS_APOLLO
	if (!is_slashc(p[1])) {	/* could be something like "/usr" */
		char	*cwd = current_directory(FALSE);
		char	temp[NFILEN];
		if (!strncmp(cwd, "//", 2)
		 && strlen(cwd) > 2
		 && (p = strchr(cwd+2, '/')) != 0) {
			(void)strcpy(strcpy(temp, cwd) + (p+1-cwd), s);
			(void)strcpy(s, temp);
		}
	}
	p = s + 1;	/* allow for leading "//" */
#endif

	p++; pp++;	/* leave the leading slash */
	while (*pp) {
		if (is_slashc(*pp)) {
			pp++;
			continue;
		}
		if (*pp == '.' && is_slashc(*(pp+1))) {
			pp += 2;
			continue;
		}
		break;
	}
	while (*pp) {
		if (is_slashc(*pp)) {
			while (is_slashc(*(pp+1)))
				pp++;
			if (p > s && !is_slashc(*(p-1)))
				*p++ = SLASHC;
			if (*(pp+1) == '.') {
				if (*(pp+2) == EOS) {
					/* change "/." at end to "" */
					*(p-1) = EOS;	/* and we're done */
					break;
				}
				if (is_slashc(*(pp+2))) {
					pp += 2;
					continue;
				} else if (*(pp+2) == '.' && (is_slashc(*(pp+3))
							|| *(pp+3) == EOS)) {
					while (p-1 > s && is_slashc(*(p-1)))
						p--;
					while (p > s && !is_slashc(*(p-1)))
						p--;
					if (p == s)
						*p++ = SLASHC;
					pp += 3;
					continue;
				}
			}
			pp++;
			continue;
		} else {
			*p++ = *pp++;
		}
	}
	if (p > s && is_slashc(*(p-1)))
		p--;
	if (p == s)
		*p++ = SLASHC;
	*p = EOS;
#endif	/* USE_RESOLVE_DIR */
#endif	/* SYS_UNIX || SYS_MSDOS */

#if OPT_VMS_PATH
	if (!is_vms_pathname(ss, -TRUE)) {
		char *tt = ss + strlen(ss);

		/*
		 * If we're not looking at "/" or some other path that ends
		 * with a slash, see if we can match the path to a directory
		 * file.  If so, force a slash on the end so that the unix2vms
		 * conversion will show a directory.
		 */
		if (tt[-1] != SLASHC) {
			struct stat sb;
#if SYS_VMS
			(void)strcpy(tt, ".DIR");
#else
			(void)mklower(ss);
#endif
			if ((stat(SL_TO_BSL(ss), &sb) >= 0)
			 && ((sb.st_mode & S_IFMT) == S_IFDIR))
				(void)strcpy(tt, "/");
			else
				*tt = EOS;
		}

		/* FIXME: this is a hack to prevent this function from
		 * returning device-level strings, since (at the moment) I
		 * don't have anything that returns a list of the mounted
		 * devices on a VMS system.
		 */
		if (!strcmp(ss, "/")) {
			(void)strcpy(ss, current_directory(FALSE));
			if ((tt = strchr(ss, ':')) != 0)
				(void)strcpy(tt+1, "[000000]");
			else
				(void)strcat(ss, ":");
			(void)mkupper(ss);
		} else {
			unix2vms_path(ss, ss);
		}
	}
#endif

#if OPT_CASELESS
	case_correct_path(ss, ss);
#endif

	TRACE((" -> '%s' canonpath\n", ss))
	return ss;
}

char *
shorten_path(path, keep_cwd)
char *path;
int keep_cwd;
{
	char	temp[NFILEN];
	char *cwd;
	char *ff;
	char *slp;
	char *f;
#if OPT_VMS_PATH
	char *dot;
#endif

	if (!path || *path == EOS)
		return path;

	if (isInternalName(path))
		return path;

	TRACE(("shorten '%s'\n", path))
	if ((f = is_appendname(path)) != 0)
		return (shorten_path(f, keep_cwd) != 0) ? path : 0;

#if OPT_VMS_PATH
	/*
	 * This assumes that 'path' is in canonical form.
	 */
	cwd = current_directory(FALSE);
	ff  = path;
	dot = 0;
	TRACE(("current '%s'\n", cwd))

	if ((slp = strchr(cwd, '[')) != 0
	 && (slp == cwd
	  || !strncmp(cwd, path, (SIZE_T)(slp-cwd)))) { /* same device? */
	  	ff += (slp-cwd);
		cwd = slp;
		(void)strcpy(temp, "[");	/* hoping for relative-path */
		while (*cwd && *ff) {
			if (*cwd != *ff) {
				if (*cwd == ']' && *ff == '.') {
					/* "[.DIRNAME]FILENAME.TYP;1" */
					;
				} else if (*cwd == '.' && *ff == ']') {
					/* "[-]FILENAME.TYP;1" */
					while (*cwd != EOS) {
						if (*cwd++ == '.')
							(void)strcat(temp, "-");
					}
					(void)strcat(temp, "]");
					ff++;
				} else if (dot != 0) {
					int diff = (ff - dot);

					/* "[-.DIRNAME]FILENAME.TYP;1" */
					while (*cwd != EOS) {
						if (*cwd++ == '.')
							(void)strcat(temp, "-");
					}
					while (dot != ff) {
						if (*dot++ == '.')
							(void)strcat(temp, "-");
					}
					(void)strcat(temp, ".");
					ff -= (diff - 1);
				}
				break;
			} else if (*cwd == ']') {
				(void)strcat(temp, cwd);
				ff++;	/* path-leaf, if any */
				break;
			}

			if (*ff == '.')
				dot = ff;
			cwd++;
			ff++;
		}
	} else {
		*temp = EOS;		/* different device, cannot relate */
	}

	if (!strcmp(temp, "[]")		/* "[]FILENAME.TYP;1" */
	 && !keep_cwd)
		*temp = EOS;

	(void) strcpy(path, strcat(temp, ff));
	TRACE(("     -> '%s' shorten\n", path))
#else
# if SYS_UNIX || OPT_MSDOS_PATH
	cwd = current_directory(FALSE);
	slp = ff = path;
	while (*cwd && *ff && *cwd == *ff) {
		if (is_slashc(*ff))
			slp = ff;
		cwd++;
		ff++;
	}

	/* if we reached the end of cwd, and we're at a path boundary,
		then the file must be under '.' */
	if (*cwd == EOS) {
		if (keep_cwd) {
			temp[0] = '.';
			temp[1] = SLASHC;
			temp[2] = EOS;
		} else
			*temp = EOS;
		if (is_slashc(*ff))
			return strcpy(path, strcat(temp, ff+1));
		if (slp == ff - 1)
			return strcpy(path, strcat(temp, ff));
	}

	/* if we mismatched during the first path component, we're done */
	if (slp == path)
		return path;

	/* if we mismatched in the last component of cwd, then the file
		is under '..' */
	if (last_slash(cwd) == 0)
		return strcpy(path, strcat(strcpy(temp, ".."), slp));

	/* we're off by more than just '..', so use absolute path */
# endif	/* SYS_UNIX || SYS_MSDOS */
#endif	/* OPT_VMS_PATH */

	return path;
}

#if OPT_VMS_PATH
static int mixed_case P((char *));

static int
mixed_case(path)
char *path;
{
	register int c;
	int	had_upper = FALSE;
	int	had_lower = FALSE;
	while ((c = *path++) != EOS) {
		if (islower(c))	had_lower = TRUE;
		if (isupper(c))	had_upper = TRUE;
	}
	return (had_upper && had_lower);
}
#endif

/*
 * Undo nominal effect of 'shorten_path()'
 */
char *
lengthen_path(path)
char *path;
{
#if SYS_VMS
	struct	FAB	my_fab;
	struct	NAM	my_nam;
	char		my_esa[NAM$C_MAXRSS];	/* expanded: sys$parse */
	char		my_rsa[NAM$C_MAXRSS];	/* result: sys$search */
#endif
	register int len;
	char	*cwd;
	char	*f;
	char	temp[NFILEN];
#if OPT_MSDOS_PATH
	char	drive;
#endif

	if ((f = is_appendname(path)) != 0)
		return (lengthen_path(f) != 0) ? path : 0;

	if ((f = path) == 0)
		return path;

	if (*path != EOS && isInternalName(path)) {
#if OPT_VMS_PATH
	    /*
	     * The conflict between VMS pathnames (e.g., "[-]") and Vile's
	     * scratch-buffer names is a little ambiguous.  On VMS, though,
	     * we'll have to give VMS pathnames the edge.  We cheat a little,
	     * by exploiting the fact (?) that the system calls return paths
	     * in uppercase only.
	     */
	    if (!is_vms_pathname(path, TRUE) && !mixed_case(path))
#endif
		return path;
	}

#if SYS_UNIX
	(void)home_path(f);
#endif

#if OPT_VMS_PATH2_patch
	if (!is_vms_pathname(path, -TRUE)) {
		unix2vms_path(path, path);
	}
#endif
#if SYS_VMS
	/*
	 * If the file exists, we can ask VMS to tell the full pathname.
	 */
	if ((*path != EOS) && maybe_pathname(path)) {
		int	fd;
		long	status;
		char	temp[NFILEN],
			leaf[NFILEN];
		register char	*s;

		if (!strchr(path, '*') && !strchr(path, '?')) {
			if ((fd = open(SL_TO_BSL(path), O_RDONLY, 0)) >= 0) {
				getname(fd, temp);
				(void)close(fd);
				return strcpy(path, temp);
			}
		}

		/*
		 * Path either contains a wildcard, or the file does
		 * not already exist.  Use the system parser to expand
		 * the pathname components.
		 */
		my_fab = cc$rms_fab;
		my_fab.fab$l_fop = FAB$M_NAM;
		my_fab.fab$l_nam = &my_nam;	/* FAB => NAM block	*/
		my_fab.fab$l_dna = "";		/* Default-selection	*/
		my_fab.fab$b_dns = strlen(my_fab.fab$l_dna);

		my_fab.fab$l_fna = path;
		my_fab.fab$b_fns = strlen(path);

		my_nam = cc$rms_nam;
		my_nam.nam$b_ess = NAM$C_MAXRSS;
		my_nam.nam$l_esa = my_esa;
		my_nam.nam$b_rss = NAM$C_MAXRSS;
		my_nam.nam$l_rsa = my_rsa;

		if ((status = sys$parse(&my_fab)) == RMS$_NORMAL) {
			my_esa[my_nam.nam$b_esl] = EOS;
			return strcpy(path, my_esa);
		} else {
			/* FIXME: try to expand partial directory specs, etc. */
		}
	}
#else
# if OPT_VMS_PATH
	/* this is only for testing! */
	if (fakevms_filename(path))
		return path;
# endif
#endif

#if SYS_UNIX || OPT_MSDOS_PATH || OPT_VMS_PATH
#if OPT_MSDOS_PATH
	if ((f = is_msdos_drive(path)) != 0)
		drive = *path;
	else {
		drive = EOS;
		f = path;
	}
#endif
	if (!is_slashc(f[0])) {
#if SYS_MSDOS || SYS_WIN31 || SYS_OS2
		cwd = curr_dir_on_drive(drive!=EOS?drive:curdrive());
#else
		cwd = current_directory(FALSE);
#endif
		(void)strcpy(temp, cwd);
#if OPT_VMS_PATH
		vms2unix_path(temp, cwd);
#else
		(void)strcpy(temp, cwd);
#endif
		len = strlen(temp);
		temp[len++] = SLASHC;
		(void)strcpy(temp + len, f);
		(void)strcpy(path, temp);
	}
#if OPT_MSDOS_PATH
	if (is_msdos_drive(path) == 0) { /* ensure that we have drive too */
		temp[0] = curdrive();
		temp[1] = ':';
		(void)strcpy(temp+2, path);
		(void)strcpy(path, temp);
	}
#endif
#endif	/* SYS_UNIX || SYS_MSDOS */

	return canonpath(path);
}

/*
 * Returns true if the argument looks more like a pathname than anything else.
 *
 * Notes:
 *	This makes a syntax-only test (e.g., at the beginning of the string).
 *	VMS can accept UNIX-style /-delimited pathnames.
 */
int
is_pathname(path)
char *path;
{
	char	*f;

	if ((f = is_appendname(path)) != 0)
		return is_pathname(f);

#if OPT_VMS_PATH
	if (is_vms_pathname(path, -TRUE))
		return TRUE;
#endif

#if SYS_UNIX || OPT_MSDOS_PATH || SYS_VMS
	if ((f = path) != 0) {
#if SYS_UNIX
		if (f[0] == '~')
			return TRUE;
#endif
		if (is_slashc(f[0]))
			return TRUE;
		else if (*f++ == '.') {
			if (*f == '.')
				f++;
			if (is_slashc(f[0]))
				return TRUE;
		}
	}
#endif	/* SYS_UNIX || OPT_MSDOS_PATH || SYS_VMS */

	return FALSE;
}

/*
 * A bit weaker than 'is_pathname()', checks to see if the string contains
 * path delimiters.
 */
int
maybe_pathname(fn)
char *fn;
{
	if (is_pathname(fn))	/* test the obvious stuff */
		return TRUE;
#if OPT_MSDOS_PATH
	if (is_msdos_drive(fn))
		return TRUE;
#endif
	if (last_slash(fn) != 0)
		return TRUE;
#if OPT_VMS_PATH
	while (*fn != EOS) {
		if (ispath(*fn) && !isident(*fn))
			return TRUE;
		fn++;
	}
#endif
	return FALSE;
}

/*
 * Returns the filename portion if the argument is an append-name (and not an
 * internal name!), otherwise null.
 */
char *
is_appendname(fn)
char *fn;
{
	if (fn != 0) {
		if (isAppendToName(fn)) {
			fn += 2;	/* skip the ">>" prefix */
			while (isspace(*fn))
				fn++;
			if (!isInternalName(fn))
				return fn;
		}
	}
	return 0;
}

/*
 * Returns true if the filename is either a scratch-name, or is the string that
 * we generate for the filename-field of [Help] and [Buffer List].  Use this
 * function rather than simple tests of '[' to make tests for VMS filenames
 * unambiguous.
 */
int
is_internalname(fn)
char *fn;
{
#if OPT_VMS_PATH
	if (is_vms_pathname(fn, FALSE))
		return FALSE;
#endif
	if (!strcmp(fn, non_filename()))
		return TRUE;
	return (*fn == EOS) || ((*fn == SCRTCH_LEFT[0]) && (fn[strlen(fn)-1] == SCRTCH_RIGHT[0]));
}

/*
 * Test if the given path is a directory
 */
int
is_directory(path)
char *	path;
{
#if OPT_VMS_PATH
	register char *s;
#endif
	struct	stat	sb;

	if (path == NULL || *path == EOS)
		return FALSE;

#if OPT_VMS_PATH
	if (is_vms_pathname(path, TRUE)) {
		return TRUE;
	}

	/* If the name doesn't look like a directory, there's no point in
	 * wasting time doing a 'stat()' call.
	 */
	s = vms_pathleaf(path);
	if ((s = strchr(s, '.')) != 0) {
		char	ftype[NFILEN];
		(void)mkupper(strcpy(ftype, s));
		if (strcmp(ftype, ".DIR")
		 && strcmp(ftype, ".DIR;1"))
			return FALSE;
	}
#endif
#if OPT_UNC_PATH
	/*
	 * WARNING: kludge alert!
	 *
	 * The problem here is that \\system\share, if it exists,
	 * must be a directory.  However, due to a bug in the win32
	 * stat function, it may be reported to exist (stat succeeds)
	 * but that it is a file, not a directory.  So we special case
	 * a stand-alone \\system\share name and force it to be reported
	 * as a dir.
	 */
	if (is_slashc(path[0]) && is_slashc(path[1])) {
		char *end = path + strlen(path);
		int slashes = 0;
		if (end > path && is_slashc(end[-1]))
			end--;
		while (--end >= path) {
			if (is_slashc(*end))
				slashes++;
		}
		if (slashes == 3)
			return 1;
	}
#endif
	return ( (stat(SL_TO_BSL(path), &sb) >= 0)
#if SYS_OS2 && CC_CSETPP
		&& ((sb.st_mode & S_IFDIR) != 0)
#else
		&& ((sb.st_mode & S_IFMT) == S_IFDIR)
#endif
	  );

}

#if (SYS_UNIX||SYS_VMS||OPT_MSDOS_PATH) && OPT_PATHLOOKUP
/*
 * Parse the next entry in a list of pathnames, returning null only when no
 * more entries can be parsed.
 */
char *
parse_pathlist(list, result)
char *list;
char *result;
{
	if (list != NULL && *list != EOS) {
		register int	len = 0;

		while (*list && (*list != PATHCHR)) {
			if (len < NFILEN-1)
				result[len++] = *list;
			list++;
		}
		if (len == 0)	/* avoid returning an empty-string */
			result[len++] = '.';
		result[len] = EOS;

		if (*list == PATHCHR)
			++list;
	} else
		list = NULL;
	return list;
}
#endif	/* OPT_PATHLOOKUP */

#if SYS_WINNT && !CC_TURBO
/********                                               \\  opendir  //
 *                                                        ===========
 * opendir
 *
 * Description:
 *      Prepares to scan the file name entries in a directory.
 *
 * Arguments:   filename in NT format
 *
 * Returns:     pointer to a (malloc-ed) DIR structure.
 *
 * Joseph E. Greer      July 22 1992
 *
 ********/

DIR *
opendir(fname)
char * fname;
{
	char buf[MAX_PATH];
	DIR *od;

	(void)strcpy(buf, fname);

	if (!strcmp(buf, ".")) /* if its just a '.', replace with '*.*' */
		(void)strcpy(buf, "*.*");
	else {
		/* If the name ends with a slash, append '*.*' otherwise '\*.*' */
		if (is_slashc(buf[strlen(buf)-1]))
			(void)strcat(buf, "*.*");
		else
			(void)strcat(buf, "\\*.*");
	}

	/* allocate the structure to maintain currency */
	if ((od = typealloc(DIR)) == NULL)
		return NULL;

	/* Let's try to find a file matching the given name */
	if ((od->hFindFile = FindFirstFile(buf, &od->ffd))
	    == INVALID_HANDLE_VALUE) {
		free(od);
		return NULL;
	}
	od->first = 1;
	return od;
}

/********                                               \\  readdir  //
 *                                                        ===========
 * readdir
 *
 * Description:
 *      Read a directory entry.
 *
 * Arguments:   a DIR pointer
 *
 * Returns:     A struct direct
 *
 * Joseph E. Greer      July 22 1992
 *
 ********/
DIRENT *
readdir(dirp)
DIR *dirp;
{
	if (dirp->first)
		dirp->first = 0;
	else if (!FindNextFile(dirp->hFindFile, &dirp->ffd))
		return NULL;
	dirp->de.d_name = dirp->ffd.cFileName;
	return &dirp->de;
}

/********                                               \\  closedir  //
 *                                                        ===========
 * closedir
 *
 * Description:
 *      Close a directory entry.
 *
 * Arguments:   a DIR pointer
 *
 * Returns:     A struct direct
 *
 * Joseph E. Greer      July 22 1992
 *
 ********/
int
closedir(dirp)
DIR *dirp;
{
	FindClose(dirp->hFindFile);
	free(dirp);
	return 0;
}

#endif /* SYS_WINNT */

#if OPT_MSDOS_PATH
static char *slconv P(( char * , char *, char, char ));
static char slconvpath[NFILEN * 2];

/*
 * Use this function to filter our internal '/' format pathnames to '\'
 * when invoking system calls (e.g., opendir, chdir).
 */
char * 
sl_to_bsl(char *p)
{
	size_t len;
	char *s = slconv(p, slconvpath, '/', '\\');
	if ((s = is_msdos_drive(s)) == 0)
		s = slconvpath;
	/* Trim trailing slash if it's not the first */
	if ((len = strlen(s)) > 1
	 && is_slashc(s[len-1]))
		s[--len] = EOS;
	return slconvpath;
}

/*
 * Use this function to tidy up and put the path-slashes into internal form.
 */
void
bsl_to_sl_inplace(char *p)
{
	(void)slconv(p, p, '\\', '/');
}

static char *
slconv(char *f, char *t, char oc, char nc)
{
	char *retp = t;
	while (*f) {
		if (*f == oc)
			*t = nc;
		else
			*t = *f;
		f++;
		t++;
	}
	*t-- = '\0';

	return retp;
}
#endif

#if NO_LEAKS
void
path_leaks()
{
#if SYS_UNIX
	while (user_paths != NULL) {
		register UPATH *paths = user_paths;
		user_paths = paths->next;
		free(paths->name);
		free(paths->path);
		free((char *)paths);
	}
#endif
}
#endif	/* NO_LEAKS */
