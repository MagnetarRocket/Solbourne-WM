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
 * $Id: wmdata.H,v 9.18 1993/08/27 16:58:15 toml Exp $
 *
 * Description:
 *	Window data structure definitions
 *
 * Author: Tom LaStrange
 *
 ******************************************************************************/


#ifndef WMDATA_H
#define WMDATA_H

#include "list.H"
#include "swmhints.H"
#include "screen.H"
#include "ol.H"

class wmData;
extern void wmLowerWindow(wmData *, Window);

extern OI_connection *wmConn;
extern wmScreen *wmScr;

enum wmFocus
{
    wmInputSpecified		= 0x01,
    wmTakeFocusSpecified	= 0x02
};

enum wmFocusType
{
    wmNoInput		= 0,
    wmPassive		= wmInputSpecified,
    wmLocallyActive	= wmInputSpecified | wmTakeFocusSpecified,
    wmGloballyActive	= wmTakeFocusSpecified
};

enum wmProtocols
{
    wmTakeFocus		= 0x01,
    wmSaveYourself	= 0x02,
    wmDeleteWindow	= 0x04,
    wmShutdown		= 0x08
};

#define WM_BUSY			0x00000001		/* OL window is busy */
#define WM_WARP_BACK		0x00000002		/* OL warp the pointer back */
#define WM_STOP			0x00000004		/* execute no more commands */
#define WM_RESIZE_CORNERS	0x00000008		/* resize corners on the frame */
#define WM_RESIZE_BARS		0x00000010		/* resize bars on the frame */
#define WM_ROOT_ICON		0x00000020 		/* is the icon on the root window? */
#define WM_MINE			0x00000040		/* is this a window manager window ? */
#define WM_GRAY_ICON		0x00000080		/* gray it on deiconify */
#define WM_NO_ICON		0x00000100		/* no icon to display */
#define WM_GOT_ICON_IMAGE	0x00000200		/* have we tried to get the icon image ? */
#define WM_TRANSIENT		0x00000400		/* is this a transient ? */
#define WM_ZOOMED		0x00000800		/* is the window zoomed ? */
#define WM_HORIZ_ZOOMED		0x00001000		/* is the horizontally window zoomed ? */
#define WM_VERT_ZOOMED		0x00002000		/* is the vertically window zoomed ? */
#define WM_KILL_SENT		0x00004000		/* waiting for WM_COMMAND to be updated */
#define WM_FREE_WM_HINTS	0x00008000		/* free the wmhints pointer */
#define WM_HILITE_FRAME		0x00010000		/* highlight the frame when pointer enters */
#define WM_CONSTRAIN		0x00020000		/* keep the window on the screen */
#define WM_SAVE_CONSTRAIN	0x00040000		/* another copy of constain */
#define WM_PANNER		0x00080000		/* is this the panner ? */
#define WM_NORMALIZED		0x00100000		/* has the window ever been in normal state? */
#define WM_POSITIONED		0x00200000		/* window was positioned via USPosition or PPosition */
#define WM_SHAPED		0x00400000		/* the client window is shaped */
#define WM_ICONIFY_BY_UNMAPPING	0x00800000		/* iconify the client by unmapping */
#define WM_IGNORE_GROUP		0x02000000		/* we should ignore group hints */
#define WM_HAS_PUSHPIN		0x04000000		/* the window has a pushpin */

	// forward reference declarations
	struct	wmIconPanel;
	struct	wmObject;
	struct	wmIconPanelSlot;
	


class wmClientData
{
protected:
		Window		w;			// the client window
		char		*name;			// name of the dude
		char		*icon_name;		// the icon name from WM_ICON_NAME
		OI_d_tech	*oi_frame;		// the frame OI object ( a box)
		OI_d_tech	*oi_client;		// the client panel
		OI_d_tech	*oi_name;		// the window name object (if any)
		OI_static_text 	*oi_size;		// the size of the client
		Window		transient_for;		// the main window for a transient
		struct		wmData *lastFocus;	// last window to have focus
		struct		wmData *tfwp;		// transient_for wmData
		Window		*colormapWindows;	// from WM_COLORMAP_WINDOWS
		int		colormapCount;		// how many colormap windows
		Atom		*protocols;		// from WM_PROTOCOLS
		int		protocolCount;		// how many atoms
		unsigned long	protocolBits;		// bitmask of protocols
		XWindowAttributes	attr;		// window attributes
		XWMHints	*wmhints;		// window manager hints
		SWMHints	myhints;		// my internal window manager hints
		XSizeHints	sizehints;		// size hints for the window
		int		group;			// group info
		int		regroup;		// an extra copy of the group info
	struct	wmObject	*op;			// pointer to the ouside panel object
		wmList		transientList;		// list of transient windows
		wmList		focusList;		// oi objects to change on focus
		short		insideBorderWidth;	// inside frame border (for OpenLook)
		short		insideBorderWidthSaved;	// inside frame border (for OpenLook)
		short		resizeWidth;		// width of the resize corners/bars
		short		resizeLength;		// width of the resize corners/bars
		short		pad;			// pad for this instance of the panel
		PIXEL		resizeBackground;	// background color of resize stuff
		int		state;			// current state of the window
		int		save_x;			// unzoomed position
		int		save_y;
		int		save_width;		// unzoomed width/height
		int		save_height;
		int		frame_vis;		// frame visibility
		wmFocusType	focusType;		// type of input focus

		// OpenLook attribute stuff
		unsigned long	olDecoration;		// decoration bits
		OLDfltBtn	*olDfltBtn;		// _OL_DFLT_BTN property
		int		root_x;			// saved pointer position
		int		root_y;

				wmClientData();
friend	class	wmData;
};

class wmIconData
{
protected:
		OI_box		*virtualibox;		// box in virtual root display for icon
		char		*iconObject;		// icon object to use
		char		*iconPanel;		// user specified icon panel
		char		*iconRegion;		// user specified icon region
	struct	wmObject	*iop;			// the icon object pointer
		long		irp;			// XXX - this is really a region pointer - fix compile/include problem!!
		OI_d_tech	*oi_icon;		// the icon object
		OI_d_tech	*oi_iconImage;		// the icon image
		OI_d_tech 	*oi_iconName;		// the icon name
		Pixmap		iconImage;		// default icon image
		short		iconLabelLength;	// maximum number of characters
	struct	wmIconPanel	*ip;			// the icon panel struture where the icon is
	struct	wmObject	*ipop;			// the icon panel object pointer
	struct	wmIconPanelSlot	*ipsp;			// the icon panel slot pointer 
		int		icon_vis;		// icon visibility

				wmIconData();
friend	class	wmData;
};

class wmData
{
protected:
		unsigned long	ctl;			// control bits
		wmClientData	*cp;
		wmIconData	*icp;
	struct	wmScreen	*scr;			// the screen pointer
		OI_d_tech	*oi_root;		// the root object the frame is on
		int		Xratio;			// pixel ratio of root
		int		Yratio;			// pixel ratio of root
		OI_box		*virtualbox;		// box in virtual root display
	struct	wmIconPanel	*myIconPanel;		// this is an icon panel
		int		lastStatus;		// status of the last command executed
		XClassHint	wclass;			// the class of the window

public:
		void		new_client()			{ cp = new wmClientData(); }
		void		new_icon()			{ icp = new wmIconData(); }
		void		del_client()			{ if (cp) delete cp; cp = NULL; }
		void		del_icon()			{ if (icp) delete icp; icp = NULL; }

		Bool		has_client()			{ return ((Bool)cp); }
		Bool		has_icon()			{ return ((Bool)icp); }

		void		set_op(struct wmObject *o)	{ cp->op = o; }
	struct	wmObject *	op()				{ return (cp->op); }

		void		set_root(OI_d_tech *r)		{ oi_root = r; }
		OI_d_tech *	root()				{ return (oi_root); }

		void		set_screen(struct wmScreen *s)	{ scr = s; }
	struct	wmScreen *	screen()			{ return (scr); }

		void		set_window(Window w)		{ cp->w = w; }
		Window		window()			{ return (cp->w); }

		void		set_myip(struct wmIconPanel *p)	{ myIconPanel = p; };
	struct	wmIconPanel *	myip()				{ return (myIconPanel); }

		void		set_oi_client(OI_d_tech *o)	{ cp->oi_client = o; }
		OI_d_tech *	oi_client()			{ return (cp->oi_client); }
		void		set_oi_frame(OI_d_tech *o)	{ cp->oi_frame = o; }
		OI_d_tech *	oi_frame()			{ return (cp ? cp->oi_frame : NULL); }
		void		set_oi_name(OI_d_tech *o)	{ cp->oi_name = o; }
		OI_d_tech *	oi_name()			{ return (cp->oi_name); }
		void		set_oi_size(OI_static_text *o)	{ cp->oi_size = o; }
		OI_static_text*	oi_size()			{ return (cp->oi_size); }
		void		set_oi_icon(OI_d_tech *o)	{ icp->oi_icon = o; }
		OI_d_tech *	oi_icon()			{ return (icp ? icp->oi_icon : NULL); }
		void		set_oi_icon_name(OI_d_tech *o)	{ icp->oi_iconName = o; }
		OI_entry_field*	oi_icon_name()			{ return ((OI_entry_field *)icp->oi_iconName); }
		void		set_oi_icon_image(OI_d_tech *o)	{ icp->oi_iconImage = o; }
		OI_d_tech *	oi_icon_image()			{ return (icp ? icp->oi_iconImage : NULL); }
		void		set_icon_image(Pixmap p)	{ icp->iconImage = p; }
		Pixmap		icon_image()			{ return (icp->iconImage); }
		void		set_inside_bw(int b)		{ cp->insideBorderWidth = b; }
		int		inside_bw()			{ return(cp->insideBorderWidth); }
		void		set_inside_bw_saved(int b)	{ cp->insideBorderWidthSaved = b; }
		int		inside_bw_saved()		{ return(cp->insideBorderWidthSaved); }
		void		set_pad(int p)			{ cp->pad = p; }
		short		pad()				{ return (cp->pad); }
		void		set_last_focus(wmData *w)	{ if (cp) cp->lastFocus = w; }
		wmData *	last_focus()			{ return(cp ? cp->lastFocus: NULL); }
		void		set_attr(XWindowAttributes a)	{ cp->attr = a; }
		XWindowAttributes attr()			{ return (cp->attr); }
		XWindowAttributes* attr_p()			{ return (&cp->attr); }
		int		attr_x()			{ return (cp->attr.x); }
		int		attr_y()			{ return (cp->attr.y); }
		unsigned	attr_width()			{ return (cp->attr.width); }
		unsigned	attr_height()			{ return (cp->attr.height); }
		unsigned	attr_bw()			{ return (cp->attr.border_width); }
		void		set_attr_width(unsigned w)	{ cp->attr.width = w; }
		void		set_attr_height(unsigned h)	{ cp->attr.height = h; }
		void		set_attr_bw(unsigned b)		{ cp->attr.border_width = b; }
		void		set_myhints(SWMHints h)		{ cp->myhints = h; }
		SWMHints	myhints()			{ return (cp->myhints); }
		SWMHints *	myhints_p()			{ return (&cp->myhints); }
		void		set_initial_state(int s)	{ cp->myhints.initial_state = s; }
		int		initial_state()			{ return (cp->myhints.initial_state); }
		Window		icon_window()			{ return (cp ? cp->myhints.icon_window : None); }
		void		set_icon_window(Window w)	{ if (cp) cp->myhints.icon_window = w; }
		void		set_icon_x(int x)		{ cp->myhints.icon_x = x; }
		int		icon_x()			{ return (cp->myhints.icon_x); }
		void		set_icon_y(int y)		{ cp->myhints.icon_y = y; }
		int		icon_y()			{ return (cp->myhints.icon_y); }
		void		set_sticky()			{ cp->myhints.flags |= SWMH_STICKY; }
		void		clear_sticky()			{ cp->myhints.flags &= ~SWMH_STICKY; }
		Bool		sticky()			{ return ((Bool)(cp->myhints.flags&SWMH_STICKY)); }
		void		set_icon_gravity()		{ cp->myhints.flags |= SWMH_ICONGRAVITY; }
		void		clear_icon_gravity()		{ cp->myhints.flags &= ~SWMH_ICONGRAVITY; }
		Bool		icon_gravity()			{ return ((Bool)cp->myhints.flags & SWMH_ICONGRAVITY); }
		void		set_gravity_order(int n)	{ cp->myhints.gravity_order = n; }
		int		gravity_order()			{ return (cp->myhints.gravity_order); }
		void		set_iconified()			{ cp->myhints.flags |= SWMH_ICONIFIED; }
		Bool		iconified()			{ return ((Bool)(cp->myhints.flags&SWMH_ICONIFIED)); }
		void		set_placed()			{ cp->myhints.flags |= SWMH_PLACED; }
		Bool		placed()			{ return ((Bool)(cp->myhints.flags&SWMH_PLACED)); }
		void		set_colormap_windows(Window *w)	{ cp->colormapWindows = w; }
		Window *	colormap_windows()		{ return(cp->colormapWindows); }
		Window **	colormap_windows_p()		{ return(&cp->colormapWindows); }
		void		set_colormap_count(int c)	{ cp->colormapCount = c; }
		int		colormap_count()		{ return(cp->colormapCount); }
		int *		colormap_count_p()		{ return(&cp->colormapCount); }
		void		set_class(XClassHint c)		{ wclass = c; }
		void		set_class_class(char *c)	{ wclass.res_class = c; }
		void		set_class_name(char *c)		{ wclass.res_name = c; }
		XClassHint *	wclass_p()			{ return (&wclass); }
		char *		wclass_class()			{ return (wclass.res_class); }
		char *		wclass_name()			{ return (wclass.res_name); }
		Window *	transient_for_p()		{ return (&cp->transient_for); }
		Window		transient_for()			{ return (cp->transient_for); }
		void		set_transient_for_wp(wmData *w)	{ cp->tfwp = w; }
		wmData *	transient_for_wp()		{ return (cp->tfwp); }
		wmData **	transient_for_wp_p()		{ return (&cp->tfwp); }
		wmList		transient_list()		{ return (cp->transientList); }
		wmList *	transient_list_p()		{ return (cp ? &cp->transientList : NULL); }
		wmList		focus_list()			{ return (cp->focusList); }
		wmList *	focus_list_p()			{ return (cp ? &cp->focusList : NULL); }
		void		set_iop(struct wmObject *o)	{ icp->iop = o; }
	struct	wmObject *	iop()				{ return (icp->iop); }
		void		set_ip(struct wmIconPanel *i)	{ icp->ip = i; }
	struct	wmIconPanel *	ip()				{ return (icp->ip); }
		void		set_ipsp(struct wmIconPanelSlot *i)	{ icp->ipsp = i; }
	struct	wmIconPanelSlot* ipsp()				{ return (icp->ipsp); }
		void		set_name(char *n)		{ cp->name = n; }
		char *		name()				{ return (cp->name); }
		char **		name_p()			{ return (&cp->name); }
		void		set_icon_name(char *n)		{ cp->icon_name = n; }
		char *		icon_name()			{ return (cp->icon_name); }
		char **		icon_name_p()			{ return (&cp->icon_name); }
		void		set_icon_panel(char *n)		{ icp->iconPanel = n; }
		void		set_icon_region(char *n)	{ icp->iconRegion = n; }
		char *		icon_panel()			{ return (icp->iconPanel); }
		char *		icon_region()			{ return (icp->iconRegion); }
		void		set_irp(long p)			{ icp->irp = p; }
		void		clear_irp()			{ icp->irp = NULL; }
		long		get_irp()			{ return(icp->irp); }
		void		set_icon_object(char *n)	{ icp->iconObject = n; }
		char *		icon_object()			{ return (icp->iconObject); }

		XSizeHints	size_hints()			{ return (cp->sizehints); }
		XSizeHints *	size_hints_p()			{ return (&cp->sizehints); }
		void		set_size_hints_flags(unsigned f)	{ cp->sizehints.flags = f; }
		wmFocusType	focus_type()			{ return (cp ? cp->focusType : wmNoInput); }
		void		set_focus_type(wmFocus f)	{ cp->focusType = (wmFocusType)(cp->focusType | f); }
		void		set_group(int g)		{ cp->group = g; }
		int		group()				{ return (cp ? cp->group : 0); }
		void		set_regroup(int g)		{ cp->regroup = g; }
		int		regroup()			{ return (cp->regroup); }
		Atom *		protocols()			{ return (cp->protocols); }
		Atom **		protocols_p()			{ return (&cp->protocols); }
		int		protocols_count()		{ return (cp->protocolCount); }
		int *		protocols_count_p()		{ return (&cp->protocolCount); }
		void		clear_protocol_bits()		{ cp->protocolBits = 0; }
		unsigned long	protocol_bits()			{ return (cp->protocolBits); }
		void		set_protocol_bits(wmProtocols p) { cp->protocolBits |= p; }
		void		set_vbox(OI_box *b)		{ virtualbox = b; }
		OI_box *	vbox()				{ return(virtualbox); }
		void		set_vibox(OI_box *b)		{ icp->virtualibox = b; }
		OI_box *	vibox()				{ return(icp ? icp->virtualibox: NULL); }
		void		set_wmhints(XWMHints *w)	{ cp->wmhints = w; }
		XWMHints *	wmhints()			{ return (cp ? cp->wmhints : NULL); }
		void		set_ol_default_btn(OLDfltBtn *b) { cp->olDfltBtn = b; }
		OLDfltBtn *	ol_default_btn()		{ return (cp->olDfltBtn); }
		OLDfltBtn **	ol_default_btn_p()		{ return (&cp->olDfltBtn); }
		void		set_ol_decoration(unsigned d)	{ cp->olDecoration = d; }
		void		or_ol_decoration(unsigned d)	{ cp->olDecoration |= d; }
		void		and_ol_decoration(unsigned d)	{ cp->olDecoration &= d; }
		unsigned	ol_decoration()			{ return (cp->olDecoration); }
		int		root_x()			{ return (cp->root_x); }
		int		root_y()			{ return (cp->root_y); }
		int *		root_x_p()			{ return (&cp->root_x); }
		int *		root_y_p()			{ return (&cp->root_y); }
		void		set_resize_length(short l)	{ cp->resizeLength = l; }
		int		resize_length()			{ return (cp->resizeLength); }
		void		set_resize_width(short l)	{ cp->resizeWidth = l; }
		int		resize_width()			{ return (cp->resizeWidth); }
		void		set_resize_bg(PIXEL p)		{ cp->resizeBackground = p; }
		PIXEL		resize_bg()			{ return (cp->resizeBackground); }
		PIXEL *		resize_bg_p()			{ return (&cp->resizeBackground); }
		void		set_state(int s)		{ cp->state = s; }
		Bool		state()				{ return (cp ? cp->state : IconicState); }
		void		set_frame_vis(int v)		{ cp->frame_vis = v; }
		int		frame_vis()			{ return (cp->frame_vis); }
		void		set_icon_vis(int v)		{ icp->icon_vis = v; }
		int		icon_vis()			{ return (icp->icon_vis); }
		void		set_last_status(int s)		{ lastStatus = s; }
		int		last_status()			{ return (lastStatus); }
		void		set_icon_label_length(short l)	{ icp->iconLabelLength = l; }
		short		icon_label_length()		{ return (icp->iconLabelLength); }
		void		set_x_ratio(int x)		{ Xratio = x; }
		int		x_ratio()			{ return (Xratio); }
		void		set_y_ratio(int y)		{ Yratio = y; }
		int		y_ratio()			{ return (Yratio); }
		void		set_save_x(int x)		{ cp->save_x = x; }
		int		save_x()			{ return (cp->save_x); }
		void		set_save_y(int y)		{ cp->save_y = y; }
		int		save_y()			{ return (cp->save_y); }
		void		set_save_width(unsigned w)	{ cp->save_width = w; }
		int		save_width()			{ return (cp->save_width); }
		void		set_save_height(unsigned h)	{ cp->save_height = h; }
		int		save_height()			{ return (cp->save_height); }

		void		raise()				{ XRaiseWindow(DPY, oi_frame()->outside_X_window()); raise_vbox(); }
		void		raise_vbox()			{ if (vbox()) XRaiseWindow(DPY, vbox()->outside_X_window()); }
		void		lower()				{ wmLowerWindow(this, oi_frame()->X_window()); }
		void		lower_vbox()			{ if (vbox()) XLowerWindow(DPY, vbox()->outside_X_window()); }
		void		map()				{ oi_frame()->set_state(OI_ACTIVE); map_vbox(); }
		void		map_vbox()			{ if (!sticky() && vbox()) vbox()->set_state(OI_ACTIVE); }
		void		unmap()				{ oi_frame()->set_state(OI_NOT_DISPLAYED); unmap_vbox(); }
		void		unmap_vbox()			{ if (vbox()) vbox()->set_state(OI_NOT_DISPLAYED); }
		void		move(int x, int y)		{ oi_frame()->set_loc(x, y); move_vbox(x, y); }
		void		move_vbox(int x, int y)		{ if (vbox()) vbox()->set_loc(x/wmScr->vscale, y/wmScr->vscale); }

		void		raise_icon()			{ XRaiseWindow(DPY, oi_icon()->outside_X_window()); raise_vibox(); }
		void		raise_vibox()			{ if (vibox()) XRaiseWindow(DPY, vibox()->outside_X_window()); }
		void		lower_icon()			{ wmLowerWindow(this, oi_icon()->X_window()); lower_vibox(); }
		void		lower_vibox()			{ if (vibox()) XLowerWindow(DPY, vibox()->outside_X_window()); }
		void		map_icon()			{ oi_icon()->set_state(OI_ACTIVE); map_vibox(); }
		void		map_vibox()			{ if (!sticky() && !ip() && vibox()) vibox()->set_state(OI_ACTIVE); }
		void		unmap_icon()			{ oi_icon()->set_state(OI_NOT_DISPLAYED); unmap_vibox(); }
		void		unmap_vibox()			{ if (vibox()) vibox()->set_state(OI_NOT_DISPLAYED); }
		void		move_icon(int x, int y)		{ oi_icon()->set_loc(x, y); move_vibox(x, y); }
		void		move_vibox(int x, int y)	{ if (vibox()) vibox()->set_loc(x/wmScr->vscale, y/wmScr->vscale); }

		void		set_positioned()		{ ctl |= WM_POSITIONED; }
		void		clear_positioned()		{ ctl &= ~WM_POSITIONED; }
		Bool		positioned()			{ return ((Bool)ctl & WM_POSITIONED); }

		void		set_transient()			{ ctl |= WM_TRANSIENT; }
		void		clear_transient()		{ ctl &= ~WM_TRANSIENT; }
		Bool		transient()			{ return ((Bool)ctl & WM_TRANSIENT); }

		void		set_busy()			{ ctl |= WM_BUSY; }
		void		clear_busy()			{ ctl &= ~WM_BUSY; }
		Bool		busy()				{ return ((Bool)ctl & WM_BUSY); }

		void		set_free_wm_hints()		{ ctl |= WM_FREE_WM_HINTS; }
		void		clear_free_wm_hints()		{ ctl &= ~WM_FREE_WM_HINTS; }
		Bool		free_wm_hints()			{ return ((Bool)ctl & WM_FREE_WM_HINTS); }

		void		set_shaped()			{ ctl |= WM_SHAPED; }
		void		clear_shaped()			{ ctl &= ~WM_SHAPED; }
		Bool		shaped()			{ return ((Bool)ctl & WM_SHAPED); }

		void		set_root_icon()			{ ctl |= WM_ROOT_ICON; }
		void		clear_root_icon()		{ ctl &= ~WM_ROOT_ICON; }
		Bool		root_icon()			{ return ((Bool)ctl & WM_ROOT_ICON); }

		void		set_mine()			{ ctl |= WM_MINE; }
		void		clear_mine()			{ ctl &= ~WM_MINE; }
		Bool		mine()				{ return ((Bool)ctl & WM_MINE); }

		void		set_constrain()			{ ctl |= WM_CONSTRAIN; }
		void		clear_constrain()		{ ctl &= ~WM_CONSTRAIN; }
		Bool		constrain()			{ return ((Bool)ctl & WM_CONSTRAIN); }

		void		set_save_constrain()		{ ctl |= WM_SAVE_CONSTRAIN; }
		void		clear_save_constrain()		{ ctl &= ~WM_SAVE_CONSTRAIN; }
		Bool		save_constrain()		{ return ((Bool)ctl & WM_SAVE_CONSTRAIN); }

		void		set_highlight_frame()		{ ctl |= WM_HILITE_FRAME; }
		void		clear_highlight_frame()		{ ctl &= ~WM_HILITE_FRAME; }
		Bool		highlight_frame()		{ return ((Bool)ctl & WM_HILITE_FRAME); }

		void		set_got_icon_image()		{ ctl |= WM_GOT_ICON_IMAGE; }
		void		clear_got_icon_image()		{ ctl &= ~WM_GOT_ICON_IMAGE; }
		Bool		got_icon_image()		{ return ((Bool)ctl & WM_GOT_ICON_IMAGE); }

		void		set_resize_corners()		{ ctl |= WM_RESIZE_CORNERS; }
		void		clear_resize_corners()		{ ctl &= ~WM_RESIZE_CORNERS; }
		Bool		resize_corners()		{ return ((Bool)ctl & WM_RESIZE_CORNERS); }

		void		set_resize_bars()		{ ctl |= WM_RESIZE_BARS; }
		void		clear_resize_bars()		{ ctl &= ~WM_RESIZE_BARS; }
		Bool		resize_bars()			{ return ((Bool)ctl & WM_RESIZE_BARS); }

		void		set_normalized()		{ ctl |= WM_NORMALIZED; }
		void		clear_normalized()		{ ctl &= ~WM_NORMALIZED; }
		Bool		normalized()			{ return ((Bool)ctl & WM_NORMALIZED); }

		void		set_no_icon()			{ ctl |= WM_NO_ICON; }
		void		clear_no_icon()			{ ctl &= ~WM_NO_ICON; }
		Bool		no_icon()			{ return ((Bool)ctl & WM_NO_ICON); }

		void		set_gray_icon()			{ ctl |= WM_GRAY_ICON; }
		void		clear_gray_icon()		{ ctl &= ~WM_GRAY_ICON; }
		Bool		gray_icon()			{ return ((Bool)ctl & WM_GRAY_ICON); }

		void		set_stop()			{ ctl |= WM_STOP; }
		void		clear_stop()			{ ctl &= ~WM_STOP; }
		Bool		stop()				{ return ((Bool)ctl & WM_STOP); }

		void		set_panner()			{ ctl |= WM_PANNER; }
		void		clear_panner()			{ ctl &= ~WM_PANNER; }
		Bool		panner()			{ return ((Bool)ctl & WM_PANNER); }

		void		set_warp_back()			{ ctl |= WM_WARP_BACK; }
		void		clear_warp_back()		{ ctl &= ~WM_WARP_BACK; }
		Bool		warp_back()			{ return ((Bool)ctl & WM_WARP_BACK); }

		void		set_kill_sent()			{ ctl |= WM_KILL_SENT; }
		void		clear_kill_sent()		{ ctl &= ~WM_KILL_SENT; }
		Bool		kill_sent()			{ return ((Bool)ctl & WM_KILL_SENT); }

		void		set_zoomed()			{ ctl |= WM_ZOOMED; }
		void		clear_zoomed()			{ ctl &= ~WM_ZOOMED; }
		Bool		zoomed()			{ return ((Bool)ctl & WM_ZOOMED); }

		void		set_hori_zoomed()		{ ctl |= WM_HORIZ_ZOOMED; }
		void		clear_hori_zoomed()		{ ctl &= ~WM_HORIZ_ZOOMED; }
		Bool		hori_zoomed()			{ return ((Bool)ctl & WM_HORIZ_ZOOMED); }

		void		set_vert_zoomed()		{ ctl |= WM_VERT_ZOOMED; }
		void		clear_vert_zoomed()		{ ctl &= ~WM_VERT_ZOOMED; }
		Bool		vert_zoomed()			{ return ((Bool)ctl & WM_VERT_ZOOMED); }

		void		set_iconify_by_unmapping()	{ ctl |= WM_ICONIFY_BY_UNMAPPING; }
		void		clear_iconify_by_unmapping()	{ ctl &= ~WM_ICONIFY_BY_UNMAPPING; }
		Bool		iconify_by_unmapping()		{ return ((Bool)ctl & WM_ICONIFY_BY_UNMAPPING); }


		void		set_ignore_group_hints()	{ ctl |= WM_IGNORE_GROUP; }
		void		clear_ignore_group_hints()	{ ctl &= ~WM_IGNORE_GROUP; }
		Bool		ignore_group_hints()		{ return ((Bool)ctl & WM_IGNORE_GROUP); }

		void		set_has_pushpin()	{ ctl |= WM_HAS_PUSHPIN; }
		void		clear_has_pushpin()	{ ctl &= ~WM_HAS_PUSHPIN; }
		Bool		has_pushpin()		{ return ((Bool)ctl & WM_HAS_PUSHPIN); }

				wmData();
				~wmData();
};

extern wmData *wmClient;


#endif /* WMDATA_H */
