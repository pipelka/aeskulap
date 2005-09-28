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
    Update Date:      $Date: 2005/09/28 20:32:03 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/widgets/seriesview.h,v $
    CVS/RCS Revision: $Revision: 1.7 $
    Status:           $State: Exp $
*/

#ifndef SERIESVIEW_H
#define SERIESVIEW_H

#include <gtkmm.h>
#include <vector>
#include "atiler.h"

namespace Aeskulap {
	class Display;
	class MultiFrameCtrl;
}

namespace ImagePool {
	class Series;
	class Instance;
}

class DisplayParameters;

class SeriesView : public Gtk::HBox, public Aeskulap::Tiler<Aeskulap::Display> {
public:

	SeriesView(const Glib::RefPtr<ImagePool::Series>& series = Glib::RefPtr<ImagePool::Series>());

	~SeriesView();
	
	void set_layout(int tilex, int tiley);

	void on_instance_added(const Glib::RefPtr<ImagePool::Instance>& instance);

	void select(bool s);

	void scroll_up();
	
	void scroll_down();

	const Glib::RefPtr<ImagePool::Series>& get_series();

	Aeskulap::Display* scroll_to(const Glib::RefPtr<ImagePool::Instance>& instance);

	sigc::signal< void, SeriesView*, bool > signal_selected;

	sigc::signal<void, SeriesView*, Aeskulap::Display*, const Glib::RefPtr<Gdk::Window>&, const Glib::RefPtr<Gdk::GC>&> signal_draw;

	sigc::signal<void, SeriesView*> signal_update;

	sigc::signal<void, GdkEventButton*, SeriesView*> signal_popup;

protected:

	bool on_scroll_event(GdkEventScroll* event);

	bool on_change_value(Gtk::ScrollType type, double value);

	void on_image_selected(unsigned int index);
	
	void on_image_changed(unsigned int index, bool smooth);

	void on_change_layout1();

	void on_change_layout2();

	void on_change_layout4();

	void on_change_layout16();

	bool on_timeout(int timer);

	void on_draw_instance(Aeskulap::Display* d, const Glib::RefPtr<Gdk::Window>& w, const Glib::RefPtr<Gdk::GC>& gc);

	void add_instance(const Glib::RefPtr<ImagePool::Instance>& instance);

	void reorder_by_instancenumber();

	void schedule_repaint(int timeout);

protected:

	void update(bool immediate = true, bool redraw = true, bool smooth = false);

	bool m_selected;

private:

	void update_scrollbar();

	void swap_index(int i1, int i2);
	
	unsigned int get_max_scrollpos();

	void scroll_to(unsigned int pos);
	
	unsigned int m_offset;

	Gtk::Table* m_table;
	
	Gtk::VScrollbar* m_scrollbar;

	unsigned int m_instancecount;
	
	unsigned int m_selected_image;

	std::vector< Glib::RefPtr<ImagePool::Instance> > m_instance;

	std::vector< Glib::RefPtr<DisplayParameters> > m_dispparam;
	
	sigc::connection m_repaint_source;
	
	Aeskulap::MultiFrameCtrl* m_ctrl_frame;

	Glib::RefPtr<ImagePool::Series> m_series;

	friend class StudyView;

};

#endif // STUDYVIEW_H
