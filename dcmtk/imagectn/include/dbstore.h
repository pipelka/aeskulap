/*
 *
 *  Copyright (C) 1998-2003, OFFIS
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
 *  Module: imagectn
 *
 *  Author: Lutz Vorwerk
 *
 *  Purpose:
 *    enables access to functions of dbstore.cc
 *
 *  Last Update:      $Author: braindead $
 *  Update Date:      $Date: 2005/08/23 19:32:07 $
 *  CVS/RCS Revision: $Revision: 1.1 $
 *  Status:           $State: Exp $
 *
 *  CVS/RCS Log at end of file
 *
 */


#ifndef DBSTORE_H
#define DBSTORE_H


#include "osconfig.h"    /* make sure OS specific configuration is included first */

#define INCLUDE_CSTDLIB
#define INCLUDE_CSTDIO
#define INCLUDE_CERRNO
#define INCLUDE_CTIME
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


/** Image file deleting
*/

OFCondition DB_deleteImageFile(char* imgFile);

/** Delete oldest study in database
 */

extern int
DB_DeleteOldestStudy(DB_Private_Handle *phandle, StudyDescRecord *pStudyDesc);

/**  Delete oldest images in database
 */

OFCondition
DB_DeleteOldestImages(DB_Private_Handle *phandle, StudyDescRecord *pStudyDesc, int StudyNum, char *StudyUID, long RequiredSize);

/**  Verify if study UID already exists
 */

extern int
DB_MatchStudyUIDInStudyDesc (StudyDescRecord *pStudyDesc, char *StudyUID, int maxStudiesAllowed);

/** Check up storage rights in Study Desk record
 */

OFCondition
DB_CheckupinStudyDesc(DB_Private_Handle *phandle, StudyDescRecord *pStudyDesc, char *StudyUID, long imageSize);


/** If the image is already stored remove it from the database.

 */

OFCondition DB_removeDuplicateImage(DB_Private_Handle *phandle,
    const char *SOPInstanceUID, const char *StudyInstanceUID,
    StudyDescRecord *pStudyDesc, const char *newImageFileName);


/** Add data from imageFileName to database
 */

OFCondition
DB_storeRequest (
    DB_Handle   *handle,
    const char  *SOPClassUID,
    const char  * /*SOPInstanceUID*/,
    const char  *imageFileName,
    DB_Status   *status,
    OFBool      isNew);


/** Prune invalid DB records.
*/

OFCondition DB_pruneInvalidRecords(DB_Handle *dbHandle);
#endif

/*
 *  $Log: dbstore.h,v $
 *  Revision 1.1  2005/08/23 19:32:07  braindead
 *  - initial savannah import
 *
 *  Revision 1.1  2005/06/26 19:26:04  pipelka
 *  - added dcmtk
 *
 *  Revision 1.10  2003/12/05 10:40:35  joergr
 *  Removed leading underscore characters from preprocessor symbols (reserved
 *  symbols). Updated copyright date where appropriate.
 *
 *  Revision 1.9  2002/11/27 13:27:52  meichel
 *  Adapted module imagectn to use of new header file ofstdinc.h
 *
 *  Revision 1.8  2001/10/12 12:43:07  meichel
 *  Adapted imagectn to OFCondition based dcmnet module (supports strict mode).
 *
 *  Revision 1.7  2001/06/01 15:51:26  meichel
 *  Updated copyright header
 *
 *  Revision 1.6  2000/11/23 17:02:14  joergr
 *  Removed default value for parameter to avoid compiler warnings (MSVC5).
 *
 *  Revision 1.5  2000/11/23 16:40:52  joergr
 *  Added new command line option to dbregimg allowing to specify whether
 *  instance reviewed status of newly registered objects should be set to 'new'
 *  or 'not new'.
 *
 *  Revision 1.4  2000/03/08 16:41:07  meichel
 *  Updated copyright header.
 *
 *  Revision 1.3  2000/02/23 15:13:23  meichel
 *  Corrected macro for Borland C++ Builder 4 workaround.
 *
 *  Revision 1.2  2000/02/01 11:43:44  meichel
 *  Avoiding to include <stdlib.h> as extern "C" on Borland C++ Builder 4,
 *    workaround for bug in compiler header files.
 *
 *  Revision 1.1  1998/12/22 15:25:34  vorwerk
 *  added for access of functions from imagectn/libsrc/dbstore.cc
 *
 *
 */
