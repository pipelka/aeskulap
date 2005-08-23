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
 *    The Configuration Module provides a common facility for parsing
 *    configuration files and encapsulates the actual configuration
 *    parameters.
 *
 *  Last Update:      $Author: braindead $
 *  Update Date:      $Date: 2005/08/23 19:32:03 $
 *  Source File:      $Source: /cvsroot/aeskulap/aeskulap/dcmtk/imagectn/apps/cnf.cc,v $
 *  CVS/RCS Revision: $Revision: 1.1 $
 *  Status:           $State: Exp $
 *
 *  CVS/RCS Log at end of file
 *
 */

#include "osconfig.h"    /* make sure OS specific configuration is included first */

#define INCLUDE_CSTDLIB
#define INCLUDE_CSTRING
#include "ofstdinc.h"

#include "cnfpriv.h"
#include "ofcmdln.h"
#include "cnf.h"

/*
**  Function name : CNF_init
**
**  Description : read configuration file and initialize the
**  intern configuration structure
**  Input : configuration file name
**  Return : 1 - ok
**     0 - error
*/
int CNF_init(const char *ConfigurationFile)
{
   int  error = 0;        /* error flag */
   FILE *cnffp;         /* configuration file pointer */

   if ((cnffp = fopen(ConfigurationFile, "r")) == NULL) {
      CNF_panic("Unable to open configuration file \"%s\"", ConfigurationFile);
      return(0);
   }

   CNF_initConfigStruct();

   if (!CNF_readConfigLines(cnffp)) {
      CNF_panic("Reading configuration file \"%s\" with errors", ConfigurationFile);
      error = 1;
   }

   fclose(cnffp);

   if (!CNF_checkForComplete()) {
      CNF_panic("Configuration file \"%s\" not complete", ConfigurationFile);
      error = 1;
   }

   return(error ? 0 : 1);
}


/*
**  Function name : CNF_printConfig
**
**  Description : printf contents of configuration stucture
**  to stdout
*/
void CNF_printConfig()
{
   int i,j;

   printf("\nHostTable: %d\n", CNF_HETable.noOfHostEntries);
   for(i = 0; i < CNF_HETable.noOfHostEntries; i++) {
      printf("%s %d\n", CNF_HETable.HostEntries[i].SymbolicName, CNF_HETable.HostEntries[i].noOfPeers);
      for(j = 0; j < CNF_HETable.HostEntries[i].noOfPeers; j++) {
         printf("%s %s %d\n", CNF_HETable.HostEntries[i].Peers[j].ApplicationTitle,
            CNF_HETable.HostEntries[i].Peers[j].HostName, CNF_HETable.HostEntries[i].Peers[j].PortNumber);
      }
   }
   printf("\nVendorTable: %d\n", CNF_VendorTable.noOfHostEntries);
   for(i = 0; i < CNF_VendorTable.noOfHostEntries; i++) {
      printf("%s %d\n", CNF_VendorTable.HostEntries[i].SymbolicName, CNF_VendorTable.HostEntries[i].noOfPeers);
      for(j = 0; j < CNF_VendorTable.HostEntries[i].noOfPeers; j++) {
         printf("%s %s %d\n", CNF_VendorTable.HostEntries[i].Peers[j].ApplicationTitle,
            CNF_VendorTable.HostEntries[i].Peers[j].HostName, CNF_VendorTable.HostEntries[i].Peers[j].PortNumber);
      }
   }
   printf("\nGlobal Parameters:\n%s\n%s\n%s\n%s\n%s\n%d\n%lu\n%d\n",
      CNF_Config.ApplicationTitle, CNF_Config.ApplicationContext, CNF_Config.ImplementationClass,
      CNF_Config.ImplementationVersion, CNF_Config.NetworkType, CNF_Config.NetworkTCPPort, CNF_Config.MaxPDUSize,
      CNF_Config.MaxAssociations);
   printf("\nAEEntries: %d\n", CNF_Config.noOfAEEntries);
   for(i = 0; i < CNF_Config.noOfAEEntries; i++) {
      printf("%s\n%s\n%s\n%d, %ld\n", CNF_Config.AEEntries[i].ApplicationTitle, CNF_Config.AEEntries[i].StorageArea,
         CNF_Config.AEEntries[i].Access, CNF_Config.AEEntries[i].StorageQuota->maxStudies,
         CNF_Config.AEEntries[i].StorageQuota->maxBytesPerStudy);
      if (CNF_Config.AEEntries[i].noOfPeers == -1)
         printf("Peers: ANY\n");
      else {
         printf("Peers: %d\n", CNF_Config.AEEntries[i].noOfPeers);
         for(j = 0; j < CNF_Config.AEEntries[i].noOfPeers; j++) {
            printf("%s %s %d\n", CNF_Config.AEEntries[i].Peers[j].ApplicationTitle,
               CNF_Config.AEEntries[i].Peers[j].HostName, CNF_Config.AEEntries[i].Peers[j].PortNumber);
         }
      }
      printf("----------------------------------\n");
   }
}


/*
**  Function name : CNF_getApplicationTitle
**
**  Description : get Application Title
**  Input :
**  Return : Application Title
*/
const char *CNF_getApplicationTitle()
{
   return(CNF_Config.ApplicationTitle);
}


/*
**  Function name : CNF_getApplicationContext
**
**  Description : get Application Context
**  Input :
**  Return : Application Context
*/
const char *CNF_getApplicationContext()
{
   return(CNF_Config.ApplicationContext);
}


/*
**  Function name : CNF_getImplementationClass
**
**  Description : get Implementation Class
**  Input :
**  Return : Implementation Class
*/
const char *CNF_getImplementationClass()
{
   return(CNF_Config.ImplementationClass);
}


/*
**  Function name : CNF_getImplementationVersion
**
**  Description : get Implementation Version
**  Input :
**  Return : Implementation Version
*/
const char *CNF_getImplementationVersion()
{
   return(CNF_Config.ImplementationVersion);
}


/*
**  Function name : CNF_getNetworkType
**
**  Description : get Network Type
**  Input :
**  Return : Network Type
*/
const char *CNF_getNetworkType()
{
   return(CNF_Config.NetworkType);
}


/*
**  Function name : CNF_getNetworkTCPPort
**
**  Description : get Network TCP Port
**  Input :
**  Return : Network TCP Port
*/
int CNF_getNetworkTCPPort()
{
   return(CNF_Config.NetworkTCPPort);
}


/*
**  Function name : CNF_getMaxPDUSize
**
**  Description : get Max PDU Size
**  Input :
**  Return : Max PDU Size
*/
OFCmdUnsignedInt CNF_getMaxPDUSize()
{
   return(CNF_Config.MaxPDUSize);
}


/*
**  Function name : CNF_getMaxAssociations
**
**  Description : get Max Associations
**  Input :
**  Return : Max Associations
*/
int CNF_getMaxAssociations()
{
   return(CNF_Config.MaxAssociations);
}


/*
**  Function name : CNF_getStorageArea
**
**  Description : get Storage Area for AETitle
**  Input : AETitle
**  Return : Storage Area
*/
const char *CNF_getStorageArea(const char *AETitle)
{
   int  i;

   for(i = 0; i < CNF_Config.noOfAEEntries; i++) {
      if (!strcmp(AETitle, CNF_Config.AEEntries[i].ApplicationTitle))
         return(CNF_Config.AEEntries[i].StorageArea);
   }
   return(NULL);        /* AETitle not found */
}

/*
**  Function name : CNF_getAccess
**
**  Description : get Access mode
**  Input : AETitle
**  Return : Access mode
*/
const char *CNF_getAccess(const char *AETitle)
{
   int  i;

   for(i = 0; i < CNF_Config.noOfAEEntries; i++) {
      if (!strcmp(AETitle, CNF_Config.AEEntries[i].ApplicationTitle))
         return(CNF_Config.AEEntries[i].Access);
   }
   return(NULL);        /* AETitle not found */
}


/*
**  Function name : CNF_getMaxStudies
**
**  Description : get Number of Maximal Studies
**  Input : AETitle
**  Return : Number of Maximal Studies
*/
int CNF_getMaxStudies(const char *AETitle)
{
   int  i;

   for(i = 0; i < CNF_Config.noOfAEEntries; i++) {
      if (!strcmp(AETitle, CNF_Config.AEEntries[i].ApplicationTitle))
         return(CNF_Config.AEEntries[i].StorageQuota->maxStudies);
   }
   return(0);       /* AETitle not found */
}

/*
**  Function name : CNF_getMaxBytesPerStudy
**
**  Description : get Number of maximal Bytes per Study
**  Input : AETitle
**  Return : Number of maximal Bytes per Study
*/
long CNF_getMaxBytesPerStudy(const char *AETitle)
{
   int  i;

   for(i = 0; i < CNF_Config.noOfAEEntries; i++) {
      if (!strcmp(AETitle, CNF_Config.AEEntries[i].ApplicationTitle))
         return(CNF_Config.AEEntries[i].StorageQuota->maxBytesPerStudy);
   }
   return(0);       /* AETitle not found */
}


/*
**  Function name : CNF_peerInAETitle
**
**  Description : check if there is an peer with calling AETitle
**  on HostName
**  Input : called AETitle, calling AETitle, Host Name
**  Return : 1 -- yes
**     0 -- no
*/
int CNF_peerInAETitle(const char *calledAETitle, const char *callingAETitle, const char *HostName)
{
   int  i,
    j;

   for(i = 0; i < CNF_Config.noOfAEEntries; i++) {
      if (!strcmp(calledAETitle, CNF_Config.AEEntries[i].ApplicationTitle)) {
         if (CNF_Config.AEEntries[i].noOfPeers == -1) /* ANY Peer allowed */
            return(1);
         for(j = 0; j < CNF_Config.AEEntries[i].noOfPeers; j++) {
            if (!strcmp(callingAETitle, CNF_Config.AEEntries[i].Peers[j].ApplicationTitle) &&
#ifdef HAVE_PROTOTYPE_STRCASECMP
                /* DNS is not case-sensitive */
                !strcasecmp(HostName, CNF_Config.AEEntries[i].Peers[j].HostName))
#elif defined(HAVE_PROTOTYPE__STRICMP)
                !_stricmp(HostName, CNF_Config.AEEntries[i].Peers[j].HostName))
#else
                /* fallback solution is to do case sensitive comparison on systems
                   which do not implement strcasecmp or _stricmp */
                !strcmp(HostName, CNF_Config.AEEntries[i].Peers[j].HostName))
#endif
               return(1);       /* Peer found */
         }
      }
   }
   return(0);           /* Peer not found */
}


/*
**  Function name : CNF_peerForAETitle
**
**  Description : search for peer with AETitle
**  Input : AETitle
**  Ouput : Host Name, Port Number
**  Return : 1 - found in AETable
**     2 - found in HostTable
**     0 - not found
*/
int CNF_peerForAETitle(const char *AETitle, const char **HostName, int *PortNumber)
{
   int  i,
    j;

   for(i = 0; i < CNF_Config.noOfAEEntries; i++) {
      for(j = 0; j < CNF_Config.AEEntries[i].noOfPeers; j++) {
         if (!strcmp(AETitle, CNF_Config.AEEntries[i].Peers[j].ApplicationTitle)) {
            *HostName = CNF_Config.AEEntries[i].Peers[j].HostName;
            *PortNumber = CNF_Config.AEEntries[i].Peers[j].PortNumber;
            return(1);        /* Peer found in AETable */
         }
      }
   }

   for(i = 0; i < CNF_HETable.noOfHostEntries; i++) {
      for(j = 0; j < CNF_HETable.HostEntries[i].noOfPeers; j++) {
         if (!strcmp(AETitle, CNF_HETable.HostEntries[i].Peers[j].ApplicationTitle)) {
            *HostName = CNF_HETable.HostEntries[i].Peers[j].HostName;
            *PortNumber = CNF_HETable.HostEntries[i].Peers[j].PortNumber;
            return(2);        /* Peer found in HostTable */
         }
      }
   }

   return(0);         /* Peer not found */
}



/*
**  Function name : CNF_checkForSameVendor
**
**  Description : check if given AETitles exist in same
**  Vendor Group
**  Input : two AETitles
**  Return : 1 - same group
**     0 - else
*/
int CNF_checkForSameVendor(const char *AETitle1, const char *AETitle2)
{
   int  i,
    j,
    k,
    found = 0;

   for(i = 0; i < CNF_VendorTable.noOfHostEntries; i++) {
      for(j = 0; j < CNF_VendorTable.HostEntries[i].noOfPeers; j++) {
         if (!strcmp(AETitle1, CNF_VendorTable.HostEntries[i].Peers[j].ApplicationTitle)) {
            for(k = 0; k < CNF_VendorTable.HostEntries[i].noOfPeers; k++) {
               if (!strcmp(AETitle2, CNF_VendorTable.HostEntries[i].Peers[k].ApplicationTitle))
                  found = 1;
            }
         }
      }
   }

   return(found);
}



/*
**  Function name : CNF_HostNamesForVendor
**
**  Description : Creates an array of string pointers
**  containing the kown Host Names for given Vendor Name.
**  The Host Names contained in the array are privately owned
**  by the config facility (you may not free them). You may
**  free the array when no longer needed.
**  Input : Vendor Name
**  Ouput : array of string pointers
**  Return : number of entries in array
**     0 if no entries exist
*/
int CNF_HostNamesForVendor(const char *Vendor, const char ***HostNameArray)
{
   int  i, j,
    found = 0;

   for(i = 0; i < CNF_VendorTable.noOfHostEntries; i++) {
      if (!strcmp(CNF_VendorTable.HostEntries[i].SymbolicName, Vendor)) {
         found = 1;
         break;
      }
   }

   if (!found)
      return(0);

   if ((*HostNameArray = (const char**)malloc(CNF_VendorTable.HostEntries[i].noOfPeers * sizeof(const char *))) == NULL) {
      CNF_panic("Memory allocation A (%d)", CNF_VendorTable.HostEntries[i].noOfPeers);
      return(0);
   }
   for(j = 0; j < CNF_VendorTable.HostEntries[i].noOfPeers; j++)
      (*HostNameArray)[j] = CNF_VendorTable.HostEntries[i].Peers[j].HostName;

   return(CNF_VendorTable.HostEntries[i].noOfPeers);
}

/*
** CVS Log
** $Log: cnf.cc,v $
** Revision 1.1  2005/08/23 19:32:03  braindead
** - initial savannah import
**
** Revision 1.1  2005/06/26 19:26:14  pipelka
** - added dcmtk
**
** Revision 1.14  2002/11/29 07:18:11  wilkens
** Adapted ti utility to command line classes and added option '-xi'.
**
** Revision 1.13  2002/11/27 13:26:57  meichel
** Adapted module imagectn to use of new header file ofstdinc.h
**
** Revision 1.12  2001/11/12 14:54:17  meichel
** Removed all ctndisp related code from imagectn
**
** Revision 1.11  2001/06/01 15:51:15  meichel
** Updated copyright header
**
** Revision 1.10  2000/03/08 16:40:55  meichel
** Updated copyright header.
**
** Revision 1.9  2000/02/23 15:13:00  meichel
** Corrected macro for Borland C++ Builder 4 workaround.
**
** Revision 1.8  2000/02/01 11:43:37  meichel
** Avoiding to include <stdlib.h> as extern "C" on Borland C++ Builder 4,
**   workaround for bug in compiler header files.
**
** Revision 1.7  1999/06/10 12:11:49  meichel
** Adapted imagectn to new command line option scheme.
**   Added support for Patient/Study Only Q/R model and C-GET (experimental).
**
** Revision 1.6  1998/12/23 13:18:39  meichel
** Corrected spelling of preprocessor symbols for strcasecmp and _stricmp
**
** Revision 1.5  1998/12/23 13:10:30  meichel
** Modified imagectn so that comparison of hostnames
**   (i.e. hostnames reported by DNS and the hostnames in the imagectn config file)
**   are case insensitive. Since DNS is case insensitive, this seems appropriate.
**
** Revision 1.4  1996/09/27 08:46:17  hewett
** Enclosed system include files with BEGIN_EXTERN_C/END_EXTERN_C.
**
** Revision 1.3  1996/06/10 08:54:06  meichel
** Added missing #include <string.h>.
**
** Revision 1.2  1996/04/22 10:16:40  hewett
** Added cast for malloc to stop compiler warnings.
**
** Revision 1.1.1.1  1996/03/28 19:24:59  hewett
** Oldenburg Image CTN Software ported to use the dcmdata C++ toolkit.
**
**
*/
