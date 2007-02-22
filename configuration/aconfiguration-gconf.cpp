/*
    Aeskulap Configuration - persistent configuration interface library
    Copyright (C) 2005  Alexander Pipelka

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    Alexander Pipelka
    pipelka@teleweb.at

    Last Update:      $Author: braindead $
    Update Date:      $Date: 2007/02/22 13:39:34 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/configuration/aconfiguration-gconf.cpp,v $
    CVS/RCS Revision: $Revision: 1.5 $
    Status:           $State: Exp $
*/

#include <gconfmm.h>
#include <iostream>

namespace Aeskulap {

static Glib::RefPtr<Gnome::Conf::Client> m_conf_client;

Configuration::Configuration() {
	if(!m_conf_client) {
		std::cout << "Gnome::Conf::init()" << std::endl;
		Gnome::Conf::init();
		m_conf_client = Gnome::Conf::Client::get_default_client();
	}

	m_conf_client->add_dir("/apps/aeskulap/preferences");
	m_conf_client->add_dir("/apps/aeskulap/presets");
	m_conf_client->add_dir("/apps/aeskulap/presets/windowlevel");

	if(!m_conf_client->dir_exists("/apps/aeskulap/presets/windowlevel/CT")) {
		add_default_presets_ct();
	}
}

std::string Configuration::get_local_aet() {
	std::string local_aet = m_conf_client->get_string("/apps/aeskulap/preferences/local_aet");

	if(local_aet.empty()) {
		local_aet = "AESKULAP";
		set_local_aet(local_aet);
	}
	
	return local_aet;
}
	
void Configuration::set_local_aet(const std::string& aet) {
	m_conf_client->set("/apps/aeskulap/preferences/local_aet", aet);
}

unsigned int Configuration::get_local_port() {
	m_conf_client = Gnome::Conf::Client::get_default_client();
	gint local_port = m_conf_client->get_int("/apps/aeskulap/preferences/local_port");

	if(local_port <= 0) {
		local_port = 6000;
		set_local_port(local_port);
	}
	
	return (unsigned int)local_port;
}
	
void Configuration::set_local_port(unsigned int port) {
	if(port <= 0) {
		port = 6000;
	}
	m_conf_client->set("/apps/aeskulap/preferences/local_port", (gint)port);
}

std::string Configuration::get_encoding() {
	std::string charset = m_conf_client->get_string("/apps/aeskulap/preferences/characterset");

	if(charset.empty()) {
		charset = "ISO_IR 100";
		set_encoding(charset);
	}
	
	return charset;
}

void Configuration::set_encoding(const std::string& encoding) {
	m_conf_client->set("/apps/aeskulap/preferences/characterset", encoding);
}

Configuration::ServerList* Configuration::get_serverlist() {
	Configuration::ServerList* list = new Configuration::ServerList;

	Gnome::Conf::SListHandle_ValueString aet_list = m_conf_client->get_string_list("/apps/aeskulap/preferences/server_aet");
	Gnome::Conf::SListHandle_ValueInt port_list = m_conf_client->get_int_list("/apps/aeskulap/preferences/server_port");
	Gnome::Conf::SListHandle_ValueString hostname_list = m_conf_client->get_string_list("/apps/aeskulap/preferences/server_hostname");
	Gnome::Conf::SListHandle_ValueString description_list = m_conf_client->get_string_list("/apps/aeskulap/preferences/server_description");
	Gnome::Conf::SListHandle_ValueString group_list = m_conf_client->get_string_list("/apps/aeskulap/preferences/server_group");
	Gnome::Conf::SListHandle_ValueBool lossy_list = m_conf_client->get_bool_list("/apps/aeskulap/preferences/server_lossy");
	
	Gnome::Conf::SListHandle_ValueString::iterator a = aet_list.begin();
	Gnome::Conf::SListHandle_ValueInt::iterator p = port_list.begin();
	Gnome::Conf::SListHandle_ValueString::iterator h = hostname_list.begin();
	Gnome::Conf::SListHandle_ValueString::iterator d = description_list.begin();
	Gnome::Conf::SListHandle_ValueString::iterator g = group_list.begin();
	Gnome::Conf::SListHandle_ValueBool::iterator l = lossy_list.begin();
	
	for(; h != hostname_list.end() && a != aet_list.end() && p != port_list.end(); a++, p++, h++) {

		std::string servername;
		if(d != description_list.end()) {
			servername = *d;
			d++;
		}
		else {
			char buffer[50];
			snprintf(buffer, sizeof(buffer), "Server%i", list->size()+1);
			servername = buffer;
		}

		ServerData& s = (*list)[servername];
		s.m_aet = *a;
		s.m_port = *p;
		s.m_hostname = *h;
		s.m_name = servername;
		
		if(l != lossy_list.end()) {
			s.m_lossy = *l;
			l++;
		}

		if(g != group_list.end()) {
			s.m_group = *g;
			g++;
		}
	}
	
	return list;
}

void Configuration::set_serverlist(std::vector<ServerData>& list) {
	std::vector< Glib::ustring > aet_list;
	std::vector< Glib::ustring > hostname_list;
	std::vector< int > port_list;
	std::vector< Glib::ustring > description_list;
	std::vector< Glib::ustring > group_list;
	std::vector< gboolean > lossy_list;

	std::vector<ServerData>::iterator i;
	for(i = list.begin(); i != list.end(); i++) {
		aet_list.push_back(i->m_aet);
		hostname_list.push_back(i->m_hostname);
		port_list.push_back(i->m_port);
		description_list.push_back(i->m_name);
		group_list.push_back(i->m_group);
		lossy_list.push_back(i->m_lossy);
	}

	m_conf_client->set_string_list("/apps/aeskulap/preferences/server_aet", aet_list);
	m_conf_client->set_string_list("/apps/aeskulap/preferences/server_hostname", hostname_list);
	m_conf_client->set_int_list("/apps/aeskulap/preferences/server_port", port_list);
	m_conf_client->set_string_list("/apps/aeskulap/preferences/server_description", description_list);
	m_conf_client->set_string_list("/apps/aeskulap/preferences/server_group", group_list);
	m_conf_client->set_bool_list("/apps/aeskulap/preferences/server_lossy", lossy_list);
}

bool Configuration::get_windowlevel(const Glib::ustring& modality, const Glib::ustring& desc, WindowLevel& w) {
	Glib::ustring base = "/apps/aeskulap/presets/windowlevel/"+modality+"/"+desc;

	if(!m_conf_client->dir_exists(base)) {
		return false;
	}

	if(m_conf_client->get_without_default(base+"/center").get_type() == Gnome::Conf::VALUE_INVALID) {
		return false;
	}

	w.modality = modality;
	w.description = desc;
	w.center = m_conf_client->get_int(base+"/center");
	w.width = m_conf_client->get_int(base+"/width");

	return true;
}

bool Configuration::get_windowlevel_list(const Glib::ustring& modality, WindowLevelList& list) {
	if(modality.empty()) {
		return false;
	}

	Glib::ustring base = "/apps/aeskulap/presets/windowlevel/"+modality;

	std::vector< Glib::ustring > dirs = m_conf_client->all_dirs(base);
	if(dirs.size() == 0) {
		return false;
	}

	list.clear();
	
	for(int i=0; i<dirs.size(); i++) {
		WindowLevel w;
		if(get_windowlevel(modality, get_name_from_path(dirs[i]), w)) {
			list[w.description] = w;
		}
	}
	
	return true;
}

bool Configuration::set_windowlevel(const WindowLevel& w) {
	Glib::ustring base = "/apps/aeskulap/presets/windowlevel/"+w.modality+"/"+w.description;

	if(!m_conf_client->dir_exists(base)) {
		m_conf_client->add_dir(base);
	}

	m_conf_client->set(base+"/center", w.center);
	m_conf_client->set(base+"/width", w.width);
	
	return true;
}

bool Configuration::set_windowlevel_list(const Glib::ustring& modality, WindowLevelList& list) {
	Glib::ustring base = "/apps/aeskulap/presets/windowlevel/"+modality;
	WindowLevelList::iterator i;
	
	for(i = list.begin(); i != list.end(); i++) {
		i->second.modality = modality;
		set_windowlevel(i->second);
	}
	
	return true;
}

bool Configuration::unset_windowlevels(const Glib::ustring& modality) {
	Glib::ustring base = "/apps/aeskulap/presets/windowlevel/"+modality;

	std::vector< Glib::ustring > dirs = m_conf_client->all_dirs(base);
	if(dirs.size() == 0) {
		return false;
	}

	for(int i=0; i<dirs.size(); i++) {
		Glib::ustring keybase = base+"/"+get_name_from_path(dirs[i]);
		m_conf_client->unset(keybase+"/center");
		m_conf_client->unset(keybase+"/width");
	}
	
	return true;
}

} // namespace Aeskulap
