/*
 *	vms2unix.c
 *
 *	Miscellaneous routines for UNIX/VMS compatibility.
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/vms2unix.c,v 1.7 1994/10/24 01:15:52 pgf Exp $
 *
 */
#include	"estruct.h"
#include	"edef.h"
#include	"dirstuff.h"

#if VMS
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
		zrsa[znam.nam$b_rsl];
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
tempnam(head, tail)
char	*head;
char	*tail;
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

static	int	DotPrefix P((char *s));
static	char	CharToVms P((int c));
static	int	leading_uc P((char *dst, char *src));

static	int	leaf_dot;   /* counts dots found in a particular leaf */
static	int	leaf_ver;   /* set if we found a DECshell version */

/*
 * If we have a dot in the second character position, force that to a dollar
 * sign.  Otherwise, keep the first dot in each unix leaf as a dot in the
 * resulting vms name.
 */
static
int	DotPrefix(s)
	char *	s;
{
	if (s[0] != EOS
	 && s[1] == '.'
	 && s[2] != EOS
	 && strchr("sp", s[0]))	/* hack for SCCS */
		return (-1);
	return (0);
}

static
char	CharToVms(c)
	int	c;
{
	if (c == '.') {
		if (leaf_dot++)
			c = '$';
	} else if (!isalnum(c) && !strchr("_-", c)) {
		c = '$';
	}
	return (c);
}

static
int	leading_uc(dst, src)
	char *	dst;
	char *	src;
{
	auto	char	*base = dst;
	register int	c;

	while ((c = *src) != EOS && c != '/') {
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

char *	unix2vms_path(dst, src)
	char *dst;
	const char *src;
{
	char	tmp[NFILEN],
		leading[NFILEN],
		*t,
		*s = strcpy(tmp, src),	/* ... to permit src == dst */
		*d = dst,
		c  = '?';
	auto	int	bracket	= FALSE,	/* true when "[" passed. */
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
			s++;		/* skip (assumed) '/' */
			if ((len > 1)
			&&  (d[len-1] == ':')) {
				on_top = TRUE;
			} else if (strchr(s, '/')) {	/* must do a splice */
				if ((len > 2)
				&&  (d[len-1] == ']')) {
					bracket++;
					if (d[len-2] == '.')
						/* rooted-device ? */
						len -= 2;
					else
						len--;
				}
			}
		}
		d[len] = EOS;
		if ((t = strchr(d, ':')) != NULL) {
			if (t[1] == ':') {
				node = TRUE;
				if ((t = strchr(t+2, ':')) != NULL)
					device = TRUE;
			} else
				device = TRUE;
		}
		d  += len;
	}

	/* look for node-name in VMS-format */
	if (!node
	&&  (t = strchr(s, '!')) != 0
	&&  (t[1] == '/' || t[1] == EOS)) {
		leaf_dot = DotPrefix(s);
		while (s < t)
			*d++ = CharToVms(*s++);
		*d++ = ':';
		*d++ = ':';
		s++;		/* skip over '!' */
	}

	/* look for device-name, indicated by a leading '/' */
	if (!device
	&&  (*s == '/')) {
		leaf_dot = DotPrefix(++s);
		if ((t = strchr(s, '/')) == 0)
			t = s + strlen(s);
		else if (t[1] == EOS)
			on_top = TRUE;
		while (s < t)
			*d++ = CharToVms(*s++);
		*d++ = ':';
	}

	/* permit leading "./" to simplify cases in which we concatenate */
	if (!strncmp(s, "./", 2))
		s += 2;

	/* translate repeated leading "../" */
	while (!strncmp(s, "../", 3)) {
		s += 3;
		if (!bracket++)
			*d++ = '[';
		*d++ = '-';
	}

	if (strchr(s, '/')) {
		if (!bracket++)
			*d++ = '[';
		if (*s == '/') {
			s++;
		} else if (!on_top) {
			*d++ = '.';
		}
		while ((c = *s++) != EOS) {
			if (c == '.')
				c = '$';
			if (c == '/') {
		    		leaf_dot = DotPrefix(s);
				if (strchr(s, '/'))
					*d++ = '.';
				else {
					break;
				}
			} else {
				*d++ = CharToVms(c);
			}
		}
	}
	if (bracket) {
		if (on_top && d[-1] == '[') {
			strcpy(d, "000000");
			d += strlen(d);
		}
		*d++ = ']';
	}
	if (c != EOS && *s) {
		leaf_dot = DotPrefix(s);
		while ((c = *s++) != EOS) {
			if (c == '.'
			 && (leaf_ver = (strtol(s, &t, 0) && (t != s) && !*t)))
				*d++ = ';';
			else
				*d++ = CharToVms(c);
		}
		if (!leaf_dot)
			*d++ = '.';
		if (!leaf_ver)
			*d++ = ';';
	}
	*d = EOS;
	return mkupper(dst);
}

/******************************************************************************
 * Convert a VMS pathname into the name of the corresponding directory-file.  *
 *                                                                            *
 * Note that this returns a pointer to a static buffer which is overwritten   *
 * by each call.                                                              *
 ******************************************************************************/

char *	vms_path2dir(src)
	const char *src;
{
	static	char	buffer[NFILEN];
	register char	*s	= buffer + strlen(strcpy(buffer, src));

	if (s != buffer && *(--s) == ']') {
		(void)strcpy(s, ".DIR");
		while (--s >= buffer) {
			if (*s == '.') {
				*s = ']';
				if (s == buffer+1) {	/* absorb "[]" */
					register char *t = s + 1;
					s = buffer;
					while ((*s++ = *t++) != EOS)
						;
				}
				break;
			}
			if (*s == '[') {		/* absorb "[" */
				register char *t = s + 1;
				while ((*s++ = *t++) != EOS)
					;
				break;
			}
		}
	}
	return (buffer);
}

/******************************************************************************
 * Translate a VMS-style pathname to a UNIX-style pathname                    *
 ******************************************************************************/

char *	vms2unix_path(dst, src)
	char *dst;
	const char *src;
{
	auto	char	current[NFILEN];
	auto	int	need_dev = FALSE,
			have_dev = FALSE;
	auto	char	tmp[NFILEN],
			*output = dst,
			*base = tmp,
			*s = strcpy(tmp, src),	/* ... to permit src == dst */
			*d;

	if ((s = strchr(s, ';')) != NULL)	/* trim off version */
		*s = EOS;

	/* look for node specification */
	if ((s = strchr(base, ':')) != 0
	&&  (s[1] == ':')) {
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
	if ((s = strchr(base, ':')) != NULL) {
		*dst++ = '/';
		while (base < s) {
			*dst++ = *base++;
		}
		base++;			/* skip over ":" */
		have_dev = TRUE;
	} else if (need_dev
	||	  ((base[0] == '[')
	&&	   (base[1] != '-')
	&&	   (base[1] != '.')
	&&	   (base[1] != ']'))) {	/* must supply a device */
		register char	*a = getcwd(current, NFILEN),
				*b = strchr(a ? a : "?", ':');
		if ((b != 0)
		&&  (b[1] == ':')) {	/* skip over node specification */
			a = b + 2;
			b = strchr(a, ':');
		}
		if (b != 0) {
			*dst++ = '/';	/* begin the device */
			while (a < b) {
				*dst++ = *a++;
			}
			have_dev = TRUE;
		}			/* else, no device in getcwd! */
	}

	/* translate directory-syntax */
	if ((s = strchr(base, '[')) != NULL) {
		if (s[1] == ']') {
			if (*dst != '/' && dst != output)
				*dst++ = '/';
			*dst++ = '.';
			if (s[2] != EOS)
				*dst++ = '/';
			s += 2;
			d = s;
		} else if (s[1] == '.') {
			if (have_dev)
				*dst++ = '/';
			s += 2;
			d = s;
		} else if (s[1] == '-' && strchr("-.]", s[2])) {
			s++;
			while (*s == '-') {
				s++;
				*dst++ = '.';
				*dst++ = '.';
				if (*s == '.' && (s[1] == '-' || s[1] == ']'))
					/* allow "-.-" */
					s++;
				if (*s == '-')
					*dst++ = '/';
			}
			d = s;
		} else if (!strncmp(s+1, "000000", 6) && strchr(".]", s[7])) {
			s += 7;
			d = s;
		} else {
			d = s;
			*s++ = '/';
		}
		/* expect s points to the last token before ']' */
		while (*s && *s != ']') {
			if (*s == '.')
				*s = '/';
			s++;
		}
		if (*s)
			*s = s[1] ? '/' : EOS;
	} else {
		if (have_dev)
			*dst++ = '/';
		d = base;
	}

	/*
	 * Copy the remainder of the string, trimming trailing "."
	 */
	for (s = dst; *d; s++, d++) {
		*s = *d;
		if (*s == '.' && d[1] == EOS)
			*s = EOS;
	}
	*s = EOS;

	s = vms_pathleaf(dst);

	/* SCCS hack */
	if (*s == '$') {
		*s++ = '.';
	} else if (s[0] == 's' && s[1] == '$') {
		s[1] = '.';
		s += 2;
	}

	return mklower(output);
}
#endif	/* OPT_VMS_PATH */
