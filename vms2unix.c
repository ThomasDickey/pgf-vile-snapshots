/*
 *	vms2unix.c
 *
 *	Miscellaneous routines for UNIX/VMS compatibility.
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/vms2unix.c,v 1.18 1996/05/22 16:01:17 pgf Exp $
 *
 */
#include	"estruct.h"
#include	"edef.h"
#include	"dirstuff.h"

#define PERIOD    '.'
#define COLON     ':'
#define SEMICOLON ';'

static const char RootDir[] = "000000";
static const char DirType[] = ".DIR";

#if SYS_VMS
#include	<starlet.h>
#include	<unixio.h>

#define	zfab	dirp->dd_fab
#define	znam	dirp->dd_nam
#define	zrsa	dirp->dd_ret.d_name
#define	zrsl	dirp->dd_ret.d_namlen
#define	zesa	dirp->dd_esa

DIR *
opendir(char *filename)
{
	DIR	*dirp = typecalloc(DIR);
	long	status;

	if (dirp == 0)
		return (0);

	zfab = cc$rms_fab;
	zfab.fab$l_fop = FAB$M_NAM;
	zfab.fab$l_nam = &znam;		/* FAB => NAM block	*/
	zfab.fab$l_dna = "*.*;*";	/* Default-selection	*/
	zfab.fab$b_dns = strlen(zfab.fab$l_dna);

	zfab.fab$l_fna = filename;
	zfab.fab$b_fns = strlen(filename);

	znam = cc$rms_nam;
	znam.nam$b_ess = NAM$C_MAXRSS;
	znam.nam$l_esa = zesa;
	znam.nam$b_rss = NAM$C_MAXRSS;
	znam.nam$l_rsa = zrsa;

	if (sys$parse(&zfab) != RMS$_NORMAL) {
		(void)closedir(dirp);
		dirp = 0;
	}
	return (dirp);
}

DIRENT *
readdir(DIR *dirp)
{
	if (sys$search(&zfab) == RMS$_NORMAL) {
		zrsl = znam.nam$b_rsl;
		return (&(dirp->dd_ret));
	}
	return (0);
}

int
closedir(DIR *dirp)
{
	cfree(dirp);
	return 0;
}

char *
tempnam(const char *head, const char *tail)
{
	char	temp[NFILEN];
	char	leaf[NFILEN];
	return mktemp(
		strmalloc(
			pathcat(temp,
				head,
				strcat(strcpy(leaf, tail), "XXXXXX"))));
}
#endif

#if OPT_VMS_PATH
/*
 * These functions are adapted from my port2vms library -- T.Dickey
 */

/******************************************************************************
 * Translate a UNIX-style name into a VMS-style name.                         *
 ******************************************************************************/

static	int	DotPrefix (char *s);
static	char	CharToVms (int c);
static	int	leading_uc (char *dst, char *src);
static	int	is_version (char *s);

static	int	leaf_dot;   /* counts dots found in a particular leaf */
static	int	leaf_ver;   /* set if we found a DECshell version */

/*
 * If we have a dot in the second character position, force that to a dollar
 * sign.  Otherwise, keep the first dot in each unix leaf as a dot in the
 * resulting vms name.
 */
static int
DotPrefix(char * s)
{
	if (s[0] != EOS
	 && s[1] == PERIOD
	 && s[2] != EOS
	 && strchr("sp", s[0]))	/* hack for SCCS */
		return (-1);
	return (0);
}

static char
CharToVms(int c)
{
	if (c == PERIOD) {
		if (leaf_dot++)
			c = '$';
	} else if (!isalnum(c) && !strchr("_-", c)) {
		c = '$';
	}
	return (c);
}

static int
leading_uc(char * dst, char * src)
{
	char	*base = dst;
	register int	c;

	while ((c = *src) != EOS && c != SLASHC) {
		if (isalpha(c)) {
			if (islower(c))
				return (0);
		} else if (!strchr("0123456789$_", c))
			return (0);
		*dst++ = c;
		*dst   = EOS;
		src++;
	}
	*dst = EOS;
	if ((*base) && (dst = getenv(base)) != 0) {
		c = strlen(base);
		while (isspace(*dst))	dst++;
		(void)strcpy(base, dst);
		return (c);
	}
	return (0);
}

/*
 * Returns true if the string points to a valid VMS version indicator, which
 * may begin with a '.' or ';', e.g.,
 *	;1
 *	;*
 *	;-1
 */
static int
is_version(char *s)
{
	char 	*t;

	if (*s == PERIOD
	 || *s == SEMICOLON) {
		if (*++s == EOS
		 || !strcmp(s, "*")
		 || !strcmp(s, "0")
		 || (strtol(s, &t, 10) && *t == EOS))
		 	return TRUE;
	}
	return FALSE;
}

static char *
path_suffix(char *path)
{
	char *leaf = pathleaf(path);
	char *type = strchr(leaf, '.');
	if (type == 0)
		type = strend(leaf);
	return type;
}

static char *
path_version(char *path)
{
	char *vers = strchr(path, SEMICOLON);
	if (vers == 0)
		vers = strend(path);
	return vers;
}

char *
unix2vms_path(char *dst, const char *src)
{
	char	tmp[NFILEN],
		leading[NFILEN],
		*t,
		*s = strcpy(tmp, src),	/* ... to permit src == dst */
		*d = dst,
		c  = '?';
	int	bracket	= FALSE,	/* true when "[" passed. */
		on_top	= FALSE,	/* true when no "[." lead */
		node	= FALSE,	/* true when node found */
		device	= FALSE,	/* true when device found */
		len;

	/*
	 * If VMS 'getenv()' is given an upper-case name, it assumes that it
	 * corresponds to a logical device assignment.  As a special case, if
	 * we have a leading token of this form, translate it.
	 */
	if ((len = leading_uc(leading,s)) != 0) {
		s  += len;
		len = strlen(strcpy(d, leading));
		while (len > 1 && d[len-1] == ' ')
			len--;
		if (*s) {		/* text follows leading token */
			s++;		/* skip (assumed) SLASHC */
			if ((len > 1)
			&&  (d[len-1] == COLON)) {
				on_top = TRUE;
			} else if (strchr(s, SLASHC)) {	/* must do a splice */
				if ((len > 2)
				&&  (d[len-1] == RBRACK)) {
					bracket++;
					if (d[len-2] == PERIOD)
						/* rooted-device ? */
						len -= 2;
					else
						len--;
				}
			}
		}
		d[len] = EOS;
		if ((t = strchr(d, COLON)) != NULL) {
			if (t[1] == COLON) {
				node = TRUE;
				if ((t = strchr(t+2, COLON)) != NULL)
					device = TRUE;
			} else
				device = TRUE;
		}
		d  += len;
	} else if (*s == '~') {		/* process home-directory reference */
		node =
		device = TRUE;
		s++;

		len = strlen(strcpy(d, getenv("SYS$LOGIN")));

		if (d[len-1] == RBRACK) {
			bracket++;
			if (strcmp(s, "/")) { /* strip right-bracket to allow new levels */
				if (d[len-2] == PERIOD)
					len--;
				d[len-1] = PERIOD;
			} else {
				s++;
				len--;
			}
		}
		d += len;
	}

	/* look for node-name in VMS-format */
	if (!node
	&&  (t = strchr(s, '!')) != 0
	&&  (t[1] == SLASHC || t[1] == EOS)) {
		leaf_dot = DotPrefix(s);
		while (s < t)
			*d++ = CharToVms(*s++);
		*d++ = COLON;
		*d++ = COLON;
		s++;		/* skip over '!' */
	}

	/* look for device-name, indicated by a leading SLASHC */
	if (!device
	&&  (*s == SLASHC)) {
		leaf_dot = DotPrefix(++s);
		if ((t = strchr(s, SLASHC)) == 0)
			t = strend(s);
		else if (t[1] == EOS)
			on_top = TRUE;
		while (s < t)
			*d++ = CharToVms(*s++);
		if (d != dst)
			*d++ = COLON;
	}

	/* permit leading "./" to simplify cases in which we concatenate */
	if (!strncmp(s, "./", 2))
		s += 2;

	/* translate repeated leading "../" */
	while (!strncmp(s, "../", 3)) {
		s += 3;
		if (!bracket++)
			*d++ = LBRACK;
		*d++ = '-';
	}
	if (!strcmp(s, "..")) {
		s += 2;
		if (!bracket++)
			*d++ = LBRACK;
		*d++ = '-';
	}

	if (strchr(s, SLASHC)) {
		if (!bracket++)
			*d++ = LBRACK;
		if (*s == SLASHC) {
			s++;
		} else if (!on_top) {
			*d++ = PERIOD;
		}
		while ((c = *s++) != EOS) {
			if (c == PERIOD)
				c = '$';
			if (c == SLASHC) {
		    		leaf_dot = DotPrefix(s);
				if (strchr(s, SLASHC))
					*d++ = PERIOD;
				else {
					break;
				}
			} else {
				*d++ = CharToVms(c);
			}
		}
	}
	if (bracket) {
		if (on_top && d[-1] == LBRACK) {
			(void)strcpy(d, RootDir);
			d += strlen(d);
		}
		*d++ = RBRACK;
	}
	if (c != EOS && *s) {
		leaf_dot = DotPrefix(s);
		while ((c = *s) != EOS) {
			if ((leaf_ver = is_version(s)) == TRUE) {
				leaf_dot = TRUE; /* no longer pertinent */
				(void)strcpy(d, s);
				*d = SEMICOLON;	/* make this unambiguous */
				d += strlen(d);
				break;
			} else {
				*d++ = CharToVms(c);
			}
			s++;
		}
		if (!leaf_dot)
			*d++ = PERIOD;
		if (!leaf_ver)
			*d++ = SEMICOLON;
	}
	*d = EOS;
	return mkupper(dst);
}

/******************************************************************************
 * Returns a pointer to a pathname's suffix iff it is likely a directory's    *
 ******************************************************************************/
char *
is_vms_dirtype(char *path)
{
	register char *t = path_suffix(path);
	register char *v = path_version(t);
	size_t len = (v-t);

	if (len == sizeof(DirType)-1
	 && !strncmp(t, DirType, len)
	 && (!*v || !strcmp(v, ";1"))) {
		return t;
	}
	return 0;
}

/******************************************************************************
 * Returns a pointer to a pathname's leaf iff it is the root directory        *
 ******************************************************************************/
char *
is_vms_rootdir(char *path)
{
	char *type;
	if ((type = is_vms_dirtype(path)) != 0) {
		char *leaf = pathleaf(path);
		size_t len = (type - leaf);
		if (len == sizeof(RootDir)-1
		 && !strncmp(leaf, RootDir, len))
		 	return leaf;
	}
	return 0;
}

/******************************************************************************
 * Convert a VMS directory-filename into the corresponding pathname           *
 ******************************************************************************/
void
vms_dir2path(char *path)
{
	char *s;

	if ((s = is_vms_rootdir(path)) != 0) {
		*s = EOS;
	} else {
		if ((s = strrchr(path, RBRACK)) != 0
		 && (s[1] != EOS)) {
			char *t;
			if ((t = is_vms_dirtype(s)) != 0) {
				*s = '.';
				*t++ = RBRACK;
				*t = EOS;
			}
		}
	}
}

/******************************************************************************
 * Convert a VMS pathname into the name of the corresponding directory-file.  *
 *                                                                            *
 * Note that this returns a pointer to a static buffer which is overwritten   *
 * by each call.                                                              *
 ******************************************************************************/

char *
vms_path2dir(const char *src)
{
	static	char	buffer[NFILEN];
	register char	*s	= strend(strcpy(buffer, src));

	if (s != buffer && *(--s) == RBRACK) {
		(void)strcpy(s, DirType);
		while (--s >= buffer) {
			if (*s == PERIOD) {
				*s = RBRACK;
				if (s == buffer+1) {	/* absorb "]" */
					register char *t = s + 1;
					s = buffer;
					while ((*s++ = *t++) != EOS)
						/*EMPTY*/;
				}
				break;
			}
			if (*s == LBRACK) {		/* absorb "[" */
				register char *t = s + 1;
				if (is_vms_rootdir(t)
				 && (s == buffer || s[-1] == COLON)) {
					(void) lsprintf(t, "%s%c%s%s",
						RootDir,
						RBRACK,
						RootDir,
						DirType);
				} else {
					while ((*s++ = *t++) != EOS)
						/*EMPTY*/;
				}
				break;
			}
		}
	}
	return (buffer);
}

/******************************************************************************
 * Translate a VMS-style pathname to a UNIX-style pathname                    *
 ******************************************************************************/

char *
vms2unix_path(char *dst, const char *src)
{
	char	current[NFILEN];
	int	need_dev = FALSE,
		have_dev = FALSE;
	char	tmp[NFILEN],
		*output = dst,
		*base = tmp,
		*s = strcpy(tmp, src),	/* ... to permit src == dst */
		*d;

	if ((s = strchr(s, SEMICOLON)) != NULL)	/* trim off version */
		*s = EOS;

	/* look for node specification */
	if ((s = strchr(base, COLON)) != 0
	&&  (s[1] == COLON)) {
		while (base < s) {
			*dst++ = *base++;
		}
		*dst++ = '!';
		base += 2;
		need_dev = TRUE;
	}

	/*
	 * Look for device specification.  If not found, see if the path must
	 * begin at the top of the device.  In this case, it would be ambiguous
	 * if no device is supplied.
	 */
	if ((s = strchr(base, COLON)) != NULL) {
		*dst++ = SLASHC;
		while (base < s) {
			*dst++ = *base++;
		}
		base++;			/* skip over ":" */
		have_dev = TRUE;
	} else if (need_dev
	||	  ((base[0] == LBRACK)
	&&	   (base[1] != '-')
	&&	   (base[1] != PERIOD)
	&&	   (base[1] != RBRACK))) {	/* must supply a device */
		register char	*a = getcwd(current, NFILEN),
				*b = strchr(a ? a : "?", COLON);
		if ((b != 0)
		&&  (b[1] == COLON)) {	/* skip over node specification */
			a = b + 2;
			b = strchr(a, COLON);
		}
		if (b != 0) {
			*dst++ = SLASHC;	/* begin the device */
			while (a < b) {
				*dst++ = *a++;
			}
			have_dev = TRUE;
		}			/* else, no device in getcwd! */
	}

	/* translate directory-syntax */
	if ((s = strchr(base, LBRACK)) != NULL) {
		int bracketed = TRUE;
		if (s[1] == RBRACK) {
			if (dst != output && *dst != SLASHC)
				*dst++ = SLASHC;
			*dst++ = PERIOD;
			if (s[2] != EOS)
				*dst++ = SLASHC;
			s += 2;
			d = s;
			bracketed = FALSE;
		} else if (s[1] == PERIOD) {
			if (have_dev && dst[-1] != SLASHC)
				*dst++ = SLASHC;
			s += 2;
			d = s;
		} else if (s[1] == '-' && strchr("-.]", s[2])) {
			s++;
			while (*s == '-') {
				s++;
				*dst++ = PERIOD;
				*dst++ = PERIOD;
				if (*s == PERIOD
				 && (s[1] == '-' || s[1] == RBRACK))
					/* allow "-.-" */
					s++;
				if (*s == '-')
					*dst++ = SLASHC;
			}
			d = s;
		} else if (!strncmp(s+1, RootDir, sizeof(RootDir)-1)
		        && strchr(".]", s[sizeof(RootDir)])) {
			s += sizeof(RootDir);
			d = s;
		} else {
			d = s;
			*s++ = SLASHC;
		}
		/* expect s points to the last token before right-bracket */
		if (bracketed) {
			while (*s && *s != RBRACK) {
				if (*s == PERIOD)
					*s = SLASHC;
				s++;
			}
			if (*s)
				*s = s[1] ? SLASHC : EOS;
		}
	} else {
		if (have_dev && dst[-1] != SLASHC)
			*dst++ = SLASHC;
		d = base;
	}

	/*
	 * Copy the remainder of the string, trimming trailing "."
	 */
	for (s = dst; *d; s++, d++) {
		*s = *d;
		if (*s == PERIOD && d[1] == EOS)
			*s = EOS;
	}
	*s = EOS;

	s = vms_pathleaf(dst);

	/* SCCS hack */
	if (*s == '$') {
		*s++ = PERIOD;
	} else if (s[0] == 's' && s[1] == '$') {
		s[1] = PERIOD;
		s += 2;
	}

	return mklower(output);
}
#endif	/* OPT_VMS_PATH */
