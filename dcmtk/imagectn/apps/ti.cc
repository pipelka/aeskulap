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
 *  Author:  Andrew Hewett
 *
 *  Purpose: Telnet Initiator (ti) Main Program
 *
 *  Last Update:      $Author: braindead $
 *  Update Date:      $Date: 2005/08/23 19:32:03 $
 *  Source File:      $Source: /cvsroot/aeskulap/aeskulap/dcmtk/imagectn/apps/ti.cc,v $
 *  CVS/RCS Revision: $Revision: 1.1 $
 *  Status:           $State: Exp $
 *
 *  CVS/RCS Log at end of file
 *
 */

#include "osconfig.h"

#define INCLUDE_CSTDLIB
#define INCLUDE_CSTDIO
#define INCLUDE_CSTRING
#define INCLUDE_CSTDARG
#define INCLUDE_CERRNO
#define INCLUDE_CTIME
#define INCLUDE_CSIGNAL
#include "ofstdinc.h"
BEGIN_EXTERN_C
#ifdef HAVE_SYS_FILE_H
#include <sys/file.h>
#endif
END_EXTERN_C
#include "ti.h"
#include "assoc.h"
#include "dimse.h"
#include "ofcmdln.h"
#include "cnf.h"
#include "dcdict.h"
#include "cmdlnarg.h"
#include "dcdebug.h"
#include "tiui.h"
#include "tinet.h"
#include "tiquery.h"
#include "dcuid.h"
#include "ofconapp.h"
#ifdef WITH_ZLIB
#include <zlib.h>          /* for zlibVersion() */
#endif

//-------------------------------------------------------------------------------------------------

#define OFFIS_CONSOLE_APPLICATION "ti"
#define MAXREMOTEDBTITLES 20
#define APPLICATIONTITLE "TELNET_INITIATOR"
#define SHORTCOL 4
#define LONGCOL 21

//-------------------------------------------------------------------------------------------------

char* progname = NULL;
OFBool verbose = OFFalse;
OFBool debug = OFFalse;
const char *remoteDBTitles[ MAXREMOTEDBTITLES ];
int remoteDBTitlesCount = 0;
TI_Config conf;
static char rcsid[] = "$dcmtk: " OFFIS_CONSOLE_APPLICATION " v" OFFIS_DCMTK_VERSION " " OFFIS_DCMTK_RELEASEDATE " $";
static const char *configFileName = "imagectn.cfg";
E_TransferSyntax networkTransferSyntax = EXS_Unknown;

//-------------------------------------------------------------------------------------------------

static OFBool addPeerName( TI_Config * aConfig, const char *peerName )
{
  int k;
  OFBool found = OFFalse;
  const char **aeTitles;

  if (aConfig->peerNamesCount == TI_MAXPEERS)
    return OFFalse;

  for( k=0; !found && k<aConfig->peerNamesCount ; k++ )
    found = ( strcmp( aConfig->peerNames[k], peerName ) == 0 );

  if( found )
    return( OFTrue );

  if( CNF_aeTitlesForPeer( peerName, &aeTitles ) <= 0 )
  {
    errmsg("no AE titles defined (in: %s) for peer: %s", configFileName, peerName);
    return( OFFalse );
  }

  free( aeTitles );

  aConfig->peerNames[ aConfig->peerNamesCount ] = (char*)peerName;
  aConfig->peerNamesCount++;

  return( OFTrue );
}

//-------------------------------------------------------------------------------------------------

static OFBool findDBPeerTitles( TI_Config * /*aConfig*/, TI_DBEntry *dbEntry, const char *peer )
{
  const char **peerTitles = NULL;
  int peerTitleCount = 0;
  int i;

  // discover all known AETitles for peer
  peerTitleCount = CNF_aeTitlesForPeer( peer,&peerTitles );
  if( peerTitleCount <= 0 )
  {
    errmsg("no AE titles defined (in: %s) for peer: %s", configFileName, peer);
    return( OFFalse );
  }

  // check to make sure peer+AEtitle has access to database areas
  for( i=0 ; i<peerTitleCount ; i++ )
  {
    if( CNF_peerInAETitle( dbEntry->title, peerTitles[i], peer ) )
    {
      // add peer title to database's peer title list
      if( dbEntry->peerTitles == NULL )
        dbEntry->peerTitles = (const char**) malloc( sizeof( const char* ) );
      else
        dbEntry->peerTitles = (const char**) realloc( dbEntry->peerTitles, (dbEntry->peerTitleCount + 1) * sizeof(const char*) );

      dbEntry->peerTitles[ dbEntry->peerTitleCount ] = peerTitles[i];
      dbEntry->peerTitleCount++;
    }
  }

  // throw away the old list
  free( peerTitles );

  return( dbEntry->peerTitleCount > 0 );
}

//-------------------------------------------------------------------------------------------------

static void createConfigEntries( TI_Config *aConfig )
{
  const char **ctnTitles = NULL;
  int ctnTitleCount = 0;
  int i, j;
  TI_DBEntry *dbEntry = NULL;

  aConfig->dbCount = 0;

  // discover all the known CTN AE titles
  ctnTitleCount = CNF_ctnTitles(&ctnTitles);
  for( i=0 ; i<ctnTitleCount ; i++ )
  {
    if( !TI_dbReadable( ctnTitles[i] ) )
      errmsg("Warning: db does not exist: %s", ctnTitles[i]);
    else
    {
      dbEntry = (TI_DBEntry*) malloc( sizeof(TI_DBEntry) );
      bzero( (char*)dbEntry, sizeof(*dbEntry) );
      dbEntry->title = ctnTitles[i];

      for( j=0 ; j<aConfig->peerNamesCount ; j++ )
        findDBPeerTitles(aConfig, dbEntry, aConfig->peerNames[j]);

      if( dbEntry->peerTitleCount > 0 )
      {
        // add database to list, it is accessable by something
        if( aConfig->dbEntries == NULL )
          aConfig->dbEntries = (TI_DBEntry**) malloc( sizeof( TI_DBEntry* ) );
        else
          aConfig->dbEntries = (TI_DBEntry**) realloc( aConfig->dbEntries, (aConfig->dbCount + 1) * sizeof(TI_DBEntry*) );

        aConfig->dbEntries[ aConfig->dbCount ] = dbEntry;
        aConfig->dbCount++;
      }
      else
        free( dbEntry );
    }
  }

  // throw away the old lists
  free( ctnTitles );

  // add any remote DB entries
  for( i=0 ; i<remoteDBTitlesCount ; i++ )
  {
    const char *peerName = NULL;
    int portNumber;
    if( CNF_peerForAETitle( remoteDBTitles[i], &peerName, &portNumber ) )
    {
      // add DB
      dbEntry = (TI_DBEntry*) malloc( sizeof( TI_DBEntry ) );
      bzero( (char*)dbEntry, sizeof(*dbEntry) );
      dbEntry->title = remoteDBTitles[i];
      dbEntry->isRemoteDB = OFTrue;

      if( aConfig->dbEntries == NULL )
        aConfig->dbEntries = (TI_DBEntry**) malloc( sizeof( TI_DBEntry* ) );
      else
        aConfig->dbEntries = (TI_DBEntry**) realloc( aConfig->dbEntries, (aConfig->dbCount + 1) * sizeof(TI_DBEntry*) );

      aConfig->dbEntries[ aConfig->dbCount ] = dbEntry;
      aConfig->dbCount++;

      for( j=0 ; j<aConfig->peerNamesCount ; j++ )
      {
        const char **peerTitles = NULL;
        int peerTitleCount = 0;
        int k;

        peerTitleCount = CNF_aeTitlesForPeer( aConfig->peerNames[j], &peerTitles );
        if( peerTitleCount <= 0 )
          errmsg("no AE titles defined (in: %s) for peer: %s", configFileName, aConfig->peerNames[j]);

        for( k=0 ; k<peerTitleCount ; k++ )
        {
          // add peer title to database's peer title list
          if( dbEntry->peerTitles == NULL )
            dbEntry->peerTitles = (const char**) malloc( sizeof( const char* ) );
          else
            dbEntry->peerTitles = (const char**) realloc( dbEntry->peerTitles, (dbEntry->peerTitleCount + 1) * sizeof(const char*) );

          dbEntry->peerTitles[ dbEntry->peerTitleCount ] = peerTitles[k];
          dbEntry->peerTitleCount++;
        }

        // throw away the old list
        free( peerTitles );
      }
    }
  }
}

//-------------------------------------------------------------------------------------------------

static void printConfig( TI_Config *aConfig )
{
  int i,j;

  printf("TI Configuration:\n");
  printf("My AE Title: %s\n", aConfig->myAETitle );
  printf("DatabaseTitles    Peer AE Titles\n");

  for( i=0 ; i<aConfig->dbCount ; i++ )
  {
    printf("%-18s", aConfig->dbEntries[i]->title );

    for( j=0 ; j<aConfig->dbEntries[i]->peerTitleCount ; j++ )
      printf("%s ", aConfig->dbEntries[i]->peerTitles[j] );

    printf("\n");
  }
}

//-------------------------------------------------------------------------------------------------

/*
 * Handle interrupt signals.
 * We only really need to make sure that the display is clear
 * before quiting.
 */
#ifdef SIGNAL_HANDLER_WITH_ELLIPSE
extern "C" void TI_signalHandler(...)
#else
extern "C" void TI_signalHandler(int)
#endif
{
  TI_detatchAssociation( &conf, OFTrue );
  exit( 1 );
}

//-------------------------------------------------------------------------------------------------

void errmsg( const char* msg, ... )
{
  va_list args;

  fprintf( stderr, "%s: ", progname );
  va_start( args, msg );
  vfprintf( stderr, msg, args );
  va_end( args );
  fprintf( stderr, "\n" );
}

//-------------------------------------------------------------------------------------------------

int main( int argc, char *argv[] )
{
  const char *currentPeer = NULL, **vendorHosts, **aeTitleList;
  OFBool noCommandLineValueForMaxReceivePDULength = OFTrue;
  int peerCount, j, n, returnValue = 0;
  OFCondition cond = EC_Normal;
  char tempstr[20];

#ifdef HAVE_GUSI_H
  // needed for Macintosh
  GUSISetup( GUSIwithSIOUXSockets );
  GUSISetup( GUSIwithInternetSockets );
#endif

#ifdef HAVE_WINSOCK_H
  WSAData winSockData;
  // we need at least version 1.1
  WORD winSockVersionNeeded = MAKEWORD( 1, 1 );
  WSAStartup( winSockVersionNeeded, &winSockData );
#endif

  // initialize conf structure
  bzero((char*)&conf, sizeof(conf));
  conf.myAETitle = APPLICATIONTITLE;

  // setup console application and command line objects
  OFConsoleApplication app( OFFIS_CONSOLE_APPLICATION , "Telnet initiator", rcsid );
  OFCommandLine cmd;

  cmd.setParamColumn( LONGCOL + SHORTCOL + 2 );
  cmd.addParam( "peer", "peer host name or symbolic name from cfg file", OFCmdParam::PM_MultiMandatory );

  cmd.setOptionColumns( LONGCOL, SHORTCOL );
  cmd.addGroup( "general options:");
    cmd.addOption( "--help",                      "-h",      "print this help text and exit" );
    cmd.addOption( "--version",                              "print version information and exit", OFTrue );
    cmd.addOption( "--verbose",                   "-v",      "verbose mode, print processing details" );
    cmd.addOption( "--debug",                     "-d",      "debug mode, print debug information" );
    OFString opt0 = "use configuration file f (default: ";
    opt0 += configFileName;
    opt0 += ")";
    cmd.addOption( "--config",                    "-c",   1, "[f]ilename: string", opt0.c_str() );

  cmd.addGroup( "network options:" );
    cmd.addOption( "--propose-implicit",          "-xi",     "propose implicit VR little endian TS only" );
    OFString opt1 = "set my AE title (default: ";
    opt1 += APPLICATIONTITLE;
    opt1 += ")";
    cmd.addOption( "--aetitle",                   "-aet", 1, "aetitle: string", opt1.c_str() );
    OFString opt2 = "[n]umber of bytes: integer [";
    sprintf(tempstr, "%ld", (long)ASC_MINIMUMPDUSIZE);
    opt2 += tempstr;
    opt2 += "..";
    sprintf(tempstr, "%ld", (long)ASC_MAXIMUMPDUSIZE);
    opt2 += tempstr;
    opt2 += "]";
    cmd.addOption( "--max-pdu",                   "-pdu", 1,  opt2.c_str(), "set max receive pdu to n bytes\n(default: use value from configuration file)" );
    cmd.addOption( "--timeout",                   "-to",  1, "[s]econds: integer (default: unlimited)", "timeout for connection requests");

  cmd.addGroup( "other options:" );
    cmd.addOption( "--disable-new-vr",            "-u",       "disable support for new VRs, convert to OB" );
    cmd.addOption( "--remote",                    "-rmt", 1,  "[t]itle: string", "connect to remote database defined in cfg file" );

  // evaluate command line
  prepareCmdLineArgs( argc, argv, OFFIS_CONSOLE_APPLICATION );
  if( app.parseCommandLine( cmd, argc, argv, OFCommandLine::ExpandWildcards ) )
  {
    // check exclusive options first
    if( cmd.getParamCount() == 0 )
    {
      if( cmd.findOption("--version") )
      {
        app.printHeader( OFTrue );
        CERR << endl << "External libraries used:";
#if !defined(WITH_ZLIB)
        CERR << " none" << endl;
#else
        CERR << endl;
#endif
#ifdef WITH_ZLIB
        CERR << "- ZLIB, Version " << zlibVersion() << endl;
#endif
        return( 0 );
      }
    }

    // command line parameters
    if( cmd.findOption("--verbose") ) verbose = OFTrue;
    if( cmd.findOption("--debug") )
    {
      debug = OFTrue;
      verbose = OFTrue;
      DUL_Debug(OFTrue);
      DIMSE_debug(OFTrue);
      SetDebugLevel(3);
    }
    if( cmd.findOption("--config") ) app.checkValue( cmd.getValue( configFileName ) );
    if( cmd.findOption("--propose-implicit") ) networkTransferSyntax = EXS_LittleEndianImplicit;
    if( cmd.findOption("--aetitle") ) app.checkValue( cmd.getValue( conf.myAETitle ) );
    if( cmd.findOption("--max-pdu") )
    {
      app.checkValue( cmd.getValueAndCheckMinMax( conf.maxReceivePDULength, ASC_MINIMUMPDUSIZE, ASC_MAXIMUMPDUSIZE ) );
      noCommandLineValueForMaxReceivePDULength = OFFalse;
    }

    if (cmd.findOption("--timeout"))
    {
      OFCmdSignedInt opt_timeout = 0;
      app.checkValue(cmd.getValueAndCheckMin(opt_timeout, 1));
      dcmConnectionTimeout.set((Sint32) opt_timeout);
    }

    if( cmd.findOption("--disable-new-vr") )
    {
      dcmEnableUnknownVRGeneration.set( OFFalse );
      dcmEnableUnlimitedTextVRGeneration.set( OFFalse );
    }

    if (cmd.findOption("--remote", 0, OFCommandLine::FOM_First))
    {
      do
      {
        if( remoteDBTitlesCount < MAXREMOTEDBTITLES )
        {
          app.checkValue( cmd.getValue( remoteDBTitles[remoteDBTitlesCount] ) );
          remoteDBTitlesCount++;
        }
        else CERR << "ti: Too many remote database titles." << endl;
      } while (cmd.findOption("--remote", 0, OFCommandLine::FOM_Next));
    }

  }

  // in case accessing the configuration file for reading is successful
  if( access( configFileName, R_OK ) != -1 )
  {
    // in case reading values from configuration file is successful
    if( CNF_init( configFileName ) == 1 )
    {
      // dump information
      if( verbose )
        CNF_printConfig();

      // determine max pdu size from configuration file
      OFCmdUnsignedInt maxPDU = CNF_getMaxPDUSize();

      // in case the max pdu size was not set in the configuration file, or
      // in case its value is not in a certain range, use the default value
      if( maxPDU == 0 || maxPDU < ASC_MINIMUMPDUSIZE || maxPDU > ASC_MAXIMUMPDUSIZE )
      {
        COUT << "ti: no or invalid max pdu size found in configuration file." << endl;
        maxPDU = ASC_DEFAULTMAXPDU;
      }

      // if no max pdu size was set on the command line then use config file value
      if( noCommandLineValueForMaxReceivePDULength )
        conf.maxReceivePDULength = maxPDU;

      // go through all peers that were specified on the command line
      peerCount = cmd.getParamCount();
      for( int i=1 ; i<=peerCount ; i++ )
      {
        // determine current peer
        cmd.getParam( i, currentPeer );

        // in general, we now want to add host names to the conf structure; it might be
        // though that currentPeer is a symbolic name that stands for a number of hosts;
        // hence we need to check first if peer can stands for a symbolic name
        if( ( n = CNF_aeTitlesForSymbolicName( currentPeer, &aeTitleList ) ) > 0 )
        {
          // in case peer is a symbolic name and can be found in the host table,
          // determine corresponding host names and add them to conf structure
          const char *peerName = NULL;
          int portNumber;
          for( j=0 ; j<n ; j++ )
          {
            if( CNF_peerForAETitle( aeTitleList[j], &peerName, &portNumber ) )
              addPeerName( &conf, peerName );
          }

          // free memory
          if( aeTitleList )
            free( aeTitleList );
          aeTitleList = NULL;
        }
        else if( ( n = CNF_HostNamesForVendor( currentPeer, &vendorHosts ) ) > 0 )
        {
          // in case peer is a symbolic name and can be interpreted as a vendor name, add the
          // corresponding host names are known for for this vendor to the conf structure
          for( j=0 ; j<n ; j++ )
            addPeerName(&conf, vendorHosts[j]);

          // free memory
          if( vendorHosts )
            free( vendorHosts );
          vendorHosts = NULL;
        }
        else
        {
          // in case peer is not a symbolic name but the name of a
          // specific host, add this host name to the conf structure
          addPeerName( &conf, currentPeer );
        }
      }

      // set "peer to talk to" to the first host
      // name in the array (this is the default)
      conf.peerHostName = conf.peerNames[0];

      // load up configuration info
      createConfigEntries( &conf );

      // only go ahead in case there is at least one database we know of
      if( conf.dbCount > 0 )
      {
        // dump information
        if( verbose )
          printConfig( &conf );

        // make sure data dictionary is loaded
        if( !dcmDataDict.isDictionaryLoaded() )
          CERR << "Warning: no data dictionary loaded, check environment variable: " << DCM_DICT_ENVIRONMENT_VARIABLE << endl;

        // if starting up network is successful
        cond = ASC_initializeNetwork( NET_REQUESTOR, 0, 200, &conf.net );
        if( cond.good() )
        {
          // set interrupts for signal handling
#ifdef SIGHUP
          signal( SIGHUP, TI_signalHandler );
#endif
#ifdef SIGINT
          signal( SIGINT, TI_signalHandler );
#endif
#ifdef SIGQUIT
          signal( SIGQUIT, TI_signalHandler );
#endif
#ifdef SIGTERM
          signal( SIGTERM, TI_signalHandler );
#endif

          // do the real work
          TI_userInput( &conf );

          // clean up network
          cond = ASC_dropNetwork( &conf.net );
          if( cond.bad() )
          {
            CERR << "ti: error dropping network: ";
            DimseCondition::dump( cond );
            returnValue = 1;
          }
        }
        else
        {
          CERR << "ti: error initialising network: ";
          DimseCondition::dump( cond );
          returnValue = 1;
        }
      }
      else
      {
        CERR << "ti: no accessable databases." << endl;
        returnValue = 1;
      }
    }
    else
    {
      CERR << "ti: error while reading configuration file '" << configFileName << "'." << endl;
      returnValue = 1;
    }
  }
  else
  {
    CERR << "ti: cannot access configuration file '" << configFileName << "'." << endl;
    returnValue = 1;
  }

#ifdef HAVE_WINSOCK_H
  WSACleanup();
#endif

  // return result
  return( returnValue );
}

//-------------------------------------------------------------------------------------------------

/*
** CVS Log
** $Log: ti.cc,v $
** Revision 1.1  2005/08/23 19:32:03  braindead
** - initial savannah import
**
** Revision 1.1  2005/06/26 19:26:14  pipelka
** - added dcmtk
**
** Revision 1.33  2004/02/10 15:38:38  joergr
** Renamed configuration file from "configrc" to "imagectn.cfg".
**
** Revision 1.32  2003/11/05 16:09:22  meichel
** Fixed "--remote" command line option which can be specified multiple times
**
** Revision 1.31  2003/09/04 10:08:05  joergr
** Replaced wrong call of OFCommandLine::getValueAndCheckMin() by
** OFCommandLine::getValueAndCheckMinMax().
**
** Revision 1.30  2003/01/31 12:00:59  wilkens
** Fixed bug in ti application (option named "--config" and "--config-file").
**
** Revision 1.29  2002/11/29 13:00:39  meichel
** Introduced new command line option --timeout for controlling the
**   connection request timeout.
**
** Revision 1.28  2002/11/29 07:18:15  wilkens
** Adapted ti utility to command line classes and added option '-xi'.
**
** Revision 1.27  2002/11/27 13:27:02  meichel
** Adapted module imagectn to use of new header file ofstdinc.h
**
** Revision 1.26  2001/11/12 14:54:22  meichel
** Removed all ctndisp related code from imagectn
**
** Revision 1.25  2001/10/12 12:42:55  meichel
** Adapted imagectn to OFCondition based dcmnet module (supports strict mode).
**
** Revision 1.24  2001/06/01 15:51:22  meichel
** Updated copyright header
**
** Revision 1.23  2000/12/15 13:25:10  meichel
** Declared qsort() and signal() callback functions as extern "C", avoids
**   warnings on Sun C++ 5.x compiler.
**
** Revision 1.22  2000/04/14 16:38:03  meichel
** Global VR generation flags are now derived from OFGlobal and, thus,
**   safe for use in multi-thread applications.
**
** Revision 1.21  2000/03/08 16:41:02  meichel
** Updated copyright header.
**
** Revision 1.20  2000/02/29 11:57:54  meichel
** Removed support for VS value representation. This was proposed in CP 101
**   but never became part of the standard.
**
** Revision 1.19  2000/02/23 15:13:14  meichel
** Corrected macro for Borland C++ Builder 4 workaround.
**
** Revision 1.18  2000/02/01 11:43:41  meichel
** Avoiding to include <stdlib.h> as extern "C" on Borland C++ Builder 4,
**   workaround for bug in compiler header files.
**
** Revision 1.17  1999/06/10 12:12:02  meichel
** Adapted imagectn to new command line option scheme.
**   Added support for Patient/Study Only Q/R model and C-GET (experimental).
**
** Revision 1.16  1999/02/09 14:30:46  meichel
** Made signal handler function parameters compile flag dependent.
**   Now passing (int) unless SIGNAL_HANDLER_WITH_ELLIPSE defined,
**   in which case (...) is passed. Required to avoid conflict between
**   Sun CC 4.2 on Solaris and SGI C++ 4.0 on IRIX.
**
** Revision 1.15  1998/07/24 14:15:51  meichel
** changed ti's signal handler parameters from (int) to (...)
**   to avoid a warning on IRIX 5.3 using SGI C++ 4.0.
**
** Revision 1.14  1998/02/06 15:07:35  meichel
** Removed many minor problems (name clashes, unreached code)
**   reported by Sun CC4 with "+w" or Sun CC2.
**
** Revision 1.13  1998/01/14 14:29:03  hewett
** Modified existing -u command line option to also disable generation
** of UT and VS (previously just disabled generation of UN).
**
** Revision 1.12  1997/08/05 07:46:33  andreas
** - Change needed version number of WINSOCK to 1.1
**   to support WINDOWS 95
**
** Revision 1.11  1997/07/21 08:59:45  andreas
** - Replace all boolean types (BOOLEAN, CTNBOOLEAN, DICOM_BOOL, BOOL)
**   with one unique boolean type OFBool.
**
** Revision 1.10  1997/06/26 12:55:31  andreas
** - Calling of signal function only for existing signals (Windows NT/95
**   do not have the same set of signals as UNIX).
**
** Revision 1.9  1997/05/29 17:06:35  meichel
** All dcmtk applications now contain a version string
** which is displayed with the command line options ("usage" message)
** and which can be queried in the binary with the "ident" command.
**
** Revision 1.8  1997/05/22 13:31:40  hewett
** Modified the test for presence of a data dictionary to use the
** method DcmDataDictionary::isDictionaryLoaded().
**
** Revision 1.7  1997/03/27 16:11:40  hewett
** Added command line switches allowing generation of UN to
** be disabled (it is enabled by default).
**
** Revision 1.6  1996/09/27 08:46:23  hewett
** Enclosed system include files with BEGIN_EXTERN_C/END_EXTERN_C.
**
** Revision 1.5  1996/05/30 17:41:27  hewett
** Added command line argument (-DD) to explicitly disable communication
** with ctndisp and override the configuration file.
**
** Revision 1.4  1996/05/06 07:33:14  hewett
** Rearranged handing of display disabling code.
**
** Revision 1.3  1996/04/25 16:29:09  hewett
** Added char* parameter casts to bzero() calls.
**
** Revision 1.2  1996/04/22 16:33:03  hewett
** Removed calls to DSU_setTraceLevel().
**
** Revision 1.1  1996/04/22 10:27:25  hewett
** Initial release.
**
*/
