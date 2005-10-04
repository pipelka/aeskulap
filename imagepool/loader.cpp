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
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/imagepool/loader.cpp,v $
    CVS/RCS Revision: $Revision: 1.11 $
    Status:           $State: Exp $
*/

#include "imagepool.h"
#include "loader.h"
#include "dcdatset.h"

namespace ImagePool {
	
Loader::Loader() :
m_loader(NULL),
m_busy(false)
{
	m_add_image.connect(sigc::mem_fun(*this, &Loader::add_image_callback));
	m_finished.connect(sigc::mem_fun(*this, &Loader::finished));
}

Loader::~Loader() {
}

bool Loader::start() {
	if(m_busy) {
		return false;
	}
	m_loader = Glib::Thread::create(sigc::mem_fun(*this, &Loader::thread), false);
	
	return true;
}

void Loader::stop() {
}

void Loader::add_image_callback() {
	if(m_imagequeue.size() == 0) {
		return;
	}

	Glib::RefPtr<ImagePool::Instance> r = m_imagequeue.front();
	m_imagequeue.pop();

	OFString value;

	// register study
	Glib::RefPtr<ImagePool::Study> new_study = r->study();
	if(new_study->size() == 0) {
		signal_study_added(new_study);
	}

	// register series
	Glib::RefPtr<ImagePool::Series> new_series = get_series(r->m_seriesinstanceuid);
	bool bEmit = (new_series->size() == 0);
	if(new_series->size() == 0) {
		new_series->m_studyinstanceuid = r->m_studyinstanceuid;
		new_series->m_institutionname = r->m_institutionname;
		new_series->m_description = r->m_seriesdescription;
		new_series->m_modality = r->m_modality;
		if(new_series->m_seriestime.empty()) {
			new_series->m_seriestime = r->m_time;
		}
	}

	new_study->at(r->m_seriesinstanceuid) = new_series;
	new_series->m_seriesinstanceuid = r->m_seriesinstanceuid;

	if(bEmit) {
		new_study->signal_series_added(new_series);
	}
	
	r->m_study = new_study;
	r->m_series = new_series;

	// check instancenumber
	if(r->m_instancenumber == 0) {
		r->m_instancenumber = new_series->size() + 1;
	}

	// register instance
	new_series->at(r->m_sopinstanceuid) = r;
	new_series->signal_instance_added(r);
	new_study->emit_progress();

	if(m_imagequeue.size() > 0) {
		add_image_callback();
	}
}

void Loader::add_image(DcmDataset* dset) {
	Glib::RefPtr<ImagePool::Instance> image = ImagePool::Instance::create(dset);

	if(!image) {
		return;
	}

	register_instance(image);

	std::string studyinstanceuid = image->studyinstanceuid();

	int imagecount = m_cache[studyinstanceuid].m_instancecount;
	int seriescount = m_cache[studyinstanceuid].m_seriescount;
	int count = image->study()->get_instancecount()+1;

	image->study()->set_instancecount(count, imagecount);
	image->study()->set_seriescount(seriescount);
	
	// add to cache
	m_cache[studyinstanceuid].m_study = image->study();

	m_imagequeue.push(image);
	m_add_image();
}

bool Loader::run() {
	return false;
}

void Loader::finished() {
	std::map<std::string, CacheEntry>::iterator i = m_cache.begin();
	while(i != m_cache.end()) {
		if(i->second.m_study) {
			i->second.m_study->signal_progress(1);
		}
		i++;
	}

	m_cache.clear();
}

void Loader::thread() {
	m_mutex.lock();
	m_busy = true;
	m_mutex.unlock();

	bool rc = run();

	std::cout << "wait for imagequeue ";
	while(m_imagequeue.size() > 0) {
		std::cout << ".";
		Glib::usleep(1000*100);
	}
	std::cout << std::endl;

	std::cout << "finished" << std::endl;
	m_finished();

	// wait for finished() (clears m_cache)
	std::cout << "wait for cache ";
	while(m_cache.size() > 0) {
		std::cout << ".";
		Glib::usleep(1000*100);
	}
	std::cout << std::endl;

	m_mutex.lock();
	m_busy = false;
	m_mutex.unlock();

	// throw error
	if(!rc) {
		std::cout << "signal_error()" << std::endl;
		signal_error();
	}

	std::cout << "thread finished" << std::endl;
}

bool Loader::busy() {
	m_mutex.lock();
	bool rc = m_busy;
	m_mutex.unlock();
	
	return rc;
}

} // namespace ImagePool
