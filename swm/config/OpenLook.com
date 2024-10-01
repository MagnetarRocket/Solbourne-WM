!**********************************************************************
!
!    Common Open Look stuff
!
!    $Id: OpenLook.com,v 9.10 1991/05/03 06:41:35 kelly Exp $
!**********************************************************************

!change some of the default cursors to something else
Swm*model: OpenLook
Swm*moveCursor:		top_left_arrow
Swm*resizeCursor:	top_left_arrow

! OpenLook only wants to see the outline of the window
Swm*resizeGrid:	False

! icon gravity, icons tile top-right to bottom-right
Swm*iconGravity:	NorthEast

! What the mouse buttons do on the root window
Swm*root.bindings: \
    <Btn3> : f.menu(workspace)

Swm*button.pin: @OLpinOut.xbm
Swm*pinIn*button.pin: @OLpinIn.xbm

Swm*panel.openLook*button.pin.bindings: \
    bindings pin
Swm*olmenu.limited*panel.openLook*button.pin.bindings: \
    bindings pin
Swm*olmenu.none*panel.openLook*button.pin.bindings: \
    bindings pin
Swm*pinIn*panel.openLook*button.pin.bindings: \
    bindings unpin
Swm*olmenu.limited*pinIn*panel.openLook*button.pin.bindings: \
    bindings unpin
Swm*olmenu.none*pinIn*panel.openLook*button.pin.bindings: \
    bindings unpin

Swm*bindings.pin: \
    <Btn> : f.ungroup f.pin f.newbuttonimage(pin, @OLpinIn.xbm) f.rebind(pin, unpin) f.regroup

Swm*bindings.unpin: \
    <Btn> : f.ungroup f.unpin f.newbuttonimage(pin, @OLpinOut.xbm) f.rebind(pin, pin) f.regroup

Swm*decoration: openLook
Swm*panel.openLook: \
	button	pullDown	+0+0	\
	button	name		+c+0	\
	panel	client	+0+1

! Here come the 16 different open look decoration 
! panels.  The bits are specified like this:
!
!     3         2        1         0
! OL_RESIZE  OL_PIN  OL_CLOSE  OL_HEADER

Swm*openLook.0*decoration: openLook.0
Swm*panel.openLook.0: \
	panel	client	+0+1
Swm*panel.openLook0.pad: 4

Swm*openLook.1*decoration: openLook.1
Swm*panel.openLook.1: \
	button	name		+c+0	\
	panel	client		+0+1

Swm*openLook.2*decoration: openLook.2
Swm*panel.openLook.2: \
	button	pullDown	+0+0	\
	panel	client		+0+1

Swm*openLook.3*decoration: openLook.3
Swm*panel.openLook.3: \
	button	pullDown	+0+0	\
	button	name		+c+0	\
	panel	client		+0+1

Swm*openLook.4*decoration: openLook.4
Swm*panel.openLook.4: \
	button	pin		+0+0	\
	panel	client		+0+1

Swm*openLook.5*decoration: openLook.5
Swm*panel.openLook.5: \
	button	pin		+0+0	\
	button	name		+c+0	\
	panel	client		+0+1

Swm*openLook.6*decoration: openLook.2

Swm*openLook.7*decoration: openLook.3

Swm*openLook.8*decoration: openLook.8
Swm*panel.openLook.8: \
	panel	client		+0+1

Swm*openLook.9*decoration: openLook.9
Swm*panel.openLook.9: \
	button	name		+c+0	\
	panel	client		+0+1

Swm*openLook.10*decoration: openLook.10
Swm*panel.openLook.10: \
	button	pullDown	+0+0	\
	panel	client		+0+1

Swm*openLook.11*decoration: openLook.11
Swm*panel.openLook.11: \
	button	pullDown	+0+0	\
	button	name		+c+0	\
	panel	client		+0+1

Swm*openLook.12*decoration: openLook.12
Swm*panel.openLook.12: \
	button	pin		+0+0	\
	panel	client		+0+1

Swm*openLook.13*decoration: openLook.13
Swm*panel.openLook.13: \
	button	pin		+0+0	\
	button	name		+c+0	\
	panel	client		+0+1

Swm*openLook.14*decoration: openLook.14
Swm*panel.openLook.14: \
	button	pullDown	+0+0	\
	panel	client		+0+1

Swm*openLook.15*decoration: openLook.15
Swm*panel.openLook.15: \
	button	pullDown	+0+0	\
	button	name		+c+0	\
	panel	client		+0+1

Swm*panel.openLook.8.resizeCorners: True
Swm*panel.openLook.9.resizeCorners: True
Swm*panel.openLook.10.resizeCorners: True
Swm*panel.openLook.11.resizeCorners: True
Swm*panel.openLook.12.resizeCorners: True
Swm*panel.openLook.13.resizeCorners: True
Swm*panel.openLook.14.resizeCorners: True
Swm*panel.openLook.15.resizeCorners: True

! transient window decoration
Swm*transient*decoration: openLook.1

! turn on the resize corners
Swm*panel.openLook.resizeCorners: True
Swm*panel.openLook*borderWidth: 1
Swm*panel.openLook*button*borderWidth: 0
Swm*panel.openLook*panel*borderWidth: 0
Swm*panel.openLook*insideBorderWidth: 1

! bindings for the title bar and surrounding frame
Swm*panel.openLook*bindings: \
	bindings		unzoomed

Swm*bindings.unzoomed: \
	bindings		standard		\
	<Btn3>		:	f.menu(menuFull1)

Swm*bindings.zoomed: \
	bindings		standard		\
	<Btn3>		:	f.menu(menuFull2)

Swm*bindings.standard: \
	<Btn1>		:	f.move f.stop f.raise

Swm*olmenu.limited*panel.openLook*bindings: \
    <Btn1> : f.move f.stop f.raise \
    <Btn3> : f.menu(menuLimited)
Swm*olmenu.none*panel.openLook*bindings: \
    <Btn1> : f.move f.stop f.raise

Swm*panel.openLook*panel.client.bindings:
Swm*olmenu.limited*panel.openLook*panel.client.bindings:
Swm*olmenu.none*panel.openLook*panel.client.bindings:

Swm*panel.openLook*button.name.bindings:
Swm*olmenu.limited*panel.openLook*button.name.bindings:
Swm*olmenu.none*panel.openLook*button.name.bindings:

! center the name in the title bar
Swm*button.name.gravity: Center
Swm*button.name.state: MappedNoSpace

! pulldown button when the window is not zoomed
Swm*panel.openLook*button.pullDown.bindings: \
    bindings	pullUnzoomed

Swm*bindings.pullUnzoomed: \
    <Btn1> : f.iconify \
    <Btn3> : f.menu(menuFull1)

Swm*bindings.pullZoomed: \
    <Btn1> : f.iconify \
    <Btn3> : f.menu(menuFull2)

Swm*olmenu.limited*panel.openLook*button.pullDown.bindings: \
    <Btn1> : f.iconify \
    <Btn3> : f.menu(menuLimited)

! Limited
Swm*menu.menuLimited: \
    Window		f.title \
    "Dismiss/Cancel"	f.delete \
    Back		f.lower \
    Refresh		f.winrefresh \
    "Owner?"		f.owner

! non zoomed window menu
Swm*menu.menuFull1: \
    Window		f.title \
    Close		f.iconify \
    "Full Size"		f.raise f.save f.zoom  f.rebind(pullDown, pullZoomed) f.rebind(openLook, zoomed) \
    "Properties..."	f.nop \
    Back		f.lower \
    Refresh		f.winrefresh \
    Quit		f.delete

! zoomed window menu
Swm*menu.menuFull2: \
    Window		f.title \
    Close		f.iconify \
    "Restore Size"	f.restore  f.rebind(pullDown, pullUnzoomed) f.rebind(openLook, unzoomed) \
    "Properties..."	f.nop \
    Back		f.lower \
    Refresh		f.winrefresh \
    Quit		f.delete

! the default icon
Swm*icon: Xicon
Swm*panel.Xicon: \
    button iconImage	+c+0 \
    button iconName	+c+1
Swm*defaultIconImage: @xlogo32

Swm*panel.Xicon.borderWidth: 1
Swm*panel.Xicon*bindings: \
    <Btn1>2 : f.deiconify f.raise \
    <Btn1> : f.move f.stop f.raise \
    <Btn3> : f.menu(iconMenu)

! the icon menu
Swm*menu.iconMenu: \
    Window		f.title \
    Open		f.deiconify f.raise \
    "Full Size"		f.map(pullDown2) f.raise f.save f.zoom f.deiconify \
    "Properties..."	f.nop \
    Back		f.lower \
    Refresh		f.winrefresh \
    Quit		f.delete

! the workspace (root window) menu
Swm*menu.workspace: \
    Workspace		f.titlepin \
    Programs		f.menu(programs) \
    Utilities		f.menu(utilities) \
    "Properties..."	f.nop \
    Restart		f.restart \
    Exit		f.quit \

Swm*menu.programs: \
    xterm		f.exec("xterm&") \
    xclock		f.exec("xclock&")

Swm*menu.utilities: \
    Refresh		f.refresh \
    "Window Controls..."	f.nop \
    "Clipboard..."	f.nop \
    "Print Screen"	f.nop


