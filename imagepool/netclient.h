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
    Update Date:      $Date: 2005/09/24 19:09:29 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/imagepool/netclient.h,v $
    CVS/RCS Revision: $Revision: 1.9 $
    Status:           $State: Exp $
*/

#ifndef IMAGEPOOL_NETCLIENT_H
#define IMAGEPOOL_NETCLIENT_H

#include <gconfmm.h>
#include "dcdatset.h"
#include "poolnetwork.h"
#include <iostream>

namespace ImagePool {

extern Network net;

template<class T>
class NetClient : public T {
public:

	sigc::signal< void, DcmStack*, std::string > signal_server_result;

	bool QueryServer(DcmDataset* query, const std::string& server, const char* syntax = NULL) {
		Glib::RefPtr<ServerList> list = ServerList::get();
		ServerList::iterator i = list->find(server);

		std::cout << "QueryServer(" << server << ")" << std::endl;

		if(i == list->end()) {
			return false;
		}
		
		Server& s = i->second;
		
		T::Create(
				s.m_aet.c_str(),
				s.m_hostname.c_str(),
				s.m_port,
				get_ouraet().c_str(),
				syntax
				);

		bool r = T::Connect(&net).good();

		if(r == true) {
			std::cout << "T::SendObject()" << std::endl;
			r = T::SendObject(query).good();
		}

		std::cout << "T::Drop()" << std::endl;
		T::Drop();
		std::cout << "T::Destroy()" << std::endl;
		T::Destroy();

		DcmStack* result = T::GetResultStack();
		if(r && result != NULL && result->card() > 0) {
			std::cout << "signal_server_result('" << server << "')" << std::endl;
			signal_server_result(result, server);
		}
		return r;
	}

	bool QueryServerGroup(DcmDataset* query, const std::string& group, const char* syntax = NULL) {
		Glib::RefPtr<ServerList> list = ServerList::get(group);
		bool rc = false;
		
		std::cout << "QueryServerGroup(" << group << ")" << std::endl;

		for(ServerList::iterator i = list->begin(); i != list->end(); i++) {
			rc |= QueryServer(query, i->first, syntax);
		}
		
		return rc;
	}

	/*bool QueryServers(DcmDataset* query, const char* syntax = NULL) {
		Glib::RefPtr<Gnome::Conf::Client> client = Gnome::Conf::Client::get_default_client();
	
		Gnome::Conf::SListHandle_ValueString aet_list = client->get_string_list("/apps/aeskulap/preferences/server_aet");
		Gnome::Conf::SListHandle_ValueInt port_list = client->get_int_list("/apps/aeskulap/preferences/server_port");
		Gnome::Conf::SListHandle_ValueString hostname_list = client->get_string_list("/apps/aeskulap/preferences/server_hostname");
	
		Gnome::Conf::SListHandle_ValueString::iterator a = aet_list.begin();
		Gnome::Conf::SListHandle_ValueInt::iterator p = port_list.begin();
		Gnome::Conf::SListHandle_ValueString::iterator h = hostname_list.begin();
	
		bool rc = false;
		for(; h != hostname_list.end() && a != aet_list.end() && p != port_list.end(); a++, p++, h++) {
			T::Create(
					(*a).c_str(),
					(*h).c_str(),
					(*p),
					get_ouraet().c_str(),
					syntax
					);
		
			bool r = SUCCESS(T::Connect(&net));
		
			if(r == true) {
				r = T::SendObject(query).good();
			}
	
			rc |= r;
	
			T::Drop();	
		}
		
		return rc;
	}*/
};
	
} // namespace ImagePool

#endif // IMAGEPOOL_NETCLIENT_H
