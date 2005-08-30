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
    Update Date:      $Date: 2005/08/30 19:47:55 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/imagepool/imagepool.h,v $
    CVS/RCS Revision: $Revision: 1.3 $
    Status:           $State: Exp $
*/

#ifndef NAMESPACE_IMAGEPOOL_H
#define NAMESPACE_IMAGEPOOL_H

#include "poolinstance.h"
#include "poolseries.h"
#include "poolstudy.h"

#include <glibmm/slisthandle.h>
#include <string>

class DcmDataset;

namespace ImagePool {
	
	void init(bool connect = true);
	
	void close();

	Glib::RefPtr<ImagePool::Instance> create_instance(DcmDataset* dset);

	void remove_instance(const std::string& sopinstanceuid);

	void remove_instance(const Glib::RefPtr<ImagePool::Instance>& image);

	void remove_series(const Glib::RefPtr<ImagePool::Series>& series);

	void remove_study(const Glib::RefPtr<ImagePool::Study>& study);

	const Glib::RefPtr<ImagePool::Instance>& get_instance(const std::string& sopinstanceuid);

	const Glib::RefPtr<ImagePool::Series>& get_series(const std::string& seriesinstanceuid);

	const Glib::RefPtr<ImagePool::Study>& get_study(const std::string& studyinstanceuid);

	void load_from_file(const Glib::SListHandle<Glib::ustring>& list);

	void load_from_net(const std::string& studyinstanceuid);

	void query_from_net(
				const std::string& patientid,
				const std::string& lastname,
				const std::string& modality,
				const std::string& date_from,
				const std::string& date_to,
				const std::string& studydescription,
				const std::string& stationname,
				const sigc::slot< void, const Glib::RefPtr< ImagePool::Study >& >& resultslot
				);

	void query_series_from_net(
				const std::string& studyinstanceuid,
				const sigc::slot< void, const Glib::RefPtr< ImagePool::Series >& >& resultslot
				);

	class Signals {
	public:	
		static sigc::signal< void, Glib::RefPtr<ImagePool::Study> > signal_study_removed;
	};

}

#endif // NAMESPACE_IMAGEPOOL_H
