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
 * $Id: reparent.C,v 9.33 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Reparent client window routines
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/

#ifndef lint
static char RCSinfo[] =
"$Id: reparent.C,v 9.33 1993/08/27 16:58:15 toml Exp $";
#endif /* lint */

#include "swm.H"
#include "debug.H"
#include "gram.H"
#include "init.H"
#include "object.H"
#include "main.H"
#include "util.H"
#include "panel.H"
#include "wmdata.H"
#include "execute.H"
#include "events.H"
#include "resize.H"
#include "icons.H"
#include "move.H"
#include "reparent.H"
#include "pan.H"
#include "atoms.H"
#include "ol.H"
#include "swmstart.H"
#include "quarks.H"
#include "region.H"

wmData *wmDummy;
IconRegion *findIconRegion(char *);

#ifdef MEMLEAK
extern int mfd, ffd, nfd, dfd, vnfd, vdfd;
#endif

char *wmNoName = "None";
static XClassHint noClass;

// these are the dispatch table entries for events on the client window 
// that I want to know about.
static OI_dispatch_entry clientEvents[] = {
{DestroyNotify,		StructureNotifyMask,	(OI_event_fnp)wmDestroyNotify,		NULL, NULL_PMF, NULL },
{UnmapNotify,		StructureNotifyMask,	(OI_event_fnp)wmUnmapNotify,		NULL, NULL_PMF, NULL },
{PropertyNotify,	PropertyChangeMask,	(OI_event_fnp)wmPropertyNotify,		NULL, NULL_PMF, NULL },
{ClientMessage,		0,			(OI_event_fnp)wmClientMessage,		NULL, NULL_PMF, NULL },
{EnterNotify,		EnterWindowMask,	(OI_event_fnp)wmEnterNotify,		NULL, NULL_PMF, NULL },
{LeaveNotify,		LeaveWindowMask,	(OI_event_fnp)wmLeaveNotify,		NULL, NULL_PMF, NULL },
{ColormapNotify,	ColormapChangeMask,	(OI_event_fnp)wmColormapNotify,		NULL, NULL_PMF, NULL },
};

#define CLIENT_EVENTS (sizeof(clientEvents)/sizeof(OI_dispatch_entry))

static OI_dispatch_entry colormapEvents[] = {
{EnterNotify,		EnterWindowMask,	(OI_event_fnp)wmEnterNotify,		NULL, NULL_PMF, NULL },
{LeaveNotify,		LeaveWindowMask,	(OI_event_fnp)wmLeaveNotify,		NULL, NULL_PMF, NULL },
{ColormapNotify,	ColormapChangeMask,	(OI_event_fnp)wmColormapNotify,		NULL, NULL_PMF, NULL },
};

#define COLORMAP_EVENTS (sizeof(colormapEvents)/sizeof(OI_dispatch_entry))

#define SUB SubstructureRedirectMask

// dispatch entries for the window that will reparent the client
static OI_dispatch_entry clientParentEvents[] = {
{MapRequest,		SUB,			(OI_event_fnp)wmMapRequest,		NULL, NULL_PMF, NULL },
{ConfigureRequest,	SUB,			(OI_event_fnp)wmConfigureRequest,	NULL, NULL_PMF, NULL },
{ButtonPress,		0,			(OI_event_fnp)wmHandlePress,		NULL, NULL_PMF, NULL },
};

#define CLIENT_PARENT_EVENTS (sizeof(clientParentEvents)/sizeof(OI_dispatch_entry))

// dispatch entries for frame
static OI_dispatch_entry frameEvents[] = {
{VisibilityNotify,	VisibilityChangeMask,	(OI_event_fnp)wmFrameVisibilityNotify,	NULL, NULL_PMF, NULL },
{Expose,		ExposureMask,		(OI_event_fnp)wmExposeFrame,		NULL, NULL_PMF, NULL },
{EnterNotify,		EnterWindowMask,	(OI_event_fnp)wmEnterNotify,		NULL, NULL_PMF, NULL },
{LeaveNotify,		LeaveWindowMask,	(OI_event_fnp)wmLeaveNotify,		NULL, NULL_PMF, NULL },
};

#define FRAME_EVENTS (sizeof(frameEvents)/sizeof(OI_dispatch_entry))

// dispatch entries for the small virtual windows
static OI_dispatch_entry virtualEvents[] = {
{ButtonPress,		ButtonPressMask,	(OI_event_fnp)wmMoveInPanner,		NULL, NULL_PMF, NULL },
{KeyPress,		KeyPressMask,		(OI_event_fnp)wmHandleKeyPress,		NULL, NULL_PMF, NULL },
{EnterNotify,		EnterWindowMask,	(OI_event_fnp)wmVirtualEnterNotify,	NULL, NULL_PMF, NULL },
{LeaveNotify,		LeaveWindowMask,	(OI_event_fnp)wmVirtualLeaveNotify,	NULL, NULL_PMF, NULL },
{Expose,		ExposureMask,		(OI_event_fnp)wmPaintVirtual,		NULL, NULL_PMF, NULL },
};

#define VIRTUAL_EVENTS (sizeof(virtualEvents)/sizeof(OI_dispatch_entry))

static int wmMappedNotOverride(Window);
static void grabKeys(wmData *, wmBindings *);
static int panner;


wmData *
wmReparent(
    Window w,			// the window to reparent
    Window parent,
    OI_d_tech *root
    )
{
    int pushCnt;
    Window junkRoot, junkChild;
    int junkX, junkY;
    unsigned junkWidth, junkHeight;
    unsigned int junkMask;
    unsigned int junkBW, junkDepth;
    wmObject *tmpop;
    XSizeHints *size_hints;
    wmData *wp;
    char *ptr;
    wmObject *op;
    XWindowAttributes *attr;
    XSetWindowAttributes attributes;
    wmObjectData *odp;
    long supplied;
    int interactivePlace = wmInitDone;
    panner = False;
    OI_d_tech *tmpoi;
    SWMHints *myhints;
    XrmQuark classQuark, nameQuark;
    int i;
    IconRegion *irp = NULL;

#ifdef DEBUG
#ifdef MEMLEAK
    if (mfd)
    {
	char *s;
	int len;
	int frees = True;
	char *p="wmReparent: name=";
	XFetchName(DPY, w, &s);
	len = strlen(p);
	write(mfd, p, len);
	write(ffd, p, len);
	write(nfd, p, len);
	write(dfd, p, len);
	write(vnfd, p, len);
	write(vdfd, p, len);
	if (!s) {
	    s = "none";
	    frees = False;
	}
	len = strlen(s);
	write(mfd, s, len);
	write(ffd, s, len);
	write(mfd, "\n", 1);
	write(ffd, "\n", 1);
	write(nfd, s, len);
	write(dfd, s, len);
	write(nfd, "\n", 1);
	write(dfd, "\n", 1);
	write(vnfd, s, len);
	write(vdfd, s, len);
	write(vnfd, "\n", 1);
	write(vdfd, "\n", 1);
	if (frees)
	    XFree(s);
    }
#endif /* MEMLEAK */
#endif /* DEBUG */

#ifdef DEBUG
    if (wmDebug)
	fprintf(dfp, "wmReparent: w = 0x%x\n", w);
#endif

#ifdef DEBUG
    if (!OI_debug)
#endif /* DEBUG */
	wmGrabServer();

    /*
     * Make sure the client window still exists.  We don't want to leave an
     * orphan frame window if it doesn't.  Since we now have the server
     * grabbed, the window can't disappear later without having been
     * reparented, so we'll get a DestroyNotify for it.  We won't have
     * gotten one for anything up to here, however.
     */
    if (XGetGeometry(DPY, w, &junkRoot, &junkX, &junkY, &junkWidth, &junkHeight, &junkBW, &junkDepth) == 0)
    {
        wmUngrabServer();
        return(NULL);
    }

    if (wmScr->vdt && w == wmScr->pan->X_window())
	panner = True;

    // don't propagate button press events through
    attributes.do_not_propagate_mask = ButtonPressMask | ButtonReleaseMask;
    attributes.win_gravity = NorthWestGravity;
    XChangeWindowAttributes(DPY, w, CWDontPropagate|CWWinGravity, &attributes);

    // allocate a new data structure
    wp = new wmData();
    wp->new_client();
    wp->new_icon();

    if (wmConn->is_my_window(w))
	wp->set_mine();

    wmClient = wp;
    wp->set_window(w);
    wp->set_screen(wmScr);
    size_hints = wp->size_hints_p();
    attr = wp->attr_p();
    XGetWindowAttributes(DPY, wp->window(), attr);
    myhints = wp->myhints_p();

    if (!wmInitDone)
	wp->set_positioned();
    if (wmScr->vdt)
        if (parent == wmScr->vroot->X_window())
            wp->set_positioned();

    wp->set_class(noClass);
    XGetClassHint(DPY, wp->window(), wp->wclass_p());
    if (!wp->wclass_name()) wp->set_class_name(wmNoName);
    if (!wp->wclass_class()) wp->set_class_class(wmNoName);

    // build the base resource string
    RM->set_stack_ptr(wmScr->rm_stack);
    classQuark = XrmStringToQuark(wp->wclass_class());
    nameQuark = XrmStringToQuark(wp->wclass_name());
    RM->pushq(classQuark, classQuark);
    RM->pushq(nameQuark, nameQuark);

#ifdef SHAPE
    if (wmHasShape)
    {
        int xws, yws, xbs, ybs;
        unsigned wws, hws, wbs, hbs;
        int boundingShaped, clipShaped;

        XShapeQueryExtents (DPY, wp->window(), &boundingShaped, &xws, &yws, &wws, &hws, &clipShaped, &xbs, &ybs, &wbs, &hbs);
	if (boundingShaped)
		wp->set_shaped();
	if (wp->shaped())
	    RM->pushq(wmQuarks->shapedName(), wmQuarks->shapedClass());
    }
#endif /* SHAPE */

    // see if there are any Open Look or Motif properties
    wmGetOpenLookAtoms(wp);
    wmGetMotifAtoms(wp);

    if (wp->ol_default_btn())
    {
	// save the pointer location
	XQueryPointer(DPY, wmScr->root, &junkRoot, &junkChild, wp->root_x_p(), wp->root_y_p(), &junkX, &junkY, &junkMask);
    }

    if (!wp->mine() && XGetTransientForHint(DPY, w, wp->transient_for_p()))
    {
	wp->set_transient();

	// save the structure for which the thing is transient
	XFindContext(DPY, wp->transient_for(), wmContext, (caddr_t*)wp->transient_for_wp_p());
	if (wp->transient_for_wp())
	    wp->transient_for_wp()->transient_list_p()->append((ent)wp);
    }

    wmGetColormapWindows(wp);
    XGetWMProtocols(DPY, w, wp->protocols_p(), wp->protocols_count_p());
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

    wp->set_wmhints(XGetWMHints(DPY, wp->window()));
    if (!wp->wmhints())
    {
	wp->set_wmhints((XWMHints *)malloc(sizeof(XWMHints)));
	wp->wmhints()->flags = StateHint;
	wp->wmhints()->initial_state = NormalState;
	wp->set_free_wm_hints();
    }

    // if the transient for window is iconic, make this one iconic also
    if (wp->transient_for_wp() && wp->transient_for_wp()->state() == IconicState) {
	wp->wmhints()->flags |= StateHint;
	wp->wmhints()->initial_state = IconicState;
    }

    if ((wp->wmhints()->flags & InputHint) && wp->wmhints()->input)
	wp->set_focus_type(wmInputSpecified);

    // if it is a transient, they may want special decoration
    if (wp->transient())
	RM->pushq(wmQuarks->transientName(), wmQuarks->transientClass());

    if ((wp->wmhints()->flags & WindowGroupHint) && !wp->mine())
    {
	wp->set_group((int)wp->wmhints()->window_group);
	wp->set_regroup((int)wp->wmhints()->window_group);
    }

    // Search the resource data base to make individual lookups faster.  This allows us
    // to use the faster RM->get_search_resouce() member function.
    RM->search_resources();

    if ((ptr = RM->get_search_resourceq(wmQuarks->ignoreGroupHintsName(), wmQuarks->ignoreGroupHintsClass())) && (ptr[0] == 't' || ptr[0] == 'T'))
    {
	wp->set_ignore_group_hints();
	wp->set_group(NULL);
	wp->set_regroup(NULL);
    }

    if (XGetWMNormalHints(DPY, w, size_hints, &supplied) == 0)
	size_hints->flags = 0;

    // get the window and icon names
    XFetchName(DPY, w, wp->name_p());
    if (!wp->name()) wp->set_name(wmNoName);

#ifdef DEBUG
    if (wmDebug > 1)
    {
	fprintf(dfp, "%s input == ", wp->name());
	switch (wp->focus_type())
	{
	    case wmNoInput:
		fprintf(dfp, "  No Input\n");
		break;
	    case wmPassive:
		fprintf(dfp, "  Passive\n");
		break;
	    case wmLocallyActive:
		fprintf(dfp, "  Locally Active\n");
		break;
	    case wmGloballyActive:
		fprintf(dfp, "  Globally Active\n");
		break;
	}
    }
#endif /* DEBUG */

    Atom actual;
    int junk1;
    unsigned long junk2, len;
    if (XGetWindowProperty(DPY, w, XA_WM_ICON_NAME, 0, 200, False, XA_STRING, &actual, &junk1, &junk2, &len, (unsigned char **)wp->icon_name_p()))
	wp->set_icon_name(wp->name());

    if (!wp->icon_name()) wp->set_icon_name(wp->name());

#ifdef DEBUG
    if (wmDebug > 1)
	fprintf(dfp, "  name = %s\n", wp->name());
#endif


    // try to get the previous state
    if (wmScr->restartPreviousState && wmGetWM_STATE(wp))
    {
	wp->wmhints()->flags |= StateHint;
	wp->wmhints()->initial_state = wp->initial_state();
	if (wp->iconified()) {
	    wp->wmhints()->flags |= IconPositionHint;
	    wp->wmhints()->icon_x = wp->icon_x();
	    wp->wmhints()->icon_y = wp->icon_y();

	    if (wp->icon_window()) 
	    	wp->set_root_icon();
	}
    }

    // if we haven't got all the inital clients, lets see if he is one
    if (wmScr->startList.count)
    {
	char *wmcmd = wmGetWM_COMMAND(wp);
	if (wmcmd)
	{
	    char *clientMachine = NULL;
	    unsigned long  nitems, bytesafter;
	    Atom actual_type;
	    int actual_format;

	    XGetWindowProperty(DPY, wp->window(), XA_WM_CLIENT_MACHINE, 0L, 1000000L, False,
		XA_STRING, &actual_type, &actual_format, &nitems, &bytesafter, (unsigned char **)&clientMachine);

	    for (SWMStart *sp = (SWMStart *)wmScr->startList.first(); sp != NULL; sp = (SWMStart *)wmScr->startList.next())
	    {
		// do we need to check the WM_CLIENT_MACHINE property?
		if (sp->machine_index != -1)
		{
		    if (clientMachine && strcmp(clientMachine, &(sp->ptr[sp->machine_index])))
			continue;
		}

		if (!strcmp(&(sp->ptr[sp->cmd_index]), wmcmd))
		{
		    // get rid of the window gravity
		    size_hints->flags &= ~PWinGravity;

		    interactivePlace = False;
		    wp->set_placed();
		    wp->set_positioned();
		    attr->x = sp->geomX;
		    attr->y = sp->geomY;
		    // convert from the unit size geometry to pixel width and height
		    if (size_hints->flags & PResizeInc)
		    {
			sp->geomWidth *= size_hints->width_inc;
			sp->geomHeight *= size_hints->height_inc;
		    }

		    if (size_hints->flags&(PMinSize|PBaseSize) &&
			size_hints->flags & PResizeInc)
		    {
			if (size_hints->flags & PBaseSize)
			{
			    sp->geomWidth += size_hints->base_width;
			    sp->geomHeight += size_hints->base_height;
			}
			else
			{
			    sp->geomWidth += size_hints->min_width;
			    sp->geomHeight += size_hints->min_height;
			}
		    }
		    attr->width = sp->geomWidth;
		    attr->height = sp->geomHeight;
		    if (wmScr->pan && wmScr->pan->X_window() == wp->window())
		    {
			wmScr->vwidth = attr->width;
			wmScr->vheight = attr->height;
			wmScr->vdt->set_object_size(wmScr->vwidth, wmScr->vheight);
			wmScr->pan->set_span(wmScr->vwidth, wmScr->vheight);
			attr->width /= wmScr->vscale;
			attr->height /= wmScr->vscale;
			wmScr->pan->set_size(attr->width, attr->height);
		    }
		    else
		    {
			XResizeWindow(DPY, wp->window(), attr->width, attr->height);
		    }
		    wp->wmhints()->flags |= StateHint;
		    wp->wmhints()->initial_state = sp->state;
		    if (sp->iconified) {
			wp->wmhints()->flags |= IconPositionHint;
			wp->wmhints()->icon_x = sp->igeomX;
			wp->wmhints()->icon_y = sp->igeomY;
		    }
		    if (sp->sticky)
			wp->set_sticky();
		    if (sp->rootIcon)
			wp->set_root_icon();
		    if (sp->iconGravity)
		    {
			wp->set_icon_gravity();
			wp->set_gravity_order(sp->gravityOrder);
			// make sure myhints are set 
			wp->set_icon_x(sp->igeomX);
			wp->set_icon_y(sp->igeomY);
		    }

		    // remove it from the list
		    wmScr->startList.rm((ent)sp);
		    break;
		}
	    }
	}
    }

    // Figure out whether the window is sticky or not,  if the
    // window is a transient, the sticky state follows the 
    // transient_for window
    wp->set_root(root);
    if (wmScr->vdt)
    {
	// was the window sticky before the restart?
	if (wp->sticky())
	    wp->set_root(wmScr->conp->abs_root());
	else
	{
	    if ((ptr = RM->get_search_resourceq(wmQuarks->stickyName(), wmQuarks->stickyClass())))
	    {
		if (ptr[0] == 'T' || ptr[0] == 't')
		    wp->set_root(wmScr->conp->abs_root());
		else if (ptr[0] == 'F' || ptr[0] == 'f')
		    wp->set_root(wmScr->vroot);
	    }
	}

	// is this a transient?
	if (wp->transient_for_wp())
	{
		if (wp->transient_for_wp()->sticky())
		    wp->set_root(wmScr->conp->abs_root());
		else
		    wp->set_root(wmScr->vroot);
	}

	if (wp->root() == wmScr->conp->abs_root())
	    wp->set_sticky();
	else
	    wp->clear_sticky();


	// if this is a menu and is not sticky,  I need to translate the coordinates
	// to the virtual root
	if (wmConn->is_my_window(w) && !wp->sticky())
	{
#ifdef OLD
	// this code was in here before OI 3.3
	    int oldx, oldy;
	    oldx = attr->x;
	    oldy = attr->y;
	    XTranslateCoordinates(DPY, wmScr->root, wp->root()->X_window(), oldx, oldy, &attr->x, &attr->y, &junkChild);
#endif
	    wp->set_positioned();
	}
    }

    // if this window is a group member, check to see if other group members are
    // the same sticky state
    if (wp->group()) {
	wmData *tmpwp;

	for (tmpwp = (wmData *)wmScr->windowList.first(); tmpwp != NULL; tmpwp = (wmData *)wmScr->windowList.next()) {
	    if (wp->group() == tmpwp->group()) {
		if (tmpwp->sticky()) {
			wp->set_sticky();
			wp->set_root(wmScr->conp->abs_root());
		}
		else {
			wp->clear_sticky();
			wp->set_root(wmScr->vroot);
		}
	    }
	}
    }
    if (wp->sticky()) {
	RM->pushq(wmQuarks->stickyName(), wmQuarks->stickyClass());
	RM->search_resources();
    }

    // find out what kind of decoration to add
    ptr = RM->get_search_resourceq(wmQuarks->decorationName(), wmQuarks->decorationClass());

    if (ptr == NULL)
    {
	fprintf(stderr, "swm: wmReparent: no decoration found for \"%s\"\n",
	    wp->name());
	XMapWindow(DPY, w);
	wmDone();
    }
    op = wmCreateObject(OBJ_PANEL, ptr);
    if (!op->expanded)
	wmExpandObjects();

    // make a panel
    wmInstantiateObject(op);

    // research the database
    RM->search_resources();

    // make sure there was a client panel in this sucker
    wp->set_oi_client(op->oi->descendant("client"));
    if (!wp->oi_client())
    {
	fprintf(stderr, "swm: wmReparent: no \"client\" object in \"%s\"\n", op->name);

	// need to destroy the object we just instantiated
	op->oi->del();

	// let's default to the "none" panel,  I hope it's there
	op = wmCreateObject(OBJ_PANEL, "none");
	if (!op->expanded)
	    wmExpandObjects();

	// create the objects
	wmInstantiateObject(op);
	wp->set_oi_client(op->oi->descendant("client"));

	// if there is no client in the none decoration, punt
	if (!wp->oi_client())
	{
	    fprintf(stderr, "swm: wmReparent: no \"client\" object in \"none\"\n");
	    XMapWindow(DPY, w);
	    wmDone();
	}
    }
    XLowerWindow(DPY, wp->oi_client()->X_window());
    if (!wp->shaped())
	XSetWindowBackgroundPixmap(DPY, wp->oi_client()->X_window(), None);

    if (panner)
	OI_dispatch_insert(wp->window(), KeyPress, KeyPressMask, (OI_event_fnp)wmHandleKeyPress, (char *)wp->oi_client());

    wp->set_oi_frame(op->oi);
    odp = (wmObjectData *)wp->oi_frame()->data();
    odp->frame = True;
    wp->set_op(op);

    // set up callbacks for events
    // I need to sync the server and pull off any existing UnmapNotify
    // events 
    XEvent dummy;
    XSync(DPY, 0);
    while (XCheckTypedWindowEvent(DPY, w, UnmapNotify, &dummy));
    for (i = 0; i < CLIENT_EVENTS; i++)
	clientEvents[i].argp = (char *)wp;
#ifdef SHAPE
    if (wp->shaped())
    {
	OI_dispatch_insert(wp->window(), wmShapeEventBase+ShapeNotify, 0, (OI_event_fnp)wmShapeNotify, (void *)wp);
	XShapeSelectInput(DPY, wp->window(), ShapeNotifyMask);
    }
#endif /* SHAPE */
    wmScr->conp->dispatch_group_insert(w, CLIENT_EVENTS, &clientEvents[0]);

    for (i = 0; i < CLIENT_PARENT_EVENTS; i++)
    {
	if (clientParentEvents[i].event == ButtonPress)
	    clientParentEvents[i].argp = (char *)wp->oi_client();
	else
	    clientParentEvents[i].argp = (char *)wp;
    }
    wmScr->conp->dispatch_group_insert(wp->oi_client()->X_window(),
	CLIENT_PARENT_EVENTS, &clientParentEvents[0]);

    for (i = 0; i < FRAME_EVENTS; i++)
	frameEvents[i].argp = (char *)wp;
    wmScr->conp->dispatch_group_insert(wp->oi_frame()->outside_X_window(),
	FRAME_EVENTS, &frameEvents[0]);

    wp->oi_client()->set_size(attr->width + 2*attr->border_width,
	attr->height + 2 * attr->border_width);

    if (XFindContext(DPY, w, wmMine, (caddr_t*)&tmpop) == XCNOENT)
    {
	XAddToSaveSet(DPY, w);
	XReparentWindow(DPY, w, wp->oi_client()->X_window(), 0, 0);
    }
    else
    {
	// is it an icon panel?
	if (tmpop && tmpop->u.p->icon)
	{
	    tmpop->u.p->icon->wp = wp;
	    wp->set_myip(tmpop->u.p->icon);
	}
        if (XFindContext(DPY, w, wmRootPanelsContext, (caddr_t*)&tmpoi) == XCNOENT)
            XReparentWindow(DPY, w, wp->oi_client()->X_window(), 0, 0);
        else {
            tmpoi->set_associated_object(wp->oi_client(), 0, 0, OI_NOT_DISPLAYED);
            tmpoi->allow_clip();
            tmpoi->set_state(OI_ACTIVE);
	}
    }
    /**********************************************************************
     * I'm going to map the client window here.  The client will get a 
     * MapNotify event meaning that it is going to NormalState.  This is 
     * so if I really want to go to iconic, the resulting iconify operation
     * will indeed generate an UnmapNotify event.
     **********************************************************************/
    XMapWindow(DPY, w);

    /**********************************************************************
     * Get some resources
     **********************************************************************/
    // should we ignore PPosition?
    if ((size_hints->flags & PPosition))
    {
	ptr = RM->get_search_resourceq(wmQuarks->ignorePPositionName(), wmQuarks->ignorePPositionClass());
	if (ptr && (ptr[0] == 't' || ptr[0] == 'T'))
	    size_hints->flags &= ~PPosition;
	else 
	{
	    ptr = RM->get_search_resourceq(wmQuarks->ignorePPositionOriginName(), wmQuarks->ignorePPositionOriginClass());
	    if (ptr && (ptr[0] == 't' || ptr[0] == 'T'))
	    {
		if (attr->x == 0 && attr->y == 0)
		    size_hints->flags &= ~PPosition;
	    }
	}
    }

    RM->pushq(wmQuarks->panelName(), wmQuarks->panelClass());
    for (i = 0, pushCnt = 0; i <= op->numQuarks; i++, pushCnt++)
	RM->pushq(op->quarks[i], op->quarks[i]);
    RM->search_resources();

    // should we try to keep the window on the screen?
    ptr = RM->get_search_resourceq(wmQuarks->constrainName(), wmQuarks->constrainClass());
    if (ptr && (ptr[0] == 't' || ptr[0] == 'T'))
    {
	wp->set_constrain();
	wp->set_save_constrain();
    }

    // is there additional border stuff to paint?
    ptr = RM->get_search_resourceq(wmQuarks->insideBorderWidthName(), wmQuarks->insideBorderWidthClass());
    if (ptr)
    {
	wp->set_inside_bw(atoi(ptr));
	wp->set_inside_bw_saved(wp->inside_bw());
	if (op->u.p->pad < wp->inside_bw())
	    op->u.p->pad = wp->inside_bw();
    }

    // should we highlight the frame ?
    ptr = RM->get_search_resourceq(wmQuarks->highlightFrameName(), wmQuarks->highlightFrameClass());
    if (ptr && (ptr[0] == 't' || ptr[0] == 'T'))
	wp->set_highlight_frame();

    // if the focusModel is Normal,  highlightFrame is automatically turned on
    if (wmFocusModel == wmFocusModelNormal)
    {
	switch (wp->focus_type())
	{
	    case wmLocallyActive:
	    case wmGloballyActive:
		wp->set_highlight_frame();
		break;
	}
    }
    if (wmFocusModel == wmFocusModelClickToType)
    {
	XGrabButton(DPY, Button1, 0, wp->oi_frame()->outside_X_window(), True,
	    ButtonPressMask | ButtonReleaseMask,
	    GrabModeSync, GrabModeAsync, None, (Cursor)None);
	OI_dispatch_insert(wp->oi_frame()->outside_X_window(), ButtonPress, 0, (OI_event_fnp)wmHandlePress, (char *)wp->oi_frame());
    }

    // do we need resize corners ?
    ptr = RM->get_search_resourceq(wmQuarks->resizeCornersName(), wmQuarks->resizeCornersClass());
    if (ptr && (ptr[0] == 't' || ptr[0] == 'T'))
	wp->set_resize_corners();

    // do we need resize bars ?
    ptr = RM->get_search_resourceq(wmQuarks->resizeBarsName(), wmQuarks->resizeBarsClass());
    if (ptr && (ptr[0] == 't' || ptr[0] == 'T'))
	wp->set_resize_bars();

    if (wp->resize_corners() || wp->resize_bars())
    {
	// how big do we make them ?
	ptr = RM->get_search_resourceq(wmQuarks->resizeWidthName(), wmQuarks->resizeWidthClass());
	if (ptr) wp->set_resize_width(atoi(ptr));

	ptr = RM->get_search_resourceq(wmQuarks->resizeLengthName(), wmQuarks->resizeLengthClass());
	if (ptr) wp->set_resize_length(atoi(ptr));
	else wp->set_resize_length(wp->resize_width() * 2);

	wp->set_resize_bg((unsigned)wp->oi_frame()->bkg_pixel());
	ptr = RM->get_search_resourceq(wmQuarks->resizeBackgroundName(), wmQuarks->resizeBackgroundClass());
	if (ptr)
	    wmGetColor(ptr, wp->resize_bg_p());

	if (op->u.p->pad < wp->resize_width())
	    op->u.p->pad = wp->resize_width();

 	if (!odp->handlePress)
	{
	    odp->handlePress = True;
	    OI_dispatch_insert(wp->oi_frame()->outside_X_window(), ButtonPress, ButtonPressMask, (OI_event_fnp)wmHandlePress, (char *)wp->oi_frame());
	    // the frame has already been put on the discard list
	}
    }
    wp->set_pad(op->u.p->pad);
    odp->pad = op->u.p->pad;

    for (i = 0; i < pushCnt; i++)
	RM->pop();
    RM->search_resources();

    wmLayoutPanel(op);

    int redo = False;
    // find the name object (if any)
    wp->set_oi_name(wp->oi_frame()->descendant("name"));
    wmDisplayName(wp);
    if (wp->oi_name())
    {
	XLowerWindow(DPY, wp->oi_name()->outside_X_window());
	odp = (wmObjectData *)wp->oi_name()->data();
	if (!(odp->state & wmNoSpace))
	   redo = True;
    }

    // see if there is a size object lurking about
    wp->set_oi_size((OI_static_text *)wp->oi_frame()->descendant("size"));
    if (!wp->oi_size() && wp->myip())
	wp->set_oi_size((OI_static_text *)wp->myip()->op->oi->descendant("size"));
    wmUpdateSize(wp);
    if (wp->oi_size())
    {
	odp = (wmObjectData *)wp->oi_size()->data();
	if (!(odp->state & wmNoSpace))
	   redo = True;
    }

    // I hate to do this, but I need to re-layout the panel if there is
    // a size object and/or a name object and it takes up space
    if (redo)
	wmLayoutPanel(op);


    if (interactivePlace)
    {
	if (size_hints->flags & USPosition)
	{
	    interactivePlace = False;
	    wp->set_positioned();
	}
	else if (size_hints->flags & PPosition)
	{
	    // Translate the coordinates to the virtual root
	    interactivePlace = False;
	    wp->set_positioned();
	    if (parent != wmScr->vroot->X_window() && !wp->mine())
	    {
		int oldx, oldy;
		oldx = attr->x;
		oldy = attr->y;
		XTranslateCoordinates(DPY, wmScr->root, wp->root()->X_window(),
		    oldx, oldy, &attr->x, &attr->y, &junkChild);
	    }
	}
	else if (wp->transient())
	    interactivePlace = False;
	else if ((wp->wmhints()->flags & StateHint) &&
	    (wp->wmhints()->initial_state == IconicState))
	    interactivePlace = False;
	else if ((ptr = RM->get_search_resourceq(wmQuarks->randomPlacementName(), wmQuarks->randomPlacementClass())))
	{
	    if (ptr[0] == 't' || ptr[0] == 'T')
	    {
		interactivePlace = False;
		if ((wmScr->randX + wp->oi_frame()->space_x()) > wmScr->width)
		    wmScr->randX = wmScr->randStartingX;
		if ((wmScr->randY + wp->oi_frame()->space_y()) > (wmScr->height-wmScr->randStartingY))
		    wmScr->randY = wmScr->randStartingY;

		// translate the coordinates to the virtual root so the window is
		// visible
		XTranslateCoordinates(DPY, wmScr->root, wp->root()->X_window(),
		    wmScr->randX, wmScr->randY, &attr->x, &attr->y, &junkChild);

		wmScr->randX += wmScr->randIncX;
		wmScr->randY += wmScr->randIncY;
		wp->set_positioned();
	    }
	}
    }
    
    if (interactivePlace)
    {
	int x_root;
	int y_root;
	int cancel;
	int saveDelta = wmScr->moveDelta;

	// set the delta to zero so the outline will appear
	wmScr->moveDelta = 0;

	// make sure all pointer buttons are up
        while (True)
	{
	    wmUngrabServer();
	    XSync(DPY, 0);
#ifdef DEBUG
	    if (!OI_debug)
#endif /* DEBUG */
		wmGrabServer();

	    // figure out where the pointer is
	    XQueryPointer(DPY, wmScr->root, &junkRoot, &junkChild,
		&x_root, &y_root, &junkX, &junkY, &junkMask);

	    // wait for buttons up
	    if ((junkMask & B_MASK) != 0)
		continue;

	     // this will cause a warp to the indicated root
	    int stat = wmConn->grab_pointer(wmScr->root, False,
		ButtonPressMask | ButtonReleaseMask,
		GrabModeAsync, GrabModeAsync,
		wmScr->root, wmScr->moveCursor, CurrentTime);

	    if (stat == GrabSuccess)
		break;
	}

	// show the name of the thing we are about to position
	char *n = (char *)malloc(strlen(wp->name())+5);
	sprintf(n, " %s ", wp->name());
	wmScr->nameOI->set_text(n);
	free(n);
	int y = (int)wmScr->nameOI->loc_y();
	int x = (wmScr->width - wmScr->nameOI->size_x()) / 2;
	wmScr->nameOI->set_loc(x, y);
	XRaiseWindow(DPY, wmScr->nameOI->outside_X_window());
	wmScr->nameOI->set_state(OI_ACTIVE);

	// put the frame there
	wp->oi_frame()->set_loc(x_root, y_root);

	// go do the move
	wmConn->ungrab_pointer(CurrentTime);
	wmStartMove(wp, wp->oi_frame(), &x_root, &y_root, &cancel, True);
	attr->x = x_root;
	attr->y = y_root;
	if (cancel == Button2)
	{
	    XButtonEvent ev;
	    wp->oi_frame()->set_associated_object(wp->root(), attr->x, attr->y,
		OI_ACTIVE_NOT_DISPLAYED);
	    wmResizeFlags = RESIZE_BOTTOM | RESIZE_RIGHT;
	    ev.x_root = attr->x;
	    ev.y_root = attr->y;
	    XWarpPointer(DPY, wp->root()->X_window(), None, 0,0,0,0, 10, 10);
	    wmStartResize(&ev, wp);
	    wmResizeFlags = 0;
	    attr->x = (int)wp->oi_frame()->loc_x();
	    attr->y = (int)wp->oi_frame()->loc_y();
	}
	else if (cancel == Button3)
	{
	    wp->oi_frame()->set_associated_object(wp->root(), attr->x, attr->y,
		OI_ACTIVE_NOT_DISPLAYED);
	    wmZoom(F_VERTZOOM, wp, &attr->x, &attr->y);
	}
	wmScr->moveDelta = saveDelta;
	wmScr->nameOI->set_state(OI_ACTIVE_NOT_DISPLAYED);
	wp->set_positioned();
    }
    else
    {
	if (!wp->placed())
	{
	    int gravx, gravy;
	    int xright, ybottom;

	    xright = attr->x + attr->width + 2*attr->border_width;
	    ybottom = attr->y + attr->height + 2*attr->border_width;

	    wmGetGravityOffsets(wp, &gravx, &gravy);
	    if (gravx == 1)
		attr->x = xright - wp->oi_frame()->space_x();
	    if (gravy == 1)
		attr->y = ybottom - wp->oi_frame()->space_y();
	}
    }

    // put the panel on the root unmapped
    wp->oi_frame()->set_associated_object(wp->root(), attr->x, attr->y,
	OI_ACTIVE_NOT_DISPLAYED);

    //------------------------------------------------------
    // Get icon related resources.  NOTE that transient windows never
    // have icons created for them.
    //------------------------------------------------------
    if (!wp->transient())
    {
	// should we even get an icon?
	ptr = RM->get_search_resourceq(wmQuarks->noIconName(), wmQuarks->noIconClass());
	if (ptr && (ptr[0] == 't' || ptr[0] == 'T'))
		wp->set_no_icon();

	if (!wp->no_icon())
	{
	    // should we gray out the icon ?
	    ptr = RM->get_search_resourceq(wmQuarks->grayIconName(), wmQuarks->grayIconClass());
	    if (ptr && (ptr[0] == 't' || ptr[0] == 'T'))
	    {
		wp->set_gray_icon();
		wmMakeIcon(wp, 0, 0);
	    }
	    if (wp->icon_gravity()) 
	    {
		if (wp->sticky())
		    RM->pushq(wmQuarks->stickyName(), wmQuarks->stickyClass());

	    	wp->set_icon_region(RM->get_resourceq(wmQuarks->iconRegionName(), wmQuarks->iconRegionClass()));
	    	if (wp->icon_region())
                    irp = findIconRegion(wp->icon_region());
            	if (!irp)
		    if(wp->sticky())
			irp = findIconRegion("sticky");
		if (!irp)
                    irp = findIconRegion(wp->wclass_name());
            	if (!irp)
                    irp = findIconRegion(wp->wclass_class());
            	if (!irp)
                    irp = findIconRegion("Default");
	    	if (!irp) {
		    wp->clear_icon_gravity();
	    	    wp->set_root_icon();
		}
		// note the region pointer set here is
		// used after the if (wmScr->vdt) ...
	    }
	}
    }
    else
	wp->set_no_icon();

    if (wp->has_pushpin())
	wp->set_no_icon();

    // make the virtual window that will show up in the virtual root
    // display
    if (wmScr->vdt)
    {
        ptr = RM->get_search_resourceq(wmQuarks->_virtualName(), wmQuarks->_virtualClass());
	RM->pushq(wmQuarks->_virtualName(), wmQuarks->_virtualClass());
	if (!ptr || (ptr && (ptr[0] == 't' || ptr[0] == 'T')))
	{
	    int x = (int)wp->oi_frame()->loc_x() / wmScr->vscale;
	    int y = (int)wp->oi_frame()->loc_y() / wmScr->vscale;
	    int width = wp->oi_frame()->space_x() / wmScr->vscale;
	    int height = wp->oi_frame()->space_y() / wmScr->vscale;
	    // subtract an additional two pixels off for the border width
	    width -= 2;
	    height -= 2;
	    if (width <= 0) width = 1;
	    if (height <= 0) height = 1;

	    wp->set_vbox(oi_create_box(NULL,width,height));
	    wp->vbox()->set_bvl_width(0);
	    wp->vbox()->set_data((void *)new wmObjectData(0));
	    wmGetOIResources((OI_d_tech *)wp->vbox());
	    if (wmScr->pannerNames && (ptr = RM->get_resourceq(wmQuarks->fontName(), wmQuarks->fontClass())))
		wp->vbox()->set_font(ptr);
	    wp->vbox()->set_bdr_width(1);
//	    XGrabButton(DPY, Button2, 0, wp->vbox()->X_window(), True, 0, GrabModeAsync, GrabModeAsync, None, None);
	    for (int j = 0; j < VIRTUAL_EVENTS; j++)
	    {
		if (virtualEvents[j].event == KeyPress)
		    virtualEvents[j].argp = (char *)wp->oi_client();
		else
		    virtualEvents[j].argp = (char *)wp;
	    }
	    wmScr->conp->dispatch_group_insert(wp->vbox()->X_window(), VIRTUAL_EVENTS, &virtualEvents[0]);
	    wp->vbox()->set_associated_object(wmScr->pan, x,y, OI_NOT_DISPLAYED);

#ifdef OLD
	    if (wp->gray_icon())
	    {
#endif
		wp->set_vibox(oi_create_box(NULL,10,10));
		wp->vibox()->set_bvl_width(0);
		wp->vibox()->set_data((void *)new wmObjectData(0));
		wp->vibox()->set_associated_object(wmScr->pan, 0,0, OI_ACTIVE_NOT_DISPLAYED);
		wmGetOIResources((OI_d_tech *)wp->vibox());
		wp->vibox()->set_bdr_width(1);
//		XGrabButton(DPY, Button2, 0, wp->vibox()->X_window(), True, 0, GrabModeAsync, GrabModeAsync, None, None);
		wmScr->conp->dispatch_group_insert(wp->vibox()->X_window(), VIRTUAL_EVENTS, &virtualEvents[0]);
#ifdef OLD
	    }
#endif
	}
	RM->pop();
    }
    // do it here - otherwise restarted icons have large
    // vibox's
    if (irp && wp->icon_gravity())
    {
	wmMakeIcon(wp, wp->icon_x(), wp->icon_y(), False);
	irp->restartIcon(wp, wp->gravity_order());
    }

    XSaveContext(DPY, w, wmContext, (caddr_t)wp);
    XSaveContext(DPY, wp->oi_frame()->outside_X_window(), wmFrameContext,
	(caddr_t)wp);

    wmScr->windowList.append((ent)wp);
    wmSave(wp);
    wmUngrabServer();
    wmClient = NULL;
#ifdef DEBUG
#ifdef MEMLEAK
    if (mfd)
    {
	int len;
	char *p="wmReparent: DONE name=";
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
    // if the window si starting out busy, handle it now
    if (wp->busy())
	wmBusy(wp);

    // tell the dude who his root window is
    wmSet__SWM_ROOT(wp);

    // send a ConfigureNotify event so the client knows where he is at
    wmSendEvent(wp);

    return (wp);
}

void
wmReparentExisting()
{
    Window root_return,parent_return,*children;
    unsigned int num_children;
    int i, j;
    XMapRequestEvent e;

    noClass.res_name = wmNoName;
    noClass.res_class = wmNoName;

    XQueryTree(DPY, wmScr->conp->abs_root()->X_window(), &root_return,&parent_return, &children, &num_children);

    if (num_children == 0)
	return;

    // don't reparent icon windows
    for (i = 0; i < num_children; i++)
    {
	if (children[i])
	{
	    XWMHints *wmhintsp = XGetWMHints(DPY, children[i]);
	    if (wmhintsp)
	    {
		if (wmhintsp->flags & IconWindowHint)
		{
		    for (j = 0; j < num_children; j++)
		    {
			if (children[j] == wmhintsp->icon_window)
			{
			    children[j] = None;
			    break;
			}
		    }
		}
		XFree ((char *) wmhintsp);
	    }
	}
    }

    // get rid of all non-mapped or override-redirect windows
    for (i = 0; i < num_children; i++)
    {
	if (children[i] && !wmMappedNotOverride(children[i]))
	    children[i] = None;
    }

    // now reparent all mapped, non-override-redirect windows
    e.parent = wmScr->root;
    for (i = 0; i < num_children; i++)
    {
	if (children[i])
	{
	    XUnmapWindow(DPY, children[i]);
	    e.window = children[i];
	    wmMapRequest(&e, NULL);
	}
    }
}

/************************************************************************
 *
 *  Procedure:
 *	wmGetGravityOffsets - map gravity to (x,y) offset signs for adding
 *		to x and y when window is mapped to get proper placement.
 * 
 *    This code was borrowed from twm and so elegantly written by
 *    Jim Fulton of MIT.  Thanks, Jim.
 *
 ************************************************************************
 */

void
wmGetGravityOffsets(
    wmData *wp,
    int *xp,
    int *yp)
{
    static struct _gravity_offset {
	int x, y;
    } gravity_offsets[11] = {
	{  0,  0 },			/* ForgetGravity */
	{ -1, -1 },			/* NorthWestGravity */
	{  0, -1 },			/* NorthGravity */
	{  1, -1 },			/* NorthEastGravity */
	{ -1,  0 },			/* WestGravity */
	{  0,  0 },			/* CenterGravity */
	{  1,  0 },			/* EastGravity */
	{ -1,  1 },			/* SouthWestGravity */
	{  0,  1 },			/* SouthGravity */
	{  1,  1 },			/* SouthEastGravity */
	{  0,  0 },			/* StaticGravity */
    };
    XSizeHints *size_hints = wp->size_hints_p();
    register int g = ((size_hints->flags & PWinGravity) 
		      ? size_hints->win_gravity : NorthWestGravity);

    if (g < ForgetGravity || g > StaticGravity) {
	*xp = *yp = 0;
    } else {
	*xp = gravity_offsets[g].x;
	*yp = gravity_offsets[g].y;
    }
}

void
wmGetColormapWindows(wmData *wp)
{
    if (wp->colormap_windows())
    {
	for (int i = 0; i < wp->colormap_count(); i++)
	{
	    // don't remove the events from the top level window
	    if (wp->colormap_windows()[i] == wp->window())
		continue;

	    wmScr->conp->dispatch()->discard(wp->colormap_windows()[i]);
	}
	XFree((char *)wp->colormap_windows());
    }

    wp->set_colormap_windows(NULL);
    wp->set_colormap_count(0);

    if (XGetWMColormapWindows(DPY, wp->window(), wp->colormap_windows_p(), wp->colormap_count_p()))
    {
	for (int i = 0; i < wp->colormap_count(); i++)
	{
	    // don't add events to the top level window
	    if (wp->colormap_windows()[i] == wp->window())
		continue;

	    for (int j = 0; j < COLORMAP_EVENTS; j++)
		colormapEvents[j].argp = (char *)wp;
	    wmScr->conp->dispatch_group_insert(wp->colormap_windows()[i],
		COLORMAP_EVENTS, &colormapEvents[0]);
	}
    }
}

int
wmMappedNotOverride(Window w)
{
    XWindowAttributes wa;

    XGetWindowAttributes(DPY, w, &wa);
    return ((wa.map_state != IsUnmapped) && (wa.override_redirect != True));
}

void
wmRemoveClientEvents(
    wmData *wp
    )
{
    for (int i = 0; i < CLIENT_EVENTS; i++)
	clientEvents[i].argp = (char *)wp;
    wmScr->conp->dispatch()->group_remove(wp->window(), CLIENT_EVENTS, clientEvents);
    OI_dispatch_remove(wp->window(), KeyPress, 0, (OI_event_fnp)wmHandleKeyPress, (char *)wp->oi_client());
}

