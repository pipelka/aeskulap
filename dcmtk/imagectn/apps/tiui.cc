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
 *  Author:  Andrew Hewett
 *
 *  Purpose: TI User Interface
 *
 *  Last Update:      $Author: braindead $
 *  Update Date:      $Date: 2005/08/23 19:32:03 $
 *  Source File:      $Source: /cvsroot/aeskulap/aeskulap/dcmtk/imagectn/apps/tiui.cc,v $
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
#include "ofstdinc.h"

BEGIN_EXTERN_C
#ifdef HAVE_SYS_FILE_H
#include <sys/file.h>
#endif
END_EXTERN_C

#include "ti.h"
#include "assoc.h"
#include "dimse.h"
#include "imagedb.h"
#include "dcuid.h"
#include "dcdeftag.h"
#include "ofcmdln.h"
#include "cnf.h"
#include "diutil.h"
#include "tiui.h"
#include "tinet.h"
#include "tiquery.h"


typedef struct {
    const char *cmdPrefix;
    OFBool (*actionFunc)(TI_Config *conf, int arg, const char *cmdbuf);
} TI_CommandEntry;


static OFBool
TI_shortHelp(TI_Config * /*conf*/ , int /*arg*/ , const char * /*cmdbuf*/ )
{
    printf("\
h)elp, t)itle, da)tabase, st)udy, ser)ies i)mage, di)splay, sen)d, e)cho, q)uit\n");

    return OFTrue;
}

static OFBool
TI_help(TI_Config * /*conf*/ , int arg, const char * /*cmdbuf*/ )
{
    if (verbose) {
        printf("TI_help: arg=%d\n", arg);
    }
    printf("Command Summary:\n");
    printf("help                list this summary\n");
    printf("?                   short help\n");
    printf("title [#]           list [set] current peer AE title\n");
    printf("database [#]        list [set] current database\n");
    printf("study [#]           list [set] current study\n");
    printf("series [#]          list [set] current series\n");
    printf("image [#]           list [set] current image\n");
    printf("send study [#]      send current [specific] study\n");
    printf("send series [#]     send current [specific] series\n");
    printf("send image [#]      send current [specific] image\n");
    printf("echo [#]            verify connectivity [# times]\n");
    printf("quit                quit program\n");
    printf("exit                synonym for quit\n");

    return OFTrue;
}


static OFBool
TI_welcome()
{
    printf("\n");
    printf("Welcome to the Image CTN Telnet Initiator\n");
    printf("\n");
    printf("This program allows you to list the contents of the CTN databases, send\n");
    printf("images to peer Application Entities (AEs), and to verify connectivity with\n");
    printf("peer AEs..\n");
    printf("The databases can only be viewed using a Study/Series/Image\n");
    printf("information model.\n");
    printf("\n");
    printf("Network associations will be started when you try to send a\n");
    printf("study/series/image or perform an echo.\n");
    printf("\n");
    printf("The prompt shows the current database title and the current peer AE title.\n");
    printf("\n");
    printf("Type help for help\n");

    return OFTrue;
}


static OFBool
TI_title(TI_Config *conf, int arg, const char * /*cmdbuf*/ )
{
    int i;
    TI_DBEntry *db;
    const char *peer;
    int port;
    DIC_AE peerTitle;

    if (verbose) {
        printf("TI_title: arg=%d\n", arg);
    }

    bzero(peerTitle, sizeof(peerTitle));
    if (conf->assoc) {
        ASC_getAPTitles(conf->assoc->params, NULL, peerTitle, NULL);
    }

    db = conf->dbEntries[conf->currentdb];
    if (arg < 0) {
        /* print list of peer AE titles we know */
        printf("Peer AE Titles:\n");
        printf("     %-16s %s\n", "Peer AE", "HostName:PortNumber");
        for (i=0; i<db->peerTitleCount; i++) {
            if (strcmp(conf->currentPeerTitle, db->peerTitles[i]) == 0) {
                printf("*");
            } else {
                printf(" ");
            }
            /* active = (strcmp(peerTitle, db->peerTitles[i]) == 0); */
            CNF_peerForAETitle(db->peerTitles[i], &peer, &port);
            printf(" %d) %-16s (%s:%d)\n", i, db->peerTitles[i],
                peer, port);
        }
    } else {
        /* choosing new peer AE title */
        if (arg >= db->peerTitleCount) {
            printf("ERROR: Peer AE Title Choice: 0 - %d\n",
                db->peerTitleCount - 1);
        } else {
            conf->currentPeerTitle = db->peerTitles[arg];
        }
    }
    return OFTrue;
}

#if 0
/* currently unused */
static OFBool
TI_release(TI_Config *conf, int /*arg*/, const char */*cmdbuf*/)
{
    return TI_detatchAssociation(conf, OFFalse);
}

static OFBool
TI_abort(TI_Config *conf, int /*arg*/, const char */*cmdbuf*/)
{
    return TI_detatchAssociation(conf, OFTrue);
}
#endif

static OFBool
TI_detatchDB(TI_DBEntry *db)
{
    if (db == NULL) return OFTrue;

    TI_destroyStudyEntries(db);

    if (!db->isRemoteDB && db->dbHandle != NULL) {
        DB_destroyHandle(&db->dbHandle);
        db->dbHandle = NULL;
    } else {

    }

    return OFTrue;
}

static OFBool
TI_attachDB(TI_DBEntry *db)
{
    OFCondition dbcond = EC_Normal;

    db->studyCount = 0;
    db->currentStudy = 0;
    db->currentImage = 0;

    if (!db->isRemoteDB && db->dbHandle == NULL) {
        /* Create a database handle */
        dbcond = DB_createHandle(CNF_getStorageArea(db->title),
            CNF_getMaxStudies(db->title),
            CNF_getMaxBytesPerStudy(db->title), &db->dbHandle);
        if (dbcond.bad()) {
            errmsg("TI_attachDB: cannot create DB Handle");
            return OFFalse;
        }
    } else {

    }

    return OFTrue;
}

static OFBool
TI_database(TI_Config *conf, int arg, const char * /*cmdbuf*/ )
{
    int i;
    TI_DBEntry *db = NULL;
    OFBool found = OFFalse;

    if (verbose) {
        printf("TI_database: arg=%d\n", arg);
    }

    if (arg < 0) {
        /* print list of database titles we know */
        printf("Database Titles:\n");
        printf("     %s\n", "Database");
        for (i=0; i<conf->dbCount; i++) {
            if (conf->currentdb == i) {
                printf("*");
            } else {
                printf(" ");
            }
            if (conf->dbEntries[i]->isRemoteDB) {
                printf("!");
            } else {
                printf(" ");
            }
            printf("%2d) %s\n", i, conf->dbEntries[i]->title);
        }
    } else {
        /* choosing new title */
        if (arg >= conf->dbCount) {
            printf("ERROR: Database Title Choice: 0 - %d\n",
                conf->dbCount - 1);
        } else {
            /* release old dbHandle */
            TI_detatchDB(conf->dbEntries[conf->currentdb]);

            conf->currentdb = arg;
            /* check to make sure that current peer AE title is
             * available for this database, if not must choose
             * another and tell user about the change.
             */
            db = conf->dbEntries[conf->currentdb];
            for (i=0; !found && i<db->peerTitleCount; i++) {
                found = (strcmp(conf->currentPeerTitle,
                          db->peerTitles[i]) == 0);
            }
            if (!found) {
                printf("WARNING: Actual Peer AE Title (%s) has no access to database: %s\n", conf->currentPeerTitle, db->title);
                printf("         Setting Default Peer AE Title: %s\n",
                    db->peerTitles[0]);
                conf->currentPeerTitle = db->peerTitles[0];
            }

            if (!TI_attachDB(conf->dbEntries[conf->currentdb]))
            {
                     errmsg("ERROR: unable to open database, bailing out.\n");
                     exit(10);
            }
        }
    }

    return OFTrue;
}

static OFBool
TI_echo(TI_Config *conf, int arg, const char * /*cmdbuf*/ )
{
    OFBool ok = OFTrue;

    if (verbose) {
        printf("TI_echo: arg=%d\n", arg);
    }

    ok = TI_changeAssociation(conf);
    if (!ok) return OFFalse;

    if (arg <= 0) arg = 1;  /* send 1 echo message */

    /* send echo message to peer AE title */

    while ( arg-- > 0 && ok) {
        ok = TI_sendEcho(conf);
    }

    ok = TI_detatchAssociation(conf, OFFalse);

    return ok;
}

static OFBool
TI_quit(TI_Config *conf, int arg, const char * /*cmdbuf*/ )
{
    if (verbose) {
        printf("TI_quit: arg=%d\n", arg);
    }
    TI_detatchAssociation(conf, OFFalse);
    printf("Good Bye, Auf Wiedersehen, Au Revoir\n");
    exit(0);
    return OFTrue;
}

static OFBool
TI_actualizeStudies(TI_Config *conf)
{
    TI_DBEntry *db;

    db = conf->dbEntries[conf->currentdb];

    /* get a list of all the available studies in the current database */
    if (!TI_buildStudies(conf, db))
        return OFFalse;

    if (db->studyCount == 0) {
        printf("No Studies in Database: %s\n", db->title);
        return OFFalse;
    }

    if (db->currentStudy < 0 || db->currentStudy >= db->studyCount)
        db->currentStudy = 0;

    return OFTrue;
}

#define STUDYFORMAT "%-30s %-12s %-12s\n"

static void
printStudyEntry(TI_StudyEntry *study)
{
    printf(STUDYFORMAT, study->patientsName, study->patientID,
      study->studyID);
}

static OFBool
TI_study(TI_Config *conf, int arg, const char * /*cmdbuf*/ )
{
    TI_DBEntry *db;
    TI_StudyEntry *se;
    int i;

    if (verbose) {
        printf("TI_study: arg=%d\n", arg);
    }

    db = conf->dbEntries[conf->currentdb];

    if (db->isRemoteDB) {
        conf->currentPeerTitle = db->title;
        /* make sure we have an association */
        OFBool ok = TI_changeAssociation(conf);
        if (!ok) return OFFalse;
    }

    if (!TI_actualizeStudies(conf))
        return OFFalse;

#ifndef RETAIN_ASSOCIATION
    if (conf->dbEntries[conf->currentdb]->isRemoteDB) {
        TI_detatchAssociation(conf, OFFalse);
    }
#endif

    if (arg >= 0) {
        /* set current study */
        if (arg >= db->studyCount) {
            printf("ERROR: Study Choice: 0 - %d\n",
                db->studyCount - 1);
            return OFFalse;
        }
        db->currentStudy = arg;
        return OFTrue;
    }

    /* list studies to user */
    printf("      ");
    printf(STUDYFORMAT, "Patient", "PatientID", "StudyID");
    for (i=0; i<db->studyCount; i++) {
        if (db->currentStudy == i) {
            printf("*");
        } else {
            printf(" ");
        }
        printf(" %2d) ", i);
        se = db->studies[i];
        printStudyEntry(se);
    }

    printf("\n");
    printf("%d Studies in Database: %s\n", db->studyCount, db->title);
    return OFTrue;
}

static OFBool
TI_actualizeSeries(TI_Config *conf)
{
    TI_DBEntry *db;
    TI_StudyEntry *study;

    db = conf->dbEntries[conf->currentdb];

    if (db->studyCount == 0)
        if (!TI_actualizeStudies(conf))
            return OFFalse;

    study = db->studies[db->currentStudy];

    /* get a list of all the available series in the current study */
    if (!TI_buildSeries(conf, db, study))
        return OFFalse;

    if (study->seriesCount == 0) {
        printf("No Series in Study %s (Database: %s)\n",
            study->studyID, db->title);
        return OFFalse;
    }
    if (db->currentSeries < 0 || db->currentSeries >= study->seriesCount)
        db->currentSeries = 0;

    return OFTrue;
}

#define SERIESFORMAT "%-6s %-8s %-s\n"

static void
printSeriesEntry(TI_SeriesEntry *series)
{
    printf(SERIESFORMAT, series->seriesNumber,
        series->modality, series->seriesInstanceUID);
}

static OFBool
TI_series(TI_Config *conf, int arg, const char * /*cmdbuf*/ )
{
    TI_DBEntry *db;
    TI_StudyEntry *study;
    TI_SeriesEntry *series;
    int i;

    if (verbose) {
        printf("TI_sseries: arg=%d\n", arg);
    }

    db = conf->dbEntries[conf->currentdb];

    if (db->isRemoteDB) {
        conf->currentPeerTitle = db->title;
        /* make sure we have an association */
        OFBool ok = TI_changeAssociation(conf);
        if (!ok) return OFFalse;
    }

    if (!TI_actualizeSeries(conf))
        return OFFalse;

#ifndef RETAIN_ASSOCIATION
    if (conf->dbEntries[conf->currentdb]->isRemoteDB) {
        TI_detatchAssociation(conf, OFFalse);
    }
#endif

    study = db->studies[db->currentStudy];

    if (arg >= 0) {
        /* set current series */
        if (arg >= study->seriesCount) {
            printf("ERROR: Series Choice: 0 - %d\n",
                study->seriesCount - 1);
            return OFFalse;
        }
        db->currentSeries = arg;
        return OFTrue;
    }

    /* list series to user */
    printf("      ");
    printf(SERIESFORMAT, "Series", "Modality", "SeriesInstanceUID");
    for (i=0; i<study->seriesCount; i++) {
        if (db->currentSeries == i) {
            printf("*");
        } else {
            printf(" ");
        }
        printf(" %2d) ", i);
        series = study->series[i];
        printSeriesEntry(series);
    }

    printf("\n");
    printf("%d Series in StudyID %s,\n",
        study->seriesCount, study->studyID);
    printf("  Patient: %s (Database: %s)\n",
        study->patientsName, db->title);
    return OFTrue;
}

static OFBool
TI_actualizeImages(TI_Config *conf)
{
    TI_DBEntry *db;
    TI_StudyEntry *study;
    TI_SeriesEntry *series;

    db = conf->dbEntries[conf->currentdb];

    if (db->studyCount == 0) {
        if (!TI_actualizeStudies(conf))
            return OFFalse;
    }

    study = db->studies[db->currentStudy];
    if (study->seriesCount == 0) {
        if (!TI_actualizeSeries(conf))
            return OFFalse;
    }

    series = study->series[db->currentSeries];

    /* get a list of all the available images in the current series */
    if (!TI_buildImages(conf, db, study, series))
        return OFFalse;

    if (series->imageCount == 0) {
        printf("No Images in Series %s, Study %s (Database: %s)\n",
            series->seriesNumber, study->studyID, db->title);
        return OFFalse;
    }
    return OFTrue;
}

#define IMAGEFORMAT "%-5s %-s\n"

static void
printImageEntry(TI_ImageEntry *image)
{
    printf(IMAGEFORMAT, image->imageNumber, image->sopInstanceUID);
}

static OFBool
TI_image(TI_Config *conf, int arg, const char * /*cmdbuf*/ )
{
    TI_DBEntry *db;
    TI_StudyEntry *study;
    TI_SeriesEntry *series;
    TI_ImageEntry *image;
    int i;

    if (verbose) {
        printf("TI_image: arg=%d\n", arg);
    }

    db = conf->dbEntries[conf->currentdb];

    if (db->isRemoteDB) {
        conf->currentPeerTitle = db->title;
        /* make sure we have an association */
        OFBool ok = TI_changeAssociation(conf);
        if (!ok) return OFFalse;
    }

    if (!TI_actualizeImages(conf))
        return OFFalse;

#ifndef RETAIN_ASSOCIATION
    if (conf->dbEntries[conf->currentdb]->isRemoteDB) {
        TI_detatchAssociation(conf, OFFalse);
    }
#endif

    study = db->studies[db->currentStudy];
    series = study->series[db->currentSeries];

    if (arg >= 0) {
        /* set current image */
        if (arg >= series->imageCount) {
            printf("ERROR: Image Choice: 0 - %d\n",
                series->imageCount - 1);
            return OFFalse;
        }
        db->currentImage = arg;
        return OFTrue;
    }

    /* list images to user */
    printf("      ");
    printf(IMAGEFORMAT, "Image", "ImageInstanceUID");
    for (i=0; i<series->imageCount; i++) {
        if (db->currentImage == i) {
            printf("*");
        } else {
            printf(" ");
        }
        printf(" %2d) ", i);
        image = series->images[i];
        printImageEntry(image);
    }

    printf("\n");
    printf("%d Images in %s Series, StudyID %s,\n",
        series->imageCount, series->modality, study->studyID);
    printf("  Patient: %s (Database: %s)\n",
        study->patientsName, db->title);
    return OFTrue;
}

static OFBool
TI_sendStudy(TI_Config *conf, int arg, const char * /*cmdbuf*/ )
{
    OFBool ok = OFTrue;
    DcmDataset *query = NULL;
    TI_DBEntry *db;
    TI_StudyEntry *study;
    OFCondition dbcond = EC_Normal;
    DB_Status dbStatus;
    DIC_UI sopClass;
    DIC_UI sopInstance;
    char imgFile[MAXPATHLEN+1];
    DIC_US nRemaining = 0;

    if (verbose) {
        printf("TI_sendStudy: arg=%d\n", arg);
    }

    db = conf->dbEntries[conf->currentdb];

    /*
    ** We cannot read images from a DB and send images to the same DB
    ** over the network because of deadlock.  The DB move routines
    ** lock the index file.  When we send over the network to the same
    ** DB it tries to lock the index file exclusively to insert the image
    ** in the database.  We end up waiting for a response from the remote
    ** peer which never comes.
    */

    if (strcmp(db->title, conf->currentPeerTitle) == 0) {
        printf("Sorry, cannot send images from a DB to itself, possible deadlock\n");
        return OFFalse;
    }

    /* make sure study info is actual */
    ok = TI_actualizeStudies(conf);
    if (!ok) return OFFalse;

    /* set arg as current study */
    if (arg < 0) {
        arg = db->currentStudy;
    }

    if (arg >= db->studyCount) {
        printf("ERROR: Study Choice: 0 - %d\n",
            db->studyCount - 1);
        return OFFalse;
    }
    study = db->studies[arg];

    /* make sure we have an association */
    ok = TI_changeAssociation(conf);
    if (!ok) return OFFalse;

    /* fabricate query */
    query = new DcmDataset;
    if (query == NULL) {
        errmsg("Help, out of memory");
        return OFFalse;
    }
    DU_putStringDOElement(query, DCM_QueryRetrieveLevel, "STUDY");
    DU_putStringDOElement(query, DCM_StudyInstanceUID, study->studyInstanceUID);

    dbStatus.status = STATUS_Pending;
    dbStatus.statusDetail = NULL;

    dbcond = DB_startMoveRequest(db->dbHandle,
        UID_MOVEStudyRootQueryRetrieveInformationModel, query, &dbStatus);
    delete query; query = NULL;
    if (dbcond.bad()) {
        errmsg("TI_sendStudy: cannot query database");
        return OFFalse;
    }

    while (ok && dbStatus.status == STATUS_Pending) {
        dbcond = DB_nextMoveResponse(db->dbHandle, sopClass, sopInstance,
            imgFile, &nRemaining, &dbStatus);
        if (dbcond.bad()) {
            errmsg("TI_sendStudy: database error");
            return OFFalse;
        }
        if (dbStatus.status == STATUS_Pending) {

            ok = TI_storeImage(conf, sopClass, sopInstance, imgFile);
            if (!ok) {
                DB_cancelMoveRequest(db->dbHandle, &dbStatus);
            }
        }
    }

    ok = TI_detatchAssociation(conf, OFFalse);

    return ok;
}


static OFBool
TI_sendSeries(TI_Config *conf, int arg, const char * /*cmdbuf*/ )
{
    OFBool ok = OFTrue;
    DcmDataset *query = NULL;
    TI_DBEntry *db;
    TI_StudyEntry *study;
    TI_SeriesEntry *series;
    OFCondition dbcond = EC_Normal;
    DB_Status dbStatus;
    DIC_UI sopClass;
    DIC_UI sopInstance;
    char imgFile[MAXPATHLEN+1];
    DIC_US nRemaining = 0;

    if (verbose) {
        printf("TI_sendSeries: arg=%d\n", arg);
    }

    db = conf->dbEntries[conf->currentdb];

    /* make sure study/series info is actual */
    ok = TI_actualizeSeries(conf);
    if (!ok) return OFFalse;

    study = db->studies[db->currentStudy];

    /* set arg as current series */
    if (arg < 0) {
        arg = db->currentSeries;
    }

    if (arg >= study->seriesCount) {
        printf("ERROR: Series Choice: 0 - %d\n",
            study->seriesCount - 1);
        return OFFalse;
    }
    series = study->series[arg];

    /* make sure we have an association */
    ok = TI_changeAssociation(conf);
    if (!ok) return OFFalse;

    /* fabricate query */
    query = new DcmDataset;
    if (query == NULL) {
        errmsg("Help, out of memory");
        return OFFalse;
    }
    DU_putStringDOElement(query, DCM_QueryRetrieveLevel, "SERIES");
    DU_putStringDOElement(query, DCM_StudyInstanceUID, study->studyInstanceUID);
    DU_putStringDOElement(query, DCM_SeriesInstanceUID,
        series->seriesInstanceUID);

    dbStatus.status = STATUS_Pending;
    dbStatus.statusDetail = NULL;

    dbcond = DB_startMoveRequest(db->dbHandle,
        UID_MOVEStudyRootQueryRetrieveInformationModel, query, &dbStatus);
    delete query; query = NULL;
    if (dbcond.bad()) {
        errmsg("TI_sendSeries: cannot query database");
        return OFFalse;
    }

    while (ok && dbStatus.status == STATUS_Pending) {
        dbcond = DB_nextMoveResponse(db->dbHandle, sopClass, sopInstance,
            imgFile, &nRemaining, &dbStatus);
        if (dbcond.bad()) {
            errmsg("TI_sendSeries: database error");
            return OFFalse;
        }
        if (dbStatus.status == STATUS_Pending) {

            ok = TI_storeImage(conf, sopClass, sopInstance, imgFile);
            if (!ok) {
                DB_cancelMoveRequest(db->dbHandle, &dbStatus);
            }
        }
    }

    ok = TI_detatchAssociation(conf, OFFalse);

    return ok;
}

static OFBool
TI_sendImage(TI_Config *conf, int arg, const char * /*cmdbuf*/ )
{
    OFBool ok = OFTrue;
    DcmDataset *query = NULL;
    TI_DBEntry *db;
    TI_StudyEntry *study;
    TI_SeriesEntry *series;
    TI_ImageEntry *image;
    OFCondition dbcond = EC_Normal;
    DB_Status dbStatus;
    DIC_UI sopClass;
    DIC_UI sopInstance;
    char imgFile[MAXPATHLEN+1];
    DIC_US nRemaining = 0;

    if (verbose) {
        printf("TI_sendImage: arg=%d\n", arg);
    }

    db = conf->dbEntries[conf->currentdb];

    /* make sure study/series/image info is actual */
    ok = TI_actualizeImages(conf);
    if (!ok) return OFFalse;

    study = db->studies[db->currentStudy];
    series = study->series[db->currentSeries];

    /* set arg as current image */
    if (arg < 0) {
        arg = db->currentImage;
    }

    if (arg >= series->imageCount) {
        printf("ERROR: Image Choice: 0 - %d\n",
            series->imageCount - 1);
        return OFFalse;
    }

    image = series->images[arg];

    /* make sure we have an association */
    ok = TI_changeAssociation(conf);
    if (!ok) return OFFalse;

    /* fabricate query */
    query = new DcmDataset;
    if (query == NULL) {
        errmsg("Help, out of memory");
        return OFFalse;
    }
    DU_putStringDOElement(query, DCM_QueryRetrieveLevel, "IMAGE");
    DU_putStringDOElement(query, DCM_StudyInstanceUID, study->studyInstanceUID);
    DU_putStringDOElement(query, DCM_SeriesInstanceUID,
        series->seriesInstanceUID);
    DU_putStringDOElement(query, DCM_SOPInstanceUID,
        image->sopInstanceUID);

    dbStatus.status = STATUS_Pending;
    dbStatus.statusDetail = NULL;

    dbcond = DB_startMoveRequest(db->dbHandle,
        UID_MOVEStudyRootQueryRetrieveInformationModel, query, &dbStatus);
    delete query; query = NULL;
    if (dbcond.bad()) {
        errmsg("TI_sendImage: cannot query database");
        return OFFalse;
    }

    /*
     * We should only ever get 1 response to the above query,
     * but you never know (there could be non-unique UIDs in
     * the database).
     */
    while (ok && dbStatus.status == STATUS_Pending) {
        dbcond = DB_nextMoveResponse(db->dbHandle, sopClass, sopInstance,
      imgFile, &nRemaining, &dbStatus);
        if (dbcond.bad()) {
            errmsg("TI_sendImage: database error");
            return OFFalse;
        }
        if (dbStatus.status == STATUS_Pending) {

            ok = TI_storeImage(conf, sopClass, sopInstance, imgFile);
            if (!ok) {
                DB_cancelMoveRequest(db->dbHandle, &dbStatus);
            }
        }
    }

    ok = TI_detatchAssociation(conf, OFFalse);

    return ok;
}

TI_CommandEntry sendCmdTable[] = {
    { "st", TI_sendStudy },
    { "se", TI_sendSeries },
    { "i", TI_sendImage }
};

static OFBool
TI_send(TI_Config *conf, int /*arg*/, const char *cmdbuf)
{
    OFBool ok = OFTrue;
    char cmdarg[128];
    int iarg;
    int narg;
    int i;
    OFBool found;

    if (conf->dbEntries[conf->currentdb]->isRemoteDB) {
        printf("Sorry, cannot send from remote DB\n");
        return OFTrue;
    }

    bzero(cmdarg, sizeof(cmdarg));

    narg = sscanf(cmdbuf, "send %s %d", cmdarg, &iarg);
    if (narg == 1)
        iarg = -1;

    found = OFFalse;
    for (i=0; !found && i<(int)DIM_OF(sendCmdTable); i++) {
        if (strncmp(sendCmdTable[i].cmdPrefix, cmdarg,
            strlen(sendCmdTable[i].cmdPrefix)) == 0) {

            found = OFTrue;
      
            ok = sendCmdTable[i].actionFunc(conf, iarg, cmdbuf);

        }
    }
    if (!found) {
        printf("What do you want to send? Type help for help\n");
    }

    return ok;
}

static TI_CommandEntry cmdTable[] = {
    { "h", TI_help },
    { "?", TI_shortHelp },
    { "t", TI_title },
    { "da", TI_database },
    { "st", TI_study },
    { "ser", TI_series },
    { "i", TI_image },
    { "send", TI_send },
    { "ec", TI_echo },
    { "q", TI_quit },
    { "exit", TI_quit }
};

void
TI_userInput(TI_Config *conf)
{
    char cmdBuf[1024];  /* can't have lines longer than this */
    int i;
    OFBool found = OFFalse;
    int arg;

    /* make the first database current */
    conf->currentdb = 0;
    /* make the first peer title for this database current */
    conf->currentPeerTitle = conf->dbEntries[conf->currentdb]->peerTitles[0];
    /* open db */
    TI_database(conf, conf->currentdb, cmdBuf);

    TI_welcome();
    printf("\n");

    while (1) {
        printf("%s->%s> ", conf->dbEntries[conf->currentdb]->title,
            conf->currentPeerTitle);
        if (fgets(cmdBuf, 1024, stdin) == NULL) {
            errmsg("unexpected end of input\n");
            return; /* give up */
        }

        DU_stripLeadingSpaces(cmdBuf);
        if (strlen(cmdBuf) == 0)
            continue; /* no input */

        if (sscanf(cmdBuf, "%*s %d", &arg) != 1) {
            arg = -1;
        }

        /* find command parser */
        found = OFFalse;
        for (i=0; !found && i<(int)DIM_OF(cmdTable); i++) {
            if (strncmp(cmdTable[i].cmdPrefix, cmdBuf,
                strlen(cmdTable[i].cmdPrefix)) == 0) {
                    found = OFTrue;
                    cmdTable[i].actionFunc(conf, arg, cmdBuf);
            }
        }
        if (!found) {
            printf("What do you want to do? Type help for help\n");
        }
    }
}

/*
** CVS Log
** $Log: tiui.cc,v $
** Revision 1.1  2005/08/23 19:32:03  braindead
** - initial savannah import
**
** Revision 1.1  2005/06/26 19:26:14  pipelka
** - added dcmtk
**
** Revision 1.19  2002/11/29 07:18:18  wilkens
** Adapted ti utility to command line classes and added option '-xi'.
**
** Revision 1.18  2002/11/27 13:27:03  meichel
** Adapted module imagectn to use of new header file ofstdinc.h
**
** Revision 1.17  2001/11/12 14:54:25  meichel
** Removed all ctndisp related code from imagectn
**
** Revision 1.16  2001/10/12 12:42:57  meichel
** Adapted imagectn to OFCondition based dcmnet module (supports strict mode).
**
** Revision 1.15  2001/06/01 15:51:24  meichel
** Updated copyright header
**
** Revision 1.14  2000/12/12 16:47:05  meichel
** Minor changes to keep gcc 2.7.x on SunOS 4.1.3 happy
**
** Revision 1.13  2000/04/12 12:18:50  meichel
** Fixed bug in ti leading to segmentation fault if database index file
**   was opened without write permission.
**
** Revision 1.12  2000/03/08 16:41:04  meichel
** Updated copyright header.
**
** Revision 1.11  2000/02/23 15:13:19  meichel
** Corrected macro for Borland C++ Builder 4 workaround.
**
** Revision 1.10  2000/02/01 11:43:42  meichel
** Avoiding to include <stdlib.h> as extern "C" on Borland C++ Builder 4,
**   workaround for bug in compiler header files.
**
** Revision 1.9  1999/06/10 12:12:07  meichel
** Adapted imagectn to new command line option scheme.
**   Added support for Patient/Study Only Q/R model and C-GET (experimental).
**
** Revision 1.8  1998/01/27 10:51:47  meichel
** Removed some unused variables, meaningless const modifiers
**   and unreached statements.
**
** Revision 1.7  1997/07/21 08:59:49  andreas
** - Replace all boolean types (BOOLEAN, CTNBOOLEAN, DICOM_BOOL, BOOL)
**   with one unique boolean type OFBool.
**
** Revision 1.6  1997/05/30 07:33:25  meichel
** Added space characters around comments and simplified
** some inlining code (needed for SunCC 2.0.1).
**
** Revision 1.5  1996/09/27 08:46:25  hewett
** Enclosed system include files with BEGIN_EXTERN_C/END_EXTERN_C.
**
** Revision 1.4  1996/06/10 08:24:50  meichel
** Added missing return in TI_quit.
**
** Revision 1.3  1996/05/06 07:33:15  hewett
** Rearranged handing of display disabling code.
**
** Revision 1.2  1996/04/22 10:29:47  hewett
** Changed welcome message to indicate relevate to Image CTN.
**
** Revision 1.1  1996/04/22 10:27:29  hewett
** Initial release.
**
**
*/
