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
 *  Purpose: TI Network Routines
 *
 *  Last Update:      $Author: braindead $
 *  Update Date:      $Date: 2005/08/23 19:32:03 $
 *  Source File:      $Source: /cvsroot/aeskulap/aeskulap/dcmtk/imagectn/apps/tinet.cc,v $
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

#include "ti.h"
#include "imagedb.h" /* for LOCK_IMAGE_FILES */
#include "assoc.h"
#include "dimse.h"
#include "diutil.h"
#include "dcuid.h"
#include "dcfilefo.h"
#include "ofcmdln.h"
#include "cnf.h"
#include "tiui.h"
#include "tiquery.h"
#include "tinet.h"

/* abstract syntaxes for storage SOP classes are taken from dcmdata */
static const char *abstractSyntaxes[] = {
    UID_VerificationSOPClass,
    UID_FINDStudyRootQueryRetrieveInformationModel
};

OFBool
TI_detatchAssociation(TI_Config *conf, OFBool abortFlag)
{
    OFCondition cond = EC_Normal;
    DIC_NODENAME presentationAddress;
    DIC_AE peerTitle;

    if (conf->assoc == NULL) {
      return OFTrue;  /* nothing to do */
    }

    ASC_getPresentationAddresses(conf->assoc->params, NULL,
        presentationAddress);
    ASC_getAPTitles(conf->assoc->params, NULL, peerTitle, NULL);

    if (abortFlag) {
        /* abort association */
        if (verbose)
            printf("Aborting Association (%s)\n", peerTitle);
        cond = ASC_abortAssociation(conf->assoc);
        if (cond.bad()) {
            errmsg("Association Abort Failed:");
            DimseCondition::dump(cond);
        }
    } else {
        /* release association */
        if (verbose)
            printf("Releasing Association (%s)\n", peerTitle);
        cond = ASC_releaseAssociation(conf->assoc);
        if (cond.bad()) {
            errmsg("Association Release Failed:");
            DimseCondition::dump(cond);
        }
    }
    ASC_dropAssociation(conf->assoc);
    ASC_destroyAssociation(&conf->assoc);

    if (abortFlag) {
        printf("Aborted Association (%s,%s)\n",
              presentationAddress, peerTitle);
    } else {
        printf("Released Association (%s,%s)\n",
              presentationAddress, peerTitle);
    }

    return OFTrue;
}



static OFCondition
addPresentationContexts(T_ASC_Parameters *params)
{
    OFCondition cond = EC_Normal;
    int i;
    int pid = 1;

    /*
    ** We prefer to accept Explicitly encoded transfer syntaxes.
    ** If we are running on a Little Endian machine we prefer
    ** LittleEndianExplicitTransferSyntax to BigEndianTransferSyntax.
    ** Some SCP implementations will just select the first transfer
    ** syntax they support (this is not part of the standard) so
    ** organise the proposed transfer syntaxes to take advantage
    ** of such behaviour.
    */

    unsigned int numTransferSyntaxes = 0;
    const char* transferSyntaxes[] = { NULL, NULL, NULL };

    if (networkTransferSyntax == EXS_LittleEndianImplicit)
    {
        transferSyntaxes[0] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 1;
    }
    else
    {
        /* gLocalByteOrder is defined in dcxfer.h */
        if (gLocalByteOrder == EBO_LittleEndian)
        {
            /* we are on a little endian machine */
            transferSyntaxes[0] = UID_LittleEndianExplicitTransferSyntax;
            transferSyntaxes[1] = UID_BigEndianExplicitTransferSyntax;
            transferSyntaxes[2] = UID_LittleEndianImplicitTransferSyntax;
            numTransferSyntaxes = 3;
        } else {
            /* we are on a big endian machine */
            transferSyntaxes[0] = UID_BigEndianExplicitTransferSyntax;
            transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
            transferSyntaxes[2] = UID_LittleEndianImplicitTransferSyntax;
            numTransferSyntaxes = 3;
        }
    }

    /* first add presentation contexts for find and verification */
    for (i=0; i<(int)DIM_OF(abstractSyntaxes) && cond.good(); i++)
    {
        cond = ASC_addPresentationContext( params, pid, abstractSyntaxes[i], transferSyntaxes, numTransferSyntaxes);
        pid += 2; /* only odd presentation context id's */
    }

    /* and then for all storage SOP classes */
    for (i=0; i<numberOfDcmStorageSOPClassUIDs && cond.good(); i++)
    {
      cond = ASC_addPresentationContext( params, pid, dcmStorageSOPClassUIDs[i], transferSyntaxes, numTransferSyntaxes);
      pid += 2;/* only odd presentation context id's */
    }

    return cond;
}


OFBool
TI_attachAssociation(TI_Config *conf)
{
    OFCondition cond = EC_Normal;
    int port;
    const char *peer;
    DIC_NODENAME presentationAddress;
    T_ASC_Parameters *params;
    DIC_NODENAME localHost;
    DIC_AE myAETitle;

    if (conf->assoc != NULL) {
        TI_detatchAssociation(conf, OFFalse);
    }

    if (conf->dbEntries[conf->currentdb]->isRemoteDB) {
        strcpy(myAETitle, conf->myAETitle);
    } else {
        strcpy(myAETitle, conf->dbEntries[conf->currentdb]->title);
    }

    cond = ASC_createAssociationParameters(&params, conf->maxReceivePDULength);
    if (cond.bad()) {
        errmsg("Help, cannot create association parameters:");
        DimseCondition::dump(cond);
        return OFFalse;
    }
    ASC_setAPTitles(params, myAETitle, conf->currentPeerTitle, NULL);

    gethostname(localHost, sizeof(localHost) - 1);
    if (!CNF_peerForAETitle(conf->currentPeerTitle, &peer, &port)) {
        errmsg("Help, AE title (%s) no longer in config",
            conf->currentPeerTitle);
        ASC_destroyAssociationParameters(&params);
        return OFFalse;
    }
    sprintf(presentationAddress, "%s:%d", peer, port);
    ASC_setPresentationAddresses(params, localHost, presentationAddress);

    cond = addPresentationContexts(params);
    if (cond.bad()) {
        errmsg("Help, cannot add presentation contexts:");
        DimseCondition::dump(cond);
        ASC_destroyAssociationParameters(&params);

        return OFFalse;
    }
    if (debug) {
        printf("Request Parameters:\n");
        ASC_dumpParameters(params, COUT);
    }

    /* create association */
    if (verbose)
        printf("Requesting Association\n");
    cond = ASC_requestAssociation(conf->net, params, &conf->assoc);
    if (cond.bad()) {
        if (cond == DUL_ASSOCIATIONREJECTED) {
            T_ASC_RejectParameters rej;
      
            ASC_getRejectParameters(params, &rej);
            errmsg("Association Rejected:");
            ASC_printRejectParameters(stderr, &rej);
            fprintf(stderr, "\n");
            ASC_dropAssociation(conf->assoc);
            ASC_destroyAssociation(&conf->assoc);
      
            return OFFalse;
        } else {
            errmsg("Association Request Failed: Peer (%s, %s)",
                presentationAddress, conf->currentPeerTitle);
            DimseCondition::dump(cond);
            ASC_dropAssociation(conf->assoc);
            ASC_destroyAssociation(&conf->assoc);
      
            return OFFalse;
        }
    }
    /* what has been accepted/refused ? */
    if (debug) {
        printf("Association Parameters Negotiated:\n");
        ASC_dumpParameters(params, COUT);
    }

    if (ASC_countAcceptedPresentationContexts(params) == 0) {
        errmsg("All Presentation Contexts Refused: Peer (%s, %s)",
                presentationAddress, conf->currentPeerTitle);
        ASC_abortAssociation(conf->assoc);
        ASC_dropAssociation(conf->assoc);
        ASC_destroyAssociation(&conf->assoc);
      
        return OFFalse;
    }

    if (verbose) {
        printf("Association Accepted (Max Send PDV: %lu)\n",
          conf->assoc->sendPDVLength);
    }

    printf("New Association Started (%s,%s)\n", presentationAddress,
        conf->currentPeerTitle);

    return OFTrue;
}

/*
 * Change Association
 */

OFBool
TI_changeAssociation(TI_Config *conf)
{
    DIC_AE actualPeerAETitle;
    OFBool ok = OFTrue;

    if (conf->assoc != NULL) {
        /* do we really need to change the association */
        ASC_getAPTitles(conf->assoc->params, NULL, actualPeerAETitle, NULL);
        if (strcmp(actualPeerAETitle, conf->currentPeerTitle) == 0) {
            /* no need to change */
            return OFTrue;
        }
    }

    ok = TI_detatchAssociation(conf, OFFalse);
    if (!ok) return ok;

    ok = TI_attachAssociation(conf);
    return ok;
}

OFBool
TI_sendEcho(TI_Config *conf)
{
    OFCondition cond = EC_Normal;
    DIC_US msgId;
    DIC_US status;
    DcmDataset *statusDetail = NULL;

    msgId = conf->assoc->nextMsgID++;
    printf("[MsgID %d] Echo, ", msgId);
    fflush(stdout);

    cond = DIMSE_echoUser(conf->assoc, msgId, DIMSE_BLOCKING, 0,
      &status, &statusDetail);

    if (cond.good()) {
        printf("Complete [Status: %s]\n",
            DU_cstoreStatusString(status));
    } else {
        errmsg("Failed:");
        DimseCondition::dump(cond);
        ASC_abortAssociation(conf->assoc);
        ASC_dropAssociation(conf->assoc);
        ASC_destroyAssociation(&conf->assoc);

    }
    if (statusDetail != NULL) {
        printf("  Status Detail (should never be any):\n");
        statusDetail->print(COUT);
        delete statusDetail;
    }
    return (cond.good());
}


static void
storeProgressCallback(void * /*callbackData*/,
    T_DIMSE_StoreProgress *progress,
    T_DIMSE_C_StoreRQ * /*req*/)
{
    int percent;
    static int dotsSoFar = 0;
    int dotsToPrint;
    int i;

    switch (progress->state) {
    case DIMSE_StoreBegin:
        printf("  0%%________25%%_________50%%__________75%%________100%%\n");
        printf("  ");
        dotsSoFar = 0;
        break;
    case DIMSE_StoreEnd:
        printf("\n");
        break;
    default:
        if (progress->totalBytes == 0) {
            percent = 100;
        } else {
            percent = (int)(((float)progress->progressBytes /
                (float)progress->totalBytes) * 100.0);
        }
        dotsToPrint = (percent/2) - dotsSoFar;
        for (i=0; i<dotsToPrint; i++) {
            putchar('-');
            fflush(stdout);
            dotsSoFar++;
        }
        break;
    }
}


OFBool
TI_storeImage(TI_Config *conf,
    char *sopClass, char *sopInstance, char * imgFile)
{
    OFCondition cond = EC_Normal;
    DIC_US msgId;
    DcmDataset *statusDetail = NULL;
    T_ASC_PresentationContextID presId;
    T_DIMSE_C_StoreRQ req;
    T_DIMSE_C_StoreRSP rsp;
    DIC_PN patientsName;
    DIC_CS studyId;
    DIC_IS seriesNumber;
    DIC_CS modality;
    DIC_IS imageNumber;

    if (strlen(sopClass) == 0) {
        printf("WARNING: CTN has deleted image, giving up (no sopClass): %s\n",
         (imgFile)?(imgFile):("(nil)"));
        /* give up because if this image is gone, then others are also
         * very likely to have disappeared.  The user should restart
         * the operation when other activities have finished.
         */
        return OFFalse;
    }

#ifdef LOCK_IMAGE_FILES
     /* shared lock image file */
    int lockfd;
#ifdef O_BINARY
    lockfd = open(imgFile, O_RDONLY | O_BINARY, 0666);
#else
    lockfd = open(imgFile, O_RDONLY, 0666);
#endif
    if (lockfd < 0) {
        printf("WARNING: CTN has deleted image, giving up (no imgFile): %s\n",
         (imgFile)?(imgFile):("(nil)"));
        /* give up because if this image is gone, then others are also
         * very likely to have disappeared.  The user should restart
         * the operation when other activities have finished.
         */
        return OFFalse;
    }
    dcmtk_flock(lockfd, LOCK_SH);
#endif

    /* which presentation context should be used */
    presId = ASC_findAcceptedPresentationContextID(conf->assoc, sopClass);
    if (presId == 0) {
        errmsg("No presentation context for: (%s) %s",
            dcmSOPClassUIDToModality(sopClass), sopClass);
        return OFFalse;
    }

    TI_getInfoFromImage(imgFile, patientsName, studyId,
        seriesNumber, modality, imageNumber);

    /* start store */
    msgId = conf->assoc->nextMsgID++;
    printf("[MsgID %d] Store,\n", msgId);
    printf("  PatientsName: %s, StudyID: %s,\n",
        patientsName, studyId);
    printf("  Series: %s, Modality: %s, Image: %s,\n",
        seriesNumber, modality, imageNumber);
    printf("  Image UID: %s\n", sopInstance);
    fflush(stdout);
    bzero((char*)&req, sizeof(req));
    req.MessageID = msgId;
    strcpy(req.AffectedSOPClassUID, sopClass);
    strcpy(req.AffectedSOPInstanceUID, sopInstance);
    req.DataSetType = DIMSE_DATASET_PRESENT;
    req.Priority = DIMSE_PRIORITY_MEDIUM;

    cond = DIMSE_storeUser(conf->assoc, presId, &req,
       imgFile, NULL, storeProgressCallback, NULL,
        DIMSE_BLOCKING, 0,
        &rsp, &statusDetail);

#ifdef LOCK_IMAGE_FILES
     /* unlock image file */
    dcmtk_flock(lockfd, LOCK_UN);
    close(lockfd);
#endif

    if (cond.good()) {
        printf("[MsgID %d] Complete [Status: %s]\n", msgId,
            DU_cstoreStatusString(rsp.DimseStatus));
    } else {
        errmsg("[MsgID %d] Failed:", msgId);
        DimseCondition::dump(cond);
        ASC_abortAssociation(conf->assoc);
        ASC_dropAssociation(conf->assoc);
        ASC_destroyAssociation(&conf->assoc);
    }
    if (statusDetail != NULL) {
        printf("  Status Detail:\n");
        statusDetail->print(COUT);
        delete statusDetail;
    }

    return (cond.good());
}


/*
 * Find for remote DBs
 */

typedef struct {
    TI_GenericEntryCallbackFunction cbf;
    TI_GenericCallbackStruct *cbs;
} TI_LocalFindCallbackData;

static void
findCallback(
  /* in */
  void *callbackData,
  T_DIMSE_C_FindRQ * /*request*/ , /* original find request */
  int responseCount,
  T_DIMSE_C_FindRSP *response,  /* pending response received */
  DcmDataset *responseIdentifiers /* pending response identifiers */
  )
{
    TI_LocalFindCallbackData *cbd;

    cbd = (TI_LocalFindCallbackData*)callbackData;

    if (verbose) {
        printf("Find Response %d:\n", responseCount);
        DIMSE_printCFindRSP(stdout, response);
        printf("Identifiers %d:\n", responseCount);
        responseIdentifiers->print(COUT);
    }

    /* call the callback function */
    cbd->cbf(cbd->cbs, responseIdentifiers);

    /* responseIdentifers will be deleted in DIMSE_findUser() */
}

OFBool
TI_remoteFindQuery(TI_Config *conf, TI_DBEntry *db, DcmDataset *query,
    TI_GenericEntryCallbackFunction callbackFunction,
    TI_GenericCallbackStruct *callbackData)
{
    OFBool ok = OFTrue;
    TI_LocalFindCallbackData cbd;
    OFCondition           cond = EC_Normal;
    T_ASC_PresentationContextID presId;
    DIC_US        msgId;
    T_DIMSE_C_FindRQ  req;
    T_DIMSE_C_FindRSP rsp;
    DcmDataset    *statusDetail = NULL;

    conf->currentPeerTitle = db->title;

    /* make sure we have an association */
    ok = TI_changeAssociation(conf);
    if (!ok) return OFFalse;

    cbd.cbf = callbackFunction;
    cbd.cbs = callbackData;

    /* which presentation context should be used */
    presId = ASC_findAcceptedPresentationContextID(conf->assoc,
        UID_FINDStudyRootQueryRetrieveInformationModel);
    if (presId == 0) {
        errmsg("No Presentation Context for Find Operation");
        return OFFalse;
    }

    msgId =  conf->assoc->nextMsgID++;

    if (verbose) {
        printf("Sending Find SCU RQ: MsgID %d:\n", msgId);
        query->print(COUT);
    }

    req.MessageID = msgId;
    strcpy(req.AffectedSOPClassUID,
     UID_FINDStudyRootQueryRetrieveInformationModel);
    req.Priority = DIMSE_PRIORITY_LOW;

    cond = DIMSE_findUser(conf->assoc, presId, &req, query,
      findCallback, &cbd, DIMSE_BLOCKING, 0, &rsp, &statusDetail);

    if (cond.good()) {
        if (verbose) {
            DIMSE_printCFindRSP(stdout, &rsp);
        }
    } else {
        errmsg("Find Failed:");
        DimseCondition::dump(cond);
    }
    if (statusDetail != NULL) {
        printf("  Status Detail:\n");
        statusDetail->print(COUT);
        delete statusDetail;
    }

    return cond.good();

}

/*
** CVS Log
** $Log: tinet.cc,v $
** Revision 1.1  2005/08/23 19:32:03  braindead
** - initial savannah import
**
** Revision 1.1  2005/06/26 19:26:14  pipelka
** - added dcmtk
**
** Revision 1.23  2002/12/13 13:43:29  meichel
** Removed unused code reported by the MIPSpro 7.3 optimizer
**
** Revision 1.22  2002/11/29 07:18:17  wilkens
** Adapted ti utility to command line classes and added option '-xi'.
**
** Revision 1.21  2002/11/27 13:27:02  meichel
** Adapted module imagectn to use of new header file ofstdinc.h
**
** Revision 1.20  2001/11/12 14:54:23  meichel
** Removed all ctndisp related code from imagectn
**
** Revision 1.19  2001/10/12 12:42:56  meichel
** Adapted imagectn to OFCondition based dcmnet module (supports strict mode).
**
** Revision 1.18  2001/06/01 15:51:23  meichel
** Updated copyright header
**
** Revision 1.17  2000/06/07 15:18:47  meichel
** Output stream now passed as mandatory parameter to ASC_dumpParameters.
**
** Revision 1.16  2000/04/14 16:38:21  meichel
** Removed default value from output stream passed to print() method.
**   Required for use in multi-thread environments.
**
** Revision 1.15  2000/03/08 16:41:03  meichel
** Updated copyright header.
**
** Revision 1.14  2000/02/23 15:13:16  meichel
** Corrected macro for Borland C++ Builder 4 workaround.
**
** Revision 1.13  2000/02/03 11:50:30  meichel
** Moved UID related functions from dcmnet (diutil.h) to dcmdata (dcuid.h)
**   where they belong. Renamed access functions to dcmSOPClassUIDToModality
**   and dcmGuessModalityBytes.
**
** Revision 1.12  2000/02/01 11:43:41  meichel
** Avoiding to include <stdlib.h> as extern "C" on Borland C++ Builder 4,
**   workaround for bug in compiler header files.
**
** Revision 1.11  1999/06/10 12:12:04  meichel
** Adapted imagectn to new command line option scheme.
**   Added support for Patient/Study Only Q/R model and C-GET (experimental).
**
** Revision 1.10  1999/04/30 16:37:14  meichel
** Renamed all flock calls to dcmtk_flock to avoid name clash between flock()
** emulation based on fcntl() and a constructor for struct flock.
**
** Revision 1.9  1998/10/26 13:08:17  meichel
** "ti" now negotiates for all known storage SOP classes.
**
** Revision 1.8  1998/08/10 08:56:46  meichel
** renamed member variable in DIMSE structures from "Status" to "DimseStatus".
**
** Revision 1.7  1997/08/06 12:20:17  andreas
** - Using Windows NT with Visual C++ 4.x the standard open mode for files
**   is TEXT with conversions. For binary files (image files, imagectn database
**   index) this must be changed (e.g. fopen(filename, "...b"); or
**   open(filename, ..... |O_BINARY);)
**
** Revision 1.6  1997/07/21 08:59:47  andreas
** - Replace all boolean types (BOOLEAN, CTNBOOLEAN, DICOM_BOOL, BOOL)
**   with one unique boolean type OFBool.
**
** Revision 1.5  1997/06/26 12:59:21  andreas
** - Include Additional headers (winsock.h, io.h) for Windows NT/95
**
** Revision 1.4  1997/05/30 07:33:24  meichel
** Added space characters around comments and simplified
** some inlining code (needed for SunCC 2.0.1).
**
** Revision 1.3  1996/09/27 08:46:24  hewett
** Enclosed system include files with BEGIN_EXTERN_C/END_EXTERN_C.
**
** Revision 1.2  1996/04/25 16:29:10  hewett
** Added char* parameter casts to bzero() calls.
**
** Revision 1.1  1996/04/22 10:27:26  hewett
** Initial release.
**
**
*/
