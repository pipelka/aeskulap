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
 *  Purpose: TI Query Routines
 *
 *  Last Update:      $Author: braindead $
 *  Update Date:      $Date: 2005/08/23 19:32:03 $
 *  Source File:      $Source: /cvsroot/aeskulap/aeskulap/dcmtk/imagectn/apps/tiquery.cc,v $
 *  CVS/RCS Revision: $Revision: 1.1 $
 *  Status:           $State: Exp $
 *
 *  CVS/RCS Log at end of file
 *
 */

#include "osconfig.h"    /* make sure OS specific configuration is included first */

#define INCLUDE_CSTDIO
#define INCLUDE_CTIME
#define INCLUDE_CERRNO
#include "ofstdinc.h"

BEGIN_EXTERN_C
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
END_EXTERN_C

#include "ti.h"
#include "dicom.h"
#include "diutil.h"
#include "dcuid.h"
#include "dcdeftag.h"
#include "dcfilefo.h"
#include "imagedb.h"
#include "ofcmdln.h"
#include "cnf.h"
#include "tiquery.h"
#include "tinet.h"

OFBool
TI_addStudyEntry(TI_DBEntry *db, DcmDataset *reply);
OFBool
TI_addSeriesEntry(TI_StudyEntry *study, DcmDataset *reply);
OFBool
TI_addImageEntry(TI_SeriesEntry *series, DcmDataset *reply);

extern "C" int TI_seriesCompare(const void *a, const void *b);
extern "C" int TI_imageCompare(const void *a, const void *b);


OFBool
TI_dbReadable(const char *dbTitle)
{
    char path[MAXPATHLEN+1];
    sprintf(path, "%s%c%s", CNF_getStorageArea(dbTitle), PATH_SEPARATOR, DBINDEXFILE);

    return (access(path, R_OK) == 0);
}

time_t
TI_dbModifyTime(const char *dbTitle)
{
    char path[MAXPATHLEN+1];
    struct stat s;

    sprintf(path, "%s%c%s", CNF_getStorageArea(dbTitle), PATH_SEPARATOR, DBINDEXFILE);

    if (stat(path, &s) < 0) {
        errmsg("cannot stat: %s", path);
        return 0;
    }
    return s.st_mtime;
}

/*
 * Study Level
 */

void
TI_destroyStudyEntries(TI_DBEntry *db)
{
    int i;

    if (db == NULL) return;

    for (i=0; i<db->studyCount; i++) {
        TI_destroySeriesEntries(db->studies[i]);
        free(db->studies[i]);
        db->studies[i] = NULL;
    }

    db->studyCount = 0;
}

OFBool
TI_addStudyEntry(TI_DBEntry *db, DcmDataset *reply)
{
    TI_StudyEntry *se;
    OFBool ok = OFTrue;

    if (db->studyCount >= TI_MAXSTUDIES) {
        errmsg("TI_addStudyEntry: too many studies");
        return OFFalse;
    }

    se = (TI_StudyEntry*) malloc(sizeof(TI_StudyEntry));
    if (se == NULL) return OFFalse;

    bzero((char*)se, sizeof(TI_StudyEntry));  /* make sure its clean */

    /* extract info from reply */
    ok = DU_getStringDOElement(reply, DCM_StudyInstanceUID,
        se->studyInstanceUID);
    ok = ok && (ok = DU_getStringDOElement(reply, DCM_StudyID,
        se->studyID));
    ok = ok && (ok = DU_getStringDOElement(reply, DCM_PatientsName,
        se->patientsName));
    ok = ok && (ok = DU_getStringDOElement(reply, DCM_PatientID,
        se->patientID));
    if (!ok) {
        errmsg("TI_addStudyEntry: missing data in DB reply");
        return OFFalse;
    }

    DU_stripLeadingAndTrailingSpaces(se->studyInstanceUID);
    DU_stripLeadingAndTrailingSpaces(se->studyID);
    DU_stripLeadingAndTrailingSpaces(se->patientsName);
    DU_stripLeadingAndTrailingSpaces(se->patientID);

    /* add to array */
    db->studies[db->studyCount] = se;
    db->studyCount++;

    return OFTrue;
}

static void
TI_buildStudyQuery(DcmDataset **query)
{
    if (*query != NULL) delete *query;
    *query = new DcmDataset;
    if (*query == NULL) {
        errmsg("Help, out of memory");
        return;
    }

    DU_putStringDOElement(*query, DCM_QueryRetrieveLevel, "STUDY");
    DU_putStringDOElement(*query, DCM_StudyInstanceUID, NULL);
    DU_putStringDOElement(*query, DCM_StudyID, NULL);
    DU_putStringDOElement(*query, DCM_PatientsName, NULL);
    DU_putStringDOElement(*query, DCM_PatientID, NULL);
}

OFBool
TI_genericEntryCallback(TI_GenericCallbackStruct *cbs, DcmDataset *reply)
{
    if (cbs->db) return TI_addStudyEntry(cbs->db, reply);
    if (cbs->study) return TI_addSeriesEntry(cbs->study, reply);
    if (cbs->series) return TI_addImageEntry(cbs->series, reply);
    return OFFalse;
}

OFBool
TI_buildRemoteStudies(TI_Config *conf, TI_DBEntry *db)
{
    TI_GenericCallbackStruct cbs;
    DcmDataset *query = NULL;
    OFBool ok = OFTrue;

    cbs.db = db;
    cbs.study = NULL; cbs.series = NULL;

    TI_destroyStudyEntries(db);

    /* get all known studies */
    TI_buildStudyQuery(&query);

    ok = TI_remoteFindQuery(conf, db, query, TI_genericEntryCallback, &cbs);

    delete query;

    return ok;
}

OFBool
TI_buildStudies(TI_Config *conf, TI_DBEntry *db)
{
    OFCondition dbcond = EC_Normal;
    DB_Status dbStatus;
    DcmDataset *query = NULL;
    DcmDataset *reply = NULL;

    if (db->isRemoteDB) {
        return TI_buildRemoteStudies(conf, db);
    }

    if (db->studyCount != 0 &&
        TI_dbModifyTime(db->title) < db->lastQueryTime) {
        /* nothing has changed */
        return OFTrue;
    }

    TI_destroyStudyEntries(db);

    /* get all known studies */
    TI_buildStudyQuery(&query);

    dbStatus.status = STATUS_Pending;
    dbStatus.statusDetail = NULL;

    printf("Querying Database for Studies ...\n");
    db->lastQueryTime = time(NULL);

    dbcond = DB_startFindRequest(db->dbHandle,
        UID_FINDStudyRootQueryRetrieveInformationModel, query, &dbStatus);
    if (dbcond.bad()) {
        errmsg("TI_buildStudies: cannot query database");
        delete query;
        return OFFalse;
    }
    if (dbStatus.statusDetail != NULL) {
        delete dbStatus.statusDetail;
    }

    while (dbStatus.status == STATUS_Pending) {
        dbcond = DB_nextFindResponse(db->dbHandle, &reply, &dbStatus);
        if (dbcond.bad()) {
            errmsg("TI_buildStudies: database error");
            return OFFalse;
        }
        if (dbStatus.status == STATUS_Pending) {
            TI_addStudyEntry(db, reply);
            delete reply;
            reply = NULL;
        }
    }

    delete query;

    return OFTrue;
}

/*
 * Series Level
 */

void
TI_destroySeriesEntries(TI_StudyEntry *study)
{
    int i;

    if (study == NULL) return;

    for (i=0; i<study->seriesCount; i++) {
        TI_destroyImageEntries(study->series[i]);
        free(study->series[i]);
        study->series[i] = NULL;
    }
    study->seriesCount = 0;
}

OFBool
TI_addSeriesEntry(TI_StudyEntry *study, DcmDataset *reply)
{
    TI_SeriesEntry *series;
    OFBool ok = OFTrue;

    if (study->seriesCount >= TI_MAXSERIES) {
        errmsg("TI_addSeriesEntry: too many series");
        return OFFalse;
    }

    series = (TI_SeriesEntry*) malloc(sizeof(TI_SeriesEntry));
    if (series == NULL) return OFFalse;

    bzero((char*)series, sizeof(TI_SeriesEntry)); /* make sure its clean */

    /* extract info from reply */
    ok = DU_getStringDOElement(reply, DCM_SeriesInstanceUID,
        series->seriesInstanceUID);
    ok = ok && (ok = DU_getStringDOElement(reply, DCM_SeriesNumber,
        series->seriesNumber));
    ok = ok && (ok = DU_getStringDOElement(reply, DCM_Modality,
        series->modality));
    if (!ok) {
        errmsg("TI_addSeriesEntry: missing data in DB reply");
        return OFFalse;
    }

    DU_stripLeadingAndTrailingSpaces(series->seriesInstanceUID);
    DU_stripLeadingAndTrailingSpaces(series->seriesNumber);
    DU_stripLeadingAndTrailingSpaces(series->modality);

    series->intSeriesNumber = atoi(series->seriesNumber);

    /* add to array */
    study->series[study->seriesCount] = series;
    study->seriesCount++;

    return OFTrue;
}

int TI_seriesCompare(const void *a, const void *b)
{
    TI_SeriesEntry **sa, **sb;
    int cmp = 0;

    /* compare function for qsort, a and b are pointers to
     * the images array elements.  The routine must return an
     * integer less than, equal to, or greater than 0 according as
     * the first argument is to be considered less than, equal to,
     * or greater than the second.
     */
    sa = (TI_SeriesEntry **)a;
    sb = (TI_SeriesEntry **)b;

    cmp = (*sa)->intSeriesNumber - (*sb)->intSeriesNumber;

    return cmp;
}

static void
TI_buildSeriesQuery(DcmDataset **query, TI_StudyEntry *study)
{
    if (*query != NULL) delete *query;
    *query = new DcmDataset;
    if (*query == NULL) {
        errmsg("Help, out of memory");
        return;
    }

    DU_putStringDOElement(*query, DCM_QueryRetrieveLevel, "SERIES");
    DU_putStringDOElement(*query, DCM_StudyInstanceUID,
        study->studyInstanceUID);
    DU_putStringDOElement(*query, DCM_SeriesInstanceUID, NULL);
    DU_putStringDOElement(*query, DCM_Modality, NULL);
    DU_putStringDOElement(*query, DCM_SeriesNumber, NULL);
}

OFBool
TI_buildRemoteSeries(TI_Config *conf, TI_DBEntry *db, TI_StudyEntry *study)
{
    TI_GenericCallbackStruct cbs;
    DcmDataset *query = NULL;
    OFBool ok = OFTrue;

    cbs.db = NULL;
    cbs.study = study; cbs.series = NULL;

    TI_destroySeriesEntries(study);

    /* get all known studies */
    TI_buildSeriesQuery(&query, study);

    ok = TI_remoteFindQuery(conf, db, query, TI_genericEntryCallback, &cbs);

    delete query;

    return ok;
}

OFBool
TI_buildSeries(TI_Config *conf, TI_DBEntry *db, TI_StudyEntry *study)
{
    OFCondition dbcond = EC_Normal;
    DB_Status dbStatus;
    DcmDataset *query = NULL;
    DcmDataset *reply = NULL;

    if (db->isRemoteDB) {
        return TI_buildRemoteSeries(conf, db, study);
    }

    if (study->seriesCount != 0 &&
        TI_dbModifyTime(db->title) < db->lastQueryTime) {
        /* nothing has changed */
        return OFTrue;
    }

    TI_destroySeriesEntries(study);

    /* get all known series for this study */
    TI_buildSeriesQuery(&query, study);

    dbStatus.status = STATUS_Pending;
    dbStatus.statusDetail = NULL;

    printf("Querying Database for Series ...\n");
    study->lastQueryTime = time(NULL);

    dbcond = DB_startFindRequest(db->dbHandle,
        UID_FINDStudyRootQueryRetrieveInformationModel, query, &dbStatus);
    if (dbcond.bad()) {
        errmsg("TI_buildSeries: cannot query database");
        delete query; query = NULL;
        return OFFalse;
    }
    if (dbStatus.statusDetail != NULL) {
        delete dbStatus.statusDetail;
        dbStatus.statusDetail = NULL;
    }

    while (dbStatus.status == STATUS_Pending) {
        dbcond = DB_nextFindResponse(db->dbHandle, &reply, &dbStatus);
        if (dbcond.bad()) {
            errmsg("TI_buildSeries: database error");
            return OFFalse;
        }
        if (dbStatus.status == STATUS_Pending) {
            TI_addSeriesEntry(study, reply);
            delete reply;
            reply = NULL;
        }
    }

    delete query;
    query = NULL;

    if (study->seriesCount > 0) {
        /* sort the seriesinto assending series number order */
        qsort(study->series, study->seriesCount, sizeof(study->series[0]),
              TI_seriesCompare);
    }

    return OFTrue;
}

/*
 * Image Level
 */

void
TI_destroyImageEntries(TI_SeriesEntry *series)
{
    int i;

    if (series == NULL) return;

    for (i=0; i<series->imageCount; i++) {
        free(series->images[i]);
        series->images[i] = NULL;
    }
    series->imageCount = 0;
}

OFBool
TI_addImageEntry(TI_SeriesEntry *series, DcmDataset *reply)
{
    TI_ImageEntry *image;
    OFBool ok = OFTrue;
    DIC_CS studyID;

    if (series->imageCount >= TI_MAXIMAGES) {
        errmsg("TI_addImageEntry: too many images");
        return OFFalse;
    }

    image = (TI_ImageEntry*) malloc(sizeof(TI_ImageEntry));
    if (image == NULL) return OFFalse;

    bzero((char*)image, sizeof(TI_ImageEntry)); /* make sure its clean */
    bzero((char*)studyID, sizeof(DIC_CS));

    /* extract info from reply */
    ok = DU_getStringDOElement(reply, DCM_SOPInstanceUID,
        image->sopInstanceUID);
    ok = ok && (ok = DU_getStringDOElement(reply, DCM_InstanceNumber,
        image->imageNumber));
    if (!ok) {
        errmsg("TI_addImageEntry: missing data in DB reply");
        return OFFalse;
    }

    DU_stripLeadingAndTrailingSpaces(image->sopInstanceUID);
    DU_stripLeadingAndTrailingSpaces(image->imageNumber);

    image->intImageNumber = atoi(image->imageNumber);

    /* add to array */
    series->images[series->imageCount] = image;
    series->imageCount++;

    return OFTrue;
}

int TI_imageCompare(const void *a, const void *b)
{
    TI_ImageEntry **ia, **ib;
    int cmp = 0;

    /* compare function for qsort, a and b are pointers to
     * the images array elements.  The routine must return an
     * integer less than, equal to, or greater than 0 according as
     * the first argument is to be considered less than, equal to,
     * or greater than the second.
     */
    ia = (TI_ImageEntry **)a;
    ib = (TI_ImageEntry **)b;

    /* compare image numbers */
    cmp = (*ia)->intImageNumber - (*ib)->intImageNumber;

    return cmp;
}

static void
TI_buildImageQuery(DcmDataset **query, TI_StudyEntry *study,
    TI_SeriesEntry *series)
{
    if (*query != NULL) delete *query;
    *query = new DcmDataset;
    if (*query == NULL) {
        errmsg("Help, out of memory!");
        return;
    }

    DU_putStringDOElement(*query, DCM_QueryRetrieveLevel, "IMAGE");
    DU_putStringDOElement(*query, DCM_StudyInstanceUID,
        study->studyInstanceUID);
    DU_putStringDOElement(*query, DCM_SeriesInstanceUID,
        series->seriesInstanceUID);
    DU_putStringDOElement(*query, DCM_InstanceNumber, NULL);
    DU_putStringDOElement(*query, DCM_SOPInstanceUID, NULL);
}

OFBool
TI_buildRemoteImages(TI_Config *conf, TI_DBEntry *db, TI_StudyEntry *study,
    TI_SeriesEntry *series)
{
    TI_GenericCallbackStruct cbs;
    DcmDataset *query = NULL;
    OFBool ok = OFTrue;

    cbs.db = NULL;
    cbs.study = NULL; cbs.series = series;

    TI_destroyImageEntries(series);

    /* get all known studies */
    TI_buildImageQuery(&query, study, series);

    ok = TI_remoteFindQuery(conf, db, query, TI_genericEntryCallback, &cbs);

    delete query;

    return ok;
}

OFBool
TI_buildImages(TI_Config *conf, TI_DBEntry *db, TI_StudyEntry *study,
    TI_SeriesEntry *series)
{
    OFCondition dbcond = EC_Normal;
    DB_Status dbStatus;
    DcmDataset *query = NULL;
    DcmDataset *reply = NULL;

    if (db->isRemoteDB) {
        return TI_buildRemoteImages(conf, db, study, series);
    }

    if (series->imageCount != 0 &&
        TI_dbModifyTime(db->title) < study->lastQueryTime) {
        /* nothing has changed */
        return OFTrue;
    }

    TI_destroyImageEntries(series);

    /* get all known images in current series */
    TI_buildImageQuery(&query, study, series);

    if (verbose) {
        printf("QUERY OBJECT:\n");
        query->print(COUT);
    }

    dbStatus.status = STATUS_Pending;
    dbStatus.statusDetail = NULL;

    printf("Querying Database for Images ...\n");
    series->lastQueryTime = time(NULL);

    dbcond = DB_startFindRequest(db->dbHandle,
        UID_FINDStudyRootQueryRetrieveInformationModel, query, &dbStatus);
    delete query; query = NULL;
    if (dbcond.bad()) {
        errmsg("TI_buildImages: cannot query database");
        return OFFalse;
    }

    while (dbStatus.status == STATUS_Pending) {
        dbcond = DB_nextFindResponse(db->dbHandle, &reply, &dbStatus);
        if (dbcond.bad()) {
            errmsg("TI_buildImages: database error");
            return OFFalse;
        }
        if (dbStatus.status == STATUS_Pending) {
            if (verbose) {
                printf("REPLY OBJECT:\n");
                reply->print(COUT);
            }
            TI_addImageEntry(series, reply);
            delete reply; reply = NULL;
        }
    }

    if (series->imageCount > 0) {
        /* sort the images into assending image number order */
        qsort(series->images, series->imageCount, sizeof(series->images[0]),
              TI_imageCompare);
    }

    return OFTrue;
}

void
TI_getInfoFromDataset(DcmDataset *dset, DIC_PN patientsName, DIC_CS studyId,
    DIC_IS seriesNumber, DIC_CS modality, DIC_IS imageNumber)
{
    DU_getStringDOElement(dset, DCM_PatientsName, patientsName);
    DU_stripLeadingAndTrailingSpaces(patientsName);
    DU_getStringDOElement(dset, DCM_StudyID, studyId);
    DU_stripLeadingAndTrailingSpaces(studyId);
    DU_getStringDOElement(dset, DCM_SeriesNumber, seriesNumber);
    DU_stripLeadingAndTrailingSpaces(seriesNumber);
    DU_getStringDOElement(dset, DCM_Modality, modality);
    DU_stripLeadingAndTrailingSpaces(modality);
    DU_getStringDOElement(dset, DCM_InstanceNumber, imageNumber);
    DU_stripLeadingAndTrailingSpaces(imageNumber);
}

void
TI_getInfoFromImage(char *imgFile, DIC_PN patientsName, DIC_CS studyId,
    DIC_IS seriesNumber, DIC_CS modality, DIC_IS imageNumber)
{
    DcmFileFormat dcmff;
    if (dcmff.loadFile(imgFile).bad())
    {
        errmsg("Help!, cannot open image file: %s", imgFile);
        return;
    }

    DcmDataset *obj = dcmff.getDataset();

    TI_getInfoFromDataset(obj, patientsName, studyId, seriesNumber,
        modality, imageNumber);
}

/*
** CVS Log
** $Log: tiquery.cc,v $
** Revision 1.1  2005/08/23 19:32:03  braindead
** - initial savannah import
**
** Revision 1.1  2005/06/26 19:26:14  pipelka
** - added dcmtk
**
** Revision 1.20  2004/02/04 15:38:55  joergr
** Removed acknowledgements with e-mail addresses from CVS log.
**
** Revision 1.19  2002/11/29 07:18:18  wilkens
** Adapted ti utility to command line classes and added option '-xi'.
**
** Revision 1.18  2002/11/27 13:27:03  meichel
** Adapted module imagectn to use of new header file ofstdinc.h
**
** Revision 1.17  2002/08/20 12:22:50  meichel
** Adapted code to new loadFile and saveFile methods, thus removing direct
**   use of the DICOM stream classes.
**
** Revision 1.16  2001/10/12 12:42:57  meichel
** Adapted imagectn to OFCondition based dcmnet module (supports strict mode).
**
** Revision 1.15  2001/09/26 16:06:35  meichel
** Adapted imagectn to class OFCondition
**
** Revision 1.14  2001/06/01 15:51:23  meichel
** Updated copyright header
**
** Revision 1.13  2000/12/15 13:25:11  meichel
** Declared qsort() and signal() callback functions as extern "C", avoids
**   warnings on Sun C++ 5.x compiler.
**
** Revision 1.12  2000/04/14 16:38:21  meichel
** Removed default value from output stream passed to print() method.
**   Required for use in multi-thread environments.
**
** Revision 1.11  2000/03/08 16:41:03  meichel
** Updated copyright header.
**
** Revision 1.10  1999/07/14 12:03:39  meichel
** Updated data dictionary for supplement 29, 39, 33_lb, CP packet 4 and 5.
**   Corrected dcmtk applications for changes in attribute name constants.
**
** Revision 1.9  1999/06/10 12:12:05  meichel
** Adapted imagectn to new command line option scheme.
**   Added support for Patient/Study Only Q/R model and C-GET (experimental).
**
** Revision 1.8  1999/03/22 09:56:13  meichel
** Reworked data dictionary based on the 1998 DICOM edition and the latest
**   supplement versions. Corrected dcmtk applications for minor changes
**   in attribute name constants.
**
** Revision 1.7  1998/05/22 07:51:43  meichel
** Corrected two memory leaks in the ti application.
**
** Revision 1.6  1997/08/05 07:40:05  andreas
** Change definition of path to database index now using consistently
** the defines PATH_SEPARATOR and DBINDEXFILE
**
** Revision 1.5  1997/07/21 08:59:48  andreas
** - Replace all boolean types (BOOLEAN, CTNBOOLEAN, DICOM_BOOL, BOOL)
**   with one unique boolean type OFBool.
**
** Revision 1.4  1997/05/30 07:33:24  meichel
** Added space characters around comments and simplified
** some inlining code (needed for SunCC 2.0.1).
**
** Revision 1.3  1996/09/27 08:46:24  hewett
** Enclosed system include files with BEGIN_EXTERN_C/END_EXTERN_C.
**
** Revision 1.2  1996/04/25 16:29:11  hewett
** Added char* parameter casts to bzero() calls.
**
** Revision 1.1  1996/04/22 10:27:27  hewett
** Initial release.
**
**
*/
