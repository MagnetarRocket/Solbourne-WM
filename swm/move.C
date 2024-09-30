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
 * $Id: move.C,v 9.12 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Window move routines
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/

#ifndef lint
static char RCSinfo[] =
"$Id: move.C,v 9.12 1993/08/27 16:58:15 toml Exp $";
#endif /* lint */

#include "swm.H"
#include "debug.H"
#include "wmdata.H"
#include "main.H"
#include "screen.H"
#include "resize.H"
#include "util.H"
#include "icons.H"
#include "init.H"
#include "move.H"
#include "execute.H"
#include "events.H"
#include "atoms.H"

static int dragx;
static int dragy;
static int origx;
static int origy;
static unsigned int dragWidth;
static unsigned int dragHeight;
static unsigned int dragBW;
static int dragBW2;
static int diffx;
static int diffy;
static Window outlineWindow;
static int xRatio, yRatio;

static void doMove(wmData *, OI_d_tech *, int, int, int *, int *);
static void reallyStartMove(wmData *, OI_d_tech *, int *, int *, int *, int, Window);
static void moveSweep(wmData *, OI_d_tech *, int, int);

XRectangle onerect;
XRectangle *rects;
int numRectangles;

int wmDoingMove = False;

/**********************************************************************
 *
 *  Procedure:
 *	wmMoveIcon
 *
 *  Function:
 *	Move an icon.  This code allows you to move an icon 
 *	within or to another icon panel or to/from the root.
 *
 **********************************************************************
 */

int
wmMoveIcon(
    XButtonEvent *ev,
    wmData *wp			// the window data struture
    )
{
    wmObjectData *odp;
    wmIconPanel *ip;
    Window child, last_child, new_child;
    int destx, desty;
    int x_root, y_root;
    int cancel;
    int size_changed;

    x_root = ev->x_root;
    y_root = ev->y_root;
    wmStartMove(wp, wp->oi_icon(), &x_root, &y_root, &cancel, False);
    if (cancel)
	return (False);

    // turn off icon gravity
    if (wp->icon_gravity())
	wmRemoveIcon(wp);

    // check to see if this dude was in the sweep list
    odp = (wmObjectData *)wp->oi_icon()->data();
    if (odp->sweep)
	moveSweep(wp, wp->oi_icon(), x_root, y_root);

    // now for the fun part, deciding where to place the icon

    // first check to see if the corner is over the root window, if
    // it is simply place it on the root window
    // we have to find the "youngest" child window

    child = 0;
    new_child = wmScr->root;
    while (True)
    {
	XTranslateCoordinates(DPY, wp->root()->X_window(), new_child,
	    x_root, y_root, &destx, &desty, &last_child);
	if (last_child == 0)
	    break;
	child = new_child = last_child;

	// check to see if this is an icon panel object
	if (!(XFindContext(DPY,child,wmIconPanelContext, (caddr_t*)&ip) ==
	    XCNOENT))
	    break;
    }

    // if the icon should go on the root
    if (child == 0 || odp->sweep || (XFindContext(DPY,child,wmIconPanelContext, (caddr_t*)&ip) == XCNOENT))
    {
	// free icon panel related stuff if it was in one
	if (wp->ip() && wp->ipsp())
	    wmRemoveIcon(wp);

	// move the icon to the root window
	wp->oi_icon()->set_associated_object(wp->root(), x_root, y_root, OI_ACTIVE);
	wp->move_vibox(x_root, y_root);

	wp->set_icon_window(wmScr->root);
	if (wp->has_client())
	    wmSetWM_STATE(wp, wp->state());
	wp->map_vibox();
    }
    else
    {
	// we can place it in the icon panel

	// take down the little window in the virtual root
	wp->unmap_vibox();

	// figure out the row and column
	int col = destx / ip->slot_width;
	int row = desty / ip->slot_height;

	// if the icon is already in an icon panel do some stuff
	if (wp->ip() && wp->ipsp())
	{
	    wp->ipsp()->used = False;		// free the entry
	    wp->ipsp()->wp = NULL;		// clear the pointer

	    // if it is a different icon panel just get rid of it
	    if (wp->ip() != ip)
		wmRemoveIcon(wp);
	    else
	    {
		// else, simply reduce the count and clear the area 
		wp->ip()->count--;
		XClearArea(DPY, wp->ip()->ob->X_window(),
		    wp->ipsp()->x, wp->ipsp()->y,
		    wp->ip()->slot_width, wp->ip()->slot_height, True);
	    }
	}

	// we need to check the validity of the row and column now
	// because a pack could have occurred which may have invalidated
	// the row and column that we just calculated
	if (row < ip->rows && col < ip->columns && !ip->row[row][col].used)
	{
	    // it's free!
	    ip->count++;
	    ip->row[row][col].used = True;
	    ip->row[row][col].wp = wp;
	    wp->set_ip(ip);
	    wp->set_ipsp(&ip->row[row][col]);
	    wp->oi_icon()->set_associated_object(ip->bp, -5000, -5000,
		OI_ACTIVE);
	    wp->set_icon_window(None);
	    if (wp->has_client())
		wmSetWM_STATE(wp, wp->state());
	    size_changed = wmNewIconPanelSizes(ip);
	    wmLayoutIconPanel(ip);
	    if (size_changed)
		wmSizeIconPanel(wp->ip());
	}
	else
	{
	    wmInsertIcon(wp, ip);
	}
    }

    return (True);
}

/**********************************************************************
 *
 *  Procedure:
 *	wmMoveClient
 *
 *  Function:
 *	Move a reparented client window
 *
 **********************************************************************
 */

int
wmMoveClient(
    XButtonEvent *ev,
    wmData *wp			// the window data struture
    )
{
    wmObjectData *odp;
    int x_root, y_root;
    int cancel;

    int old_x = (int)wp->oi_frame()->loc_x();
    int old_y = (int)wp->oi_frame()->loc_y();

    x_root = ev->x_root;
    y_root = ev->y_root;
    wmStartMove(wp, wp->oi_frame(), &x_root, &y_root, &cancel, False);

    if (cancel) {
	if (wmMoveOpaque)
		wp->move(old_x, old_y);
	return (False);
    }

    // check to see if this dude was in the sweep list
    odp = (wmObjectData *)wp->oi_frame()->data();
    if (odp->sweep)
	moveSweep(wp, wp->oi_frame(), x_root, y_root);

    wp->move(x_root, y_root);

    if (x_root != old_x || y_root != old_y)
	wmSendEvent(wp);
    
    return (True);
}

void
wmSendEvent(
    wmData *wp
    )
{
    XEvent client_event;
    int new_x, new_y;
    Window junkChild;

    // This routine used to send the ConfigureNotify event with respect
    // to the "root" window that the client application was on.  It now
    // sends the event with respect to the real root ALWAYS.

    // I need to get rid of this round trip to the server
    XTranslateCoordinates(DPY, wp->oi_client()->X_window(), wmScr->root, 0, 0, &new_x, &new_y, &junkChild);

    client_event.type = ConfigureNotify;
    client_event.xconfigure.display = DPY;
    client_event.xconfigure.event = wp->window();
    client_event.xconfigure.window = wp->window();
    client_event.xconfigure.x = new_x;
    client_event.xconfigure.y = new_y;
    client_event.xconfigure.width = wp->attr_width();
    client_event.xconfigure.height = wp->attr_height();
    client_event.xconfigure.border_width = wp->attr_bw();
    client_event.xconfigure.above = wp->oi_client()->X_window();
    client_event.xconfigure.override_redirect = False;
    XSendEvent(DPY, wp->window(), False, StructureNotifyMask, &client_event);
}

/**********************************************************************
 *
 *  Procedure:
 *	wmStartMove
 *
 *  Function:
 *	Setup and start a window move operation.  This procedure is called
 *	from wmMoveClient, wmMoveIcon, or wmReparent
 *
 **********************************************************************
 */

void
wmStartMove(
    wmData *wp,			// the client data
    OI_d_tech *oi,		// the object to move
    int *x_root,		// the starting and ending X coordinate
    int *y_root,		// the starting and ending Y coordinate
    int *cancel,		// should the move be cancelled?
    int adding,			// from wmReparent
    int panner,			// either IN_PANNER or OUT_PANNER
    unsigned objWidth,		// object width if IN_PANNER
    unsigned objHeight		// object height if IN_PANNER
    )
{
    wmObjectData *odp;
    Window junkRoot, parent, junkChild, *junkChildren = NULL;
    unsigned int junkDepth, numChildren;
    int junkx, junky;
    int junkxroot, junkyroot;
    unsigned int junkMask;
    int panX, panY;

#ifdef DEBUG
    if (wmDebug)
	fprintf(dfp, "wmStartMove:\n");
#endif

#ifdef DEBUG
    if (!OI_debug)
#endif /* DEBUG */
	wmGrabServer();

    wmConn->grab_pointer(wmScr->root, True,
        EnterWindowMask | LeaveWindowMask | ButtonPressMask | ButtonReleaseMask,
        GrabModeAsync, GrabModeAsync,
        wmScr->root, wmScr->moveCursor, CurrentTime);

    wmResetColormaps();

    // check to make sure the button is still down
    XQueryPointer(DPY, wmScr->root, &junkRoot, &junkChild, &junkxroot, &junkyroot, &junkx, &junky, &junkMask);

    if (!adding)
    {
	if ((junkMask & B_MASK) == 0)
	{
	    wmConn->ungrab_pointer(CurrentTime);
	    wmUngrabServer();
	    *cancel = True;
	    return;
	}
    }

    odp = (wmObjectData *)oi->data();
    if (odp->sweep) 
	numRectangles = wmSweptCount - 1;
    else
	numRectangles = 1;
    if (numRectangles > 1)
	rects = (XRectangle *)malloc(sizeof(XRectangle) * numRectangles);
    else
	rects = &onerect;

    XGetGeometry(DPY, oi->outside_X_window(), &junkRoot, &junkx, &junky, &dragWidth, &dragHeight, &dragBW, &junkDepth);
    origx = junkx;
    origy = junky;
    XQueryTree(DPY, oi->outside_X_window(), &junkRoot, &parent, &junkChildren, &numChildren);
    if (junkChildren)
	XFree((char *)junkChildren);
    XTranslateCoordinates(DPY, parent, junkRoot, junkx, junky, &dragx, &dragy, &junkChild);
    if (wmScr->pan)
	XTranslateCoordinates(DPY, wmScr->pan->X_window(), wmScr->root, 0,0, &panX, &panY, &junkChild);
    else
    {
	panX = 0;
	panY = 0;
    }

    dragBW2 = 2 * dragBW;
    dragx -= dragBW;
    dragy -= dragBW;
    dragWidth += dragBW2;
    dragHeight += dragBW2;
    diffx = dragx - *x_root;
    diffy = dragy - *y_root;

    // translate the real root coordinates to the virtual root coordinates
    XTranslateCoordinates(DPY, wmScr->root, wp->root()->X_window(), *x_root, *y_root, &dragx, &dragy, &junkChild);
    *x_root = dragx;
    *y_root = dragy;

    if (wp->root()->X_window() != wmScr->root)
	wmDoingMove = True;
    outlineWindow = wp->root()->X_window();

    xRatio = wp->x_ratio();
    yRatio = wp->y_ratio();
    if (panner == OUT_PANNER)
    {
	objWidth = dragWidth;
	objHeight = dragHeight;
    }

    while (True)
    {
	reallyStartMove(wp, oi, x_root, y_root, cancel, adding, outlineWindow);

	if (*cancel == IN_PANNER)
	{
	    panner = IN_PANNER;
	    dragWidth /= wmScr->vscale;
	    dragHeight /= wmScr->vscale;
	    diffx /= wmScr->vscale;
	    diffy /= wmScr->vscale;
	    outlineWindow = wmScr->pan->X_window();
	    xRatio = wmScr->vscale;
	    yRatio = wmScr->vscale;
	}
	else if (*cancel == OUT_PANNER)
	{
	    panner = OUT_PANNER;
	    dragWidth = objWidth;
	    dragHeight = objHeight;
	    diffx *= wmScr->vscale;
	    diffy *= wmScr->vscale;
	    outlineWindow = wmScr->vroot->X_window();
	    xRatio = 1;
	    yRatio = 1;
	}
	else
	    break;
    }

    if (panner == IN_PANNER)
    {
	*x_root *= wmScr->vscale;
	*y_root *= wmScr->vscale;
    }
    wmDoingMove = False;
    wmConn->ungrab_pointer(CurrentTime);
    wmUngrabServer();
    if (numRectangles > 1)
	free((char *)rects);
}

/**********************************************************************
 *
 *  Procedure:
 *	reallyStartMove
 *
 *  Function:
 *	Setup and start a window move operation.
 *
 **********************************************************************
 */

static void
reallyStartMove(
    wmData *wp,			// the client data
    OI_d_tech *oi,		// the object to move
    int *x_root,		// the starting and ending X coordinate
    int *y_root,		// the starting and ending Y coordinate
    int *cancel,		// should the move be cancelled?
    int adding,			// from wmReparent
    Window window
    )
{
#ifdef DEBUG
    if (wmDebug)
	fprintf(dfp, "wmReallyStartMove:\n");
#endif

    Window *junkChildren = NULL;
    int first, done;
    int xdest, ydest;		// the eventual destination


    xdest = *x_root;
    ydest = *y_root;
    first = True;

    while (True)
    {
	wmGetPointer(window, &first, x_root, y_root, cancel, &done, adding);
	if (done)
	    break;
	wmScr->sizeOI->set_state(OI_ACTIVE);
	doMove(wp, oi, *x_root, *y_root, &xdest, &ydest);
    }

    *x_root = xdest;
    *y_root = ydest;

}


/***********************************************************************
 *
 *  Procedure:
 *      doMove - move the rubberband around.  This is called
 *                   every time the pointer moves
 *
 *  Inputs:
 *	win	- the window to move if opaque
 *      x_root  - the X corrdinate in the root window
 *      y_root  - the Y corrdinate in the root window
 *
 ***********************************************************************
 */

static void
doMove(
    wmData *wp,
    OI_d_tech *oi,
    int x_root,
    int y_root,
    int *x_dest,
    int *y_dest
    )
{
    int i, rectcount;
    int deltax, deltay;
    char str[20];

    if (wmResizeFlags)
    {
	if (wmResizeFlags == RESIZE_LEFT ||
	    wmResizeFlags == RESIZE_RIGHT)
	    dragx = x_root;
	else if (wmResizeFlags == RESIZE_TOP ||
		 wmResizeFlags == RESIZE_BOTTOM)
	    dragy = y_root;
	else
	{
	    dragx = x_root;
	    dragy = y_root;
	}
    }
    else
    {
	dragx = x_root;
	dragy = y_root;
    }

    int xl = dragx + diffx;
    int yt = dragy + diffy;

    deltax = xl - origx/xRatio;
    deltay = yt - origy/yRatio;

    if (wp->constrain())
    {
	if (xl < 0)
	    xl = 0;
	else if ((xl + dragWidth) > wp->root()->size_x())
	    xl = wp->root()->size_x() - dragWidth;

	if (yt < 0)
	    yt = 0;
	else if ((yt + dragHeight) > wp->root()->size_y())
	    yt = wp->root()->size_y() - dragHeight;
    }

    if (wmMoveOpaque)
	oi->set_loc(xl, yt);
    else 
    {
	rectcount = 0;
	if (numRectangles == 1)
	{
	    rects[0].x = xl;
	    rects[0].y = yt;
	    rects[0].width = dragWidth;
	    rects[0].height = dragHeight;
	    rectcount = 1;
	}
	else
	{
	    for (i = 1; i < wmSweptCount; i++)
	    {
		if (wmSwept[i] && wmSwept[i]->state() == OI_ACTIVE)
		{
		    rects[rectcount].x = (short)wmSwept[i]->loc_x()/xRatio + deltax;
		    rects[rectcount].y = (short)wmSwept[i]->loc_y()/yRatio + deltay;
		    rects[rectcount].width = wmSwept[i]->space_x()/xRatio;
		    rects[rectcount].height = wmSwept[i]->space_y()/yRatio;
		    rectcount++;
		}
	    }
	}
	wmMoveOutline(outlineWindow, rectcount, rects);
    }
    sprintf(str, " %5d, %-5d  ", xl*xRatio, yt*yRatio);
    XRaiseWindow(DPY, wmScr->sizeOI->outside_X_window());
    wmScr->sizeOI->set_text(str);

    *x_dest = xl;
    *y_dest = yt;
}


/***********************************************************************
 *
 *  Procedure:
 *      moveSweep
 *
 *  Function:
 *	The object that we just moved was in the sweep.  Move the other
 *	objects accordingly
 *
 ***********************************************************************
 */

static void
moveSweep(wmData *, OI_d_tech *oi, int x_root, int y_root)
{
    int i;
    int diffx, diffy;
    int locx, locy;
    int newlocx, newlocy;
    wmObjectData *odp;

    diffx = x_root - (int)oi->loc_x();
    diffy = y_root - (int)oi->loc_y();

    for (i = 1; i < wmSweptCount; i++)
    {
	if (wmSwept[i] && wmSwept[i] != oi && wmSwept[i]->state() == OI_ACTIVE)
	{
	    odp = (wmObjectData *)wmSwept[i]->data();
	    locx = (int)wmSwept[i]->loc_x();
	    locy = (int)wmSwept[i]->loc_y();
	    newlocx = locx + diffx;
	    newlocy = locy + diffy;
	    if (locx != newlocx || locy != newlocy)
	    {
		wmSwept[i]->set_loc(newlocx, newlocy);
		if (odp->frame) {
		    odp->wp->move(newlocx, newlocy);
		    wmSendEvent(odp->wp);
		}
		else {
		    odp->wp->move_icon(newlocx, newlocy);
		    wmSet__SWM_HINTS(odp->wp);
		}
	    }
	}
    }
}
