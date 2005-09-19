#ifndef IMAGEPOOL_NETLOADER_H
#define IMAGEPOOL_NETLOADER_H

#include "loader.h"

namespace ImagePool {

class NetLoader : public Loader {
public:

	bool load(const std::string& studyinstanceuid, const std::string& server);
	
protected:

	bool run();

private:

	std::string m_studyinstanceuid;

	std::string m_server;

};
	
} // namespace ImagePool

#endif // IMAGEPOOL_NETLOADER_H
