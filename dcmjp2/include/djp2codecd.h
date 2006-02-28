/*
 *
 *  Copyright (C) 1997-2003, OFFIS
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
 *  Purpose: abstract codec class for JPEG decoders.
 *
 *  Last Update:      $Author: braindead $
 *  Update Date:      $Date: 2006/02/28 22:39:34 $
 *  Source File:      $Source: /cvsroot/aeskulap/aeskulap/dcmjp2/include/Attic/djp2codecd.h,v $
 *  CVS/RCS Revision: $Revision: 1.1.2.1 $
 *  Status:           $State: Exp $
 *
 *  CVS/RCS Log at end of file
 *
 */

#ifndef DJP2CODECD_H
#define DJP2CODECD_H

#include "osconfig.h"
#include "oftypes.h"
#include "dccodec.h"    /* for class DcmCodec */
#include "diutils.h"    /* for enums */
#include "ofstring.h"   /* for class OFString */


class DcmDataset;
class DcmItem;

/** abstract codec class for JP2 decoders.
 *  This class only supports decompression, it neither implements
 *  encoding nor transcoding.
 */
class DJP2CodecDecoder : public DcmCodec
{
public:  
 
  /// default constructor
  DJP2CodecDecoder();

  /// destructor
  virtual ~DJP2CodecDecoder();

  /** decompresses the given pixel sequence and
   *  stores the result in the given uncompressedPixelData element.
   *  @param fromRepParam current representation parameter of compressed data, may be NULL
   *  @param pixSeq compressed pixel sequence
   *  @param uncompressedPixelData uncompressed pixel data stored in this element
   *  @param cp codec parameters for this codec
   *  @param objStack stack pointing to the location of the pixel data
   *    element in the current dataset.
   *  @return EC_Normal if successful, an error code otherwise.
   */
  virtual OFCondition decode(
    const DcmRepresentationParameter * fromRepParam,
    DcmPixelSequence * pixSeq,
    DcmPolymorphOBOW& uncompressedPixelData,
    const DcmCodecParameter * cp,
    const DcmStack& objStack) const;

  /** compresses the given uncompressed DICOM image and stores
   *  the result in the given pixSeq element.
   *  @param pixelData pointer to the uncompressed image data in OW format
   *    and local byte order
   *  @param length of the pixel data field in bytes
   *  @param toRepParam representation parameter describing the desired
   *    compressed representation (e.g. JPEG quality)
   *  @param pixSeq compressed pixel sequence (pointer to new DcmPixelSequence object
   *    allocated on heap) returned in this parameter upon success.   
   *  @param cp codec parameters for this codec
   *  @param objStack stack pointing to the location of the pixel data
   *    element in the current dataset.
   *  @return EC_Normal if successful, an error code otherwise.
   */
  virtual OFCondition encode(
    const Uint16 * pixelData,
    const Uint32 length,
    const DcmRepresentationParameter * toRepParam,
    DcmPixelSequence * & pixSeq,
    const DcmCodecParameter *cp,
    DcmStack & objStack) const;

  /** transcodes (re-compresses) the given compressed DICOM image and stores
   *  the result in the given toPixSeq element.
   *  @param fromRepType current transfer syntax of the compressed image
   *  @param fromRepParam current representation parameter of compressed data, may be NULL
   *  @param fromPixSeq compressed pixel sequence
   *  @param toRepParam representation parameter describing the desired
   *    new compressed representation (e.g. JPEG quality)
   *  @param toPixSeq compressed pixel sequence (pointer to new DcmPixelSequence object
   *    allocated on heap) returned in this parameter upon success.   
   *  @param cp codec parameters for this codec
   *  @param objStack stack pointing to the location of the pixel data
   *    element in the current dataset.
   *  @return EC_Normal if successful, an error code otherwise.
   */
  virtual OFCondition encode(
    const E_TransferSyntax fromRepType,
    const DcmRepresentationParameter * fromRepParam,
    DcmPixelSequence * fromPixSeq,
    const DcmRepresentationParameter * toRepParam,
    DcmPixelSequence * & toPixSeq,
    const DcmCodecParameter * cp,
    DcmStack & objStack) const;

  /** checks if this codec is able to convert from the
   *  given current transfer syntax to the given new
   *  transfer syntax
   *  @param oldRepType current transfer syntax
   *  @param newRepType desired new transfer syntax
   *  @return true if transformation is supported by this codec, false otherwise.
   */
  virtual OFBool canChangeCoding(
    const E_TransferSyntax oldRepType,
    const E_TransferSyntax newRepType) const;

  /** returns the transfer syntax that this particular codec
   *  is able to decode.
   *  @return supported transfer syntax
   */
  virtual E_TransferSyntax supportedTransferSyntax() const = 0;

protected: 

  // static private helper methods

  /** converts an RGB or YBR frame with 8 bits/sample from
   *  color-by-pixel to color-by-plane planar configuration.
   *  @param imageFrame pointer to image frame, must contain
   *    at least 3*columns*rows bytes of pixel data.
   *  @param columns columns
   *  @param rows rows
   *  @return EC_Normal if successful, an error code otherwise
   */
  static OFCondition createPlanarConfigurationByte(
    Uint8 *imageFrame,
    Uint16 columns,
    Uint16 rows);

  /** converts an RGB or YBR frame with 16 bits/sample from
   *  color-by-pixel to color-by-plane planar configuration.
   *  @param imageFrame pointer to image frame, must contain
   *    at least 3*columns*rows words of pixel data.
   *  @param columns columns
   *  @param rows rows
   *  @return EC_Normal if successful, an error code otherwise
   */
  static OFCondition createPlanarConfigurationWord(
    Uint16 *imageFrame,
    Uint16 columns,
    Uint16 rows);

  /** examines if a given image requires color-by-plane planar configuration
   *  depending on SOP Class UID (DICOM IOD) and photometric interpretation.
   *  All SOP classes defined in the 2001 edition of the DICOM standard or earlier
   *  are handled correctly.
   *  @param sopClassUID SOP Class UID
   *  @param photometricInterpretation decompressed photometric interpretation
   *  @return true if color-by-plane is required, false otherwise.
   */
  static OFBool requiresPlanarConfiguration(
    const char *sopClassUID,
    EP_Interpretation photometricInterpretation);

};

#endif
