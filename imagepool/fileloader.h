#ifndef IMAGEPOOL_FILELOADER_H
#define IMAGEPOOL_FILELOADER_H

#include "loader.h"
#include <list>
#include <string>

namespace ImagePool {

class FileLoader: public Loader {
public:

	bool load(const std::list< Glib::ustring >& filelist);

	sigc::signal< void, double > signal_prescan_progress;

protected:

	bool run();

	void prescan_files(std::list< Glib::ustring >* filelist);

	std::list< Glib::ustring >* m_filelist;

};

} // namespace ImagePool

#endif // IMAGEPOOL_FILELOADER_H
