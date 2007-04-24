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
    Update Date:      $Date: 2007/04/24 10:49:37 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/configuration/aconfiguration.cpp,v $
    CVS/RCS Revision: $Revision: 1.5 $
    Status:           $State: Exp $
*/

#include "aconfiguration.h"
#include "gettext.h"

namespace Aeskulap {

Configuration& Configuration::get_instance() {
	static Configuration c;
	return c;
}

void Configuration::add_default_presets_ct() {

	/* DEFAULT CT WINDOWLEVELS
	 * 
	 * (Thanks Dr. Grover Ronald Pardo Holzwarth)
	 * 
	 * Abdomen     W:  250  C:  50
	 * Head        W:  150  C:  50
	 * Lung        W:  2000 C: -550
	 * Mediastinum W:  450  C:  50
	 * Spine       W:  300  C:  40
	 * Vertebrae   W:  2300 C:  530
	 */

	set_windowlevel(WindowLevel(gettext("Abdomen"), "CT", 50, 250));
	set_windowlevel(WindowLevel(gettext("Head"), "CT", 50, 150));
	set_windowlevel(WindowLevel(gettext("Lung"), "CT", -550, 2000));
	set_windowlevel(WindowLevel(gettext("Mediastinum"), "CT", 50, 450));
	set_windowlevel(WindowLevel(gettext("Spine"), "CT", 40, 300));
	set_windowlevel(WindowLevel(gettext("Vertebrae"), "CT", 530, 2300));
}

Glib::ustring Configuration::get_name_from_path(const Glib::ustring& path) {
	Glib::ustring name;

	Glib::ustring::size_type p = path.rfind("/");
	if(p == Glib::ustring::npos) {
		name = path;
	}
	else {
		name = path.substr(p+1);
	}
	
	return name;
}

} // namespace Aeskulap

#ifdef WIN32

// WIN32 configuration backend

#include "aconfiguration-win32.cpp"

#else

// gconf configuration backend

#include "aconfiguration-gconf.cpp"

#endif

