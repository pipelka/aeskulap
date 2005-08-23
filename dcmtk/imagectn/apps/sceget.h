/*
 *
 *  Copyright (C) 1993-2001, OFFIS
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
 *    Adapted from scemove.h - Copyright (C) 1993/1994, OFFIS, Oldenburg University and CERIUM
 *
 * Last Update:		$Author: braindead $
 * Update Date:		$Date: 2005/08/23 19:32:03 $
 * Source File:		$Source: /cvsroot/aeskulap/aeskulap/dcmtk/imagectn/apps/sceget.h,v $
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

#ifndef SCE_GET_H
#define SCE_GET_H

#include "osconfig.h"    /* make sure OS specific configuration is included first */

OFCondition
SCE_getSCP(T_ASC_Association * assoc, T_DIMSE_C_GetRQ * request,
	T_ASC_PresentationContextID presID, DB_Handle *dbHandle);

#endif

/*
** CVS Log
** $Log: sceget.h,v $
** Revision 1.1  2005/08/23 19:32:03  braindead
** - initial savannah import
**
** Revision 1.1  2005/06/26 19:26:14  pipelka
** - added dcmtk
**
** Revision 1.4  2001/10/12 12:42:53  meichel
** Adapted imagectn to OFCondition based dcmnet module (supports strict mode).
**
** Revision 1.3  2001/06/01 15:51:20  meichel
** Updated copyright header
**
** Revision 1.2  2000/03/08 16:41:00  meichel
** Updated copyright header.
**
** Revision 1.1  1999/06/10 12:15:42  meichel
** Adapted imagectn to new command line option scheme.
**   Added support for Patient/Study Only Q/R model and C-GET (experimental).
**
**
*/
