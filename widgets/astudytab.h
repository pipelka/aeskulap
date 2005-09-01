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

#ifndef AESKULAP_STUDYTAB_H
#define AESKULAP_STUDYTAB_H

#include <gtkmm.h>

class StudyView;

namespace ImagePool {
	class Study;
}

namespace Aeskulap {

class StudyTab : public Gtk::HBox {
public:

	StudyTab(const Glib::RefPtr<ImagePool::Study>& study, StudyView* view);

	void on_progress(double p);

	sigc::signal<void, StudyView*> signal_close;

protected:

	Gtk::Tooltips m_tooltips;

	Gtk::ToolButton* m_close;
	
	Gtk::ProgressBar* m_progress;

	Gtk::Label* m_label;
	
	Glib::RefPtr<ImagePool::Study> m_study;
	
	StudyView* m_studyview;
};

} // namespace Aeskulap

#endif  // AESKULAP_STUDYTAB_H
