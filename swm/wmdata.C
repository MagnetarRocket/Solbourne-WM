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
 * $Id: wmdata.C,v 9.10 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Window data initialization
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/

#ifndef lint
static char RCSinfo[] =
"$Id: wmdata.C,v 9.10 1993/08/27 16:58:15 toml Exp $";
#endif /* lint */

#include "swm.H"
#include "wmdata.H"
#include "main.H"
#include "icons.H"

wmData *wmClient = NULL;

wmClientData::wmClientData()
{
    w = None;
    transient_for = None;
    lastFocus = NULL;
    tfwp = NULL;
    colormapWindows = NULL;
    colormapCount = 0;
    protocols = NULL;
    protocolCount = 0;
    protocolBits = 0;
    wmhints = 0;
    name = NULL;
    icon_name = NULL;
    group = 0;
    regroup = 0;
    op = NULL;
    oi_frame = NULL;
    oi_client = NULL;
    oi_name = NULL;
    oi_size = NULL;
    insideBorderWidth = 0;
    insideBorderWidthSaved = 0;
    resizeWidth = 0;
    resizeLength = 0;
    pad = 0;
    state = WithdrawnState;
    save_x = 0;
    save_y = 0;
    save_width = 0;
    save_height = 0;
    focusType = wmNoInput;

    // OpenLook attribute stuff
    olDecoration = 0;
    olDfltBtn = NULL;
    root_x = 0;
    root_y = 0;

    myhints.icon_window = None;
    myhints.icon_x = 0;
    myhints.icon_y = 0;
    myhints.gravity_order = 0;
    myhints.flags = 0;

    focusList.init();
    transientList.init();
}

wmIconData::wmIconData()
{
    virtualibox = NULL;
    iconObject = NULL;
    iconPanel = NULL;
    iconRegion = NULL;
    iop = NULL;
    irp = NULL;
    oi_icon = NULL;
    oi_iconImage = NULL;
    oi_iconName = NULL;
    iconImage = None;
    iconLabelLength = 8;
    ip = 0;
    ipop = NULL;
    ipsp = NULL;
    icon_vis = 0;
}

wmData::wmData()
{
    ctl = 0;
    cp  = NULL;
    icp = NULL;
    scr = wmScr;
    oi_root = NULL;
    Xratio = 1;
    Yratio = 1;
    virtualbox = NULL;
    myIconPanel = 0;
    lastStatus = True;
}

wmData::~wmData()
{
	if (cp)
		delete cp;
	if (icp)
		delete icp;
}
