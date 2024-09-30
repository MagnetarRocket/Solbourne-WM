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
 * $Id: swm.H,v 9.10 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Include all other needed files
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/


#ifndef SWM_H
#define SWM_H

#include <stdio.h>
#include <OI/oi.H>
#include <OI/cvt.H>
#include <OI/Xlib.H>
#include <OI/Xutil.H>
#include <X11/Xatom.h>
#include <X11/Xos.h>
#include <X11/cursorfont.h>
#include <stdlib.h>

#if defined(ultrix) || defined(hpux)
#include <unistd.h>
#else
#include <sysent.h>
#endif

#ifndef hpux
#include <math.h>
#endif

#ifdef SHAPE
#include "shape.H"
#endif

#define DPY (wmConn->display())
#define RM (wmScr->conp->rm())

#define SWM_DEFAULTS "/usr/lib/X11/swm/"
#define SWM_VERSION "3.0"

#ifndef SIGPROCTYPE
#define SIGPROCTYPE void
#endif

typedef SIGPROCTYPE (*SigProc)();


#endif /* SWM_H */
