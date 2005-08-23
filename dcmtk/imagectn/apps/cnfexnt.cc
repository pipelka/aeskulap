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
 *  Source File:      $Source: /cvsroot/aeskulap/aeskulap/dcmtk/imagectn/apps/cnfexnt.cc,v $
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
#define INCLUDE_CCTYPE
#include "ofstdinc.h"

BEGIN_EXTERN_C
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
END_EXTERN_C

#include "cnfpriv.h"
#include "ofcmdln.h"
#include "cnf.h"


/*
**  Function name : CNF_aeTitlesForPeer
**
**  Description : searches in the host table for all AE titles
**  known for peer hostName.  Creates an array of string pointers
**  containing the known AE titles.  The AE titles contained
**  in the array are privately owned by the config facility (you
**  may not free them).  You may free the array when no longer needed.
**
**  Input Parameter: peer host name
**  Output Parameter: malloc'ed array of private string pointers.
**  Returns : number of entries in the malloced array.
**      0 if no entries found.
*/

int
CNF_aeTitlesForPeer(const char *hostName, const char *** aeTitleList)
{
    int n = 0;
    int i, j, k;
    const int chunkSize = 1;
    int maxAlloc = 0;
    const char *hname;
    const char *aetitle;
    int found;

    *aeTitleList = (const char**)malloc(chunkSize*sizeof(const char*));
    maxAlloc = chunkSize;

    /* collect up titles for peer, search in host table */
    for (i=0; i<CNF_HETable.noOfHostEntries; i++) {
        for (j=0; j<CNF_HETable.HostEntries[i].noOfPeers; j++) {
            hname = CNF_HETable.HostEntries[i].Peers[j].HostName;
            aetitle = CNF_HETable.HostEntries[i].Peers[j].ApplicationTitle;
#ifdef HAVE_PROTOTYPE_STRCASECMP
            if (strcasecmp(hname, hostName) == 0) {  /* DNS is not case-sensitive */
#elif defined(HAVE_PROTOTYPE__STRICMP)
            if (_stricmp(hname, hostName) == 0) {
#else
            if (strcmp(hname, hostName) == 0) {  /* fallback if case insensitive compare is unavailable */
#endif
                /* found an entry for peer host */
                /* make sure its not already in list */
                found = 0;
                for (k=0; !found && k<n; k++) {
                    found = (strcmp((*aeTitleList)[k], aetitle) == 0);
                }
                if (!found) {
                    if (n >= maxAlloc) {
                        *aeTitleList = (const char**)realloc(*aeTitleList,
                                      (maxAlloc + chunkSize)*sizeof(const char*));
                        maxAlloc += chunkSize;
                    }
                    (*aeTitleList)[n] = aetitle;

                    n++;
                }
            }
        }
    }
    /* collect up titles for peer, search in AE table */
    for (i=0; i<CNF_Config.noOfAEEntries; i++) {
        for (j=0; j<CNF_Config.AEEntries[i].noOfPeers; j++) {
            hname = CNF_Config.AEEntries[i].Peers[j].HostName;
            aetitle = CNF_Config.AEEntries[i].Peers[j].ApplicationTitle;

#ifdef HAVE_PROTOTYPE_STRCASECMP
            if (strcasecmp(hname, hostName) == 0) {  /* DNS is not case-sensitive */
#elif defined(HAVE_PROTOTYPE__STRICMP)
            if (_stricmp(hname, hostName) == 0) {
#else
            if (strcmp(hname, hostName) == 0) {  /* fallback if case insensitive compare is unavailable */
#endif
                /* found an entry for peer host */
                /* make sure its not already in list */
                found = 0;
                for (k=0; !found && k<n; k++) {
                    found = (strcmp((*aeTitleList)[k], aetitle) == 0);
                }
                if (!found) {
                    if (n >= maxAlloc) {
                        *aeTitleList = (const char**)realloc(*aeTitleList,
                                        (maxAlloc + chunkSize)*sizeof(const char*));
                        maxAlloc += chunkSize;
                    }
                    (*aeTitleList)[n] = aetitle;

                    n++;
                }
            }
        }
    }

    if (n == 0) {
        free(*aeTitleList);
        *aeTitleList = NULL;
    }
    return n;
}

/*
**  Function name : CNF_ctnTitles
**
**  Description : Creates an array of string pointers
**  containing the known AE titles for CTN storage areas.
**  The AE titles contained in the array are privately owned
**  by the config facility (you may not free them).  You may
**  free the array when no longer needed.
**
**  Output Parameter: malloc'ed array of private string pointers.
**  Returns : number of entries in the malloced array.
**      0 if no entries exist.
*/

int
CNF_ctnTitles(const char *** ctnTitleList)
{
    int i;
    int n = 0;

    n = CNF_Config.noOfAEEntries;
    *ctnTitleList = (const char**)malloc(n * sizeof(const char*));

    for (i=0; i<n; i++) {
        (*ctnTitleList)[i] = CNF_Config.AEEntries[i].ApplicationTitle;
    }
    return n;
}


/*
**  Function name : CNF_aeTitlesForSymbolicName
**
**  Description : searches in the host table for all AE titles
**  known for a symbolic name.  Creates an array of string pointers
**  containing the known AE titles.  The AE titles contained
**  in the array are privately owned by the config facility (you
**  may not free them).  You may free the array when no longer needed.
**
**  Input Parameter: symbolic name
**  Output Parameter: malloc'ed array of private string pointers.
**  Returns : number of entries in the malloced array.
**      0 if no entries found.
*/

int
CNF_aeTitlesForSymbolicName(const char *symbolicName, const char *** aeTitleList)
{
    int i = 0;
    int j = 0;
    int n = 0;

    for (i=0; i<CNF_HETable.noOfHostEntries; i++) {
        if (strcmp(symbolicName, CNF_HETable.HostEntries[i].SymbolicName)==0) {
            n = CNF_HETable.HostEntries[i].noOfPeers;
            *aeTitleList = (const char**)malloc(n * sizeof(const char*));
            for (j=0; j<n; j++) {
                (*aeTitleList)[j] =
                    CNF_HETable.HostEntries[i].Peers[j].ApplicationTitle;
            }
            return n;
        }
    }
    return 0;

}

const char*
CNF_vendorForPeerAETitle(const char *peerAETitle)
{
    int i = 0;
    int j = 0;

    for (i=0; i<CNF_VendorTable.noOfHostEntries; i++) {
        for (j=0; j<CNF_VendorTable.HostEntries[i].noOfPeers; j++) {
            if (strcmp(peerAETitle,
                CNF_VendorTable.HostEntries[i].Peers[j].ApplicationTitle)==0) {
                return CNF_VendorTable.HostEntries[i].SymbolicName;
            }
        }
    }
    return NULL;
}

int CNF_countCtnTitles()
{
    return CNF_Config.noOfAEEntries;
}

/*
** CVS Log
** $Log: cnfexnt.cc,v $
** Revision 1.1  2005/08/23 19:32:03  braindead
** - initial savannah import
**
** Revision 1.1  2005/06/26 19:26:14  pipelka
** - added dcmtk
**
** Revision 1.11  2002/11/29 07:18:13  wilkens
** Adapted ti utility to command line classes and added option '-xi'.
**
** Revision 1.10  2002/11/27 13:26:57  meichel
** Adapted module imagectn to use of new header file ofstdinc.h
**
** Revision 1.9  2001/06/01 15:51:16  meichel
** Updated copyright header
**
** Revision 1.8  2000/03/08 16:40:56  meichel
** Updated copyright header.
**
** Revision 1.7  2000/02/23 15:13:01  meichel
** Corrected macro for Borland C++ Builder 4 workaround.
**
** Revision 1.6  2000/02/01 11:43:38  meichel
** Avoiding to include <stdlib.h> as extern "C" on Borland C++ Builder 4,
**   workaround for bug in compiler header files.
**
** Revision 1.5  1999/06/10 12:11:50  meichel
** Adapted imagectn to new command line option scheme.
**   Added support for Patient/Study Only Q/R model and C-GET (experimental).
**
** Revision 1.4  1998/12/23 13:18:40  meichel
** Corrected spelling of preprocessor symbols for strcasecmp and _stricmp
**
** Revision 1.3  1998/12/23 13:10:31  meichel
** Modified imagectn so that comparison of hostnames
**   (i.e. hostnames reported by DNS and the hostnames in the imagectn config file)
**   are case insensitive. Since DNS is case insensitive, this seems appropriate.
**
** Revision 1.2  1996/09/27 08:46:18  hewett
** Enclosed system include files with BEGIN_EXTERN_C/END_EXTERN_C.
**
** Revision 1.1.1.1  1996/03/28 19:24:59  hewett
** Oldenburg Image CTN Software ported to use the dcmdata C++ toolkit.
**
*/
