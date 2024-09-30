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
 * $Id: button.C,v 9.8 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Button object routines
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/

#ifndef lint
static char RCSinfo[] =
"$Id: button.C,v 9.8 1993/08/27 16:58:15 toml Exp $";
#endif /* lint */

#include "swm.H"
#include "main.H"
#include "parse.H"
#include "object.H"
#include "list.H"
#include "panel.H"
#include "button.H"
#include "bitmap.H"
#include "util.H"
#include "debug.H"
#include "execute.H"
#include "wmdata.H"
#include "init.H"
#include "quarks.H"

static int firstButton = True;
#define Q_NONE		0
#define	Q_TOP		1
#define Q_BOTTOM	2
#define Q_DOWN		3
#define Q_BACKGROUND	4
#define Q_FOREGROUND	5
#define Q_MAX		6
static XrmQuark		quarks[Q_MAX];
static OI_pic_pixel	pixels[Q_MAX];

/***************************************************************
 *
 *  Procedure: 	wmCreateButton
 *
 *  Function:	Find out what will be placed in the button
 *
 ***************************************************************/

void
wmCreateButton(
    wmObject *op	    // the object pointer to rhe button
    )
{
#ifdef DEBUG
    if (wmDebug)
	fprintf(dfp, "wmCreateButton \"%s\"\n", op->name);
#endif

    if (firstButton) 
    {
	firstButton = False;
	quarks[Q_NONE] = XrmStringToQuark("none");
	quarks[Q_TOP] = XrmStringToQuark("top");
	quarks[Q_BOTTOM] = XrmStringToQuark("bottom");
	quarks[Q_DOWN] = XrmStringToQuark("down");
	quarks[Q_BACKGROUND] = XrmStringToQuark("background");
	quarks[Q_FOREGROUND] = XrmStringToQuark("foreground");

	pixels[Q_NONE] = OI_PIC_NONE;
	pixels[Q_TOP] = OI_PIC_TOP;
	pixels[Q_BOTTOM] = OI_PIC_BOTTOM;
	pixels[Q_DOWN] = OI_PIC_DOWN;
	pixels[Q_BACKGROUND] = OI_PIC_BG;
	pixels[Q_FOREGROUND] = OI_PIC_FG;
    }

    if (op->wname || op->iname || op->iimage)
	return;

}

/***************************************************************
 *
 *  Procedure: 	wmInstantiateButton
 *
 *  Function:	Actually create the OI object that will
 *		represent the button
 *
 ***************************************************************/

OI_button_menu *
wmInstantiateButton(
    wmObject *op	    // the object pointer to the button
    )
{
    char *ptr, *label;
    OI_glyph *glyph;

#ifdef DEBUG
    if (wmDebug)
	fprintf(dfp, "wmInstantiateButton: \"%s\"\n", op->name);
#endif

    label = op->name;
    RM->pushq(wmQuarks->buttonName(), wmQuarks->buttonClass());
    ptr = RM->get_resourceq(op->quark, op->quark);
    RM->pushq(op->quark, op->quark);
    op->u.b = wmStringToButtonInfo(op->name, ptr);

    // if it's the icon image, just create a box for it
    op->oi = NULL;
    if (op->iimage)
    {
	op->oi = (OI_box *)oi_create_box(NULL,1, 1);
    }
    else if (op->wname || op->iname)
    {
	if (op->iname)
	{
	    char *ptr = RM->get_resourceq(wmQuarks->maxIconLabelName(), wmQuarks->maxIconLabelClass());
	    if (ptr && wmClient)
		wmClient->set_icon_label_length(atoi(ptr));
	}
	OI_static_text *sp = oi_create_static_text(NULL," ");
	sp->disallow_cut_paste();
	op->oi = (OI_d_tech *)sp;
    }
    else
    {
	if (op->u.b->pic_spec)
	{
	    op->oi = oi_create_glyph(NULL, op->u.b->count, op->u.b->pic_spec, NULL, op->u.b->width, op->u.b->height, OI_NO, OI_NO);
	    glyph = (OI_glyph *)op->oi;
	    glyph->set_active_bkg(OI_PIC_BG);
	}

	if (op->oi == NULL)
	{
	    OI_static_text *sp = oi_create_static_text(NULL,op->u.b->label);
	    sp->disallow_cut_paste();
	    op->oi = (OI_d_tech *)sp;
	}
    }

    wmGetStandardResources(op);
    wmGetBindings(op);

    RM->pop();
    RM->pop();

    return ((OI_button_menu *)op->oi);
}

wmButtonInfo::wmButtonInfo()
{
    label = NULL;
    count = 0;
    pic_spec = NULL;
}

wmButtonInfo *
wmStringToButtonInfo(char *name, char *ptr)
{
    char *original;
    int i;
    XrmQuark inactive;
    XrmQuark active;
    wmButtonInfo *bi;
    pixmapMask pix[50];
    int error;
    int first;
    int pix_num;
    int found;
    wmBitmap *wbm;

    bi = NULL;
    original = ptr;
    if (ptr)
    {
	if (XFindContext(DPY, (Window)original, wmScr->buttons, (caddr_t*)&bi) == XCNOENT)
	{
	    wmParse("wmStrings", ptr);
	    if (!wmParseError)
	    {
		bi = new wmButtonInfo();
		error = False;
		first = True;
		if (wmStringList.count > 1)
		{
		    if ((wmStringList.count % 3) == 0)
		    {
			pix_num = 0;
			bi->count = wmStringList.count/3;
			while ((ptr = (char *)wmStringList.get()) != NULL)
			{
			    if (first)
			    {
				bi->label = ptr;
				first = False;
			    }
			    inactive = XrmStringToQuark((char *)wmStringList.get());
			    active = XrmStringToQuark((char *)wmStringList.get());
			    pix[pix_num].label = ptr;
			    pix[pix_num].pixmap = None;
			    found = 0;
			    for (i = 0; i < Q_MAX; i++)
			    {
				if (active == quarks[i])
				{
				    pix[pix_num].active = pixels[i];
				    if (++found == 2)
					    break;
				}
				if (inactive == quarks[i])
				{
				    pix[pix_num].inactive = pixels[i];
				    if (++found == 2)
					    break;
				}
			    }
			    if (found != 2)
			    {
				error = True;
				break;
			    }
			    pix_num++;
			}
		    }
		    if (error)
			bi->count = 0;
		}
		if (wmStringList.count)
		{
		    while ((ptr = (char *)wmStringList.get()) != NULL)
		    {
			if (first)
			{
			    bi->count = 1;
			    bi->label = ptr;
			    pix[0].label = ptr;
			    pix[0].active = OI_PIC_FG;
			    pix[0].inactive = OI_PIC_FG;
			    first = False;
			}
		    }
		}
		if (bi->label[0] == '@')
		{
		    // create the pic_spec describing the glyph
		    bi->pic_spec = (OI_pic_spec_mask **)malloc(sizeof(OI_pic_spec_mask *) * bi->count);
		    for (i = 0; i < bi->count; i++)
		    {
			if ((wbm = wmFindBitmap(pix[i].label)) == NULL)
			{
			    free((char *)bi->pic_spec);
			    bi->pic_spec = NULL;
			    break;
			}
			bi->pic_spec[i] = oi_create_pic_pixmap_mask(wbm->pixmap, pix[i].inactive, pix[i].active);
			bi->width = wbm->width;
			bi->height = wbm->height;
		    }
		}
		XSaveContext(DPY, (Window)original, wmScr->buttons, (caddr_t)bi);
	    }
	}
    }
    else
    {
	if (XFindContext(DPY, (Window)name, wmScr->buttons, (caddr_t*)&bi) == XCNOENT)
	{
	    bi = new wmButtonInfo();
	    bi->label = name;
	    XSaveContext(DPY, (Window)name, wmScr->buttons, (caddr_t)bi);
	}
    }
    return (bi);
}
