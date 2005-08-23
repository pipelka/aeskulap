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
 *  Purpose: Image CTN Common Constants, Types, Globals and Functions
 *
 *  Last Update:      $Author: braindead $
 *  Update Date:      $Date: 2005/08/23 19:32:03 $
 *  Source File:      $Source: /cvsroot/aeskulap/aeskulap/dcmtk/imagectn/apps/imagectn.h,v $
 *  CVS/RCS Revision: $Revision: 1.1 $
 *  Status:           $State: Exp $
 *
 *  CVS/RCS Log at end of file
 *
 */

#ifndef IMAGECTN_H
#define IMAGECTN_H

#include "osconfig.h"    /* make sure OS specific configuration is included first */

#include "dcxfer.h"
#include "dicom.h"
#include "cond.h"
#include "assoc.h"
#include "cnf.h"
#include "ofconapp.h"

/*
 * Application Conditions
 */
extern const OFCondition APP_INVALIDPEER;    /* invalid peer (for move operation) */


/*
 * Common Globals
 */

extern T_ASC_Network *net;
extern E_TransferSyntax  opt_networkTransferSyntax;
extern E_TransferSyntax  opt_writeTransferSyntax;
extern int         opt_verbose;
extern OFBool      opt_debug;
extern OFBool      opt_ignoreStoreData;
extern OFBool      opt_requireFindForMove;
extern OFBool      opt_restrictMoveToSameVendor;
extern OFBool      opt_restrictMoveToSameHost;
extern OFBool      opt_restrictMoveToSameAE;
extern OFBool      opt_bitPreserving;
extern OFBool      opt_useMetaheader;
extern E_GrpLenEncoding  opt_groupLength;
extern E_EncodingType    opt_sequenceType;
extern E_PaddingEncoding opt_paddingType;
extern OFCmdUnsignedInt  opt_filepad;
extern OFCmdUnsignedInt  opt_itempad; 

/* tables of abstract syntaxes which image ctn supports */
extern const char* nonStorageSyntaxes[];
extern int numberOfNonStorageSyntaxes;

/*
 * Common Function Definitions
 */

extern void errmsg(const char* msg, ...);

#endif

/*
** CVS Log
** $Log: imagectn.h,v $
** Revision 1.1  2005/08/23 19:32:03  braindead
** - initial savannah import
**
** Revision 1.1  2005/06/26 19:26:14  pipelka
** - added dcmtk
**
** Revision 1.10  2001/11/12 14:54:19  meichel
** Removed all ctndisp related code from imagectn
**
** Revision 1.9  2001/10/12 12:42:50  meichel
** Adapted imagectn to OFCondition based dcmnet module (supports strict mode).
**
** Revision 1.8  2001/06/01 15:51:17  meichel
** Updated copyright header
**
** Revision 1.7  2000/03/08 16:40:58  meichel
** Updated copyright header.
**
** Revision 1.6  1999/06/10 12:11:54  meichel
** Adapted imagectn to new command line option scheme.
**   Added support for Patient/Study Only Q/R model and C-GET (experimental).
**
** Revision 1.5  1997/08/26 14:17:19  hewett
** Added +B command line option to imagectn application.  Use of this option
** causes imagectn to bypass the dcmdata encode/decode routines when receiving
** images and write image data to disk exactly as received in a C-STORE
** command over the network.  This option does _not_ affect sending images.
**
** Revision 1.4  1997/07/21 08:59:43  andreas
** - Replace all boolean types (BOOLEAN, CTNBOOLEAN, DICOM_BOOL, BOOL)
**   with one unique boolean type OFBool.
**
** Revision 1.3  1996/09/24 15:52:34  hewett
** Now uses global table of Storage SOP Class UIDs (from dcuid.h).
** Also added preliminary support for the Macintosh environment (GUSI library).
**
** Revision 1.2  1996/04/22 10:20:11  hewett
** Added global variables for restricting move destinations.
**
** Revision 1.1.1.1  1996/03/28 19:24:59  hewett
** Oldenburg Image CTN Software ported to use the dcmdata C++ toolkit.
**
**
*/
