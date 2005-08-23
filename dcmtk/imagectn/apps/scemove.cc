/*
 *
 *  Copyright (C) 1993-2003, OFFIS
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
 *  Purpose: Service Class Executive (SCE) - Move Service Class Provider
 *
 *  Last Update:      $Author: braindead $
 *  Update Date:      $Date: 2005/08/23 19:32:02 $
 *  Source File:      $Source: /cvsroot/aeskulap/aeskulap/dcmtk/imagectn/apps/scemove.cc,v $
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

#include "dicom.h"
#include "imagectn.h"
#include "dimse.h"
#include "diutil.h"
#include "sce.h"
#include "imagedb.h"
#include "cnf.h"
#include "dcdeftag.h"

#include "scemove.h"

OFBool respondIntermediatePending = OFTrue;
OFBool strictMoveRequirements = OFFalse;

/*
 * C-MOVE SCP
 */


typedef struct {
    DB_Handle	*dbHandle;
    DIC_US	priorStatus;

    T_ASC_Association   *origAssoc;	/* association of requestor */
    T_ASC_Association   *subAssoc;	/* sub-association */

    OFBool assocStarted;	/* true if the association was started */
    
    DIC_US origMsgId;		/* message id of request */
    DIC_AE origAETitle;		/* title of requestor */
    DIC_NODENAME origHostName;	/* hostname of move requestor */

    T_DIMSE_Priority priority;	/* priority of move request */
    DIC_AE ourAETitle;  	/* our current title */
    DIC_AE dstAETitle;		/* destination title for move */

    char *failedUIDs;		/* instance UIDs of failed store sub-ops */

    DIC_US nRemaining; 
    DIC_US nCompleted; 
    DIC_US nFailed; 
    DIC_US nWarning;

} SCE_MoveContext;

static void
addFailedUIDInstance(SCE_MoveContext *context, const char *sopInstance)
{
    int len;

    if (context->failedUIDs == NULL) {
	if ((context->failedUIDs = (char*)malloc(DIC_UI_LEN+1)) == NULL) {
	    errmsg("malloc failure: addFailedUIDInstance");
	    return;
	}
	strcpy(context->failedUIDs, sopInstance);
    } else {
	len = strlen(context->failedUIDs);
	if ((context->failedUIDs = (char*)realloc(context->failedUIDs, 
	    (len+strlen(sopInstance)+2))) == NULL) {
	    errmsg("realloc failure: addFailedUIDInstance");
	    return;
	}
	/* tag sopInstance onto end of old with '\' between */
	strcat(context->failedUIDs, "\\");
	strcat(context->failedUIDs, sopInstance);
    }
}

static void
moveSubOpProgressCallback(void * /*callbackData*/, 
    T_DIMSE_StoreProgress *progress,
    T_DIMSE_C_StoreRQ * /*req*/)
{
  if (opt_verbose)
  {
    switch (progress->state)
    {
      case DIMSE_StoreBegin:
        printf("XMIT:");
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

static OFCondition
performMoveSubOp(SCE_MoveContext *context, 
	DIC_UI sopClass, DIC_UI sopInstance,
	char *fname)
{
    OFCondition cond = EC_Normal;
    T_DIMSE_C_StoreRQ req;
    T_DIMSE_C_StoreRSP rsp;
    DIC_US msgId;
    T_ASC_PresentationContextID presId;
    DcmDataset *statusDetail = NULL;

#ifdef LOCK_IMAGE_FILES
    /* shared lock image file */
    int lockfd;
#ifdef O_BINARY
    lockfd = open(fname, O_RDONLY | O_BINARY, 0666);
#else
    lockfd = open(fname, O_RDONLY , 0666);
#endif
    if (lockfd < 0) {
        /* due to quota system the file could have been deleted */
	errmsg("Move SCP: storeSCU: [file: %s]: %s", 
	    fname, strerror(errno));
	context->nFailed++;
	addFailedUIDInstance(context, sopInstance);
	return EC_Normal;
    }
    dcmtk_flock(lockfd, LOCK_SH);
#endif

    msgId = context->subAssoc->nextMsgID++;
 
    /* which presentation context should be used */
    presId = ASC_findAcceptedPresentationContextID(context->subAssoc,
        sopClass);
    if (presId == 0) {
	context->nFailed++;
	addFailedUIDInstance(context, sopInstance);
	errmsg("Move SCP: storeSCU: [file: %s] No presentation context for: (%s) %s", 
	    fname, dcmSOPClassUIDToModality(sopClass), sopClass);
	return DIMSE_NOVALIDPRESENTATIONCONTEXTID;
    }

    req.MessageID = msgId;
    strcpy(req.AffectedSOPClassUID, sopClass);
    strcpy(req.AffectedSOPInstanceUID, sopInstance);
    req.DataSetType = DIMSE_DATASET_PRESENT;
    req.Priority = context->priority;
    req.opts = (O_STORE_MOVEORIGINATORAETITLE | O_STORE_MOVEORIGINATORID);
    strcpy(req.MoveOriginatorApplicationEntityTitle,
	context->origAETitle);
    req.MoveOriginatorID = context->origMsgId;

    if (opt_verbose) {
	printf("Store SCU RQ: MsgID %d, (%s)\n", 
	    msgId, dcmSOPClassUIDToModality(sopClass));
    }

    cond = DIMSE_storeUser(context->subAssoc, presId, &req,
        fname, NULL, moveSubOpProgressCallback, NULL, 
	DIMSE_BLOCKING, 0, 
	&rsp, &statusDetail);
	
#ifdef LOCK_IMAGE_FILES
    /* unlock image file */
    dcmtk_flock(lockfd, LOCK_UN);
    close(lockfd);
#endif
	
    if (cond.good()) {
        if (opt_verbose) {
	    printf("Move SCP: Received Store SCU RSP [Status=%s]\n",
	        DU_cstoreStatusString(rsp.DimseStatus));
        }
	if (rsp.DimseStatus == STATUS_Success) {
	    /* everything ok */
	    context->nCompleted++;
	} else if ((rsp.DimseStatus & 0xf000) == 0xb000) {
	    /* a warning status message */
	    context->nWarning++;
	    errmsg("Move SCP: Store Waring: Response Status: %s", 
		DU_cstoreStatusString(rsp.DimseStatus));
	} else {
	    context->nFailed++;
	    addFailedUIDInstance(context, sopInstance);
	    /* print a status message */
	    errmsg("Move SCP: Store Failed: Response Status: %s", 
		DU_cstoreStatusString(rsp.DimseStatus));
	}
    } else {
	context->nFailed++;
	addFailedUIDInstance(context, sopInstance);
	errmsg("Move SCP: storeSCU: Store Request Failed:");
	DimseCondition::dump(cond);
    }
    if (statusDetail != NULL) {
        if (opt_verbose) {
	    printf("  Status Detail:\n");
	    statusDetail->print(COUT);
	}
        delete statusDetail;
    }
    return cond;
}

static OFBool
mapMoveDestination(const char *origPeer, const char *origAE,
	const char *dstAE, char *dstPeer, int *dstPort)
{
    /*
     * This routine enforces RSNA'93 Demo Requirements regarding
     * the destination of move commands.
     * 
     */
    OFBool ok = OFFalse;
    const char *dstPeerName; /* the CNF utility returns us a static char* */

    if (opt_restrictMoveToSameAE) {
        /* AE Titles the same ? */
        ok = (strcmp(origAE, dstAE) == 0);
        if (!ok) {
	    if (opt_verbose) {
	        printf("mapMoveDestination: strictMove Reqs: '%s' != '%s'\n",
		    origAE, dstAE); 
	    }
	    return OFFalse;
	}
    }
    
    ok = CNF_peerForAETitle((char*)dstAE, &dstPeerName, dstPort) > 0;
    if (!ok) {
        if (opt_verbose) {
	    printf("mapMoveDestination: unknown AE: '%s'\n", dstAE);
	}
        return OFFalse;	/* dstAE not known */
    }
    
    strcpy(dstPeer, dstPeerName);

    if (opt_restrictMoveToSameHost) {
	/* hosts the same ? */
	ok = (strcmp(origPeer, dstPeer) == 0);
	if (!ok) {
	    if (opt_verbose) {
		printf("mapMoveDestination: different hosts: '%s', '%s'\n",
		       origPeer, dstPeer);
	    }
	    return OFFalse;
	}
    }

    if (opt_restrictMoveToSameVendor) {
	/* AE titles belong to the same vendor */
	ok = CNF_checkForSameVendor((char*)origAE, (char*)dstAE) > 0;
	if (!ok) {
	    if (opt_verbose) {
		printf("mapMoveDestination: different vendors: '%s', '%s'\n",
		       origAE, dstAE);
	    }
	    return OFFalse;
	}
    }
    
    return ok;
}

static OFCondition
addAllStoragePresentationContexts(T_ASC_Parameters *params)
{
// #error need to add support for compressed transfer etc. here

    OFCondition cond = EC_Normal;

    int i;
    int pid = 1;


    const char* transferSyntaxes[] = { 
	NULL, NULL, NULL };
    int transferSyntaxesCount = 3;
        
//    if (opt_useLittleEndianImplicitOnly) {
//	transferSyntaxes[0] = UID_LittleEndianImplicitTransferSyntax;
//        transferSyntaxesCount = 1;
//    } else {
        /* 
        ** We prefer to accept Explicitly encoded transfer syntaxes.
        ** If we are running on a Little Endian machine we prefer 
        ** LittleEndianExplicitTransferSyntax to BigEndianTransferSyntax.
        ** Some SCP implementations will just select the first transfer
        ** syntax they support (this is not part of the standard) so
        ** organise the proposed transfer syntaxes to take advantage
        ** of such behaviour.
        */

        /* gLocalByteOrder is defined in dcxfer.h */
        if (gLocalByteOrder == EBO_LittleEndian) {
    	/* we are on a little endian machine */
            transferSyntaxes[0] = UID_LittleEndianExplicitTransferSyntax;
            transferSyntaxes[1] = UID_BigEndianExplicitTransferSyntax;
            transferSyntaxes[2] = UID_LittleEndianImplicitTransferSyntax;
            transferSyntaxesCount = 3;
        } else {
            /* we are on a big endian machine */
            transferSyntaxes[0] = UID_BigEndianExplicitTransferSyntax;
            transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
            transferSyntaxes[2] = UID_LittleEndianImplicitTransferSyntax;
            transferSyntaxesCount = 3;
        }
//    }

    for (i=0; i<numberOfDcmStorageSOPClassUIDs && cond.good(); i++) {
	cond = ASC_addPresentationContext(
	    params, pid, dcmStorageSOPClassUIDs[i],
	    transferSyntaxes, transferSyntaxesCount);
	pid += 2;	/* only odd presentation context id's */
    }
    return cond;
}

static OFCondition
buildSubAssociation(T_DIMSE_C_MoveRQ *request, SCE_MoveContext *context)
{
    OFCondition cond = EC_Normal;
    DIC_NODENAME dstHostName;
    DIC_NODENAME dstHostNamePlusPort;
    int dstPortNumber;
    DIC_NODENAME localHostName;
    T_ASC_Parameters *params;

    strcpy(context->dstAETitle, request->MoveDestination);
    /*
     * We must map the destination AE Title into a host name and port
     * address.  Further, we must make sure that the RSNA'93 demonstration
     * rules are observed regarding move destinations. 
     */
    ASC_getAPTitles(context->origAssoc->params, context->origAETitle,
	context->ourAETitle, NULL);
    ASC_getPresentationAddresses(context->origAssoc->params, 
        context->origHostName, NULL);

    if (!mapMoveDestination(context->origHostName, context->origAETitle,
	request->MoveDestination, dstHostName, &dstPortNumber)) {
	return APP_INVALIDPEER;
    }
    if (cond.good()) {
	cond = ASC_createAssociationParameters(&params, ASC_DEFAULTMAXPDU);
	if (cond.bad()) {
	    errmsg("moveSCP: Cannot create Association-params for sub-ops:");
	    DimseCondition::dump(cond);
	}
    }
    if (cond.good()) {
	gethostname(localHostName, sizeof(localHostName) - 1);
	sprintf(dstHostNamePlusPort, "%s:%d", dstHostName, dstPortNumber);
	ASC_setPresentationAddresses(params, localHostName, 
		dstHostNamePlusPort);
	ASC_setAPTitles(params, context->ourAETitle, context->dstAETitle,NULL);

	cond = addAllStoragePresentationContexts(params);
	if (cond.bad()) {
	    DimseCondition::dump(cond);
	}
	if (opt_debug) {
	    printf("Request Parameters:\n");
	    ASC_dumpParameters(params, COUT);
	}
    }
    if (cond.good()) {
	/* create association */
	if (opt_verbose)
	    printf("Requesting Sub-Association\n");
	cond = ASC_requestAssociation(net, params,
				      &context->subAssoc);
	if (cond.bad()) {
	    if (cond == DUL_ASSOCIATIONREJECTED) {
		T_ASC_RejectParameters rej;

		ASC_getRejectParameters(params, &rej);
		errmsg("moveSCP: Sub-Association Rejected");
		ASC_printRejectParameters(stderr, &rej);
		fprintf(stderr, "\n");
	    } else {
		errmsg("moveSCP: Sub-Association Request Failed:");
		DimseCondition::dump(cond);
		
	    }
	}
    }

    if (cond.good()) {
	context->assocStarted = OFTrue;
    }    
    return cond;
}

static OFCondition
closeSubAssociation(SCE_MoveContext *context)
{
    OFCondition cond = EC_Normal;

    if (context->subAssoc != NULL) {
	/* release association */
	if (opt_verbose)
	    printf("Releasing Sub-Association\n");
	cond = ASC_releaseAssociation(context->subAssoc);
	if (cond.bad()) {
	    errmsg("moveSCP: Sub-Association Release Failed:");
	    DimseCondition::dump(cond);
	}
	cond = ASC_dropAssociation(context->subAssoc);
	if (cond.bad()) {
	    errmsg("moveSCP: Sub-Association Drop Failed:");
	    DimseCondition::dump(cond);
	}
	cond = ASC_destroyAssociation(&context->subAssoc);
	if (cond.bad()) {
	    errmsg("moveSCP: Sub-Association Destroy Failed:");
	    DimseCondition::dump(cond);
	}

    }

    if (context->assocStarted) {
	context->assocStarted = OFFalse;
    }

    return cond;
}

static void
moveNextImage(SCE_MoveContext * context, DB_Status * dbStatus)
{
    OFCondition cond = EC_Normal;
    OFCondition dbcond = EC_Normal;
    DIC_UI subImgSOPClass;	/* sub-operation image SOP Class */
    DIC_UI subImgSOPInstance;	/* sub-operation image SOP Instance */
    char subImgFileName[MAXPATHLEN + 1];	/* sub-operation image file */

    /* clear out strings */
    bzero(subImgFileName, sizeof(subImgFileName));
    bzero(subImgSOPClass, sizeof(subImgSOPClass));
    bzero(subImgSOPInstance, sizeof(subImgSOPInstance));

    /* get DB response */
    dbcond = DB_nextMoveResponse(context->dbHandle,
	subImgSOPClass, subImgSOPInstance, subImgFileName,
	&context->nRemaining, dbStatus);
    if (dbcond.bad()) {
	errmsg("moveSCP: Database: DB_nextMoveResponse Failed (%s):",
	    DU_cmoveStatusString(dbStatus->status));
    }

    if (dbStatus->status == STATUS_Pending) {
	/* perform sub-op */
	cond = performMoveSubOp(context, subImgSOPClass,
	    subImgSOPInstance, subImgFileName);
	if (cond != EC_Normal) {
	    errmsg("moveSCP: Move Sub-Op Failed:");
	    DimseCondition::dump(cond);
	    	/* clear condition stack */
	}
    }
}

static void
failAllSubOperations(SCE_MoveContext * context, DB_Status * dbStatus)
{
    OFCondition dbcond = EC_Normal;
    DIC_UI subImgSOPClass;	/* sub-operation image SOP Class */
    DIC_UI subImgSOPInstance;	/* sub-operation image SOP Instance */
    char subImgFileName[MAXPATHLEN + 1];	/* sub-operation image file */

    /* clear out strings */
    bzero(subImgFileName, sizeof(subImgFileName));
    bzero(subImgSOPClass, sizeof(subImgSOPClass));
    bzero(subImgSOPInstance, sizeof(subImgSOPInstance));

    while (dbStatus->status == STATUS_Pending) {
        /* get DB response */
        dbcond = DB_nextMoveResponse(context->dbHandle,
	    subImgSOPClass, subImgSOPInstance, subImgFileName,
	    &context->nRemaining, dbStatus);
        if (dbcond.bad()) {
	    errmsg("moveSCP: Database: DB_nextMoveResponse Failed (%s):",
	        DU_cmoveStatusString(dbStatus->status));
        }

	if (dbStatus->status == STATUS_Pending) {
	    context->nFailed++;
	    addFailedUIDInstance(context, subImgSOPInstance);
	}
    }
    dbStatus->status = 
        STATUS_MOVE_Warning_SubOperationsCompleteOneOrMoreFailures;    
}

static void
buildFailedInstanceList(SCE_MoveContext * context,
    DcmDataset ** rspIds)
{
    OFBool ok;

    if (context->failedUIDs != NULL) {
	*rspIds = new DcmDataset();
	ok = DU_putStringDOElement(*rspIds, DCM_FailedSOPInstanceUIDList,
	    context->failedUIDs);
	if (!ok) {
	    errmsg("moveSCP: failed to build DCM_FailedSOPInstanceUIDList");
	}
	free(context->failedUIDs);
	context->failedUIDs = NULL;
    }
}


static void 
moveCallback(
	/* in */ 
	void *callbackData,  
	OFBool cancelled, T_DIMSE_C_MoveRQ *request, 
	DcmDataset *requestIdentifiers, int responseCount,
	/* out */
	T_DIMSE_C_MoveRSP *response, DcmDataset **statusDetail,	
	DcmDataset **responseIdentifiers)
{
    OFCondition cond = EC_Normal;
    OFCondition dbcond = EC_Normal;
    SCE_MoveContext *context;
    DB_Status dbStatus;

    context = (SCE_MoveContext*)callbackData;	/* recover context */

    dbStatus.status = context->priorStatus;
    dbStatus.statusDetail = NULL;
    
    if (responseCount == 1) {
        /* start the database search */
	if (opt_verbose) {
	    printf("Move SCP Request Identifiers:\n");
	    requestIdentifiers->print(COUT);
        }
        dbcond = DB_startMoveRequest(context->dbHandle, 
	    request->AffectedSOPClassUID, requestIdentifiers, &dbStatus);
        if (dbcond.bad()) {
	    errmsg("moveSCP: Database: DB_startMoveRequest Failed (%s):",
		DU_cmoveStatusString(dbStatus.status));
        }

        if (dbStatus.status == STATUS_Pending) {
            /* If we are going to be performing sub-operations, build
             * a new association to the move destination.
             */
	    cond = buildSubAssociation(request, context);
	    if (cond == APP_INVALIDPEER) {
	        dbStatus.status = STATUS_MOVE_Failed_MoveDestinationUnknown;
	    } else if (cond.bad()) {
	        /* failed to build association, must fail move */
		failAllSubOperations(context, &dbStatus);
	    }
        }
    }
    
    /* only cancel if we have pending status */
    if (cancelled && dbStatus.status == STATUS_Pending) {
	DB_cancelMoveRequest(context->dbHandle, &dbStatus);
    }

    if (dbStatus.status == STATUS_Pending) {
        moveNextImage(context, &dbStatus);
    }

    if (dbStatus.status != STATUS_Pending) {
	/*
	 * Tear down sub-association (if it exists).
	 */
	closeSubAssociation(context);

	/*
	 * Need to adjust the final status if any sub-operations failed or
	 * had warnings 
	 */
	if (context->nFailed > 0 || context->nWarning > 0) {
	    dbStatus.status = 
	        STATUS_MOVE_Warning_SubOperationsCompleteOneOrMoreFailures;
	}
        /*
         * if all the sub-operations failed then we need to generate a failed or refused status.
         * cf. DICOM part 4, C.4.2.3.1
         * we choose to generate a "Refused - Out of Resources - Unable to perform suboperations" status.
         */
        if ((context->nFailed > 0) && ((context->nCompleted + context->nWarning) == 0)) {
	    dbStatus.status = STATUS_MOVE_Refused_OutOfResourcesSubOperations;
	}
    }
    
    if (dbStatus.status != STATUS_Success && 
        dbStatus.status != STATUS_Pending) {
	/* 
	 * May only include response identifiers if not Success 
	 * and not Pending 
	 */
	buildFailedInstanceList(context, responseIdentifiers);
    }

    /* set response status */
    response->DimseStatus = dbStatus.status;
    response->NumberOfRemainingSubOperations = context->nRemaining;
    response->NumberOfCompletedSubOperations = context->nCompleted;
    response->NumberOfFailedSubOperations = context->nFailed;
    response->NumberOfWarningSubOperations = context->nWarning;
    *statusDetail = dbStatus.statusDetail;

    if (opt_verbose) {
        printf("Move SCP Response %d [status: %s]\n", responseCount,
	    DU_cmoveStatusString(dbStatus.status));
    }
    if (opt_verbose > 1) {
        DIMSE_printCMoveRSP(stdout, response);
        if (DICOM_PENDING_STATUS(dbStatus.status) && (*responseIdentifiers != NULL)) {
            printf("Move SCP Response Identifiers:\n");
            (*responseIdentifiers)->print(COUT);
        }
        if (dbStatus.statusDetail) {
            printf("Status detail:\n");
            dbStatus.statusDetail->print(COUT);
        }
    }    
}

OFCondition
SCE_moveSCP(T_ASC_Association * assoc, T_DIMSE_C_MoveRQ * request,
	T_ASC_PresentationContextID presID, DB_Handle *dbHandle)
{
    OFCondition cond = EC_Normal;
    SCE_MoveContext context;

    bzero((char*)&context, sizeof(context));
    
    context.dbHandle = dbHandle;
    context.priorStatus = STATUS_Pending;
    context.origAssoc = assoc;
    context.origMsgId = request->MessageID;
    context.priority = request->Priority;
    ASC_getAPTitles(assoc->params, NULL, context.ourAETitle, NULL);
    context.assocStarted = OFFalse;

    if (opt_verbose) {
	printf("Received Move SCP: ");
	DIMSE_printCMoveRQ(stdout, request);
    }

    cond = DIMSE_moveProvider(assoc, presID, request, 
    	moveCallback, &context, DIMSE_BLOCKING, 0);
    if (cond.bad()) {
        errmsg("Move SCP Failed:");
	DimseCondition::dump(cond);
    }
    return cond; 
}


/*
** CVS Log
** $Log: scemove.cc,v $
** Revision 1.1  2005/08/23 19:32:02  braindead
** - initial savannah import
**
** Revision 1.1  2005/06/26 19:26:14  pipelka
** - added dcmtk
**
** Revision 1.23  2003/09/04 10:07:42  joergr
** Fixed wrong use of OFBool/bool variable.
**
** Revision 1.22  2002/12/13 13:43:28  meichel
** Removed unused code reported by the MIPSpro 7.3 optimizer
**
** Revision 1.21  2002/11/27 13:27:01  meichel
** Adapted module imagectn to use of new header file ofstdinc.h
**
** Revision 1.20  2001/11/12 14:54:21  meichel
** Removed all ctndisp related code from imagectn
**
** Revision 1.19  2001/10/12 12:42:54  meichel
** Adapted imagectn to OFCondition based dcmnet module (supports strict mode).
**
** Revision 1.18  2001/06/01 15:51:20  meichel
** Updated copyright header
**
** Revision 1.17  2000/06/07 15:18:46  meichel
** Output stream now passed as mandatory parameter to ASC_dumpParameters.
**
** Revision 1.16  2000/04/14 16:38:21  meichel
** Removed default value from output stream passed to print() method.
**   Required for use in multi-thread environments.
**
** Revision 1.15  2000/03/08 16:41:00  meichel
** Updated copyright header.
**
** Revision 1.14  2000/02/23 15:13:10  meichel
** Corrected macro for Borland C++ Builder 4 workaround.
**
** Revision 1.13  2000/02/03 11:50:29  meichel
** Moved UID related functions from dcmnet (diutil.h) to dcmdata (dcuid.h)
**   where they belong. Renamed access functions to dcmSOPClassUIDToModality
**   and dcmGuessModalityBytes.
**
** Revision 1.12  2000/02/01 11:43:40  meichel
** Avoiding to include <stdlib.h> as extern "C" on Borland C++ Builder 4,
**   workaround for bug in compiler header files.
**
** Revision 1.11  1999/06/10 12:11:59  meichel
** Adapted imagectn to new command line option scheme.
**   Added support for Patient/Study Only Q/R model and C-GET (experimental).
**
** Revision 1.10  1999/04/30 16:37:12  meichel
** Renamed all flock calls to dcmtk_flock to avoid name clash between flock()
** emulation based on fcntl() and a constructor for struct flock.
**
** Revision 1.9  1998/08/10 08:56:45  meichel
** renamed member variable in DIMSE structures from "Status" to "DimseStatus".
**
** Revision 1.8  1997/08/06 12:20:16  andreas
** - Using Windows NT with Visual C++ 4.x the standard open mode for files
**   is TEXT with conversions. For binary files (image files, imagectn database
**   index) this must be changed (e.g. fopen(filename, "...b"); or
**   open(filename, ..... |O_BINARY);)
**
** Revision 1.7  1997/07/21 08:59:44  andreas
** - Replace all boolean types (BOOLEAN, CTNBOOLEAN, DICOM_BOOL, BOOL)
**   with one unique boolean type OFBool.
**
** Revision 1.6  1997/06/26 12:59:19  andreas
** - Include Additional headers (winsock.h, io.h) for Windows NT/95
**
** Revision 1.5  1996/09/27 08:46:22  hewett
** Enclosed system include files with BEGIN_EXTERN_C/END_EXTERN_C.
**
** Revision 1.4  1996/09/24 15:52:35  hewett
** Now uses global table of Storage SOP Class UIDs (from dcuid.h).
** Also added preliminary support for the Macintosh environment (GUSI library).
**
** Revision 1.3  1996/04/25 16:29:09  hewett
** Added char* parameter casts to bzero() calls.
**
** Revision 1.2  1996/04/22 10:25:38  hewett
** Implements optional restrictions on move destination.  Now uses a global
** DICOM network rather than a local instance.
**
** Revision 1.1.1.1  1996/03/28 19:24:59  hewett
** Oldenburg Image CTN Software ported to use the dcmdata C++ toolkit.
**
**
*/
