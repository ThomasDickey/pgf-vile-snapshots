/*	PATH.C	
 *		The routines in this file handle the conversion of pathname
 *		strings.
 *
 * $Log: path.c,v $
 * Revision 1.2  1993/03/16 16:04:01  pgf
 * fix 'parentheses suggested' warnings
 *
 * Revision 1.1  1993/03/16  10:53:21  pgf
 * see 3.36 section of CHANGES file
 *
 */

#include	"estruct.h"
#include        "edef.h"

#if UNIX
#include <pwd.h>
#endif

/*
 * If a pathname begins with "~", lookup the name in the password-file
 */
#if UNIX
static void
home_path(path)
char	*path;
{
	if (*path == '~') {
		char	temp[NFILEN];
		struct	passwd *p;
		char	*s, *d;

		/* parse out the user-name portion */
		for (s = path+1, d = temp; (*d = *s) != 0; d++, s++) {
			if (slashc(*d)) {
				*d = EOS;
				break;
			}
		}
		if (*temp != EOS)
			p = getpwnam(temp);
		else
			p = getpwuid((int)getuid());

		if (p != 0)
			(void)strcpy(path, strcat(strcpy(temp, p->pw_dir), s));
	}
}
#endif

/* use the shell to expand wildcards */
int
glob(buf)
char *buf;
{
#if UNIX
	char *cp;
	char cmd[NFILEN];
	FILE *cf;
	FILE *npopen();

	/* trim trailing whitespace */
	cp = &buf[strlen(buf)-1];
	while (cp != buf) {
		if (isspace(*cp))
			*cp = EOS;
		else
			break;
		cp--;
	}

	cp = buf;
	if (isInternalName(cp))		/* it's a shell command, or an */
		return TRUE;		/* internal name, don't bother */

	while (*cp) {
		if (iswild(*cp)) {
			(void)lsprintf(cmd, "echo %s", buf);
			cf = npopen(cmd,"r");
			if (cf == NULL) {
				return TRUE;
			}
			if (fread(buf,1,NFILEN,cf) <= 0) {
				npclose(cf);
				return FALSE;
			}
			npclose(cf);
			cp = buf;
			while (*cp) {
				if (*cp == ' ' ) {
					if (mlyesno(
					"Too many filenames.  Use first"
							) == TRUE) {
						*cp = EOS;
						break;
					} else {
						buf[0] = EOS;
						return FALSE;
					}
				} else if (*cp == '\n') {
					*cp = EOS;
					break;
				}
				cp++;
			}
			return TRUE;

		}
		cp++;
	}
#endif
#if MSDOS
	/* this is a just a convenient place to put this little hack */
	if (islower(buf[0]) && buf[1] == ':')
		buf[0] = toupper(buf[0]);
#endif
	return TRUE;
}

/* canonicalize a pathname, to eliminate extraneous /./, /../, and ////
	sequences.  only guaranteed to work for absolute pathnames */
char *
canonpath(ss)
char *ss;
{
	char *p, *pp;
	char *s;

	if ((s = is_appendname(ss)) != 0)
		return (canonpath(s) != 0) ? ss : 0;

	s = ss;

	if (!*s)
		return s;

#if MSDOS
	/* pretend the drive designator isn't there */
	if (isalpha(*s) && *(s+1) == ':')
		s += 2;
#endif

#if UNIX
	home_path(s);
#endif

	p = pp = s;
	if (!slashc(*s)) {
		mlforce("BUG: canonpath called with relative path");
		return ss;
	}
#if APOLLO
	if (!slashc(p[1])) {	/* could be something like "/usr" */
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
#if MSDOS
		case '\\':
#endif
			pp++;
			continue;
		case '.':
			if (slashc(*(pp+1))) {
				pp += 2;
				continue;
			}
		default:
			break;
		}
		break;
	}
	while (*pp) {
#if DEBUG
		if (pp != p)
			*p = EOS;
		printf(" s is %s\n",s);
		printf("pp is %*s%s\n",pp-s,"",pp);
#endif
		if (slashc(*pp)) {
			while (slashc(*(pp+1)))
				pp++;
			if (p > s && !slashc(*(p-1)))
				*p++ = slash;
			if (*(pp+1) == '.') {
				if (*(pp+2) == EOS) {
					/* change "/." at end to "" */
					*(p-1) = EOS;	/* and we're done */
					break;
				}
				if (slashc(*(pp+2))) {
					pp += 2;
					continue;
				} else if (*(pp+2) == '.' && (slashc(*(pp+3))
							|| *(pp+3) == EOS)) {
					while (p-1 > s && slashc(*(p-1)))
						p--;
					while (p > s && !slashc(*(p-1)))
						p--;
					if (p == s)
						*p++ = slash;
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
	if (p > s && slashc(*(p-1)))
		p--;
	if (p == s)
		*p++ = slash;
	*p = EOS;
	return ss;
}

char *
shorten_path(path)
char *path;
{
	char	temp[NFILEN];
	char *cwd;
	char *ff;
	char *slp;
	char *f;

	if (!path || *path == EOS)
		return NULL;

	if (isInternalName(path))
		return path;

	if ((f = is_appendname(path)) != 0)
		return (shorten_path(f) != 0) ? path : 0;

	cwd = current_directory(FALSE);
	slp = ff = path;
	while (*cwd && *ff && *cwd == *ff) {
		if (*ff == slash)
			slp = ff;
		cwd++;
		ff++;
	}

	/* if we reached the end of cwd, and we're at a path boundary,
		then the file must be under '.' */
	if (*cwd == EOS) {
		if (slashc(*ff))
			return strcpy(path, strcpy(temp, ff+1));
		if (slp == ff - 1)
			return strcpy(path, strcpy(temp, ff));
	}
	
	/* if we mismatched during the first path component, we're done */
	if (slp == path)
		return path;

	/* if we mismatched in the last component of cwd, then the file
		is under '..' */
	if (strchr(cwd,slash) == NULL)
		return strcpy(path, strcat(strcpy(temp, ".."), slp));

	/* we're off by more than just '..', so use absolute path */
	return path;
}

/*
 * Undo nominal effect of 'shorten_path()'
 */
char *
lengthen_path(path)
char *path;
{
	register int len;
	char	*f;
	char	temp[NFILEN];

	if ((f = is_appendname(path)) != 0)
		return (lengthen_path(f) != 0) ? path : 0;

	if ((f = path) == 0)
		return path;

#if UNIX
	home_path(f);
#endif
	if (!slashc(f[0])) {
		char *cwd;
#if MSDOS
		char drive = 0;
		if (isupper(f[0]) && f[1] == ':') {
			drive = *f;
			f += 2;
		}
		cwd = curr_dir_on_drive(drive);
#else
		cwd = current_directory(FALSE);
#endif
		len = strlen(strcpy(temp, cwd));
		temp[len++] = slash;
		(void)strcpy(temp + len, f);
		(void)strcpy(f, temp);
	}
	return canonpath(f);
}

/*
 * Returns true if the argument looks more like a pathname than anything else.
 */
int
is_pathname(path)
char *path;
{
	char	*f;

	if ((f = is_appendname(path)) != 0)
		return is_pathname(f);

	if ((f = path) != 0) {
#if UNIX
		if (f[0] == '~')
			return TRUE;
#endif
		if (slashc(f[0]))
			return TRUE;
		else if (*f++ == '.') {
			if (*f == '.')
				f++;
			if (slashc(f[0]))
				return TRUE;
		}
	}
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
