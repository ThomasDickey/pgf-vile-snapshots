/*	PATH.C	
 *		The routines in this file handle the conversion of pathname
 *		strings.
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/path.c,v 1.31 1994/10/03 13:24:35 pgf Exp $
 *
 *
 */

#include	"estruct.h"
#include        "edef.h"

#if UNIX
#include <sys/types.h>
#include <pwd.h>
#endif

#if VMS
#include <file.h>
#endif

#include <sys/stat.h>

#include "dirstuff.h"

#if WIN31 && TURBO
# include <direct.h>
# define curdrive() _getdrive()
# define curr_dir_on_drive(d) _getdcwd(d, temp, sizeof(temp))
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
	if (isalpha(*path) && path[1] == ':') {
		(void)mklower(path);	/* MS-DOS isn't case-sensitive */
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

#ifdef __WATCOMC__

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
#endif /* __WATCOMC__ */


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
	char	temp[NFILEN];
	register char	*s = dst;

	if (path == 0 || *path == EOS)
		return strcpy(dst, leaf);

	leaf = strcpy(temp, leaf);		/* in case leaf is in dst */

	if (s != path)
		(void)strcpy(s, path);
	s += strlen(s) - 1;

#if OPT_VMS_PATH
	if (!is_vms_pathname(dst, TRUE))	/* could be DecShell */
#endif
	 if (!is_slashc(*s)) {
		*(++s) = SLASHC;
	 }

	(void)strcpy(s+1, leaf);
	return dst;
}

/*
 * Tests to see if the string contains a slash-delimiter.  If so, return the
 * last one (so we can locate the path-leak).
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
#if UNIX
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
		else
			p = getpwuid((int)getuid());

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

/* canonicalize a pathname, to eliminate extraneous /./, /../, and ////
	sequences.  only guaranteed to work for absolute pathnames */
char *
canonpath(ss)
char *ss;
{
	char *p, *pp;
	char *s;

	TRACE(("canonpath '%s'\n", ss))
	if ((s = is_appendname(ss)) != 0)
		return (canonpath(s) != 0) ? ss : 0;

	s = ss;

	if (!*s)
		return s;

#if OPT_MSDOS_PATH
	(void)mklower(ss);	/* MS-DOS is case-independent */
	/* pretend the drive designator isn't there */
	if ((s = is_msdos_drive(ss)) == 0)
		s = ss;
#endif

#if UNIX
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

#if UNIX || OPT_MSDOS_PATH || OPT_VMS_PATH
	p = pp = s;
	if (!is_slashc(*s)) {
		mlforce("BUG: canonpath '%s'", s);
		return ss;
	}

#if APOLLO
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
		switch (*pp) {
		case '/':
#if OPT_MSDOS_PATH
		case '\\':
#endif
			pp++;
			continue;
		case '.':
			if (is_slashc(*(pp+1))) {
				pp += 2;
				continue;
			}
		default:
			break;
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
#endif	/* UNIX || MSDOS */

#if OPT_VMS_PATH
	if (!is_vms_pathname(ss, -TRUE)) {
		char *tt = ss + strlen(ss);
		struct stat sb;
#if VMS
		(void)strcpy(tt, ".DIR");
#else
		(void)mklower(ss);
#endif
		if ((stat(ss, &sb) >= 0)
		 && ((sb.st_mode & S_IFMT) == S_IFDIR))
			(void)strcpy(tt, "/");
		else
			*tt = EOS;

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
		return NULL;

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
					/* "[-.DIRNAME]FILENAME.TYP;1" */
					while (*cwd != EOS) {
						if (*cwd++ == '.')
							(void)strcat(temp, "-");
					}
					if (dot != ff) {
						while (dot != ff) {
							if (*dot++ == '.')
								(void)strcat(temp, "-");
						}
						(void)strcat(temp, ".");
					}
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
# if UNIX || OPT_MSDOS_PATH
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
# endif	/* UNIX || MSDOS */
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
#if VMS
	struct	FAB	my_fab;
	struct	NAM	my_nam;
	char		my_esa[NAM$C_MAXRSS];	/* expanded: SYS$PARSE */
	char		my_rsa[NAM$C_MAXRSS];	/* result: SYS$SEARCH */
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

#if UNIX
	(void)home_path(f);
#endif

#if OPT_VMS_PATH2_patch
	if (!is_vms_pathname(path, -TRUE)) {
		unix2vms_path(path, path);
	}
#endif
#if VMS
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
			if ((fd = open(path, O_RDONLY, 0)) >= 0) {
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

#if UNIX || OPT_MSDOS_PATH || OPT_VMS_PATH
#if OPT_MSDOS_PATH
	if ((f = is_msdos_drive(path)) != 0)
		drive = *path;
	else {
		drive = EOS;
		f = path;
	}
#endif
	if (!is_slashc(f[0])) {
#if MSDOS || WIN31 || OS2
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
#if DJGPP
		temp[0] = SLASHC;  /* DJGCC returns '/', we may want '\' */
#endif
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
#endif	/* UNIX || MSDOS */

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

#if UNIX || OPT_MSDOS_PATH || VMS
	if ((f = path) != 0) {
#if UNIX
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
#endif	/* UNIX || OPT_MSDOS_PATH || VMS */

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
 * Returns the special string consisting of program name + version, used to
 * fill in the filename-field for scratch buffers that are not associated with
 * an external file.
 */
char *
non_filename()
{
	static	TBUFF	*ptr;
	if (!ptr) {
		char	buf[80];
		(void)lsprintf(buf, "       %s   %s",prognam,version);
		(void)tb_scopy(&ptr, buf);
	}
	return tb_values(ptr);
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
	return (*fn == EOS) || (*fn == SCRTCH_LEFT[0]);
}

/*
 * Test if the given path is a directory
 */
int
is_directory(path)
char *	path;
{
	struct	stat	sb;

#if OPT_VMS_PATH
	register char *s;
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
	return ((*path != EOS)
	  &&	(stat(path, &sb) >= 0)
	  &&	((sb.st_mode & S_IFMT) == S_IFDIR));
}

#if (UNIX||VMS||OPT_MSDOS_PATH) && PATHLOOK
/*
 * Parse the next entry in a list of pathnames, returning null only when no
 * more entries can be parsed.
 */
char *parse_pathlist(list, result)
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
#endif	/* PATHLOOK */

#if NT
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

DIR *opendir(fname)
char * fname;
{
	char buf[256];	/* FIXME: isn't there a MAXPATHLEN defined? */
	DIR *od;

	strcpy(buf, fname);

	if (!strcmp(buf, ".")) /* if its just a '.', replace with '*.*' */
		strcpy(buf, "*.*");
	else
		strcat(buf, "\\*.*");

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
DIRENT *readdir(dirp)
DIR *dirp;
{
	static	DIRENT *dep;

	if (!dirp->first) {
		if (!FindNextFile(dirp->hFindFile, &dirp->ffd))
			return NULL;
	}
	dirp->first = 0;

#if NO_LEAKS
	if (dep != 0) {
		free(dep->d_name);
	} else
#endif
	if ((dep = typealloc(DIRENT)) == NULL)
		return NULL;

	if ((dep->d_name = strmalloc(dirp->ffd.cFileName)) == NULL) {
		free(dep);
#if NO_LEAKS
		dep = 0;
#endif
		return NULL;
	}
	return dep;
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
int closedir(dirp)
DIR *dirp;
{
	FindClose(dirp->hFindFile);
	free(dirp);
	return 0;
}

#endif /* NT */

#if NO_LEAKS
void
path_leaks()
{
#if UNIX
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
