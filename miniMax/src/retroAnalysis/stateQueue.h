/*********************************************************************\
	stateQueue.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/
#pragma once

#include "weaselEssentials/src/logger.h"
#include "weaselEssentials/src/cyclicArray.h"
#include "miniMax/src/typeDef.h"

namespace miniMax
{

namespace retroAnalysis
{
	// Class containing a queue with a large amount of states to process.
	// There is one queue for each ply number, managed as a cyclic array.
	// Each cyclic array is stored in a file, allowing for efficient disk-backed storage and retrieval.
	// The cyclic arrays are dynamically resized as needed, and each queue maintains its own state count.
	// Usage pattern: Each thread should instantiate its own stateQueue instance to avoid concurrency issues.
	// The class is not thread safe; concurrent access must be managed externally.
	// Expected usage: Push states to the queue for processing, pop states when processed, and resize queues as the search depth changes.
	class stateQueue
	{
	public:
														stateQueue						(logger& log, const wstring& fileDirectory, unsigned int threadNo);
														~stateQueue						();

														stateQueue						(stateQueue&& other) noexcept;
		stateQueue& 									operator=						(stateQueue&& other) noexcept;

		bool 											resize							(plyInfoVarType plyNumber, size_t numberOfKnots);																
		bool 											push_back						(const stateAdressStruct& state, plyInfoVarType plyNumber, stateNumberVarType numberOfKnots);
		bool 											pop_front						(stateAdressStruct& state, plyInfoVarType plyNumber);
		unsigned int				 					size							(plyInfoVarType plyNumber);
		long long 						 				getNumStatesToProcess			() { return numStatesToProcess; }
		plyInfoVarType									getMaxPlyInfoValue				() { return maxPlyInfoValue; }
	
	private:
		logger& 										log;													// logger, used for output
		vector<cyclicArray*>							statesToProcess;										// cyclic array containing the states, whose short knot value are known for sure. they have to be processed
		long long										numStatesToProcess				= 0;					// Number of states in 'statesToProcess' which have to be processed
		plyInfoVarType									maxPlyInfoValue					= 0;					// maximum ply info value
		wstring 										fileDirectory;											// directory where the files are stored
        unsigned int                                    threadNo                        = 0xffff;               // thread number, used for file names
		alignas(64) char 								dummy_cache_align;										// Align to cache line (64 bytes)
	};
    
} // namespace retroAnalysis

} // namespace miniMax
