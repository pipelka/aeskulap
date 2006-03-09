/*
    Aeskulap Configuration - persistent configuration interface library
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
    Update Date:      $Date: 2006/03/09 15:35:14 $
    Source File:      $Source: /cvsroot/aeskulap/aeskulap/configuration/aconfigclient.h,v $
    CVS/RCS Revision: $Revision: 1.1.4.1 $
    Status:           $State: Exp $
*/


#ifndef AESKULAP_CONFIGCLIENT_H
#define AESKULAP_CONFIGCLIENT_H

#include "aconfiguration.h"

namespace Aeskulap {

class ConfigClient {
protected:
	ConfigClient();
	
	Configuration& m_configuration;
};

} // namespace Aeskulap

#endif // AESKULAP_CONFIGCLIENT_H
