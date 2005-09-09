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
    Update Date:      $Date: 2005/09/09 16:12:33 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/widgets/amultiframectrl.h,v $
    CVS/RCS Revision: $Revision: 1.3 $
    Status:           $State: Exp $
*/

#ifndef AESKULAP_MULTIFRAMECTRL_H
#define AESKULAP_MULTIFRAMECTRL_H

#include <gtkmm.h>
#include "aframectrl.h"

namespace Aeskulap {

class Display;

class MultiFrameCtrl : public Gtk::HBox, public FrameCtrl {
public:

	MultiFrameCtrl();

	void connect(Display* display);
	
	void disconnect();

protected:

	void on_play();
	
	void on_stop();
	
	void on_signal_stop();

	bool on_change_value(Gtk::ScrollType type, double v);

	void on_next_frame(int frame);

	sigc::connection m_conn_play;

	sigc::connection m_conn_change_value;

	sigc::connection m_conn_next_frame;

	sigc::connection m_conn_signal_stop;

	Display* m_display;

private:

	Gtk::Button* m_play;
	
	Gtk::HScale* m_position;
};

} // namespace Aeskulap

#endif // AESKULAP_MULTIFRAMECTRL_H
