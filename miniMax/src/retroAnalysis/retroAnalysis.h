/*********************************************************************\
	retroAnalysis.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/
#pragma once

#include "weaselEssentials/src/logger.h"
#include "weaselEssentials/src/threadManager.h"
#include "miniMax/src/typeDef.h"
#include "miniMax/src/database/database.h"
#include "miniMax/src/retroAnalysis/stateQueue.h"
#include "miniMax/src/retroAnalysis/successorCountArray.h"
#include "miniMax/src/alphaBeta/commonThreadVars.h"

namespace miniMax
{

namespace retroAnalysis
{
	// class for the retro analysis
	class solver
	{
	friend struct initRetroAnalysisVars;
	friend class  successorCountArray;

	public:
														solver							(logger& log, threadManagerClass& tm, database::database& db, gameInterface& game);
														~solver							();
		bool											calcKnotValuesByRetroAnalysis	(vector<unsigned int> &layersToCalculate);

	private:
		int64_t 										roughTotalNumStatesProcessed;							// rough estimate of the total number of states to be processed
		int64_t 										totalNumStatesProcessed;								// total number of states processed by all threads
		vector<bool>									layerInitialized;										// flag indicating if the layer has already been initialized 
		vector<unsigned int> 							layersToCalculate;										// layers which shall be calculated
		vector<stateQueue>								statesToProcess;										// States already calculated, used as basis for preceding states; one queue per thread.
		logger &										log;													// logger, used for output
		database::database &							db;														// database, for storing the calculated values
		gameInterface &									game;													// game interface, for getting the game specific information
		threadManagerClass &							tm;														// thread manager, for parallel processing
		successorCountManager 							scm;													// successor count manager
		
		bool											initRetroAnalysis				();
		bool 											prepareCountArrays				();
		bool											performRetroAnalysis			();
		bool											processPredecessor				(stateQueue& queue, const stateAdressStruct& curState, const predVars& predVarState);
		size_t 											estimateTotalNumberOfKnots		();

		// static thread functions
		static DWORD									initRetroAnalysisThreadProc		(void* pParameter, int64_t index);
		static DWORD									performRetroAnalysisThreadProc	(void* pParameter);
	};

	// thread specific variables for the function 'initRetroAnalysis()'
	class initRetroAnalysisVars : public commonThreadVars
	{
	public:
		solver& 										retroVars;												// reference to the solver class

														initRetroAnalysisVars			(initRetroAnalysisVars const& master);
														initRetroAnalysisVars			(solver& retroVars, unsigned int layerNumber, const wstring& filepath);
	};
	
} // namespace retroAnalysis

} // namespace miniMax
