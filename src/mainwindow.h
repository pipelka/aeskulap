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
    Update Date:      $Date: 2005/08/23 19:32:06 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/src/mainwindow.h,v $
    CVS/RCS Revision: $Revision: 1.1 $
    Status:           $State: Exp $
*/

#ifndef AESKULAP_MAINWINDOW_H_
#define AESKULAP_MAINWINDOW_H_

#include <gtkmm.h>
#include <libglademm/xml.h>
#include <map>

namespace ImagePool {
	class Study;
}

class StudyManager;
class Settings;
class StudyView;

class MainWindow : public Gtk::Window {
public:

	MainWindow(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
	
	~MainWindow();

protected:

	void on_file_open();

	void on_net_open(const std::string& studyinstanceuid);

	void on_net_progress(const std::string& studyinstanceuid, unsigned int progress);

	void on_file_exit();
	
	void on_edit_settings();

	void on_view_fullscreen();

	void on_study_added(const Glib::RefPtr<ImagePool::Study>& study);

	void on_study_closed(const std::string& studyinstanceuid);

	void set_busy_cursor(bool busy = true);

private:

	Glib::RefPtr<Gnome::Glade::Xml> m_refGlade;
	
	Gtk::Notebook* m_mainNotebook;

	Gtk::CheckMenuItem* m_itemViewFullscreen;
	
	Gtk::FileChooserDialog m_dialogFile;

	Gtk::CheckButton* m_dialog_check;

	StudyManager* m_studymanager;

	Settings* m_settings;

	bool m_raise_opened;

	Gdk::Cursor* m_cursor_watch;

	std::map<std::string, StudyView*> m_studyview;
};

#endif // AESKULAP_MAINWINDOW_H_
