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
 * $Id: icons.H,v 9.5 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Icon definitions
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/


#ifndef ICONS_H
#define ICONS_H

#include "wmdata.H"

struct wmIconPanelSlot
{
    struct wmData *wp;			// pointer to window data
    char used;				// available?
    int x, y;				// upper left corner
};

struct wmIconPanel
{
    wmObject *op;			// outside parent object
    wmScreen *scr;			// pointer to the screen structure
    OI_scroll_box *bp;			// the OI object
    OI_d_tech *ob;			// the object
    OI_scroll_bar *rsb;			// right scrollbar
    OI_scroll_bar *bsb;			// bottom scrollbar
    struct wmData *wp;			// pointer to the window data
    int slot_width;
    int slot_height;
    int rows;				// number of rows in the object
    int columns;			// number of columns in the object
    int view_rows;			// number of rows to view
    int view_columns;			// number of columns to view
    int maxColumnUsed;			// the max column number used
    int count;				// how many slots are filled
    int pad;				// pixels between objects
    int base_width;			// for size hints
    int base_height;			// for size hints
    wmIconPanelSlot **row;
    char pack;				// should we keep it packed?
    char fit;				// should we fit on resize ?
    char squeeze;			// squeeze the panel when no icons
    char showGrid;			// should we show the grid ?
    char scrollBars;			// are there scrollbars?
    char hide;				// hide the thing when it is empty

    wmIconPanel();
};

#ifndef WithdrawnState
#define WithdrawnState 0
#endif

extern const int wmDefaultSlotSize;
extern void wmInitIconPanel(wmObject *);
extern int  wmIconify(wmData *, int, int);
extern void wmMakeIcon(wmData *, int, int, Bool =True);
extern void wmPlaceIcon(wmData *, int, int);
extern int  wmDeiconify(wmData *);
extern void wmRemoveIcon(wmData *);
extern void wmGrayIcon(wmData *);
extern int  wmNewIconPanelSizes(wmIconPanel *);
extern void wmLayoutIconPanel(wmIconPanel *);
extern void wmSizeIconPanel(wmIconPanel *);
extern void wmResizeIconPanel(struct wmData *);
extern void wmRepaintIconGrid(XExposeEvent *, struct wmIconPanel *);
extern void wmExposeIcon( XExposeEvent *, wmData *);
extern void wmFitIconPanel(struct wmIconPanel *);
extern void wmPackIconPanel(struct wmIconPanel *);
extern void wmInsertIcon(wmData *, wmIconPanel *);
extern void wmMakeIconImage(wmData *);
extern void wmSetWM_STATE(wmData *, int);
extern Bool wmGetWM_STATE(wmData *);
extern void wmSetSWM_HINTS(wmData *);
extern void wmDisplayIconName(wmData *);

extern char wmMakingIcon;

#endif /* ICONS_H */
