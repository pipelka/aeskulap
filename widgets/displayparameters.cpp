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
    Update Date:      $Date: 2006/03/06 09:58:02 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/widgets/displayparameters.cpp,v $
    CVS/RCS Revision: $Revision: 1.4 $
    Status:           $State: Exp $
*/

#include "displayparameters.h"

DisplayParameters::DisplayParameters() :
zoom_factor(100.0),
move_x(0),
move_y(0),
selected(false),
series_selected(false) {
}

Glib::RefPtr<DisplayParameters> DisplayParameters::create() {
	return Glib::RefPtr<DisplayParameters>(new DisplayParameters);
}

Glib::RefPtr<DisplayParameters> DisplayParameters::create(const Glib::RefPtr<ImagePool::Instance>& image) {
	Glib::RefPtr<DisplayParameters> d = create();
	
	d->window.center = image->default_windowcenter();
	d->window.width = image->default_windowwidth();
	d->window.modality = image->modality();

	d->default_window = d->window;
	return d;
}

void DisplayParameters::copy(const Glib::RefPtr<DisplayParameters>& a) {
	zoom_factor = a->zoom_factor;
	move_x = a->move_x;
	move_y = a->move_y;
	window = a->window;
}
