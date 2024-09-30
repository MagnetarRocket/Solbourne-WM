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
 * $Id: panel.C,v 9.9 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Panel object routines
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/

#ifndef lint
static char RCSinfo[] =
"$Id: panel.C,v 9.9 1993/08/27 16:58:15 toml Exp $";
#endif /* lint */

#include "swm.H"
#include "gram.H"
#include "main.H"
#include "parse.H"
#include "list.H"
#include "object.H"
#include "panel.H"
#include "util.H"
#include "debug.H"
#include "bitmap.H"
#include "icons.H"
#include "execute.H"
#include "init.H"
#include "quarks.H"

static int cmpy(wmPanelLayout *, wmPanelLayout *);
static int cmpx(wmPanelLayout *, wmPanelLayout *);
static void reallyLayoutPanel(wmObject *);

/**********************************************************************
 *
 *  Procedure: 	wmCreatePanel
 *
 *  Function:	Initialize a panel.  Find the children and create 
 *		objects for each child found.
 *
 **********************************************************************/

void
wmCreatePanel(
    wmObject *op		// a pointer to the panel object
    )
{
    char *ptr = NULL;
    int i, pushCnt;
    wmPanelKid *kp;

#ifdef DEBUG
    if (wmDebug)
	fprintf(dfp, "wmCreatePanel \"%s\"\n", op->name);
#endif

    if (op->client || op->icons || op->root)
    {
	op->expanded = True;
	return;
    }

    RM->pushq(wmQuarks->panelName(), wmQuarks->panelClass());
    pushCnt = 1;

    for (i = 0; i < op->numQuarks; i++)
    {
	RM->pushq(op->quarks[i], op->quarks[i]);
	pushCnt++;
    }
    ptr = RM->get_resourceq(op->quarks[i], op->quarks[i]);
    for (i = 0; i < pushCnt; i++)
	RM->pop();

    if (ptr == NULL)
    {
	fprintf(stderr, "swm: wmCreatePanel: no panel definition for \"%s\"\n",
	    op->name);
	wmDone();
    }
    wmParse("wmPanel", ptr);
    if (wmParseError)
    {
	fprintf(stderr, "swm: wmCreatePanel: error parsing panel \"%s\"\n",
	    op->name);
	wmDone();
    }

    /**********************************************************************
     * When I get back from parsing the string, a list of wmPanelKid
     * structures has been created off of wmPanelKidList.  The contents
     * of each wmPanelKid structure is as follows:
     *     type  - the object type
     *     name  - then name,  this is the malloced string from lex
     *     geom  - the geometry,  this was created via "new"
     *
     * As we go through the kids list, we figure out how many rows
     * we have so we can being to sort the children into their rows.
     **********************************************************************/

    while ((kp = (wmPanelKid *)wmPanelKidsList.get()) != NULL)
    {
	wmInsertKid(op, kp);
    }

    wmSortPanel(op);
}

void
wmInsertKid(
    wmObject *op,
    wmPanelKid *kp
    )
{
    wmRows *rp;
    int found;

    if (kp->op == NULL)
    {
	kp->op = wmCreateObject(kp->type, kp->name);
	// if kop is NULL, we got an error creating the object
	if (kp->op == NULL)
	    wmDone();
    }

    // insert the kid pointer into the list of kids
    kp->next = op->u.p->kids;
    op->u.p->kids = kp;

    // setup the parent pointer
    kp->pop = op;

    // now see if this is a new row
    found = False;
    for (rp = op->u.p->rr; rp != NULL; rp = rp->next)
    {
	if (rp->row == kp->geom->y && rp->sign == kp->geom->sign_y)
	{
	    found = True;
	    break;
	}
    }

    // if it's not a new row, allocate space for a new row
    if (!found)
    {
	wmRows *nrp;

	op->u.p->rows++;
	nrp = (wmRows *)malloc(sizeof(wmRows));
	nrp->row = kp->geom->y;
	nrp->sign = kp->geom->sign_y;
	nrp->next = op->u.p->rr;
	op->u.p->rr = nrp;
    }
}

void
wmSortPanel(
    wmObject *op
    )
{
    wmPanelKid *kp;
    wmPanelLayout *lp;
    int i;

#ifdef DEBUG
    if (wmDebug)
	fprintf(dfp, "wmSort: \"%s\"\n", op->name);
#endif /* DEBUG */
    /**********************************************************************
     * I will then create a list for each row, the list will have the entries
     * for each row sorted.  If we had a panel definition of
     *
     * Swm*panel*title:\
     *	button  iconify     +0+0    \
     *	text    name        +1+0    \
     *	button  resize      -0+0    \
     *	button  focus       -1+0    \
     *	button  me          +0+1    \
     *	button  you         -0-0
     *
     * row0 = iconify name resize focus
     * row1 = me
     * row2 = you
     **********************************************************************/
#ifdef DEBUG
    if (wmDebug > 1)
	fprintf(dfp, "    numRows = %d\n", op->u.p->rows);
#endif
    if (op->u.p->rows == 0)
	return;

    // free the existing structure if it is there
    if (op->u.p->layout)
    {
	for (i = 0; i < op->u.p->old_rows; i++)
	{
	    for (lp = op->u.p->layout[i]; lp != NULL;)
	    {
		wmPanelLayout *tmp;
		tmp = lp;
		lp = lp->next;
		free((char *)tmp);
	    }
	}
	free((char *)op->u.p->layout);
    }

    op->u.p->layout = (wmPanelLayout **)calloc(op->u.p->rows,sizeof(wmPanelLayout *));
    op->u.p->old_rows = op->u.p->rows;
    for (i = 0; i < op->u.p->rows; i++)
	op->u.p->layout[i] = NULL;

    // get the kids organized into rows
    for (kp = op->u.p->kids; kp != NULL; kp = kp->next)
    {
	for (i = 0; i < op->u.p->rows; i++)
	{
	    if (op->u.p->layout[i] != NULL)
	    {
		if (op->u.p->layout[i]->kid->geom->y == kp->geom->y &&
		    op->u.p->layout[i]->kid->geom->sign_y == kp->geom->sign_y)
		{
		    break;
		}
	    }
	    else
		break;
	}
	lp = (wmPanelLayout *)malloc(sizeof(wmPanelLayout));
	lp->kid = kp;
	lp->prev = NULL;
	lp->next = op->u.p->layout[i];
	if (lp->next)
	    lp->next->prev = lp;
	op->u.p->layout[i] = lp;
    }

#ifdef DEBUG
    if (wmDebug > 1)
    {
	fprintf(dfp, "wmSort kids into their rows:\n");
	for (i = 0; i < op->u.p->rows; i++)
	{
	    for (lp = op->u.p->layout[i]; lp != NULL; lp = lp->next)
	    {
		fprintf(dfp, "  row %2d kid \"%s\" geom = %c%d%c%d\n",
		    i, lp->kid->name,
		    lp->kid->geom->sign_x < 0 ? '-' : '+', lp->kid->geom->x,
		    lp->kid->geom->sign_y < 0 ? '-' : '+', lp->kid->geom->y);
	    }
	}
    }
#endif
    // We now have the kiddies in their rows, we first want to sort the
    // rows by their Y coordinate and sign and then we want to sort
    // each row by their X cooridnate and sign
    if (op->u.p->rows != 1)
    {
	for (i = 0; i < op->u.p->rows-1; i++)
	{
	    int k = i;
	    wmPanelLayout *x = op->u.p->layout[i];;
	    for (int j = i+1; j < op->u.p->rows; j++)
	    {
#ifdef DEBUG
    if (wmDebug > 1)
    {
	fprintf(dfp, "  comparing \"%s\" to \"%s\"\n",
	    op->u.p->layout[j]->kid->name, x->kid->name);
    }
#endif /* DEBUG */
		if (cmpy(op->u.p->layout[j], x) < 0)
		{
#ifdef DEBUG
    if (wmDebug > 1)
    {
	fprintf(dfp, "  swapping  \"%s\" and \"%s\"\n",
	    op->u.p->layout[j]->kid->name, x->kid->name);
    }
#endif /* DEBUG */
		    k = j;
		    x = op->u.p->layout[j];
		}
	    }
	    op->u.p->layout[k] = op->u.p->layout[i];
	    op->u.p->layout[i] = x;
	}
    }
#ifdef DEBUG
    if (wmDebug > 1)
    {
	fprintf(dfp, "wmSort rows are sorted:\n");
	for (i = 0; i < op->u.p->rows; i++)
	{
	    for (lp = op->u.p->layout[i]; lp != NULL; lp = lp->next)
	    {
		fprintf(dfp, "  row %2d kid \"%s\" geom = %c%d%c%d\n",
		    i, lp->kid->name,
		    lp->kid->geom->sign_x < 0 ? '-' : '+', lp->kid->geom->x,
		    lp->kid->geom->sign_y < 0 ? '-' : '+', lp->kid->geom->y);
	    }
	}
    }
#endif

    // rows are now sorted, now sort columns in each row
    for (i = 0; i < op->u.p->rows; i++)
    {
	int done = False;
	while (!done)
	{
	    wmPanelLayout *p1, *p2;
	    for (p1 = op->u.p->layout[i]; p1 != NULL; p1 = p1->next)
	    {
		if ((p2 = p1->next) == NULL)
		{
		    done = True;
		    break;
		}
#ifdef DEBUG
    if (wmDebug > 1)
    {
	fprintf(dfp, "  comparing \"%s\" to \"%s\"\n",
	    p1->kid->name, p2->kid->name);
    }
#endif /* DEBUG */
		if (cmpx(p1, p2) > 0)
		{
#ifdef DEBUG
    if (wmDebug > 1)
    {
	fprintf(dfp, "  swapping  \"%s\" with \"%s\"\n",
	    p1->kid->name, p2->kid->name);
    }
#endif /* DEBUG */
		    // need to swap them
		    wmPanelLayout *tmp = p2->next;
		    p2->next = p1;
		    p1->next = tmp;
		    if (p1->next)
			p1->next->prev = p1;
		    tmp = p1->prev;
		    p1->prev = p2;
		    p2->prev = tmp;
		    if (p2->prev)
			p2->prev->next = p2;
		    else
			op->u.p->layout[i] = p2;
		    break;
		}
	    }
	} // while (!done)
    } // for
#ifdef DEBUG
    if (wmDebug)
	fprintf(dfp, "wmSort: done:\n");
    if (wmDebug > 1)
    {
	for (i = 0; i < op->u.p->rows; i++)
	{
	    for (lp = op->u.p->layout[i]; lp != NULL; lp = lp->next)
	    {
		fprintf(dfp, "  row %2d kid \"%s\" geom = %c%d%c%d\n",
		    i, lp->kid->name,
		    lp->kid->geom->sign_x < 0 ? '-' : '+', lp->kid->geom->x,
		    lp->kid->geom->sign_y < 0 ? '-' : '+', lp->kid->geom->y);
	    }
	}
    }
#endif
}

/***************************************************************
 *
 *  Procedure: 	cmpy
 *
 *  Function:	Compares two panel layout structures and returns
 *		a value corresponding to the following:
 *		    if (p1 < p2) return (-1);
 *		    if (p1 == p2) return (0);
 *		    if (p1 > p2) return (1);
 *
 *		An example series of numbers would be sorted as
 *		follows:   0, 1, 2, -0, -1, -2
 ***************************************************************/

static int
cmpy(
    wmPanelLayout *p1,		// panel layout pointer one
    wmPanelLayout *p2		// panel layout pointer two
    )
{
    if (p1->kid->geom->sign_y == p2->kid->geom->sign_y)
    {
	if (p1->kid->geom->y == p2->kid->geom->y)
	    return (0);

	if (p1->kid->geom->y < p2->kid->geom->y)
	    return (-1);

	if (p1->kid->geom->y > p2->kid->geom->y)
	    return (1);
    }

    if (p1->kid->geom->sign_y == 1)
	return (-1);

    return (1);
}

/***************************************************************
 *
 *  Procedure: 	cmpx
 *
 *  Function:	Same as cmpy but compares x
 *
 ***************************************************************/

static int
cmpx(
    wmPanelLayout *p1,		// panel layout pointer one
    wmPanelLayout *p2		// panel layout pointer two
    )
{
    if (p1->kid->geom->sign_x == p2->kid->geom->sign_x)
    {
	if (p1->kid->geom->x == p2->kid->geom->x)
	    return (0);

	if (p1->kid->geom->x < p2->kid->geom->x)
	    return (-1);

	if (p1->kid->geom->x > p2->kid->geom->x)
	    return (1);
    }

    if (p1->kid->geom->sign_x == 1)
	return (-1);

    return (1);
}

/***************************************************************
 *
 *  Procedure: 	wmInstantiatePanel
 *
 *  Function:	Create the OI objects in a panel.
 *		Called from wmInstantiateObject on object.C
 *
 ***************************************************************/

OI_box *
wmInstantiatePanel(
    wmObject *op		// the object pointer to the panel
    )
{
    static char *p = ".panel";
    char *ptr;
    int pushCnt, i;
    wmPanelKid *kp;
    OI_d_tech *dp;

#ifdef DEBUG
    if (wmDebug)
	fprintf(dfp, "wmInstantiatePanel \"%s\"\n", op->name);
#endif
    RM->pushq(wmQuarks->panelName(), wmQuarks->panelClass());
    for (i = 0, pushCnt = 1; i <= op->numQuarks; i++, pushCnt++)
	RM->pushq(op->quarks[i], op->quarks[i]);

    // create the OI object for the panel
    // if it is an icon panel, make it a scroll_box object, otherwise
    // just a plane box
    if (op->icons && wmMakingRootPanels)
    {
	long unsigned cntrl;
	op->u.p->icon = new wmIconPanel();
	op->u.p->icon->scr = wmScr;
	ptr = RM->get_resourceq(wmQuarks->scrollBarsName(), wmQuarks->scrollBarsClass());
	if (ptr && (ptr[0] == 'F' || ptr[0] == 'f'))
	{
	    cntrl = 0;
	    op->u.p->icon->scrollBars = False;
	}
	else
	{
	    cntrl = OI_SCROLL_BAR_LEFT | OI_SCROLL_BAR_TOP;
	    cntrl = OI_SCROLL_BAR_RIGHT | OI_SCROLL_BAR_BOTTOM;
	    op->u.p->icon->scrollBars = True;
	}

	op->oi = (OI_d_tech *)oi_create_scroll_box(NULL, cntrl, 10, 10, 10, 10, wmDefaultSlotSize, wmDefaultSlotSize);
    }
    else
	op->oi = (OI_d_tech *)oi_create_box(NULL,20,20);

    op->oi->allow_model_info();
    for (kp = op->u.p->kids; kp != NULL; kp = kp->next)
    {
	if (kp->op->icons)
	    kp->op->u.p->geom = kp->geom;
	dp = wmInstantiateObject(kp->op);
	kp->op->odp->kp = kp;
	kp->op->odp->state = kp->op->state;
	kp->state = kp->op->state;
	kp->oi = dp;
	if (kp->type == OBJ_PANEL)
	{
	    kp->pad = kp->op->u.p->pad;
	    kp->innerPad = kp->op->u.p->innerPad;
	}

	// set the state of the object
	dp->set_associated_object(op->oi, -5000, -5000, wmState(kp->op->state));
    }

    wmGetStandardResources(op);
    wmGetBindings(op);
    ptr = RM->get_resourceq(wmQuarks->padName(), wmQuarks->padClass());
    op->u.p->pad = 0;
    if (ptr) op->u.p->pad = atoi(ptr);
    ptr = RM->get_resourceq(wmQuarks->objectPadName(), wmQuarks->objectPadClass());
    if (ptr) op->u.p->innerPad = atoi(ptr);

    // if this is an icon panel, go initialize it
    if (op->u.p->icon && wmMakingRootPanels)
    {
	wmInitIconPanel(op);
    }

    wmGetPixmap(op->oi);

    for (i = 0; i < pushCnt; i++)
	RM->pop();

    return ((OI_box *)op->oi);
}

void
wmGetPixmap(OI_d_tech *oi)
{
    char *ptr = RM->get_resourceq(wmQuarks->backgroundPixmapName(), wmQuarks->backgroundPixmapClass());
    if (ptr)
    {
	char *p;

	// let's do some work to tile the background
	if (ptr[0] == '@')
	    p = &ptr[1];
	else
	    p = &ptr[0];

	wmBitmap *wbm = wmFindBitmap(p);
	if (wbm != NULL && wbm->pixmap)
	{
	    wmObjectData *odp = (wmObjectData *)oi->data();
	    odp->pixmap = wbm->pixmap;
	    odp->pixmapWidth = wbm->width;
	    odp->pixmapHeight = wbm->height;
	    wmSetBackgroundPanel(oi, oi->fg_pixel(), oi->bkg_pixel());
	}
    }
}

void
wmSetBackgroundPanel(
    OI_d_tech *oi,
    unsigned long fg,
    unsigned long bg
    )
{
    GC gc;
    XGCValues gcv;
    Pixmap back;

    wmObjectData *odp = (wmObjectData *)oi->data();
    gcv.foreground = fg;
    gcv.background = bg;
    gc = XCreateGC(DPY, wmScr->root, GCForeground|GCBackground, &gcv);
    back = XCreatePixmap(DPY, wmScr->root, odp->pixmapWidth,
	odp->pixmapHeight, wmScr->depth);
    XCopyPlane(DPY, odp->pixmap, back, gc, 0, 0, odp->pixmapWidth,
	odp->pixmapHeight, 0, 0, 1);
    XSetWindowBackgroundPixmap(DPY, oi->X_window(), back);
    XClearWindow(DPY, oi->X_window());
    XFreePixmap(DPY, back);
    XFreeGC(DPY, gc);
}

void
wmLayoutPanel(
    wmObject *op		// the object pointer to the panel
    )
{
    reallyLayoutPanel(op);
#ifdef SHAPE
    // get rid of the shapeWindow, if there is one
    wmObjectData *odp;
    wmData *wp;
    OI_d_tech *oi;

    odp = (wmObjectData *)op->oi->data();
    oi = op->oi;
    if (odp->shape)
    {
	wp = odp->wp;
	if (odp->shapeWindow)
	{
	    XDestroyWindow(DPY, odp->shapeWindow);
	    odp->shapeWindow = None;
	}

	// fill in ther resize corners and bars if this is the outside
	// frame and they are present
	if (wp && wp->oi_frame() == oi && (wp->resize_corners() || wp->resize_bars()))
	{
	    XRectangle newBounding[12];
	    XRectangle *r;
	    r = newBounding;

	    if (odp->wp->resize_corners())
	    {
		// top left
		r->x = 0;
		r->y = 0;
		r->width = wp->resize_length();
		r++->height = wp->resize_width();
		r->x = 0;
		r->y = 0;
		r->width = wp->resize_width();
		r++->height = wp->resize_length();

		// bottom left
		r->x = 0;
		r->y = oi->size_y() - wp->resize_width();
		r->width = wp->resize_length();
		r++->height = wp->resize_width();
		r->x = 0;
		r->y = oi->size_y() - wp->resize_length();
		r->width = wp->resize_width();
		r++->height = wp->resize_length();

		// top right 
		r->x = oi->size_x() - wp->resize_length();
		r->y = 0;
		r->width = wp->resize_length();
		r++->height = wp->resize_width();
		r->x = oi->size_x() - wp->resize_width();
		r->y = 0;
		r->width = wp->resize_width();
		r++->height = wp->resize_length();

		// bottom right
		r->x = oi->size_x() - wp->resize_length();
		r->y = oi->size_y() - wp->resize_width();
		r->width = wp->resize_length();
		r++->height = wp->resize_width();
		r->x = oi->size_x() - wp->resize_width();
		r->y = oi->size_y() - wp->resize_length();
		r->width = wp->resize_width();
		r++->height = wp->resize_length();
	    }
	    if (r != newBounding)
		XShapeCombineRectangles(DPY, oi->X_window(), ShapeBounding, 0, 0, newBounding, r - newBounding, ShapeUnion, Unsorted);
	}
    }
#endif /* SHAPE */
}

static void
reallyLayoutPanel(
    wmObject *op		// the object pointer to the panel
    )
{
    XRectangle newBounding, newClip;
    int width = 0;		// the final width and height of the panel
    int height = 0;
    int ty;
    int y = 0;
    int minus_x, x = 0;
    int *row_height = (int *)calloc(op->u.p->rows, sizeof(int));
    int i;
    int kid_y;
    wmPanelLayout *lp;
    wmObjectData *podp, *odp;

#ifdef DEBUG
    if (wmDebug)
    {
	fprintf(dfp, "reallyLayoutPanel: \"%s\"\n", op->name);
    }
#endif

#ifdef SHAPE
    podp = (wmObjectData *)op->oi->data();
    op->u.p->pad = podp->pad;
    if (op->client && podp->shape)
    {
	podp->shapeWindow = XCreateSimpleWindow(DPY, wmScr->root, 0, 0, op->oi->space_x(), op->oi->space_y(), 0, 0, 0);

	// if the client window is shaped, set the client panel to that shape
	if (podp->wp->shaped())
	    XShapeCombineShape(DPY, podp->shapeWindow, ShapeBounding, podp->wp->attr_bw()+op->oi->bvl_width(), podp->wp->attr_bw()+op->oi->bvl_width(),
		podp->wp->window(), ShapeBounding, ShapeSet);

	// if there is a shape pixmap, intersect it with what I already have
	if (podp->shapePixmap)
	    XShapeCombineMask(DPY, podp->shapeWindow, ShapeBounding, 0, 0, podp->shapePixmap, ShapeIntersect);
    }
#endif /* SHAPE */

    if (op->client || op->icons)
    {
	if (row_height)
	    free((char *)row_height);
	return;
    }

    for (i = 0; i < op->u.p->rows; i++)
    {
	x = 0;
	row_height[i] = 0;
	for (lp = op->u.p->layout[i]; lp != NULL; lp = lp->next)
	{

	    // first figure out the ultimate width and height
	    lp->kid->oi = lp->kid->op->oi = op->oi->subobject(lp->kid->name);
	    odp = (wmObjectData *)lp->kid->oi->data();

	    if (lp->kid->type == OBJ_PANEL)
	    {
		lp->kid->op->u.p->pad = lp->kid->pad;
		lp->kid->op->u.p->innerPad = lp->kid->innerPad;
		reallyLayoutPanel(lp->kid->op);
	    }
#ifdef DEBUG
    if (wmDebug)
    {
	fprintf(dfp, "  kid \"%s\" = (%d x %d) (%d x %d) state=%d\n",
	    lp->kid->name,
	    lp->kid->oi->size_x(),
	    lp->kid->oi->size_y(),
	    lp->kid->oi->space_x(),
	    lp->kid->oi->space_y(),
	    odp->state);
    }
#endif
	    // If the state of the object is *NoSpace, it should 
	    // not take up any horizontal space.  If it is also
	    // unmapped it should not take up any vertical space
	    if (!(odp->state & wmNoSpace))
	    {
		x += lp->kid->oi->space_x();
		x += op->u.p->innerPad;
		if (x > width)
		    width = x;
	    }
		
	    if (odp->state & wmIsMapped)
	    {
		ty = lp->kid->oi->space_y() + op->u.p->innerPad;
		if (ty > row_height[i])
		    row_height[i] = ty;
	    }
	}
	height += row_height[i];
    }
    width += 2 * op->u.p->pad;
    height += 2 * op->u.p->pad;
    // set the new size
    if (!op->icons)
    {
#ifdef DEBUG
	if (wmDebug > 1)
	    fprintf(dfp, "resizing panel to %d x %d\n", 
		width ? width : 1, height ? height : 1);
#endif /* DEBUG */
	op->oi->set_size(width ? width : 1, height ? height : 1);
#ifdef SHAPE
	if (podp->shape)
	{
	    podp->shapeWindow = XCreateSimpleWindow(DPY, wmScr->root, 0, 0, op->oi->size_x(), op->oi->size_y(), op->oi->bdr_width(), 0, 0);

	    // get rid of the shape on the shape window
	    newBounding.x = -op->oi->bdr_width();
	    newBounding.y = -op->oi->bdr_width();
	    newBounding.width = op->oi->space_x();
	    newBounding.height = op->oi->space_y();
	    XShapeCombineRectangles(DPY, podp->shapeWindow, ShapeBounding, 0, 0, &newBounding, 1, ShapeSubtract, YXBanded);

	    // combine in the shape pixmap (if any)
	    if (podp->shapePixmap)
		XShapeCombineMask(DPY, podp->shapeWindow, ShapeBounding, 0, 0, podp->shapePixmap, ShapeUnion);
	}
#endif /* SHAPE */
    }

    // now layout the panel
    y = op->u.p->pad;
    for (i = 0; i < op->u.p->rows; i++)
    {
	minus_x = width - op->u.p->pad;
	x = op->u.p->pad;
	for (lp = op->u.p->layout[i]; lp != NULL; lp = lp->next)
	{
	    int kid_x = 0;

	    lp->kid->oi = lp->kid->op->oi = op->oi->subobject(lp->kid->name);
	    odp = (wmObjectData *)lp->kid->oi->data();

	    // position it
	    if (lp->kid->geom->center)
		kid_x = (op->oi->size_x() - lp->kid->oi->space_x())/2;
	    else if (lp->kid->geom->sign_x > 0)
		kid_x = x;
	    else
		kid_x = minus_x - lp->kid->oi->space_x();

	    // if the state is *NoSpace, or if it is centered, don't take
	    // up space
	    if (!lp->kid->geom->center && !(odp->state & wmNoSpace))
	    {
		if (lp->kid->geom->sign_x > 0)
		    x += lp->kid->oi->space_x() + op->u.p->innerPad;
		else
		    minus_x -= lp->kid->oi->space_x() + op->u.p->innerPad;
	    }
#ifdef DEBUG
    if (wmDebug)
	fprintf(dfp, "  placing \"%s\" at %d, %d\n", lp->kid->name, kid_x, y);
#endif

	    // finally position the thing
	    if (odp->gravity == NorthGravity)
		kid_y = y;
	    else if (odp->gravity == SouthGravity)
		    kid_y = y + row_height[i] - lp->kid->oi->space_y();
	    else // must be CenterGravity
		    kid_y = (y + ((row_height[i] - lp->kid->oi->space_y())/2));

	    lp->kid->oi->set_loc(kid_x, kid_y);
#ifdef SHAPE
	    // first take care of the child shape
	    if (odp->state & wmIsMapped)
	    {
		if (podp->shape || odp->shape)
		{
		    if (odp->shapePixmap)
			XShapeCombineMask(DPY, lp->kid->oi->X_window(), ShapeBounding, 0, 0, odp->shapePixmap, ShapeSet);
		    else if (!odp->shapeWindow)
		    {
			// reset the shape to the outside rectangle
			newBounding.x = -lp->kid->oi->bdr_width();
			newBounding.y =  -lp->kid->oi->bdr_width();
			newBounding.width = lp->kid->oi->space_x();
			newBounding.height = lp->kid->oi->space_y();
			XShapeCombineRectangles (DPY, lp->kid->oi->X_window(), ShapeBounding, 0, 0, &newBounding, 1, ShapeSet, YXBanded);

			newClip.x = 0;
			newClip.y = 0;
			newClip.width = lp->kid->oi->size_x();
			newClip.height = lp->kid->oi->size_y();
			XShapeCombineRectangles (DPY, lp->kid->oi->X_window(), ShapeClip, 0, 0, &newClip, 1, ShapeSet, YXBanded);
		    }
		}
		    
		// now combine it with the panel if needed
		if (podp->shape)
		{
		    if (odp->shapeWindow)
		    {
			// if the object is a panel, use its shapeWindow and then get rid of it
			XShapeCombineShape(DPY, podp->shapeWindow, ShapeBounding, lp->kid->oi->bdr_width()+kid_x, lp->kid->oi->bdr_width()+kid_y,
			    odp->shapeWindow, ShapeBounding, ShapeUnion);
			XDestroyWindow(DPY, odp->shapeWindow);
			odp->shapeWindow = None;
		    }
		    else
		    {
			// otherwise just use the child window
			XShapeCombineShape(DPY, podp->shapeWindow, ShapeBounding, lp->kid->oi->bdr_width()+kid_x, lp->kid->oi->bdr_width()+kid_y,
			    lp->kid->oi->X_window(), ShapeBounding, ShapeUnion);
		    }
		}
	    }

#endif /* SHAPE */
	}
	y += row_height[i];
    }
#ifdef DEBUG
    if (wmDebug)
    {
	fprintf(dfp, "reallyLayoutPanel: ----------\n");
    }
#endif

    if (row_height)
	free((char *)row_height);

#ifdef SHAPE
    // Finally set the shapeWindow to the window of the panel
    if (podp->shape)
	XShapeCombineShape(DPY, podp->op->oi->X_window(), ShapeBounding, 0, 0, podp->shapeWindow, ShapeBounding, ShapeSet);
#endif /* SHAPE */

    return;
}


wmPanelKid::wmPanelKid(
    int t,			// the object type
    char *n,			// the name
    wmGeometry *g		// the geometry
    )
{
    next = NULL;
    pop = NULL;
    op = NULL;
    type = t;
    name = n;
    pad = 0;
    innerPad = 0;
    geom = new wmGeometry(g->sign_x,g->x,g->sign_y,
	g->y, g->width,g->height, g->center);
}

wmGeometry::wmGeometry()
{
    width = 1;
    height = 1;
    sign_x = 1;
    x = -5000;
    sign_y = 1;
    y = -5000;
    center = False;
}

wmGeometry::wmGeometry(
    int sx,			// sign of the x coordinate
    int cx,			// x coordinate
    int sy,			// sign of the y coordinate
    int cy,			// y coordinate
    unsigned int w,		// width
    unsigned int h,		// height
    int c			// center
    )
{
    sign_x = sx;
    x = cx;
    sign_y = sy;
    y = cy;
    width = w;
    height = h;
    center = c;
}

wmPanelInfo::wmPanelInfo()
{
    kids = NULL;
    geom = NULL;
    layout = NULL;
    rr = NULL;
    rows = 0;
    old_rows = 0;
    pad = 0;
    innerPad = 0;
    root = False;
    icon = NULL;
}
