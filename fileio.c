/*
 * The routines in this file read and write ASCII files from the disk. All of
 * the knowledge about files are here.
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/fileio.c,v 1.87 1994/11/29 17:04:43 pgf Exp $
 *
 */

#include	"estruct.h"
#include        "edef.h"
#if SYS_UNIX || SYS_VMS || SYS_MSDOS || SYS_OS2 || SYS_WINNT
#include	<sys/stat.h>
#endif

#if SYS_VMS
#include	<file.h>
#endif

#if HAVE_SYS_IOCTL_H
#include	<sys/ioctl.h>
#endif

#if CC_NEWDOSCC
#include	<io.h>
#endif


#ifndef EISDIR
#define EISDIR EACCES
#endif

/*--------------------------------------------------------------------------*/

static	void	free_fline P(( void ));
#if OPT_FILEBACK
static	int	copy_file P(( char *, char * ));
static	int	write_backup_file P((char *, char * ));
static	int	make_backup P(( char * ));
#endif
static	int	count_fline;	/* # of lines read with 'ffgetline()' */
  

/*--------------------------------------------------------------------------*/
static void
free_fline()
{
	FreeAndNull(fline);
	flen = 0;
}

#if OPT_FILEBACK
/*
 * Copy file when making a backup
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
 * Copy-back to retain the modification-date of the original file.
 *
 * Note: for UNIX, the direction of file-copy should be reversed, if the
 *       original file happens to be a symbolic link.
 */

#if SYS_UNIX
# if ! HAVE_LONG_FILENAMES
#  define MAX_FN_LEN 14
# else
#  define MAX_FN_LEN 255
# endif
#endif

static int
write_backup_file(orig, backup)
char *orig;
char *backup;
{
	int s;

	struct stat ostat, bstat;

	if (stat(orig, &ostat) != 0)
		return FALSE;

	if (stat(backup, &bstat) == 0) {  /* the backup file exists */
		
#if SYS_UNIX
		/* same file, somehow? */
		if (bstat.st_dev == ostat.st_dev &&
		    bstat.st_ino == ostat.st_ino) {
			return FALSE;
		}
#endif

		/* remove it */
		if (unlink(backup) != 0)
			return FALSE;
	}

	/* there are many reasons for copying, rather than renaming
	   and writing a new file -- the file may have links, which
	   will follow the rename, rather than stay with the real-name.
	   additionally, if the write fails, we need to re-rename back to
	   the original name, otherwise two successive failed writes will
	   destroy the file.
	*/

	s = copy_file(orig, backup);
	if (s != TRUE)
		return s;

	/* change date and permissions to match original */
#if HAVE_UTIME
	{
	    struct utimbuf buf;
	    buf.actime = ostat.st_atime;
	    buf.modtime = ostat.st_mtime;
	    s = utime(backup, &buf);
	    if (s != 0) {
		    (void)unlink(backup);
		    return FALSE;
	    }
	}
#else
#if HAVE_UTIMES
	{
	    struct timeval buf[2];
	    buf[0].tv_sec = ostat.st_atime;
	    buf[0].tv_usec = 0;
	    buf[1].tv_sec = ostat.st_mtime;
	    buf[1].tv_usec = 0;
	    s = utimes(backup, buf);
	    if (s != 0) {
		    (void)unlink(backup);
		    return FALSE;
	    }
	}
#endif
#endif

	s = chmod(backup, ostat.st_mode & 0777);
	if (s != 0) {
		(void)unlink(backup);
		return FALSE;
	}

	return TRUE;
}

static int
make_backup (fname)
char	*fname;
{
	int	ok	= TRUE;

	if (ffexists(fname) >= 0) { /* if the file exists, attempt a backup */
		char	tname[NFILEN];
		char	*s = pathleaf(strcpy(tname, fname)),
			*t = strrchr(s, '.');
		char *gvalfileback = global_g_val_ptr(GVAL_BACKUPSTYLE);

		if (strcmp(gvalfileback,".bak") == 0) {
			if (t == 0)
				t = s + strlen(s);
			(void)strcpy(t, ".bak");
#if SYS_UNIX
		} else if (strcmp(gvalfileback, "tilde") == 0) {
			t = s + strlen(s);
#if ! HAVE_LONG_FILENAMES
			if (t - s >= MAX_FN_LEN) {
				if (t - s == MAX_FN_LEN &&
					s[MAX_FN_LEN-2] == '.')
					s[MAX_FN_LEN-2] = s[MAX_FN_LEN-1];
				t = &s[MAX_FN_LEN-1];
			}
#endif
			(void)strcpy(t, "~");
#if SOMEDAY
		} else if (strcmp(gvalfileback, "tilde_N_existing") {
			/* numbered backups if one exists, else simple */
		} else if (strcmp(gvalfileback, "tilde_N") {
			/* numbered backups of all files*/
#endif
#endif /* SYS_UNIX */
		} else {
			mlwrite("BUG: bad fileback value");
			return FALSE;
		}

		ok = write_backup_file(fname, tname);

	}
	return ok;
}
#endif /* OPT_FILEBACK */

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
#if SYS_UNIX || SYS_MSDOS || SYS_OS2 || SYS_WINNT
	        ffp = npopen(fn+1, FOPEN_READ);
#endif
#if SYS_VMS
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
		if (errno != ENOENT
		 && errno != EINVAL) {	/* a problem with Linux to DOS-files */
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
ffwopen(fn,forced)
char    *fn;
int	forced;
{
#if SYS_UNIX || SYS_MSDOS || SYS_OS2 || SYS_WINNT
	char	*name;
	char	*mode = FOPEN_WRITE;

	if (isShellOrPipe(fn)) {
		if ((ffp=npopen(fn+1, mode)) == NULL) {
	                mlerror("opening pipe for write");
	                return (FIOERR);
		}
		fileispipe = TRUE;
	} else {
		if ((name = is_appendname(fn)) != NULL) {
			fn = name;
			mode = FOPEN_APPEND;
		}
		if (is_directory(fn)) {
			set_errno(EISDIR);
			mlerror("opening directory");
			return (FIOERR);
		}

#if OPT_FILEBACK
		/* will we be able to write? (before attempting backup) */
		if (ffexists(fn) && ffronly(fn)) {
			mlerror("accessing for write");
			return (FIOERR);
		}

		if (*global_g_val_ptr(GVAL_BACKUPSTYLE) != 'o') { /* "off" ? */
			if (!make_backup(fn)) {
				if (!forced) {
					mlerror("making backup file");
					return (FIOERR);
				}
			}
		}
#endif
		if ((ffp = fopen(fn, mode)) == NULL) {
			mlerror("opening for write");
			return (FIOERR);
		}
		fileispipe = FALSE;
	}
#else
#if     SYS_VMS
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
	        if ((fd=open(fn, O_WRONLY|O_APPEND)) < 0) {
	                return TRUE;
		}
		(void)close(fd);
		return FALSE;
#endif
	}
}

#if !OPT_MAP_MEMORY
#if SYS_UNIX || SYS_VMS || SYS_OS2 || SYS_WINNT
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

#if SYS_MSDOS
#if CC_DJGPP

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

#if SYS_UNIX || SYS_VMS || SYS_OS2 || SYS_WINNT

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

#if SYS_MSDOS || SYS_WIN31

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

int
ffreadable(p)
char *p;
{
	if (ffropen(p) == FIOSUC) {
		ffclose();
		return TRUE;
	}
        return FALSE;
}

#if !SYS_MSDOS && !OPT_MAP_MEMORY
int
ffread(buf,len)
char *buf;
long len;
{
#if SYS_VMS
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
#if SYS_VMS
	ffrewind();	/* see below */
#endif
	fseek (ffp,n,0);
}

void
ffrewind()
{
#if SYS_VMS
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

#if SYS_UNIX || SYS_MSDOS || SYS_WIN31 || SYS_OS2 || SYS_WINNT
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

#if	OPT_ENCRYPT
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

#if !OPT_DOSFILES
# if	SYS_ST520
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

#if	OPT_ENCRYPT
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
#if CC_WATCOM || SYS_OS2 || SYS_WINNT
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
#    if SYS_VMS
#     define	isready_c(p)	( (*p)->_cnt > 0)
#    endif
#    if CC_TURBO
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

