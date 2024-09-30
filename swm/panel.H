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
 * $Id: panel.H,v 9.5 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Panel object definitions
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/


#ifndef PANEL_H
#define PANEL_H

#include "object.H"

	// forward reference declarations
	struct wmIconPanel;

struct wmGeometry
{
    unsigned int width;
    unsigned int height;
    int sign_x;
    int x;
    int sign_y;
    int y;
    int center;

    wmGeometry(int sx, int cx, int sy, int cy, unsigned int w, unsigned int h, int c);
    wmGeometry();
};

struct wmPanelKid
{
    wmPanelKid *next;			// pointer to the next child
    struct wmObject *pop;		// back pointer to the parent panel
    struct wmObject *op;		// pointer to the object
    int type;				// the object type
    char *name;				// name
    int pad;				// padding for panels
    int innerPad;			// pad between objects
    wmObjectState state;		// state of the object
    struct wmGeometry *geom;		// pointer to the geometry
    OI_d_tech *oi;			// pointer to the oi object

    wmPanelKid(int t, char *n, wmGeometry *g);
};

struct wmRows
{
    struct wmRows *next;
    int sign;
    int row;
};

struct wmPanelLayout
{
    struct wmPanelLayout *next;
    struct wmPanelLayout *prev;
    struct wmPanelKid *kid;
};

struct wmPanelInfo
{
    struct wmPanelKid *kids;		// pointer to panel children
    struct wmGeometry *geom;		// used for root panels
    struct wmPanelLayout **layout;	// pointer to layout array
    struct wmRows *rr;			// the row root
    struct wmIconPanel *icon;		// pointer to icon panel info
    int rows;				// rows in a panel
    int old_rows;			// rows in a panel
    int pad;				// padding for panels
    int innerPad;			// pad between objects
    char root;				// boolean, root panel

    wmPanelInfo();
};

extern void wmCreatePanel(wmObject *);
extern OI_box *wmInstantiatePanel(wmObject *);
void wmLayoutPanel(wmObject *);
void wmInsertKid(wmObject *, wmPanelKid *);
void wmSortPanel(wmObject *);
void wmGetPixmap(OI_d_tech *);
void wmSetBackgroundPanel(OI_d_tech *, unsigned long, unsigned long);

#endif /* PANEL_H */
