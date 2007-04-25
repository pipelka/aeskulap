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

    Last Update:      $Author$
    Update Date:      $Date$
    Source File:      $Source$
    CVS/RCS Revision: $Revision$
    Status:           $State$
*/
#include <windows.h>
#include <winreg.h>
#include <iostream>
#include <string>
#include <list>


namespace Aeskulap {

class Win32Registry {
public:
    bool set(const std::string &key, const std::string &value) const {
        HKEY hKey;
        DWORD disp;
        std::string path;
        std::string name;
        split_key(key, path, name);
        LONG ret = RegCreateKeyEx(HKEY_CURRENT_USER, path.c_str(), 0, "AESKULAP", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &disp);
        if( ret != ERROR_SUCCESS )
        {
            RegCloseKey(hKey);
            return false;
        }
        ret = RegSetValueEx(hKey, name.c_str(), 0, REG_SZ, (BYTE *)value.c_str(), value.size() + 1);
        RegCloseKey(hKey);
        return ret==ERROR_SUCCESS;
    }
    bool set(const std::string &key, int value) const {
        HKEY hKey;
        DWORD disp;
        std::string path;
        std::string name;
        split_key(key, path, name);
        LONG ret = RegCreateKeyEx(HKEY_CURRENT_USER, path.c_str(), 0, "AESKULAP", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &disp);
        if( ret != ERROR_SUCCESS )
        {
            RegCloseKey(hKey);
            return false;
        }
        ret = RegSetValueEx(hKey, name.c_str(), 0, REG_DWORD, (BYTE*)&value, sizeof(value));
        RegCloseKey(hKey);
        return ret==ERROR_SUCCESS;
    }

    int get_int(const std::string &dir) const {
        HKEY hKey;
        DWORD ret;
        DWORD type;
        DWORD len;
        DWORD val;
        int retval;
        std::string key, name;
        split_key(dir, key, name);
        ret = RegOpenKeyEx(HKEY_CURRENT_USER, key.c_str(), 0, KEY_READ, &hKey);
        if( ret!=ERROR_SUCCESS )
            return -1;
        len = sizeof(val);
        type=REG_DWORD;
        ret = RegQueryValueEx(hKey, name.c_str(), NULL, &type, (BYTE*)&val, &len);
        if( ret!=ERROR_SUCCESS || type != REG_DWORD )
            retval = -1;
        else
            retval = val;
        RegCloseKey(hKey);
        return retval;
    }

    std::string get_string(const std::string &dir) const {
        HKEY hKey;
        DWORD ret;
        DWORD type;
        DWORD len;
        char val[512];
        std::string retval;
        std::string key, name;
        split_key(dir, key, name);
        ret = RegOpenKeyEx(HKEY_CURRENT_USER, key.c_str(), 0, KEY_READ, &hKey);
        if( ret!=ERROR_SUCCESS )
            return "";
        len = sizeof(val);
        ret = RegQueryValueEx(hKey, name.c_str(), NULL, &type, (BYTE*)val, &len);
        if( ret!=ERROR_SUCCESS || type != REG_SZ)
            retval = "";
        else
            retval = val;
        RegCloseKey(hKey);
        return retval;
    }

private:
    void split_key(const std::string &key, std::string &path, std::string &name) const {
        std::string::size_type pos = key.find_last_of("\\");
        if( pos== std::string::npos )
        {
            path = name = "";
        }
        else
        {
            path = key.substr(0, pos);
            name = key.substr(pos+1);
        }
    }
};

static Win32Registry Win32RegsitryInstance;
static Win32Registry *m_conf_client = &Win32RegsitryInstance;


Configuration::Configuration() {
    HKEY hKey;
    DWORD ret;

    ret = RegOpenKeyEx(HKEY_CURRENT_USER, "SOFTWARE\\Aeskulap\\presets\\windowlevel\\CT", 0, KEY_READ, &hKey);
	if(ret != ERROR_SUCCESS ) {
        add_default_presets_ct();
	}
	else
        RegCloseKey(hKey);

}

std::string Configuration::get_local_aet() {
	std::string local_aet = m_conf_client->get_string("SOFTWARE\\Aeskulap\\preferences\\local_aet");
	if(local_aet.empty()) {
		local_aet = "AESKULAP";
		set_local_aet(local_aet);
	}

	return local_aet;
}

void Configuration::set_local_aet(const std::string& aet) {
	m_conf_client->set("SOFTWARE\\Aeskulap\\preferences\\local_aet", aet);
}

unsigned int Configuration::get_local_port() {
	gint local_port = m_conf_client->get_int("SOFTWARE\\Aeskulap\\preferences\\local_port");
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
	m_conf_client->set("SOFTWARE\\Aeskulap\\preferences\\local_port", port);
}

std::string Configuration::get_encoding() {
	std::string charset = m_conf_client->get_string("SOFTWARE\\Aeskulap\\preferences\\characterset");

	if(charset.empty()) {
		charset = "ISO_IR 100";
		set_encoding(charset);
	}

	return charset;
}

void Configuration::set_encoding(const std::string& encoding) {
	m_conf_client->set("SOFTWARE\\Aeskulap\\preferences\\characterset", encoding);
}

Configuration::ServerList* Configuration::get_serverlist() {
	Configuration::ServerList* list = new Configuration::ServerList;
    HKEY hServersKey;
    HKEY hKey;
    LONG ret;
    std::string serverKey = "SOFTWARE\\Aeskulap\\preferences\\serverlist";
    char servername[512];
    char buf[256];
    DWORD buflen;
    std::string aet;
    std::string hostname;
    DWORD port;
    std::string group;
    DWORD lossy;

    ret = RegOpenKeyEx(HKEY_CURRENT_USER, serverKey.c_str(), 0, KEY_READ | KEY_ENUMERATE_SUB_KEYS, &hServersKey);
    if( ret != ERROR_SUCCESS )
        return list;
    int serverIdx=0;
    while(  RegEnumKey(hServersKey, serverIdx++, servername, sizeof(servername)) == ERROR_SUCCESS )
    {
        hKey = NULL;
        ret = RegOpenKeyEx(HKEY_CURRENT_USER, (serverKey + "\\" + servername).c_str(), 0, KEY_READ, &hKey);
        if( ret == ERROR_SUCCESS )
        {
            DWORD type;

            buflen = sizeof(buf);
            ret = RegQueryValueEx(hKey, "AET", 0, &type, (BYTE*)buf, &buflen);
            if( ret != ERROR_SUCCESS || type != REG_SZ )
                continue;
            aet = buf;
            buflen = sizeof(buf);
            ret = RegQueryValueEx(hKey, "Host", 0, &type, (BYTE*)buf, &buflen);
            if( ret != ERROR_SUCCESS || type != REG_SZ )
                continue;
            hostname = buf;
            buflen = sizeof(buf);
            ret = RegQueryValueEx(hKey, "Group", 0, &type, (BYTE*)buf, &buflen);
            if( ret != ERROR_SUCCESS || type != REG_SZ )
                continue;
            group = buf;
            buflen = sizeof(buf);
            buflen = sizeof(port);
            ret = RegQueryValueEx(hKey, "Port", 0, &type, (BYTE*)&port, &buflen);
            if( ret != ERROR_SUCCESS || type != REG_DWORD )
                continue;
            buflen = sizeof(lossy);
            ret = RegQueryValueEx(hKey, "Lossy", 0, &type, (BYTE*)&lossy, &buflen);
            if( ret != ERROR_SUCCESS || type != REG_DWORD )
                continue;

            ServerData& s = (*list)[std::string(servername)];
            s.m_aet = aet;
            s.m_hostname = hostname;
            s.m_group = group;
            s.m_name = servername;
            s.m_lossy = lossy;
            s.m_port = port;
        }
        if( hKey )
            RegCloseKey(hKey);
    }
    /*
	snprintf(buf, sizeof(buf), "Server%i", list->size()+1);
    strncpy(servername, buf, sizeof(servername));
    ServerData& s = (*list)[std::string(servername)];
    s.m_aet = "AET";
    s.m_hostname = "ip";
    s.m_group = "";
    s.m_name = servername;
    s.m_lossy = 0;
    s.m_port = 104;
    */
    RegCloseKey(hServersKey);

	return list;
}

void Configuration::set_serverlist(std::vector<ServerData>& list) {
    std::vector<ServerData>::const_iterator i;
    HKEY hServersKey;
    HKEY hKey;
    DWORD disp;
    LONG ret;
    std::string serverKey = "SOFTWARE\\Aeskulap\\preferences\\serverlist";
    char servername[512];

    ret = RegCreateKeyEx(HKEY_CURRENT_USER, serverKey.c_str(), 0, "AESKULAP", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hServersKey, &disp);
    if( ret != ERROR_SUCCESS )
        return;
    int serverIdx = 0;
    while(  RegEnumKey(hServersKey, serverIdx++, servername, sizeof(servername)) == ERROR_SUCCESS )
    {
        hKey = NULL;
        ret = RegDeleteKey(hServersKey, servername);
    }

	for(i = list.begin(); i != list.end(); i++)
	{
	    ret = RegCreateKeyEx(hServersKey, i->m_name.c_str(), 0, "AESKULAP", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &disp);
        if( ret == ERROR_SUCCESS )
        {
            DWORD tmp;
            RegSetValueEx(hKey, "AET", 0,REG_SZ, (BYTE*)i->m_aet.c_str(), i->m_aet.size());
            RegSetValueEx(hKey, "Host", 0,REG_SZ, (BYTE*)i->m_hostname.c_str(), i->m_hostname.size());
            RegSetValueEx(hKey, "Group", 0,REG_SZ, (BYTE*)i->m_group.c_str(), i->m_group.size());
            tmp = i->m_port;
            RegSetValueEx(hKey, "Port", 0, REG_DWORD, (BYTE*)&tmp, sizeof(tmp));
            tmp = i->m_lossy;
            RegSetValueEx(hKey, "Lossy", 0,REG_DWORD, (BYTE*)&tmp, sizeof(tmp));
        }
        RegCloseKey(hKey);
	}
	RegCloseKey(hServersKey);
}

bool Configuration::get_windowlevel(const Glib::ustring& modality, const Glib::ustring& desc, WindowLevel& w) {

    HKEY hKey;
    DWORD ret;
    DWORD type;
    DWORD len;
    DWORD val;
    std::string key = "SOFTWARE\\Aeskulap\\presets\\windowlevel\\" + modality + "\\" + desc;

    ret = RegOpenKeyEx(HKEY_CURRENT_USER, key.c_str(), 0, KEY_READ | KEY_ENUMERATE_SUB_KEYS, &hKey);
    if( ret != ERROR_SUCCESS )
        return false;

	len = sizeof(DWORD);
	ret = RegQueryValueEx(hKey, "center", NULL, &type, (BYTE*)&val, &len);
	if( ret != ERROR_SUCCESS || type != REG_DWORD )
	{
	    RegCloseKey(hKey);
	    return false;
	}
	w.center = val;
	ret = RegQueryValueEx(hKey, "width", NULL, &type, (BYTE*)&val, &len);
	if( ret != ERROR_SUCCESS || type != REG_DWORD )
	{
	    RegCloseKey(hKey);
	    return false;
	}
	w.width = val;
	RegCloseKey(hKey);

	w.modality = modality;
	w.description = desc;
	return true;
}

bool Configuration::get_windowlevel_list(const Glib::ustring& modality, WindowLevelList& list) {

    HKEY hKey;
    DWORD ret;
    int serverIdx = 0;
    char desc[256];

    std::string key = "SOFTWARE\\Aeskulap\\presets\\windowlevel\\" + modality ;
	if(modality.empty()) {
		return false;
	}
    ret = RegOpenKeyEx(HKEY_CURRENT_USER, key.c_str(), 0, KEY_READ | KEY_ENUMERATE_SUB_KEYS, &hKey);
    if( ret != ERROR_SUCCESS )
        return false;

    list.clear();
    while(  RegEnumKey(hKey, serverIdx++, desc, sizeof(desc)) == ERROR_SUCCESS )
    {
        WindowLevel w;
        if(get_windowlevel(modality, Glib::ustring(desc), w)) {
            list[w.description] = w;
        }
    }
    RegCloseKey(hKey);
	return true;
}

bool Configuration::set_windowlevel(const WindowLevel& w) {

	std::string base = "SOFTWARE\\Aeskulap\\presets\\windowlevel\\"+w.modality+"\\"+w.description;

	m_conf_client->set(base+"\\center", w.center);
	m_conf_client->set(base+"\\width", w.width);

	return true;
}

bool Configuration::set_windowlevel_list(const Glib::ustring& modality, WindowLevelList& list) {
	WindowLevelList::iterator i;

	for(i = list.begin(); i != list.end(); i++) {
		i->second.modality = modality;
		set_windowlevel(i->second);
	}

	return true;
}

bool Configuration::unset_windowlevels(const Glib::ustring& modality) {
	std::string base = "SOFTWARE\\Aeskulap\\presets\\windowlevel\\";
	DWORD ret;
	HKEY hKey;

	ret = RegOpenKeyEx(HKEY_CURRENT_USER, base.c_str(), 0, KEY_ALL_ACCESS, &hKey);
	if( ret!= ERROR_SUCCESS )
        return false;

    ret = RegDeleteKey(hKey, modality.c_str());
    RegCloseKey(hKey);
	return ret == ERROR_SUCCESS;
}
		
} // namespace Aeskulap

