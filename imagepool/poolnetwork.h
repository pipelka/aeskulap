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

#ifndef IMAGEPOOL_NETWORK_H
#define IMAGEPOOL_NETWORK_H

#include "poolassociation.h"

class Network {
public:

	/**
	constructors
	*/
	Network();
	virtual ~Network();

	/**
	Initialize the dicom network
	*/
	CONDITION InitializeNetwork(int timeout=20, int port = 0);

	/**
	Drop the dicom network
	*/
	CONDITION DropNetwork();

	/**
	Connect an association to the specified host
	*/
	CONDITION ConnectAssociation(Association* assoc, int lossy = 0);

	/**
	Send C-Echo request to dicom node
	*/
	bool SendEchoRequest(const char* title, const char* peer, int port, const char* ouraet);

	/**
	Get the pointer to the internal dcmtk network variable (sorry)
	*/
	T_ASC_Network* GetDcmtkNet();

	void SetDcmtkNet(T_ASC_Network* n);
	
	/**
	static const char* GetCurrDate();
	*/

	/**
	static const char* GetCurrTime();
	*/

private:

	/**
	Add all possible presentation contexts to association parameters
	*/
	static CONDITION addAllStoragePresentationContexts(T_ASC_Parameters *params, bool bProposeCompression = true, int lossy = 0);

	/**
	Connect to a host and try to establish an association
	*/
	CONDITION ASC_ConnectAssociation(Association* assoc, const char* peerTitle, const char* peer, int port, const char* ouraet, const char *abstractSyntax = NULL, int lossy = 0);

	/**
	THE dicom network
	*/
	static T_ASC_Network* net;

	friend class Association;
};

#endif // IMAGEPOOL_NETWORK_H
