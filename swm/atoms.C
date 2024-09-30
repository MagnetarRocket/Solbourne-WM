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
 * $Id: atoms.C,v 9.17 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Useful atom routines
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/

#ifndef lint
static char RCSinfo[] =
"$Id: atoms.C,v 9.17 1993/08/27 16:58:15 toml Exp $";
#endif /* lint */

#include "swm.H"
#include "main.H"
#include "wmdata.H"
#include "util.H"
#include "atoms.H"
#include "init.H"
#include "ol.H"
#include "motif.H"
#include "swmstart.H"
#include "quarks.H"

// ICCCM atoms
Atom WM_STATE = None;
Atom WM_CHANGE_STATE = None;
Atom WM_COLORMAP_WINDOWS = None;
Atom WM_DELETE_WINDOW = None;
Atom WM_SHUTDOWN = None;
Atom WM_SAVE_YOURSELF = None;
Atom WM_PROTOCOLS = None;
Atom WM_TAKE_FOCUS = None;
Atom WM_COMMAND = None;

// private swm atoms
Atom __SWM_COMMAND = None;
Atom __SWM_HINTS = None;
Atom __SWM_ROOT = None;
Atom __SWM_VROOT = None;
Atom __SWM_START = None;
Atom __SWM_VERSION = None;

// Open Look atoms 
Atom _OL_WIN_ATTR = None;
Atom _OL_DECOR_ADD = None;
Atom _OL_DECOR_DEL = None;
Atom _OL_DECOR_CLOSE = None;
Atom _OL_DECOR_FOOTER = None;
Atom _OL_DECOR_RESIZE = None;
Atom _OL_DECOR_HEADER = None;
Atom _OL_DECOR_OK = None;
Atom _OL_DECOR_PIN = None;
Atom _OL_SCALE_SMALL = None;
Atom _OL_SCALE_MEDIUM = None;
Atom _OL_SCALE_LARGE = None;
Atom _OL_SCALE_XLARGE = None;
Atom _OL_PIN_STATE = None;
Atom _OL_WIN_BUSY = None;
Atom _OL_WINMSG_STATE = None;
Atom _OL_WINMSG_ERROR = None;
Atom _OL_WT_BASE = None;
Atom _OL_WT_CMD = None;
Atom _OL_WT_PROP = None;
Atom _OL_WT_HELP = None;
Atom _OL_WT_NOTICE = None;
Atom _OL_WT_OTHER = None;
Atom _OL_MENU_FULL = None;
Atom _OL_MENU_LIMITED = None;
Atom _OL_NONE = None;
Atom _OL_PIN_IN = None;
Atom _OL_PIN_OUT = None;
Atom _OL_WIN_DISMISS = None;
Atom _OL_BORDER_SIZES = None;
Atom _OL_DFLT_BTN = None;

// motif atoms
Atom _MOTIF_WM_HINTS = None;
Atom _MOTIF_WM_MENU = None;
Atom _MOTIF_WM_MESSAGES = None;
Atom _MOTIF_WM_INFO = None;

// XView atoms
Atom XA_XV_DO_DRAG_LOAD = None;

// Help Manager atoms
Atom HM_STATE = None;

/***********************************************************************
 *
 *  Procedure:
 *      wmInitAtoms
 *
 *  Function:
 *	Initialize needed atoms
 *
 ***********************************************************************
 */

void
wmInitAtoms()
{
    // init the ICCCM atoms
    WM_STATE 		= XInternAtom(DPY, "WM_STATE", False);
    WM_CHANGE_STATE 	= XInternAtom(DPY, "WM_CHANGE_STATE", False);
    WM_COLORMAP_WINDOWS	= XInternAtom(DPY, "WM_COLORMAP_WINDOWS", False);
    WM_DELETE_WINDOW 	= XInternAtom(DPY, "WM_DELETE_WINDOW", False);
    WM_SHUTDOWN 	= XInternAtom(DPY, "WM_SHUTDOWN", False);
    WM_SAVE_YOURSELF 	= XInternAtom(DPY, "WM_SAVE_YOURSELF", False);
    WM_PROTOCOLS 	= XInternAtom(DPY, "WM_PROTOCOLS", False);
    WM_TAKE_FOCUS 	= XInternAtom(DPY, "WM_TAKE_FOCUS", False);
    WM_COMMAND 		= XInternAtom(DPY, "WM_COMMAND", False);

    // init the private swm atoms
    __SWM_COMMAND 	= XInternAtom(DPY, "__SWM_COMMAND", False);
    __SWM_HINTS 	= XInternAtom(DPY, "__SWM_HINTS", False);
    __SWM_ROOT	 	= XInternAtom(DPY, "__SWM_ROOT", False);
    __SWM_VROOT	 	= XInternAtom(DPY, "__SWM_VROOT", False);
    __SWM_START 	= XInternAtom(DPY, "__SWM_START", False);
    __SWM_VERSION 	= XInternAtom(DPY, "__SWM_VERSION", False);

    // init the Open Look atoms
    _OL_WIN_ATTR  	= XInternAtom(DPY, "_OL_WIN_ATTR", False);
    _OL_DECOR_ADD  	= XInternAtom(DPY, "_OL_DECOR_ADD", False);
    _OL_DECOR_DEL  	= XInternAtom(DPY, "_OL_DECOR_DEL", False);
    _OL_DECOR_CLOSE  	= XInternAtom(DPY, "_OL_DECOR_CLOSE", False);
    _OL_DECOR_FOOTER  	= XInternAtom(DPY, "_OL_DECOR_FOOTER", False);
    _OL_DECOR_RESIZE  	= XInternAtom(DPY, "_OL_DECOR_RESIZE", False);
    _OL_DECOR_HEADER  	= XInternAtom(DPY, "_OL_DECOR_HEADER", False);
    _OL_DECOR_OK  	= XInternAtom(DPY, "_OL_DECOR_OK", False);
    _OL_DECOR_PIN  	= XInternAtom(DPY, "_OL_DECOR_PIN", False);
    _OL_SCALE_SMALL  	= XInternAtom(DPY, "_OL_SCALE_SMALL", False);
    _OL_SCALE_MEDIUM  	= XInternAtom(DPY, "_OL_SCALE_MEDIUM", False);
    _OL_SCALE_LARGE  	= XInternAtom(DPY, "_OL_SCALE_LARGE", False);
    _OL_SCALE_XLARGE  	= XInternAtom(DPY, "_OL_SCALE_XLARGE", False);
    _OL_PIN_STATE  	= XInternAtom(DPY, "_OL_PIN_STATE", False);
    _OL_WIN_BUSY  	= XInternAtom(DPY, "_OL_WIN_BUSY", False);
    _OL_WINMSG_STATE  	= XInternAtom(DPY, "_OL_WINMSG_STATE", False);
    _OL_WINMSG_ERROR  	= XInternAtom(DPY, "_OL_WINMSG_ERROR", False);
    _OL_WT_BASE  	= XInternAtom(DPY, "_OL_WT_BASE", False);
    _OL_WT_CMD  	= XInternAtom(DPY, "_OL_WT_CMD", False);
    _OL_WT_PROP  	= XInternAtom(DPY, "_OL_WT_PROP", False);
    _OL_WT_HELP  	= XInternAtom(DPY, "_OL_WT_HELP", False);
    _OL_WT_NOTICE  	= XInternAtom(DPY, "_OL_WT_NOTICE", False);
    _OL_WT_OTHER  	= XInternAtom(DPY, "_OL_WT_OTHER", False);
    _OL_MENU_FULL  	= XInternAtom(DPY, "_OL_MENU_FULL", False);
    _OL_MENU_LIMITED  	= XInternAtom(DPY, "_OL_MENU_LIMITED", False);
    _OL_NONE  		= XInternAtom(DPY, "_OL_NONE", False);
    _OL_PIN_IN  	= XInternAtom(DPY, "_OL_PIN_IN", False);
    _OL_PIN_OUT  	= XInternAtom(DPY, "_OL_PIN_OUT", False);
    _OL_WIN_DISMISS  	= XInternAtom(DPY, "_OL_WIN_DISMISS", False);
    _OL_BORDER_SIZES  	= XInternAtom(DPY, "_OL_BORDER_SIZES", False);
    _OL_DFLT_BTN  	= XInternAtom(DPY, "_OL_DFLT_BTN", False);

    _MOTIF_WM_HINTS	= XInternAtom(DPY, "_MOTIF_WM_HINTS", False);
    _MOTIF_WM_MENU	= XInternAtom(DPY, "_MOTIF_WM_MENU", False);
    _MOTIF_WM_MESSAGES	= XInternAtom(DPY, "_MOTIF_WM_MESSAGES", False);
    _MOTIF_WM_INFO	= XInternAtom(DPY, "_MOTIF_WM_INFO", False);

    // init XView atoms
    XA_XV_DO_DRAG_LOAD = XInternAtom(DPY, "XV_DO_DRAG_LOAD", False);

    // init Help Manager atoms
    HM_STATE = XInternAtom(DPY, "HM_STATE", False);
}

/***********************************************************************
 *
 *  Procedure:
 *      wmSet__SWM_HINTS
 *
 *  Function:
 *	Change the __SWM_HINTS propery on the client window
 *
 ***********************************************************************
 */

void
wmSet__SWM_HINTS(
    wmData *wp		// the window data structure
    )
{
    if (wp->has_client())
    {
	if (wp->oi_icon())
	{
	    wp->set_icon_x((int)wp->oi_icon()->loc_x());
	    wp->set_icon_y((int)wp->oi_icon()->loc_y());
	}

	if (!wp->mine())
		XChangeProperty(DPY, wp->window(), __SWM_HINTS, __SWM_HINTS, 8, PropModeReplace, (unsigned char *) wp->myhints_p(),
			 sizeof(wp->myhints()));
    }
}

/***********************************************************************
 *
 *  Procedure:
 *      wmSet__SWM_ROOT
 *
 *  Function:
 *	Change the __SWM_ROOT property on the client window
 *
 ***********************************************************************
 */

void
wmSet__SWM_ROOT(
    wmData *wp		// the window data structure
    )
{
    if (!wp->mine())
    {
	Window w = wp->root()->X_window();
	XChangeProperty(DPY, wp->window(), __SWM_ROOT, XA_WINDOW, 32, PropModeReplace, (unsigned char *)&w, 1);
    }
}

/***********************************************************************
 *
 *  Procedure:
 *      wmGetOpenLookAtoms
 *
 *  Function:
 *	Get private Open Look atoms and add the appropriate strings
 *	to the current resource string.
 *
 ***********************************************************************
 */

void
wmGetOpenLookAtoms(
    wmData *wp		// the window data structure
    )
{
#define OL_DECORATIONS 16
    static XrmQuark openLookQuark = (int)None;
    static XrmQuark decorationQuark[OL_DECORATIONS];
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytesafter;
    ShortOLWinAttr olWinAttr;
    ShortOLWinAttr *attr = NULL;
    LongOLWinAttr *lattr = NULL;
    int freeAttr = False;
    Atom *data = NULL;
    int i;
    int *intdata = NULL;
    char buff[25];

    // this code assumes that all open look windows have an OL_WIN_ATTR
    // property on them, otherwise, no open look properties are
    // looked for
    if (XGetWindowProperty (DPY, wp->window(), _OL_WIN_ATTR,0,1000,
	False, _OL_WIN_ATTR, &actual_type, &actual_format, &nitems, &bytesafter,
			    (unsigned char **) &attr) == Success && attr)
    {
	freeAttr = True;
	if (nitems == (sizeof(LongOLWinAttr)/4)) {
		lattr = (LongOLWinAttr *)attr;
		olWinAttr.win_type = lattr->win_type;
		olWinAttr.menu_type = lattr->menu_type;
		olWinAttr.pin_initial_state = lattr->pin_initial_state;
		XFree((char *)attr);
		attr = &olWinAttr;
		freeAttr = False;
	}

	if (!openLookQuark) 
	{
	    openLookQuark = XrmStringToQuark("openLook");
	    for (i = 0; i < OL_DECORATIONS; i++)
	    {
		sprintf(buff, "%d", i);
		decorationQuark[i] = XrmStringToQuark(buff);
	    }
	}

//	wp->set_ol_win_attr(attr);
	if (attr->win_type == _OL_WT_BASE)
	{
	    wp->set_ol_decoration(OL_HEADER | OL_CLOSE | OL_RESIZE);
	}
	else if (attr->win_type == _OL_WT_CMD)
	{
	    wp->set_ol_decoration(OL_HEADER | OL_PIN | OL_RESIZE);
	}
	else if (attr->win_type == _OL_WT_HELP)
	{
	    wp->set_ol_decoration(OL_HEADER | OL_PIN);
	}

	// now that we have the basic decoration setup, let's see if
	// we need to add or subtract various pieces
	if (XGetWindowProperty (DPY, wp->window(), _OL_DECOR_ADD,
		0L, 1000000L, False, XA_ATOM, &actual_type, &actual_format,
		&nitems, &bytesafter, (unsigned char **) &data)
	    == Success && data)
	{
	    if (actual_type == XA_ATOM && actual_format == 32)
	    {
		for (i = 0; i < nitems; i++)
		{
		    if (data[i] == _OL_DECOR_CLOSE)
			wp->or_ol_decoration(OL_CLOSE);
		    else if (data[i] == _OL_DECOR_RESIZE)
			wp->or_ol_decoration(OL_RESIZE);
		    else if (data[i] == _OL_DECOR_HEADER)
			wp->or_ol_decoration(OL_HEADER);
		    else if (data[i] == _OL_DECOR_PIN)
			wp->or_ol_decoration(OL_PIN);
		}
	    }
	}

	if (XGetWindowProperty (DPY, wp->window(), _OL_DECOR_DEL,
		0L, 1000000L, False, XA_ATOM, &actual_type, &actual_format,
		&nitems, &bytesafter, (unsigned char **) &data)
	    == Success && data)
	{
	    if (actual_type == XA_ATOM && actual_format == 32)
	    {
		for (i = 0; i < nitems; i++)
		{
		    if (data[i] == _OL_DECOR_CLOSE)
			wp->and_ol_decoration(~OL_CLOSE);
		    else if (data[i] == _OL_DECOR_RESIZE)
			wp->and_ol_decoration(~OL_RESIZE);
		    else if (data[i] == _OL_DECOR_HEADER)
			wp->and_ol_decoration(~OL_HEADER);
		    else if (data[i] == _OL_DECOR_PIN)
			wp->and_ol_decoration(~OL_PIN);
		}
	    }
	}

	RM->pushq(openLookQuark, openLookQuark);
	RM->pushq(decorationQuark[wp->ol_decoration()], decorationQuark[wp->ol_decoration()]);

	if (attr->menu_type == _OL_MENU_FULL)
	{
	    RM->pushq(wmQuarks->olmenuName(), wmQuarks->olmenuClass());
	    RM->pushq(wmQuarks->fullName(), wmQuarks->fullClass());
	}
	else if (attr->menu_type == _OL_MENU_LIMITED)
	{
	    RM->pushq(wmQuarks->olmenuName(), wmQuarks->olmenuClass());
	    RM->pushq(wmQuarks->limitedName(), wmQuarks->limitedClass());
	}
	else if (attr->menu_type == _OL_NONE)
	{
	    RM->pushq(wmQuarks->olmenuName(), wmQuarks->olmenuClass());
	    RM->pushq(wmQuarks->noneName(), wmQuarks->noneClass());
	}

	// if the pin is being used, check the state and add it
	// to the resource string
	if (wp->ol_decoration() & OL_PIN)
	{
	    wp->set_has_pushpin();
	    if (attr->pin_initial_state == _OL_PIN_IN)
	    {
		RM->pushq(wmQuarks->pinInName(), wmQuarks->pinInClass());
		wmSet_OL_PIN_STATE(wp, 1);
	    }
	    else
	    {
		RM->pushq(wmQuarks->pinOutName(), wmQuarks->pinOutClass());
		wmSet_OL_PIN_STATE(wp, 0);
	    }
	}

	// check to see if there was a default button for the thing
	XGetWindowProperty (DPY, wp->window(), _OL_WIN_BUSY,
		0L, 1000000L, False, XA_INTEGER, &actual_type, &actual_format,
		&nitems, &bytesafter, (unsigned char **) &intdata);

	if (intdata && *intdata)
	    wp->set_busy();

	if (wmInitDone)
	{
	    // check to see if there was a default button for the thing
	    XGetWindowProperty (DPY, wp->window(), _OL_DFLT_BTN,
		    0L, 1000000L, False, XA_INTEGER, &actual_type, &actual_format,
		    &nitems, &bytesafter, (unsigned char **) wp->ol_default_btn_p());

	    if (wp->ol_default_btn())
		wp->set_warp_back();
	}
    }
    if (freeAttr)
	XFree((char *)attr);

    return;
}

/***********************************************************************
 *
 *  Procedure:
 *      wmSet_OL_PIN_STATE
 *
 *  Function:
 *	Set the _OL_PIN_STATE property on the client window due
 *	to an f.pin or f.unpin command.
 *
 ***********************************************************************
 */

void
wmSet_OL_PIN_STATE(
    wmData *wp, 	// the window data structure
    int pin_state	// the state of the pin  1 == pinned,  0 == unpinned
    )
{
    XChangeProperty(DPY, wp->window(), _OL_PIN_STATE, XA_INTEGER, 32, PropModeReplace, (unsigned char *)&pin_state, 1);
}

/***********************************************************************
 *
 *  Procedure:
 *      wmGetMotifAtoms
 *
 *  Function:
 *	Get the Motif private atoms from the client window.  Note that
 *	this routine is not yet complete.
 *
 ***********************************************************************
 */

void
wmGetMotifAtoms(
    wmData *wp	 	// the window data structure
    )
{
#define MOTIF_DECORATION_MASK (MWM_DECOR_BORDER | MWM_DECOR_RESIZEH | MWM_DECOR_TITLE | MWM_DECOR_MENU | \
		MWM_DECOR_MINIMIZE | MWM_DECOR_MAXIMIZE | MWM_DECOR_ALL)
#define MOTIF_DECORATIONS 16
    static XrmQuark motifDecoration = (int)None;
    static XrmQuark motifResize = (int)None;
    static XrmQuark motifBorder = (int)None;
    static XrmQuark decorationQuark[MOTIF_DECORATIONS];
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytesafter;
    MotifWMHints *hints = NULL;
    Atom *data = NULL;
    int *intdata = NULL;
    char buff[25];
    long decoration;
    int i;

    if (XGetWindowProperty (DPY, wp->window(), _MOTIF_WM_HINTS,0,(sizeof(MotifWMHints)/4),
	False, _MOTIF_WM_HINTS, &actual_type, &actual_format, &nitems, &bytesafter,
			    (unsigned char **) &hints) == Success && hints)
    {
	if (!motifDecoration) 
	{
	    motifDecoration = XrmStringToQuark("motifDecoration");
	    motifResize = XrmStringToQuark("motifResize");
	    motifBorder = XrmStringToQuark("motifBorder");
	    for (i = 0; i < MOTIF_DECORATIONS; i++)
	    {
		sprintf(buff, "%d", i);
		decorationQuark[i] = XrmStringToQuark(buff);
	    }
	}

#ifdef TL_DEBUG
	printf("length req = %d\n", (sizeof(MotifWMHints)/4));
	printf("nitems     = %d\n", nitems);
	printf("bytesafter = %d\n", bytesafter);
	printf("_MOTIF_WM_HINTS\n");
	printf("  flags       = 0x%08X\n", hints->flags);
	printf("  decorations = 0x%08X\n", hints->decorations);
	printf("  functions   = 0x%08X\n", hints->functions);
	printf("  input_mode  = 0x%08X\n", hints->input_mode);

	if (hints->flags & MWM_HINTS_FUNCTIONS)
	{
	    printf("  MWM_HINTS_FUNCTIONS\n");
	    if (hints->functions & MWM_FUNC_ALL)
		printf("    MWM_FUNC_ALL\n");
	    if (hints->functions & MWM_FUNC_RESIZE)
		printf("    MWM_FUNC_RESIZE\n");
	    if (hints->functions & MWM_FUNC_MOVE)
		printf("    MWM_FUNC_MOVE\n");
	    if (hints->functions & MWM_FUNC_MINIMIZE)
		printf("    MWM_FUNC_MINIMIZE\n");
	    if (hints->functions & MWM_FUNC_MAXIMIZE)
		printf("    MWM_FUNC_MAXIMIZE\n");
	    if (hints->functions & MWM_FUNC_CLOSE)
		printf("    MWM_FUNC_CLOSE\n");
	}
#endif /* TL_DEBUG */
	if (hints->flags & MWM_HINTS_DECORATIONS)
	{
	    hints->decorations &= MOTIF_DECORATION_MASK;

	    if(hints->decorations & MWM_DECOR_ALL)
		decoration = MOTIF_DECORATION_MASK & (~hints->decorations);
	    else
		decoration = hints->decorations;
	    
	    i = (int)(decoration >> 3);
	    RM->pushq(motifDecoration, motifDecoration);
	    RM->pushq(decorationQuark[i], decorationQuark[i]);

	    if (decoration & MWM_DECOR_RESIZEH)
		RM->pushq(motifResize, motifResize);
	    if (decoration & MWM_DECOR_BORDER)
		RM->pushq(motifBorder, motifBorder);
	}
    }
}

/***********************************************************************
 *
 *  Procedure:
 *      wmGetWM_COMMAND
 *
 *  Function:
 *	Get the WM_COMMAND property from the client window
 *
 ***********************************************************************
 */

char *
wmGetWM_COMMAND(
    wmData *wp	 	// the window data structure
    )
{
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytesafter;
    char *sp, *dp;
    char *retp = NULL;

    if (XGetWindowProperty(DPY, wp->window(), XA_WM_COMMAND, 0L, 1000000L, False,
	XA_STRING, &actual_type, &actual_format, &nitems, &bytesafter, (unsigned char **)&sp) == Success && sp)
    {
	retp = dp = (char *)malloc((unsigned int)(nitems + 1));
	char *done = sp + nitems;
	for (; sp < done; sp++)
	{
	    if (*sp)
		*dp++ = *sp;
	    else
		*dp++ = ' ';
	}
	*dp = '\0';
    }
    return (retp);
}

/***********************************************************************
 *
 *  Procedure:
 *      wmGet__SWM_START
 *
 *  Function:
 *	Get the __SWM_START property from the root window then create
 *	a list of windows that should be restarting.
 *
 ***********************************************************************
 */

void
wmGet__SWM_START()
{
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytesafter;
    SWMStart *sp = NULL;

    if (XGetWindowProperty (DPY, wmScr->root, __SWM_START,0,1000000L,
	True, __SWM_START, &actual_type, &actual_format, &nitems, &bytesafter,
			    (unsigned char **) &sp) == Success && sp)
    {
	SWMStart *last = (SWMStart *)((char *)sp + nitems);
	while (sp < last)
	{
	    if (sp->bytes == 0)
		return;
	    wmScr->startList.append((ent)sp);
	    sp = (SWMStart *)((char *)sp + sp->bytes);
	}
    }
}

/***********************************************************************
 *
 *  Procedure:
 *      wmSet__SWM_START
 *
 *  Function:
 *	Append to the __SWM_START property on the root window, saving
 *	needed restart information.
 *
 ***********************************************************************
 */

void
wmSet__SWM_START(wmData *wp)
{
    OI_d_tech *oi;
    SWMStart *sp;
    int len;
    char *cmd;

    if (cmd = wmGetWM_COMMAND(wp)) {
	len = sizeof(SWMStart);
	len += strlen(cmd) + 1;
	len = (len + 3) & ~0x03;

	sp = (SWMStart *)malloc(len);

	sp->bytes = len;
	sp->sticky = wp->sticky();
	sp->state = wp->initial_state();
	sp->rootIcon = wp->icon_window() ? True : False;
	sp->iconGravity = wp->icon_gravity() ? True : False;
	sp->geomX = (int)wp->oi_frame()->loc_x();
	sp->geomY = (int)wp->oi_frame()->loc_y();
	if (wp->panner())
	{
	    sp->geomWidth = wmScr->vwidth;
	    sp->geomHeight = wmScr->vheight;
	}
	else
	{
	    sp->geomWidth = wp->attr_width();
	    sp->geomHeight = wp->attr_height();
	}
	sp->iconified = wp->iconified();
	sp->igeomX = wp->icon_x();
	sp->igeomY = wp->icon_y();
	sp->gravityOrder = wp->gravity_order();
	sp->cmd_index = 0;
	strcpy(&sp->ptr[sp->cmd_index], cmd);
	sp->machine_index = -1;

	XChangeProperty(DPY, wmScr->root, __SWM_START, __SWM_START, 8, PropModeAppend, (unsigned char *)sp, len);
    }
}

void
wmPutVersion()
{
    static char *version = SWM_VERSION;
    XChangeProperty(DPY,wmScr->root,__SWM_VERSION, XA_STRING, 8, PropModeReplace, (unsigned char *)version, strlen(version));
}

void
wmRemoveVersion()
{
    XDeleteProperty(DPY, wmScr->root, __SWM_VERSION);
}
