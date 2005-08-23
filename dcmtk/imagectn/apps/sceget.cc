/*
 *
 *  Copyright (C) 1993-2002, OFFIS
 *
 *  This file is a derivative work of an original developed by medigration 
 *  GmbH and made available to the public under the conditions of the
 *  copyright and permission notice reproduced below.
 *
 *  THIS SOFTWARE IS MADE AVAILABLE,  AS IS,  AND OFFIS MAKES NO  WARRANTY
 *  REGARDING  THE  SOFTWARE,  ITS  PERFORMANCE,  ITS  MERCHANTABILITY  OR
 *  FITNESS FOR ANY PARTICULAR USE, FREEDOM FROM ANY COMPUTER DISEASES  OR
 *  ITS CONFORMITY TO ANY SPECIFICATION. THE ENTIRE RISK AS TO QUALITY AND
 *  PERFORMANCE OF THE SOFTWARE IS WITH THE USER.
 *
 *  Module: imagectn
 *
 *  Author: Andrew Hewett, medigration GmbH
 *
 *  Purpose:
 *    Service Class Executive (SCE) - C-GET Provider
 *    Adapted from scemove.cc - Copyright (C) 1993/1994, OFFIS, Oldenburg University and CERIUM
 *
 * Last Update:		$Author: braindead $
 * Update Date:		$Date: 2005/08/23 19:32:03 $
 * Source File:		$Source: /cvsroot/aeskulap/aeskulap/dcmtk/imagectn/apps/sceget.cc,v $
 * CVS/RCS Revision:	$Revision: 1.1 $
 * Status:		$State: Exp $
 *
 * CVS/RCS Log at end of file
 */

/*
 * Copyright (c) 1998 medigration GmbH. All Rights Reserved.
 *
 * This software is the confidential and proprietary information of 
 * medigration GmbH ("Confidential Information"). You shall not
 * disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered into
 * with medigration GmbH.
 *
 * MEDIGRATION MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY OF
 * THE SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, OR NON-INFRINGEMENT. MEDIGRATION SHALL NOT BE LIABLE FOR
 * ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR
 * DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.
 *
 * Permission is granted to any individual or institution to use, copy, modify,
 * and distribute this software, provided that this complete copyright and
 * permission notice is maintained, intact, in all copies and supporting
 * documentation.
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

#include "sceget.h"

extern OFBool respondIntermediatePending;

/*
 * C-GET SCP
 */


typedef struct {
    DB_Handle	*dbHandle;
    DIC_US	priorStatus;

    T_ASC_Association   *origAssoc;	/* association of requestor */

    OFBool assocStarted;	/* true if the association was started */
    
    T_ASC_PresentationContextID origPresId; /* presentation context id of request */
    DIC_US origMsgId;		/* message id of request */
    DIC_AE origAETitle;		/* title of requestor */
    DIC_NODENAME origHostName;	/* hostname of requestor */

    T_DIMSE_Priority priority;	/* priority of request */
    DIC_AE ourAETitle;  	/* our current title */

    char *failedUIDs;		/* instance UIDs of failed store sub-ops */

    DIC_US nRemaining; 
    DIC_US nCompleted; 
    DIC_US nFailed; 
    DIC_US nWarning;

    OFBool getCancelled;    /* true if the get sub-operations have been cancelled */

} SCE_GetContext;

static void
addFailedUIDInstance(SCE_GetContext *context, const char *sopInstance)
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
getSubOpProgressCallback(void * /* callbackData */, 
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
performGetSubOp(SCE_GetContext *context, 
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
	errmsg("Get SCP: storeSCU: [file: %s]: %s", 
	    fname, strerror(errno));
	context->nFailed++;
	addFailedUIDInstance(context, sopInstance);
	return EC_Normal;
    }
    dcmtk_flock(lockfd, LOCK_SH);
#endif

    msgId = context->origAssoc->nextMsgID++;
 
    /* which presentation context should be used */
    presId = ASC_findAcceptedPresentationContextID(context->origAssoc,
        sopClass);
    if (presId == 0) {
	context->nFailed++;
	addFailedUIDInstance(context, sopInstance);
	errmsg("Get SCP: storeSCU: [file: %s] No presentation context for: (%s) %s", 
	    fname, dcmSOPClassUIDToModality(sopClass), sopClass);
	return DIMSE_NOVALIDPRESENTATIONCONTEXTID;
    } else {
        /* make sure that we can send images in this presentation context */
        T_ASC_PresentationContext pc;
        ASC_findAcceptedPresentationContext(context->origAssoc->params, presId, &pc);
        /* the acceptedRole is the association requestor role */
        if ((pc.acceptedRole != ASC_SC_ROLE_SCP) && (pc.acceptedRole != ASC_SC_ROLE_SCUSCP)) {
            /* the role is not appropriate */
            context->nFailed++;
	    addFailedUIDInstance(context, sopInstance);
	    errmsg("Get SCP: storeSCU: [file: %s] No presentation context with requestor SCP role for: (%s) %s", 
	        fname, dcmSOPClassUIDToModality(sopClass), sopClass);
	    return DIMSE_NOVALIDPRESENTATIONCONTEXTID;
        }
    }

    req.MessageID = msgId;
    strcpy(req.AffectedSOPClassUID, sopClass);
    strcpy(req.AffectedSOPInstanceUID, sopInstance);
    req.DataSetType = DIMSE_DATASET_PRESENT;
    req.Priority = context->priority;
    req.opts = 0;

    if (opt_verbose) {
	printf("Store SCU RQ: MsgID %d, (%s)\n", 
	    msgId, dcmSOPClassUIDToModality(sopClass));
    }

    T_DIMSE_DetectedCancelParameters cancelParameters;

    cond = DIMSE_storeUser(context->origAssoc, presId, &req,
        fname, NULL, getSubOpProgressCallback, context, 
	DIMSE_BLOCKING, 0, 
	&rsp, &statusDetail, &cancelParameters);

#ifdef LOCK_IMAGE_FILES
    /* unlock image file */
    dcmtk_flock(lockfd, LOCK_UN);
    close(lockfd);
#endif

    if (cond.good()) {
        if (cancelParameters.cancelEncountered) {
            if (context->origPresId == cancelParameters.presId && 
                context->origMsgId == cancelParameters.req.MessageIDBeingRespondedTo) {
                context->getCancelled = OFTrue;
            } else {
        	errmsg("Get SCP: Unexpected C-Cancel-RQ encountered: pid=%d, mid=%d", 
                    (int)cancelParameters.presId, (int)cancelParameters.req.MessageIDBeingRespondedTo);
            }
        }
        if (opt_verbose) {
	    printf("Get SCP: Received Store SCU RSP [Status=%s]\n",
	        DU_cstoreStatusString(rsp.DimseStatus));
        }
	if (rsp.DimseStatus == STATUS_Success) {
	    /* everything ok */
	    context->nCompleted++;
	} else if ((rsp.DimseStatus & 0xf000) == 0xb000) {
	    /* a warning status message */
	    context->nWarning++;
	    errmsg("Get SCP: Store Warning: Response Status: %s", 
		DU_cstoreStatusString(rsp.DimseStatus));
        } else {
	    context->nFailed++;
	    addFailedUIDInstance(context, sopInstance);
	    /* print a status message */
	    errmsg("Get SCP: Store Failed: Response Status: %s", 
		DU_cstoreStatusString(rsp.DimseStatus));
	}
    } else {
	context->nFailed++;
	addFailedUIDInstance(context, sopInstance);
	errmsg("Get SCP: storeSCU: Store Request Failed:");
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


static void
getNextImage(SCE_GetContext * context, DB_Status * dbStatus)
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
	errmsg("getSCP: Database: DB_nextMoveResponse Failed (%s):",
	    DU_cmoveStatusString(dbStatus->status));
    }

    if (dbStatus->status == STATUS_Pending) {
	/* perform sub-op */
	cond = performGetSubOp(context, subImgSOPClass,
	    subImgSOPInstance, subImgFileName);

        if (context->getCancelled) {
            dbStatus->status = STATUS_GET_Cancel_SubOperationsTerminatedDueToCancelIndication;
            if (opt_verbose) {
	        printf("Get SCP: Received C-Cancel RQ\n");
            }
       }

        if (cond != EC_Normal) {
	    errmsg("getSCP: Get Sub-Op Failed:");
	    DimseCondition::dump(cond);
	    	/* clear condition stack */
	}
    }
}

static void
buildFailedInstanceList(SCE_GetContext * context,
    DcmDataset ** rspIds)
{
    OFBool ok;

    if (context->failedUIDs != NULL) {
	*rspIds = new DcmDataset();
	ok = DU_putStringDOElement(*rspIds, DCM_FailedSOPInstanceUIDList,
	    context->failedUIDs);
	if (!ok) {
	    errmsg("getSCP: failed to build DCM_FailedSOPInstanceUIDList");
	}
	free(context->failedUIDs);
	context->failedUIDs = NULL;
    }
}


static void 
getCallback(
	/* in */ 
	void *callbackData,  
	OFBool cancelled, T_DIMSE_C_GetRQ *request, 
	DcmDataset *requestIdentifiers, int responseCount,
	/* out */
	T_DIMSE_C_GetRSP *response, DcmDataset **statusDetail,	
	DcmDataset **responseIdentifiers)
{
    OFCondition dbcond = EC_Normal;
    SCE_GetContext *context;
    DB_Status dbStatus;

    context = (SCE_GetContext*)callbackData;	/* recover context */

    dbStatus.status = context->priorStatus;
    dbStatus.statusDetail = NULL;

    if (responseCount == 1) {
        /* start the database search */
	if (opt_verbose) {
	    printf("Get SCP Request Identifiers:\n");
	    requestIdentifiers->print(COUT);
        }
        dbcond = DB_startMoveRequest(context->dbHandle, 
	    request->AffectedSOPClassUID, requestIdentifiers, &dbStatus);
        if (dbcond.bad()) {
	    errmsg("getSCP: Database: DB_startMoveRequest Failed (%s):",
		DU_cmoveStatusString(dbStatus.status));
        }
    }
    
    /* only cancel if we have pending status */
    if (cancelled && dbStatus.status == STATUS_Pending) {
	DB_cancelMoveRequest(context->dbHandle, &dbStatus);
    }

    if (dbStatus.status == STATUS_Pending) {
        getNextImage(context, &dbStatus);
    }

    if (dbStatus.status != STATUS_Pending) {

	/*
	 * Need to adjust the final status if any sub-operations failed or
	 * had warnings 
	 */
	if (context->nFailed > 0 || context->nWarning > 0) {
	    dbStatus.status = 
	        STATUS_GET_Warning_SubOperationsCompleteOneOrMoreFailures;
	}
        /*
         * if all the sub-operations failed then we need to generate a failed or refused status.
         * cf. DICOM part 4, C.4.3.3.1
         * we choose to generate a "Refused - Out of Resources - Unable to perform suboperations" status.
         */
        if ((context->nFailed > 0) && ((context->nCompleted + context->nWarning) == 0)) {
	    dbStatus.status = STATUS_GET_Refused_OutOfResourcesSubOperations;
	}
    }
    
    if (opt_verbose) {
        printf("Get SCP Response %d [status: %s]\n", responseCount,
	    DU_cmoveStatusString(dbStatus.status));
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
    
}

OFCondition
SCE_getSCP(T_ASC_Association * assoc, T_DIMSE_C_GetRQ * request,
	T_ASC_PresentationContextID presID, DB_Handle *dbHandle)
{
    OFCondition cond = EC_Normal;
    SCE_GetContext context;

    bzero((char*)&context, sizeof(context));
    
    context.dbHandle = dbHandle;
    context.priorStatus = STATUS_Pending;
    context.origAssoc = assoc;
    context.origMsgId = request->MessageID;
    context.priority = request->Priority;
    ASC_getAPTitles(assoc->params, NULL, context.ourAETitle, NULL);
    context.assocStarted = OFFalse;
    context.getCancelled = OFFalse;
    context.origPresId = presID;

    if (opt_verbose) {
	printf("Received Get SCP: ");
	DIMSE_printCGetRQ(stdout, request);
    }

    cond = DIMSE_getProvider(assoc, presID, request, 
    	getCallback, &context, DIMSE_BLOCKING, 0);
    if (cond.bad()) {
        errmsg("Get SCP Failed:");
	DimseCondition::dump(cond);
    }
    return cond; 
}


/*
** CVS Log
** $Log: sceget.cc,v $
** Revision 1.1  2005/08/23 19:32:03  braindead
** - initial savannah import
**
** Revision 1.1  2005/06/26 19:26:14  pipelka
** - added dcmtk
**
** Revision 1.11  2002/12/13 13:43:28  meichel
** Removed unused code reported by the MIPSpro 7.3 optimizer
**
** Revision 1.10  2002/11/27 13:27:00  meichel
** Adapted module imagectn to use of new header file ofstdinc.h
**
** Revision 1.9  2001/11/12 14:54:21  meichel
** Removed all ctndisp related code from imagectn
**
** Revision 1.8  2001/10/12 12:42:53  meichel
** Adapted imagectn to OFCondition based dcmnet module (supports strict mode).
**
** Revision 1.7  2001/06/01 15:51:19  meichel
** Updated copyright header
**
** Revision 1.6  2000/04/14 16:38:20  meichel
** Removed default value from output stream passed to print() method.
**   Required for use in multi-thread environments.
**
** Revision 1.5  2000/03/08 16:41:00  meichel
** Updated copyright header.
**
** Revision 1.4  2000/02/23 15:13:09  meichel
** Corrected macro for Borland C++ Builder 4 workaround.
**
** Revision 1.3  2000/02/03 11:50:29  meichel
** Moved UID related functions from dcmnet (diutil.h) to dcmdata (dcuid.h)
**   where they belong. Renamed access functions to dcmSOPClassUIDToModality
**   and dcmGuessModalityBytes.
**
** Revision 1.2  2000/02/01 11:43:40  meichel
** Avoiding to include <stdlib.h> as extern "C" on Borland C++ Builder 4,
**   workaround for bug in compiler header files.
**
** Revision 1.1  1999/06/10 12:15:41  meichel
** Adapted imagectn to new command line option scheme.
**   Added support for Patient/Study Only Q/R model and C-GET (experimental).
**
**
*/
        
