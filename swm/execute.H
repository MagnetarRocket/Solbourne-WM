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
 * $Id: execute.H,v 9.5 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Command exectution definitions
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/


#ifndef EXECUTE_H
#define EXECUTE_H

#define SWM_BUTTONS 6			/*  0 == BTN */
#define SWM_MODIFIERS 16

#define PRESS 0
#define CLICK_MASK 0x10
#define CLICK1 (CLICK_MASK | 1)
#define CLICK2 (CLICK_MASK | 2)
#define CLICK3 (CLICK_MASK | 3)

#define FROM_MENU 0
#define NOT_FROM_MENU 1

	// forward reference declarations
	struct wmData;
	class OI_d_tech;

struct wmBindings
{
    char *resource;			// XRDB pointer
    wmList *bindings;			// list of bindings
    char *button_helps[SWM_BUTTONS][SWM_MODIFIERS];	// button help strings
    char helpConstructed;		// have the help strings been constructed

    wmBindings(char *);
};

struct wmBinding
{
    struct wmBindings *next;
    unsigned mods;
    int what;				// what caused the invocation
    KeySym keysym;
    KeyCode keycode;
    OI_button_menu *pop;		// pop up menu
    int from;				// PRESS of CLICK
    char *str;
    wmList *functions;
    char move;				// a move or resize function
    char menu;				// a menu function
    char expanded;

    wmBinding();
};

struct wmFunction
{
    int function;
    char **argv;
    int argc;
    XrmQuark argvq;			// quarkified argv[0]
};

struct wmMacro
{
    char *name;
    wmBinding *bp;
};

extern int wmMoveOpaque;
extern int wmExecute(struct wmData *, Window, int, char **, int, int, int);
extern void wmExternalExecute(struct wmData *, Window, int, char **, int, int, int);
extern void wmGetBindings(wmObject *);
extern void wmAddBinding(unsigned, int, char *);
extern void wmBindingDone();
extern void wmHandlePress( XButtonEvent *, OI_d_tech *);
extern void wmHandleKeyPress(XEvent *, OI_d_tech *);
extern void wmAddFunction(int, char **, int);
extern void wmExecuteBinding(wmData *, Window, wmBinding *, int, int, int);
extern wmBinding *wmBind;
extern wmList wmBindingsList;
extern void wmHandleDrop(XPropertyEvent *, OI_d_tech *);

#endif /* EXECUTE_H */
