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
    Update Date:      $Date: 2007/05/10 14:29:59 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/configuration/aconfiguration.h,v $
    CVS/RCS Revision: $Revision: 1.4 $
    Status:           $State: Exp $
*/

#ifndef AESKULAP_CONFIGURATION_H
#define AESKULAP_CONFIGURATION_H

#include <glibmm.h>
#include <string>
#include <map>
#include <vector>

#include "awindowlevel.h"

namespace Aeskulap {

class Configuration {
public:

	// data structures

	class ServerData {
	public:
		Glib::ustring m_name;
		Glib::ustring m_hostname;
		Glib::ustring m_aet;
		unsigned int m_port;
		Glib::ustring m_group;
		bool m_lossy;
		bool m_relational;
	};

	typedef std::map< Glib::ustring, ServerData > ServerList;


	// backend independent methods
	
	static Configuration& get_instance();

	void add_default_presets_ct();

	// backend specific functions

	ServerList* get_serverlist();

	void set_serverlist(std::vector<ServerData>& list);

	std::string get_local_aet();
	
	void set_local_aet(const std::string& aet);

	unsigned int get_local_port();
	
	void set_local_port(unsigned int port);

	std::string get_encoding();
	
	void set_encoding(const std::string& encoding);

	bool get_windowlevel(const Glib::ustring& modality, const Glib::ustring& desc, WindowLevel& w);

	bool get_windowlevel_list(const Glib::ustring& modality, WindowLevelList& list);

	bool set_windowlevel(const WindowLevel& w);

	bool set_windowlevel_list(const Glib::ustring& modality, WindowLevelList& list);

	bool unset_windowlevels(const Glib::ustring& modality);

protected:

	Configuration();

private:

	// internal helper (backend independend) functions
	Glib::ustring get_name_from_path(const Glib::ustring& path);

};

} // namespace Aeskulap

#endif // AESKULAP_CONFIGURATION_H
