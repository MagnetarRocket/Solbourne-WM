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
 * $Id: list.C,v 9.6 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Generic list routines
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/

#ifndef lint
static char RCSinfo[] =
"$Id: list.C,v 9.6 1993/08/27 16:58:15 toml Exp $";
#endif /* lint */

#include "swm.H"
#include "list.H"
#include "main.H"
#include "debug.H"

wmList wmStringList;	    // a list of strings
wmList wmPanelKidsList;	    // the list of panel kids
wmList wmRegionKidsList;    // the list of region kids

void
wmList::insert(ent a)
{
    if (head)
	head = new wmItem(a, head);
    else
    {
	head = new wmItem(a, NULL);
	tail = head;
    }
    count++;
}

// should return a boolean
// insert b before a
Bool
wmList::insert(ent a, ent b)
{
    wmItem *tmp, *p;
    Bool success = False;

    // special case, only 1 element
    if (head->e == a)
    {
	head = new wmItem(a, head);
	count++;
	success = True;
    }
    else
    {
        // look for it
        for (tmp = head->next, p = head; tmp != NULL; tmp = tmp->next)
        {
	    if (tmp->e == a)
	    {
	        p->next = new wmItem(b, tmp);
                count++;
	        success = True;
	    }
	    else
	       p = tmp;
        }
    }
    return(success);
}

void
wmList::append(ent a)
{
#ifdef DEBUG
    if (wmDebug)
	fprintf(dfp, "wmList::append\n");
#endif
    if (tail)
    {
	tail->next = new wmItem(a, NULL);
	tail = tail->next;
    }
    else
    {
	head = new wmItem(a, NULL);
	tail = head;
    }
    count++;
}

// append b following a
Bool
wmList::append(ent a, ent b)
{
    wmItem *tmp, *p;
    Bool success = False;

    // look for it
    for (tmp = head; tmp != NULL; tmp = tmp->next)
    {
	if (tmp->e == a)
	{
	    p = tmp->next;
	    tmp->next = new wmItem(b, p);
    	    count++;
	    success = True;
	    if (tmp == tail)
		tail = tail->next;
	}
    }
    return(success);
}

ent
wmList::first()
{
    //fprintf(dfp, "wmList::first\n");
    if (head == NULL)
	return NULL;
    current = head;

    //fprintf(dfp, "first: current = %d\n", current);
    return (current->e);
}

ent
wmList::next()
{
    //fprintf(dfp, "wmList::next\n");
    if (current == NULL)
	return NULL;
    current = current->next;
    //fprintf(dfp, "next: current = %d\n", current);
    if (current == NULL)
	return NULL;

    return (current->e);
}

ent
wmList::get()
{
    if (head == NULL)
	return NULL;

    wmItem *f = head;
    ent r = f->e;

    //printf("get \"%s\"\n", (char *)r);
    if (head->next == NULL)
    {
	head = NULL;
	tail = NULL;
    }
    else
	head = f->next;

    delete f;
    count--;
    return r;
}

void
wmList::rm(ent a)
{
    wmItem *tmp, *p = NULL;

    // look for it
    for (tmp = head; tmp != NULL; tmp = tmp->next)
    {
	if (tmp->e == a)
	{
	    if (tmp == head)
		head = tmp->next;
	    if (tmp == tail)
		tail = p;

	    if (p)
		p->next = tmp->next;

	    delete tmp;

	    if (--count == 0)
	    {
		head = NULL;
		tail = NULL;
	    }
	    return;
	}
	p = tmp;
    }
}
