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
 * $Id: atoms.H,v 9.5 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Atoms definitions
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/


#ifndef ATOMS_H
#define ATOMS_H

// ICCCM atoms
extern Atom WM_STATE;
extern Atom WM_CHANGE_STATE;
extern Atom WM_COLORMAP_WINDOWS;
extern Atom WM_DELETE_WINDOW;
extern Atom WM_SHUTDOWN;
extern Atom WM_SAVE_YOURSELF;
extern Atom WM_PROTOCOLS;
extern Atom WM_TAKE_FOCUS;
extern Atom WM_COMMAND;

// private swm atoms
extern Atom __SWM_COMMAND;
extern Atom __SWM_HINTS;
extern Atom __SWM_ROOT;
extern Atom __SWM_VROOT;
extern Atom __SWM_START;

// Open Look atoms 
extern Atom _OL_WIN_ATTR;
extern Atom _OL_DECOR_ADD;
extern Atom _OL_DECOR_DEL;
extern Atom _OL_DECOR_CLOSE;
extern Atom _OL_DECOR_FOOTER;
extern Atom _OL_DECOR_RESIZE;
extern Atom _OL_DECOR_HEADER;
extern Atom _OL_DECOR_OK;
extern Atom _OL_DECOR_PIN;
extern Atom _OL_SCALE_SMALL;
extern Atom _OL_SCALE_MEDIUM;
extern Atom _OL_SCALE_LARGE;
extern Atom _OL_SCALE_XLARGE;
extern Atom _OL_PIN_STATE;
extern Atom _OL_WIN_BUSY;
extern Atom _OL_WINMSG_STATE;
extern Atom _OL_WINMSG_ERROR;
extern Atom _OL_WT_BASE;
extern Atom _OL_WT_CMD;
extern Atom _OL_WT_PROP;
extern Atom _OL_WT_HELP;
extern Atom _OL_WT_NOTICE;
extern Atom _OL_WT_OTHER;
extern Atom _OL_MENU_FULL;
extern Atom _OL_MENU_LIMITED;
extern Atom _OL_NONE;
extern Atom _OL_PIN_IN;
extern Atom _OL_PIN_OUT;
extern Atom _OL_WIN_DISMISS;
extern Atom _OL_BORDER_SIZES;
extern Atom _OL_DFLT_BTN;

// motif atoms
extern Atom _MOTIF_WM_HINTS;
extern Atom _MOTIF_WM_MENU;
extern Atom _MOTIF_WM_MESSAGES;
extern Atom _MOTIF_WM_INFO;

// XView atoms
extern Atom XA_XV_DO_DRAG_LOAD;

// Help Manager Atoms
extern Atom HM_STATE;

extern void wmInitAtoms();
extern void wmSet__SWM_HINTS(wmData *);
extern void wmSet__SWM_ROOT(wmData *);
extern void wmGetOpenLookAtoms(wmData *);
extern void wmGetMotifAtoms(wmData *);
extern void wmSet_OL_PIN_STATE(wmData *, int);
extern void wmGet__SWM_START();
extern void wmSet__SWM_START(wmData *);
extern void wmPutVersion();
extern void wmRemoveVersion();
extern char *wmGetWM_COMMAND(wmData *);

#endif /* ATOMS_H */
