/*
 * The routines in this file read and write ASCII files from the disk. All of
 * the knowledge about files are here.
 *
 * $Log: fileio.c,v $
 * Revision 1.31  1992/12/14 09:03:25  foxharp
 * lint cleanup, mostly malloc
 *
 * Revision 1.30  1992/08/20  23:40:48  foxharp
 * typo fixes -- thanks, eric
 *
 * Revision 1.29  1992/07/28  21:42:36  foxharp
 * linux switched to GNU stdio... okay, so maybe isready_c() _wasn't_
 * such a good idea...
 *
 * Revision 1.28  1992/07/24  18:21:17  foxharp
 * cleanup and better comments re: isready_c, and linux support
 *
 * Revision 1.27  1992/06/25  23:00:50  foxharp
 * changes for dos/ibmpc
 *
 * Revision 1.26  1992/05/19  08:55:44  foxharp
 * more prototype and shadowed decl fixups
 *
 * Revision 1.25  1992/05/16  14:02:55  pgf
 * header/typedef fixups
 *
 * Revision 1.24  1992/05/16  12:00:31  pgf
 * prototypes/ansi/void-int stuff/microsoftC
 *
 * Revision 1.23  1992/04/14  08:51:44  pgf
 * ifdef fixups for pjr and DOS
 *
 * Revision 1.22  1992/03/25  19:13:17  pgf
 * BSD portability changes
 *
 * Revision 1.21  1992/03/19  23:21:12  pgf
 * moved includes to top
 *
 * Revision 1.20  1992/01/05  00:06:13  pgf
 * split mlwrite into mlwrite/mlprompt/mlforce to make errors visible more
 * often.  also normalized message appearance somewhat.
 *
 * Revision 1.19  1992/01/01  16:17:46  pgf
 * improve behavior of long line reads (from Eric Krohn)
 *
 * Revision 1.18  1991/11/27  10:09:09  pgf
 * move some ifdef, so as not to leave empty if body
 *
 * Revision 1.17  1991/11/16  18:31:39  pgf
 * ifdef with FIONREAD instead of BSD in typahead()
 *
 * Revision 1.16  1991/11/08  13:22:56  pgf
 * moved dosfiles processing out to file.c, and
 * made aborts of file reads report FIOABRT, and
 * hopefully, editing binary files should work better (don't
 * use strncpy on lines)
 *
 * Revision 1.15  1991/11/07  02:00:32  pgf
 * lint cleanup
 *
 * Revision 1.14  1991/11/03  17:38:38  pgf
 * fixed some slop in the cr/nl checking
 *
 * Revision 1.13  1991/11/01  14:38:00  pgf
 * saber cleanup
 *
 * Revision 1.12  1991/10/23  12:05:37  pgf
 * changed filio.h to ioctl.h as the source of FIONREAD
 *
 * Revision 1.11  1991/10/22  14:08:23  pgf
 * took out old ifdef BEFORE code
 *
 * Revision 1.10  1991/10/10  12:31:53  pgf
 * added more "ff" utilities:  ffread, ffseek, ffrewind, ffsize
 *
 * Revision 1.9  1991/09/25  00:24:27  pgf
 * ffhasdata now works most of the time for system V, by simply checking
 * the stdio buffer -- no system call necessary
 *
 * Revision 1.8  1991/08/08  13:20:23  pgf
 * set "dosfile" global after reading each line
 *
 * Revision 1.7  1991/08/07  12:35:07  pgf
 * added RCS log messages
 *
 * revision 1.6
 * date: 1991/06/18 20:08:09;
 * added decl for FILE *npopen()
 * 
 * revision 1.5
 * date: 1991/04/22 09:02:03;
 * portability
 * 
 * revision 1.4
 * date: 1991/04/08 15:49:44;
 * added ffhasdata routine
 * 
 * revision 1.3
 * date: 1991/04/04 09:37:25;
 * minor fixes
 * 
 * revision 1.2
 * date: 1990/10/12 19:30:46;
 * added beeps on non-writeable
 * 
 * revision 1.1
 * date: 1990/09/21 10:25:18;
 * initial vile RCS revision
 */

#include        <stdio.h>
#include	"estruct.h"
#include        "edef.h"
#if UNIX
#include	<sys/stat.h>
#include        <errno.h>
#endif
#include        <fcntl.h>
#if	BERK
#include "sys/ioctl.h"
#endif
#if MSDOS
#include	<sys/stat.h>
#endif

FILE	*ffp;		/* File pointer, all functions. */
int fileispipe;
int eofflag;		/* end-of-file flag */

/*
 * Open a file for reading.
 */
int
ffropen(fn)
char    *fn;
{
#if UNIX
	FILE *npopen();
#endif


#if UNIX
	
	if (*fn == '!') {
	        if ((ffp=npopen(fn+1, "r")) == NULL)
	                return (FIOERR);
		fileispipe = TRUE;
	} else {
	        if ((ffp=fopen(fn, "r")) == NULL) {
			extern int errno;
			if (errno == ENOENT)
		                return (FIOFNF);
	                return (FIOERR);
		}
		fileispipe = FALSE;
	}
#else
        if ((ffp=fopen(fn, "r")) == NULL)
                return (FIOFNF);
#endif
	eofflag = FALSE;
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
#if UNIX
	FILE *npopen();
	if (*fn == '!') {
	        if ((ffp=npopen(fn+1, "w")) == NULL) {
	                mlforce("[Cannot open pipe for writing]");
			TTbeep();
	                return (FIOERR);
		}
		fileispipe = TRUE;
	} else {
	        if ((ffp=fopen(fn, "w")) == NULL) {
	                mlforce("[Cannot open file for writing]");
			TTbeep();
	                return (FIOERR);
		}
		fileispipe = FALSE;
	}
#else
#if     VMS
        register int    fd;

        if ((fd=creat(fn, 0666, "rfm=var", "rat=cr")) < 0
        || (ffp=fdopen(fd, "w")) == NULL) {
                mlforce("[Cannot open file for writing]");
                return (FIOERR);
        }
#else
        if ((ffp=fopen(fn, "w")) == NULL) {
                mlforce("[Cannot open file for writing]");
                return (FIOERR);
        }
#endif
#endif
        return (FIOSUC);
}

/* is the file read-only?  true or false */
/* #if UNIX  / * don't know how to do it for other systems */
int
ffronly(fn)
char    *fn;
{
	int fd;

	if (*fn == '!') {
		return TRUE;
	} else {
	        if ((fd=open(fn, O_WRONLY)) < 0) {
	                return TRUE;
		}
		close(fd);
		return FALSE;
	}
}
/* #endif */

#if UNIX
long
ffsize()
{
	struct stat statbuf;
	if (fstat(fileno(ffp), &statbuf) == 0) {
		return (long)statbuf.st_size;
	}
        mlforce("[File sizing error]");
        return -1;
}
#endif

#if MSDOS
long
ffsize(void)
{
	int fd = fileno(ffp);
	return  filelength(fd);
}
#endif

int
ffread(buf,len)
char *buf;
long len;
{
	return read(fileno(ffp), buf, (int)len);
}

void
ffseek(n)
long n;
{
	fseek (ffp,n,0);
}

void
ffrewind()
{
	fseek (ffp,0L,0);
}

/*
 * Close a file. Should look at the status in all systems.
 */
int
ffclose()
{
	int s = TRUE;

	/* free this since we do not need it anymore */
	if (fline) {
		free(fline);
		fline = NULL;
		flen = 0;
	}

#if UNIX | (MSDOS & (LATTICE | MSC | TURBO | ZTC))
#if UNIX
	
	if (fileispipe)
		npclose(ffp);
	else
#endif
		s = fclose(ffp);
        if (s != 0) {
                mlforce("[Error on close]");
                return(FIOERR);
        }
        return(FIOSUC);
#else
        fclose(ffp);
        return (FIOSUC);
#endif
}

/*
 * Write a line to the already opened file. The "buf" points to the buffer,
 * and the "nbuf" is its length, less the free newline. Return the status.
 * Check only at the newline.
 */
int
ffputline(buf, nbuf, do_cr)
char    buf[];
int	nbuf;
int	do_cr;
{
        register int    i;
#if	CRYPT
	char c;		/* character to translate */

	if (cryptflag) {
	        for (i = 0; i < nbuf; ++i) {
			c = buf[i] & 0xff;
			crypt(&c, 1);
			fputc(c, ffp);
		}
	} else
	        for (i = 0; i < nbuf; ++i)
        	        fputc(buf[i]&0xFF, ffp);
#else
        for (i = 0; i < nbuf; ++i)
                fputc(buf[i]&0xFF, ffp);
#endif

#if	ST520
        fputc('\r', ffp);
#endif        
#if DOSFILES
	if (do_cr) { /* put out CR, unless we just did */
		if (i == 0 || buf[i-1] != '\r')
		        fputc('\r', ffp);
	}
#endif
        fputc('\n', ffp);

        if (ferror(ffp)) {
                mlforce("[Write I/O error]");
                return (FIOERR);
        }

        return (FIOSUC);
}
/*
 * Write a charto the already opened file.
 * Return the status.
 */
int
ffputc(c)
int c;
{
	c &= 0xff;

#if	CRYPT
	if (cryptflag)
		crypt(&c, 1);
#endif
        fputc(c, ffp);

        if (ferror(ffp)) {
                mlforce("[Write I/O error]");
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
        while ((c = fgetc(ffp)) != EOF && c != '\n') {
		if (interrupted) {
			free(fline);
			fline = NULL;
			*lenp = 0;
			return FIOABRT;
		}
                fline[i++] = c;
		/* if it's longer, get more room */
                if (i >= flen) {
			/* "Small" exponential growth - EJK */
			unsigned growth = (flen >> 3) + NSTRING;
			if ((tmpline = castalloc(char,flen+growth)) == NULL)
                		return(FIOMEM);
                	memcpy(tmpline, fline, flen);
                	flen += growth;
                	free(fline);
                	fline = tmpline;
                }
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
        fline[i] = 0;

	/* test for any errors that may have occurred */
        if (c == EOF) {
                if (ferror(ffp)) {
                        mlforce("[File read error]");
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
		crypt(fline, i);
#endif
        return(FIOSUC);
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

/* #define no_isready_c 1 */

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
	/* GNU iostream/stdio library */
#    define 	isready_c(p)	( (p)->_gptr < (p)->_egptr)
#   else
	/* most other stdio's (?) */
#    define	isready_c(p)	( (p)->_cnt > 0)
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
	return(((ioctl(fileno(ffp),FIONREAD,&x) < 0) || x == 0) ? FALSE : TRUE);
	}
#else
	return FALSE;
#endif
}

#if	AZTEC & MSDOS
#undef	fgetc
/*	a1getc:		Get an ascii char from the file input stream
			but DO NOT strip the high bit
*/

int a1getc(fp)

FILE *fp;

{
	int c;		/* translated character */

	c = getc(fp);	/* get the character */

	/* if its a <LF> char, throw it out  */
	while (c == '\n')
		c = getc(fp);

	/* if its a <RETURN> char, change it to a LF */
	if (c == '\r')
		c = '\n';

	/* if its a ^Z, its an EOF */
	if (c == 26)
		c = EOF;

	return(c);
}
#endif
