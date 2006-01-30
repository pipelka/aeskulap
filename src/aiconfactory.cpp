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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    Alexander Pipelka
    pipelka@teleweb.at

    Last Update:      $Author: braindead $
    Update Date:      $Date: 2006/01/30 09:09:34 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/src/aiconfactory.cpp,v $
    CVS/RCS Revision: $Revision: 1.9 $
    Status:           $State: Exp $
*/

#include "aiconfactory.h"
#include "astockids.h"
#include "binreloc.h"

#include <iostream>

namespace Aeskulap {

std::string IconFactory::m_datadir;
std::string IconFactory::m_imagesdir;

IconFactory::IconFactory() {
	if(m_datadir.empty()) {
        	m_datadir = br_find_data_dir(AESKULAP_DATADIR);
		m_imagesdir = m_datadir + "/aeskulap/images";
	}

	add(Stock::GRID_1X1, "grid-1.png");
	add(Stock::GRID_1X2, "grid-2h.png");
	add(Stock::GRID_2X1, "grid-2v.png");
	add(Stock::GRID_2X2, "grid-4.png");
	add(Stock::GRID_4X4, "grid-16.png");
//	add(Stock::GRID_4X4, "grid-16.png");
	add(Stock::SERIES_SINGLE, "stock-tool-scale-22.png");
	add(Stock::SERIES_ALL, "stock-layers-24.png");
	add(Stock::SERIES_1X1, "series-1x1.png");
	add(Stock::SERIES_2X1, "series-2x1.png");
	add(Stock::SERIES_2X2, "series-2x2.png");
	add(Stock::SERIES_3X2, "series-3x2.png");
	add(Stock::SERIES_3X3, "series-3x3.png");
	add(Stock::SERIES_4X4, "series-4x4.png");
	add(Stock::REFFRAME, "stock-layers-24.png");
	add(Stock::DRAW_ERASER, "stock-tool-eraser-22.png");
	add(Stock::THREEDEE_CURSOR, "cursor_pan.png");
	add(Stock::MEASURE, "stock-tool-measure-22.png");
	
	Stock::init_stock_items();

	Gtk::IconFactory::add_default();
}

IconFactory::~IconFactory() {
	std::vector<Gtk::IconSet*>::iterator i = m_iconset.begin();
	for( ; i!= m_iconset.end(); i++) {
		delete (*i);
	}
	m_iconset.clear();
	Gtk::IconFactory::remove_default();
}

void IconFactory::add(const Gtk::StockID& stock_id, const std::string& filename) {
	Glib::RefPtr<Gdk::Pixbuf> pixbuf = load_from_file(filename);
	if(!pixbuf) {
		std::cerr << "unable to load file " << filename << std::endl;
		return;
	}
	Gtk::IconSet* set = new Gtk::IconSet(pixbuf);
	m_iconset.push_back(set);
	Gtk::IconFactory::add(stock_id, *set);
}
	
Glib::RefPtr<Gdk::Pixbuf> IconFactory::load_from_file(const std::string& filename) {
	Glib::RefPtr<Gdk::Pixbuf> pixbuf;
	std::string path;
	
	try {
		path = m_imagesdir + std::string("/") + filename;
		std::cerr << "trying to load '" << path << "'" << std::endl;
		pixbuf = Gdk::Pixbuf::create_from_file(path);
	}
	catch(Glib::FileError e) {
		try {
			path = "../pixmaps/"+filename;
			std::cerr << "trying to load '" << path << "'" << std::endl;
			pixbuf = Gdk::Pixbuf::create_from_file(path);
		}
		catch(Glib::FileError e) {
			std::cerr << "failed!" << std::endl;
			return Glib::RefPtr<Gdk::Pixbuf>();
		}
	}

	return pixbuf;
}

} // namespace Aeskulap
