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
    Update Date:      $Date: 2005/08/25 12:09:17 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/imagepool/imagepool.cpp,v $
    CVS/RCS Revision: $Revision: 1.3 $
    Status:           $State: Exp $
*/

#include <stdlib.h>
#include "imagepool.h"

#include "dcdatset.h"
#include "dcmimage.h"
#include "diregist.h"
#include "dcfilefo.h"
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

void context_function();

Network net;

static Glib::Dispatcher context_switch;

static Glib::StaticMutex context_mutex = GLIBMM_STATIC_MUTEX_INIT; 

static std::queue< Glib::RefPtr<ImagePool::Instance> > context_image;


sigc::signal< void, Glib::RefPtr<ImagePool::Study> > Signals::signal_study_added;

sigc::signal< void, Glib::RefPtr<ImagePool::Study> > Signals::signal_study_removed;

static std::map< std::string, Glib::RefPtr<ImagePool::Instance> > m_pool;

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
		context_switch.connect(sigc::ptr_fun(context_function));
	}

	Glib::RefPtr<Gnome::Conf::Client> client = Gnome::Conf::Client::get_default_client();

	net.InitializeNetwork(
			0,
			client->get_int("/apps/aeskulap/preferences/local_port")
			);
}
	
void close() {
	/*m_studypool.clear();
	m_seriespool.clear();
	m_pool.clear();*/
	
	DJEncoderRegistration::cleanup();
	DJDecoderRegistration::cleanup();

	DcmRLEEncoderRegistration::cleanup();
	DcmRLEDecoderRegistration::cleanup();
	
	net.DropNetwork();
}

Glib::RefPtr<ImagePool::Instance> create_instance(const std::string& filename, bool threaded) {
	DcmFileFormat dfile;

	OFCondition cond = dfile.loadFile(filename.c_str(), EXS_Unknown, EGL_noChange, DCM_MaxReadLength, false);
	
	if(!cond.good()) {
		std::cout << "unable to open file !!!" << std::endl;
		return Glib::RefPtr<ImagePool::Instance>();
	}
	dfile.loadAllDataIntoMemory();
	std::cout << "opened file:" << filename << std::endl;

	DcmDataset* dset = dfile.getAndRemoveDataset();
	
	Glib::RefPtr<ImagePool::Instance> r = ImagePool::create_instance(dset, threaded);
	delete dset;
	
	return r;
}

Glib::RefPtr<ImagePool::Instance> create_instance(DcmDataset* dset, bool threaded) {
	context_mutex.lock();

	std::string sop;
	if(dset->findAndGetOFString(DCM_SOPInstanceUID, sop).bad()) {
		std::cout << "no SOPInstanceUID in instance !!!" << std::endl;
		return Glib::RefPtr<ImagePool::Instance>();
	}
	
	// check if this sop is already in the pool
	if(m_pool[sop]) {
		std::cout << "returning object from pool !!!" << std::endl;
		context_mutex.unlock();
		return m_pool[sop];
	}

	// wrap in smartpointer
	Glib::RefPtr<ImagePool::Instance> r = Glib::RefPtr<ImagePool::Instance>(new ImagePool::Instance(sop));

	// set dicom uid's
	r->m_sopinstanceuid = sop;

	std::string seriesuid;
	if(dset->findAndGetOFString(DCM_SeriesInstanceUID, seriesuid).good()) {
		r->m_seriesinstanceuid = seriesuid;
	}

	std::string studyuid;
	if(dset->findAndGetOFString(DCM_StudyInstanceUID, studyuid).good()) {
		r->m_studyinstanceuid = studyuid;
	}

	r->m_default_windowcenter = 0;
	r->m_default_windowwidth = 0;

	std::string value;

	// get instancenumber
	if(dset->findAndGetOFString(DCM_InstanceNumber, value).good()) {
		r->m_instancenumber = atoi(value.c_str());
	}

	// get windowwidth
	if(dset->findAndGetOFString(DCM_WindowWidth, value).good()) {
		r->m_default_windowwidth = (int)strtod(value.c_str(), NULL);
	}

	// get windowcenter
	if(dset->findAndGetOFString(DCM_WindowCenter, value).good()) {
		r->m_default_windowcenter = (int)strtod(value.c_str(), NULL);
	}

	// get pixeldata
	DicomImage* m_image = new DicomImage(dset, EXS_Unknown, CIF_MayDetachPixelData);

	m_image->setNoDisplayFunction();
	m_image->hideAllOverlays();
	m_image->setNoVoiTransformation();
	
	//dset->print(COUT, DCMTypes::PF_shortenLongTagValues);

	r->m_iscolor = !m_image->isMonochrome();

	if(r->m_iscolor) {
		r->m_depth = 8;
	}
	else {
		r->m_depth = m_image->getDepth();
	}
	std::cout << "depth: " << r->m_depth << std::endl;

	std::cout << "m_default_windowcenter = " << r->m_default_windowcenter << std::endl;
	std::cout << "m_default_windowwidth = " << r->m_default_windowwidth << std::endl;

	// get signed / unsigned
	Uint16 value1 = 0;
	if(dset->findAndGetUint16(DCM_PixelRepresentation, value1).good()) {
		r->m_is_signed = (value1 == 1);
		std::cout << "pixel representation: " << r->m_is_signed << std::endl;
		if(r->m_is_signed) {
			r->m_intercept -= (1 << r->m_depth) / 2 - 1;
		}
	}

	if(dset->findAndGetOFString(DCM_RescaleIntercept, value).good()) {
		r->m_intercept += atoi(value.c_str());
		if(r->m_intercept < 0) {
			r->m_is_signed = true;
		}
	}

	if(dset->findAndGetOFString(DCM_RescaleSlope, value).good()) {
		r->m_slope = atof(value.c_str());
	}

	if(dset->findAndGetUint16(DCM_HighBit, value1).good()) {
		r->m_highbit = value1;
		std::cout << "highbit: " << r->m_highbit << std::endl;
	}
	
	// correct depth
	r->m_bpp = r->m_depth;
	if(r->m_bpp > 8 and r->m_bpp < 16) {
		r->m_bpp = 16;
	}
	std::cout << "bpp: " << r->m_bpp << std::endl;

	if(r->m_default_windowwidth == 0 && r->m_default_windowcenter == 0) {
		// try LargestImagePixelValue / SmallestImagePixelValue
		short val_large = 0;
		short val_small = 0;
		if(dset->findAndGetSint16(DCM_LargestImagePixelValue, val_large).good() &&
			dset->findAndGetSint16(DCM_SmallestImagePixelValue, val_small).good()) {
		
			if((val_large != 0 || val_small != 0) && val_small < val_large) {
				r->m_default_windowcenter = (val_small + val_large) / 2;
				r->m_default_windowwidth = (val_large - val_small);
			}
		}
	}	

	// get Min / Max from image
	if(r->m_default_windowwidth == 0 && r->m_default_windowcenter == 0) {
		double min = 0;
		double max = 0;
		if(m_image->getMinMaxValues(min, max) == 1) {

			std::cout << "min = " << min << std::endl;
			std::cout << "max = " << max << std::endl;
			r->m_default_windowwidth = (int)(max - min);
			r->m_default_windowcenter = (int)((min + max) / 2);
		}
	}

	// get rawdata

	if(r->m_iscolor) {
		std::cout << "detected color image" << std::endl;
		r->m_size = m_image->getWidth()*3*m_image->getHeight();
		r->m_default_windowwidth = 256;
		r->m_default_windowcenter = 127;
	}
	else {
		r->m_size = m_image->getWidth()*(r->m_bpp <= 8 ? 1 : 2)*m_image->getHeight();
	}

	r->m_width = m_image->getWidth();
	r->m_height = m_image->getHeight();

	r->m_pixels = (void*)malloc(r->m_size);

	if(!m_image->getOutputData(r->m_pixels, r->m_size, r->m_iscolor ? 8 : r->m_depth, 0)){
		std::cerr << "dcmImage->getOutputData(..) == FALSE" << std::endl;
		delete m_image;
		m_image = NULL;
		context_mutex.unlock();
		return Glib::RefPtr<ImagePool::Instance>();
	}

	// set date
	if(dset->findAndGetOFString(DCM_AcquisitionDate, value).good()) {
		r->m_date = value;
	}
	else if(dset->findAndGetOFString(DCM_SeriesDate, value).good()) {
		r->m_date = value;
	}
	else if(dset->findAndGetOFString(DCM_StudyDate, value).good()) {
		r->m_date = value;
	}

	// set time
	if(dset->findAndGetOFString(DCM_AcquisitionTime, value).good()) {
		r->m_time = value;
	}
	else if(dset->findAndGetOFString(DCM_SeriesTime, value).good()) {
		r->m_time = value;
	}
	else if(dset->findAndGetOFString(DCM_StudyTime, value).good()) {
		r->m_time = value;
	}

	// set ManufacturersModelName
	if(dset->findAndGetOFString(DCM_ManufacturersModelName, value).good()) {
		r->m_model = value;
	}
	
	// set pixelspacing
	if(dset->findAndGetOFString(DCM_PixelSpacing, value, 0).good()) {
		r->m_spacing_x = strtod(value.c_str(), NULL);
		//std::cout << "pixelspacing x: " << r->m_spacing_x << std::endl;
	}

	if(dset->findAndGetOFString(DCM_PixelSpacing, value, 1).good()) {
		r->m_spacing_y = strtod(value.c_str(), NULL);
		//std::cout << "pixelspacing y: " << r->m_spacing_y << std::endl;
	}

	// get ImagePositionPatient
	if(dset->findAndGetOFString(DCM_ImagePositionPatient, value, 0).good()) {
		r->m_position.x = strtod(value.c_str(), NULL);
		//std::cout << "ImagePosition x: " << r->m_position.x << std::endl;
	}
	if(dset->findAndGetOFString(DCM_ImagePositionPatient, value, 1).good()) {
		r->m_position.y = strtod(value.c_str(), NULL);
		//std::cout << "ImagePosition y: " << r->m_position.y << std::endl;
	}
	if(dset->findAndGetOFString(DCM_ImagePositionPatient, value, 2).good()) {
		r->m_position.z = strtod(value.c_str(), NULL);
		//std::cout << "ImagePosition z: " << r->m_position.z << std::endl;
	}
	
	// get ImageOrientationPatient / Row - Vector
	if(dset->findAndGetOFString(DCM_ImageOrientationPatient, value, 0).good()) {
		r->m_orientation.x.x = strtod(value.c_str(), NULL);
		//std::cout << "Orientation-Row x: " << r->m_orientation.x.x << std::endl;
	}
	if(dset->findAndGetOFString(DCM_ImageOrientationPatient, value, 1).good()) {
		r->m_orientation.x.y = strtod(value.c_str(), NULL);
		//std::cout << "Orientation-Row y: " << r->m_orientation.x.y << std::endl;
	}
	if(dset->findAndGetOFString(DCM_ImageOrientationPatient, value, 2).good()) {
		r->m_orientation.x.z = strtod(value.c_str(), NULL);
		//std::cout << "Orientation-Row z: " << r->m_orientation.x.z << std::endl;
	}

	// get ImageOrientationPatient / Column - Vector
	if(dset->findAndGetOFString(DCM_ImageOrientationPatient, value, 3).good()) {
		r->m_orientation.y.x = strtod(value.c_str(), NULL);
		//std::cout << "Orientation-Column x: " << r->m_orientation.y.x << std::endl;
	}
	if(dset->findAndGetOFString(DCM_ImageOrientationPatient, value, 4).good()) {
		r->m_orientation.y.y = strtod(value.c_str(), NULL);
		//std::cout << "Orientation-Column y: " << r->m_orientation.y.y << std::endl;
	}
	if(dset->findAndGetOFString(DCM_ImageOrientationPatient, value, 5).good()) {
		r->m_orientation.y.z = strtod(value.c_str(), NULL);
		//std::cout << "Orientation-Column z: " << r->m_orientation.y.z << std::endl;
	}

	std::cout << "slope: " << r->m_slope << std::endl;
	std::cout << "intercept: " << r->m_intercept << std::endl;

	// study params
	dset->findAndGetOFString(DCM_PatientsName, r->m_patientsname);
	dset->findAndGetOFString(DCM_PatientsBirthDate, r->m_patientsbirthdate);
	dset->findAndGetOFString(DCM_PatientsSex, r->m_patientssex);
	dset->findAndGetOFString(DCM_StudyDescription, r->m_studydescription);

	if(r->m_studydescription.empty()) {
		dset->findAndGetOFString(DCM_SeriesDescription, r->m_studydescription);
	}

	// series params
	dset->findAndGetOFString(DCM_InstitutionName, r->m_institutionname);
	dset->findAndGetOFString(DCM_SeriesDescription, r->m_seriesdescription);

	if(r->m_seriesdescription.empty()) {
		dset->findAndGetOFString(DCM_StudyDescription, r->m_seriesdescription);
	}

	dset->findAndGetOFString(DCM_Modality, r->m_modality);

	context_image.push(r);

	if(threaded) {
		context_switch();
	}
	else {
		context_function();
	}
	
	delete m_image;

	context_mutex.unlock();

	return r;
}

void context_function() {
	if(context_image.size() == 0) {
		return;
	}

	Glib::RefPtr<ImagePool::Instance> r = context_image.front();
	context_image.pop();

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
		Signals::signal_study_added(new_study);
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
	
	m_pool[r->m_sopinstanceuid] = r;

	if(context_image.size() > 0) {
		context_function();
	}

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
