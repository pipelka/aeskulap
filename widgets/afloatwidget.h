#ifndef AESKULAP_FLOATWIDGET_H
#define AESKULAP_FLOATWIDGET_H

#include <gtkmm.h>

namespace Aeskulap {

class FloatWidget : public Gtk::Window {
public:

	FloatWidget(Gtk::Widget& parent, int width, int height);
	
	virtual ~FloatWidget();

protected:

	void on_realize();

	bool on_timeout(int timer);

	sigc::connection m_motion_connection;

	int m_width;

	int m_height;
	
	int last_x;

	int last_y;
	
	Gtk::Widget* m_parent;
	
	Glib::RefPtr<Gdk::Window> m_win;
};

} // namespace Aeskulap

#endif // AESKULAP_FLOATWIDGET_H
