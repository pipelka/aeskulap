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
    Update Date:      $Date: 2006/03/09 15:35:14 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/widgets/awindowleveltoolbutton.h,v $
    CVS/RCS Revision: $Revision: 1.3.4.1 $
    Status:           $State: Exp $
*/

#ifndef AESKULAP_WINDOWLEVELTOOLBUTTON_H
#define AESKULAP_WINDOWLEVELTOOLBUTTON_H

#include <gtkmm.h>
#include <set>
#include "aconfigclient.h"

namespace Aeskulap {

class WindowLevelToolButton : public Gtk::ToolItem, public Aeskulap::ConfigClient {
public:

	WindowLevelToolButton();

	~WindowLevelToolButton();

	void set_modality(Glib::ustring modality);

	const Glib::ustring& get_modality();
	
	void set_windowlevel(const Aeskulap::WindowLevel& level, bool force = false);

	void set_windowlevel_default();

	void update();

	sigc::signal< void > signal_windowlevel_default;

	sigc::signal< void, WindowLevel > signal_windowlevel_changed;

	sigc::signal< void, WindowLevelToolButton* > signal_windowlevel_add;

	static void update_all();
	
protected:

	Aeskulap::WindowLevelList::iterator find_windowlevel(const Glib::ustring& desc);

	virtual void on_changed();

	virtual void on_add();

	Aeskulap::WindowLevel m_last_level;

	Aeskulap::WindowLevelList m_list;

	Gtk::ComboBoxText* m_combo;
	
	Glib::ustring m_modality;

	Gtk::Tooltips m_tooltips;

private:

	static std::set<WindowLevelToolButton*> m_widgetlist;
};

} // namespace Aeskulap

#endif // AESKULAP_WINDOWLEVELTOOLBUTTON_H
