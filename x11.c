/*
 * 	X11 support, Dave Lemke, 11/91
 *	X Toolkit support, Kevin Buettner, 2/94
 *
 * $Log: x11.c,v $
 * Revision 1.71  1994/04/25 20:28:14  pgf
 * fixes from kev
 *
 * Revision 1.70  1994/04/25  09:54:11  pgf
 * warning cleanup
 *
 * Revision 1.69  1994/04/22  16:06:18  pgf
 * kev's font changes, mostly
 *
 * Revision 1.68  1994/04/22  15:58:05  pgf
 * misplaced endif
 *
 * Revision 1.67  1994/04/22  14:34:15  pgf
 * changed BAD and GOOD to BADEXIT and GOODEXIT
 *
 * Revision 1.66  1994/04/22  11:33:01  pgf
 * took out resource-based window and icon title maintenance, added support
 * for the variables that can be used to set them ($title, $iconname)
 *
 * Revision 1.65  1994/04/20  11:09:47  pgf
 * eliminated #elif for HP users.  geez, HP, get a life.
 *
 * Revision 1.64  1994/04/14  09:52:17  pgf
 * added support for setting window and icon names from buffer name (m.finken)
 *
 * Revision 1.63  1994/04/13  20:40:21  pgf
 * change kcod2str, fnc2str, string2prc to all deal in "p-strings", so
 * we can store null chars in binding strings.
 *
 * Revision 1.62  1994/04/11  15:50:06  pgf
 * kev's attribute changes
 *
 * Revision 1.61  1994/04/08  21:14:24  pgf
 * kev fixes for scrollbars, new intf'c to sel_extend
 *
 * Revision 1.60  1994/04/07  19:02:20  pgf
 * kev's changes for direct pscreen access
 *
 * Revision 1.59  1994/04/07  18:46:47  pgf
 * eliminate "can't happen" abort() call, fix some off-by-ones on
 * selections (multiclick), and allow multiclicking a whole line, not just
 * a screen's worth
 *
 * Revision 1.58  1994/04/01  14:50:54  pgf
 * persistent selections and blinking cursor
 *
 * Revision 1.57  1994/04/01  14:30:02  pgf
 * tom's warning/lint patch
 *
 * Revision 1.56  1994/03/29  16:28:49  pgf
 * kev's cursor/focus fix
 *
 * Revision 1.55  1994/03/29  16:27:09  pgf
 * tom's change to kev's patch
 *
 * Revision 1.54  1994/03/29  16:24:20  pgf
 * kev's changes: selection and attributes
 *
 * Revision 1.53  1994/03/28  16:24:48  pgf
 * add call to update scrollbars when changing fonts
 *
 * Revision 1.52  1994/03/24  12:12:17  pgf
 * arrow/escape key fixes
 *
 * Revision 1.51  1994/03/11  14:02:19  pgf
 * many many changes from tom and kevin -- no more athena widget support,
 * scrollbars now supplied, a lot of cleanup.  feels good.
 *
 * Revision 1.50  1994/03/08  12:31:22  pgf
 * changed 'fulllineregions' to 'regionshape'.
 *
 * Revision 1.49  1994/03/02  10:02:02  pgf
 * no longer trim leading whitespace from pasted text -- and suppress
 * autoindent on the insertion.
 *
 * Revision 1.48  1994/02/28  11:53:53  pgf
 * kevin's motif resize patch
 *
 * Revision 1.47  1994/02/25  12:02:00  pgf
 * tom's changes for rows/cols --> geometry
 *
 * Revision 1.46  1994/02/23  05:31:58  pgf
 * rows, columns, left/right scrollbar placement
 *
 * Revision 1.45  1994/02/22  17:48:19  pgf
 * added kev's openlook and motif support
 *
 * Revision 1.44  1994/02/22  11:03:15  pgf
 * truncated RCS log for 4.0
 *
 */
/*
 * Widget set selection.
 *
 * You must have exactly one of the following defined
 *
 *    NO_WIDGETS	-- Use only Xlib and X toolkit
 *    MOTIF_WIDGETS	-- Use Xlib, Xt, and Motif widget set
 *    OL_WIDGETS	-- Use Xlib, Xt, and Openlook widget set
 */

#include	<X11/Intrinsic.h>
#include	<X11/StringDefs.h>
#include	<X11/cursorfont.h>

#ifdef lint
#include	<X11/IntrinsicI.h>
#endif

#include	"estruct.h"
#include	"edef.h"

#if XtSpecificationRelease < 4
#define XtPointer caddr_t
#endif

/* undef for the benefit of some X header files -- if you really _are_
	both ISC and X11, well, you know what to do. */
#undef ISC

#if X11 && XTOOLKIT

#define MY_CLASS	"XVile"

#if VMS
#undef UNIX
#define coreWidgetClass widgetClass /* patch for VMS 5.4-3 (dickey) */
#endif

/* redefined in X11/Xos.h */
#undef strchr
#undef strrchr

#if OL_WIDGETS
#undef BANG
#include	<Xol/OpenLook.h>
#include	<Xol/Form.h>
#include	<Xol/BulletinBo.h>
#include	<Xol/Slider.h>
#include	<Xol/Scrollbar.h>
#endif /* OL_WIDGETS */

#include	<X11/Shell.h>
#include	<X11/keysym.h>
#include	<X11/Xatom.h>

#if MOTIF_WIDGETS
#include	<Xm/Form.h>
#include	<Xm/PanedW.h>
#include	<Xm/ScrollBar.h>
#include	<Xm/SashP.h>
#endif /* MOTIF_WIDGETS */

#define	XCalloc(type)	((type*)calloc(1, sizeof(type)))

#define	MARGIN	8
#define	SCRSIZ	64
#define	NPAUSE	10		/* # times thru update to pause */
#define	absol(x)	((x) > 0 ? (x) : -(x))
#define	CEIL(a,b)	((a + b - 1) / (b))

#define onMsgRow(tw)	(ttrow == (tw->rows - 1))

/* XXX -- use xcutsel instead */
#undef	SABER_HACK		/* hack to support Saber since it doesn't do
				 * selections right */


#define X_PIXEL ULONG

#define PANE_WIDTH_MAX 200

#if NO_WIDGETS
# define PANE_WIDTH_DEFAULT 15
# define PANE_WIDTH_MIN 6
#else
# if MOTIF_WIDGETS || OL_WIDGETS
#  define PANE_WIDTH_DEFAULT 20
#  define PANE_WIDTH_MIN 10
# endif
#endif

#define MINCOLS	30
#define MINROWS MINWLNS
#define MAXSBS	((MAXROWS) / 2)

/* The following set of definitions comprise the interface to pscreen
 * in display.c.  They should perhaps be moved to a header file so
 * that other screen interfaces can make use of them.
 */
extern VIDEO **pscreen;
#define IS_DIRTY_LINE(r)	(pscreen[(r)]->v_flag & VFCHG)
#define IS_DIRTY(r,c)		(pscreen[(r)]->v_attrs[(c)] & VADIRTY)
#define IS_REVERSED(r,c)	((pscreen[(r)]->v_attrs[(c)] & VAREV) != 0)
#define MARK_LINE_DIRTY(r)	(pscreen[(r)]->v_flag |= VFCHG)
#define MARK_CELL_DIRTY(r,c)	(pscreen[(r)]->v_attrs[(c)] |= VADIRTY)
#define CLEAR_LINE_DIRTY(r)	(pscreen[(r)]->v_flag &= ~VFCHG)
#define CLEAR_CELL_DIRTY(r,c)	(pscreen[(r)]->v_attrs[(c)] &= ~VADIRTY)
#define CELL_TEXT(r,c)		(pscreen[(r)]->v_text[(c)])
#define CELL_ATTR(r,c)		(pscreen[(r)]->v_attrs[(c)])

/* Blinking cursor toggle...defined this way to leave room for future
 * cursor flags.
 */
#define	BLINK_TOGGLE	0x1

/*
 * Fonts searched flags
 */

#define FSRCH_BOLD	0x1
#define FSRCH_ITAL	0x2
#define FSRCH_BOLDITAL	0x4

/* Keyboard queue size */
#define KQSIZE 64

static Display *dpy;

#if NO_WIDGETS
typedef struct _scroll_info {
    int		top;		/* top of "thumb" */
    int		bot;		/* bottom of "thumb" */
    int		totlen;		/* total length of scrollbar */
} ScrollInfo;
#endif

typedef struct _text_win {
    /* X stuff */
    Window	win;		/* window corresponding to screen */
    XtAppContext
    		app_context;	/* application context */
    Widget	top_widget;	/* top level widget */
    Widget	screen;		/* screen widget */
    Widget	form_widget;	/* form widget */
    Widget	pane;		/* panes in which scrollbars live */
    Widget	scrollbars[MAXSBS];	
    				/* the scrollbars */
    int		nscrollbars;	/* number of currently active scroll bars */
#if OL_WIDGETS
    Widget	sliders[MAXSBS-1];
#endif
#if NO_WIDGETS
    GC		scrollbargc;	/* graphics context for scrollbar "thumb" */
    ScrollInfo	scrollinfo[MAXSBS];
    Widget	grips[MAXSBS-1]; /* grips for resizing scrollbars */
    Cursor	curs_sb_v_double_arrow;
    Cursor	curs_sb_up_arrow;
    Cursor	curs_sb_down_arrow;
    Cursor	curs_sb_left_arrow;
    Cursor	curs_sb_right_arrow;
    Cursor	curs_double_arrow;
    XtIntervalId scroll_repeat_id;
    ULONG	scroll_repeat_timeout;
#endif	/* NO_WIDGETS */
    ULONG	scroll_repeat_interval;
    XtIntervalId blink_id;
    int		blink_status;
    int		blink_interval;
    Cursor	curs_xterm;

    int		base_width;	/* width with screen widgets' width zero */
    int		base_height;
    UINT	pane_width;	/* full width of scrollbar pane */
    Dimension	top_width;	/* width of top widget as of last resize */
    Dimension	top_height;	/* height of top widget as of last resize */
    				
    int		fsrch_flags;	/* flags which indicate which fonts have
    				 * been searched for
				 */
    XFontStruct *pfont;		/* Normal font */
    XFontStruct *pfont_bold;
    XFontStruct *pfont_ital;
    XFontStruct *pfont_boldital;
    GC          textgc;
    GC          reversegc;
    Pixel	fg;
    Pixel	bg;
    int         char_width,
                char_ascent,
                char_height;
    Bool	left_ink,	/* font has "ink" past bounding box on left */
    		right_ink;	/* font has "ink" past bounding box on right */
    char       *geometry;
    char       *starting_fontname;	/* name of font at startup */
    char       *fontname;		/* name of current font */
    Bool	focus_follows_mouse;
    Bool	scrollbar_on_left;
    Bool	update_window_name;
    Bool	update_icon_name;
    Bool	persistent_selections;

    /* text stuff */
    Bool        reverse;
    unsigned    rows,
                cols;
    Bool        show_cursor;

    /* selection stuff */
    String	multi_click_char_class;	/* ?? */
    Time        lasttime;	/* for multi-click */
    Time        click_timeout;
    int         numclicks;
    Bool        have_selection;
    Bool	wipe_permitted;
    Bool	was_on_msgline;
    Atom        sel_prop;
    UCHAR *	selection_data;
    int         selection_len;
    XtIntervalId sel_scroll_id;

    /* key press queue */
    int		kqhead;
    int		kqtail;
    int		kq[KQSIZE];
}           TextWindowRec, *TextWindow;


static	TextWindow cur_win;
static	TBUFF	*PasteBuf;

static	Atom	atom_WM_PROTOCOLS;
static	Atom	atom_WM_DELETE_WINDOW;
static	Atom	atom_WM_TAKE_FOCUS;
static	Atom	atom_FONT;
static	Atom	atom_FOUNDRY;
static	Atom	atom_WEIGHT_NAME;
static	Atom	atom_SLANT;
static	Atom	atom_SETWIDTH_NAME;
static	Atom	atom_PIXEL_SIZE;
static	Atom	atom_RESOLUTION_X;
static	Atom	atom_RESOLUTION_Y;
static	Atom	atom_SPACING;
static	Atom	atom_AVERAGE_WIDTH;
static	Atom	atom_CHARSET_REGISTRY;
static	Atom	atom_CHARSET_ENCODING;

#if MOTIF_WIDGETS
static Bool lookfor_sb_resize = FALSE;
#endif

static	int	x_getc   P(( void )),
		x_cres   P(( char * ));

static	void	x_open   P(( void )),
		x_close  P(( void )),
		x_flush  P(( void )),
		x_kopen  P(( void )),
		x_kclose P(( void )),
		x_beep   P(( void )),
		x_rev    P(( int ));

#if COLOR
static	void	x_fcol   P(( int )),
		x_bcol   P(( int ));
#endif

#ifdef SCROLLCODE
static	void	x_scroll P(( int, int, int ));
#endif

static	SIGT	x_quit (DEFINE_SIG_ARGS);

static	void	free_selection P(( TextWindow ));
static	void	x_stash_selection P(( TextWindow ));
static	int	set_character_class P(( char * ));
static	void	x_touch P(( TextWindow, int, int, UINT, UINT ));
static	void	x_paste_selection P(( TextWindow ));
static	Bool	x_give_selection P(( TextWindow, XSelectionRequestEvent *, Atom, Atom ));
static	void	scroll_selection P(( XtPointer, XtIntervalId * ));
static	int	line_count_and_interval P(( long, unsigned long * ));
static	void	extend_selection P(( TextWindow, int, int, Bool ));
static	void	multi_click P(( TextWindow, int, int ));
static	void	start_selection P(( TextWindow, XButtonPressedEvent *, int, int ));
static	XMotionEvent * compress_motion P(( XMotionEvent * ));
static	void	x_process_event P(( Widget, XtPointer, XEvent *, Boolean * ));
static	void	x_configure_window P(( Widget, XtPointer, XEvent *, Boolean * ));
static	void	x_change_focus P(( Widget, XtPointer, XEvent *, Boolean * ));
static	void	x_key_press P(( Widget, XtPointer, XEvent *, Boolean * ));
static	void	x_wm_delwin P(( Widget, XtPointer, XEvent *, Boolean * ));
static	char *	strndup P(( char *, int ));
static	void	wait_for_scroll P(( TextWindow ));
static	void	flush_line P(( UCHAR *, int, VIDEO_ATTR, int, int ));
static	int	set_character_class_range P(( int, int, int ));
static	void	x_lose_selection P(( TextWindow ));
static	int	add2paste P(( TBUFF **, int ));
static	int	copy_paste P(( TBUFF **, char *, SIZE_T ));
static	void	x_get_selection P(( TextWindow, Atom, Atom, char *, SIZE_T, int ));
static	char *	x_get_font_atom_property P((XFontStruct *, Atom));
static	XFontStruct *query_font P(( TextWindow, char * ));
static	XFontStruct *alternate_font P(( char *, char * ));
static	void	configure_bar P(( Widget, XEvent *, String *, Cardinal *));
static	void	update_scrollbar_sizes P(( void ));
static	void	kqinit P(( TextWindow ));
static	int	kqempty P(( TextWindow ));
static	int	kqfull P(( TextWindow ));
static	int	kqdel P(( TextWindow ));
static	void	kqadd P(( TextWindow, int ));
static	int	kqpop P(( TextWindow ));
static	void	display_cursor P(( XtPointer, XtIntervalId * ));
#if MOTIF_WIDGETS
static	void	JumpProc P(( Widget, XtPointer, XtPointer ));
static	void	grip_moved P(( Widget, XtPointer, XEvent *, Boolean * ));
static	void	pane_button P(( Widget, XtPointer, XEvent *, Boolean * ));
#else
#if OL_WIDGETS
static	void	JumpProc P(( Widget, XtPointer, OlScrollbarVerify * ));
static	void	grip_moved P(( Widget, XtPointer, OlSliderVerify * ));
#else
#if NO_WIDGETS
static	void	x_expose_pane P(( Widget, XtPointer, XEvent *, Boolean * ));
static	void	draw_thumb P((Widget, int, int, Bool));
static	void	update_thumb P((int, int, int));
static	void	do_scroll P(( Widget, XEvent *, String *, Cardinal *));
static	void	repeat_scroll P(( XtPointer, XtIntervalId * ));
static	void	resize_bar P(( Widget, XEvent *, String *, Cardinal *));
#endif /* NO_WIDGETS */
#endif /* OL_WIDGETS */
#endif /* MOTIF_WIDGETS */

#define	FONTNAME	"7x13"

TERM        term = {
    0,				/* these four values are set dynamically at
				 * open time */
    0,
    0,
    0,
    MARGIN,
    SCRSIZ,
    NPAUSE,
    x_open,
    x_close,
    x_kopen,
    x_kclose,
    x_getc,
    psc_putchar,
    psc_flush,
    psc_move,
    psc_eeol,
    psc_eeop,
    x_beep,
    x_rev,
    x_cres

#if	COLOR
    ,x_fcol,
    x_bcol
#endif

#if SCROLLCODE
    ,x_scroll
#endif

    ,x_flush
};


#define	x_width(tw)		((tw)->cols * (tw)->char_width)
#define	x_height(tw)		((tw)->rows * (tw)->char_height)
#define	x_pos(tw, c)		((c) * (tw)->char_width)
#define	y_pos(tw, r)		((r) * (tw)->char_height)
#define	text_y_pos(tw, r)	(y_pos((tw), (r)) + (tw)->char_ascent)
#define min(a,b)		((a) < (b) ? (a) : (b))
#define max(a,b)		((a) > (b) ? (a) : (b))



#if NO_WIDGETS
/* We define our own little bulletin board widget here...if this gets
 * too unwieldly, we should move it to another file.
 */

#include	<X11/IntrinsicP.h>

/* New fields for the Bb widget class record */
typedef struct {int empty;} BbClassPart;

/* Class record declaration */
typedef struct _BbClassRec {
    CoreClassPart	core_class;
    CompositeClassPart  composite_class;
    BbClassPart	bb_class;
} BbClassRec;

extern BbClassRec bbClassRec;

/* Instance declaration */
typedef struct _BbRec {
    CorePart	    core;
    CompositePart   composite;
} BbRec;

static XtGeometryResult bbGeometryManager P(( Widget, XtWidgetGeometry *, XtWidgetGeometry * ));
static XtGeometryResult bbPreferredSize P(( Widget, XtWidgetGeometry *, XtWidgetGeometry * ));;

BbClassRec bbClassRec = {
  {
/* core_class fields      */
    /* superclass         */    (WidgetClass) &compositeClassRec,
    /* class_name         */    "Bb",
    /* widget_size        */    sizeof(BbRec),
    /* class_initialize   */    NULL,
    /* class_part_init    */	NULL,
    /* class_inited       */	FALSE,
    /* initialize         */    NULL,
    /* initialize_hook    */	NULL,
    /* realize            */    XtInheritRealize,
    /* actions            */    NULL,
    /* num_actions	  */	0,
    /* resources          */    NULL,
    /* num_resources      */    0,
    /* xrm_class          */    NULLQUARK,
    /* compress_motion	  */	TRUE,
    /* compress_exposure  */	TRUE,
    /* compress_enterleave*/	TRUE,
    /* visible_interest   */    FALSE,
    /* destroy            */    NULL,
    /* resize             */    XtInheritResize,
    /* expose             */    NULL,
    /* set_values         */    NULL,
    /* set_values_hook    */	NULL,
    /* set_values_almost  */    XtInheritSetValuesAlmost,
    /* get_values_hook    */	NULL,
    /* accept_focus       */    NULL,
    /* version            */	XtVersion,
    /* callback_private   */    NULL,
    /* tm_table           */    NULL,
    /* query_geometry     */	bbPreferredSize, /*XtInheritQueryGeometry,*/
    /* display_accelerator*/	XtInheritDisplayAccelerator,
    /* extension          */	NULL
  },{
/* composite_class fields */
    /* geometry_manager   */    bbGeometryManager,
    /* change_managed     */    XtInheritChangeManaged,
    /* insert_child	  */	XtInheritInsertChild,
    /* delete_child	  */	XtInheritDeleteChild,
    /* extension          */	NULL
  },{
/* Bb class fields */
    /* empty		  */	0,
  }
};

WidgetClass bbWidgetClass = (WidgetClass)&bbClassRec;

/*ARGSUSED*/
static XtGeometryResult
bbPreferredSize(widget, constraint, preferred)
    Widget widget;
    XtWidgetGeometry *constraint, *preferred;
{
    return XtGeometryYes;
}

/*ARGSUSED*/
static XtGeometryResult
bbGeometryManager(w, request, reply)
    Widget		w;
    XtWidgetGeometry	*request;
    XtWidgetGeometry	*reply;	/* RETURN */

{
    /* Allow any and all changes to the geometry */
    if (request->request_mode & CWWidth)
	w->core.width = request->width;
    if (request->request_mode & CWHeight)
	w->core.height = request->height;
    if (request->request_mode & CWBorderWidth)
	w->core.border_width = request->border_width;
    if (request->request_mode & CWX)
	w->core.x = request->x;
    if (request->request_mode & CWY)
	w->core.y = request->y;

    return XtGeometryYes;
}

#endif /* NO_WIDGETS */

/* why isn't this one standard? */
static char *
strndup(str, n)
    char       *str;
    int        n;
{
    register char *t;

    if (n < 0)
    	n = 0;
    if ((t = malloc((ALLOC_T)n)) != 0)
    	(void)memcpy(t, str, (SIZE_T)n);
    return t;
}

static void
free_selection(tw)
	TextWindow tw;
{
	if (tw->selection_len != 0) {
		free((char *)(tw->selection_data));
		tw->selection_len = 0;
	}
}

#if !NO_WIDGETS
static int dont_update_sb = FALSE;

static void set_scroll_window P((int));
static void
set_scroll_window(n)
    int n;
{
    register WINDOW *wp;
    for_each_window(wp) {
	if (n-- == 0)
	    break;
    }
    if (n < 0)
	set_curwp(wp);
}
#endif /* !NO_WIDGETS */

#if OL_WIDGETS
static void
JumpProc(scrollbar, num, cbs)
    Widget scrollbar;
    XtPointer num;
    OlScrollbarVerify *cbs;
{
    if ((int) num >= cur_win->nscrollbars)
	return;
    set_scroll_window((int)num);
    mvupwind(TRUE, 
             line_no(curwp->w_bufp, curwp->w_line.l) - cbs->new_location);
    dont_update_sb = TRUE;
    (void)update(FALSE);
    dont_update_sb = FALSE;
    x_flush();
}

static void
grip_moved(slider, num, cbs)
    Widget slider;
    XtPointer num;
    OlSliderVerify *cbs;
{
    register WINDOW *wp, *saved_curwp;
    int i, nlines;
    i = (int) num;
    for_each_window(wp) {
	if (i-- == 0)
	    break;
    }
    if (!wp)
	return;
    saved_curwp = curwp;
    nlines = - cbs->new_location;
    if (nlines == 0) {
	delwp(wp);
	if (saved_curwp == wp)
	    saved_curwp = wheadp;
    }
    else {
	curwp = wp;
	resize(TRUE, nlines);
    }
    set_curwp(saved_curwp);
    (void) update(FALSE);
    x_flush();
}

static void
update_scrollbar_sizes()
{
    register WINDOW *wp;
    int i, newsbcnt;
    Dimension new_height;

    i=0;
    for_each_window(wp)
	i++;
    newsbcnt=i;

    /* Create any needed new scrollbars and sliders */
    for (i = cur_win->nscrollbars+1; i <= newsbcnt; i++) 
	if (cur_win->scrollbars[i] == NULL) {
	    cur_win->scrollbars[i] = XtVaCreateWidget(
		    "scrollbar",
		    scrollbarWidgetClass,
		    cur_win->pane,
		    NULL);
	    XtAddCallback(cur_win->scrollbars[i],
		    XtNsliderMoved, JumpProc, (XtPointer) i);
	    cur_win->sliders[i] = XtVaCreateWidget(
		    "slider",
		    sliderWidgetClass,
		    cur_win->pane,
		    NULL);
	    XtAddCallback(cur_win->sliders[i],
		    XtNsliderMoved, grip_moved, (XtPointer) i);
	}

    /* Unmanage current set of scrollbars */
    if (cur_win->nscrollbars > 0)
	XtUnmanageChildren(cur_win->scrollbars, 
	                   (Cardinal) (cur_win->nscrollbars));
    if (cur_win->nscrollbars > 1)
	XtUnmanageChildren(cur_win->sliders, 
	                   (Cardinal) (cur_win->nscrollbars - 1));

    /* Set sizes and positions on scrollbars and sliders */
    cur_win->nscrollbars = newsbcnt;
    i=0;
    for_each_window(wp) {
	new_height = wp->w_ntrows * cur_win->char_height;
	XtVaSetValues(cur_win->scrollbars[i],
	    XtNy,	wp->w_toprow * cur_win->char_height,
	    XtNheight,	new_height,
	    XtNsliderMin,	1,
	    XtNsliderMax,	200,
	    XtNproportionLength, 2,
	    XtNsliderValue,	3,
	    NULL);
	if (wp->w_wndp) {
	    XtVaSetValues(cur_win->sliders[i],
		XtNy,		wp->w_toprow * cur_win->char_height,
		XtNheight,	(wp->w_ntrows + wp->w_wndp->w_ntrows + 1)
					    * cur_win->char_height,
		XtNsliderMax,	0,
		XtNsliderMin,	-(wp->w_ntrows + wp->w_wndp->w_ntrows),
		XtNsliderValue, -wp->w_ntrows,
		XtNstopPosition, OL_GRANULARITY,
		XtNendBoxes,	FALSE,
		XtNdragCBType,	OL_RELEASE,
		XtNbackground,	cur_win->fg,
		NULL);
	}
	wp->w_flag &= ~WFSBAR;
	update_scrollbar(wp);
	i++;
    }

    /* Manage the sliders */
    if (cur_win->nscrollbars > 1)
	XtManageChildren(cur_win->sliders,
			    (Cardinal) (cur_win->nscrollbars - 1));

    /* Manage the current set of scrollbars */
    XtManageChildren(cur_win->scrollbars, 
	                   (Cardinal) (cur_win->nscrollbars));


    for (i=0; i<cur_win->nscrollbars; i++)
	XRaiseWindow(dpy, XtWindow(cur_win->scrollbars[i]));
}

#else
#if MOTIF_WIDGETS

static void
JumpProc(scrollbar, num, call_data)
    Widget scrollbar;
    XtPointer num;
    XtPointer call_data;
{
    int lcur;
    lookfor_sb_resize = FALSE;
    if ((int) num >= cur_win->nscrollbars)
	return;
    set_scroll_window((int)num);
    lcur = line_no(curwp->w_bufp, curwp->w_line.l);
    mvupwind(TRUE, lcur - ((XmScrollBarCallbackStruct *)call_data)->value);
    dont_update_sb = TRUE;
    (void)update(FALSE);
    dont_update_sb = FALSE;
    x_flush();
}

static void
update_scrollbar_sizes()
{
    register WINDOW *wp;
    int i, newsbcnt;
    Dimension new_height;
    Widget *children;
    int num_children;

    i=0;
    for_each_window(wp)
	i++;
    newsbcnt=i;

    /* Remove event handlers on sashes */
    XtVaGetValues(cur_win->pane,
	XmNchildren, &children,
	XmNnumChildren, &num_children,
	NULL);
    while (num_children-- > 0)
	if (XmIsSash(children[num_children]))
	    XtRemoveEventHandler(
		children[num_children],
		ButtonReleaseMask,
		FALSE,
		pane_button,
		NULL);

    /* Create any needed new scrollbars */
    for (i = cur_win->nscrollbars+1; i <= newsbcnt; i++) 
	if (cur_win->scrollbars[i] == NULL) {
	    cur_win->scrollbars[i] = XtVaCreateWidget(
		    "scrollbar",
		    xmScrollBarWidgetClass,
		    cur_win->pane,
		    XmNsliderSize,	1,
		    XmNvalue,		1,
		    XmNminimum,		1,
		    XmNmaximum,		2,	/* so we don't get warning */
		    XmNorientation,	XmVERTICAL,
		    NULL);
	    XtAddCallback(cur_win->scrollbars[i],
	            XmNvalueChangedCallback, JumpProc, (XtPointer) i);
	    XtAddCallback(cur_win->scrollbars[i],
	            XmNdragCallback, JumpProc, (XtPointer) i);
	    XtAddEventHandler(
		    cur_win->scrollbars[i],
		    StructureNotifyMask,
		    FALSE,
		    grip_moved,
		    (XtPointer) i);
	}

    /* Unmanage current set of scrollbars */
    if (cur_win->nscrollbars >= 0)
	XtUnmanageChildren(cur_win->scrollbars, 
	                   (Cardinal) (cur_win->nscrollbars + 1));

    /* Set sizes on scrollbars */
    cur_win->nscrollbars = newsbcnt;
    i=0;
    for_each_window(wp) {
	new_height = wp->w_ntrows * cur_win->char_height;
	XtVaSetValues(cur_win->scrollbars[i],
	    XmNallowResize,		TRUE,
	    XmNheight,			new_height,
	    XmNpaneMinimum,		1,
	    XmNpaneMaximum,		1000,
	    XmNshowArrows,		wp->w_ntrows > 3 ? TRUE : FALSE,
	    NULL);
	wp->w_flag &= ~WFSBAR;
	update_scrollbar(wp);
	i++;
    }
    XtVaSetValues(cur_win->scrollbars[i],
	    XmNheight,			cur_win->char_height-1,
	    XmNallowResize,		FALSE,
	    XmNpaneMinimum,		cur_win->char_height-1,
	    XmNpaneMaximum,		cur_win->char_height-1,
	    XmNshowArrows,		FALSE,
	    NULL);

    /* Manage the current set of scrollbars */
    XtManageChildren(cur_win->scrollbars, 
	                   (Cardinal) (cur_win->nscrollbars + 1));

    /* Add event handlers for sashes */
    XtVaGetValues(cur_win->pane,
	XmNchildren, &children,
	XmNnumChildren, &num_children,
	NULL);
    while (num_children-- > 0)
	if (XmIsSash(children[num_children]))
	    XtAddEventHandler(
		children[num_children],
		ButtonReleaseMask,
		FALSE,
		pane_button,
		NULL);
}

#else
#if NO_WIDGETS
static void
update_scrollbar_sizes()
{
    register WINDOW *wp;
    int i, newsbcnt;
    Dimension new_height;
    Cardinal nchildren;

    i=0;
    for_each_window(wp)
	i++;
    newsbcnt=i;

    /* Create any needed new scrollbars and grips */
    for (i = cur_win->nscrollbars; i < newsbcnt; i++) 
	if (cur_win->scrollbars[i] == NULL) {
	    cur_win->scrollbars[i] = XtVaCreateWidget(
		    "scrollbar",
		    coreWidgetClass,
		    cur_win->pane,
		    XtNborderWidth,	0,
		    XtNheight,		1,
		    XtNwidth,		1,
		    NULL);
	    cur_win->grips[i] = XtVaCreateWidget(
		    "resizeGrip",
		    coreWidgetClass,
		    cur_win->pane,
		    XtNbackground,	cur_win->fg,
		    XtNborderWidth,	0,
		    XtNheight,		1,
		    XtNwidth,		1,
		    NULL);
	}

    if (cur_win->nscrollbars > newsbcnt) {
	nchildren = cur_win->nscrollbars - newsbcnt;
	XtUnmanageChildren(cur_win->scrollbars+newsbcnt, nchildren);
	XtUnmanageChildren(cur_win->grips+newsbcnt-1,    nchildren);
    }
    else if (cur_win->nscrollbars < newsbcnt) {
	nchildren = newsbcnt - cur_win->nscrollbars;
	XtManageChildren(cur_win->scrollbars+cur_win->nscrollbars, nchildren);
	if (cur_win->nscrollbars > 0)
	    XtManageChildren(cur_win->grips+cur_win->nscrollbars-1, nchildren);
    }

    cur_win->nscrollbars = newsbcnt;
    /* Set sizes and positions on scrollbars and grips */
    i=0;
    for_each_window(wp) {
	new_height = wp->w_ntrows * cur_win->char_height;
	XtVaSetValues(cur_win->scrollbars[i],
	    XtNx,		1,
	    XtNy,		wp->w_toprow * cur_win->char_height,
	    XtNheight,		new_height,
	    XtNwidth,		cur_win->pane_width,
	    NULL);
	/* clear the "thumb" */
	draw_thumb(cur_win->scrollbars[i], 0, (int)new_height-1, False);
	cur_win->scrollinfo[i].totlen = new_height;
	cur_win->scrollinfo[i].top = 
	cur_win->scrollinfo[i].bot = new_height - 1;
	if (wp->w_wndp) {
	    XtVaSetValues(cur_win->grips[i],
		XtNx,		1,
		XtNy,		(wp->w_wndp->w_toprow-1) * cur_win->char_height,
		XtNheight,	cur_win->char_height,
		XtNwidth,	cur_win->pane_width,
		NULL);
	}
	wp->w_flag &= ~WFSBAR;
	update_scrollbar(wp);
	i++;
    }

    /*
     * Set the cursors... It would be nice if we could do this in the
     * initialization above, but the widget needs to be realized for this
     * to work
     */
    for (i=0; i<cur_win->nscrollbars; i++) {
	if (XtIsRealized(cur_win->scrollbars[i]))
	    XDefineCursor(dpy,
		    XtWindow(cur_win->scrollbars[i]),
		    cur_win->curs_sb_v_double_arrow);
	if (i < cur_win->nscrollbars-1 && XtIsRealized(cur_win->grips[i]))
	    XDefineCursor(dpy,
		    XtWindow(cur_win->grips[i]),
		    cur_win->curs_double_arrow);
    }
}


/*
 * The X11R5 Athena scrollbar code was used as a reference for writing
 * draw_thumb and parts of update_thumb.
 */

static void
draw_thumb(w, top, bot, dofill)
    Widget w;
    int top, bot;
    Bool dofill;
{
    if (bot >= 0 && top >= 0 && bot > top) {
	UINT length = bot - top;
	if (dofill)
	    XFillRectangle(XtDisplay(w), XtWindow(w), cur_win->scrollbargc,
			    1, top, cur_win->pane_width-2, length);
	else
	    XClearArea(XtDisplay(w), XtWindow(w),
			    1, top, cur_win->pane_width-2, length, FALSE);
    }
}


#define MIN_THUMB_SIZE 7

static void
update_thumb(barnum,newtop,newlen)
    int barnum;
    int newtop;
    int newlen;
{
    int oldtop, oldbot, newbot, totlen;
    Widget w = cur_win->scrollbars[barnum];

    oldtop = cur_win->scrollinfo[barnum].top;
    oldbot = cur_win->scrollinfo[barnum].bot;
    totlen = cur_win->scrollinfo[barnum].totlen;
    newtop = min(newtop, totlen-3);
    newbot = newtop + max(newlen, MIN_THUMB_SIZE);
    newbot = min(newbot, totlen);
    cur_win->scrollinfo[barnum].top = newtop;
    cur_win->scrollinfo[barnum].bot = newbot;

    if (XtIsRealized(w)) {
	if (newtop < oldtop) draw_thumb(w, newtop, min(newbot, oldtop), True);
	if (newtop > oldtop) draw_thumb(w, oldtop, min(newtop, oldbot), False);
	if (newbot < oldbot) draw_thumb(w, max(newbot, oldtop), oldbot, False);
	if (newbot > oldbot) draw_thumb(w, max(newtop, oldbot), newbot, True);
    }
}

/*ARGSUSED*/
static void
x_expose_pane(w, unused, ev, continue_to_dispatch)
    Widget	w;
    XtPointer	unused;
    XEvent     *ev;
    Boolean    *continue_to_dispatch;
{
    int i;

    if (ev->type != Expose)
	return;

    /* FIXME: Make this smarter about updating only newly exposed scrollbars */
    for (i=0; i < cur_win->nscrollbars; i++)
	draw_thumb(cur_win->scrollbars[i],
		cur_win->scrollinfo[i].top,
		cur_win->scrollinfo[i].bot,
		TRUE);
}


static void
do_scroll(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    static enum { none, forward, backward, drag } scrollmode = none;
    int pos;
    register WINDOW *wp;
    register int i;
    XEvent nev;
    int count;

    /* 
     * Return immediately if behind in processing motion events.  Note:
     * If this is taken out, scrolling is actually smoother, but sometimes
     * takes a while to catch up.  I should think that performance would
     * be horrible on a slow server.
     */

    if (scrollmode == drag
     && event->type == MotionNotify
     && XtAppPending(cur_win->app_context)
     && XtAppPeekEvent(cur_win->app_context, &nev)
     && (nev.type == MotionNotify || nev.type == ButtonRelease))
	return;

    if (*num_params != 1)
	return;

    /* Determine vertical position */
    switch (event->type) {
	case MotionNotify :
	    pos = event->xmotion.y;
	    break;
	case ButtonPress :
	case ButtonRelease :
	    pos = event->xbutton.y;
	    break;
	default :
	    return;
    }

    /* Determine scrollbar number and corresponding vile window */
    i = 0;
    for_each_window (wp) {
	if (cur_win->scrollbars[i] == w)
	    break;
	i++;
    }

    if (!wp)
	return;

    if (pos < 0)
	pos = 0;
    if (pos > cur_win->scrollinfo[i].totlen)
	pos = cur_win->scrollinfo[i].totlen;

    switch (**params) {
	case 'E' :	/* End */
	    if (cur_win->scroll_repeat_id != (XtIntervalId) 0) {
		XtRemoveTimeOut(cur_win->scroll_repeat_id);
		cur_win->scroll_repeat_id = (XtIntervalId) 0;
	    }
	    XDefineCursor(dpy, XtWindow(w), cur_win->curs_sb_v_double_arrow);
	    scrollmode = none;
	    break;
	case 'F' :	/* Forward */
	    if (scrollmode != none)
		break;
	    count = (pos / cur_win->char_height) + 1;
	    scrollmode = forward;
	    XDefineCursor(dpy, XtWindow(w), cur_win->curs_sb_up_arrow);
	    goto do_scroll_common;
	case 'B' :	/* Backward */
	    if (scrollmode != none)
		break;
	    count = -((pos / cur_win->char_height) + 1);
	    scrollmode = backward;
	    XDefineCursor(dpy, XtWindow(w), cur_win->curs_sb_down_arrow);
do_scroll_common:
	    set_curwp(wp);
	    mvdnwind(TRUE, count);
	    cur_win->scroll_repeat_id = XtAppAddTimeOut(
		    cur_win->app_context,
		    cur_win->scroll_repeat_timeout,
		    repeat_scroll,
		    (XtPointer) count);
	    (void)update(FALSE);
	    x_flush();
	    break;
	case 'S' :	/* StartDrag */
	    if (scrollmode == none) {
		set_curwp(wp);
		scrollmode = drag;
		XDefineCursor(dpy, XtWindow(w), cur_win->curs_sb_right_arrow);
	    }
	    /* fall through */
	case 'D' :	/* Drag */
	    if (scrollmode == drag) {
		int lcur = line_no(curwp->w_bufp, curwp->w_line.l);
		int ltarg = (line_count(curwp->w_bufp) * pos 
			    	/ cur_win->scrollinfo[i].totlen) + 1;
		mvupwind(TRUE, lcur-ltarg);
		(void)update(FALSE);
		x_flush();
	    }
	    break;
    }
}

/*ARGSUSED*/
static void
repeat_scroll(count, id)
    XtPointer count;
    XtIntervalId  *id;
{
    mvdnwind(TRUE, (int)count);
    cur_win->scroll_repeat_id = XtAppAddTimeOut(
	    cur_win->app_context,
	    cur_win->scroll_repeat_interval,
	    repeat_scroll,
	    (XtPointer) count);
    (void)update(FALSE);
    x_flush();
}

static void
resize_bar(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    static motion_permitted = False;
    int pos;
    register WINDOW *wp;
    register int i;
    XEvent nev;
    Window root, child;
    int rootx, rooty, winx;
    unsigned int mask;

    /* Return immediately if behind in processing motion events */
    if (motion_permitted
     && event->type == MotionNotify
     && XtAppPending(cur_win->app_context)
     && XtAppPeekEvent(cur_win->app_context, &nev)
     && (nev.type == MotionNotify || nev.type == ButtonRelease))
	return;

    if (*num_params != 1)
	return;

    switch (**params) {
	case 'S' :	/* Start */
	    motion_permitted = True;
	    return;
	case 'D' :	/* Drag */
	    if (!motion_permitted)
		return;
	    break;
	case 'E' :	/* End */
	    if (!motion_permitted)
		return;
	    motion_permitted = False;
	    break;
    }

    /* 
     * Determine vertical position relative to the widget we are moving...
     *
     * We call XQueryPointer here because the x,y position from
     * the event structure is unreliable since the widget may have
     * moved prior to receiving the event.  Use of position information
     * from the event structure caused a lot of jumpiness.  Interesting
     * effect, but not very desirable.
     */

    if (!XQueryPointer(dpy,
	    XtWindow(w),
	    &root, &child,
	    &rootx, &rooty,
	    &winx, &pos,
	    &mask))
	return;		/* Can't get valid position */
			/* FIXME: The above is fine for motion events, but
			 * simply returning will cause problems on
			 * ButtonRelease events.
			 */

    /* Determine grip number and corresponding vile window (above grip) */
    i = 0;
    for_each_window (wp) {
	if (cur_win->grips[i] == w)
	    break;
	i++;
    }

    if (!wp)
	return;

    if (pos < 0)
	pos -= cur_win->char_height;
    pos = pos / cur_win->char_height;

    if (pos) {
	int nlines;
	if (pos >= wp->w_wndp->w_ntrows)
	    pos = wp->w_wndp->w_ntrows - 1;

	nlines = wp->w_ntrows + pos;
	if (nlines < 1)
	    nlines = 1;
	set_curwp(wp);
	resize(TRUE, nlines);
	(void)update(FALSE);
	x_flush();
    }
}
#endif /* NO_WIDGETS  */
#endif /* MOTIF_WIDGETS */
#endif /* OL_WIDGETS */

void
update_scrollbar(uwp)
    WINDOW *uwp;
{
    WINDOW *wp;
    int i;
    int lnum, lcnt;

#if !NO_WIDGETS
    if (dont_update_sb)
	return;
#endif /* !NO_WIDGETS */

    i = 0;
    for_each_window(wp) {
	if (wp == uwp)
	    break;
	i++;
    }
    if (i >= cur_win->nscrollbars || (wp->w_flag & WFSBAR)) {
	/*
	 * update_scrollbar_sizes will recursively invoke update_scrollbar,
	 * but with WFSBAR disabled.
	 */
	update_scrollbar_sizes();
	return;
    }

    lnum = line_no(wp->w_bufp, wp->w_line.l);
    lnum = (lnum > 0) ? lnum : 1;
    lcnt = line_count(wp->w_bufp);
#if MOTIF_WIDGETS
    lcnt += 1;
    XtVaSetValues(cur_win->scrollbars[i],
	    XmNmaximum,		lcnt + wp->w_ntrows,
	    XmNsliderSize,	wp->w_ntrows,
	    XmNvalue,		lnum,
	    XmNpageIncrement,	wp->w_ntrows > 1 ? wp->w_ntrows-1 : 1,
	    NULL);
#else
#if OL_WIDGETS
    lcnt += 1;
    XtVaSetValues(cur_win->scrollbars[i],
	    XtNsliderMin,	1,
	    XtNsliderMax,	lcnt + wp->w_ntrows,
	    XtNproportionLength, wp->w_ntrows,
	    XtNsliderValue,	lnum,
	    NULL);
#else
#if NO_WIDGETS
    {
	int top, len;
	lcnt  = max(lcnt, 1);
	len   = (min(lcnt, wp->w_ntrows) * cur_win->scrollinfo[i].totlen 
				/ lcnt) + 1;
	top   = ((lnum-1) * cur_win->scrollinfo[i].totlen)
		    / lcnt;
	update_thumb(i, top, len);
    }
#endif /* NO_WIDGETS */
#endif /* OL_WIDGETS */
#endif /* MOTIF_WIDGETS */
}

#define XtNscrollbarWidth	"scrollbarWidth"
#define XtCScrollbarWidth	"ScrollbarWidth"
#define XtNfocusFollowsMouse	"focusFollowsMouse"
#define XtCFocusFollowsMouse	"FocusFollowsMouse"
#define XtNscrollbarOnLeft	"scrollbarOnLeft"
#define XtCScrollbarOnLeft	"ScrollbarOnLeft"
#define XtNmultiClickTime	"multiClickTime"
#define XtCMultiClickTime	"MultiClickTime"
#define XtNcharClass		"charClass"
#define XtCCharClass		"CharClass"
#if NO_WIDGETS
#define	XtNscrollRepeatTimeout	"scrollRepeatTimeout"
#define	XtCScrollRepeatTimeout	"ScrollRepeatTimeout"
#endif
#define	XtNscrollRepeatInterval	"scrollRepeatInterval"
#define	XtCScrollRepeatInterval	"ScrollRepeatInterval"
#define XtNpersistentSelections	"persistentSelections"
#define XtCPersistentSelections	"PersistentSelections"
#define XtNblinkInterval	"blinkInterval"
#define XtCBlinkInterval	"BlinkInterval"

static XtResource resources[] = {
#if NO_WIDGETS
    {
	XtNscrollRepeatTimeout,
	XtCScrollRepeatTimeout,
	XtRInt,
	sizeof(int),
	XtOffset(TextWindow, scroll_repeat_timeout),
	XtRImmediate,
	(XtPointer) 500			/* 1/2 second */
    },
#endif	/* NO_WIDGETS */
    {
	XtNscrollRepeatInterval,
	XtCScrollRepeatInterval,
	XtRInt,
	sizeof(int),
	XtOffset(TextWindow, scroll_repeat_interval),
	XtRImmediate,
	(XtPointer) 60			/* 60 milliseconds */
    },
    {
	XtNgeometry,
	XtCGeometry,
	XtRString,
	sizeof(String *),
	XtOffset(TextWindow, geometry),
	XtRImmediate,
	"80x36"
    },
    {
	XtNfont,
	XtCFont,
	XtRString,
	sizeof(String *),
	XtOffset(TextWindow, starting_fontname),
	XtRImmediate,
	XtDefaultFont 	/* used to be FONTNAME */
    },
    {
	XtNforeground,
	XtCForeground,
	XtRPixel,
	sizeof(Pixel),
	XtOffset(TextWindow, fg),
	XtRString,
	XtDefaultForeground
    },
    {
	XtNbackground,
	XtCBackground,
	XtRPixel,
	sizeof(Pixel),
	XtOffset(TextWindow, bg),
	XtRString,
	XtDefaultBackground
    },
    {
	XtNfocusFollowsMouse,
	XtCFocusFollowsMouse,
	XtRBool,
	sizeof(Bool),
	XtOffset(TextWindow, focus_follows_mouse),
	XtRImmediate,
	(XtPointer) False
    },
    {
	XtNmultiClickTime,
	XtCMultiClickTime,
	XtRInt,
	sizeof(Time),
	XtOffset(TextWindow, click_timeout),
	XtRImmediate,
	(XtPointer) 500
    },
    {
	XtNcharClass,
	XtCCharClass,
	XtRString,
	sizeof(String *),
	XtOffset(TextWindow, multi_click_char_class),
	XtRImmediate,
	NULL
    },
    {
	XtNscrollbarOnLeft,
	XtCScrollbarOnLeft,
	XtRBool,
	sizeof(Bool),
	XtOffset(TextWindow, scrollbar_on_left),
	XtRImmediate,
	(XtPointer) False
    },
    {
	XtNscrollbarWidth,
	XtCScrollbarWidth,
	XtRInt,
	sizeof(int),
	XtOffset(TextWindow, pane_width),
	XtRImmediate,
	(XtPointer) PANE_WIDTH_DEFAULT
    },
    {
	XtNpersistentSelections,
	XtCPersistentSelections,
	XtRBool,
	sizeof(Bool),
	XtOffset(TextWindow, persistent_selections),
	XtRImmediate,
	(XtPointer) True
    },
    {
	XtNblinkInterval,
	XtCBlinkInterval,
	XtRInt,
	sizeof(int),
	XtOffset(TextWindow, blink_interval),
	XtRImmediate,
	(XtPointer) -666		/* 2/3 second; only when highlighted */
    },
};

#define CHECK_MIN_MAX(v,min,max)	\
	do {				\
	    if ((v) > (max))		\
		(v)=(max);		\
	    else if ((v) < (min))	\
		(v) = (min);		\
	} while (0)

/* ARGSUSED */
void
x_preparse_args(pargc, pargv)
    int        *pargc;
    char     ***pargv;
{
    XFontStruct *pfont;
    XGCValues   gcvals;
    ULONG	gcmask;
    int		geo_mask, startx, starty;
    Cardinal	start_cols, start_rows;
    /* Note: apollo compiler doesn't like empty declaration-list */
#if MOTIF_WIDGETS || NO_WIDGETS
    static XrmOptionDescRec options[] = {
/* FIXME: Implement scrollbarOnLeft for OL_WIDGETS */
	{"-leftbar",	"*scrollbarOnLeft", XrmoptionNoArg,	"true" },
	{"-rightbar",	"*scrollbarOnLeft", XrmoptionNoArg,	"false" },
    };
#endif /* MOTIF_WIDGETS || NO_WIDGETS*/
#if MOTIF_WIDGETS || OL_WIDGETS
    static XtActionsRec new_actions[] = {
	{ "ConfigureBar", configure_bar }
    };
    static String fallback_resources[]= {
	"*scrollbar.translations:#override \\n\\\n\
		Ctrl<Btn1Down>:ConfigureBar(Split) \\n\\\n\
		Ctrl<Btn2Down>:ConfigureBar(Kill) \\n\\\n\
		Ctrl<Btn3Down>:ConfigureBar(Only)",
	"*scrollPane.background:grey80",
	"*scrollbar.background:grey60",
	NULL
    };
#else
#if NO_WIDGETS
    static XtActionsRec new_actions[] = {
	{ "ConfigureBar", configure_bar },
	{ "DoScroll", do_scroll },
	{ "ResizeBar", resize_bar }
    };
    static String fallback_resources[]= {
	"*scrollbar.translations:#override \\n\\\n\
		Ctrl<Btn1Down>:ConfigureBar(Split) \\n\\\n\
		Ctrl<Btn2Down>:ConfigureBar(Kill) \\n\\\n\
		Ctrl<Btn3Down>:ConfigureBar(Only) \\n\\\n\
		<Btn1Down>:DoScroll(Forward) \\n\\\n\
		<Btn2Down>:DoScroll(StartDrag) \\n\\\n\
		<Btn3Down>:DoScroll(Backward) \\n\\\n\
		<Btn2Motion>:DoScroll(Drag) \\n\\\n\
		<BtnUp>:DoScroll(End)",
	"*resizeGrip.translations:#override \\n\\\n\
		<BtnDown>:ResizeBar(Start) \\n\\\n\
		<BtnMotion>:ResizeBar(Drag) \\n\\\n\
		<BtnUp>:ResizeBar(End)",
	NULL
    };
    static char stippled_pixmap_bits[] = { '\002', '\001' };
#endif /* NO_WIDGETS */
#endif /* MOTIF_WIDGETS || OL_WIDGETS */

    cur_win = (TextWindow) XCalloc(TextWindowRec);

    if (!cur_win) {
	(void)fprintf(stderr, "insufficient memory, exiting\n");
	ExitProgram(BADEXIT);
    }


    	
#if OL_WIDGETS
    /* There is a cryptic statement in the poor documentation that I have
     * on OpenLook that OlToolkitInitialize is now preferred to the older
     * OlInitialize (which is used in the examples and is documented better).
     * The documentation I have says that OlToolkitInitialize returns a
     * widget.  I don't believe it, nor do I understand what kind of widget
     * it would be.  I get the impression that it takes one argument, but
     * I don't know what that argument is supposed to be.
     *
     */
    (void) OlToolkitInitialize( NULL );
#endif /* OL_WIDGETS */

    cur_win->top_widget = XtVaAppInitialize(
	    &cur_win->app_context,
	    MY_CLASS,
#if MOTIF_WIDGETS || NO_WIDGETS
	    options, XtNumber(options),
#else
	    (XrmOptionDescRec *)0, 0,
#endif
	    (Cardinal *)pargc, *pargv,
	    fallback_resources,
	    XtNgeometry,	NULL,
	    NULL);
    dpy = XtDisplay(cur_win->top_widget);

    XtVaSetValues(cur_win->top_widget,
	    XtNinput,	TRUE,
	    NULL);

    XtGetApplicationResources(
	    cur_win->top_widget, 
	    (XtPointer)cur_win,
	    resources,
	    XtNumber(resources),
	    (ArgList)0,
	    0);

    /* Initialize atoms which may be needed to get the fully specified name */
    atom_FONT 		= XInternAtom(dpy, "FONT", False);
    atom_FOUNDRY	= XInternAtom(dpy, "FOUNDRY", False);
    atom_WEIGHT_NAME	= XInternAtom(dpy, "WEIGHT_NAME", False);
    atom_SLANT		= XInternAtom(dpy, "SLANT", False);
    atom_SETWIDTH_NAME	= XInternAtom(dpy, "SETWIDTH_NAME", False);
    atom_PIXEL_SIZE	= XInternAtom(dpy, "PIXEL_SIZE", False);
    atom_RESOLUTION_X	= XInternAtom(dpy, "RESOLUTION_X", False);
    atom_RESOLUTION_Y	= XInternAtom(dpy, "RESOLUTION_Y", False);
    atom_SPACING	= XInternAtom(dpy, "SPACING", False);
    atom_AVERAGE_WIDTH	= XInternAtom(dpy, "AVERAGE_WIDTH", False);
    atom_CHARSET_REGISTRY = XInternAtom(dpy, "CHARSET_REGISTRY", False);
    atom_CHARSET_ENCODING = XInternAtom(dpy, "CHARSET_ENCODING", False);

    pfont = query_font(cur_win, cur_win->starting_fontname);
    if (!pfont) {
	pfont = query_font(cur_win, FONTNAME);
	if (!pfont) {
	    (void)fprintf(stderr, "couldn't get font \"%s\" or \"%s\", exiting\n",
		    cur_win->starting_fontname, FONTNAME);
	    ExitProgram(BADEXIT);
	}
    }
    (void) set_character_class(cur_win->multi_click_char_class);

    /*
     * Look at our copy of the geometry resource to obtain the dimensions of
     * the window in characters.  We've provided a default value of 80x36
     * so there'll always be something to parse.  We still need to check
     * the return mask since the user may specify a position, but no size.
     */
    geo_mask = XParseGeometry(cur_win->geometry,
    		&startx, &starty,
		&start_cols, &start_rows);

    cur_win->rows = (geo_mask & HeightValue) ? start_rows : 36;
    cur_win->cols = (geo_mask & WidthValue) ? start_cols : 80;

    /* 
     * Fix up the geometry resource of top level shell providing initial
     * position if so requested by user.
     */

    if (geo_mask & (XValue | YValue)) {
	char *gp = cur_win->geometry;
	while (*gp && *gp != '+' && *gp != '-')
	    gp++;			/* skip over width and height */
	if (*gp)
	    XtVaSetValues(cur_win->top_widget,
			XtNgeometry,	gp,
			NULL);
    }

    /* Sanity check values obtained from XtGetApplicationResources */
    CHECK_MIN_MAX(cur_win->pane_width, PANE_WIDTH_MIN, PANE_WIDTH_MAX);
    CHECK_MIN_MAX(cur_win->rows, MINROWS, MAXROWS);
    CHECK_MIN_MAX(cur_win->cols, 20, MAXCOLS);

#if MOTIF_WIDGETS
    cur_win->form_widget = XtVaCreateManagedWidget(
	    "form",
	    xmFormWidgetClass,
	    cur_win->top_widget,
	    NULL);
#else
#if OL_WIDGETS
    cur_win->form_widget = XtVaCreateManagedWidget(
	    "form",
	    formWidgetClass,
	    cur_win->top_widget,
	    NULL);
#else
#if NO_WIDGETS
    cur_win->form_widget = XtVaCreateManagedWidget(
	    "form",
	    bbWidgetClass,
	    cur_win->top_widget,
	    XtNwidth,			x_width(cur_win)
	    					+ cur_win->pane_width + 2,
	    XtNheight,			x_height(cur_win),
	    XtNbackground,		cur_win->bg,
	    NULL);
#endif /* NO_WIDGETS */
#endif /* OL_WIDGETS */
#endif /* MOTIF_WIDGETS */

    cur_win->screen = XtVaCreateManagedWidget(
	    "screen",
#if MOTIF_WIDGETS
	    xmPrimitiveWidgetClass,
#else
	    coreWidgetClass,
#endif
	    cur_win->form_widget,
	    XtNwidth,			x_width(cur_win),
	    XtNheight,			x_height(cur_win),
	    XtNborderWidth, 		0,
	    XtNbackground,		cur_win->bg,
#if MOTIF_WIDGETS
	    XmNresizable,		TRUE,
	    XmNbottomAttachment,	XmATTACH_FORM,
	    XmNtopAttachment,		XmATTACH_FORM,
	    XmNleftAttachment,		XmATTACH_FORM,
	    XmNrightAttachment,		XmATTACH_NONE,
#else
#if OL_WIDGETS
	    XtNyAttachBottom,		TRUE,
	    XtNyVaryOffset,		FALSE,
	    XtNxAddWidth,		TRUE,
	    XtNyAddHeight,		TRUE,
#else
#if NO_WIDGETS
	    XtNx,			cur_win->scrollbar_on_left 
					    ? cur_win->pane_width+2
					    : 0,
	    XtNy,			0,
#endif	/* NO_WIDGETS */
#endif	/* OL_WIDGETS */
#endif	/* MOTIF_WIDGETS */
	    NULL);

    gcmask = GCForeground | GCBackground | GCFont;
    gcvals.foreground = cur_win->fg;
    gcvals.background = cur_win->bg;
    gcvals.font = cur_win->pfont->fid;
    cur_win->textgc = XCreateGC(dpy,
            DefaultRootWindow(dpy),
	    gcmask, &gcvals);

    gcvals.foreground = cur_win->bg;
    gcvals.background = cur_win->fg;
    gcvals.font = cur_win->pfont->fid;
    cur_win->reversegc = XCreateGC(dpy, 
            DefaultRootWindow(dpy),
	    gcmask, &gcvals);

#if NO_WIDGETS
    gcmask = GCFillStyle | GCStipple | GCForeground | GCBackground;
    /* FIXME: Add resource for scrollbar slider color */
    gcvals.foreground = cur_win->fg;
    gcvals.background = cur_win->bg;
    gcvals.fill_style = FillOpaqueStippled;
    gcvals.stipple = XCreatePixmapFromBitmapData(dpy,
	    DefaultRootWindow(dpy),
	    stippled_pixmap_bits,
	    2, 2,
	    cur_win->fg, cur_win->bg,
	    1);
    cur_win->scrollbargc = XCreateGC(dpy,
	    DefaultRootWindow(dpy),
	    gcmask, &gcvals);
#endif /* NO_WIDGETS */

    XtAppAddActions(cur_win->app_context, new_actions, XtNumber(new_actions));

#if MOTIF_WIDGETS
    cur_win->pane = XtVaCreateManagedWidget(
	    "scrollPane",
	    xmPanedWindowWidgetClass,
	    cur_win->form_widget,
	    XtNwidth,			cur_win->pane_width,
	    XmNbottomAttachment,	XmATTACH_FORM,
	    XmNtopAttachment,		XmATTACH_FORM,
	    XmNleftAttachment,		XmATTACH_WIDGET,
	    XmNleftWidget,		cur_win->screen,
	    XmNrightAttachment,		XmATTACH_FORM,
	    XmNspacing,			cur_win->char_height,
	    XmNsashIndent,		2,
	    XmNsashWidth,		cur_win->pane_width - 4,
	    XmNmarginHeight,		0,
	    XmNmarginWidth,		0,
	    XmNseparatorOn,		FALSE,
	    NULL);
#else
#if OL_WIDGETS
    cur_win->pane = XtVaCreateManagedWidget(
	    "scrollPane",
	    bulletinBoardWidgetClass,
	    cur_win->form_widget,
	    XtNwidth,			cur_win->pane_width,
	    XtNheight,			x_height(cur_win),
	    XtNxRefWidget,		cur_win->screen,
	    XtNyAttachBottom,		TRUE,
	    XtNyVaryOffset,		FALSE,
	    XtNxAddWidth,		TRUE,
	    XtNyAddHeight,		TRUE,
	    XtNlayout,			OL_IGNORE,
	    NULL);
#else
#if NO_WIDGETS
    cur_win->pane = XtVaCreateManagedWidget(
	    "scrollPane",
	    bbWidgetClass,
	    cur_win->form_widget,
	    XtNwidth,			cur_win->pane_width + 2,
	    XtNheight,			x_height(cur_win)
	    					- cur_win->char_height,
	    XtNx,			cur_win->scrollbar_on_left
					    ? 0
					    : x_width(cur_win),
	    XtNy,			0,
	    XtNborderWidth,		0,
	    XtNbackground,		cur_win->fg,
	    NULL);
    cur_win->curs_sb_v_double_arrow = 
    		XCreateFontCursor(dpy, XC_sb_v_double_arrow);
    cur_win->curs_sb_up_arrow = XCreateFontCursor(dpy, XC_sb_up_arrow);
    cur_win->curs_sb_down_arrow = XCreateFontCursor(dpy, XC_sb_down_arrow);
    cur_win->curs_sb_left_arrow = XCreateFontCursor(dpy, XC_sb_left_arrow);
    cur_win->curs_sb_right_arrow = XCreateFontCursor(dpy, XC_sb_right_arrow);
    cur_win->curs_double_arrow = XCreateFontCursor(dpy, XC_double_arrow);
#endif	/* NO_WIDGETS */
#endif	/* OL_WIDGETS */
#endif	/* MOTIF_WIDGETS */

#if NO_WIDGETS
    cur_win->nscrollbars = 0;
#else
    cur_win->nscrollbars = -1;
#endif

    /*
     * Move scrollbar to the left if requested via the resources. 
     * Note that this is handled elsewhere for NO_WIDGETS.
     */
    if (cur_win->scrollbar_on_left) {
/* FIXME: Implement scrollbarOnLeft for OL_WIDGETS */
#if MOTIF_WIDGETS
	XtVaSetValues(cur_win->pane,
	    XmNleftAttachment,	XmATTACH_FORM,
	    XmNrightAttachment, XmATTACH_WIDGET,
	    XmNrightWidget,	cur_win->screen,
	    NULL);
	XtVaSetValues(cur_win->screen,
	    XmNleftAttachment,	XmATTACH_NONE,
	    XmNrightAttachment,	XmATTACH_FORM,
	    NULL);
#endif /* MOTIF_WIDGETS */
    }

    XtAddEventHandler(
	    cur_win->screen,
	    KeyPressMask,
	    FALSE,
	    x_key_press,
	    (XtPointer)0);

    XtAddEventHandler(
	    cur_win->screen,
	    ButtonPressMask | ButtonReleaseMask | PointerMotionMask 
	    	| ExposureMask | VisibilityChangeMask,
	    TRUE,
	    x_process_event,
	    (XtPointer)0);
    
    XtAddEventHandler(
	    cur_win->top_widget,
	    StructureNotifyMask,
	    FALSE,
	    x_configure_window,
	    (XtPointer)0);

    XtAddEventHandler(
	    cur_win->top_widget,
	    EnterWindowMask | LeaveWindowMask | FocusChangeMask,
	    FALSE,
	    x_change_focus,
	    (XtPointer)0);

#if NO_WIDGETS
    XtAddEventHandler(
	    cur_win->pane,
	    ExposureMask,
	    FALSE,
	    x_expose_pane,
	    (XtPointer)0);
#endif /* NO_WIDGETS */

    cur_win->base_width = -1;	/* force base width to be set when configured */
    XtRealizeWidget(cur_win->top_widget);

    cur_win->win = XtWindow(cur_win->screen);

    /* We wish to participate in the "delete window" protocol */
    atom_WM_PROTOCOLS = XInternAtom(dpy, "WM_PROTOCOLS", False);
    atom_WM_DELETE_WINDOW = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    atom_WM_TAKE_FOCUS = XInternAtom(dpy, "WM_TAKE_FOCUS", False);
    {
	Atom atoms[2];
	int i = 0;
	atoms[i++] = atom_WM_DELETE_WINDOW;
#if 0
	if (!cur_win->focus_follows_mouse)
	    atoms[i++] = atom_WM_TAKE_FOCUS;
#endif
	XSetWMProtocols(dpy,
		XtWindow(cur_win->top_widget),
		atoms,
		i);
    }
    XtAddEventHandler(
	    cur_win->top_widget,
	    NoEventMask,
	    TRUE,
	    x_wm_delwin,
	    (XtPointer)0);

    /* Change screen cursor to insertion bar */
    cur_win->curs_xterm = XCreateFontCursor(dpy, XC_xterm);
    XDefineCursor(dpy, XtWindow(cur_win->screen), cur_win->curs_xterm);
}

char *
x_current_fontname()
{
    return cur_win->fontname;
}

static char *
x_get_font_atom_property(pf, atom)
    XFontStruct *pf;
    Atom atom;
{
    XFontProp *pp;
    int i;
    char *retval = NULL;

    for (i=0, pp = pf->properties; i < pf->n_properties; i++, pp++)
	if (pp->name == atom) {
	    retval = XGetAtomName(dpy, pp->card32);
	    break;
	}
    return retval;
}

static XFontStruct *
query_font(tw, fname)
	TextWindow tw;
	char	*fname;
{
    XFontStruct *pf;

    if ((pf = XLoadQueryFont(dpy, fname)) != 0) {
	char *fullname = NULL;

	if (pf->max_bounds.width != pf->min_bounds.width) {
	    (void)fprintf(stderr,
			  "proportional font, things will be miserable\n");
	}

	/*
	 * Free resources assoicated with any presently loaded fonts.
	 */
	if (tw->pfont)
	    XFreeFont(dpy, tw->pfont);
	if (tw->pfont_bold) {
	    XFreeFont(dpy, tw->pfont_bold);
	    tw->pfont_bold = NULL;
	}
	if (tw->pfont_ital) {
	    XFreeFont(dpy, tw->pfont_ital);
	    tw->pfont_ital = NULL;
	}
	if (tw->pfont_boldital) {
	    XFreeFont(dpy, tw->pfont_boldital);
	    tw->pfont_boldital = NULL;
	}
	tw->fsrch_flags = 0;

	tw->pfont = pf;
	tw->char_width  = pf->max_bounds.width;
	tw->char_height = pf->ascent + pf->descent;
	tw->char_ascent = pf->ascent;
	tw->left_ink	= (pf->min_bounds.lbearing < 0);
	tw->right_ink	= (pf->max_bounds.rbearing > tw->char_width);

	FreeIfNeeded(cur_win->fontname);
	if ((fullname = x_get_font_atom_property(pf, atom_FONT)) != NULL
	 && fullname[0] == '-') {
	    /* 
	     * Good. Not much work to do; the name was available via the FONT
	     * property.
	     */
	    tw->fontname = strmalloc(fullname);
	    XFree(fullname);
	}
	else {
	    /*
	     * Woops, fully qualified name not available from the FONT property.
	     * Attempt to get the full name piece by piece.  Ugh!
	     */
	    char str[1024], *s;
	    if (fullname != NULL)
		XFree(fullname);

	    s = str;
	    *s++ = '-';

#define GET_ATOM_OR_STAR(atom)					\
    do {							\
	char *as;						\
	if ((as = x_get_font_atom_property(pf, (atom))) != NULL) { \
	    char *asp = as;					\
	    while ((*s++ = *asp++))				\
		;						\
	    *(s-1) = '-';					\
	    XFree(as);						\
	}							\
	else {							\
	    *s++ = '*';						\
	    *s++ = '-';						\
	}							\
    } while (0)
#define GET_ATOM_OR_GIVEUP(atom)				\
    do {							\
	char *as;						\
	if ((as = x_get_font_atom_property(pf, (atom))) != NULL) { \
	    char *asp = as;					\
	    while ((*s++ = *asp++))				\
		;						\
	    *(s-1) = '-';					\
	    XFree(as);						\
	}							\
	else							\
	    goto piecemeal_done;				\
    } while (0)
#define GET_LONG_OR_GIVEUP(atom)				\
    do {							\
	long val;						\
	if (XGetFontProperty(pf, (atom), &val)) {		\
	    sprintf(s,"%ld",val);				\
	    while (*s++ != '\0')				\
		;						\
	    *(s-1) = '-';					\
	}							\
	else							\
	    goto piecemeal_done;				\
    } while (0)

	    GET_ATOM_OR_STAR(atom_FOUNDRY);
	    GET_ATOM_OR_GIVEUP(XA_FAMILY_NAME);
	    GET_ATOM_OR_GIVEUP(atom_WEIGHT_NAME);
	    GET_ATOM_OR_GIVEUP(atom_SLANT);
	    GET_ATOM_OR_GIVEUP(atom_SETWIDTH_NAME);
	    *s++ = '*';				/* ADD_STYLE_NAME */
	    *s++ = '-';
	    GET_LONG_OR_GIVEUP(atom_PIXEL_SIZE);
	    GET_LONG_OR_GIVEUP(XA_POINT_SIZE);
	    GET_LONG_OR_GIVEUP(atom_RESOLUTION_X);
	    GET_LONG_OR_GIVEUP(atom_RESOLUTION_Y);
	    GET_ATOM_OR_GIVEUP(atom_SPACING);
	    GET_LONG_OR_GIVEUP(atom_AVERAGE_WIDTH);
	    GET_ATOM_OR_STAR(atom_CHARSET_REGISTRY);
	    GET_ATOM_OR_STAR(atom_CHARSET_ENCODING);
	    *(s-1) = '\0';

#undef GET_ATOM_OR_STAR
#undef GET_ATOM_OR_GIVEUP
#undef GET_LONG_OR_GIVEUP

	    fname = str;
piecemeal_done:
	    /* 
	     * We will either use the name which was built up piecemeal or
	     * the name which was originally passed to us to assign to
	     * the fontname field.  We prefer the fully qualified name
	     * so that we can later search for bold and italic fonts.
	     */
	    tw->fontname = strmalloc(fname);
	}
    }
    return pf;
}

static XFontStruct *
alternate_font(weight, slant)
    char *weight;
    char *slant;
{
    char *newname, *np, *op;
    int cnt;
    XFontStruct *fsp = NULL;
    if (cur_win->fontname == NULL 
     || cur_win->fontname[0] != '-'
     || (newname = castalloc(char, strlen(cur_win->fontname)+32)) == NULL)
	return NULL;

    /* copy initial two fields */
    for (cnt=3, np=newname, op=cur_win->fontname; *op && cnt > 0; ) {
	if (*op == '-')
	    cnt--;
	*np++ = *op++;
    }
    if (!*op)
	goto done;

    /* substitute new weight and slant as appropriate */
#define SUBST_FIELD(field) 				\
    do {						\
	if ((field) != NULL) {				\
	    char *fp = (field);				\
	    if (nocase_eq(*fp, *op))			\
		goto done;				\
	    while ((*np++ = *fp++))			\
		;					\
	    *(np-1) = '-';				\
	    while (*op && *op++ != '-')			\
		;					\
	}						\
	else {						\
	    while (*op && (*np++ = *op++) != '-')	\
		;					\
	}						\
	if (!*op)					\
	    goto done;					\
    } while (0)

    SUBST_FIELD(weight);
    SUBST_FIELD(slant);
#undef SUBST_FIELD

    /* copy rest of name */
    while ((*np++ = *op++))
	;
    if ((fsp = XLoadQueryFont(dpy, newname)) != NULL) {
	cur_win->left_ink = cur_win->left_ink || (fsp->min_bounds.lbearing < 0);
	cur_win->right_ink = cur_win->right_ink
		    || (fsp->max_bounds.rbearing > cur_win->char_width);
    }

done:
    free(newname);
    return fsp;

}

int
x_setfont(fname)
    char       *fname;
{
    XFontStruct *pfont;
    int         oldw,
                oldh;

    if (cur_win) {
	oldw = x_width(cur_win);
	oldh = x_height(cur_win);
	if ((pfont = query_font(cur_win, fname)) != 0) {

	    XSetFont(dpy, cur_win->textgc, pfont->fid);
	    XSetFont(dpy, cur_win->reversegc, pfont->fid);

	    /* if size changed, resize it, otherwise refresh */
	    if (oldw != x_width(cur_win) || oldh != x_height(cur_win)) {
		XtVaSetValues(cur_win->top_widget,
			XtNminHeight,	cur_win->base_height
					    + MINROWS*cur_win->char_height,
			XtNminWidth,	cur_win->base_width 
					    + MINCOLS*cur_win->char_width,
			XtNheightInc,	cur_win->char_height,
			XtNwidthInc,	cur_win->char_width,
			NULL);
		update_scrollbar_sizes();
		x_touch(cur_win, 0, 0, cur_win->cols, cur_win->rows);
		XResizeWindow(dpy, XtWindow(cur_win->top_widget),
			      x_width(cur_win) + cur_win->base_width,
			      x_height(cur_win) + cur_win->base_height);

	    } else {
		x_touch(cur_win, 0, 0, cur_win->cols, cur_win->rows);
		x_flush();
	    }

	    return 1;
	}
	return 0;
    }
    return 1;
}

static
/* ARGSUSED */
SIGT x_quit (ACTUAL_SIG_ARGS)
{
    x_close();
    ExitProgram(GOODEXIT);
    /* NOTREACHED */
    SIGRET;
}

static void
x_open()
{
    kqinit(cur_win);
    cur_win->sel_prop = XInternAtom(dpy, "VILE_SELECTION", False);

    (void)signal(SIGHUP, x_quit);
    (void)signal(SIGINT, catchintr);
    (void)signal(SIGTERM, x_quit);

    term.t_mcol = MAXCOLS;
    term.t_mrow = MAXROWS;
    /* main code assumes that it can access a cell at nrow x ncol */
    term.t_ncol = cur_win->cols;
    term.t_nrow = cur_win->rows - 1;
}

static void
x_close()
{
    XtDestroyWidget(cur_win->top_widget);
}

static void
x_kopen()
{
}

static void
x_kclose()
{
}

static void
x_touch(tw, sc, sr, ec, er)
    TextWindow tw;
    int	 sc;
    int	 sr;
    UINT ec;
    UINT er;
{
    register UINT r;
    register UINT c;

    if (er > tw->rows)
	er = tw->rows;
    if (ec > tw->cols)
	ec = tw->cols;

    for (r = sr; r < er; r++) {
	MARK_LINE_DIRTY(r);
	for (c = sc; c < ec; c++)
	    MARK_CELL_DIRTY(r,c);
    }
}


static void
wait_for_scroll(tw)
    TextWindow  tw;
{
    XEvent      ev;
    int         sc,
                sr;
    unsigned    ec,
                er;
    XGraphicsExposeEvent *gev;

    while (1) {			/* loop looking for a gfx expose or no expose */
	if (XCheckTypedEvent(dpy, NoExpose, &ev))
	    return;
	if (XCheckTypedEvent(dpy, GraphicsExpose, &ev)) {
	    gev = (XGraphicsExposeEvent *) & ev;
	    sc = gev->x / tw->char_width;
	    sr = gev->y / tw->char_height;
	    ec = CEIL(gev->x + gev->width,  tw->char_width);
	    er = CEIL(gev->y + gev->height, tw->char_height);
	    x_touch(tw, sc, sr, ec, er);
	    x_flush();
	    return;
	}
	XSync(dpy, 0);
    }
}


static void
x_scroll(from, to, count)
    int from;
    int to;
    int count;
{
    if (from == to)
	return;			/* shouldn't happen */
    
    XCopyArea(dpy, cur_win->win, cur_win->win, cur_win->textgc,
	      x_pos(cur_win, 0), y_pos(cur_win, from),
	      x_width(cur_win), (unsigned)(count * cur_win->char_height),
	      x_pos(cur_win, 0), y_pos(cur_win, to));
    XFlush(dpy);
    wait_for_scroll(cur_win);

}

/*
 * The X protocol request for clearing a rectangle (PolyFillRectangle) takes
 * 20 bytes.  It will therefore be more expensive to switch from drawing text
 * to filling a rectangle unless the area to be cleared is bigger than 20
 * spaces.  Actually it is worse than this if we are going to switch
 * immediately to drawing text again since we incur a certain overhead
 * (16 bytes) for each string to be displayed.  This is how the value of
 * CLEAR_THRESH was computed (36 = 20+16).
 *
 * Kev's opinion:  If XDrawImageString is to be called, it is hardly ever
 * worth it to call XFillRectangle.  The only time where it will be a big
 * win is when the entire area to update is all spaces (in which case
 * XDrawImageString will not be called).  The following code would be much
 * cleaner, simpler, and easier to maintain if we were to just call
 * XDrawImageString where there are non-spaces to be written and
 * XFillRectangle when the entire region is to be cleared.
 */
#define	CLEAR_THRESH	36

static void
flush_line(text, len, attr, sr, sc)
    UCHAR *text;
    int	   len;
    VIDEO_ATTR attr;
    int	   sr;
    int	   sc;
{
    GC	fore_gc = ((attr & VAREV) ? cur_win->reversegc : cur_win->textgc);
    GC	back_gc = ((attr & VAREV) ? cur_win->textgc : cur_win->reversegc);
    int	fore_yy = text_y_pos(cur_win, sr);
    int	back_yy = y_pos(cur_win, sr);
    char *p;
    int   cc, tlen, i, startcol;
    int   fontchanged = FALSE;

    if (attr & (VABOLD | VAITAL)) {
	XFontStruct *fsp = NULL;
	if ((attr & (VABOLD | VAITAL)) == (VABOLD | VAITAL)) {
	    if (!(cur_win->fsrch_flags & FSRCH_BOLDITAL)) {
		if ((fsp = alternate_font("bold","i")) != NULL
	         || (fsp = alternate_font("bold","o")) != NULL)
		    cur_win->pfont_boldital = fsp;
		cur_win->fsrch_flags |= FSRCH_BOLDITAL;
	    }
	    if (cur_win->pfont_boldital != NULL) {
		XSetFont(dpy, fore_gc, cur_win->pfont_boldital->fid);
		fontchanged = TRUE;
		attr &= ~(VABOLD | VAITAL);	/* don't use fallback */
	    }
	    else
		goto tryital;
	}
	else if (attr & VAITAL) {
tryital:
	    if (!(cur_win->fsrch_flags & FSRCH_ITAL)) {
		if ((fsp = alternate_font(NULL,"i")) != NULL
	         || (fsp = alternate_font(NULL,"o")) != NULL)
		    cur_win->pfont_ital = fsp;
		cur_win->fsrch_flags |= FSRCH_ITAL;
	    }
	    if (cur_win->pfont_ital != NULL) {
		XSetFont(dpy, fore_gc, cur_win->pfont_ital->fid);
		fontchanged = TRUE;
		attr &= ~VAITAL;		/* don't use fallback */
	    }
	    else if (attr & VABOLD)
		goto trybold;
	}
	else if (attr & VABOLD) {
trybold:
	    if (!(cur_win->fsrch_flags & FSRCH_BOLD)) {
		cur_win->pfont_bold = alternate_font("bold",NULL);
		cur_win->fsrch_flags |= FSRCH_BOLD;
	    }
	    if (cur_win->pfont_bold != NULL) {
		XSetFont(dpy, fore_gc, cur_win->pfont_bold->fid);
		fontchanged = TRUE;
		attr &= ~VABOLD;		/* don't use fallback */
	    }
	}
    }

    /* break line into TextStrings and FillRects */
    p = (char *)text;
    cc = 0;
    tlen = 0;
    startcol = sc;
    for (i = 0; i < len; i++) {
	if (text[i] == ' ') {
	    cc++;
	    tlen++;
	} else {
	    if (cc >= CLEAR_THRESH) {
		tlen -= cc;
		XDrawImageString(dpy, cur_win->win, fore_gc,
				 (int)x_pos(cur_win, sc), fore_yy,
				 p, tlen);
		if (attr & VABOLD)
		    XDrawString(dpy, cur_win->win, fore_gc,
				(int)x_pos(cur_win, sc)+1, fore_yy,
				 p, tlen);
		p += tlen + cc;
		sc += tlen;
		XFillRectangle(dpy, cur_win->win, back_gc,
			       x_pos(cur_win, sc), back_yy,
			       (unsigned)(cc * cur_win->char_width),
			       (unsigned)(cur_win->char_height));
		sc += cc;
		tlen = 1;	/* starting new run */
	    } else
		tlen++;
	    cc = 0;
	}
    }
    if (cc >= CLEAR_THRESH) {
	tlen -= cc;
	XDrawImageString(dpy, cur_win->win, fore_gc,
			 x_pos(cur_win, sc), fore_yy,
			 p, tlen);
	if (attr & VABOLD)
	    XDrawString(dpy, cur_win->win, fore_gc,
			(int)x_pos(cur_win, sc)+1, fore_yy,
			 p, tlen);
	sc += tlen;
	XFillRectangle(dpy, cur_win->win, back_gc,
		       x_pos(cur_win, sc), back_yy,
		       (unsigned)(cc * cur_win->char_width),
		       (unsigned)(cur_win->char_height));
    } else if (tlen > 0) {
	XDrawImageString(dpy, cur_win->win, fore_gc,
			 x_pos(cur_win, sc), fore_yy,
			 p, tlen);
	if (attr & VABOLD)
	    XDrawString(dpy, cur_win->win, fore_gc,
			(int)x_pos(cur_win, sc)+1, fore_yy,
			 p, tlen);
    }
    if (attr & (VAUL | VAITAL))
	XDrawLine(dpy, cur_win->win, fore_gc,
	          x_pos(cur_win, startcol), fore_yy + 2,
		  x_pos(cur_win, startcol + len) - 1, fore_yy + 2);

    if (fontchanged)
	XSetFont(dpy, fore_gc, cur_win->pfont->fid);
}


/* See above comment regarding CLEAR_THRESH */
#define NONDIRTY_THRESH 16

/* make sure the screen looks like we want it to */
static void
x_flush()
{
    int r, c, sc, ec, cleanlen;
    VIDEO_ATTR attr;

    /* 
     * Write out cursor _before_ rest of the screen in order to avoid
     * flickering / winking effect noticable on some display servers.  This
     * means that the old cursor position (if different from the current
     * one) will be cleared after the new cursor is displayed.
     */

    if (ttrow >=0 && ttrow <= term.t_nrow && ttcol >= 0 && ttcol < term.t_ncol
     && !cur_win->wipe_permitted) {
	CLEAR_CELL_DIRTY(ttrow, ttcol);
	display_cursor((XtPointer) 0, (XtIntervalId *) 0);
    }

    for (r = 0; r < cur_win->rows; r++) {
	if (!IS_DIRTY_LINE(r))
	    continue;
	if (r !=  ttrow)
	    CLEAR_LINE_DIRTY(r);

	/*
	 * The following code will cause monospaced fonts with ink outside
	 * the bounding box to be cleaned up.
	 */
	if (cur_win->left_ink || cur_win->right_ink)
	    for (c=0; c < term.t_ncol; ) {
		while (c < term.t_ncol && !IS_DIRTY(r,c))
		    c++;
		if (c >= term.t_ncol)
		    break;
		if (cur_win->left_ink && c > 0)
		    MARK_CELL_DIRTY(r,c-1);
		while (c < term.t_ncol && IS_DIRTY(r,c))
		    c++;
		if (cur_win->right_ink && c < term.t_ncol) {
		    MARK_CELL_DIRTY(r,c);
		    c++;
		}
	    }

	c = 0;
	while (c < term.t_ncol) {
	    /* Find the beginning of the next dirty sequence */
	    while (c < term.t_ncol && !IS_DIRTY(r,c))
		c++;
	    if (c >= term.t_ncol)
		break;
	    if (r == ttrow && c == ttcol && !cur_win->wipe_permitted) {
		c++;
		continue;
	    }
	    CLEAR_CELL_DIRTY(r,c);
	    sc = ec = c;
	    attr = VATTRIB(CELL_ATTR(r,c));
	    cleanlen = NONDIRTY_THRESH;
	    c++;
	    /* 
	     * Scan until we find the end of line, a cell with a different
	     * attribute, a sequence of NONDIRTY_THRESH non-dirty chars, or
	     * the cursor position.
	     */
	    while (c < term.t_ncol) {
		if (attr != VATTRIB(CELL_ATTR(r,c)))
		    break;
		else if (r == ttrow && c == ttcol && !cur_win->wipe_permitted) {
		    c++;
		    break;
		}
		else if (IS_DIRTY(r,c)) {
		    ec = c;
		    cleanlen = NONDIRTY_THRESH;
		    CLEAR_CELL_DIRTY(r,c);
		}
		else if (--cleanlen <= 0)
		    break;
		c++;
	    }
	    /* write out the portion from sc thru ec */
	    flush_line(&CELL_TEXT(r,sc), ec-sc+1,
	               (VIDEO_ATTR) VATTRIB(CELL_ATTR(r,sc)), r, sc);
	}
    }
    XFlush(dpy);
}


/* selection processing stuff */

/* multi-click code stolen from xterm */
/*
 * double click table for cut and paste in 8 bits
 *
 * This table is divided in four parts :
 *
 *	- control characters	[0,0x1f] U [0x80,0x9f]
 *	- separators		[0x20,0x3f] U [0xa0,0xb9]
 *	- binding characters	[0x40,0x7f] U [0xc0,0xff]
 *  	- exceptions
 */
static int  charClass[256] = {
/* NUL  SOH  STX  ETX  EOT  ENQ  ACK  BEL */
    32, 1, 1, 1, 1, 1, 1, 1,
/*  BS   HT   NL   VT   NP   CR   SO   SI */
    1, 32, 1, 1, 1, 1, 1, 1,
/* DLE  DC1  DC2  DC3  DC4  NAK  SYN  ETB */
    1, 1, 1, 1, 1, 1, 1, 1,
/* CAN   EM  SUB  ESC   FS   GS   RS   US */
    1, 1, 1, 1, 1, 1, 1, 1,
/*  SP    !    "    #    $    %    &    ' */
    32, 33, 34, 35, 36, 37, 38, 39,
/*   (    )    *    +    ,    -    .    / */
    40, 41, 42, 43, 44, 45, 46, 47,
/*   0    1    2    3    4    5    6    7 */
    48, 48, 48, 48, 48, 48, 48, 48,
/*   8    9    :    ;    <    =    >    ? */
    48, 48, 58, 59, 60, 61, 62, 63,
/*   @    A    B    C    D    E    F    G */
    64, 48, 48, 48, 48, 48, 48, 48,
/*   H    I    J    K    L    M    N    O */
    48, 48, 48, 48, 48, 48, 48, 48,
/*   P    Q    R    S    T    U    V    W */
    48, 48, 48, 48, 48, 48, 48, 48,
/*   X    Y    Z    [    \    ]    ^    _ */
    48, 48, 48, 91, 92, 93, 94, 48,
/*   `    a    b    c    d    e    f    g */
    96, 48, 48, 48, 48, 48, 48, 48,
/*   h    i    j    k    l    m    n    o */
    48, 48, 48, 48, 48, 48, 48, 48,
/*   p    q    r    s    t    u    v    w */
    48, 48, 48, 48, 48, 48, 48, 48,
/*   x    y    z    {    |    }    ~  DEL */
    48, 48, 48, 123, 124, 125, 126, 1,
/* x80  x81  x82  x83  IND  NEL  SSA  ESA */
    1, 1, 1, 1, 1, 1, 1, 1,
/* HTS  HTJ  VTS  PLD  PLU   RI  SS2  SS3 */
    1, 1, 1, 1, 1, 1, 1, 1,
/* DCS  PU1  PU2  STS  CCH   MW  SPA  EPA */
    1, 1, 1, 1, 1, 1, 1, 1,
/* x98  x99  x9A  CSI   ST  OSC   PM  APC */
    1, 1, 1, 1, 1, 1, 1, 1,
/*   -    i   c/    L   ox   Y-    |   So */
    160, 161, 162, 163, 164, 165, 166, 167,
/*  ..   c0   ip   <<    _        R0    - */
    168, 169, 170, 171, 172, 173, 174, 175,
/*   o   +-    2    3    '    u   q|    . */
    176, 177, 178, 179, 180, 181, 182, 183,
/*   ,    1    2   >>  1/4  1/2  3/4    ? */
    184, 185, 186, 187, 188, 189, 190, 191,
/*  A`   A'   A^   A~   A:   Ao   AE   C, */
    48, 48, 48, 48, 48, 48, 48, 48,
/*  E`   E'   E^   E:   I`   I'   I^   I: */
    48, 48, 48, 48, 48, 48, 48, 48,
/*  D-   N~   O`   O'   O^   O~   O:    X */
    48, 48, 48, 48, 48, 48, 48, 216,
/*  O/   U`   U'   U^   U:   Y'    P    B */
    48, 48, 48, 48, 48, 48, 48, 48,
/*  a`   a'   a^   a~   a:   ao   ae   c, */
    48, 48, 48, 48, 48, 48, 48, 48,
/*  e`   e'   e^   e:    i`  i'   i^   i: */
    48, 48, 48, 48, 48, 48, 48, 48,
/*   d   n~   o`   o'   o^   o~   o:   -: */
    48, 48, 48, 48, 48, 48, 48, 248,
/*  o/   u`   u'   u^   u:   y'    P   y: */
48, 48, 48, 48, 48, 48, 48, 48};

static int
set_character_class_range(low, high, value)
    register int low,
                high;		/* in range of [0..255] */
    register int value;		/* arbitrary */
{

    if (low < 0 || high > 255 || high < low)
	return (-1);

    for (; low <= high; low++)
	charClass[low] = value;

    return (0);
}


/*
 * set_character_class - takes a string of the form
 *
 *                 low[-high]:val[,low[-high]:val[...]]
 *
 * and sets the indicated ranges to the indicated values.
 */

static int
set_character_class(s)
    register char *s;
{
    register int i;		/* iterator, index into s */
    int         len;		/* length of s */
    int         acc;		/* accumulator */
    int         low,
                high;		/* bounds of range [0..127] */
    int         base;		/* 8, 10, 16 (octal, decimal, hex) */
    int         numbers;	/* count of numbers per range */
    int         digits;		/* count of digits in a number */
    static char *errfmt = "xvile:  %s in range string \"%s\" (position %d)\n";

    if (!s || !s[0])
	return -1;

    base = 10;			/* in case we ever add octal, hex */
    low = high = -1;		/* out of range */

    for (i = 0, len = strlen(s), acc = 0, numbers = digits = 0;
	    i < len; i++) {
	int        c = s[i];

	if (isspace(c)) {
	    continue;
	} else if (isdigit(c)) {
	    acc = acc * base + (c - '0');
	    digits++;
	    continue;
	} else if (c == '-') {
	    low = acc;
	    acc = 0;
	    if (digits == 0) {
		(void)fprintf(stderr, errfmt, "missing number", s, i);
		return (-1);
	    }
	    digits = 0;
	    numbers++;
	    continue;
	} else if (c == ':') {
	    if (numbers == 0)
		low = acc;
	    else if (numbers == 1)
		high = acc;
	    else {
		(void)fprintf(stderr, errfmt, "too many numbers",
			s, i);
		return (-1);
	    }
	    digits = 0;
	    numbers++;
	    acc = 0;
	    continue;
	} else if (c == ',') {
	    /*
	     * now, process it
	     */

	    if (high < 0) {
		high = low;
		numbers++;
	    }
	    if (numbers != 2) {
		(void)fprintf(stderr, errfmt, "bad value number",
			s, i);
	    } else if (set_character_class_range(low, high, acc) != 0) {
		(void)fprintf(stderr, errfmt, "bad range", s, i);
	    }
	    low = high = -1;
	    acc = 0;
	    digits = 0;
	    numbers = 0;
	    continue;
	} else {
	    (void)fprintf(stderr, errfmt, "bad character", s, i);
	    return (-1);
	}			/* end if else if ... else */

    }

    if (low < 0 && high < 0)
	return (0);

    /*
     * now, process it
     */

    if (high < 0)
	high = low;
    if (numbers < 1 || numbers > 2) {
	(void)fprintf(stderr, errfmt, "bad value number", s, i);
    } else if (set_character_class_range(low, high, acc) != 0) {
	(void)fprintf(stderr, errfmt, "bad range", s, i);
    }
    return (0);
}

static void
x_lose_selection(tw)
	TextWindow  tw;
{
	tw->have_selection = False;
	tw->was_on_msgline = False;
	sel_release();
	free_selection(tw);
	(void) update(FALSE);
	x_flush();			/* show the changes */
}

/*
 * Copy a single character into the paste-buffer, quoting it if necessary
 */
static int
add2paste(p, c)
TBUFF	**p;
int	c;
{
	if (c == '\n' || isblank(c))
		;
	else if (isspecial(c) || (c == '\r') || !isprint(c))
	 	(void)tb_append(p, quotec);
	return (tb_append(p, c) != 0);
}

/*
 * Copy the selection into the PasteBuf buffer.  If we are pasting into a
 * window, check to see if:
 *
 *	+ the window's buffer is modifiable (if not, don't waste time copying
 *	  text!)
 *	+ the buffer uses 'autoindent' mode (if so, do some heuristics
 *	  for placement of the pasted text -- we may put it on lines by
 *	  itself, above or below the current line)
 */
static int
copy_paste(p, value, length)
TBUFF	**p;
char	*value;
SIZE_T	length;
{
	WINDOW	*wp = row2window(ttrow);
	BUFFER	*bp = (wp != NULL) ? wp->w_bufp : NULL;
	int	status;

	if (bp != NULL && b_val(bp,MDVIEW))
		return FALSE;

	status = TRUE;

	if (bp != NULL && (b_val(bp,MDCMOD) || b_val(bp,MDAIND))) {

		/*
		 * If the cursor points before the first nonwhite on
		 * the line, convert the insert into an 'O' command. 
		 * If it points to the end of the line, convert it into
		 * an 'o' command.  Otherwise (if it is within the
		 * nonwhite portion of the line), assume the user knows
		 * what (s)he is doing.
		 */
		if (setwmark(ttrow, ttcol)) {	/* MK gets cursor */
			LINE	*lp	= l_ref(MK.l);
			int	first   = firstchar(lp);
			int	last    = lastchar(lp);
			CMDFUNC	*f = NULL;
			extern CMDFUNC f_opendown_no_aindent;
			extern CMDFUNC f_openup_no_aindent;

			/* If the line contains only a single nonwhite,
			 * we will insert before it.
			 */
			if (first >= MK.o)
				f = &f_openup_no_aindent;
			else if (last <= MK.o)
				f = &f_opendown_no_aindent;
			if (insertmode) {
				if ((*value != '\n') && MK.o == 0)
					(void)tb_append(p, '\n');
			} else if (f) {
			    	char *pstr;
				/* we're _replacing_ the default
					insertion command, so reinit */
				tb_init(p, abortc);
				pstr = fnc2pstr(f);
				tb_bappend(p, pstr + 1, (ALLOC_T) *pstr);
			}
		}
	}

	while (length-- > 0) {
		if (!add2paste(p, *value++)) {
			status = FALSE;
			break;
		}
	}

	return status;
}

/* ARGSUSED */
static void
x_get_selection(tw, selection, type, value, length, format)
	TextWindow  tw;
	Atom        selection;
	Atom        type;
	char       *value;
	SIZE_T      length;
	int         format;
{
	int	do_ins;

	if (format != 8 || type != XA_STRING)
		return;			/* can't handle incoming data */

	if (length != 0) {
		char *s = NULL;		/* stifle warning */
		extern CMDFUNC f_insert_no_aindent;
		/* should be impossible to hit this with existing paste */
		/* XXX massive hack -- leave out 'i' if in prompt line */
		do_ins = !insertmode
			&& !onMsgRow(tw)
			&& ((s = fnc2pstr(&f_insert_no_aindent)) != NULL);

		if (tb_init(&PasteBuf, abortc)) {
			if ((do_ins && !tb_bappend(&PasteBuf, s+1, (ALLOC_T)*s))
			 || !copy_paste(&PasteBuf, value, length)
			 || (do_ins && !tb_append(&PasteBuf, abortc)))
				tb_free(&PasteBuf);
		}
	}
#if !(DOALLOC || DBMALLOC) /* cannot intercept that one */
	free(value);
#endif
}

static void
x_paste_selection(tw)
	TextWindow  tw;
{
	if (tw->have_selection) {
	    /* local transfer */
	    if (tw->selection_len == 0)	/* stash it if it hasn't been */
		x_stash_selection(tw);
	    x_get_selection(tw, XA_PRIMARY, XA_STRING,
			    strndup((char *) tw->selection_data, tw->selection_len),
			    (SIZE_T)tw->selection_len, 8);
	}
	else {
	    XConvertSelection(dpy, XA_PRIMARY, XA_STRING,
		              tw->sel_prop, tw->win, CurrentTime);
	}
}

static void
x_stash_selection(tw)
	TextWindow tw;
{
	UCHAR	*data;
	UCHAR	*dp;
	SIZE_T	length;
	KILL	*kp;		/* pointer into kill register */

	if (!tw->have_selection)
		return;
	free_selection(tw);

	sel_yank();
	for (length = 0, kp = kbs[SEL_KREG].kbufh; kp; kp = kp->d_next)
	    length += KbSize(SEL_KREG, kp);
	if (length == 0
	 || (dp = data = castalloc(UCHAR, length)) == 0
	 || (kp = kbs[SEL_KREG].kbufh) == 0)
		return;

	while (kp != NULL) {
		SIZE_T len = KbSize(SEL_KREG,kp);
		(void)memcpy((char *)dp, (char *)kp->d_chunk, len);
		kp = kp->d_next;
		dp += len;
	}
	/* FIXME: Can't select message line */
	tw->selection_len  = length;
	tw->selection_data = data;

#ifdef SABER_HACK
	XChangeProperty(dpy, RootWindow(dpy, tw->screen), XA_CUT_BUFFER0,
		XA_STRING, 8, PropModeReplace,
		tw->selection_data, tw->selection_len);
#endif
}

/* ARGSUSED */
static Bool
x_give_selection(tw, ev, target, prop)
	TextWindow  tw;
	XSelectionRequestEvent *ev;
	Atom        target;
	Atom        prop;
{
	if (!tw->have_selection || target != XA_STRING)
	    return False;
	x_stash_selection(tw);
	XChangeProperty(dpy, ev->requestor, prop, XA_STRING, 8,
		PropModeReplace, tw->selection_data, tw->selection_len);
	return True;
}

void
own_selection()
{
    free_selection(cur_win);
    if (!cur_win->have_selection)
	XSetSelectionOwner(dpy, XA_PRIMARY, cur_win->win, CurrentTime);
    cur_win->have_selection = True;
}

static void
scroll_selection(rowcol, idp)
    XtPointer rowcol;
    XtIntervalId *idp;
{
    int row, col;
    if (*idp == cur_win->sel_scroll_id)
	XtRemoveTimeOut(cur_win->sel_scroll_id);	/* shouldn't happen */
    cur_win->sel_scroll_id = (XtIntervalId) 0;

    /* Assumption: sizeof(XtPointer) == sizeof(long) */
    row = ((long) rowcol) >> 16;
    col = (((long) rowcol) << 16) >> 16;
    extend_selection(cur_win, row, col, TRUE);
}

static int
line_count_and_interval(scroll_count, ip)
    long scroll_count;
    unsigned long *ip;
{
    scroll_count = scroll_count / 4 - 2;
    if (scroll_count <= 0) {
	*ip = (1 - scroll_count) * cur_win->scroll_repeat_interval;
	return 1;
    }
    else {
	/* 
	 * FIXME: figure out a cleaner way to do this or something like it...
	 */
	if (scroll_count > 450)
	    scroll_count *= 1024;
	else if (scroll_count > 350)
	    scroll_count *= 128;
	else if (scroll_count > 275)
	    scroll_count *= 64;
	else if (scroll_count > 200)
	    scroll_count *= 16;
	else if (scroll_count > 150)
	    scroll_count *= 8;
	else if (scroll_count > 100)
	    scroll_count *= 4;
	else if (scroll_count > 75)
	    scroll_count *= 3;
	else if (scroll_count > 50)
	    scroll_count *= 2;
	*ip = cur_win->scroll_repeat_interval;
	return scroll_count;
    }
}

static void
extend_selection(tw, nr, nc, wipe)
    TextWindow tw;
    int	nr;
    int	nc;
    Bool wipe;
{
    static long scroll_count = 0;
    if (cur_win->sel_scroll_id != (XtIntervalId) 0) {
	if (nr < curwp->w_toprow || nr >= mode_row(curwp))
	    return;		/* Only let timer extend selection */
	XtRemoveTimeOut(cur_win->sel_scroll_id);
	cur_win->sel_scroll_id = (XtIntervalId) 0;
    }

    if (nr < curwp->w_toprow) {
	if (wipe) {
	    unsigned long interval;
	    mvupwind(TRUE, line_count_and_interval(scroll_count++, &interval));
	    cur_win->sel_scroll_id = XtAppAddTimeOut(
		    cur_win->app_context, interval, scroll_selection,
		    (XtPointer) ((long) ((nr << 16) | (nc & 0xffff))));
	}
	else {
	    scroll_count = 0;
	}
	nr = curwp->w_toprow;
    }
    else if (nr >= mode_row(curwp)) {
	if (wipe) {
	    unsigned long interval;
	    mvdnwind(TRUE, line_count_and_interval(scroll_count++, &interval));
	    cur_win->sel_scroll_id = XtAppAddTimeOut(
		    cur_win->app_context, interval, scroll_selection,
		    (XtPointer) ((long) ((nr << 16) | (nc & 0xffff))));
	}
	else {
	    scroll_count = 0;
	}
	nr = mode_row(curwp) - 1;
    }
    else {
	scroll_count = 0;
    }
    if (setcursor(nr,nc) && sel_extend(wipe)) {
	update(FALSE);
	x_flush();
    }
    else {
	x_beep();
    }
}

static void
multi_click(tw, nr, nc)
	TextWindow  tw;
	int         nr;
	int         nc;
{
    UCHAR	*p;
    int	cclass;
    int	sc = nc;
    int oc = nc;
    WINDOW	*wp;

    tw->numclicks++;

    if ((wp = row2window(nr)) != 0 && nr == mode_row(wp)) {
	set_curwp(wp);
	sel_release();
	(void)update(FALSE);
	x_flush();
    }
    else {
	switch (tw->numclicks) {
	case 0:
	case 1:			/* shouldn't happen */
		mlwrite("BUG: 0 or 1 multiclick value.");
		return;
	case 2:			/* word */
		/* find word start */
		p = &CELL_TEXT(nr,sc);
		cclass = charClass[*p];
		do {
			--sc;
			--p;
		} while (sc >= 0 && charClass[*p] == cclass);
		sc++;
		/* and end */
		p = &CELL_TEXT(nr,nc);
		cclass = charClass[*p];
		do {
			++nc;
			++p;
		} while (nc < tw->cols && charClass[*p] == cclass);
		--nc;
		break;
	case 3:			/* line (doesn't include trailing newline) */
		sc = 0;
		/* nc = tw->cols; */
		nc = HUGE;
		break;
	case 4:			/* screen */
		/* XXX blow off till we can figure out where screen starts
		 * and ends */
	default:
		/*
		 * This provides a mechanism for getting rid of the
		 * selection.
		 */
		sel_release();
		(void)update(FALSE);
		x_flush();
		return;
	}
	if (setcursor(nr,sc)) {
	    (void)sel_begin();
	    extend_selection(tw, nr, nc, FALSE);
	    (void) setcursor(nr,oc);
	    /* FIXME: Too many updates */
	    (void) update(FALSE);
	    x_flush();
	}
    }
}

static void
start_selection(tw, ev, nr, nc)
    TextWindow  tw;
    XButtonPressedEvent *ev;
    int	nr;
    int	nc;
{
    tw->wipe_permitted = FALSE;
    if ((tw->lasttime != 0)
     && (absol(ev->time - tw->lasttime) < tw->click_timeout)) {
	/* FIXME: This code used to ignore multiple clicks which
	 *	  spanned rows.  Do we still want this behavior?
	 *	  If so, we'll have to (re)implement it.
	 */
	multi_click(tw, nr, nc);
    }
    else {
	WINDOW *wp;

	beginDisplay;

	tw->lasttime = ev->time;
	tw->numclicks = 1;

	tw->was_on_msgline = onMsgRow(tw);

	/*
	 * If we're on the message line, do nothing.
	 *
	 * If we're on a mode line, make the window whose mode line we're
	 * on the current window.
	 *
	 * Otherwise update the cursor position in whatever window we're
	 * in and set things up so that the current position can be the
	 * possible start of a selection.
	 */
	if (reading_msg_line) {
	    ;	/* ignore */
	}
	else if ((wp = row2window(nr)) != 0 && nr == mode_row(wp)) {
	    set_curwp(wp);
	    (void)update(FALSE);
	    x_flush();
	}
	else if (setcursor(nr, nc)) {
	    if (!cur_win->persistent_selections)
		sel_release();
	    (void) sel_begin();
	    (void)update(FALSE);
	    x_flush();
	    tw->wipe_permitted = TRUE;
	}
	endofDisplay;
    }
}

static XMotionEvent *
compress_motion(ev)
    XMotionEvent *ev;
{
    XEvent      nev;

    while (XPending(ev->display)) {
	XPeekEvent(ev->display, &nev);
	if (nev.type == MotionNotify &&
		nev.xmotion.window == ev->window &&
		nev.xmotion.subwindow == ev->subwindow) {
	    XNextEvent(ev->display, (XEvent *) ev);
	} else
	    break;
    }
    return ev;
}

/*
 * handle non keyboard events associated with vile screen
 */
/*ARGSUSED*/
static void
x_process_event(w, unused, ev, continue_to_dispatch)
    Widget	w;
    XtPointer	unused;
    XEvent     *ev;
    Boolean    *continue_to_dispatch;
{
    int         sc,
                sr;
    unsigned    ec,
                er;

    int         nr,
                nc;
    XSelectionEvent event;
    XMotionEvent *mev;
    XExposeEvent *gev;
    Bool	do_sel;
    WINDOW	*wp;

    switch (ev->type) {
    case SelectionClear:
	x_lose_selection(cur_win);
	break;
    case SelectionRequest:
	event.type = SelectionNotify;
	event.display = ev->xselectionrequest.display;
	event.requestor = ev->xselectionrequest.requestor;
	event.selection = ev->xselectionrequest.selection;
	event.time = ev->xselectionrequest.time;
	event.target = ev->xselectionrequest.target;
	event.target = ev->xselectionrequest.target;
	if (ev->xselectionrequest.property == None)	/* obsolete requestor */
	    ev->xselectionrequest.property = ev->xselectionrequest.target;
	if (x_give_selection(cur_win, (XSelectionRequestEvent *) ev,
			     ev->xselectionrequest.target,
			     ev->xselectionrequest.property)) {
	    event.property = ev->xselectionrequest.property;
	} else {
	    event.property = None;
	}
	(void) XSendEvent(dpy, event.requestor, False, (long) 0,
			  (XEvent *) & event);
	break;
    case SelectionNotify:
	if (ev->xselection.property == None) {
	    x_get_selection(cur_win, ev->xselection.selection,
			    None, (char *) 0, 0, 8);
	} else {
	    ULONG	bytesafter;
	    ULONG	length;
	    int         format;
	    Atom        type;
	    UCHAR	*value;

	    (void) XGetWindowProperty(dpy, cur_win->win,
				      ev->xselection.property,
			  0L, 100000L, False, AnyPropertyType, &type, &format,
				      &length, &bytesafter, &value);
	    XDeleteProperty(dpy, cur_win->win, ev->xselection.property);
	    x_get_selection(cur_win, ev->xselection.selection,
			    type, (char *) value, (SIZE_T)length, format);
	}
	break;

    case Expose:
	gev = (XExposeEvent *)ev;
	sc = gev->x / cur_win->char_width;
	sr = gev->y / cur_win->char_height;
	ec = CEIL(gev->x + gev->width,  cur_win->char_width);
	er = CEIL(gev->y + gev->height, cur_win->char_height);
	x_touch(cur_win, sc, sr, ec, er);
	if (ev->xexpose.count == 0)
		x_flush();
	break;
    case MotionNotify:
	do_sel = cur_win->wipe_permitted;
	if (!(ev->xmotion.state & (Button1Mask | Button3Mask))) {
	    if (!cur_win->focus_follows_mouse)
		return;
	    else
		do_sel = FALSE;
	}
	mev = compress_motion((XMotionEvent *) ev);
	nc = mev->x / cur_win->char_width;
	nr = mev->y / cur_win->char_height;
	
	if (nr < 0)
	    nr = -1;	/* want to be out of bounds to force scrolling */
	else if (nr > cur_win->rows)
	    nr = cur_win->rows;

	if (nc < 0)
	    nc = 0;
	else if (nc >= cur_win->cols)
	    nc = cur_win->cols-1;

	/* ignore any spurious motion during a multi-cick */
	if (cur_win->numclicks > 1)
	    return;
	if (do_sel) {
	    if (ev->xbutton.state & ControlMask) {
		(void)sel_setshape(RECTANGLE);
	    }
	    extend_selection(cur_win, nr, nc, True);
	}
	else {
	    if (!reading_msg_line && (wp = row2window(nr)) && wp != curwp) {
		(void) set_curwp(wp);
		(void) update(FALSE);
	    }
	}
	break;
    case ButtonPress:
	nc = ev->xbutton.x / cur_win->char_width;
	nr = ev->xbutton.y / cur_win->char_height;
	switch (ev->xbutton.button) {
	case Button1:		/* move button and set selection point */
	    start_selection(cur_win, (XButtonPressedEvent *) ev, nr, nc);
	    break;
	case Button2:		/* paste selection */
	    if (ev->xbutton.state) {	/* if modifier, paste at mouse */
		if (!setcursor(nr, nc)) {
		    kbd_alarm();	/* don't know how to paste here */
		    break;
		}
	    }
	    x_paste_selection(cur_win);
	    break;
	case Button3:		/* end/extend selection */
	    if ((wp = row2window(nr)) && sel_buffer() == wp->w_bufp)
		(void) set_curwp(wp);
	    if (ev->xbutton.state & ControlMask)
		(void)sel_setshape(RECTANGLE);
	    cur_win->wipe_permitted = True;
	    extend_selection(cur_win, nr, nc, False);
	    break;
	}
	break;
    case ButtonRelease:
	switch (ev->xbutton.button) {
	case Button1:
	case Button3:
	    if (cur_win->sel_scroll_id != ((XtIntervalId) 0)) {
		XtRemoveTimeOut(cur_win->sel_scroll_id);
		cur_win->sel_scroll_id = (XtIntervalId) 0;
	    }
	    cur_win->wipe_permitted = False;
	    display_cursor((XtPointer) 0, (XtIntervalId *) 0);
	    break;
	}
	break;
    }
}

/*ARGSUSED*/
static void
x_configure_window(w, unused, ev, continue_to_dispatch)
    Widget	w;
    XtPointer	unused;
    XEvent     *ev;
    Boolean    *continue_to_dispatch;
{
    int nr, nc;
    Dimension new_width, new_height;
    Boolean changed = False;

    if (ev->type != ConfigureNotify)
	return;

    if (cur_win->base_width < 0) {
	/* First time through...figure out the base width and height */
	XtVaGetValues(cur_win->top_widget,
		XtNheight,	&cur_win->top_height,
		XtNwidth,	&cur_win->top_width,
		NULL);

	XtVaGetValues(cur_win->screen,
		XtNheight,	&new_height,
		XtNwidth,	&new_width,
		NULL);
	cur_win->base_width = cur_win->top_width - new_width;
	cur_win->base_height = cur_win->base_height;
	XtVaSetValues(cur_win->top_widget,
#if XtSpecificationRelease >= 4
		XtNbaseHeight,	cur_win->base_height,
		XtNbaseWidth,	cur_win->base_width,
#endif
		XtNminHeight,	cur_win->base_height
				    + MINROWS*cur_win->char_height,
		XtNminWidth,	cur_win->base_width 
				    + MINCOLS*cur_win->char_width,
		XtNheightInc,	cur_win->char_height,
		XtNwidthInc,	cur_win->char_width,
		NULL);
    }

    if (ev->xconfigure.height == cur_win->top_height
     && ev->xconfigure.width == cur_win->top_width)
 	return;

    XtVaGetValues(cur_win->top_widget,
	    XtNheight,	&new_height,
	    XtNwidth,	&new_width,
	    NULL);
    new_height = ((new_height - cur_win->base_height) / cur_win->char_height)
			     * cur_win->char_height;
    new_width = ((new_width - cur_win->base_width) / 
		    cur_win->char_width) * cur_win->char_width;
#if MOTIF_WIDGETS
    XtVaSetValues(cur_win->form_widget,
	    XmNresizePolicy,	XmRESIZE_NONE,
	    NULL);
    {
	WidgetList children;
	Cardinal nchildren;
	XtVaGetValues(cur_win->form_widget,
		XmNchildren, &children,
		XmNnumChildren, &nchildren,
		NULL);
	XtUnmanageChildren(children, nchildren);
    }
#else
#if NO_WIDGETS
    XtVaSetValues(cur_win->form_widget,
	    XtNwidth,		new_width + cur_win->pane_width + 2,
	    XtNheight,		new_height,
	    NULL);
#endif /* NO_WIDGETS */
#endif /* MOTIF_WIDGETS */
    XtVaSetValues(cur_win->screen,
	    XtNheight,	new_height,
	    XtNwidth,	new_width,
#if NO_WIDGETS
	    XtNx,	cur_win->scrollbar_on_left ? cur_win->pane_width+2 : 0,
#endif
	    NULL);
    XtVaSetValues(cur_win->pane,
#if !NO_WIDGETS
	    XtNwidth,	cur_win->pane_width,
#if OL_WIDGETS
	    XtNheight,	new_height,
#endif /* OL_WIDGETS */
#else	/* NO_WIDGETS */
	    XtNx,	cur_win->scrollbar_on_left ? 0 : new_width,
	    XtNwidth,	cur_win->pane_width+2,
	    XtNheight,	new_height - cur_win->char_height,
#endif /* NO_WIDGETS */
	    NULL);
#if MOTIF_WIDGETS
    {
	WidgetList children;
	Cardinal nchildren;
	XtVaGetValues(cur_win->form_widget,
		XmNchildren, &children,
		XmNnumChildren, &nchildren,
		NULL);
	XtManageChildren(children, nchildren);
    }
    XtVaSetValues(cur_win->form_widget,
	    XmNresizePolicy,	XmRESIZE_ANY,
	    NULL);
#endif /* MOTIF_WIDGETS */

    XtVaGetValues(cur_win->top_widget,
	    XtNheight,	&cur_win->top_height,
	    XtNwidth,	&cur_win->top_width,
	    NULL);
    XtVaGetValues(cur_win->screen,
	    XtNheight,	&new_height,
	    XtNwidth,	&new_width,
	    NULL);

    nr = new_height / cur_win->char_height;
    nc = new_width / cur_win->char_width;

    if (nr < MINROWS || nc < MINCOLS) {
	if (nr < MINROWS)
	    nr = MINROWS;
	if (nc < MINCOLS)
	    nc = MINCOLS;
	XResizeWindow(ev->xconfigure.display,
		      ev->xconfigure.window,
		      (unsigned)nc * cur_win->char_width 
			    + cur_win->base_width,
		      (unsigned)nr * cur_win->char_height
			    + cur_win->base_height);
	/* Calling XResizeWindow will cause another ConfigureNotify
	 * event, so we should return early and let this event occur.
	 */
	return;
    }

    if (nc != cur_win->cols) {
	changed = True;
	newwidth(True, nc);
    }
    if (nr != cur_win->rows) {
	changed = True;
	newlength(True, nr);
    }
    if (changed) {
	cur_win->rows = nr;
	cur_win->cols = nc;
	(void) refresh(True, 0);
	update_scrollbar_sizes();
	(void) update(False);
    }
#if MOTIF_WIDGETS
    lookfor_sb_resize = FALSE;
#endif
}

#if MOTIF_WIDGETS
static void
grip_moved(w, unused, ev, continue_to_dispatch)
    Widget	w;
    XtPointer	unused;
    XEvent     *ev;
    Boolean    *continue_to_dispatch;
{
    int i;
    register WINDOW *wp, *saved_curwp;
    Dimension height;
    int	lines;

#if MOTIF_WIDGETS
    if (!lookfor_sb_resize)
	return;
    lookfor_sb_resize = FALSE;
#endif

    saved_curwp = curwp;

    i = 0;
    for_each_window(wp) {
	XtVaGetValues(cur_win->scrollbars[i],
		XtNheight, &height,
		NULL);
	lines = (height+(cur_win->char_height/2)) / cur_win->char_height;
	if (lines == 0) {
	    /* Delete the window */
	    delwp(wp);
	    if (saved_curwp == wp)
		saved_curwp = wheadp;
	}
	else {
	    curwp = wp;
	    resize(TRUE, lines);
	}
	i++;
    }
    set_curwp(saved_curwp);
    (void) update(FALSE);
    x_flush();
}
#endif

static void
configure_bar(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    WINDOW *wp;
    int i;

    if (*num_params != 1
     || (event->type != ButtonPress && event->type != ButtonRelease))
	return;

    i = 0;
    for_each_window(wp) {
	if (cur_win->scrollbars[i] == w) {
	    if (strcmp(params[0], "Only") == 0) {
		set_curwp(wp);
		onlywind(TRUE,0);
	    }
	    else if (strcmp(params[0], "Kill") == 0) {
		set_curwp(wp);
		delwind(TRUE,0);
	    }
	    else if (strcmp(params[0], "Split") == 0) {
		if (wp->w_ntrows < 3) {
		    x_beep();
		    break;
		}
		else {
		    int newsize;
		    set_curwp(wp);
		    newsize = CEIL(event->xbutton.y, cur_win->char_height)-1;
		    if (newsize > wp->w_ntrows - 2)
			newsize = wp->w_ntrows - 2;
		    else if (newsize < 1)
			newsize = 1;
		    splitwind(TRUE, 1);
		    resize(TRUE, newsize);
		}
	    }
	    (void) update(FALSE);
	    x_flush();
	    break;
	}
	i++;
    }
}


#if MOTIF_WIDGETS
static void
pane_button(w, unused, ev, continue_to_dispatch)
    Widget	w;
    XtPointer	unused;
    XEvent     *ev;
    Boolean    *continue_to_dispatch;
{
    lookfor_sb_resize = TRUE;
}
#endif /* MOTIF_WIDGETS */

/*ARGSUSED*/
static void
x_change_focus(w, unused, ev, continue_to_dispatch)
    Widget	w;
    XtPointer	unused;
    XEvent     *ev;
    Boolean    *continue_to_dispatch;
{
    switch (ev->type) {
    case EnterNotify:
	if (cur_win->focus_follows_mouse) {
	    XSetInputFocus( dpy, XtWindow(w), RevertToParent, 
	                    ev->xcrossing.time );
	    /* hopefully this will generate a FocusIn event... */
	}
	return;
    case FocusIn:
	cur_win->show_cursor = True;
#if NO_WIDGETS
	XtSetKeyboardFocus(w, cur_win->screen);
#else
#if MOTIF_WIDGETS
	XmProcessTraversal(cur_win->screen, XmTRAVERSE_CURRENT);
#else
#if OL_WIDGETS
	XtSetKeyboardFocus(cur_win->top_widget, cur_win->screen);
#endif
#endif
#endif
	x_flush();
	break;
    case LeaveNotify:
	return;
    case FocusOut:
	cur_win->show_cursor = False;
	x_flush();
	break;
    }
}

/*ARGSUSED*/
static void
x_wm_delwin(w, unused, ev, continue_to_dispatch)
    Widget	w;
    XtPointer	unused;
    XEvent     *ev;
    Boolean    *continue_to_dispatch;
{
    if (ev->type == ClientMessage 
     && ev->xclient.message_type == atom_WM_PROTOCOLS) {
	if ((Atom) ev->xclient.data.l[0] == atom_WM_DELETE_WINDOW) {
	    quit(FALSE, 0);		/* quit might not return */
	    (void) update(FALSE);
	    x_flush();
	}
	else if ((Atom) ev->xclient.data.l[0] == atom_WM_TAKE_FOCUS) {
	    XSetInputFocus(XtDisplay(w), XtWindow(w), RevertToParent,
		    (Time) ev->xclient.data.l[1]);
	    XtSetKeyboardFocus(cur_win->top_widget, cur_win->screen);
	}
    }
}

/*
 * Return true if there are characters remaining to be pasted.  This is used in
 * the type-ahead check.
 */
int
x_is_pasting()
{
	return !kqempty(cur_win) || tb_more(PasteBuf);
}

/*
 * Return true if we want to disable reports of the cursor position because the
 * cursor really should be on the message-line.
 */
int
x_on_msgline()
{
	return reading_msg_line || cur_win->was_on_msgline;
}

/*
 * Because we poll our input-characters in 'x_getc()', it is possible to have
 * exposure-events pending while doing lengthy processes (e.g., reading from a
 * pipe).  This procedure is invoked from a timer-handler and is designed to
 * handle the exposure-events, and to get keypress-events (i.e., for stopping a
 * lengthy process).
 */
#if OPT_WORKING
void
x_working()
{
    XEvent ev;
    while (XtAppPending(cur_win->app_context)
        && !kqfull(cur_win)) {

	/* Get and dispatch next event */
	XtAppNextEvent(cur_win->app_context, &ev);
	XtDispatchEvent(&ev);
	
	/* 
	 * If the event was a keypress, check it to see if it was an
	 * interrupt character.  We check here to make sure that the
	 * queue was non-empty, because not all keypresses put
	 * characters into the queue.  We assume that intrc will not
	 * appear in any multi-character sequence generated by a key
	 * press, or that if it does, it will be the last character in
	 * the sequence.  If this is a bad assumption, we will have to
	 * keep track of what the state of the queue was prior to the
	 * keypress and scan the characters added to the queue as a
	 * result of the keypress.
	 */

	if (!kqempty(cur_win) && ev.type == KeyPress) {
	    int c = kqpop(cur_win);
	    if (c == intrc) {
		kqadd(cur_win, abortc);
#if VMS
		kbd_alarm(); /* signals? */
#else
		(void)signal_pg(SIGINT);
#endif
	    }
	    else
		kqadd(cur_win, c);
	}
    }
}
#endif /* OPT_WORKING */

static void
kqinit(tw)
    TextWindow tw;
{
    tw->kqhead = 0;
    tw->kqtail = 0;
}

static int
kqempty(tw)
    TextWindow tw;
{
    return tw->kqhead == tw->kqtail;
}

static int
kqfull(tw)
    TextWindow tw;
{
    return tw->kqhead == (tw->kqtail + 1) % KQSIZE;
}

static int
kqdel(tw)
    TextWindow tw;
{
    int c;
    c = tw->kq[tw->kqhead];
    tw->kqhead = (tw->kqhead + 1) % KQSIZE;
    return c;
}

static void
kqadd(tw, c)
    TextWindow tw;
    int c;
{
    tw->kq[tw->kqtail] = c;
    tw->kqtail = (tw->kqtail + 1) % KQSIZE;
}

static int
kqpop(tw)
    TextWindow tw;
{
    if (--(tw->kqtail) < 0)
	tw->kqtail = KQSIZE-1;
    return (tw->kq[tw->kqhead]);
}

static void
display_cursor(client_data, idp)
    XtPointer client_data;
    XtIntervalId *idp;
{
    static Bool am_blinking = FALSE;
    GC gc;

    /*
     * Return immediately if we are either in the process of making a
     * selection (by wiping with the mouse) or if the cursor is already
     * displayed and display_cursor() is being called explicitly from the
     * event loop in x_getc.
     */
    if (cur_win->wipe_permitted) {
	am_blinking = FALSE;
	if (cur_win->blink_id != (XtIntervalId) 0) {
	    XtRemoveTimeOut(cur_win->blink_id);
	    cur_win->blink_id = (XtIntervalId) 0;
	}
	return;
    }

    if (IS_DIRTY(ttrow,ttcol) && idp == (XtIntervalId *) 0)
	return;	

    /*
     * Guess about which graphics context to use for display of the cursor.
     */
    gc = IS_REVERSED(ttrow,ttcol) ? cur_win->textgc : cur_win->reversegc;

    if (cur_win->show_cursor) {
	if ( cur_win->blink_interval > 0 
	  || ( cur_win->blink_interval < 0 && gc == cur_win->textgc ) ) {
	    /* Reset the graphics context if its time to toggle the cursor */
	    if (cur_win->blink_status & BLINK_TOGGLE)
		gc = (gc == cur_win->textgc) ? cur_win->reversegc 
		                             : cur_win->textgc;
	    if (idp != (XtIntervalId *) 0 || !am_blinking) {
		/* Set timer to get blinking */
		cur_win->blink_id = XtAppAddTimeOut(
			cur_win->app_context,
			max(cur_win->blink_interval, -cur_win->blink_interval),
			display_cursor,
			(XtPointer) 0);
		cur_win->blink_status ^= BLINK_TOGGLE;
		am_blinking = TRUE;
	    }
	    else
		cur_win->blink_status &= ~BLINK_TOGGLE;
	}
	else {
	    am_blinking = FALSE;
	    if (cur_win->blink_id != (XtIntervalId) 0) {
		XtRemoveTimeOut(cur_win->blink_id);
		cur_win->blink_id = (XtIntervalId) 0;
	    }
	}

	MARK_CELL_DIRTY(ttrow,ttcol);
	MARK_LINE_DIRTY(ttrow);
	XDrawImageString(dpy, cur_win->win, gc,
		x_pos(cur_win, ttcol), text_y_pos(cur_win, ttrow),
		(char *)&CELL_TEXT(ttrow,ttcol), 1);
    }
    else {
	/* This code will get called when the window no longer has the focus. */
	if (cur_win->blink_id != (XtIntervalId) 0) {
	    XtRemoveTimeOut(cur_win->blink_id);
	    cur_win->blink_id = (XtIntervalId) 0;
	}
	am_blinking = FALSE;
	MARK_CELL_DIRTY(ttrow,ttcol);
	MARK_LINE_DIRTY(ttrow);
	gc = (gc == cur_win->textgc) ? cur_win->reversegc 
				     : cur_win->textgc;
	XDrawImageString(dpy, cur_win->win, gc,
		x_pos(cur_win, ttcol), text_y_pos(cur_win, ttrow),
		(char *)&CELL_TEXT(ttrow,ttcol), 1);
	XDrawRectangle(dpy, cur_win->win, gc,
	  x_pos(cur_win, ttcol), y_pos(cur_win, ttrow),
		       (unsigned)(cur_win->char_width - 1),
		       (unsigned)(cur_win->char_height - 1));
    }
}


/*
 * main event loop.  this means we'll be stuck if an event that needs
 * instant processing comes in while its off doing other work, but
 * there's no (easy) way around that.
 */
static int
x_getc()
{
    while (1) {

	if (tb_more(PasteBuf))	/* handle any queued pasted text */
	    return tb_next(PasteBuf);

	if (!kqempty(cur_win))
	    return kqdel(cur_win);

	/*
	 * Get and dispatch as many X events as possible.  This permits
	 * the editor to catch up if it gets behind in processing keyboard
	 * events since the keyboard queue will likely have something in it.
	 * update() will check for typeahead and will defer its processing
	 * until there is nothing more in the keyboard queue.
	 */
	 
	do {
	    XEvent ev;
	    XtAppNextEvent(cur_win->app_context, &ev);
	    XtDispatchEvent(&ev);
	} while (XtAppPending(cur_win->app_context) && !kqfull(cur_win));
    }
}

/*ARGSUSED*/
static void
x_key_press(w, unused, ev, continue_to_dispatch)
    Widget	w;
    XtPointer	unused;
    XEvent     *ev;
    Boolean    *continue_to_dispatch;
{
    char	buffer[128];
    KeySym	keysym;
    int		num;

    register int i, n;

    static struct {
	KeySym  key;
	int     code;
    } escapes[] = {
	{XK_F11,     ESC},
	/* Arrow keys */
	{XK_Up,      SPEC|'A'},
	{XK_Down,    SPEC|'B'},
	{XK_Right,   SPEC|'C'},
	{XK_Left,    SPEC|'D'},
	/* page scroll */
	{XK_Next,    SPEC|'n'},
	{XK_Prior,   SPEC|'p'},
	/* editing */
	{XK_Insert,  SPEC|'i'},
#if (ULTRIX || ultrix)
	{DXK_Remove, SPEC|'r'},
#endif
	{XK_Find,    SPEC|'f'},
	{XK_Select,  SPEC|'s'},
	/* command keys */
	{XK_Menu,    SPEC|'m'},
	{XK_Help,    SPEC|'h'},
	/* function keys */
	{XK_F1,      SPEC|'1'},
	{XK_F2,      SPEC|'2'},
	{XK_F3,      SPEC|'3'},
	{XK_F4,      SPEC|'4'},
	{XK_F5,      SPEC|'5'},
	{XK_F6,      SPEC|'6'},
	{XK_F7,      SPEC|'7'},
	{XK_F8,      SPEC|'8'},
	{XK_F9,      SPEC|'9'},
	{XK_F10,     SPEC|'0'},
	{XK_F12,     SPEC|'@'},
	{XK_F13,     SPEC|'#'},
	{XK_F14,     SPEC|'$'},
	{XK_F15,     SPEC|'%'},
	{XK_F16,     SPEC|'^'},
	{XK_F17,     SPEC|'&'},
	{XK_F18,     SPEC|'*'},
	{XK_F19,     SPEC|'('},
	{XK_F20,     SPEC|')'},
	/* keypad function keys */
	{XK_KP_F1,   SPEC|'P'},
	{XK_KP_F2,   SPEC|'Q'},
	{XK_KP_F3,   SPEC|'R'},
	{XK_KP_F4,   SPEC|'S'},
    };

    if (ev->type != KeyPress)
	return;

    num = XLookupString((XKeyPressedEvent *) ev, buffer, sizeof(buffer),
		&keysym, (XComposeStatus *) 0);

    if (num <= 0) {
	for (n = 0; n < SIZEOF(escapes); n++) {
	    if (keysym == escapes[n].key) {
		num = kcod2escape_seq(escapes[n].code, buffer);
		break;
	    }
	}
    }

    /* FIXME: Should do something about queue full conditions */
    if (num > 0) {
	for (i=0; i<num && !kqfull(cur_win); i++)
	    kqadd(cur_win, buffer[i]);
    }

}

/*
 * change reverse video status
 */
static void
x_rev(state)
    int         state;
{
    cur_win->reverse = state;
}

/* change screen resolution */
/*ARGSUSED*/
static int
x_cres(flag)
char *flag;
{
    return TRUE;
}

#if COLOR
static void
x_fcol(color)
int color;
{
}

static void
x_bcol(color)
int color;
{
}

#endif

/* change palette string */
/* ARGSUSED */
void
spal(dummy)
char *dummy;
{
}


/* beep */
static void
x_beep()
{
#if OPT_FLASH
    if (global_g_val(GMDFLASH)) {
	beginDisplay;
	XGrabServer(dpy);
	XSetFunction(dpy, cur_win->textgc, GXxor);
	XSetBackground(dpy, cur_win->textgc, 0);
	XSetForeground(dpy, cur_win->textgc, cur_win->fg ^ cur_win->bg);
	XFillRectangle(dpy, cur_win->win, cur_win->textgc,
		       0, 0, x_width(cur_win), x_height(cur_win));
	XFlush(dpy);
	catnap(90, FALSE);
	XFillRectangle(dpy, cur_win->win, cur_win->textgc,
		       0, 0, x_width(cur_win), x_height(cur_win));
	XFlush(dpy);
	XSetFunction(dpy, cur_win->textgc, GXcopy);
	XSetBackground(dpy, cur_win->textgc, cur_win->bg);
	XSetForeground(dpy, cur_win->textgc, cur_win->fg);
	XUngrabServer(dpy);
	endofDisplay;
    }
    else
#endif
	XBell(dpy, 0);
}

#if NO_LEAKS
void
x11_leaks()
{
	if (cur_win != 0) {
		free_selection(cur_win);
		FreeIfNeeded(cur_win->fontname);
	}
}
#endif	/* NO_LEAKS */

char x_window_name[NFILEN];
char x_icon_name[NFILEN];

void 
x_set_icon_name(name)
char *name; 
{
	XTextProperty Prop;

	strncpy0(x_icon_name, name, NFILEN);

	Prop.value = (unsigned char *)name;
	Prop.encoding = XA_STRING;
	Prop.format = 8;
	Prop.nitems = strlen(name);

	XSetWMIconName(dpy,XtWindow(cur_win->top_widget),&Prop);
}

char *
x_get_icon_name()
{
	return x_icon_name;
}

void 
x_set_window_name(name)
char *name; 
{
	XTextProperty Prop;

	strncpy0(x_window_name, name, NFILEN);

	Prop.value = (unsigned char *)name;
	Prop.encoding = XA_STRING;
	Prop.format = 8;
	Prop.nitems = strlen(name);

	XSetWMName(dpy,XtWindow(cur_win->top_widget),&Prop);
}

char * 
x_get_window_name()
{
    	return x_window_name;
}

#endif	/* X11 && XTOOLKIT */
