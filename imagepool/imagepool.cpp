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
    Update Date:      $Date: 2007/04/24 09:53:37 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/imagepool/imagepool.cpp,v $
    CVS/RCS Revision: $Revision: 1.17 $
    Status:           $State: Exp $
*/

#include <stdlib.h>
#include "imagepool.h"

#include "dcmtk/dcmdata/dcdatset.h"
#include "dcmtk/dcmimgle/dcmimage.h"
#include "dcmtk/dcmimage/diregist.h"
#include "dcmtk/dcmdata/dcdeftag.h"

#include "dcmtk/dcmjpeg/djdecode.h"
#include "dcmtk/dcmjpeg/djencode.h"
#include "dcmtk/dcmdata/dcrledrg.h"
#include "dcmtk/dcmdata/dcrleerg.h"

#include "poolnetwork.h"
#include "aconfiguration.h"

#include <locale.h>
#include <map>
#include <stdlib.h>
#include <stdio.h>
#include <glibmm.h> 

#include <queue>

namespace ImagePool {

Network net;

std::map< std::string, Glib::RefPtr<ImagePool::Instance> > m_pool;

static std::map< std::string, Glib::RefPtr<ImagePool::Series> > m_seriespool;

static std::map< std::string, Glib::RefPtr<ImagePool::Study> > m_studypool;

static std::string m_encoding;

Aeskulap::Configuration& m_configuration = Aeskulap::Configuration::get_instance();

void init() {
	DJEncoderRegistration::registerCodecs();
	DJDecoderRegistration::registerCodecs();

	DcmRLEEncoderRegistration::registerCodecs();
	DcmRLEDecoderRegistration::registerCodecs();
	
	net.InitializeNetwork(
			10,
			m_configuration.get_local_port()
			);
}
	
void close() {
	DJEncoderRegistration::cleanup();
	DJDecoderRegistration::cleanup();

	DcmRLEEncoderRegistration::cleanup();
	DcmRLEDecoderRegistration::cleanup();
	
	net.DropNetwork();
}

bool register_instance(const Glib::RefPtr<ImagePool::Instance>& image) {
	std::string sop = image->sopinstanceuid();

	if(sop.empty()) {
		std::cout << "no SOPInstanceUID in instance !!!" << std::endl;
		return false;
	}

	// check if this sop is already in the pool
	if(m_pool[sop]) {
		std::cout << "replacing existing object !!!" << std::endl;
	}

	m_pool[sop] = image;
	
	return true;
}

void remove_instance(const std::string& sopinstanceuid) {
	std::cout << "removing instance " << sopinstanceuid << std::endl;
	m_pool[sopinstanceuid]->clear();
	m_pool[sopinstanceuid].clear();
	m_pool.erase(sopinstanceuid);
}

void remove_instance(const Glib::RefPtr<ImagePool::Instance>& image) {
	ImagePool::remove_instance(image->m_sopinstanceuid);
}

void remove_series(const Glib::RefPtr<ImagePool::Series>& series) {
	std::cout << "removing series " << series->seriesinstanceuid() << std::endl;
	ImagePool::Series::iterator i;
	
	for(i = series->begin(); i != series->end(); i++) {
		remove_instance((*i).second);
	}
	
	m_seriespool[series->seriesinstanceuid()].clear();
	m_seriespool.erase(series->seriesinstanceuid());
}

void remove_study(const Glib::RefPtr<ImagePool::Study>& study) {
	std::cout << "removing study " << study->studyinstanceuid() << std::endl;
	ImagePool::Study::iterator i;
	for(i = study->begin(); i != study->end(); i++) {
		remove_series((*i).second);
	}
	
	m_studypool[study->studyinstanceuid()].clear();
	m_studypool.erase(study->studyinstanceuid());
}

const Glib::RefPtr<ImagePool::Instance>& get_instance(const std::string& sopinstanceuid) {
	return m_pool[sopinstanceuid];
}

const Glib::RefPtr<ImagePool::Series>& get_series(const std::string& seriesinstanceuid) {
	if(!m_seriespool[seriesinstanceuid]) {
		m_seriespool[seriesinstanceuid] = Glib::RefPtr<ImagePool::Series>(new ImagePool::Series);
	}
	return m_seriespool[seriesinstanceuid];
}

const Glib::RefPtr<ImagePool::Study>& get_study(const std::string& studyinstanceuid) {
	if(!m_studypool[studyinstanceuid]) {
		m_studypool[studyinstanceuid] = Glib::RefPtr<ImagePool::Study>(new ImagePool::Study);
	}
	return m_studypool[studyinstanceuid];
}

}
std::string ImagePool::convert_string_from(const char* dicom_string, const std::string& system_encoding) {
	try {
		return Glib::convert(dicom_string, "UTF-8", system_encoding);
	}
	catch(...) {
		std::cerr << "Unable to convert string from the '" << system_encoding << "' encoding." << std::endl;
		return "";
	}
}

std::string ImagePool::convert_string_to(const char* dicom_string, const std::string& system_encoding) {
	try {
		return Glib::convert(dicom_string, system_encoding, "UTF-8");
	}
	catch(...) {
		std::cerr << "Unable to convert string to the '" << system_encoding << "' encoding." << std::endl;
		return "";
	}
}

void ImagePool::set_encoding(const std::string& dicom_encoding) {
	m_encoding = dicom_encoding;
}

std::string ImagePool::get_encoding() {
	return m_encoding;
}

std::string ImagePool::get_system_encoding(const std::string& dicom_iso) {
	if (dicom_iso == "")
		return "UTF-8";
	if (dicom_iso == "ISO_IR 6")
		return "UTF-8";
	else if (dicom_iso == "ISO_IR 100")
		return "ISO-8859-1";
	else if (dicom_iso == "ISO_IR 101")
		return "ISO-8859-2";
	else if (dicom_iso == "ISO_IR 109")
		return "ISO-8859-3";
	else if (dicom_iso == "ISO_IR 110")
		return "ISO-8859-4";
	else if (dicom_iso == "ISO_IR 144")
		return "ISO-8859-5";
	else if (dicom_iso == "ISO_IR 127")
		return "ISO-8859-6";
	else if (dicom_iso == "ISO_IR 126")
		return "ISO-8859-7";
	else if (dicom_iso == "ISO_IR 138")
		return "ISO-8859-8";
	else if (dicom_iso == "ISO_IR 148")
		return "ISO-8859-9";
	else if (dicom_iso == "ISO_IR 192")
		return "UTF-8";
	else if (dicom_iso == "GB18030")
		return "GB18030";
	else if (dicom_iso == "ISO 2022 IR 87")
		return "ISO-2022-JP";
	else if (dicom_iso == "ISO 2022 IR 149")
		return "EUC-KR";

	std::cerr << "Unhandled encoding '" << dicom_iso << "'." << std::endl;
	std::cerr << "falling back to 'ISO_IR 192'." << std::endl;
	std::cerr << "Please post the unhandled ISO encoding to the Aeskulap mailing list!" << std::endl;
	return "UTF-8";
}
