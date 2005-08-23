/*
    Aeskulap - DICOM image viewer and network client
    Copyright (C) 2005  Alexander Pipelka

    This file is part of Aeskulap.

    Aeskulap is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Aeskulap is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Aeskulap; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Alexander Pipelka
    pipelka@teleweb.at

    Last Update:      $Author: braindead $
    Update Date:      $Date: 2005/08/23 19:32:03 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/widgets/adisplay.h,v $
    CVS/RCS Revision: $Revision: 1.1 $
    Status:           $State: Exp $
*/

#ifndef AESKULAP_DISPLAY_H
#define AESKULAP_DISPLAY_H

#include "asimpledisplay.h"

namespace Aeskulap {

class Display : public SimpleDisplay {
public:

	Display();

	Display(const Glib::RefPtr<DisplayParameters>& params);

	~Display();

	void set_selected(bool selected);

	void draw_point(const ImagePool::Instance::Point& p);

	void draw_line(const ImagePool::Instance::Point& p0, const ImagePool::Instance::Point& p1);

	Gdk::Color m_colorSeriesSelected;

	Gdk::Color m_colorReferenceCurrent;

	Gdk::Color m_colorReference;

	Gdk::Color m_colorSelected;

	Gdk::Color m_colorFrame;

	Gdk::Color m_colorText;

	sigc::signal< void, int > signal_selected;

	sigc::signal< void, int, bool > signal_changed;

	sigc::signal< void, Display* > signal_scroll_up;

	sigc::signal< void, Display* > signal_scroll_down;

	sigc::signal<void, Display*, const Glib::RefPtr<Gdk::Window>&, const Glib::RefPtr<Gdk::GC>&> signal_draw;

	sigc::signal<void, GdkEventButton*> signal_popup;

protected:

	void on_realize();

	bool on_expose_event(GdkEventExpose* event);

	bool on_button_press_event(GdkEventButton* button);

	bool on_button_release_event(GdkEventButton* button);

	bool on_motion_notify_event(GdkEventMotion* event);

	void draw_ruler_v();

	void draw_ruler_h();
	
	void set_window_palette(gdouble x, gdouble y);

	// drag parameters

	bool m_drag_active;
	guint m_drag_button;
	gdouble m_drag_start_x;
	gdouble m_drag_start_y;
	int m_drag_window_center;
	int m_drag_window_width;

	Glib::RefPtr<Pango::Layout> m_layoutR;

	Glib::RefPtr<Pango::Layout> m_layoutL;

	Glib::RefPtr<Pango::Layout> m_layoutB;

	Pango::FontDescription m_fntdesc;

	bool m_changed;

	Gdk::Cursor* m_cursor_pan;

	Gdk::Cursor* m_cursor_windowlevel;

	Gdk::Cursor* m_cursor_zoom;

};

} //namespace Aeskulap

#endif // AESKULAP_DISPLAY_H

