/*
 * The functions in this file negotiate with the operating system for
 * characters, and write characters in a barely buffered fashion on the display.
 * All operating systems.
 *
 * $Log: termio.c,v $
 * Revision 1.23  1992/01/22 20:28:28  pgf
 * call TTclose in ttunclean, to support tcapclose().  i hope this is safe
 * for other screen types
 *
 * Revision 1.22  1992/01/10  08:11:41  pgf
 * fixed signal handling, for sysV style handling.  also
 * fixed 2nd arg to tcsetattr()
 *
 * Revision 1.21  1992/01/02  22:42:40  pgf
 * oops -- dropped the termio restoral in ttclean
 *
 * Revision 1.20  1992/01/01  16:30:14  pgf
 * more ifdef'ing on V_SUSP (for pyramid?), and
 * now get tty chars under X11, though we still don't set new modes
 *
 * Revision 1.19  1991/12/20  08:12:00  pgf
 * check the error return from tcsetattr()
 *
 * Revision 1.18  1991/12/04  09:22:19  pgf
 * a little more massaging of ifdefs
 *
 * Revision 1.17  1991/11/27  10:13:10  pgf
 * code split/re-arranged, cleaner posix support
 *
 * Revision 1.16  1991/11/16  18:35:55  pgf
 * changed the ifdefing so termio or sgtty can be used independently of
 * other UNIX choices
 *
 * Revision 1.15  1991/11/14  08:58:17  pgf
 * switch to TCSETAW from TSSETAF in ttclean and unclean -- don't want to
 * flush input needlessly
 *
 * Revision 1.14  1991/11/13  20:09:27  pgf
 * X11 changes, from dave lemke
 *
 * Revision 1.13  1991/11/07  02:00:32  pgf
 * lint cleanup
 *
 * Revision 1.12  1991/11/01  14:38:00  pgf
 * saber cleanup
 *
 * Revision 1.11  1991/10/15  12:18:39  pgf
 * fetch more termio chars at startup, like wkillc, suspc, etc.
 *
 * Revision 1.10  1991/08/12  15:05:43  pgf
 * rearranged read-ahead in one of the tgetc options
 *
 * Revision 1.9  1991/08/07  12:35:07  pgf
 * added RCS log messages
 *
 * revision 1.8
 * date: 1991/07/19 17:17:49;
 * change backspace action to back_char_to_bol()
 * 
 * revision 1.7
 * date: 1991/06/28 10:54:00;
 * change a config ifdef
 * 
 * revision 1.6
 * date: 1991/04/22 09:06:57;
 * POSIX/ULTRIX changes
 * 
 * revision 1.5
 * date: 1991/02/14 15:50:41;
 * fixed size problem on olstate/nlstate
 * 
 * revision 1.4
 * date: 1990/12/06 18:54:48;
 * tried to get ttgetc to return -1 on interrupt -- doesn't work
 * 
 * revision 1.3
 * date: 1990/10/12 19:32:11;
 * do SETAF, not SETA in ttunclean
 * 
 * revision 1.2
 * date: 1990/10/03 16:01:04;
 * make backspace work for everyone
 * 
 * revision 1.1
 * date: 1990/09/21 10:26:10;
 * initial vile RCS revision
 */
#include        <stdio.h>
#include	"estruct.h"
#include        "edef.h"

#if UNIX
#include	<errno.h>
extern int errno;

/* there are three copies of the tt...() routines here -- one each for
	POSIX termios, traditional termio, and sgtty.  If you have a
	choice, I recommend them in that order. */

/* ttopen() and ttclose() are responsible for putting the terminal in raw
	mode, setting up terminal signals, etc.
   ttclean() prepares the terminal for shell escapes, and ttunclean() gets
   	us back into vile's mode
*/


#include	<signal.h>


/* I suppose this config stuff should move to estruct.h */

#if sun
# undef POSIX
# undef BSD
# define POSIX 1
#endif

#if POSIX
# define USE_POSIX_TERMIOS 1
# define USE_FCNTL 1
#else
# if USG
#  define USE_TERMIO 1
#  define USE_FCNTL 1
# else
#  if BSD | V7
#   define USE_SGTTY 1
#   define USE_FIONREAD 1
#  else
 huh?
#  endif
# endif
#endif

#ifdef sun
#undef USE_FCNTL 	/* I have trouble with this on my sun under xterm --
				the screen doesn't always refresh correctly,
				as if the fcntl is interfering with the
				output drain */
#endif

#if X11
# undef USE_FCNTL
# undef USE_FIONREAD
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

#include <sys/termios.h>

struct termios otermios, ntermios;

char tobuf[TBUFSIZ];		/* terminal output buffer */

ttopen()
{
	int s;
	s = tcgetattr(0, &otermios);
	if (s < 0) {
		perror("ttopen tcgetattr");
		exit(1);
	}
	setbuffer(stdout, tobuf, TBUFSIZ);

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

	{ /* this could probably be done more POSIX'ish? */
	extern	int rtfrmshell();
	signal(SIGTSTP,SIG_DFL);	/* set signals so that we can */
	signal(SIGCONT,rtfrmshell);	/* suspend & restart */
	signal(SIGTTOU,SIG_IGN);	/* ignore output prevention */
	}

#if USE_FCNTL
	kbd_flags = fcntl( 0, F_GETFL, 0 );
	kbd_is_polled = FALSE;
#endif

#if ! X11
	ntermios = otermios;

	/* setup new settings, preserve flow control, and allow BREAK */
	ntermios.c_iflag = BRKINT|(otermios.c_iflag & IXON|IXANY|IXOFF);
	ntermios.c_oflag = 0;
	ntermios.c_lflag = ISIG;
	ntermios.c_cc[VMIN] = 1;
	ntermios.c_cc[VTIME] = 0;
	ntermios.c_cc[VSWTCH] = -1;
	ntermios.c_cc[VSUSP] = -1;
	ntermios.c_cc[VSTART] = -1;
	ntermios.c_cc[VSTOP] = -1;
#endif

	ttmiscinit();

	ttunclean();


}

ttclose()
{
	ttclean(TRUE);
}

ttclean(f)
int f;
{
	if (f) {
		movecursor(term.t_nrow, ttcol); /* don't care about column */
		ttputc('\n');
		ttputc('\r');
	}
	fflush(stdout);
	tcdrain(1);
#if ! X11
	tcsetattr(0, TCSADRAIN, &otermios);
#endif
	TTclose();
#if USE_FCNTL
	fcntl(0, F_SETFL, kbd_flags);
#endif
}

ttunclean()
{
	tcdrain(1);
#if ! X11
	if (tcsetattr(0, TCSADRAIN, &ntermios) < 0) {
		perror("ttunclean: tcsetattr");
		exit(1);
	}
#endif
}


#endif /* USE_POSIX_TERMIOS */

#if USE_TERMIO

#include	<termio.h>

/* original terminal characteristics and charactoristics to use inside */
struct	termio	otermio, ntermio;

#ifdef AVAILABLE  /* setbuffer() isn't on most termio systems */
char tobuf[TBUFSIZ];		/* terminal output buffer */
#endif

ttopen()
{

	ioctl(0, TCGETA, &otermio);	/* save old settings */
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
	{
	extern	int rtfrmshell();	/* return from suspended shell */
	signal(SIGTSTP,SIG_DFL);	/* set signals so that we can */
	signal(SIGCONT,rtfrmshell);	/* suspend & restart */
	signal(SIGTTOU,SIG_IGN);	/* ignore output prevention */
	}
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
	ntermio.c_cc[VSWTCH] = -1;
#endif

	ttmiscinit();
	ttunclean();

}

ttclose()
{
	ttclean(TRUE);
}

ttclean(f)
int f;
{
#if	X11
	TTflush();
	TTclose();
#else
	if (f) {
		movecursor(term.t_nrow, ttcol); /* don't care about column */
		ttputc('\n');
		ttputc('\r');
	}
	TTflush();
	TTclose();
	ioctl(0, TCSETAF, &otermio);
#if USE_FCNTL
	fcntl(0, F_SETFL, kbd_flags);
#endif
#endif	/* X11 */
}

ttunclean()
{
#if ! X11
	ioctl(0, TCSETAW, &ntermio);
#endif
}

#endif /* USE_TERMIO */

#if USE_SGTTY

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

#if USE_FIONREAD
#include <sys/ioctl.h>		/* to get at FIONREAD */
char tobuf[TBUFSIZ];		/* terminal output buffer */
#endif

ttopen()
{
	ioctl(0,TIOCGETP,&ostate); /* save old state */
	killc = ostate.sg_kill;
	backspc = ostate.sg_erase;

#if ! X11
	nstate = ostate;
        nstate.sg_flags |= CBREAK;
        nstate.sg_flags &= ~(ECHO|CRMOD);       /* no echo for now... */
	ioctl(0,TIOCSETP,&nstate); /* set new state */
#endif

	rnstate = nstate;
        rnstate.sg_flags &= ~CBREAK;
        rnstate.sg_flags |= RAW;

	ioctl(0, TIOCGETC, &otchars);		/* Save old characters */
	intrc =  otchars.t_intrc;
	startc = otchars.t_startc;
	stopc =  otchars.t_stopc;

#if ! X11
	ntchars = otchars;
	ntchars.t_brkc = -1;
	ntchars.t_eofc = -1;
	ioctl(0, TIOCSETC, &ntchars);		/* Place new character into K */
#endif

	ioctl(0, TIOCGLTC, &oltchars);		/* Save old characters */
	wkillc = oltchars.t_werasc;
	suspc = oltchars.t_suspc;
#if ! X11
	ioctl(0, TIOCSLTC, &nltchars);		/* Place new character into K */
#endif

#if	BSD	/* this is probably not the right ifdef */
	ioctl(0, TIOCLGET, &olstate);
#if ! X11
	nlstate = olstate;
	nlstate |= LLITOUT;
	ioctl(0, TIOCLSET, &nlstate);
#endif
#endif
#if USE_FIONREAD
	setbuffer(stdout, tobuf, TBUFSIZ);
#endif
#if ! X11
	{
	extern	int rtfrmshell();	/* return from suspended shell */
	signal(SIGTSTP,SIG_DFL);	/* set signals so that we can */
	signal(SIGCONT,rtfrmshell);	/* suspend & restart */
	signal(SIGTTOU,SIG_IGN);	/* ignore output prevention */
	}
#endif

	ttmiscinit();

}

ttclose()
{
	ttclean(TRUE);
}

ttclean(f)
int f;
{
#if ! X11
	if (f) {
		movecursor(term.t_nrow, ttcol); /* don't care about column */
		ttputc('\n');
		ttputc('\r');
	}
	TTflush();
	ioctl(0, TIOCSETP, &ostate);
	ioctl(0, TIOCSETC, &otchars);
	ioctl(0, TIOCSLTC, &oltchars);
#if	BSD
	ioctl(0, TIOCLSET, &olstate);
#endif
#endif
}

ttunclean()
{
#if ! X11
	ioctl(0, TIOCSETP, &nstate);
	ioctl(0, TIOCSETC, &ntchars);
	ioctl(0, TIOCSLTC, &nltchars);
#if	BSD
	ioctl(0, TIOCLSET, &nlstate);
#endif
#endif
}

#endif /* USE_SGTTY */

ttputc(c)
int c;
{
        fputc(c, stdout);
}

ttflush()
{
        fflush(stdout);
}

extern int tungotc;

/*
 * Read a character from the terminal, performing no editing and doing no echo
 * at all.
 */
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
	return ( kbd_char & 0x7f );
#else /* USE_FCNTL */
	int c;
        c = fgetc(stdin);
	if (c == EOF) {
		if (errno == EINTR)
			return -1;
		imdying(2);
	} else {
		c &= 0x7f;
	}
	return c;
#endif
}


/* typahead:	Check to see if any characters are already in the
		keyboard buffer
*/
typahead()
{
#if	NeWS
	return(inhibit_update) ;
#else
# if X11
	return x_key_events_ready();
# else

	if (tungotc > 0)
		return TRUE;

#  if	USE_FIONREAD
	{
	long x;
	return((ioctl(0,FIONREAD,&x) < 0) ? 0 : (int)x);
	}
#  else
#   if	USE_FCNTL
	if( !kbd_char_present )
	{
		if( !kbd_is_polled &&
				fcntl(0, F_SETFL, kbd_flags|O_NDELAY ) < 0 )
			return(FALSE);
		kbd_is_polled = TRUE;  /* I think */
		kbd_char_present = (1 == read( 0, &kbd_char, 1 ));
	}
	return ( kbd_char_present );
#   else
	return FALSE;
#   endif/* USE_FCNTL */
#  endif/* USE_FIONREAD */
# endif	/* X11 */
#endif	/* NeWS */
}

/* this takes care of some stuff that's common across all ttopen's.  Some of
	it should arguably be somewhere else, but... */
ttmiscinit()
{
	/* make sure backspace is bound to backspace */
	asciitbl[backspc] = &f_backchar_to_bol;

	/* make sure backspace is considered a backspace by the code */
	_chartypes_[backspc] |= _bspace;

	/* no buffering on input */
	setbuf(stdin, NULL);
}

#else /* not UNIX */

#if   MSDOS & TURBO
#include <conio.h>
#endif

#if     AMIGA
#define NEW 1006L
#define AMG_MAXBUF      1024L
static long terminal;
static char     scrn_tmp[AMG_MAXBUF+1];
static long     scrn_tmp_p = 0;
#endif

#if ST520 & MEGAMAX
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
#define NOBUF   1024                    /* MM says bug buffers win!     */
#define EFN     0                       /* Event flag                   */

char    obuf[NOBUF];                    /* Output buffer                */
int     nobuf;                  /* # of bytes in above    */
char    ibuf[NIBUF];                    /* Input buffer          */
int     nibuf;                  /* # of bytes in above  */
int     ibufi;                  /* Read index                   */
int     oldmode[3];                     /* Old TTY mode bits            */
int     newmode[3];                     /* New TTY mode bits            */
short   iochan;                  /* TTY I/O channel             */
#endif

#if     CPM
#include        <bdos.h>
#endif

#if     MSDOS & (LATTICE | MSC | TURBO | AZTEC | MWC86)
union REGS rg;		/* cpu register for use of DOS calls */
int nxtchar = -1;	/* character held from type ahead    */
#endif

#if RAINBOW
#include "rainbow.h"
#endif

extern CMDFUNC f_backchar;
extern CMDFUNC f_backchar_to_bol;

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
                        exit(status);
                if (oname[0] == 0x1B) {
                        odsc.dsc$a_pointer += 4;
                        odsc.dsc$w_length  -= 4;
                }
        } while (status == SS$_NORMAL);
        status = SYS$ASSIGN(&odsc, &iochan, 0, 0);
        if (status != SS$_NORMAL)
                exit(status);
        status = SYS$QIOW(EFN, iochan, IO$_SENSEMODE, iosb, 0, 0,
                          oldmode, sizeof(oldmode), 0, 0, 0, 0);
        if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
                exit(status);
        newmode[0] = oldmode[0];
        newmode[1] = oldmode[1] | TT$M_NOECHO;
        newmode[1] &= ~(TT$M_TTSYNC|TT$M_HOSTSYNC);
        newmode[2] = oldmode[2] | TT2$M_PASTHRU;
        status = SYS$QIOW(EFN, iochan, IO$_SETMODE, iosb, 0, 0,
                          newmode, sizeof(newmode), 0, 0, 0, 0);
        if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
                exit(status);
        term.t_nrow = (newmode[1]>>24) - 1;
        term.t_ncol = newmode[0]>>16;

#endif
#if     CPM
#endif

#if     MSDOS & (HP150 == 0) & LATTICE
	/* kill the ctrl-break interupt */
	rg.h.ah = 0x33;		/* control-break check dos call */
	rg.h.al = 1;		/* set the current state */
	rg.h.dl = 0;		/* set it OFF */
	intdos(&rg, &rg);	/* go for it! */
#endif

	/* make sure backspace is bound to backspace */
	asciitbl[backspc] = &f_backchar_to_bol;

	/* make sure backspace is considered a backspace by the code */
	_chartypes_[backspc] |= _bspace;
}

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
        int     iosb[1];

        ttflush();
        status = SYS$QIOW(EFN, iochan, IO$_SETMODE, iosb, 0, 0,
                 oldmode, sizeof(oldmode), 0, 0, 0, 0);
        if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
                exit(status);
        status = SYS$DASSGN(iochan);
        if (status != SS$_NORMAL)
                exit(status);
#endif
#if     MSDOS & (HP150 == 0) & LATTICE
	/* restore the ctrl-break interupt */
	rg.h.ah = 0x33;		/* control-break check dos call */
	rg.h.al = 1;		/* set the current state */
	rg.h.dl = 1;		/* set it ON */
	intdos(&rg, &rg);	/* go for it! */
#endif

	ttclean(TRUE);
}

ttclean(f)
int f;
{
	if (f) {
		movecursor(term.t_nrow, ttcol); /* don't care about column */
		ttputc('\n');
		ttputc('\r');
	}
	TTflush();
	TTclose();
	TTkclose();
}

ttunclean()
{
}

/*
 * Write a character to the display. On VMS, terminal output is buffered, and
 * we just put the characters in the big array, after checking for overflow.
 * On CPM terminal I/O unbuffered, so we just write the byte out. Ditto on
 * MS-DOS (use the very very raw console output routine).
 */
ttputc(c)
#if     AMIGA | (ST520 & MEGAMAX)
        char c;
#else
	int c;
#endif
{
#if     AMIGA
        scrn_tmp[scrn_tmp_p++] = c;
        if(scrn_tmp_p>=AMG_MAXBUF)
                amg_flush();
#endif
#if	ST520 & MEGAMAX
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

#if     MSDOS & MWC86
        putcnb(c);
#endif

#if	MSDOS & (LATTICE | AZTEC) & ~IBMPC
	bdos(6, c, 0);
#endif

#if RAINBOW
        Put_Char(c);                    /* fast video */
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
        return (status);
#endif

#if     CPM
#endif

#if     MSDOS
#endif
}

extern int tungotc;

/*
 * Read a character from the terminal, performing no editing and doing no echo
 * at all. More complex in VMS that almost anyplace else, which figures. Very
 * simple on CPM, because the system can do exactly what you want.
 */
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
#if	ST520 & MEGAMAX
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
                ibufi = 0;
                term[0] = 0;
                term[1] = 0;
                status = SYS$QIOW(EFN, iochan, IO$_READLBLK|IO$M_TIMED,
                         iosb, 0, 0, ibuf, NIBUF, 0, term, 0, 0);
                if (status != SS$_NORMAL)
                        exit(status);
                status = iosb[0] & 0xFFFF;
                if (status!=SS$_NORMAL && status!=SS$_TIMEOUT)
                        exit(status);
                nibuf = (iosb[0]>>16) + (iosb[1]>>16);
                if (nibuf == 0) {
                        status = SYS$QIOW(EFN, iochan, IO$_READLBLK,
                                 iosb, 0, 0, ibuf, 1, 0, term, 0, 0);
                        if (status != SS$_NORMAL
                        || (status = (iosb[0]&0xFFFF)) != SS$_NORMAL)
                                exit(status);
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

#if RAINBOW
	{
	int c;

        while ((c = Read_Keyboard()) < 0);

        if ((c & Function_Key) == 0)
                if (!((c & 0xFF) == 015 || (c & 0xFF) == 0177))
                        c &= 0xFF;

        return c;
	}
#endif

#if     MSDOS & MWC86
	{
        return (getcnb());

	}
#endif

#if	MSDOS & (LATTICE | MSC | TURBO | AZTEC)
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

}


/* typahead:	Check to see if any characters are already in the
		keyboard buffer
*/
typahead()
{

#if	MSDOS & (MSC | TURBO)
	if (tungotc > 0)
		return TRUE;

	if (kbhit() != 0)
		return(TRUE);
	else
		return(FALSE);
#endif

#if	MSDOS & (LATTICE | AZTEC | MWC86)
	int c;		/* character read */
	int flags;	/* cpu flags from dos call */

	if (tungotc > 0)
		return TRUE;

	if (nxtchar >= 0)
		return(TRUE);

	rg.h.ah = 6;	/* Direct Console I/O call */
	rg.h.dl = 255;	/*         does console input */
#if	LATTICE | AZTEC
	flags = intdos(&rg, &rg);
#else
	intcall(&rg, &rg, 0x21);
	flags = rg.x.flags;
#endif
	c = rg.h.al;	/* grab the character */

	/* no character pending */
	if ((flags & 0x40) != 0)
		return(FALSE);

	/* save the character and return true */
	nxtchar = c;
	return(TRUE);
#endif
}

#endif /* not UNIX */
