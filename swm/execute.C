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
 * $Id: execute.C,v 9.60 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Command execution routines
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/

#ifndef lint
static char RCSinfo[] =
"$Id: execute.C,v 9.60 1993/08/27 16:58:15 toml Exp $";
#endif /* lint */

#include "swm.H"
#include "main.H"
#include "debug.H"
#include "parse.H"
#include "util.H"
#include "gram.H"
#include "execute.H"
#include "object.H"
#include "icons.H"
#include "init.H"
#include "move.H"
#include "resize.H"
#include "menus.H"
#include "pan.H"
#include "reparent.H"
#include "atoms.H"
#include "events.H"
#include "bitmap.H"
#include "panel.H"
#include <signal.h>
#include <sys/wait.h>
#include "swmhelp.H"
#include "quarks.H"
#include "region.H"
#include "button.H"
#include "toml.xbm"
#include "kelly.xbm"
#include <string.h>
#ifdef hpux
#include <sys/stat.h> /* for chmod */
#endif

#ifdef _AIX
extern "C" int usleep(unsigned int useconds);
#endif
#ifdef ultrix
#include <sys/stat.h>	/* for chmod */
extern "C" int putenv(char *);
#endif


#define STARTUP "~/.swm.restart"

wmList wmBindingsList;			// list of cached bindings
int wmMoveOpaque = False;
wmBinding *wmBind;
static wmData *firstWp;
static Window firstWindow;
static wmList macroList;
static int lastFunction = F_NOP;
static int lastStatus = True;
static int stopExecution = False;
static Atom dropAtom;
static Atom dropWindow;
static void clearStop();
static int needWindow(int);
static int groupCommand(int);
static void checkWindow(wmData **, Window *, int, int, Window);
static int lookForBinding(wmData *, Window, int, int, KeySym, KeyCode, int, int, wmBindings *);
void wmLowerWindow(wmData *, Window);

#define FIND_CONTEXT(window) \
    if (XFindContext(DPY, window, wmContext, (caddr_t*)&wp) == XCNOENT)\
	wp = NULL

static int stickIt(OI_d_tech *oi);
static void click(OI_d_tech *, void *, OI_number, OI_number, OI_number);
static int  findBinding(OI_d_tech *, OI_number, int, KeySym, KeyCode, int, int);
static int findWindow(wmData **, Window *, int, char *, int *, int *, int *);
static void shell(char *);
static void waitForButtonUp();
static wmBindings *parseBindings(char *);
static unsigned modMask = ShiftMask | ControlMask | Mod1Mask;
static void bindObject(OI_d_tech *, wmBindings *);
static void unbindObject(OI_d_tech *, wmBindings *);
static OI_cell_spec infoCells[] = { {NULL, "Dismiss", NULL,NULL,NULL_PMF,NULL,OI_TEXT_CELL,NULL,0,0,0} } ;
IconRegion *findIconRegion(char *);

/***********************************************************************
 *
 *  Procedure:
 *	wmExecute
 *
 *  Function:
 *	Execute a window manager function.
 *
 ***********************************************************************
 */

int
wmExecute(
    wmData *wp,			// window data (if any)
    Window win,			// the window (frame or icon or NULL)
    int what,			// what function to execute
    char **argv,		// argument strings
    int argc,			// argument count
    int x_root,			// the X root coordinate
    int y_root			// the Y root coordinate
    )
{
    Window junkRoot, junkChild, junkParent;
    int junkX, junkY;
    unsigned int junkBW, junkDepth;
    unsigned int junkMask;
    XButtonEvent ev;
    int first = True;
    int window_group = NULL;
    int success = False;
    Window w;
    int delta;
    int whichScreen;
    int vis;
    Atom a = None;
    int anyExecuted = False;
    OI_d_tech *oi = NULL;
    wmData *tmp_wp;
    static int terminalPlaces=False;
    int screen;

#ifdef DEBUG
    if (wmDebug)
	if (argc)
	    fprintf(dfp, "wmExecute: what = %d, argv[0] = \"%s\"\n", what, argv[0]);
	else
	    fprintf(dfp, "wmExecute: what = %d\n", what);
#endif

    // if initialization is not done, don't execute any commands
    if (!wmInitDone)
	return (False);

    ev.x_root = x_root;
    ev.y_root = y_root;

    while (True)
    {
	int status;
	Window window = win;

	status = findWindow(&wp, &window, what, argc?argv[0]:NULL, &first, &ev.x_root, &ev.y_root);
	if (!status && what != F_FOCUSCOLOR)
	{
	    if (anyExecuted == False)
		lastFunction = F_NOP;
	    else
		lastFunction = what;
	    lastStatus = success;

	    return (success);
	}

	anyExecuted = True;

	if (firstWp == NULL)
	    firstWp = wp;
	if (firstWindow == NULL)
	    firstWindow = window;

	// if we shouldn't execute on this window, don't
	if (wp && wp->stop())
	{
	    wp = NULL;
	    continue;
	}

	// assume success
	success = True;

	switch (what)
	{
	    case F_FOCUSCOLOR:
		what = F_NOP;
		if (wp) {
			XWindowAttributes attr;
			XGetWindowAttributes(DPY, wp->window(), &attr);
			XInstallColormap(DPY, attr.colormap);
			wmScr->colormapWindow = wp->window();
			wmScr->colormapFocus = wp;
		}
		else {
			wmScr->colormapFocus = NULL;
			wmResetColormaps();
		}
		break;
	
	    case F_OWNER:
		{
			wmData *tmpwp;
			tmpwp = wp->transient_for_wp();
			if (!tmpwp && wp->group()) {
				if (XFindContext(DPY, wp->group(), wmContext, (caddr_t*)&tmpwp) == XCNOENT)
					tmpwp = NULL;
			}
			if (tmpwp)
			{
				PIXEL bg;
				OI_d_tech *frame;
	
				frame = tmpwp->oi_frame();
				bg = frame->bkg_pixel();
				frame->set_bkg_pixel(frame->fg_pixel());
				XSync(DPY, False);
#if defined(ultrix) || defined(hpux) || defined(SYSV)
				sleep(1);
#else
				usleep(7500);
#endif
				frame->set_bkg_pixel(bg);
				XFlush(DPY);
			}
		}
		break;

	    case F_INFO:
		if (!wmScr->infoBox) 
		{
		    char buff[100];
		    OI_box *bp;
		    OI_static_text *st;
		    OI_glyph *gp;

		    wmScr->infoBox = oi_create_dialog_box(NULL, 100, 100, 1, infoCells);
		    wmScr->infoBox->set_layout(OI_layout_row);
		    st = oi_create_static_text(NULL, "ParcPlace Window Manager");
		    st->set_bottom_space(0);
		    st->set_gravity(OI_grav_center);
		    st->layout_associated_object(wmScr->infoBox, (OI_number)0, 0, OI_ACTIVE);
		    sprintf(buff, "Version %s", SWM_VERSION);
		    st = oi_create_static_text(NULL, buff);
		    st->set_top_space(0);
		    st->set_gravity(OI_grav_center);
		    st->layout_associated_object(wmScr->infoBox, 0, 1, OI_ACTIVE);
		    st = oi_create_static_text(NULL, "Authors");
		    st->set_top_space(10);
		    st->set_gravity(OI_grav_center);
		    st->layout_associated_object(wmScr->infoBox, 0, 4, OI_ACTIVE);
		    bp = oi_create_box(NULL, 1, 1);
		    bp->set_frame_width(0);
		    bp->set_layout(OI_layout_column);
		    bp->layout_associated_object(wmScr->infoBox, 0, 5, OI_ACTIVE);

		    gp = oi_create_glyph(NULL, toml_bits, toml_bits, NULL, OI_PIC_MASK, toml_width, toml_height, 1, OI_NO, OI_NO);
		    gp->set_gravity(OI_grav_center);
		    gp->layout_associated_object(bp, (OI_number)0, 0, OI_ACTIVE);
		    st = oi_create_static_text(NULL, "Tom");
		    st->set_bottom_space(0);
		    st->set_gravity(OI_grav_center);
		    st->layout_associated_object(bp, 0, 1, OI_ACTIVE);
		    st = oi_create_static_text(NULL, "LaStrange");
		    st->set_top_space(0);
		    st->set_gravity(OI_grav_center);
		    st->layout_associated_object(bp, 0, 2, OI_ACTIVE);

		    gp = oi_create_glyph(NULL, kelly_bits, kelly_bits, NULL, OI_PIC_MASK, kelly_width, kelly_height, 1, OI_NO, OI_NO);
		    gp->set_gravity(OI_grav_center);
		    gp->layout_associated_object(bp, 1, 0, OI_ACTIVE);
		    st = oi_create_static_text(NULL, "Kelly");
		    st->set_bottom_space(0);
		    st->set_gravity(OI_grav_center);
		    st->layout_associated_object(bp, 1, 1, OI_ACTIVE);
		    st = oi_create_static_text(NULL, "Rise");
		    st->set_top_space(0);
		    st->set_gravity(OI_grav_center);
		    st->layout_associated_object(bp, 1, 2, OI_ACTIVE);

		    wmScr->infoBox->set_associated_object(wmScr->conp->root(),
			(wmScr->width - wmScr->infoBox->space_x()) / 2,
			(wmScr->height - wmScr->infoBox->space_y()) / 2, OI_NOT_DISPLAYED);
		}
		wmScr->infoBox->wait_button();
		break;
	    case F_FREEZEDESKTOP:
		if (!wmScr->frozen)
		{
		    XSetWindowAttributes watr;
		    watr.override_redirect = True;
		    watr.background_pixmap = None;
		    watr.cursor = None;
		    wmScr->frozen = XCreateWindow(DPY, wmScr->vdt ? wmScr->vroot->X_window() : wmScr->root, 0,0,
		        ~0, ~0, 0, (int)CopyFromParent, InputOutput, (Visual *)CopyFromParent,
			CWOverrideRedirect|CWBackPixmap|CWCursor, &watr);
		}
		XMapRaised(DPY, wmScr->frozen);
		XFlush(DPY);
		break;
	    case F_THAWDESKTOP:
		if (wmScr->frozen)
		    XUnmapWindow(DPY, wmScr->frozen);
		break;
	    case F_SETDESKTOP:
		{
		    Window root = wmScr->vdt ? wmScr->vroot->X_window() : wmScr->root;

		    if (argc)
		    {
			PIXEL fg = wmScr->black;
			PIXEL bg = wmScr->white;
			Pixmap pm = NULL;
			wmGetColor(argv[0], &bg);
			if (argc > 1)
			    wmGetColor(argv[1], &fg);
			if (argc > 2)
			{
			    wmBitmap *wbm;
			    wbm = wmFindBitmap(argv[2]);
			    if (wbm)
			    {
				GC gc;
				XGCValues gcv;
				pm = XCreatePixmap(DPY, wmScr->root, wbm->width, wbm->height, wmScr->depth);
				gcv.foreground = fg;
				gcv.background = bg;
				gc = XCreateGC(DPY, wmScr->root, GCForeground | GCBackground, &gcv);
				XCopyPlane(DPY, wbm->pixmap, pm, gc, 0, 0, wbm->width, wbm->height, 0, 0, 1);
				XFreeGC(DPY, gc);
			    }
			}
			if (pm)
			{
			    XSetWindowBackgroundPixmap(DPY, root, pm);
			    XFreePixmap(DPY, pm);
			}
			else
			    XSetWindowBackground(DPY, root, bg);
		    }
		    else
		    {
			// set the background to the default root weave
			XSetWindowBackgroundPixmap(DPY, root, wmScr->rootWeave);
		    }
		    XClearWindow(DPY, root);
		}
		break;

	    case F_ROUNDUP:
		for (screen = 0; screen < wmNumScreens; screen++)
		{
		    int newX, newY;
		    wmScreen *scr = wmScreens[screen];
		    if (scr == NULL)
			continue;
		    for (wp = (wmData *)scr->windowList.first(); wp != NULL; wp = (wmData *)scr->windowList.next())
		    {
			if (wp->oi_frame()) {
			    newX = (int)wp->oi_frame()->loc_x();
			    newY = (int)wp->oi_frame()->loc_y();
			    if ((wp->oi_frame()->loc_x() + wp->oi_frame()->space_x()) <= 0)
				newX = 0;
			    if ((wp->oi_frame()->loc_y() + wp->oi_frame()->space_y()) <= 0)
				newY = 0;
			    if (newX != wp->oi_frame()->loc_x() || newY != wp->oi_frame()->loc_y()) {
				wp->oi_frame()->set_loc(newX, newY);
				wmSendEvent(wp);
			    }
			}
			if (wp->oi_icon() && wp->icon_window()) {
			    newX = (int)wp->oi_icon()->loc_x();
			    newY = (int)wp->oi_icon()->loc_y();
			    if ((wp->oi_icon()->loc_x() + wp->oi_icon()->space_x()) < 0)
				newX = 0;
			    if ((wp->oi_icon()->loc_y() + wp->oi_icon()->space_y()) < 0)
				newY = 0;
			    if (newX != wp->oi_icon()->loc_x() || newY != wp->oi_icon()->loc_y()) {
				wp->oi_icon()->set_loc(newX, newY);
			    }
			}
		    }
		}
		break;

	    case F_HANDLEDROP:
		if (argc)
		{
		    if (dropAtom == XA_XV_DO_DRAG_LOAD)
		    {
			unsigned long  nitems, bytesafter;
			Atom actual_type;
			int actual_format;
			unsigned char *str;
			char *shellStr;

			XGetWindowProperty(DPY, dropWindow, XA_XV_DO_DRAG_LOAD, 0L, 1000000L, False,
			    XA_STRING, &actual_type, &actual_format, &nitems, &bytesafter, &str);

			shellStr = (char *)malloc(strlen((char *)str) + strlen(argv[0]) + 5);
			sprintf(shellStr, argv[0], str);
			shell(shellStr);
		    }
		}
		break;

	    case F_SWEEP:
		wmSweep(&ev);
		break;

	    case F_UNSWEEP:
		wmUnSweep();
		break;

	    case F_DUMP:
		if (wp->has_client())
		    wmDumpObject(wp->oi_frame());
		break;

	    case F_ANIMATEBUTTON:
		break;

	    case F_TERMINALPLACES:
		terminalPlaces = True;
		/* fall through to f.places */
	    case F_PLACES:
		{
		    static char buf[256];
		    static char remoteDisplay[256];
		    int server;
		    char *ds = DisplayString(DPY);
		    char *colon, *dot1;
		    char *filename = (char *)malloc(strlen(STARTUP)+1);
		    strcpy(filename, STARTUP);

		    colon = strrchr (ds, ':');
		    if (colon) {			/* if host[:]:dpy */
			strcpy (buf, "DISPLAY=");
			strcat (buf, ds);
			colon = strrchr(buf, ':');
			sscanf(&colon[1], "%d", &server);
			dot1 = index (colon, '.');	/* first period after colon */
			if (!dot1) dot1 = colon + strlen (colon);  /* if not there, append */
		    }

		    if (argc)
			filename = argv[0];
		    filename = wmExpandFilename(filename);
		    FILE *fp = fopen(filename, "w");
		    if (fp == NULL)
		    {
			fprintf(stderr, "swm: wmExecute: couldn't open \"%s\"\n", filename);
		    }
		    else
		    {
			char *sp, *freesp;
			char *s1, *s2, *dp1, *dp2, *done;

			fprintf(fp, "#! /bin/sh\n");
			fprintf(fp, "\n");
			fprintf(fp, "#**********************************************************************\n");
			fprintf(fp, "#\n");
			fprintf(fp, "#  X startup script generated by the f.places swm command\n");
			fprintf(fp, "#\n");
			fprintf(fp, "#**********************************************************************\n");
			if (!terminalPlaces) {
				fprintf(fp, "if [ -x $HOME/.xinitrc.local ]\n");
				fprintf(fp, "then\n");
	   			fprintf(fp, "    $HOME/.xinitrc.local\n");
				fprintf(fp, "fi\n");
			}
			else
			{
				fprintf(fp, "if [ -x $HOME/.xsession.local ]\n");
				fprintf(fp, "then\n");
	   			fprintf(fp, "    $HOME/.xsession.local\n");
				fprintf(fp, "fi\n");
			}
			for (screen = 0; screen < wmNumScreens; screen++)
			{
			    wmScreen *scr = wmScreens[screen];
			    if (scr == NULL)
				continue;

			    sprintf(remoteDisplay, "%s:%d.%d", wmDisplay, server, screen);

			    fprintf(fp, "\n");
			    fprintf(fp, "#**********************************************************************\n");
			    fprintf(fp, "#  Screen %d\n", scr->screen);
			    fprintf(fp, "#**********************************************************************\n");
			    sprintf (dot1, ".%d", scr->screen);
			    if (!terminalPlaces) {
				    fprintf(fp, "%s\n", buf);
				    fprintf(fp, "export DISPLAY\n");
			    }
			    fprintf(fp, "\n");
			    for (wp = (wmData *)scr->windowList.first(); wp != NULL; wp = (wmData *)scr->windowList.next())
			    {
				// if (!wp->mine())
				{
				    int x, y, width, height, unitwidth, unitheight;
				    char *state;
				    char *iconGeometry = "-iconGeometry +00000000+00000000";
				    char *sticky = "";
				    char *rootIcon = "";
				    char *iconGravity = "";
				    char *gravityOrder = "-gravityOrder 00000000";
				    char *machine = "";
				    int freeMachine = False;
				    char *clientMachine = NULL;
				    unsigned long  nitems, bytesafter;
				    Atom actual_type;
				    int actual_format;

				    XGetWindowProperty(DPY, wp->window(), XA_WM_CLIENT_MACHINE, 0L, 1000000L, False,
					XA_STRING, &actual_type, &actual_format, &nitems, &bytesafter, (unsigned char **)&clientMachine);

				    // only save machine information if different from the display machine
				    if (!terminalPlaces && clientMachine && strcmp(clientMachine, wmDisplay))
				    {
					machine = (char *)malloc(strlen(clientMachine)+15);
					sprintf(machine, "-machine %s", clientMachine);
					freeMachine = True;
				    }
				    else
				    {
					if (clientMachine)
					    XFree(clientMachine);
					clientMachine = NULL;
				    }

				    if (XGetWindowProperty(DPY, wp->window(), XA_WM_COMMAND, 0L, 1000000L, False,
					XA_STRING, &actual_type, &actual_format, &nitems, &bytesafter, (unsigned char **)&sp) == Success && sp)
				    {
					freesp = sp;
					s1 = dp1 = (char *)malloc((unsigned int)(nitems + 50));
					s2 = dp2 = (char *)malloc((unsigned int)(nitems + 50));
					done = sp + nitems;
					for (; sp < done; sp++)
					{
					    if (*sp)
					    {
						if (*sp == '#' || *sp == '"' || *sp == '\'')
						{
						    *dp1++ = '\\';
						    *dp1++ = *sp;
						}
						else
						    *dp1++ = *sp;
						*dp2++ = *sp;
					    }
					    else
					    {
						*dp1++ = ' ';
						*dp2++ = ' ';
					    }
					}
					if (!clientMachine)
					    *dp1++ = '&';
					*dp1 = '\0';
					*dp2 = '\0';

					// generate the swmhints command
					XGetGeometry(DPY, wp->window(), &junkRoot, &x, &y, (unsigned int *)&width, (unsigned int *)&height, &junkBW, &junkDepth);
					if (wp->panner())
					{
					    unitwidth = wmScr->vwidth;
					    unitheight = wmScr->vheight;
					}
					else
					    wmUnitSize(wp, width, height, &unitwidth, &unitheight);
					if (wp->state() == NormalState)
					    state = "-state NormalState";
					else
					    state = "-state IconicState";

					if (wp->sticky())
					    sticky = "-sticky";

					if (wp->iconified() || wp->icon_gravity()) {
						sprintf(iconGeometry, "-iconGeometry %c%d%c%d", 
							wp->icon_x() < 0 ? '-' : '+' , abs(wp->icon_x()),
							wp->icon_y() < 0 ? '-' : '+' , abs(wp->icon_y()));
					}
					else
						iconGeometry[0] = '\0';

					if (wp->icon_window() == scr->root)
					    rootIcon = "-rootIcon";
					if (wp->icon_gravity())
					{
					    iconGravity = "-iconGravity";
					    sprintf(gravityOrder, "-gravityOrder %d", wp->gravity_order());
					}
					else
					{
						iconGravity[0] = '\0';
						gravityOrder[0] = '\0';
					}

					if (wp->oi_frame())
					{
					    fprintf(fp, "swmhints -geometry %dx%d%c%d%c%d %s %s %s %s %s %s %s -cmd \"%s\"\n",
						unitwidth, unitheight,
						wp->oi_frame()->loc_x() < 0 ? '-' : '+', abs((int)wp->oi_frame()->loc_x()),
						wp->oi_frame()->loc_y() < 0 ? '-' : '+', abs((int)wp->oi_frame()->loc_y()),
						iconGeometry, state, sticky, rootIcon, machine, iconGravity, gravityOrder, s2);
					}

					if (freeMachine)
					    free(machine);

					if (!wp->mine())
					{
					    if (clientMachine)
					    {
						fprintf(fp, wmRemoteExecution, clientMachine, remoteDisplay, s1);
						fprintf(fp, "\n\n");
					    }
					    else
						fprintf(fp, "%s\n\n", s1);
					}

					free(s1);
					free(s2);
					XFree(freesp);
				    }
				    if (clientMachine)
					XFree(clientMachine);
				}
			    }
			}
			fprintf(fp, "\n");
			fprintf(fp, "#**********************************************************************\n");
			fprintf(fp, "#  Start the window manager that makes this all possible\n");
			fprintf(fp, "#**********************************************************************\n");

			sp = (char *)malloc(256);
			freesp = sp;
			sp[0] = '\0';
			for (int i = 0; i < wmArgc; i++) {
			    strcat(sp, wmArgv[i]);
			    strcat(sp, " ");
			}
			s1 = dp1 = (char *)malloc(512);
			for (; *sp != '\0'; sp++)
			{
			    if (*sp == '#' || *sp == '"' || *sp == '\'')
			    {
				*dp1++ = '\\';
				*dp1++ = *sp;
			    }
			    else
				*dp1++ = *sp;
			}
			*dp1 = '\0';

			if (!terminalPlaces && strcmp(wmDisplay, wmHost))
			    fprintf(fp, wmRemoteExecution, wmHost, remoteDisplay, s1);
			else
			    fprintf(fp, "%s", s1);

			free(freesp);
			free(s1);

			fprintf(fp, "\n");
			fclose(fp);
			chmod(filename, 0755);
		    }
		}
		terminalPlaces = False;
		break;

	    case F_QUERY:
		success = False;
		if (argc)
		{
		    if (!strcmp("sticky", argv[0]) && wp->sticky())
			success = True;
		    else if (!strcmp("zoomed", argv[0]) && wp->zoomed())
			success = True;
		    else if (!strcmp("vertzoomed", argv[0]) && wp->vert_zoomed())
			success = True;
		    else if (!strcmp("horizoomed", argv[0]) && wp->hori_zoomed())
			success = True;
		    else if (!strcmp("iconic", argv[0]) && wp->state() == IconicState)
			success = True;
		}
		break;
	    case F_PIN:
		wmSet_OL_PIN_STATE(wp, 1);
		break;

	    case F_UNPIN:
		// if it's mine, I have to unmap it before setting the property because the
		// OI unpin code will discard all events on the window.  I need to get an
		// UnmapNotify event on this window so I can take down the decoration.
		if (wp->mine())
			XUnmapWindow(DPY, wp->window());
		wmSet_OL_PIN_STATE(wp, 0);
		if (!wp->mine())
		{
		    if (wp->protocol_bits() & wmDeleteWindow) 
		    {
			XEvent e;
			XClientMessageEvent *ce;

			ce = (XClientMessageEvent *)&e;
			ce->type = ClientMessage;
			ce->window = wp->window();
			ce->message_type = WM_PROTOCOLS;
			ce->format = 32;
			ce->data.l[0] = WM_DELETE_WINDOW;
			ce->data.l[1] = 0;	// +++ need a timestamp
			XSendEvent(DPY, wp->window(), False, 0, &e);
		    }
		    else {
			XUnmapWindow(DPY, wp->window());
		    }
		}
		break;

	    case F_RECONFIG:
		wmReConfig();
		break;

	    case F_GETRESOURCE:
		if (argc)
		{
		    char *fn;
		    fn = wmExpandFilename(argv[0]);
		    if (wmScr->conp->add_resources(fn) == OI_NO)
		    {
			// that didn't work, lets put the SWM_DEFAULTS
			// path in front of it
			char *t = (char *)malloc(strlen(SWM_DEFAULTS) + strlen(fn) + 2);
			strcpy(t, SWM_DEFAULTS);
			strcat(t, fn);
			if (wmScr->conp->add_resources(t) == OI_NO)
			{
			    fprintf(stderr, 
				"swm: wmInitialize: couldn't find resource file \"%s\"\n", fn);
			}
			free(t);
		    }
		}
		break;

	    case F_STICK:
		if (wmScr->vdt)
		{
		    // first check to see if the thing is already on the root 
		    success = False;
		    if (wp->root() == wmScr->vroot)
		    {
			wmData *saveFocus = wmFocusWp;

			// stick the dude
			if (stickIt(wp->oi_frame()))
			{
			    success = True;
			    // any reason not to set these here? sticky()
			    // is looked at during stickIcon
			    wp->set_root(wmScr->conp->abs_root());
			    wp->unmap_vbox();
			    wp->unmap_vibox();
			    wp->set_sticky();
			    wmSet__SWM_HINTS(wp);
			    wmSet__SWM_ROOT(wp);
			    // now do the icon, if there is one
			    if (wp->oi_icon() && !wp->ip())
			    {
				stickIt(wp->oi_icon());
				if (wp->icon_gravity()) {
				    IconRegionContents *irp = (IconRegionContents *)wp->get_irp();
				    if (irp)
				        irp->stickIcon(wp);
				}
			    }

			    // send a synthetic ConfigureNotify so the client will know
			    // that he moved
			    wmSendEvent(wp);
			    if (wmFocusModel == wmFocusModelClickToType && wp == saveFocus)
			    {
				wmFocusWp = NULL;
				wmSetFocus(NULL, wp);
			    }
			}
		    }
		}
		break;
	    case F_UNSTICK:
		if (wmScr->vdt)
		{
		    // first check to see if the thing is already on the vroot 
		    if (wp->root() == wmScr->vroot)
			success = False;
		    else
		    {
			// unstick the dude

			int x, y;
			wmData *saveFocus = wmFocusWp;

			if (wp->oi_frame())
			{
			    // find coordinates relative to the vroot
			    XTranslateCoordinates(DPY, wmScr->root, 
				wmScr->vroot->X_window(),
				(int)wp->oi_frame()->loc_x(),
				(int)wp->oi_frame()->loc_y(), &x, &y, &junkChild);

			    wp->oi_frame()->set_associated_object(wmScr->vroot, x, y);
			    wp->move(x, y);
			    wp->raise();
			}

			// now do the icon, if there is one
			if (wp->oi_icon())
			{
			    // if it is in an icon panel don't touch it
			    if (!wp->ip())
			    {
				XTranslateCoordinates(DPY,
				    wmScr->root, wmScr->vroot->X_window(),
				    (int)wp->oi_icon()->loc_x(),
				    (int)wp->oi_icon()->loc_y(), &x, &y, &junkChild);
				wp->oi_icon()->set_associated_object(wmScr->vroot, x, y);
				wp->move_icon(x, y);
				wp->raise_icon();
			    }
			}
			wp->clear_sticky();
			wp->set_root(wmScr->vroot);
			if (wp->state() == NormalState)
			    wp->map_vbox();
			else if (!wp->icon_gravity())
			    wp->map_vibox();

			if (wp->icon_gravity()) 
			{
			    IconRegionContents *irp = 
				(IconRegionContents *)wp->get_irp();
			    if (irp)
			    	irp->unstickIcon(wp);
			}
			wmSet__SWM_HINTS(wp);
			wmSet__SWM_ROOT(wp);
			// send a synthetic ConfigureNotify so the client will know
			// that he moved
			wmSendEvent(wp);
			if (wmFocusModel == wmFocusModelClickToType && wp == saveFocus)
			{
			    wmFocusWp = NULL;
			    wmSetFocus(NULL, wp);
			}
		    }
		}
		break;

	    case F_SCROLLSAVE:
		if (wmScr->vdt)
		{
		    // find out the current handle location
		    int x, y;
		    XTranslateCoordinates(DPY, wmScr->root, wmScr->vroot->X_window(), 0,0, &x, &y, &junkChild);
		    wmScr->vdt->set_handle_loc(wmScr->saveVrootX, wmScr->saveVrootY);
		    wmScr->saveVrootX = x;
		    wmScr->saveVrootY = y;
		}
		break;
	    case F_SAVEVROOT:
		if (wmScr->vdt)
		{
		    // find out the current handle location
		    XTranslateCoordinates(DPY, wmScr->root,
			wmScr->vroot->X_window(), 0,0, &wmScr->saveVrootX, &wmScr->saveVrootY, &junkChild);
		}
		break;
	    case F_RESTOREVROOT:
		if (wmScr->vdt)
		{
		    wmScr->vdt->set_handle_loc(wmScr->saveVrootX, wmScr->saveVrootY);
		}
		break;

	    case F_SCROLLTO:
		if (wmScr->vdt && !wp->sticky())
		{
		    if (wp->oi_frame())
		    {
			int x = wp->oi_frame()->loc_x() ? (int)wp->oi_frame()->loc_x() : 0;
			int y = wp->oi_frame()->loc_y() ? (int)wp->oi_frame()->loc_y() : 0;
			wmScr->vdt->set_handle_loc(x, y);
		    }
		}
		break;

	    case F_SCROLLLEFT:
	    case F_SCROLLRIGHT:
	    case F_SCROLLUP:
	    case F_SCROLLDOWN:
		if (wmScr->vdt)
		{
		    int x, y;
		    int count = 1;

		    if (argc)
			count = atoi(argv[0]);

		    // find out the current handle location
		    XTranslateCoordinates(DPY, wmScr->root,
			wmScr->vroot->X_window(), 0,0, &x, &y, &junkChild);

		    switch (what)
		    {
			case F_SCROLLLEFT: x -= count * wmScr->width; break;
			case F_SCROLLRIGHT: x += count * wmScr->width; break;
			case F_SCROLLUP: y -= count * wmScr->height; break;
			case F_SCROLLDOWN: y += count * wmScr->height; break;
		    }
		    if (x < 0) x = 0;
		    if (x > wmScr->vwidth) x = wmScr->vwidth;
		    if (y < 0) y = 0;
		    if (y > wmScr->vheight) y = wmScr->vheight;
		    wmScr->vdt->set_handle_loc(x, y);
		}
		break;
	    case F_SCROLLHOME:
		if (wmScr->vdt)
		{
		    wmScr->vdt->set_handle_loc(0,0);
		}
		break;

	    case F_SCROLL:
		if (wmScr->vdt)
		{
		    // if there is a string, then the geometry is screenX screenY
		    if (argc)
		    {
			int x, y; 
			unsigned int width, height;
			x = 0;
			y = 0;
			unsigned mask = XParseGeometry(argv[0], &x, &y, &width, &height);
			x *= wmScr->width;
			y *= wmScr->height;
			if (x < 0) x = 0;
			if (x > wmScr->vwidth) x = wmScr->vwidth;
			if (y < 0) y = 0;
			if (y > wmScr->vheight) y = wmScr->vheight;
			wmScr->vdt->set_handle_loc(x, y);
		    }
		    else
			// just scroll home
			wmScr->vdt->set_handle_loc(0,0);
		}
		break;

	    case F_PANNER:
		if (wmScr->vdt)
		{
		    wmData *wp;
		    FIND_CONTEXT(wmScr->pan->X_window());
		    if (wp)
		    {
			if (wp->oi_frame()->state() == OI_ACTIVE)
			    success = False;
			else
			{
			    if (wp->state() == IconicState)
				wmDeiconify(wp);
			    else
				wp->map();
			}
		    }
		    else
		    {
			wmData *wp = wmReparent(wmScr->pan->X_window(), wmScr->root, wmScr->conp->abs_root());
			wmScr->pan->set_state(OI_ACTIVE);
			XRaiseWindow(DPY, wp->oi_frame()->outside_X_window());
			wp->map();
			wp->set_panner();
		    }
		}
		break;

	    case F_ZOOM:
	    case F_HORIZOOM:
	    case F_VERTZOOM:
		success = wmZoom(what, wp, &junkX, &junkY);
		break;

	    case F_SAVE:
		wmSave(wp);
		break;

	    case F_RESTORE:
		wmRestore(wp);
		break;

	    case F_SQUEEZE:
		if (wp->myip())
		{
		    char saveSqueeze = wp->myip()->squeeze;
		    wp->myip()->squeeze = True;
		    if (wmNewIconPanelSizes(wp->myip()))
		    {
			wmLayoutIconPanel(wp->myip());
			wmSizeIconPanel(wp->myip());
			wmUpdateSize(wp->myip()->wp);
		    }
		    wp->myip()->squeeze = saveSqueeze;
		}
		else
		    success = False;
		break;

	    case F_PACK:
		if (wp->myip())
		{
		    wmPackIconPanel(wp->myip());
		    wmLayoutIconPanel(wp->myip());
		    if (wp->myip()->rows < wp->myip()->view_rows)
			wp->myip()->view_rows = wp->myip()->rows;
		    wmSizeIconPanel(wp->myip());
		    wmUpdateSize(wp->myip()->wp);
		}
		else
		    success = False;
		break;

	    case F_ICONIFY:
		success = wmIconify(wp, x_root, y_root);
		if (success && wp->oi_icon()) 
		{
		    firstWindow = wp->oi_icon()->X_window();
		}
		break;
	    case F_DEICONIFY:
		success = wmDeiconify(wp);
		if (success)
		{
		    firstWindow = wp->oi_frame()->X_window();
		}
		break;
	    case F_RAISE:
		if (window == wp->oi_frame()->X_window())
		    wp->raise();
		else
		    wp->raise_icon();
		break;
	    case F_LOWER:
		wmLowerWindow(wp, window);
		break;
	    case F_RAISELOWER:
		if (wp->panner())
		    break;
		if (wp->oi_frame() && window == wp->oi_frame()->X_window())
		    vis = wp->frame_vis();
		else
		    vis = wp->icon_vis();

		if (vis == VisibilityUnobscured)
		    wmLowerWindow(wp, window);
		else
		{
		    int myX, myY;
		    unsigned int width, height, myWidth, myHeight, myBW;
		    int x, y;
		    Window *children;
		    unsigned int nchildren;
		    int i;
		    int bw;

		    /*
		     * If the window is above all of its siblings, but partially
		     * offscreen, its visibility is VisibilityPartiallyObscured,
		     * but we'd want to lower it.
		     */
		    XGetGeometry(DPY, window, &junkRoot, &myX, &myY, &myWidth, &myHeight, &myBW, &junkDepth);
		    if (wmScr->vdt)
		    {
			XTranslateCoordinates(DPY, wp->root()->X_window(), wmScr->root,
			    myX, myY, &x, &y, &junkChild);
			myX = x;
			myY = y;
		    }
		    bw = 2*myBW;

		    /* If it's completely onscreen, it must be obscured. */
		    if (myX > 0 && (myX + myWidth+bw) <= wmScr->width &&
			myY > 0 && (myY + myHeight+bw) <= wmScr->height)
		    {
			XRaiseWindow(DPY, window);
		    }
		    else
		    {
			XQueryTree(DPY, wp->root()->X_window(), &junkRoot, &junkParent, &children, &nchildren);
			/*
			 * Start at the upper-most window and work down.  Look
			 * for an obscuring sibling above w.
			 */
			for (i = nchildren - 1; i >= 0; i--)
			{
			    if (children[i] == window)
			    {
				wmLowerWindow(wp, window);
				break;
			    }
			    else
			    {
				XGetGeometry(DPY, children[i], &junkRoot, &x, &y, &width, &height, &junkBW, &junkDepth);
				if (x + width >= myX && x < myX + myWidth &&
				    y + height >= myY && y < myY + myHeight)
				{
				    XRaiseWindow(DPY, window);
				    break;
				}
			    }
			} /* for */
			XFree((char *)children);
		    }
		}
		break;
	    case F_MOVEOPAQUE:
		wmMoveOpaque = True;
		// fall through
	    case F_FORCEMOVE:
		if (what == F_FORCEMOVE)
		    wp->clear_constrain();
		// fall through
	    case F_CONSTRAINMOVE:
		if (what == F_CONSTRAINMOVE)
		    wp->set_constrain();
		// fall through
	    case F_MOVE:
		if (wp->oi_frame() && window == wp->oi_frame()->X_window())
		    success = wmMoveClient(&ev, wp);
		else
		    success = wmMoveIcon(&ev, wp);
		wmMoveOpaque = False;
		if (wp->save_constrain())
		    wp->set_constrain();
		else
		    wp->clear_constrain();
		break;
	    case F_AUTORESIZE:
		wmAutoResize = True;
		// fall through
	    case F_RESIZE:
		if (wp->oi_frame() && window == wp->oi_frame()->X_window())
		    success = wmStartResize(&ev, wp);
		else
		    success = False;
		wmAutoResize = False;
		wmResizeFlags = 0;
		break;
	    case F_HARDRESTART:
		wmHardRestart = True;
		// fall through
	    case F_RESTART:
		wmRestart();
		break;
	    case F_QUIT:
		wmDone();
		break;
	    case F_GROUP:
		if (!window_group)
		    window_group = (int)wp->window();
		wp->set_group(window_group);
		break;
	    case F_UNGROUP:
		if (wp->group() == NULL)
		    break;
		window_group = wp->group();
		wp->set_group(NULL);
#ifdef NOT_USED
		// if the window was the leader, disband the group
		wmData *tmp_wp;
		for (tmp_wp = (wmData *)wmScr->windowList.first();
		    tmp_wp != NULL; tmp_wp = (wmData *)wmScr->windowList.next())
		{
		    if (wp->group == window_group)
			wp->group = NULL;
		}
#endif /* NOT_USED */
		break;
	    case F_REGROUP:
		wp->set_group(wp->regroup());
		break;
	    case F_BEEP:
		XBell(DPY, 0);
		break;
	    case F_CIRCLEUP:
		XCirculateSubwindowsUp(DPY, wmScr->vroot->X_window());
		break;
	    case F_CIRCLEDOWN:
		XCirculateSubwindowsDown(DPY, wmScr->vroot->X_window());
		break;
	    case F_EXEC:
		if (argc)
		    shell(argv[0]);
		break;
	    case F_FOCUS:
		wmSetFocus(NULL, wp);
		break;
	    case F_IDENTIFY:
		break;
	    case F_REFRESH:
		w = XCreateSimpleWindow(DPY, wmScr->root,
		    0, 0, 9999, 9999, 0, wmScr->black, wmScr->black);
		XMapWindow(DPY, w);
		XDestroyWindow(DPY, w);
		XFlush(DPY);
		break;
	    case F_UNFOCUS:
		wmClearFocus(NULL, wmFocusWp);
		XSetInputFocus(DPY, PointerRoot, RevertToPointerRoot, CurrentTime);
		break;
	    case F_WINREFRESH:
		w = XCreateSimpleWindow(DPY, window, 0, 0, 9999, 9999, 0, wmScr->black, wmScr->black);
		XMapWindow(DPY, w);
		XDestroyWindow(DPY, w);
		XFlush(DPY);
		break;
	    case F_WARPTO:
		if (wp)
		{
		    int x, y;
		    unsigned int width, height;

		    XGetGeometry(DPY, firstWindow, &junkRoot, &x, &y, &width, &height, &junkBW, &junkDepth);
		    ev.x_root = x + (width/2);
		    ev.y_root = y + (height/2);
		    XWarpPointer(DPY, None, wmScr->root, 0,0,0,0, ev.x_root, ev.y_root);
		}
		break;
	    case F_WARPTOSCREEN:
	    {
		whichScreen = -1;
		if (argc)
		{
		    whichScreen = atoi(argv[0]);
		    if (whichScreen < 0 || whichScreen >= wmNumScreens)
			whichScreen = -1;
		}
		if (whichScreen == -1)
		{
		    whichScreen = wmScr->screen;
		    if (++whichScreen >= wmNumScreens)
			whichScreen = 0;
		}
		wmScreen *w = wmScreens[whichScreen];
		if (w) {
			XWarpPointer(DPY, None, wmScr->root, 0,0,0,0, 5000, 5000);
			XWarpPointer(DPY, None, w->root, 0,0,0,0,
			    w->savePointerX, w->savePointerY);
			wmScr = w;
			wmScr->conp->make_default();
		}
		break;
	    }
	    case F_WARPVERTICAL:
		if (argc)
		    delta = atoi(argv[0]);
		else
		    delta = 20;
		XWarpPointer(DPY, wmScr->root, None, 0,0,0,0, 0, delta);
		break;
	    case F_WARPHORIZONTAL:
		if (argc)
		    delta = atoi(argv[0]);
		else
		    delta = 20;
		XWarpPointer(DPY, wmScr->root, None, 0,0,0,0, delta, 0);
		break;
	    case F_WARPSAVE:
		XQueryPointer(DPY, wmScr->root, &junkRoot, &junkChild,
		    &x_root, &y_root, &junkX, &junkY, &junkMask);
		XWarpPointer(DPY, None, wmScr->root, 0,0,0,0, wmScr->savePointerX, wmScr->savePointerY);
		wmScr->savePointerX = x_root;
		wmScr->savePointerY = y_root;
		break;
	    case F_SAVEPOINTER:
		XQueryPointer(DPY, wmScr->root, &junkRoot, &junkChild,
		    &wmScr->savePointerX, &wmScr->savePointerY,
		    &junkX, &junkY, &junkMask);
		break;
	    case F_RESTOREPOINTER:
		XWarpPointer(DPY, None, wmScr->root, 0,0,0,0,
		    wmScr->savePointerX, wmScr->savePointerY);
		break;

	    case F_MENU:
		break;

	    case F_DELETE:
		if (wp->protocol_bits() & wmDeleteWindow) 
		{
		    a = WM_DELETE_WINDOW;
		}
		// fall through to F_SHUTDOWN
	    case F_SHUTDOWN:
		if (a == None)
		{
		    if (wp->protocol_bits() & wmSaveYourself)
			a = WM_SAVE_YOURSELF;
		    if (wp->protocol_bits() & wmShutdown)
			a = WM_SHUTDOWN;
		}
		if (a != None && !wp->mine())
		{
		    XEvent e;
		    XClientMessageEvent *ce;

		    ce = (XClientMessageEvent *)&e;
		    ce->type = ClientMessage;
		    ce->window = wp->window();
		    ce->message_type = WM_PROTOCOLS;
		    ce->format = 32;
		    ce->data.l[0] = a;
		    ce->data.l[1] = 0;	// +++ need a timestamp
		    XSendEvent(DPY, wp->window(), False, 0, &e);
		    if (a != WM_DELETE_WINDOW);
			wp->set_kill_sent();
		    break;
		}
		// fall through to F_KILL
	    case F_KILL:
	    case F_DESTROY:
		if (wp->panner())
		{
		    wp->unmap();
		    if (wp->oi_icon())
			wp->unmap_icon();
		}
		else if (wp->mine())
		    XBell(DPY, 0);
		else
		    XKillClient(DPY, wp->window());
		break;

	    case F_REBIND:
		// fall through
	    case F_NEWBUTTONIMAGE:
		if (argc < 2)
		    break;
		// fall through
	    case F_MAP:
	    case F_UNMAP:
		if (argc)
		{
		    int len = strlen(argv[0]);

		    // check objects in the frame
		    OI_d_tech *oi = NULL;
		    OI_d_tech *parent = NULL;

		    if (what == F_REBIND)
			if (wp && wp->oi_frame() && !strncmp(wp->oi_frame()->name(), argv[0], len))
			    oi = wp->oi_frame();

		    if (!oi && wp && wp->oi_frame())
		    {
			oi = wp->oi_frame()->descendant(argv[0]);
			// if we have an object and it was a descendant of a frame, then it might be an
			// icon that was in an icon panel.  Discard it if that is the case.
			if (oi)
			{
			    wmObjectData *odp;
			    odp = (wmObjectData *)oi->data();
			    if (odp && odp->inAnIcon)
				oi = NULL;
			}
		    }

		    if (oi)
			parent = wp->oi_frame();

		    // check objects in the icon
		    if (!oi && wp && wp->oi_icon())
		    {
			if (what == F_REBIND)
			    if (!strncmp(wp->oi_icon()->name(), argv[0], len))
				oi = wp->oi_icon();

			if (!oi)
			    oi = wp->oi_icon()->descendant(argv[0]);

			if (oi)
			    parent = wp->oi_icon();
		    }

		    // If we STILL don't have an object, check root objects for a button.
		    // By definition, the root objects list will contain root panels
		    // first and then root icons
		    if (!oi)
		    {
			for (oi = (OI_d_tech *)wmScr->rootObjectsList.first(); oi != NULL; oi = (OI_d_tech *)wmScr->rootObjectsList.next())
			{
			    parent = oi;
			    oi = oi->descendant(argv[0]);
			    // if we have an object and it was a descendant of a frame, then it might be an
			    // icon that was in an icon panel.  Discard it if that is the case.
			    if (oi)
			    {
				wmObjectData *odp;
				odp = (wmObjectData *)oi->data();
				if (odp && odp->inAnIcon)
				    oi = NULL;
				else
				    break;
			    }
			    parent = NULL;
			}
		    }

		    if (oi)
		    {
			if (what == F_MAP)
			{
			    oi->set_state(OI_ACTIVE);
			    XRaiseWindow(DPY, oi->X_window());
			}
			else if (what == F_UNMAP)
			{
			    oi->set_state(OI_ACTIVE_NOT_DISPLAYED);
			}
			else if (what == F_NEWBUTTONIMAGE)
			{
			    int width = oi->space_x();
			    int height = oi->space_y();

			    if (oi->is_derived_from("OI_glyph"))
			    {
				OI_glyph *gp = (OI_glyph *)oi;
				wmBitmap *wbm;
#ifdef OLD

				wbm = wmFindBitmap(argv[1]);
				if (wbm)
				    gp->set_pixmap(wbm->pixmap, wbm->width, wbm->height, 1, OI_NO);
#endif /* OLD */
				wmButtonInfo *bi;
				bi = wmStringToButtonInfo((char*)oi->name(), argv[1]);
				if (bi && bi->pic_spec)
					gp->set_pixel_data(bi->count, bi->pic_spec, bi->width, bi->height, OI_NO, OI_NO);
#ifdef SHAPE
				wmObjectData *odp;
				odp = (wmObjectData *)oi->data();
				odp->shapePixmap = None;
				if (argc == 3)
				{
				    wbm = wmFindBitmap(argv[2]);
				    if (wbm)
					odp->shapePixmap = wbm->pixmap;
				}
#endif /* SHAPE */
			    }
			    else if (oi->is_derived_from("OI_static_text"))
			    {
				OI_static_text *stp = (OI_static_text *)oi;
				stp->set_text(argv[1]);
			    }
			    if (width != oi->space_x() || height != oi->space_y())
			    {
				if (parent)
				{
				    wmObjectData *odp = (wmObjectData *)parent->data();
				    odp->op->oi = parent;
				    wmLayoutPanel(odp->op);
#ifdef THIS_IS_A_KNOWN_PROBLEM
				    // if this is a root panel, we need to resize its decoration
				    if (odp->op->u.p->root == True)
				    {
					XSizeHints *sh = wp->size_hints_p();
					sh->min_width = odp->op->oi->space_x();
					sh->min_height = odp->op->oi->space_y();
					sh->max_width = odp->op->oi->space_x();
					sh->max_height = odp->op->oi->space_y();
					wmResizeClient(odp->decoration,sh->min_width,sh->min_height,False);
				    }
#endif /* THIS_IS_A_KNOWN_PROBLEM */
				}
			    }
#ifdef SHAPE
			    // always re-layout the panel if we are shaping the window
			    else
			    {
				if (parent)
				{
				    wmObjectData *odp = (wmObjectData *)parent->data();
				    odp->op->oi = parent;
				    wmLayoutPanel(odp->op);
#ifdef THIS_IS_A_KNOWN_PROBLEM
				    // if this is a root panel, we need to resize its decoration
				    if (odp->op->u.p->root == True)
				    {
					XSizeHints *sh = wp->size_hints_p();
					sh->min_width = odp->op->oi->space_x();
					sh->min_height = odp->op->oi->space_y();
					sh->max_width = odp->op->oi->space_x();
					sh->max_height = odp->op->oi->space_y();
					wmResizeClient(odp->decoration,sh->min_width,sh->min_height,False);
				    }
#endif /* THIS_IS_A_KNOWN_PROBLEM */
				}
			    }
#endif /* SHAPE */
			}
			else if (what == F_REBIND)
			{
			    char *ptr;
			    RM->pushq(wmQuarks->bindingsName(), wmQuarks->bindingsClass());
			    ptr = RM->get_resource(argv[1], argv[1]);
			    RM->pop();

			    if (!ptr)
			    {
				fprintf(stderr, "swm: wmExecute: \"%d\" bindings not found\n", argv[1]);
			    }
			    else
			    {
				wmBindings *bps = parseBindings(ptr);
				if (bps)
				{
				    wmData *savewmClient = wmClient;
				    wmClient = wp;

				    wmObjectData *odp = (wmObjectData *)oi->data();
				    if (odp->hs) {
					free((char *)odp->hs);
					odp->hs = NULL;
				    }
				    unbindObject(oi, odp->bps);
				    bindObject(oi, bps);
				    bps->helpConstructed = True;

				    if (wp && oi == wp->oi_frame() && (wp->resize_corners() || wp->resize_bars()) && !odp->handlePress) 
				    {
					odp->handlePress = True;
					OI_dispatch_insert(wp->oi_frame()->X_window(), ButtonPress, ButtonPressMask,
					    (OI_event_fnp)wmHandlePress, (char *)wp->oi_frame());
				    }
				    wmClient = savewmClient;
				}
			    }
			}
		    }
		}
		break;
	    case F_MACRO:
		if (argc)
		{
		    wmMacro *mp;
		    // check to see if we already have this macro
		    for (mp = (wmMacro *)macroList.first(); mp != NULL;
			mp = (wmMacro *)macroList.next())
		    {
			if (!strcmp(argv[0], mp->name))
			    break;
		    }
		    if (!mp)
		    {
			mp = new wmMacro;
			mp->bp = wmBind = new wmBinding();
			mp->name = argv[0];
			char *ptr;
			RM->pushq(wmQuarks->macroName(), wmQuarks->macroClass());
			ptr = RM->get_resource(argv[0], argv[0]);
			RM->pop();

			if (!ptr)
			{
			    fprintf(stderr,
			    "swm: wmExecute: could not find macro \"%s\"\n", argv[0]);
			    delete mp;
			    mp = NULL;
			}
			else
			{
			    wmParse("wmFunction", ptr);
			    if (wmParseError)
			    {
				fprintf(stderr,
				"swm: wmExecute: syntax error on \"%s\"\n",
				    ptr);
				delete mp;
				mp = NULL;
			    }
			}
			if (mp)
			    macroList.append((ent)mp);
		    }
		    if (mp)
		    {
			wmExecuteBinding(wp, window, mp->bp, NOT_FROM_MENU,
			    x_root, y_root);
		    }
		}
		break;
	    case F_STRING:
		if (argc)
		{
		    XKeyPressedEvent ev;
		    char c[2];

		    XQueryPointer(DPY, wp->window(), &junkRoot, &junkChild,
			&ev.x_root, &ev.y_root, &ev.x, &ev.y, &ev.state);
		    ev.type = KeyPress;
		    ev.display = DPY;
		    ev.window = wp->window();
		    ev.root = wmScr->root;
		    ev.subwindow = None;
		    ev.same_screen = True;

		    wmExpandString(argv[0]);
		    c[1] = '\0';
		    while ((c[0] = *argv[0]++) != '\0')
		    {
			char *string = c;
			if (c[0] < ' ')
			{
			    switch (c[0])
			    {
				case 0x08 : string = "BackSpace";	break;
				case 0x09 : string = "Tab";		break;
				case 0x0A : string = "Linefeed";	break;
				case 0x0B : string = "Clear";		break;
				case 0x0D : string = "Return";		break;
				case 0x13 : string = "Pause";		break;
				case 0x1B : string = "Escape";		break;
			    }
			}
			KeySym keysym = XStringToKeysym(string);
			if (keysym != NoSymbol)
			{
			    ev.keycode = XKeysymToKeycode(DPY, keysym);
			    XSendEvent(DPY, wp->window(), False, KeyPressMask,
				(XEvent *)&ev);
			}
			fprintf(stderr, "\"%s\", %d\n", string, keysym);
		    }
		}
		break;
	    case F_NOP:
		break;
	    case F_TITLE:
		break;
	    case F_STOP:
		if (wp)
		{
		    if (wp->last_status() == True)
			wp->set_stop();
		}
		else if (lastStatus == True)
		{
		    stopExecution = True;
		}
		break;
	    case F_FALSESTOP:
		if (wp)
		{
		    if (wp->last_status() == False)
			wp->set_stop();
		}
		else if (lastStatus == False)
		{
		    stopExecution = True;
		}
		break;
	    case F_GRAVITY:
		// no icon yet? just set gravity so it will get a region 
		// when iconified
		if (!wp->oi_icon()) {
		    wp->set_icon_gravity();
		    wp->clear_root_icon();
		    wmSet__SWM_HINTS(wp);
		}
		else if (!wp->icon_gravity()) {
		    RM->set_stack_ptr(wmScr->rm_stack);
		    RM->push(wp->wclass_class(), wp->wclass_class());
		    RM->push(wp->wclass_name(), wp->wclass_name());
		    if (wp->sticky())
			RM->pushq(wmQuarks->stickyName(), wmQuarks->stickyClass());
    		    wp->set_icon_region(RM->get_resourceq(wmQuarks->iconRegionName(), wmQuarks->iconRegionClass()));
		    IconRegion *irp = NULL;
		    if (wp->icon_region())
    	    		irp = findIconRegion(wp->icon_region());
		    if (!irp)
			if (wp->sticky())
			    irp = findIconRegion("sticky");
		    if (!irp)
    	    		irp = findIconRegion(wp->wclass_name());
		    if (!irp)
    	    		irp = findIconRegion(wp->wclass_class());
		    if (!irp)
    	    		irp = findIconRegion("Default");
		    if (irp)
			irp->addIcon(wp);
		}
		break;
	    case F_RESHUFFLE:
		if (wp->icon_gravity()) {
		    IconRegionContents *irp = (IconRegionContents*)wp->get_irp();
			if (irp)
			    irp->reshuffle();
		}
		else
		    success = False;
		break;

	}
	if (wp)
	{
	    wp->set_last_status(success);
	    // I HATE this special case.  The OpenLook+ template has the following binding for the window
	    // frame   "f.move f.stop f.raise".  Without this special case, if a window in a window group is
	    // moved it will not raise but all other group members will raise.  So if the command executed was
	    // a non-group command and this thing is a group member, set the status of all group members accordingly.
	    if (wp->group() && !groupCommand(what))
	    {
		for (tmp_wp = (wmData *)wmScr->windowList.first(); tmp_wp != NULL;
			tmp_wp = (wmData *)wmScr->windowList.next())
		    tmp_wp->set_last_status(success);
	    }
	}
	wp = NULL;
    }
}

/***********************************************************************
 *
 *  Procedure:
 *      wmLowerWindow
 *
 *  Function:
 *	Lower a window and don't let it go behind the virtual desktop
 *
 ***********************************************************************
 */

void
wmLowerWindow(wmData *wp, Window window)
{
    if (wmScr->vdt && (wp->root()->X_window() == wmScr->root))
    {
	XWindowChanges config;
	config.sibling = wmScr->vdt->outside_X_window();
	config.stack_mode = Above;
	XConfigureWindow(DPY, window, CWSibling|CWStackMode, &config);
    }
    else {
	if (window == wp->oi_frame()->X_window()) {
		XLowerWindow(DPY, wp->oi_frame()->outside_X_window());
		wp->lower_vbox();
	}
	else {
		XLowerWindow(DPY, wp->oi_icon()->outside_X_window());
		wp->lower_vibox();
	}
    }
}

/***********************************************************************
 *
 *  Procedure:
 *      stickIt
 *
 *  Function:
 *	Stick a window
 *
 ***********************************************************************
 */

static int
stickIt(OI_d_tech *oi)
{
    int x,y;
    Window junkChild;

    XTranslateCoordinates(DPY, wmScr->vroot->X_window(),
	wmScr->root, (int)oi->loc_x(), (int)oi->loc_y(), &x, &y, &junkChild);

    int stick = True;
    // if the thing is not at least partially on the screen, don't stick it
    if (!(x > wmScr->width || y > wmScr->height ||
	(x + oi->space_x() < 0 || (y + oi->space_y() < 0))))
    {
	oi->set_associated_object(wmScr->conp->abs_root(), x, y);
	return (True);
    }
    else
	return (False);
}

/***********************************************************************
 *
 *  Procedure:
 *      waitForButtonUp
 *
 *  Function:
 *	Wait for all mouse buttons to be released following the 
 *	execution of a command.
 *
 ***********************************************************************
 */

#define B_MASK (Button1Mask|Button2Mask|Button3Mask|Button4Mask|Button5Mask)
static void
waitForButtonUp()
{
    Window junkRoot, junkChild;
    int junkXRoot, junkYRoot, junkX, junkY;
    unsigned int junkMask;

    XQueryPointer(DPY, wmScr->root, &junkRoot, &junkChild,
	&junkXRoot, &junkYRoot, &junkX, &junkY, &junkMask);
    if ((junkMask & B_MASK) == 0)
	return;

    // change the cursor while we make sure buttons have been released
    wmConn->grab_pointer(wmScr->root, True, 0, GrabModeAsync, GrabModeAsync, None, wmScr->waitCursor, CurrentTime);
    while (True)
    {
	XQueryPointer(DPY, wmScr->root, &junkRoot, &junkChild,
	    &junkXRoot, &junkYRoot, &junkX, &junkY, &junkMask);
	if ((junkMask & B_MASK) == 0)
	    break;
    }
    wmConn->ungrab_pointer(CurrentTime);
}

/***********************************************************************
 *
 *  Procedure:
 *      findWindow
 *
 *  Function:
 *	Find a window to execute the function on.
 *
 ***********************************************************************
 */

static int
findWindow(
    wmData **wp,
    Window *window,
    int function,
    char *str,
    int *first,
    int *x_root,
    int *y_root
    )
{
    static wmData *save_wp;
    static wmData *last_wp;
    static wmData *first_wp;
    static int save_wp_done;
    wmData *tmp_wp;
    Window child;
    int first_time = *first;
    *first = False;
    int get_pointer_window = False;
    static int groupCmd;
    static int needWin;
    static int group;
    static char *last_name;

    if (first_time)
    {
	groupCmd = groupCommand(function);
	needWin = needWindow(function);
	save_wp_done = False;
	last_name = NULL;
	last_wp = NULL;
	first_wp = NULL;
    	group = NULL;
    }

    // if the function is stop or falsestop, then we
    // need to check the last command executed,  if it
    // needed a window, then f_stop and f_falsestop
    // also need a window
    if (function == F_STOP || function == F_FALSESTOP)
    {
	if (!needWindow(lastFunction))
	{
	    if (first_time)
	    {
		*wp = NULL;
		return (True);
	    }
	    else
		return (False);
	}
    }

    // check to see if we need a window for this operation
    // these functions do not need a window to do something
#ifdef OLD
    if (!needWindow(function))
    {
	if (first_time)
	{
	    if (*wp == NULL)
		return (True);
	}
	else if (!group)
	    return (False);
    }
#endif /* OLD */

    if (!needWin)
    {
	if (groupCmd)
	{
	    if (first_time)
	    {
		if (*wp == NULL)
		    return (True);
	    }
	    else if (!group)
		return (False);
	}
	else
	{
	    if (first_time)
		return (True);
	    else
		return (False);
	}
    }

    // these functions use the string for something other than a 
    // window name
    switch (function)
    {
	case F_WARPHORIZONTAL:
	case F_WARPVERTICAL:
	case F_WARPTOSCREEN:
	case F_SCROLLLEFT:
	case F_SCROLLRIGHT:
	case F_SCROLLUP:
	case F_SCROLLDOWN:
	case F_SCROLL:
	case F_MAP:
	case F_UNMAP:
	case F_MACRO:
	case F_STRING:
	case F_NEWBUTTONIMAGE:
	case F_REBIND:
	case F_SETDESKTOP:
	    str = NULL;
	    if (*wp == NULL && first_time)
		str = "#$";
	    break;
    }

    if (first_time)
    {
	// save_wp_done = False;
	// last_name = NULL;
	// last_wp = NULL;
	// first_wp = NULL;
    	// group = NULL;
	save_wp = *wp;
	if (*wp && (*wp)->group())
	{
	    last_wp = *wp;
	    *wp = NULL;
	}
    }

    if (str == NULL && *wp != NULL)
    {
    }
    else
    {
	*wp = NULL;

	// if the last window was a group member
	if (!str && last_wp && last_wp->group())
	{
	    // is the command valid for a group
	    if (groupCmd)
	    {
		// we have a valid group command
		if (!group)
		   tmp_wp = (wmData *)wmScr->windowList.first();
		else
		   tmp_wp = (wmData *)wmScr->windowList.next();

		group = last_wp->group();
		while (tmp_wp)
		{
		    if (tmp_wp != save_wp)
		    {
		        if (tmp_wp->group() && (tmp_wp->group() == group))
			    break;
		    }
		    tmp_wp = (wmData *)wmScr->windowList.next();
		}

		*wp = tmp_wp;
		if (tmp_wp)
		{
		    group = tmp_wp->group();
		    *window = tmp_wp->oi_frame()->X_window();
		}
	    }
	    // if *wp is NULL, we did not get a group member, so
	    // now let's do the first one
	    if (*wp == NULL)
	    {
		if (!save_wp_done)
		    *wp = save_wp;
		save_wp = NULL;
	    }
	}

	// if *wp is still NULL, we did not get a group member
	if (*wp == NULL)
	{
	    group = NULL;
	    if (str)
	    {
		if (!strcmp("multiple", str))
		    get_pointer_window = True;
		else
		{
		    // if the first character is a '#', it could be a window
		    // number or a '$' which means the window that the pointer
		    // is positioned over
		    if  (str[0] == '#')
		    {
			if (str[1] == '$' && first_time)
			{
			    // lets see what the pointer is over
			    Window junkRoot, junkChild;
			    int junkXRoot, junkYRoot, junkX, junkY;
			    unsigned int junkMask;

			    XQueryPointer(DPY, wmScr->root, &junkRoot,
				&junkChild, &junkXRoot, &junkYRoot, &junkX,
				&junkY, &junkMask);

			    XTranslateCoordinates(DPY, wmScr->root,
				wmScr->vroot->X_window(),
				junkXRoot, junkYRoot, &junkX, &junkY, &junkChild);

			    checkWindow(wp, window, junkXRoot, junkYRoot, junkChild);

			    if (wmScr->vdt)
			    {
				// now see if there is a window above this virtual root child
				XTranslateCoordinates(DPY, wmScr->root,
				    wmScr->root,
				    junkXRoot, junkYRoot, &junkX, &junkY, &junkChild);
				checkWindow(wp, window, junkXRoot, junkYRoot, junkChild);
			    }
			}
			else if (first_time)
			{
			    // must be a window number
			    Window wid = strtol(&str[1], (char **)NULL, 0);
			    for (tmp_wp = (wmData *)wmScr->windowList.first();
				tmp_wp != NULL; tmp_wp = (wmData *)wmScr->windowList.next())
			    {
				if ((tmp_wp->oi_frame() && wid == tmp_wp->oi_frame()->outside_X_window()) ||
				   (tmp_wp->oi_icon() && wid == tmp_wp->oi_icon()->X_window()) ||
				    wid == tmp_wp->window())
				{
				    if (wid == tmp_wp->window())
					wid = tmp_wp->oi_frame()->X_window();

				    *wp = tmp_wp;
				    *window = wid;
				    break;
				}
			    }

			}
		    }
		    else
		    {
			// assume that it is a named window
			if (!last_name)
			    tmp_wp = (wmData *)wmScr->windowList.first();
			else
			    tmp_wp = (wmData *)wmScr->windowList.next();

			last_name = str;

			while (tmp_wp)
			{
			    // see if we match the name or class
			    int l = strlen(last_name);
			    if (tmp_wp->name() && !strncmp(tmp_wp->name(), last_name, l))
				break;
			    if (tmp_wp->wclass_name() && !strncmp(tmp_wp->wclass_name(),last_name,l))
				break;
			    if (tmp_wp->wclass_class() && !strncmp(tmp_wp->wclass_class(),last_name,l))
				break;

			    tmp_wp = (wmData *)wmScr->windowList.next();
			}
			*wp = tmp_wp;
			if (tmp_wp && tmp_wp->oi_frame())
			    *window = tmp_wp->oi_frame()->X_window();
		    }
		}
	    }
	    else
	    {
		if (first_time)
		    get_pointer_window = True;
	    }
	}
    }

    if (*wp == NULL && get_pointer_window)
    {
	// change the cursor while we go find a window
	wmConn->grab_pointer(wmScr->root, True, ButtonPressMask, GrabModeAsync, GrabModeAsync,
		wmScr->root, wmScr->chooseCursor, CurrentTime);

	XEvent event;
	XButtonEvent *ev = (XButtonEvent *)&event;
	int done = False;

	while (!done)
	{
	    XMaskEvent(DPY, ButtonPressMask | ButtonReleaseMask, (XEvent *)&event);
	    if (ev->type == ButtonPress)
	    {
		int destx, desty;

		XTranslateCoordinates(DPY, wmScr->root,
		    wmScr->vroot->X_window(),
		    ev->x_root, ev->y_root, &destx, &desty, &child);

		*wp = NULL;
		checkWindow(wp, window, ev->x_root, ev->y_root, child);

		if (wmScr->vdt)
		{
		    // now see if there is a window above this virtual root child
		    XTranslateCoordinates(DPY, wmScr->root,
			wmScr->root,
			ev->x_root, ev->y_root, &destx, &desty, &child);
		    checkWindow(wp, window, ev->x_root, ev->y_root, child);
		}

		done = True;
		if (wp != NULL)
		{
		    save_wp = *wp;
		    *x_root = ev->x_root;
		    *y_root = ev->y_root;
		}
	    }
	}
	wmConn->ungrab_pointer(CurrentTime);
    }

    if (*wp != NULL)
    {
	if (save_wp == *wp)
	    save_wp_done = True;
	last_wp = *wp;
	if (!first_wp)
	    first_wp = *wp;
	return (True);
    }
    else
    {
	last_wp = NULL;
	last_name = NULL;
	return (False);
    }
}

static void
checkWindow(
    wmData **wp,
    Window *window,
    int xroot,
    int yroot,
    Window child
    )
{
    wmData *mywp;
    int destx, desty;

    if (child == 0)
	return;

    if (XFindContext(DPY, child, wmIconContext, (caddr_t*)&mywp) != XCNOENT)
    {
	// the window was an icon on the root window
	*wp = mywp;
	*window = mywp->oi_icon()->X_window();
	return;
    }

    if (XFindContext(DPY, child, wmFrameContext, (caddr_t*)&mywp) != XCNOENT)
    {
	*wp = mywp;
	*window = mywp->oi_frame()->X_window();

	// we got one, if it is an icon panel, we need to see if
	// the pointer was over an icon,  if it was, move the 
	// icon instead of the panel

	if ((*wp)->myip())
	{
	    Window new_child = wmScr->root;
	    Window last_child;
	    child = 0;
	    while (True)
	    {
		XTranslateCoordinates(DPY, wmScr->root, new_child,
		    xroot, yroot, &destx, &desty,
		    &last_child);
		if (last_child == 0)
		    break;
		child = new_child = last_child;

		// check to see if this is an icon object
		if (!(XFindContext(DPY, child, wmIconContext,
		    (caddr_t*)&mywp) == XCNOENT))
		{
		    *wp = mywp;
		    *window = mywp->oi_icon()->X_window();
		}
	    }
	}
    }
}

static int
groupCommand(int function)
{
    int valid = False;

    switch (function)
    {
	case F_STOP:
	case F_FALSESTOP:
	case F_MAP:
	case F_UNMAP:
	case F_STICK:
	case F_UNSTICK:
	case F_ICONIFY:
	case F_DEICONIFY:
	case F_RAISE:
	case F_LOWER:
	case F_RAISELOWER:
	case F_WINREFRESH:
	case F_REBIND:
	case F_NEWBUTTONIMAGE:
	    valid = True;
	    break;
    }

    return (valid);
}

static int
needWindow(int function)
{
    int status = True;
    switch (function)
    {
	case F_WARPHORIZONTAL:
	case F_WARPVERTICAL:
	case F_WARPTOSCREEN:
	case F_SAVEPOINTER:
	case F_RESTOREPOINTER:
	case F_RESTART:
	case F_QUIT:
	case F_BEEP:
	case F_CIRCLEDOWN:
	case F_CIRCLEUP:
	case F_EXEC:
	case F_NOP:
	case F_REFRESH:
	case F_UNFOCUS:
	case F_WARPTO:
	case F_MENU:
	case F_MACRO:
	case F_PANNER:
	case F_SCROLLLEFT:
	case F_SCROLLRIGHT:
	case F_SCROLLUP:
	case F_SCROLLDOWN:
	case F_SCROLLHOME:
	case F_SCROLL:
	case F_RECONFIG:
	case F_GETRESOURCE:
	case F_SAVEVROOT:
	case F_RESTOREVROOT:
	case F_SCROLLSAVE:
	case F_WARPSAVE:
	case F_PLACES:
	case F_TERMINALPLACES:
	case F_SWEEP:
	case F_UNSWEEP:
	case F_HARDRESTART:
	case F_HANDLEDROP:
	case F_REBIND:
	case F_NEWBUTTONIMAGE:
	case F_MAP:
	case F_UNMAP:
	case F_ROUNDUP:
	case F_SETDESKTOP:
	case F_FREEZEDESKTOP:
	case F_THAWDESKTOP:
	case F_INFO:
	    status = False;
	    break;
    }
    return (status);
}

static wmBindings *localbps;

/***********************************************************************
 *
 *  Procedure:
 *      wmGetBindings
 *
 *  Function:
 *	Get the bindings associated with an object
 *
 ***********************************************************************
 */

void
wmGetBindings(
    wmObject *op		// the object to get bindings on
    )
{
    char *ptr;
    wmBindings *bps;

    if (!(ptr = RM->get_resourceq(wmQuarks->bindingsName(), wmQuarks->bindingsClass())))
	return;


    if ((bps = parseBindings(ptr))) 
    {
	bindObject(op->oi, bps);
	bps->helpConstructed = True;
    }
}

static wmBindings *
parseBindings(char *ptr)
{
    wmBindings *bps;
    wmBinding *bp;
    char *nestedPtr;

    if (XFindContext(DPY, (Window)ptr, wmBindingsContext, (caddr_t*)&bps) == XCNOENT)
    {
	bps = localbps = new wmBindings(ptr);
	wmParse("wmBinding", ptr);
	if (wmParseError)
	{
	    fprintf(stderr, "swm: wmGetBindings: syntax error on \"%s\"\n",
	       ptr);
#ifdef DEBUG
	    if (wmDebug > 1)
		fprintf(dfp, "swm: wmGetBindings: syntax error\n");
#endif /* DEBUG */
	    delete localbps;
	    return (NULL);
	}
	XSaveContext(DPY, (Window)ptr, wmBindingsContext, (caddr_t)localbps);

	// let's see if we had nested bindings
	for (bp = (wmBinding *)bps->bindings->first(); bp != NULL; bp = (wmBinding *)bps->bindings->next())
	{
	    if (bp->what == BINDINGS && bp->str)
	    {
		RM->pushq(wmQuarks->bindingsName(), wmQuarks->bindingsClass());
		nestedPtr = RM->get_resource(bp->str, bp->str);
		RM->pop();

		if (nestedPtr)
		    bp->next = parseBindings(nestedPtr);
	    }
	}
    }

    return (bps);
}

static void
bindObject(
    OI_d_tech *oi, 
    wmBindings *bps)
{
#ifdef DEBUG
    if (wmDebug)
    {
	fprintf(dfp, "bindObject: \"%s\"  \"%s\"\n", oi->name(), bps->resource);
    }
#endif /* DEBUG */
    wmBinding *bp;
    wmObjectData *wod;
    wod = (wmObjectData *)oi->data();
    int client = wod->op->client;

    // now go through each binding and tie it to the object in some way
    for (bp = (wmBinding *)bps->bindings->first(); bp != NULL; 
        bp = (wmBinding *)bps->bindings->next())
    {
	if (bp->what == BINDINGS && bp->next)
	{
	    bindObject(oi, bp->next);
	    bp->next->helpConstructed = True;
	    continue;
	}

	switch (bp->what)
	{
	    case KEY:
		// if the bindings have not been expanded, do some work
		if (!bp->expanded)
		{
		    bp->from = KEY;
		    // Don't let a 0 keycode go through, since that means
		    // AnyKey to the XGrabKey call in GrabKeys().
		    if ((bp->keysym = XStringToKeysym(bp->str)) == NoSymbol || (bp->keycode = XKeysymToKeycode(DPY, bp->keysym))==0)
		    {
			fprintf(stderr, "swm: wmGetBindings: unknown key \"%s\"\n", bp->str);
		    }
		    bp->expanded = True;
		}

		// if we aren't yet handling key presses ...
		if (!wod->handleKey && bp->keycode != 0)
		{
		    wod->handleKey = True;
		    OI_dispatch_insert(oi->X_window(), KeyPress, KeyPressMask, (OI_event_fnp)wmHandleKeyPress, (char *)oi);

		}
		if (client && bp->keycode != 0)
		    XGrabKey(DPY, bp->keycode, (bp->mods & modMask), oi->X_window(), True, GrabModeAsync, GrabModeAsync);
		break;

	    case DROP:
	    case DROP1:
	    case DROP2:
	    case DROP3:
	    case DROP4:
	    case DROP5:
		bp->from = bp->what;
		if (!wod->handleDrop)
		{
		    wod->handleDrop = True;
		    OI_dispatch_insert(oi->X_window(), ClientMessage, 0, (OI_event_fnp)wmClientMessage, (char *)wod->wp);
		    OI_dispatch_insert(oi->X_window(), PropertyNotify, PropertyChangeMask, (OI_event_fnp)wmHandleDrop, (char *)oi);
		}
		break;

	    default:
		// must be a button
		if (!bps->helpConstructed && wmScr->help)
		    wmBuildHelp(bps, bp);

		if (client && (bp->what != BTN))
		    XGrabButton(DPY, bp->what, (bp->mods & modMask), oi->X_window(), True,
			ButtonPressMask | ButtonReleaseMask,
			GrabModeAsync, GrabModeAsync, None, None);

		// we have to do something special if it is a move or resize
		// function, we don't want a click, we want the button press
		if (wod && bp->move)
		{
		    bp->from = PRESS;
		    if (!wod->handlePress)
		    {
			wod->handlePress = True;
			OI_dispatch_insert(oi->X_window(), ButtonPress, ButtonPressMask, (OI_event_fnp)wmHandlePress, (char *)oi);
		    }
		}
		else
		{
		    // check for an f.menu command
		    wmFunction *fp;
		    int got_menu = False;
		    for (fp = (wmFunction *)bp->functions->first(); fp != NULL; 
			fp = (wmFunction *)bp->functions->next())
		    {
			if (fp->function != F_MENU)
			    continue;

			bp->menu = True;
			// go get the menu
			bp->pop = wmGetMenu(fp->argv[0], fp->argvq, bp->what, bp->mods & 0xff);
			
			if (bp->pop)
			{
			    if (wod->op->root)
			    {
				bp->pop->set_associated_object(wmScr->vroot,0,0,
				    OI_ACTIVE_NOT_DISPLAYED);
			    }
			    else
			    {
				bp->pop->set_associated_object(oi, 0, 0, OI_ACTIVE_NOT_DISPLAYED);
			    }
			    bp->pop->set_menu_trigger(bp->what, bp->mods & 0xff);
			    got_menu = True;
			}
		    }

		    if (got_menu)
			continue;

		    int clicks = 0;
		    if (bp->str)
			clicks = atoi(bp->str);

		    switch (clicks)
		    {
			case 0:
			case 1:
			    bp->from = CLICK1;
			    break;
			case 2:
			    bp->from = CLICK2;
			    break;
			case 3:
			    bp->from = CLICK3;
			    break;
			    
			default:
			    break;
		    }
		    if (wod->op->root)
			wmScr->conp->set_click((OI_click_fnp)click, oi);
		    else
			oi->set_click((OI_click_fnp)click, oi);
		}
		break;
	    }
    }
    wod->bps = bps;
}

void
unbindObject(OI_d_tech *oi, wmBindings *bps)
{
#ifdef DEBUG
    if (wmDebug)
    {
	fprintf(dfp, "unbindObject: \"%s\"  \"%s\"\n", oi->name(), bps->resource);
    }
#endif /* DEBUG */
    wmBinding *bp;
    wmObjectData *wod;
    OI_d_tech *child;
    wod = (wmObjectData *)oi->data();
    int client = wod->op->client;

    if (!bps)
	return;

    // now go through each binding and tie it to the object in some way
    for (bp = (wmBinding *)bps->bindings->first(); bp != NULL; 
        bp = (wmBinding *)bps->bindings->next())
    {
	if (bp->what == BINDINGS && bp->next)
	{
	    unbindObject(oi, bp->next);
	    continue;
	}

	switch (bp->what)
	{
	    case KEY:
		if (wod->handleKey)
		{
		    if (client)
			XUngrabKey(DPY, (int)AnyKey, AnyModifier, oi->X_window());
		    wod->handleKey = False;
		    OI_dispatch_remove(oi->X_window(), KeyPress, KeyPressMask, (OI_event_fnp)wmHandleKeyPress, (char *)oi);
		}
		break;

	    case DROP:
	    case DROP1:
	    case DROP2:
	    case DROP3:
	    case DROP4:
	    case DROP5:
		if (wod->handleDrop)
		{
		    wod->handleDrop = False;
		    OI_dispatch_remove(oi->X_window(), ClientMessage, 0, (OI_event_fnp)wmClientMessage, (char *)wod->wp);
		    OI_dispatch_remove(oi->X_window(), PropertyNotify, PropertyChangeMask, (OI_event_fnp)wmHandleDrop, (char *)oi);
		}
		break;

	    default:
		if (client)
		    XUngrabButton(DPY, (int)AnyButton, AnyModifier, oi->X_window());

		if (wod->handlePress)
		{
		    wod->handlePress = False;
		    OI_dispatch_remove(oi->X_window(), ButtonPress, ButtonPressMask, (OI_event_fnp)wmHandlePress, (char *)oi);
		}

		
		// bp->pop is really used as a flag to indicate there is a menu associated with 
		// this binding.  I never really delete the menu that bp->pop points to, I always
		// find children of the object that are menus
		if (bp->pop)
		{
		    while (True)
		    {
			for (child = oi->next_child(NULL); child != NULL; child = oi->next_child(child))
			{
			    if (child->is_derived_from("OI_menu"))
			    {
				wmDeleteOI(child, False);
				if (wmMenuWindow && child->descendant_by_window(wmMenuWindow))
				    child->delete_all_delayed();
				else
				    child->delete_all();
				break;
			    }
			}
			if (child == NULL)
			    break;
		    }
		}
		break;
	    }
    }
    if (wod->op->root)
	wmScr->conp->set_click((OI_click_fnp)NULL, (OI_d_tech *)NULL);
    else
	oi->set_click((OI_click_fnp)NULL, (OI_d_tech *)NULL);
    wod->bps = NULL;
}

/***********************************************************************
 *
 *  Procedure:
 *      wmAddBinding
 *
 *  Function:
 *	Start a new binding
 *
 ***********************************************************************
 */

void
wmAddBinding(
    unsigned mods,			// modifier key mask
    int what,				// BTNx or KEY
    char *str				// the string following
    )
{
#ifdef DEBUG
    if (wmDebug)
	fprintf(dfp, "wmAddBinding: mods=0x%x, what=%s, str=\"%s\"\n",
	    mods, 
	    what == KEY ? "KEY" : "BTN",
	    str);
#endif /* DEBUG */

    wmBind = new wmBinding();
    wmBind->mods = mods;
    wmBind->what = what;
    wmBind->str = str;
}

/***********************************************************************
 *
 *  Procedure:
 *      wmBindingDone
 *
 *  Function:
 *	Finish up with the bindings
 *
 ***********************************************************************
 */

void
wmBindingDone()
{
    localbps->bindings->append((ent)wmBind);
}

/***********************************************************************
 *
 *  Procedure:
 *      wmAddFunction
 *
 *  Function:
 *	Add a function to a binding
 *
 ***********************************************************************
 */

void
wmAddFunction(
    int function,
    char **argv,
    int argc
    )
{
#ifdef DEBUG
    if (wmDebug)
	fprintf(dfp, "wmAddFunction: func = %d, argv[0] = \"%s\"\n", function, argv[0]);
#endif
    int i;

    wmFunction *func = new wmFunction();
    wmBind->functions->append((ent)func);
    func->function = function;
    if (argc == 0)
	func->argv = (char **)malloc(sizeof(char *));
    else
	func->argv = (char **)malloc(sizeof(char *) * argc);
    for (i = 0; i < argc; i++)
	func->argv[i] = argv[i];
    func->argc = argc;
    func->argvq = (int)None;
    switch (function)
    {
	case F_MENU:
		if (func->argc)
		    func->argvq = XrmStringToQuark(func->argv[0]);
		break;
	case F_MOVE:
	case F_MOVEOPAQUE:
	case F_FORCEMOVE:
	case F_CONSTRAINMOVE:
	case F_RESIZE:
	case F_SWEEP:
		wmBind->move = True;
		break;
    }
}

/***********************************************************************
 *
 *  Procedure:
 *      click
 *
 *  Function:
 *	click handler
 *
 ***********************************************************************
 */

static void
click(
    OI_d_tech *oi,
    void *,
    OI_number clicks,
    OI_number button,
    OI_number mods
    )
{
    int from = 0;

    switch (clicks) 
    {
	case 0:
	case 1:
	    from = CLICK1;
	    break;
	case 2:
	    from = CLICK2;
	    break;
	case 3:
	    from = CLICK3;
	    break;
    }
    mods |= (0x80 << button);
    findBinding(oi, mods, from, 0, 0, -1, -1);
}

/***********************************************************************
 *
 *  Procedure:
 *      wmHandleDrop
 *
 *  Function:
 *	Handle a keyboard key press
 *
 ***********************************************************************
 */

void
wmHandleDrop(
    XPropertyEvent *ev,
    OI_d_tech *oi
    )
{
    if (ev->atom == XA_XV_DO_DRAG_LOAD)
    {
	dropAtom = ev->atom;
	dropWindow = ev->window;
	findBinding(oi, 0, DROP, 0, 0, -1, -1);
    }
}

/***********************************************************************
 *
 *  Procedure:
 *      wmHandleKeyPress
 *
 *  Function:
 *	Handle a keyboard key press
 *
 ***********************************************************************
 */

void
wmHandleKeyPress(
    XEvent *ev,
    OI_d_tech *oi
    )
{
    static Time lastTime = 0;
    KeySym keysym;

    if (ev->xkey.time == lastTime)
	return;
    lastTime = ev->xkey.time;
    ev->xkey.state &= ~LockMask;	// Don't care if Caps Lock is active

    keysym = XLookupKeysym(&ev->xkey, 0);
    if (!findBinding(oi, ev->xkey.state, KEY, keysym, ev->xkey.keycode, ev->xkey.x_root, ev->xkey.y_root))
    {
	wmObjectData *odp = (wmObjectData *)oi->data();
	if (odp && odp->wp && odp->wp->focus_type() != wmNoInput)
	{
	    wmData *wp = odp->wp;
	    ev->xkey.window = wp->window();
	    XSendEvent(DPY, wp->window(), False, KeyPressMask, ev);
	}
    }
}

/***********************************************************************
 *
 *  Procedure:
 *      wmHandlePress
 *
 *  Function:
 *	Handle a mouse button press, in addition to possibly handling
 *	a click.
 *
 ***********************************************************************
 */

void
wmHandlePress(
    XButtonEvent *ev,
    OI_d_tech *oi
    )
{
    static Time lastTime = 0;
    if (ev->time == lastTime)
	return;
    lastTime = ev->time;
    ev->state &= ~LockMask;	// Don't care if Caps Lock is active

    wmData *wp = NULL;
    int resize = False;
    switch (ev->button)
    {
	case Button1: ev->state |= Button1Mask; break;
	case Button2: ev->state |= Button2Mask; break;
	case Button3: ev->state |= Button3Mask; break;
	case Button4: ev->state |= Button4Mask; break;
	case Button5: ev->state |= Button5Mask; break;
    }

    // check to see if this is a frame and if resize corners or bars
    // are turned on
    wmObjectData *odp = (wmObjectData *)oi->data();
    if (odp)
	wp = odp->wp;

    // if we are in click to type mode, handle the focus change here
    if (wp && oi == wp->oi_frame() && ev->button == Button1 && wmFocusModel == wmFocusModelClickToType)
    {
	wmSetFocus(NULL, wp);

	// figure out if the button was within the client window, if it was,
	// simply allow the button press to go through and then return
	OI_d_tech *tmpoi = oi->descendant_by_window(ev->subwindow);
	if (tmpoi == wp->oi_client())
	{
	    XAllowEvents(DPY, ReplayPointer, CurrentTime);
	    return;
	}

	// If ev->subwindow is not None, then we will reset the oi variable to point
	// to the object that the mouse was on.  If we don't have any bindings on
	// the child object, then we will try the frame again
	if (ev->subwindow)
	{
	    if (!findBinding(tmpoi, ev->state, PRESS, 0, 0, ev->x_root, ev->y_root))
	    {
		// Not finding a binding for the PRESS is not good enough, now we need to 
		// see if there is a binding for a button 1 click.  If there is, then we
		// will allow events to go through and then return and let the click
		// handler execute the function.  If there are no click functions, then
		// we will process the button press on the frame.
		// BOY DO I HATE CLICK TO TYPE !!!!!
		wmObjectData *odp = (wmObjectData *)tmpoi->data();
		if (odp->bps && odp->bps->bindings)
		{
		    wmBinding *bp;
		    for (bp = (wmBinding *)odp->bps->bindings->first(); bp != NULL;
			bp = (wmBinding *)odp->bps->bindings->next())
		    {
			if (bp->from & CLICK_MASK)
			{
			    // we found a click function, we will return here and
			    // just let the click handler take care of it
			    XAllowEvents(DPY, ReplayPointer, CurrentTime);
			    return;
			}
		    }
		}
	    }
	    else
	    {
		// we found a press on the child object and executed it
		return;
	    }
	}
    }

    wmResizeEvent = (XButtonEvent)*ev;
    if (odp && odp->wp && (oi == odp->wp->oi_frame()))
    {
	// OK, the press was in the frame, let's see if it was near
	// a corner
	if (odp->wp->resize_corners())
	{
	    if (ev->x < odp->wp->resize_length() && ev->y < odp->wp->resize_length())
	    {
		wmResizeTL(ev, odp->wp);
		resize = True;
	    }
	    else if (ev->x > (odp->wp->oi_frame()->size_x() - odp->wp->resize_length()) && ev->y < odp->wp->resize_length())
	    {
		wmResizeTR(ev, odp->wp);
		resize = True;
	    }
	    else if (ev->x < odp->wp->resize_length() && ev->y > (odp->wp->oi_frame()->size_y() - odp->wp->resize_length()))
	    {
		wmResizeBL(ev, odp->wp);
		resize = True;
	    }
	    else if (ev->x > (odp->wp->oi_frame()->size_x() - odp->wp->resize_length()) && ev->y > (odp->wp->oi_frame()->size_y() - odp->wp->resize_length()))
	    {
		wmResizeBR(ev, odp->wp);
		resize = True;
	    }
	}

	// if we didn't resize from a corner and resize bars are on
	// check to see if a press was in one
	if (!resize && odp->wp->resize_bars())
	{
	    if (ev->x < odp->wp->resize_width())
	    {
		wmResizeL(ev, odp->wp);
		resize = True;
	    }
	    else if (ev->x >
		    (odp->wp->oi_frame()->size_x() - odp->wp->resize_width()))
	    {
		wmResizeR(ev, odp->wp);
		resize = True;
	    }
	    else if (ev->y < odp->wp->resize_width())
	    {
		wmResizeT(ev, odp->wp);
		resize = True;
	    }
	    else if (ev->y >
		    (odp->wp->oi_frame()->size_y() - odp->wp->resize_width()))
	    {
		wmResizeB(ev, odp->wp);
		resize = True;
	    }
	}
    }

    if (!resize)
	findBinding(oi, ev->state, PRESS, 0, 0, ev->x_root, ev->y_root);
    else
	wmScr->conp->dispatch_ignore((XEvent *)ev);

    wmResizeEvent.type = ~ButtonPress;
}

/***********************************************************************
 *
 *  Procedure:
 *      findBinding
 *
 *  Function:
 *	Look through the list of bindings for the object and check
 *	to see if one matches the combination oc modifier keys,
 *	button clicks/press, and or key press.
 *
 ***********************************************************************
 */

int
findBinding(
    OI_d_tech *oi,
    OI_number mods,
    int from,
    KeySym keysym,
    KeyCode keycode,
    int x_root,
    int y_root
    )
{
    Window win = None;
    wmObjectData *odp = (wmObjectData *)oi->data();
    int status = False;

    if (x_root == -1)
    {
	Window junkRoot, junkChild;
	int junkX, junkY;
	unsigned int junkMask;

	XQueryPointer(DPY, wmScr->root, &junkRoot, &junkChild, &x_root, &y_root, &junkX, &junkY, &junkMask);
    }

    if (odp)
    {
	if (odp->wp)
	{
	    if (odp->inAnIcon)
		win = odp->wp->oi_icon()->X_window();
	    else
		win = odp->wp->oi_frame()->X_window();
	}

	if (odp->bps && odp->bps->bindings)
	{
	    status = lookForBinding(odp->wp, win, from, mods, keysym, keycode, x_root, y_root, odp->bps);
	}
    }
    return (status);
}

static int
lookForBinding(wmData *wp, Window window, int from, int mods, KeySym keysym, KeyCode keycode, int x_root, int y_root, wmBindings *bps)
{
    int status = False;
    wmBinding *bp;

    for (bp = (wmBinding *)bps->bindings->first(); bp != NULL; bp = (wmBinding *)bps->bindings->next())
    {
	unsigned mask;

	if (bp->what == BINDINGS && bp->next)
	{
	    status = lookForBinding(wp, window, from, mods, keysym, keycode, x_root, y_root, bp->next);
	    if (status)
		break;

	    continue;
	}
	if (bp->menu)
	    continue;
	if (bp->from != from)
	    continue;
	if (bp->what == KEY && bp->keysym != keysym && bp->keycode != keycode)
	    continue;

	if (bp->what == BTN || bp->what == KEY)
	    mask = 0xFF;
	else
	    mask = ~0;
	if (bp->mods == (mods & mask))
	{
	    status = True;
	    wmFindScreen(wp, window);
	    wmExecuteBinding(wp, window, bp, NOT_FROM_MENU,x_root,y_root);
	}
    }
    return (status);
}

/***********************************************************************
 *
 *  Procedure:
 *      wmExecuteBinding
 *
 *  Function:
 *	Execute each function in a binding
 *
 ***********************************************************************
 */

void 
wmExecuteBinding(
    wmData *wp,
    Window win,
    wmBinding *bp,
    int from,
    int x_root,
    int y_root
    )
{
    wmFunction *fp;
    int stat = False;
    firstWp = NULL;
    clearStop();
    stopExecution = False;
    for (fp = (wmFunction *)bp->functions->first(); fp != NULL; 
	fp = (wmFunction *)bp->functions->next())
    {
	wmData *save;

	if (stopExecution)
	    break;
	save = wp;
	if (from == FROM_MENU &&
	    (fp->function == F_MOVE || fp->function==F_MOVEOPAQUE||fp->function == F_FORCEMOVE ||
		fp->function == F_CONSTRAINMOVE || fp->function == F_RESIZE || fp->function == F_SWEEP)) {
	    wp = NULL;
	    win = None;
	}
	firstWindow = win;
	stat = wmExecute(wp, win, fp->function, (char **)fp->argv, fp->argc, x_root, y_root);
	if (wp == NULL)
	{
	    save = firstWp;
	}
	wp = save;
	win = firstWindow;
    }
    waitForButtonUp();
}

/***********************************************************************
 *
 *  Procedure:
 *      wmExternalExecute
 *
 *  Function:
 *	Execute each function in a binding
 *
 ***********************************************************************
 */

void 
wmExternalExecute(
    wmData *wp,			// window data (if any)
    Window win,			// the window (frame or icon or NULL)
    int what,			// what function to execute
    char **argv,		// argument strings
    int argc,			// argument count
    int x_root,			// the X root coordinate
    int y_root			// the Y root coordinate
    )
{
    firstWp = NULL;
    clearStop();
    stopExecution = False;
    wmExecute(wp, win, what, argv, argc, x_root, y_root);
}

static void
clearStop()
{
    wmData *tmp_wp;

    for (tmp_wp = (wmData *)wmScr->windowList.first(); tmp_wp != NULL; tmp_wp = (wmData *)wmScr->windowList.next())
    {
	tmp_wp->clear_stop();
	tmp_wp->set_last_status(False);
    }
}

/***********************************************************************
 *
 *  Procedure:
 *      shell
 *
 *  Function:
 *	Send a string off to /bin/sh for execution
 *
 ***********************************************************************
 */

static void
shell(
    char *s
    )
{
    static char buf[256];
    char *ds = DisplayString(DPY);
    char *colon, *dot1;
    char oldDisplay[256];
    char *doisplay;
    int restorevar = 0;

    if (s == NULL)
	return;

    oldDisplay[0] = '\0';
    doisplay=(char *)getenv("DISPLAY");
    if (doisplay)
	strcpy (oldDisplay, doisplay);

    /*
     * Build a display string using the current screen number, so that
     * X programs which get fired up from a menu come up on the screen
     * that they were invoked from, unless specifically overridden on
     * their command line.
     */
    colon = strrchr (ds, ':');
    if (colon) {			/* if host[:]:dpy */
	strcpy (buf, "DISPLAY=");
	strcat (buf, ds);
	colon = buf + 8 + (colon - ds);	/* use version in buf */
	dot1 = index (colon, '.');	/* first period after colon */
	if (!dot1) dot1 = colon + strlen (colon);  /* if not there, append */
	sprintf (dot1, ".%d", wmScr->screen);
	putenv (buf);
	restorevar = 1;
    }

    (void) system (s);

    if (restorevar) {
	(void) sprintf (buf, "DISPLAY=%s", oldDisplay);
	putenv (buf);
    }
}


wmBinding::wmBinding()
{
    next = NULL;
    functions = new wmList();
    move = False;
    menu = False;
    pop = NULL;
    expanded = False;
}

wmBindings::wmBindings(char *p)
{
    int b, m;

    resource = p;
    bindings = new wmList();
    helpConstructed = False;
    for (b = 0; b < SWM_BUTTONS; b++)
	for (m = 0; m < SWM_MODIFIERS; m++)
	    button_helps[b][m] = NULL;
}
