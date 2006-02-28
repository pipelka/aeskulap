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
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/imagepool/loader.cpp,v $
    CVS/RCS Revision: $Revision: 1.12.2.1 $
    Status:           $State: Exp $
*/

#include "imagepool.h"
#include "poolinstance.h"
#include "poolseries.h"
#include "poolstudy.h"

#include "loader.h"
#include "dcdatset.h"
#include <gtkmm.h>

namespace ImagePool {
	
Loader::Loader() :
m_loader(NULL),
m_busy(false),
m_finished(false)
{
	//m_add_image.connect(sigc::mem_fun(*this, &Loader::add_image_callback));
	//m_finished.connect(sigc::mem_fun(*this, &Loader::finished));
}

Loader::~Loader() {
}

bool Loader::start() {
	if(m_busy) {
		return false;
	}

	m_finished = false;
	m_conn_timer = Glib::signal_timeout().connect(sigc::mem_fun(*this, &Loader::on_timeout), 500);
	m_loader = Glib::Thread::create(sigc::mem_fun(*this, &Loader::thread), false);
	
	return true;
}

bool Loader::on_timeout() {
	process_instance();
	
	if(m_finished) {
		finished();
		return false;
	}
	
	return true;
}

void Loader::stop() {
}

void Loader::process_instance() {
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
	Glib::RefPtr<ImagePool::Series> new_series = get_series(r->tag("SeriesInstanceUID"));
	bool bEmit = (new_series->size() == 0);
	if(new_series->size() == 0) {
		new_series->copy_tag(r, "StudyInstanceUID");
		new_series->copy_tag(r, "InstitutionName");
		new_series->copy_tag(r, "SeriesDescription");
		new_series->copy_tag(r, "Modality");
		/*if(new_series->m_seriestime.empty()) {
			new_series->m_seriestime = r->m_time;
		}*/
	}

	new_study->at(r->tag("SeriesInstanceUID")) = new_series;
	new_series->copy_tag(r, "SeriesInstanceUID");

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
	new_series->at(r->tag("SOPInstanceUID")) = r;
	new_series->signal_instance_added(r);
	new_study->emit_progress();

	if(m_imagequeue.size() > 0) {
		process_instance();
	}
}

void Loader::add_image(DcmDataset* dset) {
	//dset->print(COUT, DCMTypes::PF_shortenLongTagValues);

	Glib::RefPtr<ImagePool::Instance> image = ImagePool::Instance::create(dset);

	if(!image) {
		return;
	}

	register_instance(image);

	std::string studyinstanceuid = image->tag("StudyInstanceUID");

	int imagecount = m_cache[studyinstanceuid].m_instancecount;
	int seriescount = m_cache[studyinstanceuid].m_seriescount;
	int count = image->study()->get_instancecount()+1;

	image->study()->set_instancecount(count, imagecount);
	image->study()->set_seriescount(seriescount);
	
	// add to cache
	m_cache[studyinstanceuid].m_study = image->study();

	m_imagequeue.push(image);
}

bool Loader::run() {
	return false;
}

void Loader::finished() {
	std::cout << "wait for imagequeue ";
	while(m_imagequeue.size() > 0) {
		std::cout << ".";
		process_instance();
	}
	std::cout << std::endl;


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

	std::cout << "finished" << std::endl;
	m_finished = true;

	// wait for finished() (clears m_cache)
	std::cout << "wait for cache ";
	while(m_cache.size() > 0) {
		std::cout << ".";
		Glib::usleep(1000*100);
	}
	std::cout << std::endl;

	m_mutex.lock();
	m_conn_timer.disconnect();
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
