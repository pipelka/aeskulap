/*
    Aeskulap ImagePool - DICOM abstraction library
    Copyright (C) 2005  Alexander Pipelka
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
    Alexander Pipelka
    pipelka@teleweb.at
    Last Update:      $Author: braindead $
    Update Date:      $Date: 2007/05/04 14:47:06 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/imagepool/dicomdirloader.cpp,v $
    CVS/RCS Revision: $Revision: 1.1 $
    Status:           $State: Exp $
*/
#include <gtkmm.h>
#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/dcmdata/dcdeftag.h"
#include "dcmtk/dcmdata/dcdicdir.h"
#include "dcmtk/dcmdata/dcdirrec.h"
#include "imagepool.h"
#include "dicomdirloader.h"

namespace ImagePool {

    std::string DicomdirLoader::ImageModalities =
        "CR|CT|MR|NM|US|OT|BI|CD|DD|DG|ES|LS|PT|RG|ST|TG|XA|RF|RTIMAGE|HC|DX|MG|"\
        "IO|PX|GM|SM|XC|OP|IVUS|DF|CF|DF|VF|AS|CS|LP|FA|CP|DM|FS|MA|MS";

    bool DicomdirLoader::load(const std::string &studyinstanceuid, const Glib::ustring &dicomdir) {
        DcmDicomDir dir(dicomdir.c_str());
        OFCondition ret;

        if (busy() ) {
            return false;
        }

        if ( (ret=dir.error()) != ECC_Normal ) {
            std::cout << "DicomdirLoader::load Error: " << ret.text() << std::endl;
            return false;
        }

        // Open DICOMDIR and look for StudyInstanceUID
        // Add all IMAGE record types to m_FileList and fill cache
        // FileLoader::Start
        DcmDirectoryRecord *studyRec = find_study(studyinstanceuid, dir);
        if ( !studyRec ) {
            std::cout << "DicomdirLoader::load Error: cannot find study" << std::endl;
            return false;
        }

        m_filelist = new std::list< Glib::ustring >;
        m_cache.clear();
        if ( !scan_study(studyinstanceuid, studyRec, dicomdir) ) {
            std::cout << "DicomdirLoader::load: no visible images" << std::endl;
            return false;
        }

        FileLoader::start();

        return true;

    }

    DcmDirectoryRecord* DicomdirLoader::find_study(const std::string &studyinstanceuid, class DcmDicomDir &dir) {
        DcmDirectoryRecord *patRec;
        DcmDirectoryRecord *studyRec;
        OFCondition ret;

        DcmDirectoryRecord &root = dir.getRootRecord();
        for ( patRec = root.nextSub(NULL); patRec != NULL; patRec = root.nextSub(patRec) ) {
            if ( patRec->getRecordType() == ERT_Patient ) {
                for ( studyRec = patRec->nextSub(NULL); studyRec; studyRec = patRec->nextSub(studyRec) ) {
                    if ( studyRec->getRecordType()==ERT_Study ) {
                        OFString uid;
                        if ( studyRec->findAndGetOFString(DCM_StudyInstanceUID, uid)==ECC_Normal ) {
                            if ( studyinstanceuid == uid.c_str() )
                                return studyRec;
                        }
                    }
                }
            }
        }
        return NULL;
    }

    // Loads all dicom file in m_FileList
    // Initializes m_Cache
    // returns true if viewable files found, otherwise false
    bool DicomdirLoader::scan_study(const std::string &studyinstanceuid, class DcmDirectoryRecord *studyRec, const Glib::ustring &dicomdir) {
        DcmDirectoryRecord *seriesRec;
        DcmDirectoryRecord *sopRec;
        std::string path;
        std::string file;

        assert(studyRec->getRecordType()==ERT_Study);

        path = Glib::path_get_dirname(dicomdir);
        seriesRec = studyRec->nextSub(NULL);
        while ( seriesRec ) {
            OFString modality;

            if ( seriesRec->findAndGetOFString(DCM_Modality, modality) == ECC_Normal ) {
                OFString seriesinstanceuid;
                if ( seriesRec->findAndGetOFString(DCM_SeriesInstanceUID, seriesinstanceuid) != ECC_Normal ) {
                    seriesRec = studyRec->nextSub(seriesRec);
                    continue;
                }

                if ( ImageModalities.find(modality.c_str()) != std::string::npos  ) {
                    // Load Series...
                    OFString fileID;
                    int vm;
                    int i;
                    DcmElement *el;
                    for (sopRec = seriesRec->nextSub(NULL); sopRec; sopRec = seriesRec->nextSub(sopRec) ) {
                        switch ( sopRec->getRecordType() ) {
                        case ERT_Image:
                        case ERT_StoredPrint:
                            if ( sopRec->findAndGetElement(DCM_ReferencedFileID, el, true)!=ECC_Normal ) {
                                sopRec = seriesRec->nextSub(sopRec);
                                continue;
                            }
                            vm = el->getVM();
                            file = "";
                            for ( i=0; i<vm; i++ ) {
                                el->getOFString(fileID, i);
                                file = file + "/" + fileID.c_str();
                            }

                            if ( file.size() > 0 ) {
                                std::cout << "Loading DICOMDIR file [" << path.c_str() << file.c_str() << "]" << std::endl;
                                m_filelist->push_back(path + file);

                                std::string SeriesUID = seriesinstanceuid.c_str();
                                m_cache[studyinstanceuid].m_instancecount++;
                                m_cache[studyinstanceuid].m_seriesuid.insert(SeriesUID);
                                m_cache[studyinstanceuid].m_seriescount = m_cache[studyinstanceuid].m_seriesuid.size();
                            }
                            break;
                        default:
                            break;
                        }
                    }
                }
            }
            seriesRec = studyRec->nextSub(seriesRec);
        }
        return true;
    }

} // namespace ImagePool
