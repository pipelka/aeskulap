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
    Update Date:      $Date: 2005/09/30 16:57:53 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/widgets/atiler.h,v $
    CVS/RCS Revision: $Revision: 1.2 $
    Status:           $State: Exp $
*/

#ifndef AESKULAP_TILER_H
#define AESKULAP_TILER_H

#include <vector>

namespace Aeskulap {

class TilerBase {
public:

	virtual void set_layout(int x, int y);

	void get_layout(int& tilex, int& tiley);
	
	unsigned int max_size();

	void get_xy_from_pos(int n, int& x, int& y);

protected:

	TilerBase(int x, int y);

	virtual ~TilerBase();

	int m_tile_x;

	int m_tile_y;

};

template<class T>
class Tiler : public TilerBase {
public:

	Tiler(int x, int y) : TilerBase(x, y) {
	}

	virtual ~Tiler() {
		for(unsigned int i = 0; i < m_widgets.size(); i++) {
			delete m_widgets[i];
		}
		m_widgets.clear();
	}

	T* operator [](int i) {
		return m_widgets[i];
	}

	bool find_index(T* view, unsigned int& index) {
		for(unsigned int i=0; i<m_widgets.size(); i++) {
			if(m_widgets[i] == view) {
				index = i;
				return true;
			}
		}
		return false;
	}

protected:

	std::vector< T* > m_widgets;
};

} // namespace Aeskulap

#endif // AESKULAP_TILER_H
