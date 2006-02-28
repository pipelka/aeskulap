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
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/imagepool/poolinstance.cpp,v $
    CVS/RCS Revision: $Revision: 1.8.2.1 $
    Status:           $State: Exp $
*/

#include "poolinstance.h"
#include "poolstudy.h"
#include "poolseries.h"

#include "imagepool.h"

#include "dcdatset.h"
#include "dcmimage.h"
#include "diregist.h"
#include "dcfilefo.h"
#include "dcdeftag.h"

#include <iostream>

namespace ImagePool {
	
Instance::Instance(const std::string& sopinstanceuid) :
m_size(0),
m_depth(0),
m_bpp(0),
m_highbit(0),
m_width(0),
m_height(0),
m_iscolor(false),
m_slope(1),
m_intercept(0),
m_is_signed(false),
m_default_windowcenter(0),
m_default_windowwidth(0),
m_instancenumber(0),
//m_sopinstanceuid(sopinstanceuid),
m_spacing_x(0),
m_spacing_y(0),
m_index(1),
m_min(0),
m_max(0)
{
	set_tag("SOPInstanceUID", sopinstanceuid);

	/*Glib::ustring prop = "test";
	Glib::Value<std::string> value;
	value.set("123");
	set_tag_value(prop, value);
	
	Glib::Value<std::string> value1;
	get_property_value(prop, value1);*/
}
	
Instance::~Instance() {
	for(unsigned int i=0; i<m_pixels.size(); i++) {
		if(m_pixels[i] != NULL) {
			free(m_pixels[i]);
		}
	}
}

void* Instance::pixels(int frame) {
	return m_pixels[frame];
}
	
int Instance::depth() {
	return m_depth;
}
	
int Instance::bpp() {
	return m_bpp;
}

int Instance::highbit() {
	return m_highbit;
}

int Instance::width() {
	return m_width;
}
	
int Instance::height() {
	return m_height;
}
	
bool Instance::iscolor() {
	return m_iscolor;
}

/*const std::string& Instance::sopinstanceuid() {
	return m_sopinstanceuid;
}*/
	
/*const std::string& Instance::seriesinstanceuid() {
	return m_seriesinstanceuid;
}*/

/*const std::string& Instance::patientsname() {
	return m_patientsname;
}*/

/*const std::string& Instance::patientsbirthdate() {
	return m_patientsbirthdate;
}*/

/*const std::string& Instance::patientssex() {
	return m_patientssex;
}*/

/*const std::string& Instance::studyinstanceuid() {
	return m_studyinstanceuid;
}*/

/*const std::string& Instance::studydescription() {
	return m_studydescription;
}

const std::string& Instance::studydate() {
	return m_studydate;
}

const std::string& Instance::studytime() {
	return m_studytime;
}*/

double Instance::slope() {
	return m_slope;
}

int Instance::intercept() {
	return m_intercept;
}

bool Instance::is_signed() {
	return m_is_signed;
}

int Instance::default_windowcenter() {
	return m_default_windowcenter;
}

int Instance::default_windowwidth() {
	return m_default_windowwidth;
}

int Instance::instancenumber() {
	return m_instancenumber;
}

const Glib::RefPtr<ImagePool::Series>& Instance::series() {
	return m_series;
}

const Glib::RefPtr<ImagePool::Study>& Instance::study() {
	return m_study;
}

const std::string& Instance::date() {
	return m_date;
}

const std::string& Instance::time() {
	return m_time;
}

const std::string& Instance::model() {
	return m_model;
}

double Instance::spacing_x() {
	return m_spacing_x;
}

double Instance::spacing_y() {
	return m_spacing_y;
}

int Instance::get_index() {
	return m_index;
}

void Instance::set_index(int index) {
	m_index = index;
}

const Instance::Point& Instance::get_position() {
	return m_position;
}
	
const Instance::Orientation& Instance::get_orientation() {
	return m_orientation;
}

bool Instance::transform_to_viewport(const Instance::Point& a, Instance::Point& b) {
	if(m_orientation.x.x == 0 && m_orientation.x.y == 0 && m_orientation.x.z == 0) {
		return false;
	}

	Point c;

	// move point to our origin;
	b = a;
	b.x -= m_position.x;
	b.y -= m_position.y;
	b.z -= m_position.z;
	
	// transform point into our coordinate system
	c.x = m_orientation.x.x * b.x + m_orientation.x.y * b.y + m_orientation.x.z * b.z;
	c.y = m_orientation.y.x * b.x + m_orientation.y.y * b.y + m_orientation.y.z * b.z;
	c.z = 0;
	
	b = c;
	return true;
}

bool Instance::transform_to_world(const Point& a, Point& b) {
	b.x = m_position.x + m_orientation.x.x * a.x + m_orientation.y.x * a.y;
	b.y = m_position.y + m_orientation.x.y * a.x + m_orientation.y.y * a.y;
	b.z = m_position.z + m_orientation.x.z * a.x + m_orientation.y.z * a.y;
	
	return true;
}


void Instance::clear() {
	m_study.clear();
	m_series.clear();
}

Instance::Type Instance::get_type() {
	if(get_framecount() > 1) {
		return MULTIFRAME;
	}

	return SINGLE;
}

int Instance::get_framecount() {
	return m_pixels.size();
}

int Instance::min_value() {
	return m_min;
}
	
int Instance::max_value() {
	return m_max;
}

bool Instance::has_3d_information() {
	return (
		m_orientation.x.x != 0 ||
		m_orientation.x.y != 0 ||
		m_orientation.x.z != 0 ||
		m_orientation.y.x != 0 ||
		m_orientation.y.y != 0 ||
		m_orientation.y.z != 0
		);
}

Glib::RefPtr<ImagePool::Instance> Instance::create(DcmDataset* dset) {
	if(dset == NULL) {
		return Glib::RefPtr<ImagePool::Instance>();
	}

	// get SOPInstanceUID
	
	std::string sop;
	dset->findAndGetOFString(DCM_SOPInstanceUID, sop);
	
	// wrap in smartpointer
	Glib::RefPtr<ImagePool::Instance> r = Glib::RefPtr<ImagePool::Instance>(new ImagePool::Instance(sop));

	// set encoding
	std::string enc[2];
	dset->findAndGetOFString(DCM_SpecificCharacterSet, enc[0], 0);
	dset->findAndGetOFString(DCM_SpecificCharacterSet, enc[1], 1);
	r->set_encoding(enc[0], enc[1]);

	// set dicom uid's
	r->copy_tag(dset, DCM_SeriesInstanceUID, "SeriesInstanceUID");
	r->copy_tag(dset, DCM_StudyInstanceUID, "StudyInstanceUID");

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
	//std::cout << "depth: " << r->m_depth << std::endl;

	//std::cout << "m_default_windowcenter = " << r->m_default_windowcenter << std::endl;
	//std::cout << "m_default_windowwidth = " << r->m_default_windowwidth << std::endl;

	// get signed / unsigned
	Uint16 value1 = 0;
	if(dset->findAndGetUint16(DCM_PixelRepresentation, value1).good()) {
		r->m_is_signed = (value1 == 1);
		//std::cout << "pixel representation: " << r->m_is_signed << std::endl;
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
		//std::cout << "highbit: " << r->m_highbit << std::endl;
	}
	
	// correct depth
	r->m_bpp = r->m_depth;
	if(r->m_bpp > 8 and r->m_bpp < 16) {
		r->m_bpp = 16;
	}
	//std::cout << "bpp: " << r->m_bpp << std::endl;

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

	double min = 0;
	double max = 0;
	if(m_image->getMinMaxValues(min, max) == 1) {
		r->m_min = (int)min;
		r->m_max = (int)max;
	}

	// get Min / Max from image
	if(r->m_default_windowwidth == 0 && r->m_default_windowcenter == 0) {
			//std::cout << "min = " << min << std::endl;
			//std::cout << "max = " << max << std::endl;
			r->m_default_windowwidth = (int)(max - min);
			r->m_default_windowcenter = (int)((min + max) / 2);
	}

	// get rawdata

	if(r->m_iscolor) {
		//std::cout << "detected color image" << std::endl;
		r->m_size = m_image->getWidth()*3*m_image->getHeight();
		r->m_default_windowwidth = 256;
		r->m_default_windowcenter = 127;
	}
	else {
		r->m_size = m_image->getWidth()*(r->m_bpp <= 8 ? 1 : 2)*m_image->getHeight();
	}

	r->m_width = m_image->getWidth();
	r->m_height = m_image->getHeight();

	for(unsigned int f=0; f<m_image->getFrameCount(); f++) {
		void* pixels = (void*)malloc(r->m_size);
		r->m_pixels.push_back(pixels);
		//std::cout << "frame: " << f << std::endl;
	
		if(!m_image->getOutputData(pixels, r->m_size, r->m_iscolor ? 8 : r->m_depth, f)){
			std::cerr << "dcmImage->getOutputData(..) == FALSE" << std::endl;
			delete m_image;
			m_image = NULL;
			return Glib::RefPtr<ImagePool::Instance>();
		}
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
	}

	if(dset->findAndGetOFString(DCM_PixelSpacing, value, 1).good()) {
		r->m_spacing_y = strtod(value.c_str(), NULL);
	}

	// get ImagePositionPatient
	if(dset->findAndGetOFString(DCM_ImagePositionPatient, value, 0).good()) {
		r->m_position.x = strtod(value.c_str(), NULL);
	}
	if(dset->findAndGetOFString(DCM_ImagePositionPatient, value, 1).good()) {
		r->m_position.y = strtod(value.c_str(), NULL);
	}
	if(dset->findAndGetOFString(DCM_ImagePositionPatient, value, 2).good()) {
		r->m_position.z = strtod(value.c_str(), NULL);
	}
	
	// get ImageOrientationPatient / Row - Vector
	if(dset->findAndGetOFString(DCM_ImageOrientationPatient, value, 0).good()) {
		r->m_orientation.x.x = strtod(value.c_str(), NULL);
	}
	if(dset->findAndGetOFString(DCM_ImageOrientationPatient, value, 1).good()) {
		r->m_orientation.x.y = strtod(value.c_str(), NULL);
	}
	if(dset->findAndGetOFString(DCM_ImageOrientationPatient, value, 2).good()) {
		r->m_orientation.x.z = strtod(value.c_str(), NULL);
	}

	// get ImageOrientationPatient / Column - Vector
	if(dset->findAndGetOFString(DCM_ImageOrientationPatient, value, 3).good()) {
		r->m_orientation.y.x = strtod(value.c_str(), NULL);
	}
	if(dset->findAndGetOFString(DCM_ImageOrientationPatient, value, 4).good()) {
		r->m_orientation.y.y = strtod(value.c_str(), NULL);
	}
	if(dset->findAndGetOFString(DCM_ImageOrientationPatient, value, 5).good()) {
		r->m_orientation.y.z = strtod(value.c_str(), NULL);
	}

	//std::cout << "slope: " << r->m_slope << std::endl;
	//std::cout << "intercept: " << r->m_intercept << std::endl;

	// study params
	r->copy_tag(dset, DCM_PatientsName, "PatientsName", true);
	r->copy_tag(dset, DCM_PatientsBirthDate, "PatientsBirthDate");
	r->copy_tag(dset, DCM_PatientsSex, "PatientsSex");
	if(!r->copy_tag(dset, DCM_StudyDescription, "StudyDescription", true)) {
		r->set_tag("StudyDescription", gettext("no description"));
	}

	r->copy_tag(dset, DCM_StudyDate, "StudyDate");
	r->copy_tag(dset, DCM_StudyTime, "StudyTime");
	
	// series params
	r->copy_tag(dset, DCM_InstitutionName, "InstitutionName");
	if(!r->copy_tag(dset, DCM_SeriesDescription, "SeriesDescription", true)) {
		if(!r->copy_tag(dset, DCM_StudyDescription, "SeriesDescription", true)) {
			r->set_tag("SeriesDescription", gettext("no description"));
		}
	}

	r->copy_tag(dset, DCM_Modality, "Modality");

	Glib::RefPtr<ImagePool::Study> new_study = get_study(r->tag("StudyInstanceUID"));
	if(new_study->size() == 0) {
		/*new_study->m_studyinstanceuid = r->tag("StudyInstanceUID");
		new_study->m_patientsname = r->tag("PatientsName");
		new_study->m_patientsbirthdate = r->tag("PatientsBirthDate");
		new_study->m_patientssex = r->tag("PatientsSex");
		new_study->m_studydescription = r->tag("StudyDescription");*/
		new_study->copy_tag(r, "StudyInstanceUID");
		new_study->copy_tag(r, "PatientsName");
		new_study->copy_tag(r, "PatientsBirthDate");
		new_study->copy_tag(r, "PatientsSex");
		new_study->copy_tag(r, "StudyDescription");
	}
	r->m_study = new_study;

	delete m_image;

	return r;
}

}

