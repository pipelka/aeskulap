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
    Update Date:      $Date: 2005/09/22 15:40:46 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/widgets/studyview.cpp,v $
    CVS/RCS Revision: $Revision: 1.8 $
    Status:           $State: Exp $
*/

#include "studyview.h"
#include "seriesview.h"
#include "imagepool.h"
#include "astockids.h"
#include "adisplay.h"
#include "serieslayouttoolbutton.h"
#include "imagelayouttoolbutton.h"
#include "poolinstance.h"
#include "poolseries.h"
#include "poolstudy.h"

#include <iostream>
#include "gettext.h"

StudyView::StudyView(const Glib::RefPtr<ImagePool::Study>& study) :
Aeskulap::Tiler<SeriesView>(2, 1),
m_single_series(false),
m_study(study),
m_selected(NULL),
m_draw_reference_frames(false),
m_draw_reference_frame_ends(false) {
	m_seriescount = 0;

	Gtk::HBox* hbox = manage(new Gtk::HBox);
	hbox->show();

	// SET LAYOUT (BASED ON STUDYDATA)
	
	int count = study->seriescount();
	if(count != 0) {
		if(count == 1) {
			Aeskulap::Tiler<SeriesView>::set_layout(1, 1);
			m_single_series = true;
		}
		else if(count == 2) {
			Aeskulap::Tiler<SeriesView>::set_layout(2, 1);
		}
		else if(count <= 4) {
			Aeskulap::Tiler<SeriesView>::set_layout(2, 2);
		}		
		else if(count <= 6) {
			Aeskulap::Tiler<SeriesView>::set_layout(3, 2);
		}
		else if(count <= 9) {
			Aeskulap::Tiler<SeriesView>::set_layout(3, 3);
		}
	}

	// SERIES TABLE
	
	m_table = manage(new Gtk::Table(m_tile_y, m_tile_x, true));

	m_table->set_row_spacings(2);
	m_table->set_col_spacings(2);

	// MEASURE TOOLBAR
	
	m_toolbar_measure = manage(new Gtk::Toolbar);
	m_toolbar_measure->set_toolbar_style(Gtk::TOOLBAR_ICONS);
	m_toolbar_measure->set_orientation(Gtk::ORIENTATION_VERTICAL);
	//m_toolbar_measure->show();

	Gtk::ToggleToolButton* m_draw_eraser = manage(new Gtk::ToggleToolButton(Aeskulap::Stock::DRAW_ERASER));
	m_draw_eraser->show();
	m_toolbar_measure->append(*m_draw_eraser);
	
	// MAIN TOOLBAR (STUDY)

	m_toolbar = manage(new Gtk::Toolbar);
	m_toolbar->set_tooltips(true);

	m_series_layout = manage(new SeriesLayoutToolButton());
	m_toolbar->append(*m_series_layout);
	m_series_layout->set_layout(m_tile_x, m_tile_y);
	m_series_layout->set_tooltip(m_tooltips, gettext("Rearrange the series of the current study"));
	m_series_layout->set_arrow_tooltip(m_tooltips, gettext("Display the series tiling menu"), "");
	m_series_layout->signal_change_layout.connect(sigc::mem_fun(*this, &StudyView::on_change_layout));
	if(count != 1) {
		m_series_layout->show();
	}

	m_toggle_full = manage(new Gtk::MenuToolButton(Aeskulap::Stock::SERIES_SINGLE));
	m_toggle_full->signal_show_menu().connect(sigc::mem_fun(*this, &StudyView::on_popup_full));
	m_toolbar->append(*m_toggle_full, sigc::mem_fun(*this, &StudyView::on_toggle_full));
	m_toggle_full->set_tooltip(m_tooltips, gettext("Toggle single series mode"));
	m_toggle_full->set_arrow_tooltip(m_tooltips, gettext("Display the series selection menu"), "");
	m_toggle_full->set_menu(m_series_menu);
	if(count != 1) {
		m_toggle_full->show();
	}

	m_image_layout = manage(new ImageLayoutToolButton());
	m_toolbar->append(*m_image_layout);
	m_image_layout->set_tooltip(m_tooltips, gettext("Rearrange the images of the selected series"));
	m_image_layout->set_arrow_tooltip(m_tooltips, gettext("Display the the image tiling menu"), "");
	m_image_layout->signal_change_layout.connect(sigc::mem_fun(*this, &StudyView::on_change_layout_series));
	m_image_layout->show();

	Gtk::SeparatorToolItem* seperator = manage(new Gtk::SeparatorToolItem);
	m_toolbar->append(*seperator);
	seperator->show();
	
	Gtk::ToggleToolButton* m_refframe = manage(new Gtk::ToggleToolButton(Aeskulap::Stock::REFFRAME));
	m_refframe->set_tooltip(m_tooltips, gettext("Display references of the selected series"));
	m_toolbar->append(*m_refframe, sigc::mem_fun(*this, &StudyView::on_toggle_refframe));
	m_refframe->show();

	hbox->pack_end(*m_table);
	hbox->pack_end(*m_toolbar_measure, false, false);

	pack_end(*hbox);
	pack_end(*m_toolbar, false, false);
	
	m_table->show();
	m_toolbar->show();
	
	set_layout(m_tile_x, m_tile_y);
}

StudyView::~StudyView() {
	ImagePool::remove_study(m_study);
	m_study.clear();
}

void StudyView::accelerate(Gtk::Window& window) {
	m_image_layout->accelerate(window);
	m_series_layout->accelerate(window);
}

SeriesView* StudyView::create_seriesview() {
	SeriesView* r = new SeriesView;
	r->signal_draw.connect(sigc::mem_fun(*this, &StudyView::on_draw_instance));
	r->signal_selected.connect(sigc::mem_fun(*this, &StudyView::on_series_selected));
	r->signal_update.connect(sigc::mem_fun(*this, &StudyView::on_series_update));
	r->signal_popup.connect(sigc::mem_fun(*this, &StudyView::on_popup_series));

	return r;
}

void StudyView::add_series(const Glib::RefPtr<ImagePool::Series>& series) {
	int x = 0;
	int y = 0;

	m_seriescount++;
	m_series.push_back(series);

	get_xy_from_pos(m_seriescount, x, y);

	SeriesView* w = create_seriesview();

	series->signal_instance_added.connect(sigc::mem_fun(*w, &SeriesView::on_instance_added));
	series->signal_instance_added.connect(sigc::mem_fun(m_series_menu, &Aeskulap::SeriesMenu::set_thumbnail));

	if(m_widgets.size() >= m_seriescount) {
		delete m_widgets[m_seriescount-1];
		m_widgets[m_seriescount-1] = w;
	}
	else {
		m_widgets.push_back(w);
	}

	// all views occupied ?
	if(m_seriescount > max_size()) {
		w->hide();
	}
	else {
		m_table->attach(*w, x-1, x, y-1, y);
		w->show();
	}

	m_series_menu.add_series(series, w);

	if(m_selected == NULL) {
		w->select(true);
	}
}

void StudyView::on_series_added(const Glib::RefPtr<ImagePool::Series>& series) {
	add_series(series);	
}

void StudyView::set_layout(int tilex, int tiley) {
	Aeskulap::Tiler<SeriesView>::set_layout(tilex, tiley);

	// remove widgets (series)
	for(unsigned int i = 0; i < m_widgets.size(); i++) {
		SeriesView* v = m_widgets[i];
		if(v != NULL) {
			m_table->remove(*v);
		}
	}

	// remove unneeded seriesviews
	for(unsigned int i = m_seriescount; i < m_widgets.size(); i++) {
		SeriesView* v = m_widgets[i];
		if(m_selected == v) {
			m_selected = NULL;
		}
		delete m_widgets[i];
		m_widgets[i] = NULL;
	}

	// resize table
	m_table->resize(tiley, tilex);

	// rearrange widgets
	int x = 0;
	int y = 0;

	// create seriesviews
	for(unsigned int i=0; i< m_seriescount; i++) {
		
		get_xy_from_pos(i+1, x, y);
		Gtk::Widget* w = m_widgets[i];

		if(i+1 > max_size()) {
			w->hide();
		}
		else {
			w->show();
			m_table->attach(*w, x-1, x, y-1, y);
		}
		((SeriesView*)w)->schedule_repaint(1000);
	}
	
	// fill space with empty seriesviews
	for(unsigned int i=m_seriescount; i< max_size(); i++) {
		SeriesView* w = new SeriesView;
		m_widgets.push_back(w);
		get_xy_from_pos(i+1, x, y);
		m_table->attach(*w, x-1, x, y-1, y);
		w->show();
	}
	
}

void StudyView::on_series_update(SeriesView* view) {
	if(!m_draw_reference_frames) {
		return;
	}

	queue_draw();
}

void StudyView::on_series_selected(SeriesView* view, bool s) {
	if(!s) {
		return;
	}
	
	if(view == m_selected) {
		return;
	}

	if(m_selected != NULL) {
		m_selected->select(false);
	}
	
	m_selected = view;
	
	int x,y;
	m_selected->get_layout(x, y);
	m_image_layout->set_layout(x, y);
}

void StudyView::draw_reference(Aeskulap::Display* display, const Glib::RefPtr<ImagePool::Instance>& instance) {
	ImagePool::Instance::Point p0 = instance->get_position();
	ImagePool::Instance::Point p1;
	ImagePool::Instance::Point p2;
	ImagePool::Instance::Point p3;
	ImagePool::Instance::Orientation o = instance->get_orientation();

	double sx = instance->width() * instance->spacing_x();
	double sy = instance->height() * instance->spacing_y();

	p1.x = p0.x + o.y.x * sy;
	p1.y = p0.y + o.y.y * sy;
	p1.z = p0.z + o.y.z * sy;

	p2.x = p0.x + o.x.x * sx;
	p2.y = p0.y + o.x.y * sx;
	p2.z = p0.z + o.x.z * sx;

	p3.x = p0.x + o.y.x * sy + o.x.x * sx;
	p3.y = p0.y + o.y.y * sy + o.x.y * sx;
	p3.z = p0.z + o.y.z * sy + o.x.z * sx;

	display->draw_line(p0, p1);
	display->draw_line(p0, p2);
	display->draw_line(p1, p3);
	display->draw_line(p2, p3);

	display->draw_point(p0);
	display->draw_point(p1);
	display->draw_point(p2);
	display->draw_point(p3);
}

void StudyView::on_draw_instance(SeriesView* s, Aeskulap::Display* d, const Glib::RefPtr<Gdk::Window>& w, const Glib::RefPtr<Gdk::GC>& gc) {
	if(!m_draw_reference_frames) {
		return;
	}

	if(s == m_selected) {
		return;
	}
	if(m_selected == NULL || m_selected->m_instance.size() == 0) {
		return;
	}

	Glib::RefPtr<ImagePool::Instance> inst;
	
	if(m_draw_reference_frame_ends) {
		gc->set_foreground(d->m_colorReference);
		inst = m_selected->m_instance[0];
	
		draw_reference(d, inst);
	
		inst = m_selected->m_instance[m_selected->m_instancecount-1];
		draw_reference(d, inst);
	}

	inst = m_selected->m_instance[m_selected->m_selected_image];

	gc->set_foreground(d->m_colorSelected);
	draw_reference(d, inst);
}

void StudyView::on_change_layout(int x, int y) {
	if(m_single_series) {
		return;
	}
	set_layout(x, y);
}

void StudyView::on_change_layout_series(int x, int y) {
	if(m_selected == NULL) {
		return;
	}
	
	m_selected->set_layout(x, y);
}

void StudyView::on_toggle_full() {
	if(m_selected == NULL) {
		return;
	}

	m_single_series = !m_single_series;
	
	if(!m_single_series) {
		m_series_layout->set_sensitive(true);
		m_series_layout->show();
		set_layout(m_tile_x, m_tile_y);
		return;
	}

	view_single_series(m_selected);
}

void StudyView::view_single_series(SeriesView* view) {
	m_series_layout->set_sensitive(false);

	// remove widgets (series)
	for(unsigned int i = 0; i < m_widgets.size(); i++) {
		SeriesView* v = m_widgets[i];
		if(v != NULL) {
			m_table->remove(*v);
			v->hide();
		}
	}

	m_table->resize(1, 1);
	view->show();
	m_table->attach(*view, 0, 1, 0, 1);
	view->select(true);
	
	m_single_series = true;
	view->schedule_repaint(1000);
	
}

void StudyView::on_popup_series(GdkEventButton* button, SeriesView* view) {

	// set new signal handlers for series popup
	std::vector< Glib::RefPtr<ImagePool::Series> >::iterator i;
	for(i = m_series.begin(); i != m_series.end(); i++) {
		m_series_menu.set_connection(*i, sigc::bind(sigc::mem_fun(*this, &StudyView::on_popup_exchange_series), view));
	}

	m_series_menu.popup(button->button, button->time);
}

void StudyView::on_popup_full() {
	// connections for fullscreen seriesmenu
	std::vector< Glib::RefPtr<ImagePool::Series> >::iterator i;
	for(i = m_series.begin(); i != m_series.end(); i++) {
		m_series_menu.set_connection(*i, sigc::mem_fun(*this, &StudyView::view_single_series));
	}
}

void StudyView::on_popup_exchange_series(SeriesView* target, SeriesView* source) {
	unsigned int index1;
	unsigned int index2;

	if(m_single_series) {
		view_single_series(target);
		return;
	}
	
	if(!find_index(target, index1)) {
		return;
	}
	if(!find_index(source, index2)) {
		return;
	}

	bool v1 = m_widgets[index1]->is_visible();
	bool v2 = m_widgets[index2]->is_visible();
	
	SeriesView* help = m_widgets[index1];
	m_widgets[index1] = m_widgets[index2];
	m_widgets[index2] = help;

	int x1,y1;
	int x2,y2;
	get_xy_from_pos(index1+1, x1, y1);
	get_xy_from_pos(index2+1, x2, y2);
	
	m_table->remove(*target);
	m_table->remove(*source);
	m_table->attach(*target, x2-1, x2, y2-1, y2);
	m_table->attach(*source, x1-1, x1, y1-1, y1);

	if(v1) {
		source->show();
	}
	else {
		source->hide();
	}

	if(v2) {
		target->show();
	}
	else {
		target->hide();
	}

	target->select(true);
	m_series_menu.swap_entries(m_series[index1], m_series[index2]);
}

bool StudyView::on_key_press_event(GdkEventKey* event) {
	if(event->type != GDK_KEY_PRESS || m_selected == NULL) {
		return true;
	}
	
	if(event->keyval == GDK_Up) {
		m_selected->scroll_up();
	}
	else if(event->keyval == GDK_Down) {
		m_selected->scroll_down();
	}
	else if(event->keyval == GDK_Tab) {
		unsigned int index;
		if(find_index(m_selected, index)) {
			index++;
			if(index >= m_seriescount) {
				index = 0;
			}
			m_widgets[index]->select(true);
		}
	}
	else if(event->keyval == GDK_Menu) {
		//on_popup_series(GdkEventButton* button, m_selected);
		Glib::RefPtr<Gdk::Window> w = m_selected->get_window();
		int x,y;
		w->get_position(x, y);
		x += m_selected->get_width()/2;
		y += m_selected->get_height()/2;
		
		std::vector< Glib::RefPtr<ImagePool::Series> >::iterator i;
		for(i = m_series.begin(); i != m_series.end(); i++) {
			m_series_menu.set_connection(*i, sigc::bind(sigc::mem_fun(*this, &StudyView::on_popup_exchange_series), m_selected));
		}

		m_series_menu.popup(0, 0);
	}
	
	return true;
}

void StudyView::on_toggle_refframe() {
	m_draw_reference_frames = !m_draw_reference_frames;
	queue_draw();
}
