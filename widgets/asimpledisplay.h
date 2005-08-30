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
    Update Date:      $Date: 2005/08/30 19:47:55 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/widgets/asimpledisplay.h,v $
    CVS/RCS Revision: $Revision: 1.3 $
    Status:           $State: Exp $
*/

#ifndef AESKULAP_SIMPLEDISPLAY_H
#define AESKULAP_SIMPLEDISPLAY_H

#include <gtkmm.h>
#include <gdkmm/pixbuf.h>
#include <gdkmm/event.h>
#include <glibmm/refptr.h>
#include <glibmm/object.h>

#include "poolinstance.h"
#include "displayparameters.h"

namespace Aeskulap {

class SimpleDisplay : public Gtk::EventBox {
public:

	SimpleDisplay();

	SimpleDisplay(const Glib::RefPtr<DisplayParameters>& params);

	virtual ~SimpleDisplay();

	bool set_image(const Glib::RefPtr<ImagePool::Instance>& image, bool smooth = true);

	bool set_image(const Glib::RefPtr<ImagePool::Instance>& image, const Glib::RefPtr<DisplayParameters>& params, bool smooth);

	void set_windowlevels(int c, int w);

	void update();

	void refresh(bool smooth = true);

	void set_id(int id);
	
	int get_id();
	
	void render(Glib::RefPtr<Gdk::Pixbuf>& pixbuf, bool smooth);

	bool point_to_screen(const ImagePool::Instance::Point& p, int& x, int& y);

	bool screen_to_point(int x, int y, ImagePool::Instance::Point& p);

protected:

	bool get_blit_rectangles(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf, int& sx0, int& sy0, int& sx1, int& sy1, int& dx0, int& dy0, int& dx1, int& dy1);

	void on_realize();

	void on_unrealize();

	void on_check_resize();

	void on_size_request(Gtk::Requisition* requisition);
	
	void on_size_allocate(Gtk::Allocation& allocation);

	bool on_expose_event(GdkEventExpose* event);

	void bitstretch(bool smooth = false);

	void get_zoom_wh(int& w, int& h);

private:

	void init_display();

	void linestretch_24to24(int x1, int x2, int y1, int y2, int yr, int yw, guint8* src_pixels, guint8* dst_pixels, guint8* lut);

	void rectstretch_24to24(guint8* src, int xs1, int ys1, int xs2, int ys2, const Glib::RefPtr<Gdk::Pixbuf>& pixbuf, int xd1, int yd1, int xd2, int yd2);

	template < class ST >
	void linestretch_24(int x1, int x2, int y1, int y2, int yr, int yw, ST src_pixels, guint8* dst_pixels, guint8* lut);

	template < class ST >
	void rectstretch_24(ST src, int xs1, int ys1, int xs2, int ys2, const Glib::RefPtr<Gdk::Pixbuf>& pixbuf, int xd1, int yd1, int xd2, int yd2);

	void rect_stretch8(int xs1, int ys1, int xs2, int ys2, int xd1, int yd1, int xd2, int yd2, const Glib::RefPtr<Gdk::Pixbuf>& pixbuf);

	void rect_stretch16(int xs1, int ys1, int xs2, int ys2, int xd1, int yd1, int xd2, int yd2, const Glib::RefPtr<Gdk::Pixbuf>& pixbuf);

	void rect_stretch24(int xs1, int ys1, int xs2, int ys2, int xd1, int yd1, int xd2, int yd2, const Glib::RefPtr<Gdk::Pixbuf>& pixbuf);

	void create_windowmap();

protected:

	// window(levels) map

	int m_windowmap_depth;

	guint8* m_windowmap;

	int m_windowmap_size;

	double m_magnifier;

	int m_id;
	
	// external references

	Glib::RefPtr<ImagePool::Instance> m_image;

	Glib::RefPtr<DisplayParameters> m_disp_params;

	Glib::RefPtr<Gdk::GC> m_GC;

	Glib::RefPtr<Gdk::Pixbuf> m_pixbuf;
	
	Glib::RefPtr<Pango::Context> m_pangoctx;

	Gtk::Frame* m_bin;

	Glib::RefPtr<Gdk::Window> m_window;

	Glib::RefPtr<Gdk::Window> m_bin_window;

	Glib::RefPtr<Gdk::Colormap> m_colormap;

	Gdk::Color m_colorBackground;
};

} // namespace Aeskulap

#endif // AESKULAP_SIMPLEDISPLAY_H

