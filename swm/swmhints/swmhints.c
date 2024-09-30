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
 * $Id: swmhints.c,v 9.3 1993/08/27 16:56:48 toml Exp $
 *
 * Description:
 *	Set the __SWM_START property for swm restart
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/

#ifndef lint
static char RCSinfo[] =
"$Id: swmhints.c,v 9.3 1993/08/27 16:56:48 toml Exp $";
#endif lint
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include "swmstart.H"

char *program_name;
Display *dpy;
int screen;
Window root;

#define CMD_OPTION	0x01
#define GEOM_OPTION	0x02
#define ICON_OPTION	0x04
#define STATE_OPTION	0x08

#define OPTIONS (CMD_OPTION|GEOM_OPTION|STATE_OPTION)
usage()
{
    fprintf(stderr, "Usage: %s [options]\n", program_name);
    fprintf(stderr, "  where options are:\n");
    fprintf(stderr, "  -cmd <WM_COMMAND string>\n");
    fprintf(stderr, "  -display <display>\n");
    fprintf(stderr, "  -geometry <geometry>\n");
    fprintf(stderr, "  -iconGeometry <geometry>\n");
    fprintf(stderr, "  -machine <machine name>\n");
    fprintf(stderr, "  -rootIcon\n");
    fprintf(stderr, "  -iconGravity\n");
    fprintf(stderr, "  -gravityOrder\n");
    fprintf(stderr, "  -state <NormalState | IconicState>\n");
    fprintf(stderr, "  -sticky\n");
    exit(1);
    /*NOTREACHED*/
}

main(argc, argv) 
    int argc;
    char **argv;
{
    int options = 0;
    char *display_name = NULL;
    int state = NormalState;
    int geomX, geomY, geomWidth, geomHeight;
    int igeomX, igeomY, igeomWidth, igeomHeight;
    int rootIcon = False;
    int iconGravity = False;
    int sticky = False;
    int iconified = False;
    int	gravityOrder = 0;
    char *machine = NULL;
    int i;
    char *cmd = NULL;
    Atom __SWM_START;
    SWMStart *sp;
    int len;

    program_name=argv[0];

    for (i = 1; i < argc; i++)
    {
	if (!strcmp("-help", argv[i]))
	{
	    usage();
	}
	if (!strcmp ("-display", argv[i]))
	{
	    if (++i>=argc) usage ();
	    display_name = argv[i];
	    continue;
	}
	if (!strcmp("-state", argv[i]))
	{
	    if (++i>=argc) usage ();
	    if (!strcmp(argv[i], "NormalState"))
		state = NormalState;
	    else if (!strcmp(argv[i], "IconicState"))
		state = IconicState;
	    else
		usage();
	    options |= STATE_OPTION;
	    continue;
	}
	if (!strcmp("-geometry", argv[i]))
	{
	    if (++i>=argc) usage();
	    XParseGeometry(argv[i], &geomX, &geomY, &geomWidth, &geomHeight);
	    options |= GEOM_OPTION;
	    continue;
	}
	if (!strcmp("-iconGeometry", argv[i]))
	{
	    if (++i>=argc) usage();
	    XParseGeometry(argv[i], &igeomX, &igeomY, &igeomWidth, &igeomHeight);
	    iconified = True;
	    continue;
	}
	if (!strcmp("-rootIcon", argv[i]))
	{
	    rootIcon = True;
	    continue;
	}
	if (!strcmp("-iconGravity", argv[i]))
	{
	    iconGravity = True;
	    continue;
	}
	if (!strcmp("-gravityOrder", argv[i]))
	{
	    if (++i>=argc) usage();
	    gravityOrder = atoi(argv[i]);
	    continue;
	}
	if (!strcmp("-sticky", argv[i]))
	{
	    sticky = True;
	    continue;
	}
	if (!strcmp("-cmd", argv[i]))
	{
	    if (++i>=argc) usage();
	    cmd = argv[i];
	    options |= CMD_OPTION;
	    continue;
	}
	if (!strcmp("-machine", argv[i]))
	{
	    if (++i>=argc) usage();
	    machine = argv[i];
	    continue;
	}
	usage();
    } 

    if (options != OPTIONS)
	usage();

    dpy = XOpenDisplay(display_name);
    if (!dpy)
    {
	fprintf(stderr, "%s:  unable to open display '%s'\n",
		program_name, XDisplayName (display_name));
	usage ();
    }
    screen = DefaultScreen(dpy);
    root = RootWindow(dpy, screen);

    __SWM_START = XInternAtom(dpy, "__SWM_START", False);
    len = sizeof(SWMStart);
    len += strlen(cmd) + 1;
    if (machine)
	len += strlen(machine) + 1;
    len = (len + 3) & ~0x03;

    sp = (SWMStart *)malloc(len);

    sp->bytes = len;
    sp->sticky = sticky;
    sp->state = state;
    sp->rootIcon = rootIcon;
    sp->iconGravity = iconGravity;
    sp->geomX = geomX;
    sp->geomY = geomY;
    sp->geomWidth = geomWidth;
    sp->geomHeight = geomHeight;
    sp->igeomX = igeomX;
    sp->igeomY = igeomY;
    sp->iconified = iconified;
    sp->gravityOrder = gravityOrder;
    sp->cmd_index = 0;
    strcpy(&sp->ptr[sp->cmd_index], cmd);
    if (machine)
    {
	sp->machine_index = strlen(cmd) + 1;
	strcpy(&sp->ptr[sp->machine_index], machine);
    }
    else
	sp->machine_index = -1;

    XChangeProperty(dpy, root, __SWM_START, __SWM_START, 8, PropModeAppend, (char *)sp, len);

    XCloseDisplay(dpy);
    exit (0);
}
