/*
 *
 *  Copyright (C) 1993-2001, OFFIS
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
 *  Source File:      $Source: /cvsroot/aeskulap/aeskulap/dcmtk/imagectn/apps/tinet.h,v $
 *  CVS/RCS Revision: $Revision: 1.1 $
 *  Status:           $State: Exp $
 *
 *  CVS/RCS Log at end of file
 *
 */

#ifndef TINET_H
#define TINET_H

#include "osconfig.h"    /* make sure OS specific configuration is included first */

#include "ti.h"
#include "dicom.h"
#include "assoc.h"
#include "dimse.h"
#include "tiui.h"
#include "tinet.h"

OFBool 
TI_changeAssociation(TI_Config *conf);

OFBool
TI_attachAssociation(TI_Config *conf);

OFBool
TI_detatchAssociation(TI_Config *conf, OFBool abortFlag);

OFBool 
TI_sendEcho(TI_Config *conf);

OFBool
TI_storeImage(TI_Config *conf, char *sopClass, char *sopInstance, 
    char * imgFile);

typedef struct {
    TI_DBEntry *db;
    TI_StudyEntry *study;
    TI_SeriesEntry *series;
} TI_GenericCallbackStruct;

typedef OFBool (*TI_GenericEntryCallbackFunction)(
    TI_GenericCallbackStruct *cbstruct, DcmDataset *reply);

OFBool
TI_remoteFindQuery(TI_Config *conf, TI_DBEntry *db, DcmDataset *query,
    TI_GenericEntryCallbackFunction callbackFunction,
    TI_GenericCallbackStruct *callbackData);


#endif

/*
** CVS Log
** $Log: tinet.h,v $
** Revision 1.1  2005/08/23 19:32:03  braindead
** - initial savannah import
**
** Revision 1.1  2005/06/26 19:26:14  pipelka
** - added dcmtk
**
** Revision 1.6  2001/11/12 14:54:24  meichel
** Removed all ctndisp related code from imagectn
**
** Revision 1.5  2001/06/01 15:51:23  meichel
** Updated copyright header
**
** Revision 1.4  2000/03/08 16:41:03  meichel
** Updated copyright header.
**
** Revision 1.3  1999/06/10 12:12:04  meichel
** Adapted imagectn to new command line option scheme.
**   Added support for Patient/Study Only Q/R model and C-GET (experimental).
**
** Revision 1.2  1997/07/21 08:59:47  andreas
** - Replace all boolean types (BOOLEAN, CTNBOOLEAN, DICOM_BOOL, BOOL)
**   with one unique boolean type OFBool.
**
** Revision 1.1  1996/04/22 10:27:27  hewett
** Initial release.
**
**
*/
