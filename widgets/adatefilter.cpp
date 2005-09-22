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
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/widgets/adatefilter.cpp,v $
    CVS/RCS Revision: $Revision: 1.1 $
    Status:           $State: Exp $
*/

#include "adatefilter.h"
#include "gettext.h"

namespace Aeskulap {

DateFilter::DateFilter() {

	m_startdate.clear();
	m_enddate.clear();
	set_spacing(12);

	// filtertype button
	m_filter_type = manage(new Gtk::ComboBoxText);
	m_filter_type->append_text(gettext("None"));
	m_filter_type->append_text(gettext("Today"));
	m_filter_type->append_text(gettext("Yesterday"));
	m_filter_type->append_text(gettext("Date"));
	m_filter_type->append_text(gettext("Range"));
	m_filter_type->set_active_text(gettext("None"));
	m_filter_type->signal_changed().connect(sigc::mem_fun(*this, &DateFilter::on_filtertype_changed));
	m_filter_type->show();

	// date selection
	m_filter_date = manage(new Gtk::HBox);

	m_filter_popup_from = manage(new Gtk::Button);
	m_filter_popup_from->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &DateFilter::on_select_date), true));
	m_filter_popup_from->show();

	m_filter_date->pack_start(*m_filter_popup_from, Gtk::PACK_SHRINK);

	// date range
	m_filter_range = manage(new Gtk::HBox);

	m_filter_popup_to = manage(new Gtk::Button);
	m_filter_popup_to->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &DateFilter::on_select_date), false));
	m_filter_popup_to->show();

	Gtk::Label* label = manage(new Gtk::Label("-"));
	label->set_padding(12,0);
	label->show();
	m_filter_range->pack_start(*label, Gtk::PACK_SHRINK);
	m_filter_range->pack_start(*m_filter_popup_to, Gtk::PACK_SHRINK);

	// container for date/range selection
	Gtk::HBox* type_container = manage(new Gtk::HBox);
	type_container->pack_start(*m_filter_date, Gtk::PACK_SHRINK);
	type_container->pack_start(*m_filter_range, Gtk::PACK_SHRINK);
	type_container->show();

	// add all widgets
	pack_start(*m_filter_type, Gtk::PACK_SHRINK);
	pack_start(*type_container, Gtk::PACK_SHRINK);
}

void DateFilter::clear() {
	m_filter_date->hide();
	m_filter_range->hide();
	m_startdate.clear();
	m_enddate.clear();
	m_filter_type->set_active_text(gettext("None"));
}

void DateFilter::on_filtertype_changed() {
	Glib::ustring text = m_filter_type->get_active_text();
	
	if(text == gettext("None")) {
		m_filter_date->hide();
		m_filter_range->hide();
		m_startdate.clear();
		m_enddate.clear();
	}

	if(text == gettext("Today")) {
		m_filter_date->hide();
		m_filter_range->hide();
		set_today();
	}

	if(text == gettext("Yesterday")) {
		m_filter_date->hide();
		m_filter_range->hide();
		set_yesterday();
	}

	if(text == gettext("Date")) {
		m_filter_date->show();
		m_filter_range->hide();
		m_enddate.clear();
		if(m_startdate.empty()) {
			set_today();
		}
	}

	if(text == gettext("Range")) {
		m_filter_date->show();
		m_filter_range->show();
		if(m_startdate.empty()) {
			set_today();
		}
		if(m_enddate.empty()) {
			m_enddate = m_startdate;
		}
	}


	update_labels();
}

bool DateFilter::select_date(const Glib::ustring& title, std::string& isodate) {
	Gtk::Dialog dlg(title, true);
	Gtk::Calendar cal;
	cal.show();

	// select date
	if(!isodate.empty()) {
		guint year = atoi(isodate.substr(0,4).c_str());
		guint month = atoi(isodate.substr(4,2).c_str());
		guint day = atoi(isodate.substr(6,2).c_str());
		cal.select_month(month-1, year);
		cal.select_day(day);
	}

	dlg.get_vbox()->pack_start(cal, Gtk::PACK_SHRINK);
	dlg.add_button(Gtk::Stock::OK , Gtk::RESPONSE_OK);
	dlg.add_button(Gtk::Stock::CANCEL , Gtk::RESPONSE_CANCEL);
	
	dlg.show();

	if(dlg.run() == Gtk::RESPONSE_OK) {
		char date[10];
		guint year;
		guint month;
		guint day;
		cal.get_date(year, month, day);
		sprintf(date, "%04i%02i%02i", year, month+1, day);
		isodate = date;

		return true;
	}
	
	return false;
}

void DateFilter::on_select_date(bool rangestart) {
	if(rangestart) {
		select_date(gettext("Select date"), m_startdate);
		if(m_enddate.empty()) {
			m_enddate = m_startdate;
		}
	}
	else {
		select_date(gettext("Select Enddate"), m_enddate);
		if(m_startdate.empty()) {
			m_startdate = m_enddate;
		}
	}
	
	update_labels();
}

void DateFilter::update_labels() {	
	if(!m_startdate.empty()) {
		std::string year = m_startdate.substr(0,4).c_str();
		std::string month = m_startdate.substr(4,2).c_str();
		std::string day = m_startdate.substr(6,2).c_str();
		m_filter_popup_from->set_label(day+"."+month+"."+year);
	}

	if(!m_enddate.empty()) {
		std::string year = m_enddate.substr(0,4).c_str();
		std::string month = m_enddate.substr(4,2).c_str();
		std::string day = m_enddate.substr(6,2).c_str();
		m_filter_popup_to->set_label(day+"."+month+"."+year);
	}
}

const std::string& DateFilter::get_startdate() {
	return m_startdate;
}

const std::string& DateFilter::get_enddate() {
	if(m_enddate.empty()) {
		return m_startdate;
	}

	return m_enddate;
}

void DateFilter::set_today() {
	struct tm *l_time;
	time_t now;

	time(&now);
	l_time = localtime(&now);

	char date[10];
	sprintf(date, "%04i%02i%02i", l_time->tm_year+1900, l_time->tm_mon+1, l_time->tm_mday);
	
	m_startdate = date;
	m_enddate = date;
}

void DateFilter::set_yesterday() {
	struct tm *l_time;
	time_t now;

	time(&now);
	now += (-1 * 60 * 60 * 24);
	l_time = localtime(&now);

	char date[10];
	sprintf(date, "%04i%02i%02i", l_time->tm_year+1900, l_time->tm_mon+1, l_time->tm_mday);
	
	m_startdate = date;
	m_enddate = date;
}

} // namespace Aeskulap
