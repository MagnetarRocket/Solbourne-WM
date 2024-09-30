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
 * $Id: region.H,v 1.12 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Icon definitions
 *
 * Author: Kelly Rise
 *
 ******************************************************************************/

#ifndef REGION_H
#define REGION_H

#include "wmdata.H"
#include "list.H"
#include "panel.H"

enum	direction	{North, South, East, West};
enum	axis		{X, Y};
enum	shuffle_t	{Pack, Small, Large};

class bounds 
{
	int	width;
	int	height;
	int	x_start;
	int	y_start;

	bounds(direction, direction, int, int, int, int);
	bounds(bounds *, int, int);

friend	class IconRegionContents;
};

class	IconRegion 
{
	char		*myname;	// object, class or client name
	Bool		allscreens;	// true if region is relative
	direction	x_axis;		// which direction to travel on x axis
	direction	y_axis;		// which direction to travel on y axis
	axis		primary;	// x or y is the primary axis
	int		pad;		// spacing between icons in region
	Bool		follow_client;	// icons folow clients on desktop moves
	Bool		pack_immediate;	// fill holes as they occur
	shuffle_t	shuffle;	// sort key for reshuffling region
	int		overlay;	// overlay factor for filled regions
	wmList		contentsList;	// each element is a different logical screen

	void		setAxis(char *);		// used by constructors
	direction	primary_axis()	{return(primary == X ? x_axis : y_axis);}
	void		get_resources(Bool =False);

public:
	IconRegion(char *, char *);
	IconRegion(char *, char *, wmGeometry *);

	Bool	addIcon(wmData *);
	void	dumpIconRegions();
	void	expand(int, int, int, int);
	Bool	followClient()	{ return(follow_client); }
	Bool	packImmediate()	{ return(pack_immediate); }
	char *	name()		{ return (myname); }
	void	restartIcon(wmData *, int);

friend	class IconRegionContents;
};

class IconRegionContents 
{
	int	screen_x;	// x start of the logical screen this region lives on
	int	screen_y;	// y start of the logical screen this region lives on
	int	x_next;		// where to start the next icon on the x axis
	int	y_next;		// where to start the next icon on the y axis
	int	x_left;		// available space left on x axis
	int	y_left;		// available space left on y axis
	int	rowheight;	// height (width) of largest icon in row (column)
	int	numoverlay;
	bounds	*cbp;		// current bounds pointer
	IconRegion	*irp;	// the 'mother' region
	wmList	boundryList;	// list of region boundries
	wmList	*iconList;	// list of regular icons in this region
	wmList	*stickyList;	// list of sticky icons in this region

	int	stickyIndex;		// highest order # on restart list
	int	iconIndex;		// highest order # on restart list

	IconRegionContents(int, int, IconRegionContents *,IconRegion *);
	IconRegionContents(int, int, wmGeometry *, IconRegion *, Bool);
	int	width()		{ return(cbp->width); }
	int	height()	{ return(cbp->height); }
	int	x_start()	{ return(cbp->x_start); }
	int	y_start()	{ return(cbp->y_start); }
	void	reset_x()	{ x_left = width(); x_next = x_start(); }
	void	reset_y()	{ y_left = height(); y_next = y_start(); }

	void	reshuffle(int, int, int, int, int, int, Bool =True);
	void	packList(wmList *, Bool =True);

	void	rmSticky(wmData *);

	void	placeIcon(wmData *, Bool =True);
	void	insertIcon(wmData *);
	void	insertIcon(wmData *, int);

public:
	Bool	followClient()		{ return(irp->follow_client); }
	IconRegion *	mother()	{ return irp; }
	Bool	onScreen(long x, long y)	{return (((x >= screen_x && x < screen_x +wmScr->width) || x < 0 && screen_x == 0) && ((y >= screen_y && y < screen_y + wmScr->height) || y < 0 && screen_y == 0));}
	void	removeIcon(wmData *);
	void	reshuffle(Bool =False, Bool =True);
	void	stickIcon(wmData *);
	void	unstickIcon(wmData *);

friend class IconRegion;
};

struct wmRegionKid {
	char *name;
	char *type;
	struct wmGeometry *geom;

	wmRegionKid(char *, char*, struct wmGeometry *);

};

struct orderedItem {
	wmData 	*data;
	int	index;
};

#endif /* REGION_H */
