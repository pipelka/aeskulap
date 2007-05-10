#ifndef IMAGEPOOL_NETLOADER_H
#define IMAGEPOOL_NETLOADER_H

#include "loader.h"
#include "poolstudy.h"

namespace ImagePool {

class NetLoader : public Loader {
public:

	NetLoader(const std::string& local_aet);

	bool load(const Glib::RefPtr< ImagePool::Study >& study, const std::string& server);
	
protected:

	bool run();

private:

	Glib::RefPtr< ImagePool::Study > m_study;

	std::string m_server;

	std::string m_local_aet;
};
	
} // namespace ImagePool

#endif // IMAGEPOOL_NETLOADER_H
