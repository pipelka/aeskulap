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
 *  Purpose: routines which provide find facilities for the DB Module.
 *    Module Prefix: DB_ 
 *
 *  Last Update:      $Author: braindead $
 *  Update Date:      $Date: 2005/08/23 19:32:09 $
 *  Source File:      $Source: /cvsroot/aeskulap/aeskulap/dcmtk/imagectn/libsrc/dbfind.cc,v $
 *  CVS/RCS Revision: $Revision: 1.1 $
 *  Status:           $State: Exp $
 *
 *  CVS/RCS Log at end of file
 *
 */

#include "osconfig.h"    /* make sure OS specific configuration is included first */

#define INCLUDE_CSTDLIB
#define INCLUDE_CSTDIO
#define INCLUDE_CSTRING
#define INCLUDE_CSTDARG
#include "ofstdinc.h"

#include "imagedb.h"
#include "dbpriv.h"
#include "diutil.h"
#include "dcompat.h"
#include "dimse.h"
#include "ofconsol.h"
#include "dbcond.h"

/************
**	Try to match Two Dates
**	The first one is the "model", the second one an element
**	Returns OFTrue if matching is ok, else returns OFFalse
 */

int
DB_MatchDate (DB_SmallDcmElmt *mod, DB_SmallDcmElmt *elt)
{
    char date [DBC_MAXSTRING] ;
    char modl [DBC_MAXSTRING] ;

    /*** Get elt and model data in strings
    **/

    memcpy (date, elt->PValueField, (size_t)(elt->ValueLength)) ;
    date [elt->ValueLength] = '\0' ;
    DB_RemoveSpaces (date) ;

    memcpy (modl, mod->PValueField, (size_t)(mod->ValueLength)) ;
    modl [mod->ValueLength] = '\0' ;
    DB_RemoveSpaces (modl) ;

    /*** If no '-' in date
    *** return strict comparison result
    **/

    if (strchr (modl, '-') == NULL)
	return (strcmp (modl, date) == 0) ;

    /*** If first car is -
    **/

    if (modl [0] == '-') {
	return DB_DateToLong (date) <= DB_DateToLong (modl+1) ;
    }

    /*** If last car is -
    **/

    else if (modl [strlen (modl) - 1] == '-') {
	modl [strlen (modl) - 1] = '\0' ;
	return DB_DateToLong (date) >= DB_DateToLong (modl) ;
    }
    else {
	char *pc ;
	long d ;

	d = DB_DateToLong (date) ;
	pc = strchr (modl, '-') ;
	*pc = '\0' ;

	return (d >= DB_DateToLong (modl)) && (d <= DB_DateToLong (pc+1)) ;

    }
}

/************
**	Try to match Two Times
**	The first one is the "model", the second one an element
**	Returns OFTrue if matching is ok, else returns OFFalse
 */

int
DB_MatchTime (DB_SmallDcmElmt *mod, DB_SmallDcmElmt *elt)
{
    char aTime [DBC_MAXSTRING] ;
    char modl [DBC_MAXSTRING] ;

    /*** Get elt and model data in strings
    **/

    memcpy (aTime, elt->PValueField, (size_t)(elt->ValueLength)) ;
    aTime [elt->ValueLength] = '\0' ;
    DB_RemoveSpaces (aTime) ;

    memcpy (modl, mod->PValueField, (size_t)(mod->ValueLength)) ;
    modl [mod->ValueLength] = '\0' ;
    DB_RemoveSpaces (modl) ;

    /*** If no '-' in time
    *** return strict comparison result
    **/

    if (strchr (modl, '-') == NULL)
	return (strcmp (modl, aTime) == 0) ;

    /*** If first car is -
    **/

    if (modl [0] == '-') {
	return DB_TimeToDouble (aTime) <= DB_TimeToDouble (modl+1) ;
    }

    /*** If last car is -
    **/

    else if (modl [strlen (modl) - 1] == '-') {
	modl [strlen (modl) - 1] = '\0' ;
	return DB_TimeToDouble (aTime) >= DB_TimeToDouble (modl) ;
    }
    else {
	char *pc ;
	double t ;

	t = DB_TimeToDouble (aTime) ;
	pc = strchr (modl, '-') ;
	*pc = '\0' ;

	return (t >= DB_TimeToDouble (modl)) && (t <= DB_TimeToDouble (pc+1)) ;

    }
}

/************
**	Try to match Two UID
**	The first one is the "model", the second one an element
**	Returns OFTrue if matching is ok, else returns OFFalse
 */

int
DB_MatchUID (DB_SmallDcmElmt *mod, DB_SmallDcmElmt *elt)
{
    int match ;
    char *uid  ;
    char *modl  ;
    char *pc ;

    /*** Get elt and model data in strings
    **/

    uid = (char *) malloc ((size_t)(elt->ValueLength + 1)) ;
    if (uid == NULL) {
	return 0 ;
    }
    memcpy (uid, elt->PValueField, (size_t)(elt->ValueLength)) ;
    uid [elt->ValueLength] = '\0' ;

    modl = (char *) malloc ((size_t)(mod->ValueLength + 1)) ;
    if (modl == NULL) {
	free (uid) ;
	return 0 ;
    }
    memcpy (modl, mod->PValueField, (size_t)(mod->ValueLength)) ;
    modl [mod->ValueLength] = '\0' ;


    /*** If no '\' in model
    *** return strict comparison result
    **/

#ifdef STRICT_COMPARE
#else
    /*** Suppress Leading and Trailing spaces in
    *** model and string
    **/

    DB_RemoveEnclosingSpaces (uid) ;
    DB_RemoveEnclosingSpaces (modl) ;
#endif

    if (strchr (modl, '\\') == NULL) {
	match = (strcmp (modl, uid) == 0) ;
	free (uid) ;
	free (modl) ;
	return (match) ;
    }

    /*** UID List comparaison.
    *** Match is successful if uid is found in model
    **/

    match = OFFalse ; ;
    for (pc = modl ; *pc ; ) {
	if (strncmp (pc, uid, strlen (uid)) == 0) {
	    match = OFTrue ;
	    break ;
	}
	else {
	    pc = strchr (pc, '\\') ;
	    if (pc == NULL)
		break ;
	    else
		pc++ ;
	}
    }

    free (uid) ;
    free (modl) ;
    return (match) ;

}

/************
**	Try to match Two Strings
**	The first one is the "model", the second one an element
**	Returns OFTrue if matching is ok, else returns OFFalse
 */

int
DB_MatchStrings (DB_SmallDcmElmt *mod, DB_SmallDcmElmt *elt)
{
    int match ;
    char *string  ;
    char *modl  ;

    /*** Get elt and model data in strings
    **/

    string = (char *) malloc ((size_t)(elt->ValueLength + 1)) ;
    if (string == NULL) {
	return 0 ;
    }
    memcpy (string, elt->PValueField, (size_t)(elt->ValueLength)) ;
    string [elt->ValueLength] = '\0' ;

    modl = (char *) malloc ((size_t)(mod->ValueLength + 1)) ;
    if (modl == NULL) {
	free (string) ;
	return 0 ;
    }
    memcpy (modl, mod->PValueField, (size_t)(mod->ValueLength)) ;
    modl [mod->ValueLength] = '\0' ;

#ifdef STRICT_COMPARE
#else
    /*** Suppress Leading and Trailing spaces in
    *** model and string
    **/

    DB_RemoveEnclosingSpaces (string) ;
    DB_RemoveEnclosingSpaces (modl) ;
#endif

    /*** If no '*' and no '?' in model
    *** return strict comparison result
    **/

    if ((strchr (modl, '*') == NULL) && (strchr (modl, '?') == NULL))
	return (strcmp (modl, string) == 0) ;

    match = DB_StringUnify (modl, string) ;

    free (string) ;
    free (modl) ;
    return (match) ;

}

/************
**	Try to match Two Unknown elements
**	Strict comparaison is applied
**	The first one is the "model", the second one an element
**	Returns OFTrue if matching is ok, else returns OFFalse
 */

int
DB_MatchOther (DB_SmallDcmElmt *mod, DB_SmallDcmElmt *elt)
{
    if (mod->ValueLength != elt->ValueLength)
	return OFFalse ;

    return (memcmp (mod->PValueField, elt->PValueField, (size_t)(elt->ValueLength)) == 0) ;
}

/************
**	Try to match Two DB_SmallDcmElmts
**	The first one is the "model", the second one an element
**	Returns OFTrue if matching is ok, else returns OFFalse
 */

int
DB_Match (DB_SmallDcmElmt *mod, DB_SmallDcmElmt *elt)
{
    DB_KEY_CLASS keyClass ;

    /*** If model length is 0
    *** Universal matching is applied : return always OFTrue
    **/

    if (mod->ValueLength == 0)
	return (OFTrue) ;

    /*** Get the key class of the element
    **/

    DB_GetTagKeyClass (elt->XTag, &keyClass) ;

    switch (keyClass) {

    case DATE_CLASS :
	return DB_MatchDate (mod, elt)  ;

    case TIME_CLASS :
	return DB_MatchTime (mod, elt)  ;

    case UID_CLASS :
	return DB_MatchUID  (mod, elt) ;

    case STRING_CLASS :
	return DB_MatchStrings (mod, elt) ;

    case OTHER_CLASS :
	return DB_MatchOther (mod, elt) ;

    }
    return OFFalse;
}

/************
**	Create the response list in specified handle,
**	using informations found in an index record.
**	Old response list is supposed freed
**/

void
DB_MakeResponseList (
		DB_Private_Handle	*phandle,
		IdxRecord 		*idxRec
		)
{
    int i ;
    DB_ElementList *pRequestList = NULL;
    DB_ElementList *plist = NULL;
    DB_ElementList *last = NULL;

    phandle->findResponseList = NULL ;

    /*** For each element in Request identifier
    **/

    for (pRequestList = phandle->findRequestList ; pRequestList ; pRequestList = pRequestList->next) {

	/*** Find Corresponding Tag in index record
	**/

	for (i = 0 ; i < NBPARAMETERS ; i++)
	    if (idxRec->param [i]. XTag == pRequestList->elem. XTag)
		break ;

	/*** If Tag not found, skip the element
	**/

	if (i >= NBPARAMETERS)
	    continue ;

	/*** Append index record element to response list
	**/

	plist = (DB_ElementList *) malloc (sizeof (DB_ElementList)) ;
	if (plist == NULL) {
	    CERR << "DB_MakeResponseList: out of memory" << endl;
	    return;
	}
	plist->next = NULL ;

	DB_DuplicateElement(&idxRec->param[i], &plist->elem);

	if (phandle->findResponseList == NULL) {
	    phandle->findResponseList = last = plist ;
	}
	else {
	    last->next = plist ;
	    last = plist ;
	}

    }
}

/***********
** 
*/

static char*
DB_strdup(const char* str)
{
    if (str == NULL) return NULL;
    char* s = (char*)malloc(strlen(str)+1);
    strcpy(s, str);
    return s;
}

/************
**	Add UID in Index Record to the UID found list
 */

void
DB_UIDAddFound (
		DB_Private_Handle	*phandle,
		IdxRecord 		*idxRec
		)
{
    DB_UidList *plist ;

    plist = (DB_UidList *) malloc (sizeof (DB_UidList)) ;
    if (plist == NULL) {
	CERR << "DB_UIDAddFound: out of memory" << endl;
	return;
    }
    plist->next = phandle->uidList ;
    plist->patient = NULL ;
    plist->study = NULL ;
    plist->serie = NULL ;
    plist->image = NULL ;

    if ((int)phandle->queryLevel >= PATIENT_LEVEL)
	plist->patient = DB_strdup ((char *) idxRec->PatientID) ;
    if ((int)phandle->queryLevel >= STUDY_LEVEL)
	plist->study = DB_strdup ((char *) idxRec->StudyInstanceUID) ;
    if ((int)phandle->queryLevel >= SERIE_LEVEL)
	plist->serie = DB_strdup ((char *) idxRec->SeriesInstanceUID) ;
    if ((int)phandle->queryLevel >= IMAGE_LEVEL)
	plist->image = DB_strdup ((char *) idxRec->SOPInstanceUID) ;

    phandle->uidList = plist ;
}


/************
**	Search if an Index Record has already been found
 */

int
DB_UIDAlreadyFound (
		DB_Private_Handle	*phandle,
		IdxRecord 		*idxRec
		)
{
    DB_UidList *plist ;

    for (plist = phandle->uidList ; plist ; plist = plist->next) {
	if (  ((int)phandle->queryLevel >= PATIENT_LEVEL)
	      && (strcmp (plist->patient, (char *) idxRec->PatientID) != 0)
	    )
	    continue ;
	if (  ((int)phandle->queryLevel >= STUDY_LEVEL)
	      && (strcmp (plist->study, (char *) idxRec->StudyInstanceUID) != 0)
	    )
	    continue ;
	if (  ((int)phandle->queryLevel >= SERIE_LEVEL)
	      && (strcmp (plist->serie, (char *) idxRec->SeriesInstanceUID) != 0)
	    )
	    continue ;
	if (  ((int)phandle->queryLevel >= IMAGE_LEVEL)
	      && (strcmp (plist->image, (char *) idxRec->SOPInstanceUID) != 0)
	    )
	    continue ;
	return (OFTrue) ;
    }
    return (OFFalse) ;
}

/************
**	Test a Find Request List
**	Returns EC_Normal if ok, else returns IMAGECTN_DB_ERROR
 */

OFCondition
DB_TestFindRequestList (
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
	DB_debug(1,"Level incompatible with Information Model (level %d)\n", queryLevel) ;
	return IMAGECTN_DB_ERROR ;
    }

    if (queryLevel > lowestLevel) {
	DB_debug(1,"Level incompatible with Information Model (level %d)\n", queryLevel) ;
	return IMAGECTN_DB_ERROR ;
    }

    for (level = PATIENT_LEVEL ; level <= IMAGE_LEVEL ; level++) {

	/**** Manage exception due to StudyRoot Information Model :
	**** In this information model, queries may include Patient attributes
	**** but only if they are made at the study level
	***/

	if ((level == PATIENT_LEVEL) && (infLevel == STUDY_LEVEL)) {
	    /** In Study Root Information Model, accept only Patient Tags
	    ** if the Query Level is the Study level
	    */

	    int atLeastOneKeyFound = OFFalse ;
	    for (plist = findRequestList ; plist ; plist = plist->next) {
		DB_GetTagLevel (plist->elem. XTag, &XTagLevel) ;
		if (XTagLevel != level)
		    continue ;
		atLeastOneKeyFound = OFTrue ;
	    }
	    if (atLeastOneKeyFound && (queryLevel != STUDY_LEVEL)) {
		DB_debug(1,"Key found in Study Root Information Model (level %d)\n", level) ;
		return IMAGECTN_DB_ERROR ;
	    }
	}

	/**** If current level is above the QueryLevel
	***/

	else if (level < queryLevel) {

	    /** For this level, only unique keys are allowed
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
	}

	/**** If current level is the QueryLevel
	***/

	else if (level == queryLevel) {

	    /** For this level, all keys are allowed
	    ** Parse the request list elements reffering to
	    ** this level.
	    ** Check that at least one key is provided
	    */

	    int atLeastOneKeyFound = OFFalse ;
	    for (plist = findRequestList ; plist ; plist = plist->next) {
		DB_GetTagLevel (plist->elem. XTag, &XTagLevel) ;
		if (XTagLevel != level)
		    continue ;
		atLeastOneKeyFound = OFTrue ;
	    }
	    if (! atLeastOneKeyFound) {
		DB_debug(1,"No Key found at query level (level %d)\n", level) ;
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


/************
**	Hierarchical Search Algorithm
**	Returns OFTrue if matching is ok, else returns OFFalse
 */

OFCondition
DB_HierarchicalCompare (
		DB_Private_Handle	*phandle,
		IdxRecord 		*idxRec,
		DB_LEVEL		level,
		DB_LEVEL		infLevel,
		int			*match
		)
{
    int 		i ;
    DcmTagKey 	XTag ;
    DB_ElementList *plist ;
    DB_LEVEL 	XTagLevel ;

    /**** If current level is above the QueryLevel
    ***/

    if (level < phandle->queryLevel) {

	/** Get UID Tag for current level
	 */

	DB_GetUIDTag (level, &XTag) ;

	/** Find Element with this XTag in Identifier list
	 */

	for (plist = phandle->findRequestList ; plist ; plist = plist->next)
	    if (plist->elem. XTag == XTag)
		break ;

	/** Element not found
	 */

	if (plist == NULL) {
	    *match = OFFalse ;
	    CERR << "DB_HierarchicalCompare : No UID Key found at level " << (int) level << endl;
	    return IMAGECTN_DB_ERROR ;
	}

	/** Find element with the same XTag in index record
	 */

	for (i = 0 ; i < NBPARAMETERS ; i++)
	    if (idxRec->param [i]. XTag == XTag)
		break ;

	/** Compare with Single value matching
	** If Match fails, return OFFalse
	*/

	if (! DB_Match (&(plist->elem), &idxRec->param[i])) {
	    *match = OFFalse ;
	    return EC_Normal ;
	}

	/** Match succeeded.
	** Try at next level
	*/

	return DB_HierarchicalCompare (phandle, idxRec, (DB_LEVEL)(level + 1), infLevel, match) ;
    }

    /**** If current level is the QueryLevel
    ***/

    else if (level == phandle->queryLevel) {

	/*** For each element in Identifier list
	**/

	for (plist = phandle->findRequestList ; plist ; plist = plist->next) {

	    /** Get the Tag level of this element
	     */

	    DB_GetTagLevel (plist->elem. XTag, &XTagLevel) ;

	    /** If we are in the Study Root Information Model exception
	    ** we must accept patients keys at the study level
	    */
 
	    if (  (XTagLevel == PATIENT_LEVEL)
		  && (phandle->queryLevel == STUDY_LEVEL)
		  && (infLevel == STUDY_LEVEL)
		) ;

	    /** In other cases, only keys at the current level are
	    ** taken into account. So skip this element.
	    */

	    else if (XTagLevel != level) 
		continue ;

	    /** Find element with the same XTag in index record
	     */

	    for (i = 0 ; i < NBPARAMETERS ; i++)
		if (idxRec->param [i]. XTag == plist->elem. XTag)
		    break ;

	    /** Compare with appropriate Matching.
	    ** If Match fails, return OFFalse
	    */


	    if (! DB_Match (&(plist->elem), &idxRec->param[i])) {
		*match = OFFalse ;
		return EC_Normal ;
	    }
	}

	/*** If we are here, all matches succeeded at the current level.
	*** Perhaps check that we have tried at least one match ??
	**/

	*match = OFTrue ;
	return EC_Normal ;

    }
    return IMAGECTN_DB_ERROR;
}

/********************
**	Start find in Database
**/

OFCondition 
DB_startFindRequest(
		DB_Handle 	*handle, 			
		const char 	*SOPClassUID,
		DcmDataset 	*findRequestIdentifiers,
		DB_Status 	*status)
{
    DB_Private_Handle	*phandle = NULL;
    DB_SmallDcmElmt 	elem ;
    DB_ElementList 	*plist = NULL;
    DB_ElementList 	*last = NULL;
    int 		MatchFound ;
    IdxRecord 		idxRec ;
    DB_LEVEL		qLevel = PATIENT_LEVEL; // highest legal level for a query in the current model
    DB_LEVEL            lLevel = IMAGE_LEVEL;   // lowest legal level for a query in the current model
    
    OFCondition		cond = EC_Normal;
    OFBool qrLevelFound = OFFalse;

    phandle = (DB_Private_Handle *) handle ; 

    /**** Is SOPClassUID supported ?
    ***/

    if (strcmp( SOPClassUID, UID_FINDPatientRootQueryRetrieveInformationModel) == 0)
	phandle->rootLevel = PATIENT_ROOT ;
    else if (strcmp( SOPClassUID, UID_FINDStudyRootQueryRetrieveInformationModel) == 0)
	phandle->rootLevel = STUDY_ROOT ;
#ifndef NO_PATIENTSTUDYONLY_SUPPORT
    else if (strcmp( SOPClassUID, UID_FINDPatientStudyOnlyQueryRetrieveInformationModel) == 0)
        phandle->rootLevel = PATIENT_STUDY ;
#endif
    else {
	status->status = STATUS_FIND_Refused_SOPClassNotSupported ;
	return (IMAGECTN_DB_ERROR) ;
    }


    /**** Parse Identifiers in the Dicom Object
    **** Find Query Level and contruct a list
    **** of query identifiers
    ***/
	
    phandle->findRequestList = NULL ;

    int elemCount = (int)(findRequestIdentifiers->card());
    for (int elemIndex=0; elemIndex<elemCount; elemIndex++) {

	DcmElement* dcelem = findRequestIdentifiers->getElement(elemIndex);

	elem.XTag = dcelem->getTag().getXTag();
	if (elem.XTag == DCM_QueryRetrieveLevel || DB_TagSupported(elem.XTag)) {
	    elem.ValueLength = dcelem->getLength();
	    if (elem.ValueLength == 0) {
		elem.PValueField = NULL ;
	    } else if ((elem.PValueField = (char*)malloc((size_t)(elem.ValueLength+1))) == NULL) {
		status->status = STATUS_FIND_Refused_OutOfResources ;
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

		strncpy(level, (char*)elem.PValueField, 
			(elem.ValueLength<50)? (size_t)(elem.ValueLength) : 49) ;

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
		    if (elem. PValueField)
			free (elem. PValueField) ;
#ifdef DEBUG
		    DB_debug(1, "DB_startFindRequest () : Illegal query level (%s)\n", level) ;
#endif
		    status->status = STATUS_FIND_Failed_UnableToProcess ;
		    return (IMAGECTN_DB_ERROR) ;
		}
		qrLevelFound = OFTrue;
	    } else {
		/** Else it is a query identifier.
		** Append it to our RequestList if it is supported
		*/
		if (DB_TagSupported (elem. XTag)) {
			
		    plist = (DB_ElementList *) malloc (sizeof (DB_ElementList)) ;
		    if (plist == NULL) {
			status->status = STATUS_FIND_Refused_OutOfResources ;
			return (IMAGECTN_DB_ERROR) ;
		    }
		    plist->next = NULL ;
		    DB_DuplicateElement (&elem, &(plist->elem)) ;
		    if (phandle->findRequestList == NULL) {
			phandle->findRequestList = last = plist ;
		    } else {
			last->next = plist ;
			last = plist ;
		    }
		}
	    }

	    if ( elem. PValueField ) {
		free (elem. PValueField) ;
	    }
	}
    }

    if (!qrLevelFound) {
	/* The Query/Retrieve Level is missing */
	status->status = STATUS_FIND_Failed_IdentifierDoesNotMatchSOPClass ;
	CERR << "DB_startFindRequest(): missing Query/Retrieve Level" << endl;
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

    if (DB_doCheckFindIdentifier()) {
	cond = DB_TestFindRequestList (phandle->findRequestList, phandle->queryLevel, qLevel, lLevel) ;
	if (cond != EC_Normal) {
	    phandle->idxCounter = -1 ;
	    DB_FreeElementList (phandle->findRequestList) ;
	    phandle->findRequestList = NULL ;
#ifdef DEBUG
	    DB_debug(1, "DB_startFindRequest () : STATUS_FIND_Failed_IdentifierDoesNotMatchSOPClass - Invalid RequestList\n") ;
#endif
	    status->status = STATUS_FIND_Failed_IdentifierDoesNotMatchSOPClass ;
	    return (cond) ;
	}
    }

    /**** Goto the beginning of Index File
    **** Then find the first matching image
    ***/

    DB_lock(phandle, OFFalse);

    DB_IdxInitLoop (phandle, &(phandle->idxCounter)) ;
    MatchFound = OFFalse ;
    cond = EC_Normal ;

    while (1) {

	/*** Exit loop if read error (or end of file)
	**/

	if (DB_IdxGetNext (phandle, &(phandle->idxCounter), &idxRec) != EC_Normal)
	    break ;

	/*** Exit loop if error or matching OK
	**/

	cond = DB_HierarchicalCompare (phandle, &idxRec, qLevel, qLevel, &MatchFound) ;
	if (cond != EC_Normal)
	    break ;
	if (MatchFound)
	    break ;
    }

    /**** If an error occured in Matching function
    ****    return a failed status
    ***/

    if (cond != EC_Normal) {
	phandle->idxCounter = -1 ;
	DB_FreeElementList (phandle->findRequestList) ;
	phandle->findRequestList = NULL ;
#ifdef DEBUG
	DB_debug(1, "DB_startFindRequest () : STATUS_FIND_Failed_UnableToProcess\n") ;
#endif
	status->status = STATUS_FIND_Failed_UnableToProcess ;

	DB_unlock(phandle);

	return (cond) ;
    }


    /**** If a matching image has been found,
    ****	 add index record to UID found list
    ****    prepare Response List in handle
    ****    return status is pending
    ***/

    if (MatchFound) {
	DB_UIDAddFound (phandle, &idxRec) ;
	DB_MakeResponseList (phandle, &idxRec) ;
#ifdef DEBUG
	DB_debug(1, "DB_startFindRequest () : STATUS_Pending\n") ;
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
	DB_FreeElementList (phandle->findRequestList) ;
	phandle->findRequestList = NULL ;
#ifdef DEBUG
	DB_debug(1, "DB_startFindRequest () : STATUS_Success\n") ;
#endif
	status->status = STATUS_Success ;

	DB_unlock(phandle);

	return (EC_Normal) ;
    }

}

/********************
**	Get next find response in Database
 */

OFCondition 
DB_nextFindResponse (
		DB_Handle 	*handle, 			
		DcmDataset 	**findResponseIdentifiers,
		DB_Status 	*status)
{

    DB_Private_Handle	*phandle = NULL; 
    DB_ElementList 	*plist = NULL;
    int 		MatchFound = OFFalse;
    IdxRecord 		idxRec ;
    DB_LEVEL 		qLevel = PATIENT_LEVEL;
    const char		*queryLevelString = NULL;
    OFCondition		cond = EC_Normal;

    phandle = (DB_Private_Handle *) handle ; 


    if (phandle->findResponseList == NULL) {
#ifdef DEBUG
	DB_debug(1, "DB_nextFindResponse () : STATUS_Success\n") ;
#endif
	*findResponseIdentifiers = NULL ;
	status->status = STATUS_Success ;

	DB_unlock(phandle);

	return (EC_Normal) ;
    }	

    /***** Create the response (findResponseIdentifiers) using
    ***** the last find done and saved in handle findResponseList
    ****/

    *findResponseIdentifiers = new DcmDataset ;
    if ( *findResponseIdentifiers != NULL ) {

	/*** Put responses
	**/

	for ( plist = phandle->findResponseList ; plist != NULL ; plist = plist->next ) {
	    DcmTag t(plist->elem.XTag);
	    DcmElement *dce = newDicomElement(t);
	    if (dce == NULL) {
		status->status = STATUS_FIND_Refused_OutOfResources ;
		return IMAGECTN_DB_ERROR;
	    }
	    if (plist->elem.PValueField != NULL && 
		strlen(plist->elem.PValueField) > 0) {
		OFCondition ec = dce->putString(plist->elem.PValueField);
		if (ec != EC_Normal) {
		    CERR << "dbfind: DB_nextFindResponse: cannot put()" << endl;
		    status->status = STATUS_FIND_Failed_UnableToProcess ;
		    return IMAGECTN_DB_ERROR;
		}
	    }
	    OFCondition ec = (*findResponseIdentifiers)->insert(dce, OFTrue /*replaceOld*/);
	    if (ec != EC_Normal) {
		CERR << "dbfind: DB_nextFindResponse: cannot insert()" << endl;
		status->status = STATUS_FIND_Failed_UnableToProcess ;
		return IMAGECTN_DB_ERROR;
	    }
	}

	/*** Append the Query level
	**/

	switch (phandle->queryLevel) {
	case PATIENT_LEVEL :
	    queryLevelString = PATIENT_LEVEL_STRING ;
	    break ;
	case STUDY_LEVEL :
	    queryLevelString = STUDY_LEVEL_STRING ;
	    break ;
	case SERIE_LEVEL :
	    queryLevelString = SERIE_LEVEL_STRING ;
	    break ;
	case IMAGE_LEVEL :
	    queryLevelString = IMAGE_LEVEL_STRING ;
	    break ;
	}
	DU_putStringDOElement(*findResponseIdentifiers,
			      DCM_QueryRetrieveLevel, queryLevelString);
#ifdef DEBUG
	if (DB_debugLevel > 0) {
	    COUT << "DB: findResponseIdentifiers:" << endl;
	    (*findResponseIdentifiers)->print(COUT);
	}
#endif
    }
    else {

	DB_unlock(phandle);

	return (IMAGECTN_DB_ERROR) ;
    }
	
    switch (phandle->rootLevel) {
    case PATIENT_ROOT :	qLevel = PATIENT_LEVEL ;	break ;
    case STUDY_ROOT :	qLevel = STUDY_LEVEL ;		break ;
    case PATIENT_STUDY: qLevel = PATIENT_LEVEL ;        break ; 
    }

    /***** Free the last response...
    ****/

    DB_FreeElementList (phandle->findResponseList) ;
    phandle->findResponseList = NULL ;

    /***** ... and find the next one
    ****/

    MatchFound = OFFalse ;
    cond = EC_Normal ;

    while (1) {

	/*** Exit loop if read error (or end of file)
	**/

	if (DB_IdxGetNext (phandle, &(phandle->idxCounter), &idxRec) != EC_Normal)
	    break ;

	/*** If Response already found
	**/

	if (DB_UIDAlreadyFound (phandle, &idxRec))
	    continue ;

	/*** Exit loop if error or matching OK
	**/

	cond = DB_HierarchicalCompare (phandle, &idxRec, qLevel, qLevel, &MatchFound) ;
	if (cond != EC_Normal)
	    break ;
	if (MatchFound)
	    break ;

    }

    /**** If an error occured in Matching function
    ****    return status is pending
    ***/

    if (cond != EC_Normal) {
	phandle->idxCounter = -1 ;
	DB_FreeElementList (phandle->findRequestList) ;
	phandle->findRequestList = NULL ;
#ifdef DEBUG
	DB_debug(1, "DB_nextFindResponse () : STATUS_FIND_Failed_UnableToProcess\n") ;
#endif
	status->status = STATUS_FIND_Failed_UnableToProcess ;

	DB_unlock(phandle);

	return (cond) ;
    }


    /**** If a matching image has been found
    ****    add index records UIDs in found UID list
    ****    prepare Response List in handle
    ***/

    if (MatchFound) {
	DB_UIDAddFound (phandle, &idxRec) ;
	DB_MakeResponseList (phandle, &idxRec) ;
#ifdef DEBUG
	DB_debug(1, "DB_nextFindResponse () : STATUS_Pending\n") ;
#endif
	status->status = STATUS_Pending ;
	return (EC_Normal) ;
    }

    /**** else no matching image has been found,
    ****    free query identifiers list
    **** Response list is null, so next call will return STATUS_Success
    ***/

    else {
	phandle->idxCounter = -1 ;
	DB_FreeElementList (phandle->findRequestList) ;
	phandle->findRequestList = NULL ;
	DB_FreeUidList (phandle->uidList) ;
	phandle->uidList = NULL ;
    }


#ifdef DEBUG
    DB_debug(1, "DB_nextFindResponse () : STATUS_Pending\n") ;
#endif
    status->status = STATUS_Pending ;
    return (EC_Normal) ;
}

/********************
**	Cancel find request
 */

OFCondition 
DB_cancelFindRequest (DB_Handle *handle, DB_Status *status) 
{
    DB_Private_Handle	*phandle ; 


    phandle = (DB_Private_Handle *) handle ; 

    phandle->idxCounter = -1 ;
    DB_FreeElementList (phandle->findRequestList) ;
    phandle->findRequestList = NULL ;
    DB_FreeElementList (phandle->findResponseList) ;
    phandle->findResponseList = NULL ;
    DB_FreeUidList (phandle->uidList) ;
    phandle->uidList = NULL ;

    status->status = STATUS_FIND_Cancel_MatchingTerminatedDueToCancelRequest ;

    DB_unlock(phandle);

    return (EC_Normal) ;
}

/*
** CVS Log
** $Log: dbfind.cc,v $
** Revision 1.1  2005/08/23 19:32:09  braindead
** - initial savannah import
**
** Revision 1.1  2005/06/26 19:26:14  pipelka
** - added dcmtk
**
** Revision 1.23  2004/02/27 13:36:26  meichel
** Imagectn now refuses find/get/move operations on SERIES or IMAGE level when
**   the Patient/Study Only Q/R model is used and identifier checking is enabled
**
** Revision 1.22  2002/11/27 13:27:54  meichel
** Adapted module imagectn to use of new header file ofstdinc.h
**
** Revision 1.21  2001/11/28 13:41:13  joergr
** Check return value of DcmItem::insert() statements where appropriate to
** avoid memory leaks when insert procedure fails.
**
** Revision 1.20  2001/10/12 12:43:09  meichel
** Adapted imagectn to OFCondition based dcmnet module (supports strict mode).
**
** Revision 1.19  2001/09/26 16:06:37  meichel
** Adapted imagectn to class OFCondition
**
** Revision 1.18  2001/06/01 15:51:27  meichel
** Updated copyright header
**
** Revision 1.17  2000/04/14 16:38:30  meichel
** Removed default value from output stream passed to print() method.
**   Required for use in multi-thread environments.
**
** Revision 1.16  2000/03/08 16:41:08  meichel
** Updated copyright header.
**
** Revision 1.15  2000/03/06 15:53:08  meichel
** Introduced typecasts when printing enums to cout/cerr.
**
** Revision 1.14  2000/03/03 14:16:38  meichel
** Implemented library support for redirecting error messages into memory
**   instead of printing them to stdout/stderr for GUI applications.
**
** Revision 1.13  2000/02/23 15:13:28  meichel
** Corrected macro for Borland C++ Builder 4 workaround.
**
** Revision 1.12  2000/02/01 11:43:45  meichel
** Avoiding to include <stdlib.h> as extern "C" on Borland C++ Builder 4,
**   workaround for bug in compiler header files.
**
** Revision 1.11  1999/06/10 12:12:16  meichel
** Adapted imagectn to new command line option scheme.
**   Added support for Patient/Study Only Q/R model and C-GET (experimental).
**
** Revision 1.10  1997/09/18 08:11:03  meichel
** Many minor type conflicts (e.g. long passed as int) solved.
**
** Revision 1.9  1997/07/21 08:59:55  andreas
** - Replace all boolean types (BOOLEAN, CTNBOOLEAN, DICOM_BOOL, BOOL)
**   with one unique boolean type OFBool.
**
** Revision 1.8  1997/06/26 12:52:00  andreas
** - Changed names for enumeration values in DB_KEY_TYPE since the value
**   OPTIONAL was predefined for Windows 95/NT
**
** Revision 1.7  1997/05/29 10:17:35  hewett
** added explicit unsigned->int casts to avoid some CC compiler warnings
** under Nextstep.
**
** Revision 1.6  1997/04/18 08:40:35  andreas
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
** Revision 1.5  1996/09/27 08:46:49  hewett
** Enclosed system include files with BEGIN_EXTERN_C/END_EXTERN_C.
**
** Revision 1.4  1996/04/27 12:55:17  hewett
** Removed cause of warnings when compiled with "c++ -O -g -Wall" under
** Solaris 2.4.  Mostly due to uninitialized variables.
**
** Revision 1.3  1996/04/25 16:30:23  hewett
** Replaced strdup call with private function DB_strdup().
**
** Revision 1.2  1996/04/22 10:37:48  hewett
** Now only processes supported keys.  Others are ignored.  Added check for
** presence of Query/Retrieve Level.
**
** Revision 1.1.1.1  1996/03/28 19:25:00  hewett
** Oldenburg Image CTN Software ported to use the dcmdata C++ toolkit.
**
**
*/
