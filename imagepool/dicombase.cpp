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
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/imagepool/Attic/dicombase.cpp,v $
    CVS/RCS Revision: $Revision: 1.1.2.1 $
    Status:           $State: Exp $
*/

#include "dicombase.h"
#include "imagepool.h"

#include "dcdatset.h"

namespace ImagePool {

DicomBase::DicomBase() {
	m_encoding[0] = "UTF-8";
	m_encoding[1] = "UTF-8";
	m_encoding[2] = "UTF-8";
}

DicomBase::~DicomBase() {
}

bool DicomBase::set_encoding(const std::string& single, const std::string& ideographic) {
	m_encoding[0] = ImagePool::get_system_encoding(single);
	
	if(!ideographic.empty()) {
		m_encoding[1] = ImagePool::get_system_encoding(ideographic);
		m_encoding[2] = ImagePool::get_system_encoding(ideographic);
	}
	else {
		m_encoding[1] = m_encoding[0];
		m_encoding[2] = m_encoding[0];
	}
	
	//std::cout << "single char: " << m_encoding[0] << std::endl;
	//std::cout << "ideographic: " << m_encoding[1] << std::endl;
	
	return true;
}

std::string DicomBase::convert_string(const char* dicom_string) {
	std::string result = "";
	char part[3][500];
	part[0][0] = 0;
	part[1][0] = 0;
	part[2][0] = 0;
	
	const char* p = dicom_string;

	// split string into 3 parts
	int i = 0;
	int c = 0;
	while(*p != 0) {
		if(*p == '=') {
			part[i][c] = 0;
			i++;
			c = 0;
		}
		else {
			part[i][c] = *p;
			c++;
		}
		p++;
	}
	part[i][c] = 0;
	
	for(int i=0; i<3; i++) {
		if(part[i][0] == 0) {
			continue;
		}
		if(i != 0) {
			result += " / ";
		}
		result += ImagePool::convert_string_from(part[i], m_encoding[i]);
	}

	return result;
}

const std::string& DicomBase::tag(const std::string& tag) {
	return property_string[tag];
}

void DicomBase::set_tag(const std::string& tag, const std::string& value) {
	property_string[tag] = value;
}

bool DicomBase::copy_tag(DcmDataset* dset, const DcmTagKey& tag, const std::string& tagname, bool convert) {
	std::string value;
	if(dset->findAndGetOFString(tag, value).good()) {
		if(convert) {
			value = convert_string(value.c_str());
		}
		set_tag(tagname, value);
		return !value.empty();
	}
	
	return false;
}

bool DicomBase::copy_tag(const Glib::RefPtr<ImagePool::DicomBase>& base, const std::string& tagname) {
	std::string value = base->tag(tagname);
	set_tag(tagname, value);
	
	return !value.empty();
}

std::string DicomBase::fix_date(std::string& date) {
	std::string result = date;

	if(date.size() != 8) {
		return result;
	}
	
	std::string year = date.substr(0,4);
	std::string month = date.substr(4, 2);
	std::string day = date.substr(6, 2);
	
	result = day + "." + month + "." + year;
	return result;
}

std::string DicomBase::fix_time(std::string& time) {
	std::string result = time;

	unsigned int i = time.find(".");
	if(i != std::string::npos) {
		result = time.substr(0, i);
	}
	if(result.size() != 6) {
		return result;
	}

	std::string h = result.substr(0,2);
	std::string m = result.substr(2, 2);
	std::string s = result.substr(4, 2);
	
	result = h + ":" + m + ":" + s;
	return result;
}

} // namespace ImagePool
