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
    Update Date:      $Date: 2005/08/30 12:59:41 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/widgets/studyview.h,v $
    CVS/RCS Revision: $Revision: 1.2 $
    Status:           $State: Exp $
*/

#ifndef STUDYVIEW_H
#define STUDYVIEW_H

#include <gtkmm.h>
#include <gtkmm/menutoolbutton.h>

#include <vector>
#include <map>

// forward declarations

namespace Aeskulap {
	class Display;
}

namespace ImagePool {
	class Series;
	class Study;
	class Instance;
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
	
	void set_progress(unsigned int progress);

	void set_layout(int tilex, int tiley);

	void on_series_added(const Glib::RefPtr<ImagePool::Series>& series);

	void on_close();

	sigc::signal<void, const std::string&> signal_close;

protected:

	bool on_key_press_event(	GdkEventKey* event);

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

	void add_series(const Glib::RefPtr<ImagePool::Series>& series);

	void view_single_series(SeriesView* view);

	void draw_reference(Aeskulap::Display* display, const Glib::RefPtr<ImagePool::Instance>& instance);

private:

	SeriesView* create_seriesview();

	Gtk::Tooltips m_tooltips;

	Gtk::Table* m_table;
	
	//Gtk::ToolButton* m_btn_close;

	Gtk::Toolbar* m_toolbar;

	Gtk::Toolbar* m_toolbar_measure;

	SeriesLayoutToolButton* m_series_layout;

	ImageLayoutToolButton* m_image_layout;

	Gtk::MenuToolButton* m_toggle_full;

	Aeskulap::SeriesMenu m_series_menu;

	bool m_single_series;

	unsigned int m_seriescount;
	
	std::vector< Glib::RefPtr<ImagePool::Series> > m_series;

	Glib::RefPtr<ImagePool::Study> m_study;

	SeriesView* m_selected;
	
	bool m_draw_reference_frames;

	bool m_draw_reference_frame_ends;
	
};

#endif // STUDYVIEW_H
