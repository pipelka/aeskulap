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
    Update Date:      $Date: 2005/09/22 06:53:01 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/widgets/imagelayouttoolbutton.cpp,v $
    CVS/RCS Revision: $Revision: 1.2 $
    Status:           $State: Exp $
*/

#include "imagelayouttoolbutton.h"
#include "astockids.h"

ImageLayoutToolButton::ImageLayoutToolButton() {

	set_stock_id(Aeskulap::Stock::GRID_1X1);
	m_index = 0;

	Gtk::MenuItem* menuitem = manage(new Gtk::ImageMenuItem(Aeskulap::Stock::GRID_1X1));
	menuitem->signal_activate().connect(sigc::bind(sigc::mem_fun(*this, &ImageLayoutToolButton::on_change_layout), 0));
	m_menu.add(*menuitem);
	menuitem->show();

	menuitem = manage(new Gtk::ImageMenuItem(Aeskulap::Stock::GRID_1X2));
	menuitem->signal_activate().connect(sigc::bind(sigc::mem_fun(*this, &ImageLayoutToolButton::on_change_layout), 1));
	m_menu.add(*menuitem);
	menuitem->show();

	menuitem = manage(new Gtk::ImageMenuItem(Aeskulap::Stock::GRID_2X1));
	menuitem->signal_activate().connect(sigc::bind(sigc::mem_fun(*this, &ImageLayoutToolButton::on_change_layout), 2));
	m_menu.add(*menuitem);
	menuitem->show();

	menuitem = manage(new Gtk::ImageMenuItem(Aeskulap::Stock::GRID_2X2));
	menuitem->signal_activate().connect(sigc::bind(sigc::mem_fun(*this, &ImageLayoutToolButton::on_change_layout), 3));
	m_menu.add(*menuitem);
	menuitem->show();

	menuitem = manage(new Gtk::ImageMenuItem(Aeskulap::Stock::GRID_4X4));
	menuitem->signal_activate().connect(sigc::bind(sigc::mem_fun(*this, &ImageLayoutToolButton::on_change_layout), 4));
	m_menu.add(*menuitem);
	menuitem->show();

	set_menu(m_menu);
}

void ImageLayoutToolButton::accelerate(Gtk::Window& window) {
	m_menu.accelerate(window);
}

void ImageLayoutToolButton::on_clicked() {
	m_index++;
	if(m_index > 4) {
		m_index = 0;
	}
	on_change_layout(m_index);
}

void ImageLayoutToolButton::on_change_layout(int index) {
	m_index = index;
	if(index == 0) {
		set_layout(1, 1);
		signal_change_layout(1, 1);
		return;
	}
	if(index == 1) {
		set_layout(1, 2);
		signal_change_layout(1, 2);
		return;
	}
	if(index == 2) {
		set_layout(2, 1);
		signal_change_layout(2, 1);
		return;
	}
	if(index == 3) {
		set_layout(2, 2);
		signal_change_layout(2, 2);
		return;
	}
	if(index == 4) {
		set_layout(4, 4);
		signal_change_layout(4, 4);
		return;
	}

}

void ImageLayoutToolButton::set_layout(int tilex, int tiley) {
	if(tilex == 1 && tiley == 1) {
		m_index = 0;
		set_stock_id(Aeskulap::Stock::GRID_1X1);
	}
	if(tilex == 1 && tiley == 2) {
		m_index = 1;
		set_stock_id(Aeskulap::Stock::GRID_1X2);
	}
	if(tilex == 2 && tiley == 1) {
		m_index = 2;
		set_stock_id(Aeskulap::Stock::GRID_2X1);
	}
	if(tilex == 2 && tiley == 2) {
		m_index = 3;
		set_stock_id(Aeskulap::Stock::GRID_2X2);
	}
	if(tilex == 4 && tiley == 4) {
		m_index = 4;
		set_stock_id(Aeskulap::Stock::GRID_4X4);
	}
}
