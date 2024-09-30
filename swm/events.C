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
 * $Id: events.C,v 9.36 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Event handling routines
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/

#ifndef lint
static char RCSinfo[] =
"$Id: events.C,v 9.36 1993/08/27 16:58:15 toml Exp $";
#endif /* lint */

#include "swm.H"
#include "screen.H"
#include "init.H"
#include "wmdata.H"
#include "events.H"
#include "main.H"
#include "debug.H"
#include "reparent.H"
#include "panel.H"
#include "object.H"
#include "icons.H"
#include "util.H"
#include "resize.H"
#include "execute.H"
#include "parse.H"
#include "move.H"
#include "atoms.H"
#include "gram.H"
#include "swmhelp.H"

#ifdef MEMLEAK
extern int mfd, ffd, nfd, dfd, vnfd, vdfd;
#endif /* MEMLEAK */

void wmRootPropertyNotify(XPropertyEvent *,struct wmData *);
static void rootEnterNotify(XEnterWindowEvent *,struct wmData *);
static void rootUnmapNotify(XUnmapEvent *,struct wmData *);
static wmList enterStack;
wmData *wmFocusWp = NULL;
static wmData *setFocusWp = NULL;
extern void wmLowerWindow(wmData *, Window);
static void installColormap(Colormap);
static Time lastFocusTime = CurrentTime;

#ifdef DEBUG
void wmCheck();
#endif

// #define PE(string) \
//     if (wmDebug) \
// 	if (ev) \
// 	    fprintf(efp, "%s: w = 0x%x\n", string, ev->window);\
// 	else \
// 	    fprintf(efp, "%s:\n", string)\

#ifdef DEBUG
#define PE(string) \
	if (wmDebug) \
		DebugEvent((XEvent *)ev);
#endif

#define FIND_CONTEXT(window) \
    if (XFindContext(DPY, window, wmContext, (caddr_t*)&wp) == XCNOENT)\
	wp = NULL


#define SUB SubstructureRedirectMask

static OI_dispatch_entry rootEvents[] = {
{MapRequest,		SUB,			(OI_event_fnp)wmMapRequest,		NULL, NULL_PMF, NULL },
{ConfigureRequest,	SUB,			(OI_event_fnp)wmConfigureRequest,	NULL, NULL_PMF, NULL },
{ClientMessage,		SUB,			(OI_event_fnp)wmClientMessage,		NULL, NULL_PMF, NULL },
{PropertyNotify,	PropertyChangeMask,	(OI_event_fnp)wmRootPropertyNotify,	NULL, NULL_PMF, NULL },
{ColormapNotify,	ColormapChangeMask,	(OI_event_fnp)wmColormapNotify,		NULL, NULL_PMF, NULL },
{EnterNotify, 		EnterWindowMask,	(OI_event_fnp)rootEnterNotify,		NULL, NULL_PMF, NULL },
{UnmapNotify, 		0,			(OI_event_fnp)rootUnmapNotify,		NULL, NULL_PMF, NULL },
};

#define N_EVENTS (sizeof(rootEvents)/sizeof(OI_dispatch_entry))

/***********************************************************************
 *
 *  Procedure:
 *      wmInitEvents
 *
 *  Function:
 *	Initialize events on the specific root window.  Called once for
 *	each screen.
 *
 ***********************************************************************
 */

void
wmInitEvents()
{
    wmScr->conp->dispatch_group_insert(wmScr->root, N_EVENTS,&rootEvents[0]);
    if (wmScr->vdt)
	wmScr->conp->dispatch_group_insert(wmScr->vroot->X_window(), N_EVENTS,&rootEvents[0]);
    enterStack.init();
}

/***********************************************************************
 *
 *  Procedure:
 *      wmMapRequest
 *
 *  Function:
 *	Handle the MapRequest event.  This event could mean several things:
 *	It could be called from wmReparentExisting when starting up, to 
 *	reparent each window.  It could be a real map request from a client.
 *	If I don't know about the window when the event comes in, I 
 *	go off and reparent it.  If I do know about it, I deiconify it.
 *
 ***********************************************************************
 */

void
wmMapRequest(
    XMapRequestEvent *ev,
    wmData *wp
    )
{
#ifdef DEBUG
    PE("wmMapRequest");
#endif
#ifdef DEBUG
	wmCheck();
#endif

    // if the window has not been mapped before ...
    if (wp == NULL)
    {
	wmFindScreen(wp, ev->window);
	if (wmScr == NULL)
	    return;
	
	// make REALLY sure that we haven't
	FIND_CONTEXT(ev->window);
	if (wp == NULL)
	{
	    if (wmScr->vdt)
	    {
		if (ev->window == wmScr->pan->X_window())
		{
		    wp = wmReparent(ev->window, ev->parent, wmScr->conp->abs_root());
		    wp->set_panner();
		}
		else if (ev->window == wmScr->vdt->outside_X_window())
		{
		    wp = wmReparent(ev->window, ev->parent, wmScr->conp->abs_root());
		}
		else
		{
		    wp = wmReparent(ev->window, ev->parent, wmScr->vroot);
		}
	    }
	    else
		wp = wmReparent(ev->window, ev->parent, wmScr->vroot);

	    if (wp == NULL)
		return;
	}
    }
    else if (wp->state() == WithdrawnState)
    {
	if (wp->gray_icon() && wp->oi_icon())
	    wmPlaceIcon(wp, 0, 0);
    }

    if ((wp->state() != IconicState) && (wp->wmhints()->flags & StateHint))
    {
	switch (wp->wmhints()->initial_state)
	{
            case IconicState:
	    {
		char saveZap = wmScr->zap;
		wmScr->zap = False;
		// if the iconify works, no problem.  if it fails, then
		// fall through and map the window
		int stat = wmIconify(wp, 0, 0);
		wmScr->zap = saveZap;
		if (stat)
		    break;
		// else fall through
	    }

            case DontCareState:
            case NormalState:
            case ZoomState:
            case InactiveState:
                XMapWindow(DPY, wp->window());
                XRaiseWindow(DPY, wp->oi_frame()->outside_X_window());
		wp->map();
		if (wp->ol_default_btn())
		{
		    XWarpPointer(DPY, None, wp->window(), 0,0,0,0, wp->ol_default_btn()->warp_x, wp->ol_default_btn()->warp_y);
		    if (wmFocusModel == wmFocusModelClickToType)
			wmSetFocus(NULL, wp);
		}
                wmSetWM_STATE(wp, NormalState);
                break;
	}
    }
    // else no hints or an icon, simply deiconify it
    else
    {
	wmDeiconify(wp);
    }
#ifdef DEBUG
	wmCheck();
#endif
}

/**********************************************************************
 *
 *  Procedure:
 *	wmMapNotify
 *
 *  Function:
 *	Handle MapNotify events.  These events should only come from
 *	pinned menus that I own.
 *
 **********************************************************************
 */

void
wmMapNotify(
    XMapEvent *ev,
    char *
    )
{
#ifdef DEBUG
    PE("wmMapNotify");
#endif
#ifdef DEBUG
	wmCheck();
#endif

#ifdef OLD
    if (ev->override_redirect)
        return;

    // OK this must be a pinned menu that we want to map.
    // check to see if I have already reparented the thing
    wmData *wp;
    FIND_CONTEXT(ev->window);
    if (wp)
        return;

    if (XFindContext(DPY, ev->window, wmRootPanelsContext, (caddr_t*)&oi) == XCNOENT)
        XUnmapWindow(DPY, ev->window);
    else {
        oi->set_state(OI_NOT_DISPLAYED);
    }
    XMapRequestEvent map;
    map.display = ev->display;
    map.window = ev->window;
    map.parent = wmScr->root;
    wmMapRequest(&map, NULL);
#endif
#ifdef DEBUG
	wmCheck();
#endif
}

/**********************************************************************
 *
 *  Procedure:
 *	wmVrootCreateNotify
 *
 *  Function:
 *	Handle CreateNotify events on the virtual root.  If the window is
 *	override-redirect, add it to the save set so if swm crashes
 *	the window won't be lost.
 *
 **********************************************************************
 */

void
wmVrootCreateNotify(
    XCreateWindowEvent *ev,
    char *
    )
{
#ifdef DEBUG
	wmCheck();
#endif
#ifdef DEBUG
    PE("wmVrootCreateNotify");
#endif
    if (IS_INTERNAL(ev->window))
	return;

    XAddToSaveSet(DPY, ev->window);
#ifdef DEBUG
	wmCheck();
#endif
    return;
}
/**********************************************************************
 *
 *  Procedure:
 *	wmVrootReparentNotify
 *
 *  Function:
 *	Handle ReparentNotify events on the virtual root.  If the window is
 *	override-redirect, add it to the save set so if swm crashes
 *	the window won't be lost.
 *
 **********************************************************************
 */

void
wmVrootReparentNotify(
    XReparentEvent *ev,
    char *
    )
{
#ifdef DEBUG
    PE("wmVrootReparentNotify");
#endif
#ifdef DEBUG
	wmCheck();
#endif
    if (IS_INTERNAL(ev->window))
	return;

    if (ev->parent == wmScr->vroot->X_window())
        XAddToSaveSet(DPY, ev->window);
    else
        XRemoveFromSaveSet(DPY, ev->window);
#ifdef DEBUG
	wmCheck();
#endif
    return;
}

/***********************************************************************
 *
 *  Procedure:
 *      wmDestroyNotify
 *
 *  Function:
 *	Handle the DestroyNotify event.  This event is my cue to forget
 *	about the client window.  This can be a real DestroyNotify
 *	event or can come from the UnmapNotify event handler.
 *
 ***********************************************************************
 */

void
wmDestroyNotify(
    XDestroyWindowEvent *ev,
    wmData *wp
    )
{
    wmList *list;
    wmObjectData *odp;

#ifdef DEBUG
    PE("wmDestroyNotify");
#endif
#ifdef DEBUG
	wmCheck();
#endif

    FIND_CONTEXT(ev->window);
    if (wp == NULL)
    {
#ifdef DEBUG
	if (wmDebug > 1)
	    fprintf(dfp, "  context not found\n");
#endif
	return;
    }

#ifdef DEBUG
#ifdef MEMLEAK
    if (mfd)
    {
	int len;
	char *p="wmDestroyNotify: name=";
	len = strlen(p);
	write(mfd, p, len);
	write(ffd, p, len);
	write(nfd, p, len);
	write(dfd, p, len);
	write(vnfd, p, len);
	write(vdfd, p, len);
	len = strlen(wp->name());
	write(mfd, wp->name(), len);
	write(ffd, wp->name(), len);
	write(mfd, "\n", 1);
	write(ffd, "\n", 1);
	write(nfd, wp->name(), len);
	write(dfd, wp->name(), len);
	write(nfd, "\n", 1);
	write(dfd, "\n", 1);
	write(vnfd, wp->name(), len);
	write(vdfd, wp->name(), len);
	write(vnfd, "\n", 1);
	write(vdfd, "\n", 1);
    }
#endif /* MEMLEAK */
#endif


	if (wmScr->colormapFocus == wp) {
		wmScr->colormapFocus = NULL;
		wmResetColormaps();
	}

    // remove them from the sweep list if they are in it
    odp = (wmObjectData *)wp->oi_frame()->data();
    if (odp->sweep)
	wmSwept[odp->sweep] = NULL;

    if (wp->oi_icon())
    {
	odp = (wmObjectData *)wp->oi_icon()->data();
	if (odp->sweep)
	    wmSwept[odp->sweep] = NULL;
    }

    if (wp == wmFocusWp)
	wmClearFocus(NULL, wmFocusWp);

    // free the class hints
    if (wp->wclass_name() != wmNoName) XFree(wp->wclass_name());
    if (wp->wclass_class() != wmNoName) XFree(wp->wclass_class());

    // free the name and icon name
    if (wp->icon_name() != wp->name() && wp->icon_name() != wmNoName)
	XFree(wp->icon_name());

    if (wp->name() != wmNoName)
	XFree(wp->name());

    // if the window is mine, simply remove the events I added.  This is
    // so I don't discard events for pinned menus that have not yet been
    // processed.
    if (wp->mine())
	wmRemoveClientEvents(wp);
    else
	wmScr->conp->dispatch()->discard(wp->window());

    // free up the focus list (if any)
    list = wp->focus_list_p();
    for (OI_d_tech *oi = (OI_d_tech *)list->first(); oi != NULL; oi = (OI_d_tech *)list->get());

    if (wp->colormap_windows())
    {
	for (int i = 0; i < wp->colormap_count(); i++)
	{
	    if (wp->colormap_windows()[i] == wp->window())
		continue;

	    wmScr->conp->dispatch()->discard(wp->colormap_windows()[i]);
	}
	XFree((char *)wp->colormap_windows());
    }
    if (wp->protocols())
	XFree((char *)wp->protocols());

    XDeleteContext(DPY, wp->window(), wmContext);
    XDeleteContext(DPY, wp->oi_frame()->outside_X_window(), wmFrameContext);
    if (wp->oi_icon())
	XDeleteContext(DPY, wp->oi_icon()->outside_X_window(), wmIconContext);

    // remove the window from the list
    wmScr->windowList.rm((ent)wp);

    // get rid of all the data associated with the objects
    wmDeleteOI(wp->oi_frame(), True);
    if (wp->vbox()) 
    {
	wmDeleteOI((OI_d_tech *)wp->vbox(), True);
    }

    // get rid of the icon
    wmRemoveIcon(wp);
    if (wp->wmhints() && (wp->wmhints()->flags & IconWindowHint))
    {
	XUnmapWindow(DPY, wp->wmhints()->icon_window);
	XReparentWindow(DPY, wp->wmhints()->icon_window, wmConn->root()->X_window(), 0, 0);
    }
    if (wp->oi_icon())
	wmDeleteOI(wp->oi_icon(), True);
    if (wp->vibox()) 
    {
	wmDeleteOI((OI_d_tech *)wp->vibox(), True);
    }

    if (wp->free_wm_hints())
	free((char *)wp->wmhints());
    else
	XFree((char *)wp->wmhints());

    // fix all transients that are still hanging around
    list = wp->transient_list_p();
    for (wmData *tmpwp = (wmData *)list->first(); tmpwp != NULL; tmpwp = (wmData *)list->next())
	tmpwp->set_transient_for_wp(NULL);

    // if this is a transient, remove it from the list of transients on the parent
    if (wp->transient_for_wp())
    {
	wp->transient_for_wp()->transient_list_p()->rm((ent)wp);
    }

#ifdef DEBUG
#ifdef MEMLEAK
    if (mfd)
    {
	int len;
	char *p="wmDestroyNotify: DONE name=";
	len = strlen(p);
	write(mfd, p, len);
	write(ffd, p, len);
	write(nfd, p, len);
	write(dfd, p, len);
	write(vnfd, p, len);
	write(vdfd, p, len);
	len = strlen(wp->name());
	write(mfd, wp->name(), len);
	write(ffd, wp->name(), len);
	write(mfd, "\n", 1);
	write(ffd, "\n", 1);
	write(nfd, wp->name(), len);
	write(dfd, wp->name(), len);
	write(nfd, "\n", 1);
	write(dfd, "\n", 1);
	write(vnfd, wp->name(), len);
	write(vdfd, wp->name(), len);
	write(vnfd, "\n", 1);
	write(vdfd, "\n", 1);
    }
#endif /* MEMLEAK */
#endif /* DEBUG */
    // get rid of the structure
    delete wp;

#ifdef DEBUG
	wmCheck();
#endif
}

static void
rootUnmapNotify(
    XUnmapEvent *ev,
    wmData *wp
    )
{
	FIND_CONTEXT(ev->window);
	if (wp)
		wmUnmapNotify(ev, wp);
}


/***********************************************************************
 *
 *  Procedure:
 *      wmUnmapNotify
 *
 *  Function:
 *	Handle UnmapNotify events.  If I get this event, it means that
 *	the client wants to go to withdrawn state.  I simply call my
 *	DstroyNotify event handler to forget about the window.
 *
 ***********************************************************************
 */

void
wmUnmapNotify(
    XUnmapEvent *ev,
    wmData *wp
    )
{
    Window old_parent;

#ifdef DEBUG
    PE("wmUnmapNotify");
#endif
#ifdef DEBUG
	wmCheck();
#endif
    if (wp == NULL)
	return;

    // the window is having withdrawl symptoms, take it into withdrawn state
    int dstx, dsty;
    XDestroyWindowEvent dev;

    old_parent = wp->root()->X_window();

    // if the default button is still set, return the pointer
    // to the saved location
    if (wp->warp_back())
    {
	XWarpPointer(DPY, wp->oi_client()->X_window(), wmScr->root, wp->ol_default_btn()->button_x, wp->ol_default_btn()->button_y,
	    wp->ol_default_btn()->button_width, wp->ol_default_btn()->button_height, wp->root_x(), wp->root_y());

	// if we are in click to type mode, we probably want the focus back
	// on the window we warped from
	if (wmFocusModel != wmFocusModelNormal && wp->last_focus())
	    wmSetFocus(NULL, wp->last_focus());
    }

    // this is interesting here.  If the client did a map, unmap, flush, map
    // then there is a good chance that there is a map request on the
    // event queue.  The following code simply pulls it off if it is there
    // and then hands it to the map request handler
    wmGrabServer();
    XSync(DPY, False);
    wp->unmap();

    wmSetWM_STATE(wp, WithdrawnState);
    XRemoveFromSaveSet(DPY, ev->window);

    int gravx, gravy;
    dstx = (int)wp->oi_frame()->loc_x();
    dsty = (int)wp->oi_frame()->loc_y();
    int xright = dstx + wp->oi_frame()->space_x();
    int ybottom = dsty + wp->oi_frame()->space_y();
    wmGetGravityOffsets(wp, &gravx, &gravy);
    if (gravx == 1)
	dstx = xright - wp->attr_width() - 2*wp->attr_bw();
    if (gravy == 1)
	dsty = ybottom - wp->attr_height() - 2*wp->attr_bw();

    // if this is a menu, reparent back to the real root window
#ifdef OLD
    if (XFindContext(DPY, wp->window(), wmMenuContext, (caddr_t*)&oi) != XCNOENT)
    {
        OI_d_tech *parent;
        parent = wmScr->conp->root();
        odp = (wmObjectData *)oi->data();
        if (odp && odp->parent)
                parent = odp->parent;
        oi->set_associated_object(parent, dstx, dsty, OI_ACTIVE_NOT_DISPLAYED);
    }
    else
#endif
    {
	// This really sucks!  If the client did an XReparentWindow, I shouldn't
	// reparent the window back to the root.  I need to check to see if the
	// window's parent is who I think it should be, if it is, then I can safely
	// do the reparent.

	Window root, parent, *children;
	unsigned int num_children;
	XQueryTree(DPY, wp->oi_client()->X_window(), &root, &parent, &children, &num_children);
	// if this window has a child, it must be the client window
	if (num_children)
	{
	    XReparentWindow(DPY, wp->window(), wp->root()->X_window(), dstx, dsty);
	    XFree((char *)children);
	}
    }

    // check to see if there are any pending Map requests on
    // the parent of the client
    XEvent maprequest;
    int pendingMap = False;
    if (XCheckTypedWindowEvent(DPY, wp->oi_client()->X_window(), MapRequest, &maprequest))
	pendingMap = True;

    // check to see if there are any pending Configure requests on
    // the parent of the client
    XEvent configurerequest;
    int pendingConfigure = False;
    if (XCheckTypedWindowEvent(DPY, wp->oi_client()->X_window(), ConfigureRequest, &configurerequest))
	pendingConfigure = True;

    dev.type = DestroyNotify;
    dev.serial = ev->serial;
    dev.send_event = False;
    dev.display = DPY;
    dev.event = ev->event;
    dev.window = ev->window;
    wmDestroyNotify(&dev, wp);

    if (pendingConfigure)
	wmConfigureRequest((XConfigureRequestEvent *)&configurerequest, NULL);

    if (pendingMap) {
	maprequest.xmaprequest.parent = old_parent;
	wmMapRequest((XMapRequestEvent *)&maprequest, NULL);
    }

    wmUngrabServer();
#ifdef DEBUG
	wmCheck();
#endif
}

/***********************************************************************
 *
 *  Procedure:
 *      wmConfigureRequest
 *
 *  Function:
 *	Handle ConfigureRequest events and resize/reposition the frame
 *	accordingly.
 *
 ***********************************************************************
 */

void
wmConfigureRequest(
    XConfigureRequestEvent *ev,
    wmData *wp
    )
{
    XWindowChanges xwc;
    unsigned int xwcm;

#ifdef DEBUG
    PE("wmConfigureRequest");
#endif
#ifdef DEBUG
	wmCheck();
#endif
    if (wp == NULL)
	FIND_CONTEXT(ev->window);

    if (wp == NULL)
    {
	// respond to request for a never before mapped window
        xwcm = (unsigned int)(ev->value_mask & (CWBorderWidth | CWX | CWY | CWWidth | CWHeight));
        xwc.x = ev->x;
        xwc.y = ev->y;
        xwc.width = ev->width;
        xwc.height = ev->height;
        xwc.border_width = ev->border_width;
        XConfigureWindow(DPY, ev->window, xwcm, &xwc);
        return;
    }

    if (ev->value_mask & CWStackMode)
    {
        if (ev->detail == Above)
	{
	    wp->raise();
	    // if there is an open look default button, warp to it
	    if (wp->ol_default_btn())
	    {
		Window junkRoot, junkChild;
		int junkX, junkY;
		unsigned int junkMask;

		// save the pointer location
		XQueryPointer(DPY, wmScr->root, &junkRoot, &junkChild,
		    wp->root_x_p(), wp->root_y_p(), &junkX, &junkY, &junkMask);
		XWarpPointer(DPY, None, wp->window(), 0,0,0,0, wp->ol_default_btn()->warp_x, wp->ol_default_btn()->warp_y);
		wp->set_warp_back();

		// if we are in click to type mode, set the focus to the window
		if (wmFocusModel == wmFocusModelClickToType)
		    wmSetFocus(NULL, wp);
	    }
	}
        else if (ev->detail == Below)
	    wp->lower();
    }
    else
    {
	int ox, oy, nx, ny;
	unsigned owidth, oheight, obw;

	ox = nx = (int)wp->oi_frame()->loc_x();
	oy = ny = (int)wp->oi_frame()->loc_y();

	if (ev->value_mask & CWX)
	    nx = ev->x;
	if (ev->value_mask & CWY)
	    ny = ev->y;

	if (nx != ox || ny != oy)
	    wp->move(nx, ny);

	owidth = wp->attr_width();
	oheight = wp->attr_height();
	obw = wp->attr_bw();

	if (ev->value_mask & CWWidth)
	    wp->set_attr_width(ev->width);
	if (ev->value_mask & CWHeight)
	    wp->set_attr_height(ev->height);
	if (ev->value_mask & CWBorderWidth)
	{
	    XSetWindowBorderWidth(DPY, wp->window(), ev->border_width);
	    wp->set_attr_bw(ev->border_width);
	}

	if (owidth != wp->attr_width() || oheight != wp->attr_height() || obw != wp->attr_bw())
	    wmResizeClient(wp, wp->attr_width(), wp->attr_height(), True);
	else
	    wmSendEvent(wp);
    }
#ifdef DEBUG
	wmCheck();
#endif
}

/**********************************************************************
 *
 *  Procedure:
 *	wmFrameVisibilityNotify
 *
 *  Function:
 *	Handle VisibilityNotify events on the frame window.  This
 *	information is then used in the f.raiselower command
 *
 **********************************************************************
 */

void
wmFrameVisibilityNotify(
    XVisibilityEvent *ev,
    wmData *wp
    )
{
#ifdef DEBUG
    PE("wmVisibilityNotify");
#endif
    wp->set_frame_vis(ev->state);
}

/**********************************************************************
 *
 *  Procedure:
 *	wmIconVisibilityNotify
 *
 *  Function:
 *	Handle VisibilityNotify events on the icon frame.  This
 *	information is then used in the f.raiselower command
 *
 **********************************************************************
 */

void
wmIconVisibilityNotify(
    XVisibilityEvent *ev,
    wmData *wp
    )
{
#ifdef DEBUG
    PE("wmVisibilityNotify");
#endif
    wp->set_icon_vis(ev->state);
}

/***********************************************************************
 *
 *  Procedure:
 *      wmRootPropertyNotify
 *
 *  Function:
 *	Handle PropertyNotify events on the root window.  The only real
 *	one that I am expecting is the __SWM_COMMAND property.
 *
 ***********************************************************************
 */

void
wmRootPropertyNotify(
    XPropertyEvent *ev,
    struct wmData *
    )
{
    int junk1;
    unsigned long  junk2, len;
    Atom actual;
    char *prop;

#ifdef DEBUG
    PE("wmRootPropertyNotify");
    if (wmDebug)
    {
	char *s = XGetAtomName(DPY, ev->atom);
	fprintf(dfp, "  atom = %s\n", s);
	XFree(s);
    }
#endif
#ifdef DEBUG
	wmCheck();
#endif

    XGetWindowProperty(DPY, ev->window, ev->atom, 0, 1000, False,
        XA_STRING, &actual, &junk1, &junk2, &len, (unsigned char **)&prop);

    if (ev->atom == __SWM_COMMAND)
    {
	wmBinding *bp;
	bp = wmBind = new wmBinding();
	wmParse("wmFunction", prop);
	if (wmParseError)
	{
	    fprintf(stderr,
	    "swm: wmRootPropertyNotify: syntax error \"%s\"\n",
		prop);
	    delete bp;
	    bp = NULL;
	}
	if (bp)
	{
	    wmExecuteBinding(NULL, NULL, bp, NOT_FROM_MENU, 0, 0);
	}
    }
    XFree(prop);
#ifdef DEBUG
	wmCheck();
#endif
}

/***********************************************************************
 *
 *  Procedure:
 *      wmPropertyNotify
 *
 *  Function:
 *	Handle PropertyNotify events on a client window.
 *
 ***********************************************************************
 */

void
wmPropertyNotify(
    XPropertyEvent *ev,
    struct wmData *wp
    )
{
    int i;
    wmObjectData *odp;
    long supplied;
    unsigned long  nitems, bytesafter;
    Atom actual_type;
    int actual_format;
    char *prop = NULL;
    int old_width;
    int old_height;
    int out_width;
    int out_height;

#ifdef DEBUG
    PE("wmPropertyNotify");
    if (wmDebug)
    {
	char *s = XGetAtomName(DPY, ev->atom);
	fprintf(dfp, "  atom = %s\n", s);
	XFree(s);
    }
#endif
#ifdef DEBUG
	wmCheck();
#endif

    if (wp == NULL)
	return;

    if (ev->atom == WM_COLORMAP_WINDOWS)
    {
	// go get the new colormap windows
	wmGetColormapWindows(wp);
    }
    else if (ev->atom == WM_PROTOCOLS) {
	wp->clear_protocol_bits();
	XGetWMProtocols(DPY, wp->window(), wp->protocols_p(), wp->protocols_count_p());
	for (i = 0; i < wp->protocols_count(); i++)
	{
	    if (wp->protocols()[i] == WM_TAKE_FOCUS)
	    {
		wp->set_focus_type(wmTakeFocusSpecified);
		wp->set_protocol_bits(wmTakeFocus);
	    }
	    else if (wp->protocols()[i] == WM_SAVE_YOURSELF)
		wp->set_protocol_bits(wmSaveYourself);
	    else if (wp->protocols()[i] == WM_DELETE_WINDOW)
		wp->set_protocol_bits(wmDeleteWindow);
	    else if (wp->protocols()[i] == WM_SHUTDOWN)
		wp->set_protocol_bits(wmShutdown);
	}
    }
    else if (ev->atom == WM_COMMAND)
    {
	// if we have sent a WM_SHUTDOWN or WM_SAVE_YOURSELF message,
	// this property indicates that the client is done and can
	// be killed.
	if (wp->kill_sent())
	    XKillClient(DPY, wp->window());
    }
    else if (ev->atom == _OL_WIN_BUSY)
    {
	int *intdata;
        if (XGetWindowProperty (DPY, wp->window(), _OL_WIN_BUSY,
                0L, 1000000L, False, XA_INTEGER, &actual_type, &actual_format,
                &nitems, &bytesafter, (unsigned char **) &intdata) == Success && intdata)
	{
	    if (*intdata)
		wp->set_busy();
	    else
		wp->clear_busy();
	    XFree((char *)intdata);
	    wmBusy(wp);
	}
    }
    else
    {
	XGetWindowProperty(DPY, wp->window(), ev->atom, 0, 200, False,
	    XA_STRING, &actual_type, &actual_format, &nitems, &bytesafter, (unsigned char **)&prop);

	if (actual_type == None)
	    return;

	if (prop == NULL)
	    prop = wmNoName;

	switch (ev->atom)
	{
	    case XA_WM_ICON_NAME:
		// only do work if the name has changed
		if (strcmp(wp->icon_name(), prop))
		{
		    if (wp->icon_name() != wp->name() && wp->icon_name() != wmNoName)
			XFree(wp->icon_name());
		    wp->set_icon_name(prop);
		    wmDisplayIconName(wp);
		    prop = wmNoName;
		}
		break;

	    case XA_WM_NAME:
		// only do work if the name has changed
		if (strcmp(wp->name(), prop))
		{
		    if (wp->name() != wp->icon_name() && wp->name() != wmNoName)
			XFree(wp->name());
		    wp->set_name(prop);
		    wmDisplayName(wp);
		    prop = wmNoName;
		}    
		break;

	    case XA_WM_NORMAL_HINTS:
		if (XGetWMNormalHints(DPY, wp->window(), wp->size_hints_p(), &supplied) == 0)
		    wp->set_size_hints_flags(0);
		break;

	    case XA_WM_HINTS:
		if (wp->wmhints()) {
			if (wp->free_wm_hints()) { 
				free((char *)wp->wmhints());
				wp->clear_free_wm_hints();
			}
			else
				XFree((char *)wp->wmhints());
		}
		wp->set_wmhints(XGetWMHints(DPY, wp->window()));
		if (!wp->wmhints()) {
			wp->set_wmhints((XWMHints *)malloc(sizeof(XWMHints)));
			wp->wmhints()->flags = 0;
			wp->set_free_wm_hints();
			break;
		}

		// get group related stuff
		if (!wp->ignore_group_hints() && wp->wmhints()->flags & WindowGroupHint)
		{
		    wp->set_group((int)wp->wmhints()->window_group);
		    wp->set_regroup((int)wp->wmhints()->window_group);
		}
		// +++ remove this kludge +++
		if (wp->group() == 99)
		{
		    wp->set_group(0);
		    wp->set_regroup(0);
		}

		// do icon related stuff,  if there is no icon, we don't
		// need to do anything
		if (!wp->oi_icon())
		    break;

		// if we don't have an iconImage object, we don't need to do
		// anything
		if (!wp->oi_icon_image())
		    break;

		old_width = wp->oi_icon_image()->space_x();
		old_height = wp->oi_icon_image()->space_y();
		out_width = wp->oi_icon()->space_x();
		out_height = wp->oi_icon()->space_y();

		// go get the icon image
		wmMakeIconImage(wp);

		odp = (wmObjectData *)wp->oi_icon_image()->data();
		// did the size of the icon change?
		if (odp->shape || wp->oi_icon_image()->space_x() != old_width || wp->oi_icon_image()->space_y() != old_height)
		{
		    wp->iop()->oi = wp->oi_icon();
		    wmLayoutPanel(wp->iop());
		}

		// did the overall size change?  if it is in an icon panel,
		// relayout the icon panel if needed
		if (wp->ip() &&
		    (wp->oi_icon()->space_x() != out_width ||
		     wp->oi_icon()->space_y() != out_height))
		{
		    if (wmNewIconPanelSizes(wp->ip()))
		    {
			wmLayoutIconPanel(wp->ip());
			wmSizeIconPanel(wp->ip());
		    }
		}
		if (wp->gray_icon())
		    wmGrayIcon(wp);
		break;
	}
	if (prop != wmNoName)
	    XFree(prop);
    }
#ifdef DEBUG
	wmCheck();
#endif
}

/***********************************************************************
 *
 *  Procedure:
 *      wmClientMessage
 *
 *  Function:
 *	Handle ClientMessage events.  The only message I'm really looking
 *	for is the one from the client that puts him into iconic state.
 *
 ***********************************************************************
 */

void
wmClientMessage(
    XClientMessageEvent *ev,
    struct wmData *wp
   )
{
#ifdef DEBUG
    PE("wmClientMessage");
#endif
#ifdef DEBUG
	wmCheck();
#endif

    if (wp == NULL)
	return;

    if (ev->message_type == WM_CHANGE_STATE)
    {
	if (ev->data.l[0] == IconicState && wp->state() != IconicState)
	{
	    Window junkRoot, junkChild;
	    int x, y, junkX, junkY;
	    unsigned junkMask;

	    XQueryPointer(DPY, wmScr->root, &junkRoot, &junkChild,
		&x, &y, &junkX, &junkY, &junkMask);
	    // wmIconify(wp, x, y);
	    wmExternalExecute(wp, wp->oi_frame()->outside_X_window(), F_ICONIFY, NULL, 0, x, y);
	}
    }
    else if (ev->message_type == XA_XV_DO_DRAG_LOAD)
    {
	XConvertSelection(DPY, XA_PRIMARY, XA_STRING, XA_XV_DO_DRAG_LOAD, ev->window, CurrentTime);
    }
#ifdef DEBUG
	wmCheck();
#endif
}

/***********************************************************************
 *
 *  Procedure:
 *      rootEnterNotify
 *
 *  Function:
 *	Handle EnterNotify events on the root window and possibly install
 *	the root window colormap
 *
 ***********************************************************************
 */

static void
rootEnterNotify(
    XEnterWindowEvent *ev,
    struct wmData *
    )
{
#ifdef DEBUG
	wmCheck();
#endif
#ifdef DEBUG
    PE("rootEnterNotify");
    if (wmDebug)
    {
	fprintf(dfp, "  mode = ");
	switch (ev->mode)
	{
	    case NotifyNormal: fprintf(dfp, "NotifyNormal\n"); break;
	    case NotifyGrab: fprintf(dfp, "NotifyGrab\n"); break;
	    case NotifyUngrab: fprintf(dfp, "NotifyUngrab\n"); break;
	}
	fprintf(dfp, "  detail = ");
	switch (ev->detail)
	{
	    case NotifyAncestor: fprintf(dfp, "NotifyAncestor\n"); break;
	    case NotifyVirtual: fprintf(dfp, "NotifyVirtual\n"); break;
	    case NotifyInferior: fprintf(dfp, "NotifyInferior\n"); break;
	    case NotifyNonlinear: fprintf(dfp, "NotifyNonlinear\n"); break;
	    case NotifyNonlinearVirtual: fprintf(dfp, "NotifyNonlinearVirtual\n"); break;
	}
    }
#endif
	if (!wmScr->colormapFocus)
		installColormap(wmScr->cmap);

	if (wmFocusModel == wmFocusModelNormal)
	{
		wmClearFocus(ev, wmFocusWp);
	}
#ifdef DEBUG
	wmCheck();
#endif
}

/***********************************************************************
 *
 *  Procedure:
 *      wmEnterNotify
 *
 *  Function:
 *	Handle EnterNotify events on the client window or sub-windows
 *	if WM_COLORMAP_WINDOWS is set.
 *
 ***********************************************************************
 */

void
wmEnterNotify(
    XEnterWindowEvent *ev,
    struct wmData *wp
    )
{
#ifdef DEBUG
	wmCheck();
#endif
#ifdef DEBUG
    PE("wmEnterNotify");
    if (wmDebug)
    {
	fprintf(dfp, "  mode = ");
	switch (ev->mode)
	{
	    case NotifyNormal: fprintf(dfp, "NotifyNormal\n"); break;
	    case NotifyGrab: fprintf(dfp, "NotifyGrab\n"); break;
	    case NotifyUngrab: fprintf(dfp, "NotifyUngrab\n"); break;
	}
	fprintf(dfp, "  detail = ");
	switch (ev->detail)
	{
	    case NotifyAncestor: fprintf(dfp, "NotifyAncestor\n"); break;
	    case NotifyVirtual: fprintf(dfp, "NotifyVirtual\n"); break;
	    case NotifyInferior: fprintf(dfp, "NotifyInferior\n"); break;
	    case NotifyNonlinear: fprintf(dfp, "NotifyNonlinear\n"); break;
	    case NotifyNonlinearVirtual: fprintf(dfp, "NotifyNonlinearVirtual\n"); break;
	}
	fprintf(dfp, "  time = %d\n", ev->time);
    }
#endif
    if (ev->mode == NotifyGrab || ev->detail == NotifyInferior)
	return;

    if (wmFocusModel == wmFocusModelNormal)
    {
	if ((ev->window == wp->window() ||
	    (ev->window == wp->oi_frame()->outside_X_window() && wp->highlight_frame())))
	{
	    wmSetFocus(ev, wp);
	}
    }

    if (!wmScr->colormapFocus) {
	// do colormap stuff
	enterStack.insert((ent)ev->window);
	XWindowAttributes attr;
	XGetWindowAttributes(DPY, ev->window, &attr);
	installColormap(attr.colormap);
	wmScr->colormapWindow = ev->window;
    }
#ifdef DEBUG
	wmCheck();
#endif
}

/***********************************************************************
 *
 *  Procedure:
 *      wmLeaveNotify
 *
 *  Function:
 *	Handle LeaveNotify events on the client window and/or sub-windows
 *	if WM_COLORMAP_WINDOWS is set.
 *
 ***********************************************************************
 */

void
wmLeaveNotify(
    XLeaveWindowEvent *ev,
    struct wmData *wp
    )
{
#ifdef DEBUG
    PE("wmLeaveNotify");
    if (wmDebug)
    {
	fprintf(dfp, "  mode = ");
	switch (ev->mode)
	{
	    case NotifyNormal: fprintf(dfp, "NotifyNormal\n"); break;
	    case NotifyGrab: fprintf(dfp, "NotifyGrab\n"); break;
	    case NotifyUngrab: fprintf(dfp, "NotifyUngrab\n"); break;
	}
	fprintf(dfp, "  detail = ");
	switch (ev->detail)
	{
	    case NotifyAncestor: fprintf(dfp, "NotifyAncestor\n"); break;
	    case NotifyVirtual: fprintf(dfp, "NotifyVirtual\n"); break;
	    case NotifyInferior: fprintf(dfp, "NotifyInferior\n"); break;
	    case NotifyNonlinear: fprintf(dfp, "NotifyNonlinear\n"); break;
	    case NotifyNonlinearVirtual: fprintf(dfp, "NotifyNonlinearVirtual\n"); break;
	}
	fprintf(dfp, "  time = %d\n", ev->time);
    }
#endif
#ifdef DEBUG
	wmCheck();
#endif
    if (ev->mode != NotifyNormal || ev->detail == NotifyInferior)
	return;

    // if we left the client window, clear the default button (if any)
    if (ev->window == wp->window())
	wp->clear_warp_back();

    if (wmFocusModel == wmFocusModelNormal)
    {
	if ((ev->window == wp->oi_frame()->outside_X_window() && wp->highlight_frame()) ||
	    (ev->window == wp->window() && !wp->highlight_frame()))
	{
	    wmClearFocus(ev, wp);
	}
    }

    if (!wmScr->colormapFocus) {
	// do colormap stuff
	// pop the stack
	if (ev->window == wp->oi_frame()->outside_X_window())
	{
		while (enterStack.get() != NULL) {}
		installColormap(wmScr->cmap);
		wmScr->colormapWindow = NULL;
	}
	else
	{
		enterStack.get();

		Window w = (Window)enterStack.first();
		if (w)
		{
		    XWindowAttributes attr;
		    XGetWindowAttributes(DPY, w, &attr);
		    installColormap(attr.colormap);
		    wmScr->colormapWindow = w;
		}
		else
		{
		    installColormap(wmScr->cmap);
		    wmScr->colormapWindow = NULL;
		}
	}
    }
#ifdef DEBUG
	wmCheck();
#endif
}

/***********************************************************************
 *
 *  Procedure:
 *      wmVirtualEnterNotify
 *
 *  Function:
 *	Handle EnterNotify events on the small virtual windows
 *
 ***********************************************************************
 */

void
wmVirtualEnterNotify(
    XEnterWindowEvent *ev,
    struct wmData *wp
    )
{
#ifdef DEBUG
	wmCheck();
#endif
#ifdef DEBUG
    PE("wmVirtualEnterNotify");
    if (wmDebug)
    {
	fprintf(dfp, "  mode = ");
	switch (ev->mode)
	{
	    case NotifyNormal: fprintf(dfp, "NotifyNormal\n"); break;
	    case NotifyGrab: fprintf(dfp, "NotifyGrab\n"); break;
	    case NotifyUngrab: fprintf(dfp, "NotifyUngrab\n"); break;
	}
	fprintf(dfp, "  detail = ");
	switch (ev->detail)
	{
	    case NotifyAncestor: fprintf(dfp, "NotifyAncestor\n"); break;
	    case NotifyVirtual: fprintf(dfp, "NotifyVirtual\n"); break;
	    case NotifyInferior: fprintf(dfp, "NotifyInferior\n"); break;
	    case NotifyNonlinear: fprintf(dfp, "NotifyNonlinear\n"); break;
	    case NotifyNonlinearVirtual: fprintf(dfp, "NotifyNonlinearVirtual\n"); break;
	}
    }
#endif
    if (ev->detail == NotifyInferior)
	return;

    if (wmFocusModel == wmFocusModelNormal)
    {
	wmSetFocus(ev, wp);
    }
#ifdef DEBUG
	wmCheck();
#endif
}

/***********************************************************************
 *
 *  Procedure:
 *      wmVirtualLeaveNotify
 *
 *  Function:
 *	Handle LeaveNotify events on the small virtual windows
 *
 ***********************************************************************
 */

void
wmVirtualLeaveNotify(
    XLeaveWindowEvent *ev,
    struct wmData *wp
    )
{
#ifdef DEBUG
    PE("wmVirtualLeaveNotify");
    if (wmDebug)
    {
	fprintf(dfp, "  mode = ");
	switch (ev->mode)
	{
	    case NotifyNormal: fprintf(dfp, "NotifyNormal\n"); break;
	    case NotifyGrab: fprintf(dfp, "NotifyGrab\n"); break;
	    case NotifyUngrab: fprintf(dfp, "NotifyUngrab\n"); break;
	}
	fprintf(dfp, "  detail = ");
	switch (ev->detail)
	{
	    case NotifyAncestor: fprintf(dfp, "NotifyAncestor\n"); break;
	    case NotifyVirtual: fprintf(dfp, "NotifyVirtual\n"); break;
	    case NotifyInferior: fprintf(dfp, "NotifyInferior\n"); break;
	    case NotifyNonlinear: fprintf(dfp, "NotifyNonlinear\n"); break;
	    case NotifyNonlinearVirtual: fprintf(dfp, "NotifyNonlinearVirtual\n"); break;
	}
    }
#endif
#ifdef DEBUG
	wmCheck();
#endif
    if (ev->detail == NotifyInferior)
	return;

    if (wmFocusModel == wmFocusModelNormal)
    {
	wmClearFocus(ev, wp);
    }
#ifdef DEBUG
	wmCheck();
#endif
}

/***********************************************************************
 *
 *  Procedure:
 *      wmColormapNotify
 *
 *  Function:
 *	Handle ColormapNotify events.  If the colormap for the currently 
 *	active window changes, install it.
 *
 ***********************************************************************
 */

void
wmColormapNotify(
    XColormapEvent *ev,
    struct wmData *
    )
{
#ifdef DEBUG
    PE("wmColormapNotify");
    if (wmDebug)
    {
	fprintf(dfp, "    window   = 0x%x\n", ev->window);
	fprintf(dfp, "    colormap = 0x%x\n", ev->colormap);
	fprintf(dfp, "    new      = %d\n", ev->c_new);
	fprintf(dfp, "    state    = 0x%x\n", ev->state);
    }
#endif
#ifdef DEBUG
	wmCheck();
#endif

    if (ev->window == wmScr->root && ev->c_new)
    {
	wmScr->cmap = ev->colormap;
	XSetWindowColormap(DPY, wmScr->conp->orphanage()->X_window(), ev->colormap);
	if (wmScr->vdt)
	    XSetWindowColormap(DPY, wmScr->vroot->X_window(), ev->colormap);
#ifdef DEBUG
    if (wmDebug > 1)
	fprintf(dfp, "Root colormap notify = 0x%x, 0x%x\n", wmScr->cmap, wmScr->cmap);
#endif /* DEBUG */
	if (wmScr->colormapWindow == NULL)
	{
	    installColormap(wmScr->cmap);
	}
    }
    else if (ev->window == wmScr->colormapWindow)
    {
	installColormap(ev->colormap);
    }
#ifdef DEBUG
	wmCheck();
#endif
}

/***********************************************************************
 *
 *  Procedure:
 *      wmResetColormaps
 *
 *  Function:
 *	Reset the colormap stack.
 *
 ***********************************************************************
 */

void
wmResetColormaps()
{
	if (!wmScr->colormapFocus) {
		// clean out the colormap stack
		while (enterStack.get() != NULL) {}
		installColormap(wmScr->cmap);
		wmScr->colormapWindow = NULL;
		wmScr->colormapFocus = NULL;
		XSync(DPY, 0);
	}
}

/***********************************************************************
 *
 *  Procedure:
 *      wmSetFocus
 *
 *  Function:
 *	Show that this client has the keyboard focus by highlighting
 *	and objects that have focus colors.
 *
 ***********************************************************************
 */

void
wmSetFocus(
    XEnterWindowEvent *ev,
    wmData *wp
    )
{
    OI_d_tech *oi;
    wmObjectData *odp;
    XEvent event;
    Time time;
    XClientMessageEvent *clientEvent = (XClientMessageEvent *)&event;
    XFocusInEvent *focusEvent = (XFocusInEvent *)&event;

    if (wp == wmFocusWp)
	return;

    // save the last focus window, in case we have to warp back
    wp->set_last_focus(wmFocusWp);

    if (wmFocusWp)
    {
	if (wmFocusModel == wmFocusModelClickToType)
	{
	    if (wp->focus_type() != wmNoInput)
		wmClearFocus(NULL, wmFocusWp);
	}
	else
	    wmClearFocus(NULL, wmFocusWp);
    }

    if (ev)
	time = ev->time;
    else
	time = lastFocusTime;
    lastFocusTime = time;

    int showFocus = True;
    switch (wp->focus_type())
    {
	case wmNoInput:
	    // do nothing
	    showFocus = False;
	    break;
	case wmPassive:
	    if (wmFocusModel == wmFocusModelClickToType) {
		XSetInputFocus(DPY, wp->window(), RevertToPointerRoot, time);
#ifdef DEBUG
	if (wmDebug > 1)
		fprintf(dfp, "XSetInputFocus(0x%x, %d)\n", wp->window(), time);
#endif
	    }
	    else {
		    XSetInputFocus(DPY, PointerRoot, RevertToPointerRoot, time);
#ifdef DEBUG
	if (wmDebug > 1)
		fprintf(dfp, "XSetInputFocus(PointerRoot, %d)\n", wp->window(), time);
#endif
		    focusEvent->type = FocusIn;
		    focusEvent->display = DPY;
		    focusEvent->window = wp->window();
		    focusEvent->mode = NotifyNormal;
		    focusEvent->detail = NotifyPointer;
		    XSendEvent(DPY, wp->window(), False, FocusChangeMask, &event);
	    }
	    break;
	case wmLocallyActive:
	    XSetInputFocus(DPY, wp->window(), RevertToPointerRoot, time);
#ifdef DEBUG
	if (wmDebug > 1)
		fprintf(dfp, "XSetInputFocus(0x%x, %d)\n", wp->window(), time);
#endif
	    setFocusWp = wp;
	    // Fall through and send the WM_TAKE_FOCUS message
	    // break;
	case wmGloballyActive:
	    clientEvent->type = ClientMessage;
	    clientEvent->message_type = WM_PROTOCOLS;
	    clientEvent->format = 32;
	    clientEvent->display = DPY;
	    clientEvent->window = wp->window();
	    clientEvent->data.l[0] = WM_TAKE_FOCUS;
	    clientEvent->data.l[1] = time;
	    XSendEvent(DPY, wp->window(), False, 0, &event);
#ifdef DEBUG
	if (wmDebug > 1)
		fprintf(dfp, "Sending take focus message = 0x%x, time = %d\n", wp->window(), time);
#endif
	    setFocusWp = wp;
	    break;
    }

    if (showFocus)
    {
	// set the color for all focus changing objects
	wmFocusWp = wp;
	int paintFrame = False;
	wmList *list = wp->focus_list_p();
	for (oi = (OI_d_tech *)list->first(); oi != NULL; oi = (OI_d_tech *)list->next())
	{
	    if (oi == wp->oi_frame())
		paintFrame = True;
	    odp = (wmObjectData *)oi->data();
	    if (odp->pixmap)
	    {
		wmSetBackgroundPanel(oi, odp->focusForeground, odp->focusBackground);
		oi->set_bdr_pixel(odp->focusBorder);
	    }
	    else
	    {
		oi->set_pixels(odp->focusBackground, odp->focusForeground, odp->focusBorder);
		if (oi == wp->oi_icon_image())
		    wmMakeIconImage(wp);
	    }
	}
	if (paintFrame)
		wmExposeFrame(NULL, wp);
    }
}

/***********************************************************************
 *
 *  Procedure:
 *      wmClearFocus
 *
 *  Function:
 *	Rest the colors on objects that have focus colors.
 *
 ***********************************************************************
 */

void
wmClearFocus(
    XLeaveWindowEvent *ev,
    wmData *wp
    )
{
    Time time;
    OI_d_tech *oi;
    wmObjectData *odp;

    if (wp)
    {
	int paintFrame = False;

	if (wp->focus_type() == wmPassive) {
		XEvent event;
		XFocusOutEvent *focusEvent = (XFocusOutEvent *)&event;

		focusEvent->type = FocusOut;
		focusEvent->display = DPY;
		focusEvent->window = wp->window();
		focusEvent->mode = NotifyNormal;
		focusEvent->detail = NotifyPointer;
		XSendEvent(DPY, wp->window(), False, FocusChangeMask, &event);
	}

	// set the color for all focus changing objects
	wmList *list = wp->focus_list_p();
	if (list)
	{
	    for (oi = (OI_d_tech *)list->first(); oi != NULL; oi = (OI_d_tech *)list->next())
	    {
		if ((odp = (wmObjectData *)oi->data()))
		{
		    if (oi == wp->oi_frame())
			    paintFrame = True;
		    if (odp->pixmap)
		    {
			wmSetBackgroundPanel(oi, odp->unfocusForeground, odp->unfocusBackground);
			oi->set_bdr_pixel(odp->unfocusBorder);
		    }
		    else
		    {
			oi->set_pixels(odp->unfocusBackground, odp->unfocusForeground, odp->unfocusBorder);
			if (oi == wp->oi_icon_image())
			    wmMakeIconImage(wp);
		    }
		}
	    }
	}
	if (paintFrame)
		wmExposeFrame(NULL, wp);
    }

    if (wmFocusWp == wp)
	wmFocusWp = NULL;

    if (ev)
	time = ev->time;
    else
	time = lastFocusTime;
    lastFocusTime = time;

    if (wmFocusModel != wmFocusModelClickToType)
    {
//	if (setFocusWp == NULL)
//	    return;

//	if (setFocusWp == wp)
//	{
		XSetInputFocus(DPY, PointerRoot, RevertToPointerRoot, time);
		setFocusWp = NULL;
#ifdef DEBUG
	if (wmDebug > 1)
		fprintf(dfp, "XSetInputFocus(PointerRoot, %d)\n", time);
#endif
//	}
    }
    else
    {
	XSetInputFocus(DPY, wmFocusBeeper, RevertToPointerRoot, time);
#ifdef DEBUG
	if (wmDebug > 1)
		fprintf(dfp, "XSetInputFocus(wmFocusBeeper, %d)\n", time);
#endif
    }
}

/***************************************************************
 *
 *  Procedure:
 *	wmExposeFrame
 *
 *  Function:
 *	Handle an expose event on the frame window.  This will 
 *	cause the insideBorderWidth (if any) to be painted.  The
 *	resize corners/bars (if any) will also be painted.
 *
 ***************************************************************/

void
wmExposeFrame(
    XExposeEvent *ev,
    struct wmData *wp
    )
{
#ifdef DEBUG
    PE("wmExposeFrame");
#endif
#ifdef DEBUG
	wmCheck();
#endif

    if (!ev || ev->count == 0)
    {
	XClearWindow(DPY, wp->oi_frame()->X_window());
	if (wp->inside_bw())
	{
	    if (wmScr->conp->model() != OI_OPENLOOK)
	    {
		    wp->oi_frame()->paint_bevel(0,0, wp->oi_frame()->size_x(), wp->oi_frame()->size_y(), OI_NO, 1);
	    }
	    else
	    {
		XSetForeground(DPY, wmScr->cornerGC, wp->oi_frame()->fg_pixel());
		for (int i = 0; i < wp->inside_bw(); i++)
		{
		    XDrawRectangle(DPY, wp->oi_frame()->X_window(), wmScr->cornerGC, 0+i, 0+i,
			(wp->oi_frame()->size_x()-2*i)-1, (wp->oi_frame()->size_y()-2*i)-1);
		}
		XSetForeground(DPY, wmScr->cornerGC, wp->oi_frame()->bkg_pixel());
		XDrawRectangle(DPY, wp->oi_frame()->X_window(), wmScr->cornerGC, 0+i, 0+i,
		    (wp->oi_frame()->size_x()-2*i)-1, (wp->oi_frame()->size_y()-2*i)-1);
	    }
	}

	if (wp->resize_corners())
	{
	    // top left
	    wmDrawCorner(wp, wmResizeCornerTopLeft, 0, 0, 1, 1, 0);

	    // bottom left
	    wmDrawCorner(wp, wmResizeCornerBottomLeft, 0, wp->oi_frame()->size_y()-1, 1, -1, 0);

	    // top right 
	    wmDrawCorner(wp, wmResizeCornerTopRight, wp->oi_frame()->size_x()-1, 0, -1, 1, 0);

	    // bottom right
	    wmDrawCorner(wp, wmResizeCornerBottomRight, wp->oi_frame()->size_x()-1,wp->oi_frame()->size_y()-1, -1, -1, 0);
	}

	if (wp->resize_bars())
	{
	    // top
	    wmDrawHorizontalBar(wp, 0, 0, 1, 0);
	    // bottom
	    wmDrawHorizontalBar(wp, 0, wp->oi_frame()->size_y()-1, -1, 0);
	    // left
	    wmDrawVerticalBar(wp, 0, 0, 1, 0);
	    // right
	    wmDrawVerticalBar(wp, wp->oi_frame()->size_x()-1, 0, -1, 0);
	}
    }
#ifdef DEBUG
	wmCheck();
#endif
}

/***********************************************************************
 *
 *  Procedure:
 *      wmDrawCorner
 *
 *  Function:
 *	Draw a resize corner
 *
 ***********************************************************************
 */

static int topcnt[4] = {2, 3, 3, 4};
static int botcnt[4] = {4, 3, 3, 2};
static int topsegs[4][4] = {
	{ 0, 5, -1, -1 },
	{ 0, 1, 3, -1 },
	{ 2, 4, 5, -1 },
	{ 1, 2, 3, 4 }};
static int botsegs[4][4] = {
	{ 1, 2, 3, 4 },
	{ 2, 4, 5, -1 },
	{ 0, 1, 3, -1 },
	{ 0, 5, -1, -1 }};

void
wmDrawCorner(
    wmData *wp,			// the frame to paint it on
    wmResizeCorner what,
    int origX,			// the X origin to use
    int origY,			// the Y origin to use
    int dirX,			// the X direction
    int dirY,			// the Y direction
    int state			// 0 == not pressed,  1 == pressed
    )
{
    int i;
    XPoint points[6];
    XSegment seg[6];
    XSegment segs[4];
    int length, width;

    if (wmScr->conp->model() != OI_OPENLOOK)
    {
	origX += wp->inside_bw() * dirX;
	origY += wp->inside_bw() * dirY;
	length = wp->resize_length() - wp->inside_bw() - 1;
	width = wp->resize_width() - wp->inside_bw() - 1;
    }
    else
    {
	origX -= dirX;
	origY -= dirY;
	length = wp->resize_length();
	width = wp->resize_width();
    }

    points[0].x = origX;
    points[0].y = origY;
    points[1].x = origX + length*dirX;
    points[1].y = origY;
    points[2].x = origX + length*dirX;
    points[2].y = origY + width*dirY;
    points[3].x = origX + width*dirX;
    points[3].y = origY + width*dirY;
    points[4].x = origX + width*dirX;
    points[4].y = origY + length*dirY;
    points[5].x = origX;
    points[5].y = origY + length*dirY;

    if (wmScr->conp->model() != OI_OPENLOOK)
    {
	seg[0].x1 = points[0].x;
	seg[0].y1 = points[0].y;
	seg[0].x2 = points[1].x;
	seg[0].y2 = points[1].y;
	seg[1].x1 = points[1].x;
	seg[1].y1 = points[1].y;
	seg[1].x2 = points[2].x;
	seg[1].y2 = points[2].y;
	seg[2].x1 = points[2].x;
	seg[2].y1 = points[2].y;
	seg[2].x2 = points[3].x;
	seg[2].y2 = points[3].y;
	seg[3].x1 = points[3].x;
	seg[3].y1 = points[3].y;
	seg[3].x2 = points[4].x;
	seg[3].y2 = points[4].y;
	seg[4].x1 = points[4].x;
	seg[4].y1 = points[4].y;
	seg[4].x2 = points[5].x;
	seg[4].y2 = points[5].y;
	seg[5].x1 = points[5].x;
	seg[5].y1 = points[5].y;
	seg[5].x2 = points[0].x;
	seg[5].y2 = points[0].y;

#ifdef PRINT_DEBUG
	printf("------   DrawCorner %d  -------\n", what);
	printf("  Bottom\n");
#endif
	wp->oi_frame()->set_bvl_gc(OI_3D_BOTTOM, (OI_bool)state);
	for (i = 0; i < botcnt[what]; i++)
	{
		segs[i] = seg[botsegs[what][i]];
#ifdef PRINT_DEBUG
		printf("    segs[%d]   %d, %d    %d, %d\n", i,
			segs[i].x1, segs[i].y1,
			segs[i].x2, segs[i].y2);
#endif
	}
	XDrawSegments(DPY, wp->oi_frame()->X_window(), wmScr->conp->gc(), segs, botcnt[what]);

	wp->oi_frame()->set_bvl_gc(OI_3D_TOP, (OI_bool)state);
#ifdef PRINT_DEBUG
	printf("  Top\n");
#endif
	for (i = 0; i < topcnt[what]; i++)
	{
		segs[i] = seg[topsegs[what][i]];
#ifdef PRINT_DEBUG
		printf("    segs[%d]   %d, %d    %d, %d\n", i,
			segs[i].x1, segs[i].y1,
			segs[i].x2, segs[i].y2);
#endif
	}
	XDrawSegments(DPY, wp->oi_frame()->X_window(), wmScr->conp->gc(), segs, topcnt[what]);
#ifdef PRINT_DEBUG
	printf("------   done  -------\n");
#endif

	wp->oi_frame()->reset_bvl_gc();
    }
    else
    {
	if (state)
	    XSetForeground(DPY, wmScr->cornerGC, wp->oi_frame()->fg_pixel());
	else
	    XSetForeground(DPY, wmScr->cornerGC, wp->resize_bg());
	XFillPolygon(DPY, wp->oi_frame()->X_window(), wmScr->cornerGC,
	    points, 6, Nonconvex, CoordModeOrigin);
	XSetForeground(DPY, wmScr->cornerGC, wp->oi_frame()->fg_pixel());
	XDrawLines(DPY, wp->oi_frame()->X_window(), wmScr->cornerGC,
	    &points[1], 5, CoordModeOrigin);
    }
}

/***********************************************************************
 *
 *  Procedure:
 *      wmDrawHorizontalBar
 *
 *  Function:
 *	Draw a horizontal resize bar.
 *
 ***********************************************************************
 */

void
wmDrawHorizontalBar(
    wmData *wp,			// the frame to paint it on
    int origX,			// the X origin to use
    int origY,			// the Y origin to use
    int dir,			// the direction
    int state			// 0 == not pressed,  1 == pressed
    )
{
    XPoint points[4];
    int height, x, y;

    if (wmScr->conp->model() != OI_OPENLOOK)
    {
	height = wp->resize_width() - wp->inside_bw();
	x = origX + wp->resize_length();
	y = origY + wp->inside_bw() * dir;
	if (origY != 0)
	    y -= height-1;
	wp->oi_frame()->paint_bevel(x,y, wp->oi_frame()->size_x()-2*wp->resize_length(), height, (OI_bool)state, 1);
    }
    else
    {
	origX += wp->resize_length();
	origY -= dir;

	points[0].x = origX;
	points[0].y = origY;
	points[1].x = 0;
	points[1].y = wp->resize_width() * dir;
	points[2].x = wp->oi_frame()->size_x() - (2 * wp->resize_length()) - 1;
	points[2].y = 0;
	points[3].x = 0;
	points[3].y = -(wp->resize_width() * dir);

	if (state)
	    XSetForeground(DPY, wmScr->cornerGC, wp->oi_frame()->fg_pixel());
	else
	    XSetForeground(DPY, wmScr->cornerGC, wp->resize_bg());
	XFillPolygon(DPY, wp->oi_frame()->X_window(), wmScr->cornerGC,
	    points, 4, Nonconvex, CoordModePrevious);
	XSetForeground(DPY, wmScr->cornerGC, wp->oi_frame()->fg_pixel());
	XDrawLines(DPY, wp->oi_frame()->X_window(), wmScr->cornerGC,
	    points, 4, CoordModePrevious);
    }
}

/***********************************************************************
 *
 *  Procedure:
 *      wmDrawVerticalBar
 *
 *  Function:
 *	Draw a vertical resize bar.
 *
 ***********************************************************************
 */

void
wmDrawVerticalBar(
    wmData *wp,			// the frame to paint it on
    int origX,			// the X origin to use
    int origY,			// the Y origin to use
    int dir,			// the direction
    int state			// 0 == not pressed,  1 == pressed
    )
{
    XPoint points[4];
    int width, x, y;

    if (wmScr->conp->model() != OI_OPENLOOK)
    {
	width = wp->resize_width() - wp->inside_bw();
	y = origY + wp->resize_length();
	x = origX + wp->inside_bw() * dir;
	if (origX != 0)
	    x -= width-1;
	wp->oi_frame()->paint_bevel(x,y, width, wp->oi_frame()->size_y()-2*wp->resize_length(), (OI_bool)state, 1);
    }
    else
    {
	origX -= dir;
	origY += wp->resize_length();

	points[0].x = origX;
	points[0].y = origY;
	points[1].x = wp->resize_width() * dir;
	points[1].y = 0;
	points[2].x = 0;
	points[2].y = wp->oi_frame()->size_y() - (2 * wp->resize_length()) - 1;
	points[3].x = -(wp->resize_width() * dir);
	points[3].y = 0;

	if (state)
	    XSetForeground(DPY, wmScr->cornerGC, wp->oi_frame()->fg_pixel());
	else
	    XSetForeground(DPY, wmScr->cornerGC, wp->resize_bg());
	XFillPolygon(DPY, wp->oi_frame()->X_window(), wmScr->cornerGC,
	    points, 4, Nonconvex, CoordModePrevious);
	XSetForeground(DPY, wmScr->cornerGC, wp->oi_frame()->fg_pixel());
	XDrawLines(DPY, wp->oi_frame()->X_window(), wmScr->cornerGC,
	    points, 4, CoordModePrevious);
    }
}

/***********************************************************************
 *
 *  Procedure:
 *      wmFlushExpose
 *
 *  Function:
 *	Flush expose events on a window.
 *
 ***********************************************************************
 */

void
wmFlushExpose(
    Window w
    )
{
    XEvent dummy;

    while (XCheckTypedWindowEvent(DPY, w, Expose, &dummy));
}

static
void installColormap(Colormap colormap)
{
    static Colormap lastColormap = None;

    if (colormap != lastColormap)
    {
	lastColormap = colormap;
	XInstallColormap(DPY, colormap);
#ifdef DEBUG
	if (wmDebug)
	{
	    fprintf(dfp, "installColormap\n");
	    fprintf(dfp, "  installing colormap 0x%x\n", colormap);
	}
#endif /* DEBUG */
    }
}

#ifdef SHAPE
/***********************************************************************
 *
 *  Procedure:
 *      wmShapeNotify
 *
 *  Function:
 *	Handle a ShapeNotify event and reshape the frame
 *
 ***********************************************************************
 */

void
wmShapeNotify(XShapeEvent *, struct wmData *wp)
{
    wp->op()->oi = wp->oi_frame();
    wmLayoutPanel(wp->op());
}
#endif /* SHAPE */

#ifdef DEBUG
void
wmCheck()
{
#ifdef FOO
	// if there is no Swm*decoration resource, then chances are good that something is trashed out
	if ((ptr = RM->get_resourceq(wmQuarks->decorationName(), wmQuarks->decorationClass())) == NULL)
	{
	    abort();
	}
#endif
}
#endif /* DEBUG */
