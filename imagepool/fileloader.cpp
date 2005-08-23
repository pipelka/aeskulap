/*
    Aeskulap ImagePool - DICOM abstraction library
    Copyright (C) 2005  Alexander Pipelka

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    Alexander Pipelka
    pipelka@teleweb.at

    Last Update:      $Author: braindead $
    Update Date:      $Date: 2005/08/23 19:31:54 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/imagepool/fileloader.cpp,v $
    CVS/RCS Revision: $Revision: 1.1 $
    Status:           $State: Exp $
*/

#include <gtkmm.h>

#include "imagepool.h"

namespace ImagePool {

static void image_loader_thread(Glib::SListHandle<Glib::ustring>* list) {
	Glib::SListHandle<Glib::ustring>::iterator i = list->begin();

	for(; i != list->end(); i++) {
		ImagePool::create_instance((*i), true);
	}
	
	delete list;
}

void load_from_file(const Glib::SListHandle<Glib::ustring>& list) {
	Glib::SListHandle<Glib::ustring>* my_list = new Glib::SListHandle<Glib::ustring>(list);

	Glib::Thread *const loader = Glib::Thread::create(
      sigc::bind(sigc::ptr_fun(image_loader_thread), my_list), false);
      
	//loader->join();
}


} // namespace ImagePool
