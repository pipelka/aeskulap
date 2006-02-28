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
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/imagepool/poolstudy.h,v $
    CVS/RCS Revision: $Revision: 1.7.2.1 $
    Status:           $State: Exp $
*/

#ifndef IMAGEPOOL_STUDY_H
#define IMAGEPOOL_STUDY_H

#include "dicombase.h"
#include "poolseries.h"

#include <string>
#include <map>

namespace ImagePool {

	
class Study : public DicomBase {
public:

	typedef std::map< std::string, Glib::RefPtr<ImagePool::Series> >::iterator iterator;
	
	Study();

	~Study();

	inline Glib::RefPtr<ImagePool::Series>& at(const std::string& key) {
		return m_list[key];
	}
	
	inline Glib::RefPtr<ImagePool::Series>& operator[](const std::string& key) {
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

	/*const std::string& studyinstanceuid();
	
	const std::string& patientsname();
	
	const std::string& patientsbirthdate();

	const std::string& patientssex();
	
	const std::string& studydate();

	const std::string& studytime();

	const std::string& studydescription();*/

	const std::string& server();

	sigc::signal< void, const Glib::RefPtr<ImagePool::Series>& > signal_series_added;

	sigc::signal< void, const Glib::RefPtr<ImagePool::Series>& > signal_series_removed;

	sigc::signal< void, double > signal_progress;

	void set_instancecount(int cur, int max);

	void emit_progress();

	int get_instancecount();

	int seriescount();

	void set_seriescount(int series);

	int has_3d_information();

protected:

	std::map< std::string, Glib::RefPtr<ImagePool::Series> > m_list;
	
private:

	int m_max_series;

	int m_max_instances;
	
	int m_cur_instances;

	/*std::string m_studyinstanceuid;
	
	std::string m_patientsname;

	std::string m_patientsbirthdate;
	
	std::string m_studydescription;

	std::string m_studydate;

	std::string m_studytime;

	std::string m_patientssex;*/

	std::string m_server;
	
	friend Glib::RefPtr<ImagePool::Instance> Instance::create(DcmDataset* dset);

	friend Glib::RefPtr< ImagePool::Study > create_query_study(DcmDataset* dset, const std::string& server);

	friend class ImagePool::Loader;

};

}

#endif // IMAGEPOOL_STUDY_H
