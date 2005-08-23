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
 *  Author:  Ralph Meyer
 *
 *  Purpose: Configuration facility
 *    Module Prefix: CNF_
 *
 *  Last Update:      $Author: braindead $
 *  Update Date:      $Date: 2005/08/23 19:32:03 $
 *  Source File:      $Source: /cvsroot/aeskulap/aeskulap/dcmtk/imagectn/apps/cnf.h,v $
 *  CVS/RCS Revision: $Revision: 1.1 $
 *  Status:           $State: Exp $
 *
 *  CVS/RCS Log at end of file
 *
 */

#ifndef CNF_CONFIG_H
#define CNF_CONFIG_H

#include "osconfig.h"    /* make sure OS specific configuration is included first */
#include "ofcmdln.h"

int CNF_init(const char *ConfigurationFile);
void CNF_printConfig();

/* get global parameter values */
const char *CNF_getApplicationTitle();
const char *CNF_getApplicationContext();
const char *CNF_getImplementationClass();
const char *CNF_getImplementationVersion();
const char *CNF_getNetworkType();
int CNF_getNetworkTCPPort();
OFCmdUnsignedInt CNF_getMaxPDUSize();
int CNF_getMaxAssociations();

/* get Application Entity specific parameter values */
const char *CNF_getStorageArea(const char *AETitle);
const char *CNF_getAccess(const char *AETitle);
int CNF_getMaxStudies(const char *AETitle);
long CNF_getMaxBytesPerStudy(const char *AETitle);

/* get combined values */
int CNF_peerInAETitle(const char *calledAETitle, const char *callingAETitle, const char *HostName);
int CNF_peerForAETitle(const char *AETitle, const char **HostName, int *PortNumber);
int CNF_checkForSameVendor(const char *AETitle1, const char *AETitle2);
int CNF_HostNamesForVendor(const char *Vendor, const char ***HostNameArray);
const char* CNF_vendorForPeerAETitle(const char *peerAETitle);

int CNF_aeTitlesForPeer(const char *hostName, const char *** aeTitleList);
int CNF_ctnTitles(const char *** ctnTitleList);
int CNF_aeTitlesForSymbolicName(const char *symbolicName, const char ***aeTitleList);

int CNF_countCtnTitles();

#endif

/*
** CVS Log
** $Log: cnf.h,v $
** Revision 1.1  2005/08/23 19:32:03  braindead
** - initial savannah import
**
** Revision 1.1  2005/06/26 19:26:14  pipelka
** - added dcmtk
**
** Revision 1.6  2002/11/29 07:18:12  wilkens
** Adapted ti utility to command line classes and added option '-xi'.
**
** Revision 1.5  2001/11/12 14:54:18  meichel
** Removed all ctndisp related code from imagectn
**
** Revision 1.4  2001/06/01 15:51:15  meichel
** Updated copyright header
**
** Revision 1.3  2000/03/08 16:40:56  meichel
** Updated copyright header.
**
** Revision 1.2  1999/06/10 12:11:50  meichel
** Adapted imagectn to new command line option scheme.
**   Added support for Patient/Study Only Q/R model and C-GET (experimental).
**
** Revision 1.1.1.1  1996/03/28 19:24:59  hewett
** Oldenburg Image CTN Software ported to use the dcmdata C++ toolkit.
**
**
*/
