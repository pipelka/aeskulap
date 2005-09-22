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
    Update Date:      $Date: 2005/09/22 15:40:46 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/widgets/adatefilter.h,v $
    CVS/RCS Revision: $Revision: 1.1 $
    Status:           $State: Exp $
*/

#ifndef AESKULAP_DATEFILTER_H
#define AESKULAP_DATEFILTER_H

#include <gtkmm.h>
#include <string>

namespace Aeskulap {

class DateFilter : public Gtk::HBox {
public:

	DateFilter();

	void clear();

	const std::string& get_startdate();

	const std::string& get_enddate();

protected:

	void set_today();

	void set_yesterday();

	bool select_date(const Glib::ustring& title, std::string& isodate);

	void update_labels();

	void on_select_date(bool rangestart);

	void on_filtertype_changed();

	// filter widgets

	Gtk::ComboBoxText* m_filter_type;
	
	Gtk::Button* m_filter_popup_from;

	Gtk::Button* m_filter_popup_to;
	
	Gtk::HBox* m_filter_date;
	
	Gtk::HBox* m_filter_range;

	std::string m_startdate;

	std::string m_enddate;
};

} // namespace Aeskulap

#endif // AESKULAP_DATEFILTER_H
