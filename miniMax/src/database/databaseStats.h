/*********************************************************************\
	databaseStats.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/
#pragma once

#include <vector>
#include <list>
#include <mutex>
#include <functional>
#include <string>
#include <windows.h>

#include "weaselEssentials/src/logger.h"

namespace miniMax
{

namespace database
{
    using namespace std;

	// class to measure database io operations per second
	class speedometer
	{
	public:
		typedef function<void(wstring& name, float operationsPerSec)> printFuncType;

	private:
		long long					numOperations					= 0;			// number of operations done since last interval
		long long					printEveryNthOperations			= 0;			// print every n-th operation
		LARGE_INTEGER 				lastTime;										// time of interval for read operations
		LARGE_INTEGER 				frequency;										// performance-counter frequency, in counts per second
		wstring 					name;											// name of the speedometer
		printFuncType				printFunction;									// callback function to print the operations per second
		mutex						mutex;											// mutex for thread safety

	public:
									speedometer						(wstring const& name, long long printEveryNthOperations, printFuncType printFunction);
									~speedometer					();
		void						measureIops						();
	};

	// Structure holding the information about one big array used in the database
	struct arrayInfoStruct
	{
		enum class arrayType : unsigned int {invalid = 0, knotAlreadyCalculated, countArray, plyInfos, layerStats, size};

		arrayType					type						= arrayType::invalid;	// Use case of the array
		long long					sizeInBytes					= 0;					// Size of the array in bytes
		long long					compressedSizeInBytes		= 0;					// Size of the array in bytes after compression
		unsigned int				belongsToLayer				= 0;					// Layer number the array belongs to

		const std::wstring 			getArrTypeName				();
	};

	// Helper structure to update the GUI
	struct arrayInfoChange
	{
		unsigned int				itemIndex					= 0xffffffff;			// index of the GUI element
		arrayInfoStruct *			arrayInfo					= nullptr;				// pointer to the array info, which has to be updated. can be nullptr.
	};

	// Class to store the memory usage about all the arrays in memory
	// It also calls a callback function to update the GUI
	// The class is thread safe
	// Print statements are passed to the logger
	class arrayInfoContainer
	{
	public:
		typedef list<arrayInfoStruct>::iterator ais_itr;								// iterator for the list of array info objects

	private:
		long long					memoryUsed					= 0;					// total memory in bytes used for storing: ply information, short knot value and ...
		unsigned int				numLayers					= 0;					// number of layers
		logger&						log;												// callback function to update the GUI
		list<arrayInfoChange>		arrayInfosToBeUpdated;								// Arrays which have to be updated in the GUI
		list<arrayInfoStruct>		listArrays;											// all arrays added via addArray() in a list
		vector<ais_itr>				vectorArrays;										// iterators referencing all arrays. indexing via [layerNumber*arrayInfoStruct::arrayType::size + type]
		std::mutex 					mutex;												// mutex for thread safety

		size_t						getVectorArrayIndex				(unsigned int layerNumber, arrayInfoStruct::arrayType type);

	public:
									arrayInfoContainer				(logger& log) : log(log) {};
									~arrayInfoContainer				() {};

		bool						addArray						(unsigned int layerNumber, arrayInfoStruct::arrayType type, long long size, long long compressedSize);
		bool						removeArray						(unsigned int layerNumber, arrayInfoStruct::arrayType type, long long size, long long compressedSize);

		void						init							(unsigned int numLayers);

		bool						anyArrayInfoToUpdate			();
		arrayInfoChange 			getArrayInfoForUpdate			();
		long long					getMemoryUsed					()							{ return memoryUsed; };
	};

} // namespace database

} // namespace miniMax