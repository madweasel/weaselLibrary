/*********************************************************************
	integrityChecker.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/

#include "integrityChecker.h"

#pragma region helper functions
//-----------------------------------------------------------------------------
// Name: getIncrement()
// Desc: Returns the number of states to skip during a test, depending on 'maxNumStatesToTest' setting.
//       If it is not a full test then at maximum 'maxNumStatesToTest' states are tested.
//-----------------------------------------------------------------------------
unsigned int miniMax::integrity::checker::getIncrement(unsigned int layerNumber)
{
	unsigned int numKnots = db.getNumberOfKnots(layerNumber);
	if (maxNumStatesToTest == 0) return 1;
	if (numKnots < maxNumStatesToTest) return 1;
	return numKnots / maxNumStatesToTest;
}

//-----------------------------------------------------------------------------
// Name: checkerThreadVars()
// Desc: 
//-----------------------------------------------------------------------------
miniMax::integrity::checkerThreadVars::checkerThreadVars(checkerThreadVars const &master)
	: rChecker(master.rChecker)
	, statesProcessed(master.statesProcessed)
	, layerNumber(master.layerNumber)
	, totalNumStatesProcessed(master.totalNumStatesProcessed)
{
	subValueInDatabase		= master.subValueInDatabase;
	subPlyInfos         	= master.subPlyInfos;
	hasCurPlayerChanged 	= master.hasCurPlayerChanged;
}

//-----------------------------------------------------------------------------
// Name: checkerThreadVars()
// Desc: 
//-----------------------------------------------------------------------------
miniMax::integrity::checkerThreadVars::checkerThreadVars(checker &parent, unsigned int layerNumber, unsigned int maxNumBranches, long long &roughTotalNumStatesProcessed) : 
	rChecker(parent), layerNumber(layerNumber), statesProcessed(roughTotalNumStatesProcessed), totalNumStatesProcessed(parent.numStatesProcessed)
{
	subValueInDatabase		.resize(maxNumBranches);
	subPlyInfos         	.resize(maxNumBranches);
	hasCurPlayerChanged 	.resize(maxNumBranches);
}

//-----------------------------------------------------------------------------
// Name: reduce()
// Desc: 
//-----------------------------------------------------------------------------
void miniMax::integrity::checkerThreadVars::reduce()
{
	totalNumStatesProcessed += this->statesProcessed.getStatesProcessedByThisThread();
}

//-----------------------------------------------------------------------------
// Name: tester()
// Desc: 
//-----------------------------------------------------------------------------
miniMax::integrity::checker::checker(logger& log, threadManagerClass& tm, database::database& db, gameInterface& game) :
	log(log), tm(tm), db(db), game(game), maxNumBranches(game.getMaxNumPossibilities())
{
}

//-----------------------------------------------------------------------------
// Name: startTestThreads()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMax::integrity::checker::startTestThreads(unsigned int layerNumber, DWORD threadProc(void* pParameter, int64_t index))
{
	// Locals
	unsigned int	curThreadNo;
	unsigned int	returnValue;
	unsigned int 	increment;
	unsigned int	numKnotsInLayer = game.getNumberOfKnotsInLayer(layerNumber);

	// output
	log << "\n" << "*** Test each state in layer: " << layerNumber << " ***" << "\n";
    log << (game.getOutputInformation(layerNumber)) << "\n";

	// if nothing to test, quit
	if (numKnotsInLayer == 0) return true;

	// For debugging
	// tm.setNumThreads(1);

	game.prepareCalculation();

	// prepare thread specific data
	curAction						= activity::testingLayer;
	numStatesProcessed 				= 0;
	roughTotalNumStatesProcessed 	= 0;
	threadManagerClass::threadVarsArray<checkerThreadVars> tva(tm.getNumThreads(), checkerThreadVars(*this, layerNumber, maxNumBranches, roughTotalNumStatesProcessed));

	// Get increment and ensure it is at least 1 to avoid division by zero
	increment = getIncrement(layerNumber);
	if (increment == 0) increment = 1; 
	// process each state in the current layer
	returnValue = tm.executeParallelLoop(threadProc, tva.getPointerToArray(), tva.getSizeOfArray(), TM_SCHEDULE_STATIC, 0, numKnotsInLayer - 1, increment);		
	switch (returnValue)
	{
	case TM_RETURN_VALUE_OK: 			
	case TM_RETURN_VALUE_EXECUTION_CANCELLED:
		if (returnValue == TM_RETURN_VALUE_EXECUTION_CANCELLED) {
			log << "Main thread: Execution cancelled by user" << "\n";
			return false;	// ... better would be to return a cancel-specific value
		} else {
			break;
		}
	default:
	case TM_RETURN_VALUE_INVALID_PARAM:
		log << "Main thread: TM_RETURN_VALUE_INVALID_PARAM. Execution stopped." << "\n";
		return returnValues::falseOrStop();
	case TM_RETURN_VALUE_UNEXPECTED_ERROR:
		log << "Main thread: Unexpected error. Execution stopped." << "\n";
		return returnValues::falseOrStop();
	}
	
	// calculate the number of states processed
	tva.reduce();
	unsigned int numStatesToProcess = (numKnotsInLayer-1) / increment + 1;

	// layer is not ok
	if (numStatesProcessed < numStatesToProcess) {
		log << "DATABASE ERROR IN LAYER " << layerNumber << "\n";
		log << "Number of states processed: " << numStatesProcessed << "\n";
		log << "Number of states to process: " << numStatesToProcess << "\n";
		return returnValues::falseOrStop();
	// layer is ok
	} else {
		log << " TEST PASSED !" << "\n" << "\n";
		return true;
	}
}
#pragma endregion

#pragma region testing states in the database
//-----------------------------------------------------------------------------
// Name: testLayer()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMax::integrity::checker::testLayer(unsigned int layerNumber)
{
	// database must be open
	if (!db.isOpen()) {
		log << "ERROR: Database file not open!" << "\n";
		return returnValues::falseOrStop();
	}

	// do not read state by state from file, but load the full layer into memory once
	if (maxNumStatesToTest > loadFullLayerThreshold) {
		db.setLoadingOfFullLayerOnRead();
	}

	// run test
	return startTestThreads(layerNumber, testLayerThreadProc);
}

//-----------------------------------------------------------------------------
// Name: testLayerThreadProc()
// Desc: 
//-----------------------------------------------------------------------------
DWORD miniMax::integrity::checker::testLayerThreadProc(void* pParameter, int64_t index)
{
	// locals
	checkerThreadVars &			tlVars				= *((checkerThreadVars*) pParameter);
	checker &					c					= tlVars.rChecker;
	database::database &		db					= c.db;
	gameInterface &				game				= c.game;
	logger &					log					= c.log;
	unsigned int				layerNumber			= tlVars.layerNumber;
	unsigned int				stateNumber			= index;
	unsigned int				threadNo			= tlVars.curThreadNo;
	auto&						subValueInDatabase	= tlVars.subValueInDatabase;
	auto&						subPlyInfos			= tlVars.subPlyInfos;
	vector<bool> &				hasCurPlayerChanged	= tlVars.hasCurPlayerChanged;
	vector<unsigned int> &		possibilityIds		= tlVars.possibilityIds;
	twoBit						shortValueInDatabase;
	twoBit						shortValueInGame;
    plyInfoVarType				numPliesTillCurState;
	plyInfoVarType				min, max;
	float						floatValueInGame;
	unsigned int				numPossibilities;
	unsigned int				i, j;
	unsigned int				tmpStateNumber, tmpLayerNumber;
	unsigned int 				symOp;
	void *						pBackup;
	bool						invalidLayerOrStateNumber;
	bool						layerInDatabaseAndCompleted;
	bool						playerHasChanged;

	// output
	tlVars.statesProcessed.stateProcessed(log, db.getNumberOfKnots(layerNumber), L"Tested ");

	// situation already existend in database ?
	db.readKnotValueFromDatabase(layerNumber, stateNumber, shortValueInDatabase);
    db.readPlyInfoFromDatabase  (layerNumber, stateNumber, numPliesTillCurState);

	// prepare the situation
	if (!game.setSituation(threadNo, layerNumber, stateNumber)) {
			
		// when situation cannot be constructed then state must be marked as invalid in database
		if (shortValueInDatabase != SKV_VALUE_INVALID || numPliesTillCurState != PLYINFO_VALUE_INVALID) { 
			log << "ERROR: DATABASE ERROR IN LAYER " << layerNumber << " AND STATE " << stateNumber << ": Could not set situation, but value is not invalid." << "\n"; return TM_RETURN_VALUE_TERMINATE_ALL_THREADS; 
		} else {
			return TM_RETURN_VALUE_OK;
		}
	}

	// debug information
	if (c.verbosity > 5) {
		log.log(logger::logLevel::trace, L"Test layer: " + to_wstring(layerNumber) + L" state: " + to_wstring(stateNumber));
		game.printField(threadNo, shortValueInDatabase);
	}

	// get number of possiblities
	game.getPossibilities(threadNo, possibilityIds);
	numPossibilities = (unsigned int) possibilityIds.size();

	// unable to move
	if (numPossibilities == 0)  {
			
		// get ingame value
		game.getValueOfSituation(threadNo, floatValueInGame, shortValueInGame);

		// compare database with game
		if (shortValueInDatabase != shortValueInGame || numPliesTillCurState != 0) { 
			log << "ERROR: DATABASE ERROR IN LAYER " << layerNumber << " AND STATE " << stateNumber << ": Number of possibilities is zero, but knot value is not invalid or ply info equal zero." << "\n"; return TM_RETURN_VALUE_TERMINATE_ALL_THREADS; 
		}
        if (shortValueInDatabase == SKV_VALUE_INVALID)							   { 
			log << "ERROR: DATABASE ERROR IN LAYER " << layerNumber << " AND STATE " << stateNumber << ": Number of possibilities is zero, but knot value is invalid." << "\n"; return TM_RETURN_VALUE_TERMINATE_ALL_THREADS; 
		}

	} else {

		// check each possible move
        for (i=0; i<numPossibilities; i++) {
				
			// move
			game.move(threadNo, possibilityIds[i], playerHasChanged, pBackup);
			hasCurPlayerChanged[i] = playerHasChanged;

			// get database value
			game.getLayerAndStateNumber(threadNo, tmpLayerNumber, tmpStateNumber, symOp);
			layerInDatabaseAndCompleted = db.isLayerCompleteAndInFile(tmpLayerNumber);
			// db.isComplete() must be true, since after removing the stone from Layer 101 State 660201181 we are going to Layer 105, which is not calculated yet.
			if (!layerInDatabaseAndCompleted && db.isComplete()) {
				log << "ERROR: DATABASE ERROR IN LAYER " << layerNumber << " AND STATE " << stateNumber << ": State " << tmpStateNumber << " of layer " << tmpLayerNumber << " after move is not in database or not completed." << "\n"; return TM_RETURN_VALUE_TERMINATE_ALL_THREADS; 
			}
			if (i >= subPlyInfos.size() || i >= subValueInDatabase.size()) {
				log << "ERROR: getMaxNumPossibilities() returns a value smaller than the number of possible moves." << "\n"; return TM_RETURN_VALUE_TERMINATE_ALL_THREADS; 
			}
			db.readKnotValueFromDatabase(tmpLayerNumber, tmpStateNumber, subValueInDatabase[i]);
            db.readPlyInfoFromDatabase  (tmpLayerNumber, tmpStateNumber, subPlyInfos[i]);

			// debug information
			if (c.verbosity > 5) {
				log.log(logger::logLevel::trace, L"Test layer: " + to_wstring(layerNumber) + L" state: " + to_wstring(stateNumber));
				game.printField(threadNo, subValueInDatabase[i], 4);
			}

			// if layer or state number is invalid then value of testes state must be invalid
			if (subValueInDatabase[i] == SKV_VALUE_INVALID && shortValueInDatabase != SKV_VALUE_INVALID) { 
				log << "ERROR: DATABASE ERROR IN LAYER " << layerNumber << " AND STATE " << stateNumber << ": Succeding state  has invalid layer (" << tmpLayerNumber << ") or state number (" << tmpStateNumber << "), but tested state is not marked as invalid." << "\n"; return TM_RETURN_VALUE_TERMINATE_ALL_THREADS; 
			}

            // undo move
			game.undo(threadNo, possibilityIds[i], playerHasChanged, pBackup);
			hasCurPlayerChanged[i] = playerHasChanged;
		}

		// value possible?
		switch (shortValueInDatabase) {
			case SKV_VALUE_GAME_LOST : 
					
				// all possible moves must be lost for the current player or won for the opponent
				for (i=0; i<numPossibilities; i++) { if (subValueInDatabase[i] != ((hasCurPlayerChanged[i]) ? SKV_VALUE_GAME_WON : SKV_VALUE_GAME_LOST) && subValueInDatabase[i] != SKV_VALUE_INVALID) {
					log << "ERROR: DATABASE ERROR IN LAYER " << layerNumber << " AND STATE " << stateNumber << ": All possible moves must be lost for the current player or won for the opponent" << "\n"; return TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
				}}
				// not all options can be invalid
				for (j=0, i=0; i<numPossibilities; i++) { if (subValueInDatabase[i] == SKV_VALUE_INVALID) {
					j++;
				}}
				if (j == numPossibilities) {
					log << "DATABASE ERROR IN LAYER " << layerNumber << " AND STATE " << stateNumber << ". Not all options can be invalid" << "\n"; return TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
				}
                // ply info must be max(subPlyInfos[]+1)
                max = 0;
				for (i=0; i<numPossibilities; i++) { 
                    if (subValueInDatabase[i] == ((hasCurPlayerChanged[i]) ? SKV_VALUE_GAME_WON : SKV_VALUE_GAME_LOST))   {
                        if (subPlyInfos[i] + 1 > max) {
                            max = subPlyInfos[i] + 1;
                        }
                    }
                }
				if (numPliesTillCurState>PLYINFO_VALUE_DRAWN) { 
                    log << "DATABASE ERROR IN LAYER " << layerNumber << " AND STATE " << stateNumber << ": Knot value is LOST, but numPliesTillCurState is bigger than PLYINFO_MAX_VALUE." << "\n";  return TM_RETURN_VALUE_TERMINATE_ALL_THREADS; 
                }
                if (numPliesTillCurState!=max) { 
					log << "Number of plies:     " << numPliesTillCurState << "\n";
					log << "Max number of plies: " << max << "\n";
					game.printField(threadNo, shortValueInDatabase);
                    log << "DATABASE ERROR IN LAYER " << layerNumber << " AND STATE " << stateNumber << ": Number of needed plies is not maximal for LOST state." << "\n"; return TM_RETURN_VALUE_TERMINATE_ALL_THREADS; 
                }
				break;

			case SKV_VALUE_GAME_WON  : 
					
				// at least one possible move must be lost for the opponent or won for the current player
				{
					bool found = false;
					for (i=0; i<numPossibilities; i++) { 
						if (subValueInDatabase[i] == ((hasCurPlayerChanged[i]) ? SKV_VALUE_GAME_LOST : SKV_VALUE_GAME_WON)) {
							found = true;
							break;
						}
					}
					if (!found) { 
						log << "DATABASE ERROR IN LAYER " << layerNumber << " AND STATE " << stateNumber << ": At least one possible move must be lost for the opponent or won for the current player." << "\n"; 
						return TM_RETURN_VALUE_TERMINATE_ALL_THREADS; 
					}
				}

                // ply info must be min(subPlyInfos[]+1)
                min = PLYINFO_VALUE_DRAWN;
				for (i=0; i<numPossibilities; i++) { 
                    if (subValueInDatabase[i] == ((hasCurPlayerChanged[i]) ? SKV_VALUE_GAME_LOST : SKV_VALUE_GAME_WON))   {
                        if (subPlyInfos[i] + 1 < min) {
                            min = subPlyInfos[i] + 1;
                        }
                    }
                }
				if (numPliesTillCurState>PLYINFO_VALUE_DRAWN) { 
                    log << "DATABASE ERROR IN LAYER " << layerNumber << " AND STATE " << stateNumber << ": Knot value is WON, but numPliesTillCurState is bigger than PLYINFO_MAX_VALUE." << "\n"; return TM_RETURN_VALUE_TERMINATE_ALL_THREADS; 
                }
                if (numPliesTillCurState!=min) { 
                    log << "DATABASE ERROR IN LAYER " << layerNumber << " AND STATE " << stateNumber << ": Number of needed plies is not minimal for WON state." << "\n"; 
					log << "numPliesTillCurState: " << numPliesTillCurState << "\n";
					log << "min: " << min << "\n";
					log << "subPlyInfos: ";
					for (i=0; i<numPossibilities; i++) { 
						log << subPlyInfos[i] << " ";
					}
					log << "\n";
					log << "subValueInDatabase: ";
					for (i=0; i<numPossibilities; i++) { 
						log << subValueInDatabase[i] << " ";
					}
					log << "\n";
					log << "hasCurPlayerChanged: ";
					for (i=0; i<numPossibilities; i++) { 
						log << hasCurPlayerChanged[i] << " ";
					}
					log << "\n";
					log << "possibilityIds: ";
					for (i=0; i<numPossibilities; i++) { 
						log << possibilityIds[i] << " ";
					}
					log << "\n";
					return TM_RETURN_VALUE_TERMINATE_ALL_THREADS; 
                }
				break;

			case SKV_VALUE_GAME_DRAWN: 

				// all possible moves must be won for the opponent, lost for the current player or drawn
				for (j=0,i=0; i<numPossibilities; i++) { 
                    if (subValueInDatabase[i] != ((hasCurPlayerChanged[i]) ? SKV_VALUE_GAME_WON : SKV_VALUE_GAME_LOST) 
                        &&  subValueInDatabase[i] != SKV_VALUE_GAME_DRAWN
						&&  subValueInDatabase[i] != SKV_VALUE_INVALID)		                                             { 
						log << "DATABASE ERROR IN LAYER " << layerNumber << " AND STATE " << stateNumber << ": All possible moves must be won for the opponent, lost for the current player or drawn." << "\n"; return TM_RETURN_VALUE_TERMINATE_ALL_THREADS; 
					}
                    if (subValueInDatabase[i] == SKV_VALUE_GAME_DRAWN) j = 1;
				}

                // at least one succeding state must be drawn
                if (j == 0) { 
					log << "DATABASE ERROR IN LAYER " << layerNumber << " AND STATE " << stateNumber << ": At least one succeding state must be drawn." << "\n"; return TM_RETURN_VALUE_TERMINATE_ALL_THREADS; 
				}

                // ply info must also be drawn
                if (numPliesTillCurState != PLYINFO_VALUE_DRAWN) { 
					log << "DATABASE ERROR IN LAYER " << layerNumber << " AND STATE " << stateNumber << ": Knot value is drawn but ply info is not!" << "\n"; return TM_RETURN_VALUE_TERMINATE_ALL_THREADS; 
				}
				break;

			case SKV_VALUE_INVALID: 
				// if setSituation() returned true but state value is invalid, then all following states must be invalid
				for (i=0; i<numPossibilities; i++) { 
					if (subValueInDatabase[i] != SKV_VALUE_INVALID) break;
				}
				if (i!=numPossibilities) {
					log << "DATABASE ERROR IN LAYER " << layerNumber << " AND STATE " << stateNumber << ": If setSituation() returned true but state value is invalid, then all following states must be invalid." << "\n"; return TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
				}
                // ply info must also be invalid
                if (numPliesTillCurState != PLYINFO_VALUE_INVALID) {
					log << "DATABASE ERROR IN LAYER " << layerNumber << " AND STATE " << stateNumber << ": Knot value is invalid but ply info is not!" << "\n"; return TM_RETURN_VALUE_TERMINATE_ALL_THREADS; 
				}
				break;
		}
	}
	return TM_RETURN_VALUE_OK;
}

//-----------------------------------------------------------------------------
// Name: testState()
// Desc: Like testLayer(), but tests only a single state in the database
//-----------------------------------------------------------------------------
bool miniMax::integrity::checker::testState(unsigned int layerNumber, unsigned int stateNumber)
{
	checkerThreadVars 	tlVars(*this, layerNumber, maxNumBranches, roughTotalNumStatesProcessed);
	return testLayerThreadProc(&tlVars, stateNumber) == TM_RETURN_VALUE_OK;
}

//-----------------------------------------------------------------------------
// Name: testIfSymStatesHaveSameValue()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMax::integrity::checker::testIfSymStatesHaveSameValue(unsigned int layerNumber)
{
	// test if each state has symmetric states with the same value
	log <<"\n" << "*** Test if symmetric states have same value in layer: " << layerNumber << "***\n";
    log << game.getOutputInformation(layerNumber) << "\n";

	// simple checks
	if (!db.isOpen()) return log.log(logger::logLevel::error, L"ERROR: No database file open!");
	if (!db.isLayerCompleteAndInFile(layerNumber)) return log.log(logger::logLevel::error, L"ERROR: Layer not in file!");

	// do not read state by state from file, but load the full layer into memory once
	if (maxNumStatesToTest > loadFullLayerThreshold) {
		db.setLoadingOfFullLayerOnRead();
	}

	// run test
	return startTestThreads(layerNumber, testSymStatesSameValueThreadProc);	
}

//-----------------------------------------------------------------------------
// Name: testIfSymStatesHaveSameValue()
// Desc: 
//-----------------------------------------------------------------------------
DWORD miniMax::integrity::checker::testSymStatesSameValueThreadProc(void* pParameter, int64_t index)
{
	// Locals
	checkerThreadVars &			tlVars				= *((checkerThreadVars*) pParameter);
	checker &					c					= tlVars.rChecker;
	database::database &		db					= c.db;
	gameInterface &				game				= c.game;
	logger &					log					= c.log;
	unsigned int				layerNumber			= tlVars.layerNumber;
	unsigned int				stateNumber			= index;
	unsigned int				threadNo			= tlVars.curThreadNo;
	twoBit						shortValueInDatabase;
	twoBit						shortValueOfSymState;
    plyInfoVarType				numPliesTillCurState;
    plyInfoVarType				numPliesTillSymState;
	vector<stateAdressStruct>	symStates;

	// output
	tlVars.statesProcessed.stateProcessed(log, db.getNumberOfKnots(layerNumber), L"Tested ");

	// situation already existend in database ?
	db.readKnotValueFromDatabase(layerNumber, stateNumber, shortValueInDatabase);
    db.readPlyInfoFromDatabase  (layerNumber, stateNumber, numPliesTillCurState);

	// prepare the situation
	if (!game.setSituation(threadNo, layerNumber, stateNumber)) {
		
		// when situation cannot be constructed then state must be marked as invalid in database
		if (shortValueInDatabase != SKV_VALUE_INVALID || numPliesTillCurState != PLYINFO_VALUE_INVALID) { 
			log << "ERROR: DATABASE ERROR IN LAYER " << layerNumber << " AND STATE " << stateNumber << ": Could not set situation, but value is not invalid." << "\n"; return TM_RETURN_VALUE_TERMINATE_ALL_THREADS; 
		} else {
			return TM_RETURN_VALUE_OK;
		}
	}

	// get numbers of symmetric states
	game.getSymStateNumWithDuplicates(threadNo, symStates);

	// save value for all symmetric states
	for (auto& symState : symStates) {
		db.readKnotValueFromDatabase(symState.layerNumber, symState.stateNumber, shortValueOfSymState);
		db.readPlyInfoFromDatabase  (symState.layerNumber, symState.stateNumber, numPliesTillSymState);

		// values of symmetric states must be equal to the value of the current state
		if (shortValueOfSymState != shortValueInDatabase || numPliesTillCurState != numPliesTillSymState) {
			log.log(logger::logLevel::error, L"current tested state " + to_wstring(stateNumber) + L" has value " + to_wstring((int) shortValueInDatabase));
			game.setSituation(threadNo, layerNumber, stateNumber);
			game.printField(threadNo, shortValueInDatabase);
			log << "\n";
			log << "symmetric layer " << symState.layerNumber << " and state " << symState.stateNumber << " has value " << (int) shortValueOfSymState << "\n";
			game.setSituation(threadNo, symState.layerNumber, symState.stateNumber);
			game.printField(threadNo, shortValueOfSymState);
			game.setSituation(threadNo, layerNumber, stateNumber);
			return TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
		}
	}
	return TM_RETURN_VALUE_OK;
}
#pragma endregion

#pragma region testing functions of the gameInterface
//-----------------------------------------------------------------------------
// Name: testMoveAndUndo()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMax::integrity::checker::testSetSituationAndGetStateNum(unsigned int layerNumber)
{
	log << "\n" << "*** Test setSituation() and getLayerAndStateNumber() ***" << "\n";
	return startTestThreads(layerNumber, testSetSituationNumThreadProc);
}

//-----------------------------------------------------------------------------
// Name: testMoveAndUndo()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMax::integrity::checker::testMoveAndUndo(unsigned int layerNumber)
{
	log << "\n" << "*** Test move() and undo() ***" << "\n";
	// get succeeding layers (here game.getSuccLayers() is avoided due to performance reason, db.getSuccLayers() is cached)	
	game.getSuccLayers(layerNumber, succLayers);
	return startTestThreads(layerNumber, testMoveAndUndoThreadProc);
}

//-----------------------------------------------------------------------------
// Name: testGetPredecessors()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMax::integrity::checker::testGetPredecessors(unsigned int layerNumber)
{
	log << "\n" << "*** Test getPredecessors() ***" << "\n";
	return startTestThreads(layerNumber, testGetPredecessorsThreadProc);
}

//-----------------------------------------------------------------------------
// Name: testGetPossibilities()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMax::integrity::checker::testGetPossibilities(unsigned int layerNumber)
{
	log << "\n" << "*** Test getPossibilities() ***" << "\n";
	return startTestThreads(layerNumber, testGetPossibilitiesThreadProc);
}

//-----------------------------------------------------------------------------
// Name: testSetSituationNumThreadProc()
// Desc: 
//-----------------------------------------------------------------------------
DWORD miniMax::integrity::checker::testSetSituationNumThreadProc(void* pParameter, int64_t index)
{
	// locals
	checkerThreadVars &			tlVars				= *((checkerThreadVars*) pParameter);
	checker&					c					= tlVars.rChecker;
	gameInterface&				game				= c.game;
	logger &					log					= c.log;
	stateAdressStruct			curState			= {(stateNumberVarType) index, (unsigned char) tlVars.layerNumber};	

	// game.getLayerAndStateNumber()
	stateAdressStruct			gotState;
	unsigned int				gotSymOp, gotLayerNumber;

	// game.getSymStateNumWithDuplicates()
	vector<stateAdressStruct>	symStates;

	// game.getValueOfSituation()
	float						floatValue;
	twoBit						shortValue;

	// set state
	if (game.setSituation(tlVars.curThreadNo, curState.layerNumber, curState.stateNumber)) {

		// get symmetry operation number
		game.getLayerAndStateNumber(tlVars.curThreadNo, gotLayerNumber, gotState.stateNumber, gotSymOp);
		gotState.layerNumber = (unsigned char) gotLayerNumber;

		// get value of situation
		game.getValueOfSituation(tlVars.curThreadNo, floatValue, shortValue);

		// check if state is any duplicate
		game.getSymStateNumWithDuplicates(tlVars.curThreadNo, symStates);

		if (find(symStates.begin(), symStates.end(), gotState) == symStates.end()) {
			log << "ERROR: setSituation(" << curState.layerNumber << ", " << curState.stateNumber << "), but getLayerAndStateNumber(" << gotState.layerNumber << ", " << gotState.stateNumber << ") is not listed in getSymStateNumWithDuplicates()!" << "\n";
			return TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
		}

		// compare states from setSituation() and getLayerAndStateNumber(), they must be equal
		if (curState.layerNumber != gotState.layerNumber || curState.stateNumber != gotState.stateNumber) {
			log << "ERROR: setSituation(" << curState.layerNumber << ", " << curState.stateNumber << "), but getLayerAndStateNumber(" << gotState.layerNumber << ", " << gotState.stateNumber << ")!" << "\n";
			return TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
		}

		// value of situation must not be invalid now, otherwise setSituation() should have returned false
		if (shortValue == SKV_VALUE_INVALID) {
			log << "ERROR: setSituation(" << curState.layerNumber << ", " << curState.stateNumber << ") == true, but getValueOfSituation() == SKV_VALUE_INVALID!" << "\n";
			return TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
		}
	}

	// output
	tlVars.statesProcessed.stateProcessed(log, c.db.getNumberOfKnots(curState.layerNumber), L"Tested ");
	return TM_RETURN_VALUE_OK;
}

//-----------------------------------------------------------------------------
// Name: testMoveAndUndoThreadProc()
// Desc: 
//-----------------------------------------------------------------------------
DWORD miniMax::integrity::checker::testMoveAndUndoThreadProc(void* pParameter, int64_t index)
{
	// locals
	checkerThreadVars &			tlVars				= *((checkerThreadVars*) pParameter);
	checker&					c					= tlVars.rChecker;
	gameInterface&				game				= c.game;
	logger &					log					= c.log;
	stateAdressStruct			curState			= {(stateNumberVarType) index, (unsigned char) tlVars.layerNumber};	

	// game.getPossibilities()
	vector<unsigned int>		possibilityIds;
	void			*			pBackup				= nullptr;
	unsigned int				curPoss				= 0;
	alphaBeta::knotStruct		knot;
	alphaBeta::knotStruct		subKnot;

	// game.getValueOfSituation()
	float						floatValue			= 0;
	twoBit						shortKnotValue		= SKV_VALUE_GAME_DRAWN;

	// game.getLayerAndStateNumber()
	stateAdressStruct			subState;						

	// set state
	if (game.setSituation(tlVars.curThreadNo, curState.layerNumber, curState.stateNumber)) {
		game.getValueOfSituation(tlVars.curThreadNo, floatValue, shortKnotValue);
	} else {
		shortKnotValue = SKV_VALUE_INVALID;
	}

	if (c.verbosity >= 5) {
		game.printField(tlVars.curThreadNo, 0, 0);
	}

	// is current state consistent?
	if (shortKnotValue != SKV_VALUE_INVALID && !game.isStateIntegrityOk(tlVars.curThreadNo)) {
		log << "ERROR: setSituation(" << curState.layerNumber << ", " << curState.stateNumber << ") returned true, but fieldIntegrity() is NOT ok!" << "\n";
		return TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
	}

	// get number of possiblities
	game.getPossibilities(tlVars.curThreadNo, possibilityIds);
	knot.numPossibilities = (unsigned int) possibilityIds.size();

	// unable to move
	if (knot.numPossibilities == 0)  {
		if (shortKnotValue == SKV_VALUE_GAME_DRAWN && game.lostIfUnableToMove(tlVars.curThreadNo)) {
			log << "ERROR: setSituation(" << curState.layerNumber << ", " << curState.stateNumber << ") returned true and getValueOfSituation() returned DRAWN, although getPossibilities() yields no possible moves." << "\n";
			return TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
		}
	// moving is possible
	} else {
		if (shortKnotValue == SKV_VALUE_INVALID) {
			log << "ERROR: setSituation(" << curState.layerNumber << ", " << curState.stateNumber << ")==false or getValueOfSituation()==SKV_VALUE_INVALID, now getPossibilities() yields some possible moves." << "\n"; 	
			return TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
		}

		// check each possibility
		for (curPoss=0; curPoss<knot.numPossibilities; curPoss++) {

			// move
			game.move(tlVars.curThreadNo, possibilityIds[curPoss], subKnot.playerToMoveChanged, pBackup);

			if (c.verbosity >= 5) {
				game.printField(tlVars.curThreadNo, 0, 0);
			}

			// is this layer listed in getSuccLayers()
			if (find(c.succLayers.begin(), c.succLayers.end(), subState.layerNumber) == c.succLayers.end()) {
				// BUG: Fix TicTacToe and Muehle
				// log << "ERROR: setSituation(" << curState.layerNumber << ", " << curState.stateNumber << ") -> move() -> getLayerAndStateNumber(" << subState.layerNumber << ", " << subState.stateNumber << "). Succeding state not listed in gameInterface::getSuccLayers()!" << "\n";
				// return TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
			}

			// get state number of succeding state
			unsigned int subStateLayer, symOp;
			game.getLayerAndStateNumber(tlVars.curThreadNo, subStateLayer, subState.stateNumber, symOp);
			subState.layerNumber = subStateLayer;

			// is current consistent?
			if (!game.isStateIntegrityOk(tlVars.curThreadNo)) {
				log << "ERROR: setSituation(" << curState.layerNumber << ", " << curState.stateNumber << ") -> move() -> getLayerAndStateNumber(" << subState.layerNumber << ", " << subState.stateNumber << "). Now fieldIntegrity() is NOT ok!" << "\n";
				return TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
			}

			// undo move
			game.undo(tlVars.curThreadNo, possibilityIds[curPoss], knot.playerToMoveChanged, pBackup);

			// did playerToMove changed correctly?
			if (knot.playerToMoveChanged != subKnot.playerToMoveChanged) {
				log << "ERROR: move(playerToMoveChanged=" << subKnot.playerToMoveChanged << ") -> undo(playerToMoveChanged=" << knot.playerToMoveChanged << ")!" << "\n";
				return TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
			}

			// did we came back to original state?
			unsigned int gotLayerNumber, gotStateNumber, gotSymOp;
			game.getLayerAndStateNumber(tlVars.curThreadNo, gotLayerNumber, gotStateNumber, gotSymOp);
			if (curState.layerNumber != gotLayerNumber || curState.stateNumber != gotStateNumber) {
				log << "ERROR: setSituation(" << curState.layerNumber << ", " << curState.stateNumber << ") -> move(" << curPoss << ") -> undo(" << curPoss << ") -> getLayerAndStateNumber(" << gotLayerNumber << ", " << gotStateNumber << ")!" << "\n";
				return TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
			}

			// state reached by move() must not be invalid
			if (!game.setSituation(tlVars.curThreadNo, subState.layerNumber, subState.stateNumber)) {
				log << "ERROR: Moved from layer " << curState.layerNumber << " and state " << curState.stateNumber << " to an invalid situation layer " << subState.layerNumber << " and state " << subState.stateNumber << "\n"; 	
				return TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
			}

			// set back to current state
			game.setSituation(tlVars.curThreadNo, curState.layerNumber, curState.stateNumber);
		}
	}

	// output
	tlVars.statesProcessed.stateProcessed(log, c.db.getNumberOfKnots(curState.layerNumber), L"Tested ");
	return TM_RETURN_VALUE_OK;
}

//-----------------------------------------------------------------------------
// Name: testGetPredecessorsThreadProc()
// Desc: 
//-----------------------------------------------------------------------------
DWORD miniMax::integrity::checker::testGetPredecessorsThreadProc(void* pParameter, int64_t index)
{
	// locals
	checkerThreadVars &			tlVars				= *((checkerThreadVars*) pParameter);
	checker&					c					= tlVars.rChecker;
	gameInterface&				game				= c.game;
	logger &					log					= c.log;
	stateAdressStruct			curState			= {(stateNumberVarType) index, (unsigned char) tlVars.layerNumber};	

	// game.getPossibilities()
	vector<unsigned int>		possibilityIds;
	void			*			pBackup				= nullptr;
	unsigned int				curPoss				= 0;
	alphaBeta::knotStruct		knot;

	// game.getPredecessors()
	vector<retroAnalysis::predVars>		predVars;
	predVars.reserve(MAX_NUM_PREDECESSORS);

	// print status
	tlVars.statesProcessed.stateProcessed(log, c.db.getNumberOfKnots(curState.layerNumber), L"Tested ");

	// set situation of a valid state
	if (!game.setSituation(tlVars.curThreadNo, curState.layerNumber, curState.stateNumber)) {
		return TM_RETURN_VALUE_OK;
	}

	if (c.verbosity >= 5) {
		log.log(logger::logLevel::trace, L"setSituation(" + to_wstring(curState.layerNumber) + L", " + to_wstring(curState.stateNumber) + L")");
		game.printField(tlVars.curThreadNo, 0, 0);
	}

	// get all possible moves
	game.getPossibilities(tlVars.curThreadNo, possibilityIds);
	knot.numPossibilities = (unsigned int) possibilityIds.size();

	// go to each successor state
	for (curPoss=0; curPoss<knot.numPossibilities; curPoss++) {
					
		// move
		game.move(tlVars.curThreadNo, possibilityIds[curPoss], knot.playerToMoveChanged, pBackup);

		if (c.verbosity >= 5) {
			log.log(logger::logLevel::trace, L"move() according possibilityId=" + to_wstring(possibilityIds[curPoss]));
			game.printField(tlVars.curThreadNo, 0, 4);
		}

		// get predecessors
        game.getPredecessors(tlVars.curThreadNo, predVars);

		if (c.verbosity >= 5) {
			log << "getPredecessors()" << "\n";
			for (auto& curPredVar : predVars) {
				log << "          layerNumber=" << curPredVar.predLayerNumber << ", stateNumber=" << curPredVar.predStateNumber << ", symOperation =" << curPredVar.predSymOperation << ", playerToMoveChanged=" << curPredVar.playerToMoveChanged << "\n";
			}
		}

		// is original state listed in predVars?
		unsigned int j;
		for (j=0; j<predVars.size(); j++) { 
			if (predVars[j].predStateNumber == curState.stateNumber && predVars[j].predLayerNumber == curState.layerNumber) break;
		}
		if (j==predVars.size()) {
			log << "ERROR: Layer " << curState.layerNumber << " and state " << curState.stateNumber << " not found in predecessor list!" << "\n";
			return TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
		}

		// is playerToMoveChanged consistend between move() and getPredecessors()
		if (predVars[j].playerToMoveChanged != knot.playerToMoveChanged) {
			log << "ERROR: setSituation(" << curState.layerNumber << " and state " << curState.stateNumber << ") -> move(playerToMoveChanged=" << knot.playerToMoveChanged << "), but getPredecessors(playerToMoveChanged=" << predVars[j].playerToMoveChanged << ")!" << "\n";
			return TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
		}

		// undo move
		game.undo(tlVars.curThreadNo, possibilityIds[curPoss], knot.playerToMoveChanged, pBackup);

		if (c.verbosity >= 5) {
			log << "undo()" << "\n";
			game.printField(tlVars.curThreadNo, 0, 4);
		}
	}

	// output
	return TM_RETURN_VALUE_OK;
}

//-----------------------------------------------------------------------------
// Name: testGetPossibilitiesThreadProc()
// Desc: 
//-----------------------------------------------------------------------------
DWORD miniMax::integrity::checker::testGetPossibilitiesThreadProc(void* pParameter, int64_t index)
{
	// locals
	checkerThreadVars &			tlVars				= *((checkerThreadVars*) pParameter);
	checker&					c					= tlVars.rChecker;
	gameInterface&				game				= c.game;
	logger &					log					= c.log;
	stateAdressStruct			curState			= {(stateNumberVarType) index, (unsigned char) tlVars.layerNumber};	

	// game.getPossibilities()
	vector<unsigned int>		possibilityIds;
	void			*			pBackup				= nullptr;
	unsigned int				curPoss				= 0;
	alphaBeta::knotStruct		knot;

	// game.getValueOfSituation()
	float						floatValue			= 0;
	twoBit						shortKnotValue		= SKV_VALUE_GAME_DRAWN;

	// game.getPredecessors()
	vector<retroAnalysis::predVars>		predVars;
	predVars.reserve(MAX_NUM_PREDECESSORS);

	// game.getLayerAndStateNumber()
	stateAdressStruct			gotState;
	unsigned int				gotSymOp, gotLayerNumber;

	// print status
	tlVars.statesProcessed.stateProcessed(log, c.db.getNumberOfKnots(curState.layerNumber), L"Tested ");

	// set situation of a valid state
	if (!game.setSituation(tlVars.curThreadNo, curState.layerNumber, curState.stateNumber)) {
		return TM_RETURN_VALUE_OK;
	}

	if (c.verbosity >= 5) {
		log.log(logger::logLevel::trace, L"setSituation(" + to_wstring(curState.layerNumber) + L", " + to_wstring(curState.stateNumber) + L")");
		game.printField(tlVars.curThreadNo, 0, 0);
	}

	// get predecessors
	game.getPredecessors(tlVars.curThreadNo, predVars);
			
	// test each returned predecessor
    unsigned int j;
	for (j=0; j<predVars.size(); j++) { 

		// set situation	
		if (!game.setSituation(tlVars.curThreadNo, predVars[j].predLayerNumber, predVars[j].predStateNumber)) {
			log << "ERROR: Could not setSituation(" << predVars[j].predLayerNumber << ", " << predVars[j].predStateNumber << "), which was returned by getPredecessors(" << curState.layerNumber << ", " << curState.stateNumber << ")." << "\n";
			return TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
		}

		// Apply the symmetry operation used to reach this predecessor state, ensuring the game situation matches the predecessor's representation.
		game.applySymOp(tlVars.curThreadNo, predVars[j].predSymOperation, true, false);

		if (c.verbosity >= 5) {
			log.log(logger::logLevel::trace, L"predecessor state:");
			game.printField(tlVars.curThreadNo, 0, 0);
		}

		// get all possible moves
		game.getPossibilities(tlVars.curThreadNo, possibilityIds);
		knot.numPossibilities = (unsigned int) possibilityIds.size();
		if (!knot.numPossibilities) {
			log << "ERROR: getPredecessors(" << curState.layerNumber << ", " << curState.stateNumber << ") -> setSituation(" << predVars[j].predLayerNumber << ", " << predVars[j].predStateNumber << ") -> applySymOp(" << predVars[j].predSymOperation << ") -> getPossibilities() yields no possible moves." << "\n";
			return TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
		}

		// go to each successor state
		for (curPoss=0; curPoss<knot.numPossibilities; curPoss++) {
						
			// move
			game.move(tlVars.curThreadNo, possibilityIds[curPoss], knot.playerToMoveChanged, pBackup);

			// get corresponding state number
			game.getLayerAndStateNumber(tlVars.curThreadNo, gotLayerNumber, gotState.stateNumber, gotSymOp);
			gotState.layerNumber = (unsigned char) gotLayerNumber;

			// does states match ?
			if (curState.layerNumber == gotState.layerNumber && curState.stateNumber == gotState.stateNumber) {
				break;
			}

			// undo move
			game.undo(tlVars.curThreadNo, possibilityIds[curPoss], knot.playerToMoveChanged, pBackup);
		}
		if (curPoss==knot.numPossibilities) {
			log << "ERROR: Not all predecessors states lead back to the original layer " << curState.layerNumber << " and state " << curState.stateNumber << " by calling move()." << "\n";
			return TM_RETURN_VALUE_TERMINATE_ALL_THREADS;
		}
	}

	// output
	return TM_RETURN_VALUE_OK;
}
#pragma endregion
