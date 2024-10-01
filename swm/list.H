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
 * $Id: list.H,v 9.7 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Generic list definitions
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/


#ifndef LIST_H
#define LIST_H

typedef void *ent;

class wmItem
{
friend class wmList;
    wmItem *next;
    ent e;
    wmItem(ent a, wmItem *p)	{ e = a; next = p; }
};

class wmList
{
    wmItem *head;		// the head of the list
    wmItem *tail;		// the tail of the list
    wmItem *current;		// current item being looked at
    wmItem *sav;		// saved current location
public:
    int count;			// how many items
    void insert(ent a);		// add at the head of the list 
    Bool insert(ent a, ent b);	// add b before a
    void append(ent a);		// add at end of the list
    Bool append(ent a, ent b);	// add b following a 
    ent first();		// return the first entry
    ent next();			// return the next entry
    ent get();			// return and remove the head of the list
    void rm(ent a);		// remove it from the list
    void init()			{ head = 0; tail = 0; count = 0; }
    void save()			{ sav = current; }
    void restore()		{ current = sav; }
    wmList()			{ head = 0; tail = 0; count = 0;}
    wmList(ent a)		{ head = new wmItem(a, 0); head->next = head; }
};

extern wmList wmStringList;
extern wmList wmPanelKidsList;
extern wmList wmRegionKidsList;

#endif /* LIST_H */
