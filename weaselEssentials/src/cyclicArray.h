/*********************************************************************\
	cyclicArray.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/
#ifndef CYLCIC_ARRAY_H
#define CYLCIC_ARRAY_H

#include <windows.h>
#include <iostream>
#include <string>

#include "logger.h"

using namespace std;

/*** Description *********************************************************
 * cyclicArray is a class for cyclic data access.
 * The class uses a file as temporary data buffer for the cyclic array.
 * The class is designed for high performance file access of single bytes, on a very large file.
 * The reading and writing operations are buffered in memory, and are only written to the file when the buffer is full.
 * The class is not thread-safe. 
 * 
 * |-------------------------------------------------|   <- File on disk (hFile)
 * | Block 0 | Block 1 | Block 2 | Block 3 | Block N |
 * |xxxxxxxxx|         |         |xxxxxxxxx|         |   <- blocks loaded in memory (readingBlock, writingBlock)
 * |        ^						^				 |
 * |        |	      				|				 |
 * |      writingPointer		readingPointer		 |	 <- Pointers (curReadingPointer, curWritingPointer, curReadingBlock, curWritingBlock) 
 * 
**************************************************************************/

class cyclicArray
{
private: 
	// Constants
	static const uint64_t 			MAX_BLOCK_SIZE		= 1e9;			// 1 GB
	static const uint64_t 			MAX_NUM_BLOCKS		= 1e8;			// 1 Million
	static const uint64_t 			MAX_FILE_SIZE		= 1e15;			// 1 PetaByte
	static const unsigned int 		MAX_PATH_LENGTH		= 260;			// max length of a path
	static const unsigned int 		MAX_NUM_RETRIES		= 10;			// max number of retries for reading/writing to file
	static const unsigned int 		SLEEP_TIME_IN_MS	= 1000;			// sleep time in ms for waiting for file access

	// Variables
	logger& 						log;								// logger, used for output
	HANDLE							hFile;								// Handle of the file
	unsigned char*					readingBlock;						// Array of size [blockSize] containing the data of the block, where reading is taking place
	unsigned char*					writingBlock;						//			''
	unsigned char*  				curReadingPointer;					// pointer to the byte which is currently read
	unsigned char*  				curWritingPointer;					//			''
	uint64_t						curReadingPos;						// position in the file, where reading is taking place
	uint64_t						curWritingPos;						//			''
	uint64_t						curReadingBlock;					// index of the block, where reading is taking place
	uint64_t						curWritingBlock;					// index of the block, where writing is taking place
	const uint64_t					blockSize;							// size in bytes of a block
	const uint64_t					numBlocks;							// amount of blocks
	bool							readWriteInSameRound;				// true if curReadingBlock > curWritingBlock, false otherwise

	// Functions	
	bool 							writeDataToFile			(HANDLE hFile, uint64_t offset, unsigned int sizeInBytes, void *pData);
	bool 							readDataFromFile		(HANDLE hFile, uint64_t offset, unsigned int sizeInBytes, void *pData);

public:	
    // Constructor / destructor	
    								cyclicArray				(unsigned int blockSizeInBytes, unsigned int numberOfBlocks, wstring const& fileName, logger& log);
    								~cyclicArray			();

	// Functions	
	void							reset					();
	bool							addBytes				(unsigned int numBytes, const unsigned char* pData);
	bool							takeBytes				(unsigned int numBytes,       unsigned char* pData);
	bool							loadFile				(wstring const& fileName, uint64_t &numBytesLoaded);
	bool							saveFile				(wstring const& fileName);
	uint64_t						bytesAvailable			() const;
	uint64_t 						writeableBytes			() const;
	uint64_t						getNumBlocks			() const { return numBlocks; }
};

#endif
