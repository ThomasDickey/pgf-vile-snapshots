/*
 * The routines in this file read and write ASCII files from the disk. All of
 * the knowledge about files are here.
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/fileio.c,v 1.77 1994/08/08 16:12:29 pgf Exp $
 *
 */

#include	"estruct.h"
#include        "edef.h"
#if UNIX || VMS || MSDOS || OS2 || NT
#include	<sys/stat.h>
#endif

#if VMS
#include	<file.h>
#endif

#if HAVE_SYS_IOCTL_H
#include	<sys/ioctl.h>
#endif

#if NEWDOSCC
#include	<io.h>
#endif


#ifndef EISDIR
#define EISDIR EACCES
#endif

/*--------------------------------------------------------------------------*/

static	void	free_fline P(( void ));
#if MSDOS || OS2 || NT
static	int	copy_file P(( char *, char * ));
static	int	make_backup P(( char *, int ));
#endif
static	int	count_fline;	/* # of lines read with 'ffgetline()' */
  

/*--------------------------------------------------------------------------*/
static void
free_fline()
{
	FreeAndNull(fline);
	flen = 0;
}

#if MSDOS || OS2 || NT
/*
 * Copy file when making a backup, when we are appending.
 */
static int
copy_file (src, dst)
char	*src;
char	*dst;
{
	FILE	*ifp;
	FILE	*ofp;
	int	chr;
	int	ok = FALSE;

	if ((ifp = fopen(src, FOPEN_READ)) != 0) {
		if ((ofp = fopen(dst, FOPEN_WRITE)) != 0) {
			ok = TRUE;
			for (;;) {
				chr = fgetc(ifp);
				if (feof(ifp))
					break;
				fputc(chr, ofp);
				if (ferror(ifp) || ferror(ofp)) {
					ok = FALSE;
					break;
				}
			}
			(void)fclose(ofp);
		}
		(void)fclose(ifp);
	}
	return ok;
}

/*
 * Before overwriting a file, rename any existing version as a backup.
 * If appending, copy-back (retaining the modification-date of the original
 * file).
 *
 * Note: for UNIX, the direction of file-copy should be reversed, if the
 *       original file happens to be a symbolic link.
 */
static int
make_backup (fname, appending)
char	*fname;
int	appending;
{
	struct	stat	sb;
	int	ok	= TRUE;

	if (stat(fname, &sb) >= 0) {
		char	tname[NFILEN];
		char	*s = pathleaf(strcpy(tname, fname)),
			*t = strrchr(s, '.');
		if (t == 0)
			t = s + strlen(s);
		(void)strcpy(t, ".bak");
		(void)unlink(tname);
		ok = (rename(fname, tname) >= 0);
		if (ok && appending) {
			ok = copy_file(tname, fname);
			if (!ok) {	/* try to put things back together */
				(void)unlink(fname);
				(void)rename(tname, fname);
			}
		}
	}
	return ok;
}
#endif

/*
 * Open a file for reading.
 */
int
ffropen(fn)
char    *fn;
{
	fileispipe = FALSE;
	eofflag = FALSE;

	if (isShellOrPipe(fn)) {
		ffp = 0;
#if UNIX || MSDOS || OS2 || NT
	        ffp = npopen(fn+1, FOPEN_READ);
#endif
#if VMS
	        ffp = vms_rpipe(fn+1, 0, (char *)0);
		/* really a temp-file, but we cannot fstat it to get size */
#endif
		if (ffp == 0) {
			mlerror("opening pipe for read");
			return (FIOERR);
		}

		fileispipe = TRUE;
		count_fline = 0;

	} else if (is_directory(fn)) {
		set_errno(EISDIR);
		mlerror("opening directory");
		return (FIOERR);

	} else if ((ffp=fopen(fn, FOPEN_READ)) == NULL) {
		if (errno != ENOENT) {
			mlerror("opening for read");
			return (FIOERR);
		}
		return (FIOFNF);
	}

        return (FIOSUC);
}

/*
 * Open a file for writing. Return TRUE if all is well, and FALSE on error
 * (cannot create).
 */
int
ffwopen(fn)
char    *fn;
{
#if UNIX || MSDOS || OS2 || NT
	char	*name;
	char	*mode = FOPEN_WRITE;

	if (isShellOrPipe(fn)) {
		if ((ffp=npopen(fn+1, mode)) == NULL) {
	                mlerror("opening pipe for write");
	                return (FIOERR);
		}
		fileispipe = TRUE;
	} else {
#if MSDOS || OS2 || NT
		int	appending = FALSE;
#endif
		if ((name = is_appendname(fn)) != NULL) {
#if MSDOS || OS2 || NT
			appending = TRUE;
#endif
			fn = name;
			mode = FOPEN_APPEND;
		}
		if (is_directory(fn)) {
			set_errno(EISDIR);
			mlerror("opening directory");
			return (FIOERR);
		}
#if MSDOS	/* patch: should make this a mode */
		if (!make_backup(fn, appending)) {
			mlforce("[Can't make backup file]");
			return (FIOERR);
		}
#endif
		if ((ffp = fopen(fn, mode)) == NULL) {
			mlerror("opening for write");
			return (FIOERR);
		}
		fileispipe = FALSE;
	}
#else
#if     VMS
	char	temp[NFILEN];
        register int    fd;
	register char	*s;

	if ((s = strchr(fn = strcpy(temp, fn), ';')))	/* strip version */
		*s = EOS;

	if (is_appendname(fn)
	||  is_directory(fn)
	|| (fd=creat(fn, 0666, "rfm=var", "rat=cr")) < 0
        || (ffp=fdopen(fd, FOPEN_WRITE)) == NULL) {
                mlforce("[Cannot open file for writing]");
                return (FIOERR);
        }
#else
        if ((ffp=fopen(fn, FOPEN_WRITE)) == NULL) {
                mlerror("opening for write");
                return (FIOERR);
        }
#endif
#endif
        return (FIOSUC);
}

/* is the file read-only?  true or false */
int
ffronly(fn)
char    *fn;
{
	if (isShellOrPipe(fn)) {
		return TRUE;
	} else {
#if HAVE_ACCESS
		return (access(fn, 2) != 0);	/* W_OK==2 */
#else
		int fd;
	        if ((fd=open(fn, O_WRONLY)) < 0) {
	                return TRUE;
		}
		(void)close(fd);
		return FALSE;
#endif
	}
}

#if !OPT_MAP_MEMORY
#if UNIX || VMS || OS2 || NT
long
ffsize()
{
	struct stat statbuf;
	if (fstat(fileno(ffp), &statbuf) == 0) {
		return (long)statbuf.st_size;
	}
        return -1L;
}
#endif

#if MSDOS
#if DJGPP

long
ffsize(void)
{
	int flen, prev;
	prev = ftell(ffp);
	if (fseek(ffp, 0, 2) < 0)
		return -1L;
	flen = ftell(ffp);
	fseek(ffp, prev, 0);
	return flen;
}

#else

long
ffsize(void)
{
	int fd = fileno(ffp);
	if (fd < 0)
		return -1L;
	return  filelength(fd);
}

#endif
#endif
#endif	/* !OPT_MAP_MEMORY */

#if UNIX || VMS || OS2 || NT

int
ffexists(p)
char *p;
{
	struct stat statbuf;
	if (stat(p, &statbuf) == 0) {
		return TRUE;
	}
        return FALSE;
}

#endif

#if MSDOS || WIN31

int
ffexists(p)
char *p;
{
	if (ffropen(p) == FIOSUC) {
		ffclose();
		return TRUE;
	}
        return FALSE;
}

#endif

#if !MSDOS && !OPT_MAP_MEMORY
int
ffread(buf,len)
char *buf;
long len;
{
#if VMS
	/*
	 * If the input file is record-formatted (as opposed to stream-lf, a
	 * single read won't get the whole file.
	 */
	int	total = 0;

	while (len > 0) {
		int	this = read(fileno(ffp), buf+total, len-total);
		if (this <= 0)
			break;
		total += this;
	}
	fseek (ffp, len, 1);	/* resynchronize stdio */
	return total;
#else
	int got = read(fileno(ffp), buf, (SIZE_T)len);
	if (got >= 0)
	    fseek (ffp, len, 1);	/* resynchronize stdio */
	return got;
#endif
}

void
ffseek(n)
long n;
{
#if VMS
	ffrewind();	/* see below */
#endif
	fseek (ffp,n,0);
}

void
ffrewind()
{
#if VMS
	/* VAX/VMS V5.4-2, VAX-C 3.2 'rewind()' does not work properly, because
	 * no end-of-file condition is returned after rewinding.  Reopening the
	 * file seems to work.  We can get away with this because we only
	 * reposition in "permanent" files that we are reading.
	 */
	char	temp[NFILEN];
	fgetname(ffp, temp);
	(void)fclose(ffp);
	ffp = fopen(temp, FOPEN_READ);
#else
	fseek (ffp,0L,0);
#endif
}
#endif

/*
 * Close a file. Should look at the status in all systems.
 */
int
ffclose()
{
	int s = 0;

	free_fline();	/* free this since we do not need it anymore */

#if UNIX || MSDOS || WIN31 || OS2 || NT
	if (fileispipe) {
		npclose(ffp);
		mlforce("[Read %d lines%s]",
			count_fline,
			interrupted() ? "- Interrupted" : "");
#ifdef	MDCHK_MODTIME
		(void)check_visible_modtimes();
#endif
	} else {
		s = fclose(ffp);
	}
        if (s != 0) {
		mlerror("closing");
                return(FIOERR);
        }
#else
        (void)fclose(ffp);
#endif
        return (FIOSUC);
}

/*
 * Write a line to the already opened file. The "buf" points to the buffer,
 * and the "nbuf" is its length, less the free newline. Return the status.
 */
int
ffputline(buf, nbuf, ending)
char    buf[];
int	nbuf;
char *	ending;
{
        register int    i;
	for (i = 0; i < nbuf; ++i)
		if (ffputc(char2int(buf[i])) == FIOERR)
			return FIOERR;

	while (*ending != EOS) {
		if (*ending != '\r' || i == 0 || buf[i-1] != '\r')
			fputc(*ending, ffp);
		ending++;
	}

        if (ferror(ffp)) {
                mlerror("writing");
                return (FIOERR);
        }

        return (FIOSUC);
}

/*
 * Write a char to the already opened file.
 * Return the status.
 */
int
ffputc(c)
int c;
{
	char	d = c;

#if	CRYPT
	if (cryptflag)
		ue_crypt(&d, 1);
#endif
	fputc(d, ffp);

        if (ferror(ffp)) {
                mlerror("writing");
                return (FIOERR);
        }

        return (FIOSUC);
}

/*
 * Read a line from a file, and store the bytes in an allocated buffer.
 * "flen" is the length of the buffer. Reallocate and copy as necessary.
 * Check for I/O errors. Return status.
 */
int
ffgetline(lenp)
int *lenp;	/* to return the final length */
{
        register int c;		/* current character read */
        register int i;		/* current index into fline */
	register char *tmpline;	/* temp storage for expanding line */

	/* if we are at the end...return it */
	if (eofflag)
		return(FIOEOF);

	/* if we don't have an fline, allocate one */
	if (fline == NULL)
		if ((fline = castalloc(char,flen = NSTRING)) == NULL)
			return(FIOMEM);

	/* read the line in */
	i = 0;
	for (;;) {
		c = fgetc(ffp);
		if ((c == '\n') || feof(ffp) || ferror(ffp))
			break;
		if (interrupted()) {
			free_fline();
			*lenp = 0;
			return FIOABRT;
		}
                fline[i++] = c;
		/* if it's longer, get more room */
                if (i >= flen) {
			/* "Small" exponential growth - EJK */
			ALLOC_T growth = (flen >> 3) + NSTRING;
			if ((tmpline = castalloc(char,flen+growth)) == NULL)
                		return(FIOMEM);
                	(void)memcpy(tmpline, fline, (SIZE_T)flen);
                	flen += growth;
			free(fline);
                	fline = tmpline;
                }
#if OPT_WORKING
		cur_working++;
#endif
        }

#if !DOSFILES
# if	ST520
	if(c == '\n') {
		if(i > 0 && fline[i-1] == '\r') {
			i--;
		}
	}
# endif
#endif

	*lenp = i;	/* return the length, not including final null */
        fline[i] = EOS;

	/* test for any errors that may have occurred */
        if (c == EOF) {
		if (!feof(ffp) && ferror(ffp)) {
			mlerror("reading");
			return(FIOERR);
                }

                if (i != 0)
			eofflag = TRUE;
		else
			return(FIOEOF);
        }

#if	CRYPT
	/* decrypt the line */
	if (cryptflag)
		ue_crypt(fline, i);
#endif
	count_fline++;
        return (eofflag ? FIOFUN : FIOSUC);
}

/*
 * isready_c()
 *
 * This fairly non-portable addition to the stdio set of macros is used to
 * see if stdio has data for us, without actually reading it and possibly
 * blocking.  If you have trouble building this, just define no_isready_c
 * below, so that ffhasdata() always returns FALSE.  If you want to make it
 * work, figure out how your getc in stdio.h knows whether or not to call
 * _filbuf() (or the equivalent), and write isready_c so that it returns
 * true if the buffer has chars available now.  The big win in getting it
 * to work is that reading the output of a pipe (e.g.  ":e !co -p file.c")
 * is _much_much_ faster, and I don't have to futz with non-blocking
 * reads...
 */
#if WATCOM || OS2 || NT
#define no_isready_c 1 
#endif

#ifndef no_isready_c
# ifdef __sgetc
   /* 386bsd */
#  define	isready_c(p)	( (p)->_r > 0)
# else
#  ifdef _STDIO_UCHAR_
	/* C E Chew's package */
#   define 	isready_c(p)	( (p)->__rptr < (p)->__rend)
#  else
#   ifdef _G_FOPEN_MAX
	/* two versions of GNU iostream/stdio library */
#     if _IO_STDIO
#      define   isready_c(p)    ( (p)->_IO_read_ptr < (p)->_IO_read_end)
#     else
#      define 	isready_c(p)	( (p)->_gptr < (p)->_egptr)
#     endif
#   else
#    if VMS
#     define	isready_c(p)	( (*p)->_cnt > 0)
#    endif
#    if TURBO
#     define    isready_c(p)	( (p)->bsize > ((p)->curp - (p)->buffer) )
#    endif
#    ifndef isready_c	/* most other stdio's (?) */
#     define	isready_c(p)	( (p)->_cnt > 0)
#    endif
#   endif
#  endif
# endif
#endif


int
ffhasdata()
{
#ifdef isready_c
	if (isready_c(ffp))
		return TRUE;
#endif
#ifdef	FIONREAD
	{
	long x;
	return(((ioctl(fileno(ffp),FIONREAD,(caddr_t)&x) < 0) || x == 0) ? FALSE : TRUE);
	}
#else
	return FALSE;
#endif
}

