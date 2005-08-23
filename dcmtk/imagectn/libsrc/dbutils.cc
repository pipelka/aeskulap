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
 *  Purpose: various facilities functions used to implement
 *    the DB Module. This file also contains DB_Handle manipulation functions.
 *    Module Prefix: DB_
 *
 *  Last Update:      $Author: braindead $
 *  Update Date:      $Date: 2005/08/23 19:32:09 $
 *  Source File:      $Source: /cvsroot/aeskulap/aeskulap/dcmtk/imagectn/libsrc/dbutils.cc,v $
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
#define INCLUDE_CERRNO
#define INCLUDE_CCTYPE
#include "ofstdinc.h"

BEGIN_EXTERN_C
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_SYS_FILE_H
#include <sys/file.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
END_EXTERN_C

#include "imagedb.h"
#include "dbpriv.h"
#include "dcdatset.h"
#include "dcdeftag.h"
#include "diutil.h"
#include "offname.h"
#include "ofconsol.h"
#include "ofstd.h"
#include "dbcond.h"

/*************************
 *
 *      Static Data
 *
 *************************/

/**** The TbFindAttr table contains the description of tags (keys) supported
 **** by the DB Module.
 **** Tags described here have to be present in the Index Record file.
 **** The order is unsignificant.
 ****
 **** Each element of this table is described by
 ****           The tag value
 ****           The level of this tag (from patient to image)
 ****           The Key Type (only UNIQUE_KEY values is used)
 ****           The key matching type, specifiing which type of
 ****                   matching should be performed. The OTHER_CLASS
 ****                   value specifies that only strict comparison is applied.
 ****
 **** This table and the IndexRecord structure should contain at least
 **** all Unique and Required keys.
 ***/

static DB_FindAttr TbFindAttr [] = {
        DB_FindAttr( DCM_PatientsBirthDate ,            PATIENT_LEVEL,  OPTIONAL_KEY,   DATE_CLASS      ),
        DB_FindAttr( DCM_PatientsSex,                   PATIENT_LEVEL,  OPTIONAL_KEY,   STRING_CLASS    ),
        DB_FindAttr( DCM_PatientsName,                  PATIENT_LEVEL,  REQUIRED_KEY,   STRING_CLASS    ),
        DB_FindAttr( DCM_PatientID,                     PATIENT_LEVEL,  UNIQUE_KEY,             STRING_CLASS    ),
        DB_FindAttr( DCM_PatientsBirthTime,             PATIENT_LEVEL,  OPTIONAL_KEY,   TIME_CLASS      ),
        DB_FindAttr( DCM_OtherPatientIDs,                       PATIENT_LEVEL,  OPTIONAL_KEY,   STRING_CLASS    ),
        DB_FindAttr( DCM_OtherPatientNames,             PATIENT_LEVEL,  OPTIONAL_KEY,   STRING_CLASS    ),
        DB_FindAttr( DCM_EthnicGroup,                   PATIENT_LEVEL,  OPTIONAL_KEY,   STRING_CLASS    ),
        DB_FindAttr( DCM_PatientComments,                       PATIENT_LEVEL,  OPTIONAL_KEY,   STRING_CLASS    ),
        DB_FindAttr( DCM_NumberOfPatientRelatedStudies, PATIENT_LEVEL,  OPTIONAL_KEY,   STRING_CLASS    ),
        DB_FindAttr( DCM_NumberOfPatientRelatedSeries,  PATIENT_LEVEL,  OPTIONAL_KEY,   STRING_CLASS    ),
        DB_FindAttr( DCM_NumberOfPatientRelatedInstances,       PATIENT_LEVEL,  OPTIONAL_KEY,   STRING_CLASS    ),
        DB_FindAttr( DCM_StudyDate,                     STUDY_LEVEL,    REQUIRED_KEY,   DATE_CLASS      ),
        DB_FindAttr( DCM_StudyTime,                     STUDY_LEVEL,    REQUIRED_KEY,   TIME_CLASS      ),
        DB_FindAttr( DCM_StudyID,                               STUDY_LEVEL,    REQUIRED_KEY,   STRING_CLASS    ),
        DB_FindAttr( DCM_AccessionNumber,                       STUDY_LEVEL,    REQUIRED_KEY,   STRING_CLASS    ),
        DB_FindAttr( DCM_ReferringPhysiciansName,               STUDY_LEVEL,    OPTIONAL_KEY,   STRING_CLASS    ),
        DB_FindAttr( DCM_StudyDescription,                      STUDY_LEVEL,    OPTIONAL_KEY,   STRING_CLASS    ),
        DB_FindAttr( DCM_NameOfPhysiciansReadingStudy,          STUDY_LEVEL,    OPTIONAL_KEY,   STRING_CLASS    ),
        DB_FindAttr( DCM_StudyInstanceUID,                      STUDY_LEVEL,    UNIQUE_KEY,             UID_CLASS       ),
        DB_FindAttr( DCM_OtherStudyNumbers,             STUDY_LEVEL,    OPTIONAL_KEY,   OTHER_CLASS     ),
        DB_FindAttr( DCM_AdmittingDiagnosesDescription, STUDY_LEVEL,    OPTIONAL_KEY,   STRING_CLASS    ),
        DB_FindAttr( DCM_PatientsAge,                   STUDY_LEVEL,    OPTIONAL_KEY,   STRING_CLASS    ),
        DB_FindAttr( DCM_PatientsSize,                  STUDY_LEVEL,    OPTIONAL_KEY,   OTHER_CLASS     ),
        DB_FindAttr( DCM_PatientsWeight,                        STUDY_LEVEL,    OPTIONAL_KEY,   OTHER_CLASS     ),
        DB_FindAttr( DCM_Occupation,                    STUDY_LEVEL,    OPTIONAL_KEY,   STRING_CLASS    ),
        DB_FindAttr( DCM_AdditionalPatientHistory,              STUDY_LEVEL,    OPTIONAL_KEY,   STRING_CLASS    ),
        DB_FindAttr( DCM_NumberOfStudyRelatedSeries,    STUDY_LEVEL,    OPTIONAL_KEY,   OTHER_CLASS     ),
        DB_FindAttr( DCM_NumberOfStudyRelatedInstances, STUDY_LEVEL,    OPTIONAL_KEY,   OTHER_CLASS     ),
        DB_FindAttr( DCM_SeriesNumber,                  SERIE_LEVEL,    REQUIRED_KEY,   OTHER_CLASS     ),
        DB_FindAttr( DCM_SeriesInstanceUID,             SERIE_LEVEL,    UNIQUE_KEY,             UID_CLASS       ),
        DB_FindAttr( DCM_Modality,                              SERIE_LEVEL,    OPTIONAL_KEY,   STRING_CLASS    ),
        DB_FindAttr( DCM_InstanceNumber,                        IMAGE_LEVEL,    REQUIRED_KEY,   OTHER_CLASS     ),
        DB_FindAttr( DCM_SOPInstanceUID,                        IMAGE_LEVEL,    UNIQUE_KEY,             UID_CLASS       )
  };

/**** The NbFindAttr variable contains the length of the TbFindAttr table
 ***/

static int NbFindAttr = ((sizeof (TbFindAttr)) / (sizeof (TbFindAttr [0])));


int DB_debugLevel = 0;

void DB_setDebugLevel(int debugLevel)
{
    DB_debugLevel = debugLevel;
}

int DB_getDebugLevel()
{
    return DB_debugLevel;
}

void DB_debug(int level, const char* format, ...)
{
    va_list ap;
    char buf[4096]; /* we hope a message never gets larger */

    if (level <= DB_debugLevel) {
        CERR << "DB:";
        va_start(ap, format);
        vsprintf(buf, format, ap);
        va_end(ap);
        CERR << buf << endl;
    }
}

void DB_printDataset(DcmDataset *ds)
{
    ds->print(COUT);
}

static OFBool doCheckFindIdentifier = OFFalse;
static OFBool doCheckMoveIdentifier = OFFalse;

void DB_setIdentifierChecking(OFBool checkFind, OFBool checkMove)
{
    doCheckFindIdentifier = checkFind;
    doCheckMoveIdentifier = checkMove;
}

OFBool DB_doCheckFindIdentifier()
{
    return doCheckFindIdentifier;
}

OFBool DB_doCheckMoveIdentifier()
{
    return doCheckMoveIdentifier;
}


OFCondition
DB_lock(DB_Private_Handle *phandle, OFBool exclusive)
{
    int lockmode;

    if (exclusive) {
        lockmode = LOCK_EX;     /* exclusive lock */
    } else {
        lockmode = LOCK_SH;     /* shared lock */
    }
    if (dcmtk_flock(phandle->pidx, lockmode) < 0) {
        dcmtk_plockerr("DB_lock");
        return IMAGECTN_DB_ERROR;
    }
    return EC_Normal;
}

OFCondition
DB_unlock(DB_Private_Handle *phandle)
{
    if (dcmtk_flock(phandle->pidx, LOCK_UN) < 0) {
        dcmtk_plockerr("DB_unlock");
        return IMAGECTN_DB_ERROR;
    }
    return EC_Normal;
}



/***********************
 *      Creates a handle
 */

OFCondition
DB_createHandle(
                const char      *storageArea,
                long    maxStudiesPerStorageArea,
                long    maxBytesPerStudy,
                DB_Handle       **handle)
{
    DB_Private_Handle *phandle;

    phandle = (DB_Private_Handle *) malloc ( sizeof(DB_Private_Handle) );

#ifdef DEBUG
    DB_debug(1, "DB_createHandle () : Handle created for %s\n",storageArea);
    DB_debug(1, "                     maxStudiesPerStorageArea: %ld maxBytesPerStudy: %ld\n",
            maxStudiesPerStorageArea, maxBytesPerStudy);
#endif

    if (maxStudiesPerStorageArea > DB_UpperMaxStudies) {
        CERR << "WARING: maxStudiesPerStorageArea too large" << endl
             << "        setting to " << DB_UpperMaxStudies << endl;
        maxStudiesPerStorageArea = DB_UpperMaxStudies;
    }
    if (maxStudiesPerStorageArea < 0) {
        maxStudiesPerStorageArea = DB_UpperMaxStudies;
    }
    if (maxBytesPerStudy < 0 || maxBytesPerStudy > DB_UpperMaxBytesPerStudy) {
        maxBytesPerStudy = DB_UpperMaxBytesPerStudy;
    }

    if (phandle) {
        sprintf (phandle -> storageArea,"%s", storageArea);
        sprintf (phandle -> indexFilename,"%s%c%s", storageArea, PATH_SEPARATOR, DBINDEXFILE);

        /* create index file if it does not already exist */
        FILE* f = fopen(phandle->indexFilename, "ab");
        if (f == NULL) {
            CERR << phandle->indexFilename << ": " << strerror(errno) << endl;
            return IMAGECTN_DB_ERROR;
        }
        fclose(f);

        /* open fd of index file */
#ifdef O_BINARY
        phandle -> pidx = open(phandle -> indexFilename, O_RDWR | O_BINARY );
#else
        phandle -> pidx = open(phandle -> indexFilename, O_RDWR );
#endif
        if ( phandle -> pidx == (-1) )
            return ( IMAGECTN_DB_ERROR );
        else {
            phandle -> idxCounter = -1;
            phandle -> findRequestList = NULL;
            phandle -> findResponseList = NULL;
            phandle -> maxBytesPerStudy = maxBytesPerStudy;
            phandle -> maxStudiesAllowed = maxStudiesPerStorageArea;
            phandle -> uidList = NULL;
            *handle = (DB_Handle *)phandle;
            return ( EC_Normal );
        }
    }
    else

        return ( IMAGECTN_DB_ERROR );

}

/***********************
 *      Destroys a handle
 */

OFCondition
DB_destroyHandle(DB_Handle **handle)
{
    DB_Private_Handle *phandle;
    int closeresult;

    phandle = (DB_Private_Handle *) (*handle);
    if (phandle)
    {
#ifndef _WIN32
      /* should not be necessary because we are closing the file handle anyway.
       * On Unix systems this does no harm, but on Windows the unlock fails
       * if the file was not locked before
       * and this gives an unnecessary error message on stderr.
       */
      DB_unlock(phandle);
#endif
      closeresult = close( phandle -> pidx);

      /* Free lists */
      DB_FreeElementList (phandle -> findRequestList);
      DB_FreeElementList (phandle -> findResponseList);
      DB_FreeUidList (phandle -> uidList);

      free ( (char *)(phandle) );
      if ( closeresult )
        return ( IMAGECTN_DB_ERROR );
      else
        return ( EC_Normal );
    } else {
      return ( IMAGECTN_DB_ERROR );
    }
}

/**********************************
 *      Provides a storage filename
 */

OFCondition
DB_makeNewStoreFileName(
                DB_Handle       *handle,
                const char      *SOPClassUID,
                const char      * /* SOPInstanceUID */ ,
                char            *newImageFileName)
{

    static OFFilenameCreator fnamecreator;

    OFString filename;
    char prefix[12];

    DB_Private_Handle *phandle = (DB_Private_Handle *) handle;
    const char *m = dcmSOPClassUIDToModality(SOPClassUID);
    if (m==NULL) m = "XX";
    sprintf(prefix, "%s_", m);
    // unsigned int seed = fnamecreator.hashString(SOPInstanceUID);
    unsigned int seed = (unsigned int)time(NULL);
    newImageFileName[0]=0; // return empty string in case of error
    if (! fnamecreator.makeFilename(seed, phandle->storageArea, prefix, ".dcm", filename)) return IMAGECTN_DB_ERROR;

    strcpy(newImageFileName, filename.c_str());
    return EC_Normal;
}


/*******************
 *    Free an element List
 */

OFCondition
DB_FreeUidList (DB_UidList *lst)
{
    if (lst == NULL) return EC_Normal;

    OFCondition cond = DB_FreeUidList (lst -> next);
    if (lst -> patient)
    free (lst -> patient);
    if (lst -> study)
    free (lst -> study);
    if (lst -> serie)
    free (lst -> serie);
    if (lst -> image)
    free (lst -> image);
    free (lst);
    return (cond);
}


/*******************
 *    Free a UID List
 */

OFCondition
DB_FreeElementList (DB_ElementList *lst)
{
    if (lst == NULL) return EC_Normal;

    OFCondition cond = DB_FreeElementList (lst -> next);
    if (lst->elem.PValueField != NULL) {
    free ((char *) lst -> elem. PValueField);
    }
    free (lst);
    return (cond);
}


/*******************
 *    Matches two strings
 */

int
DB_StringUnify  (char *pmod, char *pstr)
{
    int uni;


    if (*pmod == '\0')
    return (*pstr == '\0');

    if (  *pmod == *pstr
      || (*pmod == '?' && *pstr != '\0')
    )
    return (DB_StringUnify (pmod + 1, pstr + 1));

    if (*pmod == '*') {
    if ( *(pmod + 1) == '\0' )
        return (OFTrue);
    while (  ( (uni = DB_StringUnify (pmod + 1, pstr)) == OFFalse )
         && (*pstr != '\0')
        )
        pstr++;
    return (uni);
    }
    else if (*pmod != *pstr)
    return (OFFalse);
    return OFFalse;
}

/*******************
 *    Is the specified tag supported
 */

int
DB_TagSupported (DcmTagKey tag)
{
    int i;

    for (i = 0; i < NbFindAttr; i++)
    if (TbFindAttr[i]. tag == tag)
        return (OFTrue);

    return (OFFalse);

}


/*******************
 *    Get UID tag of a specified level
 */

OFCondition
DB_GetUIDTag (DB_LEVEL level, DcmTagKey *tag)
{
    int i;

    for (i = 0; i < NbFindAttr; i++)
    if ((TbFindAttr[i]. level == level) && (TbFindAttr[i]. keyAttr == UNIQUE_KEY))
        break;

    if (i < NbFindAttr) {
    *tag = TbFindAttr[i].tag;
    return (EC_Normal);
    }
    else
    return (IMAGECTN_DB_ERROR);

}

/*******************
 *    Get tag level of a specified tag
 */

OFCondition
DB_GetTagLevel (DcmTagKey tag, DB_LEVEL *level)
{
    int i;

    for (i = 0; i < NbFindAttr; i++)
    if (TbFindAttr[i]. tag == tag)
        break;

    if (i < NbFindAttr) {
    *level = TbFindAttr[i]. level;
    return (EC_Normal);
    }
    else
    return (IMAGECTN_DB_ERROR);
}

/*******************
 *    Get tag key attribute of a specified tag
 */

OFCondition
DB_GetTagKeyAttr (DcmTagKey tag, DB_KEY_TYPE *keyAttr)
{
    int i;

    for (i = 0; i < NbFindAttr; i++)
    if (TbFindAttr[i]. tag == tag)
        break;

    if (i < NbFindAttr) {
    *keyAttr = TbFindAttr[i]. keyAttr;
    return (EC_Normal);
    }
    else
    return (IMAGECTN_DB_ERROR);
}

/*******************
 *    Get tag key attribute of a specified tag
 */

OFCondition
DB_GetTagKeyClass (DcmTagKey tag, DB_KEY_CLASS *keyAttr)
{
    int i;

    for (i = 0; i < NbFindAttr; i++)
    if (TbFindAttr[i]. tag == tag)
        break;

    if (i < NbFindAttr) {
    *keyAttr = TbFindAttr[i]. keyClass;
    return (EC_Normal);
    }
    else
    return (IMAGECTN_DB_ERROR);
}


/*******************
 *    Remove spaces in a string
 */

void
DB_RemoveSpaces (char *string)
{
    char *pc1, *pc2;

    for (pc1 = pc2 = string; *pc2; pc2++) {
    if (*pc2 != ' ') {
        *pc1 = *pc2;
        pc1++;
    }
    }
    *pc1 = '\0';
}

/*******************
 *    Remove leading and trailing spaces in a string
 */

void
DB_RemoveEnclosingSpaces (char *string)
{
    char *pc1, *pc2;

    /** Find in pc2 the first non space character
    ** If not found, string is empty
    */

    for (pc2 = string; (*pc2 != '\0') && (*pc2 == ' '); pc2++);
    if (*pc2 == '\0') {
    string [0] = '\0';
    return;
    }

    /** Shift the string if necessary
     */

    if (pc2 != string) {
    for (pc1 = string; *pc2; pc1++, pc2++)
        *pc1 = *pc2;
    *pc1 = '\0';
    }

    /** Ship trailing spaces
     */

    for (pc2 = string + strlen (string) - 1; *pc2 == ' '; pc2--);
    pc2++;
    *pc2 = '\0';
}


/*******************
 *    Convert a date YYYYMMDD in a long
 */

long
DB_DateToLong (char *date)
{
    char year [5];
    char month[3];
    char day  [3];

    strncpy (year, date, 4);
    year [4] = '\0';
    strncpy (month, date + 4, 2);
    month [2] = '\0';
    strncpy (day, date + 6, 2);
    day [2] = '\0';

    return ((atol(year) * 10000) + (atol(month) * 100) + atol(day));
}


/*******************
 *    Convert a time in a double
 */

double
DB_TimeToDouble (char *thetime)
{
    char t [20];
    char tmp [4];

    double result = 0.;
    char *pc;

    /*** Get fractionnal part if exists
    **/

    strcpy (t, thetime);
    if ((pc = strchr (t, '.')) != NULL) {
    double f;

    *pc = '\0';
    for (pc++, f = 1.; (*pc) && (isdigit (*pc)); pc++) {
        f /= 10.;
        result += (*pc - '0') * f;
    }
    }

    /*** Add default values (mm ss) if necessary
    **/

    strcat (t, "0000");
    t [6] = '\0';

    /*** Get Hours, Minutes and Seconds
    **/

    strncpy (tmp, t, 2);
    tmp [3] = '\0';
    result += 3600. * OFStandard::atof(tmp);

    strncpy (tmp, t + 2, 2);
    tmp [3] = '\0';
    result += 60. * OFStandard::atof(tmp);

    strncpy (tmp, t + 4, 2);
    tmp [3] = '\0';
    result += OFStandard::atof(tmp);

    return result;
}



/***********************
 *    Duplicate a dicom element
 *    dst space is supposed provided by the caller
 */

void
DB_DuplicateElement (DB_SmallDcmElmt *src, DB_SmallDcmElmt *dst)
{
    bzero( (char*)dst, sizeof (DB_SmallDcmElmt));
    dst -> XTag = src -> XTag;
    dst -> ValueLength = src -> ValueLength;

    if (src -> ValueLength == 0)
    dst -> PValueField = NULL;
    else {
    dst -> PValueField = (char *)malloc ((int) src -> ValueLength+1);
    bzero(dst->PValueField, (size_t)(src->ValueLength+1));
    if (dst->PValueField != NULL) {
        memcpy (dst -> PValueField,  src -> PValueField,
            (size_t) src -> ValueLength);
    } else {
        CERR << "DB_DuplicateElement: out of memory" << endl;
    }
    }
}


/***********************
 *    Compare two ImagesofStudyArray elements
 */

int DB_Compare(const void *ve1, const void *ve2)
{
    ImagesofStudyArray *e1 = (ImagesofStudyArray *)ve1;
    ImagesofStudyArray *e2 = (ImagesofStudyArray *)ve2;
    if ( e1 -> RecordedDate > e2 -> RecordedDate )
    return (1);
    else
    if ( e1 -> RecordedDate == e2 -> RecordedDate )
        return (0);
    else
        return (-1);

}


/***********************
 *    Default constructors for struct IdxRecord and DB_SSmallDcmElmt
 */

IdxRecord::IdxRecord()
: RecordedDate(0.0)
, ImageSize(0)
, hstat(DVIF_objectIsNotNew)
{
}

DB_SSmallDcmElmt::DB_SSmallDcmElmt()
: PValueField(NULL)
, ValueLength(0)
, XTag()
{
}

/*
** CVS Log
** $Log: dbutils.cc,v $
** Revision 1.1  2005/08/23 19:32:09  braindead
** - initial savannah import
**
** Revision 1.1  2005/06/26 19:26:14  pipelka
** - added dcmtk
**
** Revision 1.38  2004/02/04 15:36:57  joergr
** Removed acknowledgements with e-mail addresses from CVS log.
**
** Revision 1.37  2003/06/03 09:34:20  meichel
** Renamed local variables to avoid name clashes with STL
**
** Revision 1.36  2002/11/27 13:27:55  meichel
** Adapted module imagectn to use of new header file ofstdinc.h
**
** Revision 1.35  2002/06/20 12:08:26  meichel
** Changed toolkit to use OFStandard::atof instead of atof, strtod or
**   sscanf for all string to double conversions that are supposed to
**   be locale independent
**
** Revision 1.34  2001/10/12 12:43:11  meichel
** Adapted imagectn to OFCondition based dcmnet module (supports strict mode).
**
** Revision 1.33  2001/06/01 15:51:29  meichel
** Updated copyright header
**
** Revision 1.32  2000/12/15 13:25:14  meichel
** Declared qsort() and signal() callback functions as extern "C", avoids
**   warnings on Sun C++ 5.x compiler.
**
** Revision 1.31  2000/11/10 16:25:49  meichel
** Fixed problem with imagectn running out of available file names after
**   receipt of 1024 files with similar SOP instance UIDs.
**
** Revision 1.30  2000/04/14 16:38:30  meichel
** Removed default value from output stream passed to print() method.
**   Required for use in multi-thread environments.
**
** Revision 1.29  2000/03/08 16:41:10  meichel
** Updated copyright header.
**
** Revision 1.28  2000/03/03 14:16:40  meichel
** Implemented library support for redirecting error messages into memory
**   instead of printing them to stdout/stderr for GUI applications.
**
** Revision 1.27  2000/02/23 15:13:34  meichel
** Corrected macro for Borland C++ Builder 4 workaround.
**
** Revision 1.26  2000/02/03 11:50:32  meichel
** Moved UID related functions from dcmnet (diutil.h) to dcmdata (dcuid.h)
**   where they belong. Renamed access functions to dcmSOPClassUIDToModality
**   and dcmGuessModalityBytes.
**
** Revision 1.25  2000/02/01 11:43:46  meichel
** Avoiding to include <stdlib.h> as extern "C" on Borland C++ Builder 4,
**   workaround for bug in compiler header files.
**
** Revision 1.24  1999/11/12 16:51:16  meichel
** Corrected file locking code that did not work correctly under Win95/98.
**
** Revision 1.23  1999/08/31 09:50:12  meichel
** Introduced default constructors for some imagectn structs
**   in order to passify some compiler warnings.
**
** Revision 1.22  1999/07/14 12:03:42  meichel
** Updated data dictionary for supplement 29, 39, 33_lb, CP packet 4 and 5.
**   Corrected dcmtk applications for changes in attribute name constants.
**
** Revision 1.21  1999/06/10 12:12:19  meichel
** Adapted imagectn to new command line option scheme.
**   Added support for Patient/Study Only Q/R model and C-GET (experimental).
**
** Revision 1.20  1999/04/30 16:37:17  meichel
** Renamed all flock calls to dcmtk_flock to avoid name clash between flock()
** emulation based on fcntl() and a constructor for struct flock.
**
** Revision 1.19  1999/03/22 09:56:34  meichel
** Reworked data dictionary based on the 1998 DICOM edition and the latest
**   supplement versions. Corrected dcmtk applications for minor changes
**   in attribute name constants.
**
** Revision 1.18  1999/03/03 14:02:09  meichel
** Changed imagectn database to use new code to create filenames instead of
**   tempnam() which seems to be unreliable on Windows.
**
** Revision 1.17  1999/01/20 19:09:12  meichel
** Removed unneccessary unlock from DB_destroyHandle
**   if compiling on Windows. Avoids unneccesary error message.
**
** Revision 1.16  1998/08/10 08:56:50  meichel
** renamed member variable in DIMSE structures from "Status" to "DimseStatus".
**
** Revision 1.15  1998/01/27 10:49:27  meichel
** Minor bug corrections (string too short, incorrect return value).
**
** Revision 1.14  1997/10/06 13:48:03  hewett
** Minor correction to imagectn's index file code to use the changed
** attribute names from dcdeftag.h
**
** Revision 1.13  1997/09/18 08:11:05  meichel
** Many minor type conflicts (e.g. long passed as int) solved.
**
** Revision 1.12  1997/08/06 12:20:20  andreas
** - Using Windows NT with Visual C++ 4.x the standard open mode for files
**   is TEXT with conversions. For binary files (image files, imagectn database
**   index) this must be changed (e.g. fopen(filename, "...b"); or
**   open(filename, ..... |O_BINARY);)
**
** Revision 1.11  1997/08/05 07:40:46  andreas
** Change definition of path to database index now using consistently
** the defines PATH_SEPARATOR and DBINDEXFILE
**
** Revision 1.10  1997/07/21 08:59:59  andreas
** - Replace all boolean types (BOOLEAN, CTNBOOLEAN, DICOM_BOOL, BOOL)
**   with one unique boolean type OFBool.
**
** Revision 1.9  1997/06/26 12:52:02  andreas
** - Changed names for enumeration values in DB_KEY_TYPE since the value
**   OPTIONAL was predefined for Windows 95/NT
**
** Revision 1.8  1996/09/27 08:46:51  hewett
** Enclosed system include files with BEGIN_EXTERN_C/END_EXTERN_C.
**
** Revision 1.7  1996/06/10 13:44:01  meichel
** Corrected error producing core dump with incorrect configrc file:
** imagectn tried to unlock an unsuccessfully allocated lockfile.
**
** Revision 1.6  1996/05/30 17:45:18  hewett
** Modified the definition of a static array of structs which was causing
** some C++ compilers problems.
**
** Revision 1.5  1996/04/29 15:16:06  hewett
** Removed unused DB_GetUSValue().
**
** Revision 1.4  1996/04/27 12:55:18  hewett
** Removed cause of warnings when compiled with "c++ -O -g -Wall" under
** Solaris 2.4.  Mostly due to uninitialized variables.
**
** Revision 1.3  1996/04/25 16:35:14  hewett
** Added char* parameter casts for bzero() calls.
**
** Revision 1.2  1996/04/22 11:23:44  hewett
** Added a function to print a DcmDataset (useful for debugging).
**
** Revision 1.1.1.1  1996/03/28 19:25:00  hewett
** Oldenburg Image CTN Software ported to use the dcmdata C++ toolkit.
**
*/
