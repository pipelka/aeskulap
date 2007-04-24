/*
 *
 *  Copyright (C) 1993-2005, OFFIS
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
 *  Module:  dcmqrdb
 *
 *  Author:  Marco Eichelberg
 *
 *  Purpose: class DcmQueryRetrieveFindContext
 *
 *  Last Update:      $Author$
 *  Update Date:      $Date$
 *  Source File:      $Source$
 *  CVS/RCS Revision: $Revision$
 *  Status:           $State$
 *
 *  CVS/RCS Log at end of file
 *
 */

#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */
#include "dcmtk/dcmqrdb/dcmqrcbf.h"

#include "dcmtk/dcmqrdb/dcmqrcnf.h"
#include "dcmtk/dcmdata/dcdeftag.h"
#include "dcmtk/dcmqrdb/dcmqropt.h"
#include "dcmtk/dcmnet/diutil.h"
#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/dcmqrdb/dcmqrdbs.h"
#include "dcmtk/dcmqrdb/dcmqrdbi.h"


void DcmQueryRetrieveFindContext::callbackHandler(
	/* in */ 
	OFBool cancelled, T_DIMSE_C_FindRQ *request, 
	DcmDataset *requestIdentifiers, int responseCount,
	/* out */
	T_DIMSE_C_FindRSP *response,
	DcmDataset **responseIdentifiers,
	DcmDataset **stDetail)
{
    OFCondition dbcond = EC_Normal;
    DcmQueryRetrieveDatabaseStatus dbStatus(priorStatus);
    
    if (responseCount == 1) {
        /* start the database search */
	if (options_.verbose_) {
	    printf("Find SCP Request Identifiers:\n");
	    requestIdentifiers->print(COUT);
        }
        dbcond = dbHandle.startFindRequest(
	    request->AffectedSOPClassUID, requestIdentifiers, &dbStatus);
        if (dbcond.bad()) {
	    DcmQueryRetrieveOptions::errmsg("findSCP: Database: startFindRequest Failed (%s):",
		DU_cfindStatusString(dbStatus.status()));
        }
    }
    
    /* only cancel if we have pending responses */
    if (cancelled && DICOM_PENDING_STATUS(dbStatus.status())) {
	dbHandle.cancelFindRequest(&dbStatus);
    }

    if (DICOM_PENDING_STATUS(dbStatus.status())) {
	dbcond = dbHandle.nextFindResponse(responseIdentifiers, &dbStatus);
	if (dbcond.bad()) {
	     DcmQueryRetrieveOptions::errmsg("findSCP: Database: nextFindResponse Failed (%s):",
		 DU_cfindStatusString(dbStatus.status()));
	}
    }

    if (*responseIdentifiers != NULL)
    {

      if (! DU_putStringDOElement(*responseIdentifiers, DCM_RetrieveAETitle, ourAETitle.c_str())) {
	DcmQueryRetrieveOptions::errmsg("DO Error: adding Retrieve AE Title");
      }
    }

    /* set response status */
    response->DimseStatus = dbStatus.status();
    *stDetail = dbStatus.extractStatusDetail();

    if (options_.verbose_) {
        printf("Find SCP Response %d [status: %s]\n", responseCount,
	    DU_cfindStatusString(dbStatus.status()));
    }
    if (options_.verbose_ > 1) {
        DIMSE_printCFindRSP(stdout, response);
        if (DICOM_PENDING_STATUS(dbStatus.status()) && (*responseIdentifiers != NULL)) {
            printf("Find SCP Response Identifiers:\n");
            (*responseIdentifiers)->print(COUT);
        }
        if (*stDetail) {
            printf("Status detail:\n");
            (*stDetail)->print(COUT);
        }
    }
}


/*
 * CVS Log
 * $Log$
 * Revision 1.1  2007/04/24 09:53:31  braindead
 * - updated DCMTK to version 3.5.4
 * - merged Gianluca's WIN32 changes
 *
 * Revision 1.1.1.1  2006/07/19 09:16:42  pipelka
 * - imported dcmtk354 sources
 *
 *
 * Revision 1.2  2005/12/08 15:47:04  meichel
 * Changed include path schema for all DCMTK header files
 *
 * Revision 1.1  2005/03/30 13:34:53  meichel
 * Initial release of module dcmqrdb that will replace module imagectn.
 *   It provides a clear interface between the Q/R DICOM front-end and the
 *   database back-end. The imagectn code has been re-factored into a minimal
 *   class structure.
 *
 *
 */
