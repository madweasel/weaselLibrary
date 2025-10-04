/*********************************************************************
	backTracking.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/

#include "backTracking.h"

//-----------------------------------------------------------------------------
// Name: searchSolution()
// Desc: entry point of the algorithmn
//-----------------------------------------------------------------------------
void backTracking::searchSolution(list<solutionPath> *solList, unsigned int searchDepth, unsigned int numSolutionsToFind)
{
	// Locals
	maxSearchDepth			= searchDepth;
	numSolToFind			= numSolutionsToFind;
	curPath					.resize(maxSearchDepth);
	curDepth				= 0;
	solutionList			= solList;
	abortCalculation		= false;

	if (solutionList != NULL) solutionList->clear();

	// catch invalid parameters
	if (numSolToFind == 0) numSolToFind = 1;
	
	// valid search depth ?
	if (maxSearchDepth > 0) {

		// prepare the situation
		setBeginningSituation();

		// begin search
		tryNextStep();

		// free memory
		curPath.clear();
	}
}
	
//-----------------------------------------------------------------------------
// Name: tryNextStep()
// Desc: 
//-----------------------------------------------------------------------------
bool backTracking::tryNextStep()
{
	// Locals
	void *			pBackup				= nullptr;
	unsigned int	numPossibilities	= 0;
	unsigned int *	idPossibility		= nullptr;
	solutionPath    mySolution;
	void *			pPossibilities;
	unsigned int	i;

	// get possiblities
	idPossibility = getPossibilities(&numPossibilities, &pPossibilities);

	// unable to move
	if (numPossibilities == 0)  {
		
	// recursive call
	} else {
		for (i=0; i<numPossibilities; i++) {
				
			// move
			move(idPossibility[i], &pBackup, pPossibilities);
			curPath[curDepth] = idPossibility[i];
			curDepth++;

			// solution found ?
			if (targetReached()) {

				// savePath
				if (solutionList != NULL) {
					mySolution.path		.resize(curDepth);
					mySolution.numSteps	= curDepth;
 					mySolution.path.assign(curPath.begin(), curPath.begin() + curDepth);
					solutionList->push_back(mySolution);
				}

				numSolToFind--;
			
			// recursive call
			} else if (!abortCalculation) {
				if (curDepth < maxSearchDepth) tryNextStep();
			}

			// undo move
			curDepth--;
			undo(idPossibility[i], pBackup, pPossibilities);

			// break if desired
			if (numSolToFind == 0)	break;
			if (abortCalculation)	break;
		}
	}

	// let delete pPossibilities
	idPossibility = deletePossibilities(pPossibilities);

	// free memory
	if (idPossibility != NULL) {
		delete [] idPossibility;
		idPossibility = nullptr;
	}
	
	// continnue
	return true;
}