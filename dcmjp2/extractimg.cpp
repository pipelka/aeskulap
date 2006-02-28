#include "dcfilefo.h"
#include "dcdeftag.h"
#include "dcpixseq.h"
#include "dcpxitem.h"
#include "dcpixel.h"
#include "dctypes.h"
#include "dcmetinf.h"
#include "dcmimage.h"
#include "diregist.h"

#include <iostream>
#include <fstream>

int main(int argc, char* argv[]) {
	DcmFileFormat ff;
	
	if(ff.loadFile(argv[1]).bad()) {
		std::cerr << "unable to load file!" << std::endl;
		return 1;
	}

	ff.loadAllDataIntoMemory();
	
	DcmDataset* dset = ff.getDataset();
	DcmMetaInfo* meta = ff.getMetaInfo();

	DcmPixelData* pixData = NULL;
	DcmPixelSequence* pixSeq = NULL;
	DcmElement* elem = NULL;

	dset->print(COUT, DCMTypes::PF_shortenLongTagValues);

	OFCondition cond = dset->findAndGetElement(DCM_PixelData, elem, OFTrue);
	if(cond.bad()) {
		std::cerr << "unable to get pixeldata" << std::endl;
		std::cerr << cond.text() << std::endl;
		return 1;
	}
	
	pixData = static_cast<DcmPixelData*>(elem);

	/*if(pixSeq->getItem(pixItem, 1).bad()) {
		std::cerr << "unable to extract pixelitem" << std::endl;
		return 1;
	}*/
	const DcmRepresentationParameter *param = NULL;
	E_TransferSyntax xfer;

	pixData->getOriginalRepresentationKey(xfer, param);

	cond = pixData->getEncapsulatedRepresentation(
		xfer,
		param,
		pixSeq);

	if(cond.bad()) {
		std::cerr << "unable to get pixelsequence" << std::endl;
		std::cerr << cond.text() << std::endl;
		return 1;
	}

	DcmPixelItem* pixItem = NULL;
	cond = pixSeq->getItem(pixItem, 1);
	if(cond.bad()) {
		std::cerr << "unable to get pixelitem" << std::endl;
		std::cerr << cond.text() << std::endl;
		return 1;
	}

	Uint8* data;
	if(pixItem->getUint8Array(data).bad()) {
		std::cerr << "unable to get pixeldata" << std::endl;
	}

	Uint32 length = pixItem->getLength();
	std::cout << "datalength: " << length << std::endl;
	
	std::ofstream o(argv[2], std::ios::binary | std::ios::trunc);
	o.write((char*)data, length);
	o.close();

	return 0;
}
