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
 * $Id: quarks.C,v 1.10 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Resource quark initialization
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/

#ifndef lint
static char RCSinfo[] =
"$Id: quarks.C,v 1.10 1993/08/27 16:58:15 toml Exp $";
#endif /* lint */

#include <X11/X.h>
#ifdef ultrix
// DEC's distributed X11/Xlib.h is badly broken, so use the OI's fixed copy
#include <OI/Xlib.H>
#else
#include <X11/Xlib.h>
#endif
#include <X11/Xresource.h>
#include "quarks.H"

#define _QS(o, s1, s2)	o.name = XrmStringToQuark(s1); o._class = XrmStringToQuark(s2)

wmQuark::wmQuark()
{
	_QS(button,		"button",		"Button");
	_QS(panel,		"panel",		"Panel");
	_QS(text,		"text",			"Text");
	_QS(menu,		"menu",			"Menu");
	_QS(border,		"border",		"Border");
	_QS(foreground,		"foreground",		"Foreground");
	_QS(background,		"background",		"Background");
	_QS(focusBorder,	"focusBorder",		"FocusBorder");
	_QS(focusForeground,	"focusForeground",	"FocusForeground");
	_QS(focusBackground,	"focusBackground",	"FocusBackground");
	_QS(focusReverse,	"focusReverse",		"FocusReverse");
	_QS(backgroundPixmap,	"backgroundPixmap",	"BackgroundPixmap");
	_QS(shape,		"shape",		"Shape");
	_QS(shapeMask,		"shapeMask",		"ShapeMask");
	_QS(borderWidth,	"borderWidth",		"BorderWidth");
	_QS(bevelWidth,		"bevelWidth",		"BevelWidth");
	_QS(font,		"font",			"Font");
	_QS(cursor,		"cursor",		"Cursor");
	_QS(cursorForeground,	"cursorForeground",	"CursorForeground");
	_QS(cursorBackground,	"cursorBackground",	"CursorBackground");
	_QS(gravity,		"gravity",		"Gravity");
	_QS(state,		"state",		"State");
	_QS(decoration,		"decoration",		"Decoration");
	_QS(bindings,		"bindings",		"Bindings");
	_QS(macro,		"macro",		"Macro");
	_QS(olmenu,		"olmenu",		"Olmenu");
	_QS(full,		"full",			"Full");
	_QS(limited,		"limited",		"Limited");
	_QS(none,		"none",			"None");
	_QS(pinIn,		"pinIn",		"PinIn");
	_QS(pinOut,		"pinOut",		"PinOut");
	_QS(sticky,		"sticky",		"Sticky");
	_QS(shaped,		"shaped",		"Shaped");
	_QS(_virtual,		"virtual",		"Virtual");
	_QS(transient,		"transient",		"Transient");
	_QS(icon,		"icon",			"Icon");
	_QS(iconPanel,		"iconPanel",		"IconPanel");
	_QS(rootIcon,		"rootIcon",		"RootIcon");
	_QS(iconifyByUnmapping,	"iconifyByUnmapping",	"IconifyByUnmapping");
	_QS(pack,		"pack",			"Pack");
	_QS(fit,		"fit",			"Fit");
	_QS(squeeze,		"squeeze",		"Squeeze");
	_QS(showGrid,		"showGrid",		"ShowGrid");
	_QS(hideWhenEmpty,	"hideWhenEmpty",	"HideWhenEmpty");
	_QS(defaultIconImage,	"defaultIconImage",	"DefaultIconImage");
	_QS(scrollBars,		"scrollBars",		"ScrollBars");
	_QS(virtualDesktop,	"virtualDesktop",	"VirtualDesktop");
	_QS(ignoreGroupHints,	"ignoreGroupHints",	"IgnoreGroupHints");
	_QS(ignorePPosition,	"ignorePPosition",	"IgnorePPosition");
	_QS(ignorePPositionOrigin,	"ignorePPositionOrigin",	"IgnorePPositionOrigin");
	_QS(constrain,		"constrain",		"Constrain");
	_QS(insideBorderWidth,	"insideBorderWidth",	"InsideBorderWidth");
	_QS(highlightFrame,	"highlightFrame",	"HighlightFrame");
	_QS(resizeCorners,	"resizeCorners",	"ResizeCorners");
	_QS(resizeBars,		"resizeBars",		"ResizeBars");
	_QS(resizeWidth,	"resizeWidth",		"ResizeWidth");
	_QS(grayIcon,		"grayIcon",		"GrayIcon");
	_QS(noIcon,		"noIcon",		"NoIcon");
	_QS(resizeLength,	"resizeLength",		"ResizeLength");
	_QS(resizeBackground,	"resizeBackground",	"ResizeBackground");
	_QS(randomPlacement,	"randomPlacement",	"RandomPlacement");
	_QS(maxIconLabel,	"maxIconLabel",		"MaxIconLabel");
	_QS(xorValue,		"xorValue",		"XorValue");
	_QS(resourceFile,	"resourceFile",		"ResourceFile");
	_QS(help,		"help",			"Help");
	_QS(nameObjects,	"nameObjects",		"NameObjects");
	_QS(remoteExecution,	"remoteExecution",	"RemoteExecution");
	_QS(focusModel,		"focusModel",		"FocusModel");
	_QS(model,		"model",		"Model");
	_QS(zap,		"zap",			"Zap");
	_QS(resizeFont,		"resizeFont",		"ResizeFont");
	_QS(opaque,		"opaque",		"Opaque");
	_QS(grid,		"grid",			"Grid");
	_QS(scale,		"scale",		"Scale");
	_QS(geometry,		"geometry",		"Geometry");
	_QS(iconGeometry,	"iconGeometry",		"IconGeometry");
	_QS(iconic,		"iconic",		"Iconic");
	_QS(initialState,	"initialState",		"InitialState");
	_QS(moveDelta,		"moveDelta",		"MoveDelta");
	_QS(resizeGrid,		"resizeGrid",		"ResizeGrid");
	_QS(showOutline,	"showOutline",		"ShowOutline");
	_QS(restartPreviousState,	"restartPreviousState",	"RestartPreviousState");
	_QS(rootPanels,		"rootPanels",		"RootPanels");
	_QS(rootIcons,		"rootIcons",		"RootIcons");
	_QS(configuration,	"configuration",	"Configuration");
	_QS(configPath,		"configPath",		"ConfigPath");
	_QS(resize,		"resize",		"Resize");
	_QS(panner,		"panner",		"Panner");
	_QS(pad,		"pad",			"Pad");
	_QS(objectPad,		"objectPad",		"ObjectPad");
	_QS(iconGravity,	"iconGravity",		"IconGravity");
	_QS(showNames,		"showNames",		"ShowNames");
	_QS(iconRegions,	"iconRegions",		"IconRegions");
	_QS(iconRegion,		"iconRegion",		"IconRegion");
	_QS(reshuffle,		"reshuffle",		"Reshuffle");
	_QS(overlay,		"overlay",		"Overlay");
	_QS(followClient,	"followClient",		"followClient");
	_QS(packImmediate,	"packImmediate",	"packImmediate");

	clientQ		= XrmStringToQuark("client");
	nameQ		= XrmStringToQuark("name");
	iconsQ		= XrmStringToQuark("icons");
	iconNameQ	= XrmStringToQuark("iconName");
	iconImageQ	= XrmStringToQuark("iconImage");
	sizeQ		= XrmStringToQuark("size");
}

wmQuark	*wmQuarks;
