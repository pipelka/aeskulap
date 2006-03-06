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
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/src/studymanager.cpp,v $
    CVS/RCS Revision: $Revision: 1.16 $
    Status:           $State: Exp $
*/

#include "imagepool.h"
#include "studymanager.h"
#include "asimpledisplay.h"
#include "adatefilter.h"
#include "abusycursor.h"
#include "gettext.h"

#include <iostream>
#include <list>

StudyManager::StudyManager(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade) : 
Gtk::VBox(cobject),
m_refGlade(refGlade),
m_new_query(false)
{
	m_hbox_datefilter = NULL;
	m_refGlade->get_widget("hbox_datefilter", m_hbox_datefilter);

	m_datefilter = manage(new Aeskulap::DateFilter);
	m_datefilter->show();
	m_hbox_datefilter->pack_start(*m_datefilter, Gtk::PACK_SHRINK);

	m_button_filter_search = NULL;
	m_refGlade->get_widget("button_filter_search", m_button_filter_search);
	m_button_filter_search->signal_clicked().connect(sigc::mem_fun(*this, &StudyManager::on_filter_search));

	m_button_filter_clearfilter = NULL;
	m_refGlade->get_widget("button_filter_clearfilter", m_button_filter_clearfilter);
	m_button_filter_clearfilter->signal_clicked().connect(sigc::mem_fun(*this, &StudyManager::on_filter_clearfilter));

	m_entry_filter_patientid = NULL;
	m_refGlade->get_widget("entry_filter_patientid", m_entry_filter_patientid);
	m_entry_filter_patientid->signal_activate().connect(sigc::mem_fun(*this, &StudyManager::on_filter_search));

	m_entry_filter_name = NULL;
	m_refGlade->get_widget("entry_filter_name", m_entry_filter_name);
	m_entry_filter_name->signal_activate().connect(sigc::mem_fun(*this, &StudyManager::on_filter_search));

	m_entry_filter_stationname = NULL;
	m_refGlade->get_widget("entry_filter_stationname", m_entry_filter_stationname);
	m_entry_filter_stationname->signal_activate().connect(sigc::mem_fun(*this, &StudyManager::on_filter_search));

	m_entry_filter_modality = NULL;
	m_refGlade->get_widget("entry_filter_modality", m_entry_filter_modality);
	m_entry_filter_modality->get_entry()->signal_activate().connect(sigc::mem_fun(*this, &StudyManager::on_filter_search));

	m_entry_filter_studydescription = NULL;
	m_refGlade->get_widget("entry_filter_studydescription", m_entry_filter_studydescription);
	m_entry_filter_studydescription->signal_activate().connect(sigc::mem_fun(*this, &StudyManager::on_filter_search));

	// studylist

	m_treeview_studylist = NULL;
	m_refGlade->get_widget("treeview_studylist", m_treeview_studylist);

	m_refTreeModelStudy = Gtk::TreeStore::create(m_ColumnsStudy);
	m_refTreeModelStudy->set_sort_column(m_ColumnsStudy.m_patientsname, Gtk::SORT_ASCENDING);

	m_treeview_studylist->set_model(m_refTreeModelStudy);
	m_treeview_studylist->signal_row_activated().connect(sigc::mem_fun(*this, &StudyManager::on_study_activated));
	m_treeview_studylist->signal_row_expanded().connect(sigc::mem_fun(*this, &StudyManager::on_study_expanded));
	m_treeview_studylist->signal_test_expand_row().connect(sigc::mem_fun(*this, &StudyManager::on_test_study_expand));
	m_treeview_studylist->set_headers_clickable();

	m_treeview_studylist->append_column("", m_tree_icon);
	Gtk::TreeViewColumn* c = m_treeview_studylist->get_column(0);
	c->add_attribute(m_tree_icon.property_stock_id(), m_ColumnsStudy.m_icon);
	
	m_treeview_studylist->append_column(gettext("Patientsname"), m_ColumnsStudy.m_patientsname);
	m_treeview_studylist->append_column(gettext("Birthdate"), m_ColumnsStudy.m_patientsbirthdate);
	m_treeview_studylist->append_column(gettext("Description"), m_ColumnsStudy.m_studydescription);
	m_treeview_studylist->append_column(gettext("Modality"), m_ColumnsStudy.m_modality);
	m_treeview_studylist->append_column(gettext("Date/Time"), m_ColumnsStudy.m_studydate);
	m_treeview_studylist->append_column(gettext("Station"), m_ColumnsStudy.m_station);
	m_treeview_studylist->append_column(gettext("Server"), m_ColumnsStudy.m_server);
	
	m_treeview_studylist->get_column(1)->set_sort_column(m_ColumnsStudy.m_patientsname);
	m_treeview_studylist->get_column(1)->property_sort_indicator().set_value(true);
	m_treeview_studylist->get_column(1)->set_sort_order(Gtk::SORT_ASCENDING);

	m_treeview_studylist->get_column(2)->set_sort_column(m_ColumnsStudy.m_patientsbirthdate);
	m_treeview_studylist->get_column(3)->set_sort_column(m_ColumnsStudy.m_studydescription);
	m_treeview_studylist->get_column(4)->set_sort_column(m_ColumnsStudy.m_modality);
	m_treeview_studylist->get_column(5)->set_sort_column(m_ColumnsStudy.m_studydate);
	m_treeview_studylist->get_column(6)->set_sort_column(m_ColumnsStudy.m_station);
	m_treeview_studylist->get_column(6)->set_sort_column(m_ColumnsStudy.m_server);
	
	// grouplist

	m_treeview_grouplist = NULL;
	m_refGlade->get_widget("treeview_servergroup", m_treeview_grouplist);

	m_refTreeModelGroup = Gtk::ListStore::create(m_ColumnsGroup);
	m_refTreeModelGroup->set_sort_column(m_ColumnsGroup.m_group, Gtk::SORT_ASCENDING);

	m_treeview_grouplist->set_model(m_refTreeModelGroup);
	m_treeview_grouplist->append_column(gettext("Group"), m_ColumnsGroup.m_group);
	
	m_treeview_grouplist->get_column(0)->set_sort_column(m_ColumnsGroup.m_group);
	m_treeview_grouplist->get_column(0)->property_sort_indicator().set_value(true);
	m_treeview_grouplist->get_column(0)->set_sort_order(Gtk::SORT_ASCENDING);

	m_treeview_grouplist->get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);

	m_refGlade->get_widget("frame_servergroups", m_frame_servergroups);

	update_grouplist();
}

StudyManager::~StudyManager() {
}

void StudyManager::on_filter_search() {
	std::cout << "StudyManager::on_filter_search()" << std::endl;
	
	Aeskulap::set_busy_cursor();
	while(Gtk::Main::events_pending()) Gtk::Main::iteration(false);

	update_selected_groups();

	m_new_query = true;

	std::string local_aet = m_configuration.get_local_aet();

	ImagePool::query_from_net(
					m_entry_filter_patientid->get_text(),
					m_entry_filter_name->get_text(),
					m_entry_filter_modality->get_entry()->get_text(),
					m_datefilter->get_startdate(),
					m_datefilter->get_enddate(),
					m_entry_filter_studydescription->get_text(),
					m_entry_filter_stationname->get_text(),
					local_aet,
					m_selected_groups,
					sigc::mem_fun(*this, &StudyManager::on_queryresult_study)
					);
	
	if(m_new_query) {
		Aeskulap::set_busy_cursor(false);
		std::cout << "no results !!!" << std::endl;
		Gtk::MessageDialog error(
					gettext("<span weight=\"bold\" size=\"larger\">No results for this query</span>"),
					true,
					Gtk::MESSAGE_INFO,
					Gtk::BUTTONS_OK,
					true);
					
		error.show();
		error.run();
		error.hide();
	}
}

void StudyManager::remove_rows(const Gtk::TreeModel::Children& list) {
	Gtk::TreeModel::Children::iterator i = list.begin();
	for(; i != list.end();) {
		i = m_refTreeModelStudy->erase(i);
	}
}

void StudyManager::on_filter_clearfilter() {
	m_datefilter->clear();
	m_entry_filter_patientid->set_text("");
	m_entry_filter_name->set_text("");
	m_entry_filter_modality->get_entry()->set_text("");
	m_entry_filter_studydescription->set_text("");
	m_entry_filter_stationname->set_text("");
}

void StudyManager::on_queryresult_study(const Glib::RefPtr< ImagePool::Study >& study) {
	if(m_new_query) {
		remove_rows(m_refTreeModelStudy->children());
		m_new_query = false;
	}

	Gtk::TreeModel::Row row = *(m_refTreeModelStudy->append());

	row[m_ColumnsStudy.m_icon] = Gtk::Stock::OPEN.id;
	row[m_ColumnsStudy.m_iconsize] = 22;
	row[m_ColumnsStudy.m_patientsname] = study->patientsname();
	row[m_ColumnsStudy.m_patientsbirthdate] = study->patientsbirthdate();
	row[m_ColumnsStudy.m_studydescription] = study->studydescription();
	row[m_ColumnsStudy.m_studydate] = study->studydate();
	row[m_ColumnsStudy.m_studyinstanceuid] = study->studyinstanceuid();
	row[m_ColumnsStudy.m_server] = study->server();
	
	// add child
	Gtk::TreeModel::Row child = *(m_refTreeModelStudy->append(row.children()));	

	Aeskulap::set_busy_cursor(false);
}

void StudyManager::on_study_activated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column) {
	std::cout << "StudyManager::on_study_activated()" << std::endl;
	Gtk::TreeModel::iterator iter = m_refTreeModelStudy->get_iter(path);
	if(!iter) {
		return;
	}
	Gtk::TreeModel::Row row = *iter;
	std::string studyinstanceuid = row[m_ColumnsStudy.m_studyinstanceuid];
	std::string server = row[m_ColumnsStudy.m_server];
	
	if(studyinstanceuid.empty()) {
		return;
	}
	signal_open_study(studyinstanceuid, server);
}

void StudyManager::on_queryresult_series(const Glib::RefPtr< ImagePool::Series >& series, Gtk::TreeModel::Row& row) {
	std::cout << "StudyManager::on_queryresult_series()" << std::endl;

	Aeskulap::set_busy_cursor(false);

	Gtk::TreeModel::Row child = *(m_refTreeModelStudy->append(row.children()));

	int count = row.children().size();
	char buffer[50];

	if(series->instancecount() == 1) {
		g_snprintf(buffer, sizeof(buffer), gettext("Series %02i (1 Image)"), count);
	}
	else if(series->instancecount() > 0) {
		g_snprintf(buffer, sizeof(buffer), gettext("Series %02i (%i Images)"), count, series->instancecount());
	}
	else {
		g_snprintf(buffer, sizeof(buffer), gettext("Series %02i"), count);
	}
	
	child[m_ColumnsStudy.m_icon] = Gtk::Stock::DND_MULTIPLE.id;
	child[m_ColumnsStudy.m_iconsize] = 16;
	child[m_ColumnsStudy.m_patientsname] = buffer;
	child[m_ColumnsStudy.m_studydescription] = series->description();
	child[m_ColumnsStudy.m_modality] = series->modality();
	child[m_ColumnsStudy.m_studydate] = series->seriestime();
	child[m_ColumnsStudy.m_station] = series->stationname();
}

bool StudyManager::on_test_study_expand(const Gtk::TreeModel::iterator& iter, const Gtk::TreeModel::Path& path) {
	std::cout << "StudyManager::on_test_study_expand()" << std::endl;

	Aeskulap::set_busy_cursor();
	while(Gtk::Main::events_pending()) Gtk::Main::iteration(false);

	Gtk::TreeModel::Row row = *iter;
	std::string studyinstanceuid = row[m_ColumnsStudy.m_studyinstanceuid];
	std::string server = row[m_ColumnsStudy.m_server];

	remove_rows(row.children());

	query_series_from_net(
				studyinstanceuid,
				server,
				m_configuration.get_local_aet(),
				sigc::bind(sigc::mem_fun(*this, &StudyManager::on_queryresult_series), row)
				);

	return false;
}

void StudyManager::on_study_expanded(const Gtk::TreeModel::iterator& iter, const Gtk::TreeModel::Path& path) {
	std::cout << "StudyManager::on_study_expanded()" << std::endl;
}

void StudyManager::update_grouplist() {
	Gtk::TreeModel::Children list = m_refTreeModelGroup->children();
	Gtk::TreeModel::Children::iterator i = list.begin();

	for(; i != list.end();) {
		i = m_refTreeModelGroup->erase(i);
	}
	
	std::set< std::string > groups = ImagePool::ServerList::get_groups();
	std::set< std::string >::iterator g = groups.begin();
	for( ; g != groups.end(); g++) {
		Gtk::TreeModel::Row row = *(m_refTreeModelGroup->append());	
		row[m_ColumnsGroup.m_group] = (*g);
	}

	if(groups.size() == 0) {
		m_frame_servergroups->hide();
	}
	else {
		m_frame_servergroups->show();
	}

}

void StudyManager::update_selected_groups() {
	Glib::RefPtr<Gtk::TreeSelection> selection = m_treeview_grouplist->get_selection();
	m_selected_groups.clear();
	selection->selected_foreach_iter(sigc::mem_fun(*this, &StudyManager::selected_group_callback));
}

void StudyManager::selected_group_callback(const Gtk::TreeModel::iterator& iter) {
	Gtk::TreeModel::Row row = *iter;

	std::string group = row[m_ColumnsGroup.m_group];
	std::cout << "Group: " << group << std::endl;

	m_selected_groups.insert(row[m_ColumnsGroup.m_group]);
}
