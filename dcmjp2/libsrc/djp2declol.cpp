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
 *  Source File:      $Source: /cvsroot/aeskulap/aeskulap/dcmjp2/libsrc/Attic/djp2declol.cpp,v $
 *  CVS/RCS Revision: $Revision: 1.1.2.1 $
 *  Status:           $State: Exp $
 *
 *  CVS/RCS Log at end of file
 *
 */

#include "osconfig.h"
#include "djp2declol.h"
#include "djp2rplol.h"
#include "dcitem.h"
#include "dcdeftag.h"
#include "djutils.h"
#include "dcpxitem.h"
#include "dcpixseq.h"
#include "dcuid.h"
#include "dcswap.h"

DJP2DecoderLossless::DJP2DecoderLossless()
: DJP2CodecDecoder()
{
}


DJP2DecoderLossless::~DJP2DecoderLossless()
{
}


E_TransferSyntax DJP2DecoderLossless::supportedTransferSyntax() const
{
  return EXS_JPEG2000LosslessOnly;
}

OFCondition DJP2DecoderLossless::decode(
	const DcmRepresentationParameter * fromRepParam,
	DcmPixelSequence * pixSeq,
	DcmPolymorphOBOW& uncompressedPixelData,
	const DcmCodecParameter * cp,
	const DcmStack& objStack) const
{
  OFCondition result = EC_Normal;

  DcmStack localStack(objStack);
  (void)localStack.pop();             // pop pixel data element from stack
  DcmObject *dataset = localStack.pop(); // this is the item in which the pixel data is located
  if ((!dataset)||((dataset->ident()!= EVR_dataset) && (dataset->ident()!= EVR_item))) result = EC_InvalidTag;
  else
  {
    Uint16 imageSamplesPerPixel = 0;
    Uint16 imageRows = 0;
    Uint16 imageColumns = 0;
    Sint32 imageFrames = 1;
    Uint16 imageBitsStored = 0;
    Uint16 imageHighBit = 0;
    Uint8 bytesPerSample = 0;
    const char *sopClassUID = NULL;
    OFBool createPlanarConfiguration = OFFalse;
    OFBool createPlanarConfigurationInitialized = OFFalse;
    EP_Interpretation colorModel = EPI_Unknown;

    if (result.good()) result = ((DcmItem *)dataset)->findAndGetUint16(DCM_SamplesPerPixel, imageSamplesPerPixel);                                 
    if (result.good()) result = ((DcmItem *)dataset)->findAndGetUint16(DCM_Rows, imageRows);
    if (result.good()) result = ((DcmItem *)dataset)->findAndGetUint16(DCM_Columns, imageColumns);
    if (result.good()) result = ((DcmItem *)dataset)->findAndGetUint16(DCM_BitsStored, imageBitsStored);
    if (result.good()) result = ((DcmItem *)dataset)->findAndGetUint16(DCM_HighBit, imageHighBit);
    // number of frames is an optional attribute - we don't mind if it isn't present.
    if (result.good()) (void) ((DcmItem *)dataset)->findAndGetSint32(DCM_NumberOfFrames, imageFrames);                                 

    // we consider SOP Class UID as optional since we only need it to determine SOP Class specific 
    // encoding rules for planar configuration.
    if (result.good()) (void) ((DcmItem *)dataset)->findAndGetString(DCM_SOPClassUID, sopClassUID);                                 

    EP_Interpretation dicomPI = DcmJpegHelper::getPhotometricInterpretation((DcmItem *)dataset);
    OFBool isYBR = OFFalse;
    if ((dicomPI == EPI_YBR_Full)||(dicomPI == EPI_YBR_Full_422)||(dicomPI == EPI_YBR_Partial_422)) isYBR = OFTrue;
    
    if (imageFrames < 1) imageFrames = 1; // default in case this attribute contains garbage
    if (result.good())
    {        
      DcmPixelItem *pixItem = NULL;
      Uint8 * jpegData = NULL;        
      result = pixSeq->getItem(pixItem, 1); // first item is offset table, use second item
      if (result.good())
      {                  
        Uint32 fragmentLength = pixItem->getLength();
        result = pixItem->getUint8Array(jpegData);
        if (result.good())
        {                            
          Uint8 precision = jpeg200_scanForBitDepth(jpegData, fragmentLength);
          if (precision == 0) result = EC_CannotChangeRepresentation; // something has gone wrong, bail out
          else
          {
              Uint32 frameSize = ((precision > 8) ? sizeof(Uint16) : sizeof(Uint8)) * imageRows * imageColumns * imageSamplesPerPixel;
              if(precision >= 8) {
              	bytesPerSample = 2;
              }
              else {
              	bytesPerSample = 1;
              }
              Uint32 totalSize = frameSize * imageFrames;
              if (totalSize & 1) totalSize++; // align on 16-bit word boundary
              Uint16 *imageData16 = NULL;
              Sint32 currentFrame = 0;
              Uint32 currentItem = 1; // ignore offset table
              
              result = uncompressedPixelData.createUint16Array(totalSize/sizeof(Uint16), imageData16);
              if (result.good())
              {
                Uint8 *imageData8 = (Uint8 *)imageData16;
                
                while ((currentFrame < imageFrames)&&(result.good()))
                {
                  result = jpeg2000_decoder_init();
                  if (result.good())
                  {                    
                    result = EJ_Suspension;
                    while (EJ_Suspension == result)
                    {
                      result = pixSeq->getItem(pixItem, currentItem++);
                      if (result.good())
                      {
                        fragmentLength = pixItem->getLength();
                        result = pixItem->getUint8Array(jpegData);
                        if (result.good())
                        {
                          result = jpeg2000_decode(jpegData, fragmentLength, imageData8, frameSize);
                        }
                      }
                    }                      
                    if (result.good())
                    {
                      if (! createPlanarConfigurationInitialized)
                      {
                        // we need to know the decompressed photometric interpretation in order
                        // to determine the final planar configuration.  However, this is only 
                        // known after the first call to jpeg->decode(), i.e. here.
                        colorModel = jpeg2000_getDecompressedColorModel();
                        if (colorModel == EPI_Unknown)
                        {
                          // derive color model from DICOM photometric interpretation
                          if ((dicomPI == EPI_YBR_Full_422)||(dicomPI == EPI_YBR_Partial_422)) colorModel = EPI_YBR_Full;
                          else colorModel = dicomPI;
                        }

                        /*switch (djcp->getPlanarConfiguration())
                        {
                          case EPC_default:*/
                            createPlanarConfiguration = requiresPlanarConfiguration(sopClassUID, colorModel);
                            /*break;
                          case EPC_colorByPixel:
                            createPlanarConfiguration = OFFalse;
                            break;
                          case EPC_colorByPlane:
                            createPlanarConfiguration = OFTrue;
                            break;
                        }*/
                        createPlanarConfigurationInitialized = OFTrue;
                      }

                      // convert planar configuration if necessary
                      if ((imageSamplesPerPixel == 3) && createPlanarConfiguration)
                      {                  
                        if (precision > 8)
                          result = createPlanarConfigurationWord((Uint16 *)imageData8, imageColumns, imageRows);
                          else result = createPlanarConfigurationByte(imageData8, imageColumns, imageRows);
                      }
                      currentFrame++;
                      imageData8 += frameSize;    
                    }
                  }                  
                }

                if (result.good())
                {
                  // decompression is complete, finally adjust byte order if necessary
                  if (bytesPerSample == 1) // we're writing bytes into words
                  {
                    result = swapIfNecessary(gLocalByteOrder, EBO_LittleEndian, imageData16, 
                      totalSize, sizeof(Uint16));
                  }
                }

                // adjust photometric interpretation depending on what conversion has taken place
                if (result.good())
                {                  
                  switch (colorModel)
                  {
                    case EPI_Monochrome2:
                      result = ((DcmItem *)dataset)->putAndInsertString(DCM_PhotometricInterpretation, "MONOCHROME2");
                      if (result.good())
                      {
                        imageSamplesPerPixel = 1;
                      	result = ((DcmItem *)dataset)->putAndInsertUint16(DCM_SamplesPerPixel, imageSamplesPerPixel);
                      }
                      break;
                    case EPI_YBR_Full:
                      result = ((DcmItem *)dataset)->putAndInsertString(DCM_PhotometricInterpretation, "YBR_FULL");
                      if (result.good())
                      {
                        imageSamplesPerPixel = 3;
                      	result = ((DcmItem *)dataset)->putAndInsertUint16(DCM_SamplesPerPixel, imageSamplesPerPixel);
                      }
                      break;
                    case EPI_RGB:
                      result = ((DcmItem *)dataset)->putAndInsertString(DCM_PhotometricInterpretation, "RGB");
                      if (result.good())
                      {
                        imageSamplesPerPixel = 3;
                      	result = ((DcmItem *)dataset)->putAndInsertUint16(DCM_SamplesPerPixel, imageSamplesPerPixel);
                      }
                      break;
                    default:
                      /* leave photometric interpretation untouched unless it is YBR_FULL_422 
                       * or YBR_PARTIAL_422. In this case, replace by YBR_FULL since decompression 
                       * eliminates the subsampling.
                       */
                      if ((dicomPI == EPI_YBR_Full_422)||(dicomPI == EPI_YBR_Partial_422))
                      {
                        result = ((DcmItem *)dataset)->putAndInsertString(DCM_PhotometricInterpretation, "YBR_FULL");
                      }                      
                      break;
                  }
                }

                // Bits Allocated is now either 8 or 16               
                if (result.good())
                {
                  if (precision > 8) result = ((DcmItem *)dataset)->putAndInsertUint16(DCM_BitsAllocated, 16);
                  else result = ((DcmItem *)dataset)->putAndInsertUint16(DCM_BitsAllocated, 8);
                }

                // Planar Configuration depends on the createPlanarConfiguration flag
                if ((result.good()) && (imageSamplesPerPixel > 1))
                {
                  result = ((DcmItem *)dataset)->putAndInsertUint16(DCM_PlanarConfiguration, (createPlanarConfiguration ? 1 : 0));
                }

                // Bits Stored cannot be larger than precision
                if ((result.good()) && (imageBitsStored > precision))
                {
                  result = ((DcmItem *)dataset)->putAndInsertUint16(DCM_BitsStored, precision);
                }

                // High Bit cannot be larger than precision - 1
                if ((result.good()) && ((unsigned long)(imageHighBit+1) > (unsigned long)precision))
                {
                  result = ((DcmItem *)dataset)->putAndInsertUint16(DCM_HighBit, precision-1);
                }

                // Pixel Representation could be signed if lossless JPEG. For now, we just believe what we get.
              }
          }
        }
      }
    }

    // the following operations do not affect the Image Pixel Module
    // but other modules such as SOP Common.  We only perform these
    // changes if we're on the main level of the dataset,
    // which should always identify itself as dataset, not as item.
    if (dataset->ident() == EVR_dataset)
    {
        // create new SOP instance UID if codec parameters require so
        //if (result.good() && (djcp->getUIDCreation() == EUC_always)) result = DcmCodec::newInstance((DcmItem *)dataset);
    }

  }
  return result;
}

OFBool DJP2DecoderLossless::requiresPlanarConfiguration(const char *sopClassUID, EP_Interpretation photometricInterpretation) const {
	if (sopClassUID) {
		OFString sopClass(sopClassUID);
		  
		// Hardcopy Color Image always requires color-by-plane
		if (sopClass == UID_HardcopyColorImageStorage) {
			return OFTrue;
		}
		
		// The 1996 Ultrasound Image IODs require color-by-plane if color model is YBR_FULL.
		if (photometricInterpretation == EPI_YBR_Full) {
			if ((sopClass == UID_UltrasoundMultiframeImageStorage) ||(sopClass == UID_UltrasoundImageStorage)) {
				return OFTrue;
			}
		}
	}
	return OFFalse;
}


// IMPLEMENT ME
OFCondition DJP2DecoderLossless::jpeg2000_decoder_init() const {
	return EC_Normal;
}

// IMPLEMENT ME
Uint8 DJP2DecoderLossless::jpeg200_scanForBitDepth(Uint8* jpegData, Uint32 fragmentLength) const {
	return 0;
}

// IMPLEMENT ME
EP_Interpretation DJP2DecoderLossless::jpeg2000_getDecompressedColorModel() const {
	return EPI_Unknown;
}

// IMPLEMENT ME
OFCondition DJP2DecoderLossless::jpeg2000_decode(Uint8 *compressedFrameBuffer, Uint32 compressedFrameBufferSize, Uint8 *uncompressedFrameBuffer, Uint32 uncompressedFrameBufferSize) const {
	return EC_Normal;
}
