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
    Update Date:      $Date: 2006/03/06 16:01:23 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/widgets/aseriesmenu.cpp,v $
    CVS/RCS Revision: $Revision: 1.2 $
    Status:           $State: Exp $
*/

#include "aseriesmenu.h"
#include "poolseries.h"
#include "poolinstance.h"
#include "seriesview.h"
#include "asimpledisplay.h"

#include <iostream>
#include "gettext.h"

namespace Aeskulap {

void SeriesMenu::add_series(const Glib::RefPtr<ImagePool::Series>& series, SeriesView* w) {
	char buffer[200];
	if(!series->description().empty()) {
		g_snprintf(buffer, sizeof(buffer), gettext("Series %i (%s)\n%s"), m_menuitem.size()+1, series->modality().c_str(), series->description().c_str());
	}
	else {
		g_snprintf(buffer, sizeof(buffer), gettext("Series %i (%s)\nNo description"), m_menuitem.size()+1, series->modality().c_str());
		
	}

	Gtk::ImageMenuItem* menuitem = manage(new Gtk::ImageMenuItem(buffer, true));
	add(*menuitem);
	menuitem->show();
	int key = GDK_a + m_menuitem.size();
	Gtk::AccelGroup::activate(*menuitem, key, Gdk::CONTROL_MASK);
	m_menuitem[series->seriesinstanceuid()] = menuitem;
	m_views[series->seriesinstanceuid()] = w;
}

void SeriesMenu::set_thumbnail(const Glib::RefPtr<ImagePool::Instance>& instance) {
	Gtk::ImageMenuItem* menuitem = m_menuitem[instance->series()->seriesinstanceuid()];
	if(menuitem == NULL) {
		return;
	}

	if(menuitem->get_image() != NULL) {
		return;
	}

	Aeskulap::SimpleDisplay* image = manage(new Aeskulap::SimpleDisplay);
	image->set_image(instance);
	menuitem->set_image(*image);
	return;
}

void SeriesMenu::set_connection(const Glib::RefPtr<ImagePool::Series>& series, const sigc::slot<void, SeriesView*>& slot) {
	std::string uid = series->seriesinstanceuid();
	Gtk::ImageMenuItem* menuitem = m_menuitem[uid];
	if(menuitem == NULL) {
		return;
	}

	if(m_connections[uid]) {
		m_connections[uid].disconnect();
	}
	m_connections[uid] = menuitem->signal_activate().connect(sigc::bind(slot, m_views[uid]));
}

bool SeriesMenu::get_index(const Gtk::MenuItem& item, int& index) {
	Gtk::Menu_Helpers::MenuList::iterator i = items().begin();
	index = 0;
	for(; i != items().end(); i++) {
		if(i->gobj() == item.gobj()) {
			return true;
		}
		index++;
	}
	return false;
}

void SeriesMenu::swap_entries(const Glib::RefPtr<ImagePool::Series>& series1, const Glib::RefPtr<ImagePool::Series>& series2) {
	Gtk::ImageMenuItem* menuitem1 = m_menuitem[series1->seriesinstanceuid()];
	Gtk::ImageMenuItem* menuitem2 = m_menuitem[series2->seriesinstanceuid()];

	int index1, index2;
	if(!get_index(*menuitem1, index1)) {
		return;
	}
	if(!get_index(*menuitem2, index2)) {
		return;
	}

	reorder_child(*menuitem1, index2);
	reorder_child(*menuitem2, index1);
}

} // namespace Aeskulap
