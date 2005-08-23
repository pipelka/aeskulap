/*
 *
 *  Copyright (C) 1993-2004, OFFIS
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
 *  Source File:      $Source: /cvsroot/aeskulap/aeskulap/dcmtk/imagectn/apps/cnfpriv.cc,v $
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

#include "cnfpriv.h"        /* local header file */


/* forward declarations */
static int readHostTable(FILE *cnffp, int *lineno);
static int readVendorTable(FILE *cnffp, int *lineno);
static int readAETable(FILE *cnffp, int *lineno);
static Quota *parseQuota(char **valuehandle);
static Peer *parsePeers(char **valuehandle, int *peers);
static Peer *readPeerList(char **valuehandle, int *peers);
static char *skipmnemonic(char *rcline);
static int isgap(char gap);
static int isquote(char quote);
static char *parsevalues(char **valuehandle);
static long quota(const char *value);


/* global structures */
struct  Configuration CNF_Config;   /* configuration file contents */
struct  HostTable CNF_HETable;      /* HostEntries Table */
struct  HostTable CNF_VendorTable;  /* Vendor Table */


/*
**  Function name : CNF_initConfigStruct
**
**  Description : initialize configuration storage structure
*/
void CNF_initConfigStruct()
{
   CNF_Config.ApplicationTitle = "CTN";
   CNF_Config.ApplicationContext = "1.2.840.10008.3.1.1.1";
   CNF_Config.ImplementationClass = "1.2.3.4.5.6.7.8.9.10";
   CNF_Config.ImplementationVersion = "CEN-DICOM-1.0";
   CNF_Config.NetworkType = "tcp";
   CNF_Config.NetworkTCPPort = 104;
   CNF_Config.MaxPDUSize = 16384;
   CNF_Config.MaxAssociations = 16;
   CNF_Config.noOfAEEntries = 0;
   CNF_HETable.noOfHostEntries = 0;
   CNF_VendorTable.noOfHostEntries = 0;
}


/*
**  Function name : CNF_panic
**
**  Description : print a panic message to stderr
**  Input : variable
*/
void CNF_panic(const char *fmt, ...)
{
   va_list  ap;

   va_start(ap, fmt);
   fprintf(stderr, "CONFIG Error: ");
   vfprintf(stderr, fmt, ap);
   fprintf(stderr, "!\n");
   va_end(ap);
}


/*
**  Function name : CNF_checkForComplete
**
**  Description : check for complete configuration file
**  Return : 1 - ok
**     0 - something is missing
*/
int CNF_checkForComplete()
{
   return(1);
}


/*
**  Function name : CNF_readConfigLines
**
**  Description : read configuration file line by line
**  Input : configuration file pointer
**  Return : 1 - ok
**     0 - error
*/
int CNF_readConfigLines(FILE *cnffp)
{
   int  lineno = 0,       /* line counter */
        error = 0;        /* error flag */
   char rcline[512],      /* line in configuration file */
        mnemonic[64],     /* mnemonic in line */
        value[256],       /* parameter value */
        *valueptr;        /* pointer to value list */

   while (!feof(cnffp)) {
      fgets(rcline, sizeof(rcline), cnffp); /* read line in configuration file */
      lineno++;
      if (feof(cnffp)) continue;
      if (rcline[0] == '#' || rcline[0] == 10 || rcline[0] == 13)
         continue;        /* comment or blank line */
      sscanf(rcline, "%s", mnemonic);
      valueptr = skipmnemonic(rcline);

      if (!strcmp("ApplicationTitle", mnemonic)) {
         CNF_Config.ApplicationTitle = parsevalues(&valueptr);
      }
      else if (!strcmp("ApplicationContext", mnemonic)) {
         CNF_Config.ApplicationContext = parsevalues(&valueptr);
      }
      else if (!strcmp("ImplementationClass", mnemonic)) {
         CNF_Config.ImplementationClass = parsevalues(&valueptr);
      }
      else if (!strcmp("ImplementationVersion", mnemonic)) {
         CNF_Config.ImplementationVersion = parsevalues(&valueptr);
      }
      else if (!strcmp("NetworkType", mnemonic)) {
         CNF_Config.NetworkType = parsevalues(&valueptr);
      }
      else if (!strcmp("NetworkTCPPort", mnemonic)) {
         sscanf(valueptr, "%d", &CNF_Config.NetworkTCPPort);
      }
      else if (!strcmp("MaxPDUSize", mnemonic)) {
         sscanf(valueptr, "%lu", &CNF_Config.MaxPDUSize);
      }
      else if (!strcmp("MaxAssociations", mnemonic)) {
         sscanf(valueptr, "%d", &CNF_Config.MaxAssociations);
      }
      else if (!strcmp("Display", mnemonic))
      {
        // ignore this entry which was needed for ctndisp
      }
      else if (!strcmp("DisplayPort", mnemonic))
      {
        // ignore this entry which was needed for ctndisp
      }
      else if (!strcmp("HostTable", mnemonic)) {
         sscanf(valueptr, "%s", value);
         if (!strcmp("BEGIN", value)) {
            if (!readHostTable(cnffp, &lineno))
               error = 1;
         }
         else if (!strcmp("END", value)) {
            CNF_panic("No \"HostTable BEGIN\" before END in configuration file, line %d", lineno);
            error = 1;
         }
         else {
            CNF_panic("Unknown HostTable status \"%s\" in configuartion file, line %d", value, lineno);
            error = 1;
         }
      }
      else if (!strcmp("VendorTable", mnemonic)) {
         sscanf(valueptr, "%s", value);
         if (!strcmp("BEGIN", value)) {
            if (!readVendorTable(cnffp, &lineno))
               error = 1;
         }
         else if (!strcmp("END", value)) {
            CNF_panic("No \"VendorTable BEGIN\" before END in configuration file, line %d", lineno);
            error = 1;
         }
         else {
            CNF_panic("Unknown VendorTable status \"%s\" in configuartion file, line %d", value, lineno);
            error = 1;
         }
      }
      else if (!strcmp("AETable", mnemonic)) {
         sscanf(valueptr, "%s", value);
         if (!strcmp("BEGIN", value)) {
            if (!readAETable(cnffp, &lineno))
               error = 1;
         }
         else if (!strcmp("END", value)) {
            CNF_panic("No \"AETable BEGIN\" before END in configuration file, line %d", lineno);
            error = 1;
         }
         else {
            CNF_panic("Unknown AETable status \"%s\" in configuartion file, line %d", value, lineno);
            error = 1;
         }
      }
      else {
         CNF_panic("Unknown mnemonic \"%s\" in configuration file, line %d", mnemonic, lineno);
         error = 1;
      }
   }

   return(error ? 0 : 1);
}


/*
**  Function name : readHostTable
**
**  Description : read HostTable in configuration file
**  Input : configuration file pointer, line number
**  Output : line number
**  Return : 1 - ok
**     0 - error
*/
static int readHostTable(FILE *cnffp, int *lineno)
{
   int  error = 0,        /* error flag */
        end = 0,          /* end flag */
        noOfPeers;        /* number of peers for entry */
   char rcline[512],      /* line in configuration file */
        mnemonic[64],     /* mnemonic in line */
        value[256],       /* parameter value */
        *lineptr;         /* pointer to line */
   struct HostEntry *helpentry;

   while (!feof(cnffp)) {
      fgets(rcline, sizeof(rcline), cnffp); /* read line in configuration file */
      (*lineno)++;
      if (feof(cnffp)) continue;
      if (rcline[0] == '#' || rcline[0] == 10 || rcline[0] == 13)
         continue;        /* comment or blank line */

      sscanf(rcline, "%s %s", mnemonic, value);
      if (!strcmp("HostTable", mnemonic)) {
         if (!strcmp("END", value)) {
            end = 1;
            break;
         }
         else {
            CNF_panic("Illegal HostTable status \"%s\" in configuration file, line %d", value, *lineno);
            error = 1;
            break;
         }
      }

      lineptr = rcline;
      CNF_HETable.noOfHostEntries++;
      if ((helpentry = (struct HostEntry *)malloc(CNF_HETable.noOfHostEntries * sizeof(struct HostEntry))) == NULL)
         CNF_panic("Memory allocation 1 (%d)", CNF_HETable.noOfHostEntries);
      if (CNF_HETable.noOfHostEntries - 1) {
         memcpy((char*)helpentry, (char*)CNF_HETable.HostEntries, (CNF_HETable.noOfHostEntries - 1) *sizeof(struct HostEntry));
         free(CNF_HETable.HostEntries);
      }
      CNF_HETable.HostEntries = helpentry;

      CNF_HETable.HostEntries[CNF_HETable.noOfHostEntries - 1].SymbolicName = parsevalues(&lineptr);
      CNF_HETable.HostEntries[CNF_HETable.noOfHostEntries - 1].Peers = readPeerList(&lineptr, &noOfPeers);
      CNF_HETable.HostEntries[CNF_HETable.noOfHostEntries - 1].noOfPeers = noOfPeers;
      if (!noOfPeers)
         error = 1;
   }

   if (!end) {
      error = 1;
      CNF_panic("No \"HostTable END\" in configuration file, line %d", *lineno);
    }
   return(error ? 0 : 1);
}


/*
**  Function name : readVendorTable
**
**  Description : read VendorTable in configuration file
**  Input : configuration file pointer, line number
**  Output : line number
**  Return : 1 - ok
**     0 - error
*/
static int readVendorTable(FILE *cnffp, int *lineno)
{
   int  error = 0,        /* error flag */
        end = 0,          /* end flag */
        noOfPeers;        /* number of peers for entry */
   char rcline[512],      /* line in configuration file */
        mnemonic[64],     /* mnemonic in line */
        value[256],       /* parameter value */
        *lineptr;         /* pointer to line */
   struct HostEntry *helpentry;

   while (!feof(cnffp)) {
      fgets(rcline, sizeof(rcline), cnffp); /* read line in configuration file */
      (*lineno)++;
      if (feof(cnffp)) continue;
      if (rcline[0] == '#' || rcline[0] == 10 || rcline[0] == 13)
         continue;        /* comment or blank line */

      sscanf(rcline, "%s %s", mnemonic, value);
      if (!strcmp("VendorTable", mnemonic)) {
         if (!strcmp("END", value)) {
            end = 1;
            break;
         }
         else {
            CNF_panic("Illegal VendorTable status \"%s\" in configuration file, line %d", value, *lineno);
            error = 1;
            break;
         }
      }

      lineptr = rcline;
      CNF_VendorTable.noOfHostEntries++;
      if ((helpentry = (struct HostEntry *)malloc(CNF_VendorTable.noOfHostEntries * sizeof(struct HostEntry))) == NULL)
         CNF_panic("Memory allocation 2 (%d)", CNF_VendorTable.noOfHostEntries);
      if (CNF_VendorTable.noOfHostEntries - 1) {
         memcpy((char*)helpentry, (char*)CNF_VendorTable.HostEntries, (CNF_VendorTable.noOfHostEntries - 1) *sizeof(struct HostEntry));
         free(CNF_VendorTable.HostEntries);
      }
      CNF_VendorTable.HostEntries = helpentry;

      CNF_VendorTable.HostEntries[CNF_VendorTable.noOfHostEntries - 1].SymbolicName = parsevalues(&lineptr);
      CNF_VendorTable.HostEntries[CNF_VendorTable.noOfHostEntries - 1].Peers = readPeerList(&lineptr, &noOfPeers);
      CNF_VendorTable.HostEntries[CNF_VendorTable.noOfHostEntries - 1].noOfPeers = noOfPeers;
      if (!noOfPeers)
         error = 1;
   }

   if (!end) {
      error = 1;
      CNF_panic("No \"VendorTable END\" in configuration file, line %d", *lineno);
    }
   return(error ? 0 : 1);
}


/*
**  Function name : readAETable
**
**  Description : read AETable in configuration file
**  Input : configuration file pointer, line number
**  Output : line number
**  Return : 1 - ok
**     0 - error
*/
static int readAETable(FILE *cnffp, int *lineno)
{
   int  error = 0,          /* error flag */
        end = 0,            /* end flag */
        noOfAEEntries = 0;  /* number of AE entries */
   char rcline[512],        /* line in configuration file */
        mnemonic[64],       /* mnemonic in line */
        value[256],         /* parameter value */
        *lineptr;           /* pointer to line */
   struct AEEntry *helpentry;

   while (!feof(cnffp)) {
      fgets(rcline, sizeof(rcline), cnffp); /* read line in configuration file */
      (*lineno)++;
      if (feof(cnffp)) continue;
      if (rcline[0] == '#' || rcline[0] == 10 || rcline[0] == 13)
         continue;        /* comment or blank line */

      sscanf(rcline, "%s %s", mnemonic, value);
      if (!strcmp("AETable", mnemonic)) {
         if (!strcmp("END", value)) {
            end = 1;
            break;
         }
         else {
            CNF_panic("Illegal AETable status \"%s\" in configuration file, line %d", value, *lineno);
            error = 1;
            break;
         }
      }

      lineptr = rcline;
      noOfAEEntries++;
      if ((helpentry = (struct AEEntry *)malloc(noOfAEEntries * sizeof(struct AEEntry))) == NULL)
         CNF_panic("Memory allocation 3 (%d)", noOfAEEntries);
      if (noOfAEEntries - 1) {
         memcpy((char*)helpentry, (char*)CNF_Config.AEEntries, (noOfAEEntries - 1) *sizeof(struct AEEntry));
         free(CNF_Config.AEEntries);
      }
      CNF_Config.AEEntries = helpentry;

      CNF_Config.AEEntries[noOfAEEntries - 1].ApplicationTitle = parsevalues(&lineptr);
      CNF_Config.AEEntries[noOfAEEntries - 1].StorageArea = parsevalues(&lineptr);
      CNF_Config.AEEntries[noOfAEEntries - 1].Access = parsevalues(&lineptr);
      CNF_Config.AEEntries[noOfAEEntries - 1].StorageQuota = parseQuota(&lineptr);
      CNF_Config.AEEntries[noOfAEEntries - 1].Peers = parsePeers(&lineptr, &CNF_Config.AEEntries[noOfAEEntries - 1].noOfPeers);
      if (!CNF_Config.AEEntries[noOfAEEntries - 1].noOfPeers)
         error = 1;
   }

   if (!end) {
      error = 1;
      CNF_panic("No \"AETable END\" in configuration file, line %d", *lineno);
    }
   CNF_Config.noOfAEEntries = noOfAEEntries;
   return(error ? 0 : 1);
}


/*
**  Function name : parseQuota
**
**  Description : separate a quota from value list
**  Input : pointer to value list
**  Return : pointer to quota structure
*/
static Quota *parseQuota(char **valuehandle)
{
   int  studies;
   char *helpvalue,
        helpval[20];
   Quota *helpquota;

   if ((helpquota = (Quota *)malloc(sizeof(Quota))) == NULL)
      CNF_panic("Memory allocation4");
   helpvalue = parsevalues(valuehandle);
   sscanf(helpvalue, "%d , %s", &studies, helpval);
   helpquota->maxStudies = studies;
   helpquota->maxBytesPerStudy = quota(helpval);
   free(helpvalue);

   return(helpquota);
}


/*
**  Function name : parsePeers
**
**  Description : separate the peer list from value list
**  Input : pointer to value list
**  Output : number of peers
**  Return : pointer to peer list
*/
static Peer *parsePeers(char **valuehandle, int *peers)
{
   char *helpvalue;
   char *valueptr = *valuehandle;

   helpvalue = parsevalues(valuehandle);
   if (!strcmp("ANY", helpvalue)) {     /* keywork ANY used */
      free(helpvalue);
      *peers = -1;
      return((Peer *) 0);
   }

   free(helpvalue);         /* regular peer list */
   return(readPeerList(&valueptr, peers));
}


/*
**  Function name : readPeerList
**
**  Description : extract peers from peer list
**  Input : pointer to value list
**  Output : number of peers
**  Return : pointer to peer list
*/
static Peer *readPeerList(char **valuehandle, int *peers)
{
   int  i,
   found,
   noOfPeers = 0;
   char *helpvalue;
   Peer *helppeer,
   *peerlist = NULL;

   while((helpvalue = parsevalues(valuehandle)) != NULL) {
      found = 0;
      if (strchr(helpvalue, ',') == NULL) {   /* symbolic name */
         if (!CNF_HETable.noOfHostEntries) {
            CNF_panic("No symbolic names defined");
            *peers = 0;
            free(helpvalue);
            return((Peer *) 0);
         }
         for(i = 0; i < CNF_HETable.noOfHostEntries; i++) {
            if (!strcmp(CNF_HETable.HostEntries[i].SymbolicName, helpvalue)) {
               found = 1;
               break;
            }
         }
         if (!found) {
            CNF_panic("Symbolic name \"%s\" not defined", helpvalue);
            *peers = 0;
            free(helpvalue);
            return((Peer *) 0);
         }

         noOfPeers += CNF_HETable.HostEntries[i].noOfPeers;
        if ((helppeer = (Peer *)malloc(noOfPeers * sizeof(Peer))) == NULL)
            CNF_panic("Memory allocation 5 (%d)", noOfPeers);
        if (noOfPeers - CNF_HETable.HostEntries[i].noOfPeers) {
            memcpy((char*)helppeer, (char*)peerlist, (noOfPeers - CNF_HETable.HostEntries[i].noOfPeers) * sizeof(Peer));
            free(peerlist);
         }
         peerlist = helppeer;
         memcpy((char*)(peerlist + (noOfPeers - CNF_HETable.HostEntries[i].noOfPeers)), (char*)CNF_HETable.HostEntries[i].Peers, CNF_HETable.HostEntries[i].noOfPeers * sizeof(Peer));
      }

      else {            /* peer */
         noOfPeers++;
         if ((helppeer = (Peer *)malloc(noOfPeers * sizeof(Peer))) == NULL)
            CNF_panic("Memory allocation 6 (%d)", noOfPeers);
         if (noOfPeers - 1) {
            memcpy((char*)helppeer, (char*)peerlist, (noOfPeers - 1) *sizeof(Peer));
            free(peerlist);
         }
         peerlist = helppeer;

         char *tempvalue = helpvalue;
         peerlist[noOfPeers - 1].ApplicationTitle = parsevalues(&helpvalue);
         peerlist[noOfPeers - 1].HostName = parsevalues(&helpvalue);
         peerlist[noOfPeers - 1].PortNumber = atoi(helpvalue);
         helpvalue = tempvalue;
      }
      free(helpvalue);
   }
   *peers = noOfPeers;
   return(peerlist);
}


/*
**  Function name : skipmnemonic
**
**  Description : skip mnemonic and first gap in rc line
**  Input : rc line
**  Return : pointer to value list
*/
static char *skipmnemonic (char *rcline)
{
   char *help = rcline;

   while(*help != '\0') {                       /* leading spaces */
      if (isgap(*help)) help++;
      else break;
    }
   while(*help != '\0') {
      if (!isspace(*help)) help++;    /* Mnemonic */
      else break;
   }
   while(*help != '\0') {
      if (isgap(*help)) help++;     /* Gap */
      else break;
   }
   return(help);
}


/*
**  Function name : isgap
**
**  Description : check if character is white space or separator
**  Input : character
**  Return : 1 - yes
**     0 - no
*/
static int isgap (char gap)
{
   if (isspace(gap) || gap == '=' || gap == ',' || gap == 10 || gap == 13)
      return(1);
   else
      return(0);
}


/*
**  Function name : isquote
**
**  Description : check if character is quote
**  Input : character
**  Return : 1 - yes
**     0 - no
*/
static int isquote (char quote)
{
   if (quote == '"' || quote == '\'' || quote == '(' || quote == ')')
      return(1);
   else
      return(0);
}


/*
**  Function name : parsevalues
**
**  Description : separate on value from value list
**  Input : pointer to value list
**  Return : pointer to next value
*/
static char *parsevalues (char **valuehandle)
{
   int i,
       inquotes = 0,
       count = 0;
   char *value = NULL;
   const char *help,
   *valueptr = *valuehandle;

   if (isquote(*valueptr)) {
      inquotes = 1;
      valueptr++;
   }

   help = valueptr;

   while(*help != '\0') {
      if (inquotes) {
         if (isquote(*help)) {
            if ((value = (char*)malloc(count * sizeof(char) + 1)) == NULL)
               CNF_panic("Memory allocation 7 (%d)", count);
            for(i = 0; i < count; i++)
               value[i] = valueptr[i];
            value[count] = '\0';
            count++;
            help++;
            while (*help != '\0') {
               if (isgap(*help)) {
                  count++;
                  help++;
               }
               else
                  break;
            }
            *valuehandle += (count + 1);
            break;
         }
         else {
            count++;
            help++;
         }
      }
      else {
         if (isgap(*help)) {
            if ((value = (char*)malloc(count * sizeof(char) + 1)) == NULL)
               CNF_panic("Memory allocation 8 (%d)", count);
            for(i = 0; i < count; i++)
               value[i] = valueptr[i];
            value[count] = '\0';
            while (*help != '\0') {
               if (isgap(*help)) {
                  count++;
                  help++;
               }
               else
                  break;
            }
            *valuehandle += count;
            break;
         }
         else {
           count++;
           help++;
         }
      } /* inquotes */
   } /* while */

   return(value);
}


/*
**  Function name : quota
**
**  Description : convert string to long
**  Input : parameter string value
**  Return : parameter as long
**     -1 on error
*/
static long quota (const char *value)
{
   int  number;
   long factor;
   char last = *(value + strlen(value) - 1),  /* last character */
   mult = *(value + strlen(value) - 2);       /* multiplier */

   if (last == 'b' || last == 'B') {
      if (mult == 'k' || mult == 'K') factor = 1024;
      else if (mult == 'm' || mult == 'M') factor = 1024 * 1024;
      else if (mult == 'g' || mult == 'G') factor = 1024 * 1024 * 1024;
      else factor = 1;
   }
   else return(-1L);

   number = atoi(value);
   return(number * factor);
}

/*
** CVS Log
** $Log: cnfpriv.cc,v $
** Revision 1.1  2005/08/23 19:32:03  braindead
** - initial savannah import
**
** Revision 1.1  2005/06/26 19:26:14  pipelka
** - added dcmtk
**
** Revision 1.12  2004/02/04 15:38:55  joergr
** Removed acknowledgements with e-mail addresses from CVS log.
**
** Revision 1.11  2003/04/25 14:10:09  joergr
** Fixed memory leak in readPeerList().
** Corrected debug output of parse routines.
**
** Revision 1.10  2002/11/29 07:18:13  wilkens
** Adapted ti utility to command line classes and added option '-xi'.
**
** Revision 1.9  2002/11/27 13:26:58  meichel
** Adapted module imagectn to use of new header file ofstdinc.h
**
** Revision 1.8  2001/11/12 14:54:18  meichel
** Removed all ctndisp related code from imagectn
**
** Revision 1.7  2001/06/01 15:51:16  meichel
** Updated copyright header
**
** Revision 1.6  2000/03/08 16:40:57  meichel
** Updated copyright header.
**
** Revision 1.5  2000/02/23 15:13:03  meichel
** Corrected macro for Borland C++ Builder 4 workaround.
**
** Revision 1.4  2000/02/01 11:43:38  meichel
** Avoiding to include <stdlib.h> as extern "C" on Borland C++ Builder 4,
**   workaround for bug in compiler header files.
**
** Revision 1.3  1999/06/10 12:11:51  meichel
** Adapted imagectn to new command line option scheme.
**   Added support for Patient/Study Only Q/R model and C-GET (experimental).
**
** Revision 1.2  1996/09/27 08:46:18  hewett
** Enclosed system include files with BEGIN_EXTERN_C/END_EXTERN_C.
**
** Revision 1.1.1.1  1996/03/28 19:24:59  hewett
** Oldenburg Image CTN Software ported to use the dcmdata C++ toolkit.
**
*/
