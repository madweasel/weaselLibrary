/*********************************************************************
	stateQueue.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/

#include "stateQueue.h"

//-----------------------------------------------------------------------------
// Name: stateQueue()
// Desc: Constructor
//-----------------------------------------------------------------------------
miniMax::retroAnalysis::stateQueue::stateQueue(logger& log, const wstring& fileDirectory, unsigned int threadNo) : 
	log(log), numStatesToProcess(0), maxPlyInfoValue(0), fileDirectory(fileDirectory), threadNo(threadNo)
{
	statesToProcess.resize(PLYINFO_EXP_VALUE, nullptr);
}

//-----------------------------------------------------------------------------
// Name: ~stateQueue()
// Desc: Destructor
//-----------------------------------------------------------------------------
miniMax::retroAnalysis::stateQueue::~stateQueue()
{
	for (unsigned int plyCounter=0; plyCounter<statesToProcess.size(); plyCounter++) { 
		SAFE_DELETE(statesToProcess[plyCounter]);
	}
}

//-----------------------------------------------------------------------------
// Name: stateQueue()
// Desc: move constructor
//-----------------------------------------------------------------------------
miniMax::retroAnalysis::stateQueue::stateQueue(stateQueue&& other) noexcept :
	log(other.log), 
	statesToProcess(std::move(other.statesToProcess)), 
	fileDirectory(std::move(other.fileDirectory)), 
	numStatesToProcess(other.numStatesToProcess), 
	maxPlyInfoValue(other.maxPlyInfoValue), 
	threadNo(other.threadNo)
{
	other.numStatesToProcess = 0;
	other.maxPlyInfoValue = 0;
	other.threadNo = 0xffff;
}

//-----------------------------------------------------------------------------
// Name: operator=()
// Desc: move assignment operator
//-----------------------------------------------------------------------------
miniMax::retroAnalysis::stateQueue& miniMax::retroAnalysis::stateQueue::operator=(stateQueue&& other) noexcept
{
	if (this != &other)
	{
		log = std::move(other.log);
		statesToProcess = std::move(other.statesToProcess);
		fileDirectory = std::move(other.fileDirectory);
		numStatesToProcess = other.numStatesToProcess;
		maxPlyInfoValue = other.maxPlyInfoValue;
		threadNo = other.threadNo;
		other.numStatesToProcess = 0;
		other.maxPlyInfoValue = 0;
		other.threadNo = 0xffff;
	}
	return *this;
}

//-----------------------------------------------------------------------------
// Name: resize()
// Desc: Resizes the state queue to the given number of knots.
//-----------------------------------------------------------------------------
bool miniMax::retroAnalysis::stateQueue::resize(plyInfoVarType plyNumber, size_t numberOfKnots)
{
	// checks
	if (plyNumber >= statesToProcess.size()) {
		log << "ERROR: Ply number must be smaller than size of statesToProcess! plyNumber:" << plyNumber << ", max allowed: " << (statesToProcess.size() - 1) << "\n";
		return returnValues::falseOrStop();
	}
	if (numberOfKnots <= 0) {
		log << "ERROR: Number of knots must be greater than 0! plyNumber:" << plyNumber << "\n";
		return returnValues::falseOrStop();
	}

	// locals
	wstringstream	ssStatesToProcessFilePath;
	wstringstream	ssStatesToProcessPath;		

	// create file name
	ssStatesToProcessPath	<< fileDirectory << (fileDirectory.size()?"\\":"") << "statesToProcess";
	CreateDirectory(ssStatesToProcessPath.str().c_str(), NULL);
	ssStatesToProcessFilePath.str(L"");
	ssStatesToProcessFilePath << ssStatesToProcessPath.str() << "\\statesToProcessWithPlyCounter=" << plyNumber << "andThread=" << threadNo << ".dat";
	
	// create cyclic array for this ply number
	statesToProcess[plyNumber] = new cyclicArray(BLOCK_SIZE_IN_CYCLIC_ARRAY * sizeof(stateAdressStruct), (numberOfKnots / BLOCK_SIZE_IN_CYCLIC_ARRAY) + 1, ssStatesToProcessFilePath.str().c_str(), log);
	
	log.log(logger::logLevel::trace, L"Created cyclic array: " + ssStatesToProcessFilePath.str());
    return true;
}

//-----------------------------------------------------------------------------
// Name: push_back()
// Desc: Adds a state to the queue for the given ply number. Therefore a cyclic array is used. The cyclic array is stored in a file.
//-----------------------------------------------------------------------------
bool miniMax::retroAnalysis::stateQueue::push_back(const stateAdressStruct& state, plyInfoVarType plyNumber, stateNumberVarType numberOfKnots)
{
    // checks if the ply number is valid
    if (plyNumber < 0) {
        log << "ERROR: Ply number must be greater or equal to 0! plyNumber:" << plyNumber << "\n";
        return returnValues::falseOrStop();
    }
    if (numberOfKnots <= 0) {
        log << "ERROR: Number of knots must be greater than 0! plyNumber:" << plyNumber << "\n";
        return returnValues::falseOrStop();
    }
    if (state.stateNumber >= numberOfKnots) {
        log << "ERROR: State number must be smaller than number of knots! stateNumber:" << state.stateNumber << " numberOfKnots:" << numberOfKnots << "\n";
        return returnValues::falseOrStop();
    }

	// resize vector if too small
	if (plyNumber >= statesToProcess.size()) {
		statesToProcess.resize(max((size_t) (plyNumber+1), 10*statesToProcess.size()), nullptr);
		log.log(logger::logLevel::warning, L"statesToProcess resized to " + std::to_wstring(statesToProcess.size()));
	}

	// set max ply info value
	if (plyNumber > maxPlyInfoValue) maxPlyInfoValue = plyNumber;

	// initialize cyclic array if necessary
	if (statesToProcess[plyNumber] == nullptr) {
		return log.log(logger::logLevel::error, L"ERROR: statesToProcess[" + std::to_wstring(plyNumber) + L"] not initialized! Call resize() first"), returnValues::falseOrStop();
	}

	// add state
	if (!statesToProcess[plyNumber]->addBytes(sizeof(stateAdressStruct), (unsigned char*) &state)) {
		log << "ERROR: Cyclic list to small! numStatesToProcess:" << numStatesToProcess << "\n";
		log << "ERROR: getNumBlocks() = " << statesToProcess[plyNumber]->getNumBlocks() << "\n";
		return returnValues::falseOrStop();
	}

	// everything was fine
	numStatesToProcess++;
	return true;
}

//-----------------------------------------------------------------------------
// Name: pop_front()
// Desc: Sets the parameter 'state' to the first state in the queue for the given ply number. Returns false if the queue is empty.
//-----------------------------------------------------------------------------
bool miniMax::retroAnalysis::stateQueue::pop_front(stateAdressStruct& state, plyInfoVarType plyNumber)
{
	// check parameter
	if (plyNumber >= statesToProcess.size() || statesToProcess[plyNumber] == nullptr) return false;
	if (numStatesToProcess == 0) return false;
	if (statesToProcess[plyNumber]->bytesAvailable() < sizeof(stateAdressStruct)) return false;

	// take state
	if (!statesToProcess[plyNumber]->takeBytes(sizeof(stateAdressStruct), (unsigned char*) &state)) {
		log << "ERROR: takeBytes failed! numStatesToProcess:" << numStatesToProcess << "\n";
		return returnValues::falseOrStop();
	}

	// everything was fine
	numStatesToProcess--;
	return true;
}

//-----------------------------------------------------------------------------
// Name: size()
// Desc: Returns the number of states in the queue for the given ply number
//-----------------------------------------------------------------------------
unsigned int miniMax::retroAnalysis::stateQueue::size(plyInfoVarType plyNumber)
{
	// check parameter
	if (plyNumber >= statesToProcess.size() || statesToProcess[plyNumber] == nullptr) return 0;

	// get size
	return statesToProcess[plyNumber]->bytesAvailable() / sizeof(stateAdressStruct);
}
