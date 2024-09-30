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
 * $Id: util.H,v 9.6 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Utility definitions
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/


#ifndef UTIL_H
#define UTIL_H

#include "object.H"

class wmColormap
{
    XStandardColormap	std;
    XContext		cells_context;
    int			std_cmap_flag;
public:
    int			is_std_cmap()	{ return(std_cmap_flag); }
    XContext		context()		{ return(cells_context); }
    Colormap		colormap()		{ return(std.colormap); }
    unsigned long	red_max()		{ return(std.red_max); }
    unsigned long	red_mult()		{ return(std.red_mult); }
    unsigned long	green_max()		{ return(std.green_max); }
    unsigned long	green_mult()	{ return(std.green_mult); }
    unsigned long	blue_max()		{ return(std.blue_max); }
    unsigned long	blue_mult()		{ return(std.blue_mult); }
    unsigned long	base_pixel()	{ return(std.base_pixel); }
    wmColormap(Colormap);
};

#ifdef OLD
extern char *wmGetResource(char *, char * = NULL, char * = NULL, char * = NULL);
extern void wmNewResource(char *, int = 0);
extern void wmAddResource(char *, int = 0);
extern void wmRmResource(char *, int = 0);
extern char *wmGetDefaultResource(char * = NULL);
#endif /* OLD */
extern void wmGetStandardResources(wmObject *);
extern void wmGetOIResources(OI_d_tech *, int = False);
extern char *wmUniqueName();
extern void wmDeleteOI(OI_d_tech *, int);
extern void wmMoveOutline(Window=0, int=0, XRectangle * = NULL);
extern void wmFindScreen(struct wmData *, Window);
extern void wmDisplayName(struct wmData *);
extern int  wmNewWindowName(OI_entry_field *, void *);
extern void wmExpandString(char *);
extern char * wmExpandFilename(char *);
extern int wmGetColor(char *, PIXEL *, XColor * = NULL);
extern void wmGrabServer();
extern void wmUngrabServer();
extern void wmBusy(struct wmData *);
extern void wmBeeper(wmData *, XEvent *);
extern wmObjectState wmObjState;
extern void wmDumpObject(OI_d_tech *);
extern void wmPaintVirtual(XExposeEvent *, wmData *);

#endif /* UTIL_H */
