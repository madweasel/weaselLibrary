/*********************************************************************
	cyclicArray.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/

#include "cyclicArray.h"

//-----------------------------------------------------------------------------
// Name: cyclicArray()
// Desc: Creates a cyclic array. The passed file is used as temporary data buffer for the cyclic array.
//-----------------------------------------------------------------------------
cyclicArray::cyclicArray(unsigned int blockSizeInBytes, unsigned int numberOfBlocks, wstring const& fileName, logger& log) :
	hFile(NULL), blockSize(blockSizeInBytes), numBlocks(numberOfBlocks), readingBlock(nullptr), writingBlock(nullptr), log(log)
{
	// checks
	if (blockSize > MAX_BLOCK_SIZE) return;
	if (numBlocks > MAX_NUM_BLOCKS) return;
	if (blockSize * numBlocks > MAX_FILE_SIZE) return;
	if (fileName.length() > MAX_PATH_LENGTH) return;
	if (blockSize == 0) return;
	if (numBlocks == 0) return;
	if (fileName.length() == 0) return;

	// Init blocks
	readingBlock			= new unsigned char[blockSize];
	writingBlock			= new unsigned char[blockSize];
	reset();
	log.log(logger::logLevel::trace, L"cyclicArray created: " + fileName + L" with blockSize: " + std::to_wstring(blockSize) + L" bytes and " + std::to_wstring(numBlocks) + L" blocks.");
	
	// Open Database-File (FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH | FILE_FLAG_RANDOM_ACCESS)
	hFile = CreateFile(fileName.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	// opened file succesfully
	if (hFile == INVALID_HANDLE_VALUE) {
		hFile = NULL;
		return;
	}
}

//-----------------------------------------------------------------------------
// Name: ~cyclicArray()
// Desc: cyclicArray class destructor
//-----------------------------------------------------------------------------
cyclicArray::~cyclicArray()
{
	// delete arrays
	if (readingBlock != nullptr) delete [] readingBlock;
	if (writingBlock != nullptr) delete [] writingBlock;

	// close file
	if (hFile != NULL) CloseHandle(hFile);
}

//-----------------------------------------------------------------------------
// Name: reset()
// Desc: Resets the cyclic array. The file is not changed.
//-----------------------------------------------------------------------------
void cyclicArray::reset()
{
	curReadingPos		= 0;
	curWritingPos		= 0;
	curReadingPointer	= writingBlock;
	curWritingPointer	= writingBlock;
	readWriteInSameRound= true;
	curReadingBlock		= 0;
	curWritingBlock		= 0;
}

//-----------------------------------------------------------------------------
// Name: writeDataToFile()
// Desc: Writes 'sizeInBytes'-bytes to the position 'offset' to the file.
//-----------------------------------------------------------------------------
bool cyclicArray::writeDataToFile(HANDLE hFile, uint64_t offset, unsigned int sizeInBytes, void *pData)
{
	if (hFile == NULL) return log.log(logger::logLevel::error, L"ERROR: File handle is NULL!");
	if (sizeInBytes == 0) return true;
	if (pData == NULL) return log.log(logger::logLevel::error, L"ERROR: Pointer to data is NULL!");
	if (sizeInBytes > MAX_BLOCK_SIZE) return log.log(logger::logLevel::error, L"ERROR: Size of data to write is too large! sizeInBytes:" + std::to_wstring(sizeInBytes) + L" bytes!");
	if (offset < 0) return log.log(logger::logLevel::error, L"ERROR: Offset is negative! offset:" + std::to_wstring(offset) + L" bytes!");
	if (offset + sizeInBytes > MAX_FILE_SIZE) return log.log(logger::logLevel::error, L"ERROR: Offset + sizeInBytes is greater than max file size! offset:" + std::to_wstring(offset) + L" bytes! sizeInBytes:" + std::to_wstring(sizeInBytes) + L" bytes!");

	DWORD			dwBytesWritten;
	LARGE_INTEGER	liDistanceToMove;
	unsigned int	restingBytes = sizeInBytes;
	unsigned int   	numRetries	 = 0;

	liDistanceToMove.QuadPart = offset;

	while (!SetFilePointerEx(hFile, liDistanceToMove, NULL, FILE_BEGIN)) {
		log << L"SetFilePointerEx  failed! Retry counter:" << numRetries << L"\n";
		numRetries++;
		if (numRetries > MAX_NUM_RETRIES) return false;
		Sleep(SLEEP_TIME_IN_MS);
	}
	
	while (restingBytes > 0) {
		if (WriteFile(hFile, pData, sizeInBytes, &dwBytesWritten, NULL) == TRUE) {
			restingBytes -= dwBytesWritten;
			pData		  = (void*) (((unsigned char*) pData) + dwBytesWritten);
			if (restingBytes > 0) {
				log << L"Still " << restingBytes << L" to write!" << L"\n";
			}
		} else {
			log << L"WriteFile Failed! Retry counter:" << numRetries << L"\n";
			numRetries++;
			if (numRetries > MAX_NUM_RETRIES) {
				return log.log(logger::logLevel::error, L"ERROR: WriteFile failed! numRetries:" + std::to_wstring(numRetries));
			}
			Sleep(SLEEP_TIME_IN_MS);
		}
	}
	return true;
}

//-----------------------------------------------------------------------------
// Name: readDataFromFile()
// Desc: Reads 'sizeInBytes'-bytes from the position 'offset' of the file.
//-----------------------------------------------------------------------------
bool cyclicArray::readDataFromFile(HANDLE hFile, uint64_t offset, unsigned int sizeInBytes, void *pData)
{
	if (hFile == NULL) return log.log(logger::logLevel::error, L"ERROR: File handle is NULL!");
	if (sizeInBytes == 0) return true;
	if (pData == NULL) return log.log(logger::logLevel::error, L"ERROR: Pointer to data is NULL!");
	if (sizeInBytes > MAX_BLOCK_SIZE) return log.log(logger::logLevel::error, L"ERROR: Size of data to read is too large! sizeInBytes:" + std::to_wstring(sizeInBytes) + L" bytes!");
	if (offset < 0) return log.log(logger::logLevel::error, L"ERROR: Offset is negative! offset:" + std::to_wstring(offset) + L" bytes!");
	if (offset + sizeInBytes > MAX_FILE_SIZE) return log.log(logger::logLevel::error, L"ERROR: Offset + sizeInBytes is greater than max file size! offset:" + std::to_wstring(offset) + L" bytes! sizeInBytes:" + std::to_wstring(sizeInBytes) + L" bytes!");

	DWORD			dwBytesRead;
	LARGE_INTEGER	liDistanceToMove;
	unsigned int	restingBytes = sizeInBytes;
	unsigned int   	numRetries	 = 0;

	liDistanceToMove.QuadPart = offset;

	while (!SetFilePointerEx(hFile, liDistanceToMove, NULL, FILE_BEGIN)) {
		log << L"SetFilePointerEx failed! Retry counter:" << numRetries << L"\n";
		numRetries++;
		if (numRetries > MAX_NUM_RETRIES) return false;
		Sleep(SLEEP_TIME_IN_MS);
	}
	
	while (restingBytes > 0) {
		if (ReadFile(hFile, pData, sizeInBytes, &dwBytesRead, NULL) == TRUE) {
			restingBytes -= dwBytesRead;
			pData		  = (void*) (((unsigned char*) pData) + dwBytesRead);
			if (restingBytes > 0) {
				log << L"Still " << restingBytes << L" to read!" << L"\n";
			}
		} else {
			log << "ReadFile Failed! Retry counter:" << numRetries << L"\n";
			numRetries++;
			if (numRetries > MAX_NUM_RETRIES) {
				return log.log(logger::logLevel::error, L"ERROR: ReadFile failed! numRetries:" + std::to_wstring(numRetries));
			}
			Sleep(SLEEP_TIME_IN_MS);
		}
	}
	return true;
}

//-----------------------------------------------------------------------------
// Name: bytesAvailable()
// Desc: Returns true if there are bytes available for reading.
//-----------------------------------------------------------------------------
uint64_t cyclicArray::bytesAvailable() const
{
	// cyclic array is empty
	if (curWritingPos == curReadingPos && readWriteInSameRound) {
		return 0;
	// cyclic array is full
	} else if (curWritingPos == curReadingPos) {
		return blockSize * numBlocks;
	// reading and writing pointer are in the same round
	} else if (readWriteInSameRound) {
		return curWritingPos - curReadingPos;
	// reading and writing pointer are in different rounds
	} else {
		return blockSize * numBlocks - (curReadingPos - curWritingPos);
	}
}

//-----------------------------------------------------------------------------
// Name: writeableBytes()
// Desc: Returns the amount of bytes that can be written to the cyclic array at the moment.
//-----------------------------------------------------------------------------
uint64_t cyclicArray::writeableBytes() const
{
	// cyclic array is empty
	if (curWritingPos == curReadingPos && readWriteInSameRound) {
		return blockSize * numBlocks;
	// cyclic array is full
	} else if (curWritingPos == curReadingPos) {
		return 0;
	// reading and writing pointer are in the same round
	} else if (readWriteInSameRound) {
		return blockSize * numBlocks - (curWritingPos - curReadingPos);
	// reading and writing pointer are in different rounds
	} else {
		return curReadingPos - curWritingPos;
	}
}

//-----------------------------------------------------------------------------
// Name: addBytes()
// Desc: Add the passed data to the cyclic array. If the writing pointer reaches the end of a block, 
//       the data of the whole block is written to the file and the next block is considered for writing.
//-----------------------------------------------------------------------------
bool cyclicArray::addBytes(unsigned int numBytes, const unsigned char* pData)
{
	// checks
	if (numBytes == 0) return true;
	if (pData == NULL) return log.log(logger::logLevel::error, L"ERROR: Pointer to data is NULL!");
	if (hFile == NULL) return log.log(logger::logLevel::error, L"ERROR: File handle is NULL!");
	if (numBytes > writeableBytes()) return log.log(logger::logLevel::error, L"ERROR: Not enough space in cyclic array! numBytes:" + std::to_wstring(numBytes) + L" bytes! writeableBytes:" + std::to_wstring(writeableBytes()) + L" bytes!");

	// locals
	unsigned int	bytesWritten = 0;

	// write each byte
	while (bytesWritten < numBytes) {

		// store byte in current reading block
		*curWritingPointer = *pData;
		curWritingPointer++;
		curWritingPos++;
		bytesWritten++;
		pData++;

		// when block is full then save current one to file and begin new one
		if (curWritingPointer == writingBlock + blockSize) {

			// copy data into reading block, if reading block is the same as writing block
			if (curReadingBlock == curWritingBlock) {
				memcpy(readingBlock, writingBlock, blockSize);
				// until now the reading pointer was using the writing block
				// now it has to use the reading block
				curReadingPointer = readingBlock + (curReadingPointer - writingBlock);
			}

			// store block in file
			if (!writeDataToFile(hFile, blockSize * curWritingBlock, blockSize, writingBlock)) {
				return log.log(logger::logLevel::error, L"ERROR: writeDataToFile failed! curWritingBlock:" + std::to_wstring(curWritingBlock) + L" bytesWritten:" + std::to_wstring(bytesWritten) + L" bytes!");
			}

			// set pointer to beginnig of writing block
			curWritingPointer	= writingBlock;
			curWritingBlock		= (curWritingBlock + 1) % numBlocks;
			if (curWritingBlock == 0) {
				readWriteInSameRound 	= false;
				curWritingPos 			= 0;
			}
		}
	}

	// everything ok
	return true;
}

//-----------------------------------------------------------------------------
// Name: takeBytes()
// Desc: Load data from the cyclic array. If the reading pointer reaches the end of a block, 
//       the data of the next whole block is read from the file.
//-----------------------------------------------------------------------------
bool cyclicArray::takeBytes(unsigned int numBytes, unsigned char* pData)
{
	// checks
	if (numBytes == 0) return true;
	if (pData == NULL) return log.log(logger::logLevel::error, L"ERROR: Pointer to data is NULL!");
	if (hFile == NULL) return log.log(logger::logLevel::error, L"ERROR: File handle is NULL!");
	if (bytesAvailable() < numBytes) return log.log(logger::logLevel::error, L"ERROR: Not enough data in cyclic array! numBytes:" + std::to_wstring(numBytes) + L" bytes! bytesAvailable:" + std::to_wstring(bytesAvailable()) + L" bytes!");

	// locals
	unsigned int bytesRead = 0;

	// read each byte 
	while (bytesRead < numBytes) {

		// read current byte
		*pData = *curReadingPointer;
		curReadingPointer++;
		curReadingPos++;
		bytesRead++;
		pData++;

		// load next block?
		if (curReadingPointer == readingBlock + blockSize) {

			// go to next block
			curReadingBlock		= (curReadingBlock + 1) % numBlocks;
			if (curReadingBlock == 0) {
				readWriteInSameRound= true;
				curReadingPos		= 0;
			}

			// writing block reached ?
			if (curReadingBlock == curWritingBlock) {
				curReadingPointer	= writingBlock;

			} else {
			
				// set pointer to beginnig of reading block
				curReadingPointer	= readingBlock;

				// read whole block from file
				if (!readDataFromFile(hFile, blockSize * curReadingBlock, blockSize, readingBlock)) {
					return log.log(logger::logLevel::error, L"ERROR: readDataFromFile failed! curReadingBlock:" + std::to_wstring(curReadingBlock) + L" bytesRead:" + std::to_wstring(bytesRead) + L" bytes!");
				}
			}
		}
	}

	// everything ok
	return true;
}

//-----------------------------------------------------------------------------
// Name: loadFile()
// Desc: Load the passed file into the cyclic array.
//       The passed filename must be different than the passed filename to the constructor cyclicarray().
//-----------------------------------------------------------------------------
bool cyclicArray::loadFile(wstring const& fileName, uint64_t &numBytesLoaded)
{
	// locals
	HANDLE				hLoadFile;
	unsigned char*		dataInFile;
	LARGE_INTEGER		largeInt;
	int64_t				maxFileSize			= blockSize * numBlocks;
	int64_t				curOffset			= 0;	
	uint64_t	numBlocksInFile;
	uint64_t	curBlock;
	uint64_t	numBytesInLastBlock;
						numBytesLoaded		= 0;

	// cyclic array file must be open
	if (hFile == NULL) return log.log(logger::logLevel::error, L"ERROR: File handle is NULL!");

	// Open Database-File (FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH | FILE_FLAG_RANDOM_ACCESS)
	hLoadFile = CreateFile(fileName.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	// opened file succesfully
	if (hLoadFile == INVALID_HANDLE_VALUE) {
		return log.log(logger::logLevel::error, L"ERROR: File handle is NULL! fileName:" + fileName);
	}

	// does data of file fit into cyclic array ?
	GetFileSizeEx(hLoadFile, &largeInt);

	// file too large?
	if (maxFileSize < largeInt.QuadPart) {
		CloseHandle(hLoadFile);
		return log.log(logger::logLevel::error, L"ERROR: File too large! fileName:" + fileName + L" maxFileSize:" + std::to_wstring(maxFileSize) + L" bytes! fileSize:" + std::to_wstring(largeInt.QuadPart) + L" bytes!");
	}

	// reset
	reset();

	// calculate number of blocks
	numBlocksInFile		= (largeInt.QuadPart / blockSize) + 1;
	numBytesInLastBlock	= (largeInt.QuadPart % blockSize);
	dataInFile			= new unsigned char[blockSize];

	// load blocks
	for (curBlock=0; curBlock<numBlocksInFile-1; curBlock++, curOffset += blockSize) {
		
		// load data from file
		if (!readDataFromFile(hLoadFile, curOffset, blockSize, dataInFile)) {
			delete [] dataInFile;
			CloseHandle(hLoadFile);
			return log.log(logger::logLevel::error, L"ERROR: readDataFromFile failed! curBlock:" + std::to_wstring(curBlock) + L" bytesLoaded:" + std::to_wstring(numBytesLoaded) + L" bytes!");
		}

		// put block in cyclic array
		if (!addBytes(blockSize, dataInFile)) {
			delete [] dataInFile;
			CloseHandle(hLoadFile);
			return log.log(logger::logLevel::error, L"ERROR: addBytes failed! curBlock:" + std::to_wstring(curBlock) + L" bytesLoaded:" + std::to_wstring(numBytesLoaded) + L" bytes!");
		}
	}

	// last block
	readDataFromFile(hLoadFile, curOffset, numBytesInLastBlock, dataInFile); 
	if (!addBytes(numBytesInLastBlock, dataInFile)) {
		delete [] dataInFile;
		CloseHandle(hLoadFile);
		return log.log(logger::logLevel::error, L"ERROR: addBytes failed for last block! bytesLoaded:" + std::to_wstring(numBytesLoaded) + L" bytes!");
	}
	curOffset		+= numBytesInLastBlock;
	numBytesLoaded   = curOffset;

	// everything ok
	delete [] dataInFile;
	CloseHandle(hLoadFile);
	return true;
}

//-----------------------------------------------------------------------------
// Name: saveFile()
// Desc: Writes the whole current content of the cyclic array to the passed file.
//       The passed filename must be different than the passed filename to the constructor cyclicarray().
// 		 The cyclic array is not changed by this operation.
//		 The file is overwritten. The file is created if it does not exist.
//		 The first byte takeable byte from the cyclic array is the first byte in the file.
//		 This allows to load the file back into a fresh cyclic array.
//-----------------------------------------------------------------------------
bool cyclicArray::saveFile(wstring const& fileName)
{
	// locals
	unsigned char*		dataInFile;
	HANDLE				hSaveFile;
	uint64_t			curBlock;
	uint64_t			bytesToWrite;
	uint64_t			totalBytesToWrite;
	uint64_t			totalBytesWritten;
	void	*			pointer;

	// cyclic array file must be open
	if (hFile == NULL) {
		return log.log(logger::logLevel::error, L"ERROR: File handle is NULL!");
	}

	// Open Database-File (FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH | FILE_FLAG_RANDOM_ACCESS)
	hSaveFile = CreateFile(fileName.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	// opened file succesfully
	if (hSaveFile == INVALID_HANDLE_VALUE) {
		return log.log(logger::logLevel::error, L"ERROR: File handle is NULL! fileName:" + fileName);
	}

	// alloc mem
	curBlock			= curReadingBlock;
	dataInFile			= new unsigned char[blockSize];
	totalBytesToWrite	= bytesAvailable();
	totalBytesWritten	= 0;

	// save block by block starting with the current reading block
	const uint64_t	MAX_ITERATIONS = numBlocks * 2; // Reasonable upper bound
	uint64_t 		iterationCount = 0;
	while (totalBytesWritten < totalBytesToWrite && iterationCount < MAX_ITERATIONS) {

		// if reading and writing block are the same, and the reading pointer is in the writing block
		if (curBlock == curWritingBlock && curBlock == curReadingBlock && readWriteInSameRound) {
			pointer		 = curReadingPointer;
			bytesToWrite = curWritingPointer - curReadingPointer;
		// if reading and writing block are the same, and the reading pointer is in the reading block
		} else if (curBlock == curWritingBlock && curBlock == curReadingBlock) {
			pointer		 = curReadingPointer;
			bytesToWrite = blockSize - (curReadingPointer - readingBlock);
		// store data from writing block 
		} else if (curBlock == curWritingBlock) {
			pointer		 = writingBlock;
			bytesToWrite = curWritingPointer - writingBlock;
		// store data from reading block
		} else if (curBlock == curReadingBlock) {
			pointer		 = curReadingPointer;
			bytesToWrite = blockSize - (curReadingPointer - readingBlock);
		// store data from file
		} else {
			if (!readDataFromFile(hFile, curBlock * blockSize, blockSize, dataInFile)) {
				delete [] dataInFile;
				CloseHandle(hSaveFile);
				return log.log(logger::logLevel::error, L"ERROR: readDataFromFile failed! curBlock:" + std::to_wstring(curBlock) + L" bytesWritten:" + std::to_wstring(totalBytesWritten) + L" bytes!");
			}
			pointer		 = dataInFile;
			bytesToWrite = blockSize;
		}

		// save data to file
		if (!writeDataToFile(hSaveFile, totalBytesWritten, bytesToWrite, pointer)) {
			delete [] dataInFile;
			CloseHandle(hSaveFile);
			return log.log(logger::logLevel::error, L"ERROR: writeDataToFile failed! curBlock:" + std::to_wstring(curBlock) + L" bytesWritten:" + std::to_wstring(totalBytesWritten) + L" bytes!");
		}
		totalBytesWritten 	+= bytesToWrite;
		curBlock   			 = (curBlock+1) % numBlocks;
		iterationCount++;
	};

	// exceeded maximum iterations?
	if (iterationCount >= MAX_ITERATIONS && totalBytesWritten < totalBytesToWrite) {
		delete [] dataInFile;
		CloseHandle(hSaveFile);
		return log.log(logger::logLevel::error, L"ERROR: saveFile exceeded maximum iterations, possible corrupted state!");
	}

	// everything ok
	delete [] dataInFile;
	CloseHandle(hSaveFile);
	return true;
}
