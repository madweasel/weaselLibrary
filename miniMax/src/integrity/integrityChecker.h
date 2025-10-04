/*********************************************************************\
	integrityChecker.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/

#pragma once

#include "weaselEssentials/src/logger.h"
#include "weaselEssentials/src/threadManager.h"
#include "weaselEssentials/src/strLib.h"
#include "miniMax/src/database/database.h"
#include "miniMax/src/alphaBeta/alphaBeta.h"
#include "miniMax/src/typeDef.h"
#ifdef _MSC_VER
	#include <limits>
	#undef max
#endif

namespace miniMax
{

namespace integrity
{
	// class for testing the integrity of the database and the gameInterface
	class checker
	{
	friend class checkerThreadVars;
	
	public:
								checker							(logger& log, threadManagerClass& tm, database::database& db, gameInterface& game);

		// tests gameInterface
		bool					testSetSituationAndGetStateNum	(unsigned int layerNumber);
		bool					testMoveAndUndo					(unsigned int layerNumber);
		bool					testGetPredecessors				(unsigned int layerNumber);
		bool					testGetPossibilities			(unsigned int layerNumber);

		// tests database
		bool					testState						(unsigned int layerNumber, unsigned int stateNumber);
		bool					testLayer						(unsigned int layerNumber);
		bool					testIfSymStatesHaveSameValue	(unsigned int layerNumber);

		// settings
		void					setOutputFrequency				(unsigned int outputEveryNthStates) { this->outputEveryNthStates = outputEveryNthStates; }
		void					setMaxNumStatesToTest			(unsigned int maxNumStatesToTest) { this->maxNumStatesToTest = maxNumStatesToTest; }

	private:
		// variables
		gameInterface&			game;
		logger&					log;
		threadManagerClass&		tm;
		database::database&		db;
		long long				roughTotalNumStatesProcessed	= 0;
		long long 				numStatesProcessed				= 0;
		unsigned int			verbosity						= 2;
		#ifdef _DEBUG
			unsigned int		outputEveryNthStates 			= 10000;
			unsigned int        loadFullLayerThreshold			= 10000;
		#else
			unsigned int		outputEveryNthStates 			= 10000000;
			unsigned int        loadFullLayerThreshold			= 100000;
		#endif
		unsigned int			maxNumBranches					= 0;
		unsigned int 			maxNumStatesToTest				= std::numeric_limits<unsigned int>::max();
		vector<unsigned int> 	succLayers;																	// Stores the indices of successor layers for integrity checks

		// helper functions
		bool					startTestThreads				(unsigned int layerNumber, DWORD threadProc(void *pParameter, int64_t index));
		unsigned int 			getIncrement					(unsigned int layerNumber);

		// static thread functions
		static DWORD			testLayerThreadProc				(void* pParameter, int64_t index);
		static DWORD			testMoveAndUndoThreadProc		(void* pParameter, int64_t index);
		static DWORD			testSetSituationNumThreadProc	(void* pParameter, int64_t index);
		static DWORD			testGetPredecessorsThreadProc	(void* pParameter, int64_t index);
		static DWORD			testGetPossibilitiesThreadProc	(void* pParameter, int64_t index);
        static DWORD 			testSymStatesSameValueThreadProc(void *pParameter, int64_t index);
    };

    // variables hold by each thread
	struct checkerThreadVars : public threadManagerClass::threadVarsArrayItem
	{
		char					padding[64];					// Padding to avoid cache coherence issues
		checker &				rChecker;						// reference to the main checker object
		unsigned int			layerNumber;					// layer number to be processed
		progressCounter			statesProcessed;				// for status output
		vector<twoBit>			subValueInDatabase;				 
		vector<plyInfoVarType>	subPlyInfos;
		vector<bool>			hasCurPlayerChanged;
		vector<unsigned int>	possibilityIds;
		LONGLONG &				totalNumStatesProcessed;		// total number of states processed by all threads

								checkerThreadVars				(checkerThreadVars const& master);
								checkerThreadVars				(checker& parent, unsigned int layerNumber, unsigned int maxNumBranches, long long& roughTotalNumStatesProcessed); 

		void					reduce							(); 
	};

} // namespace test

} // namespace miniMax