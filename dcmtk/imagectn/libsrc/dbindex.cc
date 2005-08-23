/*
 *
 *  Copyright (C) 1993-2004, OFFIS
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
 *  Purpose: routines which provide index management facilities for the DB Module
 *    Module Prefix: DB_ 
 *
 *  Last Update:      $Author: braindead $
 *  Update Date:      $Date: 2005/08/23 19:32:09 $
 *  Source File:      $Source: /cvsroot/aeskulap/aeskulap/dcmtk/imagectn/libsrc/dbindex.cc,v $
 *  CVS/RCS Revision: $Revision: 1.1 $
 *  Status:           $State: Exp $
 *
 *  CVS/RCS Log at end of file
 *
 */

#include "osconfig.h"    /* make sure OS specific configuration is included first */

#define INCLUDE_CSTDLIB
#define INCLUDE_CSTDIO
#define INCLUDE_CERRNO
#include "ofstdinc.h"

BEGIN_EXTERN_C
#ifdef HAVE_SYS_FILE_H
#include <sys/file.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
END_EXTERN_C

#include "imagedb.h"
#include "dbpriv.h"
#include "ofconsol.h"
#include "dbcond.h"

/************
 *	Initializes addresses in an IdxRecord
 */

void 
DB_IdxInitRecord (IdxRecord *idx, int linksOnly) 
{
    if (! linksOnly)
    {
    	idx -> param[RECORDIDX_PatientsBirthDate]. XTag = DCM_PatientsBirthDate  ;
    	idx -> param[RECORDIDX_PatientsBirthDate]. ValueLength = DA_MAX_LENGTH  ;
    	idx -> PatientsBirthDate[0] = '\0' ;
    	idx -> param[RECORDIDX_PatientsSex]. XTag = DCM_PatientsSex  ;
    	idx -> param[RECORDIDX_PatientsSex]. ValueLength = CS_MAX_LENGTH  ;
    	idx -> PatientsSex[0] = '\0' ;
     	idx -> param[RECORDIDX_PatientsName]. XTag = DCM_PatientsName  ;
    	idx -> param[RECORDIDX_PatientsName]. ValueLength = PN_MAX_LENGTH  ;
    	idx -> PatientsName[0] = '\0' ;
    	idx -> param[RECORDIDX_PatientID]. XTag = DCM_PatientID  ;
    	idx -> param[RECORDIDX_PatientID]. ValueLength = LO_MAX_LENGTH  ;
    	idx -> PatientID[0] = '\0' ;
    	idx -> param[RECORDIDX_PatientsBirthTime]. XTag = DCM_PatientsBirthTime  ;
    	idx -> param[RECORDIDX_PatientsBirthTime]. ValueLength = TM_MAX_LENGTH  ;
    	idx -> PatientsBirthTime[0] = '\0' ;
    	idx -> param[RECORDIDX_OtherPatientIDs]. XTag = DCM_OtherPatientIDs  ;
    	idx -> param[RECORDIDX_OtherPatientIDs]. ValueLength = LO_MAX_LENGTH  ;
    	idx -> OtherPatientIDs[0] = '\0' ;
    	idx -> param[RECORDIDX_OtherPatientNames]. XTag = DCM_OtherPatientNames  ;
    	idx -> param[RECORDIDX_OtherPatientNames]. ValueLength = PN_MAX_LENGTH  ;
    	idx -> OtherPatientNames[0] = '\0' ;
    	idx -> param[RECORDIDX_EthnicGroup]. XTag = DCM_EthnicGroup  ;
    	idx -> param[RECORDIDX_EthnicGroup]. ValueLength = SH_MAX_LENGTH  ;
    	idx -> EthnicGroup[0] = '\0' ;
     	idx -> param[RECORDIDX_NumberofPatientRelatedStudies]. XTag = DCM_NumberOfPatientRelatedStudies  ;
    	idx -> param[RECORDIDX_NumberofPatientRelatedStudies]. ValueLength = IS_MAX_LENGTH  ;
    	idx -> NumberofPatientRelatedStudies[0] = '\0' ;
    	idx -> param[RECORDIDX_NumberofPatientRelatedSeries]. XTag = DCM_NumberOfPatientRelatedSeries  ;
    	idx -> param[RECORDIDX_NumberofPatientRelatedSeries]. ValueLength = IS_MAX_LENGTH  ;
    	idx -> NumberofPatientRelatedSeries[0] = '\0' ;
    	idx -> param[RECORDIDX_NumberofPatientRelatedInstances]. XTag = DCM_NumberOfPatientRelatedInstances  ;
    	idx -> param[RECORDIDX_NumberofPatientRelatedInstances]. ValueLength = IS_MAX_LENGTH  ;
    	idx -> NumberofPatientRelatedInstances[0] = '\0' ;
    	idx -> param[RECORDIDX_StudyDate]. XTag = DCM_StudyDate  ;
    	idx -> param[RECORDIDX_StudyDate]. ValueLength = DA_MAX_LENGTH  ;
    	idx -> StudyDate[0] = '\0' ;
    	idx -> param[RECORDIDX_StudyTime]. XTag = DCM_StudyTime  ;
    	idx -> param[RECORDIDX_StudyTime]. ValueLength = TM_MAX_LENGTH  ;
    	idx -> StudyTime[0] = '\0' ;
    	idx -> param[RECORDIDX_StudyID]. XTag = DCM_StudyID  ;
    	idx -> param[RECORDIDX_StudyID]. ValueLength = CS_MAX_LENGTH  ;
    	idx -> StudyID[0] = '\0' ;
    	idx -> param[RECORDIDX_StudyDescription]. XTag = DCM_StudyDescription  ;
    	idx -> param[RECORDIDX_StudyDescription]. ValueLength = LO_MAX_LENGTH  ;
    	idx -> StudyDescription[0] = '\0' ;
    	idx -> param[RECORDIDX_NameOfPhysiciansReadingStudy]. XTag = DCM_NameOfPhysiciansReadingStudy  ;
    	idx -> param[RECORDIDX_NameOfPhysiciansReadingStudy]. ValueLength = PN_MAX_LENGTH  ;
    	idx -> NameOfPhysiciansReadingStudy[0] = '\0' ;
    	idx -> param[RECORDIDX_AccessionNumber]. XTag = DCM_AccessionNumber;
    	idx -> param[RECORDIDX_AccessionNumber]. ValueLength = CS_MAX_LENGTH ;
    	idx -> AccessionNumber[0] = '\0' ;
    	idx -> param[RECORDIDX_ReferringPhysiciansName]. XTag = DCM_ReferringPhysiciansName  ;
    	idx -> param[RECORDIDX_ReferringPhysiciansName]. ValueLength = PN_MAX_LENGTH ;
    	idx -> ReferringPhysiciansName[0] = '\0' ;
    	idx -> param[RECORDIDX_ProcedureDescription]. XTag = DCM_StudyDescription  ;
    	idx -> param[RECORDIDX_ProcedureDescription]. ValueLength = LO_MAX_LENGTH ;
    	idx -> ProcedureDescription[0] = '\0' ;
    	idx -> param[RECORDIDX_AttendingPhysiciansName]. XTag = DCM_NameOfPhysiciansReadingStudy  ;
    	idx -> param[RECORDIDX_AttendingPhysiciansName]. ValueLength = PN_MAX_LENGTH ;
    	idx -> AttendingPhysiciansName[0] = '\0' ;
    	idx -> param[RECORDIDX_StudyInstanceUID]. XTag = DCM_StudyInstanceUID  ;
    	idx -> param[RECORDIDX_StudyInstanceUID]. ValueLength = UI_MAX_LENGTH ;
    	idx -> StudyInstanceUID[0] = '\0' ;
    	idx -> param[RECORDIDX_OtherStudyNumbers]. XTag = DCM_OtherStudyNumbers  ;
    	idx -> param[RECORDIDX_OtherStudyNumbers]. ValueLength = IS_MAX_LENGTH ;
    	idx -> OtherStudyNumbers[0] = '\0' ;
    	idx -> param[RECORDIDX_AdmittingDiagnosesDescription]. XTag = DCM_AdmittingDiagnosesDescription  ;
    	idx -> param[RECORDIDX_AdmittingDiagnosesDescription]. ValueLength = LO_MAX_LENGTH ;
    	idx -> AdmittingDiagnosesDescription[0] = '\0' ;
    	idx -> param[RECORDIDX_PatientsAge]. XTag = DCM_PatientsAge  ;
    	idx -> param[RECORDIDX_PatientsAge]. ValueLength = AS_MAX_LENGTH ;
    	idx -> PatientsAge[0] = '\0' ;
    	idx -> param[RECORDIDX_PatientsSize]. XTag = DCM_PatientsSize  ;
    	idx -> param[RECORDIDX_PatientsSize]. ValueLength = DS_MAX_LENGTH ;
    	idx -> PatientsSize[0] = '\0' ;
     	idx -> param[RECORDIDX_PatientsWeight]. XTag = DCM_PatientsWeight  ;
    	idx -> param[RECORDIDX_PatientsWeight]. ValueLength = DS_MAX_LENGTH ;
    	idx -> PatientsWeight[0] = '\0' ;
    	idx -> param[RECORDIDX_Occupation]. XTag = DCM_Occupation  ;
    	idx -> param[RECORDIDX_Occupation]. ValueLength = SH_MAX_LENGTH ;
    	idx -> Occupation[0] = '\0' ;
    	idx -> param[RECORDIDX_NumberofStudyRelatedSeries]. XTag = DCM_NumberOfStudyRelatedSeries  ;
    	idx -> param[RECORDIDX_NumberofStudyRelatedSeries]. ValueLength = IS_MAX_LENGTH ;
    	idx -> NumberofStudyRelatedSeries[0] = '\0' ;
    	idx -> param[RECORDIDX_NumberofStudyRelatedInstances]. XTag = DCM_NumberOfStudyRelatedInstances  ;
    	idx -> param[RECORDIDX_NumberofStudyRelatedInstances]. ValueLength = IS_MAX_LENGTH ;
    	idx -> NumberofStudyRelatedInstances[0] = '\0' ;
    	idx -> param[RECORDIDX_SeriesNumber]. XTag = DCM_SeriesNumber  ;
    	idx -> param[RECORDIDX_SeriesNumber]. ValueLength = IS_MAX_LENGTH ;
    	idx -> SeriesNumber[0] = '\0' ;
     	idx -> param[RECORDIDX_SeriesInstanceUID]. XTag = DCM_SeriesInstanceUID  ;
    	idx -> param[RECORDIDX_SeriesInstanceUID]. ValueLength = UI_MAX_LENGTH ;
    	idx -> SeriesInstanceUID[0] = '\0' ;
    	idx -> param[RECORDIDX_Modality]. XTag = DCM_Modality  ;
    	idx -> param[RECORDIDX_Modality]. ValueLength = CS_MAX_LENGTH ;
    	idx -> ImageNumber[0] = '\0' ;
    	idx -> param[RECORDIDX_ImageNumber]. XTag = DCM_InstanceNumber  ;
    	idx -> param[RECORDIDX_ImageNumber]. ValueLength = IS_MAX_LENGTH ;
    	idx -> ImageNumber[0] = '\0' ;
    	idx -> param[RECORDIDX_SOPInstanceUID]. XTag = DCM_SOPInstanceUID  ;
    	idx -> param[RECORDIDX_SOPInstanceUID]. ValueLength = UI_MAX_LENGTH ;
    	idx -> SOPInstanceUID[0] = '\0'	;
        idx -> param[RECORDIDX_SeriesDate]. XTag = DCM_SeriesDate;
        idx -> param[RECORDIDX_SeriesDate]. ValueLength = DA_MAX_LENGTH ;
        idx -> SeriesDate[0] = '\0'	;
    	idx -> param[RECORDIDX_SeriesTime]. XTag = DCM_SeriesTime;
    	idx -> param[RECORDIDX_SeriesTime]. ValueLength = TM_MAX_LENGTH ;
    	idx -> SeriesTime[0] = '\0'	;
    	idx -> param[RECORDIDX_SeriesDescription]. XTag = DCM_SeriesDescription  ;
    	idx -> param[RECORDIDX_SeriesDescription]. ValueLength = LO_MAX_LENGTH ;
    	idx -> SeriesDescription[0] = '\0'	;
        idx -> param[RECORDIDX_ProtocolName]. XTag = DCM_ProtocolName  ;
    	idx -> param[RECORDIDX_ProtocolName]. ValueLength = LO_MAX_LENGTH ;
    	idx -> ProtocolName[0] = '\0'	;
        idx -> param[RECORDIDX_OperatorsName ]. XTag = DCM_OperatorsName  ;
    	idx -> param[RECORDIDX_OperatorsName ]. ValueLength = PN_MAX_LENGTH ;
    	idx -> OperatorsName[0] = '\0';
        idx -> param[RECORDIDX_PerformingPhysiciansName]. XTag = DCM_PerformingPhysiciansName  ;
    	idx -> param[RECORDIDX_PerformingPhysiciansName]. ValueLength = PN_MAX_LENGTH ;
    	idx -> PerformingPhysiciansName[0] = '\0';
        idx -> param[RECORDIDX_PresentationLabel]. XTag = DCM_ContentLabel  ;
    	idx -> param[RECORDIDX_PresentationLabel]. ValueLength = CS_LABEL_MAX_LENGTH ;
    	idx -> PresentationLabel[0] = '\0';
    }
    idx -> param[RECORDIDX_PatientsBirthDate]. PValueField = (char *)idx -> PatientsBirthDate ;
    idx -> param[RECORDIDX_PatientsSex]. PValueField = (char *)idx -> PatientsSex ;
    idx -> param[RECORDIDX_PatientsName]. PValueField = (char *)idx -> PatientsName ;
    idx -> param[RECORDIDX_PatientID]. PValueField = (char *)idx -> PatientID ;
    idx -> param[RECORDIDX_PatientsBirthTime]. PValueField = (char *)idx -> PatientsBirthTime ;
    idx -> param[RECORDIDX_OtherPatientIDs]. PValueField = (char *)idx -> OtherPatientIDs ;
    idx -> param[RECORDIDX_OtherPatientNames]. PValueField = (char *)idx -> OtherPatientNames ;
    idx -> param[RECORDIDX_EthnicGroup]. PValueField = (char *)idx -> EthnicGroup ;
    idx -> param[RECORDIDX_NumberofPatientRelatedStudies]. PValueField = (char *)idx -> NumberofPatientRelatedStudies ;
    idx -> param[RECORDIDX_NumberofPatientRelatedSeries]. PValueField = (char *) idx -> NumberofPatientRelatedSeries ;	
    idx -> param[RECORDIDX_NumberofPatientRelatedInstances]. PValueField = (char *) idx -> NumberofPatientRelatedInstances ;
    idx -> param[RECORDIDX_StudyDate]. PValueField = (char *) idx -> StudyDate ;
    idx -> param[RECORDIDX_StudyTime]. PValueField = (char *) idx -> StudyTime ;
    idx -> param[RECORDIDX_StudyID]. PValueField = (char *) idx -> StudyID ;
    idx -> param[RECORDIDX_StudyDescription]. PValueField = (char *) idx -> StudyDescription ;
    idx -> param[RECORDIDX_NameOfPhysiciansReadingStudy]. PValueField = (char *) idx ->NameOfPhysiciansReadingStudy;
    idx -> param[RECORDIDX_AccessionNumber]. PValueField = (char *) idx -> AccessionNumber ;
    idx -> param[RECORDIDX_ReferringPhysiciansName]. PValueField = (char *) idx -> ReferringPhysiciansName ;
    idx -> param[RECORDIDX_ProcedureDescription]. PValueField = (char *) idx -> ProcedureDescription ;
    idx -> param[RECORDIDX_AttendingPhysiciansName]. PValueField = (char *) idx -> AttendingPhysiciansName ;
    idx -> param[RECORDIDX_StudyInstanceUID]. PValueField = (char *) idx -> StudyInstanceUID ;
    idx -> param[RECORDIDX_OtherStudyNumbers]. PValueField = (char *) idx -> OtherStudyNumbers ;
    idx -> param[RECORDIDX_AdmittingDiagnosesDescription]. PValueField = (char *) idx -> AdmittingDiagnosesDescription ;
    idx -> param[RECORDIDX_PatientsAge]. PValueField = (char *) idx -> PatientsAge ;
    idx -> param[RECORDIDX_PatientsSize]. PValueField = (char *) idx -> PatientsSize ;
    idx -> param[RECORDIDX_PatientsWeight]. PValueField = (char *) idx -> PatientsWeight ;
    idx -> param[RECORDIDX_Occupation]. PValueField = (char *) idx -> Occupation ;
    idx -> param[RECORDIDX_NumberofStudyRelatedSeries]. PValueField = (char *) idx -> NumberofStudyRelatedSeries ;
    idx -> param[RECORDIDX_NumberofStudyRelatedInstances]. PValueField = (char *) idx -> NumberofStudyRelatedInstances ;
    idx -> param[RECORDIDX_SeriesNumber]. PValueField = (char *) idx -> SeriesNumber ;
    idx -> param[RECORDIDX_SeriesInstanceUID]. PValueField = (char *) idx -> SeriesInstanceUID ;
    idx -> param[RECORDIDX_Modality]. PValueField = (char *) idx -> Modality ;
    idx -> param[RECORDIDX_ImageNumber]. PValueField = (char *) idx -> ImageNumber ;
    idx -> param[RECORDIDX_SOPInstanceUID]. PValueField = (char *) idx -> SOPInstanceUID ;
    idx -> param[RECORDIDX_SeriesDate]. PValueField = (char *) idx -> SeriesDate ;
    idx -> param[RECORDIDX_SeriesTime]. PValueField = (char *) idx -> SeriesTime ;
    idx -> param[RECORDIDX_SeriesDescription]. PValueField = (char *) idx -> SeriesDescription ;
    idx -> param[RECORDIDX_ProtocolName]. PValueField = (char *) idx -> ProtocolName ;
    idx -> param[RECORDIDX_OperatorsName ]. PValueField = (char *) idx -> OperatorsName ;
    idx -> param[RECORDIDX_PerformingPhysiciansName]. PValueField = (char *) idx -> PerformingPhysiciansName ;
    idx -> param[RECORDIDX_PresentationLabel]. PValueField = (char *) idx -> PresentationLabel ;
}

/******************************
 *      Seek to a file position and do error checking
 *
 * Motivation:
 * We have had situations during demonstrations where size of the DB index file
 * has exploded.  It seems that a record is being written to a position
 * way past the end of file.
 * This seek function does some sanity error checking to try to identify 
 * the problem.
 */
long DB_lseek(int fildes, long offset, int whence)
{
    long pos;
    long curpos;
    long endpos;


    /* 
    ** we should not be seeking to an offset < 0
    */
    if (offset < 0) {
	CERR << "*** DB ALERT: attempt to seek before begining of file" << endl;
    }

    /* get the current position */
    curpos = lseek(fildes, 0, SEEK_CUR);
    if (curpos < 0) {
        CERR << "DB_lseek: cannot get current position: " << strerror(errno) << endl;
	return curpos;
    }
    /* get the end of file position */
    endpos = lseek(fildes, 0, SEEK_END);
    if (endpos < 0) {
        CERR << "DB_lseek: cannot get end of file position: " << strerror(errno) << endl;
	return endpos;
    }

    /* return to current position */
    curpos = lseek(fildes, curpos, SEEK_SET);
    if (curpos < 0) {
        CERR << "DB_lseek: cannot reset current position: " << strerror(errno) << endl;
	return curpos;
    }

    /* do the requested seek */
    pos = lseek(fildes, offset, whence);
    if (pos < 0) {
	char msg[1024];
	sprintf(msg, "DB_lseek: cannot seek to %ld", offset);
	CERR << msg << ": " << strerror(errno) << endl;
	return pos;
    }

    /* 
    ** print an alert if we are seeking to far
    ** what is the limit? We don't expect the index file to be 
    ** larger than 32Mb
    */
    const long maxFileSize = 33554432;
    if (pos > maxFileSize) {
	CERR << "*** DB ALERT: attempt to seek beyond " << maxFileSize << " bytes" << endl;
    }

    /* print an alert if we are seeking beyond the end of file.
     * ignore when file is empty
     */
    if ((endpos > 0) && (pos > endpos)) {
	CERR << "*** DB ALERT: attempt to seek beyond end of file" << endl
	    << "              offset=" << offset << " filesize=" << endpos << endl;
    }

    return pos;
}

/******************************
 *	Read an Index record
 */

OFCondition DB_IdxRead (DB_Private_Handle *phandle, int idx, IdxRecord *idxRec)
{

    /*** Goto the right index in file
    **/
	
    DB_lseek (phandle -> pidx, (long) (SIZEOF_STUDYDESC + idx * SIZEOF_IDXRECORD), SEEK_SET) ; 

    /*** Read the record
    **/

    if (read (phandle -> pidx, (char *) idxRec, SIZEOF_IDXRECORD) != SIZEOF_IDXRECORD)
	return (IMAGECTN_DB_ERROR) ;

    DB_lseek (phandle -> pidx, 0L, SEEK_SET) ;	

    /*** Initialize record links
    **/

    DB_IdxInitRecord (idxRec, 1) ;
    return EC_Normal ;
}


/******************************
 *	Add an Index record
 *	Returns the index allocated for this record
 */

OFCondition DB_IdxAdd (DB_Private_Handle *phandle, int *idx, IdxRecord *idxRec)
{
    IdxRecord	rec ;
    OFCondition	cond = EC_Normal;

    /*** Find free place for the record
    *** A place is free if filename is empty
    **/

    *idx = 0 ;

    DB_lseek (phandle -> pidx, (long) SIZEOF_STUDYDESC, SEEK_SET) ; 
    while (read (phandle -> pidx, (char *) &rec, SIZEOF_IDXRECORD) == SIZEOF_IDXRECORD) {
	if (rec. filename [0] == '\0')
	    break ;
	(*idx)++ ;
    }

    /*** We have either found a free place or we are at the end of file. **/

	
    DB_lseek (phandle -> pidx, (long) (SIZEOF_STUDYDESC + (*idx) * SIZEOF_IDXRECORD), SEEK_SET) ; 

    if (write (phandle -> pidx, (char *) idxRec, SIZEOF_IDXRECORD) != SIZEOF_IDXRECORD)
	cond = IMAGECTN_DB_ERROR ;
    else
	cond = EC_Normal ;

    DB_lseek (phandle -> pidx, 0L, SEEK_SET) ;	

    return cond ;
}


/******************************
 *	Change the StudyDescRecord
 */

OFCondition DB_StudyDescChange (DB_Private_Handle *phandle, StudyDescRecord *pStudyDesc)
{
    OFCondition	cond = EC_Normal;
    DB_lseek (phandle -> pidx, 0L, SEEK_SET) ; 
    if (write (phandle -> pidx, (char *) pStudyDesc, SIZEOF_STUDYDESC) != SIZEOF_STUDYDESC) cond = IMAGECTN_DB_ERROR;
    DB_lseek (phandle -> pidx, 0L, SEEK_SET) ;	
    return cond ;
}



/******************************
 *	Init an Index record loop
 */

OFCondition DB_IdxInitLoop (DB_Private_Handle *phandle, int *idx)
{
    DB_lseek (phandle -> pidx, SIZEOF_STUDYDESC, SEEK_SET) ; 
    *idx = -1 ;
    return EC_Normal ;
}

/******************************
 *	Get next Index record
 *	On return, idx is initialized with the index of the record read
 */

OFCondition DB_IdxGetNext (DB_Private_Handle *phandle, int *idx, IdxRecord *idxRec)
{

    (*idx)++ ;
    DB_lseek (phandle -> pidx, SIZEOF_STUDYDESC + (long)(*idx) * SIZEOF_IDXRECORD, SEEK_SET) ; 
    while (read (phandle -> pidx, (char *) idxRec, SIZEOF_IDXRECORD) == SIZEOF_IDXRECORD) {
	if (idxRec -> filename [0] != '\0') {
	    DB_IdxInitRecord (idxRec, 1) ;

	    return EC_Normal ;
	}
	(*idx)++ ;
    }

    DB_lseek (phandle -> pidx, 0L, SEEK_SET) ;	

    return IMAGECTN_DB_ERROR ;
}


/******************************
 *	Get next Index record
 *	On return, idx is initialized with the index of the record read
 */

OFCondition DB_GetStudyDesc (DB_Private_Handle *phandle, StudyDescRecord *pStudyDesc)
{

    DB_lseek (phandle -> pidx, 0L, SEEK_SET) ; 
    if ( read (phandle -> pidx, (char *) pStudyDesc, SIZEOF_STUDYDESC) == SIZEOF_STUDYDESC ) 
	return EC_Normal ;

    DB_lseek (phandle -> pidx, 0L, SEEK_SET) ;	

    return IMAGECTN_DB_ERROR ;
}


/******************************
 *	Remove an Index record
 *	Just put a record with filename == ""
 */

OFCondition DB_IdxRemove (DB_Private_Handle *phandle, int idx)
{
    IdxRecord 	rec ;
    OFCondition cond = EC_Normal;

    DB_lseek (phandle -> pidx, SIZEOF_STUDYDESC + (long)idx * SIZEOF_IDXRECORD, SEEK_SET) ;
    DB_IdxInitRecord (&rec, 0) ;

    rec. filename [0] = '\0' ;
    if (write (phandle -> pidx, (char *) &rec, SIZEOF_IDXRECORD) == SIZEOF_IDXRECORD) 
	cond = EC_Normal ;

    else 
	cond = IMAGECTN_DB_ERROR ;

    DB_lseek (phandle -> pidx, 0L, SEEK_SET) ;	

    return cond ;
}

/************************
 *	Dump an index file
 */

void	
DB_PrintIndexFile (char *storeArea)
{
    int i ;
    int j ;
    IdxRecord 		idxRec ;
    StudyDescRecord	*pStudyDesc;
    DB_Private_Handle	*phandle ;

    DB_createHandle (storeArea, -1, -1, (DB_Handle **) &phandle) ;

    pStudyDesc = (StudyDescRecord *)malloc (SIZEOF_STUDYDESC) ;
    if (pStudyDesc == NULL) {
	CERR << "DB_PrintIndexFile: out of memory" << endl;
	return;
    }

    DB_lock(phandle, OFFalse);

    DB_GetStudyDesc(phandle, pStudyDesc);

    for (i=0; i<phandle->maxStudiesAllowed; i++) {
	if (pStudyDesc[i].NumberofRegistratedImages != 0 ) {
	    COUT << "******************************************************" << endl
	        << "STUDY DESCRIPTOR: " << i << endl
	        << "  Study UID: " << pStudyDesc[i].StudyInstanceUID << endl
	        << "  StudySize: " << pStudyDesc[i].StudySize << endl
	        << "  LastRecDate: " << pStudyDesc[i].LastRecordedDate << endl
	        << "  NumOfImages: " << pStudyDesc[i].NumberofRegistratedImages << endl;
	}
    }

    DB_IdxInitLoop (phandle, &j) ;
    while (1) {
    	if (DB_IdxGetNext (phandle, &j, &idxRec) != EC_Normal)
    	    break ;
    
    	COUT << "*******************************************************" << endl;
    	COUT << "RECORD NUMBER: " << j << endl << "  Status: ";
    	if (idxRec.hstat == DVIF_objectIsNotNew)
    	    COUT << "is NOT new" << endl;
        else
            COUT << "is new" << endl;
    	COUT << "  Filename: " << idxRec.filename << endl
    	     << "  ImageSize: " << idxRec.ImageSize << endl
    	     << "  RecordedDate: " << idxRec.RecordedDate << endl;
    	for (i = 0 ; i < NBPARAMETERS ; i++) {	/* new definition */
    	    DB_SmallDcmElmt *se = idxRec.param + i;
    	    const char* value = "";
    	    if (se->PValueField != NULL) value = se->PValueField;
    	    DcmTag tag(se->XTag);
    	    COUT << "    " << tag.getTagName() << ": \"" << value << "\"" << endl;
    	}
  	    COUT << "  InstanceDescription: \"" << idxRec.InstanceDescription << "\"" << endl;
    }
    COUT << "*******************************************************" << endl
         << "RECORDS IN THIS INDEXFILE: " << j << endl;

    DB_unlock(phandle);
	
    DB_destroyHandle ((DB_Handle **) &phandle) ;
}

/*
** CVS Log
** $Log: dbindex.cc,v $
** Revision 1.1  2005/08/23 19:32:09  braindead
** - initial savannah import
**
** Revision 1.1  2005/06/26 19:26:14  pipelka
** - added dcmtk
**
** Revision 1.26  2004/02/13 13:17:24  joergr
** Adapted code for changed tag names (e.g. PresentationLabel -> ContentLabel).
**
** Revision 1.25  2002/11/27 13:27:54  meichel
** Adapted module imagectn to use of new header file ofstdinc.h
**
** Revision 1.24  2001/10/12 12:43:09  meichel
** Adapted imagectn to OFCondition based dcmnet module (supports strict mode).
**
** Revision 1.23  2001/06/01 15:51:28  meichel
** Updated copyright header
**
** Revision 1.22  2000/10/16 11:35:51  joergr
** Replaced presentation description by a more general instance description.
**
** Revision 1.21  2000/07/04 09:11:20  joergr
** Modified output of 'print index file' option.
**
** Revision 1.20  2000/05/30 13:12:31  joergr
** Fixed bug in output message.
**
** Revision 1.19  2000/03/08 16:41:09  meichel
** Updated copyright header.
**
** Revision 1.18  2000/03/03 14:16:39  meichel
** Implemented library support for redirecting error messages into memory
**   instead of printing them to stdout/stderr for GUI applications.
**
** Revision 1.17  2000/02/23 15:13:30  meichel
** Corrected macro for Borland C++ Builder 4 workaround.
**
** Revision 1.16  2000/02/01 11:43:45  meichel
** Avoiding to include <stdlib.h> as extern "C" on Borland C++ Builder 4,
**   workaround for bug in compiler header files.
**
** Revision 1.15  1999/07/14 12:03:41  meichel
** Updated data dictionary for supplement 29, 39, 33_lb, CP packet 4 and 5.
**   Corrected dcmtk applications for changes in attribute name constants.
**
** Revision 1.14  1999/06/10 12:12:16  meichel
** Adapted imagectn to new command line option scheme.
**   Added support for Patient/Study Only Q/R model and C-GET (experimental).
**
** Revision 1.13  1999/04/29 15:24:09  joergr
** Added PresentationLabel to index file.
**
** Revision 1.12  1999/04/28 09:22:29  joergr
** Added item PresentationDescription to index record.
**
** Revision 1.11  1999/03/22 09:56:33  meichel
** Reworked data dictionary based on the 1998 DICOM edition and the latest
**   supplement versions. Corrected dcmtk applications for minor changes
**   in attribute name constants.
**
** Revision 1.10  1999/01/29 12:40:12  meichel
** Simplified some index file related code in imagectn to ease maintenance.
**
** Revision 1.9  1999/01/28 15:21:09  vorwerk
** Initialisation of all record attributes implemented.
**
** Revision 1.8  1998/12/22 14:40:20  vorwerk
** Added output of DVIhierarchyStatus in DB_PrintIndexFile
**
** Revision 1.7  1998/08/10 08:56:49  meichel
** renamed member variable in DIMSE structures from "Status" to "DimseStatus".
**
** Revision 1.6  1997/10/06 13:48:02  hewett
** Minor correction to imagectn's index file code to use the changed
** attribute names from dcdeftag.h
**
** Revision 1.5  1997/07/21 08:59:56  andreas
** - Replace all boolean types (BOOLEAN, CTNBOOLEAN, DICOM_BOOL, BOOL)
**   with one unique boolean type OFBool.
**
** Revision 1.4  1996/09/27 08:46:50  hewett
** Enclosed system include files with BEGIN_EXTERN_C/END_EXTERN_C.
**
** Revision 1.3  1996/04/25 16:31:06  hewett
** Revised lseek checking to allow for zero length file case.
**
** Revision 1.2  1996/04/22 11:18:56  hewett
** Added sanity checking to calls of lseek().
**
** Revision 1.1.1.1  1996/03/28 19:24:59  hewett
** Oldenburg Image CTN Software ported to use the dcmdata C++ toolkit.
**
**
*/
