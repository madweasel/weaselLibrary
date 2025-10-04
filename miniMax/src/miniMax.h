/***************************************************************************************************************************
	miniMax.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
***************************************************************************************************************************/
#ifndef MINIMAX_H
#define MINIMAX_H

// win api
#include <windows.h>
#include <Shlwapi.h>

// std
#include <sstream>
#include <iostream>
#include <cstdio>
#include <list>
#include <intrin.h>
#include <ctime>
#include <vector>
#include <algorithm>
#include <filesystem>

// weasel library
#include "weaselEssentials/src/strLib.h"
#include "weaselEssentials/src/threadManager.h"
#include "weaselEssentials/src/logger.h"

// other miniMax headers
#include "typeDef.h"
#include "alphaBeta/alphaBeta.h"
#include "database/database.h"
#include "retroAnalysis/retroAnalysis.h"
#include "statistics/statistics.h"
#include "integrity/integrityChecker.h"

#pragma intrinsic(_rotl8, _rotr8)							// for shifting bits

using namespace std;										// use standard library namespace

/*** Explanation ***************************************************************************************************************************
player:					Either ego or opponent. The player to move is the current player.
layer:					The states are divided in layers. For example depending on number of stones on the field.
state:					A unique game state reprensiting a current game situation.
situation:				Used as synonym to state.
knot:					Each knot of the graph corresponds to a game state. The knots are connected by possible valid moves.
ply info:				Number of plies/moves necessary to win the game.
state adress:			A state is identified by the corresponding layer and the state number within the layer.
short knot value:		Each knot/state can have the value SKV_VALUE_INVALID, SKV_VALUE_GAME_LOST, SKV_VALUE_GAME_DRAWN or SKV_VALUE_GAME_WON.
float point knot value:	Each knot/state can be evaluated by a floating point value. High positive values represents winning situations. Negative values stand for loosing situations.
database:				The database contains the arrays with the short knot values and the ply infos.
********************************************************************************************************************************************/

namespace miniMax
{

/**
 * @brief The miniMax class manages the minimax algorithm and related operations for game state evaluation.
 *
 * Responsibilities:
 * - Interfaces with the game logic to perform minimax and alpha-beta pruning searches.
 * - Manages the database of evaluated game states and their values.
 * - Coordinates multi-threaded calculations and progress reporting.
 * - Provides functions for querying best moves, statistics, and integrity checks.
 *
 * Usage:
 * - Instantiate with a gameInterface pointer and desired search depth.
 * - Use public methods to open/calculate databases, get best choices, and manage threading.
 */
class miniMax
{
	friend class			miniMaxWinInspectDb;
	friend class			miniMaxWinCalcDb;
	friend class			miniMaxDbCompTrans;
	friend class			statistics::monitor;
	
private:
	gameInterface*			game							= nullptr;
	logger 					log;
	
public:

	// Constructor / destructor
							miniMax							(gameInterface* game, unsigned int maxAlphaBetaSearchDepth);
							~miniMax						();	
	
	database::database		db;
	integrity::checker		checker;
	statistics::monitor		monitor;

	// getter
	unsigned int			getNumThreads					();
	bool					anyFreshlyCalculatedLayer		();

	// Functions for getting the best choice
	bool 					getBestChoice					(unsigned int& choice, stateInfo& infoAboutChoices);
	void					setSearchDepth					(unsigned int maxAlphaBetaSearchDepth);

	// Database functions
	bool					openDatabase					(wstring const& directory, bool useCompFileIfBothExist = true);
	bool					calculateDatabase           	();
	bool 					calculateStatistics				();
	bool					isCurrentStateInDatabase		(unsigned int threadNo);
	void					unloadDatabase					();
	void					closeDatabase					();
	void					pauseDatabaseCalculation		();
	void					cancelDatabaseCalculation		();
	bool					wasDatabaseCalculationCancelled	();
	unsigned int 			getLastCalculatedLayer			();
	bool					setOutputStream					(wostream& theStream);
	bool 					setNumThreads					(unsigned int numThreads);

private:

	// variables that typically remain unchanged during database calculation
	wstring					fileDirectory;									// path of the folder where the database files are located
	list<unsigned int>		lastCalculatedLayer;							// list of the recently calculated layers
	vector<unsigned int> 	layersToCalculate;								// layers to calculate, in case mutliple layers must be calculated at once
	threadManagerClass		threadManager;									// thread manager for multi-threading
	CRITICAL_SECTION		csOsPrint;										// for thread safety when output is passed to osPrint
	
	// solvers
	alphaBeta::solver		abSolver;
	retroAnalysis::solver	rtSolver;

	// thread specific or non-constant variables
	long long				numStatesProcessed				= 0;			// number of states processed by all threads 
	unsigned int			curCalculatedLayer				= 0;			// id of the currently calculated layer

	// Progress report functions
	bool					calcLayer						(unsigned int layerNumber);
	void					setCurrentActivity				(activity newAction);
};

} // namespace miniMax

#endif // MINIMAX_H
