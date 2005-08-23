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
 *  Purpose: TI Common Constants, Types, Globals and Functions
 *
 *  Last Update:      $Author: braindead $
 *  Update Date:      $Date: 2005/08/23 19:32:03 $
 *  Source File:      $Source: /cvsroot/aeskulap/aeskulap/dcmtk/imagectn/apps/ti.h,v $
 *  CVS/RCS Revision: $Revision: 1.1 $
 *  Status:           $State: Exp $
 *
 *  CVS/RCS Log at end of file
 *
 */

#ifndef TI_H
#define TI_H

#include "osconfig.h"    /* make sure OS specific configuration is included first */

#include "dicom.h"
#include "cond.h"
#include "assoc.h"
#include "dimse.h"
#include "ofcmdln.h"
#include "cnf.h"
#include "imagedb.h"

/*
 * Constants
 */

#define TI_MAXPEERS       100
#define TI_MAXDATABASES   100
#define TI_MAXSTUDIES    1000
#define TI_MAXSERIES      500
#define TI_MAXIMAGES     1000

/*
 * Type definitions
 */

typedef struct {
    DIC_UI  sopInstanceUID;
    DIC_IS  imageNumber;
    int   intImageNumber;
} TI_ImageEntry;

typedef struct {
    DIC_UI  seriesInstanceUID;
    DIC_IS  seriesNumber;
    int   intSeriesNumber;
    DIC_CS  modality;
    TI_ImageEntry *images[TI_MAXIMAGES];  /* array of image pointers */
    int     imageCount;

    time_t lastQueryTime; /* time we last queried db */
} TI_SeriesEntry;

typedef struct {
    DIC_UI  studyInstanceUID;
    DIC_CS  studyID;
    DIC_PN  patientsName;
    DIC_LO  patientID;
    TI_SeriesEntry  *series[TI_MAXSERIES];  /* array of series pointers */
    int     seriesCount;
    time_t lastQueryTime; /* time we last queried db */
} TI_StudyEntry;


typedef struct {
    const char *title;  /* the CTN AE Title associated with this DB */

    const char **peerTitles;  /* peer titles which can read this database
       * and thus we can comminicate with */
    int peerTitleCount; /* number of peer titles */

    DB_Handle *dbHandle;  /* handle to current db */

    TI_StudyEntry *studies[TI_MAXSTUDIES]; /* array of study pointers */
    int     studyCount;

    int currentStudy; /* index of current study */
    int currentSeries;  /* index of current series in current study */
    int currentImage; /* index of current image in current study */

    time_t lastQueryTime; /* time we last queried db */

    OFBool isRemoteDB;  /* true if DB is remote */

} TI_DBEntry;

typedef struct {
    TI_DBEntry **dbEntries; /* the CTN databases we know */
    int dbCount;    /* number of entries in databases we know */

    const char *peerHostName;   /* peer to talk to */
    const char *peerNames[TI_MAXPEERS];
    int peerNamesCount;

    const char *myAETitle;  /* my application entity title */

    T_ASC_Network *net;   /* active network */
    T_ASC_Association *assoc; /* currently active association */

    OFCmdUnsignedInt maxReceivePDULength; /* number of bytes we can receive */

    int currentdb;    /* current database index */
    const char *currentPeerTitle; /* current peer title */

} TI_Config;


/*
 * Common Globals (defined in ti.c)
 */

extern char* progname;
extern OFBool verbose;
extern OFBool debug;
extern E_TransferSyntax networkTransferSyntax;

/*
 * Common Function Definitions
 */

extern void errmsg(const char* msg, ...);



#endif

/*
** CVS Log
** $Log: ti.h,v $
** Revision 1.1  2005/08/23 19:32:03  braindead
** - initial savannah import
**
** Revision 1.1  2005/06/26 19:26:14  pipelka
** - added dcmtk
**
** Revision 1.8  2002/11/29 07:18:16  wilkens
** Adapted ti utility to command line classes and added option '-xi'.
**
** Revision 1.7  2001/11/12 14:54:23  meichel
** Removed all ctndisp related code from imagectn
**
** Revision 1.6  2001/10/12 12:42:56  meichel
** Adapted imagectn to OFCondition based dcmnet module (supports strict mode).
**
** Revision 1.5  2001/06/01 15:51:22  meichel
** Updated copyright header
**
** Revision 1.4  2000/03/08 16:41:02  meichel
** Updated copyright header.
**
** Revision 1.3  1999/06/10 12:12:03  meichel
** Adapted imagectn to new command line option scheme.
**   Added support for Patient/Study Only Q/R model and C-GET (experimental).
**
** Revision 1.2  1997/07/21 08:59:46  andreas
** - Replace all boolean types (BOOLEAN, CTNBOOLEAN, DICOM_BOOL, BOOL)
**   with one unique boolean type OFBool.
**
** Revision 1.1  1996/04/22 10:27:25  hewett
** Initial release.
**
*/
