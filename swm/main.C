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
 * $Id: main.C,v 9.19 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Start up and shutdown code
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/

#ifndef lint
static char RCSinfo[] =
"$Id: main.C,v 9.19 1993/08/27 16:58:15 toml Exp $";
#endif /* lint */

#include "swm.H"
#include "main.H"
#include "init.H"
#include "screen.H"
#include "debug.H"
#include "wmdata.H"
#include "reparent.H"
#include "atoms.H"
#include "region.H"
#include <signal.h>
#if defined(SYSV) || defined(_AIX) || defined(ultrix)
typedef void (*SIG_PF)(int);
#endif
#ifdef _AIX
#include <sys/socket.h>		/* for gethostname */
#endif /* _AIX */
#ifdef ultrix
extern "C" int gethostname(char*, int);
#endif

OI_connection *wmConn;

const int wmDefaultResizeWidth = 4;

int wmHardRestart = False;		// hard restart flag
wmScreen **wmScreens;			// structures for each screen
wmScreen *wmScr = NULL;			// the current screen
char wmHost[100];			// where the window manager is being run from
char *wmDisplay;			// the display system name
static void cleanup();
static void savewin(wmData *);
static void busError();
static void segViolation();

char **wmArgv;
int wmArgc;
static char **Environ;

main(int argc, char **argv, char **environ)
{
    wmArgc = argc;
    wmArgv = (char **)calloc(sizeof(char *), argc+1);
    for (int i = 0; i < argc; i++)
	wmArgv[i] = argv[i];
    wmArgv[i] = NULL;

    Environ = environ;

    SIG_PF old_handler;
    old_handler = signal(SIGINT, SIG_IGN);
    if (old_handler != SIG_IGN)
        signal(SIGINT, (SIG_PF)wmDone);

    old_handler = signal(SIGHUP, SIG_IGN);
    if (old_handler != SIG_IGN)
        signal(SIGHUP, (SIG_PF)wmDone);

    signal(SIGQUIT, (SIG_PF)wmDone);
    signal(SIGTERM, (SIG_PF)wmDone);

#ifndef NO_SIGNALS
    signal(SIGBUS, (SIG_PF)busError);
    signal(SIGSEGV, (SIG_PF)segViolation);
#endif

#ifdef MALLOC_DEBUG
    malloc_debug(2);
#endif /* MALLOC_DEBUG */
    // tell the toolkit that I am a window manager
    OI_set_wm_client();

    // connect to the server
    wmConn = OI_init(&argc, argv, "Swm");
    if (wmConn == NULL)
    {
	fprintf(stderr, "%s: OI_init failed\n", argv[0]);
	exit(1);
    }

    // I think the OI does this also, but I like doing it
    if (fcntl(ConnectionNumber(DPY), F_SETFD, 1) == -1)
    {
        fprintf(stderr, "swm: child cannot disinherit TCP fd\n");
        exit(1);
    }

    gethostname(wmHost, 100);
    char *tmp = DisplayString(DPY);
    if (!strncmp("unix", tmp, 4))
	wmDisplay = &wmHost[0];
    else
    {
	wmDisplay = (char *)malloc (strlen(tmp)+1);
	strcpy(wmDisplay, tmp);
	char *ptr = strchr(wmDisplay, ':');
	if (ptr) *ptr = '\0';
    }

    wmInitialize(argc, argv);
    OI_begin_interaction();
    
    // we really should never get here, but just in case ...
    wmDone();
}

void
busError()
{
    // attempt to clean things up
    fprintf(stderr, "Bus Error\n");
    wmHardRestart = True;
    cleanup();
    abort();
    exit(1);
}

void
segViolation()
{
    // attempt to clean things up
    fprintf(stderr, "Segmentation Violation\n");
    wmHardRestart = True;
    cleanup();
    abort();
    exit(1);
}

void
wmRestart()
{
    cleanup();
    XCloseDisplay(DPY);
#if defined(ultrix) || defined(hpux) || defined(SYSV)
    execvp(*wmArgv, wmArgv);
#else
    execvp(*wmArgv, (const char **)wmArgv);
#endif
}

void
wmDone()
{
    wmHardRestart = True;
    cleanup();
    XCloseDisplay(DPY);
    exit(0);
}

void
cleanup()
{
    wmData *wp;

    for (int screen = 0; screen < wmNumScreens; screen++)
    {
	wmScr = wmScreens[screen];
	if (wmScr == NULL)
	    continue;
	wmScr->conp->make_default();
	wmRemoveVersion();

	// cleanup all windows on the virtual root
	if (wmScr->vdt)
	{
	    Window root_return,parent_return,*children;
	    unsigned int num_children;
	    int i;
	    int x, y;
	    unsigned int width, height, bw, depth;

	    XQueryTree(DPY, wmScr->vroot->X_window(), &root_return,&parent_return, &children, &num_children);

	    for (i = 0; i < num_children; i++)
	    {
		if (!(IS_INTERNAL(children[i])))
		{
		    XGetGeometry(DPY, children[i], &root_return, &x, &y, &width, &height, &bw, &depth);
		    XReparentWindow(DPY, children[i], wmScr->root, x, y);
		    XRemoveFromSaveSet(DPY, children[i]);
		    // XUnmapWindow(DPY, children[i]);
		}
	    }
	}

	XSelectInput(DPY, wmScr->root, 0);
	for (wp = (wmData *)wmScr->windowList.first(); wp != NULL;
	    wp = (wmData *)wmScr->windowList.next())
	{
	    if (!wp->mine())
		savewin(wp); 
	    else
	    {
		if (!wmHardRestart)
		    wmSet__SWM_START(wp);
	    }
	}
	XSync(DPY, 0);
    }
    XSetInputFocus(DPY, PointerRoot, RevertToPointerRoot, CurrentTime);
}

void
putVersion()
{
}

void
removeVersion()
{
}

void
savewin(wmData *wp)
{
    int gravx, gravy;
    int dstx = (int)wp->oi_frame()->loc_x();
    int dsty = (int)wp->oi_frame()->loc_y();

    if (!wp->placed())
    {
	int xright = dstx + wp->oi_frame()->space_x();
	int ybottom = dsty + wp->oi_frame()->space_y();
	wmGetGravityOffsets(wp, &gravx, &gravy);
	if (gravx == 1)
	    dstx = xright - wp->attr_width() - 2*wp->attr_bw();
	if (gravy == 1)
	    dsty = ybottom - wp->attr_height() - 2*wp->attr_bw();
    }

    XRemoveFromSaveSet(DPY, wp->window());
    XReparentWindow(DPY, wp->window(), wmScr->root, dstx, dsty);
    XDeleteProperty(DPY, wp->window(), __SWM_ROOT);
    XMapWindow(DPY, wp->window());
    if (wp->wmhints() && (wp->wmhints()->flags & IconWindowHint))
    {
	XRemoveFromSaveSet(DPY, wp->wmhints()->icon_window);
	XReparentWindow(DPY, wp->wmhints()->icon_window, wmScr->root, 0, 0);
	XUnmapWindow(DPY, wp->wmhints()->icon_window);
    }
    // reparented, don't want to see it later
    // wmScr->windowList.rm(wp);
}
