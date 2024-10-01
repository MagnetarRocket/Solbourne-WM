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
 * $Id: quarks.H,v 1.12 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Quark data structures
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/

#ifndef QUARKS_H
#define QUARKS_H

/* this macro makes it much easier to define the member functions 
 * for the wmQuark class, I know this isn't real portable, hopefully
 * one of these will work for you
 */

#if defined(_AIX) || defined(ultrix) || defined(hpux) || defined(SVR4)
#define _Q(x)	XrmQuark	x##Name()	{ return(x.name); } \
		XrmQuark	x##Class()	{ return(x._class); }
#else
#define _Q(x)	XrmQuark	x/**/Name()	{ return(x.name); } \
		XrmQuark	x/**/Class()	{ return(x._class); }
#endif

#ifndef _Q
#define _Q(x)	XrmQuark	x\
Name()	{ return(x.name); } \
		XrmQuark	x\
Class()	{ return(x._class); }
#endif /* _Q */

typedef struct _quark {
	XrmQuark	name;
	XrmQuark	_class;
} quark;

class wmQuark {
protected:
	quark		button;
	quark		panel;
	quark		text;
	quark		menu;
	quark		border;
	quark		foreground;
	quark		background;
	quark		focusBorder;
	quark		focusForeground;
	quark		focusBackground;
	quark		focusReverse;
	quark		backgroundPixmap;
	quark		shape;
	quark		shapeMask;
	quark		borderWidth;
	quark		bevelWidth;
	quark		font;
	quark		cursor;
	quark		cursorForeground;
	quark		cursorBackground;
	quark		gravity;
	quark		state;
	quark		decoration;
	quark		bindings;
	quark		macro;
	quark		olmenu;
	quark		full;
	quark		limited;
	quark		none;
	quark		pinIn;
	quark		pinOut;
	quark		sticky;
	quark		shaped;
	quark		_virtual;
	quark		transient;
	quark		icon;
	quark		iconPanel;
	quark		rootIcon;
	quark		iconifyByUnmapping;
	quark		pack;
	quark		fit;
	quark		squeeze;
	quark		showGrid;
	quark		hideWhenEmpty;
	quark		defaultIconImage;
	quark		scrollBars;
	quark		virtualDesktop;
	quark		ignoreGroupHints;
	quark		ignorePPosition;
	quark		ignorePPositionOrigin;
	quark		constrain;
	quark		insideBorderWidth;
	quark		highlightFrame;
	quark		resizeCorners;
	quark		resizeBars;
	quark		resizeWidth;
	quark		grayIcon;
	quark		noIcon;
	quark		resizeLength;
	quark		resizeBackground;
	quark		randomPlacement;
	quark		maxIconLabel;
	quark		xorValue;
	quark		resourceFile;
	quark		help;
	quark		nameObjects;
	quark		remoteExecution;
	quark		focusModel;
	quark		model;
	quark		zap;
	quark		resizeFont;
	quark		opaque;
	quark		grid;
	quark		scale;
	quark		geometry;
	quark		iconGeometry;
	quark		iconic;
	quark		initialState;
	quark		moveDelta;
	quark		resizeGrid;
	quark		showOutline;
	quark		restartPreviousState;
	quark		rootPanels;
	quark		rootIcons;
	quark		configuration;
	quark		configPath;
	quark		resize;
	quark		panner;
	quark		pad;
	quark		objectPad;
	quark		iconGravity;
	quark		showNames;
	quark		iconRegions;
	quark		iconRegion;
	quark		reshuffle;
	quark		overlay;
	quark		followClient;
	quark		packImmediate;
	XrmQuark	clientQ;
	XrmQuark	nameQ;
	XrmQuark	iconsQ;
	XrmQuark	iconNameQ;
	XrmQuark	iconImageQ;
	XrmQuark	sizeQ;
public:
	wmQuark();
	_Q(button)
	_Q(panel)
	_Q(text)
	_Q(menu)
	_Q(border)
	_Q(foreground)
	_Q(background)
	_Q(focusBorder)
	_Q(focusForeground)
	_Q(focusBackground)
	_Q(focusReverse)
	_Q(backgroundPixmap)
	_Q(shape)
	_Q(shapeMask)
	_Q(borderWidth)
	_Q(bevelWidth)
	_Q(font)
	_Q(cursor)
	_Q(cursorForeground)
	_Q(cursorBackground)
	_Q(gravity)
	_Q(state)
	_Q(decoration)
	_Q(bindings)
	_Q(macro)
	_Q(olmenu)
	_Q(full)
	_Q(limited)
	_Q(none)
	_Q(pinIn)
	_Q(pinOut)
	_Q(sticky)
	_Q(shaped)
	_Q(_virtual)
	_Q(transient)
	_Q(icon)
	_Q(iconPanel)
	_Q(rootIcon)
	_Q(iconifyByUnmapping)
	_Q(pack)
	_Q(fit)
	_Q(squeeze)
	_Q(showGrid)
	_Q(hideWhenEmpty)
	_Q(defaultIconImage)
	_Q(scrollBars)
	_Q(virtualDesktop)
	_Q(ignoreGroupHints)
	_Q(ignorePPosition)
	_Q(ignorePPositionOrigin)
	_Q(constrain)
	_Q(insideBorderWidth)
	_Q(highlightFrame)
	_Q(resizeCorners)
	_Q(resizeBars)
	_Q(resizeWidth)
	_Q(grayIcon)
	_Q(noIcon)
	_Q(resizeLength)
	_Q(resizeBackground)
	_Q(randomPlacement)
	_Q(maxIconLabel)
	_Q(xorValue);
	_Q(resourceFile);
	_Q(help);
	_Q(nameObjects);
	_Q(remoteExecution);
	_Q(focusModel);
	_Q(model);
	_Q(zap);
	_Q(resizeFont);
	_Q(opaque);
	_Q(grid);
	_Q(scale);
	_Q(geometry);
	_Q(iconGeometry);
	_Q(iconic);
	_Q(initialState);
	_Q(moveDelta);
	_Q(resizeGrid);
	_Q(showOutline);
	_Q(restartPreviousState);
	_Q(rootPanels);
	_Q(rootIcons);
	_Q(configuration);
	_Q(configPath);
	_Q(resize);
	_Q(panner);
	_Q(pad);
	_Q(objectPad);
	_Q(iconGravity);
	_Q(showNames);
	_Q(iconRegions);
	_Q(iconRegion);
	_Q(reshuffle);
	_Q(overlay);
	_Q(followClient);
	_Q(packImmediate);
	XrmQuark	clientQuark()		{ return (clientQ); }
	XrmQuark	nameQuark()		{ return (nameQ); }
	XrmQuark	iconsQuark()		{ return (iconsQ); }
	XrmQuark	iconNameQuark()		{ return (iconNameQ); }
	XrmQuark	iconImageQuark()	{ return (iconImageQ); }
	XrmQuark	sizeQuark()		{ return (sizeQ); }
};

extern wmQuark	*wmQuarks;

#endif /* QUARKS_H */
