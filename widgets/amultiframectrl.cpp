#include "amultiframectrl.h"
#include "adisplay.h"

namespace Aeskulap {

MultiFrameCtrl::MultiFrameCtrl() {
	m_display = NULL;

	m_play = manage(new Gtk::Button(Gtk::Stock::MEDIA_PLAY));
	m_play->show();

	m_position = manage(new Gtk::HScale);
	m_position->signal_change_value().connect(sigc::mem_fun(*this, &MultiFrameCtrl::on_change_value));
	m_position->show();

	pack_start(*m_play, Gtk::PACK_SHRINK);
	pack_start(*m_position);

	m_conn_play = m_play->signal_clicked().connect(sigc::mem_fun(*this, &MultiFrameCtrl::on_play));
}

void MultiFrameCtrl::connect(Display* display) {
	m_conn_next_frame = display->signal_next_frame.connect(sigc::mem_fun(*this, &MultiFrameCtrl::on_next_frame));

	m_display = display;
	m_position->set_range(0, display->get_framecount()-1);
	m_position->set_draw_value(false);
	m_position->set_increments(1.0, 1.0);
}
	
void MultiFrameCtrl::disconnect() {
	//m_conn_play.disconnect();
	//m_conn_change_value.disconnect();
	m_conn_next_frame.disconnect();
	
	m_display = NULL;
}

void MultiFrameCtrl::on_play() {
	if(m_display->get_playing()) {
		on_stop();
		return;
	}

	m_display->play();
}
	
void MultiFrameCtrl::on_stop() {
	if(!m_display) {
		return;
	}

        m_display->stop();
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
