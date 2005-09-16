#ifndef IMAGEPOOL_FILELOADER_H
#define IMAGEPOOL_FILELOADER_H

#include "loader.h"
#include <list>
#include <map>
#include <string>

namespace ImagePool {

class FileLoader: public Loader {
public:

	bool load(const std::list< Glib::ustring >& filelist);

	sigc::signal< void, double > signal_prescan_progress;

protected:

	bool run();

	void prescan_files(std::list< Glib::ustring >* filelist);

private:

	std::list< Glib::ustring >* m_filelist;
	
	std::map< std::string, int > m_studysize;

};

} // namespace ImagePool

#endif // IMAGEPOOL_FILELOADER_H
