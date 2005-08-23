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
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/widgets/aseriesmenu.h,v $
    CVS/RCS Revision: $Revision: 1.1 $
    Status:           $State: Exp $
*/

#ifndef AESKULAP_SERIESMENU_H
#define AESKULAP_SERIESMENU_H

#include <gtkmm.h>

class SeriesView;

namespace ImagePool {
	class Series;
	class Instance;
}

namespace Aeskulap {

class SeriesMenu : public Gtk::Menu {
public:

	void add_series(const Glib::RefPtr<ImagePool::Series>& series, SeriesView* w);
	
	void set_thumbnail(const Glib::RefPtr<ImagePool::Instance>& instance);

	void set_connection(const Glib::RefPtr<ImagePool::Series>& series, const sigc::slot<void, SeriesView*>& slot);

	void swap_entries(const Glib::RefPtr<ImagePool::Series>& series1, const Glib::RefPtr<ImagePool::Series>& series2);

protected:

	bool get_index(const Gtk::MenuItem& item, int& index);

	std::map< std::string, Gtk::ImageMenuItem* > m_menuitem;

	std::map< std::string, sigc::connection > m_connections;

	std::map< std::string, SeriesView* > m_views;

};

} // namespace Aeskulap

#endif // AESKULAP_SERIESMENU_H
