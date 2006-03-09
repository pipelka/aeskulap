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
    Update Date:      $Date: 2006/03/09 15:35:14 $
    Update Date:      $Date: 2006/03/09 15:35:14 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/imagepool/poolservers.cpp,v $
    CVS/RCS Revision: $Revision: 1.4.2.2 $
    Status:           $State: Exp $
*/

#include "imagepool.h"
#include "poolservers.h"
#include "poolassociation.h"

namespace ImagePool {

extern Network net;

ServerList ServerList::m_serverlist;
std::set< std::string > ServerList::m_servergroups;


Server::Server() {
	m_lossy = false;
}

Server::Server(const std::string& hostname, const std::string& aet, int port, bool lossy) {
	m_hostname = hostname;
	m_aet = aet;
	m_port = port;
	m_lossy = lossy;
}

bool Server::send_echo(std::string& status) {
	Association a;
	Aeskulap::Configuration& conf = Aeskulap::Configuration::get_instance();
	
	a.Create(m_aet, m_hostname, m_port, conf.get_local_aet(), UID_VerificationSOPClass);
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

	Aeskulap::Configuration& conf = Aeskulap::Configuration::get_instance();
	Aeskulap::Configuration::ServerList::iterator i;
	Aeskulap::Configuration::ServerList* list = conf.get_serverlist();

	for(i = list->begin(); i != list->end(); i++) {

		Server& s = m_serverlist[i->second.m_name];
		s.m_aet = i->second.m_aet;
		s.m_port = i->second.m_port;
		s.m_hostname = i->second.m_hostname;
		s.m_name = i->second.m_name;
		s.m_group = i->second.m_group;
		s.m_lossy = i->second.m_lossy;
	}
	
	delete list;
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
