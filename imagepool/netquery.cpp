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
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/imagepool/netquery.cpp,v $
    CVS/RCS Revision: $Revision: 1.1 $
    Status:           $State: Exp $
*/

#include <gconfmm.h>

#include "imagepool.h"
#include "dcdatset.h"
#include "dcdeftag.h"
#include "DicomFindAssociation.h"
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

Glib::RefPtr< ImagePool::Study > create_query_study(DcmDataset* dset) {
	Glib::RefPtr< ImagePool::Study > result = Glib::RefPtr< ImagePool::Study >(new Study);

	dset->findAndGetOFString(DCM_StudyInstanceUID, result->m_studyinstanceuid);
	dset->findAndGetOFString(DCM_PatientsName, result->m_patientsname);
	dset->findAndGetOFString(DCM_PatientsBirthDate, result->m_patientsbirthdate);
	dset->findAndGetOFString(DCM_PatientsSex, result->m_patientssex);
	dset->findAndGetOFString(DCM_StudyDescription, result->m_studydescription);
	dset->findAndGetOFString(DCM_StudyDate, result->m_studydate);
	dset->findAndGetOFString(DCM_StudyTime, result->m_studytime);

	if(result->m_studydescription.empty()) {
		result->m_studydescription = gettext("no description");
	}

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
		result->m_description = "no description";
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

void query_from_net(
			const std::string& patientid,
			const std::string& lastname,
			const std::string& firstname,
			const std::string& modality,
			const std::string& date_from,
			const std::string& date_to,
			const std::string& studydescription,
			const sigc::slot< void, const Glib::RefPtr< ImagePool::Study >& >& resultslot
			)
{
	// create patientsname querystring
	std::string patientsname;

	if(lastname.empty() && firstname.empty()) {
		patientsname = "*";
	}
	else if(firstname.empty()) {
		patientsname = lastname;
	}
	else if(lastname.empty()) {
		patientsname = "*^" + firstname;
	}
	else {
		patientsname = lastname + "^" + firstname;
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

	DcmDataset query;
	DcmElement* e = NULL;
	
	e = newDicomElement(DCM_QueryRetrieveLevel);
	e->putString("STUDY");
	query.insert(e);

	e = newDicomElement(DCM_PatientsName);
	e->putString(patientsname.c_str());
	query.insert(e);

	e = newDicomElement(DCM_PatientID);
	e->putString(patientid.c_str());
	query.insert(e);

	e = newDicomElement(DCM_Modality);
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

	e = newDicomElement(DCM_AccessionNumber);
	query.insert(e);

	e = newDicomElement(DCM_StudyID);
	query.insert(e);

	e = newDicomElement(DCM_StudyInstanceUID);
	query.insert(e);

	e = newDicomElement(DCM_StudyDescription);
	e->putString(studydescription.c_str());
	query.insert(e);

	std::cout << "NEW QUERY:" << std::endl;
	query.print(COUT);

	Glib::RefPtr<Gnome::Conf::Client> client = Gnome::Conf::Client::get_default_client();

	NetClient<DicomFindAssociation> a;
	a.QueryServers(&query);

	DcmStack* result = a.GetResultStack();
	for(unsigned int i=0; i<result->card(); i++) {
		DcmDataset* dset = (DcmDataset*)result->elem(i);
		resultslot(create_query_study(dset));
	}
}

void query_series_from_net(const std::string& studyinstanceuid, const sigc::slot< void, const Glib::RefPtr< ImagePool::Series >& >& resultslot) {
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

	NetClient<DicomFindAssociation> a;
	a.QueryServers(&query);

	DcmStack* result = a.GetResultStack();
	for(unsigned int i=0; i<result->card(); i++) {
		DcmDataset* dset = (DcmDataset*)result->elem(i);
		dset->print(COUT);
		resultslot(create_query_series(dset));
	}
}

} // namespace ImagePool
