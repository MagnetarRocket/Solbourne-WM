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

#include <string.h>
#include <OI/oi.H>
#include "main.H"
#include "swm.H"
#include "wmdata.H"
#include "panel.H"
#include "quarks.H"
#include "atoms.H"
#include "region.H"
void wmMakeIcon(wmData *, int, int, Bool);

IconRegion *findIconRegion(char *);
//
//	IconRegion::IconRegion
//	create a new icon region
//
//
IconRegion::IconRegion(char *gravity, char *name)
{
	int	x, y;		// for creating region contents
	Bool	first = True;
	wmGeometry	geom;

	// set region specifications
	myname = name;
	allscreens = True;

	// resource default values
	follow_client = True;
	overlay = 10;
	pack_immediate = True;
	pad = 1;
	shuffle = Pack;

	// get resources
	get_resources();

	setAxis(gravity);

	geom.x = 0;
	geom.y = 0;
	geom.width = wmScr->width;
	geom.height = wmScr->height;

	if (wmScr->vdt) {
	// set up icon region contents for each logical screen on vdt
		x = 0;
		y = 0;
		while (x < wmScr->vwidth && y < wmScr->vheight) {
			geom.x = x;
			geom.y = y;
			contentsList.append((ent) new IconRegionContents(
				x, y, &geom, this, first));
			x += wmScr->width;
			if (x >= wmScr->vwidth && y < wmScr->vheight) {
				x = 0;
				y += wmScr->height;
			}
			first = False;
		}
	}
	else
		contentsList.append((ent) new IconRegionContents(0, 0, 
			&geom, this, first));
}

IconRegion::IconRegion(char *type, char *name, wmGeometry *geom)
{
	int	screen_x, screen_y;
	Bool	first = True;

	myname = name;
	// type should be an enum
	if (!strcmp("absolute", type))
		allscreens = False;
	else if (!strcmp("relative", type))
		allscreens = True;
	else 
	{
		printf("swm: illegal icon region type %s, using relative\n", type);
		allscreens = True;
	}


	// resource default values
	follow_client = True;
	overlay = 10;
	pack_immediate = True;
	pad = 1;
	shuffle = Pack;

	// get resources
	get_resources(True);

	// if no vdt the counting loops involving width/height only run once.
	// set up region contents
	if (!allscreens) 
	{
		screen_x = screen_y = 0;
		if (geom->sign_x == 1) {
			while (geom->x < screen_x && screen_x + wmScr->width <= geom->x)
				screen_x += wmScr->width;
			geom->x += screen_x;
		}
		else {
			geom->x = screen_x + wmScr->width - geom->width - geom->x;
		}
		if (geom->sign_y == 1) {
			while (geom->y < screen_y && screen_y + wmScr->height <= geom->y)
				screen_y += wmScr->height;
			geom->y += screen_y;
		}
		else { 
			geom->y = screen_y + wmScr->height - geom->height - geom->y;
		}
		contentsList.append((ent) new IconRegionContents(screen_x, screen_y, 
			geom, this, first));
	}
	else 
	{
		screen_x = screen_y = 0;
		// ASSUMPTION -  relative regions are specified relative to 0,0
		if (geom->sign_x == -1)
			geom->x = wmScr->width - geom->width - geom->x;
		if (geom->sign_y == -1)
			geom->y = wmScr->height - geom->height - geom->y;

		while (screen_x < wmScr->vwidth && screen_y < wmScr->vheight) {
			contentsList.append((ent) new IconRegionContents(screen_x, screen_y,
				geom, this, first));
			screen_x += wmScr->width;
			geom->x += wmScr->width;
			if (screen_x >= wmScr->vwidth && screen_y < wmScr->vheight) {
				geom->x -= screen_x;
				screen_x = 0;
				screen_y += wmScr->height;
				geom->y += wmScr->height;
			}
			first = False;
		}
	}
}

IconRegionContents::IconRegionContents(int x, int y, wmGeometry *geom, 
	IconRegion *mother, Bool first)
{
	boundryList.init();
	iconList = new wmList;
	if (first)
		stickyList = new wmList;
	else 
		stickyList = ((IconRegionContents *)(mother->contentsList.first()))->stickyList;

	irp = mother;
	boundryList.append((ent)new bounds(irp->x_axis, irp->y_axis, geom->x, 
		geom->y, geom->width, geom->height));
	
	cbp = (bounds *)boundryList.first();
	screen_x = x;
	screen_y = y;
	x_next = cbp->x_start;
	y_next = cbp->y_start;
	x_left = cbp->width;
	y_left = cbp->height;
	numoverlay = 0;
	rowheight = 0;			// should this be in bounds?
	stickyIndex = -1;
	iconIndex = -1;
}

//
// only used by expand
//
IconRegionContents::IconRegionContents(int x, int y, IconRegionContents *cp, IconRegion *mother)
{
	bounds *bp;
	boundryList.init();
	iconList = new wmList;
	stickyList = cp->stickyList;

	irp = mother;
	// so it can handle multiple geometries if ever implemented
	for (bp = (bounds *)cp->boundryList.first(); bp != NULL;
		bp = (bounds *)cp->boundryList.next())
		boundryList.append((ent)new bounds(bp, x, y));
	
	cbp = (bounds *)boundryList.first();
	screen_x = x;
	screen_y = y;
	x_next = cbp->x_start;
	y_next = cbp->y_start;
	x_left = cbp->width;
	y_left = cbp->height;
	numoverlay = 0;
	rowheight = 0;			// should this be in bounds?
	stickyIndex = -1;
	iconIndex = -1;
}

bounds::bounds(direction x, direction y, int x0, int y0, int w, int h)
{
	width = w;
	height = h;

	switch (x) {
	case East:
		x_start = x0 + width - 1;
		break;
	case West:
		x_start = x0;
		break;
	}
	switch (y) {
	case North:
		y_start = y0;
		break;
	case South:
		y_start = y0 + height - 1;
		break;
	}
}

bounds::bounds(bounds *bp, int x, int y)
{
	width = bp->width;
	height = bp->height;
	x_start = bp->x_start + x;
	y_start = bp->y_start + y;
}

void
IconRegion::setAxis(char *gravity)
{
	char *p, *string;

	p = strtok(gravity, "\t ");
	string = p;

	while (*p) {
		if (isalpha(*p)) {
			if (isupper(*p))
				*p = tolower(*p);
		}
		else 
			string = NULL;
		p++;
	}

	if (string) {
		if (!strncmp(string, "northwest", 9)) {
			primary = Y;
			y_axis = North;
			x_axis = West;
		}
		else if (!strncmp(string, "northeast", 9)) {
			primary = Y;
			y_axis = North;
			x_axis = East;
		}
		else if (!strncmp(string, "southwest", 9)) {
			primary = Y;
			y_axis = South;
			x_axis = West;
		}
		else if (!strncmp(string, "southeast", 9)) {
			primary = Y;
			y_axis = South;
			x_axis = East;
		}
		else if (!strncmp(string, "westnorth", 9)) {
			primary = X;
			x_axis = West;
			y_axis = North;
		}
		else if (!strncmp(string, "westsouth", 9)) {
			primary = X;
			x_axis = West;
			y_axis = South;
		}
		else if (!strncmp(string, "eastnorth", 9)) {
			primary = X;
			x_axis = East;
			y_axis = North;
		}
		else if (!strncmp(string, "eastsouth", 9)) {
			primary = X;
			x_axis = East;
			y_axis = South;
		}
		else {
			printf("swm: illegal icon gravity specification, using WestSouth\n");
			primary = X;
			x_axis = West;
			y_axis = South;
		}
	}
}

// add a new icon to the region
Bool
IconRegion::addIcon(wmData *wp)
{
	Bool	success = False;
	IconRegionContents	*cp;	// current contents pointer
	cp = (IconRegionContents *)contentsList.first();

	// if the icon is sticky & the region is mixed
	// it will have to be reshuffled, so do it first to avoid 
	// an placeIcon call
	if (wp->sticky() && cp->iconList->count)
	{
		cp->insertIcon(wp);
		cp->reshuffle();
		success = True;
	}
	else
	{
		// find a contents region on the same screen as the client
		for (cp = (IconRegionContents *)contentsList.first(); cp != NULL;
				cp = (IconRegionContents *)contentsList.next())
		{
			if (cp->onScreen(wp->oi_frame()->loc_x(), 
					wp->oi_frame()->loc_y())) 
			{
				cp->placeIcon(wp);
				cp->insertIcon(wp);
				success = True;
			}
		}
	}
	return(success);
}


// figure out icons x & y coordinates
//
void
IconRegionContents::placeIcon(wmData *wp, Bool set_location)
{
	int x = 0;
	int y = 0;
	int iwidth, iheight;

	iwidth = wp->oi_icon()->space_x();
	iheight = wp->oi_icon()->space_y();
	
	// take care of icons larger than region 

	// icons that fit in region
	// check for overlays, next boundries or new rows
	if (iwidth > x_left) {
		if (irp->primary == X) {
			if (irp->x_axis == West) 
				x_next = cbp->x_start + (irp->overlay * numoverlay);
			else
				x_next = cbp->x_start - (irp->overlay * numoverlay);
			x_left = cbp->width - (irp->overlay * numoverlay);
		}
		else {
			x_next = cbp->x_start;
			x_left = cbp->width;
		}

		switch (irp->primary_axis()) {
		case North:
			// overlay
			numoverlay++;
			y_next = cbp->y_start + (irp->overlay * numoverlay);
			y_left = cbp->height - (irp->overlay * numoverlay);
			if (iheight > y_left) 
				reset_y();
			break;
		case South:
			// overlay
			numoverlay++;
			y_next = cbp->y_start - (irp->overlay * numoverlay);
			y_left = cbp->height - (irp->overlay * numoverlay);
			if (iheight > y_left) 
				reset_y();
			break;
		case East:
		case West:
			// new row
			if (irp->y_axis == North)
				y_next += rowheight + irp->pad;
			else
				y_next -= rowheight + irp->pad;
			y_left -= rowheight + irp->pad;
			rowheight = iheight;
			break;
		}
	}
	if (iheight > y_left) {
		if (irp->primary == Y) {
			if (irp->y_axis == North)
				y_next = cbp->y_start + (irp->overlay * numoverlay);
			else
				y_next = cbp->y_start - (irp->overlay * numoverlay);
			y_left = cbp->height - (irp->overlay * numoverlay);
		}
		else {
			y_next = cbp->y_start;
			y_left = cbp->height;
		}

		switch (irp->primary_axis()) {
		case West:
			// overlay
			numoverlay++;
			x_next = cbp->x_start + (irp->overlay * numoverlay);
			x_left = cbp->width - (irp->overlay * numoverlay);
			if (iwidth > x_left) 
				reset_x();
			break;
		case East:
			// overlay
			numoverlay++;
			x_next = cbp->x_start - (irp->overlay * numoverlay);
			x_left = cbp->width - (irp->overlay * numoverlay);
			if (iwidth > x_left) 
				reset_x();
			break;
		case North:
		case South:
			// new column
			if (irp->x_axis == West)
				x_next += rowheight + irp->pad;
			else 
				x_next -= rowheight + irp->pad;
			x_left -= rowheight + irp->pad;
			rowheight = iwidth;
			// if the icon won't fit in the new
			// column need to overlay instead
			if (iwidth > x_left) {
				numoverlay++;
				x_next = cbp->x_start;
				x_left = cbp->width;
				if (irp->y_axis == North)
					y_next = cbp->y_start + (irp->overlay * numoverlay);
				else
					y_next = cbp->y_start - (irp->overlay * numoverlay);
				y_left = cbp->height - (irp->overlay * numoverlay);
				if (iheight > y_left)
					reset_y();
			}
			break;
		}
	}
	// x_next and y_next are set such that the icon will fit
	// now set the upper left corner positions
	if (irp->x_axis == West)
		x = x_next;
	else
		x = x_next - iwidth;
	if (irp->y_axis == North)
		y = y_next;
	else
		y = y_next - iheight;
	
	// now update next, left ...
	switch (irp->primary_axis()) {
	case North:
		y_next += iheight + irp->pad;
		y_left -= iheight + irp->pad;
		if (rowheight < iwidth)
			rowheight = iwidth;
		break;
	case South:
		y_next -= iheight + irp->pad;
		y_left -= iheight + irp->pad;
		if (rowheight < iwidth)
			rowheight = iwidth;
		break;
	case East:
		x_next -= iwidth + irp->pad;
		x_left -= iwidth + irp->pad;
		if (rowheight < iheight)
			rowheight = iheight;
		break;
	case West:
		x_next += iwidth + irp->pad;
		x_left -= iwidth + irp->pad;
		if (rowheight < iheight)
			rowheight = iheight;
		break;
	}

	// if i'm adding a sticky icon this has been 
	// done already, setting them relative to all
	// regions makes them go off the desktop
	if (set_location)
	{
		wp->set_icon_x(x);
		wp->set_icon_y(y);
		wp->oi_icon()->set_associated_object(wp->root(),x,y);
		wp->move_icon(x, y);
	}
}

/* 
 * update the icon lists & swmhints, no mapping is done
 */
void
IconRegionContents::insertIcon(wmData *wp)
{
	OI_state	state = wp->oi_icon()->state();

	if (wp->sticky())
	{
		stickyIndex++;
		wp->set_gravity_order(stickyIndex);
	    	stickyList->append((ent)wp);
	}
	else 
	{
		iconIndex++;
		wp->set_gravity_order(iconIndex);
	    	iconList->append((ent)wp);
	}
	
	wp->clear_root_icon();
	wp->set_icon_gravity();
	wmSet__SWM_HINTS(wp);
	if (state == OI_ACTIVE || state == OI_ACTIVE_NOT_DISPLAYED)
		wp->map_icon();
	wp->set_irp((long)this);
}

/* 
 * update the icon lists & swmhints, no mapping is done
 */
void
IconRegionContents::insertIcon(wmData *wp, int index)
{
	wmData 	*tmp;
	Bool	added = False;

	if (wp->sticky())
	{
		if (stickyIndex < index )
		{
	    		stickyList->append((ent)wp);
			added = True;
		}
		else if (index < ((wmData *)(stickyList->first()))->gravity_order())
		{
			stickyList->insert((ent)wp);
			added = True;
		}
		else
		{
			for (tmp = (wmData *)stickyList->first(); tmp != NULL;
				tmp = (wmData *)stickyList->next())
			{
				if (tmp->gravity_order() > index)
				{
					added = stickyList->insert(tmp, wp);
					break;
				}
			}
		}
		if (!added)
			stickyList->append((ent)wp);
		if (index > stickyIndex)
			stickyIndex = index;
	}
	else 
	{
		if (iconIndex < index )
		{
	    		iconList->append((ent)wp);
			added = True;
		}
		else if (index < ((wmData *)(iconList->first()))->gravity_order())
		{
			iconList->insert((ent)wp);
			added = True;
		}
		else
		{
			for (tmp = (wmData *)iconList->first(); tmp != NULL;
				tmp = (wmData *)iconList->next())
			{
				if (tmp->gravity_order() > index)
				{
					added = iconList->insert(tmp, wp);
					break;
				}
			}
		}
		if (!added)
			iconList->append((ent)wp);
		if (index > iconIndex)
			iconIndex = index;
	}
	
	wp->clear_root_icon();
	wp->set_icon_gravity();

	wp->set_irp((long)this);
	reshuffle(False, False);
}


// sort the region based on shuffle preference (Pack Small Large).
// elminates holes due to dead or moved clients.  clusters sticky
// icons at the start of the region. 
void
IconRegionContents::reshuffle(Bool force, Bool set_order)
{
	IconRegionContents *cp;

	// get around a sticky problem
	if ((force || stickyList->count != 0) 
		&& mother()->contentsList.first() != (ent)this)
	{
		cp = (IconRegionContents *)mother()->contentsList.first();
		cp->reshuffle();
		return;
	}

	// reset all location values to original state
	cbp = (bounds *)boundryList.first();
	x_next = x_start();
	y_next = y_start();
	x_left = width();
	y_left = height();
	numoverlay = 0;
	rowheight = 0;
	if (set_order)
	{
		stickyIndex = -1;
		iconIndex = -1;
	}

	// do the sticky icons first - all sticky icons in region
	// are in stickyList
	if (stickyList->count) {
		switch (irp->shuffle) {
		case Pack:
			packList(stickyList, set_order);
			break;
		case Small:
		case Large:
			break;
		}
	}

	// now do similar regions containing the sticky icons, the sticky 
	// list is sorted, but iconList needs to be sorted for each region.
	if (stickyList->count || force ) {
		for (cp = (IconRegionContents *)irp->contentsList.first(); 
			cp != NULL; 
			cp = (IconRegionContents *)irp->contentsList.next()) {
			if (cp != this)
				cp->reshuffle(x_next - x_start(), y_next - y_start(), 
					x_left, y_left, rowheight, numoverlay);
		}
	}
	// do the other icons in this region
	if (iconList->count) {
		switch (irp->shuffle) {
		case Pack:
			packList(iconList, set_order);
			break;
		case Small:
		case Large:
			break;
		}
	}
}

// reshuffle the region, sticky list is already sorted 
void
IconRegionContents::reshuffle(int xnext, int ynext, int xleft, int yleft, 
	int rh, int no, Bool set_order)
{
	// reset all location values to given state
	// NOTE! this bound pointer may not work
	// when multiple bounds are implemented
	cbp =  (bounds *)boundryList.first();
	x_next = xnext + x_start();
	y_next = ynext + y_start();
	x_left = xleft;
	y_left = yleft;
	rowheight = rh;
	numoverlay = no;
	if (set_order)
		iconIndex = -1;

	// sort the icon list
	if (iconList->count) {
		switch (irp->shuffle) {
		case Pack:
			packList(iconList, set_order);
			break;
		case Small:
		case Large:
			break;
		}

	}
}

// update the list, eliminiting icons which don't have gravity anymore.
// list order is unchanged. 
// set icons new x & y loc's with previous OI_state
void
IconRegionContents::packList(wmList *list, Bool set_order)
{
	wmData		*wp;
	OI_state	state;

	for (wp = (wmData *)list->first(); wp != NULL; wp = (wmData *)list->next()) 
	{
		if (wp->icon_gravity()) {
			state = wp->oi_icon()->state();
			placeIcon(wp);
			if (wp->sticky() && set_order)
			{
				stickyIndex++;
				wp->set_gravity_order(stickyIndex);
			}
			else if (!wp->sticky() && set_order)
			{
				iconIndex++;
				wp->set_gravity_order(iconIndex);
			}
			wmSet__SWM_HINTS(wp);
			if (state == OI_ACTIVE || state == OI_ACTIVE_NOT_DISPLAYED)
			{
				wp->map_icon();
			}
		}
		else list->rm((ent)wp);
	}
}

void
IconRegionContents::removeIcon(wmData *wp)
{
	if (wp->sticky())
		stickyList->rm((ent)wp);
	else
		iconList->rm((ent)wp);

	// if its packImmediate reshuffle() will eliminate it from
	// all regions it lives in
	if (mother()->packImmediate())
		reshuffle(wp->sticky());
}

// an existing icon becomes sticky.
// decide if it should move to a different 
// region. 
void
IconRegionContents::stickIcon(wmData *wp)
{
	IconRegion *newregion = NULL;

	RM->set_stack_ptr(wmScr->rm_stack);
	RM->push(wp->wclass_class(), wp->wclass_class());
	RM->push(wp->wclass_name(), wp->wclass_name());
	RM->pushq(wmQuarks->stickyName(), wmQuarks->stickyClass());
	
	wp->set_icon_region(RM->get_resourceq(wmQuarks->iconRegionName(), wmQuarks->iconRegionClass()));

	if (wp->icon_region())
		newregion = findIconRegion(wp->icon_region());
	if (!newregion)
		newregion = findIconRegion("sticky");
		
	// remove from iconList whether a new region or not
	iconList->rm((ent)wp);

	if (newregion) {
		// if it went to a new region i have holes to fill
		if (newregion != mother() && mother()->packImmediate())
			reshuffle();
		newregion->addIcon(wp);
	}
	else
	{
		// no region named and no sticky region so 
		// keep it in this region
		mother()->addIcon(wp);
	}
}

// an existing icon becomes unsticky.
// remove it from all stickyLists, put it on the
// correct iconList 
void
IconRegionContents::unstickIcon(wmData *wp)
{
	IconRegion	*newregion = NULL;

	RM->set_stack_ptr(wmScr->rm_stack);
	RM->push(wp->wclass_class(), wp->wclass_class());
	RM->push(wp->wclass_name(), wp->wclass_name());
	wp->set_icon_region(RM->get_resourceq(wmQuarks->iconRegionName(), wmQuarks->iconRegionClass()));

	if (wp->icon_region())
		newregion = findIconRegion(wp->icon_region());
	if (!newregion)
		newregion = findIconRegion(wp->wclass_name());
	if (!newregion)
    	    	newregion = findIconRegion(wp->wclass_class());
	if (!newregion)
    	    	newregion = findIconRegion("Default");

	// oi_icon()->[xy]_loc() are translated already
	stickyList->rm((ent)wp);
	
	// if packImmediate need to reshuffle all regions, 
	// even if stickyList count is 0 (may have removed last sticky)
	if (mother()->packImmediate())
		reshuffle(True);

	// put it in the right region
	newregion->addIcon(wp);
}

// create new icon region contents because panner grew
void
IconRegion::expand(int oldx, int oldy, int newx, int newy)
{
	wmData	*wp;
	int	x, y;
	Bool	first = True;
	IconRegionContents	*cp, *ncp;

	if (!allscreens)
		return;

	cp = (IconRegionContents *)contentsList.first();
	if (oldx % wmScr->width) 
		oldx = ((oldx/wmScr->width + 1) * wmScr->width);
	if (oldy % wmScr->height) 
		oldy = ((oldy/wmScr->height + 1) * wmScr->height);

	// set up icon region contents for each new logical screen on vdt
	// and add the sticky icons
	x = oldx;
	y = 0;
	while (x < newx && y < oldy) 
	{
		ncp = new IconRegionContents(x, y, cp, this);
		contentsList.append((ent)ncp);
		x += wmScr->width;
		if (x >= newx && y < oldy)
		{
			x = oldx;
			y += wmScr->height;
		}
	}
	x = 0;
	y = oldy;
	while (x < newx && y < newy) 
	{
		ncp = new IconRegionContents(x, y, cp, this);
		contentsList.append((ent)ncp);
		x += wmScr->width;
		if (x >= newx && y < newy)
		{
			x = 0;
			y += wmScr->height;
		}
	}
	// account for any sticky icons
	if (cp->stickyList->count)
		cp->reshuffle();
}

wmRegionKid::wmRegionKid(
    char *t,
    char *n,
    struct wmGeometry *g
    )
{
    type = t;
    name = n;
    //!!!!!! wmGeometry() should be def'd with default values !!!!
    geom = new wmGeometry(g->sign_x,g->x,g->sign_y, g->y, g->width,g->height, False);
}

void
IconRegion::dumpIconRegions()
{
	IconRegionContents  *cp;
	wmData *wp;

		printf("IconRegion %s:\n", name());
		for (cp = (IconRegionContents *)contentsList.first();
			cp != NULL; 
			cp = (IconRegionContents *)contentsList.next())
		{
			printf("contents %dx%d+%d+%d\n", cp->width(), 
				cp->height(), cp->x_start(), cp->y_start());
			printf("%lx\tstickyList:\n", cp);
			for (wp = (wmData *)cp->stickyList->first();
				wp != NULL; 
				wp = (wmData *)cp->stickyList->next())
			{
				printf("\tx = %d, y = %d\t %s\tonScreen = %d\n",
					wp->icon_x(), wp->icon_y(),
					wp->wclass_class(),
					cp->onScreen(wp->icon_x(), wp->icon_y()));
	
			}
			printf("%lx\ticonList:\n", cp);
			for (wp = (wmData *)cp->iconList->first();
				wp != NULL; 
				wp = (wmData *)cp->iconList->next())
			{
				printf("\tx = %d, y = %d\t %s\tonScreen = %d\n",
					(int)wp->icon_x(),
					(int)wp->icon_y(),
					wp->wclass_class(),
					cp->onScreen(wp->icon_x(), wp->icon_y()));
			}
		}
}

// insert at correct location
void
IconRegion::restartIcon(wmData *wp, int index)
{
	IconRegionContents	*cp;
	struct orderedItem 	* temp = new orderedItem;

	temp->data = wp;
	temp->index = index;
	// find the right contents
	for (cp = (IconRegionContents *)contentsList.first();
			cp != NULL; cp = (IconRegionContents *)contentsList.next())
		if (cp->onScreen(wp->icon_x(),wp->icon_y()))
			break;
	
	cp->insertIcon(wp, index);
}

void
IconRegion::get_resources(Bool getGravity)
{
	char	*ptr;
	char	*c;
	// get resources 
	RM->pushq(wmQuarks->iconRegionName(), wmQuarks->iconRegionClass());
	RM->push(myname, myname);

    	ptr = RM->get_resourceq(wmQuarks->padName(), wmQuarks->padClass());
	if (ptr) 
		pad = atoi(ptr);

	ptr = RM->get_resourceq(wmQuarks->overlayName(), wmQuarks->overlayClass());
	if (ptr) 
		overlay = atoi(ptr);

	ptr = RM->get_resourceq(wmQuarks->followClientName(), wmQuarks->followClientClass());
	if (ptr)
		follow_client = (ptr[0] == 't' || ptr[0] == 'T');

	ptr = RM->get_resourceq(wmQuarks->packImmediateName(), wmQuarks->packImmediateClass());
	if (ptr)
		pack_immediate = (ptr[0] == 't' || ptr[0] == 'T');

	ptr = RM->get_resourceq(wmQuarks->reshuffleName(), wmQuarks->reshuffleClass());
	if (ptr) {
		c = ptr;
		while (*c) {
			if (isupper(*c))
				*c = tolower(*c);
			c++;
		}
		if (!strcmp("pack", ptr))
			shuffle = Pack;
		else if (!strcmp("small", ptr))
			shuffle = Small;
		else if (!strcmp("large", ptr))
			shuffle = Large;
	}

	if (getGravity) 
	{
		ptr = RM->get_resourceq(wmQuarks->gravityName(), wmQuarks->gravityClass());
		if (ptr)
			setAxis(ptr);
		else
			setAxis("southwest");
	}
	RM->pop();
	RM->pop();
}
