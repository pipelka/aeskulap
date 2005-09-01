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
    Update Date:      $Date: 2005/09/01 06:49:44 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/imagepool/loader.cpp,v $
    CVS/RCS Revision: $Revision: 1.3 $
    Status:           $State: Exp $
*/

#include "loader.h"
#include "imagepool.h"
#include "dcdatset.h"

namespace ImagePool {
	
Loader::Loader() :
m_loader(NULL)
{
	m_add_image.connect(sigc::mem_fun(*this, &Loader::add_image_callback));
	m_finished.connect(sigc::mem_fun(*this, &Loader::finished));
}

Loader::~Loader() {
}

void Loader::start() {
	m_loader = Glib::Thread::create(sigc::mem_fun(*this, &Loader::thread), false);
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
	Glib::RefPtr<ImagePool::Study> new_study = get_study(r->m_studyinstanceuid);
	std::cout << "studysize: " << new_study->size() << std::endl;
	if(new_study->size() == 0) {
		new_study->m_studyinstanceuid = r->studyinstanceuid();
		new_study->m_patientsname = r->m_patientsname;
		new_study->m_patientsbirthdate = r->m_patientsbirthdate;
		new_study->m_patientssex = r->m_patientssex;
		new_study->m_studydescription = r->m_studydescription;
		
		std::cout << "emit: signal_study_added(" << r->m_studyinstanceuid << ")" << std::endl;
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

void Loader::add_image(DcmDataset* dset, int imagecount) {
	Glib::RefPtr<ImagePool::Instance> image = ImagePool::create_instance(dset);

	if(!image) {
		return;
	}

	image->study()->set_instancecount(image->study()->get_instancecount()+1, imagecount);
	data().loaded_study[image->study()] = true;

	m_imagequeue.push(image);
	m_add_image();
}

void Loader::run() {
}

void Loader::finished() {
	m_current_study->signal_progress(100);
	m_current_study.clear();
	m_mutex.unlock();
}

void Loader::thread() {
	m_mutex.lock();
	run();

	while(m_imagequeue.size() > 0) {
		Glib::usleep(1000);
	}

	std::map < Glib::RefPtr<ImagePool::Study>, bool >::iterator i = data().loaded_study.begin();
	while(i != data().loaded_study.end()) {
		if(i->second) {
			m_current_study = i->first;
			i->second = false;
			m_mutex.lock();
			m_finished();
		}
		i++;
	}
	m_data.erase(Glib::Thread::self());
}

Loader::Data& Loader::data() {
	return m_data[Glib::Thread::self()];
}

} // namespace ImagePool
