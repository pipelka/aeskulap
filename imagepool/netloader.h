#ifndef IMAGEPOOL_NETLOADER_H
#define IMAGEPOOL_NETLOADER_H

#include "loader.h"

namespace ImagePool {

class NetLoader : public Loader {
public:

	bool load(const std::string& studyinstanceuid);
	
protected:

	bool run();

private:

	std::string m_studyinstanceuid;

};
	
} // namespace ImagePool

#endif // IMAGEPOOL_NETLOADER_H
