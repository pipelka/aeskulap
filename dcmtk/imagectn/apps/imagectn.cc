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
 *  Purpose: Image Server Central Test Node (ctn) Main Program
 *
 *  Last Update:      $Author: braindead $
 *  Update Date:      $Date: 2005/08/23 19:32:03 $
 *  Source File:      $Source: /cvsroot/aeskulap/aeskulap/dcmtk/imagectn/apps/imagectn.cc,v $
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
#define INCLUDE_CERRNO
#define INCLUDE_CTIME
#include "ofstdinc.h"

BEGIN_EXTERN_C
#ifdef HAVE_LIBC_H
#include <libc.h>
#endif
#ifdef HAVE_SYS_FILE_H
#include <sys/file.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_SYS_ERRNO_H
#include <sys/errno.h>
#endif
#ifdef HAVE_IO_H
#include <io.h>
#endif
END_EXTERN_C

#include "dicom.h"
#include "imagectn.h"
#include "dimse.h"
#include "cnf.h"
#include "sce.h"
#include "imagedb.h"
#include "dcdict.h"
#include "dcdebug.h"
#include "cmdlnarg.h"
#include "ofconapp.h"
#include "dcuid.h"       /* for dcmtk version name */

#ifdef WITH_ZLIB
#include <zlib.h>        /* for zlibVersion() */
#endif

#define OFFIS_CONSOLE_APPLICATION "imagectn"

static char rcsid[] = "$dcmtk: " OFFIS_CONSOLE_APPLICATION " v"
  OFFIS_DCMTK_VERSION " " OFFIS_DCMTK_RELEASEDATE " $";

#define APPLICATIONTITLE    "IMAGECTN"

const OFConditionConst APPE_INVALIDPEER(1024, 1, OF_error, "ImageCTN: invalid peer for move operation");
const OFCondition APP_INVALIDPEER(APPE_INVALIDPEER);

typedef struct _ctn_processslot CTN_ProcessSlot;

struct _ctn_processslot {
    DIC_NODENAME    peerName;
    DIC_AE          callingAETitle;
    DIC_AE          calledAETitle;
    int             processId;
    time_t          startTime;
    OFBool          hasStorageAbility;
    CTN_ProcessSlot *next;
};

typedef struct {
    int             pcnt;
    CTN_ProcessSlot *plist;
} CTN_ProcessTable;

static CTN_ProcessTable processTable;

const char *nonStorageSyntaxes[] = {
    UID_VerificationSOPClass,
    UID_FINDPatientRootQueryRetrieveInformationModel,
    UID_MOVEPatientRootQueryRetrieveInformationModel,
    UID_GETPatientRootQueryRetrieveInformationModel,
#ifndef NO_PATIENTSTUDYONLY_SUPPORT
    UID_FINDPatientStudyOnlyQueryRetrieveInformationModel,
    UID_MOVEPatientStudyOnlyQueryRetrieveInformationModel,
    UID_GETPatientStudyOnlyQueryRetrieveInformationModel,
#endif
    UID_FINDStudyRootQueryRetrieveInformationModel,
    UID_MOVEStudyRootQueryRetrieveInformationModel,
    UID_GETStudyRootQueryRetrieveInformationModel,
    UID_PrivateShutdownSOPClass
};

int numberOfNonStorageSyntaxes = DIM_OF(nonStorageSyntaxes);

/* Reasons for refusing association */
typedef enum {
    CTN_TooManyAssociations,
    CTN_CannotFork,
    CTN_BadAppContext,
    CTN_BadAEPeer,
    CTN_BadAEService,
    CTN_NoReason
} CTN_RefuseReason;



E_TransferSyntax   opt_networkTransferSyntax = EXS_Unknown;
E_TransferSyntax   opt_writeTransferSyntax = EXS_Unknown;
OFCmdUnsignedInt   opt_maxPDU = ASC_DEFAULTMAXPDU;
int                opt_verbose = 0;
OFBool             opt_debug = OFFalse;
OFBool             opt_ignoreStoreData = OFFalse;
OFBool             opt_requireFindForMove = OFFalse;
OFBool             opt_restrictMoveToSameVendor = OFFalse;
OFBool             opt_restrictMoveToSameHost = OFFalse;
OFBool             opt_restrictMoveToSameAE = OFFalse;
OFBool             opt_bitPreserving = OFFalse;
OFBool             opt_useMetaheader = OFTrue;
OFBool             opt_correctUIDPadding = OFFalse;
E_GrpLenEncoding   opt_groupLength = EGL_recalcGL;
E_EncodingType     opt_sequenceType = EET_ExplicitLength;
E_PaddingEncoding  opt_paddingType = EPD_withoutPadding;
OFCmdUnsignedInt   opt_filepad = 0;
OFCmdUnsignedInt   opt_itempad = 0;
T_ASC_Network     *net = NULL; /* the global DICOM network */

static OFBool      opt_rejectWhenNoImplementationClassUID = OFFalse;
static OFBool      opt_refuse = OFFalse;
static const char *opt_configFileName = "imagectn.cfg";
static OFBool      opt_refuseMultipleStorageAssociations = OFFalse;
static OFBool      opt_checkFindIdentifier = OFFalse;
static OFBool      opt_checkMoveIdentifier = OFFalse;
static OFBool      opt_supportPatientRoot = OFTrue;
static OFBool      opt_supportStudyRoot = OFTrue;
static OFBool      opt_disableGetSupport = OFFalse;
static OFBool      opt_allowShutdown = OFFalse;
static int         opt_maxAssociations = 20;    /* initial guess */
static OFCmdUnsignedInt opt_port = 0;
#ifdef HAVE_FORK
static OFBool      opt_singleProcess = OFFalse;
#else
static OFBool      opt_singleProcess = OFTrue;
#endif
#ifdef NO_PATIENTSTUDYONLY_SUPPORT
static OFBool      opt_supportPatientStudyOnly = OFFalse;
#else
static OFBool      opt_supportPatientStudyOnly = OFTrue;
#endif

void errmsg(const char* msg, ...)
{
    va_list args;

    fprintf(stderr, "%s: ", OFFIS_CONSOLE_APPLICATION);
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    fprintf(stderr, "\n");
}

static OFCondition waitForAssociation(T_ASC_Network *net);
static void cleanChildren();
static OFCondition negotiateAssociation(T_ASC_Association *assoc);

#define SHORTCOL 4
#define LONGCOL 21

int
main(int argc, char *argv[])
{
    OFCondition cond = EC_Normal;
    OFCmdUnsignedInt overridePort = 0;
    OFCmdUnsignedInt overrideMaxPDU = 0;

#ifdef HAVE_GUSI_H
    /* needed for Macintosh */
    GUSISetup(GUSIwithSIOUXSockets);
    GUSISetup(GUSIwithInternetSockets);
#endif

#ifdef HAVE_WINSOCK_H
    WSAData winSockData;
    /* we need at least version 1.1 */
    WORD winSockVersionNeeded = MAKEWORD( 1, 1 );
    WSAStartup(winSockVersionNeeded, &winSockData);
#endif

  char tempstr[20];
#ifdef HAVE_FORK
  OFConsoleApplication app(OFFIS_CONSOLE_APPLICATION , "DICOM image archive (central test node)", rcsid);
#else
  OFConsoleApplication app(OFFIS_CONSOLE_APPLICATION , "DICOM image archive (central test node)\nThis version of imagectn supports only single process mode.", rcsid);
#endif

  OFCommandLine cmd;

  cmd.setParamColumn(LONGCOL+SHORTCOL+4);
  cmd.addParam("port", "tcp/ip port number to listen on\n(default: in config file)", OFCmdParam::PM_Optional);

  cmd.setOptionColumns(LONGCOL, SHORTCOL);
  cmd.addGroup("general options:", LONGCOL, SHORTCOL+2);
   cmd.addOption("--help",                      "-h",        "print this help text and exit");
   cmd.addOption("--version",                                "print version information and exit", OFTrue /* exclusive */);
   cmd.addOption("--verbose",                   "-v",        "verbose mode, print processing details");
   cmd.addOption("--very-verbose",              "-vv",       "print more processing details");
   cmd.addOption("--debug",                     "-d",        "debug mode, print debug information");
   OFString opt5 = "[f]ilename: string (default: ";
   opt5 += opt_configFileName;
   opt5 += ")";
   cmd.addOption("--config",                    "-c",     1, opt5.c_str(), "use specific configuration file");
#ifdef HAVE_FORK
   cmd.addOption("--single-process",            "-s",        "single process mode");
#endif

  cmd.addGroup("database options:");
    cmd.addSubGroup("association negotiation:");
      cmd.addOption("--require-find",                        "reject all MOVE/GET presentation contexts for\nwhich no corresponding FIND context is proposed");
      cmd.addOption("--no-parallel-store",                   "reject multiple simultaneous STORE presentation\ncontexts for one application entity title");
      cmd.addOption("--disable-get",                         "disable C-GET support");
      cmd.addOption("--allow-shutdown",                      "allow external shutdown via a private SOP class");
    cmd.addSubGroup("checking identifier validity:");
      cmd.addOption("--check-find",             "-XF",       "check C-FIND identifier validity");
      cmd.addOption("--no-check-find",                       "do not check C-FIND identif. validity (default)");
      cmd.addOption("--check-move",             "-XM",       "check C-MOVE identifier validity");
      cmd.addOption("--no-check-move",                       "do not check C-MOVE identif. validity (default)");
    cmd.addSubGroup("restriction of move targets:");
      cmd.addOption("--move-unrestricted",                   "do not restrict move destination (default)");
      cmd.addOption("--move-aetitle",           "-ZA",       "restrict move dest. to requesting AE title");
      cmd.addOption("--move-host",              "-ZH",       "restrict move destination to requesting host");
      cmd.addOption("--move-vendor",            "-ZV",       "restrict move destination to requesting vendor");
    cmd.addSubGroup("restriction of query/retrieve models:");
      cmd.addOption("--no-patient-root",        "-QP",       "do not support Patient Root Q/R models");
      cmd.addOption("--no-study-root",          "-QS",       "do not support Study Root Q/R models");
#ifndef NO_PATIENTSTUDYONLY_SUPPORT
      cmd.addOption("--no-patient-study",       "-QO",       "do not support Patient/Study Only Q/R models");
#endif

  cmd.addGroup("network options:");
    cmd.addSubGroup("preferred network transfer syntaxes for uncompressed transfer:");
      cmd.addOption("--prefer-uncompr",         "+x=",       "prefer explicit VR local byte order (default)");
      cmd.addOption("--prefer-little",          "+xe",       "prefer explicit VR little endian TS");
      cmd.addOption("--prefer-big",             "+xb",       "prefer explicit VR big endian TS");
      cmd.addOption("--implicit",               "+xi",       "accept implicit VR little endian TS only");

#ifdef WITH_TCPWRAPPER
    cmd.addSubGroup("network host access control (tcp wrapper) options:");
      cmd.addOption("--access-full",            "-ac",       "accept connections from any host (default)");
      cmd.addOption("--access-control",         "+ac",       "enforce host access control rules");
#endif

    cmd.addSubGroup("other network options:");
      cmd.addOption("--timeout",                "-to",    1, "[s]econds: integer (default: unlimited)", "timeout for connection requests");
      OFString opt3 = "set max receive pdu to n bytes (default: ";
      sprintf(tempstr, "%ld", (long)ASC_DEFAULTMAXPDU);
      opt3 += tempstr;
      opt3 += ")";
      OFString opt4 = "[n]umber of bytes: integer [";
      sprintf(tempstr, "%ld", (long)ASC_MINIMUMPDUSIZE);
      opt4 += tempstr;
      opt4 += "..";
      sprintf(tempstr, "%ld", (long)ASC_MAXIMUMPDUSIZE);
      opt4 += tempstr;
      opt4 += "]";
      cmd.addOption("--max-pdu",                "-pdu",   1,  opt4.c_str(), opt3.c_str());
      cmd.addOption("--disable-host-lookup",    "-dhl",      "disable hostname lookup");
      cmd.addOption("--refuse",                              "refuse association");
      cmd.addOption("--reject",                              "reject association if no implement. class UID");
      cmd.addOption("--ignore",                              "ignore store data, receive but do not store");
      cmd.addOption("--uid-padding",            "-up",       "silently correct space-padded UIDs");

  cmd.addGroup("encoding options:");
    cmd.addSubGroup("post-1993 value representations:");
      cmd.addOption("--enable-new-vr",          "+u",        "enable support for new VRs (UN/UT) (default)");
      cmd.addOption("--disable-new-vr",         "-u",        "disable support for new VRs, convert to OB");

  cmd.addGroup("output options:");
    cmd.addSubGroup("bit preserving mode:");
      cmd.addOption("--normal",                 "-B",        "allow implicit format conversions (default)");
      cmd.addOption("--bit-preserving",         "+B",        "write data exactly as read");
    cmd.addSubGroup("output file format:");
      cmd.addOption("--write-file",             "+F",        "write file format (default)");
      cmd.addOption("--write-dataset",          "-F",        "write data set without file meta information");
    cmd.addSubGroup("output transfer syntax (not with --bit-preserving or compressed transmission):");
      cmd.addOption("--write-xfer-same",        "+t=",       "write with same TS as input (default)");
      cmd.addOption("--write-xfer-little",      "+te",       "write with explicit VR little endian TS");
      cmd.addOption("--write-xfer-big",         "+tb",       "write with explicit VR big endian TS");
      cmd.addOption("--write-xfer-implicit",    "+ti",       "write with implicit VR little endian TS");
    cmd.addSubGroup("group length encoding (not with --bit-preserving):");
      cmd.addOption("--group-length-recalc",    "+g=",       "recalculate group lengths if present (default)");
      cmd.addOption("--group-length-create",    "+g",        "always write with group length elements");
      cmd.addOption("--group-length-remove",    "-g",        "always write without group length elements");
    cmd.addSubGroup("length encoding in sequences and items (not with --bit-preserving):");
      cmd.addOption("--length-explicit",        "+e",        "write with explicit lengths (default)");
      cmd.addOption("--length-undefined",       "-e",        "write with undefined lengths");
    cmd.addSubGroup("data set trailing padding (not with --write-dataset or --bit-preserving):");
      cmd.addOption("--padding-off",            "-p",        "no padding (default)");
      cmd.addOption("--padding-create",         "+p",    2,  "[f]ile-pad [i]tem-pad: integer", "align file on multiple of f bytes\nand items on multiple of i bytes");

    /* evaluate command line */
    prepareCmdLineArgs(argc, argv, OFFIS_CONSOLE_APPLICATION);
    if (app.parseCommandLine(cmd, argc, argv, OFCommandLine::ExpandWildcards))
    {
      /* check exclusive options first */
      if (cmd.getParamCount() == 0)
      {
        if (cmd.findOption("--version"))
        {
            app.printHeader(OFTrue /*print host identifier*/);          // uses ofConsole.lockCerr()
            CERR << endl << "External libraries used:";
#ifdef WITH_ZLIB
            CERR << endl << "- ZLIB, Version " << zlibVersion() << endl;
#else
            CERR << " none" << endl;
#endif
            return 0;
         }
      }

      /* command line parameters and options */
      if (cmd.getParamCount() > 0) app.checkParam(cmd.getParamAndCheckMinMax(1, overridePort, 1, 65535));

      if (cmd.findOption("--verbose")) opt_verbose=1;
      if (cmd.findOption("--very-verbose")) opt_verbose=2;
      if (cmd.findOption("--debug"))
      {
        opt_debug = OFTrue;
        DUL_Debug(OFTrue);
        DIMSE_debug(OFTrue);
        DB_setDebugLevel(OFTrue);   /* DB debugging */
        SetDebugLevel(3);
      }
      if (cmd.findOption("--config")) app.checkValue(cmd.getValue(opt_configFileName));
#ifdef HAVE_FORK
      if (cmd.findOption("--single-process")) opt_singleProcess = OFTrue;
#endif

      if (cmd.findOption("--require-find")) opt_requireFindForMove = OFTrue;
      if (cmd.findOption("--no-parallel-store")) opt_refuseMultipleStorageAssociations = OFTrue;
      if (cmd.findOption("--disable-get")) opt_disableGetSupport = OFTrue;
      if (cmd.findOption("--allow-shutdown")) opt_allowShutdown = OFTrue;
      cmd.beginOptionBlock();
      if (cmd.findOption("--check-find")) opt_checkFindIdentifier = OFTrue;
      if (cmd.findOption("--no-check-find")) opt_checkFindIdentifier = OFFalse;
      cmd.endOptionBlock();
      cmd.beginOptionBlock();
      if (cmd.findOption("--check-move")) opt_checkMoveIdentifier = OFTrue;
      if (cmd.findOption("--no-check-move")) opt_checkMoveIdentifier = OFFalse;
      cmd.endOptionBlock();
      cmd.beginOptionBlock();
      if (cmd.findOption("--move-unrestricted"))
      {
    opt_restrictMoveToSameAE = OFFalse;
    opt_restrictMoveToSameHost = OFFalse;
    opt_restrictMoveToSameVendor = OFFalse;
      }
      if (cmd.findOption("--move-aetitle")) opt_restrictMoveToSameAE = OFTrue;
      if (cmd.findOption("--move-host")) opt_restrictMoveToSameHost = OFTrue;
      if (cmd.findOption("--move-vendor")) opt_restrictMoveToSameVendor = OFTrue;
      cmd.endOptionBlock();

      if (cmd.findOption("--no-patient-root")) opt_supportPatientRoot = OFFalse;
      if (cmd.findOption("--no-study-root")) opt_supportStudyRoot = OFFalse;
#ifndef NO_PATIENTSTUDYONLY_SUPPORT
      if (cmd.findOption("--no-patient-study")) opt_supportPatientStudyOnly = OFFalse;
#endif
      if ((!opt_supportPatientRoot)&&(!opt_supportStudyRoot)&&(!opt_supportPatientStudyOnly))
      {
        app.printError("cannot disable all Q/R models");
      }

      cmd.beginOptionBlock();
      if (cmd.findOption("--prefer-uncompr"))  opt_networkTransferSyntax = EXS_Unknown;
      if (cmd.findOption("--prefer-little"))   opt_networkTransferSyntax = EXS_LittleEndianExplicit;
      if (cmd.findOption("--prefer-big"))      opt_networkTransferSyntax = EXS_BigEndianExplicit;
      if (cmd.findOption("--implicit"))        opt_networkTransferSyntax = EXS_LittleEndianImplicit;
      cmd.endOptionBlock();

#ifdef WITH_TCPWRAPPER
      cmd.beginOptionBlock();
      if (cmd.findOption("--access-full")) dcmTCPWrapperDaemonName.set(NULL);
      if (cmd.findOption("--access-control")) dcmTCPWrapperDaemonName.set(OFFIS_CONSOLE_APPLICATION);
      cmd.endOptionBlock();
#endif

      if (cmd.findOption("--timeout"))
      {
        OFCmdSignedInt opt_timeout = 0;
        app.checkValue(cmd.getValueAndCheckMin(opt_timeout, 1));
        dcmConnectionTimeout.set((Sint32) opt_timeout);
      }

      if (cmd.findOption("--max-pdu")) app.checkValue(cmd.getValueAndCheckMinMax(overrideMaxPDU, ASC_MINIMUMPDUSIZE, ASC_MAXIMUMPDUSIZE));
      if (cmd.findOption("--disable-host-lookup")) dcmDisableGethostbyaddr.set(OFTrue);
      if (cmd.findOption("--refuse")) opt_refuse = OFTrue;
      if (cmd.findOption("--reject")) opt_rejectWhenNoImplementationClassUID = OFTrue;
      if (cmd.findOption("--ignore")) opt_ignoreStoreData = OFTrue;
      if (cmd.findOption("--uid-padding")) opt_correctUIDPadding = OFTrue;

      cmd.beginOptionBlock();
      if (cmd.findOption("--enable-new-vr"))
      {
        dcmEnableUnknownVRGeneration.set(OFTrue);
        dcmEnableUnlimitedTextVRGeneration.set(OFTrue);
      }
      if (cmd.findOption("--disable-new-vr"))
      {
        dcmEnableUnknownVRGeneration.set(OFFalse);
        dcmEnableUnlimitedTextVRGeneration.set(OFFalse);
      }
      cmd.endOptionBlock();

      cmd.beginOptionBlock();
      if (cmd.findOption("--normal")) opt_bitPreserving = OFFalse;
      if (cmd.findOption("--bit-preserving")) opt_bitPreserving = OFTrue;
      cmd.endOptionBlock();

      cmd.beginOptionBlock();
      if (cmd.findOption("--write-file")) opt_useMetaheader = OFTrue;
      if (cmd.findOption("--write-dataset")) opt_useMetaheader = OFFalse;
      cmd.endOptionBlock();

      cmd.beginOptionBlock();
      if (cmd.findOption("--write-xfer-same")) opt_writeTransferSyntax = EXS_Unknown;
      if (cmd.findOption("--write-xfer-little"))
      {
        app.checkConflict("--write-xfer-little", "--bit-preserving", opt_bitPreserving);
        opt_writeTransferSyntax = EXS_LittleEndianExplicit;
      }
      if (cmd.findOption("--write-xfer-big"))
      {
        app.checkConflict("--write-xfer-big", "--bit-preserving", opt_bitPreserving);
        opt_writeTransferSyntax = EXS_BigEndianExplicit;
      }
      if (cmd.findOption("--write-xfer-implicit"))
      {
        app.checkConflict("--write-xfer-implicit", "--bit-preserving", opt_bitPreserving);
        opt_writeTransferSyntax = EXS_LittleEndianImplicit;
      }
      cmd.endOptionBlock();


      cmd.beginOptionBlock();
      if (cmd.findOption("--group-length-recalc"))
      {
        app.checkConflict("--group-length-recalc", "--bit-preserving", opt_bitPreserving);
        opt_groupLength = EGL_recalcGL;
      }
      if (cmd.findOption("--group-length-create"))
      {
        app.checkConflict("--group-length-create", "--bit-preserving", opt_bitPreserving);
        opt_groupLength = EGL_withGL;
      }
      if (cmd.findOption("--group-length-remove"))
      {
        app.checkConflict("--group-length-remove", "--bit-preserving", opt_bitPreserving);
        opt_groupLength = EGL_withoutGL;
      }
      cmd.endOptionBlock();

      cmd.beginOptionBlock();
      if (cmd.findOption("--length-explicit"))
      {
        app.checkConflict("--length-explicit", "--bit-preserving", opt_bitPreserving);
        opt_sequenceType = EET_ExplicitLength;
      }
      if (cmd.findOption("--length-undefined"))
      {
        app.checkConflict("--length-undefined", "--bit-preserving", opt_bitPreserving);
        opt_sequenceType = EET_UndefinedLength;
      }
      cmd.endOptionBlock();

      cmd.beginOptionBlock();
      if (cmd.findOption("--padding-off")) opt_paddingType = EPD_withoutPadding;
      if (cmd.findOption("--padding-create"))
      {
        app.checkConflict("--padding-create", "--write-dataset", ! opt_useMetaheader);
        app.checkConflict("--padding-create", "--bit-preserving", opt_bitPreserving);
        app.checkValue(cmd.getValueAndCheckMin(opt_filepad, 0));
        app.checkValue(cmd.getValueAndCheckMin(opt_itempad, 0));
        opt_paddingType = EPD_withPadding;
      }
      cmd.endOptionBlock();

   }

    /* read config file */
    if (access(opt_configFileName, R_OK) < 0) {
    errmsg("cannot access %s: %s", opt_configFileName, strerror(errno));
    return 10;
    }
    if (!CNF_init(opt_configFileName)) {
    errmsg("bad config file: %s", opt_configFileName);
    return 10;
    }
    opt_maxAssociations = CNF_getMaxAssociations();

    opt_port = CNF_getNetworkTCPPort();
    if (opt_port == 0) opt_port = 104; /* not set, use default */
    if (overridePort > 0) opt_port = overridePort;

    opt_maxPDU = CNF_getMaxPDUSize();
    if (opt_maxPDU == 0) opt_maxPDU = ASC_DEFAULTMAXPDU; /* not set, use default */
    if (opt_maxPDU < ASC_MINIMUMPDUSIZE || opt_maxPDU > ASC_MAXIMUMPDUSIZE)
    {
      app.printError("invalid MaxPDUSize in config file");
    }
    if (overrideMaxPDU > 0) opt_maxPDU = overrideMaxPDU;

    DB_setIdentifierChecking(opt_checkFindIdentifier, opt_checkMoveIdentifier);

    /* make sure data dictionary is loaded */
    if (!dcmDataDict.isDictionaryLoaded()) {
    fprintf(stderr, "Warning: no data dictionary loaded, check environment variable: %s\n",
        DCM_DICT_ENVIRONMENT_VARIABLE);
    }

    processTable.pcnt = 0;
    processTable.plist = NULL;

#ifdef HAVE_GETEUID
    /* if port is privileged we must be as well */
    if (opt_port < 1024) {
        if (geteuid() != 0) {
        errmsg("cannot listen on port %d, insufficient privileges", (int)opt_port);
            return 10;
    }
    }
#endif

    cond = ASC_initializeNetwork(NET_ACCEPTORREQUESTOR, (int)opt_port, 10, &net);
    if (cond.bad()) {
    errmsg("Error initialising network:");
    DimseCondition::dump(cond);
        return 10;
    }

#if defined(HAVE_SETUID) && defined(HAVE_GETUID)
    /* return to normal uid so that we can't do too much damage in case
     * things go very wrong.   Only relevant if the program is setuid root,
     * and run by another user.  Running as root user may be
     * potentially disasterous if this program screws up badly.
     */
    setuid(getuid());
#endif

    /* loop waiting for associations */
    while (cond.good())
    {
      cond = waitForAssociation(net);
      if (!opt_singleProcess) cleanChildren();  /* clean up any child processes */
    }

    cond = ASC_dropNetwork(&net);
    if (cond.bad()) {
    errmsg("Error dropping network:");
    DimseCondition::dump(cond);
        return 10;
    }

#ifdef HAVE_WINSOCK_H
    WSACleanup();
#endif

    return 0;
}

/*
 * Process Table List Handling
 */

#ifdef HAVE_FORK

static void
addProcessToTable(int pid, T_ASC_Association * assoc)
{
    CTN_ProcessSlot *ps;
    int i;

    if ((ps = (CTN_ProcessSlot*)malloc(sizeof(CTN_ProcessSlot))) == NULL) {
    errmsg("out of memory");
    return;
    }
    /* fill in info */
    ASC_getPresentationAddresses(assoc->params, ps->peerName, NULL);
    ASC_getAPTitles(assoc->params, ps->callingAETitle,
    ps->calledAETitle, NULL);
    ps->processId = pid;
    ps->startTime = time(NULL);
    ps->hasStorageAbility = OFFalse;
    for (i=0; i<numberOfDcmStorageSOPClassUIDs; i++) {
    if (ASC_findAcceptedPresentationContextID(
        assoc, dcmStorageSOPClassUIDs[i])) {
        ps->hasStorageAbility = OFTrue;
        break;  /* out of for loop */
    }
    }
    /* add to start of list */
    processTable.pcnt++;
    ps->next = processTable.plist;
    processTable.plist = ps;
}

#endif

#if defined(HAVE_WAITPID) || defined(HAVE_WAIT3)

static void
removeProcessFromTable(int pid)
{
    CTN_ProcessSlot *ps, *pps;
    OFBool      found = OFFalse;

    pps = NULL;
    ps = processTable.plist;

    while (ps && !found) {
    found = (ps->processId == pid);
    if (!found) {
        pps = ps;
        ps = ps->next;
    }
    }

    if (!found) {
    errmsg("WARNING: process %d not found", pid);
    return;
    }

    if (pps == NULL) {
    /* first in list */
    processTable.plist = ps->next;
    } else {
    pps->next = ps->next;
    }

    free(ps);
    processTable.pcnt--;
}

#endif

static int
countChildProcesses()
{
    int n = 0;
    CTN_ProcessSlot *ps;

    ps = processTable.plist;
    while (ps) {
    n++;
    ps = ps->next;
    }
    return n;
}


#ifdef HAVE_WAITPID
static void
cleanChildren()
{
    int         options;
    int         stat_loc;
    int         child = 1;

    options = WNOHANG;
    while (child > 0) {
    child = (int)(waitpid(-1, &stat_loc, options));
    if (child == 0) {
        /* child not yet finished so don't complain */
    } else if (child < 0) {
        if (errno == ECHILD) {
        /* no children so don't complain */
        } else if (errno != 0)
        {
        errmsg("wait for child failed: %s", strerror(errno));
        }
    } else if (child > 0) {
        if (opt_verbose) {
        time_t t = time(NULL);
        printf("Cleaned up after child (%d) %s", child, ctime(&t));
        }
    /* Remove Entry from Process Table */
        removeProcessFromTable(child);
    }
    }
}
#elif HAVE_WAIT3
static void
cleanChildren()
{
#if defined(__NeXT__)
    /* some systems need a union wait as argument to wait3 */
    union wait          status;
#else
    int                 status;
#endif
    int                 options;
    struct rusage       rusage;
    int                 child = 1;

    options = WNOHANG;
    while (child > 0) {
    child = wait3(&status, options, &rusage);
    if (child < 0) {
        if (errno == ECHILD) {
        /* no children so don't complain */
        } else if (errno != 0)
        {
        errmsg("wait for child failed: %s", strerror(errno));
        }
    } else if (child > 0) {
        if (opt_verbose) {
        printf("Cleaned up after child (%d)\n", child);
        }
    /* Remove Entry from Process Table */
        removeProcessFromTable(child);
    }
    }
}
#else
/* don't know how to wait for my children */
static void
cleanChildren()
{
    errmsg("cannot wait for child processes");
}
#endif


/*
 * Refuse an Association
 */

static OFCondition
refuseAssociation(T_ASC_Association ** assoc, CTN_RefuseReason reason)
{
    OFCondition cond = EC_Normal;
    T_ASC_RejectParameters rej;

    if (opt_verbose) {
    printf("Refusing Association (");
    switch (reason) {
    case CTN_TooManyAssociations:
        printf("TooManyAssociations");
        break;
    case CTN_CannotFork:
        printf("CannotFork");
        break;
    case CTN_BadAppContext:
        printf("BadAppContext");
        break;
    case CTN_BadAEPeer:
        printf("BadAEPeer");
        break;
    case CTN_BadAEService:
        printf("BadAEService");
        break;
    case CTN_NoReason:
        printf("NoReason");
        break;
    default:
        printf("???");
        break;
    }
    printf(")\n");
    }
    switch (reason) {
    case CTN_TooManyAssociations:
    rej.result = ASC_RESULT_REJECTEDTRANSIENT;
    rej.source = ASC_SOURCE_SERVICEPROVIDER_PRESENTATION_RELATED;
    rej.reason = ASC_REASON_SP_PRES_LOCALLIMITEXCEEDED;
    break;
    case CTN_CannotFork:
    rej.result = ASC_RESULT_REJECTEDPERMANENT;
    rej.source = ASC_SOURCE_SERVICEPROVIDER_PRESENTATION_RELATED;
    rej.reason = ASC_REASON_SP_PRES_TEMPORARYCONGESTION;
    break;
    case CTN_BadAppContext:
    rej.result = ASC_RESULT_REJECTEDTRANSIENT;
    rej.source = ASC_SOURCE_SERVICEUSER;
    rej.reason = ASC_REASON_SU_APPCONTEXTNAMENOTSUPPORTED;
    break;
    case CTN_BadAEPeer:
    rej.result = ASC_RESULT_REJECTEDPERMANENT;
    rej.source = ASC_SOURCE_SERVICEUSER;
    rej.reason = ASC_REASON_SU_CALLINGAETITLENOTRECOGNIZED;
    break;
    case CTN_BadAEService:
    rej.result = ASC_RESULT_REJECTEDPERMANENT;
    rej.source = ASC_SOURCE_SERVICEUSER;
    rej.reason = ASC_REASON_SU_CALLEDAETITLENOTRECOGNIZED;
    break;
    case CTN_NoReason:
    default:
    rej.result = ASC_RESULT_REJECTEDPERMANENT;
    rej.source = ASC_SOURCE_SERVICEUSER;
    rej.reason = ASC_REASON_SU_NOREASON;
    break;
    }

    cond = ASC_rejectAssociation(*assoc, &rej);
    if (cond.bad()) {
    fprintf(stderr, "Association Reject Failed:\n");
    DimseCondition::dump(cond);
    }

    cond = ASC_dropAssociation(*assoc);
    if (cond.bad()) {
    fprintf(stderr, "Cannot Drop Association:\n");
    DimseCondition::dump(cond);
    }
    cond = ASC_destroyAssociation(assoc);
    if (cond.bad()) {
    fprintf(stderr, "Cannot Destroy Association:\n");
    DimseCondition::dump(cond);
    }

    return cond;
}

/*
 * Wait for and negotiate incoming association.
 */

static OFCondition
waitForAssociation(T_ASC_Network * theNet)
{
    OFCondition cond = EC_Normal;
#ifdef HAVE_FORK
    int                 pid;
#endif
    T_ASC_Association  *assoc;
    char                buf[BUFSIZ];
    int timeout;
    OFBool go_cleanup = OFFalse;

    if (opt_singleProcess) timeout = 1000;
    else
    {
      if (countChildProcesses() > 0)
      {
        timeout = 1;
      } else {
        timeout = 1000;
      }
    }

    if (ASC_associationWaiting(theNet, timeout))
    {
        cond = ASC_receiveAssociation(theNet, &assoc, (int)opt_maxPDU);
        if (cond.bad())
        {
          if (opt_verbose)
          {
            errmsg("Failed to receive association:");
            DimseCondition::dump(cond);
          }
          go_cleanup = OFTrue;
        }
    } else return EC_Normal;

    if (! go_cleanup)
    {
        if (opt_verbose)
        {
            time_t t = time(NULL);
            printf("Association Received (%s:%s -> %s) %s",
               assoc->params->DULparams.callingPresentationAddress,
               assoc->params->DULparams.callingAPTitle,
               assoc->params->DULparams.calledAPTitle,
               ctime(&t));
        }

        if (opt_debug)
        {
          printf("Parameters:\n");
          ASC_dumpParameters(assoc->params, COUT);
        }

        if (opt_refuse)
        {
            if (opt_verbose)
            {
                printf("Refusing Association (forced via command line)\n");
            }
            cond = refuseAssociation(&assoc, CTN_NoReason);
            go_cleanup = OFTrue;
        }
    }

    if (! go_cleanup)
    {
        /* Application Context Name */
        cond = ASC_getApplicationContextName(assoc->params, buf);
        if (cond.bad() || strcmp(buf, DICOM_STDAPPLICATIONCONTEXT) != 0)
        {
            /* reject: the application context name is not supported */
            if (opt_verbose)
            {
                errmsg("Bad AppContextName: %s", buf);
            }
            cond = refuseAssociation(&assoc, CTN_BadAppContext);
            go_cleanup = OFTrue;
        }
    }

    if (! go_cleanup)
    {
        /* Implementation Class UID */
        if (opt_rejectWhenNoImplementationClassUID &&
        strlen(assoc->params->theirImplementationClassUID) == 0)
        {
            /* reject: no implementation Class UID provided */
            if (opt_verbose)
            {
                errmsg("No implementation Class UID provided");
            }
            cond = refuseAssociation(&assoc, CTN_NoReason);
            go_cleanup = OFTrue;
        }
    }

    if (! go_cleanup)
    {
        /* Does peer AE have access to required service ?? */
        if (! CNF_peerInAETitle(assoc->params->DULparams.calledAPTitle,
        assoc->params->DULparams.callingAPTitle,
        assoc->params->DULparams.callingPresentationAddress))
        {
            cond = refuseAssociation(&assoc, CTN_BadAEService);
            go_cleanup = OFTrue;
        }
    }

    if (! go_cleanup)
    {
        // too many concurrent associations ??
        if (countChildProcesses() >= opt_maxAssociations)
        {
            cond = refuseAssociation(&assoc, CTN_TooManyAssociations);
            go_cleanup = OFTrue;
        }
    }

    if (! go_cleanup)
    {
        cond = negotiateAssociation(assoc);
        if (cond.bad()) go_cleanup = OFTrue;
    }

    if (! go_cleanup)
    {
        cond = ASC_acknowledgeAssociation(assoc);
        if (cond.bad())
        {
            DimseCondition::dump(cond);
            go_cleanup = OFTrue;
        }
    }

    if (! go_cleanup)
    {

        if (opt_verbose)
        {
            printf("Association Acknowledged (Max Send PDV: %lu)\n",
                   assoc->sendPDVLength);
            if (ASC_countAcceptedPresentationContexts(assoc->params) == 0)
                printf("    (but no valid presentation contexts)\n");
            if (opt_debug)
                ASC_dumpParameters(assoc->params, COUT);
        }

        if (opt_singleProcess)
        {
            /* don't spawn a sub-process to handle the association */
            cond = SCE_handleAssociation(assoc, opt_correctUIDPadding);
        }
#ifdef HAVE_FORK
        else
        {
            /* spawn a sub-process to handle the association */
            pid = (int)(fork());
            if (pid < 0)
            {
                errmsg("Cannot create association sub-process: %s",
                   strerror(errno));
                cond = refuseAssociation(&assoc, CTN_CannotFork);
                go_cleanup = OFTrue;
            }
            else if (pid > 0)
            {
                /* parent process, note process in table */
                addProcessToTable(pid, assoc);
            }
            else
            {
                /* child process, handle the association */
                cond = SCE_handleAssociation(assoc, opt_correctUIDPadding);
                /* the child process is done so exit */
                exit(0);
            }
        }
#endif
    }

    // cleanup code
    OFCondition oldcond = cond;    /* store condition flag for later use */
    if (!opt_singleProcess && (cond != ASC_SHUTDOWNAPPLICATION))
    {
        /* the child will handle the association, we can drop it */
        cond = ASC_dropAssociation(assoc);
        if (cond.bad())
        {
            errmsg("Cannot Drop Association:");
            DimseCondition::dump(cond);
        }
        cond = ASC_destroyAssociation(&assoc);
        if (cond.bad())
        {
            errmsg("Cannot Destroy Association:");
            DimseCondition::dump(cond);
        }
    }

    if (oldcond == ASC_SHUTDOWNAPPLICATION) cond = oldcond; /* abort flag is reported to top-level wait loop */
    return cond;
}

static void
refuseAnyStorageContexts(T_ASC_Association * assoc)
{
    int i;
    T_ASC_PresentationContextID pid;

    for (i = 0; i < numberOfDcmStorageSOPClassUIDs; i++) {
    pid = ASC_findAcceptedPresentationContextID(assoc,
                            dcmStorageSOPClassUIDs[i]);
    if (pid != 0) {
        /* refuse */
        ASC_refusePresentationContext(assoc->params,
                      pid, ASC_P_USERREJECTION);
    }
    }
}

static OFBool
writableStorageArea(const char *aeTitle)
{
    const char *axs;
    axs = CNF_getAccess((char*)aeTitle);
    if (strcmp(axs, "RW") == 0) return OFTrue;
    if (strcmp(axs, "WR") == 0) return OFTrue;
    if (strcmp(axs, "W") == 0) return OFTrue;
    return OFFalse;
}

static OFCondition
negotiateAssociation(T_ASC_Association * assoc)
{
    OFCondition cond = EC_Normal;
    int i;
    T_ASC_PresentationContextID movepid, findpid;
    struct { const char *moveSyntax, *findSyntax; } queryRetrievePairs[] =
    {
    { UID_MOVEPatientRootQueryRetrieveInformationModel,
      UID_FINDPatientRootQueryRetrieveInformationModel },
    { UID_MOVEStudyRootQueryRetrieveInformationModel,
      UID_FINDStudyRootQueryRetrieveInformationModel },
    { UID_MOVEPatientStudyOnlyQueryRetrieveInformationModel,
      UID_FINDPatientStudyOnlyQueryRetrieveInformationModel}
    };
    CTN_ProcessSlot *ps;
    DIC_AE calledAETitle;
    ASC_getAPTitles(assoc->params, NULL, calledAETitle, NULL);

    const char* transferSyntaxes[] = { NULL, NULL, NULL };
    int numTransferSyntaxes = 0;

    switch (opt_networkTransferSyntax)
    {
      case EXS_LittleEndianImplicit:
        /* we only support Little Endian Implicit */
        transferSyntaxes[0]  = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 1;
        break;
      case EXS_LittleEndianExplicit:
        /* we prefer Little Endian Explicit */
        transferSyntaxes[0] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[1] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[2]  = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 3;
        break;
      case EXS_BigEndianExplicit:
        /* we prefer Big Endian Explicit */
        transferSyntaxes[0] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[2]  = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 3;
        break;
      default:
        /* We prefer explicit transfer syntaxes.
         * If we are running on a Little Endian machine we prefer
         * LittleEndianExplicitTransferSyntax to BigEndianTransferSyntax.
         */
        if (gLocalByteOrder == EBO_LittleEndian)  /* defined in dcxfer.h */
        {
          transferSyntaxes[0] = UID_LittleEndianExplicitTransferSyntax;
          transferSyntaxes[1] = UID_BigEndianExplicitTransferSyntax;
        } else {
          transferSyntaxes[0] = UID_BigEndianExplicitTransferSyntax;
          transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        }
        transferSyntaxes[2] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 3;
        break;
    }

    /*
     * Support for some Q/R models can be disabled from the command line.
     */
    const char *selectedNonStorageSyntaxes[DIM_OF(nonStorageSyntaxes)];
    int numberOfSelectedNonStorageSyntaxes = 0;
    for (i=0; i<numberOfNonStorageSyntaxes; i++)
    {
        if (0 == strcmp(nonStorageSyntaxes[i], UID_FINDPatientRootQueryRetrieveInformationModel))
        {
          if (opt_supportPatientRoot) selectedNonStorageSyntaxes[numberOfSelectedNonStorageSyntaxes++] = nonStorageSyntaxes[i];
        }
        else if (0 == strcmp(nonStorageSyntaxes[i], UID_MOVEPatientRootQueryRetrieveInformationModel))
        {
          if (opt_supportPatientRoot) selectedNonStorageSyntaxes[numberOfSelectedNonStorageSyntaxes++] = nonStorageSyntaxes[i];
        }
        else if (0 == strcmp(nonStorageSyntaxes[i], UID_GETPatientRootQueryRetrieveInformationModel))
        {
          if (opt_supportPatientRoot && (! opt_disableGetSupport)) selectedNonStorageSyntaxes[numberOfSelectedNonStorageSyntaxes++] = nonStorageSyntaxes[i];
        }
        else if (0 == strcmp(nonStorageSyntaxes[i], UID_FINDPatientStudyOnlyQueryRetrieveInformationModel))
        {
          if (opt_supportPatientStudyOnly) selectedNonStorageSyntaxes[numberOfSelectedNonStorageSyntaxes++] = nonStorageSyntaxes[i];
        }
        else if (0 == strcmp(nonStorageSyntaxes[i], UID_MOVEPatientStudyOnlyQueryRetrieveInformationModel))
        {
          if (opt_supportPatientStudyOnly) selectedNonStorageSyntaxes[numberOfSelectedNonStorageSyntaxes++] = nonStorageSyntaxes[i];
        }
        else if (0 == strcmp(nonStorageSyntaxes[i], UID_GETPatientStudyOnlyQueryRetrieveInformationModel))
        {
          if (opt_supportPatientStudyOnly && (! opt_disableGetSupport)) selectedNonStorageSyntaxes[numberOfSelectedNonStorageSyntaxes++] = nonStorageSyntaxes[i];
        }
        else if (0 == strcmp(nonStorageSyntaxes[i], UID_FINDStudyRootQueryRetrieveInformationModel))
        {
          if (opt_supportStudyRoot) selectedNonStorageSyntaxes[numberOfSelectedNonStorageSyntaxes++] = nonStorageSyntaxes[i];
        }
        else if (0 == strcmp(nonStorageSyntaxes[i], UID_MOVEStudyRootQueryRetrieveInformationModel))
        {
          if (opt_supportStudyRoot) selectedNonStorageSyntaxes[numberOfSelectedNonStorageSyntaxes++] = nonStorageSyntaxes[i];
        }
        else if (0 == strcmp(nonStorageSyntaxes[i], UID_GETStudyRootQueryRetrieveInformationModel))
        {
          if (opt_supportStudyRoot && (! opt_disableGetSupport)) selectedNonStorageSyntaxes[numberOfSelectedNonStorageSyntaxes++] = nonStorageSyntaxes[i];
        }
        else if (0 == strcmp(nonStorageSyntaxes[i], UID_PrivateShutdownSOPClass))
        {
          if (opt_allowShutdown) selectedNonStorageSyntaxes[numberOfSelectedNonStorageSyntaxes++] = nonStorageSyntaxes[i];
        } else {
            selectedNonStorageSyntaxes[numberOfSelectedNonStorageSyntaxes++] = nonStorageSyntaxes[i];
        }
    }

    /*  accept any of the non-storage syntaxes */
    cond = ASC_acceptContextsWithPreferredTransferSyntaxes(
    assoc->params,
    (const char**)selectedNonStorageSyntaxes, numberOfSelectedNonStorageSyntaxes,
    (const char**)transferSyntaxes, numTransferSyntaxes);
    if (cond.bad()) {
    errmsg("Cannot accept presentation contexts:");
    DimseCondition::dump(cond);
    }

    /*  accept any of the storage syntaxes */
    if (opt_disableGetSupport)
    {
      /* accept storage syntaxes with default role only */
      cond = ASC_acceptContextsWithPreferredTransferSyntaxes(
        assoc->params,
        dcmStorageSOPClassUIDs, numberOfDcmStorageSOPClassUIDs,
        (const char**)transferSyntaxes, DIM_OF(transferSyntaxes));
      if (cond.bad()) {
        errmsg("Cannot accept presentation contexts:");
        DimseCondition::dump(cond);
      }
    } else {
      /* accept storage syntaxes with proposed role */
      T_ASC_PresentationContext pc;
      T_ASC_SC_ROLE role;
      int npc = ASC_countPresentationContexts(assoc->params);
      for (i=0; i<npc; i++)
      {
        ASC_getPresentationContext(assoc->params, i, &pc);
        if (dcmIsaStorageSOPClassUID(pc.abstractSyntax))
        {
          /*
          ** We are prepared to accept whatever role he proposes.
          ** Normally we can be the SCP of the Storage Service Class.
          ** When processing the C-GET operation we can be the SCU of the Storage Service Class.
          */
          role = pc.proposedRole;

          /*
          ** Accept in the order "least wanted" to "most wanted" transfer
          ** syntax.  Accepting a transfer syntax will override previously
          ** accepted transfer syntaxes.
          */
          for (int k=numTransferSyntaxes-1; k>=0; k--)
          {
            for (int j=0; j < (int)pc.transferSyntaxCount; j++)
            {
              /* if the transfer syntax was proposed then we can accept it
               * appears in our supported list of transfer syntaxes
               */
              if (strcmp(pc.proposedTransferSyntaxes[j], transferSyntaxes[k]) == 0)
              {
                cond = ASC_acceptPresentationContext(
                    assoc->params, pc.presentationContextID, transferSyntaxes[k], role);
                if (cond.bad()) return cond;
              }
            }
          }
        }
      } /* for */
    } /* else */

    /*
     * check if we have negotiated the private "shutdown" SOP Class
     */
    if (0 != ASC_findAcceptedPresentationContextID(assoc, UID_PrivateShutdownSOPClass))
    {
      if (opt_verbose) {
        printf("Shutting down server ... (negotiated private \"shut down\" SOP class)\n");
      }
      refuseAssociation(&assoc, CTN_NoReason);
      return ASC_SHUTDOWNAPPLICATION;
    }

    /*
     * Refuse any "Storage" presentation contexts to non-writable
     * storage areas.
     */
    if (!writableStorageArea(calledAETitle)) {
    refuseAnyStorageContexts(assoc);
    }

    /*
     * Enforce RSNA'93 Demonstration Requirements about only
     * accepting a context for MOVE if a context for FIND is also present.
     */

    for (i=0; i<(int)DIM_OF(queryRetrievePairs); i++) {
        movepid = ASC_findAcceptedPresentationContextID(assoc,
        queryRetrievePairs[i].moveSyntax);
        if (movepid != 0) {
        findpid = ASC_findAcceptedPresentationContextID(assoc,
            queryRetrievePairs[i].findSyntax);
        if (findpid == 0) {
        if (opt_requireFindForMove) {
            /* refuse the move */
            ASC_refusePresentationContext(assoc->params,
                movepid, ASC_P_USERREJECTION);
            } else {
            errmsg("WARNING: Move PresCtx but no Find (accepting for now)");
        }
        }
        }
    }

    /*
     * Enforce an Ad-Hoc rule to limit storage access.
     * If the storage area is "writable" and some other association has
     * already negotiated a "Storage" class presentation context,
     * then refuse any "storage" presentation contexts.
     */

    if (opt_refuseMultipleStorageAssociations) {
        if (writableStorageArea(calledAETitle)) {
        ps = processTable.plist;
        while (ps != NULL) {
            if (strcmp(calledAETitle, ps->calledAETitle) == 0 &&
            ps->hasStorageAbility) {
            refuseAnyStorageContexts(assoc);
            break;
            }
            ps = ps->next;
        }
        }
    }

    return cond;
}


/*
** CVS Log
** $Log: imagectn.cc,v $
** Revision 1.1  2005/08/23 19:32:03  braindead
** - initial savannah import
**
** Revision 1.1  2005/06/26 19:26:14  pipelka
** - added dcmtk
**
** Revision 1.45  2004/02/10 15:38:39  joergr
** Renamed configuration file from "configrc" to "imagectn.cfg".
**
** Revision 1.44  2003/08/14 09:00:39  meichel
** Minor code modifications to avoid warnings about unused code on MinGW
**
** Revision 1.43  2003/06/10 13:43:47  meichel
** Added support for TCP wrappers based host access control
**
** Revision 1.42  2002/11/29 13:00:38  meichel
** Introduced new command line option --timeout for controlling the
**   connection request timeout.
**
** Revision 1.41  2002/11/27 13:26:59  meichel
** Adapted module imagectn to use of new header file ofstdinc.h
**
** Revision 1.40  2002/11/26 08:43:50  meichel
** Replaced all includes for "zlib.h" with <zlib.h>
**   to avoid inclusion of zlib.h in the makefile dependencies.
**
** Revision 1.39  2002/11/25 18:01:12  meichel
** Converted compile time option to leniently handle space padded UIDs
**   in the Storage Service Class into command line / config file option.
**
** Revision 1.38  2002/09/23 18:02:56  joergr
** Added new command line option "--version" which prints the name and version
** number of external libraries used (incl. preparation for future support of
** 'config.guess' host identifiers).
**
** Revision 1.37  2001/12/19 09:51:17  meichel
** Restructured code (removed gotos) to avoid "sorry, not implemented" error
**   on Sun CC 2.0.1
**
** Revision 1.36  2001/11/12 14:54:19  meichel
** Removed all ctndisp related code from imagectn
**
** Revision 1.35  2001/11/09 15:59:40  joergr
** Renamed some of the getValue/getParam methods to avoid ambiguities reported
** by certain compilers.
**
** Revision 1.34  2001/10/12 12:42:50  meichel
** Adapted imagectn to OFCondition based dcmnet module (supports strict mode).
**
** Revision 1.33  2001/06/01 15:51:17  meichel
** Updated copyright header
**
** Revision 1.32  2001/06/01 11:02:03  meichel
** Implemented global flag and command line option to disable reverse
**   DNS hostname lookup using gethostbyaddr when accepting associations.
**
** Revision 1.31  2000/12/12 17:45:22  joergr
** Added '#include <libc.h>' to avoid problems with gcc 2.5.8 on NeXTSTEP 3.x
** systems.
**
** Revision 1.30  2000/12/11 18:17:19  joergr
** Added explicit typecast to keep SunCC 2.0.1 quiet.
**
** Revision 1.29  2000/10/16 11:32:00  joergr
** Added check to avoid wrong warning messages when shutting down application
** externally.
**
** Revision 1.28  2000/06/07 15:18:46  meichel
** Output stream now passed as mandatory parameter to ASC_dumpParameters.
**
** Revision 1.27  2000/05/30 13:21:54  joergr
** Added support for external shutdown of the application (this feature is
** enabled via a command line option, default is disabled).
** Moved parts of unused code to avoid compiler warnings when compiling with
** #define NODISPLAY.
**
** Revision 1.26  2000/04/14 16:38:03  meichel
** Global VR generation flags are now derived from OFGlobal and, thus,
**   safe for use in multi-thread applications.
**
** Revision 1.25  2000/03/08 16:40:57  meichel
** Updated copyright header.
**
** Revision 1.24  2000/02/29 11:57:54  meichel
** Removed support for VS value representation. This was proposed in CP 101
**   but never became part of the standard.
**
** Revision 1.23  2000/02/23 15:13:04  meichel
** Corrected macro for Borland C++ Builder 4 workaround.
**
** Revision 1.22  2000/02/01 11:43:38  meichel
** Avoiding to include <stdlib.h> as extern "C" on Borland C++ Builder 4,
**   workaround for bug in compiler header files.
**
** Revision 1.21  1999/06/10 12:11:53  meichel
** Adapted imagectn to new command line option scheme.
**   Added support for Patient/Study Only Q/R model and C-GET (experimental).
**
** Revision 1.20  1998/02/06 15:07:33  meichel
** Removed many minor problems (name clashes, unreached code)
**   reported by Sun CC4 with "+w" or Sun CC2.
**
** Revision 1.19  1998/01/14 14:29:02  hewett
** Modified existing -u command line option to also disable generation
** of UT and VS (previously just disabled generation of UN).
**
** Revision 1.18  1997/09/18 08:11:02  meichel
** Many minor type conflicts (e.g. long passed as int) solved.
**
** Revision 1.17  1997/08/26 14:17:18  hewett
** Added +B command line option to imagectn application.  Use of this option
** causes imagectn to bypass the dcmdata encode/decode routines when receiving
** images and write image data to disk exactly as received in a C-STORE
** command over the network.  This option does _not_ affect sending images.
**
** Revision 1.16  1997/08/05 07:46:32  andreas
** - Change needed version number of WINSOCK to 1.1
**   to support WINDOWS 95
**
** Revision 1.15  1997/07/21 08:59:42  andreas
** - Replace all boolean types (BOOLEAN, CTNBOOLEAN, DICOM_BOOL, BOOL)
**   with one unique boolean type OFBool.
**
** Revision 1.14  1997/07/04 09:24:59  meichel
** Simplified some sizeof() constructs to avoid compiler warnings
**   on the IBM xlC compiler (AIX 3.x).
**
** Revision 1.13  1997/06/26 12:54:05  andreas
** - Include Additional headers (winsock.h, io.h) for Windows NT/95
** - Include tests for changing of user IDs and the using of fork
**   in code since Windows NT/95 do not support this
**
** Revision 1.12  1997/05/29 17:06:34  meichel
** All dcmtk applications now contain a version string
** which is displayed with the command line options ("usage" message)
** and which can be queried in the binary with the "ident" command.
**
** Revision 1.11  1997/05/22 13:31:39  hewett
** Modified the test for presence of a data dictionary to use the
** method DcmDataDictionary::isDictionaryLoaded().
**
** Revision 1.10  1997/03/27 16:11:39  hewett
** Added command line switches allowing generation of UN to
** be disabled (it is enabled by default).
**
** Revision 1.9  1997/02/06 12:24:45  hewett
** Added code to print the time when an association request
** is received.
**
** Revision 1.8  1996/09/27 08:46:19  hewett
** Enclosed system include files with BEGIN_EXTERN_C/END_EXTERN_C.
**
** Revision 1.7  1996/09/24 15:52:34  hewett
** Now uses global table of Storage SOP Class UIDs (from dcuid.h).
** Also added preliminary support for the Macintosh environment (GUSI library).
**
** Revision 1.6  1996/05/30 17:41:26  hewett
** Added command line argument (-DD) to explicitly disable communication
** with ctndisp and override the configuration file.
**
** Revision 1.5  1996/05/06 07:33:13  hewett
** Rearranged handing of display disabling code.
**
** Revision 1.4  1996/04/25 16:28:09  hewett
** Reorded condition compiling for cleanChildren() function.  Code now
** prefers to use waitpid() rather than wait3() if available.
**
** Revision 1.3  1996/04/22 16:33:02  hewett
** Removed calls to DSU_setTraceLevel().
**
** Revision 1.2  1996/04/22 10:19:33  hewett
** Added command line flags to restrict move destination.  Added check for
** presence of a loaded data dictionary.  A global network is now created
** in acceptor/requestor mode.
**
** Revision 1.1.1.1  1996/03/28 19:24:59  hewett
** Oldenburg Image CTN Software ported to use the dcmdata C++ toolkit.
**
*/
