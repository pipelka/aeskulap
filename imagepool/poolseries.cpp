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
    Update Date:      $Date: 2006/02/28 22:39:34 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/imagepool/poolseries.cpp,v $
    CVS/RCS Revision: $Revision: 1.3.2.1 $
    Status:           $State: Exp $
*/

#include "poolseries.h"
#include "imagepool.h"
#include <cmath>

namespace ImagePool {

Series::Series() :
m_instancecount(-1)
{
}

Series::~Series() {
	for(iterator i = begin(); i != end(); i++) {
		i->second.clear();
	}
	m_list.clear();
}

/*const std::string& Series::seriesinstanceuid() {
	return m_seriesinstanceuid;
}

const std::string& Series::institutionname() {
	return m_institutionname;
}

const std::string& Series::description() {
	return m_description;
}

const std::string& Series::modality() {
	return m_modality;
}

const std::string& Series::seriestime() {
	return m_seriestime;
}

const std::string& Series::stationname() {
	return m_stationname;
}*/

int Series::instancecount() {
	if(m_instancecount != -1) {
		return m_instancecount;
	}
	
	return m_list.size();
}

bool Series::has_3d_information() {
	if(m_list.size() == 0) {
		return false;
	}
	
	return begin()->second->has_3d_information();
}

Glib::RefPtr<ImagePool::Instance> Series::find_nearest_instance(const Instance::Point& p) {
	Instance::Point v;
	Instance::Point r;
	double min = 1000000;
	Glib::RefPtr<ImagePool::Instance> result;
	
	for(iterator i = begin(); i != end(); i++) {
		
		// transform world point p into viewport coordinate v
		if(!i->second->transform_to_viewport(p, v)) {
			continue;
		}
		
		// transform viewport coord. to world
		if(!i->second->transform_to_world(v, r)) {
			continue;
		}

		// get distance p - r
		double d = sqrt(pow(p.x - r.x, 2) + pow(p.y - r.y, 2) + pow(p.z - r.z, 2));
		
		// new minimum =
		if(d < min) {
			min = d;
			result = i->second;
		}
	}

	return result;
}

}
