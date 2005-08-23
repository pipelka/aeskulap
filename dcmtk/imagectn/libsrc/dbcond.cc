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
 *  Purpose: routines for generating DB condition messages.
 *    Module Prefix: DB_ 
 *
 *  Last Update:      $Author: braindead $
 *  Update Date:      $Date: 2005/08/23 19:32:09 $
 *  Source File:      $Source: /cvsroot/aeskulap/aeskulap/dcmtk/imagectn/libsrc/dbcond.cc,v $
 *  CVS/RCS Revision: $Revision: 1.1 $
 *  Status:           $State: Exp $
 *
 *  CVS/RCS Log at end of file
 *
 */

#include "osconfig.h"    /* make sure OS specific configuration is included first */
#include "dbcond.h"

const OFConditionConst IMAGECTNE_DB_ERROR(OFM_imagectn, IMAGECTNC_DB_ERROR, OF_error, "ImageCTN DB Error");

const OFCondition IMAGECTN_DB_ERROR(IMAGECTNE_DB_ERROR);

/*
** CVS Log
** $Log: dbcond.cc,v $
** Revision 1.1  2005/08/23 19:32:09  braindead
** - initial savannah import
**
** Revision 1.1  2005/06/26 19:26:14  pipelka
** - added dcmtk
**
** Revision 1.7  2002/11/27 13:27:53  meichel
** Adapted module imagectn to use of new header file ofstdinc.h
**
** Revision 1.6  2001/10/12 12:43:08  meichel
** Adapted imagectn to OFCondition based dcmnet module (supports strict mode).
**
** Revision 1.5  2001/06/01 15:51:27  meichel
** Updated copyright header
**
** Revision 1.4  2000/03/08 16:41:08  meichel
** Updated copyright header.
**
** Revision 1.3  1999/06/10 12:12:15  meichel
** Adapted imagectn to new command line option scheme.
**   Added support for Patient/Study Only Q/R model and C-GET (experimental).
**
** Revision 1.2  1996/09/27 08:46:48  hewett
** Enclosed system include files with BEGIN_EXTERN_C/END_EXTERN_C.
**
** Revision 1.1.1.1  1996/03/28 19:25:00  hewett
** Oldenburg Image CTN Software ported to use the dcmdata C++ toolkit.
**
*/
