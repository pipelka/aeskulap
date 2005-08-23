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
 *  Purpose: Service Class Executive (SCE) - Store Service Class Provider
 *
 *  Last Update:      $Author: braindead $
 *  Update Date:      $Date: 2005/08/23 19:32:03 $
 *  Source File:      $Source: /cvsroot/aeskulap/aeskulap/dcmtk/imagectn/apps/scestore.cc,v $
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

BEGIN_EXTERN_C
#ifdef HAVE_SYS_FILE_H
#include <sys/file.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_IO_H
#include <io.h>
#endif
END_EXTERN_C

#include "dicom.h"
#include "imagectn.h"
#include "dimse.h"
#include "dcompat.h"
#include "diutil.h"
#include "sce.h"
#include "imagedb.h"
#include "cnf.h"
#include "dcfilefo.h"

#include "scestore.h"

/*
 * C-STORE SCP
 */

typedef struct {
    DB_Handle *dbHandle;
    DIC_US status;
    DcmDataset *statusDetail;
    char *fileName;
    DcmFileFormat *dcmff;
    OFBool opt_correctUIDPadding;
} SCE_StoreContext;


static void
updateDisplay(T_DIMSE_StoreProgress * progress)
{
  if (opt_verbose)
  {
    switch (progress->state)
    {
      case DIMSE_StoreBegin:
        printf("RECV:");
        break;
      case DIMSE_StoreEnd:
        printf("\n");
        break;
      default:
        putchar('.');
        break;
    }
    fflush(stdout);
  }
}


static void 
saveImageToDB(
    SCE_StoreContext *context,
    T_DIMSE_C_StoreRQ *req,             /* original store request */
    char *imageFileName,
    /* out */
    T_DIMSE_C_StoreRSP *rsp,            /* final store response */
    DcmDataset **statusDetail)
{
    OFCondition dbcond = EC_Normal;
    DB_Status dbStatus;

    dbStatus.status = STATUS_Success;
    dbStatus.statusDetail = NULL;
    
    /* Store image */
    if (opt_ignoreStoreData) {
        rsp->DimseStatus = STATUS_Success;
        *statusDetail = NULL;
        return; /* nothing else to do */
    }
    
    if (context->status == STATUS_Success) {

        if (dbStatus.status == STATUS_Success) {
    
            dbcond = DB_storeRequest(context->dbHandle,
                req->AffectedSOPClassUID, req->AffectedSOPInstanceUID,
                imageFileName, &dbStatus);
            if (dbcond.bad()) {
                errmsg("storeSCP: Database: DB_storeRequest Failed (%s)",
                   DU_cstoreStatusString(dbStatus.status));
                DimseCondition::dump(dbcond);
            }
        }
    } else {
        dbStatus.status = context->status;
        dbStatus.statusDetail = context->statusDetail;
    }
    rsp->DimseStatus = dbStatus.status;
    *statusDetail = dbStatus.statusDetail;
    context->status = dbStatus.status;
    context->statusDetail = dbStatus.statusDetail;
}

static void
writeToFile(
    DcmFileFormat *ff,
    const char* fileName,
    T_DIMSE_C_StoreRSP *rsp)
{
    E_TransferSyntax xfer = opt_writeTransferSyntax;
    if (xfer == EXS_Unknown) xfer = ff->getDataset()->getOriginalXfer();

    OFCondition cond = ff->saveFile(fileName, xfer, opt_sequenceType, 
        opt_groupLength, opt_paddingType, (Uint32)opt_filepad, 
        (Uint32)opt_itempad, (!opt_useMetaheader));

    if (cond.bad())
    {
      fprintf(stderr, "storescp: Cannot write image file: %s\n", fileName);
      rsp->DimseStatus = STATUS_STORE_Refused_OutOfResources;
    }
}

static void
checkRequestAgainstDataset(
    T_DIMSE_C_StoreRQ *req,     /* original store request */
    const char* fname,          /* filename of dataset */
    DcmDataset *dataSet,        /* dataset to check */
    T_DIMSE_C_StoreRSP *rsp,    /* final store response */
    OFBool opt_correctUIDPadding)    
{
    DcmFileFormat ff;

    if (dataSet == NULL)
    {
      ff.loadFile(fname);
      dataSet = ff.getDataset();
    }

    /* which SOP class and SOP instance ? */
    DIC_UI sopClass;
    DIC_UI sopInstance;
    
    if (!DU_findSOPClassAndInstanceInDataSet(dataSet, sopClass, sopInstance, opt_correctUIDPadding)) 
    {
        errmsg("Bad image file: %s", fname);
        rsp->DimseStatus = STATUS_STORE_Error_CannotUnderstand;
    } else if (strcmp(sopClass, req->AffectedSOPClassUID) != 0) {
        rsp->DimseStatus = STATUS_STORE_Error_DataSetDoesNotMatchSOPClass;
    } else if (strcmp(sopInstance, req->AffectedSOPInstanceUID) != 0) {
        rsp->DimseStatus = STATUS_STORE_Error_DataSetDoesNotMatchSOPClass;
    }
}

static void
storeProgressCallback(
    /* in */
    void *callbackData, 
    T_DIMSE_StoreProgress *progress,    /* progress state */
    T_DIMSE_C_StoreRQ *req,             /* original store request */
    char *imageFileName,       /* being received into */ 
    DcmDataset **imageDataSet, /* being received into */
    /* out */
    T_DIMSE_C_StoreRSP *rsp,            /* final store response */
    DcmDataset **statusDetail)
{
    SCE_StoreContext *context;

    updateDisplay(progress);

    if (progress->state == DIMSE_StoreEnd) {
        context = (SCE_StoreContext*)callbackData;      /* recover context */

        if (!opt_ignoreStoreData && rsp->DimseStatus == STATUS_Success) {
            if ((imageDataSet)&&(*imageDataSet)) {
                checkRequestAgainstDataset(req, NULL, *imageDataSet, rsp, context->opt_correctUIDPadding);
            } else {
                checkRequestAgainstDataset(req, imageFileName, NULL, rsp, context->opt_correctUIDPadding);
            }
        }

        if (!opt_ignoreStoreData && rsp->DimseStatus == STATUS_Success) {
            if ((imageDataSet)&&(*imageDataSet)) {
                writeToFile(context->dcmff, context->fileName, rsp);
            }
            if (rsp->DimseStatus == STATUS_Success) {
                saveImageToDB(context, req, context->fileName, rsp, statusDetail);
            }
        }

        if (opt_verbose) {
            printf("Sending:\n");
            DIMSE_printCStoreRSP(stdout, rsp);
        } else if (rsp->DimseStatus != STATUS_Success) {
            fprintf(stdout, "NOTICE: StoreSCP:\n");
            DIMSE_printCStoreRSP(stdout, rsp);
        }
        context->status = rsp->DimseStatus;
    }
}

OFCondition
SCE_storeSCP(T_ASC_Association * assoc, T_DIMSE_C_StoreRQ * request,
             T_ASC_PresentationContextID presId,
             DB_Handle *dbHandle,
             OFBool opt_correctUIDPadding)
{
    OFCondition cond = EC_Normal;
    OFCondition dbcond = EC_Normal;
    char imageFileName[MAXPATHLEN+1];
    SCE_StoreContext context;
    DcmFileFormat dcmff;

    context.dbHandle = dbHandle;
    context.status = STATUS_Success;
    context.statusDetail = NULL;
    context.dcmff = &dcmff;
    context.opt_correctUIDPadding = opt_correctUIDPadding;

    if (opt_verbose) {
        printf("Received Store SCP: ");
        DIMSE_printCStoreRQ(stdout, request);
    }

    if (!dcmIsaStorageSOPClassUID(request->AffectedSOPClassUID)) {
        /* callback will send back sop class not supported status */ 
        context.status = STATUS_STORE_Refused_SOPClassNotSupported;
        /* must still receive data */
        strcpy(imageFileName, NULL_DEVICE_NAME);
    } else if (opt_ignoreStoreData) {
        strcpy(imageFileName, NULL_DEVICE_NAME);
    } else {
        dbcond = DB_makeNewStoreFileName(dbHandle,
            request->AffectedSOPClassUID,
            request->AffectedSOPInstanceUID,
            imageFileName);
        if (dbcond.bad()) {
            errmsg("storeSCP: Database: DB_makeNewStoreFileName Failed");
            /* must still receive data */
            strcpy(imageFileName, NULL_DEVICE_NAME); 
            /* callback will send back out of resources status */ 
            context.status = STATUS_STORE_Refused_OutOfResources;
        }
    }

#ifdef LOCK_IMAGE_FILES
    /* exclusively lock image file */
#ifdef O_BINARY
    int lockfd = open(imageFileName, (O_WRONLY | O_CREAT | O_TRUNC | O_BINARY), 0666);
#else
    int lockfd = open(imageFileName, (O_WRONLY | O_CREAT | O_TRUNC), 0666);
#endif
    dcmtk_flock(lockfd, LOCK_EX);
#endif

    context.fileName = imageFileName;

    DcmDataset *dset = dcmff.getDataset();

    /* we must still retrieve the data set even if some error has occured */

    if (opt_bitPreserving) { /* the bypass option can be set on the command line */
        cond = DIMSE_storeProvider(assoc, presId, request, imageFileName, (int)opt_useMetaheader,
                                   NULL, storeProgressCallback, 
                                   (void*)&context, DIMSE_BLOCKING, 0);
    } else {
        cond = DIMSE_storeProvider(assoc, presId, request, (char *)NULL, (int)opt_useMetaheader,
                                   &dset, storeProgressCallback, 
                                   (void*)&context, DIMSE_BLOCKING, 0);
    }


    if (cond.bad()) {
        errmsg("Store SCP Failed:");
        DimseCondition::dump(cond);
    }
    if (!opt_ignoreStoreData && (cond.bad() || (context.status != STATUS_Success))) 
    {
      /* remove file */
      if (strcmp(imageFileName, NULL_DEVICE_NAME) != 0) // don't try to delete /dev/null
      {
        if (opt_verbose) fprintf(stderr, "Store SCP: Deleting Image File: %s\n", imageFileName);
        unlink(imageFileName);      
      }
      DB_pruneInvalidRecords(dbHandle);
    }

#ifdef LOCK_IMAGE_FILES
    /* unlock image file */
    dcmtk_flock(lockfd, LOCK_UN);
    close(lockfd);
#endif
    
    return cond;
}

/*
** CVS Log
** $Log: scestore.cc,v $
** Revision 1.1  2005/08/23 19:32:03  braindead
** - initial savannah import
**
** Revision 1.1  2005/06/26 19:26:14  pipelka
** - added dcmtk
**
** Revision 1.25  2002/12/13 13:43:28  meichel
** Removed unused code reported by the MIPSpro 7.3 optimizer
**
** Revision 1.24  2002/11/27 13:27:01  meichel
** Adapted module imagectn to use of new header file ofstdinc.h
**
** Revision 1.23  2002/11/25 18:01:15  meichel
** Converted compile time option to leniently handle space padded UIDs
**   in the Storage Service Class into command line / config file option.
**
** Revision 1.22  2002/08/20 12:22:50  meichel
** Adapted code to new loadFile and saveFile methods, thus removing direct
**   use of the DICOM stream classes.
**
** Revision 1.21  2001/11/12 14:54:22  meichel
** Removed all ctndisp related code from imagectn
**
** Revision 1.20  2001/10/12 12:42:54  meichel
** Adapted imagectn to OFCondition based dcmnet module (supports strict mode).
**
** Revision 1.19  2001/06/01 15:51:21  meichel
** Updated copyright header
**
** Revision 1.18  2000/11/10 16:25:25  meichel
** Fixed problem with DIMSE routines which attempted to delete /dev/null
**   under certain circumstances, which could lead to disastrous results if
**   tools were run with root permissions (what they shouldn't).
**
** Revision 1.17  2000/03/08 16:41:01  meichel
** Updated copyright header.
**
** Revision 1.16  2000/03/02 12:54:05  joergr
** Fixed bug that caused imagectn to ignore the -F (write dataset) flag when
** +B (bit preserving) was set.
**
** Revision 1.15  2000/02/23 15:13:12  meichel
** Corrected macro for Borland C++ Builder 4 workaround.
**
** Revision 1.14  2000/02/01 11:43:41  meichel
** Avoiding to include <stdlib.h> as extern "C" on Borland C++ Builder 4,
**   workaround for bug in compiler header files.
**
** Revision 1.13  1999/06/10 12:12:01  meichel
** Adapted imagectn to new command line option scheme.
**   Added support for Patient/Study Only Q/R model and C-GET (experimental).
**
** Revision 1.12  1999/04/30 16:37:13  meichel
** Renamed all flock calls to dcmtk_flock to avoid name clash between flock()
** emulation based on fcntl() and a constructor for struct flock.
**
** Revision 1.11  1998/08/10 08:56:46  meichel
** renamed member variable in DIMSE structures from "Status" to "DimseStatus".
**
** Revision 1.10  1998/01/14 14:29:47  hewett
** Added basic support for the Structured Reporting (SR) SOP Classes.
**
** Revision 1.9  1997/08/26 14:17:20  hewett
** Added +B command line option to imagectn application.  Use of this option
** causes imagectn to bypass the dcmdata encode/decode routines when receiving
** images and write image data to disk exactly as received in a C-STORE
** command over the network.  This option does _not_ affect sending images.
**
** Revision 1.8  1997/08/06 12:20:17  andreas
** - Using Windows NT with Visual C++ 4.x the standard open mode for files
**   is TEXT with conversions. For binary files (image files, imagectn database
**   index) this must be changed (e.g. fopen(filename, "...b"); or
**   open(filename, ..... |O_BINARY);)
**
** Revision 1.7  1997/07/21 08:59:45  andreas
** - Replace all boolean types (BOOLEAN, CTNBOOLEAN, DICOM_BOOL, BOOL)
**   with one unique boolean type OFBool.
**
** Revision 1.6  1997/06/26 12:59:20  andreas
** - Include Additional headers (winsock.h, io.h) for Windows NT/95
**
** Revision 1.5  1997/05/23 10:47:08  meichel
** Major rewrite of storescp application. See CHANGES for details.
** Changes required to interfaces of some DIMSE functions.
**
** Revision 1.4  1997/05/16 08:31:41  andreas
** - Revised handling of GroupLength elements and support of
**   DataSetTrailingPadding elements. The enumeratio E_GrpLenEncoding
**   got additional enumeration values (for a description see dctypes.h).
**   addGroupLength and removeGroupLength methods are replaced by
**   computeGroupLengthAndPadding. To support Padding, the parameters of
**   element and sequence write functions changed.
**
** Revision 1.3  1996/09/27 08:46:22  hewett
** Enclosed system include files with BEGIN_EXTERN_C/END_EXTERN_C.
**
** Revision 1.2  1996/04/22 10:26:25  hewett
** Now calls DB_pruneInvalidRecords on error.
**
** Revision 1.1.1.1  1996/03/28 19:24:59  hewett
** Oldenburg Image CTN Software ported to use the dcmdata C++ toolkit.
**
**
*/
