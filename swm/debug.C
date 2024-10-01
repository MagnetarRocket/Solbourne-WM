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
 * $Id: debug.C,v 9.10 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Debug utilities
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/

#ifndef lint
static char RCSinfo[] =
"$Id: debug.C,v 9.10 1993/08/27 16:58:15 toml Exp $";
#endif /* lint */

#include "swm.H"
#include "main.H"
#include "screen.H"
#include "gram.H"
#include "list.H"
#include "object.H"
#include "panel.H"
#include "util.H"

int wmDebug = False;
int wmDebugEvents = False;
FILE *dfp = NULL;		// debug file
FILE *efp = NULL;		// event debug file

void wmPrintObjectType(int t);

/***********************************************************************
 *
 *  Procedure:
 *      wmInitDebug
 *
 *  Function:
 *	Initialize debug variables
 *
 ***********************************************************************
 */

void 
wmInitDebug()
{
    char *ptr = NULL;

    setbuf(stderr, NULL);
    ptr = RM->get_resource("debugFile", "DebugFile");
    if (ptr == NULL)
	dfp = stderr;
    else
    {
	ptr = wmExpandFilename(ptr);
	if ((dfp = fopen(ptr, "w")) == NULL)
	{
	    fprintf(stderr, "swm: wmInitDebug:  couldn't open \"%s\"\n", ptr);
		dfp = stderr;
	}
    }
    ptr = RM->get_resource("eventFile", "EventFile");
    if (ptr == NULL)
	efp = dfp;
    else
    {
	ptr = wmExpandFilename(ptr);
	if ((efp = fopen(ptr, "w")) == NULL)
	{
	    fprintf(stderr, "swm: wmInitDebug:  couldn't open \"%s\"\n", ptr);
		efp = dfp;
	}
    }
#if !defined(SYSV) && !defined(_AIX) && !defined(ultrix)
    setlinebuf(dfp);
    setlinebuf(efp);
#endif

    wmDebug = 0;
    ptr = (char *)RM->get_resource("debug", "Debug");
    if (ptr) wmDebug = atoi(ptr);
    wmDebugEvents = (int)RM->get_resource("debugEvents", "DebugEvents");
}

void
wmListObjects()
{
    wmObject *op;
    wmPanelKid *kp;

    if (!wmDebug)
	return;

    for (op = (wmObject *)wmScr->objectList.first(); op != NULL;
	op = (wmObject *)wmScr->objectList.next())
    {
	fprintf(dfp, "Object --------------------------\n");
	wmPrintObjectType(op->type);
	fprintf(dfp, "  name = \"%s\"  0x%x\n", op->name, op->name);
	if (op->type == OBJ_PANEL && op->u.p->geom)
	    fprintf(dfp, "  geom = %c%d%c%d\n",
		op->u.p->geom->sign_x < 0 ? '-' : '+', op->u.p->geom->x,
		op->u.p->geom->sign_y < 0 ? '-' : '+', op->u.p->geom->y);
	if (op->type == OBJ_PANEL && op->u.p->kids)
	{
	    for (kp = op->u.p->kids; kp != NULL; kp = kp->next)
	    {
		fprintf(dfp, "  Kid  --------------------------\n");
		fprintf(dfp, "  ");
		wmPrintObjectType(kp->type);
		fprintf(dfp, "    name = \"%s\"  0x%x\n",kp->name,kp->name);
		fprintf(dfp, "    geom = %c%d%c%d\n",
		    kp->geom->sign_x < 0 ? '-' : '+', kp->geom->x,
		    kp->geom->sign_y < 0 ? '-' : '+', kp->geom->y);
	    }
	    fprintf(dfp, "  Layout  --------------------------\n");
	    if (op->type == OBJ_PANEL)
	    {
		for (int i = 0; i < op->u.p->rows; i++)
		{
		    wmPanelLayout *lp;

		    fprintf(dfp, "    Row %d --------------------------\n",i);
		    for (lp = op->u.p->layout[i]; lp != NULL; lp = lp->next)
		    {
			fprintf(dfp, "      name = \"%s\"\n",lp->kid->name);
			fprintf(dfp, "      geom = %c%d%c%d\n",
			    lp->kid->geom->sign_x < 0 ? '-' : '+',
			    lp->kid->geom->x,
			    lp->kid->geom->sign_y < 0 ? '-' : '+',
			    lp->kid->geom->y);
		    }
		}
	    }
	}
    }
}

void
wmPrintObjectType(int t)
{
    if (!wmDebug)
	return;

    fprintf(dfp, "  type = ");
    switch (t)
    {
	case OBJ_PANEL:		fprintf(dfp, "OBJ_PANEL\n"); break;
	case OBJ_BUTTON:	fprintf(dfp, "OBJ_BUTTON\n"); break;
	case OBJ_TEXT:		fprintf(dfp, "OBJ_TEXT\n"); break;
	case OBJ_MENU:		fprintf(dfp, "OBJ_MENU\n"); break;
	case OBJ_MENUBAR:	fprintf(dfp, "OBJ_MENUBAR\n"); break;
    }
}

#ifdef DEBUG
/*
 * xev - event diagnostics
 *
 * $XConsortium: xev.c,v 1.10 89/12/12 14:50:23 rws Exp $
 *
 * Copyright 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:  Jim Fulton, MIT X Consortium
 */
#include <X11/Xproto.h>

static char *Yes = "YES";
static char *No = "NO";
static char *Unknown = "unknown";


void
prologue (XEvent *eventp, char *event_name)
{
    XAnyEvent *e = (XAnyEvent *) eventp;

    fprintf(stderr, "%s event, serial %ld, synthetic %s, window 0x%lx,\n",
	    event_name, e->serial, e->send_event ? Yes : No, e->window);
    return;
}


void
do_KeyPress (XEvent *eventp)
{
    XKeyEvent *e = (XKeyEvent *) eventp;
    KeySym ks;
    char *ksname;
    int nbytes;
    char str[256+1];

    nbytes = XLookupString (e, str, 256, &ks, NULL);
    if (ks == NoSymbol)
	ksname = "NoSymbol";
    else if (!(ksname = XKeysymToString (ks)))
	ksname = "(no name)";
    fprintf(stderr, "    root 0x%lx, subw 0x%lx, time %lu, (%d,%d), root:(%d,%d),\n",
	    e->root, e->subwindow, e->time, e->x, e->y, e->x_root, e->y_root);
    fprintf(stderr, "    state 0x%x, keycode %u (keysym 0x%x, %s), same_screen %s,\n",
	    e->state, e->keycode, ks, ksname, e->same_screen ? Yes : No);
    if (nbytes < 0) nbytes = 0;
    if (nbytes > 256) nbytes = 256;
    str[nbytes] = '\0';
    fprintf(stderr, "    XLookupString gives %d characters:  \"%s\"\n", nbytes, str);

    return;
}

void
do_KeyRelease (XEvent *eventp)
{
    do_KeyPress (eventp);		/* since it has the same info */
    return;
}

void
do_ButtonPress (XEvent *eventp)
{
    XButtonEvent *e = (XButtonEvent *) eventp;

    fprintf(stderr, "    root 0x%lx, subw 0x%lx, time %lu, (%d,%d), root:(%d,%d),\n",
	    e->root, e->subwindow, e->time, e->x, e->y, e->x_root, e->y_root);
    fprintf(stderr, "    state 0x%x, button %u, same_screen %s\n",
	    e->state, e->button, e->same_screen ? Yes : No);

    return;
}

void
do_ButtonRelease (XEvent *eventp)
{
    do_ButtonPress (eventp);		/* since it has the same info */
    return;
}

void
do_MotionNotify (XEvent *eventp)
{
    XMotionEvent *e = (XMotionEvent *) eventp;

    fprintf(stderr, "    root 0x%lx, subw 0x%lx, time %lu, (%d,%d), root:(%d,%d),\n",
	    e->root, e->subwindow, e->time, e->x, e->y, e->x_root, e->y_root);
    fprintf(stderr, "    state 0x%x, is_hint %u, same_screen %s\n",
	    e->state, e->is_hint, e->same_screen ? Yes : No);

    return;
}

void
do_EnterNotify (XEvent *eventp)
{
    XCrossingEvent *e = (XCrossingEvent *) eventp;
    char *mode, *detail;
    char dmode[10], ddetail[10];

    switch (e->mode) {
      case NotifyNormal:  mode = "NotifyNormal"; break;
      case NotifyGrab:  mode = "NotifyGrab"; break;
      case NotifyUngrab:  mode = "NotifyUngrab"; break;
      case NotifyWhileGrabbed:  mode = "NotifyWhileGrabbed"; break;
      default:  mode = dmode, sprintf(dmode, "%u", e->mode); break;
    }

    switch (e->detail) {
      case NotifyAncestor:  detail = "NotifyAncestor"; break;
      case NotifyVirtual:  detail = "NotifyVirtual"; break;
      case NotifyInferior:  detail = "NotifyInferior"; break;
      case NotifyNonlinear:  detail = "NotifyNonlinear"; break;
      case NotifyNonlinearVirtual:  detail = "NotifyNonlinearVirtual"; break;
      case NotifyPointer:  detail = "NotifyPointer"; break;
      case NotifyPointerRoot:  detail = "NotifyPointerRoot"; break;
      case NotifyDetailNone:  detail = "NotifyDetailNone"; break;
      default:  detail = ddetail; sprintf(ddetail, "%u", e->detail); break;
    }

    fprintf(stderr, "    root 0x%lx, subw 0x%lx, time %lu, (%d,%d), root:(%d,%d),\n",
	    e->root, e->subwindow, e->time, e->x, e->y, e->x_root, e->y_root);
    fprintf(stderr, "    mode %s, detail %s, same_screen %s,\n",
	    mode, detail, e->same_screen ? Yes : No);
    fprintf(stderr, "    focus %s, state %u\n", e->focus ? Yes : No, e->state);

    return;
}

void
do_LeaveNotify (XEvent *eventp)
{
    do_EnterNotify (eventp);		/* since it has same information */
    return;
}

void
do_FocusIn (XEvent *eventp)
{
    XFocusChangeEvent *e = (XFocusChangeEvent *) eventp;
    char *mode, *detail;
    char dmode[10], ddetail[10];

    switch (e->mode) {
      case NotifyNormal:  mode = "NotifyNormal"; break;
      case NotifyGrab:  mode = "NotifyGrab"; break;
      case NotifyUngrab:  mode = "NotifyUngrab"; break;
      case NotifyWhileGrabbed:  mode = "NotifyWhileGrabbed"; break;
      default:  mode = dmode, sprintf(dmode, "%u", e->mode); break;
    }

    switch (e->detail) {
      case NotifyAncestor:  detail = "NotifyAncestor"; break;
      case NotifyVirtual:  detail = "NotifyVirtual"; break;
      case NotifyInferior:  detail = "NotifyInferior"; break;
      case NotifyNonlinear:  detail = "NotifyNonlinear"; break;
      case NotifyNonlinearVirtual:  detail = "NotifyNonlinearVirtual"; break;
      case NotifyPointer:  detail = "NotifyPointer"; break;
      case NotifyPointerRoot:  detail = "NotifyPointerRoot"; break;
      case NotifyDetailNone:  detail = "NotifyDetailNone"; break;
      default:  detail = ddetail; sprintf(ddetail, "%u", e->detail); break;
    }

    fprintf(stderr, "    mode %s, detail %s\n", mode, detail);
    return;
}

void
do_FocusOut (XEvent *eventp)
{
    do_FocusIn (eventp);		/* since it has same information */
    return;
}

void
do_KeymapNotify (XEvent *eventp)
{
    XKeymapEvent *e = (XKeymapEvent *) eventp;
    int i;

    fprintf(stderr, "    keys:  ");
    for (i = 0; i < 32; i++) {
	if (i == 16) fprintf(stderr, "\n           ");
	fprintf(stderr, "%-3u ", (unsigned int) e->key_vector[i]);
    }
    fprintf(stderr, "\n");
    return;
}

void
do_Expose (XEvent *eventp)
{
    XExposeEvent *e = (XExposeEvent *) eventp;

    fprintf(stderr, "    (%d,%d), width %d, height %d, count %d\n",
	    e->x, e->y, e->width, e->height, e->count);
    return;
}

void
do_GraphicsExpose (XEvent *eventp)
{
    XGraphicsExposeEvent *e = (XGraphicsExposeEvent *) eventp;
    char *m;
    char mdummy[10];

    switch (e->major_code) {
      case X_CopyArea:  m = "CopyArea";  break;
      case X_CopyPlane:  m = "CopyPlane";  break;
      default:  m = mdummy; sprintf(mdummy, "%d", e->major_code); break;
    }

    fprintf(stderr, "    (%d,%d), width %d, height %d, count %d,\n",
	    e->x, e->y, e->width, e->height, e->count);
    fprintf(stderr, "    major %s, minor %d\n", m, e->minor_code);
    return;
}

void
do_NoExpose (XEvent *eventp)
{
    XNoExposeEvent *e = (XNoExposeEvent *) eventp;
    char *m;
    char mdummy[10];

    switch (e->major_code) {
      case X_CopyArea:  m = "CopyArea";  break;
      case X_CopyPlane:  m = "CopyPlane";  break;
      default:  m = mdummy; sprintf(mdummy, "%d", e->major_code); break;
    }

    fprintf(stderr, "    major %s, minor %d\n", m, e->minor_code);
    return;
}

void
do_VisibilityNotify (XEvent *eventp)
{
    XVisibilityEvent *e = (XVisibilityEvent *) eventp;
    char *v;
    char vdummy[10];

    switch (e->state) {
      case VisibilityUnobscured:  v = "VisibilityUnobscured"; break;
      case VisibilityPartiallyObscured:  v = "VisibilityPartiallyObscured"; break;
      case VisibilityFullyObscured:  v = "VisibilityFullyObscured"; break;
      default:  v = vdummy; sprintf(vdummy, "%d", e->state); break;
    }

    fprintf(stderr, "    state %s\n", v);
    return;
}

void
do_CreateNotify (XEvent *eventp)
{
    XCreateWindowEvent *e = (XCreateWindowEvent *) eventp;

    fprintf(stderr, "    parent 0x%lx, window 0x%lx, (%d,%d), width %d, height %d\n",
	    e->parent, e->window, e->x, e->y, e->width, e->height);
    fprintf(stderr, "border_width %d, override %s\n",
	    e->border_width, e->override_redirect ? Yes : No);
    return;
}

void
do_DestroyNotify (XEvent *eventp)
{
    XDestroyWindowEvent *e = (XDestroyWindowEvent *) eventp;

    fprintf(stderr, "    event 0x%lx, window 0x%lx\n", e->event, e->window);
    return;
}

void
do_UnmapNotify (XEvent *eventp)
{
    XUnmapEvent *e = (XUnmapEvent *) eventp;

    fprintf(stderr, "    event 0x%lx, window 0x%lx, from_configure %s\n",
	    e->event, e->window, e->from_configure ? Yes : No);
    return;
}

void
do_MapNotify (XEvent *eventp)
{
    XMapEvent *e = (XMapEvent *) eventp;

    fprintf(stderr, "    event 0x%lx, window 0x%lx, override %s\n",
	    e->event, e->window, e->override_redirect ? Yes : No);
    return;
}

void
do_MapRequest (XEvent *eventp)
{
    XMapRequestEvent *e = (XMapRequestEvent *) eventp;

    fprintf(stderr, "    parent 0x%lx, window 0x%lx\n", e->parent, e->window);
    return;
}

void
do_ReparentNotify (XEvent *eventp)
{
    XReparentEvent *e = (XReparentEvent *) eventp;

    fprintf(stderr, "    event 0x%lx, window 0x%lx, parent 0x%lx,\n",
	    e->event, e->window, e->parent);
    fprintf(stderr, "    (%d,%d), override %s\n", e->x, e->y, 
	    e->override_redirect ? Yes : No);
    return;
}

void
do_ConfigureNotify (XEvent *eventp)
{
    XConfigureEvent *e = (XConfigureEvent *) eventp;

    fprintf(stderr, "    event 0x%lx, window 0x%lx, (%d,%d), width %d, height %d,\n",
	    e->event, e->window, e->x, e->y, e->width, e->height);
    fprintf(stderr, "    border_width %d, above 0x%lx, override %s\n",
	    e->border_width, e->above, e->override_redirect ? Yes : No);
    return;
}

void
do_ConfigureRequest (XEvent *eventp)
{
    XConfigureRequestEvent *e = (XConfigureRequestEvent *) eventp;
    char *detail;
    char ddummy[10];

    switch (e->detail) {
      case Above:  detail = "Above";  break;
      case Below:  detail = "Below";  break;
      case TopIf:  detail = "TopIf";  break;
      case BottomIf:  detail = "BottomIf"; break;
      case Opposite:  detail = "Opposite"; break;
      default:  detail = ddummy; sprintf(ddummy, "%d", e->detail); break;
    }

    fprintf(stderr, "    parent 0x%lx, window 0x%lx, (%d,%d), width %d, height %d,\n",
	    e->parent, e->window, e->x, e->y, e->width, e->height);
    fprintf(stderr, "    border_width %d, above 0x%lx, detail %s, value 0x%lx\n",
	    e->border_width, e->above, detail, e->value_mask);
    return;
}

void
do_GravityNotify (XEvent *eventp)
{
    XGravityEvent *e = (XGravityEvent *) eventp;

    fprintf(stderr, "    event 0x%lx, window 0x%lx, (%d,%d)\n",
	    e->event, e->window, e->x, e->y);
    return;
}

void
do_ResizeRequest (XEvent *eventp)
{
    XResizeRequestEvent *e = (XResizeRequestEvent *) eventp;

    fprintf(stderr, "    width %d, height %d\n", e->width, e->height);
    return;
}

void
do_CirculateNotify (XEvent *eventp)
{
    XCirculateEvent *e = (XCirculateEvent *) eventp;
    char *p;
    char pdummy[10];

    switch (e->place) {
      case PlaceOnTop:  p = "PlaceOnTop"; break;
      case PlaceOnBottom:  p = "PlaceOnBottom"; break;
      default:  p = pdummy; sprintf(pdummy, "%d", e->place); break;
    }

    fprintf(stderr, "    event 0x%lx, window 0x%lx, place %s\n",
	    e->event, e->window, p);
    return;
}

void
do_CirculateRequest (XEvent *eventp)
{
    XCirculateRequestEvent *e = (XCirculateRequestEvent *) eventp;
    char *p;
    char pdummy[10];

    switch (e->place) {
      case PlaceOnTop:  p = "PlaceOnTop"; break;
      case PlaceOnBottom:  p = "PlaceOnBottom"; break;
      default:  p = pdummy; sprintf(pdummy, "%d", e->place); break;
    }

    fprintf(stderr, "    parent 0x%lx, window 0x%lx, place %s\n",
	    e->parent, e->window, p);
    return;
}

void
do_PropertyNotify (XEvent *eventp)
{
    XPropertyEvent *e = (XPropertyEvent *) eventp;
    char *aname = XGetAtomName (eventp->xany.display, e->atom);
    char *s;
    char sdummy[10];

    switch (e->state) {
      case PropertyNewValue:  s = "PropertyNewValue"; break;
      case PropertyDelete:  s = "PropertyDelete"; break;
      default:  s = sdummy; sprintf(sdummy, "%d", e->state); break;
    }

    fprintf(stderr, "    atom 0x%lx (%s), time %lu, state %s\n",
	   e->atom, aname ? aname : Unknown, e->time,  s);

    if (aname) XFree (aname);
    return;
}

void
do_SelectionClear (XEvent *eventp)
{
    XSelectionClearEvent *e = (XSelectionClearEvent *) eventp;
    char *sname = XGetAtomName (eventp->xany.display, e->selection);

    fprintf(stderr, "    selection 0x%lx (%s), time %lu\n",
	    e->selection, sname ? sname : Unknown, e->time);

    if (sname) XFree (sname);
    return;
}

void
do_SelectionRequest (XEvent *eventp)
{
    XSelectionRequestEvent *e = (XSelectionRequestEvent *) eventp;
    char *sname = XGetAtomName (eventp->xany.display, e->selection);
    char *tname = XGetAtomName (eventp->xany.display, e->target);
    char *pname = XGetAtomName (eventp->xany.display, e->property);

    fprintf(stderr, "    owner 0x%lx, requestor 0x%lx, selection 0x%lx (%s),\n",
	    e->owner, e->requestor, e->selection, sname ? sname : Unknown);
    fprintf(stderr, "    target 0x%lx (%s), property 0x%lx (%s), time %lu\n",
	    e->target, tname ? tname : Unknown, e->property,
	    pname ? pname : Unknown, e->time);

    if (sname) XFree (sname);
    if (tname) XFree (tname);
    if (pname) XFree (pname);

    return;
}

void
do_SelectionNotify (XEvent *eventp)
{
    XSelectionEvent *e = (XSelectionEvent *) eventp;
    char *sname = XGetAtomName (eventp->xany.display, e->selection);
    char *tname = XGetAtomName (eventp->xany.display, e->target);
    char *pname = XGetAtomName (eventp->xany.display, e->property);

    fprintf(stderr, "    selection 0x%lx (%s), target 0x%lx (%s),\n",
	    e->selection, sname ? sname : Unknown, e->target,
	    tname ? tname : Unknown);
    fprintf(stderr, "    property 0x%lx (%s), time %lu\n",
	    e->property, pname ? pname : Unknown, e->time);

    if (sname) XFree (sname);
    if (tname) XFree (tname);
    if (pname) XFree (pname);

    return;
}

void
do_ColormapNotify (XEvent *eventp)
{
    XColormapEvent *e = (XColormapEvent *) eventp;
    char *s;
    char sdummy[10];

    switch (e->state) {
      case ColormapInstalled:  s = "ColormapInstalled"; break;
      case ColormapUninstalled:  s = "ColormapUninstalled"; break;
      default:  s = sdummy; sprintf(sdummy, "%d", e->state); break;
    }

    fprintf(stderr, "    colormap 0x%lx, new %s, state %s\n",
	    e->colormap, e->c_new ? Yes : No, s);
    return;
}

void
do_ClientMessage (XEvent *eventp)
{
    XClientMessageEvent *e = (XClientMessageEvent *) eventp;
    char *mname = XGetAtomName (eventp->xany.display, e->message_type);

    fprintf(stderr, "    message_type 0x%lx (%s), format %d\n",
	    e->message_type, mname ? mname : Unknown, e->format);

    if (mname) XFree (mname);
    return;
}

void
do_MappingNotify (XEvent *eventp)
{
    XMappingEvent *e = (XMappingEvent *) eventp;
    char *r;
    char rdummy[10];

    switch (e->request) {
      case MappingModifier:  r = "MappingModifier"; break;
      case MappingKeyboard:  r = "MappingKeyboard"; break;
      case MappingPointer:  r = "MappingPointer"; break;
      default:  r = rdummy; sprintf(rdummy, "%d", e->request); break;
    }

    fprintf(stderr, "    request %s, first_keycode %d, count %d\n",
	    r, e->first_keycode, e->count);
    return;
}

void
DebugEvent(
	XEvent		*event
	)
{
	if (event == NULL)
		return;

	switch (event->type) {
		case KeyPress:
			prologue (event, "KeyPress");
			do_KeyPress (event);
			break;
		case KeyRelease:
			prologue (event, "KeyRelease");
			do_KeyRelease (event);
			break;
		case ButtonPress:
			prologue (event, "ButtonPress");
			do_ButtonPress (event);
			break;
		case ButtonRelease:
			prologue (event, "ButtonRelease");
			do_ButtonRelease (event);
			break;
		case MotionNotify:
			prologue (event, "MotionNotify");
			do_MotionNotify (event);
			break;
		case EnterNotify:
			prologue (event, "EnterNotify");
			do_EnterNotify (event);
			break;
		case LeaveNotify:
			prologue (event, "LeaveNotify");
			do_LeaveNotify (event);
			break;
		case FocusIn:
			prologue (event, "FocusIn");
			do_FocusIn (event);
			break;
		case FocusOut:
			prologue (event, "FocusOut");
			do_FocusOut (event);
			break;
		case KeymapNotify:
			prologue (event, "KeymapNotify");
			do_KeymapNotify (event);
			break;
		case Expose:
			prologue (event, "Expose");
			do_Expose (event);
			break;
		case GraphicsExpose:
			prologue (event, "GraphicsExpose");
			do_GraphicsExpose (event);
			break;
		case NoExpose:
			prologue (event, "NoExpose");
			do_NoExpose (event);
			break;
		case VisibilityNotify:
			prologue (event, "VisibilityNotify");
			do_VisibilityNotify (event);
			break;
		case CreateNotify:
			prologue (event, "CreateNotify");
			do_CreateNotify (event);
			break;
		case DestroyNotify:
			prologue (event, "DestroyNotify");
			do_DestroyNotify (event);
			break;
		case UnmapNotify:
			prologue (event, "UnmapNotify");
			do_UnmapNotify (event);
			break;
		case MapNotify:
			prologue (event, "MapNotify");
			do_MapNotify (event);
			break;
		case MapRequest:
			prologue (event, "MapRequest");
			do_MapRequest (event);
			break;
		case ReparentNotify:
			prologue (event, "ReparentNotify");
			do_ReparentNotify (event);
			break;
		case ConfigureNotify:
			prologue (event, "ConfigureNotify");
			do_ConfigureNotify (event);
			break;
		case ConfigureRequest:
			prologue (event, "ConfigureRequest");
			do_ConfigureRequest (event);
			break;
		case GravityNotify:
			prologue (event, "GravityNotify");
			do_GravityNotify (event);
			break;
		case ResizeRequest:
			prologue (event, "ResizeRequest");
			do_ResizeRequest (event);
			break;
		case CirculateNotify:
			prologue (event, "CirculateNotify");
			do_CirculateNotify (event);
			break;
		case CirculateRequest:
			prologue (event, "CirculateRequest");
			do_CirculateRequest (event);
			break;
		case PropertyNotify:
			prologue (event, "PropertyNotify");
			do_PropertyNotify (event);
			break;
		case SelectionClear:
			prologue (event, "SelectionClear");
			do_SelectionClear (event);
			break;
		case SelectionRequest:
			prologue (event, "SelectionRequest");
			do_SelectionRequest (event);
			break;
		case SelectionNotify:
			prologue (event, "SelectionNotify");
			do_SelectionNotify (event);
			break;
		case ColormapNotify:
			prologue (event, "ColormapNotify");
			do_ColormapNotify (event);
			break;
		case ClientMessage:
			prologue (event, "ClientMessage");
			do_ClientMessage (event);
			break;
		case MappingNotify:
			prologue (event, "MappingNotify");
			do_MappingNotify (event);
			break;
		default:
			fprintf(stderr, "Unknown event type %d\n", event->type);
			break;
	}
}


#endif /* DEBUG */
