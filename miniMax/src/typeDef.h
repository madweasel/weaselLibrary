/***************************************************************************************************************************
	miniMax.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
***************************************************************************************************************************/
#pragma once

#include <vector>
#include <string>
#include <windows.h>

#include "weaselEssentials/src/logger.h"

namespace miniMax
{
using namespace std;
	
/*** Macros ***************************************************************************************************************************/
#define SAFE_DELETE(p)			{ if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p)	{ if(p) { delete[] (p);   (p)=NULL; } }

/*** typedefines ***************************************************************************************************************************/
typedef unsigned char		twoBit;											// 2-Bit variable ranging from 0 to 3
typedef unsigned short		plyInfoVarType;									// 2 Bytes for saving the ply info
typedef unsigned int		stateNumberVarType;								// 4 Bytes for addressing states within a layer
															
/*** Constants ***************************************************************************************************************************/
const float					FPKV_INV_VALUE					= -100001.0f;	// minimum float point knot value
const float					FPKV_MIN_VALUE					= -100000.0f;	// minimum float point knot value
const float					FPKV_MAX_VALUE					= 100000.0f;	// maximum float point knot value
const float					FPKV_THRESHOLD					= 0.001f;		// threshold used when choosing best move. knot values differing less than this threshold will be regarded as egal

const size_t				SKV_VALUE_INVALID               = 0;			// short knot value: knot value is invalid
const size_t				SKV_VALUE_GAME_LOST				= 1;			// game lost means that there is no perfect move possible
const size_t				SKV_VALUE_GAME_DRAWN         	= 2;			// the perfect move leads at least to a drawn game
const size_t				SKV_VALUE_GAME_WON				= 3;			// the perfect move will lead to a won game
const size_t				SKV_NUM_VALUES					= 4;			// number of different short knot values
const size_t				SKV_WHOLE_BYTE_IS_INVALID		= 0;			// four short knot values are stored in one byte. so all four knot values are invalid

const size_t				PLYINFO_EXP_VALUE				= 1000;			// expected maximum number of plies -> used as size for vector initialization
const plyInfoVarType		PLYINFO_VALUE_DRAWN				= 65001;		// knot value is drawn. since drawn means a never ending game, this is a special ply info
const plyInfoVarType		PLYINFO_VALUE_UNCALCULATED		= 65002;		// ply info is not calculated yet for this game state
const plyInfoVarType		PLYINFO_VALUE_INVALID			= 65003;		// ply info is invalid, since knot value is invalid

#ifdef _DEBUG
	const long long			OUTPUT_EVERY_N_STATES			= 1000000;		// print progress every n-th processed knot
#else
	const long long			OUTPUT_EVERY_N_STATES			= 10000000;		// print progress every n-th processed knot
#endif
const size_t				BLOCK_SIZE_IN_CYCLIC_ARRAY		= 10000;		// BLOCK_SIZE_IN_CYCLIC_ARRAY*sizeof(stateAdressStruct) = block size in bytes for the cyclic arrays
const size_t				MAX_NUM_PREDECESSORS			= 10000;		// maximum number of predecessors. important for array sizes
const size_t				FILE_BUFFER_SIZE				= 1000000;		// size in bytes

const size_t				PL_TO_MOVE_CHANGED              = 0;			// player to move changed			- first  index of the 2D-array skvPerspectiveMatrix[][]
const size_t				PL_TO_MOVE_UNCHANGED            = 1;			// player to move is still the same - second index of the 2D-array skvPerspectiveMatrix[][]

const long long				MEASURE_TIME_FREQUENCY			= 100000;		// for io operations per second: measure time every n-th operations
const bool					MEASURE_IOPS					= false;		// true or false - for measurement of the input/output operations per second

const unsigned char 		skvPerspectiveMatrix[SKV_NUM_VALUES][2] =	  	// [short knot value][PL_TO_MOVE_UNCHANGED/PL_TO_MOVE_CHANGED] - A winning situation is a loosing situation for the opponent and so on ...
{
  //  PL_TO_MOVE_CHANGED	   PL_TO_MOVE_UNCHANGED
	{ SKV_VALUE_INVALID,       SKV_VALUE_INVALID,    },   // SKV_VALUE_INVALID    
	{ SKV_VALUE_GAME_WON,      SKV_VALUE_GAME_LOST,  },   // SKV_VALUE_GAME_LOST	
	{ SKV_VALUE_GAME_DRAWN,    SKV_VALUE_GAME_DRAWN, },   // SKV_VALUE_GAME_DRAWN 
	{ SKV_VALUE_GAME_LOST,     SKV_VALUE_GAME_WON    }    // SKV_VALUE_GAME_WON	
};

const float					skvFloatValueMap[SKV_NUM_VALUES] =				// map for float values of short values
{
	FPKV_MIN_VALUE - 1.0f,		// SKV_VALUE_INVALID
	FPKV_MIN_VALUE,				// SKV_VALUE_GAME_LOST
	0,							// SKV_VALUE_GAME_DRAWN
	FPKV_MAX_VALUE				// SKV_VALUE_GAME_WON
};

/*** Enums ***************************************************************************************************************************/
enum class					activity						{initRetroAnalysis, prepareCountArray, performRetroAnalysis, performAlphaBeta, testingLayer, savingLayerToFile, loadingLayerFromFile, calcLayerStats, none};
static activity				curAction						= activity::none;	// globally defined current action

/*** Classes ***************************************************************************************************************************/
struct stateAdressStruct									// address of a state/knot within the database, representing a unique state. In the real game there might be identical ones due to symmetry.
{
	stateNumberVarType		stateNumber;					// state id within the corresponding layer
	unsigned char			layerNumber;					// layer id

	bool 					operator==						(const stateAdressStruct& rhs) const;
	bool 					operator<						(const stateAdressStruct& rhs) const;
	bool 					operator>						(const stateAdressStruct& rhs) const;
};

struct possibilityInfo										// information about a possible move
{
	unsigned int			possibilityId;					// id of the possibility returned by gameInterface::getPossibilities()
	twoBit					shortValue;						// value of the knot after the move, from the perspective of the current player
	plyInfoVarType			plyInfo;						// number of plies to win or lose
	unsigned int			freqValuesSubMoves[SKV_NUM_VALUES];		// number of branches leading to a state with a certain value, from the perspective of the current player
};

struct stateInfo											// information about a state/knot
{
	vector<possibilityInfo>	choices;						// possible moves. thereby the value of a choice is from the perspective of the current player.
	twoBit					shortValue;						// value of the knot
	plyInfoVarType			plyInfo;						// number of plies till win/lost
	unsigned int			bestAmountOfPlies;				// best amount of plies to win or lose

	void   					updateBestAmountOfPlies();
};

namespace retroAnalysis
{
	struct predVars											// variables describing a preceeding state
	{
		unsigned int  		predStateNumber;				// state number of the preceeding state
		unsigned int  		predLayerNumber;				// layer number of the preceeding state
		unsigned int  		predSymOperation;				// symmetry operation number, which was applied to the preceeding state
		bool          		playerToMoveChanged;			// true if the player to move has changed, false if it is still the same
	};
}

class gameInterface
{
public:
	// types
	using uint_1d 			= vector<unsigned int>;

	// init
	virtual void			prepareCalculation				()																													{ 					};	// is called before any database calculation, any test, any getBestChoice call

	// getter
    virtual bool            shallRetroAnalysisBeUsed    	(unsigned int layerNum)																								{ return false;		};	// selects either the alphaBeta or the retroAnalysis algorithmn for each layer
	virtual void			getPossibilities				(unsigned int threadNo, vector<unsigned int>& possibilityIds)														{ 					};	// returns the possible move ids for the current player (which can be ego and opponent)
	virtual unsigned int	getMaxNumPossibilities			()																													{ return 0;			};	// returns the maximum number of possibilities for a move
	virtual unsigned int	getNumberOfLayers				()																													{ return 0;			};	// total number of layers
	virtual unsigned int	getNumberOfKnotsInLayer			(unsigned int layerNum)																								{ return 0;			};	// total number of knots for a given layer
	virtual unsigned int 	getMaxNumPlies					()																													{ return 0;			};	// maximum number of plies for the game, although this value is not known in advance, an upper boundary value must be provided for the alphaBeta algorithmn
    virtual void            getSuccLayers               	(unsigned int layerNum, vector<unsigned int>& succLayers)															{ 					};	// layers which can be reached by a move from each knot of the passed layer
	virtual uint_1d			getPartnerLayers				(unsigned int layerNum)																								{ return {};		};	// the partner layers are calculated together at the same time
	virtual void			getValueOfSituation				(unsigned int threadNo, float& floatValue, twoBit& shortValue)														{ 					};	// value of situation for the current player (which can be ego and opponent)
	virtual void			getLayerAndStateNumber			(unsigned int threadNo, unsigned int& layerNum, unsigned int& stateNumber, unsigned int& symOp)						{ 					};	// returns the address of the current considered state for each thread for the view of the current player (which can be ego and opponent)
	virtual unsigned int	getLayerNumber					(unsigned int threadNo)																								{ return 0;			};	// gets the layer number for the state, which is active for each thread
	virtual void			getSymStateNumWithDuplicates	(unsigned int threadNo, vector<stateAdressStruct>& symStates)														{ 					};	// get all the symmetric states from the current one, since all of them have the same knot value
    virtual void            getPredecessors             	(unsigned int threadNo, vector<retroAnalysis::predVars>& predVars)													{ 					};	// get all preceeding states for retro analysis
	virtual bool			isStateIntegrityOk				(unsigned int threadNo)																								{ return false;		};	// do some checks if the state variables are consistent to each other
	virtual void			applySymOp						(unsigned int threadNo, unsigned char symmetryOperationNumber, bool doInverseOperation, bool playerToMoveChanged)	{					};  // apply this (inverse) symmetry operation on the current state of a certain thread
	virtual bool			lostIfUnableToMove				(unsigned int threadNo)																								{ return false;		};	// does it mean that the game is lost, when unable to move?

	// setter
	virtual bool			setSituation					(unsigned int threadNo, unsigned int layerNum, unsigned int stateNumber)											{ return false;		};	// set a certain game state for a thread. even if the state is invalid, the function should set the state and return false 
	virtual void			move							(unsigned int threadNo, unsigned int idPossibility, bool& playerToMoveChanged, void* &pBackup)						{ 					};	//   do a move based on the ids returned by getPossibilities() 
	virtual void			undo							(unsigned int threadNo, unsigned int idPossibility, bool& playerToMoveChanged, void*  pBackup)						{ 					};	// undo a move based on the ids returned by getPossibilities() 
	
	// output
	virtual void			printField						(unsigned int threadNo, twoBit value, unsigned int indentSpaces = 0)												{ 					};	// for console
	virtual void			printMoveInformation			(unsigned int threadNo, unsigned int idPossibility)																	{ 					};	// for console
	virtual wstring			getOutputInformation			(unsigned int layerNum)																								{ return wstring(L"");};	// for gui
};

class returnValues
{
public:
	static const bool		stopOnCriticalError				= false;	// if true, the program will stop on critical errors

	static bool				falseOrStop						();
};	

class progressCounter
{
	int64_t					statesProcessedByThisThread			= 0;					// precise number of processed states by this thread
	int64_t&  				roughTotalNumStatesProcessed;								// roughly number of total processed states by all threads

public:	
							progressCounter						(int64_t& roughTotalNumStatesProcessed);

	void					stateProcessed						(logger& log, stateNumberVarType numKnotsInLayer, const wstring& text);			// called when a state is processed
	int64_t					getStatesProcessedByThisThread		() const;
};

/*** pre-declaration ***************************************************************************************************************************/
class miniMax;
class gameInterface;

namespace database {
	class database;
}

namespace retroAnalysis {
	class solver;
}

namespace alphaBeta {
	class solver;
}

namespace statistics {
	class monitor;
}

namespace test {
	class tester;
}

} // namespace miniMax
