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

#include "poolnetwork.h"
#include "poolmoveassociation.h"



#include "dcmtk/dcmnet/dimse.h"
#include "dcmtk/dcmnet/diutil.h"
#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/dcmdata/dcuid.h"
#include "dcmtk/dcmdata/dcdict.h"
#include "dcmtk/dcmdata/cmdlnarg.h"
#include "dcmtk/ofstd/ofconapp.h"
#include "dcmtk/ofstd/ofstd.h"
#include "dcmtk/ofstd/ofdatime.h"
#include "dcmtk/dcmdata/dcuid.h"         /* for dcmtk version name */
#include "dcmtk/dcmnet/dicom.h"         /* for DICOM_APPLICATION_ACCEPTOR */
#include "dcmtk/dcmdata/dcdeftag.h"      /* for DCM_StudyInstanceUID */
#include "dcmtk/dcmdata/dcostrmz.h"      /* for dcmZlibCompressionLevel */
#include "dcmtk/dcmnet/dcasccfg.h"      /* for class DcmAssociationConfiguration */
#include "dcmtk/dcmnet/dcasccff.h"      /* for class DcmAssociationConfigurationFile */

#ifdef WITH_OPENSSL
#include "dcmtk/dcmtls/tlstrans.h"
#include "dcmtk/dcmtls/tlslayer.h"
#endif

#include "dcmtk/dcmjpeg/djencode.h"
#include "dcmtk/dcmjpeg/djrplol.h"


MoveAssociation::MoveAssociation() {
	m_abstractSyntax = UID_MOVEStudyRootQueryRetrieveInformationModel;
	m_maxReceivePDULength = ASC_DEFAULTMAXPDU;
}

MoveAssociation::~MoveAssociation() {
}

void MoveAssociation::Create(const std::string& title, const std::string& peer, int port, const std::string& ouraet, /*int ourPort,*/ const char *abstractSyntax/*, const char *abstractSyntaxMove*/) {
	Association::Create(title, peer, port, ouraet, abstractSyntax);
}

CONDITION MoveAssociation::SendObject(DcmDataset *dataset) {
	return moveSCU(dataset);	
}

void MoveAssociation::OnAddPresentationContext(T_ASC_Parameters *params, const char* transferSyntaxList[], int transferSyntaxListCount) {
	const char* mpeg_transfer[] = { UID_MPEG2MainProfileAtMainLevelTransferSyntax };

	ASC_addPresentationContext(params, 3, m_abstractSyntax, transferSyntaxList, transferSyntaxListCount);
	ASC_addPresentationContext(params, 5, m_abstractSyntax, mpeg_transfer, DIM_OF(mpeg_transfer));
}

CONDITION MoveAssociation::moveSCU(DcmDataset *pdset) {
	CONDITION cond;
	T_ASC_PresentationContextID presId;
	T_DIMSE_C_MoveRQ req;
	T_DIMSE_C_MoveRSP rsp;
	DIC_US msgId = assoc->nextMsgID++;
	DcmDataset* rspIds = NULL;
	const char* sopClass;
	DcmDataset* statusDetail = NULL;
	MoveCallbackInfo callbackData;

	if(pdset == NULL) {
		return DIMSE_NULLKEY;
	}

	//sopClass = m_abstractSyntaxMove;
	sopClass = m_abstractSyntax;

	// which presentation context should be used
	presId = ASC_findAcceptedPresentationContextID(assoc, sopClass);

	if (presId == 0) {
		return DIMSE_NOVALIDPRESENTATIONCONTEXTID;
	}

	callbackData.assoc = assoc;
	callbackData.presId = presId;
	callbackData.pCaller = this;

	req.MessageID = msgId;
	strcpy(req.AffectedSOPClassUID, sopClass);
	req.Priority = DIMSE_PRIORITY_HIGH;
	req.DataSetType = DIMSE_DATASET_PRESENT;
	strcpy(req.MoveDestination, m_ourAET.c_str());

	cond = DIMSE_moveUser(
						assoc,
						presId,
						&req,
						pdset,
						moveCallback,
						&callbackData,
						DIMSE_BLOCKING,
						0, 
						GetNetwork()->GetDcmtkNet(),
						subOpCallback,
						this,
						&rsp, &statusDetail, &rspIds);

	if (statusDetail != NULL) {
		printf("  Status Detail:\n");
		statusDetail->print(COUT);
		delete statusDetail;
	}

	if (rspIds != NULL) {
		delete rspIds;
	}

	return cond;
}

void MoveAssociation::moveCallback(void *callbackData, T_DIMSE_C_MoveRQ *request, int responseCount, T_DIMSE_C_MoveRSP *response) {
}

void MoveAssociation::subOpCallback(void *pCaller, T_ASC_Network *aNet, T_ASC_Association **subAssoc) {
	MoveAssociation* caller = (MoveAssociation*)pCaller;

	if (caller->GetNetwork() == NULL) {
		return;
	}

	if (*subAssoc == NULL) {
		// negotiate association
		caller->acceptSubAssoc(aNet, subAssoc);
	} 
	else{
		// be a service class provider
		caller->subOpSCP(subAssoc);
	}
}

CONDITION MoveAssociation::acceptSubAssoc(T_ASC_Network *aNet, T_ASC_Association **assoc) {
	CONDITION cond = ASC_NORMAL;
	const char* knownAbstractSyntaxes[] = { UID_VerificationSOPClass };
	const char* transferSyntaxes[] = { UID_JPEGProcess14SV1TransferSyntax, NULL, NULL, UID_LittleEndianImplicitTransferSyntax, UID_MPEG2MainProfileAtMainLevelTransferSyntax };

	if(m_accept_lossy) {
		transferSyntaxes[0] = UID_JPEGProcess2_4TransferSyntax;
	}

	cond = ASC_receiveAssociation(aNet, assoc, m_maxReceivePDULength);
	if (cond.bad()) {
		printf("Unable to receive association!\n");
		DimseCondition::dump(cond);
	}
	else {

		/* 
		** We prefer to accept Explicitly encoded transfer syntaxes.
		** If we are running on a Little Endian machine we prefer 
		** LittleEndianExplicitTransferSyntax to BigEndianTransferSyntax.
		*/

		/* gLocalByteOrder is defined in dcxfer.h */
		if (gLocalByteOrder == EBO_LittleEndian) {
			/* we are on a little endian machine */
			transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
			transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
		} else {
			/* we are on a big endian machine */
			transferSyntaxes[1] = UID_BigEndianExplicitTransferSyntax;
			transferSyntaxes[2] = UID_LittleEndianExplicitTransferSyntax;
		}

		// accept the Verification SOP Class if presented */
		cond = ASC_acceptContextsWithPreferredTransferSyntaxes(
					(*assoc)->params,
					knownAbstractSyntaxes, DIM_OF(knownAbstractSyntaxes),
					transferSyntaxes, DIM_OF(transferSyntaxes));

		if (cond.good()) {
			// the array of Storage SOP Class UIDs comes from dcuid.h
			cond = ASC_acceptContextsWithPreferredTransferSyntaxes(
					(*assoc)->params,
					dcmAllStorageSOPClassUIDs, numberOfAllDcmStorageSOPClassUIDs,
					transferSyntaxes, DIM_OF(transferSyntaxes));
		}
	}

	if (cond.good()) {
		cond = ASC_acknowledgeAssociation(*assoc);
	}

	if (cond.bad()) {
		ASC_dropAssociation(*assoc);
		ASC_destroyAssociation(assoc);
	}

	return cond;
	
}

CONDITION MoveAssociation::subOpSCP(T_ASC_Association **subAssoc) {
	T_DIMSE_Message msg;
	T_ASC_PresentationContextID presID;

	/* just in case */
	if (!ASC_dataWaiting(*subAssoc, 0)) {
		return DIMSE_NODATAAVAILABLE;
	}

	OFCondition cond = DIMSE_receiveCommand(*subAssoc, DIMSE_BLOCKING, 0, &presID, &msg, NULL);

	if (cond == EC_Normal) {
		switch (msg.CommandField) {
			case DIMSE_C_STORE_RQ:
				cond = storeSCP(*subAssoc, &msg, presID);
				break;
			case DIMSE_C_ECHO_RQ:
				cond = echoSCP(*subAssoc, &msg, presID);
				break;
			default:
				cond = DIMSE_BADCOMMANDTYPE;
				break;
		}
	}

	// clean up on association termination
	if (cond == DUL_PEERREQUESTEDRELEASE) {
		cond = ASC_acknowledgeRelease(*subAssoc);
		ASC_dropSCPAssociation(*subAssoc);
		ASC_destroyAssociation(subAssoc);
		return cond;
	}
	else if (cond == DUL_PEERABORTEDASSOCIATION) {
	}
	else if (cond != EC_Normal) {
		DimseCondition::dump(cond);
		// some kind of error so abort the association
		cond = ASC_abortAssociation(*subAssoc);
	}

	if (cond != EC_Normal) {
		ASC_dropAssociation(*subAssoc);
		ASC_destroyAssociation(subAssoc);
	}
	return cond;
}

CONDITION MoveAssociation::storeSCP(T_ASC_Association *assoc, T_DIMSE_Message *msg, T_ASC_PresentationContextID presID) {
	CONDITION cond;
	T_DIMSE_C_StoreRQ* req;
	DcmDataset *dset = new DcmDataset;

	req = &msg->msg.CStoreRQ;

	StoreCallbackInfo callbackData;
	callbackData.dataset = dset;
	callbackData.pCaller = this;

	cond = DIMSE_storeProvider(assoc, presID, req, (char *)NULL, 1, 
				&dset, storeSCPCallback, (void*)&callbackData,
				DIMSE_BLOCKING, 0);

	delete dset;
	return cond;
}

void MoveAssociation::storeSCPCallback(void *callbackData, T_DIMSE_StoreProgress *progress, T_DIMSE_C_StoreRQ *req, char *imageFileName, DcmDataset **imageDataSet, T_DIMSE_C_StoreRSP *rsp, DcmDataset **statusDetail) {
	DIC_UI sopClass;
	DIC_UI sopInstance;

	StoreCallbackInfo *cbdata = (StoreCallbackInfo*) callbackData;
	MoveAssociation* caller = cbdata->pCaller;

	if (progress->state == DIMSE_StoreEnd) {
		*statusDetail = NULL;	/* no status detail */

		/* could save the image somewhere else, put it in database, etc */
		rsp->DimseStatus = STATUS_Success;

		if ((imageDataSet)&&(*imageDataSet)) {
			// do not duplicate the dataset, let the user do this
			// if he wants to
			caller->OnResponseReceived(cbdata->dataset);
		}

		/* should really check the image to make sure it is consistent,
		* that its sopClass and sopInstance correspond with those in
		* the request.
		*/
		if (rsp->DimseStatus == STATUS_Success) {
			/* which SOP class and SOP instance ? */
			if (! DU_findSOPClassAndInstanceInDataSet(cbdata->dataset, sopClass, sopInstance)) {
				rsp->DimseStatus = STATUS_STORE_Error_CannotUnderstand;
			}
			else if (strcmp(sopClass, req->AffectedSOPClassUID) != 0) {
				rsp->DimseStatus = STATUS_STORE_Error_DataSetDoesNotMatchSOPClass;
			}
			else if (strcmp(sopInstance, req->AffectedSOPInstanceUID) != 0) {
				rsp->DimseStatus = STATUS_STORE_Error_DataSetDoesNotMatchSOPClass;
			}
		}

	}

}

CONDITION MoveAssociation::echoSCP(T_ASC_Association *assoc, T_DIMSE_Message *msg, T_ASC_PresentationContextID presID) {
	CONDITION cond;

	// the echo succeeded !!
	cond = DIMSE_sendEchoResponse(assoc, presID, &msg->msg.CEchoRQ, STATUS_Success, NULL);

	return cond;
}

void MoveAssociation::OnResponseReceived(DcmDataset* response) {
}
