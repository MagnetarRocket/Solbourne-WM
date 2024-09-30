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
 * $Header: /x/morgul/toml/src/RCS/swmxlate.c,v 9.2 1993/08/27 16:57:42 toml Exp $
 *
 * Description:
 *	Translate geometry to current visible virtual desktop coordinates
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/

#ifndef lint
static char RCSinfo[] =
"$Header: /x/morgul/toml/src/RCS/swmxlate.c,v 9.2 1993/08/27 16:57:42 toml Exp $";
#endif lint

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <stdio.h>

char *program_name;

usage()
{
    fprintf(stderr, "usage: %s -geometry geometry_string [options]\n", program_name);
    fprintf(stderr, "  -display <display>   or   -d <display>\n");
    exit(1);
}

main(argc, argv) 
    int argc;
    char **argv;
{
    Display *dpy;
    int i;
    int screen;
    Window root;
    int transX, transY;
    char *display_name = NULL;
    char *geom = NULL;
    Atom __SWM_ORIGIN;
    Atom __SWM_VROOT;
    Atom actual_type;
    int actual_format;
    long nitems, bytesafter;
    int *data;
    int status, x, y, width, height;
    char newgeom[50];
    char tmp[50];


    program_name=argv[0];
    for (i = 1; i < argc; i++) {
	if (!strcmp ("-display", argv[i]) || !strcmp ("-d", argv[i])) {
	    if (++i>=argc) usage ();
	    display_name = argv[i];
	    continue;
	}
	if (!strncmp ("-g", argv[i], 2)) {
	    if (++i>=argc) usage ();
	    geom = argv[i];
	    continue;
	}
	usage();
    } 

    if (!geom)
	usage();

    dpy = XOpenDisplay(display_name);
    if (!dpy) {
	fprintf(stderr, "%s:  unable to open display '%s'\n",
		program_name, XDisplayName (display_name));
	usage ();
    }
    screen = DefaultScreen(dpy);
    root = RootWindow(dpy, screen);
    transX = 0;
    transY = 0;

    __SWM_ORIGIN = XInternAtom(dpy, "__SWM_ORIGIN", False);
    __SWM_VROOT = XInternAtom(dpy, "__SWM_VROOT", False);

    if (XGetWindowProperty (dpy, root, __SWM_ORIGIN,
	    0L, 1000000L, False, XA_INTEGER, &actual_type, &actual_format,
	    &nitems, &bytesafter, (unsigned char **)&data) == Success && data)
    {
	transX = data[0];
	transY = data[1];
    }
    else
    {
	Window *children;
        Window rootReturn, parentReturn;
	int numChildren;

	/* look for it in the tree */
	XQueryTree(dpy, root, &rootReturn, &parentReturn, &children, &numChildren);
	for (i = 0; i < numChildren; i++)
	{
	    Atom actual_type;
	    int actual_format;
	    long nitems, bytesafter;
	    Window *newRoot = NULL;
	    Window junkChild;

	    if (XGetWindowProperty (dpy, children[i], __SWM_VROOT,0,1,
		False, XA_WINDOW, &actual_type, &actual_format, &nitems, &bytesafter,
				    (unsigned char **) &newRoot) == Success && newRoot)
	    {
		XTranslateCoordinates(dpy, root,
		    *newRoot, 0,0, &transX, &transY, &junkChild);
	    }
	}
    }

    x = y = 0;
    status = XParseGeometry(geom, &x, &y, &width, &height);
    if (status & (XNegative | YNegative))
	printf("%s\n", geom);
    else
    {
	x += transX;
	y += transY;
	newgeom[0] = '\0';
	if ((status & (WidthValue | HeightValue)) == (WidthValue | HeightValue))
	{
	    sprintf(newgeom, "%dx%d", width, height);
	}
	sprintf(tmp, "+%d+%d", x, y);
	strcat(newgeom, tmp);
	printf("%s\n", newgeom);
    }
}
