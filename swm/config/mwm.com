!**********************************************************************
!
!    Swm template for mwm
!
!    Author: Tom LaStrange
!    Date:   11/01/90  Renamed, was mwm
!
!**********************************************************************

Swm*font: -adobe-courier-medium-r-normal--12-140-75-75-m-90-iso8859-1
Swm*button.name.font: variable
Swm*iconGravity: WestSouth

Swm*color*panel.motif*Background: cornflowerblue
Swm*color*menu*Background: cornflowerblue

Swm*motifDecoration.0*decoration: motif.0
Swm*motifDecoration.1*decoration: motif.1
Swm*motifDecoration.2*decoration: motif.2
Swm*motifDecoration.3*decoration: motif.3
Swm*motifDecoration.4*decoration: motif.4
Swm*motifDecoration.5*decoration: motif.5
Swm*motifDecoration.6*decoration: motif.6
Swm*motifDecoration.7*decoration: motif.7
Swm*motifDecoration.8*decoration: motif.8
Swm*motifDecoration.9*decoration: motif.9
Swm*motifDecoration.10*decoration: motif.10
Swm*motifDecoration.11*decoration: motif.11
Swm*motifDecoration.12*decoration: motif.12
Swm*motifDecoration.13*decoration: motif.13
Swm*motifDecoration.14*decoration: motif.14
Swm*motifDecoration.15*decoration: motif.15

Swm*motifDecoration*motifResize*panel*resizeCorners: True
Swm*motifDecoration*motifResize*panel*resizeBars: True

Swm*monochrome*button.menuB: @Mwmmenu.xbm
Swm*button.menuB: \
	@MmenuTB.xbm top bottom		\
	@MmenuBT.xbm bottom top

Swm*monochrome*button.min: @Mwmmin.xbm
Swm*button.min: \
	@MminTB.xbm top bottom		\
	@MminBT.xbm bottom top

Swm*monochrome*button.max: @Mwmmax.xbm
Swm*button.max: \
	@MmaxTB.xbm top bottom		\
	@MmaxBT.xbm bottom top

Swm*panel.motif*resizeWidth: 5
Swm*panel.motif*resizeLength: 21
Swm*motifBorder*panel.motif*pad: 5
Swm*model: Motif

Swm*bevelWidth: 0

Swm*panel.motif*button*bevelWidth: 1
Swm*panel.motif*button*bevelWidth: 1
Swm*panel.motif*button*menu*bevelWidth: 2
Swm*panel*menu*bevelWidth: 2
Swm*button*menu*bevelWidth: 2
Swm*panel.client.bevelWidth:2
Swm*panel.Xicon.bevelWidth: 2

Swm*panel.motif*panel.client.bindings:

Swm*panel.motif*button.menuB.bindings: \
	<Btn1> : f.menu(windowMenu)
Swm*panel.motif*button.max.bindings: \
	bindings	unzoomed
Swm*panel.motif*button.min.bindings: \
	<Btn1> : f.iconify

Swm*bindings.unzoomed: \
	<Btn1> : f.save f.raise f.zoom f.rebind(max, zoomed) \
	    f.newbuttonimage(max, "@MmaxTB.xbm bottom top @MmaxBT.xbm top bottom")
Swm*bindings.zoomed: \
	<Btn1> : f.restore f.rebind(max, unzoomed) \
	    f.newbuttonimage(max, "@MmaxTB.xbm top bottom @MmaxBT.xbm bottom top")

! Restore from the menu needs to redraw and bind the zoom button for color systems
Swm*macro.Mrestore: f.restore f.rebind(max, unzoomed) f.newbuttonimage(max, "@MmaxTB.xbm top bottom @MmaxBT.xbm bottom top")
Swm*monochrome*macro.Mrestore: f.restore 

Swm*button.name.state: MappedNoSpace

! turn on the resize corners
Swm*panel.motif.resizeCorners: True
Swm*panel.motif.resizeBars: True
Swm*panel.motif*borderWidth: 0

! bindings for the title bar and surrounding frame
Swm*panel.motif*bindings: \
    <Btn1> : f.move f.raise \
    <Btn2> : f.menu(windowMenu) \
    <Btn3> : f.lower

! center the name in the title bar
Swm*button.name.gravity: Center

! the default icon
Swm*icon: Xicon
Swm*panel.Xicon: \
    button iconImage	+c+0 \
    button iconName	+c+1
Swm*defaultIconImage: @xlogo32

Swm*panel.Xicon.borderWidth: 1
Swm*panel.Xicon.bindings: \
    <Btn1>2 : f.deiconify \
    <Btn1> : f.move f.raise \n \
    <Btn3> : f.menu(iconMenu) \n

Swm*menu.windowMenu: \
    Restore		f.macro(Mrestore) \
    Move		f.move \
    Size		f.resize \
    Minimize		f.iconify \
    Maximize		f.save f.zoom \
    Lower		f.lower \
    Close		f.delete

! the icon menu
Swm*menu.iconMenu: \
    Window		f.title \
    Restore		f.deiconify \
    Move		f.move \
    Maximize		f.save f.zoom \
    Lower		f.lower \
    Close		f.delete

! the workspace (root window) menu
Swm*menu.workspace: \
    Workspace		f.titlepin \
    Programs		f.menu(programs) \
    Utilities		f.menu(utilities) \
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

