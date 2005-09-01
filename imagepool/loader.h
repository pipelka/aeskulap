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
    Update Date:      $Date: 2005/09/01 06:49:44 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/imagepool/loader.h,v $
    CVS/RCS Revision: $Revision: 1.3 $
    Status:           $State: Exp $
*/

#ifndef IMAGEPOOL_LOADER_H
#define IMAGEPOOL_LOADER_H

#include <glibmm.h>
#include <queue>
#include <map>

class DcmDataset;

namespace ImagePool {

class Instance;
class Study;

class Loader {
public:

	Loader();
	
	virtual ~Loader();
	
	void start();

	void stop();

	//sigc::signal<void, std::string> signal_finished;

	sigc::signal< void, Glib::RefPtr<ImagePool::Study> > signal_study_added;

protected:

	class Data {
	public:
		std::map < Glib::RefPtr<ImagePool::Study>, bool > loaded_study;
	};

	virtual void run();
	
	virtual void finished();

	Data& data();

	void add_image(DcmDataset* dset, int imagecount=0);

	void add_image_callback();
	
	Glib::Thread* m_loader;

	Glib::Dispatcher m_add_image;

	Glib::Dispatcher m_finished;

	Glib::Mutex m_mutex;

private:

	void thread();
	
	Glib::RefPtr<ImagePool::Study> m_current_study;

	std::queue< Glib::RefPtr<ImagePool::Instance> > m_imagequeue;
	
	std::map< Glib::Thread*, Data > m_data;

};

} // namespace ImagePool

#endif // IMAGEPOOL_LOADER_H
