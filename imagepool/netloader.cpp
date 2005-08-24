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
    Update Date:      $Date: 2005/08/24 21:55:42 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/imagepool/netloader.cpp,v $
    CVS/RCS Revision: $Revision: 1.2 $
    Status:           $State: Exp $
*/

#include <gtkmm.h>
#include <gconfmm.h>

#include "imagepool.h"
#include "poolmoveassociation.h"
#include "dcdatset.h"
#include "dcdeftag.h"
#include "netclient.h"

namespace ImagePool {

class DicomMover : public MoveAssociation {
protected:
	void OnResponseReceived(DcmDataset *response) {
		ImagePool::create_instance(response, true);
	};
};


static void net_loader_thread(const std::string& studyinstanceuid) {
	NetClient<DicomMover> mover;
	mover.SetMaxResults(1000);

	DcmDataset query;
	DcmElement* e = NULL;
	
	e = newDicomElement(DCM_QueryRetrieveLevel);
	e->putString("IMAGE");
	query.insert(e);

	e = newDicomElement(DCM_SOPInstanceUID);
	query.insert(e);

	e = newDicomElement(DCM_InstanceNumber);
	query.insert(e);

	e = newDicomElement(DCM_StudyInstanceUID);
	e->putString(studyinstanceuid.c_str());
	query.insert(e);

	mover.QueryServers(&query);
	//progress(studyinstanceuid, 100);
}

void load_from_net(const std::string& studyinstanceuid) {
	Glib::Thread *const loader = Glib::Thread::create(
      sigc::bind(sigc::ptr_fun(net_loader_thread), studyinstanceuid), false);
}


} // namespace ImagePool
