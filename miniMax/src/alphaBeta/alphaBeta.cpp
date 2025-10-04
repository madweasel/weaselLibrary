/*********************************************************************
	miniMax_alphaBetaAlgorithmn.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/

#include "alphaBeta.h"

#pragma region solver
//-----------------------------------------------------------------------------
// Name: solver()
// Desc: Constructor 
//-----------------------------------------------------------------------------
miniMax::alphaBeta::solver::solver(logger& log, threadManagerClass& tm, database::database& db, gameInterface& game) :
	log(log), db(db), game(game), tm(tm), maxNumBranches(game.getMaxNumPossibilities())
{
}

//-----------------------------------------------------------------------------
// Name: getBestChoice()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMax::alphaBeta::solver::getBestChoice(unsigned int& choice, stateInfo& infoAboutChoices)
{
	// checks
	if (depthOfFullTree == 0) {
		return log.log(logger::logLevel::error, L"Depth of full tree is zero"), false;
	}
	if (maxNumBranches == 0) {
		return log.log(logger::logLevel::error, L"Max number of branches is zero"), false;
	}

	// Locals
	unsigned int 			layerNumber				= game.getLayerNumber(0);
	knotStruct				root;
	vector<knotStruct>		branches;
	runAlphaBetaVars tva1(*this, layerNumber, L"");

	// initialization
	calcDatabase			= false;

	// if database is not available, use min-max algorithmn without database
	if (!db.isOpen() && depthOfFullTree > 2) {

		// create one thread for each possibility
		vector<unsigned int>	possibilityIds;
		game.getPossibilities(0, possibilityIds);
		if (!possibilityIds.size()) {
			choice = 0;
			infoAboutChoices.choices.clear();
			infoAboutChoices.shortValue			= SKV_VALUE_INVALID;
			infoAboutChoices.plyInfo			= 0;
			infoAboutChoices.bestAmountOfPlies = 0;
			return true;
		}
		branches.resize(possibilityIds.size());
		root.playerToMoveChanged	= true;
		root.possibilityIds 		= possibilityIds;
		root.numPossibilities 		= possibilityIds.size();
		root.branches 				= branches.data();
		threadManagerClass::threadVarsArray<runAlphaBetaVars> tva(tm.getNumThreads(), runAlphaBetaVars(*this, layerNumber, L""));
		for (unsigned int i=0; i<tm.getNumThreads(); i++) {
			tva.item[i].rootKnot	= &root;
		}

		switch (tm.executeParallelLoop(minMaxThreadProc, tva.getPointerToArray(), tva.getSizeOfArray(), TM_SCHEDULE_STATIC, 0, possibilityIds.size()-1, 1))
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
		tva.reduce();

		// fill the information from the sub knots of each thread into the root knot
		vector<unsigned int> bestBranches;
		if (!root.calcKnotValue()) {
			return log.log(logger::logLevel::error, L"knot.calcKnotValue() failed"), returnValues::falseOrStop();
		}
		if (!root.calcPlyInfo()) {
			return log.log(logger::logLevel::error, L"knot.calcPlyInfo() failed"), returnValues::falseOrStop();
		}
		if (!root.getBestBranchesBasedOnFloatValue(bestBranches)) {
			return log.log(logger::logLevel::error, L"knot.getBestBranchesBasedOnFloatValue() failed"), returnValues::falseOrStop();
		}
		unsigned int bestBranch		= (bestBranches.size() ? bestBranches[rand() % bestBranches.size()] : 0);
		root.bestMoveId				= root.possibilityIds[bestBranch];

	// use database, with one single thread
	} else {
		tva1.curThreadNo = 0;
		if (!letTheTreeGrow(root, tva1, depthOfFullTree, FPKV_MIN_VALUE, FPKV_MAX_VALUE)) {
			return log.log(logger::logLevel::error, L"letTheTreeGrow() failed"), false;
		}
	}
	
	// calc information about choices
	choice					= root.bestMoveId;
	root.getInfoAboutChoices(infoAboutChoices);
	infoAboutChoices.updateBestAmountOfPlies();

	return true;
}

//-----------------------------------------------------------------------------
// Name: minMaxThreadProc()
// Desc: 
//-----------------------------------------------------------------------------
DWORD miniMax::alphaBeta::solver::minMaxThreadProc(void *pParameter, int64_t index)
{
	// check
	if (pParameter == NULL) return TM_RETURN_VALUE_TERMINATE_ALL_THREADS;

	// locals
	runAlphaBetaVars &			rabVars			= *((runAlphaBetaVars *) pParameter);
	solver &					abSolver		= rabVars.rSolver;
	logger &					log				= abSolver.log;
	gameInterface &				game			= abSolver.game;
	void *						pBackup;
	bool 						playerToMoveChanged;

	// perform move for this thread with the corresponding possibility
	game.move(rabVars.curThreadNo, rabVars.rootKnot->possibilityIds[index], playerToMoveChanged, pBackup);

	// calc value of considered possibility
	if (!abSolver.letTheTreeGrow(rabVars.rootKnot->branches[index], rabVars, abSolver.depthOfFullTree - 1, FPKV_MIN_VALUE, FPKV_MAX_VALUE)) {
		return log.log(logger::logLevel::error, L"letTheTreeGrow() failed"), TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
	}

	// undo move
	game.undo(rabVars.curThreadNo, rabVars.rootKnot->possibilityIds[index], playerToMoveChanged, pBackup);

	return TM_RETURN_VALUE_OK;
}

//-----------------------------------------------------------------------------
// Name: calcKnotValuesByAlphaBeta()
// Desc: return value is true if calculation is stopped either by user or by an error
//-----------------------------------------------------------------------------
bool miniMax::alphaBeta::solver::calcKnotValuesByAlphaBeta(vector<unsigned int>& layersToCalculate)
{
	// loop through all layers
	for (auto layerNumber : layersToCalculate) {

		// checks
		if (layerNumber >= game.getNumberOfLayers()) {
			return log.log(logger::logLevel::error, L"Layer number is out of range"), returnValues::falseOrStop();
		}
		if (!db.isOpen()) {
			return log.log(logger::logLevel::error, L"Database is not open. It must be open to store the calculated values."), returnValues::falseOrStop();
		}

		// skip if layer is already calculated
		if (db.isLayerCompleteAndInFile(layerNumber)) {
			return log << "  Layer " << layerNumber << " is already calculated" << "\n", true;
		}

		// enable database calculation
		log << "\n" << "*** Calculate layer " << layerNumber << " by alpha-beta-algorithmn ***" << "\n";
		curAction 		= activity::performAlphaBeta;
		calcDatabase 	= true;
		depthOfFullTree = game.getMaxNumPlies() + 1;
		
		// initialization
		if (!init(layerNumber)) { return false; }

		// preload succeeding layers
		db.setLoadingOfFullLayerOnRead();

		// run alpha-beta algorithmn
		if (!run(layerNumber)) { return false; }

		// show stats
		db.updateLayerStats(layerNumber);
		db.showLayerStats(layerNumber);
	}

	return true;
}

//-----------------------------------------------------------------------------
// Name: setSearchDepth()
// Desc: Sets the maximum search depth for the alpha-beta algorithm.
//-----------------------------------------------------------------------------
void miniMax::alphaBeta::solver::setSearchDepth(unsigned int maxAlphaBetaSearchDepth)
{
	depthOfFullTree = maxAlphaBetaSearchDepth; 
}

//-----------------------------------------------------------------------------
// Name: init()
// Desc: The function setSituation is called for each state to mark the invalid ones.
//-----------------------------------------------------------------------------
bool miniMax::alphaBeta::solver::init(unsigned int layerNumber)
{
	// skip if layer has no knots
	if (db.getNumberOfKnots(layerNumber) == 0) {
		log << "  Skip calculation of layer " << layerNumber << ", since it has no knots" << "\n";
		return true;
	}

	// locals
	wstringstream				ssInvArrayDirectory;
	wstringstream				ssInvArrayFilePath;
	wstring const  				fileDirectory		= 	db.getFileDirectory();

	// set current processed layer number
	log << "\n" << "*** Signing of invalid states for layer " << layerNumber << " (" << (game.getOutputInformation(layerNumber)) << ") which has " << db.getNumberOfKnots(layerNumber) << " knots ***" << "\n";

	// file names
	ssInvArrayDirectory.str(L"");  ssInvArrayDirectory << fileDirectory << (fileDirectory.size()?"\\":"") << "invalidStates";
	ssInvArrayFilePath .str(L"");  ssInvArrayFilePath  << fileDirectory << (fileDirectory.size()?"\\":"") << "invalidStates\\invalidStatesOfLayer" << layerNumber << ".dat";

	// does initialization file exist ?
	CreateDirectory(ssInvArrayDirectory.str().c_str(), NULL);
	
	// prepare parameters
	totalNumStatesProcessed 		= 0;
	roughTotalNumStatesProcessed 	= 0;
	threadManagerClass::threadVarsArray<initAlphaBetaVars> tva(tm.getNumThreads(), initAlphaBetaVars(*this, layerNumber, ssInvArrayFilePath.str()));
		
	// process each state in the current layer
	switch (tm.executeParallelLoop(initThreadProc, tva.getPointerToArray(), tva.getSizeOfArray(), TM_SCHEDULE_STATIC, 0, db.getNumberOfKnots(layerNumber) - 1, 1))
	{
	case TM_RETURN_VALUE_OK: 			
		break;
	case TM_RETURN_VALUE_EXECUTION_CANCELLED:
		log << "\n" << "****************************************\nMain thread: Execution cancelled by user!\n****************************************\n" << "\n";
		return false;
	default:
	case TM_RETURN_VALUE_INVALID_PARAM:
	case TM_RETURN_VALUE_UNEXPECTED_ERROR:
		log << "\n" << "****************************************\nMain thread: Invalid or unexpected param!\n****************************************\n" << "\n";
		return returnValues::falseOrStop();
	}

	// reduce and delete thread specific data
	tva.reduce();

	// check if all states have been processed
	if (totalNumStatesProcessed != db.getNumberOfKnots(layerNumber)) {
		return log.log(logger::logLevel::error, L"totalNumStatesProcessed != db.getNumberOfKnots(layerNumber)"), returnValues::falseOrStop();
	}
						
	// show statistics
	db.updateLayerStats(layerNumber);
	db.showLayerStats(layerNumber);

	return true;
}

//-----------------------------------------------------------------------------
// Name: initThreadProc()
// Desc: set short knot value to SKV_VALUE_INVALID, ply info to PLYINFO_VALUE_INVALID and knotAlreadyCalculated to true or false, whether setSituation() returns true or false
//-----------------------------------------------------------------------------
DWORD miniMax::alphaBeta::solver::initThreadProc(void* pParameter, int64_t index)
{
	// check
	if (pParameter == NULL) return TM_RETURN_VALUE_TERMINATE_ALL_THREADS;

	// locals
	initAlphaBetaVars &			iabVars			= *((initAlphaBetaVars *) pParameter);
	solver &					abSolver		= iabVars.rSolver;
	logger &					log				= abSolver.log;
	gameInterface &				game			= abSolver.game;
	database::database &		db				= abSolver.db;
	float		  				floatValue;						// dummy variable for calls of getValueOfSituation()
	stateAdressStruct			curState;						// current state counter for loops
	twoBit		  				curStateValue	= 0;			// for calls of getValueOfSituation()
	plyInfoVarType				plyInfo;						// depends on the curStateValue
	
	curState.layerNumber	= iabVars.layerNumber;
	curState.stateNumber	= index;

	// print status
	iabVars.statesProcessed.stateProcessed(log, db.getNumberOfKnots(curState.layerNumber), L"Already initialized ");

	// layer initialization already done ? if so, then read from file
	if (iabVars.loadFromFile) {
		if (!iabVars.readByte(curState.stateNumber * sizeof(twoBit), curStateValue)) {
			return log.log(logger::logLevel::error, L"initThreadProc::readBytes failed"), TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
		}
	
	// initialization not done
	} else {
		// set current selected situation
		if (!game.setSituation(iabVars.curThreadNo, curState.layerNumber, curState.stateNumber)) {
			curStateValue = SKV_VALUE_INVALID;
		} else {
			// get value of current situation
			game.getValueOfSituation(iabVars.curThreadNo, floatValue, curStateValue);
		}
	}

	// calc ply info
	if (curStateValue == SKV_VALUE_GAME_WON || curStateValue == SKV_VALUE_GAME_LOST) {
		plyInfo = 0;
	} else if (curStateValue == SKV_VALUE_INVALID) {
		plyInfo = PLYINFO_VALUE_INVALID;
	} else {
		plyInfo = PLYINFO_VALUE_UNCALCULATED;
	}

	// save short knot value & ply info
	if (!db.writeKnotValueInDatabase(curState.layerNumber, curState.stateNumber, curStateValue)) {
		return log.log(logger::logLevel::error, L"db.writeKnotValueInDatabase() failed"), TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
	}
	if (!db.writePlyInfoInDatabase  (curState.layerNumber, curState.stateNumber, plyInfo)) {
		return log.log(logger::logLevel::error, L"db.writePlyInfoInDatabase() failed"), TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
	}

	// write data to buffered file
	if (!iabVars.loadFromFile) {
		if (!iabVars.writeByte(curState.stateNumber * sizeof(twoBit), curStateValue)) {
			return log.log(logger::logLevel::error, L"initThreadProc writeBytes failed!"), TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
		}
	}

	return TM_RETURN_VALUE_OK;
}

//-----------------------------------------------------------------------------
// Name: run()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMax::alphaBeta::solver::run(unsigned int layerNumber)
{
	// skip if layer has no knots
	if (db.getNumberOfKnots(layerNumber) == 0) {
		log << "Skip calculation of layer " << layerNumber << ", since it has no knots" << "\n";
		return true;
	}

	// prepare parameters
	log << "\n" << "*** Calculate layer " << layerNumber << " with function letTheTreeGrow(): ***" << "\n";
	totalNumStatesProcessed 		= 0;
	roughTotalNumStatesProcessed 	= 0;
	threadManagerClass::threadVarsArray<runAlphaBetaVars> tva(tm.getNumThreads(), runAlphaBetaVars(*this, layerNumber, L""));

	// process each state in the current layer
	switch (tm.executeParallelLoop(runThreadProc, tva.getPointerToArray(), tva.getSizeOfArray(), TM_SCHEDULE_STATIC, 0, db.getNumberOfKnots(layerNumber) - 1, 1))
	{
	case TM_RETURN_VALUE_OK: 			
		break;
	case TM_RETURN_VALUE_EXECUTION_CANCELLED:
		log << "\n" << "****************************************\nMain thread: Execution cancelled by user!\n****************************************\n" << "\n";
		return false;
	default:
	case TM_RETURN_VALUE_INVALID_PARAM:
	case TM_RETURN_VALUE_UNEXPECTED_ERROR:
		return returnValues::falseOrStop();
	}

	// reduce and delete thread specific data
	tva.reduce();
	if (totalNumStatesProcessed != db.getNumberOfKnots(layerNumber)) {
		return log.log(logger::logLevel::error, L"totalNumStatesProcessed < db.getNumberOfKnots(layerNumber)"), returnValues::falseOrStop();
	}
						
	// show statistics
	db.updateLayerStats(layerNumber);
	db.showLayerStats(layerNumber);

	return true;
} 

//-----------------------------------------------------------------------------
// Name: runThreadProc()
// Desc: 
//-----------------------------------------------------------------------------
DWORD miniMax::alphaBeta::solver::runThreadProc(void* pParameter, int64_t index)
{
	// check
	if (pParameter == NULL) return TM_RETURN_VALUE_TERMINATE_ALL_THREADS;

	// locals
	runAlphaBetaVars &			rabVars			= *((runAlphaBetaVars *) pParameter);
	solver &					abSolver		= rabVars.rSolver;
	logger &					log				= abSolver.log;
	database::database &		db				= abSolver.db;
	gameInterface &				game			= abSolver.game;
	stateAdressStruct			curState;						// current state counter for loops
	knotStruct					root;							// root knot of the tree, which is calculated
	plyInfoVarType				plyInfo;						// for checking if knot value is already calculated or not
	
	curState.layerNumber	= rabVars.layerNumber;
	curState.stateNumber	= index;

	// print status
	rabVars.statesProcessed.stateProcessed(log, db.getNumberOfKnots(curState.layerNumber), L"  Processed ");

	// state already calculated? if so, leave.
	if (!db.readPlyInfoFromDatabase(curState.layerNumber, curState.stateNumber, plyInfo)) {
		return log.log(logger::logLevel::error, L"db.readPlyInfoFromDatabase() failed"), TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
	}
	if (plyInfo != PLYINFO_VALUE_UNCALCULATED) {
		return TM_RETURN_VALUE_OK;
	}

	// set current selected situation
	if (game.setSituation(rabVars.curThreadNo, curState.layerNumber, curState.stateNumber)) {

		// debug print
		if (log.getLevel() >= logger::logLevel::trace) {
			log << "Calc layer: " << curState.layerNumber << " state: " << curState.stateNumber << "\n";
			game.printField(rabVars.curThreadNo, SKV_VALUE_INVALID);
		}

		// calc value of situation
		if (!abSolver.letTheTreeGrow(root, rabVars, abSolver.depthOfFullTree, SKV_VALUE_GAME_LOST, SKV_VALUE_GAME_WON)) {
			return log.log(logger::logLevel::error, L"letTheTreeGrow() failed"), TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
		}

	} else {
		// should not occur, because already tested by plyInfo == PLYINFO_VALUE_UNCALCULATED
		return log.log(logger::logLevel::error, L"This event should never occur. if (!m->setSituation())"), TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
	}
	return TM_RETURN_VALUE_OK;
}

//-----------------------------------------------------------------------------
// Name: letTheTreeGrow()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMax::alphaBeta::solver::letTheTreeGrow(knotStruct& knot, runAlphaBetaVars& rabVars, unsigned int tilLevel, float alpha, float beta)
{
	// checks
	if (tilLevel > depthOfFullTree) {
		return log.log(logger::logLevel::error, L"tilLevel > depthOfFullTree"), returnValues::falseOrStop();
	}
	if (maxNumBranches == 0) {
		return log.log(logger::logLevel::error, L"Max number of branches is zero"), returnValues::falseOrStop();
	}

	// Locals
	unsigned int	layerNumber							= 0;		// layer number of current state
	unsigned int	stateNumber							= 0;		// state number of current state
	unsigned int	maxWonfreqValuesSubMoves			= 0;		// maximum number of freqValuesSubMoves[SKV_VALUE_GAME_WON]

	// evaluate situation, if last search depth level
	if (tilLevel == 0) {
		
		// If tilLevel is equal to zero while calculating the database, this indicates that the recursion has reached its maximum depth and exhausted available memory resources.
		// Each recursive step consumes memory, so hitting zero means the algorithm cannot continue deeper, which may result in incomplete calculations or failure to store all required states.
		// Consider increasing available memory or optimizing the recursion depth to avoid this situation during large database calculations.
		if (calcDatabase) {
			knot.setInvalid();
			return log.log(logger::logLevel::error, L"tilLevel == 0 while calculating database"), returnValues::falseOrStop();
		} else {
			game.getValueOfSituation(rabVars.curThreadNo, knot.floatValue, knot.shortValue);
		}

	// investigate branches
	} else {

		// standard values
		knot.initForCalculation(&rabVars.branchArray[(depthOfFullTree - tilLevel) * maxNumBranches]);

		// get layer and state number of current state and look if short knot value can be found in database or in an array
		if (tryDataBase(knot, rabVars, tilLevel, layerNumber, stateNumber)) return true;

		// get number of possiblities
		game.getPossibilities(rabVars.curThreadNo, knot.possibilityIds);
		knot.numPossibilities = (unsigned int) knot.possibilityIds.size();

		// debug print
		if (log.getLevel() >= logger::logLevel::trace) {
			log << wstring(2*(depthOfFullTree-tilLevel), L' ') << "Number of move possibilities: " << knot.numPossibilities << "\n";
		}

		// unable to move
		if (knot.numPossibilities == 0)  {
				
			// if unable to move a final state is reached
			game.getValueOfSituation(rabVars.curThreadNo, knot.floatValue, knot.shortValue);
			knot.plyInfo = (SKV_VALUE_INVALID == knot.shortValue) ? PLYINFO_VALUE_INVALID : 0;
			if (tilLevel == depthOfFullTree - 1) knot.freqValuesSubMoves[knot.shortValue]++;

			// if unable to move an invalid state was reached if nobody has won
			if (calcDatabase && game.lostIfUnableToMove(rabVars.curThreadNo) && knot.shortValue == SKV_VALUE_GAME_DRAWN) {
				knot.setInvalid();
			}

		// movement is possible
		} else {

			// move, letTreeGrow, undo
			if (!tryPossibilities(knot, rabVars, tilLevel, maxWonfreqValuesSubMoves, alpha, beta)) {
				return log.log(logger::logLevel::error, L"tryPossibilities() failed"), returnValues::falseOrStop();
			}

			// calculate value of knot - its the value of the best branch
			if (!knot.calcKnotValue()) {
				return log.log(logger::logLevel::error, L"knot.calcKnotValue() failed"), returnValues::falseOrStop();
			}

			// calc ply info
			if (!knot.calcPlyInfo()) {
				return log.log(logger::logLevel::error, L"knot.calcPlyInfo() failed"), returnValues::falseOrStop();
			}

			// select randomly one of the best moves, if they are equivalent
			if (tilLevel == depthOfFullTree && !calcDatabase) {
				vector<unsigned int> bestBranches;
				if (db.isOpen()) {
					if (!knot.getBestBranchesBasedOnSkvValue(bestBranches)) {
						return log.log(logger::logLevel::error, L"knot.getBestBranchesBasedOnSkvValue() failed"), returnValues::falseOrStop();
					}
				} else {
					if (!knot.getBestBranchesBasedOnFloatValue(bestBranches)) {
						return log.log(logger::logLevel::error, L"knot.getBestBranchesBasedOnFloatValue() failed"), returnValues::falseOrStop();
					}
				}
				unsigned int bestBranch		= (bestBranches.size() ? bestBranches[rand() % bestBranches.size()] : 0);
				knot.bestMoveId				= knot.possibilityIds[bestBranch];
			} else if (!calcDatabase) {
				knot.bestMoveId = (knot.possibilityIds.size() > 0) ? knot.possibilityIds[0] : 0;
			}
		}

		// debug print
		if (log.getLevel() >= logger::logLevel::trace) {
			log << wstring(2*(depthOfFullTree-tilLevel), L' ') << "Write value of current state to database: " << knot.plyInfo << " plies" << "\n";
			game.printField(rabVars.curThreadNo, knot.shortValue, 2*(depthOfFullTree-tilLevel));
		}

		// save value and best branch into database and set value as valid 
		if (calcDatabase && db.isOpen()) {
			if (!saveInDatabase(knot, rabVars, layerNumber, stateNumber)) {
				log << " layerNumber: " 			<< layerNumber 				<< " stateNumber: " << stateNumber << "\n";
				log << "tilLevel: " 				<< tilLevel 				<< "\n";
				log << "depthOfFullTree: " 			<< depthOfFullTree 			<< "\n";
				log << "knot.shortValue: " 			<< knot.shortValue 			<< "\n";
				log << "knot.plyInfo: " 			<< knot.plyInfo 			<< "\n";
				log << "knot.numPossibilities: " 	<< knot.numPossibilities 	<< "\n";
				return log.log(logger::logLevel::error, L"saveInDatabase() failed"), returnValues::falseOrStop();
			}
		}
	}
	return true;
}

//-----------------------------------------------------------------------------
// Name: tryDataBase()
// Desc: 
// 1 - Determines layerNumber and stateNumber for the given game situation.
// 2 - Look into database if knot value and ply info are already calculated. If so sets knot.shortValue, knot.floatValue and knot.plyInfo.
//-----------------------------------------------------------------------------
bool miniMax::alphaBeta::solver::tryDataBase(knotStruct& knot, const runAlphaBetaVars& rabVars, unsigned int tilLevel, unsigned int& layerNumber, unsigned int& stateNumber)
{
	// locals
	twoBit			shortKnotValue	= SKV_VALUE_INVALID;
	plyInfoVarType	plyInfo			= PLYINFO_VALUE_UNCALCULATED;
	unsigned int 	symOp;

	// use database ?
	if (db.isOpen() && (calcDatabase || db.isLayerCompleteAndInFile(rabVars.layerNumber))) {

		// lock mutex for database access
		std::lock_guard<std::mutex> lock(dbMutex);	

		// situation already existend in database ?
		game.getLayerAndStateNumber(rabVars.curThreadNo, layerNumber, stateNumber, symOp);
		bool layerInDatabaseAndCompleted = db.isLayerCompleteAndInFile(layerNumber);
		if (!db.readKnotValueFromDatabase(layerNumber, stateNumber, shortKnotValue)) {
			return log.log(logger::logLevel::error, L"db.readKnotValueFromDatabase() failed"), returnValues::falseOrStop();
		}
		if (!db.readPlyInfoFromDatabase(layerNumber, stateNumber, plyInfo)) {
			return log.log(logger::logLevel::error, L"db.readPlyInfoFromDatabase() failed"), returnValues::falseOrStop();
		}

		// debug print
		if (log.getLevel() >= logger::logLevel::trace) {
			log << wstring(2*(depthOfFullTree-tilLevel), L' ') << "Current state: " << layerNumber << " state: " << stateNumber << "\n";
			game.printField(rabVars.curThreadNo, shortKnotValue, 2*(depthOfFullTree-tilLevel));
		}
			   
		// it was possible to achieve an invalid state using move(), so the original state was an invalid one
		if ((tilLevel < depthOfFullTree && shortKnotValue == SKV_VALUE_INVALID && layerInDatabaseAndCompleted)
		||  (tilLevel < depthOfFullTree && shortKnotValue == SKV_VALUE_INVALID && (plyInfo != PLYINFO_VALUE_UNCALCULATED && plyInfo != PLYINFO_VALUE_INVALID))) {
			knot.setInvalid();
			return true;
		}

		// print output, if not calculating database, but requesting a knot value
		if (!calcDatabase && shortKnotValue != SKV_VALUE_INVALID && tilLevel == depthOfFullTree && layerInDatabaseAndCompleted) {
			log.log(logger::logLevel::trace, wstring(L"This state is marked as ") + ((shortKnotValue == SKV_VALUE_GAME_WON) ? L"WON" : ((shortKnotValue == SKV_VALUE_GAME_LOST) ? L"LOST" : ((shortKnotValue == SKV_VALUE_GAME_DRAWN) ? L"DRAW" : L"INVALID"))));
		}

		// when knot value is valid then return best branch
		if (( calcDatabase && tilLevel < depthOfFullTree     && shortKnotValue != SKV_VALUE_INVALID && plyInfo != PLYINFO_VALUE_UNCALCULATED)
		||  (!calcDatabase && tilLevel < depthOfFullTree - 1 && shortKnotValue != SKV_VALUE_INVALID)) {

			// switch if is not opponent level
			knot.shortValue = shortKnotValue;
			knot.plyInfo	= plyInfo;
			knot.floatValue = skvFloatValueMap[knot.shortValue];
			
			// debug print
			if (log.getLevel() >= logger::logLevel::trace) {
				log << wstring(2*(depthOfFullTree-tilLevel), L' ') << "Reading from database was SUCCESFUL" << "\n";
			}
			return true;
		}
	}

	// debug print
	if (log.getLevel() >= logger::logLevel::trace) {
		log << wstring(2*(depthOfFullTree-tilLevel), L' ') << "Reading from database FAILED" << "\n";
	}
	return false;
}

//-----------------------------------------------------------------------------
// Name: tryPossibilities()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMax::alphaBeta::solver::tryPossibilities(knotStruct& knot, runAlphaBetaVars& rabVars, unsigned int tilLevel, unsigned int &maxWonfreqValuesSubMoves, float &alpha, float &beta)
{
	// locals
	void *			pBackup;
	unsigned int	curPoss;

	for (curPoss=0; curPoss<knot.numPossibilities; curPoss++) {

		// debug output
		if (false && tilLevel == depthOfFullTree && !calcDatabase) {
			game.printMoveInformation(rabVars.curThreadNo, knot.possibilityIds[curPoss]);
		}

		// move
		game.move(rabVars.curThreadNo, knot.possibilityIds[curPoss], knot.branches[curPoss].playerToMoveChanged, pBackup);

		// debug print
		if (log.getLevel() >= logger::logLevel::trace) {
			log << wstring(2*(depthOfFullTree-tilLevel), L' ') << "Moved according to possiblity " << curPoss;
		}		

		// recursive call
		if (!letTheTreeGrow(knot.branches[curPoss], rabVars, tilLevel - 1, alpha, beta)) {
			return log.log(logger::logLevel::error, L"letTheTreeGrow() failed"), returnValues::falseOrStop();
		}

		// undo move
		game.undo(rabVars.curThreadNo, knot.possibilityIds[curPoss], knot.branches[curPoss].playerToMoveChanged, pBackup);
				
		// debug print
		if (log.getLevel() >= logger::logLevel::trace) {
			log << wstring(2*(depthOfFullTree-tilLevel), L' ') << "Last move was undone" << "\n";
		}		

		// output 
		if (tilLevel == depthOfFullTree && !calcDatabase) {
			if (knot.freqValuesSubMoves[SKV_VALUE_GAME_WON] > maxWonfreqValuesSubMoves && knot.branches[curPoss].shortValue == SKV_VALUE_GAME_DRAWN) {
				maxWonfreqValuesSubMoves = knot.freqValuesSubMoves[SKV_VALUE_GAME_WON];
			}
			if (db.isOpen()) { 
				knot.increaseFreqValuesSubMoves(curPoss);
			}
		} else if (tilLevel == depthOfFullTree - 1 && !calcDatabase) {
			knot.increaseFreqValuesSubMoves(curPoss);
		}

		// don't use cutting off if we are calculating the database, since we want to calculate all states
		if (db.isOpen() && calcDatabase)					continue;
		if (db.isOpen() && tilLevel + 1 >= depthOfFullTree)	continue;

		// check if we can spare the other possibilities according to alpha beta algorithmn
		if (knot.canCutOff(curPoss, alpha, beta)) break;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Name: saveInDatabase()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMax::alphaBeta::solver::saveInDatabase(const knotStruct& knot, runAlphaBetaVars& rabVars, unsigned int layerNumber, unsigned int stateNumber)
{
	// locals
	stateAdressStruct	symState;
	unsigned int		i;

	// invalid value?
	if (knot.shortValue > SKV_VALUE_GAME_WON) {
		return log.log(logger::logLevel::error, L"alphaBeta::solver::saveInDatabase(): knot.shortValue > SKV_VALUE_GAME_WON"), returnValues::falseOrStop();
	}

	// get numbers of symmetric states
	game.getSymStateNumWithDuplicates(rabVars.curThreadNo, rabVars.symStates);

	// lock mutex for database access
	std::lock_guard<std::mutex> lock(dbMutex);	

	// save
	if (!db.writeKnotValueInDatabase(layerNumber, stateNumber, knot.shortValue)) {
		return log.log(logger::logLevel::error, L"db.writeKnotValueInDatabase() failed"), returnValues::falseOrStop();
	}
	if (!db.writePlyInfoInDatabase  (layerNumber, stateNumber, knot.plyInfo)) {
		return log.log(logger::logLevel::error, L"db.writePlyInfoInDatabase() failed"), returnValues::falseOrStop();
	}

	// save value for all symmetric states
	for (i=0; i<rabVars.symStates.size(); i++) {

		// get state number
		symState = rabVars.symStates[i];

		// don't save original state twice
		if (symState.layerNumber == layerNumber && symState.stateNumber == stateNumber) continue;

		// don't save states of completed layers
		if (db.isLayerCompleteAndInFile(symState.layerNumber)) continue;

	    // save
		if (!db.writeKnotValueInDatabase(symState.layerNumber, symState.stateNumber, knot.shortValue)) {
			return log.log(logger::logLevel::error, L"db.writeKnotValueInDatabase() failed"), returnValues::falseOrStop();
		}
		if (!db.writePlyInfoInDatabase  (symState.layerNumber, symState.stateNumber, knot.plyInfo)) {
			return log.log(logger::logLevel::error, L"db.writePlyInfoInDatabase() failed"), returnValues::falseOrStop();
		}
	}

	return true;
}
#pragma endregion

#pragma region thread vars
//-----------------------------------------------------------------------------
// Name: initAlphaBetaVars()
// Desc: 
//-----------------------------------------------------------------------------
inline miniMax::alphaBeta::initAlphaBetaVars::initAlphaBetaVars(solver& rSolver, unsigned int layerNumber, const wstring& filepath) : 
	rSolver(rSolver),
	commonThreadVars(layerNumber, filepath, rSolver.db.getNumberOfKnots(layerNumber), rSolver.roughTotalNumStatesProcessed, rSolver.totalNumStatesProcessed, rSolver.log)
{
}

//-----------------------------------------------------------------------------
// Name: initAlphaBetaVars()
// Desc: 
//-----------------------------------------------------------------------------
miniMax::alphaBeta::initAlphaBetaVars::initAlphaBetaVars(initAlphaBetaVars const& master) : 
	rSolver(master.rSolver), commonThreadVars(master)
{
}

//-----------------------------------------------------------------------------
// Name: runAlphaBetaVars()
// Desc: 
//-----------------------------------------------------------------------------
miniMax::alphaBeta::runAlphaBetaVars::runAlphaBetaVars(solver& rSolver, unsigned int layerNumber, const wstring& filepath) : 
	rSolver(rSolver),
	commonThreadVars(layerNumber, filepath, rSolver.db.getNumberOfKnots(layerNumber), rSolver.roughTotalNumStatesProcessed, rSolver.totalNumStatesProcessed, rSolver.log)
{
	branchArray.resize(rSolver.maxNumBranches * rSolver.depthOfFullTree);
}

//-----------------------------------------------------------------------------
// Name: runAlphaBetaVars()
// Desc: 
//-----------------------------------------------------------------------------
miniMax::alphaBeta::runAlphaBetaVars::runAlphaBetaVars(runAlphaBetaVars const& master) : 
	commonThreadVars(master), symStates(master.symStates), rSolver(master.rSolver)
{
	branchArray.resize(master.rSolver.maxNumBranches * master.rSolver.depthOfFullTree); 
}
#pragma endregion
