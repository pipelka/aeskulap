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

    Last Update:      $Author: braindead $
    Update Date:      $Date: 2006/01/12 10:45:30 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/imagepool/poolservers.cpp,v $
    CVS/RCS Revision: $Revision: 1.4 $
    Status:           $State: Exp $
*/

#include <gconfmm.h>

#include "imagepool.h"
#include "poolassociation.h"

namespace ImagePool {

extern Network net;

ServerList ServerList::m_serverlist;
std::set< std::string > ServerList::m_servergroups;


Server::Server() :
m_lossy(false) {
}

Server::Server(const std::string& hostname, const std::string& aet, int port, bool lossy) :
m_hostname(hostname),
m_aet(aet),
m_port(port),
m_lossy(lossy) {
}

bool Server::send_echo(std::string& status) {
	Association a;
	a.Create(m_aet, m_hostname, m_port, get_ouraet(), UID_VerificationSOPClass);
	if(a.Connect(&net).bad()) {
		status = gettext("Unable to create association");
		return false;
	}

	if(!a.SendEchoRequest()) {
		status = gettext("no response for echo request");
		return false;
	}
	
	a.Drop();
	a.Destroy();

	status = "echotest succeeded";
	return true;
}
	
bool Server::send_echo() {
	static std::string dummystatus;
	return send_echo(dummystatus);
}


Glib::RefPtr<ImagePool::ServerList> ServerList::get(const std::string groupfilter) {
	update();

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

ImagePool::Server* ServerList::find_server(const std::string& name) {
	ServerList::iterator i = m_serverlist.find(name);
	if(i == m_serverlist.end()) {
		return NULL;
	}
	
	return &(i->second);
}

void ServerList::update() {
	m_serverlist.clear();

	Glib::RefPtr<Gnome::Conf::Client> client = Gnome::Conf::Client::get_default_client();

	Gnome::Conf::SListHandle_ValueString aet_list = client->get_string_list("/apps/aeskulap/preferences/server_aet");
	Gnome::Conf::SListHandle_ValueInt port_list = client->get_int_list("/apps/aeskulap/preferences/server_port");
	Gnome::Conf::SListHandle_ValueString hostname_list = client->get_string_list("/apps/aeskulap/preferences/server_hostname");
	Gnome::Conf::SListHandle_ValueString description_list = client->get_string_list("/apps/aeskulap/preferences/server_description");
	Gnome::Conf::SListHandle_ValueString group_list = client->get_string_list("/apps/aeskulap/preferences/server_group");
	Gnome::Conf::SListHandle_ValueBool lossy_list = client->get_bool_list("/apps/aeskulap/preferences/server_lossy");
	
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
			sprintf(buffer, "Server%i", m_serverlist.size()+1);
			servername = buffer;
		}

		Server& s = m_serverlist[servername];
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
}

const std::set<std::string>& ServerList::get_groups() {
	update();

	m_servergroups.clear();
	for(ServerList::iterator i = m_serverlist.begin(); i != m_serverlist.end(); i++) {
		if(!i->second.m_group.empty()) {
			m_servergroups.insert(i->second.m_group);
		}
	}

	return m_servergroups;
}

} // namespace ImagePool
