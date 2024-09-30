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
 * $Id: events.H,v 9.4 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Event handling definitions
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/


#ifndef EVENTS_H
#define EVENTS_H

#include "resize.H"

extern void wmInitEvents();

extern void wmMapRequest(XMapRequestEvent *, struct wmData *);
extern void wmMapNotify(XMapEvent *, char *);
extern void wmVrootReparentNotify(XReparentEvent *, char *);
extern void wmVrootCreateNotify(XCreateWindowEvent *, char *);
extern void wmDestroyNotify(XDestroyWindowEvent *, struct wmData *);
extern void wmUnmapNotify( XUnmapEvent *, struct wmData *);
extern void wmConfigureRequest(XConfigureRequestEvent *, struct wmData *);
extern void wmPropertyNotify( XPropertyEvent *, struct wmData *);
extern void wmClientMessage( XClientMessageEvent *, struct wmData *);
extern void wmEnterNotify( XEnterWindowEvent *, struct wmData *);
extern void wmLeaveNotify( XLeaveWindowEvent *, struct wmData *);
extern void wmColormapNotify( XColormapEvent *, struct wmData *);
extern void wmIconVisibilityNotify( XVisibilityEvent *, wmData *);
extern void wmFrameVisibilityNotify( XVisibilityEvent *, wmData *);
extern void wmVirtualEnterNotify( XEnterWindowEvent *, struct wmData *);
extern void wmVirtualLeaveNotify( XLeaveWindowEvent *, struct wmData *);
#ifdef SHAPE
extern void wmShapeNotify(XShapeEvent *, struct wmData *);
#endif /* SHAPE */

extern void wmExposeFrame( XExposeEvent *, struct wmData *);
extern void wmFlushExpose(Window);
extern void wmResetColormaps();
extern void wmDrawCorner(wmData *, wmResizeCorner, int, int, int, int, int);
extern void wmClearFocus( XLeaveWindowEvent *, wmData *);
extern void wmSetFocus( XEnterWindowEvent *, wmData *);
extern void wmDrawVerticalBar(wmData *, int, int, int, int);
extern void wmDrawHorizontalBar(wmData *, int, int, int, int);

extern wmData *wmFocusWp;		// current focus window

#endif /* EVENTS_H */
