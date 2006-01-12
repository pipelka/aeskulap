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
    Update Date:      $Date: 2006/01/12 15:41:23 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/imagepool/netquery.cpp,v $
    CVS/RCS Revision: $Revision: 1.17 $
    Status:           $State: Exp $
*/

#include <gconfmm.h>

#include "imagepool.h"
#include "dcdatset.h"
#include "dcdeftag.h"
#include "poolfindassociation.h"
#include "netclient.h"
#include "gettext.h"

#include <iostream>

namespace ImagePool {
	
static void fix_date(std::string& date) {
	if(date.size() != 8) {
		return;
	}
	
	std::string year = date.substr(0,4);
	std::string month = date.substr(4, 2);
	std::string day = date.substr(6, 2);
	
	date = day + "." + month + "." + year;
}

static void fix_time(std::string& time) {
	unsigned int i = time.find(".");
	if(i != std::string::npos) {
		time = time.substr(0, i);
	}
	if(time.size() != 6) {
		return;
	}

	std::string h = time.substr(0,2);
	std::string m = time.substr(2, 2);
	std::string s = time.substr(4, 2);
	
	time = h + ":" + m + ":" + s;
}

Glib::RefPtr< ImagePool::Study > create_query_study(DcmDataset* dset, const std::string& server) {
	Glib::RefPtr< ImagePool::Study > result = Glib::RefPtr< ImagePool::Study >(new Study);

	Glib::RefPtr< ImagePool::Instance > item = Instance::create(dset);

	result->m_server = server;
	result->m_studyinstanceuid = item->studyinstanceuid();
	result->m_patientsname = item->patientsname();
	result->m_patientsbirthdate = item->patientsbirthdate();
	result->m_patientssex = item->patientssex();
	result->m_studydescription = item->studydescription();
	result->m_studydate = item->studydate();
	result->m_studytime = item->studytime();
	
	fix_date(result->m_patientsbirthdate);
	fix_date(result->m_studydate);
	fix_time(result->m_studytime);

	return result;
}

Glib::RefPtr< ImagePool::Series > create_query_series(DcmDataset* dset) {
	Glib::RefPtr< ImagePool::Series > result = Glib::RefPtr< ImagePool::Series >(new Series);

	dset->findAndGetOFString(DCM_SeriesInstanceUID, result->m_seriesinstanceuid);
	dset->findAndGetOFString(DCM_SeriesDescription, result->m_description);
	if(result->m_description.empty()) {
		dset->findAndGetOFString(DCM_StudyDescription, result->m_description);
	}
	if(result->m_description.empty()) {
		result->m_description = gettext("no description");
	}

	dset->findAndGetOFString(DCM_Modality, result->m_modality);

	dset->findAndGetOFString(DCM_SeriesTime, result->m_seriestime);
	if(result->m_seriestime.empty()) {
		dset->findAndGetOFString(DCM_StudyTime, result->m_seriestime);
	}

	dset->findAndGetOFString(DCM_StationName, result->m_stationname);

	std::string buffer;
	dset->findAndGetOFString(DCM_NumberOfSeriesRelatedInstances, buffer);
	int i = atoi(buffer.c_str());
	if(i != 0) {
		result->m_instancecount = i;
	}

	fix_time(result->m_seriestime);

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
		patientsname = "*" + convert_string_to(name.c_str(), system_enc) + "*";
	}

	std::string description;
	if(!studydescription.empty()) {
		description = "*" + convert_string_to(studydescription.c_str(), system_enc) + "*";
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
			a.QueryServerGroup(&query, *i);
			i++;
		}
	}
	
	// no query all servers
	else {
		a.QueryServerGroup(&query, "");
	}
}

void query_series_from_net(const std::string& studyinstanceuid, const std::string& server, const sigc::slot< void, const Glib::RefPtr< ImagePool::Series >& >& resultslot) {
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

	Glib::RefPtr<Gnome::Conf::Client> client = Gnome::Conf::Client::get_default_client();

	NetClient<FindAssociation> a;
	a.QueryServer(&query, server, UID_FINDStudyRootQueryRetrieveInformationModel);
	//a.QueryServers(&query, UID_FINDStudyRootQueryRetrieveInformationModel);

	DcmStack* result = a.GetResultStack();
	for(unsigned int i=0; i<result->card(); i++) {
		DcmDataset* dset = (DcmDataset*)result->elem(i);
		dset->print(COUT);
		resultslot(create_query_series(dset));
	}
}

int query_study_instances(const std::string& studyinstanceuid, const std::string& server) {
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
	a.QueryServer(&query, server, UID_FINDStudyRootQueryRetrieveInformationModel);
	//a.QueryServers(&query, UID_FINDStudyRootQueryRetrieveInformationModel);

	DcmStack* result = a.GetResultStack();

	std::cout << result->card() << " Responses" << std::endl;
	
	return result->card();
}

int query_study_series(const std::string& studyinstanceuid, const std::string& server) {
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
	a.QueryServer(&query, server, UID_FINDStudyRootQueryRetrieveInformationModel);

	DcmStack* result = a.GetResultStack();

	std::cout << result->card() << " Responses" << std::endl;
	
	return result->card();
}

std::string get_ouraet() {
	Glib::RefPtr<Gnome::Conf::Client> client = Gnome::Conf::Client::get_default_client();
	return client->get_string("/apps/aeskulap/preferences/local_aet");
}

} // namespace ImagePool
