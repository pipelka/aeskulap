#ifndef IMAGEPOOL_DICOMDIRLOADER_H
#define IMAGEPOOL_DICOMDIRLOADER_H

#include "fileloader.h"
#include <list>
#include <string>

namespace ImagePool {

class DicomdirLoader: public FileLoader {
public:

    bool load(const std::string &studyinstanceuid, const Glib::ustring &dicomdir);

protected:

    class DcmDirectoryRecord *find_study(const std::string &studyinstanceuid, class DcmDicomDir &dir);

    bool scan_study(const std::string &studyinstanceuid, class DcmDirectoryRecord *study, const Glib::ustring &dicomdir);

private:

    static std::string ImageModalities;
};

} // namespace ImagePool

#endif // IMAGEPOOL_FILELOADER_H
