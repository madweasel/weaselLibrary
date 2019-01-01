/*********************************************************************
	pgsThread.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/

#include "pgsThread.h"

#pragma region controlledSingleThread
//-----------------------------------------------------------------------------
// Name: controlledSingleThread constructor()
// Desc: 
//-----------------------------------------------------------------------------
controlledSingleThread::controlledSingleThread()
{
}

//-----------------------------------------------------------------------------
// Name: controlledSingleThread destructor()
// Desc: 
//-----------------------------------------------------------------------------
controlledSingleThread::~controlledSingleThread()
{
	// stop thread. actually it continues to run until it notices itself that it shall be terminated.
	// thus, it is important to only use copies of the thread input variables (see members of tc and tv)
	stop();

	// lock
	if (tv && tv->mutexVars) {
		unique_lock<mutex>	myLock{ *tv->mutexVars };
	}

	// do not care about thread vars if they have not been initialized
	if (tc == nullptr) return;

	// notify that class has been deleted
	tc->classHasBeenDeleted = true;

	// deletion is only done when the the thread has finished. if not, the thread takes care of deletion.
	if (tc->threadHasBeenFinished) {
		delete tc;
		delete tv;
	}
}

//-----------------------------------------------------------------------------
// Name: start()
// Desc: 
//-----------------------------------------------------------------------------
bool controlledSingleThread::start(mutex* mutexVars, mutex* mutexPause, wildWeasel::threadEvent* eventUpdateOnThreadVars, wildWeasel::threadEvent* eventThreadFinished)
{
	if (tv				 == nullptr			 ) return false;		// tv must be allocated by calling parent thread
	if (tc				 == nullptr			 ) return false;		// tv must be allocated by calling parent thread
	if (mutexVars		 == nullptr			 ) return false;		// a mutex for the shared variables is mandatory

	tc->threadHasBeenFinished		= false;
	tc->threadShallBeStopped		= false;
	tc->inheritorThreadProc			= bind(&controlledSingleThread::inheritorThreadProc, this, tc, tv);
	tc->eventThreadFinished			= eventThreadFinished;
	tc->eventUpdateOnThreadVars		= eventUpdateOnThreadVars;

	tv->mutexVars					= mutexVars;
	tv->mutexPause					= mutexPause;
	tv->threadPaused				= false;

	if (tc->eventThreadFinished			) tc->eventThreadFinished			->reset();
	if (tc->eventUpdateOnThreadVars		) tc->eventUpdateOnThreadVars		->reset();

	// create thread and detach, so that this controlledSingleThread-object can be deleted while the thread stops itself
	hThread = thread(&controlledSingleThread::rootThreadProc, this, tc, tv);
	hThread.detach();

	return true;
}

//-----------------------------------------------------------------------------
// Name: isRunning()
// Desc: 
//-----------------------------------------------------------------------------
bool controlledSingleThread::isRunning()
{
	if (tv && tv->mutexVars) {
		unique_lock<mutex>	myLock{ *tv->mutexVars };
	}

	if (tc == nullptr) {
		return false;
	} else {
		return !tc->threadHasBeenFinished;
	}
}

//-----------------------------------------------------------------------------
// Name: isPaused()
// Desc: 
//-----------------------------------------------------------------------------
bool controlledSingleThread::isPaused()
{
	if (tv) {
		return tv->threadPaused;
	} else {
		return false;
	}
}

//-----------------------------------------------------------------------------
// Name: togglePauseMode()
// Desc: 
//-----------------------------------------------------------------------------
void controlledSingleThread::togglePauseMode()
{
	// no mutex, no thread
	if (tv				== nullptr) return;
	if (tv->mutexPause	== nullptr) return;

	// already paused?
	if (!tv->threadPaused) {
		lock_guard<mutex> lk(*tv->mutexPause);
		tv->threadPaused=true;
	} else {
		lock_guard<mutex> lk(*tv->mutexPause);
		tv->threadPaused=false;
		tv->cvPause.notify_one();
	}
}

//-----------------------------------------------------------------------------
// Name: pauseIfDesired()
// Desc: 
//-----------------------------------------------------------------------------
void controlledSingleThread::pauseIfDesired(threadVariables * tv)
{
	while(tv->threadPaused) {
		unique_lock<mutex> lk(*tv->mutexPause );
		tv->cvPause.wait(lk);
		lk.unlock();
	}
}

//-----------------------------------------------------------------------------
// Name: stop()
// Desc: 
//-----------------------------------------------------------------------------
void controlledSingleThread::stop()
{
	// stop thread. actually it continues to run until it notices itself that it shall be terminated.
	// thus, it is important to only use copies of the input variables
	if (tc) tc->threadShallBeStopped	= true;
	if (tv) tv->threadPaused			= false;
	if (tv) tv->cvPause.notify_one();
}

//-----------------------------------------------------------------------------
// Name: rootThreadProc()
// Desc: 
//-----------------------------------------------------------------------------
void controlledSingleThread::rootThreadProc(threadConstants* tc, threadVariables* tv)
{
	// set priority
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);

	// perform calculation
	tc->inheritorThreadProc(tc, tv);

	// lock
	unique_lock<mutex>	myLock{ *tv->mutexVars };

	// call user function
	tc->threadHasBeenFinished = true;
	if (tc->eventThreadFinished != nullptr) {
		tc->eventThreadFinished->set();
	}

	// result shall be available after thread has finished. thus deletion is only done when the owning class has also been deleted.
	if (tc->classHasBeenDeleted) {
		delete tc;
		delete tv;
		// tv = nullptr;	// must NOT be called since class has already been deleted
		// tc = nullptr;	// must NOT be called since class has already been deleted
	}
}

//-----------------------------------------------------------------------------
// Name: lockThreadVariables()
// Desc: 
//-----------------------------------------------------------------------------
controlledSingleThread::threadVariables* controlledSingleThread::lockThreadVariables()
{
	if (tv) tv->mutexVars->lock();
	return tv;
}

//-----------------------------------------------------------------------------
// Name: unlockThreadVariables()
// Desc: 
//-----------------------------------------------------------------------------
void controlledSingleThread::unlockThreadVariables()
{
	if (tv) tv->mutexVars->unlock();
}

//-----------------------------------------------------------------------------
// Name: newPointerOnTc()
// Desc: 
//-----------------------------------------------------------------------------
template <class T> T* controlledSingleThread::newPointerOnTc()
{
	if (tc == nullptr) {
		T*		myTc;
		myTc	= new T();
		tc		= myTc;
	} 
	return (T*) tc;
}

//-----------------------------------------------------------------------------
// Name: newPointerOnTv()
// Desc: 
//-----------------------------------------------------------------------------
template <class T> T* controlledSingleThread::newPointerOnTv()
{
	if (tv == nullptr) {
		T*		myTv;
		myTv	= new T();
		tv		= myTv;
	} 
	return (T*) tv;
}
#pragma endregion

#pragma region pgsThread
//-----------------------------------------------------------------------------
// Name: start()
// Desc: 
//-----------------------------------------------------------------------------
bool pgsThread::start(	mutex*					mutexVars, 
						mutex*					mutexPause, 
						wildWeasel::threadEvent*	eventUpdateOnThreadVars, 
						wildWeasel::threadEvent*	eventThreadFinished,
						zoneClass*				curZones, 
						unsigned int			selZone, 
						wstring&				databaseDir,
						wstring&				databaseFileType,
						vector<unsigned int>&	moveIndicesMap,
						vector<unsigned int>&	reverseMoves)
{
	// param ok?
	if (tv				 == nullptr			 ) return false;		// tv must be allocated by calling parent thread
	if (tc				 == nullptr			 ) return false;		// tv must be allocated by calling parent thread
	auto	pgsTv = (pgsThreadVars *) tv;
	auto	pgsTc = (pgsThreadConst*) tc;

	// make copy for thread
	curZones->copyTo(&pgsTc->zones);

	// vanish duplicates in moveIndicesMap
	pgsTc->zones.vanishDuplicates(moveIndicesMap, reverseMoves, &pgsTc->moveIndicesMap, &pgsTc->reverseMoves, &pgsTc->duplicatesMapSolve, &pgsTc->invDuplicatesMapSolve);

	// copy parameters
	pgsTc->strDatabaseDir			= databaseDir;
	pgsTc->strDatabaseFileType		= databaseFileType;
	
	// set values to be shown by for user
	pgsTv->curBranch.resize(pgsTv->showProgressBranchDepth, 0);
	pgsTv->numSolutionsFound		= 0;
	pgsTv->zoneNumber				= selZone;
	if (pgsTv->zoneNumber < pgsTc->zones.numZones) {
		pgsTv->numPossibleMoves			= pgsTc->zones.calcNumPossibleMoves(pgsTv->zoneNumber);
		pgsTv->maxSearchDepth			= pgsTc->zones.zoneProp[pgsTv->zoneNumber].maxSequenceLength;	
		pgsTv->numRectangles			= pgsTc->zones.zoneProp[pgsTv->zoneNumber].numRectangles;
		pgsTv->useDoubleFeature			= pgsTc->zones.zoneProp[pgsTv->zoneNumber].useDoubleFeature;
		pgsTv->useSingleReturn			= pgsTc->zones.zoneProp[pgsTv->zoneNumber].useSingleReturn;
	}
	pgsTv->timeElapsed				= L"00:00:00";

	// create thread
	return controlledSingleThread::start(mutexVars, mutexPause, eventUpdateOnThreadVars, eventThreadFinished);
}

//-----------------------------------------------------------------------------
// Name: updateTimeElapsed()
// Desc: 
//-----------------------------------------------------------------------------
void pgsThread::updateTimeElapsed(pgsThreadVars* pgsTv, pgsThreadConst* pgsTc, unsigned int solutionsFound, unsigned int currentDepth, unsigned int idPossibility)
{
	if (currentDepth < pgsTv->showProgressBranchDepth) {

		DWORD				elapsedSeconds;
		FILETIME			ftTime, ft1, ft2, ft3;
		SYSTEMTIME			elapsedTime;
		ULONGLONG			qwResult;
		wstringstream		wss;
		
		// calc elapsed time
		GetThreadTimes(GetCurrentThread(), &ft1, &ft2, &ft3, &ftTime);
		qwResult			= (((ULONGLONG) ftTime.dwHighDateTime) << 32) + ftTime.dwLowDateTime;
		elapsedSeconds		= (DWORD) (qwResult / 10000000);
		elapsedTime.wSecond = elapsedSeconds % 60;
		elapsedTime.wMinute = (elapsedSeconds / 60) % 60;
		elapsedTime.wHour   = (WORD) elapsedSeconds / 3600;

		// update variables
		wss.str(L""); wss << setfill(L'0') << setw(2) << elapsedTime.wHour << L":" << setw(2) << elapsedTime.wMinute << L":" << setw(2) << elapsedTime.wSecond;
		pgsTv->timeElapsed				= wss.str();
		pgsTv->numSolutionsFound		= solutionsFound;
		if (currentDepth < pgsTv->showProgressBranchDepth && currentDepth < pgsTv->curBranch.size()) {
			pgsTv->curBranch[currentDepth] = ((idPossibility < pgsTc->idPossToBranch.size()) ? pgsTc->idPossToBranch[idPossibility] : idPossibility);
		}
		if (pgsTc->eventUpdateOnThreadVars) {
			pgsTc->eventUpdateOnThreadVars->set();
		}
	}
}

//-----------------------------------------------------------------------------
// Name: lockThreadVariables()
// Desc: 
//-----------------------------------------------------------------------------
pgsThread::pgsThreadVars* pgsThread::lockThreadVariables()
{
	return (pgsThreadVars*) controlledSingleThread::lockThreadVariables();
}
#pragma endregion

#pragma region find sequences
//-----------------------------------------------------------------------------
// Name: start()
// Desc: 
//-----------------------------------------------------------------------------
bool pgsThreadFindSequences::start(	mutex*					mutexVars, 
									mutex*					mutexPause, 
									wildWeasel::threadEvent*	eventUpdateOnThreadVars, 
									wildWeasel::threadEvent*	eventThreadFinished,
									zoneClass*				curZones,
									unsigned int			selZone,
									wstring&				databaseDir,
									wstring&				databaseFileType,
									vector<unsigned int>&	moveIndicesMap,
									vector<unsigned int>&	reverseMoves,
									unsigned int			maxFoundSolutions)
{
	// locals
	auto pgsTc					= newPointerOnTc<threadConstFindSequences>();
	auto pgsTv					= newPointerOnTv<threadVarsFindSequences >();

	pgsTc->tv					= pgsTv;
	pgsTc->maxFoundSolutions	= maxFoundSolutions;

	return pgsThread::start(mutexVars, mutexPause, eventUpdateOnThreadVars, eventThreadFinished, curZones, selZone, databaseDir, databaseFileType, moveIndicesMap, reverseMoves);
}

//-----------------------------------------------------------------------------
// Name: inheritorThreadProc()
// Desc: 
//-----------------------------------------------------------------------------
void pgsThreadFindSequences::inheritorThreadProc(threadConstants* tc, threadVariables* tv)
{
	// locals
	vectui						pgsInitialBlocked	;
	vectui						pgsMovesBlocked		;
	vectui						pgsZones			;
	pgsSeqBlUserClass *			seqBlUser			= nullptr;
	pgsFindSequences*			solver				= new pgsFindSequences();
	unsigned int				showProgressDepth	=  4;				
	unsigned int				minChangeInRelZone	=  1;
	unsigned int				maxChangeInRelZone	= 30;
	bool						useMaxDepth			= false;									// when true, then only solutions of the maximum search depth will be found
	auto						pgsTc				= (threadConstFindSequences*) tc;			// get variables, which are shared with calling thread
	auto						pgsTv				= (threadVarsFindSequences *) tv;			// get variables, which are shared with calling thread

	// init solver
	solver->setParentClassPointer((void*) tc);
	solver->setFieldSize(pgsTc->zones.numFields, pgsTc->strDatabaseDir, pgsTc->strDatabaseFileType);
	solver->setMaxFoundSolutions(pgsTc->maxFoundSolutions);

	pgsTc->zones.calcPossToBranch(pgsTv->zoneNumber, pgsTc->idPossToBranch);		

	// translate variables from zoneClass to permutationGameSolver
	pgsTc->zones.translateForPGS(pgsTv->zoneNumber, pgsZones, pgsMovesBlocked, pgsInitialBlocked, seqBlUser);

	// load sequences
	solver->searchSequences(	pgsZones,			
								&pgsInitialBlocked,	
								&pgsMovesBlocked,
								seqBlUser,
								showProgressDepth,
								minChangeInRelZone, 
								maxChangeInRelZone, 
								pgsTc->zones.zoneProp[pgsTv->zoneNumber].maxSequenceLength,
								pgsTc->zones.numMoves,
								pgsTc->moveIndicesMap, 
								&pgsTc->reverseMoves, 
								true, 
								useMaxDepth, 
								pgsTc->zones.zoneProp[pgsTv->zoneNumber].useDoubleFeature  > 0 ? true : false, 
								pgsTc->zones.zoneProp[pgsTv->zoneNumber].useSingleReturn   > 0 ? true : false,
								pgsTc->zones.zoneProp[pgsTv->zoneNumber].overWriteDatabase > 0 ? true : false,
								true);

	// free mem
	if (seqBlUser			 != nullptr) delete seqBlUser;
	if (solver				 != nullptr) delete solver;
}

//-----------------------------------------------------------------------------
// Name: showProgress()
// Desc: 
//-----------------------------------------------------------------------------
bool pgsThreadFindSequences::pgsFindSequences::showProgress(unsigned int solutionsFound, unsigned int currentDepth, unsigned int idPossibility)
{
	// locals
	auto						pgsTc				= (threadConstFindSequences*) parentClassPointer;	// get variables, which are shared with calling thread
	auto						pgsTv				= (threadVarsFindSequences *) pgsTc->tv;			// get variables, which are shared with calling thread

	// stop search ?
	if (pgsTc->threadShallBeStopped) {
		return false;
	}

	// pause ?
	pauseIfDesired(pgsTv);

	// lock solution variables
	unique_lock<mutex>	myLock	{*pgsTv->mutexVars};

	// update progress
	updateTimeElapsed(pgsTv, pgsTc, solutionsFound, currentDepth, idPossibility);

	// continue search
	return true;
}

//-----------------------------------------------------------------------------
// Name: lockThreadVariables()
// Desc: 
//-----------------------------------------------------------------------------
pgsThreadFindSequences::threadVarsFindSequences* pgsThreadFindSequences::lockThreadVariables()
{
	return (threadVarsFindSequences*) controlledSingleThread::lockThreadVariables();
}
#pragma endregion

#pragma region concatenate
//-----------------------------------------------------------------------------
// Name: start()
// Desc: 
//-----------------------------------------------------------------------------
bool pgsThreadConcatenateSequences::start(	mutex*					mutexVars, 
											mutex*					mutexPause, 
											wildWeasel::threadEvent*	eventUpdateOnThreadVars, 
											wildWeasel::threadEvent*	eventThreadFinished,
											zoneClass*				curZones,
											unsigned int			selZone,
											wstring&				databaseDir,
											wstring&				databaseFileType,
											vector<unsigned int>&	moveIndicesMap,
											vector<unsigned int>&	reverseMoves)
{
	// locals
	auto pgsTc	= newPointerOnTc<threadConstConcatenate>();
	auto pgsTv	= newPointerOnTv<threadVarsConcatenate >();

	pgsTc->tv					= pgsTv;

	return pgsThread::start(mutexVars, mutexPause, eventUpdateOnThreadVars, eventThreadFinished, curZones, selZone, databaseDir, databaseFileType, moveIndicesMap, reverseMoves);
}

//-----------------------------------------------------------------------------
// Name: ThreadProcConcatenateSequences()
// Desc: Calculates how much permutations/solutions could be found if the sequences of one zone would be applied conquetaned. 
//       The result is listed in the zones list view, but not stored to the database.
//-----------------------------------------------------------------------------
void pgsThreadConcatenateSequences::inheritorThreadProc(threadConstants* tc, threadVariables* tv)
{
	// locals
	list<sequenceStruct*>::iterator	seqItr;
	list<sequenceStruct*>			calculatedSequences;
	list<sequenceStruct*>			concatenatedSequences;
	unsigned int					i;
	vectui							pgsZones;
	vectui							moveIndicesMap;
	unsigned int					showProgressDepth	=  2;				
	unsigned int					minChangeInRelZone	=  1;
	unsigned int					maxChangeInRelZone	= 30;
	bool							useMaxDepth			= true;
	auto							solver				= new pgsConcatenateSequences();
	auto							pgsTc				= (threadConstConcatenate*) tc;			// get variables, which are shared with calling thread
	auto							pgsTv				= (threadVarsConcatenate *) tv;			// get variables, which are shared with calling thread
	bool							iterateAllZones		= (pgsTv->zoneNumber >= pgsTc->zones.numZones);

	// init solver
	solver->setParentClassPointer((void*) tc);
	solver->setFieldSize(pgsTc->zones.numFields, pgsTc->strDatabaseDir, pgsTc->strDatabaseFileType);

	// allocate mem for result array
	pgsTv->solutionsThroughConcatenation.resize(pgsTc->zones.numZones);

	// process each zone (only if pgsTv->zoneNumber >= pgsTc->zones.numZones iterate through all zones)
	for (pgsTv->zoneNumber = (iterateAllZones ? 0 : pgsTv->zoneNumber); pgsTv->zoneNumber < pgsTc->zones.numZones; pgsTv->zoneNumber++) {
		
		// shall process be terminated?
		if (pgsTc->threadShallBeStopped) {
			break;
		}
		
		// translate variables from zoneClass to permutationGameSolver
		pgsTc->zones.translateForPGS(pgsTv->zoneNumber, pgsZones);
	
	    solver->loadSequences(pgsZones, calculatedSequences);

		// extract data of sequences
		moveIndicesMap.resize(calculatedSequences.size() * pgsTc->zones.numFields);
		for (i=0,seqItr=calculatedSequences.begin(); seqItr!=calculatedSequences.end(); seqItr++, i++) {
			memcpy(&moveIndicesMap[i*pgsTc->zones.numFields], &(*seqItr)->indicesMap[0], sizeof(unsigned int) * pgsTc->zones.numFields);
		}

		pgsTv->maxSearchDepth	= pgsTc->zones.zoneProp[pgsTv->zoneNumber].searchDepthUsingSequences;
		pgsTv->numPossibleMoves	= calculatedSequences.size();

		// concatenatedSequences alias foundSequences will be cleared in solver->searchSequences()  -> this way no memory leak should occur, so clearing concatenatedSequences is not necessary
		// sequenceStruct::clearList(concatenatedSequences);

		concatenatedSequences = solver->searchSequences(	pgsZones,
															nullptr,	
															nullptr,
															nullptr,
															showProgressDepth,
															minChangeInRelZone, 
															maxChangeInRelZone, 
															pgsTc->zones.zoneProp[pgsTv->zoneNumber].searchDepthUsingSequences,
															calculatedSequences.size(),
															moveIndicesMap, 
															nullptr, 
															true,
															useMaxDepth,
															false, 
															false,
															false,
															false);
	
		// update list item
		if (!pgsTc->threadShallBeStopped) {
			pgsTv->solutionsThroughConcatenation[pgsTv->zoneNumber] = concatenatedSequences.size();
			if (pgsTc->eventUpdateOnThreadVars) {
				pgsTc->eventUpdateOnThreadVars->set();
			}
		} else {
			pgsTv->solutionsThroughConcatenation[pgsTv->zoneNumber] = 0;
		}

		// reset
		sequenceStruct::clearList(calculatedSequences);

		// quit if only one zone shall be calculated
		if (!iterateAllZones) break;
	}

	// finish (concatenatedSequences alias foundSequences will be cleared here)
	delete solver;
}

//-----------------------------------------------------------------------------
// Name: showProgress()
// Desc: 
//-----------------------------------------------------------------------------
bool pgsThreadConcatenateSequences::pgsConcatenateSequences::showProgress(unsigned int solutionsFound, unsigned int currentDepth, unsigned int idPossibility)
{
	// locals
	auto pgsTc = (threadConstConcatenate*)		parentClassPointer;
	auto pgsTv = (threadVarsConcatenate*)		pgsTc->tv;

	// stop search ?
	if (pgsTc->threadShallBeStopped) {
		pgsTv->solutionsThroughConcatenation[pgsTv->zoneNumber] = 0;
		return false;
	}

	// pause ?
	pauseIfDesired(pgsTv);

	// lock solution variables
	unique_lock<mutex>	myLock	{*pgsTv->mutexVars};

	updateTimeElapsed(pgsTv, pgsTc, solutionsFound, currentDepth, idPossibility);

	pgsTv->solutionsThroughConcatenation[pgsTv->zoneNumber] = solutionsFound;

	// continue search
	return true;
}

//-----------------------------------------------------------------------------
// Name: lockThreadVariables()
// Desc: 
//-----------------------------------------------------------------------------
pgsThreadConcatenateSequences::threadVarsConcatenate* pgsThreadConcatenateSequences::lockThreadVariables()
{
	return (threadVarsConcatenate*) controlledSingleThread::lockThreadVariables();;
}
#pragma endregion

#pragma region solve
//-----------------------------------------------------------------------------
// Name: start()
// Desc: 
//-----------------------------------------------------------------------------
bool pgsThreadSolve::start(	mutex*						mutexVars,
							mutex*						mutexPause,
							wildWeasel::threadEvent*		eventUpdateOnThreadVars, 
							wildWeasel::threadEvent*		eventThreadFinished,
							zoneClass*					curZones,
							unsigned int				selZone,
							wstring&					databaseDir,
							wstring&					databaseFileType,
							vector<unsigned int>&		theMoveIndicesMap,
							vector<unsigned int>&		theReverseMoves,
							/*const */vector<unsigned int>*	stateToSolve,
							const vector<unsigned int>&	initialState,
							const vector<unsigned int>&	finalState,
							void doMove					(unsigned int moveId))
{
	// locals
	auto pgsTc					= newPointerOnTc<threadConstSolve>();
	auto pgsTv					= newPointerOnTv<threadVarsSolve >();

	pgsTc->tv					= pgsTv;
	pgsTc->stateToSolve			= stateToSolve;
	pgsTc->initialState			= initialState;
	pgsTc->finalState			= finalState;
	pgsTc->doMove				= doMove;

	return pgsThread::start(mutexVars, mutexPause, eventUpdateOnThreadVars, eventThreadFinished, curZones, selZone, databaseDir, databaseFileType, theMoveIndicesMap, theReverseMoves);
}

//-----------------------------------------------------------------------------
// Name: ThreadProcSolve()
// Desc: 
//-----------------------------------------------------------------------------
void pgsThreadSolve::inheritorThreadProc(threadConstants* tc, threadVariables* tv)
{
	// locals
	unsigned int						curSearchDepth;
	unsigned int						showProgressDepth		= 1;
	list<sequenceStruct*>::iterator		seqItr;
	list<sequenceStruct*> *				usedSequences;														// pointer to the actually used sequence set
	list<sequenceStruct*>				trivialSequences;													// each is sequence is a single move
	list<sequenceStruct*>				calculatedSequences;												// these are the previously calculated sequences
	list<backTracking::solutionPath>	listSolPath;
	sequenceStruct	*					tmpSequence				= nullptr;
	vectui								pgsInitialBlocked		;
	vectui								pgsMovesBlocked			;
	vectui								pgsZones				;
	pgsSeqBlUserClass *					seqBlUser				= nullptr;
	pgsSolve*							solver					= new pgsSolve();
	auto								pgsTc					= (threadConstSolve*) tc;					// get variables, which are shared with calling thread
	auto								pgsTv					= (threadVarsSolve *) tv;					// get variables, which are shared with calling thread

	// init solver
	pgsTv->listSolPath.clear();
	solver->setParentClassPointer((void*) tc);
	solver->setFieldSize(pgsTc->zones.numFields, pgsTc->strDatabaseDir, pgsTc->strDatabaseFileType);
	solver->setInitialState(pgsTc->initialState);
	solver->setFinalState(pgsTc->finalState);

	// zero shared variables
	pgsTv->totalNumSteps		= 0;
	pgsTv->numZonesSolved		= 0;
	pgsTv->numPossibleMoves		= 0;
	pgsTv->curBranch[0]			= 0;
	pgsTv->totalNumZones		= pgsTc->zones.numZones;

	// set basic moves as trivial sequences
	for (unsigned int i=0; i<pgsTc->zones.numMoves; i++) {
		tmpSequence = new sequenceStruct(pgsTc->zones.numFields, &pgsTc->moveIndicesMap[i * pgsTc->zones.numFields], 1, &i);
		trivialSequences.push_back(tmpSequence);
	}

	// Complete the zones step by step using Back-Tracking
	for (pgsTv->zoneNumber=0; pgsTv->zoneNumber < pgsTc->zones.numZones; pgsTv->zoneNumber++) {
		
		// 
		pgsTc->zones.translateForPGS(pgsTv->zoneNumber, pgsZones, pgsMovesBlocked, pgsInitialBlocked, seqBlUser);
	
		// calc possible moves from zones->zoneProp[selZone].excludedMove[]
		pgsTv->numPossibleMoves	= pgsTc->zones.calcNumPossibleMoves(pgsTv->zoneNumber);
		pgsTv->curBranch[0]		= 0;
		pgsTc->zones.calcPossToBranch(pgsTv->zoneNumber, pgsTc->idPossToBranch);

		// Try using simple back tracking
		for (curSearchDepth = 1; curSearchDepth <= pgsTc->zones.zoneProp[pgsTv->zoneNumber].backTrackingDepth; curSearchDepth++) {

			// shall process be terminated?
			if (pgsTc->threadShallBeStopped) {
				pgsTv->zoneNumber = pgsTc->zones.numZones;
				break;
			}

			solver->finishZone(	listSolPath,
								*pgsTc->stateToSolve,
								pgsZones, 
								&pgsInitialBlocked, 
								&pgsMovesBlocked,
								seqBlUser,
								showProgressDepth,
								curSearchDepth, 
								trivialSequences, 
								false, true);

			// leave when solution was found
			if (listSolPath.size()) {
				usedSequences = &trivialSequences;
				break;
			}
		}

		// free mem
		if (seqBlUser				!= nullptr) delete		seqBlUser;				seqBlUser				= nullptr;

		// Solve using sequences
		if (listSolPath.size() == 0 && pgsTc->threadShallBeStopped == false) {

			// load sequences
		    solver->loadSequences(pgsZones, calculatedSequences);

			// Try using simple back tracking
			for (curSearchDepth = 1; curSearchDepth <= pgsTc->zones.zoneProp[pgsTv->zoneNumber].searchDepthUsingSequences; curSearchDepth++) {

				// shall process be terminated?
				if (pgsTc->threadShallBeStopped) {
					pgsTv->zoneNumber = pgsTc->zones.numZones;
					break;
				}

				// calc possible moves
				pgsTv->numPossibleMoves	= calculatedSequences.size();
				pgsTv->curBranch[0]		= 0;

				// Complete zone using the calculated sequences
				solver->finishZone(	listSolPath,
									*pgsTc->stateToSolve,
									pgsZones, 
									NULL, 
									NULL, 
									NULL,
									showProgressDepth,
									curSearchDepth, 
									calculatedSequences, 
									false, calculatedSequences.size() < pgsTc->maxNumSequences);
			
				// leave when solution was found
				if (listSolPath.size()) {
					usedSequences = &calculatedSequences;
					break;
				};
			}
		}

		// Break when no solution was found
		if (listSolPath.size() == 0) {
			listSolPath.clear();
			sequenceStruct::clearList(calculatedSequences);
			break;
		} else {
			// apply path on solve state
			addSolAndApplyOnState(pgsTc, pgsTv, listSolPath, *usedSequences, pgsTc->reverseMoves, pgsTc->duplicatesMapSolve, pgsTc->invDuplicatesMapSolve);
			listSolPath.clear();
			sequenceStruct::clearList(calculatedSequences);
		}
	}
	pgsTv->numSolutionsFound	= ((pgsTv->zoneNumber == pgsTc->zones.numZones) ? 1 : 0);
	
	// free mem
	sequenceStruct::clearList(trivialSequences);
	delete solver;
}

//-----------------------------------------------------------------------------
// Name: addSolAndApplyOnState()
// Desc: 
//-----------------------------------------------------------------------------
void pgsThreadSolve::addSolAndApplyOnState(threadConstSolve* tc, threadVarsSolve* tv, list<backTracking::solutionPath> &listSolPath, list<sequenceStruct*> &usedSequences, vector<unsigned int>& reducedReverseMoves, vector<unsigned int>& duplicatesMapSolve, vector<unsigned int>& invDuplicatesMapSolve)
{
	// locals
	backTracking::solutionPath					curPath;
	list<backTracking::solutionPath>::iterator	solPathItr;
	list<sequenceStruct*>::iterator				usedSeqItr;
	unsigned int								i, j, k;
	unsigned int								reducedMove1, reducedMove2;

	// leave if thread shall be terminated
	if (tc->threadShallBeStopped) return;

	// lock solution variables
	unique_lock<mutex>	myLock	{*tv->mutexVars};
	
	// count number of moves
	for (curPath.numSteps = 0, solPathItr = listSolPath.begin(); solPathItr != listSolPath.end(); solPathItr++) {

		// process each step of the solution, which consists of sequences
		for (i=0; i<(*solPathItr).numSteps; i++) {

			// goto used sequence
			for (j=0, usedSeqItr = usedSequences.begin(); j<(*solPathItr).path[i]; j++) {
				usedSeqItr++;
			}

			// add sequence length
			curPath.numSteps += (*usedSeqItr)->solPath.numSteps;
		}
	}

	// allocate mem
	curPath.path	 .resize(curPath.numSteps);
	curPath.numSteps = 0;

	// perform moves on state
	for (solPathItr = listSolPath.begin(); solPathItr != listSolPath.end(); solPathItr++) {

		// process each step of the solution, which consists of sequences
		for (i=0; i<(*solPathItr).numSteps; i++) {

			// goto used sequence
			for (j=0, usedSeqItr = usedSequences.begin(); j<(*solPathItr).path[i]; j++) {
				usedSeqItr++;
			}

			// apply moves of the used sequence
			for (k=0; k < (*usedSeqItr)->solPath.numSteps; k++) {
				curPath.path[curPath.numSteps] = duplicatesMapSolve[(*usedSeqItr)->solPath.path[k]];
				tc->doMove(curPath.path[curPath.numSteps]);

				// ... needs to be updated if copy instead of reference
//				tc->stateToSolve = ; 

				curPath.numSteps++;
			}
		}
	}

	// if last move of the so far calculated solution is the reverse move of the first additional steps, then delete both steps
	if (listSolPath.size() > 1) { 
		while (curPath.numSteps>0) {
			solPathItr = listSolPath.end();
			solPathItr--;
			if ((*solPathItr).numSteps>0) {
				reducedMove1 = invDuplicatesMapSolve[(*solPathItr).path[(*solPathItr).numSteps-1]];
				reducedMove2 = invDuplicatesMapSolve[curPath.path[0]];
				if (reducedMove1 == reducedReverseMoves[reducedMove2]) {
					(*solPathItr).numSteps--;
					for (i=0; i<curPath.numSteps-1; i++) curPath.path[i] = curPath.path[i+1];
					curPath.numSteps--;
					tv->totalNumSteps--;
				} else {
					break;
				}
			} else {
				break;
			}
		}
	}

	// if steps in curPath are consecutive back- and forward moves, so delete them too
	for (i=1; i<curPath.numSteps; i++) {
		reducedMove1 = invDuplicatesMapSolve[curPath.path[i-1]];
		reducedMove2 = invDuplicatesMapSolve[curPath.path[i  ]];
		if (reducedMove1 == reducedReverseMoves[reducedMove2]) {
			for (j=i-1; j<curPath.numSteps-2; j++) curPath.path[j] = curPath.path[j+2];
			curPath.numSteps -= 2;
			i--;
		}
	}

	// store solution
	tv->totalNumSteps += curPath.numSteps;
	tv->numZonesSolved++;
	tv->listSolPath.push_back(curPath);
	if (tv->listSolPath.size() == 1) {
		tv->solPathItr	= tv->listSolPath.begin();
	}

	if (tc->eventUpdateOnThreadVars) {
		tc->eventUpdateOnThreadVars->set();
	}
}

//-----------------------------------------------------------------------------
// Name: showProgress()
// Desc: 
//-----------------------------------------------------------------------------
bool pgsThreadSolve::pgsSolve::showProgress(unsigned int solutionsFound, unsigned int currentDepth, unsigned int idPossibility)
{
	// locals
	auto pgsTc = (threadConstSolve*)	parentClassPointer;
	auto pgsTv = (threadVarsSolve *)	pgsTc->tv;

	// stop search ?
	if (pgsTc->threadShallBeStopped) {
		return false;
	}
	
	// lock solution variables
	unique_lock<mutex>	myLock	{*pgsTv->mutexVars};

	// update current processed branch
	if (pgsTc->idPossToBranch.size() <= idPossibility) {
		pgsTv->curBranch[currentDepth]		= idPossibility;
	} else {
		pgsTv->curBranch[currentDepth]		= pgsTc->idPossToBranch[idPossibility];
	}

	// continue search
	return true;
}

//-----------------------------------------------------------------------------
// Name: lockThreadVariables()
// Desc: 
//-----------------------------------------------------------------------------
pgsThreadSolve::threadVarsSolve* pgsThreadSolve::lockThreadVariables()
{
	return (threadVarsSolve*) controlledSingleThread::lockThreadVariables();
}
#pragma endregion
