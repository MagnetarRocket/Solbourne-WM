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
 * $Id: init.C,v 9.32 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Initialization routines
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/

#ifndef lint
static char RCSinfo[] =
"$Id: init.C,v 9.32 1993/08/27 16:58:15 toml Exp $";
#endif /* lint */

#include "swm.H"
#include "init.H"
#include "main.H"
#include "gram.H"
#include "parse.H"
#include "list.H"
#include "object.H"
#include "panel.H"
#include "bitmap.H"
#include "util.H"
#include "reparent.H"
#include "events.H"
#include "execute.H"
#include "icons.H"
#include "debug.H"
#include "cursor.H"
#include "pan.H"
#include "atoms.H"
#include "swmhelp.H"
#include "quarks.H"
#include "region.H"
#include "gray.xbm"
#include "gray3.xbm"
#include "root.xbm"

#define DEFAULT_SCALE 20

XContext wmContext;
XContext wmFrameContext;	// context for frame windows
XContext wmIconContext;		// context for icon windows
XContext wmMine;		// context for my root panels
XContext wmIconPanelContext;	// context for icon panel object windows
XContext wmInternalContext;	// context for ALL my windows
XContext wmRootPanelsContext;	// context for root panels
XContext wmStateContext;	// context for object states
XContext wmBindingsContext;	// cache for bindings
XContext wmButtonsContext;	// cache for button names
int wmNumScreens;		// the number of screens 
int wmInitDone = False;		// initialization is done
int *wmTmpPtr;
char *wmRemoteExecution;
FILE *wmResourceDebug = NULL;
wmFocusMod wmFocusModel = wmFocusModelNormal;
Window wmFocusBeeper = None;
int wmNameObjects = False;
int wmMakingRootPanels = False;

int wmErrorHandler(Display *, XErrorEvent *);
static void getConfig(char *);
static void initConfigPath();
static int redirectError;
static void setCommand(Window, char *);
static void initRootPanels();
static void initRootIcons();
static void initIconRegions();

extern XButtonEvent wmResizeEvent;

#ifdef SHAPE
int wmHasShape;
int wmShapeEventBase;
int wmShapeErrorBase;
#endif /* SHAPE */

void
wmInitialize(int argc, char **argv)
{
    int error;
    int multiScreen = True;
    int firstScreen, lastScreen;
    int numManaged = 0;
    unsigned int width, height;
    IconRegion *irp;
    char *ptr;

    wmResizeEvent.type = ~ButtonPress;

    error = False;
    for (int i = 1; i < argc; i++)
    {
	if (argv[i][0] != '-')
	{
	    error = True;
	    break;
	}
	switch (argv[i][1])
	{
	    case 's':	multiScreen = False; break;
	}
    }

    if (error)
    {
	fprintf(stderr, "Usage: swm [-singlescreen]\n");
	wmDone();
    }

    wmInitHelp();
    wmQuarks = new wmQuark();

    wmContext = XUniqueContext();
    wmMine = XUniqueContext();
    wmIconPanelContext = XUniqueContext();
    wmFrameContext = XUniqueContext();
    wmIconContext = XUniqueContext();
    wmInternalContext = XUniqueContext();
    wmRootPanelsContext = XUniqueContext();
    wmStateContext = XUniqueContext();
    wmBindingsContext = XUniqueContext();
    wmButtonsContext = XUniqueContext();

    wmNumScreens = ScreenCount(DPY);
    if (multiScreen)
    {
	firstScreen = 0;
	lastScreen = wmNumScreens - 1;
    }
    else
    {
	firstScreen = lastScreen = DefaultScreen(DPY);
    }

#ifdef DEBUG
    if (!OI_debug)
#endif /* DEBUG */
	wmGrabServer();
    XSync(DPY, 0);

    XSetErrorHandler(wmErrorHandler);

#ifdef SHAPE
    wmHasShape = XShapeQueryExtension (DPY, &wmShapeEventBase, &wmShapeErrorBase);
#endif /* SHAPE */

    // go get the atoms we need
    wmInitAtoms();

    // for simplicity, always allocate NumScreens ScreenInfo struct pointers
    wmScreens = (wmScreen **)calloc(wmNumScreens, sizeof(wmScreen *));
    for (int screen = firstScreen; screen <= lastScreen; screen++)
    {
	XWindowAttributes attr;

	redirectError = False;
	XSelectInput(DPY, RootWindow(DPY, screen), SubstructureRedirectMask);
	XGetWindowAttributes(DPY, RootWindow(DPY, screen), &attr);

	if (redirectError)
	{
	    fprintf(stderr, "swm:  Are you running another window manager");
	    if (multiScreen && wmNumScreens > 0)
		fprintf(stderr, " on screen %d?\n", screen);
	    else
		fprintf(stderr, "?\n");
	    continue;
	}

	numManaged++;
	wmScr = wmScreens[screen] = (wmScreen *)calloc(1, sizeof(wmScreen));
	wmScr->screen = screen;

	if (screen != DefaultScreen(DPY))
	{
	    wmScr->conp = wmConn->open_screen(wmScr->screen);
	    if (wmScr->conp == NULL)
	    {
		fprintf(stderr, "swm: wmInitialize: can't open screen %d\n",
		    screen);
		wmDone();
	    }
	}
	else
	    wmScr->conp = wmConn;

#ifdef NEW
	wmScr->conp->set_bdr_width(OI_BUTTON_MENU, 1);
	wmScr->conp->set_bdr_width(OI_BOX, 0);
	wmScr->conp->set_bdr_width(OI_M_BOX, 0);
	wmScr->conp->set_bvl_width(OI_M_BOX, 0);
	wmScr->conp->set_bdr_width(OI_GLYPH, 0);
#endif /* NEW */

	/* I know the following mechanisms are very nice and in fact, I added them
	 * to OI, however, swm was written long before they were there and swm behaves
	 * much better if we don't use the default database - TL 08/19/91
	 */
	wmScr->conp->disallow_object_resources();
#if OI_VERSION >= 400
	wmScr->conp->database_disable(OI_default_database);
#else
	wmScr->conp->rm()->disallow_default_database();
#endif


	wmScr->conp->disallow_help();
	wmScr->conp->set_data((void *)wmScr, (void **)&wmScr);
	wmScr->conp->make_default();
	wmScr->root = RootWindow(DPY, screen);
	wmScr->depth = DefaultDepth(DPY, screen);
	wmScr->cmap = wmScr->conp->colormap();
	XInstallColormap(DPY, wmScr->cmap);
        wmScr->colorCache = XUniqueContext();
        wmScr->badColors = XUniqueContext();
        wmScr->buttons = XUniqueContext();
#ifdef DEBUG
    if (wmDebug > 1)
	fprintf(dfp, "Root colormap = %d, 0x%x\n", wmScr->cmap, wmScr->cmap);
#endif /* DEBUG */
	wmScr->black = BlackPixel(DPY, screen);
	wmScr->white = WhitePixel(DPY, screen);
	wmScr->vwidth = wmScr->width = DisplayWidth(DPY, screen);
	wmScr->vheight = wmScr->height = DisplayHeight(DPY, screen);
	wmScr->savePointerX = wmScr->width/2;
	wmScr->savePointerY = wmScr->height/2;
	wmScr->saveVrootX = 1;
	wmScr->saveVrootY = 1;
	wmPutVersion();
#ifdef OLD
	sprintf(wmScr->res, "%s.screen%d.", DisplayCells(DPY, screen)>2?"color":"monochrome", wmScr->screen);
	RM->push("swm", "Swm");
	if (DisplayCells(DPY, screen) > 2)
		RM->push("color", "Color");
	else
		RM->push("monochrome", "Monochrome");
	sprintf(string, "screen%d", wmScr->screen);
	RM->push(string, string);
#endif /* OLD */
	wmScr->rm_stack = RM->stack_ptr();

	wmScr->bitmaps = XUniqueContext();

#ifdef DEBUG
	if (screen == firstScreen)
	    wmInitDebug();
#endif
	wmScr->objectList.init();
        wmScr->windowList.init();
        wmScr->bitmapPathList.init();
        wmScr->configPathList.init();
        wmScr->iconPanelList.init();
        wmScr->startList.init();
	wmScr->rootObjectsList.init();
	wmScr->iconRegionList.init();

	// go get start up information
	wmGet__SWM_START();

	initConfigPath();
	getConfig((char *)NULL);
	// if there is no Swm*decoration resource, then chances are
	// pretty good that swm is going to crash,  go get the default
	// configuration
	if ((ptr = RM->get_resourceq(wmQuarks->decorationName(), wmQuarks->decorationClass())) == NULL)
	{
	    getConfig("Default");
	}

	//------------------------------------------------------
	//  Initialize Pixmaps
	//------------------------------------------------------
	wmScr->gray = XCreatePixmapFromBitmapData(DPY, wmScr->root, gray_bits, gray_width, gray_height, 1, 0, 1);
	wmScr->gray3 = XCreatePixmapFromBitmapData(DPY, wmScr->root, gray3_bits, gray3_width, gray3_height, 1, 0, 1);

	//------------------------------------------------------
	//  Initialize GCs
	//------------------------------------------------------
	unsigned long gcm;
	XGCValues gcv;

	wmScr->gc = XCreateGC(DPY, wmScr->root, (unsigned long)0, (XGCValues *)0);

	gcm = 0;
	gcm |= GCForeground;   		gcv.foreground = wmScr->black;
	gcm |= GCBackground;   		gcv.background = wmScr->white;
	gcm |= GCLineWidth;		gcv.line_width = 0;
	//gcm |= GCCapStyle;		gcv.cap_style = CapProjecting;
	wmScr->cornerGC = XCreateGC(DPY, wmScr->root, gcm, &gcv);

	gcv.foreground = ~((~0L) << DisplayPlanes(DPY,screen));
	ptr = RM->get_resourceq(wmQuarks->xorValueName(), wmQuarks->xorValueClass());
	if (ptr)
	    gcv.foreground = strtol(ptr, (char **)NULL, 0);

	gcm = 0;
	gcm |= GCFunction;     		 gcv.function = GXxor;
	gcm |= GCLineWidth;    		 gcv.line_width = 0;
	gcm |= GCForeground;
	gcm |= GCSubwindowMode;		 gcv.subwindow_mode = IncludeInferiors;
	wmScr->outlineGC = XCreateGC(DPY, wmScr->root, gcm, &gcv);

	gcm = 0;
	gcm |= GCFillStyle;		gcv.fill_style = FillStippled;
	gcm |= GCStipple;		gcv.stipple = wmScr->gray3;
	gcm |= GCSubwindowMode;	gcv.subwindow_mode = IncludeInferiors;
	wmScr->gray3GC = XCreateGC(DPY, wmScr->root, gcm, &gcv);

	gcm = 0;
	gcm |= GCFillStyle;		gcv.fill_style = FillStippled;
	gcm |= GCStipple;		gcv.stipple = wmScr->gray;
	gcm |= GCLineWidth;		gcv.line_width = 0;
	gcm |= GCForeground;		gcv.foreground = wmScr->black;
	gcm |= GCBackground;		gcv.background = wmScr->white;
	wmScr->grayGC = XCreateGC(DPY, wmScr->root, gcm, &gcv);


	wmInitBitmapPath();

	ptr = RM->get_resourceq(wmQuarks->resourceFileName(), wmQuarks->resourceFileClass());
	if (ptr)
	{
	    ptr = wmExpandFilename(ptr);
	    if ((wmResourceDebug = fopen(ptr, "w")) == NULL)
	    {
		fprintf(stderr, "swm: wmInitialize: couldn't open \"%s\"\n",
		    ptr);
	    }
	}
	ptr = RM->get_resourceq(wmQuarks->helpName(), wmQuarks->helpClass());
	if (ptr && (ptr[0] == 't' || ptr[0] == 'T'))
	    wmScr->help = True;

	if (screen == firstScreen)
	{
	    ptr = RM->get_resourceq(wmQuarks->nameObjectsName(), wmQuarks->nameObjectsClass());
	    if (ptr && (ptr[0] == 't' || ptr[0] == 'T'))
		wmNameObjects = True;

	    wmRemoteExecution = RM->get_resourceq(wmQuarks->remoteExecutionName(), wmQuarks->remoteExecutionClass());
	    if (!wmRemoteExecution)
		wmRemoteExecution = "machine=%s; cmd='DISPLAY=%s PATH=/usr/bin/X11:/usr/ucb:/usr/bin:/usr/local/bin %s < /dev/null > /dev/null 2>&1 &'; cmd=\\'$cmd\\'; rsh -n $machine /bin/sh -c $cmd &";

	    XSetInputFocus(DPY, PointerRoot, RevertToPointerRoot, CurrentTime);
	    ptr = RM->get_resourceq(wmQuarks->focusModelName(), wmQuarks->focusModelClass());
	    if (ptr && (ptr[0] == 'c' || ptr[0] == 'C'))
	    {
		wmFocusModel = wmFocusModelClickToType;
		XSetWindowAttributes attr;
		attr.event_mask = KeyPressMask;
		attr.override_redirect = True;
		wmFocusBeeper = XCreateWindow(DPY, wmScr->root, -10,-10, 10, 10, 0,
		    (int)CopyFromParent, InputOnly, (Visual *)CopyFromParent, (unsigned long)(CWEventMask|CWOverrideRedirect), &attr);
		OI_dispatch_insert(wmFocusBeeper, KeyPress, KeyPressMask, (OI_event_fnp)wmBeeper, (char *)0);
		XMapWindow(DPY, wmFocusBeeper);
		XSetInputFocus(DPY, wmFocusBeeper, RevertToPointerRoot, CurrentTime);
	    }
	}

	// what model do we want to run in
	if ((ptr = RM->get_resourceq(wmQuarks->modelName(), wmQuarks->modelClass())))
	{
	    int len = strlen(ptr);
	    if (ptr[0] == 'm' || ptr[0] == 'M')
		wmScr->conp->set_model(OI_MOTIF);
	    else if (ptr[0] == 'o' || ptr[0] == 'O') {
		if ((len > 2) && (ptr[len-2] == '3'))
		    wmScr->conp->set_model(OI_OPENLOOK_3D);
		else
		    wmScr->conp->set_model(OI_OPENLOOK);
	    }
	}

	wmScr->zap = True;
	if ((ptr = RM->get_resourceq(wmQuarks->zapName(), wmQuarks->zapClass())))
	{
	    if (ptr[0] == 'f' || ptr[0] == 'F')
		wmScr->zap = False;
	}

	wmScr->nameOI = oi_create_static_text(NULL," ");
	wmScr->nameOI->allow_map_raised();
	wmScr->sizeOI = oi_create_static_text(NULL,"       x       ");
	wmScr->sizeOI->allow_map_raised();
	ptr = RM->get_resourceq(wmQuarks->resizeFontName(), wmQuarks->resizeFontClass());
	if (!ptr)
	{
	    RM->pushq(wmQuarks->resizeName(), wmQuarks->resizeClass());
	    ptr = RM->get_resourceq(wmQuarks->fontName(), wmQuarks->fontClass());
	    RM->pop();
	}
	if (ptr) {
	    wmScr->sizeOI->set_font(ptr);
	    wmScr->nameOI->set_font(ptr);
	}

	int x = (wmScr->width - wmScr->sizeOI->size_x()) / 2;
	int y = (wmScr->height - wmScr->sizeOI->size_y()) / 2;
	wmScr->nameOI->set_bdr_width(2);
	wmScr->sizeOI->set_bdr_width(2);
	wmScr->sizeOI->set_associated_object(wmScr->conp->abs_root(), x, y,
	    OI_ACTIVE_NOT_DISPLAYED);
	y -= wmScr->sizeOI->space_y();
	wmScr->nameOI->set_associated_object(wmScr->conp->abs_root(), x, y,
	    OI_ACTIVE_NOT_DISPLAYED);


	wmScr->rootWeave = XCreatePixmapFromBitmapData(DPY, wmScr->root, root_weave_bits, root_weave_width, root_weave_height, 
		wmScr->black, wmScr->white, wmScr->depth);

	wmScr->vdt = NULL;
	wmScr->vroot = wmScr->conp->abs_root();
	if ((ptr = RM->get_resourceq(wmQuarks->virtualDesktopName(), wmQuarks->virtualDesktopClass())))
	{
	    XSetWindowAttributes sattr;
	    unsigned int width, height;
	    int xret, yret;

	    int flags = XParseGeometry(ptr, &xret, &yret, &width, &height);
	    if (!(flags & WidthValue))
		width = 2;
	    if (!(flags & HeightValue))
		height = 2;

	    if (width < wmScr->width)
		width = width * wmScr->width;
	    if (height < wmScr->height)
		height = height * wmScr->height;

	    if (width > 32000)
		width = 32000;
	    if (height > 32000)
		height = 32000;

	    RM->set_stack_ptr(wmScr->rm_stack);
	    RM->pushq(wmQuarks->virtualDesktopName(), wmQuarks->virtualDesktopClass());
	    ptr = RM->get_resourceq(wmQuarks->scrollBarsName(), wmQuarks->scrollBarsClass());
	    wmScr->scrollBars = !(ptr && (ptr[0] == 'F' || ptr[0] == 'f'));
	    wmScr->vdt = oi_create_scroll_box(NULL,
	    	OI_SCROLL_BAR_RIGHT | OI_SCROLL_BAR_BOTTOM,
		100, 100, width, height);
	    sattr.backing_store = NotUseful;
	    XChangeWindowAttributes(DPY, wmScr->vdt->X_window(), CWBackingStore, &sattr);
	    SAVE_INTERNAL(wmScr->vdt->outside_X_window());
	    wmScr->vdt->set_name("_swm_virtual_root");
	    OI_d_tech *d_tech = (OI_d_tech *)wmScr->vdt->viewport();
	    d_tech->set_bdr_width(0);
	    d_tech->set_bvl_width(0);
	    wmScr->vdt->set_bdr_width(0);
	    wmScr->vdt->set_bvl_width(0);
	    if (wmScr->scrollBars)
	    {
		OI_scroll_bar *rsb, *bsb;
		rsb = (OI_scroll_bar *)wmScr->vdt->right_scroll_bar();
		bsb = (OI_scroll_bar *)wmScr->vdt->bottom_scroll_bar();
		rsb->disallow_motion_callback();
		bsb->disallow_motion_callback();
		wmScr->vdt->set_size(wmScr->width, wmScr->height);
		RM->pushq(wmQuarks->scrollBarsName(), wmQuarks->scrollBarsClass());
		if ((ptr = RM->get_resourceq(wmQuarks->opaqueName(), wmQuarks->opaqueClass())))
		{
		    if (ptr[0] == 't' || ptr[0] == 'T')
		    {
			rsb->allow_motion_callback();
			bsb->allow_motion_callback();
		    }
		}
		RM->pop();
	    }
	    else
	    {
		wmScr->vdt->set_view_size(wmScr->width, wmScr->height);
	    }

	    ptr = NULL;
#ifdef DEBUG
	    ptr = RM->get_resource("reparent", "Reparent");
#endif /* DEBUG */
	    if (ptr == NULL || ptr[0] == 'F' || ptr[0] == 'f') 
	    {
		XSetWindowAttributes sattr;
		sattr.override_redirect = True;
		XChangeWindowAttributes(DPY, wmScr->vdt->outside_X_window(), CWOverrideRedirect, &sattr);
	    }
	    wmScr->vroot = (OI_d_tech *)wmScr->vdt->object_box();
	    wmScr->vroot->set_bdr_width(0);
	    wmScr->vroot->set_bvl_width(0);

	    wmObjectData *odp = new wmObjectData(NULL);
	    wmScr->vroot->set_data((void *)odp);
	    XSetWindowBackgroundPixmap(DPY, wmScr->vroot->X_window(), wmScr->rootWeave);
	    wmGetOIResources((OI_d_tech *)wmScr->vroot);
	    wmGetPixmap(wmScr->vroot);
	    XSaveContext(DPY, wmScr->vdt->outside_X_window(), wmMine, (caddr_t)0);
	    Window tmpWin = wmScr->vroot->X_window();
	    XChangeProperty(DPY, wmScr->vdt->outside_X_window(), __SWM_VROOT, XA_WINDOW, 32,
		PropModeReplace, (unsigned char *)&tmpWin, 1);

	    // figure out the size of the panner object
	    int pwidth = wmScr->vwidth = width;
	    int pheight = wmScr->vheight = height;
	    RM->pushq(wmQuarks->pannerName(), wmQuarks->pannerClass());

	    wmScr->pannerGrid = False;
	    if ((ptr = RM->get_resourceq(wmQuarks->gridName(), wmQuarks->gridClass())))
	    {
		wmScr->pannerGrid = True;
		wmGetColor(ptr, &wmScr->pannerGridFg);
	    }

	    if ((ptr = RM->get_resourceq(wmQuarks->scaleName(), wmQuarks->scaleClass())))
	    {
		int badScale = False;
		int scale = 0;
		int tmp_width, tmp_height;
		sscanf(ptr, "%d", &scale);
		if (scale <= 0)
		{
		    scale = DEFAULT_SCALE;
		    badScale = True;
		}
		tmp_width = pwidth/scale;
		tmp_height = pheight/scale;
		if (tmp_width == 0 || tmp_height == 0)
		{
		    pwidth /= DEFAULT_SCALE;
		    pheight /= DEFAULT_SCALE;
		    badScale = True;
		}
		else
		{
		    pwidth = tmp_width;
		    pheight = tmp_height;
		}
		if (badScale)
			fprintf(stderr, "swm: wmInitialize: Bad panner scale, defaulting to %d\n", DEFAULT_SCALE);
	    }
	    else
	    {
		pwidth /= DEFAULT_SCALE;
		pheight /= DEFAULT_SCALE;
	    }

	    // now create the panner
	    wmScr->pan = oi_create_panner(NULL,pwidth, pheight, (OI_scroll_2d_fnp)NULL, NULL, (OI_pan_paint_fnp)wmPaintPanner, NULL);
	    wmScr->pan->allow_model_info();
	    XSaveContext(DPY, wmScr->pan->X_window(), wmMine, (caddr_t)0);
	    XSetWindowBackgroundPixmap(DPY, wmScr->pan->X_window(), wmScr->rootWeave);
	    wmScr->pan->set_span(width, height);
	    wmScr->pan->set_view(wmScr->width, wmScr->height);
	    odp = new wmObjectData(NULL);
	    wmScr->pan->set_data((void *)odp);
	    wmGetOIResources((OI_d_tech *)wmScr->pan);
	    ptr = RM->get_resourceq(wmQuarks->showNamesName(), wmQuarks->showNamesClass());
	    wmScr->pannerNames = (ptr && (ptr[0] == 't' || ptr[0] == 'T'));
	    wmGetPixmap((OI_d_tech *)wmScr->pan);
	    wmScr->pan->set_bdr_width(1);
	    wmScr->pan->set_name("Virtual Desktop");
	    OI_dispatch_insert(wmScr->vroot->X_window(), Expose, ExposureMask, (OI_event_fnp)wmExposeVroot, (char *)0);
	    OI_dispatch_insert(wmScr->vroot->X_window(), ReparentNotify, SubstructureNotifyMask, (OI_event_fnp)wmVrootReparentNotify, (char *)0);
	    OI_dispatch_insert(wmScr->vroot->X_window(), CreateNotify, SubstructureNotifyMask, (OI_event_fnp)wmVrootCreateNotify, (char *)0);

	    OI_dispatch_insert(wmScr->pan->X_window(), ButtonRelease, ButtonReleaseMask, (OI_event_fnp)wmReleasePanner, (char *)0);
	    OI_dispatch_insert(wmScr->pan->X_window(), EnterNotify, EnterWindowMask, (OI_event_fnp)NULL, (char *)0);
	    OI_dispatch_insert(wmScr->pan->X_window(), LeaveNotify, LeaveWindowMask, (OI_event_fnp)NULL, (char *)0);

	    XGetWindowAttributes(DPY, wmScr->pan->X_window(), &attr);

	    // if there is a geometry resource, place the thing now so
	    // it can get reparented.
	    wmScr->vdt->set_associated_object(wmScr->conp->abs_root(),0,0,OI_ACTIVE);
	    ptr = RM->get_resourceq(wmQuarks->geometryName(), wmQuarks->geometryClass());
	    char *iptr = RM->get_resourceq(wmQuarks->iconGeometryName(), wmQuarks->iconGeometryClass());

	    // if the geometry is specified, it should be displayed
	    XSizeHints sizeHints;
	    XClassHint xclass;
	    xclass.res_class = "Swm";
	    xclass.res_name = "virtualDesktop";
	    XWMHints wmhints;
	    sizeHints.flags = USPosition | USSize | PWinGravity;
	    static XTextProperty wName={(unsigned char *) "Virtual Desktop",XA_STRING,8,15};
	    static XTextProperty iName={(unsigned char *) "Desktop",XA_STRING,8,7};
	    int x, y;
	    char def[20];

	    sprintf(def, "%dx%d+0+0", pwidth, pheight);
	    XWMGeometry(DPY, wmScr->screen, ptr, def, wmScr->pan->bdr_width(), &sizeHints, &x, &y, (int *)&width, (int *)&height, &sizeHints.win_gravity);

	    // parse the icon geometry (if any)
	    wmhints.icon_x = 0;
	    wmhints.icon_y = 0;
	    wmhints.flags = StateHint;
	    wmhints.initial_state = NormalState;
	    int mask = XParseGeometry(iptr, &wmhints.icon_x, &wmhints.icon_y, &width, &height);
	    if ((mask & XValue) || (mask & YValue))
		wmhints.flags |= IconPositionHint;

	    // This was the old resource
	    ptr = RM->get_resourceq(wmQuarks->iconicName(), wmQuarks->iconicClass());
	    if (ptr && (ptr[0] == 'T' || ptr[0] == 't'))
		wmhints.initial_state = IconicState;

	    // now handle the new resource
	    if ((ptr = RM->get_resourceq(wmQuarks->initialStateName(), wmQuarks->initialStateClass())))
	    {
		switch (ptr[0])
		{
		    case 'w':
		    case 'W':
			wmhints.initial_state = WithdrawnState;
			break;
		    case 'n':
		    case 'N':
			wmhints.initial_state = NormalState;
			break;
		    case 'i':
		    case 'I':
			wmhints.initial_state = IconicState;
			break;
		}
	    }
	    
	    wmScr->vscale = wmScr->vwidth / wmScr->pan->size_x();

	    wmScr->pan->set_associated_object(wmScr->conp->abs_root(),x,y, OI_ACTIVE_NOT_DISPLAYED);
	    // if NormalState or IconicState, map the panner
	    if (wmhints.initial_state == NormalState || wmhints.initial_state == IconicState)
		wmScr->pan->set_state(OI_ACTIVE);

	    XSetWMProperties(DPY, wmScr->pan->X_window(), &wName, &iName, NULL, 0, &sizeHints, &wmhints, &xclass);
	    setCommand(wmScr->pan->X_window(), "VirtualDesktopPanner");

	    RM->pop();
	    RM->pop();
	}

	ptr = RM->get_resourceq(wmQuarks->moveDeltaName(), wmQuarks->moveDeltaClass());
	if (ptr) wmScr->moveDelta = atoi(ptr);
	else wmScr->moveDelta = 3;

	ptr = RM->get_resourceq(wmQuarks->resizeGridName(), wmQuarks->resizeGridClass());
	if (ptr && (ptr[0] == 'F' || ptr[0] == 'f'))
	    wmScr->resizeGrid = False;
	else
	    wmScr->resizeGrid = True;

	ptr = RM->get_resourceq(wmQuarks->showOutlineName(), wmQuarks->showOutlineClass());
	if (ptr && (ptr[0] == 'F' || ptr[0] == 'f'))
	    wmScr->showGridSave = False;
	else
	    wmScr->showGridSave = True;

	ptr = RM->get_resourceq(wmQuarks->restartPreviousStateName(), wmQuarks->restartPreviousStateClass());
	if (ptr && (ptr[0] == 'F' || ptr[0] == 'f'))
	    wmScr->restartPreviousState = False;
	else
	    wmScr->restartPreviousState = True;

	wmScr->randStartingX = 30;
	wmScr->randStartingY = 30;
	wmScr->randX = 30;
	wmScr->randY = 30;
	wmScr->randIncX = 20;
	wmScr->randIncY = 20;

	wmInitCursors();
	wmInitEvents();

	initRootPanels();
	initIconRegions();
	// get width & height, check if changed after reparent
	// if so, expand (YUCK)
	width = wmScr->vwidth;
	height = wmScr->vheight;
	wmReparentExisting();
	initRootIcons();
	if (width != wmScr->vwidth || height != wmScr->vheight)
	{
	    for (irp = (IconRegion *)wmScr->iconRegionList.first(); 
		irp != NULL; irp = (IconRegion *)wmScr->iconRegionList.next())
		irp->expand(width, height, wmScr->vwidth, wmScr->vheight);
	}
    }

    if (numManaged == 0)
    {
	if (multiScreen && wmNumScreens > 0)
	    fprintf(stderr, "swm:  No screens to manage,  bye.\n");
	wmDone();
    }
    wmUngrabServer();
    wmInitDone = True;
}

static void
initRootPanels()
{
    static char *ptr = NULL;
    XSizeHints size;
    wmPanelKid *kp;
    wmObject *op;
    wmList *mylist = new wmList();

    ptr = RM->get_resourceq(wmQuarks->rootPanelsName(), wmQuarks->rootPanelsClass());
    if (ptr != NULL)
    {
	wmParse("wmPanel", ptr);
	if (wmParseError)
	{
	    fprintf(stderr, "swm: initRootPanels: error parsing root panels\n");
	    return;
	}

	while ((kp = (wmPanelKid *)wmPanelKidsList.get()) != NULL)
	{
	    // go create a panel for it
	    op = wmCreateObject(OBJ_PANEL, kp->name);
	    op->u.p->geom = kp->geom;
	    op->u.p->root = True;
	}

	// OK, we have the root panel objects created and in the 
	// object list.  Now let's expand those suckers
	wmExpandObjects();
    }

    // let's create our dummy object on which to tie root bindings
    op = wmCreateObject(OBJ_PANEL, "root");
    op->root = True;		// so no problems when we instantiate it
    op->u.p->geom = new wmGeometry();
    op->u.p->root = True;

    // Now let's instantiate the dudes
    RM->set_stack_ptr(wmScr->rm_stack);
    for (op = (wmObject *)wmScr->objectList.first(); op != NULL; op = (wmObject *)wmScr->objectList.next())
	mylist->append((ent)op);

    wmMakingRootPanels = True;
    for (op = (wmObject *)mylist->first(); op != NULL; op = (wmObject *)mylist->next())
    {
	// The only objects with a geometry in the object structure 
	// are root window panels.  Other objects will have a kid
	// structure that will contain the geometry
	if (op->type == OBJ_PANEL && op->u.p->root)
	{
	    XClassHint xclass;
	    int x, y;
	    OI_d_tech *tmpoi;
	    int placed;

	    wmInstantiateObject(op);
	    // the name may have been stripped off, so put it back
	    op->oi->set_name(op->name);
	    wmScr->rootObjectsList.append((ent)op->oi);

	    XSaveContext(DPY, op->oi->outside_X_window(), wmMine, (caddr_t)op);
	    XSaveContext(DPY, op->oi->outside_X_window(), wmRootPanelsContext, (caddr_t)op->oi);

	    placed = False;
	    if ((tmpoi = op->oi->descendant("icons")) != NULL)
	    {
		wmObjectData *odp = (wmObjectData *)tmpoi->data();
		op->u.p->icon = odp->op->u.p->icon;
		op->u.p->icon->op = op;
		if (op->u.p->icon->hide)
		{
		    op->state = wmUnmapped;
		    placed = True;
		}
	    }
	    wmLayoutPanel(op);

	    if (op->u.p->geom->sign_x < 0)
		x = DisplayWidth(DPY, wmScr->screen) - op->u.p->geom->x - op->oi->space_x();
	    else
		x = op->u.p->geom->x;

	    if (op->u.p->geom->sign_y < 0)
		y = DisplayHeight(DPY, wmScr->screen) - op->u.p->geom->y - op->oi->space_y();
	    else
		y = op->u.p->geom->y;

	    xclass.res_class = "Swm";
	    xclass.res_name = (char*)op->oi->name();
	    XSetClassHint(DPY, op->oi->outside_X_window(), &xclass);

	    setCommand(op->oi->outside_X_window(), (char*)op->oi->name());

	    size.flags = PMinSize | PMaxSize;
	    size.min_width = size.max_width = op->oi->size_x();
	    size.min_height = size.max_height = op->oi->size_y();
	    XSetNormalHints(DPY, op->oi->outside_X_window(), &size);

	    if (op->root)
	    {
		Window root_window;
		if (wmScr->vdt)
			root_window = wmScr->vroot->X_window();
		else
			root_window = wmScr->root;
		OI_dispatch_insert(root_window, KeyPress, KeyPressMask, (OI_event_fnp)wmHandleKeyPress, (char *)op->oi);
		OI_dispatch_insert(root_window, ButtonPress, ButtonPressMask, (OI_event_fnp)wmHandlePress, (char *)op->oi);
		if (wmScr->help)
		{
		    OI_dispatch_insert(wmScr->root, EnterNotify, EnterWindowMask, (OI_event_fnp)wmHelpEnterNotify, (char *)op->oi);
		    if (wmScr->vdt)
			OI_dispatch_insert(wmScr->vroot->X_window(), EnterNotify, EnterWindowMask, (OI_event_fnp)wmHelpEnterNotify, op->oi);
		}
		wmScr->conp->abs_root()->set_data(op->oi->data());
		op->oi->set_associated_object(wmScr->conp->abs_root(), x, y, OI_ACTIVE_NOT_DISPLAYED);
	    }
	    else
	    {
		op->oi->set_associated_object(wmScr->conp->abs_root(), x, y, wmState(op->state));
		if (placed)
		    wmReparent(op->oi->X_window(), wmScr->root, wmScr->vroot);
	    }
	}
    }
    wmMakingRootPanels = False;

#ifdef DEBUG
    wmListObjects();
#endif /* DEBUG */
}

static void
initRootIcons()
{
    static char *ptr = NULL;
    wmData *wp;
    int x, y;
    wmPanelKid *kp;
    wmList *mylist = new wmList();

    ptr = RM->get_resourceq(wmQuarks->rootIconsName(), wmQuarks->rootIconsClass());
    if (ptr != NULL)
    {
	wmParse("wmPanel", ptr);
	if (wmParseError)
	{
	    fprintf(stderr, "swm: initRootIcons: error parsing root icons\n");
	    return;
	}

	while ((kp = (wmPanelKid *)wmPanelKidsList.get()) != NULL)
	    mylist->append((ent)kp);

	while ((kp = (wmPanelKid *)mylist->get()) != NULL)
	{
	    wp = new wmData();
	    wp->new_icon();

	    wp->set_root(wmScr->vroot);
	    wp->set_mine();
	    wp->set_class_class("Swm");
	    wp->set_class_name(kp->name);
	    wmClient = wp;
	    RM->set_stack_ptr(wmScr->rm_stack);
	    RM->push(wp->wclass_class(), wp->wclass_class());
	    RM->push(wp->wclass_name(), wp->wclass_name());
	    if ((ptr = RM->get_resourceq(wmQuarks->stickyName(), wmQuarks->stickyClass())))
	    {
		if (ptr[0] == 'T' || ptr[0] == 't')
		{
		    wp->set_sticky();
		    wp->set_root(wmScr->conp->abs_root());
		    RM->pushq(wmQuarks->stickyName(), wmQuarks->stickyClass());
		}
	    }
	    wp->set_icon_object(kp->name);
	    x = kp->geom->x;
	    y = kp->geom->y;
	    wmMakeIcon(wp, x, y);
	    if (wp->oi_icon())
		wmScr->rootObjectsList.append((ent)wp->oi_icon());
	    wmClient = NULL;
	}
    }
}

static void
setCommand(Window w, char *name)
{
    char buff[256];
    char *cp = buff;

    sprintf(buff, "SwmInternal.%s", name);
    XSetCommand(DPY, w, &cp, 1);
}

static char *lastConfig = NULL;

void
wmReConfig()
{
    lastConfig = NULL;
    getConfig(wmScr->firstConfig);
} 

static void
getConfig(char *config)
{
    char *path;
    char *ptr;
    wmList list;
    wmList pathList;

    // get configuration files (if any) loaded into the resource
    // database
    pathList.init();
    ptr = config;
    if (!config)
	ptr = RM->get_resourceq(wmQuarks->configurationName(), wmQuarks->configurationClass());
    if (!wmScr->firstConfig)
    {
	if (ptr)
	{
	    wmScr->firstConfig = (char *)malloc(strlen(ptr)+1);
	    strcpy(wmScr->firstConfig, ptr);
	}
    }
    if (lastConfig && ptr && !strcmp(lastConfig, ptr))
	return;
    lastConfig = ptr;

    if (ptr)
    {
	wmParse("wmStrings", ptr);
	list = wmStringList;
	wmStringList.init();
	if (wmParseError)
	{
	    fprintf(stderr,
		"swm: wmInitialize: error parsing configuration\n");
	}
	else
	{
	    while ((ptr = (char *)list.get()) != NULL)
	    {
		for (path = (char *)wmScr->configPathList.first(); path != NULL; path = (char *)wmScr->configPathList.next())
		    pathList.append((ent)path);
		path = NULL;

		ptr = wmExpandFilename(ptr);
		if (wmScr->conp->add_resources(ptr) == OI_NO)
		{
		    for (path = (char *)pathList.first(); path != NULL; path = (char *)pathList.next())
		    {
			char *t = (char *)malloc(strlen(path) + strlen(ptr) + 4);
			strcpy(t, path);
			strcat(t, "/");
			strcat(t, ptr);
			int status = wmScr->conp->add_resources(t);
			free(t);
			if (status == OI_YES)
			{
			    getConfig((char *)NULL);
			    break;
			}
		    }
		    if (path == NULL)
		    {
			fprintf(stderr, "swm: wmInitialize: couldn't find configuration file \"%s\"\n", ptr);
			break;
		    }
		}
		else
		    getConfig((char *)NULL);
		free(ptr);
	    }
	}
    }
    while ((path = (char *)pathList.get()) != NULL) ;
}

static char *defaultPath1 = SWM_DEFAULTS;

static void
initConfigPath()
{
#ifdef DEBUG
    if (wmDebug)
	fprintf(dfp, "initConfigPath:\n");
#endif

    char *ptr = RM->get_resourceq(wmQuarks->configPathName(), wmQuarks->configPathClass());
    if (ptr != NULL)
    {
	wmParse("wmStrings", ptr);
	if (wmParseError)
	{
	    fprintf(stderr,
		"swm: initBitmapPath: error parsing config path\n");
	}
	else
	{
	    while ((ptr = (char *)wmStringList.get()) != NULL)
	    {
		ptr = wmExpandFilename(ptr);
		wmScr->configPathList.append((ent)ptr);
	    }
	}
    }
    wmScr->configPathList.append((ent)defaultPath1);
}

int
wmErrorHandler(
    Display *,
    XErrorEvent *
    )
{
    redirectError = True;
    return (0);
}

static void
initIconRegions()
{
    Bool defaultRegion = False;
    char *ptr;
    wmRegionKid *kp;

    ptr = RM->get_resourceq(wmQuarks->iconRegionsName(), wmQuarks->iconRegionsClass());
    if (ptr != NULL)
    {
	wmParse("wmRegion", ptr);
	if (wmParseError)
	{
	    fprintf(stderr, "swm: initIconRegions: error parsing icon regions\n");
	    return;
	}
	while ((kp = (wmRegionKid *)wmRegionKidsList.get()) != NULL)
	{
	    // go create a region for it
	    wmScr->iconRegionList.append((ent) new IconRegion(kp->type, kp->name, kp->geom));
	    if (!defaultRegion)
		defaultRegion = !strcmp("Default", kp->name);
	}
    }
    // check iconGravity only if Default isn't already defined
    if (!defaultRegion)
    {
	ptr = RM->get_resourceq(wmQuarks->iconGravityName(), 
		wmQuarks->iconGravityClass());
	if (ptr) 
	    wmScr->iconRegionList.append((ent) new IconRegion(ptr, "Default"));
    }

}
