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
 *  Author:  Ralph Meyer
 *
 *  Purpose: Configuration facility
 *    Module Prefix: CNF_ 
 *
 *  Last Update:      $Author: braindead $
 *  Update Date:      $Date: 2005/08/23 19:32:03 $
 *  Source File:      $Source: /cvsroot/aeskulap/aeskulap/dcmtk/imagectn/apps/cnfpriv.h,v $
 *  CVS/RCS Revision: $Revision: 1.1 $
 *  Status:           $State: Exp $
 *
 *  CVS/RCS Log at end of file
 *
 */

#ifndef CNF_CONFPRIV_H
#define CNF_CONFPRIV_H

#include "osconfig.h"    /* make sure OS specific configuration is included first */

/* includes */
#define INCLUDE_CSTDIO
#include "ofstdinc.h"
#include "ofcmdln.h"

/* typedefs */
typedef struct {
    int maxStudies;
    long        maxBytesPerStudy;
} Quota;

typedef struct {
    const char  *ApplicationTitle;
    const char  *HostName;
    int PortNumber;
} Peer;

/* Configuration Parameters (not yet complete) */

struct  AEEntry {
    const char          *ApplicationTitle;
    const char          *StorageArea;
    const char          *Access;        
    Quota               *StorageQuota;
    int                 noOfPeers;
    Peer                *Peers;
};

/* configuration structure */
struct  Configuration {
    /* global Configuration Parameters */
    const char          *ApplicationTitle;
    const char          *ApplicationContext;
    const char          *ImplementationClass;
    const char          *ImplementationVersion;
    const char          *NetworkType;
    int         NetworkTCPPort;
    OFCmdUnsignedInt MaxPDUSize;
    int         MaxAssociations;
    
   /* AETable Entries */
    int         noOfAEEntries;
    AEEntry     *AEEntries;
};

/* HostTable Entries */
struct  HostEntry {
    const char *SymbolicName;
    int noOfPeers;
    Peer *Peers;
};

/* Host and Vendor Table */
struct HostTable {
    int         noOfHostEntries;
    HostEntry   *HostEntries;
};

/*
 * global structures
 */
extern struct Configuration CNF_Config;
extern struct HostTable CNF_HETable;
extern struct HostTable CNF_VendorTable;

/* local functions */
void CNF_initConfigStruct();
void CNF_panic(const char *fmt, ...);
int CNF_checkForComplete();
int CNF_readConfigLines(FILE *cnffp);

#endif

/*
** CVS Log
** $Log: cnfpriv.h,v $
** Revision 1.1  2005/08/23 19:32:03  braindead
** - initial savannah import
**
** Revision 1.1  2005/06/26 19:26:14  pipelka
** - added dcmtk
**
** Revision 1.7  2002/11/29 07:18:14  wilkens
** Adapted ti utility to command line classes and added option '-xi'.
**
** Revision 1.6  2002/11/27 13:26:58  meichel
** Adapted module imagectn to use of new header file ofstdinc.h
**
** Revision 1.5  2001/11/12 14:54:18  meichel
** Removed all ctndisp related code from imagectn
**
** Revision 1.4  2001/06/01 15:51:16  meichel
** Updated copyright header
**
** Revision 1.3  2000/03/08 16:40:57  meichel
** Updated copyright header.
**
** Revision 1.2  1999/06/10 12:11:52  meichel
** Adapted imagectn to new command line option scheme.
**   Added support for Patient/Study Only Q/R model and C-GET (experimental).
**
** Revision 1.1.1.1  1996/03/28 19:24:59  hewett
** Oldenburg Image CTN Software ported to use the dcmdata C++ toolkit.
**
*/
