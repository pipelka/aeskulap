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
    Update Date:      $Date: 2007/05/10 14:29:59 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/src/settings.cpp,v $
    CVS/RCS Revision: $Revision: 1.17 $
    Status:           $State: Exp $
*/

#include "settings.h"
#include "abusycursor.h"
#include "imagepool.h"
#include "gettext.h"

#include <vector>
#include <iostream>

Settings::Settings(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade) :
Gtk::Window(cobject),
m_refGlade(refGlade) {

	// dicom settings

	m_refGlade->get_widget("local_aet", m_local_aet);
	
	m_refGlade->get_widget("local_port", m_local_port);

	Gtk::HBox* hbox_characterset;
	m_refGlade->get_widget("hbox_characterset",hbox_characterset);
	
	m_characterset = manage(new Gtk::ComboBoxText);
	m_characterset->append_text("ISO_IR 6");
	m_characterset->append_text("ISO_IR 100");
	m_characterset->append_text("ISO_IR 101");
	m_characterset->append_text("ISO_IR 109");
	m_characterset->append_text("ISO_IR 110");
	m_characterset->append_text("ISO_IR 144");
	m_characterset->append_text("ISO_IR 127");
	m_characterset->append_text("ISO_IR 126");
	m_characterset->append_text("ISO_IR 138");
	m_characterset->append_text("ISO_IR 148");
	m_characterset->append_text("ISO_IR 192");

	m_characterset->show();
	hbox_characterset->pack_start(*m_characterset);
	
	// server details

	m_refGlade->get_widget("server_detail_server", m_server_detail_server);
	m_server_detail_server->signal_changed().connect(sigc::mem_fun(*this, &Settings::on_server_apply));
	
	m_refGlade->get_widget("server_detail_aet", m_server_detail_aet);
	m_server_detail_aet->signal_changed().connect(sigc::mem_fun(*this, &Settings::on_server_apply));
	
	m_refGlade->get_widget("server_detail_port", m_server_detail_port);
	m_server_detail_port->signal_changed().connect(sigc::mem_fun(*this, &Settings::on_server_apply));
	
	m_refGlade->get_widget("server_detail_group", m_server_detail_group);
	m_server_detail_group->get_entry()->signal_changed().connect(sigc::mem_fun(*this, &Settings::on_server_apply));

	m_refGlade->get_widget("server_detail_description", m_server_detail_description);
	m_server_detail_description->signal_changed().connect(sigc::mem_fun(*this, &Settings::on_server_apply));

	m_refGlade->get_widget("server_detail_lossy", m_server_detail_lossy);
	m_server_detail_lossy->signal_toggled().connect(sigc::mem_fun(*this, &Settings::on_server_apply));

	m_refGlade->get_widget("server_detail_relational", m_server_detail_relational);
	m_server_detail_relational->signal_toggled().connect(sigc::mem_fun(*this, &Settings::on_server_apply));

	m_refGlade->get_widget("server_detail_echo", m_server_detail_echo);
	m_server_detail_echo->signal_clicked().connect(sigc::mem_fun(*this, &Settings::on_echotest));

	m_refGlade->get_widget("server_detail_echostatus", m_server_detail_echostatus);

	// disable server detail
	
	set_server_detail_sensitive(false);

	// connect buttons
	m_refGlade->get_widget("settings_ok", m_settings_ok);
	m_settings_ok->signal_clicked().connect(sigc::mem_fun(*this, &Settings::on_settings_save));

	Gtk::Button* button;
	m_refGlade->get_widget("settings_cancel", button);
	button->signal_clicked().connect(sigc::mem_fun(*this, &Settings::on_settings_cancel));	

	m_refGlade->get_widget("servers_add", button);
	button->signal_clicked().connect(sigc::mem_fun(*this, &Settings::on_server_add));

	m_refGlade->get_widget("servers_remove", button);
	button->signal_clicked().connect(sigc::mem_fun(*this, &Settings::on_server_remove));

	// create server list
	m_refGlade->get_widget("serverlist", m_list_servers);

	m_refTreeModel = Gtk::ListStore::create(m_Columns);
	m_refTreeModel->set_sort_column(m_Columns.m_name, Gtk::SORT_ASCENDING);

	m_list_servers->set_model(m_refTreeModel);

	m_list_servers->append_column(gettext("Name"), m_Columns.m_name);
	m_list_servers->append_column(gettext("AET"), m_Columns.m_aet);
	m_list_servers->append_column_numeric(gettext("Port"), m_Columns.m_port, "%i");
	m_list_servers->append_column(gettext("Hostname"), m_Columns.m_hostname);
	m_list_servers->append_column(gettext("Group"), m_Columns.m_group);

	m_list_servers->get_column(0)->set_sort_column(m_Columns.m_name);
	m_list_servers->get_column(0)->property_sort_indicator().set_value(true);
	m_list_servers->get_column(0)->set_sort_order(Gtk::SORT_ASCENDING);
	m_list_servers->get_column(1)->set_sort_column(m_Columns.m_aet);
	m_list_servers->get_column(2)->set_sort_column(m_Columns.m_port);
	m_list_servers->get_column(3)->set_sort_column(m_Columns.m_hostname);
	m_list_servers->get_column(3)->set_sort_column(m_Columns.m_group);

	Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_list_servers->get_selection();
	refTreeSelection->signal_changed().connect(sigc::mem_fun(*this, &Settings::on_server_activated));

	// presets - windowlevel
	
	m_refGlade->get_widget("presets_windowlevels_modality", m_presets_windowlevels_modality);
	m_presets_windowlevels_modality->signal_changed().connect(sigc::mem_fun(*this, &Settings::on_windowlevels_modality_changed));

	m_refGlade->get_widget("presets_windowlevels", m_presets_windowlevels);

	m_refWindowLevelModel = Gtk::ListStore::create(m_WindowLevelColumns);
	m_refTreeModel->set_sort_column(m_WindowLevelColumns.m_description, Gtk::SORT_ASCENDING);

	m_presets_windowlevels->set_model(m_refWindowLevelModel);

	m_presets_windowlevels->append_column_editable(gettext("Description"), m_WindowLevelColumns.m_description);
	m_presets_windowlevels->append_column_numeric_editable(gettext("Center"), m_WindowLevelColumns.m_center, "%i");
	m_presets_windowlevels->append_column_numeric_editable(gettext("Width"), m_WindowLevelColumns.m_width, "%i");

	m_presets_windowlevels->get_column(0)->set_sort_column(m_WindowLevelColumns.m_description);
	m_presets_windowlevels->get_column(0)->property_sort_indicator().set_value(true);
	m_presets_windowlevels->get_column(0)->set_sort_order(Gtk::SORT_ASCENDING);
	m_presets_windowlevels->get_column(1)->set_sort_column(m_WindowLevelColumns.m_center);
	m_presets_windowlevels->get_column(2)->set_sort_column(m_WindowLevelColumns.m_width);

	m_refGlade->get_widget("presets_windowlevels_add", button);
	button->signal_clicked().connect(sigc::mem_fun(*this, &Settings::on_windowlevels_add));

	m_refGlade->get_widget("presets_windowlevels_remove", button);
	button->signal_clicked().connect(sigc::mem_fun(*this, &Settings::on_windowlevels_remove));

	if(m_presets_windowlevels_modality->get_active_row_number() == -1) {
		m_presets_windowlevels_modality->set_active(0);
	}

	restore_settings();
}

Settings::~Settings() {
}

void Settings::on_show() {
	Gtk::Window::on_show();
	restore_settings();
}


void Settings::on_settings_save() {
	save_settings();
	ImagePool::close();
	ImagePool::init();
	hide();
	signal_apply();
}

void Settings::on_settings_cancel() {
	restore_settings();
	hide();
}

void Settings::save_settings() {
	Glib::ustring value;

	m_configuration.set_local_aet(m_local_aet->get_text());
	m_configuration.set_local_port(atoi(m_local_port->get_text().c_str()));

	std::string encoding = m_characterset->get_active_text().c_str();
	ImagePool::set_encoding(encoding);
	m_configuration.set_encoding(encoding);

	// store serverlist
	// ----------------
	
	// create list from treeview

	std::vector<Aeskulap::Configuration::ServerData> list;
	Gtk::TreeModel::Children::iterator i = m_refTreeModel->children().begin();
	for(; i != m_refTreeModel->children().end(); i++) {
		Aeskulap::Configuration::ServerData s;
		s.m_aet = (*i)[m_Columns.m_aet];
		s.m_hostname = (*i)[m_Columns.m_hostname];
		s.m_port = (*i)[m_Columns.m_port];
		s.m_name = (*i)[m_Columns.m_name];
		s.m_group = (*i)[m_Columns.m_group];
		s.m_lossy = (*i)[m_Columns.m_lossy];
		s.m_relational = (*i)[m_Columns.m_relational];
		
		list.push_back(s);
	}

	// write list to configuration
	m_configuration.set_serverlist(list);
	ImagePool::ServerList::update();

	// store windowlevelpresets
	// ------------------------
	
	store_windowlevel_preset();
}

void Settings::restore_settings() {
	char buffer[10];

	m_local_aet->set_text(m_configuration.get_local_aet());
	g_snprintf(buffer, sizeof(buffer), "%i", m_configuration.get_local_port());
	m_local_port->set_text(buffer);

	Glib::ustring charset = ImagePool::get_encoding();
	m_characterset->set_active_text(charset);


	Gtk::TreeModel::Children::iterator i = m_refTreeModel->children().begin();
	for(; i != m_refTreeModel->children().end();) {
		i = m_refTreeModel->erase(i);
	}

	Glib::RefPtr<ImagePool::ServerList> list = ImagePool::ServerList::get();
	for(ImagePool::ServerList::iterator i = list->begin(); i != list->end(); i++) {
		Gtk::TreeModel::Row row = *(m_refTreeModel->append());

		row[m_Columns.m_aet] = i->second.m_aet;
		row[m_Columns.m_port] = i->second.m_port;
		row[m_Columns.m_hostname] = i->second.m_hostname;
		row[m_Columns.m_name] = i->second.m_name;
		row[m_Columns.m_group] = i->second.m_group;
		row[m_Columns.m_lossy] = i->second.m_lossy;
		row[m_Columns.m_relational] = i->second.m_relational;
	}

	reload_windowlevel_preset(m_windowlevels_modality);
}

void Settings::on_server_add() {
	static int count = m_refTreeModel->children().size();
	set_server_detail_sensitive(false);

	Gtk::TreeModel::Row row = *(m_refTreeModel->append());
	Glib::RefPtr<Gtk::TreeSelection> selection = m_list_servers->get_selection();

	char servername[50];
	g_snprintf(servername, sizeof(servername), "Server%i", ++count);

	row[m_Columns.m_name] = servername;
	row[m_Columns.m_aet] = "AET";
	row[m_Columns.m_port] = 6100;
	row[m_Columns.m_hostname] = "hostname";

	Gtk::TreePath path(row);
	m_list_servers->row_activated (path, *(m_list_servers->get_column(0)));
	selection->select(row);
}

void Settings::on_server_remove() {
	Glib::RefPtr<Gtk::TreeSelection> selection = m_list_servers->get_selection();
	m_refTreeModel->erase(selection->get_selected());
}

void Settings::set_server_detail_sensitive(bool sensitive) {
	m_server_detail_server->set_sensitive(sensitive);
	m_server_detail_aet->set_sensitive(sensitive);
	m_server_detail_port->set_sensitive(sensitive);
	m_server_detail_group->set_sensitive(sensitive);
	m_server_detail_description->set_sensitive(sensitive);
	m_server_detail_lossy->set_sensitive(sensitive);
	m_server_detail_relational->set_sensitive(sensitive);
	m_server_detail_echo->set_sensitive(sensitive);

	if(!sensitive) {
		m_server_detail_server->set_text("");		
		m_server_detail_aet->set_text("");
		m_server_detail_port->set_text("");
		m_server_detail_group->get_entry()->set_text("");
		m_server_detail_description->set_text("");
		m_server_detail_lossy->set_active(false);
		m_server_detail_relational->set_active(false);
	}
}

void Settings::on_server_activated() {
	set_server_detail_sensitive(false);
	Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_list_servers->get_selection();
	Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();

	if(!iter) {
		return;
	}
	Gtk::TreeModel::Row row = *iter;

	m_server_detail_server->set_text(row[m_Columns.m_hostname]);
	
	m_server_detail_aet->set_text(row[m_Columns.m_aet]);
	
	char buffer[10];
	guint i = row[m_Columns.m_port];
	g_snprintf(buffer, sizeof(buffer), "%i", i);
	m_server_detail_port->set_text(buffer);
	m_server_detail_group->get_entry()->set_text(row[m_Columns.m_group]);
	m_server_detail_description->set_text(row[m_Columns.m_name]);
	m_server_detail_lossy->set_active(row[m_Columns.m_lossy]);
	m_server_detail_relational->set_active(row[m_Columns.m_relational]);

	set_server_detail_sensitive(true);
}

void Settings::on_server_apply() {
	Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_list_servers->get_selection();
	Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();

	if(!iter) {
		return;
	}
	Gtk::TreeModel::Row row = *iter;

	if(m_server_detail_server->is_sensitive()) {
		row[m_Columns.m_hostname] = m_server_detail_server->get_text();
	}
	if(m_server_detail_aet->is_sensitive()) {
		row[m_Columns.m_aet] = m_server_detail_aet->get_text();
	}
	if(m_server_detail_port->is_sensitive()) {
		guint i = atoi(m_server_detail_port->get_text().c_str());
		row[m_Columns.m_port] = i;
	}
	if(m_server_detail_group->is_sensitive()) {
		row[m_Columns.m_group] = m_server_detail_group->get_entry()->get_text();
	}
	if(m_server_detail_description->is_sensitive()) {
		Glib::ustring desc = m_server_detail_description->get_text();
		row[m_Columns.m_name] = desc;
		if(desc.empty()) {
			m_settings_ok->set_sensitive(false);
		}
		else {
			m_settings_ok->set_sensitive(true);
		}
	}
	if(m_server_detail_lossy->is_sensitive()) {
		row[m_Columns.m_lossy] = m_server_detail_lossy->get_active();
	}
	if(m_server_detail_relational->is_sensitive()) {
		row[m_Columns.m_relational] = m_server_detail_relational->get_active();
	}

	// enable echo button
	if(!m_server_detail_aet->get_text().empty() &&
		!m_server_detail_server->get_text().empty() &&
		!m_server_detail_port->get_text().empty()) {
			m_server_detail_echo->set_sensitive(true);
	}
	else {
			m_server_detail_echo->set_sensitive(false);
	}
}

void Settings::on_echotest() {
	Aeskulap::set_busy_cursor(true, this);
	
	Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_list_servers->get_selection();
	Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();
	Gtk::TreeModel::Row row = *iter;

	m_server_detail_echo->set_sensitive(false);
	while(Gtk::Main::events_pending()) Gtk::Main::iteration(false);

	Glib::ustring hostname = row[m_Columns.m_hostname];
	Glib::ustring aet = row[m_Columns.m_aet];
	unsigned int port = row[m_Columns.m_port];
	std::string status;

	ImagePool::Server server(hostname, aet, port);
	
	if(!server.send_echo(status)) {
		Gtk::MessageDialog error(
					*this,
					gettext("<span weight=\"bold\" size=\"larger\">Echo test failed!</span>\n\n")
					+ status,
					true,
					Gtk::MESSAGE_ERROR,
					Gtk::BUTTONS_OK,
					true);
					
		Aeskulap::set_busy_cursor(false, this);

		error.show();
		error.run();
		error.hide();
	}
	else {
		Gtk::MessageDialog msg(
					*this,
					gettext("<span weight=\"bold\" size=\"larger\">Echo succeeded</span>"),
					true,
					Gtk::MESSAGE_INFO,
					Gtk::BUTTONS_OK,
					true);
					
		Aeskulap::set_busy_cursor(false, this);

		msg.show();
		msg.run();
		msg.hide();
	}

	m_server_detail_echo->set_sensitive(true);
}

void Settings::store_windowlevel_preset() {
	if(m_windowlevels_modality.empty()) {
		return;
	}

	// create list from treeview
	Aeskulap::WindowLevelList list;
	Gtk::TreeModel::Children::iterator i = m_refWindowLevelModel->children().begin();
	for(; i != m_refWindowLevelModel->children().end(); i++) {
		Aeskulap::WindowLevel l(
					(*i)[m_WindowLevelColumns.m_description],
					m_windowlevels_modality,
					(*i)[m_WindowLevelColumns.m_center],
					(*i)[m_WindowLevelColumns.m_width]);

		list[l.description] = l;
	}

	// write list to configuration
	m_configuration.unset_windowlevels(m_windowlevels_modality);
	m_configuration.set_windowlevel_list(m_windowlevels_modality, list);
}

void Settings::reload_windowlevel_preset(const Glib::ustring& modality) {
	Aeskulap::WindowLevelList list;

	m_configuration.get_windowlevel_list(m_windowlevels_modality, list);
	Aeskulap::WindowLevelList::iterator i;
	
	m_refWindowLevelModel->clear();

	for(i = list.begin(); i != list.end(); i++) {
		Gtk::TreeModel::Row row = *(m_refWindowLevelModel->append());

		row[m_WindowLevelColumns.m_description] = i->second.description;
		row[m_WindowLevelColumns.m_center] = i->second.center;
		row[m_WindowLevelColumns.m_width] = i->second.width;
	}
}

void Settings::on_windowlevels_modality_changed() {
	int index = m_presets_windowlevels_modality->get_active_row_number();
	std::cout << "on_windowlevels_modality_changed() - indexint: " << index << std::endl;
	
	store_windowlevel_preset();

	if(index == 0) {
		m_windowlevels_modality = "CT";
	}
	if(index == 1) {
		m_windowlevels_modality = "CR";
	}

	reload_windowlevel_preset(m_windowlevels_modality);
}

void Settings::on_windowlevels_add() {
	Gtk::TreeModel::Row row = *(m_refWindowLevelModel->append());
	Glib::RefPtr<Gtk::TreeSelection> selection = m_presets_windowlevels->get_selection();

	row[m_WindowLevelColumns.m_description] = gettext("Description");

	Gtk::TreePath path(row);
	m_presets_windowlevels->row_activated(path, *(m_presets_windowlevels->get_column(0)));
	selection->select(row);
}

void Settings::on_windowlevels_remove() {
	Glib::RefPtr<Gtk::TreeSelection> selection = m_presets_windowlevels->get_selection();
	Gtk::TreeModel::Row row = *(selection->get_selected());
	m_refWindowLevelModel->erase(row);
}
