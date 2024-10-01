/*****************************************************************************
 *
 * Copyright 1989-1993 ParcPlace Systems
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of ParcPlace not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  ParcPlace makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * ParcPlace DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL ParcPlace BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA  OR  PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * $Id: cursor.C,v 9.6 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Cursor utilities
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/

#ifndef lint
static char RCSinfo[] =
"$Id: cursor.C,v 9.6 1993/08/27 16:58:15 toml Exp $";
#endif /* lint */

#include "swm.H"
#include "main.H"
#include "screen.H"
#include "hglass.xbm"
#include "hglassm.xbm"
#include "util.H"
#include "cursor.H"

static struct _CursorName {
    char		*name;
    int			shape;
    XrmQuark		quark;
} cursor_names[] = {

{"X_cursor",		XC_X_cursor		,0},
{"arrow",		XC_arrow		,0},
{"based_arrow_down",	XC_based_arrow_down	,0},
{"based_arrow_up",	XC_based_arrow_up	,0},
{"boat",		XC_boat			,0},
{"bogosity",		XC_bogosity		,0},
{"bottom_left_corner",	XC_bottom_left_corner	,0},
{"bottom_right_corner",	XC_bottom_right_corner	,0},
{"bottom_side",		XC_bottom_side		,0},
{"bottom_tee",		XC_bottom_tee		,0},
{"box_spiral",		XC_box_spiral		,0},
{"center_ptr",		XC_center_ptr		,0},
{"circle",		XC_circle		,0},
{"clock",		XC_clock		,0},
{"coffee_mug",		XC_coffee_mug		,0},
{"cross",		XC_cross		,0},
{"cross_reverse",	XC_cross_reverse	,0},
{"crosshair",		XC_crosshair		,0},
{"diamond_cross",	XC_diamond_cross	,0},
{"dot",			XC_dot			,0},
{"dotbox",		XC_dotbox		,0},
{"double_arrow",	XC_double_arrow		,0},
{"draft_large",		XC_draft_large		,0},
{"draft_small",		XC_draft_small		,0},
{"draped_box",		XC_draped_box		,0},
{"exchange",		XC_exchange		,0},
{"fleur",		XC_fleur		,0},
{"gobbler",		XC_gobbler		,0},
{"gumby",		XC_gumby		,0},
{"hand1",		XC_hand1		,0},
{"hand2",		XC_hand2		,0},
{"heart",		XC_heart		,0},
{"icon",		XC_icon			,0},
{"iron_cross",		XC_iron_cross		,0},
{"left_ptr",		XC_left_ptr		,0},
{"left_side",		XC_left_side		,0},
{"left_tee",		XC_left_tee		,0},
{"leftbutton",		XC_leftbutton		,0},
{"ll_angle",		XC_ll_angle		,0},
{"lr_angle",		XC_lr_angle		,0},
{"man",			XC_man			,0},
{"middlebutton",	XC_middlebutton		,0},
{"mouse",		XC_mouse		,0},
{"pencil",		XC_pencil		,0},
{"pirate",		XC_pirate		,0},
{"plus",		XC_plus			,0},
{"question_arrow",	XC_question_arrow	,0},
{"right_ptr",		XC_right_ptr		,0},
{"right_side",		XC_right_side		,0},
{"right_tee",		XC_right_tee		,0},
{"rightbutton",		XC_rightbutton		,0},
{"rtl_logo",		XC_rtl_logo		,0},
{"sailboat",		XC_sailboat		,0},
{"sb_down_arrow",	XC_sb_down_arrow	,0},
{"sb_h_double_arrow",	XC_sb_h_double_arrow	,0},
{"sb_left_arrow",	XC_sb_left_arrow	,0},
{"sb_right_arrow",	XC_sb_right_arrow	,0},
{"sb_up_arrow",		XC_sb_up_arrow		,0},
{"sb_v_double_arrow",	XC_sb_v_double_arrow	,0},
{"shuttle",		XC_shuttle		,0},
{"sizing",		XC_sizing		,0},
{"spider",		XC_spider		,0},
{"spraycan",		XC_spraycan		,0},
{"star",		XC_star			,0},
{"target",		XC_target		,0},
{"tcross",		XC_tcross		,0},
{"top_left_arrow",	XC_top_left_arrow	,0},
{"top_left_corner",	XC_top_left_corner	,0},
{"top_right_corner",	XC_top_right_corner	,0},
{"top_side",		XC_top_side		,0},
{"top_tee",		XC_top_tee		,0},
{"trek",		XC_trek			,0},
{"ul_angle",		XC_ul_angle		,0},
{"umbrella",		XC_umbrella		,0},
{"ur_angle",		XC_ur_angle		,0},
{"watch",		XC_watch		,0},
{"xterm",		XC_xterm		,0},
};

/***********************************************************************
 *
 *  Procedure:
 *      wmInitCursors
 *
 *  Function:
 *	Initialize default cursors
 *
 ***********************************************************************
 */

void
wmInitCursors()
{
    int i;
    struct _CursorName *cp;
    char *ptr;
    Pixmap pm, mpm;
    XColor fore, back;

    for (i = 0, cp = cursor_names; i < sizeof(cursor_names)/sizeof(struct _CursorName); i++, cp++)
	cp->quark = XrmStringToQuark(cp->name);

    fore.pixel = wmScr->black;
    XQueryColor(DPY, wmScr->cmap, &fore);
    back.pixel = wmScr->white;
    XQueryColor(DPY, wmScr->cmap, &back);

    ptr = RM->get_resource("resizeCursor", "ResizeCursor");
    if (ptr) wmScr->resizeCursor = XCreateFontCursor(DPY, wmCursor(ptr));
    else wmScr->resizeCursor = XCreateFontCursor(DPY, XC_sizing);

    ptr = RM->get_resource("moveCursor", "MoveCursor");
    if (ptr) wmScr->moveCursor = XCreateFontCursor(DPY, wmCursor(ptr));
    else wmScr->moveCursor = XCreateFontCursor(DPY, XC_fleur);

    wmScr->chooseCursor = XCreateFontCursor(DPY, XC_question_arrow);
    wmScr->sweepCursor = XCreateFontCursor(DPY, XC_crosshair);
    pm = XCreatePixmapFromBitmapData(DPY, wmScr->root,
	hglass_bits, hglass_width, hglass_height, 1, 0, 1);
    mpm = XCreatePixmapFromBitmapData(DPY, wmScr->root,
	hglassm_bits, hglassm_width, hglassm_height, 1, 0, 1);
    wmScr->waitCursor = XCreatePixmapCursor(DPY, pm, mpm, &fore, &back,
	hglass_x_hot, hglass_y_hot);
    XFreePixmap(DPY, pm);
    XFreePixmap(DPY, mpm);
}

/***********************************************************************
 *
 *  Procedure:
 *      wmCursor
 *
 *  Function:
 *	Return a cursor ID for the name passed in.
 *
 ***********************************************************************
 */

int
wmCursor(
    char *str			// the cursor to look for
    )
{
    int i;
    struct _CursorName *cp;
    XrmQuark quark ;

    if (str == NULL)
	return (XC_top_left_arrow);

    quark = XrmStringToQuark(str);
    for (i = 0, cp = cursor_names; i < sizeof(cursor_names)/sizeof(struct _CursorName); i++, cp++)
    {
	if (quark == cp->quark)
	    return (cp->shape);
    }

    // return the default if we couldn't find it
    return (XC_top_left_arrow);
}
