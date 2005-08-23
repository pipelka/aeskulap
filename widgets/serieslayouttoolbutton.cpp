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
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/widgets/serieslayouttoolbutton.cpp,v $
    CVS/RCS Revision: $Revision: 1.1 $
    Status:           $State: Exp $
*/

#include "serieslayouttoolbutton.h"
#include "astockids.h"

SeriesLayoutToolButton::SeriesLayoutToolButton() {

	set_stock_id(Aeskulap::Stock::SERIES_2X1);
	m_index = 0;

	Gtk::MenuItem* menuitem = manage(new Gtk::ImageMenuItem(Aeskulap::Stock::SERIES_2X1));
	menuitem->signal_activate().connect(sigc::bind(sigc::mem_fun(*this, &SeriesLayoutToolButton::on_change_layout), 0));
	m_menu.add(*menuitem);
	menuitem->show();

	menuitem = manage(new Gtk::ImageMenuItem(Aeskulap::Stock::SERIES_2X2));
	menuitem->signal_activate().connect(sigc::bind(sigc::mem_fun(*this, &SeriesLayoutToolButton::on_change_layout), 1));
	m_menu.add(*menuitem);
	menuitem->show();

	set_menu(m_menu);
}

void SeriesLayoutToolButton::on_clicked() {
	m_index++;
	if(m_index > 1) {
		m_index = 0;
	}
	on_change_layout(m_index);
}

void SeriesLayoutToolButton::on_change_layout(int index) {
	m_index = index;
	if(index == 0) {
		set_stock_id(Aeskulap::Stock::SERIES_2X1);
		signal_change_layout(2, 1);
		return;
	}
	if(index == 1) {
		set_stock_id(Aeskulap::Stock::SERIES_2X2);
		signal_change_layout(2, 2);
		return;
	}
}
