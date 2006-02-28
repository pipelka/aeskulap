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
    Update Date:      $Date: 2006/02/28 22:39:34 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/imagepool/Attic/dicombase.h,v $
    CVS/RCS Revision: $Revision: 1.1.2.1 $
    Status:           $State: Exp $
*/

#ifndef IMAGEPOOL_DICOMBASE_H
#define IMAGEPOOL_DICOMBASE_H

#include <map>

#include <glibmm/refptr.h>
#include <glibmm/object.h>

class DcmTagKey;
class DcmDataset;

namespace ImagePool {

class DicomBase : public Glib::Object {
public:

	DicomBase();
	
	virtual ~DicomBase();

	void set_tag(const std::string& tag, const std::string& value);

	bool copy_tag(DcmDataset* dset, const DcmTagKey& tag, const std::string& tagname, bool convert = false);

	bool copy_tag(const Glib::RefPtr<ImagePool::DicomBase>& base, const std::string& tagname);

	const std::string& tag(const std::string& tag);
	
protected:

	std::string fix_date(std::string& date);

	std::string fix_time(std::string& time);

	bool set_encoding(const std::string& single, const std::string& ideographic="");

	std::string convert_string(const char* dicom_string);

private:

	std::string m_encoding[3];

	std::map<std::string, std::string> property_string;
};

} // namespace ImagePool

#endif // IMAGEPOOL_DICOMBASE_H
