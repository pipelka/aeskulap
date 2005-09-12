#ifndef AESKULAP_PRESCANDIALOG_H
#define AESKULAP_PRESCANDIALOG_H

#include <gtkmm.h>
#include <libglademm/xml.h>

class PrescanDialog : public Gtk::Window {
public:

	PrescanDialog(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);

	void set_progress(double progress);

protected:

	Gtk::ProgressBar* m_progress;
};

#endif // AESKULAP_PRESCANDIALOG_H
