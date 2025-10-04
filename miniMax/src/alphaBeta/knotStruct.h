/*********************************************************************\
	knotStruct.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/
#pragma once

#include "miniMax/src/typeDef.h"

namespace miniMax
{

namespace alphaBeta
{
	// this represents a state of the game
	struct knotStruct
	{
		bool						playerToMoveChanged					= true;						// the player to move switched compared to the parent/origin knot/state
		float						floatValue							= 0.0f;						// Value of knot (for normal mode). must be symmetric around 0 from the view of the current player
		twoBit						shortValue							= SKV_VALUE_INVALID;		// Value of knot (for database)
		unsigned int				bestMoveId							= 0;						// for calling class
		unsigned int				numPossibilities					= 0;						// number of branches - differs from possibilityIds.size() in case of cut off
		plyInfoVarType				plyInfo								= 0;						// number of moves till win/lost
		knotStruct*					branches							= nullptr;					// pointer to branches, in sync with possibilityIds
		unsigned int				freqValuesSubMoves[SKV_NUM_VALUES] 	= {0,0,0,0};				// number of branches leading to a state with a certain value, from the perspective of the current player
		vector<unsigned int>		possibilityIds;													// filled by game->getPossibilities(); contains IDs for all possible moves, 
																									// while 'branches' points to the corresponding knotStructs for moves that are actually explored (may differ in size if cutoffs occur)

		bool                        initForCalculation                  (knotStruct* branchArray);
		void						setInvalid							();
		bool						calcPlyInfo							();
		bool						calcKnotValue						();
		bool 						getBestBranchesBasedOnSkvValue		(vector<unsigned int>& bestBranches);
		bool						getBestBranchesBasedOnFloatValue	(vector<unsigned int>& bestBranches);
		bool 						getInfoAboutChoices					(stateInfo& infoAboutChoices);
		bool						increaseFreqValuesSubMoves			(unsigned int curPoss);
		bool						canCutOff							(unsigned int curPoss, float& alpha, float& beta);
	};
}}
