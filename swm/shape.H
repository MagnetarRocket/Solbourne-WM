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
/************************************************************
Copyright 1989 by The Massachusetts Institute of Technology

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
no- tice appear in all copies and that both that copyright
no- tice and this permission notice appear in supporting
docu- mentation, and that the name of MIT not be used in
advertising or publicity pertaining to distribution of the
software without specific prior written permission.
M.I.T. makes no representation about the suitability of
this software for any purpose. It is provided "as is"
without any express or implied warranty.

MIT DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

/* $XConsortium: shape.h,v 1.12 89/10/08 19:22:37 jim Exp $ */

#ifndef _SHAPE_H_
#define _SHAPE_H_

#ifndef NeedFunctionPrototypes
#if defined(FUNCPROTO) || defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus)
#define NeedFunctionPrototypes 1
#else
#define NeedFunctionPrototypes 0
#endif /* __STDC__ */
#endif /* NeedFunctionPrototypes */

#ifndef NeedWidePrototypes
#if defined(NARROWPROTO)
#define NeedWidePrototypes 0
#else
#define NeedWidePrototypes 1            /* default to make interropt. easier */
#endif
#endif

#ifdef __cplusplus                      /* do not leave open across includes */
extern "C" {                                    /* for C++ V2.0 */
#endif

#define X_ShapeQueryVersion		0
#define X_ShapeRectangles		1
#define X_ShapeMask			2
#define X_ShapeCombine			3
#define X_ShapeOffset			4
#define X_ShapeQueryExtents		5
#define X_ShapeSelectInput		6
#define X_ShapeInputSelected		7
#define X_ShapeGetRectangles		8

#define ShapeSet			0
#define ShapeUnion			1
#define ShapeIntersect			2
#define ShapeSubtract			3
#define ShapeInvert			4

#define ShapeBounding			0
#define ShapeClip			1

#define ShapeNotifyMask			(1L << 0)
#define ShapeNotify			0

#define ShapeNumberEvents		(ShapeNotify + 1)

#ifndef _SHAPE_SERVER_
typedef struct {
    int	type;		    /* of event */
    unsigned long serial;   /* # of last request processed by server */
    Bool send_event;	    /* true if this came frome a SendEvent request */
    Display *display;	    /* Display the event was read from */
    Window window;	    /* window of event */
    int kind;		    /* ShapeBounding or ShapeClip */
    int x, y;		    /* extents of new region */
    unsigned width, height;
    Time time;		    /* server timestamp when region changed */
    Bool shaped;	    /* true if the region exists */
} XShapeEvent;

extern Bool XShapeQueryExtension (
#if NeedFunctionPrototypes
    Display *		/* display */,
    int *		/* event_base */,
    int *		/* error_base */
#endif
);

extern Status XShapeQueryVersion (
#if NeedFunctionPrototypes
    Display *		/* display */,
    int *		/* major_version */,
    int *		/* minor_version */
#endif
);

extern void XShapeCombineRegion (
#if NeedFunctionPrototypes
    Display *		/* display */,
    Window		/* dest */,
    int			/* dest_kind */,
    int			/* x_off */,
    int			/* y_off */,
    Region *		/* region */,
    int			/* op */
#endif
);

extern void XShapeCombineRectangles (
#if NeedFunctionPrototypes
    Display *		/* display */,
    Window		/* dest */,
    int			/* dest_kind */,
    int			/* x_off */,
    int			/* y_off */,
    XRectangle *	/* rectangles */,
    int			/* n_rects */,
    int			/* op */,
    int			/* ordering */
#endif
);

extern void XShapeCombineMask (
#if NeedFunctionPrototypes
    Display *		/* display */,
    Window		/* dest */,
    int			/* dest_kind */,
    int			/* x_off */,
    int			/* y_off */,
    Pixmap		/* src */,
    int			/* op */
#endif
);

extern void XShapeCombineShape (
#if NeedFunctionPrototypes
    Display *		/* display */,
    Window		/* dest */,
    int			/* dest_kind */,
    int			/* x_off */,
    int			/* y_off */,
    Window		/* src */,
    int			/* src_kind */,
    int			/* op */
#endif
);

extern void XShapeOffsetShape (
#if NeedFunctionPrototypes
    Display *		/* display */,
    Window		/* dest */,
    int			/* dest_kind */,
    int			/* x_off */,
    int			/* y_off */
#endif
);

extern Status XShapeQueryExtents (
#if NeedFunctionPrototypes
    Display *		/* display */,
    Window		/* window */,
    Bool *		/* bounding_shaped */,
    int *		/* x_bounding */,
    int *		/* y_bounding */,
    unsigned int *	/* w_bounding */,
    unsigned int *	/* h_bounding */,
    Bool *		/* clip_shaped */,
    int *		/* x_clip */,
    int *		/* y_clip */,
    unsigned int *	/* w_clip */,
    unsigned int *	/* h_clip */
#endif
);

extern void XShapeSelectInput (
#if NeedFunctionPrototypes
    Display *		/* display */,
    Window		/* window */,
    unsigned long	/* mask */
#endif
);

extern unsigned long XShapeInputSelected (
#if NeedFunctionPrototypes
    Display *		/* display */,
    Window		/* window */
#endif
);

extern XRectangle *XShapeGetRectangles (
#if NeedFunctionPrototypes
    Display *		/* display */,
    Window		/* window */,
    int			/* kind */,
    int *		/* count */,
    int *		/* ordering */
#endif
);

#endif /* _SHAPE_SERVER_ */

#ifdef __cplusplus
}                                               /* for C++ V2.0 */
#endif

#endif /* _SHAPE_H_ */
