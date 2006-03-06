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
    Update Date:      $Date: 2006/03/06 11:07:25 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/src/settings.h,v $
    CVS/RCS Revision: $Revision: 1.10 $
    Status:           $State: Exp $
*/

#ifndef AESKULAP_SETTINGS_H
#define AESKULAP_SETTINGS_H

#include <gtkmm.h>
#include <libglademm/xml.h>

#include "aconfigclient.h"

class Settings : public Gtk::Window, public Aeskulap::ConfigClient {
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

	void on_windowlevels_modality_changed();

	void on_windowlevels_add();

	void on_windowlevels_remove();

	void save_settings();

	void restore_settings();

	void set_server_detail_sensitive(bool sensitive = true);

	void reload_windowlevel_preset(const Glib::ustring& modality);

	void store_windowlevel_preset();

	class ModelColumns : public Gtk::TreeModel::ColumnRecord {
	public:
	
		ModelColumns() {
			add(m_aet);
			add(m_port);
			add(m_hostname);
			add(m_group);
			add(m_name);
			add(m_lossy);
		}
		
		Gtk::TreeModelColumn<Glib::ustring> m_aet;
		Gtk::TreeModelColumn<guint> m_port;
		Gtk::TreeModelColumn<Glib::ustring> m_hostname;
		Gtk::TreeModelColumn<Glib::ustring> m_group;
		Gtk::TreeModelColumn<Glib::ustring> m_name;
		Gtk::TreeModelColumn<bool> m_lossy;
	};
	
	ModelColumns m_Columns;

	Glib::RefPtr< Gtk::ListStore > m_refTreeModel;

	Gtk::TreeView* m_list_servers;

	class WindowLevelColumns : public Gtk::TreeModel::ColumnRecord {
	public:
	
		WindowLevelColumns() {
			add(m_description);
			add(m_center);
			add(m_width);
		}
		
		Gtk::TreeModelColumn<Glib::ustring> m_description;
		Gtk::TreeModelColumn<gint> m_center;
		Gtk::TreeModelColumn<gint> m_width;
	};
	
	WindowLevelColumns m_WindowLevelColumns;

	Glib::RefPtr< Gtk::ListStore > m_refWindowLevelModel;

private:

	// dicom settings
	
	Gtk::Entry* m_local_aet;

	Gtk::Entry* m_local_port;

	// server details
	
	Gtk::Entry* m_server_detail_server;
	
	Gtk::Entry* m_server_detail_aet;
	
	Gtk::Entry* m_server_detail_port;
	
	Gtk::ComboBoxEntry* m_server_detail_group;
	
	Gtk::ComboBoxText* m_characterset;

	Gtk::Entry* m_server_detail_description;

	Gtk::Button* m_server_detail_echo;
	
	Gtk::CheckButton* m_server_detail_lossy;

	Gtk::Label* m_server_detail_echostatus;

	// presets

	Gtk::ComboBox* m_presets_windowlevels_modality;

	Gtk::TreeView* m_presets_windowlevels;

	Glib::ustring m_windowlevels_modality;

	// main buttons
	
	Gtk::Button* m_settings_ok;


	Glib::RefPtr<Gnome::Glade::Xml> m_refGlade;

};

#endif // AESKULAP_SETTINGS_H
