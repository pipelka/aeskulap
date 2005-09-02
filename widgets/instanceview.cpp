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
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/widgets/Attic/instanceview.cpp,v $
    CVS/RCS Revision: $Revision: 1.1 $
    Status:           $State: Exp $
*/

#include "instanceview.h"
#include "seriesview.h"
#include "adisplay.h"

static void create_singleframe_display(Aeskulap::Display*& display, Gtk::Widget*& control) {
	display = manage(new Aeskulap::Display);
	control = NULL;
}

InstanceView* InstanceView::create(InstanceView::Type type, SeriesView* seriesview) {
	InstanceView* instance = new InstanceView(type);
	
	Aeskulap::Display* display = NULL;
	Gtk::Widget* control = NULL;

	if(type == InstanceView::SINGLE) {
		create_singleframe_display(display, control);
	}

	instance->m_display = display;
	
	display->show();
	instance->pack_start(*display);
	if(control != NULL) {
		control->show();
		instance->pack_start(*control, Gtk::PACK_SHRINK);
	}

	// connect signals to seriesview
	display->signal_scroll_event().connect(sigc::mem_fun(*seriesview, &SeriesView::on_scroll_event));
	display->signal_selected.connect(sigc::mem_fun(*seriesview, &SeriesView::on_image_selected));
	display->signal_changed.connect(sigc::mem_fun(*seriesview, &SeriesView::on_image_changed));
	display->signal_draw.connect(sigc::mem_fun(*seriesview, &SeriesView::on_draw_instance));
	display->signal_popup.connect(sigc::bind(seriesview->signal_popup, seriesview));

	return instance;
}

InstanceView::InstanceView(Type type) : m_type(type) {
}
	
Aeskulap::Display* InstanceView::get_display() {
	return m_display;
}
