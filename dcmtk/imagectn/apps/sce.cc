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
 *  Purpose: Service Class Executive (SCE)
 *
 *  Last Update:      $Author: braindead $
 *  Update Date:      $Date: 2005/08/23 19:32:03 $
 *  Source File:      $Source: /cvsroot/aeskulap/aeskulap/dcmtk/imagectn/apps/sce.cc,v $
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

#include "sceecho.h"
#include "scestore.h"
#include "scefind.h"
#include "scemove.h"
#include "sceget.h"


/*
 *
 */

static OFCondition
SCE_dispatch(T_ASC_Association *assoc, OFBool opt_correctUIDPadding)
{
    OFCondition cond = EC_Normal;
    OFCondition dbcond = EC_Normal;
    T_DIMSE_Message msg;
    T_ASC_PresentationContextID presID;
    DB_Handle *dbHandle = NULL;
    char *myAE = NULL;
    
    myAE = assoc->params->DULparams.calledAPTitle;
    
    /* Create a database handle for this association */
    dbcond = DB_createHandle(CNF_getStorageArea(myAE), 
	CNF_getMaxStudies(myAE), 
	CNF_getMaxBytesPerStudy(myAE), &dbHandle); 
    if (dbcond.bad()) {
        errmsg("SCE_dispatch: cannot create DB Handle");
	cond = dbcond;	/* will abort association */
    }

    while (cond.good())
    {
	cond = DIMSE_receiveCommand(assoc, DIMSE_BLOCKING, 0, &presID, &msg, NULL);

	/* did peer release, abort, or do we have a valid message ? */
	if (cond.good())
	{
	    /* process command */
	    switch (msg.CommandField) {
	    case DIMSE_C_ECHO_RQ:
		cond = SCE_echoSCP(assoc, &msg.msg.CEchoRQ, presID);
		break;
	    case DIMSE_C_STORE_RQ:
		cond = SCE_storeSCP(assoc, &msg.msg.CStoreRQ, presID,
		    dbHandle, opt_correctUIDPadding);
		break;
	    case DIMSE_C_FIND_RQ:
		cond = SCE_findSCP(assoc, &msg.msg.CFindRQ, presID, 
		    dbHandle);
		break;
	    case DIMSE_C_MOVE_RQ:
		cond = SCE_moveSCP(assoc, &msg.msg.CMoveRQ, presID,
		    dbHandle);
		break;
	    case DIMSE_C_GET_RQ:
		cond = SCE_getSCP(assoc, &msg.msg.CGetRQ, presID,
		    dbHandle);
		break;
	    case DIMSE_C_CANCEL_RQ:
		/* This is a late cancel request, just ignore it */
		if (opt_verbose) {
		    printf("SCE_dispatch: late C-CANCEL-RQ, ignoring\n");
		}
		break;
	    default:
		/* we cannot handle this kind of message */
		cond = DIMSE_BADCOMMANDTYPE;
		errmsg("Cannot handle command: 0x%x\n", 
			(unsigned)msg.CommandField);
	        /* the condition will be returned, the caller
	         * will abort the assosiation.
	         */
	    }
	}
	else if ((cond == DUL_PEERREQUESTEDRELEASE)||(cond == DUL_PEERABORTEDASSOCIATION))
	{
	    /* association gone */
        }
        else
	{
	    /* the condition will be returned, the caller
	     * will abort the assosiation.
	     */
        }

    }

    /* Association done, destroy db handle */
    dbcond = DB_destroyHandle(&dbHandle);
    if (dbcond.bad()) {
        errmsg("SCE_dispatch: cannot destroy DB Handle");
    }

    return cond;
}

OFCondition
SCE_handleAssociation(T_ASC_Association * assoc, OFBool opt_correctUIDPadding)
{
    OFCondition           cond = EC_Normal;
    DIC_NODENAME	peerHostName;
    DIC_AE		peerAETitle;
    DIC_AE		myAETitle;

    ASC_getPresentationAddresses(assoc->params, peerHostName, NULL);
    ASC_getAPTitles(assoc->params, peerAETitle, myAETitle, NULL);
    
 /* now do the real work */
    cond = SCE_dispatch(assoc, opt_correctUIDPadding);

 /* clean up on association termination */
    if (cond == DUL_PEERREQUESTEDRELEASE) {
	if (opt_verbose)
	    printf("Association Release\n");
	cond = ASC_acknowledgeRelease(assoc);
	ASC_dropSCPAssociation(assoc);
    } else if (cond == DUL_PEERABORTEDASSOCIATION) {
	if (opt_verbose)
	    printf("Association Aborted\n");
    } else {
	errmsg("DIMSE Failure (aborting association):\n");
	DimseCondition::dump(cond);
    /* some kind of error so abort the association */
	cond = ASC_abortAssociation(assoc);
    }

    cond = ASC_dropAssociation(assoc);
    if (cond.bad()) {
	fprintf(stderr, "Cannot Drop Association:\n");
	DimseCondition::dump(cond);
    }
    cond = ASC_destroyAssociation(&assoc);
    if (cond.bad()) {
	fprintf(stderr, "Cannot Destroy Association:\n");
	DimseCondition::dump(cond);
    }

    return cond;
}


/*
** CVS Log
** $Log: sce.cc,v $
** Revision 1.1  2005/08/23 19:32:03  braindead
** - initial savannah import
**
** Revision 1.1  2005/06/26 19:26:14  pipelka
** - added dcmtk
**
** Revision 1.13  2002/11/27 13:26:59  meichel
** Adapted module imagectn to use of new header file ofstdinc.h
**
** Revision 1.12  2002/11/25 18:01:14  meichel
** Converted compile time option to leniently handle space padded UIDs
**   in the Storage Service Class into command line / config file option.
**
** Revision 1.11  2001/11/12 14:54:20  meichel
** Removed all ctndisp related code from imagectn
**
** Revision 1.10  2001/10/12 12:42:51  meichel
** Adapted imagectn to OFCondition based dcmnet module (supports strict mode).
**
** Revision 1.9  2001/06/01 15:51:18  meichel
** Updated copyright header
**
** Revision 1.8  2000/03/08 16:40:58  meichel
** Updated copyright header.
**
** Revision 1.7  2000/02/23 15:13:05  meichel
** Corrected macro for Borland C++ Builder 4 workaround.
**
** Revision 1.6  2000/02/01 11:43:39  meichel
** Avoiding to include <stdlib.h> as extern "C" on Borland C++ Builder 4,
**   workaround for bug in compiler header files.
**
** Revision 1.5  1999/06/10 12:11:54  meichel
** Adapted imagectn to new command line option scheme.
**   Added support for Patient/Study Only Q/R model and C-GET (experimental).
**
** Revision 1.4  1997/08/05 07:38:19  andreas
** Corrected error in DUL finite state machine
** SCPs shall close sockets after the SCU have closed the socket in
** a normal association release. Therfore, an ARTIM timer is described
** in DICOM part 8 that is not implemented correctly in the
** DUL. Since the whole DUL finite state machine is affected, we
** decided to solve the proble outside the fsm. Now it is necessary to call the
** ASC_DropSCPAssociation() after the calling ASC_acknowledgeRelease().
**
** Revision 1.3  1997/07/21 08:59:43  andreas
** - Replace all boolean types (BOOLEAN, CTNBOOLEAN, DICOM_BOOL, BOOL)
**   with one unique boolean type OFBool.
**
** Revision 1.2  1996/09/27 08:46:20  hewett
** Enclosed system include files with BEGIN_EXTERN_C/END_EXTERN_C.
**
** Revision 1.1.1.1  1996/03/28 19:24:59  hewett
** Oldenburg Image CTN Software ported to use the dcmdata C++ toolkit.
**
**
*/
