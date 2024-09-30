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
 * $Id: lex.L,v 9.16 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Lexical definition of tokens
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/

#ifndef lint
static char RCSinfo[] =
"$Id: lex.L,v 9.16 1993/08/27 16:58:15 toml Exp $";
#endif /* lint */

#include "gram.H"
#include "parse.H"
// #include <OI/Xlib.H>
// #include <X11/Xos.h>

#ifndef hpux
#include <string.h>
#else
#include <strings.h>
#endif

#include <malloc.h>
#include <stdlib.h>

char *copy(char *);
char *rmquote(char *);
int yylook();
int yywrap();
int yyback(int *, int);
/* Ctrl		    	{ return (CTRL); } */
%}

%k 3000
%e 2000
%a 10000
%o 15000
%p 8000
%n 1500

drop			[Dd][Rr]?[Oo]?[Pp]?
button			[Bb][Uu]?[Tt]?[Tt]?[Oo]?[Nn]?
mapped			[Mm][Aa][Pp][Pp][Ee][Dd]
unmapped		[Uu][Nn][Mm][Aa][Pp][Pp][Ee][Dd]
nospace			[Nn][Oo][Ss][Pp][Aa][Cc][Ee]

%%

","		    	{ ; }
":"		    	{ ; }
"<"		    	{ ; }
">"		    	{ ; }
"("		    	{ return (LP); }
")"		    	{ return (RP); }
wmStrings	    	{ return (WM_STRINGS); }
wmPanel		    	{ return (WM_PANEL); }
wmBinding	    	{ return (WM_BINDING); }
wmFunction	    	{ return (WM_FUNCTION); }
wmMenu		    	{ return (WM_MENU); }
wmState		    	{ return (WM_ST); }
wmRegion		{ return (WM_REGION); }

{mapped}				{ return (MAPPED); }
{mapped}{nospace}			{ return (MAPPED_NO_SPACE); }
{unmapped}				{ return (UNMAPPED); }
{unmapped}{nospace}			{ return (UNMAPPED_NO_SPACE); }

[Ss][Hh]?[Ii]?[Ff]?[Tt]?	    	{ return (SHIFT); }
[Mm][Ee]?[Tt]?[Aa]?		    	{ return (META); }
[Cc][Oo]?[Nn]?[Tt]?[Rr]?[Oo]?[Ll]?    	{ return (CTRL); }

[Kk][Ee]?[Yy]?		{ return (KEY); }
{button}		{ return (OBJ_BUTTON); }
{button}1		{ return (BTN1); }
{button}2		{ return (BTN2); }
{button}3		{ return (BTN3); }
{button}4		{ return (BTN4); }
{button}5		{ return (BTN5); }
{drop}			{ return (DROP); }
{drop}1			{ return (DROP1); }
{drop}2			{ return (DROP2); }
{drop}3			{ return (DROP3); }
{drop}4			{ return (DROP4); }
{drop}5			{ return (DROP5); }

[Pp][Aa]?[Nn]?[Ee]?[Ll]?    		{ return (OBJ_PANEL); }
[Tt][Ee]?[Xx]?[Tt]?	    		{ return (OBJ_TEXT); }
[Bb][Ii][Nn]?[Dd]?[Ii]?[Nn]?[Gg]?[Ss]?	{ return (BINDINGS); }

f.raise		    	{ return (F_RAISE); }
f.lower		    	{ return (F_LOWER); }
f.iconify	    	{ return (F_ICONIFY); }
f.deiconify	    	{ return (F_DEICONIFY); }
f.restart	    	{ return (F_RESTART); }
f.quit		    	{ return (F_QUIT); }
f.move		    	{ return (F_MOVE); }
f.forcemove	    	{ return (F_FORCEMOVE); }
f.moveopaque	    	{ return (F_MOVEOPAQUE); }
f.resize	    	{ return (F_RESIZE); }
f.group			{ return (F_GROUP); }
f.ungroup		{ return (F_UNGROUP); }
f.regroup		{ return (F_REGROUP); }
f.beep			{ return (F_BEEP); }
f.circledown		{ return (F_CIRCLEDOWN); }
f.circleup		{ return (F_CIRCLEUP); }
f.exec			{ return (F_EXEC); }
f.focus			{ return (F_FOCUS); }
f.identify		{ return (F_IDENTIFY); }
f.nop			{ return (F_NOP); }
f.raiselower		{ return (F_RAISELOWER); }
f.refresh		{ return (F_REFRESH); }
f.unfocus		{ return (F_UNFOCUS); }
f.winrefresh		{ return (F_WINREFRESH); }
f.warpto		{ return (F_WARPTO); }
f.scroll		{ return (F_SCROLL); }
f.scrollleft		{ return (F_SCROLLLEFT); }
f.scrollright		{ return (F_SCROLLRIGHT); }
f.scrollup		{ return (F_SCROLLUP); }
f.scrolldown		{ return (F_SCROLLDOWN); }
f.warphorizontal	{ return (F_WARPHORIZONTAL); }
f.warpvertical		{ return (F_WARPVERTICAL); }
f.warptoscreen		{ return (F_WARPTOSCREEN); }
f.savepointer		{ return (F_SAVEPOINTER); }
f.restorepointer	{ return (F_RESTOREPOINTER); }
f.menu			{ return (F_MENU); }
f.title			{ return (F_TITLE); }
f.titlepin		{ return (F_TITLEPIN); }
f.stop			{ return (F_STOP); }
f.truestop		{ return (F_STOP); }
f.falsestop		{ return (F_FALSESTOP); }
f.pack			{ return (F_PACK); }
f.squeeze		{ return (F_SQUEEZE); }
f.kill			{ return (F_KILL); }
f.destroy		{ return (F_KILL); }
f.delete		{ return (F_DELETE); }
f.shutdown		{ return (F_SHUTDOWN); }
f.map			{ return (F_MAP); }
f.unmap			{ return (F_UNMAP); }
f.macro			{ return (F_MACRO); }
f.string		{ return (F_STRING); }
f.zoom			{ return (F_ZOOM); }
f.horizoom		{ return (F_HORIZOOM); }
f.vertzoom		{ return (F_VERTZOOM); }
f.save			{ return (F_SAVE); }
f.restore		{ return (F_RESTORE); }
f.panner		{ return (F_PANNER); }
f.stick			{ return (F_STICK); }
f.unstick		{ return (F_UNSTICK); }
f.scrollhome		{ return (F_SCROLLHOME); }
f.reconfig		{ return (F_RECONFIG); }
f.getresource		{ return (F_GETRESOURCE); }
f.pin			{ return (F_PIN); }
f.unpin			{ return (F_UNPIN); }

f.query			{ return (F_QUERY); }
f.scrollto		{ return (F_SCROLLTO); }
f.savevdt		{ return (F_SAVEVROOT); }
f.restorevdt		{ return (F_RESTOREVROOT); }
f.warpsave		{ return (F_WARPSAVE); }
f.scrollsave		{ return (F_SCROLLSAVE); }
f.places		{ return (F_PLACES); }
f.constrainmove	    	{ return (F_CONSTRAINMOVE); }
f.rebind	    	{ return (F_REBIND); }
f.newbuttonimage    	{ return (F_NEWBUTTONIMAGE); }
f.animatebutton    	{ return (F_ANIMATEBUTTON); }
f.dump		   	{ return (F_DUMP); }
f.autoresize	   	{ return (F_AUTORESIZE); }
f.sweep			{ return (F_SWEEP); }
f.unsweep		{ return (F_UNSWEEP); }
f.hardrestart		{ return (F_HARDRESTART); }
f.handledrop		{ return (F_HANDLEDROP); }
f.roundup		{ return (F_ROUNDUP); }

f.setdesktop		{ return (F_SETDESKTOP); }
f.freezedesktop		{ return (F_FREEZEDESKTOP); }
f.thawdesktop		{ return (F_THAWDESKTOP); }
f.gravity		{ return (F_GRAVITY); }
f.reshuffle		{ return (F_RESHUFFLE); }
f.info			{ return (F_INFO); }
f.owner			{ return (F_OWNER); }
f.focuscolor		{ return (F_FOCUSCOLOR); }
f.terminalplaces	{ return (F_TERMINALPLACES); }

\"[^"]*\"	    	{ yylval.ptr = copy((char *)rmquote((char *)&yytext[0])); return (STRING); }
[^ \t\n\,\:\>\<\(\)]*	{ yylval.ptr = copy((char *)&yytext[0]); return (STRING); }
[\n\t ]		    	{;}

.		    	{ fprintf(stderr, "******** %s\n", yytext); }

%%

yywrap() { return(1);}

#ifdef hpux
extern "C" void yyerror(char *foo);
#endif
void
yyerror(char *foo)
{
    wmParseError = 1;
}

char*
rmquote(char *s)
{
    char *p;

    ++s;
    p = strchr(s, '"');
    *p = '\0';

    return(s);
}

char *
copy(char *s)
{
    char *ptr;

    ptr = (char *)malloc(strlen(s) + 1);
    if (ptr == NULL) {
	fprintf(stderr, "out of space");
    exit(1);
     }
    return(strcpy(ptr, s));
}

#undef unput /* (c) */
#undef input /* () */
#undef output /* (c) */
#define unput(c)	wmParseStr[--wmParseIndex] = c
#define input()		(wmParseIndex == wmParseLen ? NULL : wmParseStr[wmParseIndex++])
#define output(c)	putchar(c)

