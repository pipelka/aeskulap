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
    Update Date:      $Date: 2005/08/23 19:32:03 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/widgets/imagelayouttoolbutton.h,v $
    CVS/RCS Revision: $Revision: 1.1 $
    Status:           $State: Exp $
*/

#ifndef AESKULAP_IMAGELAYOUTTOOLBUTTON_H_
#define AESKULAP_IMAGELAYOUTTOOLBUTTON_H_

#include <gtkmm.h>
#include <gtkmm/menutoolbutton.h>

class ImageLayoutToolButton : public Gtk::MenuToolButton {
public:

	ImageLayoutToolButton();

	sigc::signal<void, int, int> signal_change_layout;

	void set_layout(int tilex, int tiley);

protected:

	void on_clicked();

	void on_change_layout(int index);

	Gtk::Menu m_menu;

private:

	int m_index;
};

#endif // AESKULAP_IMAGELAYOUTTOOLBUTTON_H_
