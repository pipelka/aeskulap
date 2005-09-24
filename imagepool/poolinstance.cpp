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
    Update Date:      $Date: 2005/09/24 19:09:29 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/imagepool/poolinstance.cpp,v $
    CVS/RCS Revision: $Revision: 1.6 $
    Status:           $State: Exp $
*/

#include "poolinstance.h"
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
m_sopinstanceuid(sopinstanceuid),
m_spacing_x(0),
m_spacing_y(0),
m_index(1),
m_min(0),
m_max(0)
{
}
	
Instance::~Instance() {
	for(int i=0; i<m_pixels.size(); i++) {
		if(m_pixels[i] != NULL) {
			free(m_pixels[i]);
		}
	}
	
	std::cout << "removed instance '" << m_sopinstanceuid << "'" << std::endl;
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

const std::string& Instance::sopinstanceuid() {
	return m_sopinstanceuid;
}
	
const std::string& Instance::seriesinstanceuid() {
	return m_seriesinstanceuid;
}
	
const std::string& Instance::studyinstanceuid() {
	return m_studyinstanceuid;
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

}

