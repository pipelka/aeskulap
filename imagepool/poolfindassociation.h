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

#ifndef IMAGEPOOL_FINDASSOCIATION_H
#define IMAGEPOOL_FINDASSOCIATION_H


#include "poolassociation.h"

class FindAssociation : public Association  
{
public:

	/**
	Constructors
	*/
	FindAssociation();
	virtual ~FindAssociation();

	/**
	Send a query object (C-Find) through association
	*/
	CONDITION SendObject(DcmDataset *dataset);

	/**
	Get the result stack of the last query
	*/
	virtual DcmStack* GetResultStack();

	DcmStack CopyResultStack();

	/**
	Delete all objects from the result stack
	*/
	virtual void DeleteResultStack();

	/**
	Set the maximum number of results
	*/
	void SetMaxResults(int max);

	/**
	Get the maximum number of results
	*/
	int GetMaxResults();
	
protected:

	/**
	Response handler
	*/
	virtual void OnResponseReceived(DcmDataset* response);

	/**
	Protected data
	*/
	DcmStack result;

	bool bPushResults;

private:

	/**
	C-Find service class user
	*/
	CONDITION findSCU(T_ASC_Association * assoc, DcmDataset * query);

	/**
	Callback function for C-Find service class user
	*/
	static void findCallback(void*, T_DIMSE_C_FindRQ*, int responseCount, T_DIMSE_C_FindRSP*, DcmDataset *responseIdentifiers);
	int maxResults;
	
};

#endif // IMAGEPOOL_FINDASSOCIATION_H
