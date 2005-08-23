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
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/imagepool/poolseries.cpp,v $
    CVS/RCS Revision: $Revision: 1.1 $
    Status:           $State: Exp $
*/

#include "poolseries.h"
#include "imagepool.h"

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

const std::string& Series::seriesinstanceuid() {
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
}

int Series::instancecount() {
	if(m_instancecount != -1) {
		return m_instancecount;
	}
	
	return m_list.size();
}

}
