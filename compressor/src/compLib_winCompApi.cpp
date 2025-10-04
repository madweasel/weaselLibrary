/*********************************************************************\
	compressor.h												  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/

#include "compLib_winCompApi.h"

//-----------------------------------------------------------------------------
// Name: 
// Desc: class constructor
//-----------------------------------------------------------------------------
compressor::winCompApi::winCompApi()
{
	name	= L"winCompApi";
	id		= libId::winCompApi;

	if (!CreateCompressor(COMPRESS_ALGORITHM_MSZIP, NULL, &Compressor)) {
		Compressor = NULL;
	}
	if (!CreateDecompressor(COMPRESS_ALGORITHM_MSZIP, NULL, &Decompressor)) {
		Decompressor = NULL;
	}
}

//-----------------------------------------------------------------------------
// Name: 
// Desc: class deconstructor
//-----------------------------------------------------------------------------
compressor::winCompApi::~winCompApi()
{
	if (Compressor   != NULL) CloseCompressor  (Compressor  );
	if (Decompressor != NULL) CloseDecompressor(Decompressor);
}

//-----------------------------------------------------------------------------
// Name: compress()
// Desc: 
//-----------------------------------------------------------------------------
bool compressor::winCompApi::compress(void *compressedData, void *sourceData, unsigned int nBytesToCompress, unsigned int &nBytesCompressed)
{
	// locals
	SIZE_T CompressedDataSize, myCompressedBufferSize;

	// Query compressed buffer size.
    Compress(Compressor, (PBYTE) sourceData, nBytesToCompress, NULL, 0, &myCompressedBufferSize);

	// if buffer is too small, return false
	if (myCompressedBufferSize > CompressedBufferSize) {
		return false;
	}

	// Call Compress() again to do real compression
	Compress(Compressor, (PBYTE) sourceData, nBytesToCompress, (PBYTE) compressedData, CompressedBufferSize, &CompressedDataSize);

	nBytesCompressed = (unsigned int) CompressedDataSize;

	return true;
}

//-----------------------------------------------------------------------------
// Name: decompress()
// Desc: 
//-----------------------------------------------------------------------------
bool compressor::winCompApi::decompress(void *destData, void *compressedData, unsigned int nBytesCompressed, unsigned int &nBytesDecompressed)
{
	// locals
	SIZE_T DecompressedDataSize, myDecompressedBufferSize;

	//  Query decompressed buffer size.
	Decompress(Decompressor, (PBYTE) compressedData, nBytesCompressed, NULL, 0, &myDecompressedBufferSize);
	
	// if buffer is too small, return false
	if (myDecompressedBufferSize > DecompressedBufferSize) {
		return false;
	}

	//  Decompress data and write data to DecompressedBuffer.
    Decompress(Decompressor, (PBYTE) compressedData, nBytesCompressed, (PBYTE) destData, DecompressedBufferSize, &DecompressedDataSize); 

	nBytesDecompressed = (unsigned int) DecompressedDataSize;

	return true;
}

//-----------------------------------------------------------------------------
// Name: estimateMaxSizeOfCompressedData()
// Desc: 
//-----------------------------------------------------------------------------
long long compressor::winCompApi::estimateMaxSizeOfCompressedData(long long amountUncompressedData)
{
	CompressedBufferSize	= static_cast<SIZE_T>(amountUncompressedData + 1000);
	DecompressedBufferSize	= CompressedBufferSize;
	return CompressedBufferSize;
}
