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
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/imagepool/netclient.h,v $
    CVS/RCS Revision: $Revision: 1.12 $
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
		ImagePool::Server* s = ServerList::find_server(server);

		if(s == NULL) {
			return false;
		}

		T::SetAcceptLossyImages(s->m_lossy);

		T::Create(
				s->m_aet,
				s->m_hostname,
				s->m_port,
				get_ouraet(),
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

};
	
} // namespace ImagePool

#endif // IMAGEPOOL_NETCLIENT_H
