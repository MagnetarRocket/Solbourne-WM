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
 * $Id: screen.H,v 9.13 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Screen structure definition
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/


#ifndef SCREEN_H
#define SCREEN_H

#include "list.H"
#include <OI/Xutil.H>

class wmData;
	
struct wmScreen
{
    int screen;				// the screen number
    OI_connection *conp;		// connection to the server
    short rm_stack;			// resource manager default stack pointer
    Window root;			// the root window
    Window frozen;			// window used to "freeze" the desktop
    int width;				// width of the root window
    int height;				// height of the root window
    int depth;				// depth of the display
    Colormap cmap;
    Window colormapWindow;		// window that has cmap installed
    wmData *colormapFocus;		// client that has HARD colormap focus
    PIXEL black;
    PIXEL white;
    PIXEL pannerGridFg;			// panner grid foreground pixel
    char res[40];			// resource string
    Pixmap gray;			// gray pixmap
    Pixmap gray3;			// gray pixmap
    Pixmap rootWeave;			// the root weave pattern

    GC gc;				// general purpose GC
    GC outlineGC;			// used for moves and resizes
    GC cornerGC;			// used to draw resize corners
    GC grayGC;				// used to draw slot lines on icon pan
    GC gray3GC;				// used to gray out icons

    XContext buttons;			// store button glyph structures
    XContext bitmaps;			// store bitmap path names
    XContext colorCache;		// cache for colors
    XContext badColors;			// bad color specifications

    Cursor resizeCursor;		// used during resize
    Cursor moveCursor;			// used during move
    Cursor waitCursor;			// used while busy
    Cursor chooseCursor;		// to choose a window
    Cursor sweepCursor;			// prior to actual sweep

    wmList objectList;			// the list of objects
    wmList windowList;			// the list of reparented windows
    wmList bitmapPathList;		// the paths to search for bitmaps
    wmList configPathList;		// the paths to search for configuration files
    wmList iconPanelList;		// icon panels
    wmList startList;			// startup list
    wmList rootObjectsList;		// list of root panels and root icons
    wmList iconRegionList;		// list of icon regions

    char *firstConfig;			// the first configuration string
    OI_static_text *nameOI;		// display window name
    OI_static_text *sizeOI;		// display window size

    OI_dialog_box *infoBox;		// information dialog box

    OI_scroll_box *vdt;			// virtual desktop
    OI_d_tech *vroot;			// virutal root
    OI_panner *pan;			// panner for vdt
    int vwidth;				// virtual width
    int vheight;			// virtual height
    int vscale;				// virtual desktop scale
    short scrollBars;			// are they on the display?
    short pannerNames;			// should we show names in the panner?
    GC pannerGC;			// GC to paint panner window names

    int savePointerX;
    int savePointerY;
    int saveVrootX;
    int saveVrootY;

    int randStartingX;
    int randStartingY;
    int randX;
    int randY;
    int randIncX;
    int randIncY;

    short moveDelta;			// move delta for move, resize
    char showGrid;			// show resize grid
    char showGridSave;			// save value of show resize grid

    char zap;				// "zap" the icons
    char resizeGrid;			// display a grid during resize/move
    char restartPreviousState;		// use WM_STATE when starting
    char pannerGrid;			// paint a grid on the panner 
    char help;				// is help on?
};

#endif /* SCREEN_H */
