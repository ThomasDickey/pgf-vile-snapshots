/*
 * msgs.c
 *
 * Support functions for "popup-msgs" mode.
 * Written by T.E.Dickey for vile (august 1994).
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/msgs.c,v 1.3 1994/10/06 14:13:24 pgf Exp $
 */
#include "estruct.h"

#if	OPT_POPUP_MSGS

#include "edef.h"

#define MSG_BUF_NAME ScratchName(Messages)

static	BUFFER *create_msgs P((void));

/*
 * Create the message-buffer if it doesn't already exist
 */
static BUFFER *
create_msgs()
{
	BUFFER	*bp = bfind(MSG_BUF_NAME, BFSCRTCH);

	if (bp == NULL)
		return FALSE;

	b_set_scratch(bp);
	return bp;
}

/*
 * This is invoked as a wrapper for 'kbd_putc()'.  It writes to the Messages
 * scratch buffer, and also to the message line.  If the Messages buffer isn't
 * visible, it is automatically popped up when a new message line is begun. 
 * Since it's a scratch buffer, popping it down destroys it.
 */
void
msg_putc(c)
int	c;
{
	BUFFER	*savebp = curbp;
	WINDOW	*savewp = curwp;
	MARK	savemk;
	int	saverow = ttrow;
	int	savecol = ttcol;
	register BUFFER *bp;
	register WINDOW *wp;

	if (savewp)
	    savemk  = DOT;
	bp = create_msgs();
	beginDisplay;
	/*
	 * Modify the current-buffer state as unobtrusively as possible (i.e.,
	 * don't modify the buffer order, and don't make the buffer visible if
	 * it isn't already!).  To use the 'bputc()' logic, though, we've got
	 * to have a window, even if it's not real.
	 */
	curbp = bp;
	if ((wp = bp2any_wp(bp)) == NULL) {
		static WINDOW dummy;
		wp = &dummy;
		wp->w_bufp = bp;
	}
	curwp = wp;
	DOT.l = lBACK(buf_head(bp));
	DOT.o = lLength(DOT.l);

	/*
	 * Write into the [Messages]-buffer
	 */
	if ((c != '\n') || (DOT.o > 0)) {
		bputc(c);
		b_clr_changed(bp);
	}

	/* Finally, restore the original current-buffer and write the character
	 * to the message line.
	 */
	curbp = savebp;
	curwp = savewp;
	if (savewp)
	    DOT   = savemk;
	movecursor(saverow, savecol);
	if (c != '\n') {
		if (sgarbf) {
			mlsavec(c);
		} else {
			kbd_putc(c);
		}
	}
	endofDisplay;
}

void
popup_msgs()
{
	BUFFER	*savebp = curbp;
	WINDOW	*savewp = curwp;
	MARK	savemk;
	register BUFFER *bp;
	WINDOW  *wp;

	if (savewp)
	    savemk = DOT;
	bp = create_msgs();
	if (!is_empty_buf(bp)) {
		if ((curwp == 0) || sgarbf) {
			return;		/* CAN'T popup yet */
		}
		if (popupbuff(bp) == FALSE) {
			(void)zotbuf(bp);
			return;
		}

		if ((wp = bp2any_wp(bp)) != NULL) {
			make_local_w_val(wp,WMDNUMBER);
			set_w_val(wp,WMDNUMBER,FALSE);
		}
		set_rdonly(bp, non_filename());
		curbp = savebp;
		curwp = savewp;
		if (savewp)
		    DOT   = savemk;
	}
}

/*
 * If no warning messages were encountered during startup, and the popup-msgs
 * mode wasn't enabled, discard the informational messages that are there
 * already.
 */
void
purge_msgs()
{
	if ((global_g_val(GMDPOPUP_MSGS) == -TRUE)
	 && (warnings == 0)) {
		BUFFER	*bp = find_b_name(MSG_BUF_NAME);
		if (bp != 0) {
			(void)zotbuf(bp);
		}
		set_global_g_val(GMDPOPUP_MSGS, FALSE);
	}
}
#endif
