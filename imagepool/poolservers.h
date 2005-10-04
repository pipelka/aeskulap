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
    Update Date:      $Date: 2005/10/04 21:42:29 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/imagepool/poolservers.h,v $
    CVS/RCS Revision: $Revision: 1.3 $
    Status:           $State: Exp $
*/

#ifndef IMAGEPOOL_POOLSERVERS_H
#define IMAGEPOOL_POOLSERVERS_H

#include <glibmm.h>
#include <string>
#include <map>
#include <set>


class DcmDataset;

namespace ImagePool {
	
class Server {
public:

	Server();

	Server(const std::string& hostname, const std::string& aet, int port, bool lossy = false);

	bool send_echo(std::string& status);
	
	bool send_echo();

	std::string m_name;
	std::string m_hostname;
	std::string m_aet;
	guint m_port;
	std::string m_group;
	bool m_lossy;
};

class ServerList : public Glib::Object {
public:
	inline ImagePool::Server& operator[](const std::string& key) {
		return m_map[key];
	}

	typedef std::map< std::string, Server >::iterator iterator;

	inline iterator begin() {
		return m_map.begin();
	}
		
	inline iterator end() {
		return m_map.end();
	}
		
	inline void clear() {
		m_map.clear();
	}
		
	inline int size() {
		return m_map.size();
	}

	inline iterator find(const std::string& key) {
		return m_map.find(key);
	}

	static Glib::RefPtr<ImagePool::ServerList> get(const std::string groupfilter = "");

	static ImagePool::Server* find_server(const std::string& name);

	static void update();

	static const std::set< std::string >& get_groups();

protected:

	std::map< std::string, Server > m_map;

private:

	static ServerList m_serverlist;
	
	static std::set< std::string > m_servergroups;

};

} // namespace ImagePool

#endif // IMAGEPOOL_POOLSERVERS_H
