/*
    Aeskulap ImagePool - DICOM abstraction library
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

    Last Update:      $Author$
    Update Date:      $Date$
    Source File:      $Source$
    CVS/RCS Revision: $Revision$
    Status:           $State$
*/

#include <gconfmm.h>

#include "imagepool.h"

namespace ImagePool {

static ServerList m_serverlist;

Glib::RefPtr<ImagePool::ServerList> get_serverlist(const std::string groupfilter) {
	update_serverlist();

	ImagePool::ServerList* list = new ServerList;
	for(ServerList::iterator i = m_serverlist.begin(); i != m_serverlist.end(); i++) {
		// no filter -> all servers
		if(groupfilter.empty()) {
			(*list)[i->first] = i->second;
		}
		// filter
		else if(i->second.m_group == groupfilter) {
			(*list)[i->first] = i->second;
		}
	}	
	return Glib::RefPtr<ImagePool::ServerList>(list);
}

void update_serverlist() {
	m_serverlist.clear();

	Glib::RefPtr<Gnome::Conf::Client> client = Gnome::Conf::Client::get_default_client();

	Gnome::Conf::SListHandle_ValueString aet_list = client->get_string_list("/apps/aeskulap/preferences/server_aet");
	Gnome::Conf::SListHandle_ValueInt port_list = client->get_int_list("/apps/aeskulap/preferences/server_port");
	Gnome::Conf::SListHandle_ValueString hostname_list = client->get_string_list("/apps/aeskulap/preferences/server_hostname");
	Gnome::Conf::SListHandle_ValueString description_list = client->get_string_list("/apps/aeskulap/preferences/server_description");
	Gnome::Conf::SListHandle_ValueString group_list = client->get_string_list("/apps/aeskulap/preferences/server_group");
	
	Gnome::Conf::SListHandle_ValueString::iterator a = aet_list.begin();
	Gnome::Conf::SListHandle_ValueInt::iterator p = port_list.begin();
	Gnome::Conf::SListHandle_ValueString::iterator h = hostname_list.begin();
	Gnome::Conf::SListHandle_ValueString::iterator d = description_list.begin();
	Gnome::Conf::SListHandle_ValueString::iterator g = group_list.begin();
	
	for(; h != hostname_list.end() && a != aet_list.end() && p != port_list.end(); a++, p++, h++) {

		std::string servername;
		if(d != description_list.end()) {
			servername = *d;
			d++;
		}
		else {
			char buffer[50];
			sprintf(buffer, "Server%i", m_serverlist.size()+1);
			servername = buffer;
		}

		Server& s = m_serverlist[servername];
		s.m_aet = *a;
		s.m_port = *p;
		s.m_hostname = *h;
		s.m_name = servername;
		
		if(g != group_list.end()) {
			s.m_group = *g;
			g++;
		}
	}
}

const std::set<std::string>& get_servergroups() {
	static std::set<std::string> groups;
	update_serverlist();

	groups.clear();
	for(ServerList::iterator i = m_serverlist.begin(); i != m_serverlist.end(); i++) {
		if(!i->second.m_group.empty()) {
			groups.insert(i->second.m_group);
		}
	}

	return groups;
}

} // namespace ImagePool
