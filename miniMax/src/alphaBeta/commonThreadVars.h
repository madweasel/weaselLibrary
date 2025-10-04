/*********************************************************************\
	commonThreadVars.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/
#pragma once
#ifdef _MSC_VER
	#define NOMINMAX // Prevent macro conflicts with min/max in Windows headers
#endif
#include "weaselEssentials/src/logger.h"
#include "weaselEssentials/src/threadManager.h"
#include "miniMax/src/typeDef.h"

namespace miniMax
{
    // Base class for thread specific variables
	// it provides readByte() and writeByte() for buffered file access
	// and a mutex for synchronization of the buffer access.
	// Although writing is thread safe, no overlapping writing is allowed.
	// Since the file is opened in exclusive mode, only one thread can write to the file at a time.
	class commonThreadVars : public threadManagerClass::threadVarsArrayItem
	{
    private:
		char											padding[64];											// Padding to avoid cache coherence issues
		const unsigned int 								maxBufferSize 					= FILE_BUFFER_SIZE;		// maximum size of the buffer in bytes
		HANDLE 											hFile							= INVALID_HANDLE_VALUE;	// handle of the file
		wstring 										filePath;												// file path for the file. same for all threads.
		int64_t 										fileSize						= 0;					// size in bytes of the file at the moment
		int64_t 										filePosition					= 0;					// position within the file, being the state number
        int64_t                                         targetFileSize                  = 0;					// target file size in bytes, being the number of states in the layer
		int64_t 										bufferPosition					= 0;					// position within the buffer
		int64_t 										bufferOffset					= 0;					// offset for the buffer within the file
		int64_t &										totalNumStatesProcessed;								// total number of states processed by all threads
		static std::mutex								bufferMutex;											// mutex for the buffer, since it is used by all threads
		std::vector<unsigned char>						buffer;													// since writing/reading happens byte by byte a buffer is used to store the data
        logger &										log;													// logger, used for output

		bool 											loadDataToBuffer				();
		bool 											flush							();

    public:
		unsigned int									layerNumber						= 0;					// current calculated layer
		progressCounter									statesProcessed;										// number of states already calculated in the current layer
		bool											loadFromFile					= false;				// flag indicating if the initialization has already been done

														commonThreadVars		        (commonThreadVars const& master);
														commonThreadVars		        (unsigned int layerNumber, const wstring& filepath, int64_t targetFileSize, int64_t& roughTotalNumStatesProcessed, int64_t& totalNumStatesProcessed, logger &log);
                                                        ~commonThreadVars();

		bool 											readByte						(int64_t positionInFile, unsigned char& data);
		bool 											writeByte						(int64_t positionInFile, unsigned char  data);
		void											reduce                          ();
	};

} // namespace miniMax
