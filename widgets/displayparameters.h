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
    Update Date:      $Date: 2006/03/16 13:50:53 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/widgets/displayparameters.h,v $
    CVS/RCS Revision: $Revision: 1.4 $
    Status:           $State: Exp $
*/

#ifndef DISPLAYPARAMETERS_H_
#define DISPLAYPARAMETERS_H_

#include <glibmm/refptr.h>
#include <glibmm/object.h>

#include "poolinstance.h"
#include "awindowlevel.h"

// display parameters

class DisplayParameters : public Glib::Object {
public:

	void copy(const Glib::RefPtr<DisplayParameters>& a);

	static Glib::RefPtr<DisplayParameters> create();

	static Glib::RefPtr<DisplayParameters> create(const Glib::RefPtr<ImagePool::Instance>& image);

	double zoom_factor;

	int move_x;
	int move_y;

	Aeskulap::WindowLevel window;
	Aeskulap::WindowLevel default_window;

	bool selected;

	bool series_selected;

	bool inverted;

protected:

	DisplayParameters();

};


#endif // DISPLAYPARAMETERS_H_
