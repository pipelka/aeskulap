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
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/imagepool/Attic/DicomFindAssociation.h,v $
    CVS/RCS Revision: $Revision: 1.1 $
    Status:           $State: Exp $
*/

#ifndef DICOMFINDASSOCIATION_H
#define DICOMFINDASSOCIATION_H


#include "DicomAssociation.h"

class DicomFindAssociation : public DicomAssociation  
{
public:

	/**
	Constructors
	*/
	DicomFindAssociation();
	virtual ~DicomFindAssociation();

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

#endif
