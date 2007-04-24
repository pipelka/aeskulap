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

// dcmtk includes
#include "dcmtk/ofstd/ofstdinc.h"
#include <dcmtk/dcmnet/diutil.h>
#include "dcmtk/dcmdata/dcdeftag.h"

#include "dcmtk/dcmdata/dcuid.h"     /* for dcmtk version name */
#include "dcmtk/dcmjpeg/djdecode.h"  /* for dcmjpeg decoders */
#include "dcmtk/dcmjpeg/djencode.h"  /* for dcmjpeg encoders */
#include "dcmtk/dcmjpeg/djrplol.h"   /* for DJ_RPLossless */
#include "dcmtk/dcmjpeg/djrploss.h"  /* for DJ_RPLossy */
#include "dcmtk/dcmjpeg/dipijpeg.h"  /* for dcmimage JPEG plugin */
#include "dcmtk/dcmimage/diregist.h"  /* include to support color images */
#include "dcmtk/ofstd/ofcmdln.h"

// Association class header
#include "poolassociation.h"
#include "poolnetwork.h"

#include "dcmtk/dcmjpeg/djencode.h"
#include "dcmtk/dcmjpeg/djdecode.h"
#include "dcmtk/dcmjpeg/djrplol.h"
#include "dcmtk/dcmjpeg/djrploss.h"


Association::Association() :
m_abstractSyntax(NULL),
m_timeout(0),
m_accept_lossy(false),
assoc(NULL),
presId(0),
msgId(0),
dcmNet(NULL)
{
	sopClass[0] = 0;
	sopInstance[0] = 0;
	m_CompressionQuality = 70;
	m_ProposeCompression = true;
}

Association::~Association()
{
	// drop an existing association on shutdown
	if(assoc != NULL)
		Drop();
}

CONDITION Association::Drop(CONDITION cond) {
	// tear down association
	if(cond == DIMSE_NORMAL) {
		/* release association */
		cond = ASC_releaseAssociation(assoc);
	}
	else if(cond == DIMSE_PEERREQUESTEDRELEASE) {
		cond = ASC_abortAssociation(assoc);
		if (SUCCESS(cond)) {
			return cond;
		}
	}
	else if(cond == DIMSE_PEERABORTEDASSOCIATION) {
		return cond;
	}
	else {
		cond = ASC_abortAssociation(assoc);
		if (SUCCESS(cond)) {
			return cond;
		}
	}

	Destroy();
	return cond;
}

void Association::Destroy() {
	CONDITION cond = ASC_destroyAssociation(&assoc);

	dcmNet = NULL;
	assoc = NULL;
	msgId = 0;
	presId = 0;
	sopClass[0] = 0;
	sopInstance[0] = 0;

}

CONDITION Association::SendObject(DcmDataset *dataset) {
	CONDITION cond;
	DcmDataset *statusDetail = NULL;

	T_DIMSE_C_StoreRQ req;
	T_DIMSE_C_StoreRSP rsp;

	// check if we SOPClass and SOPInstance in dataset
	if (!DU_findSOPClassAndInstanceInDataSet(dataset, sopClass, sopInstance))
	{
		return DIMSE_BADDATA;
	}

	/* which presentation context should be used */
	presId = ASC_findAcceptedPresentationContextID(assoc, sopClass);
	if (presId == 0)
	{
		const char *modalityName = dcmSOPClassUIDToModality(sopClass);
		if (!modalityName) modalityName = dcmFindNameOfUID(sopClass);
		if (!modalityName) modalityName = "unknown SOP class";
		return DIMSE_BADDATA;
	}

	// init store

	bzero((char*)&req, sizeof(req));
	req.MessageID = msgId;
	strncpy(req.AffectedSOPClassUID, sopClass, sizeof(req.AffectedSOPClassUID));
	strncpy(req.AffectedSOPInstanceUID, sopInstance, sizeof(req.AffectedSOPInstanceUID));
	req.DataSetType = DIMSE_DATASET_PRESENT;
	req.Priority = DIMSE_PRIORITY_LOW;

	// convert to accepted transfer syntax
	T_ASC_PresentationContext pc;
	cond = ASC_findAcceptedPresentationContext(assoc->params, presId, &pc);
	ASC_dumpPresentationContext(&pc, COUT);

	DJEncoderRegistration::registerCodecs(
		ECC_lossyYCbCr,
		EUC_never,					// UID generation (never create new UID's)
		OFFalse,							// verbose
		OFTrue);							// optimize huffman table
	
	DJDecoderRegistration::registerCodecs();

	DcmXfer opt_oxferSyn(pc.acceptedTransferSyntax);
	E_TransferSyntax ori_oxferSyn = dataset->getOriginalXfer();

    DcmXfer original_xfer(dataset->getOriginalXfer());

	if(opt_oxferSyn.getXfer() != ori_oxferSyn) {
		std::cout << "Converting object to accepted transfer-syntax " << opt_oxferSyn.getXferName() << std::endl;
	
		CONDITION cond;
		// create RepresentationParameter
		DJ_RPLossless rp_lossless(6, 0);
		DJ_RPLossy rp_lossy(m_CompressionQuality);

		// NEW
		
		const DcmRepresentationParameter *rp = NULL;

		if(opt_oxferSyn.getXfer() == EXS_JPEGProcess14SV1TransferSyntax || opt_oxferSyn.getXfer() == EXS_JPEGProcess14TransferSyntax) {
			rp = &rp_lossless;
		}
		else if(opt_oxferSyn.getXfer() == EXS_JPEGProcess1TransferSyntax || opt_oxferSyn.getXfer() == EXS_JPEGProcess2_4TransferSyntax) {
			rp = &rp_lossy;
		}

		// recompress ?
		if(rp != NULL) {
		    if(original_xfer.isEncapsulated()) {
				std::cout << "DICOM file is already compressed, convert to uncompressed xfer syntax first\n";
				if(EC_Normal != dataset->chooseRepresentation(EXS_LittleEndianExplicit, NULL)) {
					std::cout << "No conversion from compressed original to uncompressed xfer syntax possible!\n";
				}
			}
		}

		cond = dataset->chooseRepresentation(opt_oxferSyn.getXfer(), rp);
		if(cond.bad()) {
			DimseCondition::dump(cond);
		}
	
		if (dataset->canWriteXfer(opt_oxferSyn.getXfer())) {
			std::cout << "Output transfer syntax " << opt_oxferSyn.getXferName() << " can be written" << std::endl;
		}
		else {
			std::cout << "No conversion to transfer syntax " << opt_oxferSyn.getXferName() << " possible!" << std::endl;
		}
	}

	// store it

	cond = DIMSE_storeUser(
				assoc, 
				presId, 
				&req, 
				NULL, 
				dataset, 
				NULL,
				NULL, 
				(m_timeout == 0) ? DIMSE_BLOCKING : DIMSE_NONBLOCKING,
				m_timeout, 
				&rsp, 
				&statusDetail);
	
	// increase message id
	msgId++;

	// what happened

	if(rsp.DataSetType == DIMSE_DATASET_PRESENT) {
		printf("Response with dataset:\n");
	}
	
	if (statusDetail != NULL) {
		printf("Status detail:\n");
		statusDetail->print(COUT, false);
		delete statusDetail;
	}

	if (cond != DIMSE_NORMAL)
	{
		return cond;
	}

	return (rsp.DimseStatus == STATUS_Success) ? DIMSE_NORMAL : DIMSE_BADDATA;
}

CONDITION Association::SendObject(DcmFileFormat *dcmff)
{
	DcmDataset* dataset = dcmff->getDataset();
	return SendObject(dataset);
}

void Association::Create(const std::string& title, const std::string& peer, int port, const std::string& ouraet, const char *abstractSyntax)
{
	// no connected association till now
	assoc = NULL;

	// fill in parameters
	if(abstractSyntax != NULL){
		m_abstractSyntax = (char*)abstractSyntax;
	}

	m_calledAET = title;
	m_calledPeer = peer;
	m_calledPort = port;
	
	m_ourAET = ouraet;

	msgId = 0;
	presId = 0;
}

bool Association::SendEchoRequest()
{
	DIC_US status;
	DcmDataset *statusDetail = NULL;

	OFCondition cond = DIMSE_echoUser(assoc, ++msgId, DIMSE_BLOCKING, 0, &status, &statusDetail);
	if (cond.good()) {
		std::cout << "Complete [Status: " << DU_cstoreStatusString(status) << "]" << std::endl;
	} else {
		std::cout << "Failed:" << std::endl;
		DimseCondition::dump(cond);
	}
 
	if(statusDetail != NULL) {
		delete statusDetail;
	}

	return cond.good();
}

bool Association::AddKey(DcmDataset *query, const DcmTagKey& tag, int value) {
	static char temp[16];
	snprintf(temp, sizeof(temp), "%i", value);
	return AddKey(query, tag, temp);
}

bool Association::AddKey(DcmDataset *query, const DcmTagKey& tag, double value, const char* format) {
	static char temp[16];
	snprintf(temp, sizeof(temp), format, value);
	return AddKey(query, tag, temp);
}

bool Association::AddKey(DcmItem *query, const DcmTagKey& tag, int value) {
	static char temp[16];
	snprintf(temp, sizeof(temp), "%i", value);
	return AddKey(query, tag, temp);
}

bool Association::AddKey(DcmItem *query, const DcmTagKey& tag, double value, const char* format) {
	static char temp[16];
	snprintf(temp, sizeof(temp), format, value);
	return AddKey(query, tag, temp);
}

bool Association::AddKey(DcmDataset *query, const DcmTagKey& t, const char* value) {
	return AddCustomKey/*< DcmDataset >*/(query, t, value);
}

bool Association::AddKey(DcmItem *query, const DcmTagKey& t, const char* value) {
	return AddCustomKey/*< DcmItem >*/(query, t, value);
}

bool Association::AddQueryLevel(DcmDataset *query, const std::string& level)
{
	return AddKey(query, DCM_QueryRetrieveLevel, level.c_str());
}


void Association::OnAddPresentationContext(T_ASC_Parameters *params, const char* transferSyntaxList[], int transferSyntaxListCount)
{

}

Network* Association::GetNetwork()
{
	return dcmNet;
}

CONDITION Association::Connect(Network *network, int lossy)
{
	dcmNet = network;
	return network->ConnectAssociation(this, lossy);
}

const std::string& Association::GetOurAET() {
	return m_ourAET;
}

const char* Association::GetKey(DcmDataset* query, const DcmTagKey& tag) {
	OFString val;
	static char buffer[129];
	query->findAndGetOFString(tag, val, 0, OFTrue);
	strncpy(buffer, val.c_str(), sizeof(buffer));
	return buffer;
}

void Association::SetTimeout(int t) {
	m_timeout = t;
}

int Association::GetTimeout() {
	return m_timeout;
}

void Association::SetCompressionQuality(int q) {
	m_CompressionQuality = q;
}
	
int Association::GetCompressionQuality() {
	return m_CompressionQuality;
}

void Association::SetProposeCompression(bool propose) {
	m_ProposeCompression = propose;
}

bool Association::GetProposeCompression() {
	return m_ProposeCompression;
}

void Association::SetAcceptLossyImages(bool lossy) {
	m_accept_lossy = lossy;
}
