/*********************************************************************
	successorCountArray.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/

#include "successorCountArray.h"

#pragma region successorCountManager
//-----------------------------------------------------------------------------
// Name: successorCountManager()
// Desc: Constructor for the successor count manager
//-----------------------------------------------------------------------------
miniMax::retroAnalysis::successorCountManager::successorCountManager(logger& log, threadManagerClass& tm, database::database& db, gameInterface& game, vector<stateQueue>& statesToProcess)
	: log(log), tm(tm), db(db), game(game), statesToProcess(statesToProcess)
{
}

//-----------------------------------------------------------------------------
// Name: ~successorCountManager()
// Desc: Delete successor count manager and all successor count arrays
//-----------------------------------------------------------------------------
miniMax::retroAnalysis::successorCountManager::~successorCountManager()
{
	for (auto& sca : succCountArrays) {
		SAFE_DELETE(sca);
	}
}

//-----------------------------------------------------------------------------
// Name: isReady()
// Desc: Check if all successor count arrays are initialized
//-----------------------------------------------------------------------------
bool miniMax::retroAnalysis::successorCountManager::isReady()
{
	if (!succCountArrays.size()) return false;
	for (auto& sca : succCountArrays) {
		if (sca->getLayerNumber() >= db.getNumLayers()) return false;
	}
	return true;
}

//-----------------------------------------------------------------------------
// Name: getAndDecreaseCounter()
// Desc: Get the number of succeding states for the state and decrease the counter by one in a thread safe way.
//-----------------------------------------------------------------------------
miniMax::countArrayVarType miniMax::retroAnalysis::successorCountManager::getAndDecreaseCounter(unsigned int layerNumber, stateNumberVarType stateNumber)
{
	for (auto& sca : succCountArrays) { 
		if (sca->getLayerNumber() == layerNumber) {
			return sca->decreaseCounter(stateNumber);
		};
	}
	return COUNT_ARRAY_MAX_VALUE;
}

//-----------------------------------------------------------------------------
// Name: init()
// Desc: Read from file or calculate the number of succeding states for each state all layers to calculate
//-----------------------------------------------------------------------------
bool miniMax::retroAnalysis::successorCountManager::init(vector<unsigned int>& layersToCalculate)
{
	// checks
	for (auto layerNumber : layersToCalculate) {
		if (layerNumber >= db.getNumLayers()) {
			log << "ERROR: Layer number is out of range!\n";
			return returnValues::falseOrStop();
		}	
	}
	curAction = activity::prepareCountArray;

	// clear old data
	for (auto& sca : succCountArrays) {
		SAFE_DELETE(sca);
	}
	succCountArrays.clear();
	layerProcessed.clear();

	// allocate memory for the successor count arrays, one for each layer in 'layersToCalculate'
	for (size_t id = 0; id < layersToCalculate.size(); id++) {
		succCountArrays.push_back(new successorCountArray(log, db, layersToCalculate[id]));
	}

	// prepare file read/write
	vector<successorCountFileStorage::layerInfoStruct> layersInfo;
	layersInfo.reserve(layersToCalculate.size());
	for (auto& sca : succCountArrays) {
		layersInfo.push_back(successorCountFileStorage::layerInfoStruct{ sca->layerNumber, db.getNumberOfKnots(sca->layerNumber), *sca });
	}
	successorCountFileStorage scfs = successorCountFileStorage(log, db.getFileDirectory(), layersInfo);

	// try to load the count arrays from file
	loadedScaFromFile = scfs.read();

	for (auto& sca : succCountArrays) {
		if (!initLayer(*sca)) {
			log << "ERROR: Could not initialize count array for layer " << sca->getLayerNumber() << "!\n";
			return returnValues::falseOrStop();
		}
	}

	// save the count arrays to file
	if (!loadedScaFromFile) {
		if (!scfs.write()) {
			log << "ERROR: Could not save count arrays to file!\n";
			return returnValues::falseOrStop();
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// Name: initLayer()
// Desc: Initialize the count array for the current layer 
//-----------------------------------------------------------------------------
bool miniMax::retroAnalysis::successorCountManager::initLayer(successorCountArray& sca)
{
	// locals
	unsigned int layerNumber = sca.getLayerNumber();

	// don't process layers without states
	if (db.getNumberOfKnots(layerNumber) == 0) return true;

	// output & filenames
	log << "*********************************************\n" 
		<< "*** Prepare count arrays for layers " << layerNumber << " ***\n"
		<< "*********************************************\n";
		
	// calculate number of succedding states
	if (!calcNumSuccedors(layerNumber)) {
		return log.log(logger::logLevel::error, L"Could not calculate number of succedors!");
	}
	
	// finish
	return true;
}

//-----------------------------------------------------------------------------
// Name: calcNumSuccedors()
// Desc: Calculate the number of succeding states for each state of the current layer. 
//-----------------------------------------------------------------------------
bool miniMax::retroAnalysis::successorCountManager::calcNumSuccedors(unsigned int layerNumber)
{
	// checks
	if (layerNumber >= db.getNumLayers()) {
		log << "ERROR: Layer number is out of range!\n";
		return returnValues::falseOrStop();
	}
	if (db.getNumberOfKnots(layerNumber) == 0) return true;

	log << "*** Calculate number of succeding states for each state of layer " << layerNumber << " ***" << "\n";

	// mark all layers as not processed. this is important, since db/game might return the same layer multiple times as succeding layer
	layerProcessed.resize(db.getNumLayers(), false);

	// go through each state in the current layer ...
	if (!addNumSuccedors(layerNumber)) {
		log << "ERROR: Could not calculate number of succedors for layer " << layerNumber << "!\n";
		return returnValues::falseOrStop();
	}

	// ... and go through each state in each succeding layer
	for (auto& curSuccLayer : db.getSuccLayers(layerNumber)) {
		
		// don't process layers without states
		if (!db.getNumberOfKnots(curSuccLayer)) continue;

		// preload succeeding layers
		db.setLoadingOfFullLayerOnRead();

		// dont't process layers that have already been processed
		if (layerNumber == curSuccLayer) continue;

		log << "- Do the same for the succeding layer " << (int) curSuccLayer << " (" << game.getOutputInformation(curSuccLayer) << ")" << "\n";

		// load layer into memory
		if (!db.loadLayerFromFile(curSuccLayer)) {
			log << "ERROR: Could not load layer " << curSuccLayer << " from file!\n";
			return returnValues::falseOrStop();
		}

		if (!addNumSuccedors(curSuccLayer)) {
			log << "ERROR: Could not calculate number of succedors for layer " << curSuccLayer << "!\n";
			return returnValues::falseOrStop();
		}
	}

	// everything fine
	return true;
}

//-----------------------------------------------------------------------------
// Name: addNumSuccedors()
// Desc: Add the number of succeding states for each state of the current layer
//-----------------------------------------------------------------------------
bool miniMax::retroAnalysis::successorCountManager::addNumSuccedors(unsigned int layerNumber)
{
	// checks
	if (layerNumber >= db.getNumLayers()) return returnValues::falseOrStop();
	if (db.getNumberOfKnots(layerNumber) == 0) return returnValues::falseOrStop();
	if (layerProcessed.size() <= layerNumber) return returnValues::falseOrStop();
	if (layerProcessed[layerNumber]) return true;

	// prepare parameters for multithreading
	totalNumStatesProcessed 		= 0;
	roughTotalNumStatesProcessed	= 0;
	threadManagerClass::threadVarsArray<addNumSuccedorsVars> tva(tm.getNumThreads(), addNumSuccedorsVars(*this, layerNumber, roughTotalNumStatesProcessed));

	// process each state in the current layer
	switch (tm.executeParallelLoop(addNumSuccedorsThreadProc, tva.getPointerToArray(), tva.getSizeOfArray(), TM_SCHEDULE_STATIC, 0, db.getNumberOfKnots(layerNumber) - 1, 1))
	{
	case TM_RETURN_VALUE_OK: 			
		break;
	case TM_RETURN_VALUE_EXECUTION_CANCELLED:
		log << "\n" << "****************************************\nMain thread: Execution cancelled by user!\n****************************************\n";
		return false;
	default:
	case TM_RETURN_VALUE_INVALID_PARAM:
	case TM_RETURN_VALUE_UNEXPECTED_ERROR:
		log << "ERROR: Unexpected error in thread manager!\n";
		return returnValues::falseOrStop();
	}

	// reduce and delete thread specific data
	tva.reduce();
	if (totalNumStatesProcessed != db.getNumberOfKnots(layerNumber)) {
		log.log(logger::logLevel::error, L"Number of processed states is less than the number of knots in the layer!");
		return returnValues::falseOrStop();
	} else {
		log << "Number of states processed: " << totalNumStatesProcessed << "\n";
	}

	// mark layer as processed
	layerProcessed[layerNumber] = true;

	// everything fine
	return true;
}

//-----------------------------------------------------------------------------
// Name: addNumSuccedorsThreadProc()
// Desc: 
//-----------------------------------------------------------------------------
DWORD miniMax::retroAnalysis::successorCountManager::addNumSuccedorsThreadProc(void* pParameter, int64_t index)
{
	// check parameter
	if (pParameter == nullptr) return returnValues::falseOrStop();

	// locals
	addNumSuccedorsVars &			ansVars					= *((addNumSuccedorsVars *) pParameter);
	successorCountManager&			scm						= ansVars.scm;
	vector<successorCountArray*>&	sca						= scm.succCountArrays;
	logger&							log						= scm.log;		// all successor count arrays have the same logger
	database::database&				db						= scm.db;		// ''
	gameInterface&					game					= scm.game;		// ''
	stateAdressStruct				curState;
	twoBit							curStateValue;

	curState.layerNumber	= ansVars.layerNumber;
	curState.stateNumber	= (stateNumberVarType) index;

	// print status
	ansVars.statesProcessed.stateProcessed(log, db.getNumberOfKnots(curState.layerNumber), L"    Already processed ");

	// invalid state? Then don't care about it.
	if (!db.readKnotValueFromDatabase(curState.layerNumber, curState.stateNumber, curStateValue)) {
		log.log(logger::logLevel::error, L"readKnotValueFromDatabase() returned false!");
		return TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
	}
	if (curStateValue == SKV_VALUE_INVALID) return TM_RETURN_VALUE_OK;

	// add solved states to list of states to process
	if (curStateValue == SKV_VALUE_GAME_LOST || curStateValue == SKV_VALUE_GAME_WON) {
		plyInfoVarType numPlies;
		if (!db.readPlyInfoFromDatabase(curState.layerNumber, curState.stateNumber, numPlies)) {
			log.log(logger::logLevel::error, L"readPlyInfoFromDatabase() returned false!");
			return TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
		}
		if (!scm.statesToProcess[ansVars.curThreadNo].push_back(curState, numPlies, db.getNumberOfKnots(curState.layerNumber))) {
			log.log(logger::logLevel::error, L"push_back() returned false!");
			return TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
		};
	}

	// if loaded from file, then don't process the states again.
	// the statesToProcess are already filled at this point here.
	if (scm.loadedScaFromFile) {
		return TM_RETURN_VALUE_OK;
	}

    // set current selected situation
	if (!game.setSituation(ansVars.curThreadNo, curState.layerNumber, curState.stateNumber)) {
		log.log(logger::logLevel::error, L"setSituation() returned false!");
		return TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
	}

    // get list with statenumbers of predecessors
    game.getPredecessors(ansVars.curThreadNo, ansVars.predVars);

	// store the predecessor states in a thread specific file
	for (unsigned int curPred=0; curPred<ansVars.predVars.size(); curPred++) {
		stateAdressStruct predState;
        predState.layerNumber = ansVars.predVars[curPred].predLayerNumber;
        predState.stateNumber = ansVars.predVars[curPred].predStateNumber;
		if (!ansVars.storePredecessorState(predState)) {
			log.log(logger::logLevel::error, L"storePredecessorState() returned false!");
			return TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
		}
	}

	// everything is fine
	return TM_RETURN_VALUE_OK;
}
#pragma endregion

#pragma region successorCountArray
//-----------------------------------------------------------------------------
// Name: successorCountArray()
// Desc: Constructor for the successor count array
//-----------------------------------------------------------------------------
miniMax::retroAnalysis::successorCountArray::successorCountArray(logger& log, database::database& db, unsigned int layerNumber)
	: log(log), db(db), layerNumber(layerNumber)
{
	// allocate memory for count arrays and set default value to 0
	long long numKnotsInCurLayer = db.getNumberOfKnots(layerNumber);
	succCountArray.resize(numKnotsInCurLayer, 0);
	db.arrayInfos.addArray(layerNumber, database::arrayInfoStruct::arrayType::countArray, numKnotsInCurLayer * sizeof(countArrayVarType), 0);
}

//-----------------------------------------------------------------------------
// Name: ~successorCountArray()
// Desc: Delete successor count array and remove it from the database array information
//-----------------------------------------------------------------------------
miniMax::retroAnalysis::successorCountArray::~successorCountArray()
{
	if (!db.getNumberOfKnots(layerNumber)) return;
	db.arrayInfos.removeArray(layerNumber, database::arrayInfoStruct::arrayType::countArray, db.getNumberOfKnots(layerNumber) * sizeof(countArrayVarType), 0);
}

//-----------------------------------------------------------------------------
// Name: increaseCounter()
// Desc: Increase the successor counter of the state by one in a NON-thread safe way.
// 		 The counter is increased by one, if the state is a possible move for the current state.
//	 	 Returns the new counter value, after increasing. 
//-----------------------------------------------------------------------------
miniMax::countArrayVarType miniMax::retroAnalysis::successorCountArray::increaseCounter(stateNumberVarType stateNumber)
{
	if (stateNumber >= succCountArray.size()) {
		log.log(logger::logLevel::error, L"State number is out of range!");
		return COUNT_ARRAY_MAX_VALUE;
	}

	countArrayVarType& countValue = succCountArray[stateNumber];
	if (countValue == COUNT_ARRAY_MAX_VALUE) {
		log.log(logger::logLevel::error, L"maximum value for Count[] reached!");
		return COUNT_ARRAY_MAX_VALUE;
	} else {
		countValue++;
	}
	return countValue;
}

//-----------------------------------------------------------------------------
// Name: decreaseCounter()
// Desc: Decrease the successor counter of the state by one in a thread safe way. 
//       If the counter is already zero, the function will return COUNT_ARRAY_MAX_VALUE.
//       Returns the new counter value, after decreasing.
//-----------------------------------------------------------------------------
miniMax::countArrayVarType miniMax::retroAnalysis::successorCountArray::decreaseCounter(stateNumberVarType stateNumber)
{
	if (stateNumber >= succCountArray.size()) {
		log.log(logger::logLevel::error, L"State number is out of range!");
		return COUNT_ARRAY_MAX_VALUE;
	}

	std::lock_guard<std::mutex> lock(succCountArrayMutex);
	countArrayVarType& countValue = succCountArray[stateNumber];
	if (countValue > 0) {
		countValue--;
	} else {
		log.log(logger::logLevel::error, L"Count is already zero!");
		return COUNT_ARRAY_MAX_VALUE;
	}
	return countValue;
}
#pragma endregion

#pragma region addNumSuccedorsVars

// Use one Mutex for all threads to modify succCountArray
std::mutex miniMax::retroAnalysis::addNumSuccedorsVars::succCountArrayMutex = {};

//-----------------------------------------------------------------------------
// Name: addNumSuccedorsVars()
// Desc: Called by the master thread once for each thread.
//-----------------------------------------------------------------------------
inline miniMax::retroAnalysis::addNumSuccedorsVars::addNumSuccedorsVars(addNumSuccedorsVars const& master) : 
	scm(master.scm), layerNumber(master.layerNumber), statesProcessed(master.statesProcessed), mapLayerNumberToScaId(master.mapLayerNumberToScaId)
{
	bufferPredStates.clear();
	bufferPredStates.reserve(predStatesChunkSize);
}

//-----------------------------------------------------------------------------
// Name: addNumSuccedorsVars()
// Desc: Called by the master thread once.
//-----------------------------------------------------------------------------
inline miniMax::retroAnalysis::addNumSuccedorsVars::addNumSuccedorsVars(successorCountManager& scm, unsigned int layerNumber, long long& roughTotalNumStatesProcessed) : 
	scm(scm), layerNumber(layerNumber), statesProcessed(roughTotalNumStatesProcessed)
{
	// make mapping
	mapLayerNumberToScaId.resize(scm.db.getNumLayers(), -1);
	for (unsigned int id = 0; id < scm.succCountArrays.size(); id++) {
		mapLayerNumberToScaId[scm.succCountArrays[id]->getLayerNumber()] = id;
	}
}

//-----------------------------------------------------------------------------
// Name: storePredecessorState()
// Desc: Called by the worker threads to store the predecessor state in a file. 
//-----------------------------------------------------------------------------
bool miniMax::retroAnalysis::addNumSuccedorsVars::storePredecessorState(const stateAdressStruct& predState)
{
	// care only about layers for which the successor count array shall be calculated
	if (mapLayerNumberToScaId[predState.layerNumber] < 0) {
		return true;
	}

	// add this state to the buffer
	bufferPredStates.push_back(predState);

	// check if the buffer is full. if so, flush.
	if (bufferPredStates.size() >= predStatesChunkSize) {
		if (!flush()) {
			return scm.log.log(logger::logLevel::error, L"addNumSuccedorsVars::storePredecessorState(): flush() returned false!");
		}
	}
	return true;
}

//-----------------------------------------------------------------------------
// Name: flush()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMax::retroAnalysis::addNumSuccedorsVars::flush()
{
	// lock since increaseCounter() is not thread safe
	std::lock_guard<std::mutex> lock(succCountArrayMutex);

	for (auto& predState : bufferPredStates) {

		if (predState.layerNumber >= mapLayerNumberToScaId.size()) {
			return scm.log.log(logger::logLevel::error, L"addNumSuccedorsVars::flush(): Layer number is out of range!");
		}

		if (mapLayerNumberToScaId[predState.layerNumber] < 0) {
			return scm.log.log(logger::logLevel::error, L"addNumSuccedorsVars::flush(): Layer number is not in the list of layers to calculate!");
		}

		// add this state as possible move, for the preceding state
		if (scm.succCountArrays[mapLayerNumberToScaId[predState.layerNumber]]->increaseCounter(predState.stateNumber) == COUNT_ARRAY_MAX_VALUE) {
			return scm.log.log(logger::logLevel::error, L"addNumSuccedorsVars::storePredecessorState(): Counter is at maximum value!");
		}
	}
	bufferPredStates.clear();
	
	return true;
}

//-----------------------------------------------------------------------------
// Name: reduce()
// Desc: Called by the master thread once for each thread.
//-----------------------------------------------------------------------------
void miniMax::retroAnalysis::addNumSuccedorsVars::reduce()
{ 
	if (!flush()) {
		scm.log.log(logger::logLevel::error, L"addNumSuccedorsVars::reduce(): flush() returned false!");
		return;
	}
	scm.totalNumStatesProcessed += statesProcessed.getStatesProcessedByThisThread();
}
#pragma endregion

#pragma region successorCountFileStorage
//-----------------------------------------------------------------------------
// Name: successorCountFileStorage()
// Desc: Constructor for the successor count file storage
//-----------------------------------------------------------------------------
miniMax::retroAnalysis::successorCountFileStorage::successorCountFileStorage(logger& log, const wstring& fileDirectory, vector<layerInfoStruct>& layersToCalculate) :
	log(log), layersToCalculate(layersToCalculate)
{
	wstringstream ssCountArrayPath;
	ssCountArrayPath << fileDirectory << (fileDirectory.size()?"\\":"") << "countArray";
	CreateDirectory(ssCountArrayPath.str().c_str(), NULL);

	for (auto& layer : this->layersToCalculate) {
		wstringstream ssCountArrayFilePath;
		ssCountArrayFilePath	<< ssCountArrayPath.str() << "\\countArray" << layer.layerNumber << ".dat";
		layer.sCountArrayFilePath = ssCountArrayFilePath.str();
		if ((layer.hFileCountArray = CreateFile(layer.sCountArrayFilePath.c_str(),  GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE) {
			log << "ERROR: Could not open File " << layer.sCountArrayFilePath.c_str() << "!\n";
			return;
		}
	}
}

//-----------------------------------------------------------------------------
// Name: ~successorCountFileStorage()
// Desc: Destructor for the successor count file storage 
//-----------------------------------------------------------------------------
miniMax::retroAnalysis::successorCountFileStorage::~successorCountFileStorage()
{
	// close all file handles
	for (auto& layer : layersToCalculate) {
		if (layer.hFileCountArray != NULL) {
			CloseHandle(layer.hFileCountArray);
			layer.hFileCountArray = NULL;
		}
	}
}

//-----------------------------------------------------------------------------
// Name: write()
// Desc: Write the successor count arrays to file 
//-----------------------------------------------------------------------------
bool miniMax::retroAnalysis::successorCountFileStorage::write()
{
	// write data to file
	for (auto& layer : layersToCalculate) {
		DWORD dwWritten;
		if (!WriteFile(layer.hFileCountArray, layer.succCountArray->succCountArray.data(), layer.numKnotsInLayer * sizeof(countArrayVarType), &dwWritten, NULL)) {
			log << "ERROR: Could not write data to file " << layer.sCountArrayFilePath.c_str() << "!\n";
			return returnValues::falseOrStop();
		}

		// check if all data has been written
		if (dwWritten != layer.numKnotsInLayer * sizeof(countArrayVarType)) {
			log << "ERROR: Could not write all data to file " << layer.sCountArrayFilePath.c_str() << "!\n";
			return returnValues::falseOrStop();
		}
		log << "  Count array saved to file: " << layer.sCountArrayFilePath.c_str() << "\n";
	}
	return true;
}

//-----------------------------------------------------------------------------
// Name: read()
// Desc: Read the successor count arrays from file, if they exist
//-----------------------------------------------------------------------------
bool miniMax::retroAnalysis::successorCountFileStorage::read()
{
	// locals
	DWORD 			dwRead;

	// are all files existend? if one is missing, then return false
	for (auto& layer : layersToCalculate) {
		LARGE_INTEGER fileSize;
		if (layer.hFileCountArray == NULL || layer.hFileCountArray == INVALID_HANDLE_VALUE) {
			return false;
		}
		if (!GetFileSizeEx(layer.hFileCountArray, &fileSize) || fileSize.QuadPart != layer.numKnotsInLayer * sizeof(countArrayVarType)) {
			return false;
		}
	}
		
	// read data from file
	for (auto& layer : layersToCalculate) {
		log << "  Load number of succedors from file: " << layer.sCountArrayFilePath.c_str() << "\n";
		
		// read data from file
		if (!ReadFile(layer.hFileCountArray, layer.succCountArray->succCountArray.data(), layer.numKnotsInLayer * sizeof(countArrayVarType), &dwRead, NULL)) {
			log << "ERROR: Could not read data from file " << layer.sCountArrayFilePath.c_str() << "!\n";
			return returnValues::falseOrStop();
		}
	
		// check if all data was read
		if (dwRead != layer.numKnotsInLayer * sizeof(countArrayVarType)) {
			log << "ERROR: Could not read all data from file " << layer.sCountArrayFilePath.c_str() << "!\n";
			return returnValues::falseOrStop();
		}
	}	
	
	return true;
}
#pragma endregion
