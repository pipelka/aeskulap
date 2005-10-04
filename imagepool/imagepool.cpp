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
    Update Date:      $Date: 2005/10/04 06:45:52 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/imagepool/imagepool.cpp,v $
    CVS/RCS Revision: $Revision: 1.13 $
    Status:           $State: Exp $
*/

#include <stdlib.h>
#include "imagepool.h"

#include "dcdatset.h"
#include "dcmimage.h"
#include "diregist.h"
#include "dcdeftag.h"

#include "djdecode.h"
#include "djencode.h"
#include "dcrledrg.h"
#include "dcrleerg.h"

#include "poolnetwork.h"

#include <locale.h>
#include <map>
#include <stdlib.h>
#include <stdio.h>
#include <glibmm.h> 
#include <gconfmm.h>

#include <queue>

namespace ImagePool {

Network net;

std::map< std::string, Glib::RefPtr<ImagePool::Instance> > m_pool;

static std::map< std::string, Glib::RefPtr<ImagePool::Series> > m_seriespool;

static std::map< std::string, Glib::RefPtr<ImagePool::Study> > m_studypool;


void init(bool connect) {

	DJEncoderRegistration::registerCodecs();
	DJDecoderRegistration::registerCodecs();

	DcmRLEEncoderRegistration::registerCodecs();
	DcmRLEDecoderRegistration::registerCodecs();
	
	if(connect) {
		Gnome::Conf::init();
		Glib::thread_init();
	}

	Glib::RefPtr<Gnome::Conf::Client> client = Gnome::Conf::Client::get_default_client();

	net.InitializeNetwork(
			0,
			client->get_int("/apps/aeskulap/preferences/local_port")
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
