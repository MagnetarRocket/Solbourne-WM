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
 * $Id: hm.H,v 9.4 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Help Manager definitions
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/

#ifndef HLP_H
#define HLP_H

#include <OI/oi.H>

#define NUM_BUTTONS 5

#define HelpBasic	1

typedef struct _HelpAnyEvent {
	int		type;
	int		override;
} HelpAnyEvent;

typedef struct _HelpBasicEvent {
	int		type;
	int		override;
	short		size[NUM_BUTTONS];
	char 		text[2];
} HelpBasicEvent;

typedef union _HelpState {
	int		type;
	HelpAnyEvent	hany;
	HelpBasicEvent	hbasic;
} HelpState;

typedef struct _HelpText {
	char		*event_data;
	char		*text[NUM_BUTTONS];
} HelpText;

class HelpButton {
private:
	OI_box		*box;								/* enclosing box */
	OI_glyph	*glyph;								/* glyph mouse button (0-2) */
	OI_static_text	*label;								/* label mouse button (3-4) */
	OI_static_text	*text;								/* help text */
public:
			HelpButton(int);						/* create a new help button */
	void		set_help_text(char *s)	{ text->set_text(s); }			/* set the displayed help text */
	OI_d_tech	*objp()			{ return(box); }			/* pointer to the outside object */
};

class HelpManager {
private:
	OI_app_window	*app;								/* enclosing app window */
	OI_glyph	*icon;								/* glyph for the icon */
	HelpButton	*button[NUM_BUTTONS];						/* help button objects */
	HelpText	override;							/* override help text */
	HelpText	normal;								/* normal help text */
	Atom		HM_STATE;							/* HM_STATE property */

	Display		*dpy()			{ return (app->connection()->display());}
	OI_connection	*conp()			{ return (app->connection());}
	void		strategy(XEvent *, void *);
public:
			HelpManager();
};


#endif /* HLP_H */
