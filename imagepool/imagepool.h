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
    Update Date:      $Date: 2007/05/10 17:37:57 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/imagepool/imagepool.h,v $
    CVS/RCS Revision: $Revision: 1.15 $
    Status:           $State: Exp $
*/

#ifndef NAMESPACE_IMAGEPOOL_H
#define NAMESPACE_IMAGEPOOL_H

#include "poolinstance.h"
#include "poolseries.h"
#include "poolstudy.h"
#include "poolservers.h"

#include <glibmm/slisthandle.h>
#include <string>


class DcmDataset;

namespace ImagePool {

	void init();

	void close();

	bool register_instance(const Glib::RefPtr<ImagePool::Instance>& image);

	void remove_instance(const std::string& sopinstanceuid);

	void remove_instance(const Glib::RefPtr<ImagePool::Instance>& image);

	void remove_series(const Glib::RefPtr<ImagePool::Series>& series);

	void remove_study(const Glib::RefPtr<ImagePool::Study>& study);

	const Glib::RefPtr<ImagePool::Instance>& get_instance(const std::string& sopinstanceuid);

	const Glib::RefPtr<ImagePool::Series>& get_series(const std::string& seriesinstanceuid);

	const Glib::RefPtr<ImagePool::Study>& get_study(const std::string& studyinstanceuid);

	void query_from_net(
				const std::string& patientid,
				const std::string& lastname,
				const std::string& modality,
				const std::string& date_from,
				const std::string& date_to,
				const std::string& studydescription,
				const std::string& stationname,
				const std::string& local_aet,
				const std::set<std::string>& groups,
				const sigc::slot< void, const Glib::RefPtr< ImagePool::Study >& >& resultslot
				);

	void query_series_from_net(
				const std::string& studyinstanceuid,
				const std::string& server,
				const std::string& local_aet,
				const sigc::slot< void, const Glib::RefPtr< ImagePool::Series >& >& resultslot
				);

	int query_study_instances(
				const std::string& studyinstanceuid,
				const std::string& server,
				const std::string& local_aet
				);

	int query_study_series(
				const std::string& studyinstanceuid,
				const std::string& server,
				const std::string& local_aet
				);

	int query_study_series(
				const std::string& studyinstanceuid, 
				const std::string& server, 
				const std::string& local_aet, 
				std::list<std::string>& seriesinstanceuids
				);

	int query_series_instances(
				const std::string& studyinstanceuid, 
				const std::string& seriesinstanceuid, 
				const std::string& server, 
				const std::string& local_aet
				);

    void open_dicomdir(
                const Glib::ustring &dicomdir,
                const sigc::slot< void, const Glib::RefPtr< ImagePool::Study >& >& resultslot
                );

	void open_dicomdir_series(
				const std::string& studyinstanceuid,
				const Glib::ustring& dicomdir,
				const sigc::slot< void, const Glib::RefPtr< ImagePool::Series >& >& resultslot
				);

	std::string convert_string_from(const char* dicom_string, const std::string& system_encoding);

	std::string convert_string_to(const char* dicom_string, const std::string& system_encoding);

	void set_encoding(const std::string& dicom_encoding);

	std::string get_encoding();

	std::string get_system_encoding(const std::string& dicom_iso);

}

#endif // NAMESPACE_IMAGEPOOL_H
