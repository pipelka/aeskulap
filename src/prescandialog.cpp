#include "prescandialog.h"
#include <iostream>

PrescanDialog::PrescanDialog(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade) :
Gtk::Window(cobject){

	refGlade->get_widget("progress_prescan", m_progress);
	set_progress(0);
}

void PrescanDialog::set_progress(double progress) {
	std::cout << "prescan: " << progress << std::endl;
	m_progress->set_fraction(progress);
	while(Gtk::Main::events_pending()) Gtk::Main::iteration(false);
}
