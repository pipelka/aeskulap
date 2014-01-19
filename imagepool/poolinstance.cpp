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
    Update Date:      $Date: 2007/09/05 10:31:14 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/imagepool/poolinstance.cpp,v $
    CVS/RCS Revision: $Revision: 1.17 $
    Status:           $State: Exp $
*/

#include "poolinstance.h"
#include "imagepool.h"

#include "dcmtk/dcmdata/dcdatset.h"
#include "dcmtk/dcmimgle/dcmimage.h"
#include "dcmtk/dcmimage/diregist.h"
#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/dcmdata/dcdeftag.h"
#include "../gettext.h"

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
m_sopinstanceuid(sopinstanceuid),
m_spacing_x(0),
m_spacing_y(0),
m_index(1),
m_min(0),
m_max(0),
m_studyrelatedinstances(-1),
m_seriesrelatedinstances(-1),
m_studyrelatedseries(-1),
m_invert_lut_shape(false)
{
	m_encoding[0] = "UTF-8";
	m_encoding[1] = "UTF-8";
	m_encoding[2] = "UTF-8";
}
	
Instance::~Instance() {
	for(unsigned int i=0; i<m_pixels.size(); i++) {
		if(m_pixels[i] != NULL) {
			free(m_pixels[i]);
		}
	}
}

void* Instance::pixels(unsigned int frame) {
	if(frame >= m_pixels.size()) {
		return NULL;
	}

	return m_pixels[frame];
}

double Instance::pixel_value(int x, int y, int frame) {
	if(x < 0 || y < 0) {
		return 0;
	}
	
	if(x >= width() || y >= height()) {
		return 0;
	}

	double result = 0;
	int samplesize = (bpp()/8) * (iscolor() ? 3 : 1);
	int pitch = width() * samplesize;
	
	guint8* p = static_cast<guint8*>(pixels(frame)) + pitch*y + samplesize * x;
	guint16* p16 = 0;

	switch(samplesize) {
		case 1:
			result = (double)(*p);
			break;
		case 2:
			p16 = (guint16*)p;
			result = (double)(*p16);
			break;
		case 3:
			result = (double)(*p + (*++p) << 8 + (*++p) << 16);
			break;
	}
	
	if(slope() != 0) {
		result *= slope();
	}
	
	result += intercept();

	return result;
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

const std::string& Instance::sopinstanceuid() {
	return m_sopinstanceuid;
}
	
const std::string& Instance::seriesinstanceuid() {
	return m_seriesinstanceuid;
}

const std::string& Instance::patientsname() {
	return m_patientsname;
}

const std::string& Instance::patientsbirthdate() {
	return m_patientsbirthdate;
}

const std::string& Instance::patientssex() {
	return m_patientssex;
}

const std::string& Instance::studyinstanceuid() {
	return m_studyinstanceuid;
}

const std::string& Instance::studydescription() {
	return m_studydescription;
}

const std::string& Instance::studydate() {
	return m_studydate;
}

const std::string& Instance::studytime() {
	return m_studytime;
}

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
    OFString ofstr;
	if(dset == NULL) {
		return Glib::RefPtr<ImagePool::Instance>();
	}

	// get SOPInstanceUID
	
	std::string sop;
	dset->findAndGetOFString(DCM_SOPInstanceUID, ofstr).bad();
	sop = ofstr.c_str();
	
	// wrap in smartpointer
	Glib::RefPtr<ImagePool::Instance> r = Glib::RefPtr<ImagePool::Instance>(new ImagePool::Instance(sop));

	// set encoding
	std::string enc[2];
	dset->findAndGetOFString(DCM_SpecificCharacterSet, ofstr, 0);
	enc[0] = ofstr.c_str();
	dset->findAndGetOFString(DCM_SpecificCharacterSet, ofstr, 1);
	enc[1] = ofstr.c_str();
	r->set_encoding(enc[0], enc[1]);

	// set dicom uid's
	r->m_sopinstanceuid = sop;

	std::string seriesuid;
	if(dset->findAndGetOFString(DCM_SeriesInstanceUID, ofstr).good()) {
	    seriesuid = ofstr.c_str();
		r->m_seriesinstanceuid = ofstr.c_str();
	}

	std::string studyuid;
	if(dset->findAndGetOFString(DCM_StudyInstanceUID, ofstr).good()) {
	    studyuid = ofstr.c_str();
		r->m_studyinstanceuid = studyuid;
	}

	r->m_default_windowcenter = 0;
	r->m_default_windowwidth = 0;

	std::string value;

	// get instancenumber
	if(dset->findAndGetOFString(DCM_InstanceNumber, ofstr).good()) {
		r->m_instancenumber = atoi(ofstr.c_str());
	}

	// get windowwidth
	if(dset->findAndGetOFString(DCM_WindowWidth, ofstr).good()) {
		r->m_default_windowwidth = (int)strtod(ofstr.c_str(), NULL);
	}

	// get windowcenter
	if(dset->findAndGetOFString(DCM_WindowCenter, ofstr).good()) {
		r->m_default_windowcenter = (int)strtod(ofstr.c_str(), NULL);
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
		Uint16 depth;
		if(dset->findAndGetUint16(DCM_BitsStored, depth).good()) {
		    r->m_depth = depth;
		}
		if( r->m_depth > 16 ) {
			r->m_depth = 16;
		}
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

	if(dset->findAndGetOFString(DCM_RescaleIntercept, ofstr).good()) {
		r->m_intercept += atoi(ofstr.c_str());
		if(r->m_intercept < 0) {
			r->m_is_signed = true;
		}
	}

	if(dset->findAndGetOFString(DCM_RescaleSlope, ofstr).good()) {
		r->m_slope = atof(ofstr.c_str());
		if(r->m_slope == 0) {
		    r->m_slope = atoi(ofstr.c_str());
		}
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

	// inverted lut shape
	if(dset->findAndGetOFString(DCM_PresentationLUTShape, ofstr).good()) {
		r->m_invert_lut_shape = (ofstr == "INVERSE");
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
	if(dset->findAndGetOFString(DCM_AcquisitionDate, ofstr).good()) {
		r->m_date = ofstr.c_str();
	}
	else if(dset->findAndGetOFString(DCM_SeriesDate, ofstr).good()) {
		r->m_date = ofstr.c_str();
	}
	else if(dset->findAndGetOFString(DCM_StudyDate, ofstr).good()) {
		r->m_date = ofstr.c_str();
	}

	// set time
	if(dset->findAndGetOFString(DCM_AcquisitionTime, ofstr).good()) {
		r->m_time = ofstr.c_str();
	}
	else if(dset->findAndGetOFString(DCM_SeriesTime, ofstr).good()) {
		r->m_time = ofstr.c_str();
	}
	else if(dset->findAndGetOFString(DCM_StudyTime, ofstr).good()) {
		r->m_time = ofstr.c_str();
	}

	// set ManufacturersModelName
	if(dset->findAndGetOFString(DCM_ManufacturerModelName, ofstr).good()) {
		r->m_model = ofstr.c_str();
	}
	
	// set pixelspacing
	if(dset->findAndGetOFString(DCM_PixelSpacing, ofstr, 0).good()) {
		r->m_spacing_x = strtod(ofstr.c_str(), NULL);
	}

	if(dset->findAndGetOFString(DCM_PixelSpacing, ofstr, 1).good()) {
		r->m_spacing_y = strtod(ofstr.c_str(), NULL);
	}

	if(r->m_spacing_x == 0 && r->m_spacing_y == 0) {
		if(dset->findAndGetOFString(DCM_ImagerPixelSpacing, ofstr, 0).good()) {
			r->m_spacing_x = strtod(ofstr.c_str(), NULL);
		}

		if(dset->findAndGetOFString(DCM_ImagerPixelSpacing, ofstr, 1).good()) {
			r->m_spacing_y = strtod(ofstr.c_str(), NULL);
		}		
	}

	// get ImagePositionPatient
	if(dset->findAndGetOFString(DCM_ImagePositionPatient, ofstr, 0).good()) {
		r->m_position.x = strtod(ofstr.c_str(), NULL);
	}
	if(dset->findAndGetOFString(DCM_ImagePositionPatient, ofstr, 1).good()) {
		r->m_position.y = strtod(ofstr.c_str(), NULL);
	}
	if(dset->findAndGetOFString(DCM_ImagePositionPatient, ofstr, 2).good()) {
		r->m_position.z = strtod(ofstr.c_str(), NULL);
	}
	
	// get ImageOrientationPatient / Row - Vector
	if(dset->findAndGetOFString(DCM_ImageOrientationPatient, ofstr, 0).good()) {
		r->m_orientation.x.x = strtod(ofstr.c_str(), NULL);
	}
	if(dset->findAndGetOFString(DCM_ImageOrientationPatient, ofstr, 1).good()) {
		r->m_orientation.x.y = strtod(ofstr.c_str(), NULL);
	}
	if(dset->findAndGetOFString(DCM_ImageOrientationPatient, ofstr, 2).good()) {
		r->m_orientation.x.z = strtod(value.c_str(), NULL);
	}

	// get ImageOrientationPatient / Column - Vector
	if(dset->findAndGetOFString(DCM_ImageOrientationPatient, ofstr, 3).good()) {
		r->m_orientation.y.x = strtod(ofstr.c_str(), NULL);
	}
	if(dset->findAndGetOFString(DCM_ImageOrientationPatient, ofstr, 4).good()) {
		r->m_orientation.y.y = strtod(ofstr.c_str(), NULL);
	}
	if(dset->findAndGetOFString(DCM_ImageOrientationPatient, ofstr, 5).good()) {
		r->m_orientation.y.z = strtod(ofstr.c_str(), NULL);
	}

	//std::cout << "slope: " << r->m_slope << std::endl;
	//std::cout << "intercept: " << r->m_intercept << std::endl;

	// study params
	if(dset->findAndGetOFString(DCM_PatientName, ofstr).good()) {
		r->m_patientsname = r->convert_string(ofstr.c_str());
	}
	dset->findAndGetOFString(DCM_PatientBirthDate, ofstr);
	r->m_patientsbirthdate = ofstr.c_str();
	dset->findAndGetOFString(DCM_PatientSex, ofstr);
    r->m_patientssex = ofstr.c_str();
	if(dset->findAndGetOFString(DCM_StudyDescription, ofstr).good()) {
		r->m_studydescription = r->convert_string(ofstr.c_str());
	}

	if(r->m_studydescription.empty()) {
		if(dset->findAndGetOFString(DCM_SeriesDescription, ofstr).good()) {
			r->m_studydescription = r->convert_string(ofstr.c_str());
		}
	}

	if(r->m_studydescription.empty()) {
		r->m_studydescription = gettext("no description");
	}

	dset->findAndGetOFString(DCM_StudyDate, ofstr);
	r->m_studydate = ofstr.c_str();
	dset->findAndGetOFString(DCM_StudyTime, ofstr);
    r->m_studytime = ofstr.c_str();

	// series params
	if(dset->findAndGetOFString(DCM_InstitutionName, ofstr).good()) {
		r->m_seriesdescription = r->convert_string(ofstr.c_str());
	}
	if(dset->findAndGetOFString(DCM_SeriesDescription, ofstr).good()) {
		r->m_seriesdescription = r->convert_string(ofstr.c_str());
	}

	if(r->m_seriesdescription.empty()) {
		if(dset->findAndGetOFString(DCM_StudyDescription, ofstr).good()) {
			r->m_seriesdescription = r->convert_string(ofstr.c_str());
		}
	}

	if(r->m_seriesdescription.empty()) {
		r->m_seriesdescription = gettext("no description");
	}

	dset->findAndGetOFString(DCM_Modality, ofstr);
    r->m_modality = ofstr.c_str();

	// number of study related instances
	if(dset->findAndGetOFString(DCM_NumberOfStudyRelatedInstances, ofstr).good()) {
		r->m_studyrelatedinstances = atoi(ofstr.c_str());
	}

	// number of study related series
	if(dset->findAndGetOFString(DCM_NumberOfStudyRelatedSeries, ofstr).good()) {
		r->m_studyrelatedseries = atoi(ofstr.c_str());
	}

	// number of series related instances
	if(dset->findAndGetOFString(DCM_NumberOfSeriesRelatedInstances, ofstr).good()) {
		r->m_seriesrelatedinstances = atoi(ofstr.c_str());
	}

	Glib::RefPtr<ImagePool::Study> new_study = get_study(r->m_studyinstanceuid);
	if(new_study->size() == 0) {
		new_study->m_studyinstanceuid = r->studyinstanceuid();
		new_study->m_patientsname = r->m_patientsname;
		new_study->m_patientsbirthdate = r->m_patientsbirthdate;
		new_study->m_patientssex = r->m_patientssex;
		new_study->m_studydescription = r->m_studydescription;
	}
	r->m_study = new_study;

	delete m_image;

	return r;
}

bool Instance::set_encoding(const std::string& single, const std::string& ideographic) {
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

std::string Instance::convert_string(const char* dicom_string) {
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

const std::string& Instance::modality() {
	return m_modality;
}

int Instance::studyrelatedinstances() {
	return m_studyrelatedinstances;
}

int Instance::studyrelatedseries() {
	return m_studyrelatedseries;
}

int Instance::seriesrelatedinstances() {
	return m_seriesrelatedinstances;
}

bool Instance::invert_lut_shape() {
	return m_invert_lut_shape;
}

}

