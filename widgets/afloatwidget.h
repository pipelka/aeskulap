#ifndef AESKULAP_FLOATWIDGET_H
#define AESKULAP_FLOATWIDGET_H

#include <gtkmm.h>
#include <set>

namespace Aeskulap {

class FloatWidget : public Gtk::Window {
public:

	FloatWidget(Gtk::Widget& parent, int width, int height);
	
	virtual ~FloatWidget();

	static void raise_global();

protected:

	void on_realize();

	void on_show();

	void on_hide();
	
	bool on_timeout(int timer);

	sigc::connection m_motion_connection;

	int m_width;

	int m_height;
	
	int last_x;

	int last_y;
	
	Gtk::Widget* m_parent;
	
	Glib::RefPtr<Gdk::Window> m_win;

private:

	static std::set<FloatWidget*> m_widgetlist;

};

} // namespace Aeskulap

#endif // AESKULAP_FLOATWIDGET_H
