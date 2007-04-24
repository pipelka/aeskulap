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

#ifndef IMAGEPOOL_ASSOCIATION_H
#define IMAGEPOOL_ASSOCIATION_H

// dcmtk includes
#include "dcmtk/config/osconfig.h"
#include <dcmtk/dcmnet/assoc.h>
#include <dcmtk/dcmnet/cond.h>
#include <dcmtk/dcmnet/dimse.h>
#include <dcmtk/dcmdata/dcfilefo.h>

class Network;

class Association  
{
public:

	/**
	Constructors
	*/
	Association();
	virtual ~Association();

	/**
	Create the association object (connect through DicomNetwork::Connect(..) )
	*/
	void Create(const std::string& title, const std::string& peer, int port, const std::string& ouraet, const char *abstractSyntax = NULL);

	/**
	Connect the association to a dicom network
	*/
	CONDITION Connect(Network* network, int lossy = 0);

	void Destroy();
	
	/**
	Drop the association
	*/
	CONDITION Drop(CONDITION cond=DIMSE_NORMAL);

	/**
	Send a dataset through the association (C-Store)
	*/
	virtual CONDITION SendObject(DcmDataset* dataset);

	/**
	Send a fileformat object through the association (C-Store)
	*/
	virtual CONDITION SendObject(DcmFileFormat* dcmff);

	/**
	Send a C-Echo request through the association
	*/
	bool SendEchoRequest();

	/**
	Return the DicomNetwork this association is connected to
	*/
	Network* GetNetwork();

	/**
	add a query key to a dataset
	*/

	static bool AddKey(DcmItem *query, const DcmTagKey& tag, const char* value=NULL);

	static bool AddKey(DcmItem *query, const DcmTagKey& tag, int value);
	static bool AddKey(DcmItem *query, const DcmTagKey& tag, double value, const char* format = "%lf");
	
	static bool AddKey(DcmDataset *query, const DcmTagKey& tag, const char* value=NULL);
	
	//template< class T >
	static bool AddCustomKey(DcmItem* query, const DcmTagKey& t, const char* value) {
		DcmTag tag(t);
		Uint16 g = tag.getGTag();
		Uint16 e = tag.getETag();

		if (tag.error() != EC_Normal) {
			printf("unknown tag: (%04x,%04x)", g, e);
			return false;
			}

		DcmElement *elem = newDicomElement(tag);
		if (elem == NULL) {
			printf("cannot create element for tag: (%04x,%04x)", g, e);
			return false;
		}

		if(value != NULL) {
			if (strlen(value) > 0) {
				elem->putString(value);

				if (elem->error() != EC_Normal) {
					printf("cannot put tag value: (%04x,%04x)=\"%s\"", g, e, value);
					return false;
				}
			}
		}

		delete query->remove(t);
		query->insert(elem, OFTrue);

		return true;
	}
	
	static bool AddKey(DcmDataset *query, const DcmTagKey& tag, int value);
	static bool AddKey(DcmDataset *query, const DcmTagKey& tag, double value, const char* format = "%lf");

	/**
	get a key from the dataset
	*/
	static const char* GetKey(DcmDataset* query, const DcmTagKey& tag);
	
	/**
	add a query level to a dataset
	*/
	static bool AddQueryLevel(DcmDataset* query, const std::string& level);

	const std::string& GetOurAET();

	void SetTimeout(int t);

	int GetTimeout();

	void SetCompressionQuality(int q);
	
	int GetCompressionQuality();
	
	void SetProposeCompression(bool propose);
	
	bool GetProposeCompression();

	void SetAcceptLossyImages(bool lossy);
	
protected:

	/**
	Callback function to add user defined presentation context to association parameters
	*/
	virtual void OnAddPresentationContext(T_ASC_Parameters *params, const char* transferSyntaxList[], int transferSyntaxListCount);

	/**
	Protected data
	*/

	char* m_abstractSyntax;
	std::string m_calledAET;
	std::string m_calledPeer;
	std::string m_ourAET;

	int m_calledPort;
	int m_timeout;
	bool m_accept_lossy;

	T_ASC_Association* assoc;
	T_ASC_PresentationContextID presId;
	DIC_UI sopClass;
	DIC_UI sopInstance;
	DIC_US msgId;

protected:

	/**
	Private data
	*/
	Network* dcmNet;
	int m_CompressionQuality;
	bool m_ProposeCompression;

	friend class Network;
};

#endif
