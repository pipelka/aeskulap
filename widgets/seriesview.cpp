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
    Update Date:      $Date: 2005/09/02 10:13:12 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/widgets/seriesview.cpp,v $
    CVS/RCS Revision: $Revision: 1.4 $
    Status:           $State: Exp $
*/

#include "seriesview.h"
#include "adisplay.h"
#include "imagepool.h"
#include "poolinstance.h"
#include <iostream>

SeriesView::SeriesView() : Aeskulap::Tiler<InstanceView>(1, 1),
m_selected(false) {
	m_instancecount = 0;
	m_offset = 0;
	m_selected_image = 0;

	m_table = manage(new Gtk::Table(m_tile_y, m_tile_x, true));

	m_table->set_row_spacings(0);
	m_table->set_col_spacings(0);

	m_scrollbar = manage(new Gtk::VScrollbar);
	m_scrollbar->set_range(0,1);
	m_scrollbar->set_value(0);
	m_scrollbar->signal_change_value().connect(sigc::mem_fun(*this, &SeriesView::on_change_value));

	pack_end(*m_scrollbar, false, false);
	pack_end(*m_table);
	
	set_layout(m_tile_x, m_tile_y);
	
	m_table->show();
	m_scrollbar->show();
}

SeriesView::~SeriesView() {
	m_dispparam.clear();
	m_instance.clear();
}

void SeriesView::add_instance(const Glib::RefPtr<ImagePool::Instance>& instance) {
	if(!instance) {
		return;
	}

	int x = 0;
	int y = 0;

	m_instance.push_back(instance);
	Glib::RefPtr<DisplayParameters> p = DisplayParameters::create(instance);
	p->series_selected = m_selected;
	m_dispparam.push_back(p);

	get_xy_from_pos(m_instancecount+1, x, y);

	// all views occupied ?
	if(m_instancecount < max_size()) {
		Aeskulap::Display* d = m_widgets[m_instancecount]->get_display();
		if(d != NULL) {
			d->set_image(instance, p, true);
			//d->signal_draw.connect(sigc::mem_fun(*this, &SeriesView::on_draw_instance));
			//d->signal_popup.connect(sigc::bind(signal_popup, this));
			//signal_display_added(d, instance->series());
		}
	}
	m_instancecount++;

	reorder_by_instancenumber();

	update(false, false);

	update_scrollbar();
}

void SeriesView::on_instance_added(const Glib::RefPtr<ImagePool::Instance>& instance) {
	add_instance(instance);	
}

void SeriesView::update_scrollbar() {
	int page_size = max_size();
	m_scrollbar->set_range(0,get_max_scrollpos()+ page_size);
	m_scrollbar->set_value(m_offset);
	m_scrollbar->set_increments(1, page_size);
	m_scrollbar->get_adjustment()->set_page_size(page_size);
	m_scrollbar->get_adjustment()->changed();
}

void SeriesView::set_layout(int tilex, int tiley) {
	Aeskulap::Tiler<InstanceView>::set_layout(tilex, tiley);

	m_offset = m_selected_image;

	if(m_offset > get_max_scrollpos()) {
		m_offset = get_max_scrollpos();
	}
 
	// remove widgets (instances)
	for(unsigned int i = 0; i < m_widgets.size(); i++) {
		m_table->remove(*(m_widgets[i]));
		delete m_widgets[i];
	}
	m_widgets.clear();

	update_scrollbar();
	
	// resize table
	m_table->resize(tiley, tilex);

	// rearrange widgets
	unsigned int i=0;
	for(int y = 0; y < m_tile_y; y++) {
		for(int x = 0; x < m_tile_x; x++) {
			/*Aeskulap::Display* w = new Aeskulap::Display;

			w->signal_scroll_event().connect(sigc::mem_fun(*this, &SeriesView::on_scroll_event));
			w->signal_selected.connect(sigc::mem_fun(*this, &SeriesView::on_image_selected));
			w->signal_changed.connect(sigc::mem_fun(*this, &SeriesView::on_image_changed));
			w->signal_draw.connect(sigc::mem_fun(*this, &SeriesView::on_draw_instance));
			w->signal_popup.connect(sigc::bind(signal_popup, this));

			m_widgets.push_back(w);*/
			InstanceView* w = InstanceView::create(InstanceView::SINGLE, this);
			m_widgets.push_back(w);

			m_table->attach(*w, x, x+1, y, y+1);
			if(m_offset + i < m_instancecount) {
				if(m_instance[m_offset + i]) {
					w->get_display()->set_image(m_instance[m_offset + i], m_dispparam[m_offset + i], false);
					w->get_display()->set_id(m_offset + i);
				}
			}
			w->show();
			i++;
		}
	}
	schedule_repaint(1000);
}

void SeriesView::scroll_up() {
	scroll_to(m_offset - 1);
	schedule_repaint(1000);
}
	
void SeriesView::scroll_down() {
	scroll_to(m_offset + 1);
	schedule_repaint(1000);
}

bool SeriesView::on_scroll_event(GdkEventScroll* event) {
	select(true);

	if(event->direction == GDK_SCROLL_UP) {
		scroll_up();
		return true;
	}
	if(event->direction == GDK_SCROLL_DOWN) {
		scroll_down();
		return true;
	}
	
	return false;
}

bool SeriesView::on_change_value(Gtk::ScrollType type, double value) {
	select(true);
	scroll_to((int)value);
	schedule_repaint(1000);
	return true;
}

bool SeriesView::on_timeout(int timer) {
	if(timer == 1) {
		update(true, true, true);
	}
	
	return false;
}

void SeriesView::on_image_selected(unsigned int index) {

	for(unsigned int i = 0; i < m_dispparam.size(); i++) {
		m_dispparam[i]->selected = false;
	}
	m_dispparam[index]->selected = true;

	if(index >= m_offset && index < m_offset + max_size()) {
		m_widgets[index - m_offset]->get_display()->update();
	}

	if(m_selected_image >= m_offset && m_selected_image < m_offset + max_size()) {
		m_widgets[m_selected_image - m_offset]->get_display()->update();
	}
	
	m_selected_image = index;
	
	select(true);
	signal_update(this);
}

void SeriesView::on_image_changed(unsigned int index, bool smooth) {
	if(!smooth) {
		return;
	}
	
	for(unsigned int i = 0; i < m_dispparam.size(); i++) {
		if(i != index) {
			m_dispparam[i]->copy(m_dispparam[index]);
		}
	}

	for(unsigned int i = 0; i < m_widgets.size(); i++) {
		m_widgets[i]->get_display()->refresh(smooth);
	}
}

unsigned int SeriesView::get_max_scrollpos() {
	unsigned int rc = m_instancecount - max_size();
	if(rc < 0) {
		rc = 0;
	}
	
	return rc;
}

void SeriesView::scroll_to(unsigned int pos) {
	if(pos > get_max_scrollpos() || pos < 0) {
		return;
	}

	int diff = pos - m_offset;
	m_offset = pos;

	m_dispparam[m_selected_image]->selected = false;
	m_selected_image += diff;
	m_dispparam[m_selected_image]->selected = true;

	m_scrollbar->set_value(m_offset);

	update();	
}

void SeriesView::update(bool immediate, bool redraw, bool smooth) {
	int i=0;
	for(int y = 0; y < m_tile_y; y++) {
		for(int x = 0; x < m_tile_x; x++) {
			if(m_offset + i < m_instancecount) {
				if(m_instance[m_offset + i]) {
					if(redraw) {
						m_widgets[i]->get_display()->set_image(m_instance[m_offset + i], m_dispparam[m_offset + i], smooth);
						m_widgets[i]->get_display()->set_id(m_offset + i);
					}
					if(!immediate) {
						m_widgets[i]->get_display()->queue_draw();
					}
					else {
						m_widgets[i]->get_display()->update();
					}
				}
			}
			i++;
		}
	}

	signal_update(this);
}

void SeriesView::swap_index(int i1, int i2) {
	m_instance[i1].swap(m_instance[i2]);
	m_dispparam[i1].swap(m_dispparam[i2]);
}

void SeriesView::reorder_by_instancenumber() {
	bool changed = true;
	bool redraw = false;
	
	while(changed) {
		changed = false;
		for(unsigned int i1 = 0; i1 < m_instance.size() - 1; i1++) {
			int i2 = i1 + 1;
			if(m_instance[i2]->instancenumber() < m_instance[i1]->instancenumber()) {
				swap_index(i2, i1);
				changed = true;
				redraw = true;
			}
			m_instance[i1]->set_index(i1+1);
			m_instance[i2]->set_index(i2+1);
		}
	}
	
	if(redraw) {
		update();
	}
}

void SeriesView::select(bool s) {
	if(m_selected == s) {
		return;
	}
	
	m_selected = s;
	
	for(unsigned int i = 0; i < m_dispparam.size(); i++) {
		m_dispparam[i]->selected = false;
		m_dispparam[i]->series_selected = m_selected;
	}
	
	if(m_selected && m_dispparam.size() > m_selected_image) {
		m_dispparam[m_selected_image]->selected = true;
	}

	set_state(m_selected ? Gtk::STATE_SELECTED : Gtk::STATE_NORMAL);

	update(false, false);
	
	signal_selected(this, m_selected);
}

void SeriesView::schedule_repaint(int timeout) {
	m_repaint_source.disconnect();
	m_repaint_source = Glib::signal_timeout().connect(sigc::bind(sigc::mem_fun(*this, &SeriesView::on_timeout), 1), 1000);
}

void SeriesView::on_draw_instance(Aeskulap::Display* d, const Glib::RefPtr<Gdk::Window>& w, const Glib::RefPtr<Gdk::GC>& gc) {
	signal_draw(this, d, w, gc);
}
