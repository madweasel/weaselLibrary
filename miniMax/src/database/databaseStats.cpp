/*********************************************************************
	databaseStats.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/

#include <windows.h> 	// QueryPerformanceCounter, QueryPerformanceFrequency

#include "databaseStats.h"

#pragma region speedometer
//-----------------------------------------------------------------------------
// Name: speedometer()
// Desc: constructor
//-----------------------------------------------------------------------------
miniMax::database::speedometer::speedometer(wstring const& name, long long printEveryNthOperations, printFuncType printFunction) :
	name(name), printEveryNthOperations(printEveryNthOperations), numOperations(0), printFunction(printFunction)
{
	// for io operations per second measurement
	QueryPerformanceFrequency(&frequency);
}

//-----------------------------------------------------------------------------
// Name: ~speedometer()
// Desc: destructor
//-----------------------------------------------------------------------------
miniMax::database::speedometer::~speedometer()
{
}

//-----------------------------------------------------------------------------
// Name: measureIops()
// Desc: 
//-----------------------------------------------------------------------------
void miniMax::database::speedometer::measureIops()
{
	// thread safety
	std::lock_guard<std::mutex> lock(mutex);
	
	// first call
	if (numOperations == 0) {
		QueryPerformanceCounter(&lastTime);
	}

	// count operation
	numOperations++;

	if (numOperations >= printEveryNthOperations) {
		LARGE_INTEGER curTime;
		QueryPerformanceCounter(&curTime);
		double totalTimeGone = (double) (curTime.QuadPart - lastTime.QuadPart) / frequency.QuadPart;
		printFunction(name, numOperations / totalTimeGone);
		lastTime.QuadPart	= curTime.QuadPart;
		numOperations		= 0;
	}
}
#pragma endregion

#pragma region arrayInfoContainer
//-----------------------------------------------------------------------------
// Name: arrayInfoContainer::addArray()
// Desc: Register a new array in the database
//		 Layer number must be smaller than numLayers, passed in init()
// 		 Size must be greater than 0, compressedSize can be 0
//		 Type must be valid
//		 If the array already exists, the function returns false
// 		 If the array is added, the function returns true
//-----------------------------------------------------------------------------
bool miniMax::database::arrayInfoContainer::addArray(unsigned int layerNumber, arrayInfoStruct::arrayType type, long long size, long long compressedSize)
{
	// checks
	if (!size) return false;
	if (numLayers <= layerNumber) return false;
	if (type == arrayInfoStruct::arrayType::invalid) return false;
	if (type == arrayInfoStruct::arrayType::size) return false;

	// locals
	auto vectorArrayIndex = getVectorArrayIndex(layerNumber, type);

	// thread safety
	std::lock_guard<std::mutex> lock(mutex);

	// if index is out of range, the function returns false
	if (vectorArrays.size() <= vectorArrayIndex) return false;

	// If the array already exists, the function returns false
	if (vectorArrays[vectorArrayIndex] != listArrays.end()) return false;

	// create new info object and add to list
	arrayInfoStruct ais;
	ais.belongsToLayer			= layerNumber;
	ais.compressedSizeInBytes	= compressedSize;
	ais.sizeInBytes				= size;
	ais.type					= type;
	listArrays.push_back(ais);

	// notify change
	arrayInfoChange	aic;
	aic.arrayInfo				= &listArrays.back();
	aic.itemIndex				= (unsigned int) listArrays.size() - 1;
	arrayInfosToBeUpdated.push_back(aic);

	// save pointer of info in vector for direct access
	vectorArrays[vectorArrayIndex] = (--listArrays.end());

	// update total memory usage
	memoryUsed += size;
	
	// update GUI
	log.log(logger::logLevel::trace, L"Allocated " + to_wstring(size) + L" bytes in memory for array type " + ais.getArrTypeName() + L" of layer " + to_wstring(layerNumber));

	return true;
}

//-----------------------------------------------------------------------------
// Name: getArrTypeName::removeArray()
// Desc: Get the name of the array type
//-----------------------------------------------------------------------------
const std::wstring miniMax::database::arrayInfoStruct::getArrTypeName()
{
	switch (type)
	{
	case miniMax::database::arrayInfoStruct::arrayType::countArray: 			return L"countArray";
	case miniMax::database::arrayInfoStruct::arrayType::knotAlreadyCalculated: 	return L"knotAlreadyCalculated";
	case miniMax::database::arrayInfoStruct::arrayType::layerStats: 			return L"layerStats";
	case miniMax::database::arrayInfoStruct::arrayType::plyInfos: 				return L"plyInfos";
	case miniMax::database::arrayInfoStruct::arrayType::size: 					return L"size";
	default: 																	return L"invalid";
	}
}

//-----------------------------------------------------------------------------
// Name: arrayInfoContainer::removeArray()
// Desc: Remove an array from the database
//-----------------------------------------------------------------------------
bool miniMax::database::arrayInfoContainer::removeArray(unsigned int layerNumber, arrayInfoStruct::arrayType type, long long size, long long compressedSize)
{
	// checks
	if (!size) return false;
	if (numLayers <= layerNumber) return false;
	if (type == arrayInfoStruct::arrayType::size) return false;

	// locals
	auto vectorArrayIndex = getVectorArrayIndex(layerNumber, type);

	// find info object in list
	std::lock_guard<std::mutex> lock(mutex);

	// if index is out of range, the function returns false
	if (vectorArrays.size() <= vectorArrayIndex) return false;

	// If the array does not exist, the function returns false
	list<arrayInfoStruct>::iterator& itr = vectorArrays[vectorArrayIndex];
	if (listArrays.size() == 0)	return false;
	if (itr == listArrays.end()) return false;

	// does sizes fit?
	if (itr->belongsToLayer != layerNumber || itr->type!=type || itr->sizeInBytes!=size || itr->compressedSizeInBytes!=compressedSize) return false;

	// notify cahnge
	arrayInfoChange	aic;
	aic.arrayInfo				= NULL;
	aic.itemIndex				= (unsigned int) std::distance(listArrays.begin(), itr);
	arrayInfosToBeUpdated.push_back(aic);

	// delete item from list
	listArrays.erase(itr);
	vectorArrays[vectorArrayIndex] = listArrays.end();
	
	// update total memory usage
	memoryUsed -= size;

	// update GUI
	log.log(logger::logLevel::trace, L"Deallocated " + to_wstring(size) + L" bytes in memory for array type " + to_wstring(static_cast<unsigned int>(type)) + L" of layer " + to_wstring(layerNumber));

	return true;
}

//-----------------------------------------------------------------------------
// Name: arrayInfoContainer::init()
// Desc: Initialize the array info container 
//-----------------------------------------------------------------------------
void miniMax::database::arrayInfoContainer::init(unsigned int numLayers)
{
	std::lock_guard<std::mutex> lock(mutex);
	this->numLayers = numLayers;
	vectorArrays.resize(numLayers * static_cast<size_t>(arrayInfoStruct::arrayType::size), listArrays.end());
	arrayInfosToBeUpdated.clear();
	listArrays.clear();
}

//-----------------------------------------------------------------------------
// Name: anyArrayInfoToUpdate()
// Desc: Return true if there are any array infos to update in the GUI
//-----------------------------------------------------------------------------
bool miniMax::database::arrayInfoContainer::anyArrayInfoToUpdate()
{
	std::lock_guard<std::mutex> lock(mutex);
	return (arrayInfosToBeUpdated.size() > 0);
}

//-----------------------------------------------------------------------------
// Name: getArrayInfoForUpdate()
// Desc: Get the next array info to update in the GUI
//-----------------------------------------------------------------------------
miniMax::database::arrayInfoChange miniMax::database::arrayInfoContainer::getArrayInfoForUpdate()
{
	std::lock_guard<std::mutex> lock(mutex);

	// no array info to update
	if (arrayInfosToBeUpdated.size() == 0) {
		arrayInfoChange aic;
		aic.arrayInfo = nullptr;
		aic.itemIndex = 0xffffffff;
		return aic;
	}
	// get the array info
	arrayInfoChange tmp = arrayInfosToBeUpdated.front();
	arrayInfosToBeUpdated.pop_front();
	return tmp;
}

//-----------------------------------------------------------------------------
// Name: getVectorArrayIndex()
// Desc: Get the index of the array in the vector
//-----------------------------------------------------------------------------
size_t miniMax::database::arrayInfoContainer::getVectorArrayIndex(unsigned int layerNumber, arrayInfoStruct::arrayType type) 
{
	return layerNumber * static_cast<unsigned int>(arrayInfoStruct::arrayType::size) + static_cast<unsigned int>(type); 
}

#pragma endregion

