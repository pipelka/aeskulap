/*
 *
 *  Copyright (C) 1994-2005, OFFIS
 *
 *  This software and supporting documentation were developed by
 *
 *    Kuratorium OFFIS e.V.
 *    Healthcare Information and Communication Systems
 *    Escherweg 2
 *    D-26121 Oldenburg, Germany
 *
 *  THIS SOFTWARE IS MADE AVAILABLE,  AS IS,  AND OFFIS MAKES NO  WARRANTY
 *  REGARDING  THE  SOFTWARE,  ITS  PERFORMANCE,  ITS  MERCHANTABILITY  OR
 *  FITNESS FOR ANY PARTICULAR USE, FREEDOM FROM ANY COMPUTER DISEASES  OR
 *  ITS CONFORMITY TO ANY SPECIFICATION. THE ENTIRE RISK AS TO QUALITY AND
 *  PERFORMANCE OF THE SOFTWARE IS WITH THE USER.
 *
 *  Module:  dcmdata
 *
 *  Author:  Andreas Barth
 *
 *  Purpose: functions to derive VM from string
 *
 *  Last Update:      $Author$
 *  Update Date:      $Date$
 *  Source File:      $Source$
 *  CVS/RCS Revision: $Revision$
 *  Status:           $State$
 *
 *  CVS/RCS Log at end of file
 *
 */

#ifndef DCVM_H
#define DCVM_H

#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */

// get the number of values stored in string
unsigned long getVMFromString(const char * val);

// get first value stored in string, set the parameter to beginning of the
// next value
char * getFirstValueFromString(const char * & s);


#endif 

/*
** CVS/RCS Log:
** $Log$
** Revision 1.1  2007/04/24 09:53:31  braindead
** - updated DCMTK to version 3.5.4
** - merged Gianluca's WIN32 changes
**
** Revision 1.1.1.1  2006/07/19 09:16:41  pipelka
** - imported dcmtk354 sources
**
**
** Revision 1.6  2005/12/08 16:28:49  meichel
** Changed include path schema for all DCMTK header files
**
** Revision 1.5  2001/06/01 15:48:47  meichel
** Updated copyright header
**
** Revision 1.4  2000/03/08 16:26:20  meichel
** Updated copyright header.
**
** Revision 1.3  1999/03/31 09:24:53  meichel
** Updated copyright header in module dcmdata
**
** Revision 1.2  1996/03/26 09:59:20  meichel
** corrected bug (deletion of const char *) which prevented compilation on NeXT
**
** Revision 1.1  1996/01/29 13:38:15  andreas
** - new put method for every VR to put value as a string
** - better and unique print methods
**
*/

