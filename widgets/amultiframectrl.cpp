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
    Update Date:      $Date: 2005/10/03 10:05:29 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/widgets/amultiframectrl.cpp,v $
    CVS/RCS Revision: $Revision: 1.4 $
    Status:           $State: Exp $
*/

#include "amultiframectrl.h"
#include "adisplay.h"

namespace Aeskulap {

MultiFrameCtrl::MultiFrameCtrl() {
	m_display = NULL;

	m_play = manage(new Gtk::Button(Gtk::Stock::MEDIA_PLAY));
	m_play->set_focus_on_click(false);
	m_play->set_use_stock();

	m_position = manage(new Gtk::HScale);
	m_position->set_draw_value(false);
	m_position->set_increments(1.0, 1.0);
	m_position->signal_change_value().connect(sigc::mem_fun(*this, &MultiFrameCtrl::on_change_value));

	pack_start(*m_play, Gtk::PACK_SHRINK);
	pack_start(*m_position);

	m_play->show();
	m_position->show();

	m_conn_play = m_play->signal_clicked().connect(sigc::mem_fun(*this, &MultiFrameCtrl::on_play));
}

void MultiFrameCtrl::connect(Display* display) {
	if(display == NULL) {
		return;
	}

	if(!display->get_image()) {
		return;
	}
	m_conn_next_frame = display->signal_next_frame.connect(sigc::mem_fun(*this, &MultiFrameCtrl::on_next_frame));
	m_conn_signal_stop = display->signal_stop.connect(sigc::mem_fun(*this, &MultiFrameCtrl::on_signal_stop));

	m_display = display;
	m_position->set_range(0, display->get_framecount()-1);
	m_position->set_value(display->get_current_frame());
}
	
void MultiFrameCtrl::disconnect() {

	m_conn_signal_stop.disconnect();
	m_conn_next_frame.disconnect();
	
	m_display = NULL;
}

void MultiFrameCtrl::on_play() {
	if(m_display == NULL) {
		return;
	}

	if(m_display->get_playing()) {
		on_stop();
		return;
	}

	m_play->set_label(Gtk::Stock::MEDIA_STOP.id);
	m_display->play();
}
	
void MultiFrameCtrl::on_stop() {
	if(!m_display) {
		return;
	}

        m_display->stop();
}

void MultiFrameCtrl::on_signal_stop() {
	m_play->set_label(Gtk::Stock::MEDIA_PLAY.id);
}

bool MultiFrameCtrl::on_change_value(Gtk::ScrollType type, double v) {
	if(m_display != NULL) {
		m_display->set_current_frame((int)v);
	}

	return true;
}

void MultiFrameCtrl::on_next_frame(int frame) {
	m_position->set_value(frame);
}

} // namespace Aeskulap
