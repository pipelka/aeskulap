/*
    Aeskulap - DICOM image viewer and network client
    Copyright (C) 2005  Alexander Pipelka

    This file is part of Aeskulap.

    Aeskulap is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Aeskulap is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Aeskulap; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Alexander Pipelka
    pipelka@teleweb.at

    Last Update:      $Author: braindead $
    Update Date:      $Date: 2006/03/05 19:37:28 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/src/windowleveldialog.cpp,v $
    CVS/RCS Revision: $Revision: 1.1 $
    Status:           $State: Exp $
*/

#include "windowleveldialog.h"

WindowLevelDialog::WindowLevelDialog(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade) :
Gtk::Dialog(cobject),
m_refGlade(refGlade) {

	m_refGlade->get_widget("add_windowlevel_description", m_description);

	m_refGlade->get_widget("add_windowlevel_modality", m_modality);

	m_refGlade->get_widget("add_windowlevel_center", m_center);

	m_refGlade->get_widget("add_windowlevel_width", m_width);
}

WindowLevelDialog::~WindowLevelDialog() {
}

void WindowLevelDialog::set(const Aeskulap::WindowLevel& level) {
	m_description->set_text(level.description);
	m_modality->set_text(level.modality);
	m_center->set_value(level.center);
	m_width->set_value(level.width);
}

Aeskulap::WindowLevel WindowLevelDialog::get() {
	Aeskulap::WindowLevel level;
	level.description = m_description->get_text();
	level.modality = m_modality->get_text();
	level.center = m_center->get_value_as_int();
	level.width = m_width->get_value_as_int();
	
	return level;
}
