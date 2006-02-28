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
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/displaylist/Attic/aobjectlist.h,v $
    CVS/RCS Revision: $Revision: 1.1.2.1 $
    Status:           $State: Exp $
*/

#ifndef AESKULAP_DISPLIST_H
#define AESKULAP_DISPLIST_H

#include "aobject.h"
#include <list>

namespace Aeskulap {

class DisplayObject;

class ObjectList;
typedef Glib::RefPtr<ObjectList> RObjectList;

class ObjectList : public Object {
public:

	typedef std::list<RObject>::iterator iterator;

	void draw(DisplayObject* d);

	void set_selected(bool selected);
	
	double get_distance_screen(DisplayObject* d, double x, double y);

	RObject get_nearest_screen(DisplayObject* d, double x, double y);

	std::list<RObject>::iterator begin();

	std::list<RObject>::iterator end();

	int size();

	static RObjectList create();

protected:

	ObjectList();

	~ObjectList();

	std::list<RObject> m_list;

};

} // namespace Aeskulap

#endif // AESKULAP_DISPLIST_H
