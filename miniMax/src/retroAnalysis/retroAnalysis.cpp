/*********************************************************************
	miniMax_retroAnalysis.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/

#include "retroAnalysis.h"

#pragma region solver
//-----------------------------------------------------------------------------
// Name: solver()
// Desc: 
//-----------------------------------------------------------------------------
miniMax::retroAnalysis::solver::solver(logger& log, threadManagerClass& tm, database::database& db, gameInterface& game) : 
	log(log), db(db), game(game), tm(tm), scm(log, tm, db, game, statesToProcess)
{
}

//-----------------------------------------------------------------------------
// Name: ~solver()
// Desc:
//-----------------------------------------------------------------------------
miniMax::retroAnalysis::solver::~solver()
{
}

//-----------------------------------------------------------------------------
// Name: estimateTotalNumberOfKnots()
// Desc:
//-----------------------------------------------------------------------------
size_t  miniMax::retroAnalysis::solver::estimateTotalNumberOfKnots()
{
	vector<unsigned int> layersConsidered;
	size_t totalNumberOfKnots = 0;

	// consider all layers to calculate
	for (auto layerNumber : layersToCalculate) {

		// only consider layers which are not already considered
		if (std::find(layersConsidered.begin(), layersConsidered.end(), layerNumber) != layersConsidered.end()) continue;
		totalNumberOfKnots += db.getNumberOfKnots(layerNumber);
		layersConsidered.push_back(layerNumber);

		// consider all successor layers of the current layer
		for (auto& curSuccLayer : db.getSuccLayers(layerNumber)) {
			if (std::find(layersConsidered.begin(), layersConsidered.end(), curSuccLayer) != layersConsidered.end()) continue;
			totalNumberOfKnots += db.getNumberOfKnots(curSuccLayer);
			layersConsidered.push_back(curSuccLayer);
		}
	}
	return totalNumberOfKnots;
}

//-----------------------------------------------------------------------------
// Name: calcKnotValuesByRetroAnalysis()
// Desc: 
// The COUNT-ARRAY is the main element of the algorithmn. It contains the number of succeding states for the drawn gamestates,
// whose short knot value has to be determined. If all succeding states (branches representing possible moves) are for example won than,
// a state can be marked as lost, since no branch will lead to a drawn or won situation any more.
// Each time the short knot value of a game state has been determined, the state will be added to 'statesToProcess'.
// This list is like a queue of states, which still has to be processed.
//-----------------------------------------------------------------------------
bool miniMax::retroAnalysis::solver::calcKnotValuesByRetroAnalysis(vector<unsigned int>& layersToCalculate)
{
	// check if database is open
	if (!db.isOpen()) {
		log << "ERROR: Database is not open!\n";
		return returnValues::falseOrStop();
	}
	// check if there are layers to calculate
	if (layersToCalculate.size() == 0) {
		log << "ERROR: No layers to calculate!\n";
		return returnValues::falseOrStop();
	}
	// check if any layer to calculate has a bigger index than the number of layers in the database
	for (auto layerNumber : layersToCalculate) {
		if (layerNumber >= db.getNumLayers()) {
			log << "ERROR: Layer number " << layerNumber << " is out of range!\n";
			return returnValues::falseOrStop();
		}
	}

    // init
					this->layersToCalculate		= layersToCalculate;
	wstringstream	ssLayers;
	size_t 			totalNumberOfKnots			= estimateTotalNumberOfKnots();

	statesToProcess.clear();
	statesToProcess.reserve(tm.getNumThreads());
	for (unsigned int threadNo=0; threadNo<tm.getNumThreads(); threadNo++) {
		statesToProcess.push_back(stateQueue(log, db.getFileDirectory(), threadNo));

		// resize state queue for each thread
		for (unsigned int plyNumber = 0; plyNumber < PLYINFO_EXP_VALUE; plyNumber++) {
			statesToProcess[threadNo].resize(plyNumber, totalNumberOfKnots);
		}
	}
	layerInitialized.resize(db.getNumLayers(), false);
	
	// stdout
	for (auto layerNumber : layersToCalculate) ssLayers << " " << layerNumber;
	log << "==================================================================\n" 
		<< "=== Calculate layers" << ssLayers.str() << " by retro analysis ===\n" 
		<< "==================================================================\n";

	// initialization
	log << "Bytes in memory: " << db.getMemoryUsed() << "\n";
	if (!initRetroAnalysis()) { 
		log << "ERROR: Could not initialize retro analysis!\n";
		return returnValues::falseOrStop();
	 }
	
	// prepare count arrays
	log << "Bytes in memory: " << db.getMemoryUsed() << "\n";
	if (!prepareCountArrays()) { 
		log << "ERROR: Could not prepare count arrays!\n";
		return returnValues::falseOrStop();
	}

	// iteration
	log << "Bytes in memory: " << db.getMemoryUsed() << "\n";
	if (!performRetroAnalysis()) { 
		log << "ERROR: Could not perform retro analysis!\n";
		return returnValues::falseOrStop();
	 }

	// show output
	log << "Bytes in memory: " << db.getMemoryUsed() << "\n";
	for (auto layerNumber : layersToCalculate) { 
		db.updateLayerStats(layerNumber);
		db.showLayerStats(layerNumber);
	}
	log << "\n";

	// everything fine
	return true;
}

//-----------------------------------------------------------------------------
// Name: initRetroAnalysis()
// Desc: The state values for all game situations in the database are marked as invalid, as undecided, as won or as  lost by using the function getValueOfSituation().
//-----------------------------------------------------------------------------
bool miniMax::retroAnalysis::solver::initRetroAnalysis()
{
	// locals
	wstringstream				ssInitArrayPath;				// path of the working directory
	wstringstream				ssInitArrayFilePath;			// filename corresponding to a cyclic array file which is used for storage
	wstring const 				fileDirectory		= db.getFileDirectory();

	// process each layer
	for (auto layerNumber : layersToCalculate) { 

		// ensure that layer has any knots
		if (!db.getNumberOfKnots(layerNumber)) {
			log.log(logger::logLevel::debug, L"Skip initialization of layer " + std::to_wstring(layerNumber) + L" since it has no knots.");
			continue;
		}

		// set current processed layer number
		curAction = activity::initRetroAnalysis;
		log << "*****************************************\n" 
	    	<< "*** Initialization of layer " << layerNumber << " (" << (game.getOutputInformation(layerNumber)) << ") which has " << db.getNumberOfKnots(layerNumber) << " knots ***\n"
			<< "*****************************************\n";

		// file names
		ssInitArrayPath.str(L"");		ssInitArrayPath     << fileDirectory << (fileDirectory.size()?"\\":"") << "initLayer";
		ssInitArrayFilePath.str(L"");	ssInitArrayFilePath << fileDirectory << (fileDirectory.size()?"\\":"") << "initLayer\\initLayer" << layerNumber << ".dat";

		// does initialization file exist ?
		CreateDirectory(ssInitArrayPath.str().c_str(), NULL);

		// don't add layers twice
		if (layerInitialized[layerNumber]) {
			log.log(logger::logLevel::debug, L"Layer " + std::to_wstring(layerNumber) + L" already initialized.");
			continue;
		}
		layerInitialized[layerNumber] = true;
		
		// prepare parameters
		totalNumStatesProcessed 		= 0;
		roughTotalNumStatesProcessed 	= 0;
		threadManagerClass::threadVarsArray<initRetroAnalysisVars> tva(tm.getNumThreads(), initRetroAnalysisVars(*this, layerNumber, ssInitArrayFilePath.str()));
		
		// process each state in the current layer
		switch (tm.executeParallelLoop(initRetroAnalysisThreadProc, tva.getPointerToArray(), tva.getSizeOfArray(), TM_SCHEDULE_STATIC, 0, db.getNumberOfKnots(layerNumber) - 1, 1))
		{
		case TM_RETURN_VALUE_OK: 			
			break;
		case TM_RETURN_VALUE_EXECUTION_CANCELLED:
			log << "\n" << "****************************************\nMain thread: Execution cancelled by user!\n****************************************\n";
			return false;
		default:
		case TM_RETURN_VALUE_INVALID_PARAM:
		case TM_RETURN_VALUE_UNEXPECTED_ERROR:
			return returnValues::falseOrStop();
		}

		// reduce and delete thread specific data
		tva.reduce();

		// check if all states have been processed
		if (totalNumStatesProcessed != db.getNumberOfKnots(layerNumber)) {
			return log.log(logger::logLevel::error, L"Number of processed states is less than the number of knots in the layer!"), returnValues::falseOrStop();
		}

		// show statistics
		db.updateLayerStats(layerNumber);
		db.showLayerStats(layerNumber);
	}
	return true;
}

//-----------------------------------------------------------------------------
// Name: initRetroAnalysisParallelSub()
// Desc: 
//-----------------------------------------------------------------------------
DWORD miniMax::retroAnalysis::solver::initRetroAnalysisThreadProc(void* pParameter, int64_t index)
{
	// check parameter
	if (pParameter == NULL) return TM_RETURN_VALUE_TERMINATE_ALL_THREADS;

	// locals
	initRetroAnalysisVars &		iraVars		= *((initRetroAnalysisVars *) pParameter);
	solver&						retroVars	= iraVars.retroVars;
	logger&						log			= retroVars.log;
	gameInterface&				game		= retroVars.game;
	database::database&			db			= retroVars.db;
	float		  				floatValue;						// dummy variable for calls of getValueOfSituation()
	stateAdressStruct			curState;						// current state counter for loops
	twoBit		  				curStateValue;					// for calls of getValueOfSituation()
	
	curState.layerNumber	= iraVars.layerNumber;
	curState.stateNumber	= index;

	// print status
	iraVars.statesProcessed.stateProcessed(log, db.getNumberOfKnots(curState.layerNumber), L"Already initialized ");

	// layer initialization already done ? if so, then read from buffered file
	if (iraVars.loadFromFile) {
		if (!iraVars.readByte(curState.stateNumber * sizeof(twoBit), curStateValue)) {
			return log.log(logger::logLevel::error, L"initRetroAnalysisVars::readBytes() failed"), TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
		}

	// initialization not done
	} else {

		// set current selected situation
		if (!game.setSituation(iraVars.curThreadNo, curState.layerNumber, curState.stateNumber)) {
			curStateValue = SKV_VALUE_INVALID;
		} else {
			// get value of current situation
			game.getValueOfSituation(iraVars.curThreadNo, floatValue, curStateValue);
		}
	}

	// save init value
	if (curStateValue != SKV_VALUE_INVALID) {

		// save short knot value
		if (!db.writeKnotValueInDatabase(curState.layerNumber, curState.stateNumber, curStateValue)) {
			return log.log(logger::logLevel::error, L"writeKnotValueInDatabase() returned false!"), TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
		}

		// put in queue if state is final
		if (curStateValue == SKV_VALUE_GAME_WON || curStateValue == SKV_VALUE_GAME_LOST) {

			// ply info
			if (!db.writePlyInfoInDatabase(curState.layerNumber, curState.stateNumber, 0)) {
				return log.log(logger::logLevel::error, L"writePlyInfoInDatabase() returned false!"), TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
			}
		}
	}

	// write data to buffered file
	if (!iraVars.loadFromFile) {
		if (!iraVars.writeByte(curState.stateNumber * sizeof(twoBit), curStateValue)) {
			log << "ERROR: initRetroAnalysisVars::writeBytes() failed!" << "\n";
			return TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
		}
	}

	return TM_RETURN_VALUE_OK;
}

//-----------------------------------------------------------------------------
// Name: prepareCountArrays()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMax::retroAnalysis::solver::prepareCountArrays()
{
	if (!scm.init(layersToCalculate)) {
		log << "ERROR: Could not initialize successor count arrays!\n";
		return returnValues::falseOrStop();
	}
	return true;
}

//-----------------------------------------------------------------------------
// Name: performRetroAnalysis()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMax::retroAnalysis::solver::performRetroAnalysis()
{
	// checks
	if (layersToCalculate.size() == 0) {
		log << "ERROR: No layers to calculate!\n";
		return returnValues::falseOrStop();
	}
	if (statesToProcess.size() != tm.getNumThreads()) {
		log << "ERROR: Number of threads and number of state queues do not match!\n";
		return returnValues::falseOrStop();
	}
	if (!scm.isReady()) {
		log << "ERROR: Number of layers to calculate and number of count arrays do not match!\n";
		return returnValues::falseOrStop();
	}
	if (db.getNumLayers() != layerInitialized.size()) {
		log << "ERROR: Number of layers and number of layer initialized flags do not match!\n";
		return returnValues::falseOrStop();
	}

	// locals
	stateAdressStruct		curState;									// current state counter for loops
	twoBit		  			curStateValue;								// current state value

	log << "******************************************\n" 
	    << "*** Begin Iteration for Retro Analysis ***\n"
		<< "******************************************\n";
	totalNumStatesProcessed = 0;
	curAction = activity::performRetroAnalysis;
	
	db.setLoadingOfFullLayerOnRead();

	// process each state in the current layer
	switch (tm.executeInParallel(performRetroAnalysisThreadProc, (void**) this, 0)) 
	{
	case TM_RETURN_VALUE_OK: 			
		break;
	case TM_RETURN_VALUE_EXECUTION_CANCELLED:
		log << "\n" << "****************************************\nMain thread: Execution cancelled by user!\n****************************************\n";
		return false;
	default:
	case TM_RETURN_VALUE_INVALID_PARAM:
	case TM_RETURN_VALUE_UNEXPECTED_ERROR:
		return returnValues::falseOrStop();
	}
	
	// if there are still states to process, than something went wrong
	for (auto& queue : statesToProcess) {
		if (queue.getNumStatesToProcess()) {
			log << "ERROR: There are still states to process after performing retro analysis!" << "\n";
			return returnValues::falseOrStop();
		}
	}
	
	// copy drawn and invalid states to ply info
	log << "    Copy drawn and invalid states to ply info database..." << "\n";
	for (auto layerNumber : layersToCalculate) { 
		for (curState.layerNumber=layerNumber, curState.stateNumber=0; curState.stateNumber < db.getNumberOfKnots(curState.layerNumber); curState.stateNumber++) {
			// get value of current situation
			if (!db.readKnotValueFromDatabase(curState.layerNumber, curState.stateNumber, curStateValue)) {
				log.log(logger::logLevel::error, L"readKnotValueFromDatabase() returned false!");
				return returnValues::falseOrStop();
			}
			// store ply info for drawn and invalid states
			if (curStateValue == SKV_VALUE_GAME_DRAWN) {
				vector<unsigned int> possibilityIds;
				game.setSituation(0, curState.layerNumber, curState.stateNumber);
				game.getPossibilities(0, possibilityIds);
				plyInfoVarType curPlyValue = (possibilityIds.size() > 0 ? PLYINFO_VALUE_DRAWN : 0);
				if (!db.writePlyInfoInDatabase(curState.layerNumber, curState.stateNumber, curPlyValue)) {
					log.log(logger::logLevel::error, L"writePlyInfoInDatabase() returned false!");
					return returnValues::falseOrStop();
				}
			}
			if (curStateValue == SKV_VALUE_INVALID) {
				if (!db.writePlyInfoInDatabase(curState.layerNumber, curState.stateNumber, PLYINFO_VALUE_INVALID)) {
					log.log(logger::logLevel::error, L"writePlyInfoInDatabase() returned false!");
					return returnValues::falseOrStop();
				}
			}  
		}
	}
	log << "\n" << "*** Iteration finished! ***" << "\n";
	
	// every thing ok
	return true;
}

//-----------------------------------------------------------------------------
// Name: performRetroAnalysisThreadProc()
// Desc: 
//-----------------------------------------------------------------------------
DWORD miniMax::retroAnalysis::solver::performRetroAnalysisThreadProc(void* pParameter)
{
	// check parameter
	if (pParameter == NULL) return TM_RETURN_VALUE_INVALID_PARAM;

	// locals
	solver &					retroVars					= *((solver*) pParameter);
	logger &					log							= retroVars.log;
	gameInterface &				game						= retroVars.game;
	threadManagerClass &		tm							= retroVars.tm;
	unsigned int				threadNo					= tm.getThreadNumber();
	vector<predVars> 			predVars					{MAX_NUM_PREDECESSORS};

	// checks
	if (threadNo >= retroVars.statesToProcess.size()) {
		log.log(logger::logLevel::error, L"Thread number is out of range! ");
		return TM_RETURN_VALUE_INVALID_PARAM;
	}

	// more locals
	stateQueue&					queue						= retroVars.statesToProcess[threadNo];
	long long					numStatesProcessed			= 0;			// number of states already processed by this thread
	plyInfoVarType				curNumPlies					= 0;			// current number of plies considered
	stateAdressStruct			curState;									// current state counter for while-loop

	// iterate through all states in the queue, ply by ply
	// IMPORTANT: All threads must process all plies, since the barrier below expects all threads
	for (curNumPlies=0; curNumPlies<PLYINFO_EXP_VALUE;curNumPlies++) {
		
		// IMPORTANT: Since the barrier below expects all threads we cannot skip here
		// if (!queue.size(curNumPlies)) continue;
		
		// process all states in the queue
		while (queue.pop_front(curState, curNumPlies)) {

			// execution cancelled by user?
			if (tm.wasExecutionCancelled()) {
				log << "\n" << "****************************************\nSub-thread no. " << threadNo << ": Execution cancelled by user!\n****************************************\n";
				return TM_RETURN_VALUE_EXECUTION_CANCELLED;
			}
			
			// console output
			if (numStatesProcessed % OUTPUT_EVERY_N_STATES == 0) {
				wstringstream ss;
				ss << "    Current number of plies: " << (unsigned int) curNumPlies << "/" << queue.getMaxPlyInfoValue()
				   << "      States to process for thread " << threadNo << ": " << queue.getNumStatesToProcess();
				log.log(logger::logLevel::info, ss.str());
			}
			numStatesProcessed++;

			// set current selected situation
			if (!game.setSituation(threadNo, curState.layerNumber, curState.stateNumber)) {
				log.log(logger::logLevel::error, L"No database file open!");
				return TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
			}

			// DEBUGGING
			// if (curState.layerNumber == 65 && curState.stateNumber == 17961264) {
			// 	game.printField(threadNo, SKV_VALUE_INVALID, 0);
			// }

			// get list with statenumbers of predecessors
			predVars.clear();
			game.getPredecessors(threadNo, predVars);

			// iteration
			for (auto& predState : predVars) {
				if (!retroVars.processPredecessor(queue, curState, predState)) {
					log.log(logger::logLevel::error, L"processPredecessor() returned false!");
					log.log(logger::logLevel::error, L"Thread no. " + std::to_wstring(threadNo) + L" Layer: " + std::to_wstring(curState.layerNumber) + L" State: " + std::to_wstring(curState.stateNumber));
					return TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
				}
			}
		}

		// there might be other threads still processing states with this ply number
		tm.waitForOtherThreads();
	}

	// every thing ok
	return TM_RETURN_VALUE_OK;
}

//-----------------------------------------------------------------------------
// Name: processPredecessor()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMax::retroAnalysis::solver::processPredecessor(stateQueue& queue, const stateAdressStruct& curState, const predVars& predVarState)
{
	// locals
	twoBit 						curStateValue;				// value of the current state
	plyInfoVarType 				curNumPlies;				// number of plies of the current considered state
	stateAdressStruct			predState;					// predecessor state
	twoBit						predStateValue;				// value of the predecessor state
	plyInfoVarType				numPliesTillPredState;		// number of plies of the current considered predecessor state
	static std::mutex 			dbMutex;					// mutex for database access

	// current predecessor
	predState.layerNumber = predVarState.predLayerNumber;
	predState.stateNumber = predVarState.predStateNumber;

	// DEBUGGING
	bool v = false;
	// if (predState.layerNumber == 65 && predState.stateNumber == 17961264) {
	// 	v = true;
	// 	if (v) log.log(logger::logLevel::info, L"---------------------");
	// }

	// only states from a layer which is to be calculated are relevant
	if (predState.layerNumber >= db.getNumLayers() || std::find(layersToCalculate.begin(), layersToCalculate.end(), predState.layerNumber) == layersToCalculate.end()) {
		if (v) log.log(logger::logLevel::info, L"Skipping irrelevant state: Layer " + std::to_wstring(predState.layerNumber) + L", State " + std::to_wstring(predState.stateNumber));
		return true;
	}

	// mutex for db
	std::lock_guard<std::mutex> lock(dbMutex);

	// get value of predecessor
	if (!db.readKnotValueFromDatabase(predState.layerNumber, predState.stateNumber, predStateValue)) {
		log.log(logger::logLevel::error, L"readKnotValueFromDatabase() returned false!");
		return false;
	}

	// only drawn states are relevant here, since the others are already calculated
	if (predStateValue == SKV_VALUE_GAME_DRAWN) {

		// get value and plyInfo of current state
		if (!db.readKnotValueFromDatabase(curState.layerNumber, curState.stateNumber, curStateValue)) {
			log.log(logger::logLevel::error, L"readKnotValueFromDatabase() returned false!");
			return TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
		}
		if (!db.readPlyInfoFromDatabase  (curState.layerNumber, curState.stateNumber, curNumPlies)) {
			log.log(logger::logLevel::error, L"readPlyInfoFromDatabase() returned false!");
			return TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
		}

		if (v) log.log(logger::logLevel::info, L"ThreadId " + std::to_wstring(tm.getThreadNumber()) + L": Current state is drawn: Layer " + std::to_wstring(curState.layerNumber) + L", State " + std::to_wstring(curState.stateNumber));
		if (v) log.log(logger::logLevel::info, L"ThreadId " + std::to_wstring(tm.getThreadNumber()) + L": Current state value: " + std::to_wstring(curStateValue));
		if (v) log.log(logger::logLevel::info, L"ThreadId " + std::to_wstring(tm.getThreadNumber()) + L": Current number of plies: " + std::to_wstring(curNumPlies));
		if (v) game.printField(tm.getThreadNumber(), curStateValue, 0);

		// if current considered state is a lost game then all predecessors are a won game, if the player to move has changed
		if (curStateValue == skvPerspectiveMatrix[SKV_VALUE_GAME_WON][predVarState.playerToMoveChanged ? PL_TO_MOVE_CHANGED : PL_TO_MOVE_UNCHANGED]) {
			if (!db.writeKnotValueInDatabase(predState.layerNumber, predState.stateNumber, SKV_VALUE_GAME_WON)) {
				log.log(logger::logLevel::error, L"writeKnotValueInDatabase() returned false!");
				return false;
			}
			if (!db.writePlyInfoInDatabase  (predState.layerNumber, predState.stateNumber, curNumPlies + 1)) {
				log.log(logger::logLevel::error, L"writePlyInfoInDatabase() returned false!");
				return false;
			}
			// add state to queue
			if (!queue.push_back(predState, curNumPlies + 1, db.getNumberOfKnots(predState.layerNumber))) {
				log.log(logger::logLevel::error, L"push_back() returned false!");
				return false;
			}
			if (v) log.log(logger::logLevel::info, L"ThreadId " + std::to_wstring(tm.getThreadNumber()) + L": Current state is a won game with " + std::to_wstring(curNumPlies + 1) + L" plies.");
		// if current state is a won game, then this state is not an option any more for all predecessors
		} else {
			// reduce count value by one
			countArrayVarType countValue = scm.getAndDecreaseCounter(predState.layerNumber, predState.stateNumber);
			if (countValue == COUNT_ARRAY_MAX_VALUE) {
				log.log(logger::logLevel::error, L"Counter is at minimum value!");
				log.log(logger::logLevel::error, L"Layer: " + std::to_wstring(predState.layerNumber) + L" State: " + std::to_wstring(predState.stateNumber));
				log.log(logger::logLevel::error, L"Count value: " + std::to_wstring(countValue));
				return false;
			}
			if (v) log.log(logger::logLevel::info, L"ThreadId " + std::to_wstring(tm.getThreadNumber()) + L": Reduced count value of predecessor to " + std::to_wstring(countValue));

			// ply info
			if (!db.readPlyInfoFromDatabase(predState.layerNumber, predState.stateNumber, numPliesTillPredState)) {
				log.log(logger::logLevel::error, L"readPlyInfoFromDatabase() returned false!");
				return false;
			}
			if (v) log.log(logger::logLevel::info, L"ThreadId " + std::to_wstring(tm.getThreadNumber()) + L": Current number of plies to pred state: " + std::to_wstring(numPliesTillPredState));
			// write ply info, if not already done
			if (numPliesTillPredState == PLYINFO_VALUE_UNCALCULATED || curNumPlies + 1 > numPliesTillPredState) {
				if (!db.writePlyInfoInDatabase(predState.layerNumber, predState.stateNumber, curNumPlies + 1)) {
					log.log(logger::logLevel::error, L"writePlyInfoInDatabase() returned false!");
					return false;
				}
				if (v) log.log(logger::logLevel::info, L"ThreadId " + std::to_wstring(tm.getThreadNumber()) + L": Updated ply info for predecessor to " + std::to_wstring(curNumPlies + 1));
			}
				
			// when all successor are won states then this is a lost state (this should only be the case for one thread)
			if (countValue == 0) {
				if (!db.writeKnotValueInDatabase(predState.layerNumber, predState.stateNumber, SKV_VALUE_GAME_LOST)) {
					log.log(logger::logLevel::error, L"writeKnotValueInDatabase() returned false!");
					return false;
				}
				if (!queue.push_back(predState, curNumPlies + 1, db.getNumberOfKnots(predState.layerNumber))) {
					log.log(logger::logLevel::error, L"push_back() returned false!");
					return false;
				}
				if (v) log.log(logger::logLevel::info, L"ThreadId " + std::to_wstring(tm.getThreadNumber()) + L": Current state is a lost game with " + std::to_wstring(curNumPlies + 1) + L" plies.");
			}
		} 
	}

	// everything fine
	return true;
}

#pragma endregion

#pragma region retro analysis thread structs
//-----------------------------------------------------------------------------
// Name: initRetroAnalysisVars()
// Desc: Copy constructor for initRetroAnalysisVars, duplicates thread-specific variables from master.
//-----------------------------------------------------------------------------
miniMax::retroAnalysis::initRetroAnalysisVars::initRetroAnalysisVars(initRetroAnalysisVars const& master)
	: retroVars(master.retroVars),
	commonThreadVars(master)
{
}

//-----------------------------------------------------------------------------
// Name: initRetroAnalysisVars()
// Desc: Initializes thread-specific variables for retro analysis, including references to solver, layer number, and file path.
//-----------------------------------------------------------------------------
inline miniMax::retroAnalysis::initRetroAnalysisVars::initRetroAnalysisVars(solver& retroVars, unsigned int layerNumber, const wstring& filepath) : 
	retroVars(retroVars), 
	commonThreadVars(layerNumber, filepath, retroVars.db.getNumberOfKnots(layerNumber), retroVars.roughTotalNumStatesProcessed, retroVars.totalNumStatesProcessed, retroVars.log)
{
}
#pragma endregion
