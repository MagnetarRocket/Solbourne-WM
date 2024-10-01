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
 * $Id: resize.C,v 9.13 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Window resize code
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/

#ifndef lint
static char RCSinfo[] =
"$Id: resize.C,v 9.13 1993/08/27 16:58:15 toml Exp $";
#endif /* lint */

#include "swm.H"
#include "debug.H"
#include "wmdata.H"
#include "object.H"
#include "main.H"
#include "screen.H"
#include "resize.H"
#include "util.H"
#include "panel.H"
#include "icons.H"
#include "move.H"
#include "gram.H"
#include "events.H"
#include "init.H"
#include "execute.H"
#include "region.H"

// the majority of the window resize code is from twm
/*****************************************************************************/
/**       Copyright 1988 by Evans & Sutherland Computer Corporation,        **/
/**                          Salt Lake City, Utah                           **/
/**                                                                         **/
/**                           All Rights Reserved                           **/
/**                                                                         **/
/**    Permission to use, copy, modify, and distribute this software and    **/
/**    its documentation  for  any  purpose  and  without  fee is hereby    **/
/**    granted, provided that the above copyright notice appear  in  all    **/
/**    copies and that both  that  copyright  notice  and  this  permis-    **/
/**    sion  notice appear in supporting  documentation,  and  that  the    **/
/**    name  of Evans & Sutherland  not be used in advertising or publi-    **/
/**    city pertaining to distribution  of the software without  specif-    **/
/**    ic, written prior permission.                                        **/
/**                                                                         **/
/**    EVANS  & SUTHERLAND  DISCLAIMS  ALL  WARRANTIES  WITH  REGARD  TO    **/
/**    THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILI-    **/
/**    TY AND FITNESS, IN NO EVENT SHALL EVANS &  SUTHERLAND  BE  LIABLE    **/
/**    FOR  ANY  SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY  DAM-    **/
/**    AGES  WHATSOEVER RESULTING FROM  LOSS OF USE,  DATA  OR  PROFITS,    **/
/**    WHETHER   IN  AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS    **/
/**    ACTION, ARISING OUT OF OR IN  CONNECTION  WITH  THE  USE  OR PER-    **/
/**    FORMANCE OF THIS SOFTWARE.                                           **/
/*****************************************************************************/

#define MINHEIGHT 0     /* had been 32 */
#define MINWIDTH 0      /* had been 60 */

static int dragx;       /* all these variables are used */
static int dragy;       /* in resize operations */
static int dragWidth;
static int dragHeight;
static unsigned int dragBW;
static unsigned int dragBW2;

static int origx;
static int origy;
static int origWidth;
static int origHeight;

static int heightPad;
static int widthPad;

static int clampTop;
static int clampBottom;
static int clampLeft;
static int clampRight;

static int last_width;
static int last_height;

static void constrainSize(wmData *);
static void doResize(wmData *, int, int);
static void displaySize(wmData *, int, int);
static void findWindows(Window, int, int);

int wmResizeFlags = 0;
int wmAutoResize = False;

XButtonEvent wmResizeEvent;
OI_d_tech **wmSwept;	// array of swept objects
int wmSweptCount = 0;

/***********************************************************************
 *
 *  Procedure:
 *      wmResizeTL
 *
 *  Function:
 *	Start the resize operation from the top-left resize corner
 *
 ***********************************************************************
 */

void
wmResizeTL( XButtonEvent *ev, wmData *wp)
{
    wmDrawCorner(wp, wmResizeCornerTopLeft, 0, 0, 1, 1, 1);
    wmResizeFlags = RESIZE_TOP | RESIZE_LEFT;
    wmHandleResizeBar(ev, wp);
    XClearArea(DPY, wp->oi_frame()->X_window(), 0,0,0,0, True);
}

/***********************************************************************
 *
 *  Procedure:
 *      wmResizeTR
 *
 *  Function:
 *	Start the resize operation from the top-right resize corner
 *
 ***********************************************************************
 */

void
wmResizeTR( XButtonEvent *ev, wmData *wp)
{
    wmDrawCorner(wp, wmResizeCornerTopRight, wp->oi_frame()->size_x()-1, 0, -1, 1, 1);
    wmResizeFlags = RESIZE_TOP | RESIZE_RIGHT;
    wmHandleResizeBar(ev, wp);
    XClearArea(DPY, wp->oi_frame()->X_window(), 0,0,0,0, True);
}

/***********************************************************************
 *
 *  Procedure:
 *      wmResizeBL
 *
 *  Function:
 *	Start the resize operation from the bottom-left resize corner
 *
 ***********************************************************************
 */

void
wmResizeBL( XButtonEvent *ev, wmData *wp)
{
    wmDrawCorner(wp, wmResizeCornerBottomLeft, 0, wp->oi_frame()->size_y()-1, 1, -1, 1);
    wmResizeFlags = RESIZE_BOTTOM | RESIZE_LEFT;
    wmHandleResizeBar(ev, wp);
    XClearArea(DPY, wp->oi_frame()->X_window(), 0,0,0,0, True);
}

/***********************************************************************
 *
 *  Procedure:
 *      wmResizeBR
 *
 *  Function:
 *	Start the resize operation from the bottom-right resize corner
 *
 ***********************************************************************
 */

void
wmResizeBR( XButtonEvent *ev, wmData *wp)
{
    wmDrawCorner(wp, wmResizeCornerBottomRight, wp->oi_frame()->size_x()-1,wp->oi_frame()->size_y()-1,-1,-1, 1);
    wmResizeFlags = RESIZE_BOTTOM | RESIZE_RIGHT;
    wmHandleResizeBar(ev, wp);
    XClearArea(DPY, wp->oi_frame()->X_window(), 0,0,0,0, True);
}

/***********************************************************************
 *
 *  Procedure:
 *      wmResizeT
 *
 *  Function:
 *	Start the resize operation from the top resize bar
 *
 ***********************************************************************
 */

void
wmResizeT( XButtonEvent *ev, wmData *wp)
{
    wmDrawHorizontalBar(wp, 0, 0, 1, 1);
    wmResizeFlags = RESIZE_TOP;
    wmHandleResizeBar(ev, wp);
    XClearArea(DPY, wp->oi_frame()->X_window(), 0,0,0,0, True);
}

/***********************************************************************
 *
 *  Procedure:
 *      wmResizeB
 *
 *  Function:
 *	Start the resize operation from the bottom resize bar
 *
 ***********************************************************************
 */

void
wmResizeB( XButtonEvent *ev, wmData *wp)
{
    wmDrawHorizontalBar(wp, 0, wp->oi_frame()->size_y()-1, -1, 1);
    wmResizeFlags = RESIZE_BOTTOM;
    wmHandleResizeBar(ev, wp);
    XClearArea(DPY, wp->oi_frame()->X_window(), 0,0,0,0, True);
}

/***********************************************************************
 *
 *  Procedure:
 *      wmResizeL
 *
 *  Function:
 *	Start the resize operation from the left resize bar
 *
 ***********************************************************************
 */

void
wmResizeL( XButtonEvent *ev, wmData *wp)
{
    wmDrawVerticalBar(wp, 0, 0, 1, 1);
    wmResizeFlags = RESIZE_LEFT;
    wmHandleResizeBar(ev, wp);
    XClearArea(DPY, wp->oi_frame()->X_window(), 0,0,0,0, True);
}

/***********************************************************************
 *
 *  Procedure:
 *      wmResizeR
 *
 *  Function:
 *	Start the resize operation from the right resize bar
 *
 ***********************************************************************
 */

void
wmResizeR( XButtonEvent *ev, wmData *wp)
{
    wmDrawVerticalBar(wp, wp->oi_frame()->size_x()-1, 0, -1, 1);
    wmResizeFlags = RESIZE_RIGHT;
    wmHandleResizeBar(ev, wp);
    XClearArea(DPY, wp->oi_frame()->X_window(), 0,0,0,0, True);
}

/***********************************************************************
 *
 *  Procedure:
 *      wmhandleResizeBar
 *
 *  Function:
 *	Start the resize operation from one of the resize bars or
 *	corners.
 *
 ***********************************************************************
 */

void
wmHandleResizeBar(
    XButtonEvent *ev,		// the event that caused the resize
    wmData *wp			// pointer to the window data
    )
{
    if (ev->button == Button2)
	wmMoveClient(ev, wp);
    else
	wmStartResize(ev, wp);
    wmResizeFlags = 0;
}

/***********************************************************************
 *
 *  Procedure:
 *      wmStartResize
 *
 *  Function:
 *	Start a resize operation
 *
 ***********************************************************************
 */

int
wmStartResize(
    XButtonEvent *ev,		// the event that caused the resize
    wmData *wp			// pointer to the window data
    )
{
#ifdef DEBUG
    if (wmDebug)
	fprintf(dfp, "wmStartResize:\n");
#endif

    Window      junkRoot, junkChild;
    int         junkX, junkY;
    unsigned int junkDepth, junkMask;
    unsigned int clientWidth, clientHeight, clientBW;
    int		x_root, y_root;
    int		first, cancel, done;

#ifdef DEBUG
    if (!OI_debug)
#endif /* DEBUG */
	wmGrabServer();
    if (wp) 
	wmConn->grab_pointer(wp->root()->X_window(), True, EnterWindowMask | LeaveWindowMask | ButtonPressMask | ButtonReleaseMask,
	    GrabModeAsync, GrabModeAsync, wp->root()->X_window(), wmScr->resizeCursor, CurrentTime);
    else
	wmConn->grab_pointer(wmScr->root, True, EnterWindowMask | LeaveWindowMask | ButtonPressMask | ButtonReleaseMask,
	    GrabModeAsync, GrabModeAsync, wmScr->root, wmScr->resizeCursor, CurrentTime);

    wmResetColormaps();

    // check to make sure the button is still down
    XQueryPointer(DPY, wmScr->root, &junkRoot, &junkChild, &x_root, &y_root, &junkX, &junkY, &junkMask);

    if ((junkMask & B_MASK) == 0)
    {
	wmConn->ungrab_pointer(CurrentTime);
	wmUngrabServer();
	return (False);
    }

    if (wp)
    {
	XGetGeometry(DPY, wp->oi_frame()->outside_X_window(), &junkRoot, &dragx, &dragy, (unsigned int *)&dragWidth, (unsigned int *)&dragHeight,
	    &dragBW,&junkDepth);
	XGetGeometry(DPY, wp->window(), &junkRoot, &junkX, &junkY, &clientWidth, &clientHeight, &clientBW, &junkDepth);
    }
    else
    {
	dragx = ev->x_root;
	dragy = ev->y_root;
	dragWidth = dragHeight = clientWidth = clientHeight = 1;
	dragBW = 0;
    }

    dragBW2 = 2 * dragBW;
    dragx += dragBW;
    dragy += dragBW;
    origx = dragx;
    origy = dragy;
    origWidth = dragWidth;
    origHeight = dragHeight;

    widthPad = dragWidth - clientWidth;
    heightPad = dragHeight - clientHeight;

    // if we auto start resizing, find the quadrant
    if (wmAutoResize)
    {
	int frame_x_third, frame_y_third;
	int x_frame, y_frame;

	wmResizeFlags = 0;
	XTranslateCoordinates(DPY, wmScr->root, wp->oi_frame()->outside_X_window(), ev->x_root, ev->y_root, &x_frame, &y_frame, &junkChild);
	frame_x_third = wp->oi_frame()->space_x() / 3;
	frame_y_third = wp->oi_frame()->space_y() / 3;

	if (x_frame <= frame_x_third)
	    wmResizeFlags |= RESIZE_LEFT;
	else if (x_frame >= (frame_x_third * 2))
	    wmResizeFlags |= RESIZE_RIGHT;

	if (y_frame <= frame_y_third)
	    wmResizeFlags |= RESIZE_TOP;
	else if (y_frame >= (frame_y_third * 2))
	    wmResizeFlags |= RESIZE_BOTTOM;

	if (!wmResizeFlags)
	{
	    if (x_frame < (wp->oi_frame()->space_x() / 2))
		wmResizeFlags |= RESIZE_LEFT;
	    else
		wmResizeFlags |= RESIZE_RIGHT;

	    if (y_frame < (wp->oi_frame()->space_y() / 2))
		wmResizeFlags |= RESIZE_TOP;
	    else
		wmResizeFlags |= RESIZE_BOTTOM;
	}
    }

    clampTop = wmResizeFlags & RESIZE_TOP;
    clampBottom = wmResizeFlags & RESIZE_BOTTOM;
    clampLeft = wmResizeFlags & RESIZE_LEFT;
    clampRight = wmResizeFlags & RESIZE_RIGHT;

    last_width = 0;
    last_height = 0;

    first = True;

    // translate the real root coordinates to the virtual root coordinates
    if (wp)
	XTranslateCoordinates(DPY, wmScr->root, wp->root()->X_window(), ev->x_root, ev->y_root, &x_root, &y_root, &junkChild);

    while (True)
    {
	wmGetPointer(wp ? wp->root()->X_window() : wmScr->root, &first, &x_root, &y_root, &cancel, &done, False);
	if (done)
	    break;
	if (wp) {
	    wmScr->sizeOI->set_state(OI_ACTIVE);
	    XRaiseWindow(DPY, wmScr->sizeOI->outside_X_window());
	}
	doResize(wp, x_root, y_root);
    }
    if (!cancel)
    {
	// change the cursor while we play games 
//	wmConn->grab_pointer(wmScr->root, True, ButtonPressMask | ButtonReleaseMask,
//		GrabModeAsync, GrabModeAsync, wmScr->root, wmScr->waitCursor, CurrentTime);

	// bring dragWidth and dragHeight into a valid range
	constrainSize(wp);

	// adjust them for decorations
	dragWidth -= widthPad;
	dragHeight -= heightPad;
	if (wp)
	{
	    wp->move(dragx - dragBW, dragy - dragBW);
	    wmResizeClient(wp, dragWidth, dragHeight, True);
	}
    }

    wmConn->ungrab_pointer(CurrentTime);
    wmUngrabServer();
    if (cancel)
	return (False);
    else
	return (True);
}

/***********************************************************************
 *
 *  Procedure:
 *      doResize
 *
 *  Function:
 *	Move the rubberband around.  This is called each time the
 *	pointer moves.
 *
 ***********************************************************************
 */

static void
doResize(
    wmData *wp,			// the window data
    int x_root,			// the root X coordinate
    int y_root			// the root Y coordinate
    )
{
    XRectangle rect;
    int action = 0;

    if (clampTop) {
        int         delta = y_root - dragy;
        if (dragHeight - delta < MINHEIGHT) {
            delta = dragHeight - MINHEIGHT;
            clampTop = 0;
        }
        dragy += delta;
        dragHeight -= delta;
        action = 1;
    }
    else if (y_root <= dragy/* ||
             y_root == findRootInfo(root)->rooty*/) {
        dragy = y_root;
        dragHeight = origy + origHeight -
            y_root;
        clampBottom = 0;
        clampTop = 1;
        action = 1;
    }
    if (clampLeft) {
        int         delta = x_root - dragx;
        if (dragWidth - delta < MINWIDTH) {
            delta = dragWidth - MINWIDTH;
            clampLeft = 0;
        }
        dragx += delta;
        dragWidth -= delta;
        action = 1;
    }
    else if (x_root <= dragx/* ||
             x_root == findRootInfo(root)->rootx*/) {
        dragx = x_root;
        dragWidth = origx + origWidth -
            x_root;
        clampRight = 0;
        clampLeft = 1;
        action = 1;
    }
    if (clampBottom) {
        int         delta = y_root - dragy - dragHeight;
        if (dragHeight + delta < MINHEIGHT) {
            delta = MINHEIGHT - dragHeight;
            clampBottom = 0;
        }
        dragHeight += delta;
        action = 1;
    }
    else if (y_root >= dragy + dragHeight - 1/* ||
           y_root == findRootInfo(root)->rooty
           + findRootInfo(root)->rootheight - 1*/) {
        dragy = origy;
        dragHeight = 1 + y_root - dragy;
        clampTop = 0;
        clampBottom = 1;
        action = 1;
    }
    if (clampRight) {
        int         delta = x_root - dragx - dragWidth;
        if (dragWidth + delta < MINWIDTH) {
            delta = MINWIDTH - dragWidth;
            clampRight = 0;
        }
        dragWidth += delta;
        action = 1;
    }
    else if (x_root >= dragx + dragWidth - 1/* ||
             x_root == findRootInfo(root)->rootx +
             findRootInfo(root)->rootwidth - 1*/) {
        dragx = origx;
        dragWidth = 1 + x_root - origx;
        clampLeft = 0;
        clampRight = 1;
        action = 1;
    }
    if (action) {
        constrainSize(wp);
        if (clampLeft)
            dragx = origx + origWidth - dragWidth;
        if (clampTop)
            dragy = origy + origHeight - dragHeight;
	rect.x = dragx - dragBW;
	rect.y = dragy - dragBW;
	rect.width = dragWidth + dragBW2;
	rect.height = dragHeight + dragBW2;
        wmMoveOutline( wp ? wp->root()->X_window() :  wmScr->root, 1, &rect);
    }

    displaySize(wp, dragWidth, dragHeight);
}

/***********************************************************************
 *
 *  Procedure:
 *      displaySize - display the size in the dimensions window
 *
 *  Function:
 *	Display the current window size in the dimensions window
 *
 ***********************************************************************
 */

static void
displaySize(
    wmData *wp,			// the window data
    int width,			// width of the rubberband
    int height			// height of the rubberband
    )
{
    char str[100];
    int dwidth;
    int dheight;

    if (wp == NULL)
	return;

    if (last_width == width && last_height == height)
        return;

    last_width = width;
    last_height = height;

    height -= heightPad;
    width -= widthPad;

    wmUnitSize(wp, width, height, &dwidth, &dheight);

    if (wp->panner())
    {
	dwidth *= wmScr->vscale;
	dheight *= wmScr->vscale;
    }
    sprintf(str, " %5d x %-5d ", dwidth, dheight);
    wmScr->sizeOI->set_text(str);
}

/***********************************************************************
 *
 *  Procedure:
 *      wmUnitSize
 *
 *  Function:
 *	Take the width and height parameters and figure out the unit
 *	width and height using reszing hints (if specified).
 *
 ***********************************************************************
 */

void
wmUnitSize(
    wmData *wp,			// the window data
    int width,			// raw width
    int height,			// raw height
    int *dwidth,		// returned unit width
    int *dheight		// returned unit height
    )
{
    *dheight = height;
    *dwidth = width;

    XSizeHints *sh = wp->size_hints_p();
    if (sh->flags&(PMinSize|PBaseSize) && sh->flags & PResizeInc)
    {
	if (sh->flags & PBaseSize)
	{
	    *dwidth -= sh->base_width;
	    *dheight -= sh->base_height;
	}
	else
	{
	    *dwidth -= sh->min_width;
	    *dheight -= sh->min_height;
	}
    }

    if (sh->flags & PResizeInc)
    {
        *dwidth /= sh->width_inc;
        *dheight /= sh->height_inc;
    }
}

/***********************************************************************
 *
 *  Procedure:
 *      constrainSize
 *
 *  Function:
 *	Adjust dragWidth and dragHeight to account for constraints
 *	imposed by size hints.  The general algorithm, especially the
 *	aspect ratio stuff, is borrowed from uwm's CheckConsistency routine.
 *
 ***********************************************************************
 */

static void
constrainSize(
    wmData *wp			// the window data
    )
{
#define MAXSIZE 32767
#define makemult(a,b) ((b==1) ? (a) : (((int)((a)/(b))) * (b)) )

    int j = 0;
    int minWidth, minHeight, maxWidth, maxHeight, xinc, yinc, delta;
    int baseWidth, baseHeight;

    if (wp == NULL)
	return;

    dragHeight -= heightPad;
    dragWidth -= widthPad;

    XSizeHints *sh = wp->size_hints_p();
    if (sh->flags & PMinSize) {
        minWidth = sh->min_width;
        minHeight = sh->min_height;
    } else if (sh->flags & PBaseSize) {
        minWidth = sh->base_width;
        minHeight = sh->base_height;
    } else
        minWidth = minHeight = 1;

    if (sh->flags & PBaseSize) {
        baseWidth = sh->base_width;
        baseHeight = sh->base_height;
    } else if (sh->flags & PMinSize) {
        baseWidth = sh->min_width;
        baseHeight = sh->min_height;
    } else
        baseWidth = baseHeight = 0;

    if (sh->flags & PMaxSize) {
        maxWidth = sh->max_width;
        maxHeight = sh->max_height;
    } else
        maxWidth = maxHeight = MAXSIZE;

    if (sh->flags & PResizeInc) {
        xinc = sh->width_inc;
        yinc = sh->height_inc;
    } else
        xinc = yinc = 1;

    // first clamp to max and min values

    if (dragWidth < minWidth) dragWidth = minWidth;
    if (dragHeight < minHeight) dragHeight = minHeight;

    if (dragWidth > maxWidth) dragWidth = maxWidth;
    if (dragHeight > maxHeight) dragHeight = maxHeight;

    // second, fit to base + N * inc

    dragWidth = (((dragWidth - baseWidth) / xinc) * xinc) + baseWidth;
    dragHeight = (((dragHeight - baseHeight) / yinc) * yinc) + baseHeight;

    // third, adjust for aspect ratio

#define maxAspectX sh->max_aspect.x
#define maxAspectY sh->max_aspect.y
#define minAspectX sh->min_aspect.x
#define minAspectY sh->min_aspect.y

    if (sh->flags & PAspect)
    {
        if (dragWidth * maxAspectX > dragHeight * maxAspectY)
        {
            delta = makemult(dragWidth * maxAspectY / maxAspectX - dragHeight,
                             yinc);
            if (dragHeight + delta <= maxHeight) dragHeight += delta;
            else
            {
                delta = makemult(dragWidth - maxAspectX*dragHeight/maxAspectY,
                                 xinc);
                if (dragWidth - delta >= minWidth) dragWidth -= delta;
            }
        }

        if (dragWidth * minAspectX < dragHeight * minAspectY)
        {
            delta = makemult(minAspectX * dragHeight / minAspectY - dragWidth,
                             xinc);
            if (dragWidth + delta <= maxWidth) dragWidth += delta;
            else
            {
                delta = makemult(dragHeight - dragWidth*minAspectY/minAspectX,
                                 yinc);
                if (dragHeight - delta >= minHeight) dragHeight -= delta;
            }
        }
    }

    dragHeight += heightPad;
    dragWidth += widthPad;
}


/***************************************************************
 *
 *  Procedure: 
 *	wmGetPointer
 *
 *  Function:
 *	Get the current pointer location.  This routine will not
 *	return until the pointer has moved.  In addition, when a 
 *	button release event is detected and no buttons are still
 *	pressed, the done flag will be set to true.  If another 
 *	button is pressed, the cancel flag will be set to true,
 *	the wait cursor will be displayed, the window outline will
 *	be removed, and the procedure will not return until all
 *	buttons have been released.
 *
 ***************************************************************/

void
wmGetPointer(
    Window window,		// the window to get the pointer location
    int *first,			// first time flag
    int *x_root,		// return the X pointer location
    int *y_root,		// return the Y pointer location
    int *cancel,		// has the operation been cancelled?
    int *done,			// are we done?
    int adding			// we are adding a window, return if button 2 is pressed
    )
{
    Window junkRoot, junkChild;
    int junkX, junkY;
    unsigned int junkMask;
    XEvent event;
    XButtonEvent *ev;
    int origX, origY;
    static int moved;
    static int last_x, last_y;
    static int last_mask;
    int wait_for_release = False;

    // if this is the first time, set up some stuff
    if (*first)
    {
	moved = False;
	origX = *x_root;
	origY = *y_root;
	*first = False;
	last_x = last_y = -1;
	last_mask = 0;

	// assume we are going to cancel the command until we have
	// moved the pointer moveDelta pixels
	*cancel = Button1;
    }

    *done = False;

    while (True)
    {
	while (XCheckMaskEvent(DPY, EnterWindowMask|LeaveWindowMask|ButtonPressMask|ButtonReleaseMask, &event))
	{
	    ev = (XButtonEvent *)&event;
	    if (!wait_for_release && wmDoingMove && !wmMoveOpaque && wmScr->pan)
	    {
		if (ev->type == EnterNotify && event.xcrossing.window == wmScr->pan->X_window() &&
			event.xcrossing.detail != NotifyInferior)
		{
		    wmMoveOutline();
		    *cancel = IN_PANNER;
		    *done = True;
		    return;
		}
#ifdef NOT_NEEDED
		if (ev->type == LeaveNotify && event.xcrossing.window == wmScr->pan->X_window()) {
			printf("LeaveNotify  detail = ");
			switch (event.xcrossing.detail)
			{
			    case NotifyAncestor: printf( "NotifyAncestor\n"); break;
			    case NotifyVirtual: printf( "NotifyVirtual\n"); break;
			    case NotifyInferior: printf( "NotifyInferior\n"); break;
			    case NotifyNonlinear: printf( "NotifyNonlinear\n"); break;
			    case NotifyNonlinearVirtual: printf( "NotifyNonlinearVirtual\n"); break;
			}
			printf("  mode = ");
			switch (event.xcrossing.mode)
			{
			    case NotifyNormal: printf("NotifyNormal\n"); break;
			    case NotifyGrab: printf("NotifyGrab\n"); break;
			    case NotifyUngrab: printf("NotifyUngrab\n"); break;
			}
		}
#endif /* NOT_NEEDED */

		if (ev->type == LeaveNotify && event.xcrossing.window == wmScr->pan->X_window() &&
			event.xcrossing.detail != NotifyInferior && event.xcrossing.mode == NotifyNormal)
		{
		    wmMoveOutline();
		    *cancel = OUT_PANNER;
		    *done = True;
		    return;
		}
	    }
	    // if we get a button press here, we need to abort the command
	    // so we will stay here until we get all the buttons released
	    if (ev->type == ButtonPress)
	    {
		wmMoveOutline();
		wmScr->sizeOI->set_state(OI_ACTIVE_NOT_DISPLAYED);
		XFlush(DPY);
		*cancel = ev->button;
		wait_for_release = True;
		wmResetColormaps();
		if (adding && ev->button == Button2)
		{
		    wmScr->sizeOI->set_state(OI_ACTIVE_NOT_DISPLAYED);
		    XFlush(DPY);
		    *done = True;
		    return;
		}
		wmConn->grab_pointer(window, True, EnterWindowMask | LeaveWindowMask | ButtonPressMask | ButtonReleaseMask,
		    GrabModeAsync, GrabModeAsync, window, wmScr->waitCursor, CurrentTime);
	    }

	    if (ev->type == ButtonRelease)
	    {
		// clear the mask bit for the button just released
		unsigned mask = B_MASK;
		switch (ev->button)
		{
		    case Button1: mask &= ~Button1Mask; break;
		    case Button2: mask &= ~Button2Mask; break;
		    case Button3: mask &= ~Button3Mask; break;
		    case Button4: mask &= ~Button4Mask; break;
		    case Button5: mask &= ~Button5Mask; break;
		}

		// if all buttons have been released
		if ((ev->state & mask) == 0)
		{
		    // button zero resets OI objects
		    if (wmResizeEvent.type == ButtonPress)
		    {
			wmResizeEvent.type = ButtonRelease;
			wmResizeEvent.time = ev->time;
			XPutBackEvent(DPY, (XEvent *)&wmResizeEvent);
		        wmResizeEvent.type = ~ButtonPress;
		    }
		    wmMoveOutline();
		    wmScr->sizeOI->set_state(OI_ACTIVE_NOT_DISPLAYED);
		    XFlush(DPY);
		    if (wait_for_release)
			wmConn->ungrab_pointer(CurrentTime);
		    *done = True;
		    return;
		}
	    }
	}
	if (wait_for_release)
	    continue;

	XQueryPointer(DPY, window, &junkRoot, &junkChild,
	    &junkX, &junkY, x_root, y_root, &junkMask);

	wmScr->showGrid = wmScr->showGridSave;
	if (!wmScr->showGrid)
		wmScr->showGrid = junkMask & (ShiftMask|LockMask|ControlMask|Mod1Mask);

	if (!moved &&
	    (abs(*x_root - origX) < wmScr->moveDelta &&
	    abs(*y_root - origY) < wmScr->moveDelta))
	    continue;

	// if the pointer has not moved, try again
	if (*x_root == last_x && *y_root == last_y && wmScr->showGrid == last_mask)
	    continue;

	// OK we have moved "moveDelta" pixels
	last_x = *x_root;
	last_y = *y_root;
	last_mask = wmScr->showGrid;

	moved = True;
	*cancel = False;
	return;
    }
}

/***************************************************************
 *
 *  Procedure: 
 *	wmResizeClient
 *
 *  Function:
 *	Resize the client window to the new width and height.
 *	This routine will also re-layout the frame surrounding the 
 *	window.
 *
 ***************************************************************/

void
wmResizeClient(
    wmData *wp,			// the wmData pointer
    int width,			// the new client width
    int height,			// the new client height
    int from_resize		// if this was called from resize
    )
{
    IconRegion	*irp;

    // save the new dimensions in the attribute structure
    wp->set_attr_width(width);
    wp->set_attr_height(height);

    wp->op()->u.p->pad = wp->pad();
    wp->op()->oi = wp->oi_frame();

    // resize the client's parent object
    wp->oi_client()->set_size(wp->attr_width() + 2 * wp->attr_bw(), wp->attr_height() + 2 * wp->attr_bw());
    wmLayoutPanel(wp->op());
    if (wp->vbox()) {
	int w = wp->oi_frame()->space_x() / wmScr->vscale;
	int h = wp->oi_frame()->space_y() / wmScr->vscale;
	// subtract an additional two pixels off for the border width
	w -= 2;
	h -= 2;
	if (w <= 0) w = 1;
	if (h <= 0) h = 1;
	wp->vbox()->set_size(w, h);
    }

    // resize the client
    if (!wp->mine())
	XResizeWindow(DPY, wp->window(), width, height);
    else
    {
	if (wp->myip() && from_resize)
	{
	    wmResizeIconPanel(wp);
	}
	else if (wp->panner())
	{
	    int bwidth = width * wmScr->vscale;
	    int bheight = height * wmScr->vscale;
	    if (bwidth < wmScr->width)
	    {
		bwidth = wmScr->width;
		width = bwidth / wmScr->vscale;
	    }
	    if (bheight < wmScr->height)
	    {
		bheight = wmScr->height;
		height = bheight / wmScr->vscale;
	    }

	    // expand the icon regions
	    if (bwidth > wmScr->vwidth || bheight > wmScr->vheight) {
	        for (irp = (IconRegion *)wmScr->iconRegionList.first();
			irp != NULL;
			irp = (IconRegion *)wmScr->iconRegionList.next())
		    irp->expand(wmScr->vwidth, wmScr->vheight, bwidth, bheight);
	    }
	    wmScr->vwidth = bwidth;
	    wmScr->vheight = bheight;
	    wmScr->vdt->set_object_size(bwidth, bheight);
	    wmScr->pan->set_size(width, height);
	    wmScr->pan->set_span(bwidth, bheight);
	    wp->set_attr_width(width);
	    wp->set_attr_height(height);
	}
    }

    // if there is a size object, update it
    wmUpdateSize(wp);
}

/***********************************************************************
 *
 *  Procedure:
 *      wmUpdateSize
 *
 *  Function:
 *	If the decoration panel has a size object, show the new size
 *
 ***********************************************************************
 */

void
wmUpdateSize(
    wmData *wp
    )
{
    if (wp == NULL)
	return;

    if (wp->oi_size())
    {
	int dwidth, dheight;
	char str[20];

	if (wp->myip())
	{
	    dwidth = wp->myip()->columns;
	    dheight = wp->myip()->rows;
	}
	else
	    wmUnitSize(wp, wp->attr_width(), wp->attr_height(), &dwidth, &dheight);
	sprintf(str, " %d x %d ", dwidth, dheight);
	wp->oi_size()->set_text(str);
    }
}

/***********************************************************************
 *
 *  Procedure:
 *      wmSave
 *
 *  Function:
 *	Save the current location and size of the window
 *
 ***********************************************************************
 */

void
wmSave(
    wmData *wp
    )
{
    wp->set_save_x((int)wp->oi_frame()->loc_x());
    wp->set_save_y((int)wp->oi_frame()->loc_y());
    wp->set_save_width(wp->attr_width());
    wp->set_save_height(wp->attr_height());
}

/***********************************************************************
 *
 *  Procedure:
 *      wmRestore
 *
 *  Function:
 *	Restore the widnoe to the size and locations that was saved.
 *
 ***********************************************************************
 */

void
wmRestore(
    wmData *wp
    )
{
    wp->clear_zoomed();
    wp->clear_hori_zoomed();
    wp->clear_vert_zoomed();

    // move the frame to the saved location
    wp->move(wp->save_x(), wp->save_y());

    // resize the client only if the size is different
    if (wp->save_width() != wp->attr_width() || wp->save_height() != wp->attr_height())
	wmResizeClient(wp, wp->save_width(), wp->save_height(), True);
}

/***********************************************************************
 *
 *  Procedure:
 *      wmZoom
 *
 *  Function:
 *	Make the window the full size of the screen, or the max window
 *	size if the client has specified the appropriate hints.
 *
 ***********************************************************************
 */

int
wmZoom(
    int what,			// F_ZOOM, F_HORIZOOM, or F_VERTZOOM
    wmData *wp,			// the window data
    int *x_return,		// the new X coordinate
    int *y_return		// the new Y coordinate
    )
{
    Window      junkRoot, junkChild;
    int         junkX, junkY;
    unsigned int clientWidth, clientHeight, clientBW, junkDepth;
    int x, y;
    int width, height;

    XGetGeometry(DPY, wp->oi_frame()->outside_X_window(), &junkRoot, &dragx, &dragy, (unsigned int *)&dragWidth, (unsigned int *)&dragHeight, &dragBW,&junkDepth);
    XGetGeometry(DPY, wp->window(), &junkRoot, &junkX, &junkY, &clientWidth, &clientHeight, &clientBW, &junkDepth);

    dragx += dragBW;
    dragy += dragBW;

    widthPad = dragWidth - clientWidth;
    heightPad = dragHeight - clientHeight;

    dragWidth = wmScr->width - widthPad;
    dragHeight = wmScr->height - heightPad;

    if (wmScr->vdt && wmScr->scrollBars)
    {
	// subtract off the sizes of the scroll bars on the virtual root
	OI_d_tech *sb = wmScr->vdt->right_scroll_bar();
	if (sb)
	    dragWidth -= sb->space_x();
	sb = wmScr->vdt->bottom_scroll_bar();
	if (sb)
	    dragHeight -= sb->space_y();
    }

    widthPad = 0;
    heightPad = 0;
    constrainSize(wp);

    // translate 0,0 to the top left of the visible virtual root
    XTranslateCoordinates(DPY, wmScr->root, wp->root()->X_window(),
	0, 0, &x, &y, &junkChild);

    width = dragWidth;
    height = dragHeight;

    switch (what)
    {
	case F_ZOOM:
	    if (wp->zoomed())
		return (False);
	    wp->set_zoomed();
	    break;
	case F_HORIZOOM:
	    if (wp->hori_zoomed())
		return (False);
	    wp->set_hori_zoomed();
	    y = (int)wp->oi_frame()->loc_y();
	    height = wp->attr_height();
	    break;
	case F_VERTZOOM:
	    if (wp->vert_zoomed())
		return (False);
	    wp->set_vert_zoomed();
	    x = (int)wp->oi_frame()->loc_x();
	    width = wp->attr_width();
	    break;
    }

    wp->move(x, y);
    wmResizeClient(wp, width, height, True);

    *x_return = x;
    *y_return = y;

    return (True);
}

static wmList sweepList;

void
wmUnSweep()
{
    int i;
    wmObjectData *odp;

    // clean out the sweep list
    while (sweepList.get() != NULL) {}
    sweepList.init();

    if (wmSweptCount)
    {
	for (i = 1; i < wmSweptCount; i++)
	{
	    if (wmSwept[i])
	    {
		odp = (wmObjectData *)wmSwept[i]->data();
		odp->sweep = 0;
	    }
	}
	free((char *)wmSwept);
	wmSweptCount = 0;
    }
}

void
wmSweep(XButtonEvent *ev)
{
    XEvent event;
    Window junkRoot, junkChild;
    int junkX, junkY;
    int x_root, y_root;
    unsigned int junkMask;
    int i;
    char resizeGrid = wmScr->resizeGrid;
    wmObjectData *odp;
    int newx, newy;
    Window child;

    // if a pointer button is not down, wait for one to go down
    XQueryPointer(DPY, wmScr->root, &junkRoot, &junkChild, &x_root, &y_root, &junkX, &junkY, &junkMask);

    if ((junkMask & B_MASK) == 0)
    {
	ev = (XButtonEvent *)&event;

	// change the cursor while we play games 
	wmConn->grab_pointer(wmScr->root, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, wmScr->root, wmScr->sweepCursor, CurrentTime);

	int done = False;
	while (!done)
	{
	    XMaskEvent(DPY, ButtonPressMask | ButtonReleaseMask, (XEvent *)&event);
	    if (ev->type == ButtonPress)
		done = True;
	}
    }

    wmScr->resizeGrid = False;
    wmStartResize(ev, NULL);
    wmScr->resizeGrid = resizeGrid;

    // if the virtual desktop is on, translate the coordinates to it.
    // clean up the old sweep information
    wmUnSweep();

    // OK, look for windows within the drag box
    // first look for windows on the virtual desktop
    if (wmScr->vdt)
    {
	XTranslateCoordinates(DPY, wmScr->root, wmScr->vroot->X_window(), dragx, dragy, &newx, &newy, &child);
	findWindows(wmScr->vroot->X_window(), newx, newy);
    }

    // try the real root if we found nothing
    if (sweepList.count == 0)
	findWindows(wmScr->root, dragx, dragy);

    if (sweepList.count != 0)
    {
	wmSweptCount = sweepList.count + 1;
	wmSwept = (OI_d_tech **)malloc(sizeof(OI_d_tech *) * wmSweptCount);
	wmSwept[0] = NULL;
	for (i = 1; i < wmSweptCount; i++)
	{
	    wmSwept[i] = (OI_d_tech *)sweepList.get();
	    odp = (wmObjectData *)wmSwept[i]->data();
	    odp->sweep = i;
	}
    }
}


/***********************************************************************
 *
 *  Procedure:
 *	findWindows
 *
 *  Function:
 *	Look for windows enclosed in the drag rectangle
 *
 ***********************************************************************
 */

static void
findWindows(
    Window root,	// the root window to look in
    int x,		// the x coordinate of the bounding box
    int y		// the y coordinate of the bounding box
    )
{
    Window root_return, parent_return, *child;
    unsigned int nchildren;
    int i;
    wmData *wp;
    OI_d_tech *oi;

    XQueryTree(DPY, root, &root_return, &parent_return, &child, &nchildren);
    for (i = 0; i < nchildren; i++)
    {
	// see if we know about this window, we only care about frames and icons
	oi = NULL;
	if (XFindContext(DPY, child[i], wmFrameContext, (caddr_t*)&wp) != XCNOENT)
	    oi = wp->oi_frame();
	else if (XFindContext(DPY, child[i], wmIconContext, (caddr_t*)&wp) != XCNOENT)
	    oi = wp->oi_icon();

	if (oi && oi->state() == OI_ACTIVE)
	{
	    if (oi->loc_x() >= x && oi->loc_y() >= y &&
		(oi->loc_x() + oi->space_x()) <= (x + dragWidth) &&
		(oi->loc_y() + oi->space_y()) <= (y + dragHeight))
	    {
		sweepList.append((ent)oi);
	    }
	}
    }
}
