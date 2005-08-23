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
 *  Author:  Didier Lemoine
 *
 *  Purpose: private data definitions used to implement the DB Module Module. Prefix: DB_ 
 *
 *  Last Update:      $Author: braindead $
 *  Update Date:      $Date: 2005/08/23 19:32:07 $
 *  Source File:      $Source: /cvsroot/aeskulap/aeskulap/dcmtk/imagectn/include/dbpriv.h,v $
 *  CVS/RCS Revision: $Revision: 1.1 $
 *  Status:           $State: Exp $
 *
 *  CVS/RCS Log at end of file
 *
 */

#ifndef DATAPRIV_H
#define DATAPRIV_H

#include "osconfig.h"    /* make sure OS specific configuration is included first */

#include "imagedb.h"
#include "dicom.h"
#include "dcdatset.h"
#include "dcuid.h"
#include "dcdeftag.h"
BEGIN_EXTERN_C
#ifdef HAVE_IO_H
#define access my_access    // Workaround to make Visual C++ Compiler happy!
#include <io.h>
#undef access
#endif
END_EXTERN_C

#define STORE_SERVICE   1
#define FIND_SERVICE    2
#define MOVE_SERVICE    3

/*
** Maximum size of things to put in db records.
** Some values will have vm>1 thus these maximums are
** intended to leave enough space for most common uses.
*/
#define AE_MAX_LENGTH   128     /* Application Entity    */
#define AS_MAX_LENGTH   32      /* Age String            */
#define AT_MAX_LENGTH   32      /* Attribute Tag         */
#define CS_MAX_LENGTH   128     /* Code String           */
#define DA_MAX_LENGTH   80      /* Date                  */
#define DS_MAX_LENGTH   128     /* Decimal String        */
#define DT_MAX_LENGTH   208     /* Date Time             */
#define FL_MAX_LENGTH   32      /* FLoating point single */
#define FD_MAX_LENGTH   64      /* Floating point Double */
#define IS_MAX_LENGTH   96      /* Integer String        */
#define LO_MAX_LENGTH   64      /* Long String           */
#define LT_MAX_LENGTH   10240   /* Long Text             */
#define PN_MAX_LENGTH   64      /* Person Name           */
#define SH_MAX_LENGTH   16      /* Short String          */
#define SL_MAX_LENGTH   32      /* Signed Long           */
#define SS_MAX_LENGTH   16      /* Signed Short          */
#define ST_MAX_LENGTH   1024    /* Short Text            */
#define TM_MAX_LENGTH   128     /* Time                  */
#define UI_MAX_LENGTH   64      /* Unique Identifier     */
#define UL_MAX_LENGTH   32      /* Unsigned Long         */
#define US_MAX_LENGTH   16      /* Unsigned Short        */
#define CS_LABEL_MAX_LENGTH 16  /* Code String - Presentation Label */
#define DESCRIPTION_MAX_LENGTH 128  /* Not related to any particular DICOM attribute */

#define DBC_MAXSTRING           256

#define MAX_MAX_STUDIES         DB_UpperMaxStudies
#define MAX_NUMBER_OF_IMAGES    10000
#define SIZEOF_IDXRECORD        (sizeof (IdxRecord))
#define SIZEOF_STUDYDESC        (sizeof (StudyDescRecord) * MAX_MAX_STUDIES)


typedef struct DB_SSmallDcmElmt {
    char* PValueField ;     
    Uint32 ValueLength ;        
    DcmTagKey XTag ;
    DB_SSmallDcmElmt(); /* default constructor defined in dbutils.cc */
    
private:
    /* undefined */ DB_SSmallDcmElmt(const DB_SSmallDcmElmt& copy);
    /* undefined */ DB_SSmallDcmElmt& operator=(const DB_SSmallDcmElmt& copy);
} DB_SmallDcmElmt ; 


typedef enum {
    UNIQUE_KEY,
    REQUIRED_KEY,
    OPTIONAL_KEY
} DB_KEY_TYPE ;

/*
typedef enum {
    DB_CANNOTCREATEHANDLE_CannotAllocate,
    DB_CANNOTCREATEHANDLE_CannotOpenIdx,
    DB_CANNOTDESTROYHANDLE_CannotCloseIdx,
    DB_FIND,
    DB_NOTFIND,
    DB_INVALIDDATA,
    DB_ERROR
} DB_CONDITION ;
*/

typedef enum {
    PATIENT_LEVEL,
    STUDY_LEVEL,
    SERIE_LEVEL,
    IMAGE_LEVEL
} DB_LEVEL ;

typedef enum {
    PATIENT_ROOT,
    STUDY_ROOT,
    PATIENT_STUDY
} DB_QUERY_CLASS ;

struct db_ElementList {
    DB_SmallDcmElmt elem ;      
    struct db_ElementList *next ;

    db_ElementList(): elem(), next(NULL) {}
private:
    /* undefined */ db_ElementList(const db_ElementList& copy);
    /* undefined */ db_ElementList& operator=(const db_ElementList& copy);
};

typedef struct db_ElementList DB_ElementList;

struct db_UidList {
    char *patient ;
    char *study ;
    char *serie ;
    char *image ;
    struct db_UidList *next ;
};

typedef struct db_UidList DB_UidList ;


struct db_IntegerList {
    int idxCounter ;
    struct db_IntegerList *next ;
};
typedef struct db_IntegerList DB_CounterList ;


typedef enum {
    DATE_CLASS,
    TIME_CLASS,
    UID_CLASS,
    STRING_CLASS,
    OTHER_CLASS
} DB_KEY_CLASS ;

struct DB_FindAttr {
    DcmTagKey tag ;
    DB_LEVEL level ;
    DB_KEY_TYPE keyAttr ;
    DB_KEY_CLASS keyClass ;

    /* to passify some C++ compilers */
    DB_FindAttr(const DcmTagKey& t, DB_LEVEL l, DB_KEY_TYPE kt, DB_KEY_CLASS kc) 
        : tag(t), level(l), keyAttr(kt), keyClass(kc) { }
};



typedef struct {
    int pidx ;
    DB_ElementList *findRequestList ;
    DB_ElementList *findResponseList ;
    DB_LEVEL queryLevel ;
    char indexFilename[DBC_MAXSTRING+1] ;
    char storageArea[DBC_MAXSTRING+1] ;
    long maxBytesPerStudy ;
    long maxStudiesAllowed ;
    int idxCounter ;
    DB_CounterList *moveCounterList ;
    int NumberRemainOperations ;
    DB_QUERY_CLASS rootLevel ;
    DB_UidList *uidList ;
} DB_Private_Handle ;


typedef struct  {
    char StudyInstanceUID [UI_MAX_LENGTH] ;
    long StudySize ;
    double LastRecordedDate ;
    int NumberofRegistratedImages ;
} StudyDescRecord ;


typedef struct  {
    int idxCounter ;
    double RecordedDate ; 
    long ImageSize ;
} ImagesofStudyArray ;


/* the following constants define which array element
 * of the param[] array in the IdxRecord structure
 * is linked to which value field in the IdxRecord.
 * numbers must be continuous, starting with 0.
 *
 * The constant NBPARAMETERS must contain the number
 * of array elements to be referenced in param[]
 * (= highest index +1)
 */

#define RECORDIDX_PatientsBirthDate               0
#define RECORDIDX_PatientsSex                     1
#define RECORDIDX_PatientsName                    2
#define RECORDIDX_PatientID                       3
#define RECORDIDX_PatientsBirthTime               4
#define RECORDIDX_OtherPatientIDs                 5
#define RECORDIDX_OtherPatientNames               6
#define RECORDIDX_EthnicGroup                     7
#define RECORDIDX_NumberofPatientRelatedStudies   8
#define RECORDIDX_NumberofPatientRelatedSeries    9
#define RECORDIDX_NumberofPatientRelatedInstances 10
#define RECORDIDX_StudyDate                      11
#define RECORDIDX_StudyTime                      12
#define RECORDIDX_StudyID                        13
#define RECORDIDX_StudyDescription               14
#define RECORDIDX_NameOfPhysiciansReadingStudy   15
#define RECORDIDX_AccessionNumber                16
#define RECORDIDX_ReferringPhysiciansName        17
#define RECORDIDX_ProcedureDescription           18
#define RECORDIDX_AttendingPhysiciansName        19
#define RECORDIDX_StudyInstanceUID               20
#define RECORDIDX_OtherStudyNumbers              21
#define RECORDIDX_AdmittingDiagnosesDescription  22
#define RECORDIDX_PatientsAge                    23
#define RECORDIDX_PatientsSize                   24
#define RECORDIDX_PatientsWeight                 25
#define RECORDIDX_Occupation                     26
#define RECORDIDX_NumberofStudyRelatedSeries     27
#define RECORDIDX_NumberofStudyRelatedInstances  28
#define RECORDIDX_SeriesNumber                   29
#define RECORDIDX_SeriesInstanceUID              30
#define RECORDIDX_Modality                       31
#define RECORDIDX_ImageNumber                    32
#define RECORDIDX_SOPInstanceUID                 33
#define RECORDIDX_SeriesDate                     34
#define RECORDIDX_SeriesTime                     35
#define RECORDIDX_SeriesDescription              36
#define RECORDIDX_ProtocolName                   37
#define RECORDIDX_OperatorsName                  38
#define RECORDIDX_PerformingPhysiciansName       39
#define RECORDIDX_PresentationLabel              40

#define NBPARAMETERS                             41


struct IdxRecord {

    char    filename                        [DBC_MAXSTRING+1] ;
    char    SOPClassUID                     [UI_MAX_LENGTH+1] ;
    double  RecordedDate ; 
    int     ImageSize ;

    DB_SmallDcmElmt param                   [NBPARAMETERS] ;

    char    PatientsBirthDate               [DA_MAX_LENGTH+1] ;
    char    PatientsSex                     [CS_MAX_LENGTH+1] ;
    char    PatientsName                    [PN_MAX_LENGTH+1] ;
    char    PatientID                       [LO_MAX_LENGTH+1] ; 
    char    PatientsBirthTime               [TM_MAX_LENGTH+1] ; 
    char    OtherPatientIDs                 [LO_MAX_LENGTH+1] ; 
    char    OtherPatientNames               [PN_MAX_LENGTH+1] ; 
    char    EthnicGroup                     [SH_MAX_LENGTH+1] ; 
    char    NumberofPatientRelatedStudies   [IS_MAX_LENGTH+1] ; 
    char    NumberofPatientRelatedSeries    [IS_MAX_LENGTH+1] ; 
    char    NumberofPatientRelatedInstances [IS_MAX_LENGTH+1] ;
    
    char    StudyDate                       [DA_MAX_LENGTH+1] ; 
    char    StudyTime                       [TM_MAX_LENGTH+1] ; 
    char    StudyID                         [CS_MAX_LENGTH+1] ;
    char    StudyDescription                [LO_MAX_LENGTH+1] ; 
    char    NameOfPhysiciansReadingStudy    [PN_MAX_LENGTH+1] ;

    char    AccessionNumber                 [CS_MAX_LENGTH+1] ;
    char    ReferringPhysiciansName         [PN_MAX_LENGTH+1] ;
    char    ProcedureDescription            [LO_MAX_LENGTH+1] ;
    char    AttendingPhysiciansName         [PN_MAX_LENGTH+1] ;
    char    StudyInstanceUID                [UI_MAX_LENGTH+1] ;
    char    OtherStudyNumbers               [IS_MAX_LENGTH+1] ;
    char    AdmittingDiagnosesDescription   [LO_MAX_LENGTH+1] ;
    char    PatientsAge                     [AS_MAX_LENGTH+1] ;
    char    PatientsSize                    [DS_MAX_LENGTH+1] ;
    char    PatientsWeight                  [DS_MAX_LENGTH+1] ;
    char    Occupation                      [SH_MAX_LENGTH+1] ;
    char    NumberofStudyRelatedSeries      [IS_MAX_LENGTH+1] ;
    char    NumberofStudyRelatedInstances   [IS_MAX_LENGTH+1] ;
    
    char    SeriesNumber                    [IS_MAX_LENGTH+1] ;
    char    SeriesInstanceUID               [UI_MAX_LENGTH+1] ;
    char    Modality                        [CS_MAX_LENGTH+1] ;
    
    char    ImageNumber                     [IS_MAX_LENGTH+1] ;
    char    SOPInstanceUID                  [UI_MAX_LENGTH+1] ;

    char    SeriesDate                      [DA_MAX_LENGTH+1] ; 
    char    SeriesTime                      [TM_MAX_LENGTH+1] ; 
    char    SeriesDescription               [LO_MAX_LENGTH+1] ; 
    char    ProtocolName                    [LO_MAX_LENGTH+1] ;
    char    OperatorsName                   [PN_MAX_LENGTH+1] ;
    char    PerformingPhysiciansName        [PN_MAX_LENGTH+1] ;
    char    PresentationLabel               [CS_LABEL_MAX_LENGTH+1] ;

    DVIFhierarchyStatus hstat;  

  // Not related to any particular DICOM attribute !
    char    InstanceDescription             [DESCRIPTION_MAX_LENGTH+1] ;
   
    IdxRecord(); /* defined in dbutils.cc */

private:
    /* undefined */ IdxRecord(const IdxRecord& copy);
    /* undefined */ IdxRecord& operator=(const IdxRecord& copy);
};


/** Level Strings
 */

#define PATIENT_LEVEL_STRING    "PATIENT"
#define STUDY_LEVEL_STRING      "STUDY"
#define SERIE_LEVEL_STRING      "SERIES"
#define IMAGE_LEVEL_STRING      "IMAGE"

/** Index functions prototypes
 */

extern void         DB_IdxInitRecord (IdxRecord *, int linksOnly) ; 

extern OFCondition    DB_IdxRead (DB_Private_Handle *phandle, int idx, IdxRecord *idxRec) ;
extern OFCondition    DB_IdxAdd (DB_Private_Handle *phandle, int *idx, IdxRecord *idxRec) ;
extern OFCondition    DB_IdxRemove (DB_Private_Handle *phandle, int idx);
extern OFCondition    DB_StudyDescChange (DB_Private_Handle *phandle, StudyDescRecord *pStudyDesc) ;
extern OFCondition    DB_IdxInitLoop (DB_Private_Handle *phandle, int *idx) ;
extern OFCondition    DB_IdxGetNext (DB_Private_Handle *phandle, int *idx, IdxRecord *idxRec) ;

/** Utility functions
 */

extern int DB_debugLevel;
extern void DB_debug(int level, const char* format, ...);

extern int      DB_StringUnify(char *, char *) ;
extern OFCondition    DB_FreeElementList (DB_ElementList *) ;
extern OFCondition    DB_FreeUidList (DB_UidList *) ;
extern int      DB_TagSupported (DcmTagKey) ;
extern OFCondition    DB_GetUIDTag (DB_LEVEL, DcmTagKey *) ;
extern OFCondition    DB_GetTagLevel (DcmTagKey, DB_LEVEL *) ;
extern OFCondition    DB_GetTagKeyAttr (DcmTagKey, DB_KEY_TYPE *) ;
extern OFCondition    DB_GetTagKeyClass (DcmTagKey, DB_KEY_CLASS *) ;
extern int      DB_CharsetInElement (char *charset, DB_SmallDcmElmt *elt) ;
extern void     DB_RemoveSpaces (char *string) ;
extern void     DB_RemoveEnclosingSpaces (char *string) ;
extern long     DB_DateToLong (char *date) ;
extern double       DB_TimeToDouble (char *time) ;
extern void     DB_DuplicateElement (DB_SmallDcmElmt *src, DB_SmallDcmElmt *dst) ;
extern int      DB_MatchDate (DB_SmallDcmElmt *mod, DB_SmallDcmElmt *elt) ;
extern int      DB_MatchTime (DB_SmallDcmElmt *mod, DB_SmallDcmElmt *elt) ;
extern int      DB_MatchUID (DB_SmallDcmElmt *mod, DB_SmallDcmElmt *elt) ;
extern int      DB_MatchStrings (DB_SmallDcmElmt *mod, DB_SmallDcmElmt *elt) ;
extern int      DB_Match (DB_SmallDcmElmt *mod, DB_SmallDcmElmt *elt) ;
extern void     DB_MakeResponseList (DB_Private_Handle *phandle, IdxRecord *idxRec) ;
extern OFCondition    DB_HierarchicalCompare (DB_Private_Handle *phandle, IdxRecord *idxRec,
                    DB_LEVEL level, DB_LEVEL infLevel, int *match) ;
extern OFCondition    DB_CheckupinStudyDesc(DB_Private_Handle *phandle, StudyDescRecord *pStudyDesc, char *StudyUID, long imageSize) ;
extern int      DB_MatchStudyUIDInStudyDesc(StudyDescRecord *pStudyDesc, char *StudyUID, int maxStudiesAllowed) ;
extern OFCondition    DB_GetStudyDesc(DB_Private_Handle *phandle, StudyDescRecord *pStudyDesc) ;
extern OFCondition    DB_DeleteOldestImages(DB_Private_Handle *phandle, StudyDescRecord *pStudyDesc, int StudyNum, char *StudyUID, long RequiredSize) ;
extern int      DB_DeleteOldestStudy(DB_Private_Handle *phandle, StudyDescRecord *pStudyDesc) ;
extern "C" int      DB_Compare(const void *e1, const void *e2) ;

extern OFCondition DB_lock(DB_Private_Handle *phandle, OFBool exclusive);
extern OFCondition DB_unlock(DB_Private_Handle *phandle);

extern OFBool DB_doCheckFindIdentifier();
extern OFBool DB_doCheckMoveIdentifier();

extern void DB_printDataset(DcmDataset *ds);

#endif

/*
** CVS Log
** $Log: dbpriv.h,v $
** Revision 1.1  2005/08/23 19:32:07  braindead
** - initial savannah import
**
** Revision 1.1  2005/06/26 19:26:04  pipelka
** - added dcmtk
**
** Revision 1.15  2001/10/12 12:43:06  meichel
** Adapted imagectn to OFCondition based dcmnet module (supports strict mode).
**
** Revision 1.14  2001/06/01 15:51:26  meichel
** Updated copyright header
**
** Revision 1.13  2000/12/15 13:25:14  meichel
** Declared qsort() and signal() callback functions as extern "C", avoids
**   warnings on Sun C++ 5.x compiler.
**
** Revision 1.12  2000/10/16 11:34:48  joergr
** Replaced presentation description by a more general instance description.
**
** Revision 1.11  2000/03/08 16:41:07  meichel
** Updated copyright header.
**
** Revision 1.10  2000/03/06 16:27:55  meichel
** Added constructor declarations needed by gcc 2.5.8.
**
** Revision 1.9  1999/08/31 09:50:01  meichel
** Introduced default constructors for some imagectn structs
**   in order to passify some compiler warnings.
**
** Revision 1.8  1999/07/14 12:03:40  meichel
** Updated data dictionary for supplement 29, 39, 33_lb, CP packet 4 and 5.
**   Corrected dcmtk applications for changes in attribute name constants.
**
** Revision 1.7  1999/06/10 12:12:12  meichel
** Adapted imagectn to new command line option scheme.
**   Added support for Patient/Study Only Q/R model and C-GET (experimental).
**
** Revision 1.6  1999/04/29 15:23:15  joergr
** Added PresentationLabel to index file.
**
** Revision 1.5  1999/04/28 09:21:10  joergr
** Added item PresentationDescription to index record.
**
** Revision 1.4  1999/01/29 12:40:11  meichel
** Simplified some index file related code in imagectn to ease maintenance.
**
** Revision 1.3  1999/01/29 09:54:19  vorwerk
** Recordsize changed.
**
** Revision 1.2  1998/12/22 15:16:29  vorwerk
** - add elements in IdxRecord
**      char      StudyDescription        [LO_MAX_LENGTH+1] ;
**      char      NameOfPhysiciansReadingStudy [PN_MAX_LENGTH+1] ;
**      char      SOPInstanceUID          [UI_MAX_LENGTH+1] ;
**      DVIFhierarchyStatus       hstat;
**      char      SeriesDate              [DA_MAX_LENGTH+1] ;
**      char      SeriesTime              [TM_MAX_LENGTH+1] ;
**      char      SeriesDescription       [LO_MAX_LENGTH+1] ;
**      char      ProtocolName            [LO_MAX_LENGTH+1] ;
**      char      OperatorsName            [PN_MAX_LENGTH+1] ;
**      char      PerformingPhysiciansName                 [PN_MAX_LENGTH+1] ;
**
** Revision 1.1  1998/12/22 15:11:27  vorwerk
** removed from libsrc and added in include
**
** Revision 1.8  1997/07/21 08:59:57  andreas
** - Replace all boolean types (BOOLEAN, CTNBOOLEAN, DICOM_BOOL, BOOL)
**   with one unique boolean type OFBool.
**
** Revision 1.7  1997/06/26 12:51:00  andreas
** - Changed names for enumeration values in DB_KEY_TYPE since the value
**   OPTIONAL was predefined for Windows 95/NT
** - Include Additional headers (winsock.h, io.h) for Windows NT/95
**
** Revision 1.6  1996/05/30 17:45:18  hewett
** Modified the definition of a static array of structs which was causing
** some C++ compilers problems.
**
** Revision 1.5  1996/05/06 07:39:08  hewett
** Added explicit initialization (bzero) of idx record.  Rearranged
** size of string attributes in idx record.
**
** Revision 1.4  1996/04/29 15:16:05  hewett
** Removed unused DB_GetUSValue().
**
** Revision 1.3  1996/04/25 16:34:37  hewett
** Added workaround for gcc 2.5.8 compiler problem.  The compiler was
** complaining that the structs DB_FindAttr and IdxRecord had a bad
** constructor (they didn't have one!).  Conditionally added a constructor.
**
** Revision 1.2  1996/04/22 11:21:57  hewett
** Added declaration of DB_printDataset().  Useful for debugging help.
**
** Revision 1.1.1.1  1996/03/28 19:25:00  hewett
** Oldenburg Image CTN Software ported to use the dcmdata C++ toolkit.
**
**
*/
