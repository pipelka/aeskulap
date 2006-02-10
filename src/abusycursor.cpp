#include "abusycursor.h"
#include "aiconfactory.h"

#include <iostream>

namespace Aeskulap {

static Gtk::Window* mainwindow = NULL;

void set_busy_cursor(bool busy, Gtk::Window* w) {
	if(w == NULL) {
		w = mainwindow;
	}

	if(w == NULL) {
		return;
	}

	if(busy) {
		std::cout << "turning cursor busy" << std::endl;
		w->get_window()->set_cursor(Aeskulap::IconFactory::get_cursor_watch());
	}
	else {
		std::cout << "cursor busy off" << std::endl;
		w->get_window()->set_cursor();
	}
}

void set_mainwindow(Gtk::Window* w) {
	mainwindow = w;
}

} //namespace Aeskulap
