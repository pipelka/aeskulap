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
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/imagepool/Attic/DicomMoveAssociation.h,v $
    CVS/RCS Revision: $Revision: 1.1 $
    Status:           $State: Exp $
*/

#ifndef DICOMMOVEASSOCIATION_H
#define DICOMMOVEASSOCIATION_H

#include "DicomFindAssociation.h"

class DicomMoveAssociation : public DicomFindAssociation {
public:

	DicomMoveAssociation();
	virtual ~DicomMoveAssociation();

	void Create(const char *title, const char *peer, int port, const char *ouraet, /*int ourPort,*/ const char *abstractSyntax = UID_MOVEPatientRootQueryRetrieveInformationModel/*, const char *abstractSyntaxMove = UID_MOVEPatientRootQueryRetrieveInformationModel*/);

	CONDITION SendObject(DcmDataset *dataset);

protected:

	void OnAddPresentationContext(T_ASC_Parameters *params, const char* transferSyntaxList[], int transferSyntaxListCount);

	virtual void OnResponseReceived(DcmDataset* response);

	int m_maxReceivePDULength;

private:

	typedef struct _MoveCallbackInfo {
		T_ASC_Association *assoc;
		T_ASC_PresentationContextID presId;
		DicomMoveAssociation* pCaller;
	} MoveCallbackInfo;

	typedef struct _StoreCallbackInfo {
		DcmDataset* dataset;
		DicomMoveAssociation* pCaller;
	} StoreCallbackInfo;

	CONDITION moveSCU(DcmDataset *pdset);
	static void moveCallback(void *callbackData, T_DIMSE_C_MoveRQ *request, int responseCount, T_DIMSE_C_MoveRSP *response);

	CONDITION acceptSubAssoc(T_ASC_Network *aNet, T_ASC_Association **assoc);
	CONDITION subOpSCP(T_ASC_Association **subAssoc);
	static void subOpCallback(void * pCaller, T_ASC_Network *aNet, T_ASC_Association **subAssoc);

	CONDITION storeSCP(T_ASC_Association *assoc, T_DIMSE_Message *msg, T_ASC_PresentationContextID presID);
	static void storeSCPCallback(void *callbackData, T_DIMSE_StoreProgress *progress, T_DIMSE_C_StoreRQ *req, char *imageFileName, DcmDataset **imageDataSet, T_DIMSE_C_StoreRSP *rsp, DcmDataset **statusDetail);

	CONDITION echoSCP(T_ASC_Association *assoc, T_DIMSE_Message *msg, T_ASC_PresentationContextID presID);
};

#endif
