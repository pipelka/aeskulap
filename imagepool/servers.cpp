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
	
const std::set<std::string>& get_servergroups() {
	static std::set<std::string> groups;

	Glib::RefPtr<Gnome::Conf::Client> client = Gnome::Conf::Client::get_default_client();
	Gnome::Conf::SListHandle_ValueString group_list = client->get_string_list("/apps/aeskulap/preferences/server_group");
	Gnome::Conf::SListHandle_ValueString::iterator g = group_list.begin();

	for(; g != group_list.end(); g++) {
		if(!(*g).empty()) {
			groups.insert(*g);
		}
	}

	return groups;
}

} // namespace ImagePool
