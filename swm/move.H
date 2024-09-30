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
 * $Id: move.H,v 9.4 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Window move definitions
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/


#ifndef MOVE_H
#define MOVE_H

#define IN_PANNER	0x80000000
#define OUT_PANNER	0x40000000

extern int wmDoingMove;

extern int wmMoveClient( XButtonEvent *,wmData *);
extern int wmMoveIcon( XButtonEvent *, wmData *);
extern void wmStartMove(wmData *, OI_d_tech *, int *, int *, int *, int, int=OUT_PANNER,unsigned=0,unsigned=0);
extern void wmSendEvent(wmData *);

#endif /* MOVE_H */
