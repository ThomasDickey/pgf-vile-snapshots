/*	FILE.C:   for MicroEMACS
 *
 *	The routines in this file handle the reading, writing
 *	and lookup of disk files.  All of details about the
 *	reading and writing of the disk are in "fileio.c".
 *
 *
 * $Log: file.c,v $
 * Revision 1.14  1991/08/08 13:19:08  pgf
 * fixed MDDOS processing, and don't allow writes of view-only buffers
 *
 * Revision 1.13  1991/08/07  12:35:07  pgf
 * added RCS log messages
 *
 * revision 1.12
 * date: 1991/08/06 15:21:21;
 * global/local values
 * 
 * revision 1.11
 * date: 1991/06/25 19:52:35;
 * massive data structure restructure
 * 
 * revision 1.10
 * date: 1991/06/13 15:18:28;
 * fixed comment on glob()
 * 
 * revision 1.9
 * date: 1991/06/03 12:18:17;
 * ifdef'ed TAGS for unresolved ref
 * 
 * revision 1.8
 * date: 1991/05/31 10:58:11;
 * fixed bug in writereg, and
 * made writeregion more like writefile
 * 
 * revision 1.7
 * date: 1991/04/25 12:08:28;
 * use npopen instead of popen for globbing
 * 
 * revision 1.6
 * date: 1991/04/22 08:59:37;
 * fixed globbing to always use /bin/sh
 * 
 * revision 1.5
 * date: 1991/04/08 15:48:59;
 * only update() in readin() if no input pending
 * 
 * revision 1.4
 * date: 1991/04/04 09:36:10;
 * allow for internal callers of ifile()
 * fixed bug with non-unique buffer names at startup
 * 
 * revision 1.3
 * date: 1990/10/01 12:16:11;
 * make mkdir() stuff conditional on ifdef HAVE_MKDIR
 * 
 * revision 1.2
 * date: 1990/09/25 11:38:17;
 * took out old ifdef BEFORE code
 * 
 * revision 1.1
 * date: 1990/09/21 10:25:16;
 * initial vile RCS revision
*/

#include        <stdio.h>
#include	"estruct.h"
#include        "edef.h"

char *strchr();
char *strrchr();

extern int fileispipe;
#if DOSFILES
extern int dosfile;
#endif

/*
 * Read a file into the current
 * buffer. This is really easy; all you do it
 * find the name of the file, and call the standard
 * "read a file into the current buffer" code.
 */
fileread(f, n)
{
        register int    s;
        static char fname[NFILEN];

        if ((s=mlreply("Replace with file: ", fname, NFILEN)) != TRUE)
                return s;
	if ((s = glob(fname)) != TRUE)
		return FALSE;
	/* we want no errors or complaints, so mark it unchanged */
	curbp->b_flag &= ~BFCHG;
        return readin(fname, TRUE, curbp, TRUE);
}

/*
 * Select a file for editing.
 * Look around to see if you can find the
 * file in another buffer; if you can find it
 * just switch to the buffer. If you cannot find
 * the file, create a new buffer, read in the
 * text, and switch to the new buffer.
 * This is ": e"
 */
filefind(f, n)
{
        static char fname[NFILEN];	/* file user wishes to find */
	char nfname[NFILEN];
	char rnfname[NFILEN];
        register int s;		/* status return */
	LINE *lp;

	if (clexec || isnamedcmd) {
	        if ((s=mlreply("Find file: ", fname, NFILEN)) != TRUE)
	                return s;
        } else {
		screen_string(fname,NFILEN,_path);
        }
	if ((s = glob(fname)) != TRUE)
		return FALSE;
	strcpy (nfname, fname);
#if TAGS
	if (othmode & OTH_LAZY) {
		extern BUFFER *filesbp;
		lp = NULL;
		while (flook(nfname, FL_HERE) == NULL) {
			rvstrcpy(rnfname, fname);
			if (makeflist() == FALSE || !sortsearch(rnfname, 
					strlen(rnfname), filesbp, FALSE, &lp)) {
				/* give up, and try what they asked for */
				strcpy (nfname, fname);
				break;
			}
			rvstrncpy(nfname, lp->l_text, llength(lp));
		}
	}
#endif
	return getfile(nfname, TRUE);
}

viewfile(f, n)	/* visit a file in VIEW mode */
{
        char fname[NFILEN];	/* file user wishes to find */
        register int s;		/* status return */

	fname[0] = 0;
        if ((s=mlreply("View file: ", fname, NFILEN)) != TRUE)
                return s;
	if ((s = glob(fname)) != TRUE)
		return FALSE;
	s = getfile(fname, FALSE);
	if (s == TRUE) {	/* if we succeed, put it in view mode */
		make_local_b_val(curwp->w_bufp,MDVIEW);
		set_b_val(curwp->w_bufp,MDVIEW,TRUE);
		markWFMODE(curwp->w_bufp);
	}
	return s;
}

/*
 * Insert a file into the current
 * buffer. This is really easy; all you do it
 * find the name of the file, and call the standard
 * "insert a file into the current buffer" code.
 */
static char insfname[NFILEN];

insfile(f, n)
{
        register int    s;

	if (!calledbefore) {
	        if ((s=mlreply("Insert file: ", insfname, NFILEN)) != TRUE)
	                return s;
		if ((s = glob(insfname)) != TRUE)
			return FALSE;
	}
	if (ukb == 0)
	        return ifile(insfname,TRUE,NULL);
	else
	        return kifile(insfname);
}

getfile(fname, lockfl)
char fname[];		/* file name to find */
int lockfl;		/* check the file for locks? */
{
        register BUFFER *bp;
        register LINE   *lp;
        register int    i;
        register int    s;
        char bname[NBUFN];	/* buffer name to put file */

#if	MSDOS
	mklower(fname);		/* msdos isn't case sensitive */
#endif
        if ((bp=bfind(fname, NO_CREAT, 0)) == NULL) {
		/* it's not already here by that buffer name */
	        for (bp=bheadp; bp!=NULL; bp=bp->b_bufp) {
			/* is it here by that filename? */
	                if (strcmp(bp->b_fname, fname)==0) {
				swbuffer(bp);
	                        lp = curwp->w_dot.l;
	                        i = curwp->w_ntrows/2;
	                        while (i-- && lback(lp)!=curbp->b_line.l)
	                                lp = lback(lp);
	                        curwp->w_line.l = lp;
	                        curwp->w_flag |= WFMODE|WFHARD;
				if (fname[0] != '!') {
		                        mlwrite("[Old buffer]");
				} else {
		                        if (mlyesno(
		                         "Old command output -- rerun")) {
					        return readin(fname, lockfl, 
								curbp, TRUE);
					}
				}
					
	                        return TRUE;
	                }
	        }
		/* it's not here */
	        makename(bname, fname);            /* New buffer name.     */
		/* make sure the buffer name doesn't exist */
	        while ((bp=bfind(bname, NO_CREAT, 0)) != NULL) {
			if ( !(bp->b_flag & BFCHG) && is_empty_buf(bp)) {
				/* empty and unmodified -- then it's okay 
					to re-use this buffer */
				bp->b_active = 0;
				return readin(fname, lockfl, bp, TRUE) &&
						swbuffer(bp);
			}
			/* old buffer name conflict code */
			unqname(bname,TRUE);
	                s = mlreply("Will use buffer name: ", bname, NBUFN);
	                if (s == ABORT)
	                        return s;
			if (s == FALSE || bname[0] == 0)
		                makename(bname, fname);
	        }
		/* okay, we've got a unique name -- create it */
	        if (bp==NULL && (bp=bfind(bname, OK_CREAT, 0))==NULL) {
	                mlwrite("Cannot create buffer");
	                return FALSE;
	        }
		/* switch and read it in. */
	        strcpy(bp->b_fname, fname);
	}
	return swbuffer(bp);
}

/*
	Read file "fname" into a buffer, blowing away any text
	found there.  Returns the final status of the read.
*/

readin(fname, lockfl, bp, mflg)
char    *fname;		/* name of file to read */
int	lockfl;		/* check for file locks? */
register BUFFER *bp;	/* read into this buffer */
int	mflg;		/* print messages? */
{
        register WINDOW *wp;
        register int    s;
        register int    nline;
        int    len;
	char *errst;
	int flag = 0;
#if UNIX
        int    done_update = FALSE;
#endif
#if VMALLOC
	extern int doverifys;
	int odv;
#endif

#if	FILOCK
	if (lockfl && lockchk(fname) == ABORT)
		return ABORT;
#endif
#if	CRYPT
	s = resetkey(bp);
	if (s != TRUE)
		return s;
#endif
        if ((s=bclear(bp)) != TRUE)             /* Might be old.        */
                return s;
        bp->b_flag &= ~(BFINVS|BFCHG);
        strcpy(bp->b_fname, fname);
#if DOSFILES
	make_local_b_val(bp,MDDOS);
	set_b_val(bp, MDDOS, global_b_val(MDDOS) );
#endif

	/* turn off ALL keyboard translation in case we get a dos error */
	TTkclose();

        if ((s = ffropen(fname)) == FIOERR)       /* Hard file open.      */
                goto out;

        if (s == FIOFNF) {                      /* File not found.      */
                if (mflg) mlwrite("[New file]");
                goto out;
        }

        if (mflg) {
		 mlwrite("[Reading %s ]", fname);
	}

#if UNIX & before
	if (fileispipe)
		ttclean(TRUE);
#endif
	/* read the file in */
        nline = 0;
#if VMALLOC
	/* we really think this stuff is clean... */
	odv = doverifys;
	doverifys = 0;
#endif
        while ((s = ffgetline(&len)) == FIOSUC) {
		if (addline(bp,fline,len) != TRUE) {
                        s = FIOMEM;             /* Keep message on the  */
                        break;                  /* display.             */
                } else {
#if UNIX
                	/* reading from a pipe, and internal? */
			if (fileispipe && !ffhasdata()) {
				flag |= WFEDIT;
				if (!done_update || bp->b_nwnd > 1)
					flag |= WFHARD;
			        for (wp=wheadp; wp!=NULL; wp=wp->w_wndp) {
			                if (wp->w_bufp == bp) {
			                        wp->w_line.l=
							lforw(bp->b_line.l);
			                        wp->w_dot.l =
							lback(bp->b_line.l);
			                        wp->w_dot.o = 0;
						wp->w_flag |= flag;
			                }
			        }
				set_b_val(bp, MDDOS, 
					dosfile && global_b_val(MDDOS) );
				curwp->w_flag |= WFMODE;
				update(FALSE);
				done_update = TRUE;
				flag = 0;
			} else {
				flag |= WFHARD;
			}
			
		}
#endif
                ++nline;
        }
#if VMALLOC
	doverifys = odv;
#endif
	bp->b_flag &= ~BFCHG;
#if FINDERR
	if (fileispipe == TRUE) {
		strncpy(febuff,bp->b_bname,NBUFN);
		newfebuff = TRUE;
	}
#endif
        ffclose();                              /* Ignore errors.       */
	if (mflg)
		readlinesmsg(nline,s,fname,ffronly(fname));

	/* set read-only mode for read-only files */
	if (fname[0] == '!' 
#if RONLYVIEW
		|| ffronly(fname) 
#endif
	) {
		make_local_b_val(bp,MDVIEW);
		set_b_val(bp,MDVIEW,TRUE);
	}
	
	bp->b_active = TRUE;

out:
#if DOSFILES
	set_b_val(bp, MDDOS, dosfile && global_b_val(MDDOS) );
#endif
	/* set C mode for C files */
	make_local_b_val(bp,MDCMOD); /* make it local for all, so that
					subsequent changes to global value
					will _not_ affect this buffer */
	set_b_val(bp,MDCMOD,FALSE); /* assume non-C */
	if (global_b_val(MDCMOD)) { 
		char *cp;
		cp = &fname[strlen(fname)-2];
		if (cp >= fname && cp[0] == '.' && 
			strchr(b_val_ptr(bp,VAL_CSUFFIXES),cp[1]) ) {
			/* assumption proven wrong */
			set_b_val(bp,MDCMOD,TRUE);
		}
	}

	TTkopen();	/* open the keyboard again */
        for (wp=wheadp; wp!=NULL; wp=wp->w_wndp) {
                if (wp->w_bufp == bp) {
                        wp->w_line.l = lforw(bp->b_line.l);
                        wp->w_dot.l  = lforw(bp->b_line.l);
                        wp->w_dot.o  = 0;
#ifdef WINMARK
                        wp->w_mark = nullmark;
#endif
                        wp->w_lastdot = nullmark;
                        wp->w_flag |= WFMODE|WFHARD;
                }
        }
        if (s == FIOERR || s == FIOFNF) {	/* False if error.      */
#if UNIX
		extern int sys_nerr, errno;
		extern char *sys_errlist[];
		if (errno > 0 && errno < sys_nerr)
			mlwrite("%s: %s",fname,sys_errlist[errno]);
#endif
                return FALSE;
	}
#if     NeWS
        newsreportmodes() ;
#endif
        return TRUE;
}

/* utility routine for no. of lines read */
readlinesmsg(n,s,f,rdonly)
{
	mlwrite("[%sRead %d line%s from \"%s\"%s]",
     (s==FIOERR ? "I/O ERROR, " : (s == FIOMEM ? "OUT OF MEMORY, ":"")),
	n, n != 1 ? "s":"", f, rdonly ? "  (read-only)":"" );
}

/*
 * Take a file name, and from it
 * fabricate a buffer name. This routine knows
 * about the syntax of file names on the target system.
 * I suppose that this information could be put in
 * a better place than a line of code.
 */

makename(bname, fname)
char    bname[];
char    fname[];
{
        register char *cp2;
        register char *lastsl;

        register char *fcp;
        register char *bcp;

	fcp = &fname[strlen(fname)];
	/* trim trailing whitespace */
	while (fcp != fname && (fcp[-1] == ' ' || fcp[-1] == '\t'
#if UNIX  /* trim trailing slashes as well */
					 || fcp[-1] == '/'
#endif
							) )
                *(--fcp) = '\0';
	fcp = fname;
	/* trim leading whitespace */
	while (*fcp == ' ' || *fcp == '\t')
		fcp++;

#if     AMIGA
        while (cp1!=fcp && cp1[-1]!=':' && cp1[-1]!='/')
                --cp1;
#endif
#if     VMS
        while (cp1!=fcp && cp1[-1]!=':' && cp1[-1]!=']')
                --cp1;
#endif
#if     CPM
        while (cp1!=fcp && cp1[-1]!=':')
                --cp1;
#endif
#if     MSDOS
        while (cp1!=fcp && cp1[-1]!=':' && cp1[-1]!='\\'&&cp1[-1]!='/')
                --cp1;
#endif
#if     ST520
        while (cp1!=fcp && cp1[-1]!=':' && cp1[-1]!='\\')
                --cp1;
#endif
#if     UNIX
	bcp = bname;
	if (*fcp == '!') { /* then it's a shell command.  bname is first word */
		*bcp++ = '!';
		while (isspace(*++fcp))
			;
		while (!isspace(*fcp) && bcp < &bname[NBUFN-1])
			*bcp++ = *fcp++;
		*bcp = '\0';
		return;
	}
	if (lastsl = strrchr(fcp,'/')) {
		strncpy(bcp,lastsl+1,NBUFN);
		bcp[NBUFN-1] = '\0';
	} else {  /* no slashes, use the filename as is */
		strncpy(bcp,fcp,NBUFN);
		bcp[NBUFN-1] = '\0';
	}
	return;

#else
        cp2 = &bname[0];
        while (cp2!=&bname[NBUFN-1] && *cp1!=0 && *cp1!=';')
                *cp2++ = *cp1++;
        *cp2 = 0;
#endif
}

unqname(name,ok_to_ask)	/* make sure a buffer name is unique */
char *name;	/* name to check on */
{
	register char *sp;

	/* check to see if it is in the buffer list */
	while (bfind(name, 0, NO_CREAT) != NULL) {

		sp = &name[strlen(name)-1];  /* last char */
		if (sp - name >= 2 && sp[-1] == '-') {
			if (sp[0] == '9')
				sp[0] = 'A';
			else if (sp[0] == 'Z')
				goto choosename;
			else if (isdigit(sp[0]) || isupper(sp[0]))
				sp[0] += 1;
		} else if (sp + 2 < &name[NBUFN-1])  {
			strcat(sp, "-1");
		} else {
		choosename:
			if (ok_to_ask) {
				do {
					mlreply("Choose a unique buffer name: ",
						 name, NBUFN);
				} while (name[0] == '\0');
			} else { /* can't ask, just overwrite end of name */
				sp[-1] = '-';
				sp[0] = '1';
			}
		}
	}
}

/*
 * Ask for a file name, and write the
 * contents of the current buffer to that file.
 */
filewrite(f, n)
{
        register int    s;
        static char            fname[NFILEN];

	strncpy(fname, curbp->b_fname, NFILEN);
	
	/* HACK -- this implies knowledge of how kbd_engl works! */
	if (isnamedcmd && lastkey != '\r') {
	        if ((s=mlreply("Write to file: ", fname, NFILEN)) != TRUE)
	                return s;
		if ((s = glob(fname)) != TRUE)
			return FALSE;
		if (strcmp(fname,curbp->b_fname) &&
			fname[0] != '!' && flook(fname,FL_HERE)) {
			if (mlyesno("File exists, okay to overwrite") != TRUE) {
				mlwrite("File not written");
				return FALSE;
			}
		}
        }
	if (!strcmp(fname,curbp->b_fname) && b_val(curbp,MDVIEW)) {
		mlwrite("[Can't write-back from view mode]");
		return FALSE;
	}
        if ((s=writeout(fname,curbp,TRUE)) == TRUE) {
                curbp->b_flag &= ~BFCHG;
		markWFMODE(curbp);
        }
        return s;
}

/*
 * Save the contents of the current
 * buffer in its associatd file. Do nothing
 * if nothing has changed (this may be a bug, not a
 * feature). Error if there is no remembered file
 * name for the buffer.
 */
filesave(f, n)
{
        register int    s;

#if its_a_bug
        if ((curbp->b_flag&BFCHG) == 0)         /* Return, no changes.  */
                return TRUE;
#endif
        if (curbp->b_fname[0] == 0) {           /* Must have a name.    */
                mlwrite("No file name");
                return FALSE;
        }
        if ((s=writeout(curbp->b_fname,curbp,TRUE)) == TRUE) {
                curbp->b_flag &= ~BFCHG;
		markWFMODE(curbp);
        }
        return s;
}

/*
 * This function performs the details of file
 * writing. Uses the file management routines in the
 * "fileio.c" package. The number of lines written is
 * displayed. Sadly, it looks inside a LINE; provide
 * a macro for this. Most of the grief is error
 * checking of some sort.
 */
writeout(fn,bp,msgf)
char    *fn;
BUFFER *bp;
{
        register LINE   *lp;		/* current line */
        register long   numchars;	/* # of chars in file */
        REGION region;

	/* starting at the beginning of the buffer */
        lp = lforw(bp->b_line.l);
        region.r_orig.l = lp;
        region.r_orig.o = 0;

	/* start counting chars */
        numchars = 0;
        while (lp != bp->b_line.l) {
		numchars += llength(lp) + 1;
		lp = lforw(lp);
        }
        region.r_size = numchars;
        region.r_end = bp->b_line;
        
#if DOSFILES
	dosfile = b_val(bp, MDDOS);
#endif

	return writereg(&region,fn,msgf);
}

writeregion(f,n)
{
        REGION region;
	int s;
        static char fname[NFILEN];

	if (isnamedcmd && lastkey == '\r') {
		strncpy(fname, curbp->b_fname, NFILEN);

		if (mlyesno("Okay to write [possible] partial range") != TRUE) {
			mlwrite("Range not written");
			return FALSE;
		}
	} else {
		/* HACK -- this implies knowledge of 
					how kbd_engl works! */
	        if ((s=mlreply("Write region to file: ", fname, NFILEN))
							 != TRUE)
	                return s;
		if ((s = glob(fname)) != TRUE)
			return FALSE;
		if (strcmp(fname,curbp->b_fname) &&
			fname[0] != '!' && flook(fname,FL_HERE)) {
			if (mlyesno("File exists, okay to overwrite")
							!= TRUE) {
				mlwrite("File not written");
				return FALSE;
			}
		}
        }
        if ((s=getregion(&region,NULL)) != TRUE)
                return s;
#if DOSFILES
	dosfile = b_val(curbp, MDDOS);
#endif
	s = writereg(&region,fname,TRUE);
        return s;
}


writereg(rp,fn,msgf)
REGION *rp;
char    *fn;
{
        register int    s;
        register LINE   *lp;
        register int    nline;
	register int i;
	long lim;
	long nchar;

#if	CRYPT
	s = resetkey(curbp);
	if (s != TRUE)
		return s;
#endif
        if (*fn == '[' || *fn == ' ') {
        	mlwrite("No filename");
        	return FALSE;
        }
        
        if ((s=ffwopen(fn)) != FIOSUC)       /* Open writes message. */
                return FALSE;

	/* tell us we're writing */
	if (msgf == TRUE)
		mlwrite("[Writing...]");

#if UNIX & ! NeWS
	if (fileispipe)
		ttclean(TRUE);
#else
	TTkclose();
#endif

        lp = rp->r_orig.l;
        nline = 0;                              /* Number of lines     */
        nchar = 0;                              /* Number of chars     */

	/* First and maybe only line. */
	if (rp->r_orig.o <= llength(lp)) {
		if ((lim = rp->r_orig.o+rp->r_size) > llength(lp))
			lim = (long)llength(lp);
		for (i = rp->r_orig.o; i < lim; i++) {
		        if ((s=ffputc(lgetc(lp,i))) != FIOSUC)
		                goto out;
			nchar++;
		}
		rp->r_size -= nchar;

		if (rp->r_size <= 0)
			goto out;

	        if ((s=ffputc('\n')) != FIOSUC)
	                goto out;

		nchar++;
		nline++;
		rp->r_size--;
                lp = lforw(lp);

	}

	/* whole lines */
        while (rp->r_size >= llength(lp)+1) {
                if ((s=ffputline(&lp->l_text[0], llength(lp))) != FIOSUC)
                        goto out;
                ++nline;
		nchar += llength(lp) + 1;
		rp->r_size -= llength(lp) + 1;
                lp = lforw(lp);
        }

	/* last line */
	if (rp->r_size > 0) {
		lim = rp->r_size;
		for (i = 0; i < lim; i++) {
		        if ((s=ffputc(lgetc(lp,i))) != FIOSUC)
		                goto out;
			nchar++;
			rp->r_size--;
		}
	}
	if (rp->r_size != 0)
		mlwrite("Bug: writereg, rsize == %d",rp->r_size);

 out:
        if (s == FIOSUC) {                      /* No write error.      */
                s = ffclose();
                if (s == FIOSUC && msgf) {      /* No close error.      */
	                mlwrite("[Wrote %d line%s %d char%s to %s]", 
				nline, (nline>1)?"s":"",
				nchar, (nchar>1)?"s":"", fn);
                }
        } else {                                /* Ignore close error   */
                ffclose();                      /* if a write error.    */
	}
#if UNIX & ! NeWS
	if (fileispipe == TRUE) {
		ttunclean();
	        TTflush();
		sgarbf = TRUE;
		pressreturn();
	}
#else
	TTkopen();
#endif
        if (s != FIOSUC)                        /* Some sort of error.  */
                return FALSE;
        return TRUE;
}

/*
 * This function writes the kill register to a file
 * Uses the file management routines in the
 * "fileio.c" package. The number of lines written is
 * displayed.
 */
kwrite(fn,msgf)
char    *fn;
{
	register KILL *kp;		/* pointer into kill register */
	register int	nline;
	register int	s;
	register int	c;
	register int	i;
	register char	*sp;	/* pointer into string to insert */

	/* make sure there is something to put */
	if (kbs[ukb].kbufh == NULL) {
		if (msgf) mlwrite("Nothing to write");
		return FALSE;		/* not an error, just nothing */
	}

#if	CRYPT
	s = resetkey(curbp);
	if (s != TRUE)
		return s;
#endif
	/* turn off ALL keyboard translation in case we get a dos error */
	TTkclose();

	if ((s=ffwopen(fn)) != FIOSUC) {	/* Open writes message. */
		TTkopen();
		return FALSE;
	}
	/* tell us we're writing */
	if (msgf == TRUE)
		mlwrite("[Writing...]");
	nline = 0;				/* Number of lines.	*/

	kp = kbs[ukb].kbufh;
	while (kp != NULL) {
		if (kp->d_next == NULL)
			i = kbs[ukb].kused;
		else
			i = KBLOCK;
		sp = kp->d_chunk;
		while (i--) {
			if ((c = *sp++) == '\n')
				nline++;
			if ((s = ffputc(c)) != FIOSUC)
				break;
		}
		kp = kp->d_next;
	}
	if (s == FIOSUC) {			/* No write error.	*/
		s = ffclose();
		if (s == FIOSUC && msgf) {	/* No close error.	*/
			mlwrite("[Wrote %d line%s to %s ]",
					nline,nline!=1?"s":"", fn);
		}
	} else	{				/* Ignore close error	*/
		ffclose();			/* if a write error.	*/
	}
	TTkopen();
	if (s != FIOSUC)			/* Some sort of error.	*/
		return FALSE;
	return TRUE;
}


/*
 * The command allows the user
 * to modify the file name associated with
 * the current buffer. It is like the "f" command
 * in UNIX "ed". The operation is simple; just zap
 * the name in the BUFFER structure, and mark the windows
 * as needing an update. You can type a blank line at the
 * prompt if you wish.
 */
filename(f, n)
{
        register int    s;
        static char            fname[NFILEN];

        if ((s=mlreply("Name: ", fname, NFILEN)) == ABORT)
                return s;
	if ((s = glob(fname)) != TRUE)
		return FALSE;
        if (s == FALSE)
                strcpy(curbp->b_fname, "");
        else
                strcpy(curbp->b_fname, fname);
	make_global_b_val(curbp,MDVIEW); /* no longer read only mode */
	markWFMODE(curbp);
        return TRUE;
}

/*
 * Insert file "fname" into the current
 * buffer, Called by insert file command. Return the final
 * status of the read.
 */
ifile(fname,belowthisline,haveffp)
char    fname[];
FILE *haveffp;
{
        register LINE   *lp0;
        register LINE   *lp1;
        register LINE   *lp2;
        register int    i;
        register BUFFER *bp;
        register int    s;
        int    nbytes;
        register int    nline;
	char mesg[NSTRING];
	extern FILE	*ffp;

        bp = curbp;                             /* Cheap.               */
        bp->b_flag |= BFCHG;			/* we have changed	*/
	bp->b_flag &= ~BFINVS;			/* and are not temporary*/
	if (!haveffp) {
	        if ((s=ffropen(fname)) == FIOERR)       /* Hard file open.      */
	                goto out;
	        if (s == FIOFNF) {                      /* File not found.      */
	                mlwrite("[No such file \"%s\" ]", fname);
			return FALSE;
	        }
	        mlwrite("[Inserting...]");
#if UNIX
		if (fileispipe)
			ttclean(TRUE);
#endif

#if	CRYPT
		s = resetkey(curbp);
		if (s != TRUE)
			return s;
#endif
	} else { /* we already have the file pointer */
		ffp = haveffp;
	}
	lp0 = curwp->w_dot.l;
	curwp->w_dot.o = 0;
	MK = DOT;

	nline = 0;
	while ((s=ffgetline(&nbytes)) == FIOSUC) {
		if ((lp1=lalloc(nbytes)) == NULL) {
			s = FIOMEM;		/* Keep message on the	*/
			break;			/* display.		*/
		}
		if (belowthisline) {
			lp2 = lp0->l_fp;	/* line after insert */
		} else {
			lp2 = lp0;
			lp0 = lp0->l_bp;
		}

		/* re-link new line between lp0 and lp2 */
		lp2->l_bp = lp1;
		lp0->l_fp = lp1;
		lp1->l_bp = lp0;
		lp1->l_fp = lp2;

		if (nbytes)  /* l_text may be NULL in this case */
			memcpy(lp1->l_text, fline, nbytes);
		tag_for_undo(lp1);
		if (belowthisline)
			lp0 = lp1;
		else
			lp0 = lp2;
		++nline;
	}
	if (!haveffp) {
#if UNIX
		if (fileispipe == TRUE) {
			ttunclean();
			TTflush();
			sgarbf = TRUE;
		}
#endif
		ffclose();				/* Ignore errors.	*/
		readlinesmsg(nline,s,fname,FALSE);
	}
out:
	/* advance to the next line and mark the window for changes */
	curwp->w_dot.l = lforw(curwp->w_dot.l);
	curwp->w_flag |= WFHARD | WFMODE;

	/* copy window parameters back to the buffer structure */
	curbp->b_wtraits = curwp->w_traits;

        if (s == FIOERR)                        /* False if error.      */
                return FALSE;
        return TRUE;
}

/*
 * Insert file "fname" into the kill register
 * Called by insert file command. Return the final
 * status of the read.
 */
kifile(fname)
char    fname[];
{
        register int    i;
        register int    s;
        register int    nline;
        int    nbytes;

	kdelete();
        if ((s=ffropen(fname)) == FIOERR)       /* Hard file open.      */
                goto out;
        if (s == FIOFNF) {                      /* File not found.      */
                mlwrite("[No such file \"%s\"]", fname);
		return FALSE;
        }
        mlwrite("[Reading...]");

#if UNIX
	if (fileispipe)
		ttclean(TRUE);
#endif

#if	CRYPT
	s = resetkey(curbp);
	if (s != TRUE)
		return s;
#endif
        nline = 0;
        while ((s=ffgetline(&nbytes)) == FIOSUC) {
                for (i=0; i<nbytes; ++i)
                        kinsert(fline[i]);
                kinsert('\n');
                ++nline;
        }
#if UNIX
	if (fileispipe == TRUE) {
		ttunclean();
	        TTflush();
	        sgarbf = TRUE;
	}
#endif
        ffclose();                              /* Ignore errors.       */
	readlinesmsg(nline,s,fname,FALSE);

out:
        if (s == FIOERR)                        /* False if error.      */
                return FALSE;
        return TRUE;
}

#if UNIX

/* called on hangups, interrupts, and quits */
/* This code is definitely not production quality, or probably very
	robust, or probably very secure.  I whipped it up to save
	myself while debugging...		pgf */
imdying(signo)
{
#if HAVE_MKDIR
	static char dirnam[NSTRING] = "/tmp/vileDXXXXXX";
#else
	static char dirnam[NSTRING] = "/tmp";
#endif
	char filnam[50];
	char cmd[80];
	BUFFER *bp;
	char *np;
	int wrote = 0;
	int created = 0;
	char *getenv();
	char *mktemp();


	bp = bheadp;
	while (bp != NULL) {
		if (((bp->b_flag & BFINVS) == 0) && 
			 bp->b_active == TRUE && 
	                 (bp->b_flag&BFCHG) != 0) {
#if HAVE_MKDIR
			if (!created) {
				(void)mktemp(dirnam);
				if(mkdir(dirnam,0700) != 0) {
					vttidy(FALSE);
					exit(1);
				}
				created = 1;
			}
#endif
			strcpy(filnam,dirnam);
			strcat(filnam,"/");
#if ! HAVE_MKDIR
			strcat(filnam,"V");
#endif
			strcat(filnam,bp->b_bname);
			if (writeout(filnam,bp,FALSE) != TRUE) {
				vttidy(FALSE);
				exit(1);
			}
			wrote++;
		}
		bp = bp->b_bufp;
	}
	if (wrote) {
		if ((np = getenv("LOGNAME")) || (np = getenv("USER"))) {
			lsprintf(cmd,
#if HAVE_MKDIR
    "(echo Subject: vile died; echo Files saved: ; ls %s/* ) | /bin/mail %s",
#else
    "(echo Subject: vile died; echo Files saved: ; ls %s/V* ) | /bin/mail %s",
#endif
				dirnam, np);
			system(cmd);
		}
	}
	vttidy(FALSE);
	if (signo > 2) abort();
	exit(wrote);
}
#endif

markWFMODE(bp)
BUFFER *bp;
{
	register WINDOW *wp;	/* scan for windows that need updating */
        wp = wheadp;                    /* Update mode lines.   */
        while (wp != NULL) {
                if (wp->w_bufp == bp)
                        wp->w_flag |= WFMODE;
                wp = wp->w_wndp;
        }
}

/* use the shell to expand wildcards */
glob(buf)
char *buf;
{
#if UNIX
	char *cp;
	char cmd[NFILEN+50];
	FILE *cf;
	FILE *npopen();

	/* trim trailing whitespace */
	cp = &buf[strlen(buf)-1];
	while (cp != buf) {
		if (isspace(*cp))
			*cp = '\0';
		else
			break;
		cp--;
	}

	cp = buf;
	if (*cp == '!' || *cp == '[')	/* it's a shell command, or an */
		return TRUE;		/* internal name, don't bother */

	while (*cp) {
		if (iswild(*cp)) {
			lsprintf(cmd, "echo %s", buf);
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
						*cp = '\0';
						break;
					} else {
						buf[0] = 0;
						return FALSE;
					}
				} else if (*cp == '\n') {
					*cp = '\0';
					break;
				}
				cp++;
			}
			return TRUE;

		}
		cp++;
	}
	return TRUE;
#endif
}

#if	CRYPT
resetkey(bp)	/* reset the encryption key if needed */
BUFFER *bp;
{
	register int s;	/* return status */

	/* turn off the encryption flag */
	cryptflag = FALSE;

	/* if we are in crypt mode */
	if (b_val(bp, MDCRYPT)) {
		if (bp->b_key[0] == 0) {
			s = setkey(FALSE, 0);
			if (s != TRUE)
				return s;
		}

		/* let others know... */
		cryptflag = TRUE;

		/* and set up the key to be used! */
		/* de-encrypt it */
		crypt((char *)NULL, 0);
		crypt(bp->b_key, strlen(bp->b_key));

		/* re-encrypt it...seeding it to start */
		crypt((char *)NULL, 0);
		crypt(bp->b_key, strlen(bp->b_key));
	}

	return TRUE;
}
#endif

