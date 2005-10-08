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
    Update Date:      $Date: 2005/10/08 10:32:58 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/imagepool/loader.h,v $
    CVS/RCS Revision: $Revision: 1.9 $
    Status:           $State: Exp $
*/

#ifndef IMAGEPOOL_LOADER_H
#define IMAGEPOOL_LOADER_H

#include <glibmm.h>
#include <queue>
#include <set>
#include <map>

class DcmDataset;

namespace ImagePool {

class Instance;
class Study;

class Loader {
public:

	Loader();
	
	virtual ~Loader();
	
	bool start();

	void stop();

	bool busy();

	sigc::signal< void, Glib::RefPtr<ImagePool::Study> > signal_study_added;

	Glib::Dispatcher signal_error;

protected:

	class CacheEntry {
	public:

		CacheEntry() : m_instancecount(0), m_seriescount(0) {
		};

		Glib::RefPtr<ImagePool::Study> m_study;

		guint m_instancecount;

		std::set< std::string > m_seriesuid;
		
		guint m_seriescount;

	};

	virtual bool run();
	
	virtual void finished();

	virtual bool on_timeout();

	void add_image(DcmDataset* dset);

	void process_instance();
	
	Glib::Thread* m_loader;

	//Glib::Dispatcher m_add_image;

	//Glib::Dispatcher m_finished;
	
	Glib::Mutex m_mutex;

	bool m_busy;

	std::map<std::string, CacheEntry> m_cache;

	sigc::connection m_conn_timer;

	bool m_finished;

private:

	void thread();
	
	std::queue< Glib::RefPtr<ImagePool::Instance> > m_imagequeue;
	
};

} // namespace ImagePool

#endif // IMAGEPOOL_LOADER_H
