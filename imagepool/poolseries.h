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
    Update Date:      $Date: 2005/08/23 19:31:54 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/imagepool/poolseries.h,v $
    CVS/RCS Revision: $Revision: 1.1 $
    Status:           $State: Exp $
*/

#ifndef IMAGEPOOL_SERIES_H
#define IMAGEPOOL_SERIES_H

#include <glibmm/refptr.h>
#include <glibmm/object.h>
#include "poolinstance.h"

#include <string>
#include <map>

namespace ImagePool {

class Series : public Glib::Object {
public:

	Series();

	~Series();

	typedef std::map< std::string, Glib::RefPtr<ImagePool::Instance> >::iterator iterator;
	
	inline Glib::RefPtr<ImagePool::Instance>& at(const std::string& key) {
		return m_list[key];
	}
	
	inline Glib::RefPtr<ImagePool::Instance>& operator[](const std::string& key) {
		return m_list[key];
	}

	inline iterator begin() {
		return m_list.begin();
	}
	
	inline iterator end() {
		return m_list.end();
	}

	inline int size() {
		return m_list.size();
	}

	const std::string& studyinstanceuid();

	const std::string& seriesinstanceuid();

	const std::string& institutionname();

	const std::string& seriestime();

	const std::string& description();

	const std::string& modality();

	const std::string& stationname();

	int instancecount();

	sigc::signal< void, const Glib::RefPtr<ImagePool::Instance>& > signal_instance_added;

	sigc::signal< void, const Glib::RefPtr<ImagePool::Instance>& > signal_instance_removed;

protected:

	std::map< std::string, Glib::RefPtr<ImagePool::Instance> > m_list;
	
private:


	std::string m_studyinstanceuid;

	std::string m_seriesinstanceuid;

	std::string m_institutionname;

	std::string m_description;

	std::string m_modality;

	std::string m_seriestime;

	std::string m_stationname;

	int m_instancecount;
	
	friend Glib::RefPtr<ImagePool::Instance> create_instance(DcmDataset* dset);

	friend void context_function();

	friend Glib::RefPtr< ImagePool::Series > create_query_series(DcmDataset* dset);

};

}

#endif // IMAGEPOOL_SERIES_H
