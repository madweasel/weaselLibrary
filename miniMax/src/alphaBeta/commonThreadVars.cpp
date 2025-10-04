/*********************************************************************
	commonThreadVars.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/

#include "commonThreadVars.h"
#ifdef _MSC_VER
	#define NOMINMAX // Prevent macro conflicts with min/max in Windows headers
#endif
#include <algorithm> // For std::min

// Use one Mutex for all threads to modify succCountArray
std::mutex miniMax::commonThreadVars::bufferMutex = {};

//-----------------------------------------------------------------------------
// Name: commonThreadVars()
// Desc: Constructor
//-----------------------------------------------------------------------------
miniMax::commonThreadVars::commonThreadVars(commonThreadVars const& master) : 
	layerNumber(master.layerNumber), 
	totalNumStatesProcessed(master.totalNumStatesProcessed), 
	loadFromFile(master.loadFromFile), 
	statesProcessed(master.statesProcessed), 
	filePath(master.filePath), 
	fileSize(master.fileSize),
	targetFileSize(master.targetFileSize),
    log(master.log)
{
	if (filePath.empty()) {
		log.log(logger::logLevel::trace, L"File path is empty. No file will be opened.");
		return;
	}

	buffer.reserve(maxBufferSize);

	// each thread needs its own file handle
	if (loadFromFile) {
		hFile = CreateFile(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	} else {
		hFile = CreateFile(filePath.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	}
	if (hFile == INVALID_HANDLE_VALUE) {
		hFile = NULL;
		loadFromFile = false;
		log.log(logger::logLevel::error, L"File handle is null. Failed to open file: " + filePath);
	}
}

//-----------------------------------------------------------------------------
// Name: commonThreadVars()
// Desc: Constructor
//-----------------------------------------------------------------------------
miniMax::commonThreadVars::commonThreadVars(unsigned int layerNumber, const wstring& filepath, int64_t targetFileSize, int64_t& roughTotalNumStatesProcessed, int64_t& totalNumStatesProcessed, logger &log) : 
    layerNumber(layerNumber), 
    totalNumStatesProcessed(totalNumStatesProcessed), 
    statesProcessed(roughTotalNumStatesProcessed), 
    targetFileSize(targetFileSize),
    filePath(filepath),
    log(log)
{
	if (filePath.empty()) {
		log.log(logger::logLevel::trace, L"File path is empty. No file will be opened.");
		return;
	}

	// try to open file 
	hFile = CreateFile(filepath.c_str(), GENERIC_READ, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		log.log(logger::logLevel::error, L"File handle is null. Failed to open file: " + filepath);
		hFile = NULL;
	}

	if (hFile) {
		LARGE_INTEGER liFileSize;
		GetFileSizeEx(hFile, &liFileSize);
		fileSize = liFileSize.QuadPart;
	}

	if (fileSize == targetFileSize) { 
		log << "    Loading init states from file: " << filepath << "\n";
		loadFromFile = true;
	}

	// close file again
	if (hFile != NULL && hFile != INVALID_HANDLE_VALUE) {
		CloseHandle(hFile);
	}
	hFile = NULL;		// each thread needs its own file handle
	buffer.reserve(maxBufferSize);
}

//-----------------------------------------------------------------------------
// Name: ~commonThreadVars()
// Desc: Called by the thread manager. Thus no locking necessary here.
//-----------------------------------------------------------------------------
miniMax::commonThreadVars::~commonThreadVars()
{
	// close file
	if (hFile != NULL && hFile != INVALID_HANDLE_VALUE) {
		CloseHandle(hFile);
	}
}

//-----------------------------------------------------------------------------
// Name: readBytes()
// Desc:  
//-----------------------------------------------------------------------------
bool miniMax::commonThreadVars::readByte(long long positionInFile, unsigned char& data)
{
	// checks
	if (!loadFromFile) return log.log(logger::logLevel::error, L"File not open for reading!");
	if (hFile == NULL || hFile == INVALID_HANDLE_VALUE) return log.log(logger::logLevel::error, L"File handle is null. Failed to read!");
	if (positionInFile < 0) return log.log(logger::logLevel::error, L"Position in file is negative!");
    if (positionInFile >= targetFileSize) return log.log(logger::logLevel::error, L"Position in file is out of range!");
	
	// reload data, if maximum buffer size reached
	if (!buffer.size() ||bufferPosition >= maxBufferSize) {
		bufferOffset 	= positionInFile;
		bufferPosition 	= 0;
		loadDataToBuffer();
	}

	// read data from buffer
	data = buffer[bufferPosition];
	bufferPosition += 1;
	filePosition = positionInFile;

    return true;
}

//-----------------------------------------------------------------------------
// Name: writeBytes()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMax::commonThreadVars::writeByte(long long positionInFile, unsigned char data)
{
	// checks
	if (loadFromFile) return log.log(logger::logLevel::error, L"File is open for reading! Writing not possible!");
	if (hFile == NULL || hFile == INVALID_HANDLE_VALUE) return log.log(logger::logLevel::error, L"File not open for writing!");
	if (positionInFile < 0) return log.log(logger::logLevel::error, L"Position in file is negative!");
    if (positionInFile >= targetFileSize) return log.log(logger::logLevel::error, L"Position in file is out of range!");

	// flush, if maximum buffer size reached
	if (bufferPosition >= maxBufferSize) {
		if (!flush()) {
			return log.log(logger::logLevel::error, L"Flush() failed!");
		}
	}

	// when buffer is empty, then set offset to current file position
	if (!buffer.size()) {
		bufferOffset 	= positionInFile;
		bufferPosition 	= 0;
	}

	// assume that only one byte is written at a time
	buffer.push_back(data);
	bufferPosition += 1;
	filePosition = positionInFile;
	
    return true;
}

//-----------------------------------------------------------------------------
// Name: loadDataToBuffer()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMax::commonThreadVars::loadDataToBuffer()
{
	// check if file is open
	if (hFile == NULL || hFile == INVALID_HANDLE_VALUE) return log.log(logger::logLevel::error, L"File not open for reading. Loading data to buffer failed!");

	// read data from file
	buffer.clear();
	bufferPosition = 0;
	unsigned int bytesToLoad = std::min(static_cast<unsigned int>(fileSize - bufferOffset), maxBufferSize);
	buffer.resize(bytesToLoad);

	DWORD bytesRead = 0;
	{
		std::lock_guard<std::mutex> lock(bufferMutex);
		SetFilePointer(hFile, bufferOffset, NULL, FILE_BEGIN);
		if (ReadFile(hFile, &buffer[0], buffer.size(), &bytesRead, NULL) == FALSE) {
			return log.log(logger::logLevel::error, L"ReadFile() failed at position " + std::to_wstring(bufferOffset) + L"!");
		}
	}
	return true;
}

//-----------------------------------------------------------------------------
// Name: flush()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMax::commonThreadVars::flush()
{
	// check if file is open
	if (hFile == NULL || hFile == INVALID_HANDLE_VALUE) return log.log(logger::logLevel::error, L"File not open for writing. Flushing failed!");

	// write data to file
	if (buffer.size()) {
		DWORD bytesWritten = 0;
		std::lock_guard<std::mutex> lock(bufferMutex);
		SetFilePointer(hFile, bufferOffset, NULL, FILE_BEGIN);
		WriteFile(hFile, &buffer[0], buffer.size(), &bytesWritten, NULL);
		if (bytesWritten != buffer.size()) {
			return log.log(logger::logLevel::error, L"WriteFile() failed at position " + std::to_wstring(bufferOffset) + L"!");
		}
	}

	buffer.clear();
	bufferPosition = 0;
	return true;
}

//-----------------------------------------------------------------------------
// Name: reduce()
// Desc: Called by the thread manager to reduce the thread specific data to the main thread.
//-----------------------------------------------------------------------------
void miniMax::commonThreadVars::reduce()
{
	// when init file was created new then save it now
	if (!loadFromFile && hFile != NULL && hFile != INVALID_HANDLE_VALUE) {
		if (!flush()) {
			log.log(logger::logLevel::error, L"Flush() failed!");
			return;
		}
		if (curThreadNo == 0) {
			log << "Saved initialized states to file: " << filePath << "\n";
		}
	}

	totalNumStatesProcessed += statesProcessed.getStatesProcessedByThisThread(); 
}
