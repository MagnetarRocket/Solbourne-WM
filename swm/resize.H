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
 * $Id: resize.H,v 9.4 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Window resize definitions
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/


#ifndef RESIZE_H
#define RESIZE_H


#define RESIZE_TOP 1
#define RESIZE_BOTTOM 2
#define RESIZE_LEFT 4
#define RESIZE_RIGHT 8

#define RESIZE_TOP_LEFT (RESIZE_TOP|RESIZE_LEFT)
#define RESIZE_TOP_RIGHT (RESIZE_TOP|RESIZE_RIGHT)
#define RESIZE_BOTTOM_LEFT (RESIZE_BOTTOM|RESIZE_LEFT)
#define RESIZE_BOTTOM_RIGHT (RESIZE_BOTTOM|RESIZE_RIGHT)

enum wmResizeCorner
{
    wmResizeCornerTopLeft	= 0,
    wmResizeCornerTopRight	= 1,
    wmResizeCornerBottomLeft	= 2,
    wmResizeCornerBottomRight	= 3
};

#define B_MASK (Button1Mask|Button2Mask|Button3Mask|Button4Mask|Button5Mask)

extern int wmAutoResize;
extern int wmResizeFlags;
extern XButtonEvent wmResizeEvent;
extern int wmStartResize( XButtonEvent *ev, wmData *);
extern void wmHandleResizeBar( XButtonEvent *ev, wmData *);
extern void wmResizeTL( XButtonEvent *ev, wmData *);
extern void wmResizeTR( XButtonEvent *ev, wmData *);
extern void wmResizeBL( XButtonEvent *ev, wmData *);
extern void wmResizeBR( XButtonEvent *ev, wmData *);
extern void wmResizeT( XButtonEvent *ev, wmData *);
extern void wmResizeB( XButtonEvent *ev, wmData *);
extern void wmResizeL( XButtonEvent *ev, wmData *);
extern void wmResizeR( XButtonEvent *ev, wmData *);
extern void wmResizeClient(wmData *, int, int, int);
extern void wmGetPointer(Window, int *, int *, int *, int *, int *, int);
extern void wmUnitSize(wmData *, int, int, int *, int *);
extern void wmUpdateSize(wmData *);
extern int wmZoom(int, wmData *, int *, int *);
extern void wmSave(wmData *);
extern void wmRestore(wmData *);
extern void wmSweep(XButtonEvent *);
extern void wmUnSweep();
extern OI_d_tech **wmSwept;	// array of swept objects
extern int wmSweptCount;

#endif /* RESIZE_H */
