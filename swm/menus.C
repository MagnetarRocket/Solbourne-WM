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
 * $Id: menus.C,v 9.12 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Menu routines
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/

#ifndef lint
static char RCSinfo[] =
"$Id: menus.C,v 9.12 1993/08/27 16:58:15 toml Exp $";
#endif /* lint */

#include "swm.H"
#include "debug.H"
#include "util.H"
#include "parse.H"
#include "list.H"
#include "execute.H"
#include "wmdata.H"
#include "gram.H"
#include "bitmap.H"
#include "events.H"
#include "init.H"
#include "main.H"
#include "quarks.H"

static void menuFire(OI_menu_cell *, void *, OI_number);

Window wmMenuWindow = NULL;
static wmBindings *bps;

OI_button_menu *
wmGetMenu(
    char *name,
    XrmQuark quark_name,
    int button,
    int mods
    )
{
    char *ptr;
    wmList list;
    OI_button_menu *pull;
    OI_button_menu *pop;
    wmBinding *bp;
    wmBindings *mybps;
    wmObjectData *odp;

    if (name == NULL)
	return (NULL);

#ifdef DEBUG
    if (wmDebug)
	fprintf(dfp, "wmGetMenu: \"%s\"\n", name);
#endif
    RM->pushq(wmQuarks->menuName(), wmQuarks->menuClass());
    ptr = RM->get_resourceq(quark_name, quark_name);
    RM->pop();
    if (ptr == NULL)
	return (NULL);

    if (XFindContext(DPY, (Window)ptr, wmBindingsContext, (caddr_t*)&mybps) == XCNOENT)
    {
	bps = mybps = new wmBindings(ptr);
	wmParse("wmMenu", ptr);
	if (wmParseError)
	{
	    fprintf(stderr, "swm: wmGetMenu: error parsing menu \"%s\"\n", name);
	    delete mybps;
	    return (NULL);
	}
	XSaveContext(DPY, (Window)ptr, wmBindingsContext, (caddr_t)bps);
    }

    bps = mybps;
    // if there are no menu entries, leave
    if (mybps->bindings->count == 0)
	return (NULL);

    // allocate space for the cells
    OI_menu_cell **cell = (OI_menu_cell **)calloc(mybps->bindings->count,
	sizeof(OI_menu_cell *));
    int count = mybps->bindings->count;
    char *title = NULL;
    int pin = False;

    int i;
    for (i = 0, bp = (wmBinding *)mybps->bindings->first(); i < count;
	i++, bp = (wmBinding *)mybps->bindings->next())
    {
	wmFunction *fp;
	pull = NULL;
	int nop = False;
	for (fp = (wmFunction *)bp->functions->first(); fp != NULL;
	    fp = (wmFunction *)bp->functions->next())
	{
	    // is this a pull right?
	    if (fp->function == F_MENU)
	    {
		if (fp->argc)
		{
		    // yup, go get the next menu
		    pull = wmGetMenu(fp->argv[0], fp->argvq, button, mods);
		}
		else
		    fp->function = F_NOP;
	    }
	    if (fp->function == F_NOP)
		nop = True;
	}

	// if this is the first entry and no title has been set yet,
	// look for a title
	if (i == 0 && !title)
	{
	    for (fp = (wmFunction *)bp->functions->first(); fp != NULL;
		fp = (wmFunction *)bp->functions->next())
	    {
		if (fp->function == F_TITLE || fp->function == F_TITLEPIN)
		{
		    // got a title
		    title = bp->str;
		    i -= 1;
		    count -= 1;
		    if (fp->function == F_TITLEPIN)
			pin = True;
		    break;
		}
	    }
	}

	// if i is equal to -1, then we got a title, so let's just go back
	// around
	if (i == -1)
	    continue;

	if (bp->str[0] == '@')
	{
	    wmBitmap *wbm;
	    if ((wbm = wmFindBitmap(&bp->str[1])) != NULL)
	    {
		cell[i] = oi_create_menu_cell(NULL, wbm->path, (OI_action_fnp)menuFire, (char *)bp, OI_ICON_CELL);
	    }
	    else
		bp->str = &bp->str[1];
	}
	if (!cell[i])
	{
	    cell[i] = oi_create_menu_cell(NULL,bp->str, (OI_action_fnp)menuFire, (char *)bp, OI_TEXT_CELL);
	}
	cell[i]->set_data((void *)bp);
	/* +++
	if (nop)
	    cell[i]->set_state(OI_INACTIVE);
	*/

	if (pull)
        {
            pull->set_associated_object(cell[i], OI_DEF_LOC, OI_DEF_LOC, OI_ACTIVE_NOT_DISPLAYED);
            odp = (wmObjectData *)pull->data();
            odp->parent = cell[i];
        }
    }
    pop = oi_button_menu(name, count, cell, OI_VERTICAL, title);
    if (pin)
    {
	pop->allow_pushpin();
    }

    odp = new wmObjectData(NULL);
    pop->set_data((void *)odp);
    pop->set_menu_trigger(button, mods);
    pop->set_trigger(button, mods);

    RM->pushq(wmQuarks->menuName(), wmQuarks->menuClass());
    RM->pushq(quark_name, quark_name);
    wmGetOIResources((OI_d_tech *)pop);
    RM->pop();
    RM->pop();
    free((char *)cell);

    return (pop);
}

void
wmMenuEntryDone()
{
#ifdef DEBUG
    if (wmDebug)
	fprintf(dfp, "wmMenuEntryDone\n");
#endif
    bps->bindings->append((ent)wmBind);
}

static void
menuFire(
    OI_menu_cell *cp,
    void *argp,
    OI_number)
{
    Window win = NULL;

    wmBinding *bp = (wmBinding *)argp;
    OI_button_menu *pop = (OI_button_menu *)cp->parent();
    wmObjectData *odp = (wmObjectData *)pop->data();
    wmData *wp = odp->wp;
    if (odp->inAnIcon && wp)
	win = odp->wp->oi_icon()->X_window();
    else if (wp)
	win = odp->wp->oi_frame()->X_window();

    wmFindScreen(odp->wp, win ? win : cp->X_window());

    Window junkRoot, junkChild;
    int x_root, y_root, junkX, junkY;
    unsigned int junkMask;

    XQueryPointer(DPY, wmScr->root, &junkRoot, &junkChild, &x_root, &y_root, &junkX, &junkY, &junkMask);

    wmMenuWindow = cp->X_window();
    wmExecuteBinding(wp, win, bp, FROM_MENU, x_root, y_root);
    wmMenuWindow = NULL;
}
