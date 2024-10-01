%{
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
 * $Id: gram.Y,v 9.11 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Grammer
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/

#ifndef lint
static char RCSinfo[] =
"$Id: gram.Y,v 9.11 1993/08/27 16:58:15 toml Exp $";
#endif /* lint */

#include "swm.H"
#include "list.H"
#include "panel.H"
#include "debug.H"
#include "execute.H"
#include "menus.H"
#include "region.H"
#include "util.H"

#define PARAMETERS 100

/* extern "C" { void yyerror(char *); }*/
extern void yyerror(char *);
extern int yylex();

static wmGeometry geom;
static int mask;
static unsigned mods;
static int what;
static int argc;
static char *argv[PARAMETERS];

static void 
resetArgs()
{
    argc = 0;
    argv[0] = NULL;
}

%}

%union
{
    int num;
    char *ptr;
    char *g;
};

%token <num> F_RAISE F_LOWER F_ICONIFY F_DEICONIFY F_RESTART F_QUIT F_GROUP
%token <num> F_MOVE F_RESIZE F_BEEP F_CIRCLEDOWN F_CIRCLEUP F_UNGROUP
%token <num> F_DESTROY F_EXEC F_FOCUS F_IDENTIFY F_NOP F_RAISELOWER
%token <num> F_REFRESH F_UNFOCUS F_WINREFRESH F_WARPTO F_MENU F_TITLE F_STOP
%token <num> F_MOVEOPAQUE F_PACK F_KILL F_MAP F_UNMAP F_MACRO F_STRING
%token <num> F_ZOOM F_HORIZOOM F_VERTZOOM F_SAVE F_RESTORE F_RESTOREPOINTER
%token <num> F_WARPHORIZONTAL F_WARPVERTICAL F_SAVEPOINTER F_WARPTOSCREEN
%token <num> F_SHUTDOWN F_DELETE F_TITLEPIN F_REGROUP F_SQUEEZE F_FORCEMOVE
%token <num> F_PANNER F_SCROLLLEFT F_SCROLLRIGHT F_SCROLLUP F_SCROLLDOWN
%token <num> F_STICK F_UNSTICK F_SCROLLHOME F_FALSESTOP F_SCROLL
%token <num> F_RECONFIG F_GETRESOURCE F_PIN F_UNPIN F_QUERY F_SCROLLTO
%token <num> F_SAVEVROOT F_RESTOREVROOT F_WARPSAVE F_SCROLLSAVE F_PLACES
%token <num> F_CONSTRAINMOVE F_REBIND F_NEWBUTTONIMAGE F_ANIMATEBUTTON
%token <num> F_DUMP F_AUTORESIZE F_SWEEP F_UNSWEEP F_HARDRESTART F_HANDLEDROP
%token <num> F_ROUNDUP F_SETDESKTOP F_FREEZEDESKTOP F_THAWDESKTOP F_OWNER
%token <num> F_FOCUSCOLOR F_TERMINALPLACES
%token <num> WM_STRINGS WM_PANEL WM_REGION WM_BINDING WM_FUNCTION WM_MENU WM_ST
%token <num> OBJ_PANEL OBJ_BUTTON OBJ_TEXT OBJ_MENU OBJ_MENUBAR
%token <num> PLUS MINUS TIMES LP RP SHIFT CTRL META BINDINGS
%token <num> MAPPED MAPPED_NO_SPACE UNMAPPED UNMAPPED_NO_SPACE
%token <num> KEY BTN BTN1 BTN2 BTN3 BTN4 BTN5 DROP DROP1 DROP2 DROP3 DROP4 DROP5
%token <ptr> STRING
%token <num> F_GRAVITY F_RESHUFFLE F_INFO

%type <num> object
%type <ptr> string

%start swmrc 

%%
swmrc	    : stmts
	    ;

stmts	    : /* Empty */
	    | stmts stmt
	    ;

stmt	    : error
	    | WM_STRINGS strings
	    | WM_PANEL panel_entries
	    | WM_REGION region_entries
	    | WM_BINDING		{ mods = 0; resetArgs(); }
	      bindings
	    | WM_FUNCTION		{ resetArgs(); }
	      functions
	    | WM_MENU			{ resetArgs(); }
	      menu_entries
	    | WM_ST			{ resetArgs(); }
	      object_states
	    ;

object_states	: MAPPED		{ wmObjState = wmMapped; }
		| MAPPED_NO_SPACE	{ wmObjState = wmMappedNoSpace; }
		| UNMAPPED		{ wmObjState = wmUnmapped; }
		| UNMAPPED_NO_SPACE	{ wmObjState = wmUnmappedNoSpace; }
		;

menu_entries	: /* empty */
		| menu_entries menu_entry
		;

menu_entry	: string		{ wmAddBinding(0, 0, $1); resetArgs(); }
		  menu_functions	{ wmMenuEntryDone(); resetArgs(); }
		; 

menu_functions	: /* Empty */
		| menu_functions function { wmAddFunction(what, argv, argc);
					    resetArgs();
					  }
		;

bindings	: /* empty */
		| bindings other_bindings
		| bindings binding
		;

other_bindings	: BINDINGS string	{ wmAddBinding(0, BINDINGS, $2); wmBindingDone(); mods=0; resetArgs(); }
		;

binding		: modifiers key_or_click { wmAddBinding(mods, what, argv[0]); resetArgs(); }
		  functions		 { wmBindingDone(); mods=0; resetArgs(); }
		;

key_or_click	: key
		| click
		;

key		: KEY string		{ what = KEY; argv[argc++] = $2; }
		;

click		: button opt_string
		;

opt_string	: /* empty */
		| opt_string string		{ if (argc < PARAMETERS) argv[argc++] = $2; }
		;

button		: BTN1		{ what = Button1; mods |= Button1Mask; }
		| BTN2		{ what = Button2; mods |= Button2Mask; }
		| BTN3		{ what = Button3; mods |= Button3Mask; }
		| BTN4		{ what = Button4; mods |= Button4Mask; }
		| BTN5		{ what = Button5; mods |= Button5Mask; }
		| OBJ_BUTTON	{ what = BTN; }
		| DROP		{ what = DROP; }
		| DROP1		{ what = DROP1; }
		| DROP2		{ what = DROP2; }
		| DROP3		{ what = DROP3; }
		| DROP4		{ what = DROP4; }
		| DROP5		{ what = DROP5; }
		;

modifiers	: /* empty */
		| modifiers modifier
		;

modifier	: META			{ mods |= Mod1Mask; }
		| SHIFT			{ mods |= ShiftMask; }
		| CTRL			{ mods |= ControlMask; }
		;

functions	: /* empty */
		| functions function	{ wmAddFunction(what, argv, argc); resetArgs(); }
		;

function	: wm_func LP opt_string RP
		| wm_func
		;

wm_func		: F_RAISE		{ what = F_RAISE; }
		| F_LOWER		{ what = F_LOWER; }
		| F_ICONIFY		{ what = F_ICONIFY; }
		| F_DEICONIFY		{ what = F_DEICONIFY; }
		| F_RESTART		{ what = F_RESTART; }
		| F_QUIT		{ what = F_QUIT; }
		| F_MOVE		{ what = F_MOVE; }
		| F_FORCEMOVE		{ what = F_FORCEMOVE; }
		| F_MOVEOPAQUE		{ what = F_MOVEOPAQUE; }
		| F_RESIZE		{ what = F_RESIZE; }
		| F_GROUP		{ what = F_GROUP; }
		| F_UNGROUP		{ what = F_UNGROUP; }
		| F_REGROUP		{ what = F_REGROUP; }
		| F_BEEP		{ what = F_BEEP; }
		| F_CIRCLEDOWN		{ what = F_CIRCLEDOWN; }
		| F_CIRCLEUP		{ what = F_CIRCLEUP; }
		| F_DESTROY		{ what = F_DESTROY; }
		| F_EXEC		{ what = F_EXEC; }
		| F_FOCUS		{ what = F_FOCUS; }
		| F_IDENTIFY		{ what = F_IDENTIFY; }
		| F_NOP			{ what = F_NOP; }
		| F_RAISELOWER		{ what = F_RAISELOWER; }
		| F_REFRESH		{ what = F_REFRESH; }
		| F_UNFOCUS		{ what = F_UNFOCUS; }
		| F_WINREFRESH		{ what = F_WINREFRESH; }
		| F_WARPTO		{ what = F_WARPTO; }
		| F_MENU		{ what = F_MENU; }
		| F_TITLE		{ what = F_TITLE; }
		| F_TITLEPIN		{ what = F_TITLEPIN; }
		| F_STOP		{ what = F_STOP; }
		| F_PACK		{ what = F_PACK; }
		| F_SQUEEZE		{ what = F_SQUEEZE; }
		| F_KILL		{ what = F_KILL; }
		| F_SHUTDOWN		{ what = F_SHUTDOWN; }
		| F_DELETE		{ what = F_DELETE; }
		| F_MAP			{ what = F_MAP; }
		| F_UNMAP		{ what = F_UNMAP; }
		| F_MACRO		{ what = F_MACRO; }
		| F_STRING		{ what = F_STRING; }
		| F_ZOOM		{ what = F_ZOOM; }
		| F_HORIZOOM		{ what = F_HORIZOOM; }
		| F_VERTZOOM		{ what = F_VERTZOOM; }
		| F_SAVE		{ what = F_SAVE; }
		| F_RESTORE		{ what = F_RESTORE; }
		| F_SAVEPOINTER		{ what = F_SAVEPOINTER; }
		| F_RESTOREPOINTER	{ what = F_RESTOREPOINTER; }
		| F_WARPHORIZONTAL	{ what = F_WARPHORIZONTAL; }
		| F_WARPVERTICAL	{ what = F_WARPVERTICAL; }
		| F_WARPTOSCREEN	{ what = F_WARPTOSCREEN; }
		| F_PANNER		{ what = F_PANNER; }
		| F_SCROLLLEFT		{ what = F_SCROLLLEFT; }
		| F_SCROLLRIGHT		{ what = F_SCROLLRIGHT; }
		| F_SCROLLUP		{ what = F_SCROLLUP; }
		| F_SCROLLDOWN		{ what = F_SCROLLDOWN; }
		| F_STICK		{ what = F_STICK; }
		| F_UNSTICK		{ what = F_UNSTICK; }
		| F_SCROLLHOME		{ what = F_SCROLLHOME; }
		| F_FALSESTOP		{ what = F_FALSESTOP; }
		| F_SCROLL		{ what = F_SCROLL; }
		| F_RECONFIG		{ what = F_RECONFIG; }
		| F_GETRESOURCE		{ what = F_GETRESOURCE; }
		| F_PIN			{ what = F_PIN; }
		| F_UNPIN		{ what = F_UNPIN; }
		| F_QUERY		{ what = F_QUERY; }
		| F_SCROLLTO		{ what = F_SCROLLTO; }
		| F_SAVEVROOT		{ what = F_SAVEVROOT; }
		| F_RESTOREVROOT	{ what = F_RESTOREVROOT; }
		| F_WARPSAVE		{ what = F_WARPSAVE; }
		| F_SCROLLSAVE		{ what = F_SCROLLSAVE; }
		| F_PLACES		{ what = F_PLACES; }
		| F_CONSTRAINMOVE	{ what = F_CONSTRAINMOVE; }
		| F_REBIND		{ what = F_REBIND; }
		| F_NEWBUTTONIMAGE	{ what = F_NEWBUTTONIMAGE; }
		| F_ANIMATEBUTTON	{ what = F_ANIMATEBUTTON; }
		| F_DUMP		{ what = F_DUMP; }
		| F_AUTORESIZE		{ what = F_AUTORESIZE; }
		| F_SWEEP		{ what = F_SWEEP; }
		| F_UNSWEEP		{ what = F_UNSWEEP; }
		| F_HARDRESTART		{ what = F_HARDRESTART; }
		| F_HANDLEDROP		{ what = F_HANDLEDROP; }
		| F_ROUNDUP		{ what = F_ROUNDUP; }
		| F_SETDESKTOP		{ what = F_SETDESKTOP; }
		| F_FREEZEDESKTOP	{ what = F_FREEZEDESKTOP; }
		| F_THAWDESKTOP		{ what = F_THAWDESKTOP; }
		| F_GRAVITY		{ what = F_GRAVITY; }
		| F_RESHUFFLE		{ what = F_RESHUFFLE; }
		| F_INFO		{ what = F_INFO; }
		| F_OWNER		{ what = F_OWNER; }
		| F_FOCUSCOLOR		{ what = F_FOCUSCOLOR; }
		| F_TERMINALPLACES	{ what = F_TERMINALPLACES; }
		;

panel_entries	: /* empty */
		| panel_entries panel_entry
		;

panel_entry	: object string string  {
					char *p;
					geom.x = geom.y = 0;
					geom.width = geom.height = 1;
					geom.sign_x = geom.sign_y = 1;
					geom.center = False;
					if (!(p = strchr($3, 'c')))
					    p = strchr($3, 'C');
					if (p)
					{
					    *p = '0';
					    geom.center = True;
					}
					else
					    geom.center = False;
					mask = XParseGeometry($3,
					    &geom.x, &geom.y,
					    &geom.width, &geom.height);
					if (mask & XNegative)
					{
					    geom.x *= -1;
					    geom.sign_x = -1;
					}
					if (mask & YNegative)
					{
					    geom.y *= -1;
					    geom.sign_y = -1;
					}
					free($3);
					wmPanelKidsList.append((ent)
					    new wmPanelKid($1, $2 ,&geom));
				    }
	    ;

region_entries	: /* empty */
		| region_entries region_entry
		;

region_entry	: string string string  {
					geom.x = geom.y = 0;
					geom.width = geom.height = 1;
					geom.sign_x = geom.sign_y = 1;
					geom.center = False;
					mask = XParseGeometry($3,
					    &geom.x, &geom.y,
					    &geom.width, &geom.height);
					if (mask & XNegative)
					{
					    geom.x *= -1;
					    geom.sign_x = -1;
					}
					if (mask & YNegative)
					{
					    geom.y *= -1;
					    geom.sign_y = -1;
					}
					free($3);
					wmRegionKidsList.append((ent)
					    new wmRegionKid($1, $2 ,&geom));
				    }
	    ;

object	    : OBJ_PANEL		    { $$ = OBJ_PANEL; }
	    | OBJ_BUTTON	    { $$ = OBJ_BUTTON; }
	    | OBJ_TEXT		    { $$ = OBJ_TEXT; }
	    | OBJ_MENU		    { $$ = OBJ_MENU; }
	    | OBJ_MENUBAR	    { $$ = OBJ_MENUBAR; }
	    ;

strings	    : /* empty */
	    | strings string	    { wmStringList.append((ent)$2); }
	    ;

string	    : STRING	    { $$ = $1; }
	    ;
%%
