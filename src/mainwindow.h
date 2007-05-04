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
    Update Date:      $Date: 2007/05/04 14:47:07 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/src/mainwindow.h,v $
    CVS/RCS Revision: $Revision: 1.17 $
    Status:           $State: Exp $
*/

#ifndef AESKULAP_MAINWINDOW_H_
#define AESKULAP_MAINWINDOW_H_

#include <gtkmm.h>
#include <libglademm/xml.h>

#include "fileloader.h"
#include "netloader.h"
#include "dicomdirloader.h"
#include "awindowlevel.h"
#include "aconfigclient.h"

namespace ImagePool {
	class Study;
}

class StudyManager;
class Settings;
class StudyView;
class PrescanDialog;
class AboutDialog;
class WindowLevelDialog;

class MainWindow : public Gtk::Window, public Aeskulap::ConfigClient {
public:

	MainWindow(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);

	~MainWindow();

	void load_files(std::list< Glib::ustring > list);

    void open_dicomdir(const Glib::ustring &dicomdir);

protected:

	void on_file_open();

    void on_dicomdir_open();

	void on_study_open(const std::string& studyinstanceuid, const std::string& server);

	void on_study_progress(const std::string& studyinstanceuid, unsigned int progress);

	void on_study_error();

	void on_file_exit();

	void on_edit_settings();

	void on_edit_settings_apply();

	void on_view_fullscreen();

	void on_study_added(const Glib::RefPtr<ImagePool::Study>& study);

	void on_study_closed(StudyView* page);

	void on_about();

	bool on_windowlevel_add(const Aeskulap::WindowLevel& level);

	void on_switch_page(GtkNotebookPage* page, guint index);

private:

	const std::string& find_pageuid(Gtk::Widget* page);

	Glib::RefPtr<Gnome::Glade::Xml> m_refGlade;

	Gtk::Notebook* m_mainNotebook;

	Gtk::CheckMenuItem* m_itemViewFullscreen;

    Gtk::FileFilter m_filter_dicom;
    Gtk::FileFilter m_filter_any;
    Gtk::FileFilter m_filter_dicomdir;
	Gtk::FileChooserDialog m_dialogFile;

	Gtk::CheckButton* m_dialog_check;

	StudyManager* m_studymanager;

	PrescanDialog* m_prescandialog;

	Settings* m_settings;

	bool m_raise_opened;

	std::map<std::string, StudyView*> m_studyview;

	Gtk::Tooltips m_tooltips;

    ImagePool::DicomdirLoader m_dicomdirloader;

	ImagePool::NetLoader m_netloader;

	ImagePool::FileLoader m_fileloader;

	AboutDialog* m_about;

	WindowLevelDialog* m_windowlevel;

};

#endif // AESKULAP_MAINWINDOW_H_
