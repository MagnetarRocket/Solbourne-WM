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
 * $Id: text.C,v 9.7 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Text object routines
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/

#ifndef lint
static char RCSinfo[] =
"$Id: text.C,v 9.7 1993/08/27 16:58:15 toml Exp $";
#endif /* lint */

#include "swm.H"
#include "main.H"
#include "object.H"
#include "text.H"
#include "util.H"
#include "parse.H"
#include "debug.H"
#include "wmdata.H"
#include "execute.H"
#include "quarks.H"


void
wmCreateText(
    wmObject *op
    )
{
    char *ptr;
#ifdef DEBUG
    if (wmDebug)
	fprintf(dfp, "wmCreateText: \"%s\"\n", op->name);
#endif
    if (op->wname || op->iname || op->size)
	return;

    RM->pushq(wmQuarks->textName(), wmQuarks->textClass());
    ptr = RM->get_resourceq(op->quark, op->quark);
    RM->pop();

    if (ptr == NULL)
    {
	fprintf(stderr, "swm: wmCreateText: no text definition for \"%s\"\n",
	    op->name);
	wmDone();
    }
    wmParse("wmStrings", ptr);
    if (wmParseError)
    {
	fprintf(stderr, "swm: wmCreateText: error parsing text \"%s\"\n",
	    op->name);
	wmDone();
    }
    op->u.t->lines = wmStringList.count;
    op->u.t->text = (char **)calloc(op->u.t->lines, sizeof(char *));
    op->u.t->len = (int *)calloc(op->u.t->lines, sizeof(int));
    for (int i = 0; i < op->u.t->lines; i++)
    {
	op->u.t->text[i] = (char *)wmStringList.get();
	op->u.t->len[i] = strlen(op->u.t->text[i]);
    }
}

OI_d_tech *
wmInstantiateText(
    wmObject *op
    )
{
    static char *t = ".text";
    OI_entry_field *ep;
    OI_d_tech *dp;

#ifdef DEBUG
    if (wmDebug)
	fprintf(dfp, "wmInstantiateText: \"%s\"\n", op->name);
#endif
    RM->pushq(wmQuarks->textName(), wmQuarks->textClass());
    RM->pushq(op->quark, op->quark);

    if (op->iname)
    {
	int length = 2;
	char *ptr = RM->get_resourceq(wmQuarks->maxIconLabelName(), wmQuarks->maxIconLabelClass());
	if (ptr && wmClient)
	    wmClient->set_icon_label_length(atoi(ptr));
	if (wmClient)
	    length = wmClient->icon_label_length();
	ep = oi_create_entry_field(NULL,length, NULL, "", 200);
	ep->disallow_underline();
	dp = (OI_d_tech *)ep;
    }
    else if (op->wname)
    {
	ep = oi_create_entry_field(NULL,2, NULL, "", 200);
	ep->disallow_underline();
	dp = (OI_d_tech *)ep;
	ep->set_entry_chk((OI_ef_entry_chk_fnp)wmNewWindowName, wmClient);
    }
    else if (op->size)
    {
	dp = (OI_d_tech *)oi_create_static_text(NULL," 0 x 0 ");
    }
    else
    {
	int width = 0;
	for (int i = 0; i < op->u.t->lines; i++)
	{
	    if (op->u.t->len[i] > width)
		width = op->u.t->len[i];
	}
	OI_multi_text *mp = oi_create_multi_text(NULL,op->u.t->lines,
	    width, op->u.t->lines, width);
	for (i = 0; i < op->u.t->lines; i++)
	{
#ifdef DEBUG
	    if (wmDebug)
		fprintf(dfp, "  replacing line %d with \"%s\"\n",
		    i, op->u.t->text[i]);
#endif
	    mp->insert_line(-1, "");
	    mp->replace_line(i, op->u.t->text[i]);
	}

	dp = (OI_d_tech *)mp;
    }

    op->oi = dp;
    wmGetStandardResources(op);
    wmGetBindings(op);

    RM->pop();
    RM->pop();

    return (dp);
}

wmTextInfo::wmTextInfo()
{
    lines = 0;
    text = NULL;
    len = NULL;
}
