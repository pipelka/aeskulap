/*
 *
 *  Copyright (C) 1993-2002, OFFIS
 *
 *  This software and supporting documentation were developed by
 *
 *    Kuratorium OFFIS e.V.
 *    Healthcare Information and Communication Systems
 *    Escherweg 2
 *    D-26121 Oldenburg, Germany
 *
 *  THIS SOFTWARE IS MADE AVAILABLE,  AS IS,  AND OFFIS MAKES NO  WARRANTY
 *  REGARDING  THE  SOFTWARE,  ITS  PERFORMANCE,  ITS  MERCHANTABILITY  OR
 *  FITNESS FOR ANY PARTICULAR USE, FREEDOM FROM ANY COMPUTER DISEASES  OR
 *  ITS CONFORMITY TO ANY SPECIFICATION. THE ENTIRE RISK AS TO QUALITY AND
 *  PERFORMANCE OF THE SOFTWARE IS WITH THE USER.
 *
 *  Module:  imagectn
 *
 *  Author:  Didier Lemoine
 *
 *  Purpose: routines which provide move facilities for the DB Module.
 *    Module Prefix: DB_ 
 *
 *  Last Update:      $Author: braindead $
 *  Update Date:      $Date: 2005/08/23 19:32:09 $
 *  Source File:      $Source: /cvsroot/aeskulap/aeskulap/dcmtk/imagectn/libsrc/dbmove.cc,v $
 *  CVS/RCS Revision: $Revision: 1.1 $
 *  Status:           $State: Exp $
 *
 *  CVS/RCS Log at end of file
 *
 */

#include "osconfig.h"    /* make sure OS specific configuration is included first */

#define INCLUDE_CSTDLIB
#define INCLUDE_CSTDIO
#define INCLUDE_CSTDARG
#include "ofstdinc.h"

#include "imagedb.h"
#include "dbpriv.h"
#include "dcdatset.h"
#include "dcdeftag.h"
#include "dimse.h"
#include "ofconsol.h"
#include "dbcond.h"

/************
 *	Test a Move Request List
 *	Returns EC_Normal if ok, else returns IMAGECTN_DB_ERROR
 */

OFCondition
DB_TestMoveRequestList (
		DB_ElementList 	*findRequestList,
		DB_LEVEL	queryLevel,
		DB_LEVEL	infLevel,
		DB_LEVEL	lowestLevel
		)
{
    DB_ElementList *plist ;
    DB_LEVEL 	XTagLevel ;
    DB_KEY_TYPE 	XTagType ;
    int	level ;

    /**** Query level must be at least the infLevel
    ***/

    if (queryLevel < infLevel) {
	DB_debug(1,"Level incompatible with Information Model (level %d)\n", (int)queryLevel) ;
	return IMAGECTN_DB_ERROR ;
    }

    if (queryLevel > lowestLevel) {
	DB_debug(1,"Level incompatible with Information Model (level %d)\n", (int)queryLevel) ;
	return IMAGECTN_DB_ERROR ;
    }

    for (level = PATIENT_LEVEL ; level <= IMAGE_LEVEL ; level++) {

	/**** Manage exception due to StudyRoot Information Model :
	**** In this information model, move may not include any
	**** Patient attributes.
	***/

	if ((level == PATIENT_LEVEL) && (infLevel == STUDY_LEVEL)) {

	    /** In Study Root Information Model, do not accept any 
	    ** Patient Tag
	    */

	    int atLeastOneKeyFound = OFFalse ;
	    for (plist = findRequestList ; plist ; plist = plist->next) {
		DB_GetTagLevel (plist->elem. XTag, &XTagLevel) ;
		if (XTagLevel != level)
		    continue ;
		atLeastOneKeyFound = OFTrue ;
	    }
	    if (atLeastOneKeyFound) {
		DB_debug(1,"Key found in Study Root Information Model (level %d)\n", level) ;
		return IMAGECTN_DB_ERROR ;
	    }
	}

	/**** If current level is above or equal to the QueryLevel
	***/

	else if (level <= queryLevel) {

	    /** For these levels, only unique keys are allowed
	    ** Parse the request list elements reffering to
	    ** this level.
	    ** Check that only unique key attr are provided
	    */

	    int uniqueKeyFound = OFFalse ;
	    for (plist = findRequestList ; plist ; plist = plist->next) {
		DB_GetTagLevel (plist->elem. XTag, &XTagLevel) ;
		if (XTagLevel != level)
		    continue ;
		DB_GetTagKeyAttr (plist->elem. XTag, &XTagType) ;
		if (XTagType != UNIQUE_KEY) {
		    DB_debug(1,"Non Unique Key found (level %d)\n", level) ;
		    return IMAGECTN_DB_ERROR ;
		}
		else if (uniqueKeyFound) {
		    DB_debug(1,"More than one Unique Key found (level %d)\n", level) ;
		    return IMAGECTN_DB_ERROR ;
		}
		else
		    uniqueKeyFound = OFTrue ;
	    }
	    if (! uniqueKeyFound) {
		DB_debug(1,"No Unique Key found (level %d)\n", level) ;
		return IMAGECTN_DB_ERROR ;
	    }
	}

	/**** If current level beyond the QueryLevel
	***/

	else if (level > queryLevel) {

	    /** For this level, no key is allowed
	    ** Parse the request list elements reffering to
	    ** this level.
	    ** Check that no key is provided
	    */

	    int atLeastOneKeyFound = OFFalse ;
	    for (plist = findRequestList ; plist ; plist = plist->next) {
		DB_GetTagLevel (plist->elem. XTag, &XTagLevel) ;
		if (XTagLevel != level)
		    continue ;
		atLeastOneKeyFound = OFTrue ;
	    }
	    if (atLeastOneKeyFound) {
		DB_debug(1,"Key found beyond query level (level %d)\n", level) ;
		return IMAGECTN_DB_ERROR ;
	    }
	}

    }
    return EC_Normal ;
}



OFCondition 
DB_startMoveRequest(
	DB_Handle 	*handle, 
	const char 	*SOPClassUID, 
	DcmDataset 	*moveRequestIdentifiers, 
	DB_Status 	*status) 
{

    DB_Private_Handle	*phandle = NULL;
    DB_SmallDcmElmt	elem ;
    DB_ElementList 	*plist = NULL;
    DB_ElementList 	*last = NULL;
    DB_CounterList	*pidxlist = NULL;
    DB_CounterList	*lastidxlist = NULL;
    int 		MatchFound = OFFalse;
    IdxRecord 		idxRec ;
    DB_LEVEL		qLevel = PATIENT_LEVEL; // highest legal level for a query in the current model
    DB_LEVEL            lLevel = IMAGE_LEVEL;   // lowest legal level for a query in the current model
    OFCondition		cond = EC_Normal;
    OFBool qrLevelFound = OFFalse;

    phandle = (DB_Private_Handle *) handle ; 

    /**** Is SOPClassUID supported ?
    ***/

    if (strcmp( SOPClassUID, UID_MOVEPatientRootQueryRetrieveInformationModel) == 0)
	phandle->rootLevel = PATIENT_ROOT ;
    else if (strcmp( SOPClassUID, UID_MOVEStudyRootQueryRetrieveInformationModel) == 0)
	phandle->rootLevel = STUDY_ROOT ;
#ifndef NO_PATIENTSTUDYONLY_SUPPORT
    else if (strcmp( SOPClassUID, UID_MOVEPatientStudyOnlyQueryRetrieveInformationModel) == 0)
        phandle->rootLevel = PATIENT_STUDY ;
#endif
#ifndef NO_GET_SUPPORT
    /* experimental support for GET */
    else if (strcmp( SOPClassUID, UID_GETPatientRootQueryRetrieveInformationModel) == 0)
	phandle->rootLevel = PATIENT_ROOT ;
    else if (strcmp( SOPClassUID, UID_GETStudyRootQueryRetrieveInformationModel) == 0)
	phandle->rootLevel = STUDY_ROOT ;
#ifndef NO_PATIENTSTUDYONLY_SUPPORT
    else if (strcmp( SOPClassUID, UID_GETPatientStudyOnlyQueryRetrieveInformationModel) == 0)
        phandle->rootLevel = PATIENT_STUDY ;
#endif
#endif    

    else {
	status->status = STATUS_MOVE_Failed_SOPClassNotSupported ;
	return (IMAGECTN_DB_ERROR) ;
    }


    /**** Parse Identifiers in the Dicom Object
    **** Find Query Level and contruct a list
    **** of query identifiers
    ***/
	
    int elemCount = (int)(moveRequestIdentifiers->card());
    for (int elemIndex=0; elemIndex<elemCount; elemIndex++) {

	DcmElement* dcelem = moveRequestIdentifiers->getElement(elemIndex);

	elem.XTag = dcelem->getTag().getXTag();
	if (elem.XTag == DCM_QueryRetrieveLevel || DB_TagSupported(elem.XTag)) {
	    elem.ValueLength = dcelem->getLength();
	    if (elem.ValueLength == 0) {
		elem.PValueField = NULL ;
	    } else if ((elem.PValueField = (char*)malloc((size_t)(elem.ValueLength+1))) == NULL) {
		status->status = STATUS_MOVE_Failed_UnableToProcess ;
		return (IMAGECTN_DB_ERROR) ;
	    } else {
		/* only char string type tags are supported at the moment */
		char *s = NULL;
		dcelem->getString(s);
		strcpy(elem.PValueField, s);
	    }

	    /** If element is the Query Level, store it in handle
	     */

	    if (elem. XTag == DCM_QueryRetrieveLevel) {
		char *pc ;
		char level [50] ;

		strncpy (level, (char *) elem. PValueField, (size_t)((elem. ValueLength < 50) ? elem. ValueLength : 49)) ;
		
		/*** Skip this two lines if you want strict comparison
		**/

		for (pc = level ; *pc ; pc++)
		    *pc = ((*pc >= 'a') && (*pc <= 'z')) ? 'A' - 'a' + *pc : *pc ;

		if (strncmp (level, PATIENT_LEVEL_STRING, 
			     strlen (PATIENT_LEVEL_STRING)) == 0)
		    phandle->queryLevel = PATIENT_LEVEL ;
		else if (strncmp (level, STUDY_LEVEL_STRING, 
				  strlen (STUDY_LEVEL_STRING)) == 0)
		    phandle->queryLevel = STUDY_LEVEL ;
		else if (strncmp (level, SERIE_LEVEL_STRING, 
				  strlen (SERIE_LEVEL_STRING)) == 0)
		    phandle->queryLevel = SERIE_LEVEL ;
		else if (strncmp (level, IMAGE_LEVEL_STRING, 
				  strlen (IMAGE_LEVEL_STRING)) == 0)
		    phandle->queryLevel = IMAGE_LEVEL ;
		else {
#ifdef DEBUG
		    DB_debug(1,"DB_startMoveRequest : STATUS_MOVE_Failed_UnableToProcess\n") ;
#endif
		    status->status = STATUS_MOVE_Failed_UnableToProcess ;
		    return (IMAGECTN_DB_ERROR) ;
		}
		qrLevelFound = OFTrue;
	    } else {
		/** Else it is a query identifier
		** Append it to our RequestList
		*/
		if (! DB_TagSupported (elem. XTag))
		    continue ;
			
		plist = (DB_ElementList *) malloc (sizeof( DB_ElementList ) ) ;
		if (plist == NULL) {
		    status->status = STATUS_FIND_Refused_OutOfResources ;
		    return (IMAGECTN_DB_ERROR) ;
		}
		plist->next = NULL ;
		DB_DuplicateElement (&elem, & (plist->elem)) ;
		if (phandle->findRequestList == NULL) {
		    phandle->findRequestList = last = plist ;
		} else {
		    last->next = plist ;
		    last = plist ;
		}
	    }

	    if ( elem. PValueField ) {
		free (elem. PValueField) ;
	    }
	}
    }

    if (!qrLevelFound) {
	/* The Query/Retrieve Level is missing */
	status->status = STATUS_MOVE_Failed_IdentifierDoesNotMatchSOPClass ;
	CERR << "DB_startMoveRequest(): missing Query/Retrieve Level" << endl;
	phandle->idxCounter = -1 ;
	DB_FreeElementList (phandle->findRequestList) ;
	phandle->findRequestList = NULL ;
	return (IMAGECTN_DB_ERROR) ;
    }

    switch (phandle->rootLevel)
    {
      case PATIENT_ROOT :       
      	qLevel = PATIENT_LEVEL ;        
      	lLevel = IMAGE_LEVEL ;        
      	break ;
      case STUDY_ROOT :   
      	qLevel = STUDY_LEVEL ;          
      	lLevel = IMAGE_LEVEL ;        
      	break ;
      case PATIENT_STUDY: 
        qLevel = PATIENT_LEVEL ;        
      	lLevel = STUDY_LEVEL ;        
        break ;
    }

    /**** Test the consistency of the request list
    ***/

    if (DB_doCheckMoveIdentifier()) {
	cond = DB_TestMoveRequestList (phandle->findRequestList, 
				       phandle->queryLevel, qLevel, lLevel) ;
	if (cond != EC_Normal) {
	    phandle->idxCounter = -1 ;
	    DB_FreeElementList (phandle->findRequestList) ;
	    phandle->findRequestList = NULL ;
#ifdef DEBUG
	    DB_debug(1,"DB_startMoveRequest () : STATUS_MOVE_Failed_IdentifierDoesNotMatchSOPClass - Invalid RequestList\n") ;
#endif
	    status->status = STATUS_MOVE_Failed_IdentifierDoesNotMatchSOPClass ;
	    return (cond) ;
	}
    }

    /**** Goto the beginning of Index File
    **** Then find all matching images
    ***/

    MatchFound = OFFalse ;
    phandle->moveCounterList = NULL ;
    phandle->NumberRemainOperations = 0 ;

    /**** Find matching images
    ***/

    DB_lock(phandle, OFFalse);

    DB_IdxInitLoop (phandle, &(phandle->idxCounter)) ;
    while (1) {

	/*** Exit loop if read error (or end of file)
	**/

	if (DB_IdxGetNext (phandle, &(phandle->idxCounter), &idxRec) != EC_Normal)
	    break ;

	/*** If matching found
	**/

	cond = DB_HierarchicalCompare (phandle, &idxRec, qLevel, qLevel, &MatchFound) ;
	if (MatchFound) {
	    pidxlist = (DB_CounterList *) malloc (sizeof( DB_CounterList ) ) ;
	    if (pidxlist == NULL) {
		status->status = STATUS_FIND_Refused_OutOfResources ;
		return (IMAGECTN_DB_ERROR) ;
	    }

	    pidxlist->next = NULL ;
	    pidxlist->idxCounter = phandle->idxCounter ;
	    phandle->NumberRemainOperations++ ;
	    if ( phandle->moveCounterList == NULL ) 
		phandle->moveCounterList = lastidxlist = pidxlist ;
	    else {
		lastidxlist->next = pidxlist ;
		lastidxlist = pidxlist ;
	    }

	}
    }

    DB_FreeElementList (phandle->findRequestList) ;
    phandle->findRequestList = NULL ;

    /**** If a matching image has been found,
    ****    status is pending
    ***/

    if ( phandle->NumberRemainOperations > 0 ) {
#ifdef DEBUG
	DB_debug(1,"DB_startMoveRequest : STATUS_Pending\n") ;
#endif
	status->status = STATUS_Pending ;
	return (EC_Normal) ;
    }

    /**** else no matching image has been found,
    ****    free query identifiers list
    ****    status is success
    ***/

    else {
	phandle->idxCounter = -1 ;
#ifdef DEBUG
	DB_debug(1,"DB_startMoveRequest : STATUS_Success\n") ;
#endif
	status->status = STATUS_Success ;

	DB_unlock(phandle);

	return (EC_Normal) ;
    }


}

OFCondition 
DB_nextMoveResponse(
		DB_Handle 	*handle, 
		char 		*SOPClassUID, 
		char 		*SOPInstanceUID, 
		char 		*imageFileName, 
		unsigned short 	*numberOfRemainingSubOperations,
		DB_Status 	*status) 
{
    DB_Private_Handle	*phandle ;
    IdxRecord 		idxRec ;
    DB_CounterList		*nextlist ;

    phandle = (DB_Private_Handle *) handle ; 

    /**** If all matching images have been retrieved,
    ****    status is success
    ***/

    if ( phandle->NumberRemainOperations <= 0 ) {
	status->status = STATUS_Success ;

	DB_unlock(phandle);

	return (EC_Normal) ;
    }

    /**** Goto the next matching image number of Index File
    ***/

    if (DB_IdxRead (phandle, phandle->moveCounterList->idxCounter, &idxRec) != EC_Normal) {
#ifdef DEBUG
	DB_debug(1,"DB_nextMoveResponse : STATUS_MOVE_Failed_UnableToProcess\n") ;
#endif
	status->status = STATUS_MOVE_Failed_UnableToProcess ;

	DB_unlock(phandle);

	return (IMAGECTN_DB_ERROR) ;
    }

    strcpy (SOPClassUID, (char *) idxRec. SOPClassUID) ;		
    strcpy (SOPInstanceUID, (char *) idxRec. SOPInstanceUID) ;		
    strcpy (imageFileName, (char *) idxRec. filename) ;

    *numberOfRemainingSubOperations = --phandle->NumberRemainOperations ;

			
    nextlist = phandle->moveCounterList->next ;
    free (phandle->moveCounterList) ;
    phandle->moveCounterList = nextlist ;
    status->status = STATUS_Pending ;
#ifdef DEBUG
    DB_debug(1,"DB_nextMoveResponse : STATUS_Pending\n") ;
#endif
    return (EC_Normal) ;
}


 
OFCondition 
DB_cancelMoveRequest (DB_Handle *handle, DB_Status *status) 
{
    DB_Private_Handle	*phandle ;
    DB_CounterList		*plist ;

    phandle = (DB_Private_Handle *) handle ; 

    while (phandle->moveCounterList) {
	plist  = phandle->moveCounterList ;
	phandle->moveCounterList = phandle->moveCounterList->next ;
	free (plist) ;
    }
		
    status->status = STATUS_MOVE_Cancel_SubOperationsTerminatedDueToCancelIndication ;

    DB_unlock(phandle);

    return (EC_Normal) ;
}



/*
** CVS Log
** $Log: dbmove.cc,v $
** Revision 1.1  2005/08/23 19:32:09  braindead
** - initial savannah import
**
** Revision 1.1  2005/06/26 19:26:14  pipelka
** - added dcmtk
**
** Revision 1.18  2004/02/27 13:36:26  meichel
** Imagectn now refuses find/get/move operations on SERIES or IMAGE level when
**   the Patient/Study Only Q/R model is used and identifier checking is enabled
**
** Revision 1.17  2002/11/27 13:27:55  meichel
** Adapted module imagectn to use of new header file ofstdinc.h
**
** Revision 1.16  2001/10/12 12:43:10  meichel
** Adapted imagectn to OFCondition based dcmnet module (supports strict mode).
**
** Revision 1.15  2001/09/26 16:06:38  meichel
** Adapted imagectn to class OFCondition
**
** Revision 1.14  2001/06/01 15:51:29  meichel
** Updated copyright header
**
** Revision 1.13  2000/03/08 16:41:09  meichel
** Updated copyright header.
**
** Revision 1.12  2000/03/03 14:16:39  meichel
** Implemented library support for redirecting error messages into memory
**   instead of printing them to stdout/stderr for GUI applications.
**
** Revision 1.11  2000/02/23 15:13:31  meichel
** Corrected macro for Borland C++ Builder 4 workaround.
**
** Revision 1.10  2000/02/01 11:43:45  meichel
** Avoiding to include <stdlib.h> as extern "C" on Borland C++ Builder 4,
**   workaround for bug in compiler header files.
**
** Revision 1.9  1999/06/10 12:12:17  meichel
** Adapted imagectn to new command line option scheme.
**   Added support for Patient/Study Only Q/R model and C-GET (experimental).
**
** Revision 1.8  1997/09/18 08:11:04  meichel
** Many minor type conflicts (e.g. long passed as int) solved.
**
** Revision 1.7  1997/07/21 08:59:57  andreas
** - Replace all boolean types (BOOLEAN, CTNBOOLEAN, DICOM_BOOL, BOOL)
**   with one unique boolean type OFBool.
**
** Revision 1.6  1997/06/26 12:52:01  andreas
** - Changed names for enumeration values in DB_KEY_TYPE since the value
**   OPTIONAL was predefined for Windows 95/NT
**
** Revision 1.5  1997/04/18 08:40:37  andreas
** - The put/get-methods for all VRs did not conform to the C++-Standard
**   draft. Some Compilers (e.g. SUN-C++ Compiler, Metroworks
**   CodeWarrier, etc.) create many warnings concerning the hiding of
**   overloaded get methods in all derived classes of DcmElement.
**   So the interface of all value representation classes in the
**   library are changed rapidly, e.g.
**   OFCondition get(Uint16 & value, const unsigned long pos);
**   becomes
**   OFCondition getUint16(Uint16 & value, const unsigned long pos);
**   All (retired) "returntype get(...)" methods are deleted.
**   For more information see dcmdata/include/dcelem.h
**
** Revision 1.4  1996/09/27 08:46:50  hewett
** Enclosed system include files with BEGIN_EXTERN_C/END_EXTERN_C.
**
** Revision 1.3  1996/04/27 12:55:18  hewett
** Removed cause of warnings when compiled with "c++ -O -g -Wall" under
** Solaris 2.4.  Mostly due to uninitialized variables.
**
** Revision 1.2  1996/04/22 11:20:35  hewett
** Now only processes supported attributes.  Added check for presence of
** Query/Retrieve Level.
**
** Revision 1.1.1.1  1996/03/28 19:24:59  hewett
** Oldenburg Image CTN Software ported to use the dcmdata C++ toolkit.
**
*/
