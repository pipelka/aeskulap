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
 *  Purpose: TI Query Routines
 *
 *  Last Update:      $Author: braindead $
 *  Update Date:      $Date: 2005/08/23 19:32:03 $
 *  Source File:      $Source: /cvsroot/aeskulap/aeskulap/dcmtk/imagectn/apps/tiquery.h,v $
 *  CVS/RCS Revision: $Revision: 1.1 $
 *  Status:           $State: Exp $
 *
 *  CVS/RCS Log at end of file
 *
 */

#ifndef TI_QUERY_H
#define TI_QUERY_H

#include "osconfig.h"    /* make sure OS specific configuration is included first */

#include "ti.h"
#include "dicom.h"
#include "imagedb.h"
#include "diutil.h"

OFBool
TI_dbReadable(const char *dbTitle);
/*
 * Returns true if the db associated with dbTitle exists and is readable.
 */
 
time_t 
TI_dbModifyTime(const char *dbTitle);
/*
 * Returns the time the db associated with dbTitle was last modified.
 */

/* study level */
void
TI_destroyStudyEntries(TI_DBEntry *db);
OFBool
TI_buildStudies(TI_Config *conf, TI_DBEntry *db);
/*
 * Interogate the database and build up a study structure.
 *
 */

/* series level */
void
TI_destroySeriesEntries(TI_StudyEntry *study);
OFBool
TI_buildSeries(TI_Config *conf, TI_DBEntry *db, TI_StudyEntry *study);

/* image level */
void
TI_destroyImageEntries(TI_SeriesEntry *series);
OFBool
TI_buildImages(TI_Config *conf, TI_DBEntry *db, TI_StudyEntry *study,
    TI_SeriesEntry *series);


/* image info */

void 
TI_getInfoFromDataset(DcmDataset *dset, DIC_PN patientsName, DIC_CS studyId,
    DIC_IS seriesNumber, DIC_CS modality, DIC_IS imageNumber);

void
TI_getInfoFromImage(char *imgFile, DIC_PN patientsName, DIC_CS studyId,
    DIC_IS seriesNumber, DIC_CS modality, DIC_IS imageNumber);

#endif /* TI_QUERY_H */

/*
** CVS Log
** $Log: tiquery.h,v $
** Revision 1.1  2005/08/23 19:32:03  braindead
** - initial savannah import
**
** Revision 1.1  2005/06/26 19:26:14  pipelka
** - added dcmtk
**
** Revision 1.5  2001/06/01 15:51:24  meichel
** Updated copyright header
**
** Revision 1.4  2000/03/08 16:41:04  meichel
** Updated copyright header.
**
** Revision 1.3  1999/06/10 12:12:06  meichel
** Adapted imagectn to new command line option scheme.
**   Added support for Patient/Study Only Q/R model and C-GET (experimental).
**
** Revision 1.2  1997/07/21 08:59:48  andreas
** - Replace all boolean types (BOOLEAN, CTNBOOLEAN, DICOM_BOOL, BOOL)
**   with one unique boolean type OFBool.
**
** Revision 1.1  1996/04/22 10:27:28  hewett
** Initial release.
**
**
*/
