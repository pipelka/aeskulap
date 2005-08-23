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
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/src/studymanager.h,v $
    CVS/RCS Revision: $Revision: 1.1 $
    Status:           $State: Exp $
*/

#ifndef AESKULAP_STUDYMANAGER_H
#define AESKULAP_STUDYMANAGER_H

#include <gtkmm.h>
#include <libglademm/xml.h>
#include "asimpledisplay.h"

class StudyManager : public Gtk::VBox {
public:

	StudyManager(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
	
	~StudyManager();

	sigc::signal<void, const std::string&> signal_open_study;

protected:

	void on_filter_search();

	void on_filter_today();

	void on_filter_yesterday();

	void on_filter_clearfilter();

	void on_queryresult_study(const Glib::RefPtr< ImagePool::Study >& study);

	void on_queryresult_series(const Glib::RefPtr< ImagePool::Series >& series, Gtk::TreeModel::Row& row);

	void on_study_activated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);

	void on_study_expanded(const Gtk::TreeModel::iterator& iter, const Gtk::TreeModel::Path& path);

	bool on_test_study_expand(const Gtk::TreeModel::iterator& iter, const Gtk::TreeModel::Path& path);

	Gtk::Button* m_button_filter_search;
	
	Gtk::Button* m_button_filter_today;

	Gtk::Button* m_button_filter_yesterday;

	Gtk::Button* m_button_filter_clearfilter;

	Gtk::CheckButton* m_checkbutton_filter_from;

	Gtk::CheckButton* m_checkbutton_filter_to;

	Gtk::Calendar* m_calendar_filter_from;

	Gtk::Calendar* m_calendar_filter_to;

	Gtk::Entry* m_entry_filter_patientid;

	Gtk::Entry* m_entry_filter_lastname;

	Gtk::Entry* m_entry_filter_firstname;

	Gtk::ComboBoxEntry* m_entry_filter_modality;

	Gtk::Entry* m_entry_filter_studydescription;

	class ModelColumns : public Gtk::TreeModel::ColumnRecord {
	public:
	
		ModelColumns() {
			add(m_icon);
			add(m_iconsize);
			add(m_studyinstanceuid);
			add(m_studydate);
			add(m_studydescription);
			add(m_patientsbirthdate);
			add(m_patientsname);
			add(m_modality);
			add(m_station);
		}
		
		Gtk::TreeModelColumn<std::string> m_studyinstanceuid;
		Gtk::TreeModelColumn<Glib::ustring> m_studydate;
		Gtk::TreeModelColumn<Glib::ustring> m_studydescription;
		Gtk::TreeModelColumn<Glib::ustring> m_patientsbirthdate;
		Gtk::TreeModelColumn<Glib::ustring> m_patientsname;
		Gtk::TreeModelColumn<Glib::ustring> m_modality;
		Gtk::TreeModelColumn<Glib::ustring> m_icon;
		Gtk::TreeModelColumn<guint> m_iconsize;
		Gtk::TreeModelColumn<Glib::ustring> m_station;
	};
	
	ModelColumns m_Columns;

	Glib::RefPtr< Gtk::TreeStore > m_refTreeModel;

	Gtk::TreeView* m_treeview_studylist;

	Gtk::CellRendererPixbuf m_tree_icon;

private:

	void remove_rows(const Gtk::TreeModel::Children& list);

	Glib::RefPtr<Gnome::Glade::Xml> m_refGlade;

};

#endif // AESKULAP_STUDYMANAGER_H
