/*********************************************************************\
	strLib.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/
#pragma once

#include "weaselEssentials/src/logger.h"
#include "weaselEssentials/src/threadManager.h"
#include "miniMax/src/typeDef.h"
#include "miniMax/src/database/database.h"
#include "miniMax/src/alphaBeta/knotStruct.h"
#include "miniMax/src/alphaBeta/commonThreadVars.h"

#include <mutex>
#include <vector>

namespace miniMax
{

namespace alphaBeta
{
	// thread specific variables for initialization
	struct initAlphaBetaVars : public commonThreadVars
	{
		solver& 										rSolver;
		bool											initAlreadyDone 				= false;					// true, when the file is complete and contains all states of a layer

														initAlphaBetaVars				(initAlphaBetaVars const& master);
														initAlphaBetaVars				(solver& rSolver, unsigned int layerNumber, const wstring& filepath);
	};

	// thread specific variables for calculation
	struct runAlphaBetaVars : public commonThreadVars
	{
		solver& 										rSolver;
		std::vector<knotStruct>							branchArray;												// array of size [(depthOfFullTree - tilLevel) * maxNumBranches] for storage of the branches at each search depth
		std::vector<stateAdressStruct>					symStates;													// filled by game->getSymmetricStates()
		knotStruct*										rootKnot;													// used to run the min-max calculation without database

														runAlphaBetaVars				(runAlphaBetaVars const& master);
														runAlphaBetaVars				(solver& rSolver, unsigned int layerNumber, const wstring& filepath);
	};

	class solver
	{
	friend struct commonThreadVars;
	friend struct initAlphaBetaVars;
	friend struct runAlphaBetaVars;

	public:
														solver							(logger& log, threadManagerClass& tm, database::database& db, gameInterface& game);
		bool											getBestChoice					(unsigned int& choice, stateInfo& infoAboutChoices);
		bool											calcKnotValuesByAlphaBeta		(std::vector<unsigned int>& layersToCalculate);
		void											setSearchDepth					(unsigned int maxAlphaBetaSearchDepth);

	private:
		logger &										log;													// logger, used for output
		database::database &							db;														// database, for storing the calculated values
		gameInterface &									game;													// game interface, for getting the game specific information
		threadManagerClass &							tm;														// thread manager, for parallel processing
		int64_t 										totalNumStatesProcessed			= 0;					// number of states processed by all threads
		int64_t 										roughTotalNumStatesProcessed	= 0;					// number of states processed by all threads (roughly)
		unsigned int									depthOfFullTree					= 0;					// maximum search depth, equivalent to the maximum number of plies
		unsigned int									maxNumBranches					= 0;					// maximum number of branches/moves
		bool											calcDatabase					= false;				// true if the database is currently beeing calculated
		std::mutex 										dbMutex;

		bool											init							(unsigned int layerNumber);
		bool											run								(unsigned int layerNumber);
		bool											letTheTreeGrow					(	   knotStruct& knot, 	   runAlphaBetaVars& rabVars, unsigned int tilLevel, float alpha, float beta);
		bool											tryDataBase						(	   knotStruct& knot, const runAlphaBetaVars& rabVars, unsigned int tilLevel, unsigned int &layerNumber, unsigned int &stateNumber);
		bool											tryPossibilities				(	   knotStruct& knot, 	   runAlphaBetaVars& rabVars, unsigned int tilLevel, unsigned int &maxWonfreqValuesSubMoves, float &alpha, float &beta);
		bool											saveInDatabase					(const knotStruct& knot, 	   runAlphaBetaVars& rabVars, unsigned int layerNumber, unsigned int stateNumber);

		// static thread functions
		static DWORD									initThreadProc					(void* pParameter, int64_t index);		// used to initialize the database calculation
		static DWORD									runThreadProc					(void* pParameter, int64_t index);		// used to run the database calculation
		static DWORD									minMaxThreadProc				(void* pParameter, int64_t index);		// used to run the min-max calculation without database
	};

} // namespace alphaBeta

} // namespace miniMax
