/*
 * The routines in this file read and write ASCII files from the disk. All of
 * the knowledge about files are here.
 *
 * $Log: fileio.c,v $
 * Revision 1.11  1991/10/22 14:08:23  pgf
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
#include        <errno.h>
#include        <fcntl.h>
#endif
#if	BSD
#include "sys/filio.h"
#endif

FILE	*ffp;		/* File pointer, all functions. */
int fileispipe;
int eofflag;		/* end-of-file flag */
#if DOSFILES
int doslines, unixlines;
int dosfile;
#endif


/*
 * Open a file for reading.
 */
ffropen(fn)
char    *fn;
{
#if UNIX
	FILE *npopen();
#endif

#if DOSFILES
	doslines = unixlines = 0;
	dosfile = FALSE;
#endif

#if UNIX
	
	if (*fn == '!') {
	        if ((ffp=npopen(fn+1, "r")) == NULL)
	                return (FIOERR);
		fileispipe = TRUE;
	} else {
	        if ((ffp=fopen(fn, "r")) == NULL) {
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
ffwopen(fn)
char    *fn;
{
#if UNIX
	FILE *npopen();
	if (*fn == '!') {
	        if ((ffp=npopen(fn+1, "w")) == NULL) {
	                mlwrite("Cannot open pipe for writing");
			TTbeep();
	                return (FIOERR);
		}
		fileispipe = TRUE;
	} else {
	        if ((ffp=fopen(fn, "w")) == NULL) {
	                mlwrite("Cannot open file for writing");
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
                mlwrite("Cannot open file for writing");
                return (FIOERR);
        }
#else
        if ((ffp=fopen(fn, "w")) == NULL) {
                mlwrite("Cannot open file for writing");
                return (FIOERR);
        }
#endif
#endif
        return (FIOSUC);
}

/* is the file read-only?  true or false */
#if UNIX  /* don't know how to do it for other systems */
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
#endif

#ifdef UNIX

#include "sys/types.h"
#include "sys/stat.h"

long
ffsize()
{
	struct stat statbuf;
	if (fstat(fileno(ffp), &statbuf) == 0) {
		return (long)statbuf.st_size;
	}
        mlwrite("File sizing error");
        return -1;
}

#endif

ffread(buf,len)
char *buf;
off_t len;
{
	return read(fileno(ffp), buf, (int)len);
}

ffseek(n)
long n;
{
	fseek (ffp,n,0);
}

ffrewind()
{
	fseek (ffp,0L,0);
}

/*
 * Close a file. Should look at the status in all systems.
 */
ffclose()
{
	int s;

	/* free this since we do not need it anymore */
	if (fline) {
		free(fline);
		fline = NULL;
		flen = 0;
	}

#if	MSDOS & CTRLZ & NEVER
	 but we NEVER want to do this on read closes!!!
	fputc(26, ffp);		/* add a ^Z at the end of the file */
#endif
	
#if DOSFILES
	/* did we get more dos-style lines than unix-style? */
	dosfile = (doslines > unixlines);
	unixlines = 0;
#endif
#if UNIX | (MSDOS & (LATTICE | MSC | TURBO))
#if UNIX
	
	if (fileispipe)
		s = npclose(ffp);
	else
#endif
		s = fclose(ffp);
        if (s != 0) {
                mlwrite("Error on close");
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
ffputline(buf, nbuf)
char    buf[];
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
	if (dosfile) {
		/* put out CR, unless we just did */
		if (i == 0 || buf[i-1] != '\r')
		        fputc('\r', ffp);
	}
#endif
        fputc('\n', ffp);

        if (ferror(ffp)) {
                mlwrite("Write I/O error");
                return (FIOERR);
        }

        return (FIOSUC);
}
/*
 * Write a charto the already opened file.
 * Return the status.
 */
ffputc(c)
{
	static lastc;
	c &= 0xff;
#if DOSFILES
	if (dosfile && c == '\n' && lastc != '\r')
		 fputc('\r',ffp);
#endif
	lastc = c;

#if	CRYPT
	if (cryptflag)
		crypt(&c, 1);
#endif
        fputc(c, ffp);

        if (ferror(ffp)) {
                mlwrite("Write I/O error");
                return (FIOERR);
        }

        return (FIOSUC);
}

/*
 * Read a line from a file, and store the bytes in an allocated buffer.
 * "flen" is the length of the buffer. Reallocate and copy as necessary.
 * Check for I/O errors. Return status.
 */
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
		if ((fline = malloc(flen = NSTRING)) == NULL)
			return(FIOMEM);

	/* read the line in */
        i = 0;
        while ((c = fgetc(ffp)) != EOF && c != '\n') {
                fline[i++] = c;
		/* if it's longer, get more room */
                if (i >= flen) {
                	if ((tmpline = malloc(flen+NSTRING)) == NULL)
                		return(FIOMEM);
                	strncpy(tmpline, fline, flen);
                	flen += NSTRING;
                	free(fline);
                	fline = tmpline;
                }
        }

#if !DOSFILES
#if	ST520
	if(fline[i-1] == '\r') {
		i--;
	}
#endif
#else
	if(fline[i-1] == '\r') {
		doslines++;
		i--;
	} else {
		unixlines++;
	}
	dosfile = (doslines > unixlines);
#endif

	*lenp = i;	/* return the length, not including final null */
        fline[i] = 0;

	/* test for any errors that may have occured */
        if (c == EOF) {
                if (ferror(ffp)) {
                        mlwrite("File read error");
                        return(FIOERR);
                }

                if (i != 0)
			eofflag = TRUE;
		else
			return(FIOEOF);
        }

#if	CRYPT
	/* decrypt the string */
	if (cryptflag)
		crypt(fline, strlen(fline));
#endif
        return(FIOSUC);
}

/* this possibly non-portable addition to the stdio set of macros 
	is used to see if stdio has data for us, without actually
	reading it and possibly blocking.  if you have trouble building
	this, just force ffhasdata() to always return FALSE */
#define	isready_c(p)	( (p)->_cnt > 0)

ffhasdata()
{
	long x;
	if (isready_c(ffp))
		return TRUE;
#if	BSD
	return(((ioctl(fileno(ffp),FIONREAD,&x) < 0) || x == 0) ? FALSE : TRUE);
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
