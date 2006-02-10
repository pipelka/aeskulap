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
    Update Date:      $Date: 2006/02/10 12:26:15 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/src/aiconfactory.h,v $
    CVS/RCS Revision: $Revision: 1.5 $
    Status:           $State: Exp $
*/

#ifndef AESKULAP_ICONFACTORY_H
#define AESKULAP_ICONFACTORY_H

#include <gtkmm.h>
#include <gdkmm/pixbuf.h>
#include <vector>
#include <string>

namespace Aeskulap {

class IconFactory : public Gtk::IconFactory {
public:

	IconFactory();

	~IconFactory();

	static Glib::RefPtr<Gdk::Pixbuf> load_from_file(const std::string& filename);

	static Gdk::Cursor& get_cursor_watch();

protected:

	void add(const Gtk::StockID& stock_id, const std::string& filename);

private:

	std::vector<Gtk::IconSet*> m_iconset;
	
	static std::string m_datadir;
	
	static std::string m_imagesdir;
	
	static Gdk::Cursor* m_cursor_watch;
	
	static std::map< std::string, Glib::RefPtr<Gdk::Pixbuf> > m_imagemap;
};

} // namespace Aeskulap

#endif // AESKULAP_ICONFACTORY_H
