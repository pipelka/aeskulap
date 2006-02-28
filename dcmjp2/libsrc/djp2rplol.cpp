/*
 *
 *  Copyright (C) 1997-2001, OFFIS
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
 *  Module:  dcmjpeg
 *
 *  Author:  Norbert Olges, Marco Eichelberg
 *
 *  Purpose: representation parameter for lossless JPEG
 *
 *  Last Update:      $Author: braindead $
 *  Update Date:      $Date: 2006/02/28 22:39:34 $
 *  Source File:      $Source: /cvsroot/aeskulap/aeskulap/dcmjp2/libsrc/Attic/djp2rplol.cpp,v $
 *  CVS/RCS Revision: $Revision: 1.1.2.1 $
 *  Status:           $State: Exp $
 *
 *  CVS/RCS Log at end of file
 *
 */

#include "osconfig.h"
#include "djp2rplol.h"


DJP2_RPLossless::DJP2_RPLossless()
: DcmRepresentationParameter()
{
}

DJP2_RPLossless::DJP2_RPLossless(const DJP2_RPLossless& arg)
: DcmRepresentationParameter(arg)
{
}

DJP2_RPLossless::~DJP2_RPLossless()
{
}  

DcmRepresentationParameter *DJP2_RPLossless::clone() const
{
  return new DJP2_RPLossless(*this);
}

const char *DJP2_RPLossless::className() const
{
  return "DJP2_RPLossless";
}

OFBool DJP2_RPLossless::operator==(const DcmRepresentationParameter &arg) const
{
  const char *argname = arg.className();
  if (argname)
  {
    OFString argstring(argname);
    if (argstring == className())
    {
      const DJP2_RPLossless& argll = (const DJP2_RPLossless &)arg;
      return OFTrue;
    }   
  }
  return OFFalse;
}
