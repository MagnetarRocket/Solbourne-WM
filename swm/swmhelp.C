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
 * $Id: swmhelp.C,v 9.6 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	swm help support routines
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/

#ifndef lint
static char RCSinfo[] =
"$Id: swmhelp.C,v 9.6 1993/08/27 16:58:15 toml Exp $";
#endif /* lint */

#include "swm.H"
#include "main.H"
#include "list.H"
#include "object.H"
#include "execute.H"
#include "gram.H"
#include "hm.H"
#include "atoms.H"

struct _helpData {
    int			function;
    char		*help;
};

static struct _helpData helpDefaults[] = {
{ F_RAISE,			"Raise window", },
{ F_LOWER,			"Lower window", },
{ F_ICONIFY,			"Iconify window", },
{ F_DEICONIFY,			"Deiconify window", },
{ F_RESTART,			"Restart the window manager", },
{ F_QUIT,			"Quit the window manager", },
{ F_MOVE,			"Move window", },
{ F_FORCEMOVE,			"Force move a window", },
{ F_MOVEOPAQUE,			"Move window opaque", },
{ F_RESIZE,			"Resize window", },
{ F_GROUP,			"Group windows", },
{ F_UNGROUP,			"Ungroup window", },
{ F_REGROUP,			"Regroup window", },
{ F_BEEP,			"Sound bell", },
{ F_CIRCLEDOWN,			"Circulate down", },
{ F_CIRCLEUP,			"Circulate up", },
{ F_EXEC,			"Execute", },
{ F_FOCUS,			"Set input focus", },
{ F_IDENTIFY,			"Not implemented", },
{ F_NOP,			"No op", },
{ F_RAISELOWER,			"Raise-lower window", },
{ F_REFRESH,			"Refresh screen", },
{ F_UNFOCUS,			"Clear input focus", },
{ F_WINREFRESH,			"Refresh window", },
{ F_WARPTO,			"Not implemented", },
{ F_SCROLL,			"Scroll Virtual Desktop", },
{ F_SCROLLLEFT,			"Scroll Virtual Desktop left", },
{ F_SCROLLRIGHT,		"Scroll Virtual Desktop right", },
{ F_SCROLLUP,			"Scroll Virtual Desktop up", },
{ F_SCROLLDOWN,			"Scroll Virtual Desktop down", },
{ F_WARPHORIZONTAL,		"Warp pointer horizontally", },
{ F_WARPVERTICAL,		"Warp pointer vertically", },
{ F_WARPTOSCREEN,		"Warp pointer to next screen", },
{ F_SAVEPOINTER,		"Save pointer location", },
{ F_RESTOREPOINTER,		"Restore pointer location", },
{ F_MENU,			"Pop up menu", },
{ F_TITLE,			"Not implemented", },
{ F_TITLEPIN,			"Not implemented", },
{ F_STOP,			"Stop if successful", },
{ F_FALSESTOP,			"Stop if not successful", },
{ F_PACK,			"Pack icon panel", },
{ F_SQUEEZE,			"Squeeze icon panel", },
{ F_KILL,			"Kill client", },
{ F_DELETE,			"Delete client window", },
{ F_SHUTDOWN,			"Shutdown client", },
{ F_MAP,			"Map object", },
{ F_UNMAP,			"Unmap object", },
{ F_MACRO,			"Macro", },
{ F_STRING,			"Not implemented", },
{ F_ZOOM,			"Zoom window", },
{ F_HORIZOOM,			"Zoom window horizontally", },
{ F_VERTZOOM,			"Zoom window vertically", },
{ F_SAVE,			"Save location and size of window", },
{ F_RESTORE,			"Restore window size and location", },
{ F_PANNER,			"Make the panner visible", },
{ F_STICK,			"Stick window", },
{ F_UNSTICK,			"Unstick window", },
{ F_SCROLLHOME,			"Scroll Virtual Desktop home", },
{ F_RECONFIG,			"Not implemented", },
{ F_GETRESOURCE,		"Not implemented", },
{ F_PIN,			"Pin window", },
{ F_UNPIN,			"Unpin window", },
{ F_QUERY,			"Query window attributes", },
{ F_SCROLLTO,			"Scroll Virtual Desktop to window", },
{ F_SAVEVROOT,			"Save Virtual Desktop location", },
{ F_RESTOREVROOT,		"Restore Virtual Desktop location", },
{ F_WARPSAVE,			"Save the pointer location and warp to last saved location", },
{ F_SCROLLSAVE,			"Save the Virtual Desktop location and scroll to the last saved location", },
{ F_PLACES,			"Save window layout", },
{ F_CONSTRAINMOVE,		"Move window constrained", },
{ F_REBIND,			"Rebind button", },
{ F_NEWBUTTONIMAGE,		"Change button", },
{ F_ANIMATEBUTTON,		"Animate button", },
{ F_DUMP,			"Dump object hierarchy", },
{ F_AUTORESIZE,			"Auto resize", },
{ F_SWEEP,			"Sweep a group of windows", },
{ F_UNSWEEP,			"Forget a swept group", },
{ F_HARDRESTART,		"Hard restart", },
{ F_HANDLEDROP,			"Handle a dropped file", },
{ F_ROUNDUP,			"Pull negative windows back on the screen", },
{ F_SETDESKTOP,			"Set the desktop appearance", },
{ F_FREEZEDESKTOP,		"Freeze the desktop appearance", },
{ F_THAWDESKTOP,		"Unfreeze the desktop appearance", },
{ F_GRAVITY,			"Gravitize an icon", },
{ F_RESHUFFLE,			"Re-shuffle icons", },
{ F_INFO,			"Show information about swm", },
{ F_OWNER,			"Show the owner of a transient window", },
{ F_FOCUSCOLOR,			"Set the colormap focus on a specific window", },
{ F_TERMINALPLACES,		"Save X terminal window layout"},
};

#define num_helps sizeof(helpDefaults)/sizeof(_helpData)
static int *helpIndex;

void
wmInitHelp()
{
    int largest;
    int i;

    largest = 0;
    for (i = 0; i < num_helps; i++)
	if (helpDefaults[i].function > largest)
	    largest = helpDefaults[i].function;

    // YACC tokens start at 256
    largest -= 255;

    helpIndex = (int *)malloc(largest * sizeof(int));

    for (i = 0; i < num_helps; i++)
	helpIndex[helpDefaults[i].function-256] = i;
}

void
wmBuildHelp(
    wmBindings *bps,
    wmBinding *bp
    )
{
    wmFunction *fp;
    char str[500];
    char temp[100];
    int first;
    int func;
    int mods;
    char *ptr;
    int len;

    mods = bp->mods & 0xff;
    first = True;
    ptr = NULL;
    str[0] = '\0';
    for (fp = (wmFunction *)bp->functions->first(); fp != NULL; fp = (wmFunction *)bp->functions->next())
    {
	func = helpIndex[fp->function-256];
	if (!first)
	    strcat(str,", ");
	if (fp->argc)
	{
	    sprintf(temp, "%s \"%s\"", helpDefaults[func].help, fp->argv[0]);
	    strcat(str, temp);
	}
	else
	{
	    strcat(str, helpDefaults[func].help);
	}
	first = False;
    }
    if ((len = strlen(str))) {
	ptr = (char *)malloc(len+1);
	strcpy(ptr, str);

	if (bp->what == BTN)
	{
	    if (!bps->button_helps[0][mods])
		bps->button_helps[0][mods] = ptr;
	    else
		free(ptr);
	}
	else
	{
	    if (!bps->button_helps[bp->what][mods])
		bps->button_helps[bp->what][mods] = ptr;
	    else
		free(ptr);
	}
    }
}

static void
helpTimer()
{
    Window junkRoot, junkChild;
    int rootX, rootY;
    int junkX, junkY;
    unsigned int junkMask;

    // save the pointer location
    XQueryPointer(DPY, wmScr->root, &junkRoot, &junkChild, &rootX, &rootY, &junkX, &junkY, &junkMask);
}

void
wmStartHelpTimer()
{
    OI_add_timeout(500, (OI_timeout_fnp)helpTimer, NULL);
}

void
wmStopHelpTimer()
{
    OI_delete_timeout((OI_timeout_fnp)helpTimer, NULL);
}

static int	len[SWM_BUTTONS];
static char	*cp[SWM_BUTTONS];
static int	total;

static void
collapseHelp(wmBindings *bps)
{
    int i;
    wmBinding *bp;

    for (i = 0; i < SWM_BUTTONS; i++)
	if (!cp[i])
	    cp[i] = bps->button_helps[i][0];

    for (bp = (wmBinding *)bps->bindings->first(); bp != NULL; bp = (wmBinding *)bps->bindings->next())
	if (bp->next)
	    collapseHelp(bp->next);
}

void
wmHelpEnterNotify(
    XEnterWindowEvent *ev,
    OI_d_tech *oi
    )
{
    wmObjectData *odp;
    wmBindings *bps;
    HelpState *hs;
    int i, p;

    /*
    if (ev->detail == NotifyInferior)
	return;
    */

    total = 0;
    if (oi && (odp = (wmObjectData *)oi->data()) && (bps = odp->bps)) {
	    if ((hs = (HelpState *)odp->hs) == NULL) {
		for (i = 0; i < SWM_BUTTONS; i++)
		    cp[i] = NULL;
		collapseHelp(bps);
		if (cp[0])
		    for (i = 1; i <=3; i++)
			if (!cp[i])
			    cp[i] = cp[0];
		total = 0;
		for (i = 1; i < SWM_BUTTONS; i++) { 
			if (cp[i])
			    len[i] = strlen(cp[i])+1;
			else
			    len[i] = 0;
			total += len[i];
		}
		total += sizeof(HelpState);
		hs = (HelpState *)malloc(total);
		hs->hbasic.type = HelpBasic;
		hs->hbasic.override = True;
		p = 0;
		for (i = 1; i < SWM_BUTTONS; i++) {
			hs->hbasic.size[i-1] = len[i];
			if (len[i]) {
				strcpy(&hs->hbasic.text[p], cp[i]);
				p += len[i];
			}
		}
		odp->hs = (char *)hs;
		odp->hsSize = total;
	    }
	    else
		total = odp->hsSize;

	    if (hs)
		XChangeProperty(DPY, wmScr->root, HM_STATE, HM_STATE, 8, PropModeReplace, (unsigned char *)hs, total);
    }
}
