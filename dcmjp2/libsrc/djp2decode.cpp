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
 *  Author:  Marco Eichelberg
 *
 *  Purpose: singleton class that registers decoders for all supported JPEG processes.
 *
 *  Last Update:      $Author: braindead $
 *  Update Date:      $Date: 2006/02/28 22:39:34 $
 *  Source File:      $Source: /cvsroot/aeskulap/aeskulap/dcmjp2/libsrc/Attic/djp2decode.cpp,v $
 *  CVS/RCS Revision: $Revision: 1.1.2.1 $
 *  Status:           $State: Exp $
 *
 *  CVS/RCS Log at end of file
 *
 */

#include "osconfig.h"
#include "djdecode.h"

#include "dccodec.h"  /* for DcmCodecStruct */
#include "djp2decode.h"
#include "djp2declol.h"

// initialization of static members
OFBool DJP2DecoderRegistration::registered                  = OFFalse;
DJP2DecoderLossless *DJP2DecoderRegistration::declol        = NULL;

void DJP2DecoderRegistration::registerCodecs(
    E_DecompressionColorSpaceConversion pDecompressionCSConversion,
    E_UIDCreation pCreateSOPInstanceUID,
    E_PlanarConfiguration pPlanarConfiguration,
    OFBool pVerbose)
{
  if (! registered)
  {
      // JPEG 2000 lossless only
      declol = new DJP2DecoderLossless();
      if (declol) DcmCodecList::registerCodec(declol, NULL, NULL);

      registered = OFTrue;
  }
}

void DJP2DecoderRegistration::cleanup()
{
  if (registered)
  {
    DcmCodecList::deregisterCodec(declol);
    delete declol;
    registered = OFFalse;
#ifdef DEBUG
    // not needed but useful for debugging purposes
    declol = NULL;
#endif

  }
}
