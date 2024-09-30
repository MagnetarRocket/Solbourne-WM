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
 * $Id: util.C,v 9.16 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Utility routines
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/

#ifndef lint
static char RCSinfo[] =
"$Id: util.C,v 9.16 1993/08/27 16:58:15 toml Exp $";
#endif /* lint */

#include "swm.H"
#include "screen.H"
#include "main.H"
#include "cursor.H"
#include "object.H"
#include "debug.H"
#include "wmdata.H"
#include "init.H"
#include "util.H"
#include "panel.H"
#include "execute.H"
#include "parse.H"
#include "bitmap.H"
#include "quarks.H"

static char *string = "string";
wmObjectState wmObjState;		// set when parsing the object state


/**********************************************************************
 *
 *  Procedure:
 *	wmExpandFilename
 *
 *  Function:
 *	Check the first character of a filename, if it is a '~'
 *	character, replace it with the user's home directory.
 *
 **********************************************************************
 */

char *
wmExpandFilename(
    char *filename		// the filename to eexpand
    )
{
    static char *home = NULL;
    static int homelen;

    if (!home)
    {
	home = (char *)getenv("HOME");
	if (home == NULL)
	    home = "./";
	homelen = strlen(home);
    }

    if (filename[0] == '~')
    {
	char *np = (char *)malloc(homelen + strlen(filename) + 3);
	sprintf(np, "%s/%s", home, &filename[1]);
	free(filename);
	filename = np;
    }
    return (filename);
}

/***************************************************************
 *
 *  Procedure: 
 *	wmDeleteOI
 *
 *  Function:
 *	Delete an OI object and all of its decendents.  I would
 *	have used the member function delete_all() but that would
 *	leave wmObjectData pointers unfreed.
 *
 ***************************************************************/

void
wmDeleteOI(
    OI_d_tech *oi,			// the object to delete
    int	delete_flag			// should we really delete the object?
    )
{
    OI_d_tech *child;
    wmObjectData *odp;
    int		n;

#ifdef DEBUG
    static int level = 0;
    if (wmDebug)
    {
	if (level == 0)
	    fprintf(dfp, "wmDeleteOI: %s\n", oi->name());
    }
    level += 1;
#endif
    if (oi != NULL)
    {
#ifdef DEBUG
    if (wmDebug > 1)
    {
	for (int i = 0; i < level; i++)
	    fprintf(dfp, "  ");
	fprintf(dfp, "%s\n", oi->name() ? oi->name() : "No Name");
    }
#endif
	// OK to go back to the next_child() version when OI stops
	// returning internal objects.
//	for (child = oi->next_child(NULL); child != NULL; child = oi->next_child(delete_flag?NULL:child))
	for (child = oi->numbered_child(0), n = 0; child != NULL; child = oi->numbered_child(delete_flag?0:n++))
	{
	    wmDeleteOI(child, delete_flag);
	}

	if (!oi->is_derived_from("OI_menu_cell"))
	{
	    odp = (wmObjectData *)oi->data();
	    oi->set_data(NULL);
	    if (odp)
		delete odp;
	}
	XDeleteContext(DPY, oi->X_window(), wmInternalContext);
	if (delete_flag)
	    oi->del();
    }
#ifdef DEBUG
    level -= 1;
#endif
}


#ifdef VERY_OLD
char *
wmGetResource(
    char *str0,				// the resource to look for
    char *str1,				// the resource to look for
    char *str2,				// the resource to look for
    char *str3				// the resource to look for
    )
{
    char buff[400];
    char *ptr = NULL;

    if (str0[0] == '.')
	str0 = &str0[1];
    strcpy(buff, wmScr->res);
    strcat(buff, str0);
    if (str1) {
	if (str1[0] != '.')
	    strcat(buff, ".");
	strcat(buff, str1);
    }
    if (str2) { strcat(buff, "."); strcat(buff, str2); }
    if (str3) { strcat(buff, "."); strcat(buff, str3); }

    // OI_get_resource(buff, &string, &ptr);
    wmScr->conp->get_resource(buff, &string, &ptr, OI_NO);
    if (wmResourceDebug)
    {
	if (ptr == NULL)
	    fprintf(wmResourceDebug, "!swm*%s:\n", buff);
	else
	    fprintf(wmResourceDebug, "swm*%s: %s\n", buff, ptr);
    }
#ifdef DEBUG
    if (wmDebug)
    {
	fprintf(dfp, "wmGetResource: \"%s\"\n", buff);
	if (ptr == NULL)
	    fprintf(dfp, "    resource not found\n");
	else
	    fprintf(dfp, "    found \"%s\"\n", ptr);
    }
#endif

    return (ptr);
}

static char res[1000];
static int curlen = 0;

void
wmNewResource(
    char *str,
    int len
    )
{
    res[0] = '\0';

    if (len)
	curlen = len + 1;
    else
	curlen = strlen(str)+1;
    strcpy(res, str);
}

void
wmAddResource(
    char *str,
    int len
    )
{
    if (len)
	curlen += len;
    else
    {
	curlen += strlen(str)+1;
	strcat(res, ".");
    }

    strcat(res, str);
}

void
wmRmResource(
    char *str,
    int len
    )
{
    if (len)
	curlen -= len;
    else
	curlen -= strlen(str)+1;

    res[curlen-1] = '\0';
}

char *
wmGetDefaultResource(char *p1)
{
   return (wmGetResource(res, p1));
}
#endif /* VERY_OLD */

static struct _st
{
    char *name;
    wmObjectState state;
} st[] = {
{"Mapped",		wmMapped		},
{"MappedNoSpace",	wmMappedNoSpace		},
{"Unmapped",		wmUnmapped		},
{"UnmappedNoSpace",	wmUnmappedNoSpace	}
};
#define NUM_STATES (sizeof(st)/sizeof(struct _st))

void
wmGetOIResources(
    OI_d_tech *oi,
    int client
    )
{
    wmBitmap *wbm;
    int bdr_width;
    XColor cursor_fg;
    XColor cursor_bg;
    PIXEL pixel;
    wmObjectData *odp = (wmObjectData *)oi->data();
    char *cfg, *cbg;
    char *ptr;

    RM->search_resources();
#ifdef SHAPE
    if (wmHasShape)
    {
	// see if we should shape this object
	ptr = RM->get_search_resourceq(wmQuarks->shapeName(), wmQuarks->shapeClass());
	if (ptr && (ptr[0] == 't' || ptr[0] == 'T'))
	    odp->shape = True;

	ptr = RM->get_search_resourceq(wmQuarks->shapeMaskName(), wmQuarks->shapeMaskClass());
	if (ptr)
	{
#ifdef DEBUG
	    if (sscanf(ptr, "0x%x", &odp->shapePixmap) != 1)
	    {
#endif /* DEBUG */
	    wbm = wmFindBitmap(ptr);
	    if (wbm)
		odp->shapePixmap = wbm->pixmap;
#ifdef DEBUG
	    }
#endif
	}
    }
#endif /* SHAPE */

    // see if the user has specified a border width
    ptr = RM->get_search_resourceq(wmQuarks->borderWidthName(), wmQuarks->borderWidthClass());
    if (ptr)
	bdr_width = atoi(ptr);
    else
	bdr_width = 0;
    oi->set_bdr_width(bdr_width);

    ptr = RM->get_search_resourceq(wmQuarks->bevelWidthName(), wmQuarks->bevelWidthClass());
    if (ptr)
    {
	int bvl_width = atoi(ptr);
	oi->set_bvl_width(bvl_width);
    }

    // only get a border color if there is some border to display
    if (bdr_width)
    {
	if ((ptr = RM->get_search_resourceq(wmQuarks->borderName(), wmQuarks->borderClass())))
	    if (wmGetColor(ptr, &pixel))
		oi->set_bdr_pixel(pixel);
    }

    // don't need these if this is the client panel object, they
    // will never show through
    if (!client)
    {
	if ((ptr = RM->get_search_resourceq(wmQuarks->foregroundName(), wmQuarks->foregroundClass())))
	    if (wmGetColor(ptr, &pixel)) {
		oi->set_fg_pixel(pixel);
		if (oi->is_derived_from("OI_menu")) {
			OI_menu *mnup = (OI_menu *)oi;
			for (int i = 0; i < mnup->num_cells(); i++)
				mnup->numbered_cell(i)->set_fg_pixel(pixel);
		}
	    }
    }

    if ((ptr = RM->get_search_resourceq(wmQuarks->backgroundName(), wmQuarks->backgroundClass())))
	if (wmGetColor(ptr, &pixel)) {
		oi->set_bkg_pixel(pixel);
		if (oi->is_derived_from("OI_menu")) {
			OI_menu *mnup = (OI_menu *)oi;
			for (int i = 0; i < mnup->num_cells(); i++)
				mnup->numbered_cell(i)->set_bkg_pixel(pixel);
		}
	}

    if (wmClient && !(oi->is_derived_from("OI_menu")))
    {
	odp->unfocusForeground = odp->focusForeground = oi->fg_pixel();
	odp->unfocusBackground = odp->focusBackground = oi->bkg_pixel();
	odp->unfocusBorder = odp->focusBorder = oi->bdr_pixel();

	int got_it = False;

	// don't need these if this is the client panel object, they
	// will never show through
	ptr = RM->get_search_resourceq(wmQuarks->focusReverseName(), wmQuarks->focusReverseClass());
	if (ptr && (ptr[0] == 't' || ptr[0] == 'T'))
	{
	    got_it = True;
	    unsigned long tmp = odp->focusForeground;
	    odp->focusForeground = odp->focusBackground;
	    odp->focusBackground = tmp;
	}

	if (!client)
	{
	    if ((ptr = RM->get_search_resourceq(wmQuarks->focusForegroundName(), wmQuarks->focusForegroundClass())))
	    {
		got_it = True;
		wmGetColor(ptr, &odp->focusForeground);
	    }
	}

	if ((ptr = RM->get_search_resourceq(wmQuarks->focusBackgroundName(), wmQuarks->focusBackgroundClass())))
	{
	    got_it = True;
	    wmGetColor(ptr, &odp->focusBackground);
	}

	if ((ptr = RM->get_search_resourceq(wmQuarks->focusBorderName(), wmQuarks->focusBorderClass())))
	{
	    got_it = True;
	    wmGetColor(ptr, &odp->focusBorder);
	}

	if (got_it)
	{
	    wmList *list = wmClient->focus_list_p();
	    list->append((ent)oi);
	}
    }

    if (!(oi->is_derived_from("OI_glyph") || oi->is_derived_from("OI_box")))
    {
	ptr = RM->get_search_resourceq(wmQuarks->fontName(), wmQuarks->fontClass());
	if (ptr)
		oi->set_font(ptr);
		if (oi->is_derived_from("OI_menu")) {
			OI_menu *mnup = (OI_menu *)oi;
			for (int i = 0; i < mnup->num_cells(); i++)
				mnup->numbered_cell(i)->set_font(ptr);
		}
    }

    ptr = RM->get_search_resourceq(wmQuarks->cursorName(), wmQuarks->cursorClass());
    oi->set_cursor(wmCursor(ptr));

    cfg = RM->get_search_resourceq(wmQuarks->cursorForegroundName(), wmQuarks->cursorForegroundClass());
    cbg = RM->get_search_resourceq(wmQuarks->cursorBackgroundName(), wmQuarks->cursorBackgroundClass());
    if (cfg || cbg)
    {
	if (cfg)
	    wmGetColor(cfg, &pixel, &cursor_fg);
	else
	    wmGetColor("black", &pixel, &cursor_fg);

	if (cbg)
	    wmGetColor(cbg, &pixel, &cursor_bg);
	else
	    wmGetColor("black", &pixel, &cursor_bg);
	XRecolorCursor(DPY, oi->cursor(), &cursor_fg, &cursor_bg);
    }

    if (odp)
    {
	ptr = RM->get_search_resourceq(wmQuarks->gravityName(), wmQuarks->gravityClass());
	if (ptr)
	{
	    if (ptr[0] == 'S' || ptr[0] == 's')
		odp->gravity = SouthGravity;
	    else if (ptr[0] == 'C' || ptr[0] == 'c')
		odp->gravity = CenterGravity;
	}
    }
}

void
wmGetStandardResources(wmObject *op)
{
    op->oi->set_data((void *)op->odp);
    wmGetOIResources(op->oi, (int)op->client);
    char *ptr = RM->get_resourceq(wmQuarks->stateName(), wmQuarks->stateClass());
    wmObjState = wmMapped;
    if (ptr)
    {
	if (XFindContext(DPY, (Window)ptr, wmStateContext, (caddr_t*)&wmObjState) == XCNOENT)
	{
	    wmParse("wmState", ptr);
	    if (wmParseError)
	    {
		fprintf(stderr, "swm: wmGetStandardResources: error parsing state \"%s\"\n", ptr);
		wmObjState = wmMapped;
	    }
	    XSaveContext(DPY, (Window)ptr, wmStateContext, (caddr_t)wmObjState);
	}
    }
    op->state = wmObjState;
}

int
wmGetColor(char *name, PIXEL *p, XColor *return_cp)
{
    char *err_msg;
    int status;
    wmColormap *wcm;
    XColor *cp;
    XColor exact_color;
    XrmQuark quark;

    cp = NULL;
    status = True;
    err_msg = NULL;
    quark = XrmStringToQuark(name);
    if (XFindContext(DPY, (Window)wmScr->cmap, wmScr->colorCache, (caddr_t*)&wcm) == XCNOENT)
	wcm = new wmColormap(wmScr->cmap);

    if (XFindContext(DPY, (Window)quark, wcm->context(), (caddr_t*)&cp) == XCNOENT)
    {
	/* check to see if we've had problems allocating this color before we try */
	if (XFindContext(DPY, (Window)quark, wmScr->badColors, (caddr_t*)&cp) == XCNOENT)
	{
	    cp = (XColor *)malloc(sizeof(XColor));
	    cp->flags = 0;
	    cp->pixel = BlackPixel(DPY, wmScr->screen);
	    if (!wcm->is_std_cmap()) 
	    {
		if (name[0] != '#')
		{
		    if (!XAllocNamedColor(DPY, wcm->colormap(), name, cp, &exact_color))
		    {
			err_msg = "swm: wmGetColor: couldn't allocate ";
			status = False;
		    }
		}
		else
		{
		    if (!XParseColor(DPY, wcm->colormap(), name, cp))
		    {
			err_msg = "swm: wmGetColor: couldn't parse ";
			status = False;
		    }
		    else if (!XAllocColor(DPY, wcm->colormap(), cp))
		    {
			err_msg = "swm: wmGetColor: couldn't allocate ";
			status = False;
		    }
		}
	    }
	    else
	    {
		if (!XParseColor(DPY, wcm->colormap(), name, cp))
		{
		    err_msg = "swm: wmGetColor: couldn't parse ";
		    status = False;
		}
		cp->pixel = wcm->base_pixel() +
			(unsigned long)((cp->red / 65535.0) * wcm->red_max()  + 0.5) * wcm->red_mult() +
			(unsigned long)((cp->green /65535.0) * wcm->green_max() + 0.5)* wcm->green_mult() +
			(unsigned long)((cp->blue  / 65535.0) * wcm->blue_max() + 0.5)* wcm->blue_mult();
	    }
	    if (status)
		XSaveContext(DPY, (Window)quark, wcm->context(), (caddr_t)cp);
	}
	else
	    status = False;
    }

    if (status)
    {
	*p = cp->pixel;
	if (return_cp)
	    *return_cp = *cp;
    }
    else if (cp)
    {
	free((char *)cp);
    }
    if (err_msg)
    {
	fprintf(stderr, "%s\"%s\"\n", err_msg, name);
	XSaveContext(DPY, (Window)quark, wmScr->badColors, (caddr_t)NULL);
    }

    return (status);
}

char *
wmUniqueName()
{
    static int sequence = 0;
    static char *pre = "__swm";

    char *ptr = (char *)malloc(sizeof(pre)+10);
    sprintf(ptr, "%s%d", pre, sequence++);

#ifdef DEBUG
    if (wmDebug)
	fprintf(dfp, "wmUniqueName: returning \"%s\"\n", ptr);
#endif
    return (ptr);
}


/***********************************************************************
 *
 *  Procedure:
 *	wmMoveOutline - move a window outline
 *
 *  Inputs:
 *	window	    - the window we are outlining
 *	count	    - how many rectangles are we to paint
 *	rect	    - pointer to rectangle array
 *
 ***********************************************************************
 */

#define DRAW \
    if (lastWidth || lastHeight)\
    {\
	xl = lastx;\
	xr = lastx + lastWidth - 1;\
	yt = lasty;\
	yb = lasty + lastHeight - 1;\
	xthird = lastWidth/3;\
	ythird = lastHeight/3;\
\
	r->x1 = xl;\
	r->y1 = yt;\
	r->x2 = xr;\
	r++->y2 = yt;\
\
	r->x1 = xl;\
	r->y1 = yb;\
	r->x2 = xr;\
	r++->y2 = yb;\
\
	r->x1 = xl;\
	r->y1 = yt;\
	r->x2 = xl;\
	r++->y2 = yb;\
\
	r->x1 = xr;\
	r->y1 = yt;\
	r->x2 = xr;\
	r++->y2 = yb;\
\
	if (wmScr->resizeGrid) \
	{ \
	    r->x1 = xl + xthird;\
	    r->y1 = yt;\
	    r->x2 = r->x1;\
	    r++->y2 = yb;\
    \
	    r->x1 = xl + (2 * xthird);\
	    r->y1 = yt;\
	    r->x2 = r->x1;\
	    r++->y2 = yb;\
    \
	    r->x1 = xl;\
	    r->y1 = yt + ythird;\
	    r->x2 = xr;\
	    r->y2 = r->y1;\
	    r++;\
    \
	    r->x1 = xl;\
	    r->y1 = yt + (2 * ythird);\
	    r->x2 = xr;\
	    r->y2 = r->y1;\
	    r++;\
	} \
    }

void wmMoveOutline(
    Window window,		// the window to draw the outline on
    int count,
    XRectangle *rect
    )
{
#define MAX_RECTS      20
#define LINES_PER_RECT 8

    static Window lastWindow = 0;
    static XRectangle lastrectangles[MAX_RECTS];
    static XRectangle *lastrect = lastrectangles;
    static int lastcount = 0;
    int i;
    int		lastx, lasty, lastWidth, lastHeight;
    int		xl, xr, yt, yb;
    int		xthird, ythird;

    XSegment	outline[2*(MAX_RECTS*LINES_PER_RECT)];
    XSegment	*r = outline;

    if (!wmScr->showGrid)
	count = 0;

    if (window == 0)
	window = lastWindow;

    if (window == 0)
	return;

    lastWindow = window;

    // draw out any existing rectangles
    for (i = 0; i < lastcount; i++)
    {
	lastx = lastrect[i].x;
	lasty = lastrect[i].y;
	lastWidth = lastrect[i].width;
	lastHeight = lastrect[i].height;

	DRAW;
    }
    

    if (lastcount > MAX_RECTS && count != lastcount)
    {
	if (count <= MAX_RECTS)
	{
	    free((char *)lastrect);
	    lastrect = lastrectangles;
	}
	else
	{
	    free((char *)lastrect);
	    lastrect = (XRectangle *)malloc(sizeof(XRectangle) * count);
	}
    }

    // draw new rectangles
    for (i = 0; i < count; i++)
    {
	lastx = rect[i].x;
	lasty = rect[i].y;
	lastWidth = rect[i].width;
	lastHeight = rect[i].height;
	lastrect[i] = rect[i];

	DRAW;
    }

    lastcount = count;

    if (r != outline)
    {
	XDrawSegments(DPY, window, wmScr->outlineGC, outline, r - outline);
    }
}

/**********************************************************************
 *
 *  Procedure:
 *	wmFindScreen
 *
 *  Function:
 *	Ensure that the global wmScr pointer is correct following 
 *	any event.  This procedure MUST be called 
 *
 **********************************************************************
 */

void
wmFindScreen(
    wmData *wp,
    Window w
    )
{
}

static char *unknownName= " ";

int
wmNewWindowName(
    OI_entry_field *ef,
    void *tmp
    )
{
    if (tmp) 
    {
	wmData *wp = (wmData *)tmp;
	wp->set_name(ef->part_text());
	if (wp->name() == NULL)
	    wp->set_name(unknownName);
	wmDisplayName(wp);
    }
    return (True);
}

void 
wmDisplayName(
    wmData *wp
    )
{
    XExposeEvent ev;
    int re_layout;

    re_layout = False;
    if (wp->oi_name())
    {
	wmObjectData *odp;
	int x;

	odp = (wmObjectData *)wp->oi_name()->data();
	if (wp->oi_name()->is_derived_from("OI_entry_field"))
	{
	    OI_entry_field *ef = (OI_entry_field *)wp->oi_name();
	    if (odp->kp->geom->center)
	    {
		// unmap the window to make this look better
		ef->set_state(OI_ACTIVE_NOT_DISPLAYED);
		ef->set_dsp_length(strlen(wp->name()));
		ef->set_default_text(wp->name(), OI_NO);
		x = (wp->oi_frame()->size_x()/2) - (ef->size_x()/2);
		ef->set_loc(x, ef->loc_y());
		ef->set_state(OI_ACTIVE);
	    }
	    else
	    {
		ef->set_dsp_length(strlen(wp->name()));
		ef->set_default_text(wp->name(), OI_NO);
		re_layout = True;
	    }
	}
	else
	{
	    OI_static_text *sp = (OI_static_text *)wp->oi_name();
	    // must be a static text object
	    if (odp->kp->geom->center)
	    {
		// unmap the window to make this look better
		sp->set_state(OI_ACTIVE_NOT_DISPLAYED);
		sp->set_text(wp->name());
		x = (wp->oi_frame()->size_x()/2) - (wp->oi_name()->size_x()/2);
		sp->set_loc(x, wp->oi_name()->loc_y());
		sp->set_state(OI_ACTIVE);
	    }
	    else
	    {
		sp->set_text(wp->name());
	    }
	}
	odp = (wmObjectData *)wp->oi_frame()->data();
	wp->op()->oi = wp->oi_frame();
	wmLayoutPanel(wp->op());

	if (wmScr->pannerNames && wp->vbox()) {
		ev.count = 0;
		ev.window = wp->vbox()->X_window();
		wmPaintVirtual(&ev, wp);
	}
    }
}

void
wmExpandString(char *str)
{
    register char *i, *o;
    register n;
    register count;

    for (i=str, o=str; *i && *i != '\0'; o++)
    {
	if (*i == '\\')
	{
	    switch (*++i)
	    {
	    case 'n':
		*o = '\n';
		i++;
		break;
	    case 'b':
		*o = '\b';
		i++;
		break;
	    case 'r':
		*o = '\r';
		i++;
		break;
	    case 't':
		*o = '\t';
		i++;
		break;
	    case 'f':
		*o = '\f';
		i++;
		break;
	    case '0':
		if (*++i == 'x')
		    goto hex;
		else
		    --i;
	    case '1': case '2': case '3':
	    case '4': case '5': case '6': case '7':
		n = 0;
		count = 0;
		while (*i >= '0' && *i <= '7' && count < 3)
		{
		    n = (n<<3) + (*i++ - '0');
		    count++;
		}
		*o = n;
		break;
	    hex:
	    case 'x':
		n = 0;
		count = 0;
		while (i++, count++ < 2)
		{
		    if (*i >= '0' && *i <= '9')
			n = (n<<4) + (*i - '0');
		    else if (*i >= 'a' && *i <= 'f')
			n = (n<<4) + (*i - 'a') + 10;
		    else if (*i >= 'A' && *i <= 'F')
			n = (n<<4) + (*i - 'A') + 10;
		    else
			break;
		}
		*o = n;
		break;
	    case '\"':
	    case '\'':
	    case '\\':
	    default:
		*o = *i++;
		break;
	    }
	}
	else
	    *o = *i++;
    }
    *o = '\0';
}

static int grabCount = 0;

void
wmGrabServer()
{
    if (grabCount++ == 0)
	XGrabServer(DPY);
}

void
wmUngrabServer()
{
    if (--grabCount <= 0)
    {
	grabCount = 0;
	XUngrabServer(DPY);
    }
}

void
wmBusy(
    wmData *wp
    )
{
    XSetWindowAttributes attr;

    if (wp->busy())
    {
	wp->oi_client()->set_working();
    }
    else
    {
	wp->oi_client()->clear_working();
    }
}

void
wmBeeper(
    struct wmData *,
    XEvent *
    )
{
    XBell(DPY, 0);
}

static Atom colormaps[] = {
XA_RGB_COLOR_MAP,
XA_RGB_BEST_MAP,
XA_RGB_BLUE_MAP,
XA_RGB_DEFAULT_MAP,
XA_RGB_GRAY_MAP,
XA_RGB_GREEN_MAP,
XA_RGB_RED_MAP };

static int nummaps = sizeof(colormaps) / sizeof(Atom);

wmColormap::wmColormap(Colormap cmap)
{
    int i;

    cells_context = XUniqueContext();
    std_cmap_flag = False;

    // check to see if this is a standard colormap
    for (i = 0; i < nummaps; i++)
    {
	if (XGetStandardColormap(DPY, wmScr->root, &std, colormaps[i]))
	{
	    if (std.colormap == cmap)
	    {
		std_cmap_flag = True;
		break;
	    }
	}
    }
    std.colormap = cmap;
    XSaveContext(DPY, (Window)cmap, wmScr->colorCache, (caddr_t)this);
}

void
wmDumpObject(OI_d_tech *oi)
{
    OI_d_tech *child;

    static int level = 0;
    for (int i = 0; i < level; i++)
        fprintf(stderr, "  ");
    fprintf(stderr, "window: 0x%08X ", oi->X_window());
    if (oi->name())
        fprintf(stderr, "name: \"%s\" ", oi->name());
    fprintf(stderr, "\n");
    level += 1;
    for (child = oi->next_child(NULL); child != NULL; child = oi->next_child(child))
    {
        wmDumpObject(child);
    }
    level -= 1;
}

void
wmPaintVirtual(
    XExposeEvent *ev,
    wmData *wp
    )
{
    int len;

    if (wmScr->pannerNames && !ev->count && wp->vbox() && ev->window == wp->vbox()->X_window()) {
	wp->vbox()->set_gc();
	len = strlen(wp->name());
	XClearArea(DPY, wp->vbox()->X_window(), 1, 1, 9999, wp->vbox()->font_height(), False);
	XDrawString(DPY, wp->vbox()->X_window(), wmScr->conp->gc(), 1, 1+wp->vbox()->font_y_base(), wp->name(), len);
    }
}
