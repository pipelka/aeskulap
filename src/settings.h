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
    Update Date:      $Date: 2005/09/19 17:04:28 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/src/settings.h,v $
    CVS/RCS Revision: $Revision: 1.5 $
    Status:           $State: Exp $
*/

#ifndef AESKULAP_SETTINGS_H
#define AESKULAP_SETTINGS_H

#include <gtkmm.h>
#include <libglademm/xml.h>
#include <gconfmm.h>

class Settings : public Gtk::Window {
public:

	Settings(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
	
	~Settings();

	sigc::signal<void> signal_apply;

protected:

	void on_show();

	void on_settings_save();
	
	void on_settings_cancel();

	void on_server_add();

	void on_server_remove();

	void on_server_activated();

	void on_server_apply();

	void on_echotest();

	void save_settings();

	void restore_settings();

	void set_server_detail_sensitive(bool sensitive = true);

	class ModelColumns : public Gtk::TreeModel::ColumnRecord {
	public:
	
		ModelColumns() {
			add(m_aet);
			add(m_port);
			add(m_hostname);
			add(m_group);
			add(m_name);
		}
		
		Gtk::TreeModelColumn<Glib::ustring> m_aet;
		Gtk::TreeModelColumn<guint> m_port;
		Gtk::TreeModelColumn<Glib::ustring> m_hostname;
		Gtk::TreeModelColumn<Glib::ustring> m_group;
		Gtk::TreeModelColumn<Glib::ustring> m_name;
	};
	
	ModelColumns m_Columns;

	Glib::RefPtr< Gtk::ListStore > m_refTreeModel;

	Gtk::TreeView* m_list_servers;

private:

	// dicom settings
	
	Gtk::Entry* m_local_aet;

	Gtk::Entry* m_local_port;

	// server details
	
	Gtk::Entry* m_server_detail_server;
	
	Gtk::Entry* m_server_detail_aet;
	
	Gtk::Entry* m_server_detail_port;
	
	Gtk::ComboBoxEntry* m_server_detail_group;
	
	Gtk::Entry* m_server_detail_description;

	Gtk::Button* m_server_detail_echo;
	
	Gtk::Label* m_server_detail_echostatus;

	// main buttons
	
	Gtk::Button* m_settings_ok;


	Glib::RefPtr<Gnome::Glade::Xml> m_refGlade;

	Glib::RefPtr<Gnome::Conf::Client> m_conf_client;
};

#endif // AESKULAP_SETTINGS_H
