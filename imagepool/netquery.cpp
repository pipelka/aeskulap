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
    Update Date:      $Date: 2006/03/09 15:35:14 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/imagepool/netquery.cpp,v $
    CVS/RCS Revision: $Revision: 1.18.2.2 $
    Status:           $State: Exp $
*/

#include "imagepool.h"
#include "dcdatset.h"
#include "dcdeftag.h"
#include "poolfindassociation.h"
#include "poolservers.h"
#include "poolstudy.h"
#include "poolseries.h"
#include "netclient.h"
#include "gettext.h"

#include <iostream>

namespace ImagePool {
	
Glib::RefPtr< ImagePool::Study > create_query_study(DcmDataset* dset, const std::string& server) {
	Glib::RefPtr< ImagePool::Study > result = Glib::RefPtr< ImagePool::Study >(new Study);

	Glib::RefPtr< ImagePool::Instance > item = Instance::create(dset);

	result->m_server = server;
	result->copy_tag(item, "StudyInstanceUID");
	result->copy_tag(item, "PatientsName");
	result->copy_tag(item, "PatientsBirthDate");
	result->copy_tag(item, "PatientsSex");
	result->copy_tag(item, "StudyDescription");
	result->copy_tag(item, "StudyDate");
	result->copy_tag(item, "StudyTime");
	
	return result;
}

Glib::RefPtr< ImagePool::Series > create_query_series(DcmDataset* dset) {
	Glib::RefPtr< ImagePool::Series > result = Glib::RefPtr< ImagePool::Series >(new Series);

	result->copy_tag(dset, DCM_SeriesInstanceUID, "SeriesInstanceUID");
	if(!result->copy_tag(dset, DCM_SeriesDescription, "SeriesDescription")) {
		if(!result->copy_tag(dset, DCM_StudyDescription, "SeriesDescription")) {
			result->set_tag("SeriesDescription", gettext("no description"));
		}
	}

	result->copy_tag(dset, DCM_Modality, "Modality");
	if(!result->copy_tag(dset, DCM_SeriesTime, "SeriesTime")) {
		result->copy_tag(dset, DCM_StudyTime, "SeriesTime");
	}

	result->copy_tag(dset, DCM_StationName, "StationName");

	std::string buffer;
	dset->findAndGetOFString(DCM_NumberOfSeriesRelatedInstances, buffer);
	int i = atoi(buffer.c_str());
	if(i != 0) {
		result->m_instancecount = i;
	}

	return result;
}

static void on_query_from_net_result(DcmStack* resultstack, const std::string& server, const sigc::slot< void, const Glib::RefPtr< ImagePool::Study > >& resultslot) {
	for(unsigned int i=0; i<resultstack->card(); i++) {
		DcmDataset* dset = (DcmDataset*)resultstack->elem(i);
		resultslot(create_query_study(dset, server));
	}
}

void query_from_net(
			const std::string& patientid,
			const std::string& name,
			const std::string& modality,
			const std::string& date_from,
			const std::string& date_to,
			const std::string& studydescription,
			const std::string& stationname,
			const std::string& local_aet,
			const std::set<std::string>& groups,
			const sigc::slot< void, const Glib::RefPtr< ImagePool::Study >& >& resultslot
			)
{
	// get encodings
	std::string dicom_enc = ImagePool::get_encoding();
	std::string system_enc = ImagePool::get_system_encoding(dicom_enc);
	
	// create patientsname querystring
	std::string patientsname;

	if(name.empty()) {
		patientsname = "*";
	}
	else {
		patientsname = convert_string_to(name.c_str(), system_enc);
	}

	std::string description;
	if(!studydescription.empty()) {
		description = convert_string_to(studydescription.c_str(), system_enc);
	}
	
	// create date querystring
	std::string date;
	if(date_from.empty() && date_to.empty()) {
		date = "";
	}
	else if(date_to.empty()) {
		date = date_from + "-";
	}
	else if(date_from.empty()) {
		date = "-" + date_to;
	}
	else {
		date = date_from + "-" + date_to;
	}

	if(date_from == date_to) {
		date = date_from;
	}

	std::string station;
	if(!stationname.empty()) {
		station = "*" + convert_string_to(stationname.c_str(), system_enc) + "*";
	}

	DcmDataset query;
	DcmElement* e = NULL;
	
	e = newDicomElement(DCM_QueryRetrieveLevel);
	e->putString("STUDY");
	query.insert(e);

	e = newDicomElement(DCM_SpecificCharacterSet);
	e->putString(dicom_enc.c_str());
	query.insert(e);

	e = newDicomElement(DCM_PatientsName);
	e->putString(patientsname.c_str());
	query.insert(e);

	e = newDicomElement(DCM_PatientID);
	e->putString(convert_string_to(patientid.c_str(), system_enc).c_str());
	query.insert(e);

	e = newDicomElement(DCM_ModalitiesInStudy);
	e->putString(modality.c_str());
	query.insert(e);

	e = newDicomElement(DCM_PatientsBirthDate);
	query.insert(e);

	e = newDicomElement(DCM_PatientsSex);
	query.insert(e);

	e = newDicomElement(DCM_StudyDate);
	e->putString(date.c_str());
	query.insert(e);

	e = newDicomElement(DCM_StudyTime);
	query.insert(e);

	//e = newDicomElement(DCM_AccessionNumber);
	//query.insert(e);

	e = newDicomElement(DCM_StudyID);
	query.insert(e);

	e = newDicomElement(DCM_StudyInstanceUID);
	query.insert(e);

	e = newDicomElement(DCM_StudyDescription);
	e->putString(description.c_str());
	query.insert(e);

	e = newDicomElement(DCM_StationName);
	e->putString(station.c_str());
	query.insert(e);

	std::cout << "NEW QUERY:" << std::endl;
	query.print(COUT);

	NetClient<FindAssociation> a;
	a.signal_server_result.connect(sigc::bind(sigc::ptr_fun(on_query_from_net_result), resultslot));

	//std::set<std::string> groups = get_servergroups();
	std::set<std::string>::iterator i = groups.begin();

	// do we have groups defined ?
	if(groups.size() > 0) {
		while(i != groups.end()) {
			a.QueryServerGroup(&query, *i, local_aet);
			i++;
		}
	}
	
	// no query all servers
	else {
		a.QueryServerGroup(&query, "", local_aet);
	}
}

void query_series_from_net(const std::string& studyinstanceuid, const std::string& server, const std::string& local_aet, const sigc::slot< void, const Glib::RefPtr< ImagePool::Series >& >& resultslot) {
	DcmDataset query;
	DcmElement* e = NULL;
	
	e = newDicomElement(DCM_QueryRetrieveLevel);
	e->putString("SERIES");
	query.insert(e);

	e = newDicomElement(DCM_SpecificCharacterSet);
	query.insert(e);

	e = newDicomElement(DCM_StudyInstanceUID);
	e->putString(studyinstanceuid.c_str());
	query.insert(e);

	e = newDicomElement(DCM_SeriesInstanceUID);
	query.insert(e);

	e = newDicomElement(DCM_SeriesNumber);
	query.insert(e);

	e = newDicomElement(DCM_Modality);
	query.insert(e);

	e = newDicomElement(DCM_SeriesDescription);
	query.insert(e);

	e = newDicomElement(DCM_SeriesTime);
	query.insert(e);

	e = newDicomElement(DCM_StudyDescription);
	query.insert(e);

	e = newDicomElement(DCM_StudyTime);
	query.insert(e);

	e = newDicomElement(DCM_StationName);
	query.insert(e);

	e = newDicomElement(DCM_NumberOfSeriesRelatedInstances);
	query.insert(e);


	std::cout << "NEW QUERY:" << std::endl;
	query.print(COUT);

	NetClient<FindAssociation> a;
	a.QueryServer(&query, server, local_aet, UID_FINDStudyRootQueryRetrieveInformationModel);
	//a.QueryServers(&query, UID_FINDStudyRootQueryRetrieveInformationModel);

	DcmStack* result = a.GetResultStack();
	for(unsigned int i=0; i<result->card(); i++) {
		DcmDataset* dset = (DcmDataset*)result->elem(i);
		dset->print(COUT);
		resultslot(create_query_series(dset));
	}
}

int query_study_instances(const std::string& studyinstanceuid, const std::string& server, const std::string& local_aet) {
	DcmDataset query;
	DcmElement* e = NULL;
	
	e = newDicomElement(DCM_QueryRetrieveLevel);
	e->putString("IMAGE");
	query.insert(e);

	e = newDicomElement(DCM_StudyInstanceUID);
	e->putString(studyinstanceuid.c_str());
	query.insert(e);

	e = newDicomElement(DCM_SeriesInstanceUID);
	query.insert(e);

	e = newDicomElement(DCM_SOPInstanceUID);
	query.insert(e);

	e = newDicomElement(DCM_InstanceNumber);
	query.insert(e);

	std::cout << "NEW QUERY:" << std::endl;
	query.print(COUT);

	NetClient<FindAssociation> a;
	a.SetMaxResults(5000);
	a.QueryServer(&query, server, local_aet, UID_FINDStudyRootQueryRetrieveInformationModel);
	//a.QueryServers(&query, UID_FINDStudyRootQueryRetrieveInformationModel);

	DcmStack* result = a.GetResultStack();

	std::cout << result->card() << " Responses" << std::endl;
	
	return result->card();
}

int query_study_series(const std::string& studyinstanceuid, const std::string& server, const std::string& local_aet) {
	DcmDataset query;
	DcmElement* e = NULL;
	
	e = newDicomElement(DCM_QueryRetrieveLevel);
	e->putString("SERIES");
	query.insert(e);

	e = newDicomElement(DCM_StudyInstanceUID);
	e->putString(studyinstanceuid.c_str());
	query.insert(e);

	e = newDicomElement(DCM_SeriesInstanceUID);
	query.insert(e);

	e = newDicomElement(DCM_SeriesNumber);
	query.insert(e);

	std::cout << "NEW QUERY:" << std::endl;
	query.print(COUT);

	NetClient<FindAssociation> a;
	a.QueryServer(&query, server, local_aet, UID_FINDStudyRootQueryRetrieveInformationModel);

	DcmStack* result = a.GetResultStack();

	std::cout << result->card() << " Responses" << std::endl;
	
	return result->card();
}

} // namespace ImagePool
