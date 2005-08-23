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
 *  Purpose: Service Class Executive (SCE) - ECHO Service Class Provider
 *
 *  Last Update:      $Author: braindead $
 *  Update Date:      $Date: 2005/08/23 19:32:03 $
 *  Source File:      $Source: /cvsroot/aeskulap/aeskulap/dcmtk/imagectn/apps/sceecho.cc,v $
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
#include "ofstdinc.h"

#include "dicom.h"
#include "imagectn.h"
#include "dimse.h"

#include "sceecho.h"

/*
 * C-ECHO SCP
 */

OFCondition
SCE_echoSCP(T_ASC_Association * assoc, T_DIMSE_C_EchoRQ * req,
	T_ASC_PresentationContextID presId)
{
    OFCondition cond = EC_Normal;

    if (opt_verbose) {
	printf("Received Echo SCP RQ: MsgID %d\n", 
		req->MessageID);
    }
    /* we send an echo response back */
    cond = DIMSE_sendEchoResponse(assoc, presId, 
        req, STATUS_Success, NULL);
    
    if (cond.bad()) {
	errmsg("echoSCP: Echo Response Failed:");
	DimseCondition::dump(cond);
    }
    return cond;
}

/*
** CVS Log
** $Log: sceecho.cc,v $
** Revision 1.1  2005/08/23 19:32:03  braindead
** - initial savannah import
**
** Revision 1.1  2005/06/26 19:26:14  pipelka
** - added dcmtk
**
** Revision 1.9  2002/11/27 13:27:00  meichel
** Adapted module imagectn to use of new header file ofstdinc.h
**
** Revision 1.8  2001/10/12 12:42:51  meichel
** Adapted imagectn to OFCondition based dcmnet module (supports strict mode).
**
** Revision 1.7  2001/06/01 15:51:18  meichel
** Updated copyright header
**
** Revision 1.6  2000/03/08 16:40:59  meichel
** Updated copyright header.
**
** Revision 1.5  2000/02/23 15:13:07  meichel
** Corrected macro for Borland C++ Builder 4 workaround.
**
** Revision 1.4  2000/02/01 11:43:39  meichel
** Avoiding to include <stdlib.h> as extern "C" on Borland C++ Builder 4,
**   workaround for bug in compiler header files.
**
** Revision 1.3  1999/06/10 12:11:56  meichel
** Adapted imagectn to new command line option scheme.
**   Added support for Patient/Study Only Q/R model and C-GET (experimental).
**
** Revision 1.2  1996/09/27 08:46:21  hewett
** Enclosed system include files with BEGIN_EXTERN_C/END_EXTERN_C.
**
** Revision 1.1.1.1  1996/03/28 19:24:59  hewett
** Oldenburg Image CTN Software ported to use the dcmdata C++ toolkit.
**
**
*/
