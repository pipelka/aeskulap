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
    Update Date:      $Date: 2007/05/10 17:39:37 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/imagepool/netquery.cpp,v $
    CVS/RCS Revision: $Revision: 1.25 $
    Status:           $State: Exp $
*/

#include "imagepool.h"
#include "dcmtk/dcmdata/dcdatset.h"
#include "dcmtk/dcmdata/dcdeftag.h"
#include "poolfindassociation.h"
#include "netclient.h"
#include "../gettext.h"

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
	std::string::size_type i = time.find(".");
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

	if(item->studyrelatedinstances() != -1) {
		result->set_instancecount(-1, item->studyrelatedinstances());
	}
	if(item->studyrelatedseries() != -1) {
		result->set_seriescount(item->studyrelatedseries());
	}

	fix_date(result->m_patientsbirthdate);
	fix_date(result->m_studydate);
	fix_time(result->m_studytime);

	return result;
}

Glib::RefPtr< ImagePool::Series > create_query_series(DcmDataset* dset) {
	Glib::RefPtr< ImagePool::Series > result = Glib::RefPtr< ImagePool::Series >(new Series);

	OFString seriesUID;
	OFString desc;
	OFString ofstr;

	dset->findAndGetOFString(DCM_SeriesInstanceUID, seriesUID);
	dset->findAndGetOFString(DCM_SeriesDescription, desc);
	if(result->m_description.empty()) {
		dset->findAndGetOFString(DCM_StudyDescription, desc);
	}
	result->m_seriesinstanceuid = seriesUID.c_str();
    result->m_description = desc.c_str();

	if(result->m_description.empty()) {
		result->m_description = gettext("no description");
	}

	dset->findAndGetOFString(DCM_Modality, ofstr);
	result->m_modality = ofstr.c_str();

	dset->findAndGetOFString(DCM_SeriesTime, ofstr);
	result->m_seriestime = ofstr.c_str();
	if(result->m_seriestime.empty()) {
		dset->findAndGetOFString(DCM_StudyTime, ofstr);
		result->m_seriestime = ofstr.c_str();
	}

	dset->findAndGetOFString(DCM_StationName, ofstr);
    result->m_stationname = ofstr.c_str();

	dset->findAndGetOFString(DCM_NumberOfSeriesRelatedInstances, ofstr);
	int i = atoi(ofstr.c_str());
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

	e = newDicomElement(DCM_NumberOfStudyRelatedSeries);
	query.insert(e);

	e = newDicomElement(DCM_NumberOfStudyRelatedInstances);
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

	// StationName not allowed in StudyRoot
	/*e = newDicomElement(DCM_StationName);
	e->putString(station.c_str());
	query.insert(e);*/

	std::cout << "NEW QUERY:" << std::endl;
	query.print(COUT);

	NetClient<FindAssociation> a;
	a.signal_server_result.connect(sigc::bind(sigc::ptr_fun(on_query_from_net_result), resultslot));

	//std::set<std::string> groups = get_servergroups();
	std::set<std::string>::iterator i = groups.begin();

	// do we have groups defined ?
	if(groups.size() > 0) {
		while(i != groups.end()) {
			a.QueryServerGroup(&query, *i, local_aet, UID_FINDStudyRootQueryRetrieveInformationModel);
			i++;
		}
	}
	
	// no query all servers
	else {
		a.QueryServerGroup(&query, "", local_aet, UID_FINDStudyRootQueryRetrieveInformationModel);
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

	e = newDicomElement(DCM_StationName);
	query.insert(e);

	e = newDicomElement(DCM_NumberOfSeriesRelatedInstances);
	query.insert(e);


	std::cout << "NEW QUERY:" << std::endl;
	query.print(COUT);

	NetClient<FindAssociation> a;
	a.QueryServer(&query, server, local_aet, UID_FINDStudyRootQueryRetrieveInformationModel);

	DcmStack* result = a.GetResultStack();
	for(unsigned int i=0; i<result->card(); i++) {
		DcmDataset* dset = (DcmDataset*)result->elem(i);
		dset->print(COUT);
		resultslot(create_query_series(dset));
	}
}

/*
 * query the number instances of a study
 */
int query_study_instances(const std::string& studyinstanceuid, const std::string& server, const std::string& local_aet) {
	std::list<std::string> seriesinstanceuids;
	int sum = 0;

	if(query_study_series(studyinstanceuid, server, local_aet, seriesinstanceuids) == 0) {
		return 0;
	}
	
	for(std::list<std::string>::iterator i = seriesinstanceuids.begin(); i != seriesinstanceuids.end(); i++) {
		sum += query_series_instances(studyinstanceuid, *i, server, local_aet);
	} 
	
	std::cout << "query_study_instances = " << sum << std::endl;
	return sum;
}

/*
 * query the number instances of a series
 */
int query_series_instances(const std::string& studyinstanceuid, const std::string& seriesinstanceuid, const std::string& server, const std::string& local_aet) {
	DcmDataset query;
	DcmElement* e = NULL;
	
	e = newDicomElement(DCM_QueryRetrieveLevel);
	e->putString("IMAGE");
	query.insert(e);

	e = newDicomElement(DCM_StudyInstanceUID);
	e->putString(studyinstanceuid.c_str());
	query.insert(e);

	e = newDicomElement(DCM_SeriesInstanceUID);
	e->putString(seriesinstanceuid.c_str());
	query.insert(e);

	e = newDicomElement(DCM_SOPInstanceUID);
	query.insert(e);

	e = newDicomElement(DCM_InstanceNumber);
	query.insert(e);

	std::cout << "NEW QUERY:" << std::endl;
	query.print(COUT);

	NetClient<FindAssociation> a;
	a.QueryServer(&query, server, local_aet, UID_FINDStudyRootQueryRetrieveInformationModel);

	DcmStack* result = a.GetResultStack();
	std::cout << "query_series_instances = " << result->card() << std::endl;
 
	return result->card();
}

/*
 * query all seriesinstanceuid's and the number series of a study
 */
int query_study_series(const std::string& studyinstanceuid, const std::string& server, const std::string& local_aet, std::list<std::string>& seriesinstanceuids) {
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
	DcmDataset* dset;
	OFString ofstr;

	seriesinstanceuids.clear();
	for(int i = 0; i < result->card(); i++) {
		dset = (DcmDataset*)result->elem(i);
		if(dset->findAndGetOFString(DCM_SeriesInstanceUID, ofstr).good()) {
			seriesinstanceuids.push_back(ofstr.c_str());
		}
	}

	std::cout << result->card() << " Responses" << std::endl;
	int count = result->card();

	return count;
}

/*
 * query the number series of a study
 */
int query_study_series(const std::string& studyinstanceuid, const std::string& server, const std::string& local_aet) {
	std::list<std::string> dummy;
	return query_study_series(studyinstanceuid, server, local_aet, dummy);
}

} // namespace ImagePool
