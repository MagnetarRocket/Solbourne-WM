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
 * $Id: object.C,v 9.9 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Basic object code
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/

#ifndef lint
static char RCSinfo[] =
"$Id: object.C,v 9.9 1993/08/27 16:58:15 toml Exp $";
#endif /* lint */

#include "swm.H"
#include "main.H"
#include "gram.H"
#include "list.H"
#include "object.H"
#include "panel.H"
#include "button.H"
#include "text.H"
#include "debug.H"
#include "wmdata.H"
#include "util.H"
#include "parse.H"
#include "icons.H"
#include "init.H"
#include "swmhelp.H"
#include "quarks.H"

/***************************************************************
 *
 *  Procedure:	wmCreateObject
 *
 *  Function:	Allocate a new object and add it into the object
 *		structure.
 *
 ***************************************************************/

wmObject *
wmCreateObject(
    int t,			// the object type
    char *n			// the name of the object
    )
{
    XrmQuark quark;		// quarkified name
    wmObject *op;

#ifdef DEBUG
    if (wmDebug)
	fprintf(dfp, "wmCreateObject: \"%s\"\n", n);
#endif

    quark = XrmStringToQuark(n);
    // let's first make sure we don't already have an object by this name
    for (op = (wmObject *)wmScr->objectList.first(); op != NULL;
	op = (wmObject *)wmScr->objectList.next())
    {
#ifdef DEBUG
    if (wmDebug)
	fprintf(dfp, "  op->name = \"%s\"\n", op->name);
#endif
	if (quark == op->quark && t == op->type)
		return (op);
    }

    // put it in the object list
    op = new wmObject(t, n, quark);
    wmScr->objectList.append((ent)op);

    switch (op->type)
    {
	case OBJ_PANEL:
	    if (wmQuarks->clientQuark() == op->quark)
		op->client = True;
	    else if (wmQuarks->iconsQuark() == op->quark)
		op->icons = True;
	    op->u.p = new wmPanelInfo();
	    break;

	case OBJ_BUTTON:
	    if (wmQuarks->nameQuark() == op->quark)
		op->wname = True;
	    else if (wmQuarks->iconNameQuark() == op->quark)
		op->iname = True;
	    else if (wmQuarks->iconImageQuark() == op->quark)
		op->iimage = True;
	    op->u.b = new wmButtonInfo();
	    break;

	case OBJ_TEXT:
	    if (wmQuarks->nameQuark() == op->quark)
		op->wname = True;
	    else if (wmQuarks->iconNameQuark() == op->quark)
		op->iname = True;
	    else if (wmQuarks->sizeQuark() == op->quark)
		op->size = True;
	    op->u.t = new wmTextInfo();
	    break;
	case OBJ_MENU:		break;
	case OBJ_MENUBAR:	break;
    }
    return (op);
}

void
wmExpandObjects()
{
    wmObject *op;
    int done = False;

    while (!done)
    {
	// assume we are going to be done
	done = True;

	for (op = (wmObject *)wmScr->objectList.first(); op != NULL;
	    op = (wmObject *)wmScr->objectList.next())
	{
	    if (op->expanded == False)
	    {
#ifdef DEBUG
    if (wmDebug)
	fprintf(dfp, "wmExpandObjects: expanding \"%s\"\n",op->name);
#endif
		op->expanded = True;
		done = False;

		switch (op->type)
		{
		    case OBJ_PANEL: 
			wmCreatePanel(op);
			break;

		    case OBJ_BUTTON:
			wmCreateButton(op);
			break;
			    
		    case OBJ_TEXT:
			wmCreateText(op);
			break;

		    case OBJ_MENU:
			break;

		    case OBJ_MENUBAR:
			break;
		}
	    }
	}
    }
}

OI_d_tech *
wmInstantiateObject(wmObject *op)
{
    OI_d_tech *dp;

    op->odp = new wmObjectData(op);

    switch (op->type)
    {
	case OBJ_PANEL: 
	    dp = (OI_d_tech *)wmInstantiatePanel(op);
	    op->odp->pad = op->u.p->pad;
	    break;
		
	case OBJ_BUTTON:
	    dp = (OI_d_tech *)wmInstantiateButton(op);
	    break;
		
	case OBJ_TEXT:
	    dp = (OI_d_tech *)wmInstantiateText(op);
	    break;
		
	case OBJ_MENU:
	case OBJ_MENUBAR:
		break;
    }

    dp->set_data((void *)op->odp);
    dp->set_name(op->name);
    if (!wmNameObjects)
	XDeleteProperty(DPY, dp->outside_X_window(), XA_WM_NAME);
    if (wmScr->help)
	OI_dispatch_insert(dp->outside_X_window(), EnterNotify, EnterWindowMask, (OI_event_fnp)wmHelpEnterNotify, (char *)dp);
    SAVE_INTERNAL(dp->outside_X_window());
    return (dp);
}

OI_state
wmState(wmObjectState st)
{
    if ((st & wmIsMapped))
	return (OI_ACTIVE);
    else
	return (OI_ACTIVE_NOT_DISPLAYED);
}

/***************************************************************
 *
 *  Procedure:	wmObject::wmObject
 *
 *  Function:	Allocate a new wmObject structure
 *
 ***************************************************************/

wmObject::wmObject(
    int t,			// the object type
    char *n,			// the name of the object
    XrmQuark q			// quarkified name
    )
{
    char *src, *dst;
    char buff[200];
#ifdef DEBUG
    if (wmDebug)
	fprintf(dfp, "  new wmObject: t=%d, n=\"%s\"\n", t, n);
#endif

    type = t;
    name = n;
    quark = q;
    numQuarks = 0;
    for (src = n, dst = buff; *src != '\0'; *src++)
    {
	if (*src == '.')
	{
	    *dst = '\0';
	    quarks[numQuarks++] = XrmStringToQuark(buff);
	    dst = buff;
	}
	else
	    *dst++ = *src;	
    }
    if (dst != buff)
    {
	*dst = '\0';
	quarks[numQuarks++] = XrmStringToQuark(buff);
    }
    numQuarks--;
    expanded = False;
    client = False;
    wname = False;
    size = False;
    iname = False;
    iimage = False;
    icons = False;
    root = False;
    oi = NULL;
}

/***************************************************************
 *
 *  Procedure:	wmObjectData::wmObjectData
 *
 *  Function:	Allocate a new wmObjectData structure
 *
 ***************************************************************/

wmObjectData::wmObjectData(
    wmObject *o			// the object pointer
    )
{
#ifdef DEBUG
    if (wmDebug)
	fprintf(dfp, "  new wmObjectData:\n");
#endif

    op = o;	
    bps = NULL;
    kp = NULL;
    wp = wmClient;
    state = wmMapped;
    gravity = NorthGravity;
    inAnIcon = wmMakingIcon;
    handlePress = False;
    handleKey = False;
    handleDrop = False;
    pixmap = NULL;
    frame = False;
    icon = False;
    sweep = 0;
    shape = False;
    shapePixmap = None;
    shapeWindow = None;
    hs = NULL;
    parent = NULL;
    pad = 0;
}
