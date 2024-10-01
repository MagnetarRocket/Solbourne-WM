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
 * $Id: init.H,v 9.5 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Initialization defintions
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/


#ifndef INIT_H
#define INIT_H

enum wmFocusMod
{
    wmFocusModelNormal		= 0,
    wmFocusModelPassive		= 1,
    wmFocusModelClickToType	= 2
};

extern XContext wmContext;
extern XContext wmMine;
extern XContext wmIconPanelContext;
extern XContext wmFrameContext;
extern XContext wmIconContext;
extern XContext wmInternalContext;
extern XContext wmRootPanelsContext;
extern XContext wmStateContext;
extern XContext wmBindingsContext;
extern XContext wmButtonsContext;
extern int wmNumScreens;
extern int wmInitDone;
extern int *wmTmpPtr;
extern char *wmRemoteExecution;
extern wmFocusMod wmFocusModel;

extern void wmInitialize(int, char **);
extern void wmReConfig();
extern FILE *wmResourceDebug;
extern Window wmFocusBeeper;
extern int wmNameObjects;
extern int wmMakingRootPanels;

#define SAVE_INTERNAL(win) (XSaveContext(DPY, win, wmInternalContext, (caddr_t)0))
#define IS_INTERNAL(win) (XFindContext(DPY, win, wmInternalContext, (caddr_t*)&wmTmpPtr) != XCNOENT)

extern int wmHasShape;
extern int wmShapeEventBase;
extern int wmShapeErrorBase;

#endif /* INIT_H */
