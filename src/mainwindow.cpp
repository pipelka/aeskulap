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
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/src/mainwindow.cpp,v $
    CVS/RCS Revision: $Revision: 1.5 $
    Status:           $State: Exp $
*/

#include "mainwindow.h"
#include "aiconfactory.h"
#include "imagepool.h"
#include "poolstudy.h"
#include "studyview.h"
#include "studymanager.h"
#include "settings.h"

#include "assert.h"
#include "gettext.h"
#include <iostream>

MainWindow::MainWindow(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade) : 
Gtk::Window(cobject), 
m_refGlade(refGlade), 
m_dialogFile(gettext("Open DICOM Image files")),
m_raise_opened(true)
{
	set_icon(Aeskulap::IconFactory::load_from_file("aeskulap.png"));

	m_studymanager = NULL;
	m_refGlade->get_widget_derived("vbox_studymanager", m_studymanager);
	assert(m_studymanager != NULL);
	m_studymanager->signal_open_study.connect(sigc::mem_fun(*this, &MainWindow::on_net_open));

	m_settings = NULL;
	m_refGlade->get_widget_derived("settingswindow", m_settings);
	assert(m_settings != NULL);

	m_mainNotebook = NULL;
	m_refGlade->get_widget("notebook_main", m_mainNotebook);
	m_mainNotebook->popup_enable();

	Gtk::ImageMenuItem* item = NULL;

	m_refGlade->get_widget("file_open", item);
	item->signal_activate().connect(sigc::mem_fun(*this, &MainWindow::on_file_open));

	m_refGlade->get_widget("file_exit", item);
	item->signal_activate().connect(sigc::mem_fun(*this, &MainWindow::on_file_exit));

	m_itemViewFullscreen = NULL;
	m_refGlade->get_widget("view_fullscreen", m_itemViewFullscreen);
	m_itemViewFullscreen->signal_toggled().connect(sigc::mem_fun(*this, &MainWindow::on_view_fullscreen));

	m_refGlade->get_widget("edit_settings", item);
	item->signal_activate().connect(sigc::mem_fun(*this, &MainWindow::on_edit_settings));

	m_dialogFile.set_select_multiple();
	
	m_dialog_check = manage(new Gtk::CheckButton(gettext("Bring opened files to front")));
	m_dialog_check->set_active(true);
	m_dialog_check->show();
	m_dialogFile.add_action_widget (*m_dialog_check, 1001);

	m_dialogFile.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	m_dialogFile.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);
	
	Gtk::FileFilter filter_dicom;
	filter_dicom.set_name(gettext("DICOM files"));
	filter_dicom.add_pattern("*.dcm");
	m_dialogFile.add_filter(filter_dicom);

	Gtk::FileFilter filter_any;
	filter_any.set_name(gettext("Any files"));
	filter_any.add_pattern("*");
	m_dialogFile.add_filter(filter_any);

	//ImagePool::Signals::signal_study_added.connect(sigc::mem_fun(*this, &MainWindow::on_study_added));
	m_netloader.signal_study_added.connect(sigc::mem_fun(*this, &MainWindow::on_study_added));
	m_netloader.signal_finished.connect(sigc::mem_fun(*this, &MainWindow::on_load_finished));
	m_fileloader.signal_study_added.connect(sigc::mem_fun(*this, &MainWindow::on_study_added));
	m_fileloader.signal_finished.connect(sigc::mem_fun(*this, &MainWindow::on_load_finished));

	m_cursor_watch = new Gdk::Cursor(Gdk::WATCH);
}

MainWindow::~MainWindow() {
	delete m_cursor_watch;
}

void MainWindow::set_busy_cursor(bool busy) {
	if(busy) {
		get_window()->set_cursor(*m_cursor_watch);
	}
	else {
		get_window()->set_cursor();
	}
}

void MainWindow::on_file_open() {
	bool bExit = false;
	
	m_dialogFile.show();

	int rc = 0;
	
	while(!bExit) {
		rc = m_dialogFile.run();
		bExit = (rc == Gtk::RESPONSE_CANCEL) || (rc == Gtk::RESPONSE_OK);
	}

	m_raise_opened = m_dialog_check->get_active();

	m_dialogFile.hide();

	if(rc == Gtk::RESPONSE_CANCEL) {
		return;
	}

	set_busy_cursor();
	m_fileloader.load(m_dialogFile.get_filenames());
}

void MainWindow::on_net_open(const std::string& studyinstanceuid) {
	m_raise_opened = true;
	set_busy_cursor();
	m_netloader.load(studyinstanceuid);
	//ImagePool::load_from_net(studyinstanceuid);
}

void MainWindow::on_net_progress(const std::string& studyinstanceuid, unsigned int progress) {
	std::cout << "MainWindow::on_net_progress('" << studyinstanceuid << ", " << progress << "')" << std::endl;
	StudyView* v = m_studyview[studyinstanceuid];
	if(v != NULL) {
		v->set_progress(progress);
		return;
	}
}

void MainWindow::on_file_exit() {
	Gtk::Main::quit();
}
	
void MainWindow::on_view_fullscreen() {
	if(m_itemViewFullscreen->property_active()) {
		fullscreen();
	}
	else {
		unfullscreen();
	}
}

void MainWindow::on_study_added(const Glib::RefPtr<ImagePool::Study>& study) {
	std::cout << "new study " << study->studyinstanceuid() << std::endl;

	std::string labeltext = study->patientsname().substr(0,20) + "\n" + study->studydescription().substr(0,20);

	StudyView* frame = manage(new StudyView(study));
	m_studyview[study->studyinstanceuid()] = frame;

	// create notebook tab widget
	Gtk::Label* label = manage(new Gtk::Label(labeltext, Gtk::ALIGN_LEFT));
	label->set_padding(2,0);
	label->show();

	Gtk::Image* image = manage(new Gtk::Image(Gtk::Stock::CLOSE, Gtk::ICON_SIZE_MENU));
	image->show();

	Gtk::Image* image2 = manage(new Gtk::Image(Gtk::Stock::DND_MULTIPLE, Gtk::ICON_SIZE_LARGE_TOOLBAR));
	image2->set_padding(2,0);
	image2->show();
	
	Gtk::ToolButton* btn = manage(new Gtk::ToolButton(*image));
	btn->set_tooltip(m_tooltips, gettext("Close this study"));
	btn->set_size_request(22, 22);
	btn->signal_clicked().connect(sigc::mem_fun(*frame, &StudyView::on_close));
	btn->set_sensitive(false);
	btn->show();

	Gtk::HBox* hbox = manage(new Gtk::HBox);
	hbox->pack_start(*image2);
	hbox->pack_start(*label);
	hbox->pack_start(*btn);
	hbox->show();

	frame->set_close_button(btn);

	m_mainNotebook->append_page(*frame, *hbox);
	frame->show();
	if(m_raise_opened) {
		m_raise_opened = false;
		m_mainNotebook->set_current_page(-1);
	}
	study->signal_series_added.connect(sigc::mem_fun(*frame, &StudyView::on_series_added));
	frame->signal_remove().connect(sigc::mem_fun(*this, &MainWindow::on_study_closed));

	set_busy_cursor(false);
}

void MainWindow::on_study_closed(Gtk::Widget* page) {
	std::cout << "MainWindow::on_study_closed" << std::endl;
	m_studyview.erase(find_pageuid(page));
}

void MainWindow::on_edit_settings() {
	m_settings->show();
}

const std::string& MainWindow::find_pageuid(Gtk::Widget* page) {
	static std::string empty = "";

	std::map<std::string, StudyView*>::iterator i = m_studyview.begin();
	
	while(i != m_studyview.end()) {
		if(i->second == page) {
			return i->first;
		}
		i++;
	}
	
	return empty;
}

void MainWindow::on_load_finished(const std::string& studyinstanceuid) {
	StudyView* v = m_studyview[studyinstanceuid];
	if(v != NULL) {
		set_busy_cursor(false);
		v->set_progress(100);
		return;
	}
}
