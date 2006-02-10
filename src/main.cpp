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
    Update Date:      $Date: 2006/02/10 12:03:38 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/src/main.cpp,v $
    CVS/RCS Revision: $Revision: 1.7 $
    Status:           $State: Exp $
*/

#include "imagepool.h"
#include "mainwindow.h"

#include "aiconfactory.h"
#include "abusycursor.h"

#include "binreloc.h"

#include <gtkmm.h>
#include <libglademm/xml.h>
#include <gconfmm.h>

#include <iostream>

int main(int argc, char* argv[]) {
	ImagePool::init();

	Gtk::Main kit(argc, argv);

	br_init(NULL);

	std::string datadir = br_find_data_dir(AESKULAP_DATADIR);
	std::cout << "datadir: " << datadir << std::endl;

	std::string localedir = datadir + "/locale";
	std::string gladedir = datadir + "/aeskulap/glade";

	bindtextdomain("aeskulap", localedir.c_str());
	bind_textdomain_codeset("aeskulap", "UTF-8");
	textdomain("aeskulap");
 
 	// set locale "C" for numeric conversion (strtod)
 
 	if(setlocale(LC_NUMERIC, "C") == NULL) {
		std::cout << "locale C failed" << std::endl;
	}
 
	Glib::RefPtr<Gnome::Glade::Xml> refXml;

	try  {
		refXml = Gnome::Glade::Xml::create(gladedir+"/aeskulap.glade");
	}
	catch(Gnome::Glade::XmlError) {	
		refXml = Gnome::Glade::Xml::create("aeskulap.glade");
	}

	Aeskulap::IconFactory aeskulap_icons;

	MainWindow* mainWindow = NULL;
	refXml->get_widget_derived("window_main", mainWindow);
	
	if(mainWindow == NULL) {
		exit(-1);
	}

	Aeskulap::set_mainwindow(mainWindow);

	mainWindow->maximize();
	mainWindow->show();

	std::list< Glib::ustring > list;
	for(int c=1; c<argc; c++) {
		if(argv[c][0] != '-') {
			list.push_back(argv[c]);
		}
	}
	
	if(list.size() > 0) {
		mainWindow->load_files(list);
		list.clear();
	}

	kit.run(*mainWindow);

	delete mainWindow;

	ImagePool::close();
	
	return 0;
}
