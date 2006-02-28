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
 *  Author:  Marco Eichelberg, Norbert Olges
 *
 *  Purpose: Abstract base class for JPEG 2000 decoder
 *
 *  Last Update:      $Author: braindead $
 *  Update Date:      $Date: 2006/02/28 22:39:34 $
 *  Source File:      $Source: /cvsroot/aeskulap/aeskulap/dcmjp2/libsrc/Attic/djp2codecd.cpp,v $
 *  CVS/RCS Revision: $Revision: 1.1.2.1 $
 *  Status:           $State: Exp $
 *
 *  CVS/RCS Log at end of file
 *
 */

#include "osconfig.h"
#include "djp2codecd.h"

// dcmdata includes
#include "dcdatset.h"  /* for class DcmDataset */
#include "dcdeftag.h"  /* for tag constants */
#include "dcpixseq.h"  /* for class DcmPixelSequence */
#include "dcpxitem.h"  /* for class DcmPixelItem */
#include "dcvrpobw.h"  /* for class DcmPolymorphOBOW */
#include "dcswap.h"    /* for swapIfNecessary() */
#include "dcuid.h"     /* for dcmGenerateUniqueIdentifer()*/

// dcmjpeg includes

DJP2CodecDecoder::DJP2CodecDecoder()
: DcmCodec()
{
}


DJP2CodecDecoder::~DJP2CodecDecoder()
{
}


OFBool DJP2CodecDecoder::canChangeCoding(
    const E_TransferSyntax oldRepType,
    const E_TransferSyntax newRepType) const
{
  E_TransferSyntax myXfer = supportedTransferSyntax();
  DcmXfer newRep(newRepType);
  if (newRep.isNotEncapsulated() && (oldRepType == myXfer)) return OFTrue; // decompress requested

  // we don't support re-coding for now.
  return OFFalse;
}

OFCondition DJP2CodecDecoder::decode(
    const DcmRepresentationParameter * fromRepParam,
    DcmPixelSequence * pixSeq,
    DcmPolymorphOBOW& uncompressedPixelData,
    const DcmCodecParameter * cp,
    const DcmStack& objStack) const
{
  OFCondition result = EC_Normal;
  return result;
}

OFCondition DJP2CodecDecoder::encode(
        const Uint16 * /* pixelData */,
        const Uint32 /* length */,
        const DcmRepresentationParameter * /* toRepParam */,
        DcmPixelSequence * & /* pixSeq */,
        const DcmCodecParameter * /* cp */,
        DcmStack & /* objStack */) const
{
  // we are a decoder only
  return EC_IllegalCall;
}


OFCondition DJP2CodecDecoder::encode(
    const E_TransferSyntax /* fromRepType */,
    const DcmRepresentationParameter * /* fromRepParam */,
    DcmPixelSequence * /* fromPixSeq */,
    const DcmRepresentationParameter * /* toRepParam */,
    DcmPixelSequence * & /* toPixSeq */,
    const DcmCodecParameter * /* cp */,
    DcmStack & /* objStack */) const
{
  // we don't support re-coding for now.
  return EC_IllegalCall;
}

OFCondition DJP2CodecDecoder::createPlanarConfigurationByte(
  Uint8 *imageFrame,
  Uint16 columns,
  Uint16 rows)
{
  if (imageFrame == NULL) return EC_IllegalCall;

  unsigned long numPixels = columns * rows;
  if (numPixels == 0) return EC_IllegalCall;

  Uint8 *buf = new Uint8[3*numPixels + 3];
  if (buf)
  {
    memcpy(buf, imageFrame, (size_t)(3*numPixels));
    register Uint8 *s = buf;                        // source
    register Uint8 *r = imageFrame;                 // red plane
    register Uint8 *g = imageFrame + numPixels;     // green plane
    register Uint8 *b = imageFrame + (2*numPixels); // blue plane
    for (register unsigned long i=numPixels; i; i--)
    {
      *r++ = *s++;
      *g++ = *s++;
      *b++ = *s++;
    }
    delete[] buf;
  } else return EC_MemoryExhausted;
  return EC_Normal;
}

OFCondition DJP2CodecDecoder::createPlanarConfigurationWord(
  Uint16 *imageFrame,
  Uint16 columns,
  Uint16 rows)
{
  if (imageFrame == NULL) return EC_IllegalCall;

  unsigned long numPixels = columns * rows;
  if (numPixels == 0) return EC_IllegalCall;

  Uint16 *buf = new Uint16[3*numPixels + 3];
  if (buf)
  {
    memcpy(buf, imageFrame, (size_t)(3*numPixels*sizeof(Uint16)));
    register Uint16 *s = buf;                        // source
    register Uint16 *r = imageFrame;                 // red plane
    register Uint16 *g = imageFrame + numPixels;     // green plane
    register Uint16 *b = imageFrame + (2*numPixels); // blue plane
    for (register unsigned long i=numPixels; i; i--)
    {
      *r++ = *s++;
      *g++ = *s++;
      *b++ = *s++;
    }
    delete[] buf;
  } else return EC_MemoryExhausted;
  return EC_Normal;
}

/* This method examines if a given image requires color-by-plane planar configuration
 * depending on SOP Class UID (DICOM IOD) and photometric interpretation.
 * All SOP classes defined in the 2001 edition of the DICOM standard or earlier
 * are handled correctly.
 */

OFBool DJP2CodecDecoder::requiresPlanarConfiguration(
  const char *sopClassUID,
  EP_Interpretation photometricInterpretation)
{
  if (sopClassUID)
  {
    OFString sopClass(sopClassUID);
  
    // Hardcopy Color Image always requires color-by-plane
    if (sopClass == UID_HardcopyColorImageStorage) return OFTrue;

    // The 1996 Ultrasound Image IODs require color-by-plane if color model is YBR_FULL.
    if (photometricInterpretation == EPI_YBR_Full)
    {
      if ((sopClass == UID_UltrasoundMultiframeImageStorage) 
        ||(sopClass == UID_UltrasoundImageStorage)) return OFTrue;
    }

  }
  return OFFalse;
}
