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
 * $Header: /x/morgul/toml/src/RCS/swmcmd.c,v 9.3 1993/08/27 16:55:35 toml Exp $
 *
 * Description:
 *	External swm command interface
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/

#ifndef lint
static char RCSinfo[] =
"$Header: /x/morgul/toml/src/RCS/swmcmd.c,v 9.3 1993/08/27 16:55:35 toml Exp $";
#endif  /* lint */

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/Xatom.h>


Display *dpy;			/* which display are we talking to */
Window root;

main(argc, argv)
    int argc;
    char *argv[];
{
    int i;
    char buff[3000];
    Atom swmCommand;

    if (argc < 2)
    {
	fprintf(stderr, "Usage: %s swm_command(s)\n", argv[0]);
	exit(1);
    }

    if ((dpy = XOpenDisplay("")) == NULL)
    {
	fprintf(stderr, "swmcmd: can't open the display\n");
	exit(1);
    }

    buff[0] = '\0';
    for (i = 1; i < argc; i++)
    {
	strcat(buff, argv[i]);
	strcat(buff, " ");
    }

    root = DefaultRootWindow(dpy);
    swmCommand = XInternAtom(dpy, "__SWM_COMMAND", False);
    XChangeProperty(dpy, root, swmCommand, XA_STRING, 8,
	PropModeReplace, buff, strlen(buff));

    XSync(dpy, 0);
}
