/*
**
** Author: Andrew Hewett, medigration GmbH
**
** Module: extneg.h
**
** Purpose:
**   Extended Negotiation for A-ASSOCIATE
**
** Last Update:         $Author$
** Update Date:         $Date$
** Source File:         $Source$
** CVS/RCS Revision:    $Revision$
** Status:              $State$
**
** CVS/RCS Log at end of file
**
*/

#ifndef EXTNEG_H
#define EXTNEG_H

#include "dcmtk/config/osconfig.h" /* make sure OS specific configuration is included first */

#include "dcmtk/ofstd/ofstring.h"
#include "dcmtk/ofstd/oflist.h"

#define DUL_TYPESOPCLASSEXTENDEDNEGOTIATION (unsigned char)0x56


struct SOPClassExtendedNegotiationSubItem {
    unsigned char itemType;
    unsigned char reserved1;
    unsigned short itemLength;
    unsigned short sopClassUIDLength;
    OFString sopClassUID;
    unsigned char* serviceClassAppInfo;
    unsigned short serviceClassAppInfoLength;

    SOPClassExtendedNegotiationSubItem()
    : itemType(0x56)
    , reserved1(0)
    , itemLength(0)
    , sopClassUIDLength(0)
    , sopClassUID()
    , serviceClassAppInfo(NULL)
    , serviceClassAppInfoLength(0)
    {
    }

private:
    // private undefined copy constructor and operator=
    SOPClassExtendedNegotiationSubItem(const SOPClassExtendedNegotiationSubItem& arg);
    SOPClassExtendedNegotiationSubItem& operator= (const SOPClassExtendedNegotiationSubItem& arg);
};

typedef OFList<SOPClassExtendedNegotiationSubItem*> SOPClassExtendedNegotiationSubItemList;

void appendList(const SOPClassExtendedNegotiationSubItemList& from, SOPClassExtendedNegotiationSubItemList& to);

void deleteListMembers(SOPClassExtendedNegotiationSubItemList& lst);

#endif

/*
** CVS/RCS Log:
** $Log$
** Revision 1.1  2007/04/24 09:53:40  braindead
** - updated DCMTK to version 3.5.4
** - merged Gianluca's WIN32 changes
**
** Revision 1.1.1.1  2006/07/19 09:16:46  pipelka
** - imported dcmtk354 sources
**
**
** Revision 1.3  2005/12/08 16:02:24  meichel
** Changed include path schema for all DCMTK header files
**
** Revision 1.2  2003/06/02 16:44:11  meichel
** Renamed local variables to avoid name clashes with STL
**
** Revision 1.1  1999/04/19 08:39:55  meichel
** Added experimental support for extended SOP class negotiation.
**
**
*/
