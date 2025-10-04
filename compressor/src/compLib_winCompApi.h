/*********************************************************************\
	compressor.h												  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/
#pragma once

#include "compressor.h"
#include <compressapi.h>

/*** Classes *********************************************************/

namespace compressor
{
	class winCompApi : public generalLib
	{
	public:
							winCompApi						();
							~winCompApi						();
		bool				compress						(void *compressedData, void *sourceData, unsigned int nBytesToCompress, unsigned int &nBytesCompressed) override;
		bool				decompress						(void *destData, void *compressedData, unsigned int nBytesCompressed, unsigned int &nBytesDecompressed) override;
		long long			estimateMaxSizeOfCompressedData	(long long amountUncompressedData) override;

	private:
		COMPRESSOR_HANDLE	Compressor						= NULL;
		DECOMPRESSOR_HANDLE Decompressor					= NULL;
		PBYTE				CompressedBuffer				= NULL;
		SIZE_T				CompressedBufferSize			= 0;
		SIZE_T				DecompressedBufferSize			= 0;
	};
} // namespace compressor