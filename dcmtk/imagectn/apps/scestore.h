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
 *  Purpose: Service Class Executive (SCE) - Store Service Class Provider
 *
 *  Last Update:      $Author: braindead $
 *  Update Date:      $Date: 2005/08/23 19:32:03 $
 *  Source File:      $Source: /cvsroot/aeskulap/aeskulap/dcmtk/imagectn/apps/scestore.h,v $
 *  CVS/RCS Revision: $Revision: 1.1 $
 *  Status:           $State: Exp $
 *
 *  CVS/RCS Log at end of file
 *
 */

#ifndef SCE_STORE_H
#define SCE_STORE_H

OFCondition
SCE_storeSCP(T_ASC_Association * assoc, T_DIMSE_C_StoreRQ * req,
	     T_ASC_PresentationContextID presId,
	     DB_Handle *dbHandle,
             OFBool opt_correctUIDPadding);

#endif

/*
 * $Log: scestore.h,v $
 * Revision 1.1  2005/08/23 19:32:03  braindead
 * - initial savannah import
 *
 * Revision 1.1  2005/06/26 19:26:14  pipelka
 * - added dcmtk
 *
 * Revision 1.6  2002/11/25 18:01:15  meichel
 * Converted compile time option to leniently handle space padded UIDs
 *   in the Storage Service Class into command line / config file option.
 *
 * Revision 1.5  2001/10/12 12:42:55  meichel
 * Adapted imagectn to OFCondition based dcmnet module (supports strict mode).
 *
 * Revision 1.4  2001/06/01 15:51:21  meichel
 * Updated copyright header
 *
 * Revision 1.3  2000/03/08 16:41:02  meichel
 * Updated copyright header.
 *
 * Revision 1.2  1999/06/10 12:12:01  meichel
 * Adapted imagectn to new command line option scheme.
 *   Added support for Patient/Study Only Q/R model and C-GET (experimental).
 *
 * Revision 1.1.1.1  1996/03/28 19:24:59  hewett
 * Oldenburg Image CTN Software ported to use the dcmdata C++ toolkit.
 *
 * Revision 2.1  1995/02/02 12:49:24  hewett
 * Corrected spelling of Ossietzky
 *
 * Revision 2.0  1994/11/17  09:44:07  meichel
 * Version 2.0 for RSNA 1994 demonstration.
 * New features: Metaheader support, new SOP classes, lots of bugs fixed.
 *
 * Revision 1.3  1994/03/17  09:27:45  hewett
 * Updated copyright notice.
 *
 * Revision 1.2  1993/10/14  10:46:06  hewett
 * Added Euro Header
 *
 * Revision 1.1  1993/08/30  09:48:32  hewett
 * Initial Revision
 *
 */
