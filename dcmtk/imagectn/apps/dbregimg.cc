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
 *  Purpose: This test program registers image files in the image database.
 *
 *  Last Update:      $Author: braindead $
 *  Update Date:      $Date: 2005/08/23 19:32:03 $
 *  Source File:      $Source: /cvsroot/aeskulap/aeskulap/dcmtk/imagectn/apps/dbregimg.cc,v $
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
#include "ofstdinc.h"

#include "dicom.h"
#include "imagedb.h"
#include "diutil.h"
#include "dcdebug.h"
#include "dcompat.h"
#include "dcdict.h"
#include "cmdlnarg.h"
#include "dcuid.h"       /* for dcmtk version name */
#include "ofconapp.h"
#include "ofcmdln.h"

#ifdef WITH_ZLIB
#include <zlib.h>        /* for zlibVersion() */
#endif

#define OFFIS_CONSOLE_APPLICATION "dbregimg"

static char rcsid[] = "$dcmtk: " OFFIS_CONSOLE_APPLICATION " v"
  OFFIS_DCMTK_VERSION " " OFFIS_DCMTK_RELEASEDATE " $";


// ********************************************


#define SHORTCOL 2
#define LONGCOL  9


int main (int argc, char *argv[])
{
    DB_Handle *hdl = NULL;
    char sclass [120] ;
    char sinst  [120] ;
#ifdef DEBUG
    char fname  [120] ;
#endif
    DB_Status status;

    const char *opt_storageArea = NULL;
    OFBool opt_debug = OFFalse;
    OFBool opt_verbose = OFFalse;
    OFBool opt_print = OFFalse;
    OFBool opt_isNewFlag = OFTrue;

    SetDebugLevel(( 0 ));

    OFCommandLine cmd;
    OFConsoleApplication app(OFFIS_CONSOLE_APPLICATION, "Register a DICOM image file in an image database index file", rcsid);
    cmd.setOptionColumns(LONGCOL, SHORTCOL);
    cmd.setParamColumn(LONGCOL + SHORTCOL + 2);

    cmd.addParam("index-out",         "storage area for the index file (directory)");
    cmd.addParam("dcmimg-in",         "DICOM image file to be registered in the index file", OFCmdParam::PM_MultiOptional);

    cmd.addGroup("options:", LONGCOL, SHORTCOL);
     cmd.addOption("--help",    "-h", "print this help text and exit");
     cmd.addOption("--version",       "print version information and exit", OFTrue /* exclusive */);
     cmd.addOption("--verbose", "-v", "verbose mode, print processing details");
     cmd.addOption("--debug",   "-d", "debug mode, print debug information");
     cmd.addOption("--print",   "-p", "list contents of database index file");
     cmd.addOption("--not-new", "-n", "set instance reviewed status to 'not new'");

#ifdef HAVE_GUSI_H
    /* needed for Macintosh */
    GUSISetup(GUSIwithSIOUXSockets);
    GUSISetup(GUSIwithInternetSockets);
#endif

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
        cmd.getParam(1, opt_storageArea);

        if (cmd.findOption("--verbose"))
            opt_verbose = OFTrue;
        if (cmd.findOption("--debug"))
        {
            SetDebugLevel(3);
            DB_setDebugLevel(3);
            opt_debug = OFTrue;
        }
        if (cmd.findOption("--print"))
            opt_print = OFTrue;

        if (cmd.findOption("--not-new"))
            opt_isNewFlag = OFFalse;
    }

    /* make sure data dictionary is loaded */
    if (!dcmDataDict.isDictionaryLoaded())
        fprintf(stderr, "Warning: no data dictionary loaded, check environment variable: %s\n", DCM_DICT_ENVIRONMENT_VARIABLE);

    DB_enableQuotaSystem(OFFalse); /* disable deletion of images */

    if (DB_createHandle(opt_storageArea, DB_UpperMaxStudies, DB_UpperMaxBytesPerStudy, &hdl).good())
    {
        int paramCount = cmd.getParamCount();
        for (int param = 2; param <= paramCount; param++)
        {
            const char *opt_imageFile = NULL;
            cmd.getParam(param, opt_imageFile);
            if (access(opt_imageFile, R_OK) < 0)
                fprintf(stderr, "cannot access: %s\n", opt_imageFile);
            else
            {
                if (opt_verbose)
                    printf("registering: %s\n", opt_imageFile);
                if (DU_findSOPClassAndInstanceInFile(opt_imageFile, sclass, sinst))
                {
#ifdef DEBUG
                    if (DB_getDebugLevel() > 0)
                    {
                        /*** Test what filename is recommended by DB_Module **/
                        DB_makeNewStoreFileName (hdl, sclass, sinst, fname) ;
                        printf("DB_Module recommends %s for filename\n", fname) ;
                    }
#endif
                    DB_storeRequest(hdl, sclass, sinst, opt_imageFile, &status, opt_isNewFlag) ;
                } else
                    fprintf(stderr, "%s: cannot load dicom file: %s\n", OFFIS_CONSOLE_APPLICATION, opt_imageFile);
            }
        }
        DB_destroyHandle (&hdl);
        if (opt_print)
        {
            printf("-- DB Index File --\n");
            DB_PrintIndexFile((char *)opt_storageArea);
        }
        return 0;
    }
    DB_destroyHandle (&hdl);

    return 1;
}


/*
 * CVS Log
 * $Log: dbregimg.cc,v $
 * Revision 1.1  2005/08/23 19:32:03  braindead
 * - initial savannah import
 *
 * Revision 1.1  2005/06/26 19:26:14  pipelka
 * - added dcmtk
 *
 * Revision 1.1  2004/02/10 15:39:52  joergr
 * Moved dbregimg from folder "tests" to "apps".
 *
 * Revision 1.21  2002/11/27 13:27:56  meichel
 * Adapted module imagectn to use of new header file ofstdinc.h
 *
 * Revision 1.20  2002/11/26 08:43:56  meichel
 * Replaced all includes for "zlib.h" with <zlib.h>
 *   to avoid inclusion of zlib.h in the makefile dependencies.
 *
 * Revision 1.19  2002/09/23 18:03:57  joergr
 * Added new command line option "--version" which prints the name and version
 * number of external libraries used (incl. preparation for future support of
 * 'config.guess' host identifiers).
 *
 * Revision 1.18  2001/10/12 12:43:12  meichel
 * Adapted imagectn to OFCondition based dcmnet module (supports strict mode).
 *
 * Revision 1.17  2001/06/01 15:51:30  meichel
 * Updated copyright header
 *
 * Revision 1.16  2000/11/23 16:41:01  joergr
 * Added new command line option to dbregimg allowing to specify whether
 * instance reviewed status of newly registered objects should be set to 'new'
 * or 'not new'.
 *
 * Revision 1.15  2000/07/04 09:13:02  joergr
 * Added test whether database index file can be opened/created to avoid
 * application crashes.
 *
 * Revision 1.14  2000/03/08 16:41:11  meichel
 * Updated copyright header.
 *
 * Revision 1.13  2000/02/23 15:13:40  meichel
 * Corrected macro for Borland C++ Builder 4 workaround.
 *
 * Revision 1.12  2000/02/01 11:43:47  meichel
 * Avoiding to include <stdlib.h> as extern "C" on Borland C++ Builder 4,
 *   workaround for bug in compiler header files.
 *
 * Revision 1.11  1999/10/21 15:33:39  joergr
 * Adapted command line tool "dbregimg" to OFCommandLine class (provides
 * support for wildcard expansion under non-UNIX environments).
 *
 * Revision 1.10  1999/06/10 12:12:20  meichel
 * Adapted imagectn to new command line option scheme.
 *   Added support for Patient/Study Only Q/R model and C-GET (experimental).
 *
 */
