/*
 * The functions in this file negotiate with the operating system for
 * characters, and write characters in a barely buffered fashion on the display.
 * All operating systems.
 *
 * $Log: termio.c,v $
 * Revision 1.99  1994/04/25 21:07:13  pgf
 * changes for ANSI screen under MSDOS
 *
 * Revision 1.98  1994/04/22  14:34:15  pgf
 * changed BAD and GOOD to BADEXIT and GOODEXIT
 *
 * Revision 1.97  1994/04/20  19:54:50  pgf
 * changes to support 'BORLAND' console i/o screen driver
 *
 * Revision 1.96  1994/04/18  14:26:27  pgf
 * merge of OS2 port patches, and changes to tungetc operation
 *
 * Revision 1.94  1994/04/13  20:56:25  pgf
 * added SONY to sys/ioctl list
 *
 * Revision 1.93  1994/02/22  11:03:15  pgf
 * truncated RCS log for 4.0
 *
 */
#include	"estruct.h"
#include        "edef.h"

#if DJGPP
# include <pc.h>   /* for kbhit() */
#endif

#if UNIX

/* there are three copies of the tt...() routines here -- one each for
	POSIX termios, traditional termio, and sgtty.  If you have a
	choice, I recommend them in that order. */

/* ttopen() and ttclose() are responsible for putting the terminal in raw
	mode, setting up terminal signals, etc.
   ttclean() prepares the terminal for shell escapes, and ttunclean() gets
   	us back into vile's mode
*/

/* I suppose this config stuff should move to estruct.h */

#if POSIX
# define USE_POSIX_TERMIOS 1
# define USE_FCNTL 1
#else
# if USG
#  define USE_TERMIO 1
#  define USE_FCNTL 1
# else
#  if BERK || V7
#   define USE_SGTTY 1
#   define USE_FIONREAD 1
#  else
 huh?
#  endif
# endif
#endif

#if OSF1		/* don't know why termios doesn't work in osf */
# undef POSIX
# undef BERK
# define BERK 1
# undef USE_FIONREAD	/* because of osf screen refresh problems */
#endif

#if (BERK || AIX || AUX2) && !defined(FIONREAD)
/* try harder to get it */
# if SUNOS
#  include "sys/filio.h"
# else /* if you have trouble including ioctl.h, try "sys/ioctl.h" instead */
#  if APOLLO || AIX || OSF1 || ULTRIX || AUX2 || NETBSD || SONY
#   include <sys/ioctl.h>
#  else
#   include <ioctl.h>
#  endif
# endif
#endif

#if X11	/* don't use either one */
# undef USE_FCNTL
# undef USE_FIONREAD
#else
# if defined(FIONREAD)
  /* there seems to be a bug in someone's implementation of fcntl -- it
   * causes output to be flushed if you change to ndelay input while output
   * is pending.  for these systems, we use FIONREAD instead, if possible. 
   * In fact, if try and use FIONREAD in any case if present.  If you have
   * the problem with fcntl, you'll notice that the screen doesn't always
   * refresh correctly, as if the fcntl is interfering with the output drain
   */
#  undef USE_FCNTL
#  define USE_FIONREAD 1
# endif
#endif

#if USE_FCNTL
/* this is used to determine whether input is pending from the user */
#include	<fcntl.h>
int kbd_flags;			/* saved keyboard fcntl flags	*/
int kbd_is_polled;		/* in O_NDELAY mode?		*/
int kbd_char_present;		/* there is a char in kbd_char	*/
char kbd_char;			/* the char we've already read	*/
#endif

#define SMALL_STDOUT 1

#if defined(SMALL_STDOUT) && (defined (USE_FCNTL) || defined(USE_FIONREAD))
#define	TBUFSIZ	128 /* Provide a smaller terminal output buffer so that
	   the type-ahead detection works better (more often).
	   That is, we overlap screen writing with more keyboard polling */
#else
#define	TBUFSIZ	1024	/* reduces the number of writes */
#endif

extern CMDFUNC f_backchar;
extern CMDFUNC f_backchar_to_bol;


#if USE_POSIX_TERMIOS

#include <sys/param.h>		/* defines 'VDISABLE' */
#include <termios.h>

#ifndef VDISABLE
# define VDISABLE '\0'
#endif

struct termios otermios, ntermios;

char tobuf[TBUFSIZ];		/* terminal output buffer */

void
ttopen()
{
	int s;
	s = tcgetattr(0, &otermios);
	if (s < 0) {
		perror("ttopen tcgetattr");
		ExitProgram(BADEXIT);
	}
 /* the "|| USG" below is to support SVR4 -- let me know if
 	it conflicts on other systems */
#if ODT || ISC || USG
	setvbuf(stdout, tobuf, _IOLBF, TBUFSIZ);
#else
  	setbuffer(stdout, tobuf, TBUFSIZ);
#endif

	suspc =   otermios.c_cc[VSUSP];
	intrc =   otermios.c_cc[VINTR];
	killc =   otermios.c_cc[VKILL];
	startc =  otermios.c_cc[VSTART];
	stopc =   otermios.c_cc[VSTOP];
	backspc = otermios.c_cc[VERASE];
#ifdef VWERASE  /* Sun has it.  any others? */
	wkillc = otermios.c_cc[VWERASE];
#else
	wkillc =  tocntrl('W');
#endif

	/* this could probably be done more POSIX'ish? */
	(void)signal(SIGTSTP,SIG_DFL);		/* set signals so that we can */
	(void)signal(SIGCONT,rtfrmshell);	/* suspend & restart */
	(void)signal(SIGTTOU,SIG_IGN);		/* ignore output prevention */

#if USE_FCNTL
	kbd_flags = fcntl( 0, F_GETFL, 0 );
	kbd_is_polled = FALSE;
#endif

#if ! X11
	ntermios = otermios;

	/* new input settings: turn off crnl mapping, cr-ignoring,
	 * case-conversion, and allow BREAK
	 */
	ntermios.c_iflag = BRKINT | (otermios.c_iflag & 
				~(INLCR|IGNCR|ICRNL|IUCLC));

	ntermios.c_oflag = 0;
	ntermios.c_lflag = ISIG;
	ntermios.c_cc[VMIN] = 1;
	ntermios.c_cc[VTIME] = 0;
#ifdef	VSWTCH
	ntermios.c_cc[VSWTCH] = VDISABLE;
#endif
	ntermios.c_cc[VSUSP]  = VDISABLE;
	ntermios.c_cc[VSTART] = VDISABLE;
	ntermios.c_cc[VSTOP]  = VDISABLE;
#endif

	ttmiscinit();

	ttunclean();


}

void
ttclose()
{
	ttclean(TRUE);
}

/*ARGSUSED*/
void
ttclean(f)
int f;
{
#if !X11
	if (f) {
		movecursor(term.t_nrow, ttcol); /* don't care about column */
		TTputc('\n');
		TTputc('\r');
	}
	(void)fflush(stdout);
#if ! LINUX
	tcdrain(1);
#endif
	tcsetattr(0, TCSADRAIN, &otermios);
	TTkclose();	/* xterm */
	TTclose();
	TTflush();
#if USE_FCNTL
	fcntl(0, F_SETFL, kbd_flags);
	kbd_is_polled = FALSE;
#endif
#endif
}

void
ttunclean()
{
#if ! X11
#if ! LINUX
	tcdrain(1);
#endif
	tcsetattr(0, TCSADRAIN, &ntermios);
#endif
	TTkopen();	/* xterm */
}


#endif /* USE_POSIX_TERMIOS */

#if USE_TERMIO

#include	<termio.h>

/* original terminal characteristics and characteristics to use inside */
struct	termio	otermio, ntermio;

#ifdef AVAILABLE  /* setbuffer() isn't on most termio systems */
char tobuf[TBUFSIZ];		/* terminal output buffer */
#endif

void
ttopen()
{

	ioctl(0, TCGETA, (char *)&otermio);	/* save old settings */
#ifdef AVAILABLE
	setbuffer(stdout, tobuf, TBUFSIZ);
#endif

	intrc =   otermio.c_cc[VINTR];
	killc =   otermio.c_cc[VKILL];
	startc =  tocntrl('Q');
	stopc =   tocntrl('S');
	backspc = otermio.c_cc[VERASE];
	wkillc =  tocntrl('W');

#if USE_FCNTL
	kbd_flags = fcntl( 0, F_GETFL, 0 );
	kbd_is_polled = FALSE;
#endif

#if SIGTSTP
/* be careful here -- VSUSP is sometimes out of the range of the c_cc array */
# ifdef VSUSP /* ODT (all POSIX?) uses this... */
	ntermio.c_cc[VSUSP] = -1;
	ntermio.c_cc[VSTART] = -1;
	ntermio.c_cc[VSTOP] = -1;
	suspc =   otermio.c_cc[VSUSP];
# else /* use V_SUSP */
#  ifdef V_SUSP
	ntermio.c_cc[V_SUSP] = -1;
	suspc = otermio.c_cc[V_SUSP];
#  else
	suspc = -1;
#  endif
#  ifdef V_DSUSP
	ntermio.c_cc[V_DSUSP] = -1;
#  endif
# endif

	(void)signal(SIGTSTP,SIG_DFL);		/* set signals so that we can */
	(void)signal(SIGCONT,rtfrmshell);	/* suspend & restart */
	(void)signal(SIGTTOU,SIG_IGN);		/* ignore output prevention */

#else /* no SIGTSTP */
	suspc =   tocntrl('Z');
#endif

#if ! X11
	ntermio = otermio;

	/* setup new settings, preserve flow control, and allow BREAK */
	ntermio.c_iflag = BRKINT|(otermio.c_iflag & IXON|IXANY|IXOFF);
	ntermio.c_oflag = 0;
	ntermio.c_lflag = ISIG;
	ntermio.c_cc[VMIN] = 1;
	ntermio.c_cc[VTIME] = 0;
#ifdef	VSWTCH
	ntermio.c_cc[VSWTCH] = -1;
#endif
#endif

	ttmiscinit();
	ttunclean();

}

void
ttclose()
{
	ttclean(TRUE);
}

void
ttclean(f)
int f;
{
#if ! X11
	if (f) {
		movecursor(term.t_nrow, ttcol); /* don't care about column */
		TTputc('\n');
		TTputc('\r');
	}
	(void)fflush(stdout);
	TTkclose();	/* xterm */
	TTflush();
	TTclose();
	ioctl(0, TCSETAF, (char *)&otermio);
#if USE_FCNTL
	fcntl(0, F_SETFL, kbd_flags);
	kbd_is_polled = FALSE;
#endif
#endif	/* X11 */
}

void
ttunclean()
{
#if ! X11
	ioctl(0, TCSETAW, (char *)&ntermio);
#endif
	TTkopen();	/* xterm */
}

#endif /* USE_TERMIO */

#if USE_SGTTY

#if USE_FIONREAD
char tobuf[TBUFSIZ];		/* terminal output buffer */
#endif

#undef	CTRL
#include        <sgtty.h>        /* for stty/gtty functions */
struct  sgttyb  ostate;          /* saved tty state */
struct  sgttyb  nstate;          /* values for editor mode */
struct  sgttyb  rnstate;          /* values for raw editor mode */
int olstate;		/* Saved local mode values */
int nlstate;		/* new local mode values */
struct ltchars	oltchars;	/* Saved terminal special character set */
struct ltchars	nltchars = { -1, -1, -1, -1, -1, -1 }; /* a lot of nothing */
struct tchars	otchars;	/* Saved terminal special character set */
struct tchars	ntchars; /*  = { -1, -1, -1, -1, -1, -1 }; */

void
ttopen()
{
	ioctl(0,TIOCGETP,(char *)&ostate); /* save old state */
	killc = ostate.sg_kill;
	backspc = ostate.sg_erase;

#if ! X11
	nstate = ostate;
        nstate.sg_flags |= CBREAK;
        nstate.sg_flags &= ~(ECHO|CRMOD);       /* no echo for now... */
	ioctl(0,TIOCSETN,(char *)&nstate); /* set new state */
#endif

	rnstate = nstate;
        rnstate.sg_flags &= ~CBREAK;
        rnstate.sg_flags |= RAW;

	ioctl(0, TIOCGETC, (char *)&otchars);	/* Save old characters */
	intrc =  otchars.t_intrc;
	startc = otchars.t_startc;
	stopc =  otchars.t_stopc;

#if ! X11
	ntchars = otchars;
	ntchars.t_brkc = -1;
	ntchars.t_eofc = -1;
	ioctl(0, TIOCSETC, (char *)&ntchars);	/* Place new character into K */
#endif

	ioctl(0, TIOCGLTC, (char *)&oltchars);	/* Save old characters */
	wkillc = oltchars.t_werasc;
	suspc = oltchars.t_suspc;
#if ! X11
	ioctl(0, TIOCSLTC, (char *)&nltchars);	/* Place new character into K */
#endif

#ifdef	TIOCLGET
	ioctl(0, TIOCLGET, (char *)&olstate);
#if ! X11
	nlstate = olstate;
	nlstate |= LLITOUT;
	ioctl(0, TIOCLSET, (char *)&nlstate);
#endif
#endif
#if USE_FIONREAD
	setbuffer(stdout, tobuf, TBUFSIZ);
#endif
#if ! X11

	(void)signal(SIGTSTP,SIG_DFL);		/* set signals so that we can */
	(void)signal(SIGCONT,rtfrmshell);	/* suspend & restart */
	(void)signal(SIGTTOU,SIG_IGN);		/* ignore output prevention */

#endif

	ttmiscinit();
}

void
ttclose()
{
	ttclean(TRUE);
}

void
ttclean(f)
int f;
{
#if ! X11
	if (f) {
		movecursor(term.t_nrow, ttcol); /* don't care about column */
		TTputc('\n');
		TTputc('\r');
	}
	TTflush();
	TTkclose();	/* xterm */
	TTclose();
	ioctl(0, TIOCSETN, (char *)&ostate);
	ioctl(0, TIOCSETC, (char *)&otchars);
	ioctl(0, TIOCSLTC, (char *)&oltchars);
#ifdef	TIOCLSET
	ioctl(0, TIOCLSET, (char *)&olstate);
#endif
#if APOLLO
	TTflush();
#endif
#endif
}

void
ttunclean()
{
#if ! X11
#if APOLLO
	int literal = LLITOUT;

	(void)fflush(stdout);
	ioctl(0, TIOCLSET, (caddr_t)&olstate);
	ioctl(0, TIOCSETP, (caddr_t)&nstate);	/* setting nlstate changes sb_flags */
	TTflush();
	ioctl(0, TIOCLBIS, (caddr_t)&literal);	/* set this before nltchars! */
	ioctl(0, TIOCSETC, (caddr_t)&ntchars);
	ioctl(0, TIOCSLTC, (caddr_t)&nltchars);

#else

	ioctl(0, TIOCSETN, (char *)&nstate);
	ioctl(0, TIOCSETC, (char *)&ntchars);
	ioctl(0, TIOCSLTC, (char *)&nltchars);
#ifdef	TIOCLSET
	ioctl(0, TIOCLSET, (char *)&nlstate);
#endif

#endif	/* APOLLO */
	TTkopen();	/* xterm */
#endif	/* !X11 */
}

#endif /* USE_SGTTY */

void
ttputc(c)
int c;
{
        (void)putchar(c);
}

void
ttflush()
{
        (void)fflush(stdout);
}

/*
 * Read a character from the terminal, performing no editing and doing no echo
 * at all.
 */
int
ttgetc()
{
#if	USE_FCNTL
	int n;
	if( kbd_char_present ) {
		kbd_char_present = FALSE;
	} else {
		if( kbd_is_polled && fcntl( 0, F_SETFL, kbd_flags ) < 0 )
			imdying(2);
		kbd_is_polled = FALSE;
		n = read(0, &kbd_char, 1);
		if (n <= 0) {
			if (n < 0 && errno == EINTR)
				return -1;
			imdying(2);
		}
	}
	return ( kbd_char );
#else /* USE_FCNTL */
#if APOLLO
	/*
	 * If we try to read a ^C in cooked mode it will echo anyway.  Also,
	 * the 'getchar()' won't be interruptable.  Setting raw-mode
	 * temporarily here still allows the program to be interrupted when we
	 * are not actually looking for a character.
	 */
	int	c;
	ioctl(0, TIOCSETN, (char *)&rnstate);
        c = getchar();
	ioctl(0, TIOCSETN, (char *)&nstate);
#else
	int c;
	c = getchar();
#endif
	if (c == EOF) {
		if (errno == EINTR)
			return -1;
		imdying(2);
	}
	return c;
#endif
}


/* typahead:	Check to see if any characters are already in the
		keyboard buffer
*/
int
typahead()
{
#if X11
#if BEFORE
this seems to think there is something ready more often than there really is
	return x_key_events_ready();
#else
	return x_is_pasting() || did_tungetc();
#endif
#else

	if (did_tungetc())
		return TRUE;

# if	USE_FIONREAD
	{
	long x;
	return((ioctl(0,FIONREAD,(caddr_t)&x) < 0) ? 0 : (int)x);
	}
# else
#  if	USE_FCNTL
	if( !kbd_char_present )
	{
		if( !kbd_is_polled &&
				fcntl(0, F_SETFL, kbd_flags|O_NDELAY ) < 0 )
			return(FALSE);
		kbd_is_polled = TRUE;  /* I think */
		kbd_char_present = (1 == read( 0, &kbd_char, 1 ));
	}
	return ( kbd_char_present );
#  else
	return FALSE;
#  endif/* USE_FCNTL */
# endif/* USE_FIONREAD */
#endif	/* X11 */
}

/* this takes care of some stuff that's common across all ttopen's.  Some of
	it should arguably be somewhere else, but... */
void
ttmiscinit()
{
	/* make sure backspace is bound to backspace */
	asciitbl[backspc] = &f_backchar_to_bol;

	/* no buffering on input */
	setbuf(stdin, (char *)0);
}

#if defined(SA_RESTART)
/*
 * Redefine signal in terms of sigaction for systems which have the
 * SA_RESTART flag defined through <signal.h>
 *
 * This definition of signal will cause system calls to get restarted for a
 * more BSD-ish behavior.  This will allow us to use the OPT_WORKING feature
 * for such systems.
 */

void (*signal(sig,disp))()
    int sig;
    void (*disp) P((int));
{
    struct sigaction act, oact;
    int status;

    act.sa_handler = disp;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_RESTART;

    status = sigaction(sig, &act, &oact);

    return (status < 0) ? SIG_ERR : oact.sa_handler;
}
#endif

#else /* not UNIX */

#if MSDOS || OS2
# if DJGPP
#  include <gppconio.h>
# else
#  if NEWDOSCC
#   include <conio.h>
#  endif
# endif
#endif

#if     AMIGA
#define NEW 1006L
#define AMG_MAXBUF      1024L
static long terminal;
static char     scrn_tmp[AMG_MAXBUF+1];
static long     scrn_tmp_p = 0;
#endif

#if ST520 && MEGAMAX
#include <osbind.h>
	int STscancode = 0;	
#endif

#if     VMS
#include        <stsdef.h>
#include        <ssdef.h>
#include        <descrip.h>
#include        <iodef.h>
#include        <ttdef.h>
#include	<tt2def.h>

#define NIBUF   128                     /* Input buffer size            */
#define NOBUF   1024			/* MM says big buffers win!	*/
#define EFN     0			/* Event flag			*/

char	obuf[NOBUF];			/* Output buffer		*/
int	nobuf;				/* # of bytes in above		*/
char	ibuf[NIBUF];			/* Input buffer			*/
int	nibuf;				/* # of bytes in above		*/
int	ibufi;				/* Read index			*/
int	oldmode[3];			/* Old TTY mode bits		*/
int	newmode[3];			/* New TTY mode bits		*/
short	iochan;				/* TTY I/O channel		*/
#endif

#if     CPM
#include        <bdos.h>
#endif

#if     MSDOS && NEWDOSCC && !MSC
union REGS rg;		/* cpu register for use of DOS calls (ibmpc.c) */
int nxtchar = -1;	/* character held from type ahead    */
#endif

extern CMDFUNC f_backchar;
extern CMDFUNC f_backchar_to_bol;

void
ttopen()
{
#if     AMIGA
	char oline[NSTRING];
#if	AZTEC
	extern	Enable_Abort;	/* Turn off ctrl-C interrupt */

	Enable_Abort = 0;	/* for the Manx compiler */
#endif
	strcpy(oline, "RAW:0/0/640/200/");
	strcat(oline, PROGNAME);
	strcat(oline, " ");
	strcat(oline, VERSION);
	strcat(oline, "/Amiga");
        terminal = Open(oline, NEW);
#endif
#if     VMS
        struct  dsc$descriptor  idsc;
        struct  dsc$descriptor  odsc;
        char    oname[40];
        int     iosb[2];
        int     status;

        odsc.dsc$a_pointer = "TT";
        odsc.dsc$w_length  = strlen(odsc.dsc$a_pointer);
        odsc.dsc$b_dtype        = DSC$K_DTYPE_T;
        odsc.dsc$b_class        = DSC$K_CLASS_S;
        idsc.dsc$b_dtype        = DSC$K_DTYPE_T;
        idsc.dsc$b_class        = DSC$K_CLASS_S;
        do {
                idsc.dsc$a_pointer = odsc.dsc$a_pointer;
                idsc.dsc$w_length  = odsc.dsc$w_length;
                odsc.dsc$a_pointer = &oname[0];
                odsc.dsc$w_length  = sizeof(oname);
                status = LIB$SYS_TRNLOG(&idsc, &odsc.dsc$w_length, &odsc);
                if (status!=SS$_NORMAL && status!=SS$_NOTRAN)
			ExitProgram(status);
                if (oname[0] == 0x1B) {
                        odsc.dsc$a_pointer += 4;
                        odsc.dsc$w_length  -= 4;
                }
        } while (status == SS$_NORMAL);
        status = SYS$ASSIGN(&odsc, &iochan, 0, 0);
        if (status != SS$_NORMAL)
		ExitProgram(status);
        status = SYS$QIOW(EFN, iochan, IO$_SENSEMODE, iosb, 0, 0,
                          oldmode, sizeof(oldmode), 0, 0, 0, 0);
        if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
		ExitProgram(status);
        newmode[0] = oldmode[0];
        newmode[1] = oldmode[1] | TT$M_NOECHO;
        newmode[1] &= ~(TT$M_TTSYNC|TT$M_HOSTSYNC);
        newmode[2] = oldmode[2] | TT2$M_PASTHRU;
        status = SYS$QIOW(EFN, iochan, IO$_SETMODE, iosb, 0, 0,
                          newmode, sizeof(newmode), 0, 0, 0, 0);
        if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
		ExitProgram(status);
        term.t_nrow = (newmode[1]>>24) - 1;
        term.t_ncol = newmode[0]>>16;

#endif
#if     CPM
#endif

	/* make sure backspace is bound to backspace */
	asciitbl[backspc] = &f_backchar_to_bol;

}

void
ttclose()
{
#if     AMIGA
#if	LATTICE
        amg_flush();
        Close(terminal);
#endif
#if	AZTEC
        amg_flush();
	Enable_Abort = 1;	/* Fix for Manx */
        Close(terminal);
#endif
#endif

#if     VMS
        int     status;
        int     iosb[2];

        ttflush();
        status = SYS$QIOW(EFN, iochan, IO$_SETMODE, iosb, 0, 0,
                 oldmode, sizeof(oldmode), 0, 0, 0, 0);
	if (status == SS$_IVCHAN)
		return;	/* already closed it */
        if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
		ExitProgram(status);
        status = SYS$DASSGN(iochan);
        if (status != SS$_NORMAL)
		ExitProgram(status);
#endif
#if	!VMS
	ttclean(TRUE);
#endif
}

void
ttclean(f)
int f;
{
	if (f) {
		movecursor(term.t_nrow, ttcol); /* don't care about column */
		TTputc('\n');
		TTputc('\r');
	}
	TTflush();
	TTclose();
	TTkclose();
}

void
ttunclean()
{
	TTkopen();	/* xterm */
}

/*
 * Write a character to the display. On VMS, terminal output is buffered, and
 * we just put the characters in the big array, after checking for overflow.
 * On CPM terminal I/O unbuffered, so we just write the byte out. Ditto on
 * MS-DOS (use the very very raw console output routine).
 */
void
ttputc(c)
int c;
{
#if     AMIGA
        scrn_tmp[scrn_tmp_p++] = c;
        if(scrn_tmp_p>=AMG_MAXBUF)
                amg_flush();
#endif
#if	ST520 && MEGAMAX
	Bconout(2,c);
#endif
#if     VMS
        if (nobuf >= NOBUF)
                ttflush();
        obuf[nobuf++] = c;
#endif
#if     CPM
        bios(BCONOUT, c, 0);
#endif
#if	OS2
	putch(c);
#endif
#if	MSDOS
# if IBMPC
	/* unneeded currently -- output is memory-mapped */
# endif
# if ANSI
	putchar(c);
# endif
#endif
}

#if	AMIGA
amg_flush()
{
        if(scrn_tmp_p)
                Write(terminal,scrn_tmp,scrn_tmp_p);
        scrn_tmp_p = 0;
}
#endif

/*
 * Flush terminal buffer. Does real work where the terminal output is buffered
 * up. A no-operation on systems where byte at a time terminal I/O is done.
 */
void
ttflush()
{
#if     AMIGA
        amg_flush();
#endif
#if     VMS
        int     status;
        int     iosb[2];

        status = SS$_NORMAL;
        if (nobuf != 0) {
                status = SYS$QIOW(EFN, iochan, IO$_WRITELBLK|IO$M_NOFORMAT,
                         iosb, 0, 0, obuf, nobuf, 0, 0, 0, 0);
                if (status == SS$_NORMAL)
                        status = iosb[0] & 0xFFFF;
                nobuf = 0;
        }
#endif

#if     CPM
#endif

#if     MSDOS
# if ANSI
	fflush(stdout);
# endif
#endif
}

/*
 * Read a character from the terminal, performing no editing and doing no echo
 * at all. More complex in VMS that almost anyplace else, which figures. Very
 * simple on CPM, because the system can do exactly what you want.
 */
int
ttgetc()
{
#if     AMIGA
	{
        char ch;

        amg_flush();
        Read(terminal, &ch, 1L);
        return(255 & (int)ch);
	}
#endif
#if	ST520 && MEGAMAX
	{
	long ch;

/*
 * blink the cursor only if nothing is happening, this keeps the
 * cursor on steadily during movement making it easier to track
 */
	STcurblink(TRUE);  /* the cursor blinks while we wait */
	ch = Bconin(2);
	STcurblink(FALSE); /* the cursor is steady while we work */
	STscancode = (ch >> 16) & 0xff;
       	return(255 & (int)ch);
	}
#endif
#if     VMS
	{
        int     status;
        int     iosb[2];
        int     term[2];

        while (ibufi >= nibuf) {
		term[0] =
		term[1] = 0;
		if (!typahead()) {
                        status = SYS$QIOW(EFN, iochan, IO$_READLBLK,
                                 iosb, 0, 0, ibuf, 1, 0, term, 0, 0);
                        if (status != SS$_NORMAL
                        || (status = (iosb[0]&0xFFFF)) != SS$_NORMAL)
				ExitProgram(status);
                        nibuf = (iosb[0]>>16) + (iosb[1]>>16);
                }
        }
        return (ibuf[ibufi++] & 0xFF);    /* Allow multinational  */

	}
#endif
#if     CPM
	{
        return (biosb(BCONIN, 0, 0));

	}
#endif

#if MSDOS || OS2
	/*
	 * If we've got a mouse, poll waiting for mouse movement and mouse
	 * clicks until we've got a character to return.
	 */
# if OPT_MS_MOUSE
	if (ms_exists()) {
		for(;;) {
			if (typahead())
				break;
			ms_processing();
		}
	}
# endif /* OPT_MS_MOUSE */
#if	MSC || TURBO || OS2
	return getch();
#endif
#if	NEWDOSCC && !(MSC||TURBO)
	{
	int c;

	/* if a char already is ready, return it */
	if (nxtchar >= 0) {
		c = nxtchar;
		nxtchar = -1;
		return(c);
	}

	/* call the dos to get a char */
	rg.h.ah = 7;		/* dos Direct Console Input call */
	intdos(&rg, &rg);
	c = rg.h.al;		/* grab the char */
	return(c & 0xff);
	}
#endif
#endif	/* MSDOS */

}


/* typahead:	Check to see if any characters are already in the
		keyboard buffer
*/
int
typahead()
{
	if (did_tungetc())
		return TRUE;

#if	X11
	return x_is_pasting();
#endif

#if	VMSVT
	if (ibufi >= nibuf) {
		int	status,
			iosb[2],
			term[2];

                ibufi = 0;
                term[0] = 0;
                term[1] = 0;

                status = SYS$QIOW(EFN, iochan, IO$_READLBLK|IO$M_TIMED,
                         iosb, 0, 0, ibuf, NIBUF, 0, term, 0, 0);
                if (status != SS$_NORMAL)
			ExitProgram(status);
                status = iosb[0] & 0xFFFF;
                if (status!=SS$_NORMAL && status!=SS$_TIMEOUT)
			ExitProgram(status);
                nibuf = (iosb[0]>>16) + (iosb[1]>>16);
                return (nibuf > 0);
	}
	return TRUE;
#endif

#if	MSDOS || OS2
	return (kbhit() != 0);
#endif

}

#endif /* not UNIX */
