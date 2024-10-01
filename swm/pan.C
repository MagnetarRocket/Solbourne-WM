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
 * $Id: pan.C,v 9.9 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Virtual Desktop panner routines
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/

#ifndef lint
static char RCSinfo[] =
"$Id: pan.C,v 9.9 1993/08/27 16:58:15 toml Exp $";
#endif /* lint */

#include "swm.H"
#include "wmdata.H"
#include "main.H"
#include "init.H"
#include "move.H"
#include "icons.H"
#include "atoms.H"

void
wmPaintPanner(OI_panner *, char *, XExposeEvent *ev)
{
    static int nested = 0;

    if (ev && ev->count)
	return;

#ifdef OLD
    if (nested++ != 0)
	return;

    XQueryTree(DPY, wmScr->vroot->X_window(), &junkRoot, &junkParent,
	&children, &nchildren);

    // need to temporarily disable expose events on the panner window
    // while I shuffle the little windows around
    XGetWindowAttributes(DPY, wmScr->pan->X_window(), &attr);
    XSelectInput(DPY, wmScr->pan->X_window(),
	attr.your_event_mask & ~ExposureMask);
    for (int i = 0; i < nchildren; i++)
    {
	XWindowAttributes wa;
	Window w;
	int x, y, width, height;
	wmData *wp = NULL;
	OI_box *box;

	w = children[i];
	XGetWindowAttributes(DPY, w, &wa);
	if (wa.map_state == IsUnmapped)
	    continue;

	wp = NULL;
	if (XFindContext(DPY, w, wmFrameContext,
	    (caddr_t*)&wp) != XCNOENT)
	{
	    box = wp->vbox();
	    x = (int)wp->oi_frame()->loc_x();
	    y = (int)wp->oi_frame()->loc_y();
	    width = wp->oi_frame()->space_x();
	    height = wp->oi_frame()->space_y();
	}
	else if (XFindContext(DPY, w, wmIconContext,(caddr_t*)&wp) != XCNOENT)
	{
	    x = (int)wp->oi_icon()->loc_x();
	    y = (int)wp->oi_icon()->loc_y();
	    width = wp->oi_icon()->space_x();
	    height = wp->oi_icon()->space_y();

	    if (wp->gray_icon())
		box = wp->vibox();
	    else
		box = wp->vbox();
	}
	if (!wp)
	    continue;

	if (!box)
	    continue;

	//XGetGeometry(DPY, w, &junkRoot, &x, &y, &width, &height, &junkBW,
	//    &junkDepth);

	x /= wmScr->vscale;
	y /= wmScr->vscale;
	width /= wmScr->vscale;
	height /= wmScr->vscale;
	// subtract an additional two pixels off for the border width
	width -= 2;
	height -= 2;
	if (width <= 0) width = 1;
	if (height <= 0) height = 1;

	//box->set_state(OI_ACTIVE_NOT_DISPLAYED);
	if (x != box->loc_x() || y != box->loc_y())
	    box->set_loc(x, y);
	if (width != box->size_x() || height != box->size_y())
	    box->set_size(width, height);
	XRaiseWindow(DPY, box->X_window());
	box->set_state(OI_ACTIVE);
    }
    // restore the event mask on the panner window
    XSelectInput(DPY, wmScr->pan->X_window(), attr.your_event_mask);
#endif /* OLD */
    XClearWindow(DPY, wmScr->pan->X_window());

    if (wmScr->pannerGrid)
    {
	int i;

	XSetForeground(DPY, wmScr->gc, wmScr->pannerGridFg);
	for (i = wmScr->width; i < wmScr->vwidth; i += wmScr->width)
	{
	    int x = (i / wmScr->vscale) - 1;
	    XDrawLine(DPY, wmScr->pan->X_window(), wmScr->gc, x, 0, x, 10000);
	}
	for (i = wmScr->height; i < wmScr->vheight; i += wmScr->height)
	{
	    int y = (i / wmScr->vscale) - 1;
	    XDrawLine(DPY, wmScr->pan->X_window(), wmScr->gc, 0, y, 10000, y);
	}
    }

    nested = 0;
}

void
wmExposeVroot(
    XExposeEvent *ev,
    char *
    )
{
    static int lastX = -1;
    static int lastY = -1;
    Window junkChild;
    int x, y;
    wmData *wp;

    if (ev->count)
	return;
    XTranslateCoordinates(DPY, wmScr->root, wmScr->vroot->X_window(), 0,0, &x, &y, &junkChild);
    if (x == lastX && y == lastY)
	return;

    lastX = x;
    lastY = y;
    wmScr->pan->change_paint((OI_pan_paint_fnp)NULL, NULL);
    wmScr->pan->set_pan_win_loc(x < 1 ? 1 : x, y < 1 ? 1 : y);
    wmScr->pan->change_paint((OI_pan_paint_fnp)wmPaintPanner, NULL);

    // this is ucky but I need to send a synthetic ConfigureNotify event
    // to every window that I've reparented.
    for (wp = (wmData *)wmScr->windowList.first(); wp != NULL; wp = (wmData *)wmScr->windowList.next())
	if (!wp->sticky())
	    wmSendEvent(wp);
}

void
wmReleasePanner(
    XButtonEvent *,
    char *
    )
{
    wmScr->vdt->set_handle_loc( wmScr->pan->pan_psn_x(), wmScr->pan->pan_psn_y());
}

void
wmMoveInPanner(
    XButtonEvent *ev,
    wmData *wp
    )
{
    static wmData *twp = NULL;
    int x_root, y_root, cancel;
    OI_d_tech *oi;
    int moveIcon = False;
    unsigned width, height;

    // this little jazzy piece of code gets around some OpenWindows 2.0 
    // button grab problems.  I used to just grab button 2 on the small
    // panner windows.  Because grabs don't work properly, I now get all
    // ButtonPress events and only look at those from Button2.  I send Button1
    // and Button3 to the panner window.
    if (ev->button != Button2) {
	ev->window = wmScr->pan->X_window();
	OI_handle_event((XEvent *)ev);
	return;
    }
    if (!twp)
	twp = new wmData();

    // patch up the window data structure to make the move happen
    twp->set_root(wmScr->pan);
    twp->set_x_ratio(wmScr->vscale);
    twp->set_y_ratio(wmScr->vscale);
    if (wp->constrain())
	twp->set_constrain();

    x_root = ev->x_root;
    y_root = ev->y_root;
    if (ev->window == wp->vbox()->X_window())
	oi = (OI_d_tech *)wp->vbox();
    else
    {
	oi = (OI_d_tech *)wp->vibox();
	moveIcon = True;
    }
    // figure out whether to move the icon or the frame
    if (wp->vibox() == NULL && wp->state() == IconicState)
	moveIcon = True;

    if (moveIcon)
    {
	width = wp->oi_icon()->space_x();
	height = wp->oi_icon()->space_y();
    }
    else
    {
	width = wp->oi_frame()->space_x();
	height = wp->oi_frame()->space_y();
    }

    wmStartMove(twp, oi, &x_root, &y_root, &cancel, False, IN_PANNER, width, height);

    // pull off all motion notify events for the panner
    XSync(DPY, False);
    XEvent dummy;
    while (XCheckTypedWindowEvent(DPY, wmScr->pan->X_window(), MotionNotify,
	&dummy));

    if (cancel)
	return;

    if (moveIcon)
    {
	wp->move_icon(x_root, y_root);
	wmSet__SWM_HINTS(wp);
    }
    else
    {
	wp->move(x_root, y_root);
	wmSendEvent(wp);
    }
}
