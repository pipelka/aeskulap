#ifndef IMAGEPOOL_FILELOADER_H
#define IMAGEPOOL_FILELOADER_H

#include "loader.h"
#include <vector>

namespace ImagePool {

class FileLoader: public Loader {
public:

	bool load(const Glib::SListHandle< Glib::ustring >& filelist);

protected:

	void run();

private:

	Glib::SListHandle< Glib::ustring >* m_filelist;
};

} // namespace ImagePool

#endif // IMAGEPOOL_FILELOADER_H
