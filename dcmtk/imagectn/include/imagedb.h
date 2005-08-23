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
 *  Author:  Andrew Hewett
 *
 *  Purpose: Public interface for Image Database Module. Module Prefix: DB_ 
 *
 *  Last Update:      $Author: braindead $
 *  Update Date:      $Date: 2005/08/23 19:32:07 $
 *  Source File:      $Source: /cvsroot/aeskulap/aeskulap/dcmtk/imagectn/include/imagedb.h,v $
 *  CVS/RCS Revision: $Revision: 1.1 $
 *  Status:           $State: Exp $
 *
 *  CVS/RCS Log at end of file
 *
 */

#ifndef IMGDB_H
#define IMGDB_H

#include "osconfig.h"    /* make sure OS specific configuration is included first */

#define INCLUDE_CSTDLIB
#define INCLUDE_CSTDIO
#include "ofstdinc.h"

BEGIN_EXTERN_C
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
END_EXTERN_C

#include "dicom.h"
#include "cond.h"
#include "dcdatset.h"
#include "dimse.h"


#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif


#define DBINDEXFILE "index.dat"

#ifndef _WIN32
/* we lock image files on all platforms except Win32 where it does not work
 * due to the different semantics of LockFile/LockFileEx compared to flock.
 */
#define LOCK_IMAGE_FILES
#endif


  /* This enum describes the status of one entry in the database hierarchy.
       An entry can describe a study, a series or an instance.
       A study or series is new exactly if all subobjects (series and instances) are new.
       A study or series contains new subobjecs as long as any subobject
       (series or instance) has the status objectIsNew.
       Instances can never have the status DVIF_objectContainsNewSubobjects.
     */

    enum DVIFhierarchyStatus
    {
      DVIF_objectIsNotNew,
      DVIF_objectIsNew,
      DVIF_objectContainsNewSubobjects
    };



/* Status Structure */

typedef struct {
    unsigned short status;
    DcmDataset *statusDetail;
} DB_Status;


/*
 * A Database Handle (Opaque Type)
 * The Database routines define this differently and its
 * structure is hidden from the caller.
 */
typedef struct {
    void	*privateData;
} DB_Handle;


/*
 * Public Function Prototypes
 */

void DB_enableQuotaSystem(OFBool enable);
/*
** Enabled/Disabled the DB quota system.  
** If the quota system is disabled then no images will be
** deleted due to quotas being exceeded.
**
** Default: quota system enabled.
*/

void DB_setDebugLevel(int debugLevel);
int DB_getDebugLevel();
/*
** set and get the DB module debug level.
*/

void DB_PrintIndexFile(char *storeArea);
/*
** Dump DB index file to stdout.
*/

void DB_setIdentifierChecking(OFBool checkFind, OFBool checkMove);
OFBool DB_doCheckFindIdentifier();
OFBool DB_doCheckMoveIdentifier();
/*
 * Globally set the DB module to perform (or not perform) checking
 * of FIND and MOVE request identifiers.
 * Default: don't do checking
 */

/* max 500 studies in the database */
#define DB_UpperMaxStudies		500
/* max 1 GByte per study */
#define DB_UpperMaxBytesPerStudy	0x40000000L

OFCondition DB_createHandle(const char *storageArea,
			long maxStudiesPerStorageArea,
			long maxBytesPerStudy,
			DB_Handle **handle);
/* 
 * Creates and initializes a DB_Handle handle for the given 
 * database storage area (storageArea) as a mechanism for keeping 
 * track of state between sub-operations.
 * In addition to the storage area directory, limitations on the
 * number of studies per storage area and the number of bytes per study
 * are passed over to control the use of the C-STORE operation.
 * The DB Module will use these limitations to enforce RSNA'93 Demonstration
 * Requirements (i.e. by deleting older studies if the limitation is
 * exceeded). 
 *
 * The routine should return EC_Normal upon normal completion, or
 * some other OFCondition code upon failure.
 */

OFCondition DB_destroyHandle(DB_Handle **handle);
/* 
 * Destroy a DB_Handle, cancel any ongoing
 * request if necessary, delete temporary files used for C-STORE and
 * sub-operations of C-MOVE.
 * 
 * The routine should return EC_Normal upon normal completion, or
 * some other OFCondition code upon failure.
 */


OFCondition DB_makeNewStoreFileName(DB_Handle *handle,
			const char *SOPClassUID,
			const char *SOPInstanceUID,
			char *newImageFileName);
/*
 * This routine, given a SOPClassUID and a SOPInstanceUID
 * (from a store request), should provide a filename (in newImageFileName)
 * where an incomming image can be stored.  
 * The Service Class Executive (SCE) will save the image data associated
 * with a C-STORE request in this file.
 * This file should be semi-permanent.  That is, it should only
 * be removed when the image is deleted to satisfy the RSNA 
 * Demonstration Requirements, or if the store operation failed.
 *
 * The file name provided here will subsequently be passed to
 * the DB_storeRequest routine for registration in the database.
 * The routine will be called by the SCE prior to
 * each DB_storeRequest.
 *
 * This behaviour is needed since the CTN Display process operates
 * asynchronously and requires access to stored image data so that it 
 * may be displayed.  The SCE will post a request to the CTN Display 
 * to display the first image stored in a study.  This file must then
 * exist when the CTN Display process tries to perform the display
 * action at some later date.
 *
 * Memory for newImageFileName will be provided by the caller and
 * should be at least MAXPATHLEN+1 characters.   
 * The file name generated should be an absolute file name.
 * 
 * The routine should return EC_Normal upon normal completion, or
 * some other OFCondition code upon failure.
 */

OFCondition DB_storeRequest(DB_Handle *handle,
			const char *SOPClassUID,
			const char *SOPInstanceUID,
			const char *imageFileName,
			DB_Status  *status,
			OFBool     isNew = OFTrue /* default: set instance reviewed status to DVIF_objectIsNew */);
/* 
 * Add data from imageFileName to database, store given SOP class UID,
 * SOP instance UID and read data from imageFileName to store in database
 *
 * Upon invoking this routine the image stored in imageFileName becomes
 * the responsibility of the DB module.
 * The status structure should contain SUCCESS or some failure status.
 * 
 * The routine should return EC_Normal upon normal completion, or
 * some other OFCondition code upon failure.
 */

OFCondition DB_startFindRequest(DB_Handle *handle,
			const char *SOPClassUID,
			DcmDataset *findRequestIdentifiers,
			DB_Status *status);	
/* 
 * Start FIND action using the given SOP class UID
 * and DICOM Object containing the find request identifiers.
 *
 * The caller retains responsibility for destroying the 
 * findRequestIdentifiers when no longer needed.
 * 
 * Status should contain a value of PENDING if any FIND responses
 * will be generated, or a value of SUCCESS if no FIND responses will
 * be generated (SUCCESS indicates the completion of a operation), or
 * other status codes upon failure.
 *
 * The routine should return EC_Normal upon normal completion, or
 * some other OFCondition code upon failure.
 */
		
OFCondition DB_nextFindResponse(DB_Handle *handle,
			DcmDataset **findResponseIdentifiers,
			DB_Status *status);
/* 
 * Create a DicomObject containing the next available FIND response
 * identifiers (result of previous FIND operation).
 *
 * The caller is responsible for destroying the findResponseIdentifiers
 * when no longer needed.
 *
 * Status should contain a value of PENDING if a FIND response
 * is available, or a value of SUCCESS on completion, or
 * other status codes upon failure.
 *
 * The routine should return EC_Normal upon normal completion, or
 * some other OFCondition code upon failure.
 */

OFCondition DB_cancelFindRequest(DB_Handle *handle,
			DB_Status *status);
/* 
 * Cancel ongoing FIND request, stop and reset every running operation
 * associated with this request, delete existing temporary files.
 *
 * The routine should return EC_Normal upon normal completion, or
 * some other OFCondition code upon failure.
 */
	
OFCondition DB_startMoveRequest(DB_Handle *handle,
			const char *SOPClassUID,
			DcmDataset *moveRequestIdentifiers,
			DB_Status *status);
/* 
 * Start MOVE action using the given SOP class UID
 * and DICOM Object containing the move request identifiers.
 *
 * The caller retains responsibility for destroying the 
 * moveRequestIdentifiers when no longer needed.
 * 
 * Status should contain a value of PENDING if any MOVE responses
 * will be generated, or a value of SUCCESS if no MOVE responses will
 * be generated (SUCCESS indicates the completion of a operation), or
 * other status codes upon failure.
 *
 * The routine should return EC_Normal upon normal completion, or
 * some other OFCondition code upon failure.
 * 
 */

OFCondition DB_nextMoveResponse(DB_Handle *handle,
			char *SOPClassUID,
			char *SOPInstanceUID,
			char *imageFileName,
			unsigned short *numberOfRemainingSubOperations,
			DB_Status *status);
/* 
 * Constructs the information required for the next available C-MOVE 
 * sub-operation (the image SOP class UID, SOP Instance UID and an
 * imageFileName containing the requested data).  
 *
 * The caller will pass sufficient storage for the string parameters.
 * 
 * The DB module is reponsible for creating and destroying any
 * temporary image files.
 *
 * On return, the numberOfRemainingSubOperations parameter will contain
 * the number of suboperations still remaining for the request
 * (this number is needed by move responses with PENDING status).
 *
 * Status should contain a value of PENDING if a MOVE response
 * is available, or a value of SUCCESS on completion, or
 * other status codes upon failure.
 *
 * The routine should return EC_Normal upon normal completion, or
 * some other OFCondition code upon failure.
 */

OFCondition DB_cancelMoveRequest(DB_Handle *handle,
			DB_Status *status);
/* 
 * Cancel ongoing MOVE request, stop and reset every running operation
 * associated with this request, delete existing temporary files
 *
 * The routine should return EC_Normal upon normal completion, or
 * some other OFCondition code upon failure.
 */

OFCondition DB_pruneInvalidRecords(DB_Handle *dbHandle);
/*
 * Prune invalid records from the database.
 * Records referring to non-existant image files are invalid.
 */

#endif

/*
** CVS Log
** $Log: imagedb.h,v $
** Revision 1.1  2005/08/23 19:32:07  braindead
** - initial savannah import
**
** Revision 1.1  2005/06/26 19:26:04  pipelka
** - added dcmtk
**
** Revision 1.17  2002/11/27 13:27:52  meichel
** Adapted module imagectn to use of new header file ofstdinc.h
**
** Revision 1.16  2001/11/12 14:54:31  meichel
** Removed all ctndisp related code from imagectn
**
** Revision 1.15  2001/10/12 12:43:07  meichel
** Adapted imagectn to OFCondition based dcmnet module (supports strict mode).
**
** Revision 1.14  2001/06/01 15:51:26  meichel
** Updated copyright header
**
** Revision 1.13  2000/11/23 16:40:52  joergr
** Added new command line option to dbregimg allowing to specify whether
** instance reviewed status of newly registered objects should be set to 'new'
** or 'not new'.
**
** Revision 1.12  2000/03/08 16:41:07  meichel
** Updated copyright header.
**
** Revision 1.11  2000/02/23 15:13:26  meichel
** Corrected macro for Borland C++ Builder 4 workaround.
**
** Revision 1.10  2000/02/01 11:43:44  meichel
** Avoiding to include <stdlib.h> as extern "C" on Borland C++ Builder 4,
**   workaround for bug in compiler header files.
**
** Revision 1.9  1999/06/10 12:12:13  meichel
** Adapted imagectn to new command line option scheme.
**   Added support for Patient/Study Only Q/R model and C-GET (experimental).
**
** Revision 1.8  1998/12/22 15:18:43  vorwerk
** remove enum DVIFhierarchyStatus from dcmpstat/libsrc/dviface.h and add it
** here
**
** Revision 1.7  1998/12/22 15:11:28  vorwerk
** removed from libsrc and added in include
**
** Revision 1.6  1997/08/05 07:40:32  andreas
** Change definition of path to database index now using consistently
** the defines PATH_SEPARATOR and DBINDEXFILE
**
** Revision 1.5  1997/07/21 08:59:53  andreas
** - Replace all boolean types (BOOLEAN, CTNBOOLEAN, DICOM_BOOL, BOOL)
**   with one unique boolean type OFBool.
**
** Revision 1.4  1996/09/27 08:46:36  hewett
** Enclosed system include files with BEGIN_EXTERN_C/END_EXTERN_C.
**
** Revision 1.3  1996/04/29 10:11:46  hewett
** Added function to disable DB quota system from deleting image files.
**
** Revision 1.2  1996/04/22 10:31:16  hewett
** Added function declaration for DB_pruneInvalidRecords().
**
** Revision 1.1.1.1  1996/03/28 19:24:59  hewett
** Oldenburg Image CTN Software ported to use the dcmdata C++ toolkit.
**
*/
