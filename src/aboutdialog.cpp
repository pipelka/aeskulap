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
    Update Date:      $Date: 2005/10/03 10:42:41 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/src/aboutdialog.cpp,v $
    CVS/RCS Revision: $Revision: 1.1 $
    Status:           $State: Exp $
*/

#include "aboutdialog.h"
#include "aiconfactory.h"

AboutDialog::AboutDialog(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade) :
Gtk::AboutDialog(cobject),
m_refGlade(refGlade) {
	set_logo(Aeskulap::IconFactory::load_from_file("aeskulap.png"));
}

AboutDialog::~AboutDialog() {
}
