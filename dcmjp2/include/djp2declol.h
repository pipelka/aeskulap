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
 *  Purpose: Codec class for decoding JPEG Lossless (8/12/16-bit)
 *
 *  Last Update:      $Author: braindead $
 *  Update Date:      $Date: 2006/02/28 22:39:34 $
 *  Source File:      $Source: /cvsroot/aeskulap/aeskulap/dcmjp2/include/Attic/djp2declol.h,v $
 *  CVS/RCS Revision: $Revision: 1.1.2.1 $
 *  Status:           $State: Exp $
 *
 *  CVS/RCS Log at end of file
 *
 */

#ifndef DJP2DECLOL_H
#define DJP2DECLOL_H

#include "osconfig.h"
#include "djp2codecd.h" /* for class DJCodecDecoder */


/** Decoder class for JPEG200 Lossless Only
 */
class DJP2DecoderLossless : public DJP2CodecDecoder
{
public: 

  /// default constructor
  DJP2DecoderLossless();

  /// destructor
  virtual ~DJP2DecoderLossless();

  /** returns the transfer syntax that this particular codec
   *  is able to encode and decode.
   *  @return supported transfer syntax
   */
  virtual E_TransferSyntax supportedTransferSyntax() const;

  OFCondition decode(
    const DcmRepresentationParameter * fromRepParam,
    DcmPixelSequence * pixSeq,
    DcmPolymorphOBOW& uncompressedPixelData,
    const DcmCodecParameter * cp,
    const DcmStack& objStack) const;

protected:

	OFBool requiresPlanarConfiguration(const char *sopClassUID, EP_Interpretation photometricInterpretation) const;

	OFCondition jpeg2000_decode(Uint8 *compressedFrameBuffer, Uint32 compressedFrameBufferSize, Uint8 *uncompressedFrameBuffer, Uint32 uncompressedFrameBufferSize) const;

private:

	OFCondition jpeg2000_decoder_init() const;

	Uint8 jpeg200_scanForBitDepth(Uint8* jpegData, Uint32 fragmentLength) const;

	EP_Interpretation jpeg2000_getDecompressedColorModel() const;

};

#endif
