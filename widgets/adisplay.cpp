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
    Update Date:      $Date: 2005/09/02 13:11:52 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/widgets/adisplay.cpp,v $
    CVS/RCS Revision: $Revision: 1.5 $
    Status:           $State: Exp $
*/

#include <iostream>
#include <cmath>

#include "adisplay.h"
#include "poolstudy.h"
#include "aiconfactory.h"
#include "gettext.h"

namespace Aeskulap {

Display::Display() : SimpleDisplay() ,
m_cursor_pan(NULL),
m_cursor_windowlevel(NULL),
m_cursor_zoom(NULL)
{
	set_events(Gdk::BUTTON_PRESS_MASK);
}

Display::Display(const Glib::RefPtr<DisplayParameters>& params) : SimpleDisplay(params) ,
m_cursor_pan(NULL),
m_cursor_windowlevel(NULL),
m_cursor_zoom(NULL)
{
	set_events(Gdk::BUTTON_PRESS_MASK);
}

Display::~Display() {
	delete m_cursor_pan;
	delete m_cursor_windowlevel;
	delete m_cursor_zoom;

}

void Display::on_realize() {
	SimpleDisplay::on_realize();

	m_fntdesc.set_family("sans");

	Glib::RefPtr<Gdk::Pixbuf> p;
	p = Aeskulap::IconFactory::load_from_file("cursor_pan.png");
	if(p) {
		m_cursor_pan = new Gdk::Cursor(
						Gdk::Display::get_default(),
						p,
						11, 11);
	}
	
	p = Aeskulap::IconFactory::load_from_file("cursor_windowlevel.png");
	if(p) {
		m_cursor_windowlevel = new Gdk::Cursor(
						Gdk::Display::get_default(),
						p,
						11, 11);
	}
	
	p = Aeskulap::IconFactory::load_from_file("cursor_zoom.png");
	if(p) {
		m_cursor_zoom = new Gdk::Cursor(
						Gdk::Display::get_default(),
						p,
						11, 11);
	}

	m_changed = false;

	m_drag_active = false;
	m_drag_button = 0;
	m_drag_start_x = 0;
	m_drag_start_y = 0;

	m_colorSelected = Gdk::Color("green");
	m_colorSeriesSelected = Gdk::Color("orange");
	m_colorFrame  = Gdk::Color("white");
	m_colorText = Gdk::Color("white");
	m_colorReferenceCurrent = Gdk::Color("orange");
	m_colorReference = Gdk::Color("grey");

	m_colormap->alloc_color(m_colorSelected);
	m_colormap->alloc_color(m_colorSeriesSelected);
	m_colormap->alloc_color(m_colorFrame);
	m_colormap->alloc_color(m_colorText);
	m_colormap->alloc_color(m_colorReferenceCurrent);
	m_colormap->alloc_color(m_colorReference);

	m_layoutL = Pango::Layout::create(m_pangoctx);
	m_layoutL->set_alignment(Pango::ALIGN_LEFT);

	m_layoutR = Pango::Layout::create(m_pangoctx);
	m_layoutR->set_alignment(Pango::ALIGN_RIGHT);

	m_layoutB = Pango::Layout::create(m_pangoctx);
	m_layoutB->set_alignment(Pango::ALIGN_LEFT);
}

bool Display::on_expose_event(GdkEventExpose* event) {
	if(!SimpleDisplay::on_expose_event(event)) {
		return false;
	}

	m_GC->set_foreground(m_colorFrame);

	if(m_disp_params && m_disp_params->selected) {
		m_GC->set_foreground(m_colorSelected);
		m_window->draw_rectangle(m_GC, false, 2, 2, get_width()-5, get_height()-5);
	}
	else if(m_disp_params && m_disp_params->series_selected) {
		m_GC->set_foreground(m_colorSeriesSelected);
	}

	m_window->draw_rectangle(m_GC, false, 1, 1, get_width()-3, get_height()-3);

	if(m_image) {
		m_GC->set_foreground(m_colorText);
	
		m_GC->set_background(m_colorText);
		m_fntdesc.set_size((get_width() * 12 * PANGO_SCALE) / 1280 + 2 * PANGO_SCALE);
		std::string text = m_image->series()->institutionname() + "\n";
		text += m_image->study()->patientsname() + "\n";
		text += m_image->study()->patientsbirthdate() + " " + m_image->study()->patientssex() + "\n";
		text += gettext("Acc:\n");
		text += m_image->date() + "\n";
		text += gettext("Acq Tm: ") + m_image->time() + "\n";
		text += "\n";
		
		char buffer[50];
		sprintf(buffer, "%i x %i", m_image->width(), m_image->height());

		text += buffer;
		
		m_layoutR->set_font_description(m_fntdesc);
		m_layoutR->set_text(text);
		m_layoutR->set_width((get_width()/2 - 10) * PANGO_SCALE);
		
		text = m_image->model() + "\n";
		sprintf(buffer, gettext("Image: %i / %i"), m_image->get_index(), m_image->series()->size());
		text += buffer;
	
		m_layoutL->set_font_description(m_fntdesc);
		m_layoutL->set_text(text);
		m_layoutL->set_width((get_width()/2 - 10) * PANGO_SCALE);

		sprintf(buffer, "C: %i\nW: %i", m_disp_params->window_center, m_disp_params->window_width);

		text = buffer;

		m_layoutB->set_font_description(m_fntdesc);
		m_layoutB->set_text(text);
		m_layoutB->set_width((get_width()/2 - 10) * PANGO_SCALE);
		int sw = 0;
		int sh = 0;
		m_layoutB->get_pixel_size(sw, sh);
		
		m_window->draw_layout(m_GC, 10, 5, m_layoutL);
		m_window->draw_layout(m_GC, get_width()/2, 5, m_layoutR);
		m_window->draw_layout(m_GC, 10, get_height()-5-sh, m_layoutB);

		draw_ruler_v();
		draw_ruler_h();
	}

	signal_draw(this, m_window, m_GC);

	return true;
}

bool Display::on_button_press_event(GdkEventButton* button) {
	if(m_drag_active) {
		return false;
	}

	if(!m_disp_params) {
		return false;
	}
	
	if(button->button == 1) {
		m_drag_active = true;
		m_drag_button = 1;
		m_drag_start_x = button->x;
		m_drag_start_y = button->y;
		m_drag_window_center = m_disp_params->window_center;
		m_drag_window_width = m_disp_params->window_width;

		get_window()->set_cursor(*m_cursor_windowlevel);
		add_modal_grab();
		return true;
	}

	if(button->button == 2) {
		m_drag_active = true;
		m_drag_button = 2;
		m_drag_start_x = 0;
		m_drag_start_y = button->y;

		get_window()->set_cursor(*m_cursor_zoom);
		add_modal_grab();
		return true;
	}

	if(button->button == 3) {
		if(button->state & GDK_CONTROL_MASK) {
			signal_popup(button);
			return true;
		}
		else {
			m_drag_active = true;
			m_drag_button = 3;
			m_drag_start_x = button->x;
			m_drag_start_y = button->y;
	
			get_window()->set_cursor(*m_cursor_pan);
			add_modal_grab();
			return true;
		}
	}

	return false;
}

bool Display::on_button_release_event(GdkEventButton* button) {

	Gtk::EventBox::on_button_release_event(button);

	if(!m_disp_params) {
		return false;
	}
	
	if(m_image && !m_disp_params->selected) {
		if(button->button == 1) {
			m_disp_params->selected = true;
			signal_selected(get_id());
		}
	}

	if(!m_drag_active) {
		return false;
	}

	get_window()->set_cursor();

	m_drag_active = false;
	m_drag_button = 0;
	remove_modal_grab();
	
	if(m_changed) {
		signal_changed(get_id(), true);

		bitstretch(true);
		queue_draw();
	}
	m_changed = false;

	return true;
}

bool Display::on_motion_notify_event(GdkEventMotion* event) {
	static bool block = false;

	if(!m_disp_params || !m_image) {
		return false;
	}

	if(!m_drag_active || block) {
		return true;
	}

	int x,y;
	get_pointer(x, y);
	
	if(m_drag_button == 1) {
		set_window_palette(x, y);
		update();
	}

	if(m_drag_button == 2) {
		gdouble dy = m_drag_start_y - y;

		int wo,ho;
		get_zoom_wh(wo, ho);

		int dxm = wo/2 + m_disp_params->move_x;
		int dym = ho/2 + m_disp_params->move_y;

		m_disp_params->zoom_factor += dy*2;

		if(m_disp_params->zoom_factor < 100) {
			m_disp_params->zoom_factor = 100;
		}
		
		if(m_disp_params->zoom_factor > 1000) {
			m_disp_params->zoom_factor = 1000;
		}

		get_zoom_wh(wo, ho);

		m_disp_params->move_x = dxm - wo/2;
		m_disp_params->move_y = dym - ho/2;

		bitstretch();
		
		m_drag_start_y = y;
		
		update();
	}

	if(m_drag_button == 3) {
		gdouble dx = x - m_drag_start_x;
		gdouble dy = y - m_drag_start_y;

		m_disp_params->move_x -= (int)dx;
		m_disp_params->move_y -= (int)dy;

		bitstretch();

		m_drag_start_x = x;
		m_drag_start_y = y;

		update();
	}

	m_changed = true;

	signal_changed(get_id(), false);

	block = true;
	while(Gtk::Main::events_pending()) Gtk::Main::iteration(false);
	block = false;

	return true;
}

void Display::set_window_palette(gdouble x, gdouble y) {
	gdouble c,w;

	// convert x,y to center & width of windowlevel
	gdouble dx = x - m_drag_start_x;
	gdouble dy = y - m_drag_start_y;

	c = m_drag_window_center + (dx / get_width()) * (m_windowmap_size-1)/2;
	w = m_drag_window_width + (dy / get_height()) * (m_windowmap_size-1)/2;

	if(c < 1 && !m_image->is_signed()) {
		c=1;
	}
	
	if(c > m_windowmap_size-1) {
		c = m_windowmap_size-1;
	}

	if(w < 1) {
		w = 1;
	}
	
	if(w > (m_windowmap_size-1)*2) {
		w = (m_windowmap_size-1)*2;
	}

	set_windowlevels((int)c, (int)w);
	bitstretch();
}

void Display::set_selected(bool selected) {
	m_disp_params->selected = selected;
	queue_draw();
}

void Display::draw_ruler_v() {
	if(!m_image) {
		return;
	}

	if(m_image->spacing_y() == 0) {
		return;
	}

	int ruler_height = get_height()/2;
	
	int line_width1 = (get_width() * 20) / 1280;
	int line_width2 = line_width1*2;

	int mm = (int)((ruler_height / m_magnifier) * m_image->spacing_y());
	
	// round to 10 mm
	mm /= 10;
	mm *= 10;
	
	ruler_height = (int)((mm / m_image->spacing_y()) * m_magnifier);
	
	int offset = (get_height() - ruler_height) / 2;

	m_window->draw_line(m_GC, get_width()-10, offset, get_width()-10, offset+ruler_height);
	
	int ym = 0;
	for(int y=0; y<mm+1; y+=10) {
		ym = (int)((y / m_image->spacing_y()) * m_magnifier);
		int lw = line_width1;
		div_t result;
		result = div (y, 50);
		if(result.rem == 0) {
			lw = line_width2;
		}
		m_window->draw_line(
			m_GC,
			get_width()-10-lw,
			offset+ym,
			get_width()-10,
			offset+ym);
	}

	char buffer[10];
	sprintf(buffer, gettext("%i mm"), mm);
	m_layoutR->set_text(buffer);
	m_layoutR->set_width(100 * PANGO_SCALE);

	m_window->draw_layout(m_GC, get_width()-110, offset + ym + 3, m_layoutR);

}

void Display::draw_ruler_h() {
	if(!m_image) {
		return;
	}

	if(m_image->spacing_x() == 0) {
		return;
	}

	int ruler_width = get_width()/2;
	
	int line_height1 = (get_width() * 20) / 1280;
	int line_height2 = line_height1*2;

	int mm = (int)((ruler_width / m_magnifier) * m_image->spacing_x());
	
	// round to 10 mm
	mm /= 10;
	mm *= 10;
	
	ruler_width = (int)((mm / m_image->spacing_x()) * m_magnifier);
	
	int offset = (get_width() - ruler_width) / 2;

	m_window->draw_line(m_GC, offset, get_height()-10, offset+ruler_width, get_height()-10);
	
	int xm = 0;
	for(int x=0; x<mm+1; x+=10) {
		xm = (int)((x / m_image->spacing_x()) * m_magnifier);
		int lh = line_height1;
		div_t result;
		result = div (x, 50);
		if(result.rem == 0) {
			lh = line_height2;
		}
		m_window->draw_line(
			m_GC,
			offset+xm,
			get_height()-10-lh,
			offset+xm,
			get_height()-10);
	}

	char buffer[10];
	sprintf(buffer, gettext("%i mm"), mm);
	m_layoutL->set_text(buffer);
	m_layoutL->set_width(100 * PANGO_SCALE);

	int tw = 0;
	int th = 0;
	m_layoutL->get_pixel_size(tw, th);

	m_window->draw_layout(m_GC, offset + xm + 3, get_height()-10-th, m_layoutL);

}

void Display::draw_line(const ImagePool::Instance::Point& p0, const ImagePool::Instance::Point& p1) {
	if(!m_image) {
		return;
	}

	int x0;
	int y0;
	int x1;
	int y1;
	ImagePool::Instance::Point s0;
	ImagePool::Instance::Point s1;

	if(!m_image->transform_to_viewport(p0, s0)) {
		return;
	}

	if(!point_to_screen(s0, x0, y0)) {
		return;
	}

	if(!m_image->transform_to_viewport(p1, s1)) {
		return;
	}

	if(!point_to_screen(s1, x1, y1)) {
		return;
	}

	if(x0 == x1 && y0 == y1) {
		return;
	}

	m_window->draw_line(m_GC, x0, y0, x1, y1);
}

void Display::draw_point(const ImagePool::Instance::Point& p) {
	if(!m_image) {
		return;
	}

	int x;
	int y;
	ImagePool::Instance::Point p1;
	
	if(!m_image->transform_to_viewport(p, p1)) {
		return;
	}

	if(!point_to_screen(p1, x, y)) {
		return;
	}
	
	//ImagePool::Instance::Point t, t1;
	//screen_to_point(x, y, t);
	//m_image->transform_to_world(t, t1);
	
	//std::cout << "O-Point: " << p.x << "/" << p.y << "/" << p.z << std::endl;
	//std::cout << "T-Point: " << t1.x << "/" << t1.y << "/" << t1.z << std::endl;
	//std::cout << "Screen: " << x << "/" << y << std::endl;
	
	m_window->draw_rectangle(m_GC, false, x-1, y-1, 2, 2);
}


} // namespace Aeskulap
