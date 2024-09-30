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
 * $Id: object.H,v 9.8 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Basic object definitions
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/


#ifndef OBJECT_H
#define OBJECT_H

enum wmSt
{
    wmNoSpace	= 0x01,
    wmIsMapped	= 0x02,
    wmNotUsed	= 0x04
};

enum wmObjectState
{
    wmMapped		= wmIsMapped,		  // visible and uses up space
    wmUnmapped		= 0,			  // not visible and uses space
    wmMappedNoSpace	= wmIsMapped | wmNoSpace, // visible and uses no space
    wmUnmappedNoSpace	= wmNoSpace,		  // not visible and no space
    wmUnused		= wmNotUsed | wmNoSpace,  // not used and uses no space
    wmUnknwon		= -1
};

	// forward reference declarations
	struct wmObjectData;
	struct wmPanelInfo;
	struct wmButtonInfo;
	struct wmTextInfo;
	struct wmPanelKid;
	struct wmData;
	struct wmBindings;


#define MAX_QUARKS 5
class wmObject
{
public:
    int	type;				// type of the object
    char *name;				// name of the thing
    XrmQuark quark;			// full quarkified name
    XrmQuark quarks[MAX_QUARKS];	// quarkified name, up to MAX_QUARKS seperated by '.' characters
    short numQuarks;			// number of names in the quark array
    wmObjectState state;		// state of the object
    struct wmObjectData *odp;		// pointer to the object data
    OI_d_tech *oi;			// pointer to the oi object
    char expanded;			// boolean, object has been expanded
    char client;			// boolean, is this the client
    char wname;				// boolean, is this the name object
    char iname;				// boolean, is this the icon name object
    char iimage;			// boolean, is this the icon image obj
    char icons;				// boolean, is this in icon panel
    char size;				// boolean, this is the size object
    char root;				// this is the root panel

    // the following union is for the specific objects
    union
    {
	struct wmPanelInfo *p;
	struct wmButtonInfo *b;
	struct wmTextInfo *t;
    } u;

    wmObject(int, char *, XrmQuark);
};

// this data structure will be attatched to each OI object
struct wmObjectData
{
    wmObject *op;			// a pointer to the object
    struct wmPanelKid *kp;		// a pointer to the kid structure
    struct wmData *wp;			// pointer to window data structure
    wmObjectState state;		// what state is it in
    struct wmBindings *bps;		// list of bindings
    OI_d_tech *parent;			// parent object, used for pop-up menus
    PIXEL focusForeground;
    PIXEL focusBackground;
    PIXEL unfocusForeground;
    PIXEL unfocusBackground;
    PIXEL focusBorder;
    PIXEL unfocusBorder;
    Pixmap pixmap;
    int pixmapWidth;
    int pixmapHeight;
    int sweep;				// this object's index in the sweep list
    int pad;				// padding for this panel
    char gravity;			// TOP or BOTTOM
    char inAnIcon;			// object is part of an icon
    char handlePress;
    char handleKey;
    char handleDrop;
    char frame;				// this is the frame object
    char icon;				// this is the icon object
    char shape;				// is this object shaped
    Pixmap shapePixmap;			// the shape pixmap for this object
    Window shapeWindow;			// mask window used during shaping
    char *hs;				// help state pointer
    short unsigned hsSize;		// size of the help state structure 

    wmObjectData(wmObject *);
};

extern wmObject *wmCreateObject(int, char *);
extern void wmExpandObjects();
extern OI_d_tech *wmInstantiateObject(wmObject *);
extern OI_state wmState(wmObjectState);

#endif /* OBJECT_H */
