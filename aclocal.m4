dnl
dnl Local definitions for autoconf.
dnl ------------------------
dnl $Header: /usr/build/VCS/pgf-vile/RCS/aclocal.m4,v 1.12 1995/04/22 03:22:53 pgf Exp $
dnl
dnl ---------------------------------------------------------------------------
dnl
define([VC_ERRNO],
[
AC_MSG_CHECKING([for errno external decl])
AC_CACHE_VAL(vc_cv_extern_errno,[
	AC_TRY_COMPILE([
#include <errno.h>],
		[int x = errno],
		[vc_cv_extern_errno=yes],
		[vc_cv_extern_errno=no])
	])
AC_MSG_RESULT($vc_cv_extern_errno)
test $vc_cv_extern_errno = yes && AC_DEFINE(HAVE_EXTERN_ERRNO)
])
dnl ---------------------------------------------------------------------------
dnl
define(VC_SETPGRP,
[
AC_MSG_CHECKING([for BSD style setpgrp])
AC_CACHE_VAL(vc_cv_bsd_setpgrp,[
AC_TRY_RUN([
main()
{
    if (setpgrp(1,1) == -1)
	exit(0);
    else
	exit(1);
}],
	[vc_cv_bsd_setpgrp=yes],
	[vc_cv_bsd_setpgrp=no],
	[vc_cv_bsd_setpgrp=unknown])
	])
AC_MSG_RESULT($vc_cv_bsd_setpgrp)
test $vc_cv_bsd_setpgrp = yes && AC_DEFINE(HAVE_BSD_SETPGRP)
])dnl
dnl ---------------------------------------------------------------------------
dnl
dnl Note: must follow VC_SETPGRP, but cannot use AC_REQUIRE, since that messes
dnl up the messages...
define(VC_KILLPG,
[
AC_MSG_CHECKING([if killpg is needed])
AC_CACHE_VAL(vc_cv_need_killpg,[
AC_TRY_RUN([
#include <sys/types.h>
#include <signal.h>
RETSIGTYPE
handler(s)
    int s;
{
    exit(0);
}

main()
{
#ifdef HAVE_BSD_SETPGRP
    (void) setpgrp(0,0);
#else
    (void) setpgrp();
#endif
    (void) signal(SIGINT, handler);
    (void) kill(-getpid(), SIGINT);
    exit(1);
}],
	[vc_cv_need_killpg=no],
	[vc_cv_need_killpg=yes],
	[vc_cv_need_killpg=unknown]
)])
AC_MSG_RESULT($vc_cv_need_killpg)
test $vc_cv_need_killpg = yes && AC_DEFINE(HAVE_KILLPG)
])dnl
dnl ---------------------------------------------------------------------------
dnl
dnl VC_RESTARTABLE_PIPEREAD is a modified version of AC_RESTARTABLE_SYSCALLS
dnl from acspecific.m4, which uses a read on a pipe (surprise!) rather than
dnl a wait() as the test code.  apparently there is a POSIX change, which OSF/1
dnl at least has adapted to, which says reads (or writes) on pipes for which
dnl no data has been transferred are interruptable _regardless_ of the 
dnl SA_RESTART bit.  yuck.
define(VC_RESTARTABLE_PIPEREAD,
[
AC_MSG_CHECKING(for restartable reads on pipes)
AC_CACHE_VAL(vc_can_restart_read,[
AC_TRY_RUN(
[/* Exit 0 (true) if wait returns something other than -1,
   i.e. the pid of the child, which means that wait was restarted
   after getting the signal.  */
#include <sys/types.h>
#include <signal.h>
#ifdef SA_RESTART
sigwrapper(sig, disp)
int sig;
void (*disp)();
{
    struct sigaction act, oact;

    act.sa_handler = disp;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_RESTART;

    (void)sigaction(sig, &act, &oact);

}
#else
# define sigwrapper signal
#endif
ucatch (isig) { }
main () {
  int i, status;
  int fd[2];
  char buff[2];
  pipe(fd);
  i = fork();
  if (i == 0) {
      sleep (2);
      kill (getppid (), SIGINT);
      sleep (2);
      write(fd[1],"done",4);
      close(fd[1]);
      exit (0);
  }
  sigwrapper (SIGINT, ucatch);
  status = read(fd[0], buff, sizeof(buff));
  wait (&i);
  exit (status == -1);
}
],
[vc_can_restart_read=yes],
[vc_can_restart_read=no],
[vc_can_restart_read=unknown])])
AC_MSG_RESULT($vc_can_restart_read)
test $vc_can_restart_read = yes && AC_DEFINE(HAVE_RESTARTABLE_PIPEREAD)
])dnl
dnl ---------------------------------------------------------------------------
dnl
define(VC_MISSING_CHECK,
[
AC_MSG_CHECKING([for missing "$1" extern])
AC_CACHE_VAL([vc_cv_func_$1],[
AC_TRY_LINK([
#include <stdio.h>
#include <sys/types.h>
#include <setjmp.h>
#include <signal.h>
#include <errno.h>
#ifdef HAVE_TYPES_H
#include <types.h>
#endif
#ifdef HAVE_LIBC_H
#include <libc.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#else
#ifdef HAVE_VARARGS_H
#include <varargs.h>
#endif
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STDDEF_H
#include <stddef.h>
#endif
#if HAVE_UTIME_H
# include <utime.h>
#endif

#if STDC_HEADERS || HAVE_STRING_H
#include <string.h>
  /* An ANSI string.h and pre-ANSI memory.h might conflict.  */
#if !STDC_HEADERS && HAVE_MEMORY_H
#include <memory.h>
#endif /* not STDC_HEADERS and HAVE_MEMORY_H */
#else /* not STDC_HEADERS and not HAVE_STRING_H */
#if HAVE_STRINGS_H
#include <strings.h>
  /* memory.h and strings.h conflict on some systems */
#endif
#endif /* not STDC_HEADERS and not HAVE_STRING_H */

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

/* unistd.h defines _POSIX_VERSION on POSIX.1 systems.  */
#if defined(HAVE_DIRENT_H) || defined(_POSIX_VERSION)
#include <dirent.h>
#else /* not (HAVE_DIRENT_H or _POSIX_VERSION) */
#ifdef HAVE_SYS_NDIR_H
#include <sys/ndir.h>
#endif /* HAVE_SYS_NDIR_H */
#ifdef HAVE_SYS_DIR_H
#include <sys/dir.h>
#endif /* HAVE_SYS_DIR_H */
#ifdef HAVE_NDIR_H
#include <ndir.h>
#endif /* HAVE_NDIR_H */
#endif /* not (HAVE_DIRENT_H or _POSIX_VERSION) */

#ifdef HAVE_SYS_FILE_H
#include <sys/file.h>
#endif
#ifdef HAVE_STAT_H
#include <stat.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#ifdef HAVE_SIGINFO_H
#include <siginfo.h>
#endif

#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif

#ifdef HAVE_SYS_TIMES_H
#include <sys/times.h>
#endif
#ifdef HAVE_UCONTEXT_H
#include <ucontext.h>
#endif
#ifdef HAVE_LIBGEN_H
#include <libgen.h>
#else
#ifdef HAVE_BSD_REGEX_H
#include <bsd/regex.h>
#endif
#endif
#ifdef HAVE_MATH_H
#include <math.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef HAVE_SYS_UN_H
#include <sys/un.h>
#endif
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#ifdef HAVE_SYS_IPC_H
#include <sys/ipc.h>
#endif
#ifdef HAVE_SYS_MSG_H
#include <sys/msg.h>
#endif
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif
#ifdef HAVE_SELECT_H
#include <select.h>
#endif
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#if !defined(FIONREAD)
/* try harder to get it */
# if HAVE_SYS_FILIO_H
#  include <sys/filio.h>
# else /* if you have trouble including ioctl.h, try <sys/ioctl.h> instead */
#  if HAVE_IOCTL_H
#   include <ioctl.h>
#  else
#   if HAVE_SYS_IOCTL_H
#    include <sys/ioctl.h>
#   endif
#  endif
# endif
#endif

#undef $1
struct zowie { int a; double b; struct zowie *c; char d; };
extern struct zowie *$1();
],
[
#if HAVE_LIBXT		/* needed for SunOS 4.0.3 or 4.1 */
XtToolkitInitialize();
#endif
],
[eval 'vc_cv_func_'$1'=yes'],
[eval 'vc_cv_func_'$1'=no'])])
eval 'vc_result=$vc_cv_func_'$1
AC_MSG_RESULT($vc_result)
test $vc_result = yes && AC_DEFINE_UNQUOTED(MISSING_EXTERN_$2)
])dnl
dnl ---------------------------------------------------------------------------
dnl
define(VC_MISSING_EXTERN,
[for ac_func in $1
do
changequote(,)dnl
ac_tr_func=`echo $ac_func | tr '[a-z]' '[A-Z]'`
changequote([,])dnl
VC_MISSING_CHECK(${ac_func}, ${ac_tr_func})dnl
done
])dnl
dnl ---------------------------------------------------------------------------
dnl	On both Ultrix and CLIX, I find size_t defined in <stdio.h>
define([VC_SIZE_T],
[
AC_MSG_CHECKING(for size_t in <sys/types.h> or <stdio.h>)
AC_CACHE_VAL(vc_cv_type_size_t,[
	AC_TRY_COMPILE([
#include <sys/types.h>
#include <stdio.h>],
		[size_t x],
		[vc_cv_type_size_t=yes],
		[vc_cv_type_size_t=no])
	])
AC_MSG_RESULT($vc_cv_type_size_t)
test $vc_cv_type_size_t = no && AC_DEFINE(size_t, unsigned)
])dnl
dnl ---------------------------------------------------------------------------
dnl	Check for declarion of sys_errlist in one of stdio.h and errno.h.  
dnl	Declaration of sys_errlist on BSD4.4 interferes with our declaration.
dnl	Reported by Keith Bostic.
define([VC_SYS_ERRLIST],
[
AC_MSG_CHECKING([declaration of sys_errlist])
AC_CACHE_VAL(vc_cv_dcl_sys_errlist,[
	AC_TRY_COMPILE([
#include <stdio.h>
#include <sys/types.h>
#include <errno.h> ],
	[ char *c = (char *) *sys_errlist; ],
	[vc_cv_dcl_sys_errlist=yes],
	[vc_cv_dcl_sys_errlist=no])
	])
AC_MSG_RESULT($vc_cv_dcl_sys_errlist)
test $vc_cv_dcl_sys_errlist = yes && AC_DEFINE(HAVE_EXTERN_SYS_ERRLIST)
])dnl
dnl ---------------------------------------------------------------------------
dnl	Check if 'getpgrp()' accepts an argument.
define([VC_TEST_GETPGRP],
[
AC_MSG_CHECKING([for argument of getpgrp])
AC_CACHE_VAL(vc_cv_arg_getpgrp,[
	AC_TRY_COMPILE([
#include <sys/types.h>
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif],
	[ getpgrp(0) ],
	[vc_cv_arg_getpgrp=yes],
	[vc_cv_arg_getpgrp=no])
	])
AC_MSG_RESULT($vc_cv_arg_getpgrp)
test $vc_cv_arg_getpgrp = yes && AC_DEFINE(GETPGRP_HAS_ARG)
])dnl
