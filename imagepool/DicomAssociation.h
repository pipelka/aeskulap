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
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/imagepool/Attic/DicomAssociation.h,v $
    CVS/RCS Revision: $Revision: 1.1 $
    Status:           $State: Exp $
*/

#ifndef DICOMASSOCIATION_H
#define DICOMASSOCIATION_H

// dcmtk includes
#include "osconfig.h"
#include <assoc.h>
#include <cond.h>
#include <dimse.h>
#include <dcfilefo.h>

class DicomNetwork;

class DicomAssociation  
{
public:

	/**
	Constructors
	*/
	DicomAssociation();
	virtual ~DicomAssociation();

	/**
	Create the association object (connect through DicomNetwork::Connect(..) )
	*/
	void Create(const char *title, const char *peer, int port, const char* ouraet, const char *abstractSyntax = NULL);

	/**
	Connect the association to a dicom network
	*/
	CONDITION Connect(DicomNetwork* network, int lossy = 0);

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
	DicomNetwork* GetNetwork();

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
	static bool AddQueryLevel(DcmDataset* query, const char* level);

	static void SetPatientData(
			DcmDataset* dset,
			const char* PatientsName,
			const char* PatientID,
			const char* PatientsBirthDate,
			const char* PatientsSex);

	static void SetSOPInstanceUID(DcmDataset* dset, const char* sop);
				
	const char* GetOurAET();

	void SetTimeout(int t);

	int GetTimeout();

	void SetCompressionQuality(int q);
	
	int GetCompressionQuality();
	
	void SetProposeCompression(bool propose);
	
	bool GetProposeCompression();

protected:

	/**
	Callback function to add user defined presentation context to association parameters
	*/
	virtual void OnAddPresentationContext(T_ASC_Parameters *params, const char* transferSyntaxList[], int transferSyntaxListCount);

	/**
	Protected data
	*/

	char* m_abstractSyntax;
	const char* m_calledAET;
	const char* m_calledPeer;
	const char* m_ourAET;
	int m_calledPort;
	int m_timeout;

	T_ASC_Association* assoc;
	T_ASC_PresentationContextID presId;
	DIC_UI sopClass;
	DIC_UI sopInstance;
	DIC_US msgId;

protected:

	/**
	Private data
	*/
	DicomNetwork* dcmNet;
	int m_CompressionQuality;
	bool m_ProposeCompression;

	friend class DicomNetwork;
};

#endif
