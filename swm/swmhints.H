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
 * $Id: swmhints.H,v 9.7 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	__SWMHINTS property definition
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/


#ifndef SWMHINTS_H
#define SWMHINTS_H

typedef struct 
{
        int initial_state;      	// WithdrawnState, NormalState, or IconicState
        Window icon_window;     	// None or the root window ID
        int icon_x, icon_y;     	// position of the icon
        int gravity_order;		// order in icon region, only valid if SWMH_ICONGRAVITY
	unsigned long flags;		// various boolean flags
} SWMHints;

#define SWMH_STICKY		0x00000001
#define SWMH_ICONIFIED		0x00000002
#define SWMH_PLACED		0x00000004
#define SWMH_ICONGRAVITY	0x00000008

#endif /* SWMHINTS_H */
