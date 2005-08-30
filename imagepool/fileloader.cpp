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
    Update Date:      $Date: 2005/08/30 19:47:55 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/imagepool/fileloader.cpp,v $
    CVS/RCS Revision: $Revision: 1.2 $
    Status:           $State: Exp $
*/

#include <gtkmm.h>

#include "dcfilefo.h"

#include "imagepool.h"
#include "fileloader.h"

namespace ImagePool {

void FileLoader::load(const Glib::SListHandle< Glib::ustring >& filelist) {
	m_filelist = new Glib::SListHandle< Glib::ustring >(filelist);
	start();
}

void FileLoader::run() {
	Glib::SListHandle< Glib::ustring >* filelist = m_filelist;
	m_mutex.unlock();

	Glib::SListHandle< Glib::ustring >::iterator i = filelist->begin();

	for(; i != filelist->end(); i++) {
		DcmFileFormat dfile;
	
		OFCondition cond = dfile.loadFile(
							(*i).c_str(),
							EXS_Unknown,
							EGL_noChange,
							DCM_MaxReadLength,
							false);
		
		if(!cond.good()) {
			std::cout << "unable to open file !!!" << std::endl;
		}
		else {
			dfile.loadAllDataIntoMemory();
			std::cout << "opened file:" << (*i) << std::endl;
		
			DcmDataset* dset = dfile.getDataset();
			add_image(dset);
		}
	}
	
	delete filelist;
}

} // namespace ImagePool
