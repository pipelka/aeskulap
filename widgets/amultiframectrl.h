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
	
	bool on_change_value(Gtk::ScrollType type, double v);

	void on_next_frame(int frame);

	sigc::connection m_conn_play;

	sigc::connection m_conn_change_value;

	sigc::connection m_conn_next_frame;

	Display* m_display;

private:

	Gtk::Button* m_play;
	
	Gtk::HScale* m_position;
};

} // namespace Aeskulap

#endif // AESKULAP_MULTIFRAMECTRL_H
