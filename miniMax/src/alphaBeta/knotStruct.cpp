/*********************************************************************
	knotStruct.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/

#include "knotStruct.h"

//-----------------------------------------------------------------------------
// Name: initForCalculation()
// Desc: Initialize knot for calculation
//-----------------------------------------------------------------------------
bool miniMax::alphaBeta::knotStruct::initForCalculation(knotStruct* branchArray)
{
	if (branchArray == nullptr) return false;
	
	branches										= branchArray;
	numPossibilities								= 0;
	bestMoveId										= 0;
	plyInfo											= PLYINFO_VALUE_UNCALCULATED;
	shortValue										= SKV_VALUE_GAME_DRAWN;
	floatValue										= skvFloatValueMap[shortValue];
	freqValuesSubMoves[SKV_VALUE_INVALID		] 	= 0;
	freqValuesSubMoves[SKV_VALUE_GAME_LOST		] 	= 0;
	freqValuesSubMoves[SKV_VALUE_GAME_DRAWN		] 	= 0;
	freqValuesSubMoves[SKV_VALUE_GAME_WON		] 	= 0;
	return true;
}

//-----------------------------------------------------------------------------
// Name: setInvalid()
// Desc: Set knot to invalid state
//-----------------------------------------------------------------------------
void miniMax::alphaBeta::knotStruct::setInvalid()
{
	shortValue		= SKV_VALUE_INVALID;
	plyInfo			= PLYINFO_VALUE_INVALID;
	floatValue		= skvFloatValueMap[shortValue];
}

//-----------------------------------------------------------------------------
// Name: increaseFreqValuesSubMoves()
// Desc: Increase freqValuesSubMoves according to knot value of the chosen branch by 'curPoss'
//		 Required are: branches[i].shortValue
//					   branches[i].playerToMoveChanged
//					   numPossibilities
//		 Output: 	   freqValuesSubMoves
//-----------------------------------------------------------------------------
bool miniMax::alphaBeta::knotStruct::increaseFreqValuesSubMoves(unsigned int curPoss)
{
	if (curPoss >= numPossibilities) {
		return false;
	}
	twoBit skv = (branches[curPoss].playerToMoveChanged ? skvPerspectiveMatrix[branches[curPoss].shortValue][PL_TO_MOVE_CHANGED] : branches[curPoss].shortValue);
	freqValuesSubMoves[skv]++;
	return true;
}

//-----------------------------------------------------------------------------
// Name: getInfoAboutChoices()
// Desc: Fills infoAboutChoices with information about choices
//		 Required are: possibilityIds
//					   branches[i].shortValue
//					   branches[i].plyInfo
//					   branches[i].playerToMoveChanged
//					   branches[i].freqValuesSubMoves
//					   numPossibilities
//					   shortValue
//		 Output: 	   infoAboutChoices
//-----------------------------------------------------------------------------
bool miniMax::alphaBeta::knotStruct::getInfoAboutChoices(stateInfo& infoAboutChoices)
{
	infoAboutChoices.plyInfo			= plyInfo;
	infoAboutChoices.shortValue			= shortValue;
	infoAboutChoices.choices.resize(possibilityIds.size());
	for (unsigned int i = 0; i < possibilityIds.size(); i++) {
		infoAboutChoices.choices[i].possibilityId	= possibilityIds[i];
		infoAboutChoices.choices[i].shortValue		= (branches[i].playerToMoveChanged ? skvPerspectiveMatrix[branches[i].shortValue][PL_TO_MOVE_CHANGED] : branches[i].shortValue);
		infoAboutChoices.choices[i].plyInfo			= branches[i].plyInfo;
		infoAboutChoices.choices[i].freqValuesSubMoves[SKV_VALUE_INVALID	] = branches[i].freqValuesSubMoves[SKV_VALUE_INVALID	];
		infoAboutChoices.choices[i].freqValuesSubMoves[SKV_VALUE_GAME_LOST	] = branches[i].freqValuesSubMoves[SKV_VALUE_GAME_LOST	];
		infoAboutChoices.choices[i].freqValuesSubMoves[SKV_VALUE_GAME_DRAWN	] = branches[i].freqValuesSubMoves[SKV_VALUE_GAME_DRAWN	];
		infoAboutChoices.choices[i].freqValuesSubMoves[SKV_VALUE_GAME_WON	] = branches[i].freqValuesSubMoves[SKV_VALUE_GAME_WON	];
	}
	return true;
}

//-----------------------------------------------------------------------------
// Name: calcKnotValue()
// Desc: Calculates floatValue and shortValue of knot based on branches
//		 Required are: branches[i].shortValue 
//					   branches[i].floatValue
//					   branches[i].playerToMoveChanged
//					   numPossibilities
//		 Output: 	   floatValue
//-----------------------------------------------------------------------------
bool miniMax::alphaBeta::knotStruct::calcKnotValue()
{
	// check
	if (numPossibilities == 0) return false;
	if (branches == nullptr) return false;

	// locals
	shortValue		= SKV_VALUE_INVALID;
	floatValue		= FPKV_INV_VALUE;
	twoBit			skv; 
	float			fv;
	unsigned int	curPoss;
			
	// maximize the value
	for (curPoss=0; curPoss<numPossibilities; curPoss++) {

		// do not use invalid states
		if (branches[curPoss].shortValue == SKV_VALUE_INVALID) {
			continue;
		}

		// get perspekvtive corrective value from current considered branch
		if (branches[curPoss].playerToMoveChanged) {
			skv = skvPerspectiveMatrix[branches[curPoss].shortValue][PL_TO_MOVE_CHANGED];
			fv	= -1.0f * branches[curPoss].floatValue;				// due to this negation the floatvalue must be symmetric around zero
		} else {
			skv = branches[curPoss].shortValue;
			fv	= branches[curPoss].floatValue;
		}

		// maximize skv
		if (skv > shortValue) {
			shortValue	= skv;
		}

		// maximize float value
		if (fv > floatValue) {
			floatValue = fv;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// Name: calcPlyInfo()
// Desc: Calculates plyInfo of knot based on branches
//		 Required are: branches[i].shortValue
//					   branches[i].plyInfo
//					   branches[i].playerToMoveChanged
//					   numPossibilities
//					   shortValue
//		 Output: 	   plyInfo
//-----------------------------------------------------------------------------
bool miniMax::alphaBeta::knotStruct::calcPlyInfo()
{
	// checks
	if (numPossibilities == 0) return false;
	if (branches == nullptr) return false;

	// locals
	unsigned int	maxBranch;
	plyInfoVarType  maxPlyInfo;

	// ignore invalid and drawn states
	if (shortValue == SKV_VALUE_GAME_DRAWN) {
		plyInfo = PLYINFO_VALUE_DRAWN;
	} else if (shortValue == SKV_VALUE_INVALID) {
		plyInfo = PLYINFO_VALUE_INVALID;
	} else {

		// calculate ply info of knot
        maxPlyInfo		= (shortValue == SKV_VALUE_GAME_WON) ? PLYINFO_VALUE_DRAWN : 0;
		maxBranch		= 0;

		// when current knot is a won state
		if (shortValue == SKV_VALUE_GAME_WON) {

			for (unsigned int i=0; i<numPossibilities; i++) {

				// take the minimum ply of the lost states, when the opponent moved last
				if ((branches[i].plyInfo < maxPlyInfo && branches[i].shortValue == SKV_VALUE_GAME_LOST &&  branches[i].playerToMoveChanged)

				// after this move the same player will continue, so take the minimum ply of the won states
				||  (branches[i].plyInfo < maxPlyInfo && branches[i].shortValue == SKV_VALUE_GAME_WON  && !branches[i].playerToMoveChanged)) {

					maxPlyInfo	= branches[i].plyInfo;
					maxBranch	= i;
				}
			}

		// current state is a lost state
		} else {

			for (unsigned int i=0; i<numPossibilities; i++) {

				// take the maximum of the won states (won for the opponent), since that's the longest path for him
				if ((branches[i].plyInfo > maxPlyInfo && branches[i].shortValue == SKV_VALUE_GAME_WON  &&  branches[i].playerToMoveChanged)

				// after this move the same player will continue, so take the maximum ply of the lost states
				||  (branches[i].plyInfo > maxPlyInfo && branches[i].shortValue == SKV_VALUE_GAME_LOST && !branches[i].playerToMoveChanged)) {
					
					maxPlyInfo	= branches[i].plyInfo;
					maxBranch	= i;
				}
			}
		}

		// set value
		plyInfo = branches[maxBranch].plyInfo + 1;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Name: getBestBranchesBasedOnSkvValue()
// Desc: select randomly one of the best moves, if they are equivalent
//		 Required are: 	branches[i].shortValue
//					   	branches[i].plyInfo
//					   	branches[i].playerToMoveChanged
//					   	numPossibilities
//					   	shortValue
//						plyInfo
//		 Output:	   	bestBranches: indices of the best branches
//-----------------------------------------------------------------------------
bool miniMax::alphaBeta::knotStruct::getBestBranchesBasedOnSkvValue(vector<unsigned int>& bestBranches)
{
	// checks
	if (numPossibilities == 0) return false;
	if (branches == nullptr) return false;

	bestBranches.clear();
	bestBranches.reserve(numPossibilities);

	// check every possible move
	for (unsigned int curPoss=0; curPoss<numPossibilities; curPoss++) {

		// value of selected move is equal to knot value
		twoBit branchSkv = (branches[curPoss].playerToMoveChanged ? skvPerspectiveMatrix[branches[curPoss].shortValue][PL_TO_MOVE_CHANGED] : branches[curPoss].shortValue);
		if (branchSkv == shortValue) {

			// best move lead to drawn state
			if (shortValue == SKV_VALUE_GAME_DRAWN) {
				bestBranches.push_back(curPoss);

			// best move lead to lost or won state
			} else {
				if (plyInfo == branches[curPoss].plyInfo + 1) {
					bestBranches.push_back(curPoss);
				}
			}
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// Name: getBestBranchesBasedOnFloatValue()
// Desc: select randomly one of the best moves, if they are equivalent
//	     Parameters:	useSkvValue: true if skvValue should be used, false if floatValue should be used
//		 Required are: 	branches[i].playerToMoveChanged
//					   	branches[i].floatValue
//					   	numPossibilities
//		                floatValue
//		 Output:	   	bestBranches: indices of the best branches
//-----------------------------------------------------------------------------
bool miniMax::alphaBeta::knotStruct::getBestBranchesBasedOnFloatValue(vector<unsigned int>& bestBranches)
{
	// checks
	if (numPossibilities == 0) return false;
	if (branches == nullptr) return false;

	bestBranches.reserve(numPossibilities);

	// check every possible move
	for (unsigned int curPoss=0; curPoss<numPossibilities; curPoss++) {

		// skip branches leading to invalid states
		if (branches[curPoss].floatValue <= FPKV_INV_VALUE + FPKV_THRESHOLD) {
			continue;
		}

		// conventionell mini-max algorithm
		float dif;
		float fPlayerToMoveChanged = (branches[curPoss].playerToMoveChanged) ? -1.0f : 1.0f;
		dif = fPlayerToMoveChanged * branches[curPoss].floatValue - floatValue;
		dif = (dif > 0) ? dif : -1.0f * dif;
		if (dif < FPKV_THRESHOLD) {
			bestBranches.push_back(curPoss);
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// Name: canCutOff()
// Desc: returns true if a cut off (ignoring further possible moves) can be used
//-----------------------------------------------------------------------------
bool miniMax::alphaBeta::knotStruct::canCutOff(unsigned int curPoss, float & alpha, float & beta)
{
	if (!branches[curPoss].playerToMoveChanged) {
		if (branches[curPoss].floatValue >= beta ) {
			numPossibilities = curPoss + 1;
			return true;
		} else if (branches[curPoss].floatValue >  alpha) {
			alpha = branches[curPoss].floatValue;
		}
	} else {
		if (branches[curPoss].floatValue <= alpha) {
			numPossibilities = curPoss + 1;
			return true;
		} else if (branches[curPoss].floatValue <  beta ) {
			beta	= branches[curPoss].floatValue;
		}
	}
	return false;
}
