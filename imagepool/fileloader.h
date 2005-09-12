#ifndef IMAGEPOOL_FILELOADER_H
#define IMAGEPOOL_FILELOADER_H

#include "loader.h"
#include <vector>
#include <map>
#include <string>

namespace ImagePool {

class FileLoader: public Loader {
public:

	bool load(const Glib::SListHandle< Glib::ustring >& filelist);

protected:

	void run();

	void prescan_files(Glib::SListHandle< Glib::ustring >* filelist);

private:

	Glib::SListHandle< Glib::ustring >* m_filelist;
	
	std::map< std::string, int > m_studysize;

};

} // namespace ImagePool

#endif // IMAGEPOOL_FILELOADER_H
