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
#include <diutil.h>

// ImagePool Network class header
#include "poolnetwork.h"

T_ASC_Network* Network::net = NULL;


Network::Network() {
}

Network::~Network() {
	DropNetwork();
}

CONDITION Network::ConnectAssociation(Association* assoc, int lossy) {
	CONDITION cond;

	cond = ASC_ConnectAssociation(
						assoc,
						assoc->m_calledAET,
						assoc->m_calledPeer,
						assoc->m_calledPort,
						assoc->m_ourAET,
						assoc->m_abstractSyntax,
						lossy);

	if (!SUCCESS(cond)) {
		assoc->Drop(cond);
		return cond;
	}

	assoc->dcmNet = this;
	assoc->msgId = assoc->assoc->nextMsgID; //++;
	
	return cond;
}

CONDITION Network::InitializeNetwork(int timeout, int port) {
#ifdef _WIN32
	WORD wVersionRequested;
	WSADATA wsaData;
	
	wVersionRequested = MAKEWORD( 1, 1 );
 
	WSAStartup( wVersionRequested, &wsaData );
#endif

	return ASC_initializeNetwork(NET_ACCEPTORREQUESTOR, port, timeout, &net);
}

CONDITION Network::DropNetwork()
{
	CONDITION cond = ASC_dropNetwork(&net);
	net = NULL;

#ifdef _WIN32
    WSACleanup();
#endif

	return cond;
}

CONDITION Network::ASC_ConnectAssociation(Association* assoc, const std::string& peerTitle, const std::string& peer, int port, const std::string& ouraet, const char *abstractSyntax, int lossy)
{
    CONDITION cond;
    T_ASC_Parameters *params;
	bool bProposeCompression = assoc->GetProposeCompression();

	DIC_NODENAME peerHost;
    DIC_NODENAME localHost;

    cond = ASC_createAssociationParameters(&params, ASC_DEFAULTMAXPDU);
    if (!SUCCESS(cond)) 
	{
		return cond;
    }
    ASC_setAPTitles(params, ouraet.c_str(), peerTitle.c_str(), NULL);

    gethostname(localHost, sizeof(localHost) - 1);
    snprintf(peerHost, sizeof(peerHost), "%s:%d", peer.c_str(), port);
    ASC_setPresentationAddresses(params, localHost, peerHost);

	if(abstractSyntax == NULL)
	{
		cond = addAllStoragePresentationContexts(params, bProposeCompression, lossy);
		if (!SUCCESS(cond)) 
		{
			return cond;
		}
	}
	else
	{
		const char** transferSyntaxes;
		int transferSyntaxes_count;
		const char* const_transferSyntaxes[] = { UID_LittleEndianExplicitTransferSyntax, UID_BigEndianExplicitTransferSyntax, UID_LittleEndianImplicitTransferSyntax };

		transferSyntaxes = &const_transferSyntaxes[0];
		transferSyntaxes_count = DIM_OF(const_transferSyntaxes);

/*		const char* const_transferSyntaxes[] = { UID_JPEGProcess14SV1TransferSyntax, UID_LittleEndianExplicitTransferSyntax, UID_BigEndianExplicitTransferSyntax, UID_LittleEndianImplicitTransferSyntax };
		if(bProposeCompression) {
			if(lossy == 8) {
				const_transferSyntaxes[0] = UID_JPEGProcess1TransferSyntax;
			}
			if(lossy == 12) {
				const_transferSyntaxes[0] = UID_JPEGProcess2_4TransferSyntax;
			}
		}
		const_transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
		const_transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;

		if(bProposeCompression) {
			transferSyntaxes = &const_transferSyntaxes[0];
			transferSyntaxes_count = DIM_OF(const_transferSyntaxes);
		}
		else {
			transferSyntaxes = &const_transferSyntaxes[1];
			transferSyntaxes_count = DIM_OF(const_transferSyntaxes)-1;
		}*/
	
		cond = ASC_addPresentationContext(params, 1, abstractSyntax, transferSyntaxes, transferSyntaxes_count);
		assoc->OnAddPresentationContext(params, transferSyntaxes, transferSyntaxes_count);
	}


    /* create association */
    cond = ASC_requestAssociation(net, params, &(assoc->assoc));
    if (cond != ASC_NORMAL)
	{
		if (cond == ASC_ASSOCIATIONREJECTED) 
		{
			T_ASC_RejectParameters rej;

			ASC_getRejectParameters(params, &rej);
			ASC_printRejectParameters(stderr, &rej);
			return cond;
		}
		else
		{
			return cond;
		}
    }

    /* what has been accepted/refused ? */

    if (ASC_countAcceptedPresentationContexts(params) == 0) 
	{
		return cond;
    }

	return ASC_NORMAL;
}

CONDITION Network::addAllStoragePresentationContexts(T_ASC_Parameters *params, bool bProposeCompression, int lossy)
{
    CONDITION cond = ASC_NORMAL;
    int i;
    int pid = 1;

    /* 
    ** We prefer to accept Explicitly encoded transfer syntaxes.
    ** If we are running on a Little Endian machine we prefer 
    ** LittleEndianExplicitTransferSyntax to BigEndianTransferSyntax.
    ** Some SCP implementations will just select the first transfer
    ** syntax they support (this is not part of the standard) so
    ** organise the proposed transfer syntaxes to take advantage
    ** of such behaviour.
    */

	const char** transferSyntaxes;
	int transferSyntaxes_count;
    const char* const_transferSyntaxes[] = {UID_JPEGProcess14SV1TransferSyntax, NULL, NULL, UID_LittleEndianImplicitTransferSyntax };

	if(bProposeCompression) {
		if(lossy == 8) {
			const_transferSyntaxes[0] = UID_JPEGProcess1TransferSyntax;
		}
		
		if(lossy == 12) {
			const_transferSyntaxes[0] = UID_JPEGProcess2_4TransferSyntax;
		}
	}
	
    /* gLocalByteOrder is defined in dcxfer.h */
    if (gLocalByteOrder == EBO_LittleEndian) {
		/* we are on a little endian machine */
		const_transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
		const_transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
    } else {
		/* we are on a big endian machine */
		const_transferSyntaxes[1] = UID_BigEndianExplicitTransferSyntax;
		const_transferSyntaxes[2] = UID_LittleEndianExplicitTransferSyntax;
    }

	if(bProposeCompression) {
		transferSyntaxes = &const_transferSyntaxes[0];
		transferSyntaxes_count = DIM_OF(const_transferSyntaxes);
	}
	else {
		transferSyntaxes = &const_transferSyntaxes[1];
		transferSyntaxes_count = DIM_OF(const_transferSyntaxes)-1;
	}

	/* the array of Storage SOP Class UIDs comes from dcuid.h */
	for (i=0; i<numberOfDcmStorageSOPClassUIDs && SUCCESS(cond); i++) {
		cond = ASC_addPresentationContext(params, pid, dcmStorageSOPClassUIDs[i], transferSyntaxes, transferSyntaxes_count);
		pid += 2;	/* only odd presentation context id's */
	}

    return cond;
}

bool Network::SendEchoRequest(const std::string& title, const std::string& peer, int port, const std::string& ouraet)
{
	Association dcmEcho;
	dcmEcho.Create(title, peer, port, ouraet, UID_VerificationSOPClass);

	ConnectAssociation(&dcmEcho);

	return dcmEcho.SendEchoRequest();
}

T_ASC_Network* Network::GetDcmtkNet() {
	return net;
}

void Network::SetDcmtkNet(T_ASC_Network* n) {
	net = n;
}
