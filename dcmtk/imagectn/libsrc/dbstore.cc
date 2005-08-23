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
 *  Purpose: routines which provide store facilities for the DB Module.
 *    Module Prefix: DB_
 *
 *  Last Update:      $Author: braindead $
 *  Update Date:      $Date: 2005/08/23 19:32:09 $
 *  Source File:      $Source: /cvsroot/aeskulap/aeskulap/dcmtk/imagectn/libsrc/dbstore.cc,v $
 *  CVS/RCS Revision: $Revision: 1.1 $
 *  Status:           $State: Exp $
 *
 *  CVS/RCS Log at end of file
 *
 */

#include "osconfig.h"    /* make sure OS specific configuration is included first */

#define INCLUDE_CSTDLIB
#define INCLUDE_CSTDIO
#define INCLUDE_CTIME
#define INCLUDE_CERRNO
#include "ofstdinc.h"

BEGIN_EXTERN_C
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_FILE_H
#include <sys/file.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
END_EXTERN_C

#include "dcompat.h"
#include "imagedb.h"
#include "dbpriv.h"
#include "dcfilefo.h"
#include "dimse.h"
#include "ofconsol.h"
#include "dbcond.h"

static OFBool quotaSystemEnabled = OFTrue;

void DB_enableQuotaSystem(OFBool enable)
{
    quotaSystemEnabled = enable;
}


/*
** Image file deleting
*/


OFCondition DB_deleteImageFile(char* imgFile)
{
    if (!quotaSystemEnabled) {
    CERR << "DB QUOTA: disabled: retaining: " << imgFile << endl;
    return EC_Normal;
    } else {
    CERR << "DB QUOTA: enabled: deleting: " << imgFile << endl;
    }

#ifdef LOCK_IMAGE_FILES
    int lockfd;
#ifdef O_BINARY
    lockfd = open(imgFile, O_RDWR | O_BINARY, 0666);    /* obtain file descriptor */
#else
    lockfd = open(imgFile, O_RDWR, 0666);   /* obtain file descriptor */
#endif
    if (lockfd < 0) {
    CERR << "DB ERROR: cannot open image file for deleting: " << imgFile << endl;
    return IMAGECTN_DB_ERROR;
    }
    if (dcmtk_flock(lockfd, LOCK_EX) < 0) { /* exclusive lock (blocking) */
    CERR << "DB ERROR: cannot lock image file  for deleting: " << imgFile << endl;
        dcmtk_plockerr("DB ERROR");
    }
#endif

    if (unlink(imgFile) < 0) {
        /* delete file */
    CERR << "DB ERROR: cannot delete image file: " << imgFile << endl
        << "IMAGECTN_DB_ERROR: " << strerror(errno) << endl;
   }

#ifdef LOCK_IMAGE_FILES
    if (dcmtk_flock(lockfd, LOCK_UN) < 0) { /* unlock */
    CERR << "DB ERROR: cannot unlock image file  for deleting: " << imgFile << endl;
        dcmtk_plockerr("DB ERROR");
     }
    close(lockfd);              /* release file descriptor */
#endif

    return EC_Normal;
}


/*************************
**   Delete oldest study in database
 */

extern int
DB_DeleteOldestStudy(DB_Private_Handle *phandle, StudyDescRecord *pStudyDesc)
{
    int oldestStudy ;
    double OldestDate ;
    int s ;
    int n ;
    int idx = 0 ;
    IdxRecord idxRec ;

    oldestStudy = 0 ;
    OldestDate = 0.0 ;

#ifdef DEBUG
    DB_debug(1, "DB_DeleteOldestStudy\n") ;
#endif

    for ( s = 0 ; s < phandle -> maxStudiesAllowed ; s++ ) {
    if ( ( pStudyDesc[s]. NumberofRegistratedImages != 0 ) &&
        ( ( OldestDate == 0.0 ) || ( pStudyDesc[s]. LastRecordedDate < OldestDate ) ) ) {
        OldestDate = pStudyDesc[s]. LastRecordedDate ;
        oldestStudy = s ;
    }
    }

#ifdef DEBUG
    DB_debug(1, "DB_DeleteOldestStudy oldestStudy = %d\n",oldestStudy) ;
#endif

    n = strlen(pStudyDesc[oldestStudy].StudyInstanceUID) ;
    while ( DB_IdxRead (phandle, idx, &idxRec) == EC_Normal ) {

    if ( ! ( strncmp(idxRec. StudyInstanceUID, pStudyDesc[oldestStudy].StudyInstanceUID, n) ) ) {
        DB_IdxRemove (phandle, idx) ;
        DB_deleteImageFile(idxRec.filename);
    }
    idx++ ;
    }

    pStudyDesc[oldestStudy].NumberofRegistratedImages = 0 ;
    pStudyDesc[oldestStudy].StudySize = 0 ;
    return(oldestStudy) ;
}




/*************************
**   Delete oldest images in database
 */

OFCondition
DB_DeleteOldestImages(DB_Private_Handle *phandle, StudyDescRecord *pStudyDesc, int StudyNum, char *StudyUID, long RequiredSize)
{

    ImagesofStudyArray *StudyArray ;
    IdxRecord idxRec ;
    int nbimages = 0 , s = 0 , n ;
    long DeletedSize ;

#ifdef DEBUG
    DB_debug(1, "DB_DeleteOldestImages RequiredSize = %ld\n",RequiredSize) ;
#endif
    n = strlen(StudyUID) ;
    StudyArray = (ImagesofStudyArray *)malloc(MAX_NUMBER_OF_IMAGES * sizeof(ImagesofStudyArray)) ;

    if (StudyArray == NULL) {
    CERR << "DB_DeleteOldestImages: out of memory" << endl;
    return IMAGECTN_DB_ERROR;
    }

    /** Find all images having the same StudyUID
     */

    DB_IdxInitLoop (phandle, &(phandle -> idxCounter)) ;
    while ( DB_IdxGetNext(phandle, &(phandle -> idxCounter), &idxRec) == EC_Normal ) {
    if ( ! ( strncmp(idxRec. StudyInstanceUID, StudyUID, n) ) ) {

        StudyArray[nbimages]. idxCounter = phandle -> idxCounter ;
        StudyArray[nbimages]. RecordedDate = idxRec. RecordedDate ;
        StudyArray[nbimages++]. ImageSize = idxRec. ImageSize ;
    }
    }

    /** Sort the StudyArray in order to have the oldest images first
     */
    qsort((char *)StudyArray, nbimages, sizeof(ImagesofStudyArray), DB_Compare) ;

#ifdef DEBUG
    {
    int i ;
    DB_debug(1, "DB_DeleteOldestImages : Sorted images ref array\n") ;
    for (i = 0 ; i < nbimages ; i++)
        DB_debug(1, "[%2d] :   Size %ld   Date %20.3f   Ref %d \n",
            i, StudyArray[i]. ImageSize, StudyArray[i]. RecordedDate, StudyArray[i]. idxCounter) ;
    DB_debug(1, "DB_DeleteOldestImages : end of ref array\n") ;
    }
#endif

    s = 0 ;
    DeletedSize = 0 ;

    while ( DeletedSize < RequiredSize ) {

    IdxRecord idxRemoveRec ;
    DB_IdxRead (phandle, StudyArray[s]. idxCounter, &idxRemoveRec) ;
#ifdef DEBUG
    DB_debug(1, "Removing file : %s\n", idxRemoveRec. filename) ;
#endif
    DB_deleteImageFile(idxRemoveRec.filename);

    DB_IdxRemove (phandle, StudyArray[s]. idxCounter) ;
    pStudyDesc[StudyNum].NumberofRegistratedImages -= 1 ;
    pStudyDesc[StudyNum].StudySize -= StudyArray[s]. ImageSize ;
    DeletedSize += StudyArray[s++]. ImageSize ;
    }

#ifdef DEBUG
    DB_debug(1, "DB_DeleteOldestImages DeletedSize = %d\n",
         (int)DeletedSize) ;
#endif
    free(StudyArray) ;
    return( EC_Normal ) ;

}



/*************************
 *   Verify if study UID already exists
 *   If the study UID exists, its index in the study descriptor is returned.
 *   If the study UID does not exist, the index of the first unused descriptor entry is returned.
 *   If no entries are free, maxStudiesAllowed is returned.
 */

extern int
DB_MatchStudyUIDInStudyDesc (StudyDescRecord *pStudyDesc, char *StudyUID, int maxStudiesAllowed)
{
    int s = 0 ;
    while  (s < maxStudiesAllowed)
    {
      if ((pStudyDesc[s].NumberofRegistratedImages > 0) && (0 == strcmp(pStudyDesc[s].StudyInstanceUID, StudyUID))) break;
      s++ ;
    }
    if (s==maxStudiesAllowed) // study uid does not exist, look for free descriptor
    {
      s=0;
      while  (s < maxStudiesAllowed)
      {
        if (pStudyDesc[s].NumberofRegistratedImages == 0) break;
        s++ ;
      }
    }
    return s;
}


/*************************
**  Check up storage rights in Study Desk record
 */

OFCondition
DB_CheckupinStudyDesc(DB_Private_Handle *phandle, StudyDescRecord *pStudyDesc, char *StudyUID, long imageSize)
{
    int         s ;
    long        RequiredSize ;

    s = DB_MatchStudyUIDInStudyDesc (pStudyDesc, StudyUID,
                     (int)(phandle -> maxStudiesAllowed)) ;

    /** If Study already exists
     */

    if ( pStudyDesc[s]. NumberofRegistratedImages != 0 ) {

#ifdef DEBUG
    DB_debug(1, "DB_CheckupinStudyDesc: study already exists : %d\n",s) ;
#endif
    if ( ( pStudyDesc[s]. StudySize + imageSize )
         > phandle -> maxBytesPerStudy ) {
        if ( imageSize > phandle -> maxBytesPerStudy ) {
#ifdef DEBUG
        DB_debug(1,
             "DB_CheckupinStudyDesc: imageSize = %ld too large\n",
             imageSize) ;
#endif
        return ( IMAGECTN_DB_ERROR ) ;
        }

        RequiredSize = imageSize -
        ( phandle -> maxBytesPerStudy - pStudyDesc[s]. StudySize ) ;
        DB_DeleteOldestImages(phandle, pStudyDesc, s,
                  StudyUID, RequiredSize) ;
    }


    }
    else {
#ifdef DEBUG
    DB_debug(1, "DB_CheckupinStudyDesc: study doesn't already exist\n") ;
#endif
    if ( imageSize > phandle -> maxBytesPerStudy ) {
#ifdef DEBUG
        DB_debug(1, "DB_CheckupinStudyDesc: imageSize = %ld too large\n",
             imageSize) ;
#endif
        return ( IMAGECTN_DB_ERROR ) ;
    }
    if ( s > ( phandle -> maxStudiesAllowed - 1 ) )
        s = DB_DeleteOldestStudy(phandle, pStudyDesc) ;

    }

    pStudyDesc[s]. StudySize += imageSize ;
#ifdef DEBUG
    DB_debug(1, "DB_CheckupinStudyDesc: ~~~~~~~~ StudySize = %ld\n",
         pStudyDesc[s]. StudySize) ;
#endif

    /* we only have second accuracy */
    pStudyDesc[s]. LastRecordedDate =  (double) time(NULL);

    pStudyDesc[s]. NumberofRegistratedImages++ ;
    strcpy(pStudyDesc[s].StudyInstanceUID,StudyUID) ;

    if ( DB_StudyDescChange (phandle, pStudyDesc) == EC_Normal)
    return ( EC_Normal ) ;
    else
    return ( IMAGECTN_DB_ERROR ) ;


}

/*
 * If the image is already stored remove it from the database.
 * hewett - Nov. 1, 93
 */
OFCondition DB_removeDuplicateImage(DB_Private_Handle *phandle,
    const char *SOPInstanceUID, const char *StudyInstanceUID,
    StudyDescRecord *pStudyDesc, const char *newImageFileName)
{
    int idx = 0;
    IdxRecord       idxRec ;
    int studyIdx = 0;

    studyIdx = DB_MatchStudyUIDInStudyDesc (pStudyDesc, (char*)StudyInstanceUID,
                        (int)(phandle -> maxStudiesAllowed)) ;

    if ( pStudyDesc[studyIdx].NumberofRegistratedImages == 0 ) {
    /* no study images, cannot be any old images */
    return EC_Normal;
    }

    while (DB_IdxRead(phandle, idx, &idxRec) == EC_Normal) {

    if (strcmp(idxRec.SOPInstanceUID, SOPInstanceUID) == 0) {

#ifdef DEBUG
        DB_debug(1,"--- Removing Existing DB Image Record: %s\n",
           idxRec.filename) ;
#endif
        /* remove the idx record  */
        DB_IdxRemove (phandle, idx);
        /* only remove the image file if it is different than that
         * being entered into the database.
         */
        if (strcmp(idxRec.filename, newImageFileName) != 0) {
        DB_deleteImageFile(idxRec.filename);
        }
        /* update the study info */
        pStudyDesc[studyIdx].NumberofRegistratedImages--;
        pStudyDesc[studyIdx].StudySize -= idxRec.ImageSize;
    }
    idx++;
    }
    /* the study record should be written to file later */
    return EC_Normal;
}


/*************************
**  Add data from imageFileName to database
 */

OFCondition
DB_storeRequest (
    DB_Handle   *handle,
    const char  *SOPClassUID,
    const char  * /*SOPInstanceUID*/,
    const char  *imageFileName,
    DB_Status   *status,
    OFBool      isNew)
{
    DB_Private_Handle   *phandle ;
    IdxRecord           idxRec ;
    StudyDescRecord     *pStudyDesc ;
    int                 i ;
    struct stat         buf ;


    /**** Initialize an IdxRecord
    ***/

    bzero((char*)&idxRec, sizeof(idxRec));

    DB_IdxInitRecord (&idxRec, 0) ;

    strncpy(idxRec.filename, imageFileName, DBC_MAXSTRING);
#ifdef DEBUG
    DB_debug(1,"DB_storeRequest () : storage request of file : %s\n",idxRec . filename) ;
#endif
    strncpy (idxRec.SOPClassUID, SOPClassUID, UI_MAX_LENGTH);

    /**** Get IdxRec values from ImageFile
    ***/

    DcmFileFormat dcmff;
    if (dcmff.loadFile(imageFileName).bad())
    {
      CERR << "DB: Cannot open file: " << imageFileName << ": "
           << strerror(errno) << endl;
      status->status = STATUS_STORE_Error_CannotUnderstand ;
      return (IMAGECTN_DB_ERROR) ;
    }

    DcmDataset *dset = dcmff.getDataset();

    for (i = 0 ; i < NBPARAMETERS ; i++ ) {
        OFCondition ec = EC_Normal;
        DB_SmallDcmElmt *se = idxRec.param + i;

        const char *strPtr = NULL;
        ec = dset->findAndGetString(se->XTag, strPtr);
        if ((ec != EC_Normal) || (strPtr == NULL)) {
            /* not found or empty */
            se->PValueField[0] = '\0';
            se->ValueLength = 0;
        } else {
            /* found and non-empty */
            strncpy(se->PValueField, strPtr, (size_t)(se->ValueLength));
            /* important: do not change the ValueLength field before the string is copied! */
            se->ValueLength = strlen(se->PValueField);
        }
    }

    /* InstanceStatus */
    idxRec.hstat = (isNew) ? DVIF_objectIsNew : DVIF_objectIsNotNew;

    /* InstanceDescription */
    OFBool useDescrTag = OFTrue;
    DcmTagKey descrTag = DCM_ImageComments;
    if (SOPClassUID != NULL)
    {
        /* fill in value depending on SOP class UID (content might be improved) */
        if (strcmp(SOPClassUID, UID_GrayscaleSoftcopyPresentationStateStorage) == 0)
        {
            descrTag = DCM_ContentDescription;
        } else if (strcmp(SOPClassUID, UID_HardcopyGrayscaleImageStorage) == 0)
        {
            strcpy(idxRec.InstanceDescription, "Hardcopy Grayscale Image");
            useDescrTag = OFFalse;
        } else if ((strcmp(SOPClassUID, UID_BasicTextSR) == 0) ||
                   (strcmp(SOPClassUID, UID_EnhancedSR) == 0) ||
                   (strcmp(SOPClassUID, UID_ComprehensiveSR) == 0))
        {
            OFString string;
            OFString description = "unknown SR";
            const char *name = dcmFindNameOfUID(SOPClassUID);
            if (name != NULL)
                description = name;
            if (dset->findAndGetOFString(DCM_VerificationFlag, string) == EC_Normal)
            {
                description += ", ";
                description += string;
            }
            if (dset->findAndGetOFString(DCM_CompletionFlag, string) == EC_Normal)
            {
                description += ", ";
                description += string;
            }
            if (dset->findAndGetOFString(DCM_CompletionFlagDescription, string) == EC_Normal)
            {
                description += ", ";
                description += string;
            }
            strncpy(idxRec.InstanceDescription, description.c_str(), DESCRIPTION_MAX_LENGTH);
            useDescrTag = OFFalse;
        } else if (strcmp(SOPClassUID, UID_StoredPrintStorage) == 0)
        {
            strcpy(idxRec.InstanceDescription, "Stored Print");
            useDescrTag = OFFalse;
        }
    }
    /* get description from attribute specified above */
    if (useDescrTag)
    {
        OFString string;
        /* return value is irrelevant */
        dset->findAndGetOFString(descrTag, string);
        strncpy(idxRec.InstanceDescription, string.c_str(), DESCRIPTION_MAX_LENGTH);
    }
    /* is dataset digitally signed? */
    if (strlen(idxRec.InstanceDescription) + 9 < DESCRIPTION_MAX_LENGTH)
    {
        DcmStack stack;
        if (dset->search(DCM_DigitalSignaturesSequence, stack, ESM_fromHere, OFTrue /* searchIntoSub */) == EC_Normal)
        {
            /* in principle it should be checked whether there is _any_ non-empty digital signatures sequence, but ... */
            if (((DcmSequenceOfItems *)stack.top())->card() > 0)
            {
                if (strlen(idxRec.InstanceDescription) > 0)
                    strcat(idxRec.InstanceDescription, " (Signed)");
                else
                    strcpy(idxRec.InstanceDescription, "Signed Instance");
            }
        }
    }

    /**** Print Elements
    ***/

#ifdef DEBUG
    DB_debug(1,"-- BEGIN Parameters to Register in DB\n") ;
    for (i = 0 ; i < NBPARAMETERS ; i++) {  /* new definition */
    DB_SmallDcmElmt *se = idxRec.param + i;
    const char* value = "";
    if (se->PValueField != NULL) value = se->PValueField;
    DcmTag tag(se->XTag);
    DB_debug(1," %s: \"%s\"\n", tag.getTagName(), value);
    }
    DB_debug(1,"-- END Parameters to Register in DB\n") ;
#endif

    /**** Goto the end of IndexFile, and write the record
    ***/

    phandle = (DB_Private_Handle *) handle ;

    DB_lock(phandle, OFTrue);

    pStudyDesc = (StudyDescRecord *)malloc (SIZEOF_STUDYDESC) ;
    if (pStudyDesc == NULL) {
      CERR << "DB_storeRequest: out of memory" << endl;
      status -> status = STATUS_STORE_Refused_OutOfResources ;
      DB_unlock(phandle);
      return (IMAGECTN_DB_ERROR) ;
    }

    bzero((char *)pStudyDesc, SIZEOF_STUDYDESC);
    DB_GetStudyDesc(phandle, pStudyDesc) ;

    stat(imageFileName, &buf) ;
    idxRec. ImageSize = (int)(buf. st_size) ;

    /* we only have second accuracy */
    idxRec. RecordedDate =  (double) time(NULL);

    /*
     * If the image is already stored remove it from the database.
     * hewett - Nov. 1, 93
     */

    DB_removeDuplicateImage(phandle, idxRec.SOPInstanceUID,
                idxRec.StudyInstanceUID, pStudyDesc,
                imageFileName);


    if ( DB_CheckupinStudyDesc(phandle, pStudyDesc, idxRec. StudyInstanceUID, idxRec. ImageSize) != EC_Normal ) {
    free (pStudyDesc) ;
    status -> status = STATUS_STORE_Refused_OutOfResources ;

    DB_unlock(phandle);

    return (IMAGECTN_DB_ERROR) ;
    }

    free (pStudyDesc) ;

    if (DB_IdxAdd (phandle, &i, &idxRec) == EC_Normal)
    {
    status -> status = STATUS_Success ;
    DB_unlock(phandle);
    return (EC_Normal) ;
    }
    else
    {
    status -> status = STATUS_STORE_Refused_OutOfResources ;
    DB_unlock(phandle);
    }
    return IMAGECTN_DB_ERROR;
}

/*
** Prune invalid DB records.
*/

OFCondition DB_pruneInvalidRecords(DB_Handle *dbHandle)
{
    DB_Private_Handle *phandle = (DB_Private_Handle *)dbHandle;
    int idx = 0;
    IdxRecord idxRec ;
    StudyDescRecord *pStudyDesc;

    DB_lock(phandle, OFTrue);

    pStudyDesc = (StudyDescRecord *)malloc (SIZEOF_STUDYDESC) ;
    if (pStudyDesc == NULL) {
    CERR << "DB_pruneInvalidRecords: out of memory" << endl;
    DB_unlock(phandle);
    return (IMAGECTN_DB_ERROR) ;
    }

    for (int i = 0 ; i < phandle -> maxStudiesAllowed ; i++ )
    pStudyDesc[i]. NumberofRegistratedImages = 0 ;

    DB_GetStudyDesc(phandle, pStudyDesc) ;

    while (DB_IdxRead(phandle, idx, &idxRec) == EC_Normal) {

    if (access(idxRec.filename, R_OK) < 0) {

#ifdef DEBUG
        DB_debug(1,"*** Pruning Invalid DB Image Record: %s\n",
           idxRec.filename);
#endif
        /* update the study info */
        int studyIdx = DB_MatchStudyUIDInStudyDesc(
        pStudyDesc, idxRec.StudyInstanceUID,
        (int)(phandle->maxStudiesAllowed)) ;
        if (studyIdx < phandle->maxStudiesAllowed) {
        if (pStudyDesc[studyIdx].NumberofRegistratedImages > 0) {
            pStudyDesc[studyIdx].NumberofRegistratedImages--;
        } else {
            pStudyDesc[studyIdx].NumberofRegistratedImages = 0;
            pStudyDesc[studyIdx].StudySize = 0;
            pStudyDesc[studyIdx].StudyInstanceUID[0] = '\0';
        }
        if (pStudyDesc[studyIdx].StudySize > idxRec.ImageSize) {
            pStudyDesc[studyIdx].StudySize -= idxRec.ImageSize;
        }
        }

        /* remove the idx record  */
        DB_IdxRemove (phandle, idx);

    }
    idx++;
    }

    DB_StudyDescChange (phandle, pStudyDesc);

    DB_unlock(phandle);

    free (pStudyDesc) ;

    return EC_Normal;
}

/*
** CVS Log
** $Log: dbstore.cc,v $
** Revision 1.1  2005/08/23 19:32:09  braindead
** - initial savannah import
**
** Revision 1.1  2005/06/26 19:26:14  pipelka
** - added dcmtk
**
** Revision 1.35  2004/02/13 13:17:24  joergr
** Adapted code for changed tag names (e.g. PresentationLabel -> ContentLabel).
**
** Revision 1.34  2002/12/13 13:43:30  meichel
** Removed unused code reported by the MIPSpro 7.3 optimizer
**
** Revision 1.33  2002/11/27 13:27:55  meichel
** Adapted module imagectn to use of new header file ofstdinc.h
**
** Revision 1.32  2002/08/20 12:22:53  meichel
** Adapted code to new loadFile and saveFile methods, thus removing direct
**   use of the DICOM stream classes.
**
** Revision 1.31  2001/12/18 10:31:36  meichel
** Added typecast to avoid warning on Sun CC 2.0.1
**
** Revision 1.30  2001/10/12 12:43:10  meichel
** Adapted imagectn to OFCondition based dcmnet module (supports strict mode).
**
** Revision 1.29  2001/10/02 11:50:10  joergr
** Introduced new general purpose functions to get/put DICOM element values
** from/to an item/dataset - removed some old and rarely used functions.
**
** Revision 1.28  2001/09/28 14:04:02  joergr
** Corrected bug in DB_DeleteOldestStudy().
**
** Revision 1.27  2001/09/26 16:06:38  meichel
** Adapted imagectn to class OFCondition
**
** Revision 1.26  2001/06/01 15:51:29  meichel
** Updated copyright header
**
** Revision 1.25  2001/02/20 12:36:16  joergr
** Added detection of signed instances to DB_storeRequest(). This mechanism is
** used to add a short note to the instance description (in the index file)
** that an instance is digitally signed - database index file format has _not_
** changed.
**
** Revision 1.24  2000/12/15 13:25:13  meichel
** Declared qsort() and signal() callback functions as extern "C", avoids
**   warnings on Sun C++ 5.x compiler.
**
** Revision 1.23  2000/11/23 16:40:56  joergr
** Added new command line option to dbregimg allowing to specify whether
** instance reviewed status of newly registered objects should be set to 'new'
** or 'not new'.
**
** Revision 1.22  2000/11/10 17:47:25  joergr
** Enhanced instance description for structured reports.
**
** Revision 1.21  2000/10/16 11:35:52  joergr
** Replaced presentation description by a more general instance description.
**
** Revision 1.20  2000/03/08 16:41:09  meichel
** Updated copyright header.
**
** Revision 1.19  2000/03/03 14:16:39  meichel
** Implemented library support for redirecting error messages into memory
**   instead of printing them to stdout/stderr for GUI applications.
**
** Revision 1.18  2000/02/23 15:13:32  meichel
** Corrected macro for Borland C++ Builder 4 workaround.
**
** Revision 1.17  2000/02/01 11:43:46  meichel
** Avoiding to include <stdlib.h> as extern "C" on Borland C++ Builder 4,
**   workaround for bug in compiler header files.
**
** Revision 1.16  1999/11/12 16:51:15  meichel
** Corrected file locking code that did not work correctly under Win95/98.
**
** Revision 1.15  1999/09/15 15:43:26  meichel
** Fixed imagectn DB problem resulting from an uninitialized structure.
**
** Revision 1.14  1999/09/14 18:14:30  meichel
** Fixed imagectn DB problem resulting from an uninitialized structure.
**
** Revision 1.13  1999/06/10 12:12:18  meichel
** Adapted imagectn to new command line option scheme.
**   Added support for Patient/Study Only Q/R model and C-GET (experimental).
**
** Revision 1.12  1999/04/30 16:37:16  meichel
** Renamed all flock calls to dcmtk_flock to avoid name clash between flock()
** emulation based on fcntl() and a constructor for struct flock.
**
** Revision 1.11  1999/02/01 14:09:35  vorwerk
** DB_MatchStudyUIDInStudyDesc searchroutine changed to remove dbregimg quota-bug.
**
** Revision 1.10  1999/01/27 15:37:15  vorwerk
** Changes made in DB_MatchStudyUIDinStudyDesc to enable a search over the whole indexfile.
**
** Revision 1.9  1998/12/22 14:57:08  vorwerk
** Added initialization of DVIhierarchyStatus in DB_storeRequest
**
** Revision 1.8  1997/09/18 08:11:05  meichel
** Many minor type conflicts (e.g. long passed as int) solved.
**
** Revision 1.7  1997/08/06 12:20:19  andreas
** - Using Windows NT with Visual C++ 4.x the standard open mode for files
**   is TEXT with conversions. For binary files (image files, imagectn database
**   index) this must be changed (e.g. fopen(filename, "...b"); or
**   open(filename, ..... |O_BINARY);)
**
** Revision 1.6  1997/07/21 08:59:58  andreas
** - Replace all boolean types (BOOLEAN, CTNBOOLEAN, DICOM_BOOL, BOOL)
**   with one unique boolean type OFBool.
**
** Revision 1.5  1996/09/27 08:46:51  hewett
** Enclosed system include files with BEGIN_EXTERN_C/END_EXTERN_C.
**
** Revision 1.4  1996/05/06 07:39:09  hewett
** Added explicit initialization (bzero) of idx record.  Rearranged
** size of string attributes in idx record.
**
** Revision 1.3  1996/04/29 10:15:48  hewett
** Added global flag to enable/disable quota system from deleting
** image files.  By default this flag is true (images will be
** deleted).  The flag can be set true/false via the D
** B_enableQuotaSystem() function.
**
** Revision 1.2  1996/04/22 11:22:54  hewett
** Added function to prune the db index file of records pointing to
** non-existant image files.
**
** Revision 1.1.1.1  1996/03/28 19:25:00  hewett
** Oldenburg Image CTN Software ported to use the dcmdata C++ toolkit.
**
**
*/
