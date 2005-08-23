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
 *  Purpose: Service Class Executive (SCE) - Find Service Class Provider
 *
 *  Last Update:      $Author: braindead $
 *  Update Date:      $Date: 2005/08/23 19:32:03 $
 *  Source File:      $Source: /cvsroot/aeskulap/aeskulap/dcmtk/imagectn/apps/scefind.cc,v $
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

#include "dicom.h"
#include "imagectn.h"
#include "dimse.h"
#include "diutil.h"
#include "sce.h"
#include "imagedb.h"
#include "cnf.h"
#include "dcdeftag.h"

#include "scefind.h"


typedef struct {
    DB_Handle	*dbHandle;
    DIC_US	priorStatus;
    DIC_AE	ourAETitle;
} SCE_FindContext;

static void
addRetreiveAETitle(DcmDataset *rspIds, DIC_AE ourAETitle)
{
    /*
     * Since images are stored only by us (for RSNA'93 demo),
     * we must add in our AE Title to the response identifiers.
     * The DB cannot do this since it does not know our AE Title.
     */
    OFBool ok;

    ok = DU_putStringDOElement(rspIds, DCM_RetrieveAETitle, ourAETitle);
    if (!ok) {
	errmsg("DO Error: adding Retreive AE Title");
    }
}



static void
findCallback(
	/* in */ 
	void *callbackData,  
	OFBool cancelled, T_DIMSE_C_FindRQ *request, 
	DcmDataset *requestIdentifiers, int responseCount,
	/* out */
	T_DIMSE_C_FindRSP *response,
	DcmDataset **responseIdentifiers,
	DcmDataset **statusDetail)
{
    OFCondition dbcond = EC_Normal;
    SCE_FindContext *context;
    DB_Status dbStatus;

    context = (SCE_FindContext*)callbackData;	/* recover context */

    dbStatus.status = context->priorStatus;
    dbStatus.statusDetail = NULL;
    
    if (responseCount == 1) {
        /* start the database search */
	if (opt_verbose) {
	    printf("Find SCP Request Identifiers:\n");
	    requestIdentifiers->print(COUT);
        }
        dbcond = DB_startFindRequest(context->dbHandle, 
	    request->AffectedSOPClassUID, requestIdentifiers, &dbStatus);
        if (dbcond.bad()) {
	    errmsg("findSCP: Database: DB_startFindRequest Failed (%s):",
		DU_cfindStatusString(dbStatus.status));
        }
    }
    
    /* only cancel if we have pending responses */
    if (cancelled && DICOM_PENDING_STATUS(dbStatus.status)) {
	DB_cancelFindRequest(context->dbHandle, &dbStatus);
    }

    if (DICOM_PENDING_STATUS(dbStatus.status)) {
	dbcond = DB_nextFindResponse(context->dbHandle,
		responseIdentifiers, &dbStatus);
	if (dbcond.bad()) {
	     errmsg("findSCP: Database: DB_nextFindResponse Failed (%s):",
		 DU_cfindStatusString(dbStatus.status));
	}
    }

    if (*responseIdentifiers != NULL) {
	addRetreiveAETitle(*responseIdentifiers, context->ourAETitle);
    }

    /* set response status */
    response->DimseStatus = dbStatus.status;
    *statusDetail = dbStatus.statusDetail;

    if (opt_verbose) {
        printf("Find SCP Response %d [status: %s]\n", responseCount,
	    DU_cfindStatusString(dbStatus.status));
    }
    if (opt_verbose > 1) {
        DIMSE_printCFindRSP(stdout, response);
        if (DICOM_PENDING_STATUS(dbStatus.status) && (*responseIdentifiers != NULL)) {
            printf("Find SCP Response Identifiers:\n");
            (*responseIdentifiers)->print(COUT);
        }
        if (dbStatus.statusDetail) {
            printf("Status detail:\n");
            dbStatus.statusDetail->print(COUT);
        }
    }
}


OFCondition
SCE_findSCP(T_ASC_Association * assoc, T_DIMSE_C_FindRQ * request,
	T_ASC_PresentationContextID presID,
	DB_Handle *dbHandle)

{
    OFCondition cond = EC_Normal;
    SCE_FindContext context;

    context.dbHandle = dbHandle;
    context.priorStatus = STATUS_Pending;
    ASC_getAPTitles(assoc->params, NULL, context.ourAETitle, NULL);

    if (opt_verbose) {
	printf("Received Find SCP: ");
	DIMSE_printCFindRQ(stdout, request);
    }

    cond = DIMSE_findProvider(assoc, presID, request, 
    	findCallback, &context, DIMSE_BLOCKING, 0);
    if (cond.bad()) {
        errmsg("Find SCP Failed:");
	DimseCondition::dump(cond);
    }
    return cond; 
}


/*
** CVS Log
** $Log: scefind.cc,v $
** Revision 1.1  2005/08/23 19:32:03  braindead
** - initial savannah import
**
** Revision 1.1  2005/06/26 19:26:14  pipelka
** - added dcmtk
**
** Revision 1.14  2002/11/27 13:27:00  meichel
** Adapted module imagectn to use of new header file ofstdinc.h
**
** Revision 1.13  2001/11/12 14:54:20  meichel
** Removed all ctndisp related code from imagectn
**
** Revision 1.12  2001/10/12 12:42:52  meichel
** Adapted imagectn to OFCondition based dcmnet module (supports strict mode).
**
** Revision 1.11  2001/06/01 15:51:19  meichel
** Updated copyright header
**
** Revision 1.10  2000/04/14 16:38:20  meichel
** Removed default value from output stream passed to print() method.
**   Required for use in multi-thread environments.
**
** Revision 1.9  2000/03/08 16:40:59  meichel
** Updated copyright header.
**
** Revision 1.8  2000/02/23 15:13:08  meichel
** Corrected macro for Borland C++ Builder 4 workaround.
**
** Revision 1.7  2000/02/01 11:43:40  meichel
** Avoiding to include <stdlib.h> as extern "C" on Borland C++ Builder 4,
**   workaround for bug in compiler header files.
**
** Revision 1.6  1999/06/10 12:11:57  meichel
** Adapted imagectn to new command line option scheme.
**   Added support for Patient/Study Only Q/R model and C-GET (experimental).
**
** Revision 1.5  1998/08/10 08:56:44  meichel
** renamed member variable in DIMSE structures from "Status" to "DimseStatus".
**
** Revision 1.4  1997/07/21 08:59:44  andreas
** - Replace all boolean types (BOOLEAN, CTNBOOLEAN, DICOM_BOOL, BOOL)
**   with one unique boolean type OFBool.
**
** Revision 1.3  1996/09/27 08:46:21  hewett
** Enclosed system include files with BEGIN_EXTERN_C/END_EXTERN_C.
**
** Revision 1.2  1996/04/22 10:21:38  hewett
** Formatting change.
**
** Revision 1.1.1.1  1996/03/28 19:24:59  hewett
** Oldenburg Image CTN Software ported to use the dcmdata C++ toolkit.
**
**
*/
