#include "imagepool.h"
#include "gtkmm.h"
#include "dcmtk/dcmdata/dcdatset.h"
#include "dcmtk/dcmdata/dcdeftag.h"
#include "dcmtk/dcmdata/dcdicdir.h"
#include "dcmtk/dcmdata/dcdirrec.h"
#include "gettext.h"

#include <iostream>

namespace ImagePool {

    // Some forward declaration
    
	Glib::RefPtr< ImagePool::Study > create_query_study(DcmDataset* dset, const std::string& server);
  
	Glib::RefPtr< ImagePool::Series > create_query_series(DcmDataset* dset);
  
    static bool open_dicomdir_patient(
        const Glib::ustring &dicomdir,
        DcmDirectoryRecord *patRec,
        const sigc::slot< void, const Glib::RefPtr< ImagePool::Study >& >& resultslot
    );

    static bool open_dicomdir_study(
        const Glib::ustring &dicomdir,
        DcmDirectoryRecord *patRec,
        DcmDirectoryRecord *studyRec,
        const sigc::slot< void, const Glib::RefPtr< ImagePool::Study >& >& resultslot
    );

    static void open_dicomdir_series_result(
        const Glib::ustring &dicomdir,
        DcmDirectoryRecord *patRec,
        DcmDirectoryRecord *studyRec,
        const sigc::slot< void, const Glib::RefPtr< ImagePool::Series >& >& resultslot
    );

    /**
     * Reads a dicomdir and signals every found study
     */
    void open_dicomdir( const Glib::ustring &dicomdir, const sigc::slot< void, const Glib::RefPtr< ImagePool::Study >& >& resultslot) {
        DcmDicomDir dir(dicomdir.c_str());
        OFCondition ret;

        if ( (ret=dir.error()) != ECC_Normal ) {
            std::cout << "DICOMDIR Error: " << ret.text() << std::endl;
            return;
        }

        DcmDirectoryRecord root = dir.getRootRecord();
        DcmDirectoryRecord *rec = root.nextSub(NULL);
        std::cout << "Reading DICOMDIR from [" << dicomdir << "]\n";
        while ( rec != NULL ) {
            switch ( rec->getRecordType() ) {
            case ERT_Patient:
                open_dicomdir_patient(dicomdir, rec, resultslot);
                break;
            case ERT_HangingProtocol:
                // FALLTHROUGH
            case ERT_Private:
                break;
            default:
                std::cout << "WARNING: Bad DICOMDIR Record type[" << rec->getRecordType() << "] found\n";
            }
            rec = root.nextSub(rec);
        }
        /*
        // Leggo il root record, � un paziente ?
        {
        case ERT_root = 0,
        case ERT_Curve = 1,
        case ERT_FilmBox = 2,
        case ERT_FilmSession = 3,
        case ERT_Image = 4,
        case ERT_ImageBox = 5,
        case ERT_Interpretation = 6,
        ERT_ModalityLut = 7,
        ERT_Mrdr = 8,
        ERT_Overlay = 9,
        ERT_Patient = 10,
        ERT_PrintQueue = 11,
        ERT_Private = 12,
        ERT_Results = 13,
        ERT_Series = 14,
        ERT_Study = 15,
        ERT_StudyComponent = 16,
        ERT_Topic = 17,
        ERT_Visit = 18,
        ERT_VoiLut = 19,
        ERT_StructReport = 20,
        ERT_Presentation = 21,
        ERT_Waveform = 22,
        ERT_RTDose = 23,
        ERT_RTStructureSet = 24,
        ERT_RTPlan = 25,
        ERT_RTTreatRecord = 26,
        ERT_StoredPrint = 27,
        ERT_KeyObjectDoc = 28,
        ERT_Registration = 29,
        ERT_Fiducial = 30,
        ERT_RawData = 31,
        ERT_Spectroscopy = 32,
        ERT_EncapDoc = 33,
        ERT_ValueMap = 34,
        ERT_HangingProtocol = 35
        */
    }

    static bool open_dicomdir_patient(const Glib::ustring& dicomdir, DcmDirectoryRecord *patRec, const sigc::slot< void, const Glib::RefPtr< ImagePool::Study >& >& resultslot) {
        bool studyFound(false);
        DcmDirectoryRecord *subRec;

        assert(patRec->getRecordType()==ERT_Patient);
        subRec = patRec->nextSub(NULL);
        while ( subRec ) {
            switch ( subRec->getRecordType() ) {
            case ERT_Study:
                studyFound = open_dicomdir_study(dicomdir, patRec, subRec, resultslot);
                break;
            case ERT_Private:
                break;
            default:
                std::cout << "WARNING: Bad DICOMDIR SubRecord type[" << subRec->getRecordType() << "] for Patient found\n";
            }
            subRec = patRec->nextSub(subRec);
        }
        return studyFound;
    }

    static bool open_dicomdir_study(const Glib::ustring& dicomdir, DcmDirectoryRecord *patRec, DcmDirectoryRecord *studyRec, const sigc::slot< void, const Glib::RefPtr< ImagePool::Study >& >& resultslot) {
        bool seriesFound(false);
        DcmDirectoryRecord *subRec;

        assert(studyRec->getRecordType()==ERT_Study);
        subRec = studyRec->nextSub(NULL);
        while ( subRec && !seriesFound ) {
            switch ( subRec->getRecordType() ) {
            case ERT_Series:
                // Check if valid series (contains at least 1 image)
                seriesFound = true;
                break;
            case ERT_Private:
                break;
            default:
                std::cout << "WARNING: Bad DICOMDIR SubRecord type[" << subRec->getRecordType() << "] for Study found\n";
            }
            subRec = studyRec->nextSub(subRec);
        }
        if ( seriesFound ) {
            DcmDataset study;
            DcmElement *el;

            if ( studyRec->findAndGetElement(DCM_SpecificCharacterSet, el, OFFalse, OFTrue) == ECC_Normal )
                study.insert(el);
            if ( studyRec->findAndGetElement(DCM_StudyInstanceUID, el, OFFalse, OFTrue) == ECC_Normal )
                study.insert(el);
            if ( studyRec->findAndGetElement(DCM_StudyDate, el, OFFalse, OFTrue) == ECC_Normal )
                study.insert(el);
            if ( studyRec->findAndGetElement(DCM_StudyTime, el, OFFalse, OFTrue) == ECC_Normal )
                study.insert(el);
            if ( studyRec->findAndGetElement(DCM_StudyDescription, el, OFFalse, OFTrue) == ECC_Normal )
                study.insert(el);
            if ( patRec->findAndGetElement(DCM_PatientName, el, OFFalse, OFTrue) == ECC_Normal )
                study.insert(el);
            if ( patRec->findAndGetElement(DCM_PatientBirthDate, el, OFFalse, OFTrue) == ECC_Normal )
                study.insert(el);
            if ( patRec->findAndGetElement(DCM_PatientSex, el, OFFalse, OFTrue) == ECC_Normal )
                study.insert(el);
            resultslot(create_query_study(&study, std::string("DICOMDIR:") + dicomdir));
        }
        return seriesFound;
    }

    void open_dicomdir_series(const std::string& studyinstanceuid, const Glib::ustring& dicomdir, const sigc::slot< void, const Glib::RefPtr< ImagePool::Series >& >& resultslot) {
        DcmDicomDir dir(dicomdir.c_str());
        DcmDirectoryRecord *patRec;
        DcmDirectoryRecord *studyRec;
        DcmDirectoryRecord *seriesRec;
        OFCondition ret;

        if ( dir.error() != ECC_Normal ) {
            std::cout << "DICOMDIR Error: " << ret.text() << std::endl;
            return;
        }

        DcmDirectoryRecord &root = dir.getRootRecord();
        for ( patRec = root.nextSub(NULL); patRec!=NULL; patRec = root.nextSub(patRec) ) {
            switch ( patRec->getRecordType() ) {
            case ERT_Patient:
                for ( studyRec=patRec->nextSub(NULL); studyRec; studyRec = patRec->nextSub(studyRec) ) {
                    if ( studyRec->getRecordType()==ERT_Study ) {
                        OFString uid;
                        if ( studyRec->findAndGetOFString(DCM_StudyInstanceUID, uid)==ECC_Normal ) {
                            if ( studyinstanceuid == uid.c_str() ) {
                                open_dicomdir_series_result(dicomdir, patRec, studyRec, resultslot);
                                return;
                            }
                        }
                    }
                }
                break;
            case ERT_HangingProtocol:
                // FALLTHROUGH
            case ERT_Private:
                break;
            default:
                break;
            }
        }
        std::cout << "WARNING: study[" << studyinstanceuid << "] not found in DICOMDIR\n";
    }

    static void open_dicomdir_series_result(const Glib::ustring &dicomdir, DcmDirectoryRecord *patRec, DcmDirectoryRecord *studyRec, const sigc::slot< void, const Glib::RefPtr< ImagePool::Series >& >& resultslot) {
        DcmDirectoryRecord *seriesRec;
        DcmDirectoryRecord *sopRec;
        assert(studyRec->getRecordType()==ERT_Study);

        seriesRec = studyRec->nextSub(NULL);
        while ( seriesRec ) {
            DcmDataset series;
            DcmElement *el;

            if ( seriesRec->findAndGetElement(DCM_SpecificCharacterSet, el, OFFalse, OFTrue) == ECC_Normal )
                series.insert(el);
            if ( seriesRec->findAndGetElement(DCM_SeriesDescription, el, OFFalse, OFTrue) == ECC_Normal )
                series.insert(el);
            if ( seriesRec->findAndGetElement(DCM_SeriesInstanceUID, el, OFFalse, OFTrue) == ECC_Normal )
                series.insert(el);
            if ( seriesRec->findAndGetElement(DCM_Modality, el, OFFalse, OFTrue) == ECC_Normal )
                series.insert(el);
            if ( seriesRec->findAndGetElement(DCM_SeriesDate, el, OFFalse, OFTrue) == ECC_Normal )
                series.insert(el);
            if ( seriesRec->findAndGetElement(DCM_SeriesTime, el, OFFalse, OFTrue) == ECC_Normal )
                series.insert(el);
            if ( studyRec->findAndGetElement(DCM_StudyDescription, el, OFFalse, OFTrue) == ECC_Normal )
                series.insert(el);
            if ( studyRec->findAndGetElement(DCM_StationName, el, OFFalse, OFTrue) == ECC_Normal )
                series.insert(el);

            // Count Related SOP Instances
            int nSop=0;
            sopRec = seriesRec->nextSub(NULL);
            while (sopRec) {
                nSop++;
                sopRec = seriesRec->nextSub(sopRec);
            }
            series.putAndInsertUint16(DCM_NumberOfSeriesRelatedInstances, nSop);
            resultslot(create_query_series(&series));
            seriesRec = studyRec->nextSub(seriesRec);
        }
    }

} // namespace ImagePool

