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
    Update Date:      $Date: 2006/02/28 22:39:34 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/displaylist/Attic/aobjectlist.cpp,v $
    CVS/RCS Revision: $Revision: 1.1.2.1 $
    Status:           $State: Exp $
*/

#include "aobjectlist.h"

namespace Aeskulap {

RObjectList ObjectList::create() {
	return RObjectList(new ObjectList);
}


ObjectList::ObjectList() {
}

ObjectList::~ObjectList() {
}

std::list<RObject>::iterator ObjectList::begin() {
	return m_list.begin();
}

std::list<RObject>::iterator ObjectList::end() {
	return m_list.end();
}

int ObjectList::size() {
	return m_list.size();
}

void ObjectList::draw(DisplayObject* d) {
	for(iterator i = begin(); i != end(); i++) {
		(*i)->draw(d);
	}
}

void ObjectList::set_selected(bool selected) {
	Object::set_selected(selected);

	if(size() == 0) {
		return;
	}

	for(iterator i = begin(); i != end(); i++) {
		(*i)->set_selected(selected);
	}
}

double ObjectList::get_distance_screen(DisplayObject* d, double x, double y) {
	RObject obj = get_nearest_screen(d, x, y);

	if(!obj) {
		return 1e+10;
	}

	return obj->get_distance_screen(d, x, y);
}

RObject ObjectList::get_nearest_screen(DisplayObject* d, double x, double y) {
	double min = 1e+10;
	RObject obj;

	for(iterator i = begin(); i != end(); i++) {
		double dist = (*i)->get_distance_screen(d, x, y);
		if(dist < min) {
			min = dist;
			obj = (*i);
		}
	}

	return obj;
}

} // namespace Aeskulap
