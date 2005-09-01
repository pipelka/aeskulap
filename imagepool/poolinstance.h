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
    Update Date:      $Date: 2005/09/01 21:07:59 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/imagepool/poolinstance.h,v $
    CVS/RCS Revision: $Revision: 1.4 $
    Status:           $State: Exp $
*/

#ifndef IMAGEPOOL_INSTANCE_H
#define IMAGEPOOL_INSTANCE_H

#include <glibmm/refptr.h>
#include <glibmm/object.h>

#include <string>
#include <vector>

class DcmDataset;
class DicomImage;

namespace ImagePool {

class Series;
class Study;
class Loader;

class Instance : public Glib::Object {
protected:

	Instance(const std::string& sopinstanceuid);

	friend Glib::RefPtr<ImagePool::Instance> create_instance(DcmDataset* dset);
	
	friend void ImagePool::remove_instance(const Glib::RefPtr<ImagePool::Instance>&);

	friend class ImagePool::Loader;

public:

	typedef struct _Point {
		_Point() {
			x = 0;
			y = 0;
			z = 0;
		};
		double x;
		double y;
		double z;
	} Point;
	
	typedef Point Vector;

	typedef struct _Orientation {
		Vector x;
		Vector y;
	} Orientation;
	
	~Instance();

	void* pixels(int frame=0);
	
	int depth();
	
	int bpp();

	int highbit();
	
	int width();
	
	int height();
	
	bool iscolor();
	
	const std::string& sopinstanceuid();
	
	const std::string& seriesinstanceuid();
	
	const std::string& studyinstanceuid();

	double slope();

	int intercept();
	
	bool is_signed();

	int default_windowcenter();

	int default_windowwidth();

	int instancenumber();

	const Glib::RefPtr<ImagePool::Series>& series();

	const Glib::RefPtr<ImagePool::Study>& study();

	const std::string& date();

	const std::string& time();
	
	const std::string& model();

	double spacing_x();

	double spacing_y();
	
	void set_index(int index);
	
	int get_index();

	const Point& get_position();
	
	const Orientation& get_orientation();

	/**
	 * transform a point.
	 * param a source point in world coordinates
	 * param b result in the patients coordinate system
	 * transforms the point a from the world coordinate system
	 * into point b in the patients coordinate system
	*/
	bool transform_to_viewport(const Point& a, Point& b);

	bool transform_to_world(const Point& a, Point& b);

	void clear();
	
private:

	std::vector<void*> m_pixels;

	int m_size;

	int m_depth;

	int m_bpp;

	int m_highbit;
	
	int m_width;

	int m_height;

	bool m_iscolor;

	double m_slope;
	
	int m_intercept;
	
	bool m_is_signed;

	int m_default_windowcenter;
	
	int m_default_windowwidth;

	int m_instancenumber;

	std::string m_sopinstanceuid;

	std::string m_seriesinstanceuid;

	std::string m_studyinstanceuid;

	std::string m_patientsname;

	std::string m_patientsbirthdate;

	std::string m_patientssex;

	std::string m_studydescription;

	std::string m_institutionname;

	std::string m_seriesdescription;

	std::string m_modality;

	Glib::RefPtr<ImagePool::Series> m_series;

	Glib::RefPtr<ImagePool::Study> m_study;

	std::string m_date;
	
	std::string m_time;

	std::string m_model;

	double m_spacing_x;

	double m_spacing_y;

	int m_index;
	
	Point m_position;

	Orientation m_orientation;
};

}

#endif // IMAGEPOOL_INSTANCE_H
