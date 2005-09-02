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
    Update Date:      $Date: 2005/09/02 10:13:12 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/widgets/Attic/instanceview.h,v $
    CVS/RCS Revision: $Revision: 1.1 $
    Status:           $State: Exp $
*/

#ifndef AESKULAP_INSTANCEVIEW_H
#define AESKULAP_INSTANCEVIEW_H

#include <gtkmm.h>

class SeriesView;

namespace Aeskulap {
	class Display;
}

class InstanceView : public Gtk::VBox {
public:

	typedef enum Type {
		SINGLE,
		MULTIFRAME,
		MPEG
	};

protected:

	InstanceView(Type type);
	
public:

	Aeskulap::Display* get_display();

	static InstanceView* create(Type type, SeriesView* seriesview);

protected:

	Type m_type;

	Aeskulap::Display* m_display;
};

#endif // AESKULAP_INSTANCEVIEW_H
