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
    Update Date:      $Date: 2006/03/20 18:19:15 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/widgets/studyview.h,v $
    CVS/RCS Revision: $Revision: 1.18 $
    Status:           $State: Exp $
*/

#ifndef STUDYVIEW_H
#define STUDYVIEW_H

#include <gtkmm.h>
#include <gtkmm/menutoolbutton.h>

#include <vector>
#include <map>

#include "awindowlevel.h"
#include "poolinstance.h"

// forward declarations

namespace Aeskulap {
	class Display;
	class WindowLevelToolButton;
	class ValueTool;
}

namespace ImagePool {
	class Series;
	class Study;
}

class SeriesView;
class SeriesLayoutToolButton;
class ImageLayoutToolButton;

#include "aseriesmenu.h"
#include "atiler.h"

class StudyView : public Gtk::VBox, public Aeskulap::Tiler<SeriesView> {
public:

	StudyView(const Glib::RefPtr<ImagePool::Study>& study);

	~StudyView();

	void enable_mouse_functions(bool enable);

	void accelerate(Gtk::Window& window);

	void set_layout(int tilex, int tiley);

	void on_series_added(const Glib::RefPtr<ImagePool::Series>& series);

	sigc::signal< bool, Aeskulap::WindowLevel > signal_windowlevel_add;

protected:

	void on_realize();

	bool on_key_press_event(GdkEventKey* event);

	void on_series_selected(SeriesView* view, bool s);

	void on_series_update(SeriesView* view);

	void on_change_layout(int x, int y);

	void on_change_layout_series(int x, int y);

	void on_draw_instance(SeriesView* s, Aeskulap::Display* d, const Glib::RefPtr<Gdk::Window>& w, const Glib::RefPtr<Gdk::GC>& gc);

	void on_toggle_full();

	void on_popup_series(GdkEventButton* button, SeriesView* view);

	void on_popup_full();

	void on_popup_exchange_series(SeriesView* target, SeriesView* source);

	void on_toggle_refframe();

	void on_toggle_3dcursor();

	void on_toggle_measure();

	void on_toggle_valuecursor();

	void on_instance_added(const Glib::RefPtr<ImagePool::Instance>& instance);

	void on_signal_motion(GdkEventMotion* event, Aeskulap::Display* d, SeriesView* s);

	void on_image_selected(SeriesView* s, Aeskulap::Display* d);

	void on_image_changed(SeriesView* s, Aeskulap::Display* d);

	void on_windowlevel_changed(const Aeskulap::WindowLevel& l);

	void on_windowlevel_default();

	void on_windowlevel_add(Aeskulap::WindowLevelToolButton* btn);

	void on_windowlevel_invert(bool invert);

	void on_doubleclick(SeriesView* s, Aeskulap::Display* d);

	void add_series(const Glib::RefPtr<ImagePool::Series>& series);

	void view_single_series(SeriesView* view);

	void draw_reference(Aeskulap::Display* display, const Glib::RefPtr<ImagePool::Instance>& instance);

private:

	SeriesView* create_seriesview(const Glib::RefPtr<ImagePool::Series>& series);

	Gtk::Tooltips m_tooltips;

	Gtk::Table* m_table;
	
	Gtk::Toolbar* m_toolbar;

	Gtk::Toolbar* m_toolbar_measure;

	SeriesLayoutToolButton* m_series_layout;

	ImageLayoutToolButton* m_image_layout;

	Gtk::MenuToolButton* m_toggle_full;

	Aeskulap::SeriesMenu m_series_menu;

	Gtk::ToggleToolButton* m_refframe;

	Gtk::ToggleToolButton* m_btn_3dcursor;

	Gtk::ToggleToolButton* m_btn_valuecursor;

	Gtk::ToggleToolButton* m_measure;

	Aeskulap::WindowLevelToolButton* m_windowlevel;

	bool m_single_series;

	unsigned int m_seriescount;
	
	std::vector< Glib::RefPtr<ImagePool::Series> > m_series;

	Glib::RefPtr<ImagePool::Study> m_study;

	SeriesView* m_selected;
	
	//bool m_draw_reference_frames;

	bool m_draw_reference_frame_ends;

	//bool m_3dcursor_enabled;

	ImagePool::Instance::Point m_3dcursor;	
	
	Gtk::SeparatorToolItem* m_seperator_reference;

	Aeskulap::ValueTool* m_valuetool;
};

#endif // STUDYVIEW_H
