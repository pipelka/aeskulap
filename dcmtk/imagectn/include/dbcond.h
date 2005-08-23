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
 *  Purpose: Routines for generating DB condition messages. Module Prefix: DB_ 
 *
 *  Last Update:      $Author: braindead $
 *  Update Date:      $Date: 2005/08/23 19:32:07 $
 *  Source File:      $Source: /cvsroot/aeskulap/aeskulap/dcmtk/imagectn/include/dbcond.h,v $
 *  CVS/RCS Revision: $Revision: 1.1 $
 *  Status:           $State: Exp $
 *
 *  CVS/RCS Log at end of file
 *
 */

#ifndef DBCOND_H
#define DBCOND_H

#include "osconfig.h"    /* make sure OS specific configuration is included first */
#include "dicom.h"
#include "cond.h"

// condition code constants used in the database module
const unsigned short IMAGECTNC_DB_ERROR = 0x001;

// condition constants used in the database module
extern const OFCondition IMAGECTN_DB_ERROR;  /* ImageCTN DB Error */

#endif

/*
** CVS Log
** $Log: dbcond.h,v $
** Revision 1.1  2005/08/23 19:32:07  braindead
** - initial savannah import
**
** Revision 1.1  2005/06/26 19:26:04  pipelka
** - added dcmtk
**
** Revision 1.5  2001/10/12 12:43:06  meichel
** Adapted imagectn to OFCondition based dcmnet module (supports strict mode).
**
** Revision 1.4  2001/06/01 15:51:25  meichel
** Updated copyright header
**
** Revision 1.3  2000/03/08 16:41:07  meichel
** Updated copyright header.
**
** Revision 1.2  1999/06/10 12:12:12  meichel
** Adapted imagectn to new command line option scheme.
**   Added support for Patient/Study Only Q/R model and C-GET (experimental).
**
** Revision 1.1  1998/12/22 15:11:26  vorwerk
** removed from libsrc and added in include
**
** Revision 1.1.1.1  1996/03/28 19:25:00  hewett
** Oldenburg Image CTN Software ported to use the dcmdata C++ toolkit.
**
*/
