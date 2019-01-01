/*********************************************************************
	pgsThread.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/
#ifndef pgsThread_H
#define pgsThread_H

#pragma once

// Windows Header Files:
#include "wildWeasel\wildWeasel.h"
#include "zoneClass.h"
#include "permutationGameSolver.h"
#include "debugHelper.h"
#include <mutex>
#include <vector>
#include <thread>
#include <queue> 

class controlledSingleThread
{
protected: 
	// structures
	struct threadVariables
	{
		mutex*					mutexVars					= nullptr;		// used for locking the threadVariables* tv
		mutex*					mutexPause					= nullptr;		// used for suspending the thread
		bool					threadPaused				= false;		// used for suspending the thread
		condition_variable		cvPause;									// for pausing
	};	
	
	struct threadConstants
	{
		wildWeasel::threadEvent*	eventUpdateOnThreadVars		= nullptr;		// can be set by the thread to indicate that there is an update on the thread variables
		wildWeasel::threadEvent*	eventThreadFinished			= nullptr;		// ist set as soon as the thread ends
		bool					threadShallBeStopped		= false;		// thread ends itself 
		bool					threadHasBeenFinished		= false;		// used to state that the thread has stopped
		bool					classHasBeenDeleted			= false;		// the class 'controlledSingleThread' can be deleted, before the thread ends

		function<void(threadConstants*, threadVariables*)>	
								inheritorThreadProc			= nullptr;		// virtual thread function of the inheriting class
	};

	// Variables
	thread						hThread;
	threadConstants *			tc							= nullptr;		// maybe shared_ptr<> might be appropriate
	threadVariables *			tv							= nullptr;		// maybe shared_ptr<> might be appropriate

	// Functions for main thread
	bool						start						(mutex* mutexVars, mutex* mutexPause, wildWeasel::threadEvent* eventUpdateOnThreadVars, wildWeasel::threadEvent* eventThreadFinished);

	// Functions for thread running here
	void						rootThreadProc				(threadConstants* tc, threadVariables* tv);
	virtual void				inheritorThreadProc			(threadConstants* tc, threadVariables* tv) {};
	template <class T> T*		newPointerOnTc				();
	template <class T> T*		newPointerOnTv				();
	inline static void			pauseIfDesired				(threadVariables * tv);

public:
	// Constructor / destructor
								controlledSingleThread		();
								~controlledSingleThread		();
	
	// Functions for the main thread	
	void						togglePauseMode				();
	bool						isRunning					();
	bool						isPaused					();
	void						stop						();
	threadVariables* 			lockThreadVariables			();
	void						unlockThreadVariables		();
};

// common class for pgsThreadFindSequences, pgsThreadConcatenateSequences, pgsThreadSolve
class pgsThread : public controlledSingleThread
{
protected: 
	// structures
	struct pgsThreadConst : public threadConstants
	{
		zoneClass				zones						;					// 
		wstring					strDatabaseDir				;					// 
		wstring					strDatabaseFileType			;					// 
		vectui					moveIndicesMap				;					// 
		vectui					reverseMoves				;					// 
		vectui					duplicatesMapSolve			;					// map duplicated moves calculated by curZones->vanishDuplicates()
		vectui					invDuplicatesMapSolve		;					// map duplicated moves calculated by curZones->vanishDuplicates()
		vectui					idPossToBranch				;					// 
		threadVariables *		tv							= nullptr;			// 
	};

	struct pgsThreadVars : public threadVariables
	{
		unsigned int			zoneNumber					;					// 
		unsigned int			maxSearchDepth				;					// 
		unsigned int			numPossibleMoves			;					// 
		unsigned int			numRectangles				;					// actually the field size
		unsigned int			useDoubleFeature			;					// 
		unsigned int			useSingleReturn				;					// 
		unsigned int			showProgressBranchDepth		= 4;				// 
		unsigned int			numSolutionsFound			;					// 
		vectui					curBranch					;					// 
		wstring					timeElapsed					;					// 
	};

	// Functions
	static void					updateTimeElapsed			(pgsThreadVars* pgsTv, pgsThreadConst* pgsTc, unsigned int solutionsFound, unsigned int currentDepth, unsigned int idPossibility);

public:
	// Functions
	pgsThreadVars*				lockThreadVariables			();
	bool						start						(mutex* mutexVars, mutex* mutexPause, wildWeasel::threadEvent* eventUpdateOnThreadVars, wildWeasel::threadEvent* eventThreadFinished, zoneClass* curZones, unsigned int selZone, wstring& databaseDir, wstring& databaseFileType, vector<unsigned int>& theMoveIndicesMap, vector<unsigned int>& theReverseMoves);
};

// Calculates sequneces for a given zone. The sequences are later on (during solving mode) used to solve a zone. 
class pgsThreadFindSequences : public pgsThread
{
protected: 
	// structures
	struct threadConstFindSequences : public pgsThreadConst
	{
	public:
		unsigned int			maxFoundSolutions;
	};

	struct threadVarsFindSequences : public pgsThreadVars
	{
//		vector<unsigned int>	solutionsThroughConcatenation;
	};

	class pgsFindSequences : public	permutationGameSolver
	{
		bool					showProgress				(unsigned int solutionsFound, unsigned int currentDepth, unsigned int idPossibility);
	};
	
	// Functions
	void						inheritorThreadProc			(threadConstants* tc, threadVariables* tv);
	threadVarsFindSequences*	lockThreadVariables			();

public:
	// Functions
	bool						start						(mutex* mutexVars, mutex* mutexPause, wildWeasel::threadEvent* eventUpdateOnThreadVars, wildWeasel::threadEvent* eventThreadFinished, zoneClass* curZones, unsigned int selZone, wstring& databaseDir, wstring& databaseFileType, vector<unsigned int>& theMoveIndicesMap, vector<unsigned int>& theReverseMoves, unsigned int maxFoundSolutions);
};

// calculates the amount of changes can be achieved by concatenating sequences of the same zone
class pgsThreadConcatenateSequences : public pgsThread
{
protected: 
	// structures
	struct threadConstConcatenate : public pgsThreadConst
	{
	};

	struct threadVarsConcatenate : public pgsThreadVars
	{
		vector<unsigned int>	solutionsThroughConcatenation;
	};

	class pgsConcatenateSequences : public	permutationGameSolver
	{
		bool					showProgress				(unsigned int solutionsFound, unsigned int currentDepth, unsigned int idPossibility);
	};

	// Functions
	void						inheritorThreadProc			(threadConstants* tc, threadVariables* tv);
	threadVarsConcatenate*		lockThreadVariables			();

public:
	// Functions
	bool						start						(mutex* mutexVars, mutex* mutexPause, wildWeasel::threadEvent* eventUpdateOnThreadVars, wildWeasel::threadEvent* eventThreadFinished, zoneClass* curZones, unsigned int selZone, wstring& databaseDir, wstring& databaseFileType, vector<unsigned int>& theMoveIndicesMap, vector<unsigned int>& theReverseMoves);
};

// solves the cube, barrel, etc. from an initial to final state by using precalculated sequences
class pgsThreadSolve : public pgsThread
{
protected: 
	// structures
	struct threadConstSolve : public pgsThreadConst
	{
		vector<unsigned int>	initialState;													// [curZones->numFields] initial state before solving, will not be changed while solving
		vector<unsigned int>	finalState;														// [curZones->numFields] state to achieve after solving, will not be changed while solving
		vector<unsigned int>*	stateToSolve;													// [curZones->numFields] state to solve, will be used and changed while solving
		unsigned int			maxNumSequences				= 10000;							// 
		void					(*doMove)					(unsigned int moveId);				// user defined function called during solving, so that move is shown in gui
	};

	struct threadVarsSolve : pgsThreadVars
	{
		unsigned int			totalNumSteps				= 0;								//
		unsigned int			numZonesSolved				= 0;								//
		unsigned int			totalNumZones				= 0;								//
		list<backTracking::solutionPath>::iterator			solPathItr;							//
		list<backTracking::solutionPath>					listSolPath;						//
	};
	
	class pgsSolve : public	permutationGameSolver
	{
		bool					showProgress				(unsigned int solutionsFound, unsigned int currentDepth, unsigned int idPossibility);
	};

	// Functions
	static void					addSolAndApplyOnState		(threadConstSolve* tc, threadVarsSolve* tv, list<backTracking::solutionPath> &listSolPath, list<sequenceStruct*> &usedSequences, vector<unsigned int>& reducedReverseMoves, vector<unsigned int>& duplicatesMapSolve, vector<unsigned int>& invDuplicatesMapSolve);
	void						inheritorThreadProc			(threadConstants * tc, threadVariables* tv);

public:

	// Functions
	bool						start						(mutex* mutexVars, mutex* mutexPause, wildWeasel::threadEvent* eventUpdateOnThreadVars, wildWeasel::threadEvent* eventThreadFinished, zoneClass* curZones, unsigned int selZone, wstring& databaseDir, wstring& databaseFileType, vector<unsigned int>& theMoveIndicesMap, vector<unsigned int>& theReverseMoves, /*const */vector<unsigned int>* stateToSolve, const vector<unsigned int>& initialState, const vector<unsigned int>& finalState, void doMove(unsigned int moveId));
	threadVarsSolve*			lockThreadVariables			();
	const vector<unsigned int>&	getInitialState				()									{ return  ((threadConstSolve*) tc)->initialState;	};
	const vector<unsigned int>&	getFinalState				()									{ return  ((threadConstSolve*) tc)->finalState;		};
	vector<unsigned int>*		getStateToSolve				()									{ return  ((threadConstSolve*) tc)->stateToSolve;	};
};

#endif