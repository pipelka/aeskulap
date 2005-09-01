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

    Last Update:      $Author$
    Update Date:      $Date$
    Source File:      $Source$
    CVS/RCS Revision: $Revision$
    Status:           $State$
*/

#include "astudytab.h"
#include "poolstudy.h"
#include "studyview.h"
#include "gettext.h"

namespace Aeskulap {

StudyTab::StudyTab(const Glib::RefPtr<ImagePool::Study>& study, StudyView* view) {

	set_size_request(-1, 35);

	m_study = study;
	m_studyview = view;
	
	std::string labeltext = m_study->patientsname().substr(0,20);

	m_label = manage(new Gtk::Label(labeltext, Gtk::ALIGN_LEFT));
	m_label->set_padding(2,0);
	m_label->show();

	m_progress = manage(new Gtk::ProgressBar);
	m_progress->set_size_request(-1, 14);
	m_progress->show();

	Gtk::Image* image_left = manage(new Gtk::Image(Gtk::Stock::DND_MULTIPLE, Gtk::ICON_SIZE_LARGE_TOOLBAR));
	image_left->set_padding(2,0);
	image_left->show();

	Gtk::Image* image_close = manage(new Gtk::Image(Gtk::Stock::CLOSE, Gtk::ICON_SIZE_MENU));
	image_close->show();

	m_close = manage(new Gtk::ToolButton(*image_close));
	m_close->set_tooltip(m_tooltips, gettext("Close study"));
	m_close->set_size_request(22, 22);
	m_close->signal_clicked().connect(sigc::bind(signal_close, view));
	m_close->set_sensitive(false);
	m_close->show();

	Gtk::VBox* vbox = manage(new Gtk::VBox);
	vbox->show();

	vbox->pack_start(*m_label);
	vbox->pack_start(*m_progress, Gtk::PACK_SHRINK);

	pack_start(*image_left);
	pack_start(*vbox);
	pack_start(*m_close);
}

void StudyTab::on_progress(double p) {
	m_progress->set_fraction(p);
	if(p >= 100.0) {
		m_close->set_sensitive(true);
		m_progress->hide();
		std::string labeltext = m_study->patientsname().substr(0,20) + "\n" + m_study->studydescription().substr(0,20);
		m_label->set_text(labeltext);
	}
}

} // namespace Aeskulap
