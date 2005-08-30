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

    Last Update:      $Author$
    Update Date:      $Date$
    Source File:      $Source$
    CVS/RCS Revision: $Revision$
    Status:           $State$
*/

#include "dcdebug.h"
#include "poolfindassociation.h"


FindAssociation::FindAssociation() {
	Association::m_abstractSyntax = UID_FINDPatientRootQueryRetrieveInformationModel;
	maxResults = 400;
	bPushResults = true;
}

FindAssociation::~FindAssociation() {
	DeleteResultStack();
}

CONDITION FindAssociation::findSCU(T_ASC_Association *assoc, DcmDataset *query) {
	CONDITION cond;
	DIC_US msgId = assoc->nextMsgID++;
	T_ASC_PresentationContextID presId;
	T_DIMSE_C_FindRQ req;
	T_DIMSE_C_FindRSP rsp;
	DcmDataset *statusDetail = NULL;

	if(query == NULL) {
		return DIMSE_BADDATA;
	}

	DeleteResultStack();

	/* which presentation context should be used */
	presId = ASC_findAcceptedPresentationContextID(assoc, m_abstractSyntax);
	if (presId == 0) {
		return DIMSE_NOVALIDPRESENTATIONCONTEXTID;
	}

	req.MessageID = msgId;
	req.DataSetType = DIMSE_DATASET_PRESENT;
	req.Priority = DIMSE_PRIORITY_LOW;
	strcpy(req.AffectedSOPClassUID, m_abstractSyntax);

	cond = DIMSE_findUser(assoc, presId, &req, query, 
			findCallback, (void*)this, 
			(m_timeout == 0) ? DIMSE_BLOCKING : DIMSE_NONBLOCKING,
			m_timeout, 
			&rsp, &statusDetail);
	
	
	if (cond == DIMSE_NORMAL)  {
			if (rsp.DimseStatus == STATUS_Success)  {
			}
	}

	if (statusDetail != NULL) {
		printf("  Status Detail:\n");
			statusDetail->print(COUT);
			delete statusDetail;
	}
	return cond;
}

void FindAssociation::findCallback(void* callbackData, T_DIMSE_C_FindRQ*, int responseCount, T_DIMSE_C_FindRSP*, DcmDataset *responseIdentifiers) {
	FindAssociation* caller = (FindAssociation*)callbackData;

	if(responseCount > caller->maxResults) {
		return;
	}
	
	DcmDataset* response = new DcmDataset(*responseIdentifiers);

	if(caller->bPushResults) {	
		caller->result.push(response);
	}
	caller->OnResponseReceived(response);
}

CONDITION FindAssociation::SendObject(DcmDataset *dataset) {
	return findSCU(assoc, dataset);
}

void FindAssociation::OnResponseReceived(DcmDataset *response) {
}

DcmStack* FindAssociation::GetResultStack() {
	return &result;
}

DcmStack FindAssociation::CopyResultStack() {
	DcmStack copy;

	for(unsigned int i=0; i<result.card(); i++) {
		DcmDataset* dset = new DcmDataset(*(DcmDataset*)result.elem(i));
		copy.push(dset);
	}
	
	return copy;
}

void FindAssociation::DeleteResultStack() {
	DcmDataset* obj = NULL;
	unsigned int size = result.card();

	for(Uint32 i=0; i < size; i++){
		obj = (DcmDataset*)result.elem(i);
		delete obj;
	}

	result.clear();
}

void FindAssociation::SetMaxResults(int max) {
	maxResults = max;
}

int FindAssociation::GetMaxResults() {
	return maxResults;
}
