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
 * $Id: icons.C,v 9.22 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Icon routines
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/

#ifndef lint
static char RCSinfo[] =
"$Id: icons.C,v 9.22 1993/08/27 16:58:15 toml Exp $";
#endif /* lint */

#include "swm.H"
#include "gram.H"
#include "screen.H"
#include "main.H"
#include "list.H"
#include "object.H"
#include "util.H"
#include "panel.H"
#include "parse.H"
#include "debug.H"
#include "init.H"
#include "wmdata.H"
#include "resize.H"
#include "move.H"
#include "icons.H"
#include "events.H"
#include "atoms.H"
#include "execute.H"
#include "bitmap.H"
#include "quarks.H"
#include "region.H"

#ifdef DEBUG
extern void wmCheck();
#endif

// dispatch entries for the small virtual windows
static OI_dispatch_entry iconEvents[] = {
{EnterNotify, EnterWindowMask, (OI_event_fnp)wmVirtualEnterNotify, NULL, NULL_PMF, NULL },
{LeaveNotify, LeaveWindowMask, (OI_event_fnp)wmVirtualLeaveNotify, NULL, NULL_PMF, NULL },
};

#define ICON_EVENTS (sizeof(iconEvents)/sizeof(OI_dispatch_entry))

static void newIconPanelRow(wmIconPanel *);
static wmObject *findIconPanel(char *);
IconRegion *findIconRegion(char *);
static void lookForExpose(OI_d_tech *, wmData *);
static void grayIcons(wmIconPanel *);
static void zap(OI_d_tech *, OI_d_tech *);
static void findMaxColumn(wmIconPanel *);

const int wmDefaultSlotSize=10;
char wmMakingIcon = False;


/**********************************************************************
 *
 *  Procedure:
 *	wmInitIconPanel
 *
 *  Function:
 *	Initialize an icon panel.  This is called from
 *	wmInstantiatePanel.
 *
 **********************************************************************
 */

void
wmInitIconPanel(
    wmObject *op
    )
{
    char *ptr;
    wmIconPanel *ip;
    OI_scroll_box *bp;

    bp = (OI_scroll_box *)op->oi;
    ip = op->u.p->icon;
    ip->bp = (OI_scroll_box *)op->oi;
    ip->ob = (OI_d_tech *)bp->object_box();
    XSaveContext(DPY, ip->ob->X_window(), wmIconPanelContext, (caddr_t)ip);

    if (op->u.p->icon->scrollBars)
    {
	ip->rsb = (OI_scroll_bar *)bp->left_scroll_bar();
	ip->bsb = (OI_scroll_bar *)bp->top_scroll_bar();
	bp->allow_auto_controller_visibility();
	bp->set_gravity(OI_grav_northwest);
	bp->set_object_size(op->u.p->geom->width, op->u.p->geom->height);
	bp->set_view_size(op->u.p->geom->width, op->u.p->geom->height);
	ip->view_rows = op->u.p->geom->height;
	ip->view_columns = op->u.p->geom->width;
	ip->columns = op->u.p->geom->width;
    }
    else
    {
	bp->set_view_size(1, 1);
	bp->set_object_size(1, 1);
	ip->view_rows = 1;
	ip->view_columns = op->u.p->geom->width;
	ip->columns = op->u.p->geom->width;
    }

    ip->base_width = 0;
    ip->base_height = 0;
    ip->maxColumnUsed = 0;
    for (int i = 0; i < ip->view_rows; i++)
	newIconPanelRow(ip);
    ptr = RM->get_resourceq(wmQuarks->packName(), wmQuarks->packClass());
    ip->pack = (ptr && (ptr[0] == 't' || ptr[0] == 'T'));
    ptr = RM->get_resourceq(wmQuarks->fitName(), wmQuarks->fitClass());
    ip->fit = (ptr && (ptr[0] == 't' || ptr[0] == 'T'));
    ptr = RM->get_resourceq(wmQuarks->squeezeName(), wmQuarks->squeezeClass());
    ip->squeeze = (ptr && (ptr[0] == 't' || ptr[0] == 'T'));
    ptr = RM->get_resourceq(wmQuarks->showGridName(), wmQuarks->showGridClass());
    ip->showGrid = (ptr && (ptr[0] == 't' || ptr[0] == 'T'));
    ptr = RM->get_resourceq(wmQuarks->hideWhenEmptyName(), wmQuarks->hideWhenEmptyClass());
    ip->hide = (ptr && (ptr[0] == 't' || ptr[0] == 'T'));

    // if the grid is on, increase the pad so we can see it
    if (ip->showGrid)
    {
	ip->pad += 1;
	// we care about the expose event so we can repaint the grid and set size hints
	OI_dispatch_insert(op->oi->X_window(), Expose, ExposureMask, (OI_event_fnp)wmRepaintIconGrid, (char *)ip);
    }
}

/***************************************************************
 *
 *  Procedure: 
 *	wmIconify
 *
 *  Function:
 *	Iconify a client
 *
 ***************************************************************/

int
wmIconify(
    wmData *wp,			// the window data structure
    int x_pos,			// the default x position
    int y_pos			// the default y position
    )
{
    wmData *tmpwp;
    XWindowAttributes attr;
    unsigned long eventMask;
    IconRegionContents *irp;

#ifdef DEBUG
    if (wmDebug)
	fprintf(dfp, "wmIconify: \"%s\"\n", wp->name());
#endif
#ifdef DEBUG
	wmCheck();
#endif
	
    if (wp->state() == IconicState)
	return (False);

    // if it has gravity it moves to a new region if window moved
    // and followClient is True
    if (wp->icon_gravity())
    {
	irp = (IconRegionContents *)wp->get_irp();
	if (irp)
	{
	    if (irp->followClient() && !irp->onScreen((int)wp->oi_frame()->loc_x(), (int)wp->oi_frame()->loc_y()))
	    {
	        irp->removeIcon(wp);
	        irp->mother()->addIcon(wp);
	    }
	}
   }
										
    if (wp->transient_for() && !wp->transient_for_wp())
	return (False);
    
    // try to make an icon if we don't already have one
    if (!wp->no_icon() && !wp->oi_icon())
	wmMakeIcon(wp, x_pos, y_pos);

    // don't allow the window to iconify if there isn't an icon
    if (!wp->iconify_by_unmapping() && wp->no_icon() && !wp->transient_for() && !wp->has_pushpin())
	return (False);

    // if this is the focus window and we are in click to type
    // mode, reset the focus to my beeper window
    if (wmFocusModel == wmFocusModelClickToType && wp == wmFocusWp)
	wmClearFocus(NULL, wmFocusWp);

    // bring down the frame, if there is already an icon, zap the
    // thing before bringing down the frame.  It just looks better 
    // that way
    int zapped = False;
    if (wp->oi_icon())
    {
#ifdef DEBUG
	if (!OI_debug)
#endif /* DEBUG */
	    wmGrabServer();
	zap(wp->oi_frame(), wp->oi_icon());
	zap(wp->oi_frame(), wp->oi_icon());
	zapped = True;
	wmUngrabServer();
    }
    wp->unmap();

    // need to set the event mask so we don't get an unmap notify
    // and think he is going into withdrawl
    XGetWindowAttributes(DPY, wp->window(), &attr);
    eventMask = attr.your_event_mask;
    XSelectInput(DPY, wp->window(), eventMask & ~StructureNotifyMask);
    XUnmapWindow(DPY, wp->window());
    XSelectInput(DPY, wp->window(), eventMask);

    wp->set_iconified();
    wmSetWM_STATE(wp, IconicState);

    if (!wp->no_icon())
    {
	// if gray icons are set, get rid of the icon and bring it back
	if (wp->gray_icon())
	{
	    wp->unmap_icon();
	    // if it is in an icon panel, get rid of it
	    if (wp->ip())
	    {
		XClearArea(DPY, wp->ip()->ob->X_window(),
		    wp->ipsp()->x, wp->ipsp()->y,
		    wp->ip()->slot_width, wp->ip()->slot_height, True);
	    }
	}
	wp->map_icon();
	// if we didn't already zap the dude, do it now
	if (!zapped)
	{
#ifdef DEBUG
	    if (!OI_debug)
#endif /* DEBUG */
		wmGrabServer();
	    zap(wp->oi_frame(), wp->oi_icon());
	    zap(wp->oi_frame(), wp->oi_icon());
	    wmUngrabServer();
	}
    }

    // iconify transient windows (if any)
    // only iconify the transients if they are not a member of the same group as the leader
    // if they are group members, the command grouping will do the iconification
    wmList *list = wp->transient_list_p();
    for (tmpwp = (wmData *)list->first(); tmpwp != NULL; tmpwp = (wmData *)list->next())
	if (!tmpwp->group() || (tmpwp->group() != wp->group()))
	    wmIconify(tmpwp, x_pos, y_pos);

    // if this is a transient, iconify the leader, which should cause all transients to iconify
    // only iconify the leader if it is not a member of the same group as this window
    if (wp->transient_for() && wp->transient_for_wp())
	if (!wp->group() || (wp->group() != wp->transient_for_wp()->group()))
	    wmIconify(wp->transient_for_wp(), x_pos, y_pos);

#ifdef DEBUG
	wmCheck();
#endif
    return (True);
}

/**********************************************************************
 *
 *  Procedure:
 *	zap
 *
 *  Function:
 *	Draw "zap" lines between the icon and window
 *
 **********************************************************************
 */

static void
zap(OI_d_tech *oi1, OI_d_tech *oi2)
{
    if (!wmScr->zap || !wmInitDone)
	return;

    Window	junkChild;
    XSegment	outline[4];
    XSegment	*r = outline;
    int		xr1, xl1, yt1, yb1;
    int		xr2, xl2, yt2, yb2;

    XTranslateCoordinates(DPY, oi1->X_window(), wmScr->root,
	0, 0, &xl1, &yt1, &junkChild);
    XTranslateCoordinates(DPY, oi2->X_window(), wmScr->root,
	0, 0, &xl2, &yt2, &junkChild);

    xr1 = xl1 + oi1->space_x();
    yb1 = yt1 + oi1->space_y();
    xr2 = xl2 + oi2->space_x();
    yb2 = yt2 + oi2->space_y();

    // top left
    r->x1 = xl1;
    r->y1 = yt1;
    r->x2 = xl2;
    r++->y2 = yt2;

    // top right
    r->x1 = xr1;
    r->y1 = yt1;
    r->x2 = xr2;
    r++->y2 = yt2;

    // bottom left
    r->x1 = xl1;
    r->y1 = yb1;
    r->x2 = xl2;
    r++->y2 = yb2;

    // bottom right
    r->x1 = xr1;
    r->y1 = yb1;
    r->x2 = xr2;
    r++->y2 = yb2;

    if (r != outline)
    {
	XDrawSegments(DPY, wmScr->root, wmScr->outlineGC, outline, r - outline);
	XFlush(DPY);
    }
}

/***************************************************************
 *
 *  Procedure: 
 *	wmMakeIcon
 *
 *  Function:
 *	Create the icon for a client and place it somewhere
 *
 ***************************************************************/

void
wmMakeIcon(
    wmData *wp,			// the window data structure
    int x_pos,			// the default x position
    int y_pos,			// the default y position
    Bool place_it		// only used by iconRegion after restarts, True otherwise
    )
{
    unsigned int width, height;
    wmObjectData *wod;

#ifdef DEBUG
	wmCheck();
#endif
    if (wp->no_icon())
	return;

    if (!wp->has_icon())
	wp->new_icon();

    char *ptr;
    wmData *savewmClient = wmClient;
    wmClient = wp;

    if (!savewmClient)
    {
	RM->set_stack_ptr(wmScr->rm_stack);
	RM->push(wp->wclass_class(), wp->wclass_class());
	RM->push(wp->wclass_name(), wp->wclass_name());
	if (wp->sticky())
	    RM->pushq(wmQuarks->stickyName(), wmQuarks->stickyClass());
    }

    ptr = RM->get_resourceq(wmQuarks->iconifyByUnmappingName(), wmQuarks->iconifyByUnmappingClass());
    if (ptr && (ptr[0] == 't' || ptr[0] == 'T'))
    {
	wp->set_iconify_by_unmapping();
	wp->set_no_icon();
	wp->del_icon();
	return;
    }

    // is the icon to be placed on the root?
    if (!wp->root_icon())
    {
	ptr = RM->get_resourceq(wmQuarks->rootIconName(), wmQuarks->rootIconClass());
	if (ptr && (ptr[0] == 't' || ptr[0] == 'T'))
	    wp->set_root_icon();
    }
    if (wp->icon_window() == wmScr->root)
	wp->set_root_icon();

    // should I use a specific icon panel ?
    wp->set_icon_panel(RM->get_resourceq(wmQuarks->iconPanelName(), wmQuarks->iconPanelClass()));

    // should I use a specific icon region ?
    wp->set_icon_region(RM->get_resourceq(wmQuarks->iconRegionName(), wmQuarks->iconRegionClass()));
    if (!wp->icon_object())
    {
	// what type of icon should we put on
	wp->set_icon_object(RM->get_resourceq(wmQuarks->iconName(), wmQuarks->iconClass()));

	if (!wp->icon_object())
	{
	    wp->set_no_icon();
	    wp->del_icon();
	    return;
	}
    }

    wp->set_iop(wmCreateObject(OBJ_PANEL, wp->icon_object()));
    if (!wp->iop()->expanded)
	wmExpandObjects();

    wmMakingIcon = True;
    wp->set_oi_icon(wmInstantiateObject(wp->iop()));
    wod = (wmObjectData *)wp->oi_icon()->data();
    wod->icon = True;
    if (wmFocusModel == wmFocusModelNormal)
    {
	// if we don't have key events yet, get them so focus can follow the mouse
	if (!wod->handleKey)
	    OI_dispatch_insert(wp->oi_icon()->X_window(), KeyPress, KeyPressMask, (OI_event_fnp)wmHandleKeyPress, (char *)wp->oi_icon());
	for (int j = 0; j < ICON_EVENTS; j++)
	    iconEvents[j].argp = (char *)wp;
	wmScr->conp->dispatch_group_insert(wp->oi_icon()->X_window(), ICON_EVENTS, &iconEvents[0]);
    }
    wmMakingIcon = False;
    XSaveContext(DPY, wp->oi_icon()->outside_X_window(), wmIconContext,
	(caddr_t)wp);

    // if we want to gray out the icon, when deiconified, we want to look
    // for expose events
    if (wp->gray_icon())
    {
	lookForExpose(wp->oi_icon(), wp);
    }

    // watch for visibility changes for f.raiselower
    OI_dispatch_insert(wp->oi_icon()->X_window(), VisibilityNotify, VisibilityChangeMask, (OI_event_fnp)wmIconVisibilityNotify, (char *)wp);

    wp->oi_icon()->set_name(wmUniqueName());

    // do we need to do something with the icon image ?
    wp->set_oi_icon_image(wp->oi_icon()->descendant("iconImage"));
    if (wp->oi_icon_image())
    {
	wmMakeIconImage(wp);
    }

    // find the icon name object (if any)  wmLayoutPanel will get called in wmDisplayIconName
    wp->set_oi_icon_name(wp->oi_icon()->descendant("iconName"));
    wmDisplayIconName(wp);

    // layout the icon
    // wmLayoutPanel(wp->iop());

#ifdef DEBUG
    if (wmDebug > 1)
	fprintf(dfp, "  icon object size = (%d x %d)\n",
	    wp->iop()->oi->space_x(), 
	    wp->iop()->oi->space_y());
#endif

    if (wp->vibox()) {
	width = wp->oi_icon()->space_x()/wmScr->vscale;
	height = wp->oi_icon()->space_y()/wmScr->vscale;
	if (!width)
		width = 1;
	if (!height)
		height = 1;
	wp->vibox()->set_size(width, height);
    }
    if (place_it)
	wmPlaceIcon(wp, x_pos, y_pos);
    wmClient = savewmClient;
#ifdef DEBUG
	wmCheck();
#endif
}

void
wmPlaceIcon(
    wmData *wp,
    int x_pos,
    int y_pos
    )
{
    Bool positionHint;
#ifdef DEBUG
	wmCheck();
#endif

    positionHint = (wp->wmhints() && (wp->wmhints()->flags & IconPositionHint));

    // rootIcon overrides an icon panel, so only find an icon panel 
    // to place icon in if the icon isn't to go on root
    wmObject *op = NULL;
    if (!wp->root_icon())
    {
	// first look to see if a specific icon panel is desired
	if (wp->icon_panel())
	    op = findIconPanel(wp->icon_panel());
	if (!op)
	    op = findIconPanel(wp->wclass_name());
	if (!op)
	    op = findIconPanel(wp->wclass_class());
	if (!op)
	    op = findIconPanel("Default");
    }

    // an icon panel overrides icon gravity regions if we have
    // a panel don't bother finding a region
    // if IconPositionHint is set it is a root icon unless icon_gravity() is also set
    IconRegion *irp = NULL;
    if (!op) 
    {
	if (!positionHint || (positionHint && wp->icon_gravity()))
    	{
	    if (wp->icon_region())
    	    	irp = findIconRegion(wp->icon_region());
	    if (!irp) 
		if (wp->sticky())
		    irp = findIconRegion("sticky");
	    if (!irp)
    		irp = findIconRegion(wp->wclass_name());
	    if (!irp)
    	    	irp = findIconRegion(wp->wclass_class());
	    if (!irp)
    	    	irp = findIconRegion("Default");
	}
    }

    // so we have either a panel, a region or neither
    if (op)
    {
	wmIconPanel *ip = op->u.p->icon;
	wmInsertIcon(wp, ip);
	wp->set_icon_window(None);
    }
    else if (irp) 
    {
	wp->oi_icon()->set_state(OI_ACTIVE_NOT_DISPLAYED);
	irp->addIcon(wp);
	// do i really want to do this?
	wp->set_icon_window(wmScr->root);
    }
    else 
   {
	// neither a panel or gravity
	if (positionHint)
	{
	    x_pos = wp->wmhints()->icon_x;
	    y_pos = wp->wmhints()->icon_y;
	}
	else  if (wmScr->vdt)
	{
	    // translate the position to the virtual root coordinates if the
	    // window is on the virtual root
	    if (wp->root() == wmScr->vroot)
	    {
		int x,y;
		Window junkChild;

		XTranslateCoordinates(DPY, wmScr->root, wmScr->vroot->X_window(), x_pos, y_pos, &x, &y, &junkChild);
		x_pos = x;
		y_pos = y;
	    }
	}

	wp->oi_icon()->set_associated_object(wp->root(), x_pos, y_pos);
	wp->move_icon(x_pos, y_pos);
	wp->map_icon();
	wp->set_icon_window(wmScr->root);
    }
#ifdef DEBUG
	wmCheck();
#endif
}

void
wmDisplayIconName(
    wmData *wp
    )
{
    if (wp->oi_icon())
    {
	if (wp->oi_icon_name())
	{
	    int len = strlen(wp->icon_name());
	    if (wp->oi_icon_name()->is_derived_from("OI_entry_field"))
	    {
		if (len > wp->icon_label_length())
		    len = wp->icon_label_length();
		wp->oi_icon_name()->set_dsp_length(len);
		wp->oi_icon_name()->set_default_text(wp->icon_name());
	    }
	    else
	    {
		// must be a static text object
		OI_static_text *sp = (OI_static_text *)wp->oi_icon_name();
		if (len > wp->icon_label_length())
		{
		    len = wp->icon_label_length();
		    wp->icon_name()[len] = '\0';
		}
		sp->set_text(wp->icon_name());
	    }
	}
	OI_state st = wp->oi_icon()->state();
	wp->oi_icon()->set_state(OI_NOT_DISPLAYED);
	wp->iop()->oi = wp->oi_icon();
	wmLayoutPanel(wp->iop());
	wp->oi_icon()->set_state(st);
    }
}

/**********************************************************************
 *
 *  Procedure:
 *	wmDeiconify
 *
 *  Function:
 *	Take a client into NormalState
 *
 **********************************************************************
 */

int
wmDeiconify(
    wmData *wp
    )
{
    wmData *tmpwp;

#ifdef DEBUG
	wmCheck();
#endif
    if (wp->state() == NormalState || !wp->oi_frame())
	return (False);

    wp->set_state(NormalState);

    if (!(wp->normalized() || wp->positioned()))
    {
        if (wmScr->vdt)
        {
            int x_pos = (int)wp->oi_frame()->loc_x();
            int y_pos = (int)wp->oi_frame()->loc_y();
            int x, y;
            Window junkChild;
            XTranslateCoordinates(DPY, wmScr->root, wp->root()->X_window(), x_pos, y_pos, &x, &y, &junkChild);
	    wp->move(x, y);
	    wmSendEvent(wp);
        }
    }

    if (wp->oi_icon())
    {
#ifdef DEBUG
	if (!OI_debug)
#endif /* DEBUG */
	    wmGrabServer();
	zap(wp->oi_icon(), wp->oi_frame());
	zap(wp->oi_icon(), wp->oi_frame());
	wmUngrabServer();
	if (wp->gray_icon())
	    wmGrayIcon(wp);
	else
	    wp->unmap_icon();
    }
    XMapWindow(DPY, wp->window());
    wmSetWM_STATE(wp, NormalState);
    wp->map();

    // deiconify transient windows (if any)
    // only deiconify the transients if they are not a member of the same group as the leader
    // if they are group members, the command grouping will do the deiconification
    wmList *list = wp->transient_list_p();
    for (tmpwp = (wmData *)list->first(); tmpwp != NULL; tmpwp = (wmData *)list->next())
	if (!tmpwp->group() || (tmpwp->group() != wp->group()))
	    wmDeiconify(tmpwp);

    // if this is a transient, deiconify the leader, which should cause all transients to deiconify
    // only deiconify the leader if it is not a member of the same group as this window
    if (wp->transient_for() && wp->transient_for_wp())
	if (!wp->group() || (wp->group() != wp->transient_for_wp()->group()))
	    wmDeiconify(wp->transient_for_wp());

#ifdef DEBUG
	wmCheck();
#endif
    return (True);
}

static void
lookForExpose(
    OI_d_tech *oi,
    wmData *wp
    )
{
    OI_d_tech *c;
    OI_dispatch_insert(oi->X_window(), Expose, ExposureMask, (OI_event_fnp)wmExposeIcon, (char *)wp);
    for (c = oi->next_child(NULL); c != NULL; c = oi->next_child(c))
	lookForExpose(c, wp);
}

/**********************************************************************
 *
 *  Procedure:
 *	wmMakeIconImage
 *
 *  Function:
 *	We know at this point that we have an iconImage object.  This
 *	routine looks at the window manager hints and does the right
 *	thing in regards to a client supplied icon pixmap or icon window.
 *
 **********************************************************************
 */

void
wmMakeIconImage(
    wmData *wp
    )
{
    Window junkRoot;
    int junkX, junkY;
    unsigned int width, height, junkBW, junkDepth;
    Pixmap pix = None;

#ifdef DEBUG
	wmCheck();
#endif
#ifdef SHAPE
    wmObjectData *odp;
    odp = (wmObjectData *)wp->oi_icon_image()->data();
#endif /* SHAPE */

    // if the client supplies its own icon window
    if (wp->wmhints() && (wp->wmhints()->flags & IconWindowHint))
    {
	XGetGeometry(DPY, wp->wmhints()->icon_window, &junkRoot, &junkX, &junkY, &width, &height, &junkBW, &junkDepth);
	wp->oi_icon_image()->set_size(width + 2*junkBW, height + 2*junkBW);
	XReparentWindow(DPY, wp->wmhints()->icon_window, wp->oi_icon_image()->X_window(), 0, 0);
	XAddToSaveSet(DPY, wp->wmhints()->icon_window);
	XMapWindow(DPY, wp->wmhints()->icon_window);
#ifdef SHAPE
	if (odp->shape)
	{
	    if (odp->shapePixmap)
		XShapeCombineMask(DPY, wp->wmhints()->icon_window, ShapeBounding, 0, 0, odp->shapePixmap, ShapeSet);
	    else if (wp->wmhints()->flags & IconMaskHint)
	    {
		odp->shapePixmap =  wp->wmhints()->icon_mask;
		XShapeCombineMask(DPY, wp->wmhints()->icon_window, ShapeBounding, 0, 0, wp->wmhints()->icon_mask, ShapeSet);
	    }
	}
#endif /* SHAPE */
    }
    // else if the application is supplying its own pixmap, use it.
    else if (wp->wmhints() && (wp->wmhints()->flags & IconPixmapHint))
    {
	pix = wp->wmhints()->icon_pixmap;
    }
    else if (wp->icon_image())
    {
	pix = wp->icon_image();
    }
    else if (!wp->icon_image())
    {
	char *ptr = RM->get_resourceq(wmQuarks->defaultIconImageName(), wmQuarks->defaultIconImageClass());
	if (ptr)
	{
	    wmBitmap *wbm = wmFindBitmap(ptr);
	    if (wbm)
	    {
		pix = wbm->pixmap;
		wp->set_icon_image(wbm->pixmap);
	    }
	}
	wp->set_got_icon_image();
    }

    if (pix != None)
    {
	Pixmap pm;
	XGCValues gcv;

	XGetGeometry(DPY, pix, &junkRoot, &junkX, &junkY, &width, &height, &junkBW, &junkDepth);

	gcv.foreground = wp->oi_icon_image()->fg_pixel();
	gcv.background = wp->oi_icon_image()->bkg_pixel();
	GC gc = XCreateGC(DPY, wmScr->root,GCForeground|GCBackground,&gcv);
	pm = XCreatePixmap(DPY, wmScr->root, width, height, wmScr->depth);
	XCopyPlane(DPY, pix, pm, gc, 0, 0, width, height, 0, 0, 1);
	wp->oi_icon_image()->set_size(width, height);
	XSetWindowBackgroundPixmap(DPY, wp->oi_icon_image()->X_window(), pm);
	XClearWindow(DPY, wp->oi_icon_image()->X_window());
	XFreePixmap(DPY, pm);
	XFreeGC(DPY, gc);
    }
#ifdef SHAPE
    if (odp->shape && !odp->shapePixmap && (wp->wmhints()->flags & IconMaskHint))
	    odp->shapePixmap =  wp->wmhints()->icon_mask;
#endif /* SHAPE */
#ifdef DEBUG
	wmCheck();
#endif
}

static wmObject *
findIconPanel(
    char *str			// the name to match to
    )
{
    wmObject *op;
    int len = strlen(str);

#ifdef DEBUG
    if (wmDebug)
	fprintf(dfp, "findIconPanel: \"%s\"\n", str);
#endif
    if (len == 0)
	return (NULL);

    for (op = (wmObject *)wmScr->objectList.first(); op != NULL;
        op = (wmObject *)wmScr->objectList.next())
    {
	if (!(op->type == OBJ_PANEL && op->u.p->icon))
	    continue;

#ifdef DEBUG
    if (wmDebug > 1)
	fprintf(dfp, "  comparing to \"%s\"\n", op->name);
#endif
	if (!strncmp(str, op->name, len))
	    return (op);
    }
    return (NULL);
}

IconRegion *
findIconRegion(
    char *str			// the name to match to
)
{
    IconRegion *irp;
    int len = strlen(str);
    int x0= 0;
    int y0 = 0;

#ifdef DEBUG
	wmCheck();
#endif
    if (len == 0)
	return (NULL);

#ifdef DEBUG
	wmCheck();
#endif
    for (irp = (IconRegion *)wmScr->iconRegionList.first(); irp != NULL;
	irp = (IconRegion *)wmScr->iconRegionList.next())
    {
	    if (!strncmp(str, irp->name(), len)) 
		return (irp);
    }
#ifdef DEBUG
	wmCheck();
#endif
    return (NULL);
}

void
wmInsertIcon(
    wmData *wp,
    wmIconPanel *ip
    )
{
    OI_scroll_box *bp;
    int row, col;
    int x_pos, y_pos;
    int saveColumns;
    int newRow;

    // wp->ipop  +++ do we need this field in wmData ?
    wp->set_ip(ip);
    bp = ip->bp;
    int size_changed = False;

    // is the slot size big enough?
    if (ip->slot_width < (wp->oi_icon()->space_x() + ip->pad))
    {
	size_changed = True;
	ip->slot_width = wp->oi_icon()->space_x() + ip->pad;
	if (ip->slot_width % 4)
	    ip->slot_width += 4 - (ip->slot_width % 4);
    }
    if (ip->slot_height < (wp->oi_icon()->space_y() + ip->pad))
    {
	size_changed = True;
	ip->slot_height = wp->oi_icon()->space_y() + ip->pad;
	if (ip->slot_height % 4)
	    ip->slot_height += 4 - (ip->slot_height % 4);
    }
	    
    // find a slot for the icon
    int found = False;

    for (row = 0; row < ip->rows; row++)
    {
	for (col = 0; col < ip->columns; col++)
	{
	    if (!ip->row[row][col].used)
	    {
		found = True;
		break;
	    }
	}
	if (found)
	    break;
    }
    newRow = False;
    if (!found)
    {
	col = 0;
	if (!ip->columns)
	{
	    ip->columns = ip->view_columns;
	    if (ip->columns == 0)
		ip->columns = 1;
	}
	newIconPanelRow(ip);
	newRow = True;
    }
    // put it in the slot
    ip->count++;
    ip->row[row][col].used = True;
    ip->row[row][col].wp = wp;
    wp->set_ipsp(&ip->row[row][col]);
    x_pos = ((ip->slot_width - wp->oi_icon()->space_x()) / 2) + col * ip->slot_width;
    y_pos = ((ip->slot_height - wp->oi_icon()->space_y()) / 2) + row * ip->slot_height;
    ip->row[row][col].x = col * ip->slot_width;
    ip->row[row][col].y = row * ip->slot_height;
    wp->oi_icon()->set_associated_object(bp, x_pos, y_pos, OI_ACTIVE);
    wp->unmap_vibox();

    saveColumns = ip->maxColumnUsed;
    findMaxColumn(ip);
    if (size_changed)
    {
	wmLayoutIconPanel(ip);
	// wmSizeIconPanel(ip);
    }
    else if (!ip->scrollBars && (newRow || (saveColumns != ip->maxColumnUsed)))
    {
	// wmSizeIconPanel(ip);
    }
    wmSizeIconPanel(ip);

    bp->set_object_size(ip->columns, ip->rows);
    if (ip->hide && ip->wp->oi_frame()->state() != OI_ACTIVE)
    {
	ip->op->oi->set_state(OI_ACTIVE);
	ip->wp->map();
    }

    wmUpdateSize(ip->wp);
}

static void
findMaxColumn(
    wmIconPanel *ip
    )
{
    int row, col, max_col;

    max_col = 0;
    for (row = 0; row < ip->rows; row++)
    {
        for (col = 0; col < ip->columns; col++)
        {
            if (ip->row[row][col].used)
            {
		if (col > max_col)
		    max_col = col;
            }
        }
    }
    ip->maxColumnUsed = max_col;
}

static void
newIconPanelRow(
    wmIconPanel *ip
    )
{
    int row;
    wmIconPanelSlot **newp;

    newp = (wmIconPanelSlot **)calloc(ip->rows+1, sizeof(wmIconPanelSlot *));
    for (row = 0; row < ip->rows; row++)
	newp[row] = ip->row[row];
    newp[row] = (wmIconPanelSlot *)calloc(ip->columns, sizeof(wmIconPanelSlot));

    // the calloc will have zeroed the memory, effectively marking
    // the slots as unused.

    if (ip->row)
	free((char *)ip->row);

    // increase row count and replace the pointer
    ip->rows++;
    ip->row = newp;
}

/***************************************************************
 *
 *  Procedure: 
 *	wmPackIconPanel
 *
 *  Function:
 *	Pack the icon panel.  This is called when the user 
 *	specifically asks to pack the panel or if the panel
 *	is to remain packed following an icon deletion.
 *	Packing the icon panel simply means to get rid of as 
 *	many free slots as possible.
 *
 *      Note that this routine never decreases the number of 
 *	columns.
 *
 ***************************************************************/

void
wmPackIconPanel(
    wmIconPanel *ip		// the icon panel to pack
    )
{
    // how many rows do we need
    int rows = ip->count / ip->columns;
    if ((ip->count % ip->columns) != 0)
	rows++;

    if (rows == 0)
	rows = 1;
    wmIconPanelSlot **oldp = ip->row;
    int old_rows = ip->rows;
    ip->row = NULL;
    ip->rows = 0;
    for (int i = 0; i < rows; i++)
	newIconPanelRow(ip);
    int nrow = 0;
    int ncol = 0;
    for (int row = 0; row < old_rows; row++)
    {
	for (int col = 0; col < ip->columns; col++)
	{
	    // if the entry is not free, move it into the new structure
	    if (oldp[row][col].used)
	    {
		ip->row[nrow][ncol] = oldp[row][col];
		ip->row[nrow][ncol].wp->set_ipsp(&ip->row[nrow][ncol]);
		if (++ncol >= ip->columns)
		{
		    ++nrow;
		    ncol = 0;
		}
	    }
	}
    }

    // free up the old icon panel stuff
    for (i = 0; i < rows; i++)
	free((char *)oldp[i]);
    if (oldp)
	free((char *)oldp);
    XClearArea(DPY, ip->ob->X_window(), 0,0,0,0, True);
}

/***************************************************************
 *
 *  Procedure: 
 *	wmFitIconPanel
 *
 *  Function:
 *	Fit the icon panel into the current number of viewable
 *	columns.  This routine does not remove free slots.
 *
 ***************************************************************/

void
wmFitIconPanel(
    wmIconPanel *ip		// the icon panel to fit
    )
{
    // how many rows do we need
    int slots = ip->rows * ip->columns;
    int rows = slots / ip->view_columns;
    // make sure we have enough rows
    while ((ip->view_columns * rows) < slots)
	rows++;

    wmIconPanelSlot **oldp = ip->row;
    int old_rows = ip->rows;
    int old_columns = ip->columns;
    ip->row = NULL;
    ip->rows = 0;
    ip->columns = ip->view_columns;
    for (int i = 0; i < rows; i++)
	newIconPanelRow(ip);
    int orow = 0;
    int ocol = 0;
    for (int row = 0; row < ip->rows; row++)
    {
	for (int col = 0; col < ip->columns; col++)
	{
	    if (orow >= old_rows)
		break;
	    ip->row[row][col] = oldp[orow][ocol];
	    if (ip->row[row][col].used)
		ip->row[row][col].wp->set_ipsp(&ip->row[row][col]);
	    if (++ocol >= old_columns)
	    {
		++orow;
		ocol = 0;
	    }
	}
    }

    // free up the old icon panel stuff
    for (i = 0; i < old_rows; i++)
	free((char *)oldp[i]);
    if (oldp)
	free((char *)oldp);
    XClearArea(DPY, ip->ob->X_window(), 0,0,0,0, True);
}

void
wmRemoveIcon(
    wmData *wp
    )
{
    int saveColumns;

    if (!wp->oi_icon())
	return;

    // make it invisible
    wp->unmap_icon();
    if (wp->ip())
    {
	// the icon was in an icon panel
	// mark the icon panel slot free
	wp->ipsp()->used = False;
	wp->ipsp()->wp = NULL;

	// if gray icons were on, clear the area
	if (wp->gray_icon())
	    XClearArea(DPY, wp->ip()->ob->X_window(),
		wp->ipsp()->x, wp->ipsp()->y,
		wp->ip()->slot_width, wp->ip()->slot_height, True);

	// if either of the slot_width or slot_height is equal to
	// the size of this icon, we can shuffle things to the new
	// sizes
	wp->ip()->count--;

	// if we need to, hide the thing
	if (wp->ip()->hide && wp->ip()->count == 0)
	{
	    wp->ip()->wp->unmap();
	}

	// if we need to pack the icon panel, let's do it
	if (wp->ip()->pack)
	    wmPackIconPanel(wp->ip());

	int size_changed = wmNewIconPanelSizes(wp->ip());

	wmLayoutIconPanel(wp->ip());

	saveColumns = wp->ip()->maxColumnUsed;
	findMaxColumn(wp->ip());

	if (size_changed || wp->ip()->rows < wp->ip()->view_rows)
	{
	    if (wp->ip()->rows < wp->ip()->view_rows)
		wp->ip()->view_rows = wp->ip()->rows;
	    // wmSizeIconPanel(wp->ip());
	}
	else if (!wp->ip()->scrollBars && (saveColumns != wp->ip()->maxColumnUsed))
	{
	    // wmSizeIconPanel(wp->ip());
	}
	wmSizeIconPanel(wp->ip());

	wp->ip()->bp->set_object_size(wp->ip()->columns, wp->ip()->rows);
	wmUpdateSize(wp->ip()->wp);
	wp->set_ipsp(NULL);
	wp->set_ip(NULL);
    }
    // remove it from icon region
    if (wp->icon_gravity()) {
	IconRegionContents *irp = (IconRegionContents *)wp->get_irp();
	if (irp)
	    irp->removeIcon(wp);
    	wp->clear_icon_gravity();
	wp->clear_irp();
	wmSet__SWM_HINTS(wp);
    }
}

int
wmNewIconPanelSizes(
    wmIconPanel *ip
    )
{
    int row, col;
    int old_width, old_height;

    // find the new slot_width and slot_height
    old_width = ip->slot_width;
    old_height = ip->slot_height;
    if (ip->squeeze)
    {
	ip->slot_width = 0;
	ip->slot_height = 0;
    }
    for (row = 0; row < ip->rows; row++)
    {
	for (col = 0; col < ip->columns; col++)
	{
	    if (!ip->row[row][col].used)
		continue;
	    if ((ip->row[row][col].wp->oi_icon()->space_x() + ip->pad) >
		ip->slot_width)
	    	ip->slot_width = ip->row[row][col].wp->oi_icon()->space_x() +
		    ip->pad;
	    if ((ip->row[row][col].wp->oi_icon()->space_y() + ip->pad) >
		ip->slot_height)
	    	ip->slot_height = ip->row[row][col].wp->oi_icon()->space_y() +
		    ip->pad;
	}
    }
    if (ip->slot_width == 0)
	ip->slot_width = wmDefaultSlotSize;
    if (ip->slot_height == 0)
	ip->slot_height = wmDefaultSlotSize;

    // need to round the sizes up to a multiple of four so the gray 
    // bitmap will line up with each slot
    if (ip->slot_width % 4)
	ip->slot_width += 4 - (ip->slot_width % 4);
    if (ip->slot_height % 4)
	ip->slot_height += 4 - (ip->slot_height % 4);

    if (old_width != ip->slot_width || old_height != ip->slot_height)
	return (True);
    else
	return (False);
}

void
wmLayoutIconPanel(
    wmIconPanel *ip
    )
{
    int row, col;
    int x_pos, y_pos;

#ifdef DEBUG
    if (wmDebug)
	fprintf(dfp, "wmLayoutIconPanel: \"%s\"\n", ip->op->name);
#endif /* DEBUG */
    // now move the icons around
    for (row = 0; row < ip->rows; row++)
    {
	for (col = 0; col < ip->columns; col++)
	{
	    if (ip->row[row][col].used)
	    {
		ip->row[row][col].x = col * ip->slot_width;
		ip->row[row][col].y = row * ip->slot_height;
		x_pos = ((ip->slot_width -
		    ip->row[row][col].wp->oi_icon()->space_x())/2) +
		    col * ip->slot_width;
		y_pos = ((ip->slot_height -
		    ip->row[row][col].wp->oi_icon()->space_y())/2) +
		    row * ip->slot_height;
		ip->row[row][col].wp->oi_icon()->set_loc(x_pos, y_pos);
		wmGrayIcon(ip->row[row][col].wp);
	    }
	}
    }
}

void 
wmSizeIconPanel(
    wmIconPanel *ip
    )
{
    ip->bp->set_units(ip->slot_width, ip->slot_height);
    if (ip->scrollBars) 
    {
	ip->bp->set_view_size(ip->view_columns, ip->view_rows);
	ip->bp->set_object_size(ip->columns, ip->rows);
    }
    else
    {
	ip->view_rows = ip->rows;
	ip->bp->set_view_size(ip->maxColumnUsed+1, ip->rows);
	ip->bp->set_object_size((ip->maxColumnUsed+1), ip->rows);
    }
    wmLayoutPanel(ip->op);
    // if the things has been reparented, we need to call the resize
    // client routine to move the stupid resize corners/bars
    if (ip->wp)
    {
	XSizeHints *sh = ip->wp->size_hints_p();
	sh->flags = PMinSize | PResizeInc | PBaseSize;
	sh->base_width = ip->op->oi->size_x() - (ip->view_columns * ip->slot_width);
	sh->base_height = ip->op->oi->size_y() - (ip->view_rows * ip->slot_height);
	sh->min_width = sh->base_width;
	sh->min_height = sh->base_height;
	sh->width_inc = ip->slot_width;
	sh->height_inc = ip->slot_height;

	wmResizeClient(ip->wp, ip->op->oi->size_x(), ip->op->oi->size_y(), False);
    }
}

void
wmResizeIconPanel(
    wmData *wp
    )
{
    wmIconPanel *ip = wp->myip();
    int row, col;

    // set the new object size
    int pad_width = ip->op->oi->size_x() - ip->bp->size_x();
    int pad_height = ip->op->oi->size_y() - ip->bp->size_y();
    ip->bp->set_size(wp->attr_width() - pad_width, wp->attr_height() - pad_height);

    // now figure out how big the object really is.
    XGetWindowAttributes(DPY, ip->op->oi->outside_X_window(), wp->attr_p());

    ip->view_rows = ip->bp->view_size_y();
    ip->view_columns = ip->bp->view_size_x();
    if (ip->view_rows == 0)
	ip->view_rows = 1;
    if (ip->view_columns == 0)
	ip->view_columns = 1;

    if (ip->fit)
	wmFitIconPanel(ip);

    int old_columns = ip->columns;
    int old_rows = ip->rows;

    // if the view_columns is less than columns do nothing
    if (ip->view_columns > ip->columns || ip->view_rows > ip->rows)
    {
	// save the old pointer and rows and columns
	wmIconPanelSlot **oldp = ip->row;
	int new_rows = ip->rows;
	if (ip->view_rows > ip->rows)
	    new_rows = ip->view_rows;
	if (ip->view_columns > ip->columns)
	    ip->columns = ip->view_columns;

	ip->row = NULL;
	ip->rows = 0;
	for (int i = 0; i < new_rows; i++)
	    newIconPanelRow(ip);
	for (row = 0; row < old_rows; row++)
	{
	    for (col = 0; col < old_columns; col++)
	    {
		ip->row[row][col] = oldp[row][col];
		if (ip->row[row][col].used)
		    ip->row[row][col].wp->set_ipsp(&ip->row[row][col]);
	    }
	}
	// free up the old icon panel stuff
	for (i = 0; i < old_rows; i++)
	    free((char *)oldp[i]);
	if (oldp)
	    free((char *)oldp);
    }

    if (ip->pack)
	wmPackIconPanel(ip);

    findMaxColumn(ip);
    wmLayoutIconPanel(ip);
    wmSizeIconPanel(ip);
}

/***************************************************************
 *
 *  Procedure: 
 *	wmExposeIcon
 *
 *  Function:
 *	Handle an exposure on one of the icon subwindows.  If
 *	we are currently non-iconic, we need to gray out the icon.
 *
 ***************************************************************/

void
wmExposeIcon(
    XExposeEvent *ev,
    wmData *wp
    )
{
    if (ev->count != 0)
	return;

#ifdef DEBUG
    if (wmDebug)
	fprintf(dfp, "wmExposeIcon:\n");
#endif /* DEBUG */

    wmGrayIcon(wp);
}

/***************************************************************
 *
 *  Procedure: 
 *	wmRepaintIconGrid
 *
 *  Function:
 *	Redraw the grid lines in the icon panel
 *
 ***************************************************************/

void
wmRepaintIconGrid(
    XExposeEvent *ev,
    struct wmIconPanel *ip
    )
{
#ifdef DEBUG
    if (wmDebug)
    {
	fprintf(dfp, "wmRepaintIconGrid: %d, %d  (%d x %d)\n",
	    ev->x, ev->y, ev->width, ev->height);
    }
#endif /* DEBUG */

    if (ip->showGrid)
    {
	int xr = ev->x + ev->width;
	int yb = ev->y + ev->height;

	// draw horizontal lines first
	for (int y = -1; y < ip->ob->size_y(); y += ip->slot_height)
	{
	    if (y >= ev->y && y <= yb)
		XDrawLine(DPY, ev->window, wmScr->grayGC, ev->x, y, xr, y);
	}

	// now vertical lines
	for (int x = -1; x < ip->ob->size_x(); x += ip->slot_width)
	{
	    if (x >= ev->x && x <= xr)
		XDrawLine(DPY, ev->window, wmScr->grayGC, x, ev->y, x, yb);
	}

	if (ev->count == 0)
	    grayIcons(ip);
    }
}

static void
grayIcons(
    wmIconPanel *ip
    )
{
    for (int row = 0; row < ip->rows; row++)
    {
	for (int col = 0; col < ip->columns; col++)
	{
	    if (ip->row[row][col].used)
		wmGrayIcon(ip->row[row][col].wp);
	}
    }
}

/***************************************************************
 *
 *  Procedure: 
 *	wmGrayIcon
 *
 *  Function:
 *	Try and gray out an icon.  Note that this probably won't
 *	work well on client supplied icon windows.
 *
 ***************************************************************/

void
wmGrayIcon(
    wmData *wp
    )
{
#ifdef DEBUG
    if (wmDebug)
    {
	fprintf(dfp, "wmGrayIcon:\n");
    }
#endif /* DEBUG */

    if (wp->state() == IconicState)
	return;

    if (!wp->gray_icon())
	return;

    unsigned long gcm;
    XGCValues gcv;
    Window w;
    int x, y, width, height;

    if (wp->ip())
    {
	w = wp->ip()->ob->X_window();
	x = wp->ipsp()->x;
	y = wp->ipsp()->y;
	width = wp->ip()->slot_width;
	height = wp->ip()->slot_height;
	gcv.foreground = wp->ip()->ob->fg_pixel();
	gcv.background = wp->ip()->ob->bkg_pixel();
    }
    else
    {
	w = wp->oi_icon()->X_window();
	x = y = 0;
	width = wp->oi_icon()->size_x();
	height = wp->oi_icon()->size_y();
	gcv.foreground = wp->oi_icon()->fg_pixel();
	gcv.background = wp->oi_icon()->bkg_pixel();
    }

    gcm = GCForeground | GCBackground |
	GCTileStipXOrigin | GCTileStipYOrigin;
    gcv.ts_x_origin = 0;
    gcv.ts_y_origin = 0;

#ifdef DEBUG
    if (wmDebug > 1)
	fprintf(dfp, "  paint at %d, %d    origin at %d, %d\n",
	    x, y, gcv.ts_x_origin, gcv.ts_y_origin);
#endif /* DEBUG */
    XChangeGC(DPY, wmScr->gray3GC, gcm, &gcv);
    XFillRectangle(DPY, w, wmScr->gray3GC, x, y, width, height);
}

void
wmSetWM_STATE(
    wmData *wp,
    int state
    )
{
    unsigned long data[2];

    if (wp->has_client())
    {
	wp->set_state(state);
	if (!wp->mine()) {
		data[0] = (unsigned long) state;
		data[1] = (unsigned long) (wp->oi_icon() ? wp->oi_icon()->outside_X_window() : None);
	
		XChangeProperty(DPY, wp->window(), WM_STATE, WM_STATE, 32, PropModeReplace, (unsigned char *) data, 2);
	}

	if (state == NormalState)
	    wp->set_normalized();

	// now set the internal SWMHints on the window so I can restart
	// properly if restartPreviousState is set
	wp->set_initial_state(state);
	wmSet__SWM_HINTS(wp);
    }
}

Bool
wmGetWM_STATE(
    wmData *wp)
{
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytesafter;
    SWMHints *hints = NULL;

    if (XGetWindowProperty (DPY, wp->window(), __SWM_HINTS,0,(sizeof(wp->myhints())+3)/4,
	False, __SWM_HINTS, &actual_type, &actual_format, &nitems, &bytesafter,
			    (unsigned char **) &hints) != Success || !hints)
      return (False);

    wp->set_myhints(*hints);
    XFree ((char *) hints);
    return (True);
}

wmIconPanel::wmIconPanel()
{
    slot_width = wmDefaultSlotSize;
    slot_height = wmDefaultSlotSize;
    rows = 0;
    columns = 0;
    view_rows = 0;
    view_columns = 0;
    count = 0;
    pad = 2;
    pack = False;
    fit = False;
    showGrid = False;
    squeeze = False;
    wp = NULL;
    row = NULL;
}
