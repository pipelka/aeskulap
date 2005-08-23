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
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/widgets/asimpledisplay.cpp,v $
    CVS/RCS Revision: $Revision: 1.1 $
    Status:           $State: Exp $
*/

#include <iostream>
#include <cmath>

#include "asimpledisplay.h"
#include "poolstudy.h"
#include "aiconfactory.h"

#define sign(x) ((x)>0 ? 1:-1)

namespace Aeskulap {

SimpleDisplay::SimpleDisplay() {
	
	m_disp_params = DisplayParameters::create();

	// set default display parameters

	m_disp_params->window_center = 0;
	m_disp_params->window_width = 0;

	m_disp_params->zoom_factor = 100.0;

	m_disp_params->move_x = 0;
	m_disp_params->move_y = 0;

	m_disp_params->selected = false;

	init_display();
}

SimpleDisplay::SimpleDisplay(const Glib::RefPtr<DisplayParameters>& params) {
	m_disp_params = params;
	init_display();
}

SimpleDisplay::~SimpleDisplay() {
	if(m_windowmap != NULL) {
		free(m_windowmap);
	}
}

void SimpleDisplay::init_display() {

	m_windowmap = NULL;
	m_windowmap_depth = 8;
	m_windowmap_size = 0;
	m_magnifier = 1;

	m_id = 0;

	m_pixbuf = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, false, 8, 64, 64);
	bitstretch();

	m_bin = manage(new Gtk::Frame);
	add(*m_bin);
	m_bin->show();
}

void SimpleDisplay::on_realize() {
	Gtk::EventBox::on_realize();

	m_window = get_parent_window();

	m_bin_window = m_bin->get_parent_window();

	m_colormap = m_bin->get_default_colormap();

	m_colorBackground = Gdk::Color("black");
	m_colormap->alloc_color(m_colorBackground);

	m_GC = Gdk::GC::create(m_bin_window);

	m_pangoctx = create_pango_context();
}

void SimpleDisplay::on_unrealize() {
    m_GC.clear();
    Gtk::EventBox::on_unrealize();
}

void SimpleDisplay::on_check_resize() {
	Gtk::EventBox::on_check_resize();
}

void SimpleDisplay::on_size_request(Gtk::Requisition* requisition) {
	*requisition = Gtk::Requisition();
	requisition->width = 64;
	requisition->height = 64;
}
	
void SimpleDisplay::on_size_allocate(Gtk::Allocation& allocation) {
	set_allocation(allocation);

    if(allocation.get_width() != m_pixbuf->get_width() || allocation.get_height() != m_pixbuf->get_height()) {
	    m_pixbuf = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, false, 8, allocation.get_width(), allocation.get_height());
	    bitstretch(true);
    }

	if(m_window) {
		m_window->move_resize( allocation.get_x(), allocation.get_y(), allocation.get_width(), allocation.get_height() );
	}
	
	if(m_bin_window) {
		m_bin_window->move_resize( allocation.get_x(), allocation.get_y(), allocation.get_width(), allocation.get_height() );
	}
}

bool SimpleDisplay::on_expose_event(GdkEventExpose* event) {
	if(!m_GC) {
		return false;
	}

	if(m_image) {
		m_bin_window->draw_pixbuf(
			m_GC, 
			m_pixbuf, 
			event->area.x, 
			event->area.y, 
			event->area.x, 
			event->area.y, 
			event->area.width, 
			event->area.height, 
			Gdk::RGB_DITHER_NONE, 
			Gdk::RGB_DITHER_NONE, 
			Gdk::RGB_DITHER_NONE);
	}
	else {
		m_GC->set_foreground(m_colorBackground);
		m_GC->set_background(m_colorBackground);
		m_bin_window->draw_rectangle(
			m_GC,
			true,
			event->area.x, 
			event->area.y, 
			event->area.width, 
			event->area.height);
	}
		
	return true;
}

bool SimpleDisplay::set_image(const Glib::RefPtr<ImagePool::Instance>& image, bool smooth) {
	if(!image) {
		return false;
	}

	m_image = image;
	
	if(m_image->iscolor()) {
		m_windowmap_depth = 8;
	}
	else {
		m_windowmap_depth = m_image->depth();
	}

	if(m_disp_params->window_center == 0) {
		m_disp_params->window_center = m_image->default_windowcenter();
	}
	
	if(m_disp_params->window_center == 0) {
		m_disp_params->window_center = (1 << m_image->depth()) / 2;
	}

	if(m_disp_params->window_width == 0) {
		m_disp_params->window_width = m_image->default_windowwidth();
	}
	
	if(m_disp_params->window_width == 0) {
		m_disp_params->window_width = 1 << m_image->depth();
	}

	create_windowmap();
	set_windowlevels(m_disp_params->window_center, m_disp_params->window_width);
	bitstretch(smooth);

	queue_draw();
	return true;
}

bool SimpleDisplay::set_image(const Glib::RefPtr<ImagePool::Instance>& image, const Glib::RefPtr<DisplayParameters>& params, bool smooth) {
	if(!image) {
		return false;
	}

	m_image = image;
	
	if(m_image->iscolor()) {
		m_windowmap_depth = 8;
	}
	else {
		m_windowmap_depth = m_image->depth();
	}

	m_disp_params = params;

	create_windowmap();
	set_windowlevels(m_disp_params->window_center, m_disp_params->window_width);
	bitstretch(smooth);

	queue_draw();
	return true;
}

void SimpleDisplay::create_windowmap() {
	if(m_windowmap != NULL) {
		free(m_windowmap);
	}

	m_windowmap_size = 1 << m_windowmap_depth;
	m_windowmap = (guint32*)malloc(sizeof(guint32)* m_windowmap_size);
}

void SimpleDisplay::set_windowlevels(int c, int w) {
	if(!m_image) {
		return;
	}

	m_disp_params->window_center = c;
	m_disp_params->window_width = w;

	int intercept = m_image->intercept();
	double slope = m_image->slope();
	//bool is_signed = m_image->is_signed();
	//int highbit = m_image->highbit();

	// handle slope, intercept
	c = (int)((c - intercept) / slope);
	w = (int)(w / slope);
	
	int ramp_start = 0;
	int ramp_end = m_windowmap_size/* - 1*/;

	float k = 256.0/(float)w;

	float c0 = (k * ((float)c - (float)w/2.0)) * -1.0;
	float g = 0;
	int r;
	guint32 v;

	for(int i = ramp_start; i < ramp_end; i++) {
		
		// get colorvalue
		g = (k*(float)(i)) + c0;
	
		if(g > 255) {
			g = 255;
		}
		if(g < 0) {
			g = 0;
		}

		r = (int)g;

		v = r;
		v += (r<<8);
		v += (r<<16);
		v += (255<<24);

		m_windowmap[i] = v;
	}
}

void SimpleDisplay::get_zoom_wh(int& w, int& h) {
	double dis_width = (double)get_width();
	double dis_height = (double)get_height();

	// try to scale on height

	double zf = (double)m_image->height() / dis_height;

	double zw = dis_width * zf;
	double zh = dis_height * zf;

	// scale on width

	if(zw < m_image->width()) {
		zf = (double)m_image->width() / dis_width;
		zw = dis_width * zf;
		zh = dis_height * zf;
	}

	zw *= (100.0 / m_disp_params->zoom_factor);
	zh *= (100.0 / m_disp_params->zoom_factor);

	if(zw > m_image->width()) {
		zw = m_image->width();
	}

	if(zh > m_image->height()) {
		zh = m_image->height();
	}

	w = (int)zw;
	h = (int)zh;
}

void SimpleDisplay::linestretch_24to24(int x1, int x2, int y1, int y2, int yr, int yw, guint8* src_pixels, guint8* dst_pixels, guint32* lut) {
	int dx, dy, e, d, dx2;

	guint32 p;

	dx = (x2 - x1);
	dy = (y2 - y1);

	dy <<= 1;
	e = dy - dx;
	dx2 = dx << 1;

	for (d = 0; d < dx; d++) {
		p = lut[*src_pixels];
		*dst_pixels++ = p & 0xFF;

		p = lut[*(src_pixels+1)];
		*dst_pixels++ = p & 0xFF;

		p = lut[*(src_pixels+2)];
		*dst_pixels++ = p & 0xFF;

		while (e >= 0) {
			src_pixels += 3;
			e -= dx2;
		}

		e += dy;
	}
}

void SimpleDisplay::rectstretch_24to24(guint8* src, int xs1, int ys1, int xs2, int ys2, const Glib::RefPtr<Gdk::Pixbuf>& pixbuf, int xd1, int yd1, int xd2, int yd2) {
	int dx, dy, e, d, dx2;
	int sx, sy;
	dx = abs((int)(yd2 - yd1));
	dy = abs((int)(ys2 - ys1));
	sx = sign(yd2 - yd1);
	sy = sign(ys2 - ys1);
	e = (dy << 1)-dx;
	dx2 = dx << 1;
	dy <<= 1;

	guint8* dst = (guint8*)pixbuf->get_pixels();
	int src_bpp = 3; //m_image_depth / 8;
	int dst_bpp = 3;

	guint16 src_pitch = m_image->width() * src_bpp;
	guint16 dst_pitch = pixbuf->get_rowstride();

	long src_pixels = ((long)src + ys1 * src_pitch + xs1 * src_bpp);
	long dst_pixels = ((long)dst + yd1 * dst_pitch + xd1 * dst_bpp);
	guint32* lut = m_windowmap;

	int dst_h = m_pixbuf->get_height();

	// Stretch with lookup table
	for (d = 0; (d <= dx) && (yd1 < dst_h) && (ys1 < m_image->height()); d++) {
		linestretch_24to24(xd1, xd2, xs1, xs2, ys1, yd1, (guint8*)src_pixels, (guint8*)dst_pixels, lut);

		while (e >= 0) {
			src_pixels += src_pitch;
			ys1++;
			e -= dx2;
		}
		dst_pixels += dst_pitch;
		yd1++;
		e += dy;
	}

}

template < class ST >
void SimpleDisplay::linestretch_24(int x1, int x2, int y1, int y2, int yr, int yw, ST src_pixels, guint8* dst_pixels, guint32* lut) {
	int dx, dy, e, d, dx2;

	guint32 p;

	dx = (x2 - x1);
	dy = (y2 - y1);

	dy <<= 1;
	e = dy - dx;
	dx2 = dx << 1;

	for (d = 0; d < dx; d++) {
		p = lut[*src_pixels];
		*dst_pixels++ = p & 0xFF;
		*dst_pixels++ = (p >> 8) & 0xFF;
		*dst_pixels++ = (p >> 16) & 0xFF;

		while (e >= 0) {
			src_pixels ++;
			e -= dx2;
		}

		e += dy;
	}
}

template < class ST >
void SimpleDisplay::rectstretch_24(ST src, int xs1, int ys1, int xs2, int ys2, const Glib::RefPtr<Gdk::Pixbuf>& pixbuf, int xd1, int yd1, int xd2, int yd2) {
	int dx, dy, e, d, dx2;
	int sx, sy;
	dx = abs((int)(yd2 - yd1));
	dy = abs((int)(ys2 - ys1));
	sx = sign(yd2 - yd1);
	sy = sign(ys2 - ys1);
	e = (dy << 1)-dx;
	dx2 = dx << 1;
	dy <<= 1;

	guint8* dst = (guint8*)pixbuf->get_pixels();
	int src_bpp = m_image->bpp() / 8;
	int dst_bpp = 3;

	guint16 src_pitch = m_image->width() * src_bpp;
	guint16 dst_pitch = pixbuf->get_rowstride();

	long src_pixels = ((long)src + ys1 * src_pitch + xs1 * src_bpp);
	long dst_pixels = ((long)dst + yd1 * dst_pitch + xd1 * dst_bpp);
	guint32* lut = m_windowmap;

	int dst_h = pixbuf->get_height();

	// Stretch with lookup table
	for (d = 0; (d <= dx) && (yd1 < dst_h) && (ys1 < m_image->height()); d++) {
		linestretch_24(xd1, xd2, xs1, xs2, ys1, yd1, (ST)src_pixels, (guint8*)dst_pixels, lut);

		while (e >= 0) {
			src_pixels += src_pitch;
			ys1++;
			e -= dx2;
		}
		dst_pixels += dst_pitch;
		yd1++;
		e += dy;
	}

}

void SimpleDisplay::rect_stretch8(int xs1, int ys1, int xs2, int ys2, int xd1, int yd1, int xd2, int yd2, const Glib::RefPtr<Gdk::Pixbuf>& pixbuf) {
	guint8* src_pixels = (guint8*)m_image->pixels();

	rectstretch_24(src_pixels, xs1, ys1, xs2, ys2, pixbuf, xd1, yd1, xd2, yd2);
}

void SimpleDisplay::rect_stretch16(int xs1, int ys1, int xs2, int ys2, int xd1, int yd1, int xd2, int yd2, const Glib::RefPtr<Gdk::Pixbuf>& pixbuf) {
	guint16* src_pixels = (guint16*)m_image->pixels();

	rectstretch_24(src_pixels, xs1, ys1, xs2, ys2, pixbuf, xd1, yd1, xd2, yd2);
}

void SimpleDisplay::rect_stretch24(int xs1, int ys1, int xs2, int ys2, int xd1, int yd1, int xd2, int yd2, const Glib::RefPtr<Gdk::Pixbuf>& pixbuf) {
	guint8* src_pixels = (guint8*)m_image->pixels();

	rectstretch_24to24(src_pixels, xs1, ys1, xs2, ys2, pixbuf, xd1, yd1, xd2, yd2);
}

void SimpleDisplay::render(Glib::RefPtr<Gdk::Pixbuf>& pixbuf, bool smooth) {
	int sx0,sy0,sx1,sy1;
	int dx0,dy0,dx1,dy1;

	if(!get_blit_rectangles(pixbuf, sx0, sy0, sx1, sy1, dx0, dy0, dx1, dy1)) {
		return;
	}

	if(dx0 > 0 || dy0 > 0 || dx1 < pixbuf->get_width()-1 || dy1 < pixbuf->get_height()-1) {
	    pixbuf->fill(0x0000FF);
	}

	// do a smooth scale (slow)
	if(smooth) {
		Glib::RefPtr<Gdk::Pixbuf> pixbuf_save = pixbuf;
	    pixbuf = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, false, 8, sx1-sx0/*+1*/, sy1-sy0/*+1*/);

		if(m_image->iscolor()) {
			rect_stretch24(sx0, sy0, sx1, sy1, 0,  0, sx1-sx0, sy1-sy0, pixbuf);
		}
		else {
			switch(m_image->bpp()) {
				case 8:
					rect_stretch8(sx0, sy0, sx1, sy1, 0,  0, sx1-sx0, sy1-sy0, pixbuf);
					break;
				case 16:
					rect_stretch16(sx0, sy0, sx1, sy1, 0,  0, sx1-sx0, sy1-sy0, pixbuf);
					break;
				default:
					break;
			}
		}
		
		// create scaled pixbuf
		Glib::RefPtr<Gdk::Pixbuf> pixbuf_scaled = pixbuf->scale_simple(dx1-dx0+1, dy1-dy0+1, Gdk::INTERP_BILINEAR);

		pixbuf_save->fill(0x0000FF);
		pixbuf_scaled->copy_area(0, 0, dx1-dx0+1, dy1-dy0+1, pixbuf_save, dx0, dy0);
		
		pixbuf = pixbuf_save;
		return;
	}
	
	// do a super fast blocky scale
	if(m_image->iscolor()) {
		rect_stretch24(sx0, sy0, sx1, sy1, dx0,  dy0, dx1, dy1, pixbuf);
		return;
	}

	switch(m_image->bpp()) {
		case 8:
			rect_stretch8(sx0, sy0, sx1, sy1, dx0,  dy0, dx1, dy1, pixbuf);
			break;
		case 16:
			rect_stretch16(sx0, sy0, sx1, sy1, dx0,  dy0, dx1, dy1, pixbuf);
			break;
		default:
			break;
	}
}

bool SimpleDisplay::get_blit_rectangles(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf, int& sx0, int& sy0, int& sx1, int& sy1, int& dx0, int& dy0, int& dx1, int& dy1) {
	if(!m_image) {
		return false;
	}
	if(!pixbuf) {
		return false;
	}

	int zoom_w;
	int zoom_h;
	int width = pixbuf->get_width();
	int height = pixbuf->get_height();

	get_zoom_wh(zoom_w, zoom_h);

	if((m_disp_params->move_x + zoom_w) > m_image->width()) {
		m_disp_params->move_x = (m_image->width() - zoom_w);
	}

	if(m_disp_params->move_x < 0) {
		m_disp_params->move_x = 0;
	}

	if((m_disp_params->move_y + zoom_h) > m_image->height()) {
		m_disp_params->move_y = (m_image->height() - zoom_h);
	}

	if(m_disp_params->move_y < 0) {
		m_disp_params->move_y = 0;
	}

	sx0 = m_disp_params->move_x;
	sy0 = m_disp_params->move_y;
	sx1 = m_disp_params->move_x+zoom_w-1;
	sy1 = m_disp_params->move_y+zoom_h-1;

	dx0 = 0;
	dy0 = 0;
	dx1 = width-1;
	dy1 = height-1;

	int ih = (int)(((double)(sy1-sy0+1) / (double)(sx1-sx0+1)) * (double)(dx1-dx0+1));
	int iw = (int)(((double)(sx1-sx0+1) / (double)(sy1-sy0+1)) * (double)(dy1-dy0+1));

	if(ih < dy1) {
		if((dy1-ih)/2 > 0) {
			dy0 = (dy1-ih)/2;
			dy1 = dy0+ih;
		}
	}

	if(iw < dx1) {
		if((dx1-iw)/2 > 0) {
			dx0 = (dx1-iw)/2;
			dx1 = dx0+iw;
		}
	}

	return true;
}

void SimpleDisplay::bitstretch(bool smooth) {

	int sx0,sy0,sx1,sy1;
	int dx0,dy0,dx1,dy1;

	if(!get_blit_rectangles(m_pixbuf, sx0, sy0, sx1, sy1, dx0, dy0, dx1, dy1)) {
		return;
	}

	m_magnifier = (double)(dx1-dx0)/(double)(sx1-sx0);

	render(m_pixbuf, smooth);
}

void SimpleDisplay::update() {
	if(!m_GC) {
		return;
	}

	Gdk::Rectangle r(0,0,get_width(),get_height());
	m_bin_window->invalidate_rect(r, false);
	m_bin_window->process_updates(false);
}

void SimpleDisplay::refresh(bool smooth) {
	set_windowlevels(m_disp_params->window_center, m_disp_params->window_width);
	bitstretch(smooth);
	update();
}

void SimpleDisplay::set_id(int id) {
	m_id = id;
}
	
int SimpleDisplay::get_id() {
	return m_id;
}

bool SimpleDisplay::point_to_screen(const ImagePool::Instance::Point& p, int& x, int& y) {
	if(m_image->spacing_x() == 0 || m_image->spacing_y() == 0) {
		return false;
	}

	int sx0,sy0,sx1,sy1;
	int dx0,dy0,dx1,dy1;

	if(!get_blit_rectangles(m_pixbuf, sx0, sy0, sx1, sy1, dx0, dy0, dx1, dy1)) {
		return false;
	}

	double mx = (double)m_disp_params->move_x * m_image->spacing_x();
	double my = (double)m_disp_params->move_y * m_image->spacing_y();
	
	x = dx0 + (int)(((p.x - mx) / m_image->spacing_x()) * m_magnifier);
	y = dy0 + (int)(((p.y - my) / m_image->spacing_y()) * m_magnifier);

	return true;
}

} // namespace Aeskulap
