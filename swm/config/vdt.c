/***********************************************************************
 *
 * Solbourne Computer, Inc.
 * Copyright (c) 1990, Solbourne Computer, Inc.  USA
 * All rights reserved.
 *
 * vdt.c
 *
 * Find the Virtual Desktop window
 *
 * 08-Feb-90 Thomas E. LaStrange        File created
 *
 ***********************************************************************/

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

/***********************************************************************
 *
 *  Procedure:
 *	FindVirtualDesktop
 *
 *  Arguments:
 *	dpy	- Display pointer
 *
 *  Returns:
 *	Window	- the Virtual Desktop window or the actual root window
 *
 *  Function:
 *	This routine will look for the swm Virtual Desktop window.  If
 *	it is not there, the actual root window is returned.
 *
 ***********************************************************************
 */

#ifndef NULL
#define NULL 0
#endif

Window
FindVirtualDesktop(dpy)
Display *dpy;
{
    Window root;	/* the returned root window */
    Window rootReturn;
    Window parentReturn;
    Window *children;
    unsigned int numChildren;
    int i;
    Atom __SWM_VROOT;

    /* get the default root window */
    root = RootWindow(dpy, DefaultScreen(dpy));

    /* go look for a virtual desktop */
    __SWM_VROOT = XInternAtom(dpy, "__SWM_VROOT", False);

    children = NULL;
    XQueryTree(dpy, root, &rootReturn, &parentReturn, &children, &numChildren);

    for (i = 0; i < numChildren; i++)
    {
	Atom actual_type;
	int actual_format;
	long nitems, bytesafter;
	Window *newRoot = NULL;

	if (XGetWindowProperty (dpy, children[i], __SWM_VROOT,0,1,
	    False, XA_WINDOW, &actual_type, &actual_format, &nitems, &bytesafter,
				(unsigned char **) &newRoot) == Success && newRoot)
	{
	    root = *newRoot;
	    break;
	}
    }
    if (children)
	XFree(children);

    return (root);
}
