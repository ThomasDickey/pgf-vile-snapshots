/*
 * Prototypes for functions in the mode-tables (must be declared before the
 * point at which proto.h is included).
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/chgdfunc.h,v 1.2 1995/07/27 14:13:40 pgf Exp $
 */
extern int chgd_autobuf  P((CHGD_ARGS));
extern int chgd_buffer   P((CHGD_ARGS));
extern int chgd_charset  P((CHGD_ARGS));
#if OPT_COLOR
extern int chgd_color    P((CHGD_ARGS));
#endif
extern int chgd_disabled P((CHGD_ARGS));
extern int chgd_fences   P((CHGD_ARGS));
extern int chgd_major    P((CHGD_ARGS));
extern int chgd_major_w  P((CHGD_ARGS));
extern int chgd_status   P((CHGD_ARGS));
extern int chgd_window   P((CHGD_ARGS));
extern int chgd_working  P((CHGD_ARGS));
extern int chgd_xterm    P((CHGD_ARGS));

