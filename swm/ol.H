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
 * $Id: ol.H,v 9.5 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Open Look definitions
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/


#ifndef OL_H
#define OL_H

typedef struct {                        /* _OL_WIN_ATTR */
        Atom    win_type;
        Atom    menu_type;
        Atom    pin_initial_state;
} ShortOLWinAttr;

typedef struct {                        /* _OL_WIN_ATTR */
	unsigned	flags;
        Atom    win_type;
        Atom    menu_type;
        Atom    pin_initial_state;
	unsigned	cancel;
} LongOLWinAttr;

typedef struct OLDfltBtn {              /* _OL_DFLT_BTN */
        int	warp_x;
        int	warp_y;
        int	button_x;
        int	button_y;
        int	button_width;
        int	button_height;
} OLDfltBtn;


enum OLDecoration
{
    OL_HEADER	= 0x01,
    OL_CLOSE	= 0x02,
    OL_PIN	= 0x04,
    OL_RESIZE	= 0x08
};

#endif /* OL_H */
