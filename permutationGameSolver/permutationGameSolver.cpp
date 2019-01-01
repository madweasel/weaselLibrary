/*********************************************************************
	permutationGameSolver.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/

#include "permutationGameSolver.h"

#pragma region stuff
//-----------------------------------------------------------------------------
// Name: targetReached()
// Desc: Returns true, when desired state is reached.
//-----------------------------------------------------------------------------
bool permutationGameSolver::targetReached()
{
	// locals
	unsigned int numBallsUnequal = 0;
	unsigned int i, j;

	// look at field
	for (i=0; i<fieldSize; i++) {

		// square is in which zone type?
		switch (zones[i])
		{
		case PGS_SS_REL:
			// changes ?
			if (curState.field[i] != cmpField[i]) numBallsUnequal++;

			// quit when there is no way any more
			if (numBallsUnequal > maxDiffInRelZone) 
				return false;

			break;

		case PGS_SS_VAR:
			break;

		case PGS_SS_FIX:

			// when a fixed square is changed then target not reached
			if (curState.field[i] != cmpField[i])
				return false;
			break;
		}
	}

	// when less then x balls are changed then target is reached
	if (numBallsUnequal >= minDiffInRelZone) {

		// write new solution in "newFoundSeq"
		newFoundSeq.solPath.numSteps	= curDepth;
		newFoundSeq.solPath.path.assign(curPath.begin(), curPath.begin() + curDepth);
		
		if (searchingSequences) {
			for (i=0; i<fieldSize; i++) newFoundSeq.indicesMap[curState.field[i]] = i;

		} else {
			// init indices map
			for (i=0; i < fieldSize; i++) newFoundSeq.indicesMap[i] = i;

			// apply steps
			for (i=0; i<newFoundSeq.solPath.numSteps; i++) {
				tmpField.assign(newFoundSeq.indicesMap.begin(), newFoundSeq.indicesMap.begin() + fieldSize);
				for (j=0; j<fieldSize; j++) {
					newFoundSeq.indicesMap[indicesMaps[newFoundSeq.solPath.path[i] * fieldSize + j]] = tmpField[j];
				}
			}

			// calc indices Map
			tmpField.assign(newFoundSeq.indicesMap.begin(), newFoundSeq.indicesMap.begin() + fieldSize);
			for (j=0; j<fieldSize; j++) { 
				for (i=0; i<fieldSize; i++) if (tmpField[i] == j) break;
				newFoundSeq.indicesMap[j] = i;
			}
		}

		// vanish double sequences
		if (searchingSequences && removeSeqDuplicates) { 
			if (!putInListIfItIsNotDuplicate(foundSequences, &newFoundSeq, nullptr)) return false; 
		} else {
			foundSequences.push_back(new sequenceStruct(newFoundSeq));
		}

		return (useMaxSearchDepth?false:true);
	} else {
		return false;
	}
}

//-----------------------------------------------------------------------------
// Name: setBeginningSituation()
// Desc: 
//-----------------------------------------------------------------------------
void permutationGameSolver::setBeginningSituation()
{
}

//-----------------------------------------------------------------------------
// Name: getPossibilities()
// Desc: 
//-----------------------------------------------------------------------------
unsigned int *permutationGameSolver::getPossibilities(unsigned int *numPossibilities, void  **pPossibilities)
{
	// locals
	bool		 dfMoveAllowed;
	unsigned int curSeq, i;
    unsigned int *idPossibility = &idPossibilities[curDepth * numSequences];

	// no special information needed to pass
	*pPossibilities		= NULL;
	*numPossibilities	= 0;

	// test each move
	for (curSeq=0; curSeq<numSequences; curSeq++) {

		// normally a forward move
		if (useDoubleFeature) dfMoveIsForward[curDepth*numSequences+curSeq] = true;

		// was the last move the inverse one ?
		if (curState.lastSeq == inverseSeq[curSeq]) continue;

		// forbidden move ?
		if (curState.blockedSeq[curSeq] == PGS_BS_BLOCKED) continue;

		// excluded move, while using user function to calculate the unallowed sequences
		if (seqBlUser.setBlockedSequences != NULL) {
			if (seqBlUser.excludedMoves[curSeq] != 0) continue;
		}

		// single return feature
		if (useSingleReturn) {
			if (maxSearchDepth - curDepth <= srNumMovesOpen) {
				if (srIsOpenMove[inverseSeq[curSeq]] == 0) {
					continue;
				}
			}
		}

		// double feature ?
		if (useDoubleFeature) {
			
			// look in each square if move is allowed
			for (dfMoveAllowed = false, i=0; i<fieldSize; i++) {

				// does move change current square "i" ?
				if (dfCurIndex[i] > 0 && indicesMaps[curSeq * fieldSize + i] != i) {

					// only interesting if the stone is FIX
					if (zones[i] == PGS_SS_FIX) {
					
						// move only allowed when it is the current needed inverse move
						if (dfList[(dfCurIndex[i]-1) * fieldSize + i] != inverseSeq[curSeq]) {
							dfMoveAllowed = false;
							break;
						} else {
							dfMoveAllowed=true;
						}
					}
				}
			}

			// when move not allowed continue
			if (!dfMoveAllowed && (maxSearchDepth - curDepth <= dfNumForwardMoves)) {
				continue;
			// otherwise mark him as "reverse move"
			} else if (dfMoveAllowed) {
				dfMoveIsForward[curDepth*numSequences+curSeq] = false;
			}
		}

		// move is possible
		idPossibility[*numPossibilities] =  curSeq;
		(*numPossibilities)++;
	}

	return idPossibility;
}

//-----------------------------------------------------------------------------
// Name: deletePossibilities()
// Desc: 
//-----------------------------------------------------------------------------
unsigned int *permutationGameSolver::deletePossibilities(void *pPossibilities)
{
    return NULL;
}

//-----------------------------------------------------------------------------
// Name: undo()
// Desc: 
//-----------------------------------------------------------------------------
void permutationGameSolver::undo(unsigned int idPossibility, void *pBackup, void *pPossibilities)
{
	// locals
	stateStruct *oldStates			= (stateStruct*) pBackup;
	unsigned int	j, i;
	bool			dfIsForwardMove;

	// set old state to current one
	curState.field			= oldStates->field;
	curState.blockedSeq		= oldStates->blockedSeq;
	curState.lastSeq		= oldStates->lastSeq;

	// single return feature
	if (useSingleReturn) {
		
		// can open move be closed?
		if (srMoveWasClosed[curDepth]) {
			srIsOpenMove[inverseSeq[idPossibility]]++;
			srNumMovesOpen++;
		
		// open move
		} else {
			srIsOpenMove[idPossibility]--;
			srNumMovesOpen--;
		}
	}

	// double feature
	if (useDoubleFeature) { 
		
		// set dfIsForwardMove
		if (dfMoveIsForward[curDepth*numSequences+idPossibility]) {
			dfNumForwardMoves--;
			dfIsForwardMove = true;
		} else {
			dfNumForwardMoves++;
			dfIsForwardMove = false;
		}

		// only interesting if the move changes the current square "i"
		for (j=idPossibility*fieldSize, i=0; i<fieldSize; i++) { if (zones[i] == PGS_SS_FIX) { if (indicesMaps[j + i] != i) {
		
			// undo reverse move
			if (!dfIsForwardMove) {
				dfList[dfCurIndex[i] * fieldSize + i] = inverseSeq[idPossibility];
				dfCurIndex[i]++;
			// undo forward move
			} else {
				dfCurIndex[i]--;
				dfList[dfCurIndex[i] * fieldSize + i] = numSequences;
			}
		}}}
	}

	// show progress in gui
	if (showProgressDepth > curDepth) {
		abortCalculation = (!showProgress((unsigned int) foundSequences.size(), curDepth, idPossibility));
	}
}

//-----------------------------------------------------------------------------
// Name: move()
// Desc: 
//-----------------------------------------------------------------------------
void permutationGameSolver::move(unsigned int idPossibility, void **pBackup, void *pPossibilities)
{
	// locals
	unsigned int i, j;
	bool		 dfIsForwardMove;

	// make backup
	*pBackup						= (void*)&oldStates[curDepth];
	oldStates[curDepth].lastSeq		= curState.lastSeq;
	oldStates[curDepth].field		= curState.field;
	oldStates[curDepth].blockedSeq	= curState.blockedSeq;

	// single return feature
	if (useSingleReturn) {
		
		// can open move be closed?
		if (srIsOpenMove[inverseSeq[idPossibility]] > 0) {
			srMoveWasClosed[curDepth] = true;
			srIsOpenMove[inverseSeq[idPossibility]]--;
			srNumMovesOpen--;
		
		// open move
		} else {
			srMoveWasClosed[curDepth] = false;
			srIsOpenMove[idPossibility]++;
			srNumMovesOpen++;
		}
	}

	// double feature
	if (useDoubleFeature) {
		
		// set dfIsForwardMove
		if (dfMoveIsForward[curDepth*numSequences+idPossibility]) {
			dfNumForwardMoves++;
			dfIsForwardMove = true;
		} else {
			dfNumForwardMoves--;
			dfIsForwardMove = false;
		}
		
		// only interesting if the move changes the current square "i"
		for (j=idPossibility*fieldSize, i=0; i<fieldSize; i++) { if (zones[i] == PGS_SS_FIX) { if (indicesMaps[j + i] != i) {

			// reverse move
			if (!dfIsForwardMove) {
				dfCurIndex[i]--;
				dfList[dfCurIndex[i] * fieldSize + i] = numSequences;
			// forward move
			} else {
				dfList[dfCurIndex[i] * fieldSize + i] = idPossibility;
				dfCurIndex[i]++;
			}
		}}}
	}

	// last sequence
	curState.lastSeq	= idPossibility;

	// move balls
	unsigned int * p1 = &indicesMaps[idPossibility * fieldSize];
	unsigned int * p2 = &oldStates[curDepth].field[0];
	for (i=0; i<fieldSize; i++, p1++, p2++) {
		curState.field[*p1] = *p2;
	}

	// update blocked moves
	if (seqBlUser.setBlockedSequences != NULL) {
		seqBlUser.setBlockedSequences(idPossibility, curState.field, curState.blockedSeq, searchingSequences, &seqBlUser);
	} else if (seqBlock.empty()) { 
		p2 = &curState.blockedSeq[0];
		for (i=0; i<numSequences; i++, p2++) {
			*p2 = PGS_BS_ALLOWED;
		}
	} else {
		p1 = &seqBlock[idPossibility * numSequences];
		p2 = &curState.blockedSeq[0];
		for (i=0; i<numSequences; i++, p1++, p2++) {
			     if (*p1 == PGS_SB_BLOCKED)	*p2 = PGS_BS_BLOCKED;
			else if (*p1 == PGS_SB_RELEASE)	*p2 = PGS_BS_ALLOWED;
		}
	}
}

//-----------------------------------------------------------------------------
// Name: permutationGameSolver()
// Desc: Constructor
//-----------------------------------------------------------------------------
permutationGameSolver::permutationGameSolver()
{
	dbDirectory.clear();
	parentClassPointer			= 0;		
	useDoubleFeature			= false;			
	useSingleReturn				= false;			
	useMaxSearchDepth			= 0;			
	searchingSequences			= false;			
	removeSeqDuplicates			= false;		
	numRelStones				= 0;				
	fieldSize					= 0;					
	minDiffInRelZone			= 0;			
	maxDiffInRelZone			= 0;			
	numSequences				= 0;				
	dfNumForwardMoves			= 0;			
	srNumMovesOpen				= 0;				
	showProgressDepth			= 0;			
}

//-----------------------------------------------------------------------------
// Name: permutationGameSolver()
// Desc: Destructor
//-----------------------------------------------------------------------------
permutationGameSolver::~permutationGameSolver()
{
	parentClassPointer		= 0;
	numRelStones			= 0;
	numSequences			= 0;
	fieldSize				= 0;
	
	sequenceStruct::clearList(foundSequences);
}

//-----------------------------------------------------------------------------
// Name: getMemForStates()
// Desc: 
//-----------------------------------------------------------------------------
void permutationGameSolver::getMemForStates(vector<stateStruct>& states, unsigned int searchDepth, unsigned int fieldSize, unsigned int numSequences)
{
	states.resize(searchDepth);
	for (auto& curState : states) {
		curState.blockedSeq	.resize(numSequences);
		curState.field		.resize(fieldSize   );
	}
}

//-----------------------------------------------------------------------------
// Name: deleteStatesFromMemory()
// Desc: 
//-----------------------------------------------------------------------------
void permutationGameSolver::deleteStatesFromMemory(vector<stateStruct>& states)
{
	for (auto& curState : states) {
		curState.blockedSeq .clear();
		curState.field		.clear();
	}
	states.clear();
}

//-----------------------------------------------------------------------------
// Name: numDifferencesInIndicesMap()
// Desc: 
//-----------------------------------------------------------------------------
unsigned int permutationGameSolver::numDifferencesInIndicesMap(vectui& firstIndicesMap, vectui& secondIndicesMap)
{
	// locals
	unsigned int i, numDifferences;

	// compare indices maps
	for (numDifferences=0, i=0; i<fieldSize; i++) {
		if (firstIndicesMap[i] != secondIndicesMap[i]) numDifferences++;
	}

	// return value
	return numDifferences;
}

//-----------------------------------------------------------------------------
// Name: doesIndicesMapFitToZone()
// Desc: 
//-----------------------------------------------------------------------------
bool permutationGameSolver::doesIndicesMapFitToZone(vectui& theIndicesMap, vectui& theZone)
{
	// locals
	unsigned int i;

	// investigate indices map
	for (i=0; i<fieldSize; i++) {
		if (theIndicesMap[i] != i && theZone[i] == PGS_SS_FIX) return false;
	}

	// return value
	return true;
}

//-----------------------------------------------------------------------------
// Name: getZoneOfIndicesMap()
// Desc: 
//-----------------------------------------------------------------------------
void permutationGameSolver::getZoneOfIndicesMap(vectui& outZone, vectui& theIndicesMap, vectui& theZone)
{
	outZone.resize(fieldSize);

	// investigate indices map
	for (unsigned int i=0; i<fieldSize; i++) {
		if (theIndicesMap[i] == i && theZone[i] == PGS_SS_REL) {
			outZone[i] = PGS_SS_FIX;
		} else {
			outZone[i] = theZone[i];
		}
	}
}

//-----------------------------------------------------------------------------
// Name: getSequenceTypes()
// Desc: Each sequence changes only certain stones. So an indexMap can be converted to a zone. 
//       If there is no change in a relevant field then this field is set from PGS_SS_REL to PGS_SS_VAR.
//       Now, those zones returned in a list, whereby all doubly occurend zones are vanished, but counted.
//-----------------------------------------------------------------------------
list<sequenceStatistics> permutationGameSolver::getSequenceTypes(list<sequenceStruct*> *theSequences, vectui& theZone)
{
	// locals
	list<sequenceStruct*>::iterator		mySeqItr;
	list<sequenceStatistics>::iterator	mySeqStatisItr;
	list<sequenceStatistics>			mySeqStatisList;
	sequenceStatistics					mySeqStatis;

	// put occuring zone types in a list
	if (theSequences->size()>0) {

		// compare each sequence with each zone type in current list
		for (mySeqItr = theSequences->begin(); mySeqItr != theSequences->end(); mySeqItr++) {
			
			// look if zone of sequence is already in list
			if (mySeqStatisList.size()>0) for (mySeqStatisItr = mySeqStatisList.begin(); mySeqStatisItr != mySeqStatisList.end(); mySeqStatisItr++) {
		
				if (doesIndicesMapFitToZone((*mySeqItr)->indicesMap, (*mySeqStatisItr).zones) == true) {
					(*mySeqStatisItr).numSequences++;
					break;
				}
			}

			// zone type isn't already in list
			if (mySeqStatisList.size()==0 || mySeqStatisItr==mySeqStatisList.end()) {
				mySeqStatis.numSequences	= 1;
				getZoneOfIndicesMap(mySeqStatis.zones, (*mySeqItr)->indicesMap, theZone);
				mySeqStatisList.push_back(mySeqStatis);
			}
		}
	}

	// return value
	return mySeqStatisList;
}

//-----------------------------------------------------------------------------
// Name: countIncidences()
// Desc: Returns the number of occuring PGS_SS_RELs in each zone.
//-----------------------------------------------------------------------------
unsigned int *permutationGameSolver::countIncidences(list<sequenceStatistics> *theSeqStatisList)
{
	// locals
	unsigned int						i;
	unsigned int						*myIncidences = new unsigned int[fieldSize];
	list<sequenceStatistics>::iterator	mySeqStatisItr;

	// zero array
	for (i=0; i<fieldSize; i++) myIncidences[i] = 0;

	// go through list
	if (theSeqStatisList->size()>0) for (mySeqStatisItr = theSeqStatisList->begin(); mySeqStatisItr != theSeqStatisList->end(); mySeqStatisItr++) {
		for (i=0; i<fieldSize; i++) {
			if ((*mySeqStatisItr).zones[i] == PGS_SS_REL) {
				myIncidences[i] += (*mySeqStatisItr).numSequences;
			}
		}
	}

	// return value
	return myIncidences;
}

//-----------------------------------------------------------------------------
// Name: power()
// Desc: The result of base ^ exponent.
//-----------------------------------------------------------------------------
unsigned int permutationGameSolver::power(unsigned int base, unsigned int exponent)
{
	unsigned int i, result = 1; 
	for (i=0; i < exponent; i++) result *= base;
	return result;
}

//-----------------------------------------------------------------------------
// Name: searchValue()
// Desc: 
//-----------------------------------------------------------------------------
unsigned int permutationGameSolver::searchValue(unsigned int value, unsigned int position, unsigned int arraySize, vectui& theArray)
{
	// locals
	unsigned int i, j;

	for (j=0, i=0; i<arraySize; i++) {
		if (value==theArray[i]) {
			if (j==position) break;
			j++;
		}
	}
	return i;
}

//-----------------------------------------------------------------------------
// Name: sequencesSufficientForZone()
// Desc: 
//-----------------------------------------------------------------------------
bool permutationGameSolver::sequencesSufficientForZone(list<sequenceStruct*> *theSequences, vectui& theZone)
{
	// locals
	list<sequenceStruct*>::iterator myItr;
	unsigned int	*indexMapFromBarrelToMyNo, *indexMapFromMyNoToBarrel;
	unsigned int	numFixStones = 0, numVarStones = 0, numRelStones = 0;
	unsigned int	minNumSeq, *checkedSeq, myIndex;
	unsigned int	i;
	bool			allChecked;

	// examine zone
	for (i=0; i<fieldSize; i++) {
		switch (theZone[i])
		{
		case PGS_SS_FIX: numFixStones++; break;
		case PGS_SS_VAR: numVarStones++; break;
		case PGS_SS_REL: numRelStones++; break;
		}
	}

	// make index map
	indexMapFromBarrelToMyNo = new unsigned int[fieldSize];
	indexMapFromMyNoToBarrel = new unsigned int[numVarStones + numRelStones];
	
	for (i=0; i<fieldSize; i++)		indexMapFromBarrelToMyNo[i] = numVarStones + numRelStones;
	for (i=0; i<numRelStones; i++)	indexMapFromBarrelToMyNo[searchValue(PGS_SS_REL, i, fieldSize, theZone)] = i;
	for (i=0; i<numVarStones; i++)	indexMapFromBarrelToMyNo[searchValue(PGS_SS_VAR, i, fieldSize, theZone)] = i + numRelStones;
	for (i=0; i<numRelStones; i++)  indexMapFromMyNoToBarrel[i				 ] = searchValue(PGS_SS_REL, i, fieldSize, theZone);
	for (i=0; i<numVarStones; i++)  indexMapFromMyNoToBarrel[i + numRelStones] = searchValue(PGS_SS_VAR, i, fieldSize, theZone);

	// uncheck needed sequences
	minNumSeq  = power(numVarStones + numRelStones, numRelStones);
	checkedSeq = new unsigned int[minNumSeq];
	for (i=0; i<minNumSeq; i++) checkedSeq[i] = 0;

	// check impossible indicesMaps;
	unsigned int j, k, sum = 1, *combination = new unsigned int[numRelStones];
	
	// starting point
	for (i=0; i<numRelStones; i++) combination[i] = 0;

	while (sum != 0) {

		// when the indices on two or more relevant stones are the same then check
		for (j=0; j<numRelStones-1; j++) {
			for (i=j+1; i<numRelStones; i++) {
				if (combination[j] == combination[i]) {
					
					// check this combination
					for (myIndex=0, k=0; k<numRelStones; k++) {
						myIndex += power(numRelStones + numVarStones, k) * combination[k];
					}
					checkedSeq[myIndex]++;
					j = numRelStones-1;
					i = numRelStones;
				}
			}
		}

		// next combination
		for (i=0; i<numRelStones; i++) {
			if (combination[i] < numRelStones + numVarStones - 1) { 
				combination[i]++;
				break;
			} else {
				combination[i] = 0;
			}
		}

		// when all combination[i] are zero then its we have finished
		for (i=0, sum=0; i<numRelStones; i++) sum += combination[i];
	}

	// test each passed sequence
	for (myItr = theSequences->begin(); myItr != theSequences->end(); myItr++) {
		myIndex = 0; 
		if (checkedSeq[myIndex]) continue;
		for (i=0; i<numRelStones; i++) {
			myIndex += power(numRelStones + numVarStones, i) * indexMapFromBarrelToMyNo[(*myItr)->indicesMap[indexMapFromMyNoToBarrel[i]]];
		}
		checkedSeq[myIndex]++;
	}

	// when not all sequences checked then passed sequences are insufficient
	for (allChecked = true, i=1; i<minNumSeq; i++) if (checkedSeq[i] == 0) allChecked = false;

	// free mem
	delete [] indexMapFromBarrelToMyNo;
	delete [] indexMapFromMyNoToBarrel;
	delete [] checkedSeq;

	// result
	if (!allChecked)
		return false;
	else
		return true;
}

//-----------------------------------------------------------------------------
// Name: setInitialState()
// Desc: Overwrites the initial state array.
//-----------------------------------------------------------------------------
void permutationGameSolver::setInitialState(const vectui& theInitialField)
{
	initialField = theInitialField;
}

//-----------------------------------------------------------------------------
// Name: setFinalState()
// Desc: Overwrites the final state array.
//-----------------------------------------------------------------------------
void permutationGameSolver::setFinalState(const vectui& theFinalField)
{
	finalField = theFinalField;
}

//-----------------------------------------------------------------------------
// Name: setMaxFoundSolutions()
// Desc: if that many solutions have been found, then the calculation is stopped
//-----------------------------------------------------------------------------
void permutationGameSolver::setMaxFoundSolutions(const unsigned int maxNumSolutions)
{
	this->maxNumSolutions = maxNumSolutions;
}

//-----------------------------------------------------------------------------
// Name: setInitialState()
// Desc: Must be called before searchSequences() and finishZone() to define the
//       initial and final state of the field.
//-----------------------------------------------------------------------------
void permutationGameSolver::setFieldSize(unsigned int theFieldSize, wstring& databaseDirectory, wstring& databaseFileType)
{
	// set vars
	fieldSize		= theFieldSize;
	dbDirectory		= databaseDirectory;
	dbFileType		= databaseFileType;

	zones			.resize(fieldSize);
	initialField	.resize(fieldSize);
	finalField		.resize(fieldSize);
	cmpField		.resize(fieldSize);
	curState.field	.resize(fieldSize);
}

//-----------------------------------------------------------------------------
// Name: putInListIfItIsNotDuplicate()
// Desc: return true if the "newSeq" was added to "listSeq"
//-----------------------------------------------------------------------------
bool permutationGameSolver::putInListIfItIsNotDuplicate(list<sequenceStruct*> &listSeq, sequenceStruct *newSeq, unsigned int* posInList)
{
	// locals
	list<sequenceStruct*>::iterator	seqItr;
	unsigned int					i, j = 0;
	bool							isDuplicate			= false;

	// calc inverse indices map
	for (i=0; i<fieldSize; i++) inverseIndicesMap[newSeq->indicesMap[i]] = i;

	// compare new solution with each element in the list
	if (listSeq.size()) {
		
		for (seqItr = listSeq.begin(); seqItr != listSeq.end(); seqItr++, j++) {
			
			// compare each relevant stone in the indices map
			for (i=0; i < numRelStones; i++) {
				if ((*seqItr)->indicesMap[inverseIndicesMap[relStones[i]]] != relStones[i]) break;
			}

			// when every relevant index is the same, then its a duplicate
			if (i == numRelStones) {
				isDuplicate = true;
				break;
			}
		}
	}

	// if it is not a duplicate then put it in the list
	if (!isDuplicate) {
		listSeq.push_back(new sequenceStruct(*newSeq));
	// overwrite existing solution if the new one is shorter
	} else {
		if (newSeq->solPath.numSteps < (*seqItr)->solPath.numSteps) {
			(*seqItr)->copy(*newSeq);
		}
	}
	
	// return value
	if (posInList != NULL) *posInList = j;
	return !isDuplicate;
}

//-----------------------------------------------------------------------------
// Name: getCPU()
// Desc: cpuName should be of size [64]
//-----------------------------------------------------------------------------
void permutationGameSolver::getCPU(char* cpuName)
{
	// Get extended ids.
    int CPUInfo[4] = {-1};
    __cpuid(CPUInfo, 0x80000000);
    unsigned int nExIds = CPUInfo[0];

    // Get the information associated with each extended ID.
    for( unsigned int i=0x80000000; i<=nExIds; ++i) 
	{
        __cpuid(CPUInfo, i);

        // Interpret CPU brand string and cache information.
        if  (i == 0x80000002) {
            memcpy( cpuName,      CPUInfo, sizeof(CPUInfo));
        } else if( i == 0x80000003 ) {
            memcpy( cpuName + 16, CPUInfo, sizeof(CPUInfo));
        } else if( i == 0x80000004 ) {
            memcpy( cpuName + 32, CPUInfo, sizeof(CPUInfo));
        }
	}
}
#pragma endregion

#pragma endregion loadAndSave
//-----------------------------------------------------------------------------
// Name: openFile()
// Desc: 
//-----------------------------------------------------------------------------
bool zoneContainer::openFile(const vectui& theZones, fstream& fs, bool writeMode, wstring& dbDirectory, wstring& fileType)
{
	// locals
	wstring	filename;

	// init vars
	makeFileNameFromZones(theZones, filename, dbDirectory, fileType);

	/* rename file if possible
	wstringstream wss;
	wstring	oldFilename;
	if (dbDirectory.size()==0) return false;
	wss << dbDirectory << L"\\";
	for (auto& curZone : theZones) {
		wss << curZone;
	}
	wss	<< L"." << fileType;
	oldFilename = wss.str();

	//setup converter
	using convert_type = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_type, wchar_t> converter;

	//use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)

	fs.open(oldFilename, ios_base::binary | (writeMode ? ios_base::in | ios_base::out : ios_base::in));
	if (fs.is_open()) {
		fs.close();
		std::string strTo = converter.to_bytes( filename );
		std::string strFrom = converter.to_bytes( oldFilename );
		
		rename(strFrom.c_str(), strTo.c_str());
	}*/

	// open file
	fs.open(filename, ios_base::binary | (writeMode ? ios_base::in | ios_base::out : ios_base::in));

	// if file does not exist than 
	if (!fs.is_open()) {
		if (writeMode) {
			fs.open(filename, ios_base::binary | ios_base::in | ios_base::out | ios_base::trunc);
		}
		return fs.is_open();
	// file exists
	} else {
		return convertToNewestType(theZones, fs, writeMode, filename);
	}
}

//-----------------------------------------------------------------------------
// Name: convertToNewestType()
// Desc: 
//-----------------------------------------------------------------------------
bool zoneContainer::convertToNewestType(const vectui& theZones, fstream& fs, bool writeMode, wstring& filename)
{
	// locals
	unsigned int myId;

	// read id
	fs.seekg(0);
	fs.read((char*) &myId, sizeof(unsigned int));
	fs.seekg(0);

	// if "old"-file type, convert to newest type
	if (myId != id) {

		numZones			= 1;
		numFields			= myId;
		zones				.resize(1, theZones);
		offsetSeqContainers	.resize(1, 0);
		fs.seekg(0);

		seqContainers.resize(numZones);
		auto curOffset = offsetSeqContainers.begin(); 
		for (auto& curSeqCont : seqContainers) {
			fs.seekg(*curOffset, fstream::beg); 
			curSeqCont.loadFromFile(fs, true, true);
			curOffset++;
		}

		fs.close();
		fs.open(filename, ios_base::binary | ios_base::in | ios_base::out);
		saveToFile(fs);
		fs.close();
		fs.open(filename, ios_base::binary | (writeMode ? ios_base::in | ios_base::out : ios_base::in));
	}

	return true;
}

//-----------------------------------------------------------------------------
// Name: getHashStr()
// Desc: 
//-----------------------------------------------------------------------------
wstring zoneContainer::getHashStr(const vectui& theZones)
{
	// locals
	wstring			strIn;
	wstringstream	wss;
	hash<wstring>	hashValue;
		
	for (auto& curZone : theZones) {
		wss << curZone;
	}
	strIn = wss.str();
	wss.str(L""); 
	
	// BUG: Problem when build is done in 64 bit, since function hashValue() returns a size_t value, which is eitehr 4 or 8 bytes long. 
	wss << setbase(16) << setw(sizeof(size_t) * 2) << setfill(L'0') << hashValue(strIn);

	return wss.str();
}

//-----------------------------------------------------------------------------
// Name: makeFileNameFromZones()
// Desc: 
//-----------------------------------------------------------------------------
void zoneContainer::makeFileNameFromZones(const vectui& theZones, wstring& filename, wstring& dbDirectory, wstring& fileType)
{
	// locals
	wstringstream wss;
	if (dbDirectory.size()==0) return;
	wss << dbDirectory << L"\\" << getHashStr(theZones) << L"." << fileType;
	filename = wss.str();
}

//-----------------------------------------------------------------------------
// Name: loadFromFile()
// Desc:
//-----------------------------------------------------------------------------
bool sequenceStruct::loadFromFile(istream& is, unsigned int numFields)
{
	indicesMap	.resize(numFields);
	is.read((char*) &indicesMap[0],	sizeof(unsigned int)*numFields			);
	is.read((char*) &solPath.numSteps, sizeof(unsigned int)					);

	solPath.path.resize(solPath.numSteps);
	if (solPath.numSteps) {
		is.read((char*) &solPath.path[0],	sizeof(unsigned int)*solPath.numSteps	);
	}

	return true;
}

//-----------------------------------------------------------------------------
// Name: saveToFile()
// Desc:
//-----------------------------------------------------------------------------
bool sequenceStruct::saveToFile(ostream& os)
{
	os.write((char*) &indicesMap[0]		, sizeof(unsigned int) * indicesMap.size()		);
	os.write((char*) &solPath.numSteps	, sizeof(unsigned int)							);
	os.write((char*) &solPath.path[0]	, sizeof(unsigned int) * solPath.path.size()	);

	return true;
}

//-----------------------------------------------------------------------------
// Name: equalTo()
// Desc: 
//-----------------------------------------------------------------------------
bool sequenceStruct::equalTo(vectui& theZone, sequenceStruct& anotherSeq)
{
	if (theZone.size() !=            indicesMap.size()) return false;
	if (theZone.size() != anotherSeq.indicesMap.size()) return false;

	auto itrThisIndicesMap		=			 indicesMap.begin();
	auto itrAnotherIndicesMap	= anotherSeq.indicesMap.begin();

	for (auto& curField : theZone) {
		if (curField == PGS_SS_REL) {
			if (*itrThisIndicesMap != *itrAnotherIndicesMap) {
				return false;
			}
		}
		itrThisIndicesMap++;
		itrAnotherIndicesMap++;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Name: addIfNotDuplicate()
// Desc: 
//-----------------------------------------------------------------------------
bool sequenceStruct::addIfNotDuplicate(vectui& theZone, vector<sequenceStruct>& sequences, unsigned int& posInFile, unsigned int* posInVector)
{
	// locals
	bool			isDuplicate = false;
	unsigned int	curPos		= 0;

	for (auto& curSequence : sequences) {
		if (equalTo(theZone, curSequence)) {
			if (solPath.numSteps < curSequence.solPath.numSteps) {
				curSequence		= *this;
			}
			if (posInVector) {
				*posInVector	 = curPos;
			}
			isDuplicate		= true;
			break;
		}
		curPos++;
	}

	if (!isDuplicate) {
		if (posInVector) {
			*posInVector	 = posInFile;
		}
		sequences.push_back(*this);
		posInFile++;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Name: loadFromFile()
// Desc:
//-----------------------------------------------------------------------------
bool sequenceCalculation::loadFromFile(istream& is, unsigned int numFields, unsigned int numMoves)
{
	// Extract calculation details														
	is.read((char*) &dateOfCalculation		, sizeof(SYSTEMTIME	) );					//
	is.read((char*) &calculationDuration	, sizeof(DWORD		) );					//
	is.read((char*) &cpuType[0]				, sizeof(sequenceCalculation::cpuType));	//
	is.read((char*) &useSingleReturn		, sizeof(bool) * 2    );					// ... * 2 is sehr strange
	is.read((char*) &useDoubleFeature		, sizeof(bool) * 2    );					// ... * 2 is sehr strange
	is.read((char*) &maxSearchDepth			, sizeof(unsigned int));					// 
	is.seekg(sizeof(unsigned int*)			, ifstream::cur);							// skip initialBlockedMoves
	is.seekg(sizeof(unsigned int*)			, ifstream::cur);							// skip seqBlock
	is.read((char*) &numSequencesInFile		, sizeof(unsigned int));					// 
	is.seekg(sizeof(unsigned int*)			, ifstream::cur);							// skip sequenceIndexInFile

	initialBlockedMoves.resize(numMoves);
	is.read((char*) &initialBlockedMoves[0],	sizeof(unsigned int) * initialBlockedMoves.size());		
				
	seqBlock		.resize(numMoves*numMoves);
	is.read((char*) &seqBlock[0],				sizeof(unsigned int) * seqBlock.size());
				
	sequenceIndexInFile.resize(numSequencesInFile);
	is.read((char*) &sequenceIndexInFile[0],	sizeof(unsigned int) * sequenceIndexInFile.size());	

	return true;
}

//-----------------------------------------------------------------------------
// Name: saveToFile()
// Desc:
//-----------------------------------------------------------------------------
bool sequenceCalculation::saveToFile(ostream& os)
{
	os.write((char*) &dateOfCalculation			, sizeof(SYSTEMTIME	));		//
	os.write((char*) &calculationDuration		, sizeof(DWORD		));		//
	os.write((char*) &cpuType[0]				, sizeof(char) * 0x40);		//
	os.write((char*) &useSingleReturn			, sizeof(bool		)*2);	// ... * 2 ist sehr strange
	os.write((char*) &useDoubleFeature			, sizeof(bool		)*2);	// ... * 2 ist sehr strange
	os.write((char*) &maxSearchDepth			, sizeof(unsigned int));	//
	os.seekp(sizeof(unsigned int*)				, ofstream::cur);			// 
	os.seekp(sizeof(unsigned int*)				, ofstream::cur);			// 
	os.write((char*) &numSequencesInFile		, sizeof(unsigned int));	// 
	os.seekp(sizeof(unsigned int*)				, ofstream::cur);			// 

	os.write((char*) &initialBlockedMoves[0]	, sizeof(unsigned int) * initialBlockedMoves.size());
	os.write((char*) &seqBlock[0]				, sizeof(unsigned int) * seqBlock			.size());
	os.write((char*) &sequenceIndexInFile[0]	, sizeof(unsigned int) * sequenceIndexInFile.size());

	return true;
}

//-----------------------------------------------------------------------------
// Name: loadFromFile()
// Desc:
//-----------------------------------------------------------------------------
bool seqContainer::loadFromFile(istream& is, bool loadSequences, bool loadCalculations)
{
	// locals
	streamoff startSeek	= is.tellg();

	// Read Header
	is.read((char*) &numFields					,	sizeof(unsigned int		));
	is.read((char*) &numMoves					,	sizeof(unsigned int		));
	is.read((char*) &numCalculations			,	sizeof(unsigned int		));
	is.read((char*) &numSequences				,	sizeof(unsigned int		));
	is.read((char*) &offsetSequences			,	sizeof(LONG				));
	is.read((char*) &offsetCalculations			,	sizeof(LONG				));
	is.read((char*) &totalCalculationTime		,	sizeof(unsigned __int64	));
	is.seekg(sizeof(unsigned int*)				, ofstream::cur);			// skip sequences
	is.seekg(sizeof(unsigned int*)				, ofstream::cur);			// skip calculations

	// read each calculation
	if (!loadCalculations || numCalculations==0) {
		calculations	.clear();
	} else {
		is.seekg(startSeek + offsetCalculations);
		calculations	.resize(numCalculations);
		for (auto& curCalculation : calculations) {
			curCalculation.loadFromFile(is, numFields, numMoves);
		}
	}

	// Read each sequence
	if (!loadSequences || numSequences==0) {
		sequences		.clear();
	} else {
		is.seekg(startSeek + offsetSequences);
		sequences		.resize(numSequences);
		for (auto& curSequence : sequences) {
			curSequence.loadFromFile(is, numFields);
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// Name: loadFromFile()
// Desc:
//-----------------------------------------------------------------------------
bool seqContainer::saveToFile(ostream& os)
{
	// locals
	streamoff startSeek	= os.tellp();
	
	// write Header
	os.write((char*) &numFields				,	sizeof(unsigned int		));
	os.write((char*) &numMoves				,	sizeof(unsigned int		));
	os.write((char*) &numCalculations		,	sizeof(unsigned int		));
	os.write((char*) &numSequences			,	sizeof(unsigned int		));
	os.seekp(sizeof(offsetSequences)		, ostream::cur);			// skip offsetSequences
	os.seekp(sizeof(offsetCalculations)		, ostream::cur);			// skip offsetCalculations
	os.write((char*) &totalCalculationTime	,	sizeof(unsigned __int64	));
	os.seekp(sizeof(unsigned int*)			, ostream::cur);			// skip sequences
	os.seekp(sizeof(unsigned int*)			, ostream::cur);			// skip calculations

	// write each calculation
	offsetCalculations	= (LONG) (os.tellp() - startSeek);
	for (auto& curCalculation : calculations) {
		curCalculation.saveToFile(os);
	}

	// write each sequence
	offsetSequences		= (LONG) (os.tellp() - startSeek);
	for (auto& curSequence : sequences) {
		curSequence.saveToFile(os);
	}
	streamoff endSeek	= os.tellp();

	// now, the offsets contain the correct values
	os.seekp(startSeek + 4 * sizeof(unsigned int));
	os.write((char*) &offsetSequences		,	sizeof(LONG				));
	os.write((char*) &offsetCalculations	,	sizeof(LONG				));
	os.seekp(endSeek);

	return true;
}

//-----------------------------------------------------------------------------
// Name: addSeqContainer()
// Desc: 
//-----------------------------------------------------------------------------
bool seqContainer::addSeqContainer(vectui& theZone, seqContainer& newSeqCont)
{
	// locals
	unsigned int			seqIndexInFile			= 0;
	unsigned int			seqIndexInCalculation	= 0;
	unsigned int			curCalcId				= 0;
	vector<sequenceStruct>	tmpSequences;

	if (numFields			!= newSeqCont.numFields) return false;
	if (numMoves			!= newSeqCont.numMoves ) return false;
	numCalculations			+= newSeqCont.numCalculations;
	totalCalculationTime	+= newSeqCont.totalCalculationTime;
	
	// merge sequences without duplicates
	tmpSequences.reserve(sequences.size() + newSeqCont.sequences.size());
	for (auto& curNewSequence : sequences) {
		curNewSequence.addIfNotDuplicate(theZone, tmpSequences, seqIndexInFile, nullptr);
	}
	for (auto& curNewSequence : newSeqCont.sequences) {
		curNewSequence.addIfNotDuplicate(theZone, tmpSequences, seqIndexInFile, &newSeqCont.calculations[curCalcId].sequenceIndexInFile[seqIndexInCalculation]);
		seqIndexInCalculation++;
	}
	tmpSequences			.shrink_to_fit();
	sequences				= move(tmpSequences);
	numSequences			= (unsigned int) sequences.size();

	// merge calculations
	calculations.insert(calculations.end(), newSeqCont.calculations.begin(), newSeqCont.calculations.end());

	return true;
}

//-----------------------------------------------------------------------------
// Name: removeCalculation()
// Desc: 
//-----------------------------------------------------------------------------
bool seqContainer::removeCalculation(unsigned int calcIndex)
{
	// param ok?
	if (calcIndex >= calculations.size()) return false;

	// locals
	list<unsigned int> seqToRemove;

	// update stuff
	offsetSequences			= 0;	// is updated during saving
	offsetCalculations		= 0;	// is updated during saving
	totalCalculationTime   -= calculations[calcIndex].calculationDuration;

	// remove sequences belonging only to this calculation
	for (auto& curSeq : calculations[calcIndex].sequenceIndexInFile) {

		bool			uniqueSeq			= true;
		unsigned int	anotherCalcIndex	= 0;

		// look in every other calculation
		for (auto& curCalc : calculations) {
			if (calcIndex != anotherCalcIndex) {
				// check each sequence of every other calculation
				vectui& a = calculations[anotherCalcIndex].sequenceIndexInFile;
				if (find(a.begin(), a.end(), curSeq) != a.end()) uniqueSeq = false;
				if (!uniqueSeq) break;
			}
			anotherCalcIndex++;
		}

		// if sequence was found only by the considered calculation, then remember
		if (uniqueSeq) seqToRemove.push_back(curSeq);
	}

	// remove all sequences listed in 'seqToRemove'
	if (seqToRemove.size() == sequences.size()) {
		sequences.clear();
	} else if (seqToRemove.size()) {
		vector<sequenceStruct> tmpSeq;
		tmpSeq.reserve(sequences.size() - seqToRemove.size());
		seqToRemove.sort();

		auto itrBeginOfBlock = sequences.begin();

		for(auto it = seqToRemove.begin(); it != seqToRemove.end(); ++ it) {
			
			auto itrEndOfBlock = sequences.begin() + *it;

			if (itrBeginOfBlock != itrEndOfBlock) {
				copy(itrBeginOfBlock, itrEndOfBlock, back_inserter(tmpSeq));
			}
			itrBeginOfBlock = itrEndOfBlock + 1;
		}

		if (itrBeginOfBlock != sequences.end()) {
			copy(itrBeginOfBlock, sequences.end(), back_inserter(tmpSeq));
		}

		// swap
		sequences = move(tmpSeq);
	}

	// remove calculation
	calculations.erase(calculations.begin() + calcIndex);
	numCalculations			= (unsigned int) calculations.size();

	// return value
	return true;
}

//-----------------------------------------------------------------------------
// Name: loadFromFile()
// Desc:
//-----------------------------------------------------------------------------
bool zoneContainer::loadFromFile(istream& is, const vectui& theZones, bool loadSeqContainers)
{
	// read Header
	is.read((char*) &id,						sizeof(unsigned int));
	is.read((char*) &numZones,					sizeof(unsigned int));
	is.read((char*) &numFields,					sizeof(unsigned int));

	zones.resize(numZones);
	for (auto& curZone : zones) {
		curZone.resize(numFields);
		is.read((char*) &curZone[0],			sizeof(unsigned int) * curZone.size());
	}

	offsetSeqContainers.resize(numZones);
	is.read((char*) &offsetSeqContainers[0],	sizeof(unsigned int) * offsetSeqContainers.size());

	if (loadSeqContainers) {
		seqContainers.resize(numZones);
		auto curOffset = offsetSeqContainers.begin(); 
		for (auto& curSeqCont : seqContainers) {
			is.seekg(*curOffset, fstream::beg); 
			curSeqCont.loadFromFile(is, true, true);
			curOffset++;
		}
	}
	
	return true;
}

//-----------------------------------------------------------------------------
// Name: saveToFile()
// Desc:
//-----------------------------------------------------------------------------
bool zoneContainer::saveToFile(ostream& os)
{
	// check integrity
	if (offsetSeqContainers.size()	!= seqContainers.size()	) return false;
	for (auto& curZone : zones) {
		if (curZone.size()			!= numFields			) return false;
	}
	if (numZones					== 0					) return false;

	// header
	os.seekp(0);
	os.write((char*) &id,			sizeof(unsigned int));
	os.write((char*) &numZones,		sizeof(unsigned int));
	os.write((char*) &numFields,	sizeof(unsigned int));

	// zones
	for (auto& curZone : zones) {
		os.write((char*) &curZone[0],	sizeof(unsigned int) * curZone.size());
	}

	// leave space for 'offsetSeqContainers'
	streamoff	sOffSeqContainers = os.tellp();
	os.seekp(sizeof(unsigned int) * offsetSeqContainers.size(), ios_base::cur);

	// sequence containers
	auto curOffset = offsetSeqContainers.begin(); 
	for (auto& curSeqCont : seqContainers) {
		*curOffset = (unsigned int) os.tellp();
		curSeqCont.saveToFile(os);
		curOffset++;
	}

	// now, 'offsetSeqContainers' is filled with correct vales
	os.seekp(sOffSeqContainers);
	os.write((char*) &offsetSeqContainers[0],	sizeof(unsigned int) * offsetSeqContainers.size());

	return true;
}

//-----------------------------------------------------------------------------
// Name: setFilePointer()
// Desc:
//-----------------------------------------------------------------------------
bool zoneContainer::setFilePointer(istream& is, const vectui& theZones)
{
	is.seekg(offsetSeqContainers[getSeqContIndex(theZones)], istream::beg);
	return true;
}

//-----------------------------------------------------------------------------
// Name: addSeqContainer()
// Desc:
//-----------------------------------------------------------------------------
bool zoneContainer::addSeqContainer(const vectui& theZones, seqContainer& seqCont)
{
	// locals
	unsigned int					zoneIndex		= 0;

	// param ok?
	if (theZones.size() != numFields) return false;

	// zone already existend?
	for (auto& curZone : zones) {
		if (curZone == theZones) {
			break;
		}
		zoneIndex++;
	}

	// add zone if not already contained in zone container
	if (zoneIndex == zones.size()) {
		zones				.push_back(theZones);
		seqContainers		.push_back(seqCont);
		offsetSeqContainers	.push_back(0);			// value will be calculated during saving
		numZones			= zones.size();
	// add sequence container to existing one
	} else {
		seqContainers[zoneIndex].addSeqContainer(zones[zoneIndex], seqCont);
	}
	
	return true;
}

//-----------------------------------------------------------------------------
// Name: loadSeqContainer()
// Desc:
//-----------------------------------------------------------------------------
bool permutationGameSolver::loadSeqContainer(vectui& theZones, seqContainer &seqCont, bool loadSequences)
{
	// locals
	wstring			filename;
	fstream			fs;
	zoneContainer	myZoneCont;

	// handle gültig
	if (!myZoneCont.openFile(theZones, fs, false, dbDirectory, dbFileType)) {
		return false;
	} else {
		myZoneCont.loadFromFile(fs, theZones, false);
		myZoneCont.setFilePointer(fs, theZones);
		seqCont.loadFromFile(fs, loadSequences, true);
	}

	// release mem
	fs.close();

	// return value
	return true;
}

//-----------------------------------------------------------------------------
// Name: loadSequences()
// Desc: 
//-----------------------------------------------------------------------------
bool permutationGameSolver::loadSequences(const vectui& theZones, list<sequenceStruct*> &sequences)
{
	// locals
	seqContainer	seqCont;
	unsigned int	i;
	wstring			filename;
	sequenceStruct	*tmpSequenceStruct;
	fstream			fs;
	zoneContainer	myZoneCont;

	// init vars
	sequenceStruct::clearList(sequences);

	// handle gültig
	if (!myZoneCont.openFile(theZones, fs, false, dbDirectory, dbFileType)) {
		return false;
	} else {
		myZoneCont.loadFromFile(fs, theZones, false);
		myZoneCont.setFilePointer(fs, theZones);

		// remember position of sequence container, read header of it and set pointer to sequences in the sequence container
		streamoff startSeek	= fs.tellg();
		seqCont.loadFromFile(fs, false, false);
		fs.seekg(startSeek + seqCont.offsetSequences);

		// Read each sequence
		for (i=0; i<seqCont.numSequences; i++) {

			// Extract Sequence & Put Sequence in List
			tmpSequenceStruct				= new sequenceStruct();
			tmpSequenceStruct->loadFromFile(fs, seqCont.numFields);
			sequences.push_back(tmpSequenceStruct);
		}
	}

	// release mem
	fs.close();

	// return value
	return true;
}

//-----------------------------------------------------------------------------
// Name: saveSequences()
// Desc: 
//-----------------------------------------------------------------------------
bool permutationGameSolver::saveSequences(vectui&					theZones,					
										  list<sequenceStruct*> &	sequences, 
										  unsigned int				numMoves,					
										  unsigned int				maxSearchDepth,
										  bool						useSingleReturn,						
										  bool						useDoubleFeature, 
										  vectui&					initialBlockedMoves,		
										  vectui&					seqBlock,
										  DWORD						calculationDurationInMilliseconds,	
										  bool						overWriteDatabase)
{
	// locals
	fstream							fs;
	zoneContainer					zoneCont;
	seqContainer					seqCont;

	// make seqContainer which shall be added to file
	seqCont.numCalculations			= 1;
	seqCont.numFields				= fieldSize;
	seqCont.numMoves				= numMoves;
	seqCont.totalCalculationTime	= calculationDurationInMilliseconds;
	seqCont.numSequences			= sequences.size();
	seqCont.sequences				.reserve(sequences.size());
	for (auto& curSequence : sequences) {
		seqCont.sequences.push_back(*curSequence);
	}
	seqCont.calculations.resize(1);
	seqCont.calculations.front().useSingleReturn		= useSingleReturn;
	seqCont.calculations.front().useDoubleFeature		= useDoubleFeature;
	seqCont.calculations.front().numSequencesInFile		= sequences.size();
	seqCont.calculations.front().sequenceIndexInFile	.resize(sequences.size());
	seqCont.calculations.front().maxSearchDepth			= maxSearchDepth;
	seqCont.calculations.front().calculationDuration	= calculationDurationInMilliseconds;
	seqCont.calculations.front().initialBlockedMoves	= initialBlockedMoves;
	seqCont.calculations.front().seqBlock				= seqBlock;
	getCPU(seqCont.calculations.front().cpuType);
	GetSystemTime(&seqCont.calculations.front().dateOfCalculation);

	// open file
	if (!zoneCont.openFile(theZones, fs, true, dbDirectory, dbFileType)) {
		return false;
	} else {
	
		// get filesize
		fs.seekg( 0, std::ios::end );
		streamoff fileSize = fs.tellg();
		fs.seekg( 0 );

		// load zone container
		if (!overWriteDatabase && fileSize > 0) {
			zoneCont.loadFromFile(fs, theZones, true);
			zoneCont.addSeqContainer(theZones, seqCont);
		} else {
			zoneCont.numFields				= fieldSize;
			zoneCont.numZones				= 1;
			zoneCont.zones					.resize(1);
			zoneCont.seqContainers			.resize(1, seqCont);
			zoneCont.offsetSeqContainers	.resize(1);
			zoneCont.zones[0].assign(theZones.begin(), theZones.end());
		}

		// write everything
		zoneCont.saveToFile(fs);
	}

	// release mem
	fs.close();

	// return value
	return true;
}

//-----------------------------------------------------------------------------
// Name: getSeqContIndex()
// Desc: 
//-----------------------------------------------------------------------------
unsigned int zoneContainer::getSeqContIndex(const vectui& theZones)
{
	// locals
	unsigned int seqContIndex = 0;
	
	for (auto& curZone : zones) {
		if (curZone == theZones) {
			break;
		}
		seqContIndex++;
	}
	if (seqContIndex >= offsetSeqContainers.size()) {
		return 0;
	}

	return seqContIndex;
}

//-----------------------------------------------------------------------------
// Name: removeCalculation()
// Desc: 
//-----------------------------------------------------------------------------
bool zoneContainer::removeCalculation(const vectui& theZones, unsigned int calcIndex)
{
	// locals
	unsigned int zoneIndex = getSeqContIndex(theZones);

	// remove calculation
	seqContainers[zoneIndex].removeCalculation(calcIndex);

	// if no calculation remain then delete zone
	if (seqContainers[zoneIndex].numCalculations == 0) {
		seqContainers		.erase(seqContainers		.begin() + zoneIndex);
		zones				.erase(zones				.begin() + zoneIndex);
		offsetSeqContainers	.erase(offsetSeqContainers	.begin() + zoneIndex);
		numZones			= zones.size();
	}

	// return value
	return true;
}

//-----------------------------------------------------------------------------
// Name: removeCalculationFromFile()
// Desc: 
//-----------------------------------------------------------------------------
bool permutationGameSolver::removeCalculationFromFile(const vectui& theZones, unsigned int calcIndex)
{
	// locals
	fstream							fs;
	zoneContainer					zoneCont;

	// open file, load, remove calculation, save and close
	if (!zoneCont.openFile(theZones, fs, true, dbDirectory, dbFileType)) return false;
	if (!zoneCont.loadFromFile(fs, theZones, true)) return false;
	if (!zoneCont.removeCalculation(theZones, calcIndex)) return false;

	// if no calculations remain than delete file
	if (zoneCont.numZones == 0) {
		fs.close();
		wstring filename;
		zoneCont.makeFileNameFromZones(theZones, filename, dbDirectory, dbFileType);
		_wremove(filename.c_str());
	// otherwise write the rest
	} else {
		if (!zoneCont.saveToFile(fs)) return false;
		fs.close();
	}
	
	// return value
	return true;
}

#pragma endregion

#pragma region calculation functions
//-----------------------------------------------------------------------------
// Name: searchSequences()
// Desc: Return sequences, which changes at least 1 and maximal maxChangesInRelevantZone relevant squares.
//	in:	 theZones[fieldSize]:			PGS_SS_FIX for squares of the field, which musn't change their value.
//	in:									PGS_SS_VAR when it doesn't matter and PGS_SS_REL for a relevant square.
//	in:	 initialBlockedMoves[numMoves]:	PGS_BS_BLOCKED when the move isn't 	allowed, otherwise PGS_BS_ALLOWED.
//	in:	 curSeqBlock[numMoves^2]:		Index is of type [A*numMoves+B]. When move A is performed then B is not allowed when the value is PGS_SB_BLOCKED,
//	in:									is allowed when the value is PGS_SB_RELEASE and not changed when the value is PGS_SB_NEUTRAL.
//	in:	 seqBlUser:						If not null this indicates that the user calculates the possibles moves.
//	in:	 callShowProgressUntilDepth:	The virtual function showProgress() is called until this search depth.
//	in:	 maxChangesInRelevantZone:		Maximum number of changes in the relevant zone.
//	in:	 maxLengthOfSequence:			Maximum length of returned sequences.
//	in:	 numMoves:						Number of moves defined in indicesMapsOfMoves.
//	in:	 indicesMapsOfMoves[numMoves*fieldSize]:	Index is of type [A*fieldSize+B]. When move A is performed then the value from square B is put to
//	in:												square with index indicesMapsOfMoves[].
//	in:	 inverseMoves[numMoves]:		Set value to numMoves, when a move don't have an inverse one, otherwise the index of the inverse move.
//	in:	 removeSequenceDuplicates:		When the indicesMap of two sequences are equal in the relevant stones then one sequence is removed from the list.
//	in:	 overwriteSequenceFile:			If true then existing sequence database files are overwritten.
//	in:	 useTheDoubleFeature:			The sequencelength is doubled by applying reverse moves.
//  in:  saveSequencesToFile:			If true the sequences are written to the database.
//-----------------------------------------------------------------------------
list<sequenceStruct*> permutationGameSolver::searchSequences(	vectui&					theZones,						
																vectui*					initialBlockedMoves,			
																vectui*					curSeqBlock, 					
																pgsSeqBlUserClass	*	seqBlUser,						
																unsigned int 			callShowProgressUntilDepth, 	
																unsigned int 			minChangesInRelevantZone,  		
																unsigned int 			maxChangesInRelevantZone,		
																unsigned int 			maxLengthOfSequence,			
																unsigned int 			numMoves,						
																vectui&					indicesMapsOfMoves,			
																vectui*					inverseMoves,					
																bool		 			removeSequenceDuplicates,		
																bool		 			useMaxDepth,					
																bool		 			useTheDoubleFeature,			
																bool		 			useTheSingleReturnFeature,		
																bool		 			overWriteDatabase,				
																bool					saveSequencesToFile)
{
	// locals
	DWORD		 				elapsedMilliSeconds;
	FILETIME	 				ftTime, ft1, ft2, ft3;
	ULONGLONG	 				qwResult;
	unsigned int				i;
	useSingleReturn				= false;
	useDoubleFeature			= false;
	abortCalculation			= false;
	showProgressDepth			= callShowProgressUntilDepth;

	// clear list
	sequenceStruct::clearList(foundSequences);

	// parameters ok?
	if (indicesMapsOfMoves.empty()) return foundSequences;
	if (theZones		  .empty()) return foundSequences;

	// user defined blocked sequences?
	if (seqBlUser != NULL) {
		seqBlUser->copyTo(numMoves, &this->seqBlUser);
	}
	
	// single return feature
	if (useTheSingleReturnFeature) {
		useSingleReturn		 = true;		
		srNumMovesOpen		 = 0;
		srMoveWasClosed		 .resize(2*maxLengthOfSequence+1);
		srIsOpenMove		 .resize(numMoves);
		for (i=0; i<numMoves;				i++) srIsOpenMove[i]	= 0;
		for (i=0; i<2*maxLengthOfSequence;	i++) srMoveWasClosed[i] = false;
		maxLengthOfSequence *= 2;
	}

	// double feature
	if (useTheDoubleFeature) {
		dfNumForwardMoves	= 0;
		useDoubleFeature	= true;
		dfList				.resize(fieldSize * maxLengthOfSequence);
		dfCurIndex			.resize(fieldSize);
		dfMoveIsForward		.resize(2 * maxLengthOfSequence * numMoves);
		for (i=0; i<fieldSize; i++)							dfCurIndex[i] = 0;
		for (i=0; i<fieldSize * maxLengthOfSequence; i++)	dfList[i]	  = numMoves;
		maxLengthOfSequence *= 2;
	}

	// prepare backTracking
	minDiffInRelZone			= minChangesInRelevantZone;
	maxDiffInRelZone			= maxChangesInRelevantZone;
	removeSeqDuplicates			= removeSequenceDuplicates;
	numSequences				= numMoves;
	useMaxSearchDepth			= useMaxDepth;
	curState.lastSeq			= numSequences + 1;
	searchingSequences			= true;

	// make new arrays
	newFoundSeq.indicesMap		.resize(fieldSize);
	newFoundSeq.solPath.path	.resize(maxLengthOfSequence+1);
	curState.blockedSeq			.resize(numSequences);
	seqBlock					.resize(numSequences * numSequences);
	indicesMaps					.resize(fieldSize * numSequences);
	inverseSeq					.resize(numSequences);
	relStones					.resize(fieldSize);
    idPossibilities     		.resize(numSequences * maxLengthOfSequence);
	inverseIndicesMap			.resize(fieldSize);
	tmpField					.resize(fieldSize);

	// fill new arrays
	indicesMaps					= indicesMapsOfMoves;
	zones						= theZones;
	
	for (i=0; i < fieldSize; i++) { 
		curState.field[i]	= i;
		cmpField[i]			= i;
	}
	
	getMemForStates(oldStates, maxLengthOfSequence, fieldSize, numSequences);

	if (inverseMoves != NULL)			inverseSeq = *inverseMoves;
	else 								fill(inverseSeq.begin(), inverseSeq.end(), numSequences);

	if (initialBlockedMoves != NULL)	curState.blockedSeq = *initialBlockedMoves;
	else 								fill(curState.blockedSeq.begin(), curState.blockedSeq.end(), PGS_BS_ALLOWED);
	
	if (curSeqBlock != NULL)			seqBlock	=  *curSeqBlock;
	else								fill(seqBlock.begin(), seqBlock.end(), PGS_SB_RELEASE);
	
	// count relevant stones
	for (numRelStones=0, i=0; i<fieldSize; i++) if (zones[i] == PGS_SS_REL) {
		relStones[numRelStones] = i;
		numRelStones++;
	}

	// get solutions
	searchSolution(NULL, maxLengthOfSequence, maxNumSolutions);

	// add sequences to container file
	if (!abortCalculation && saveSequencesToFile) {

		// calc elapsed time
		GetThreadTimes(GetCurrentThread(), &ft1, &ft2, &ft3, &ftTime);
		qwResult				= (((ULONGLONG) ftTime.dwHighDateTime) << 32) + ftTime.dwLowDateTime;
		elapsedMilliSeconds		= (DWORD) (qwResult / 10000);

		// reset "curState.blockedSeq", since it has been modified by searchSolution()
		if (initialBlockedMoves != NULL)	curState.blockedSeq = *initialBlockedMoves;
		else 								fill(curState.blockedSeq.begin(), curState.blockedSeq.end(), PGS_BS_ALLOWED);

		// save it!
		saveSequences(theZones, foundSequences, numMoves, maxLengthOfSequence, useTheSingleReturnFeature, useTheDoubleFeature, curState.blockedSeq, seqBlock, elapsedMilliSeconds, overWriteDatabase);
	}

	// release mem
	this->seqBlUser.~pgsSeqBlUserClass();
	deleteStatesFromMemory(oldStates);

	inverseIndicesMap		.clear();
	idPossibilities			.clear();
    indicesMaps				.clear();
	seqBlock				.clear();
	curState.blockedSeq		.clear();
	inverseSeq				.clear();
	relStones				.clear();

	if (useDoubleFeature) {
		dfList		   .clear();
		dfCurIndex	   .clear();
		dfMoveIsForward.clear();
	}
	if (useSingleReturn) {
		srIsOpenMove	.clear();
		srMoveWasClosed	.clear();
	}

	// return value
	return foundSequences;
}

//-----------------------------------------------------------------------------
// Name: finishZone()
// Desc: The backtracking algorithmn is applied to search a path leading to a field where the field is finished in the relevant zone.
//		 currentField[fieldSize]:		The starting state.
//		 theZones[fieldSize]:			PGS_SS_FIX for squares of the field, which musn't change their value.
//										PGS_SS_VAR when it doesn't matter and PGS_SS_REL for a relevant square.
//		 initialBlockedMoves[numMoves]:	PGS_BS_BLOCKED when the move isn't 	allowed, otherwise PGS_BS_ALLOWED.
//		 curSeqBlock[numMoves^2]:		Index is of type [A*numMoves+B]. When move A is performed then B is not allowed when the value is PGS_SB_BLOCKED,
//										is allowed when the value is PGS_SB_RELEASE and not changed when the value is PGS_SB_NEUTRAL.
//		 searchDepth:					Depth of search.
//		 useSequences:					List containing the sequences, which shall be used for the search.
//-----------------------------------------------------------------------------
void permutationGameSolver::finishZone(list<backTracking::solutionPath> &	solPath,						// out:
									   vectui&								currentField,					// in: 
									   vectui&								theZones,						// in: 	  
									   vectui*								initialBlockedMoves, 			// in: 
									   vectui*								curSeqBlock,					// in: 
									   pgsSeqBlUserClass	 *				seqBlUser,						// in: 
									   unsigned int							callShowProgressUntilDepth,		// in: 
									   unsigned int							searchDepth,	  				// in: 
									   list<sequenceStruct*> &				useSequences,					// in: 
									   bool									useMaxDepth,		  			// in: 
									   bool									returnShortestSolutionPath)		// in: 
{
	// locals
	unsigned int					i, j, h;
	list<sequenceStruct*>::iterator	itrSeq, itrSqs1, itrSqs2;
	list<solutionPath>::iterator	itrSolPath, itrShortest, itrSolPathTmp;
	solutionPath					mySolPath;
	unsigned int					lengthOfShortestSolution;

	// prepare backTracking
	for (i=0; i < fieldSize; i++) curState.field[i] = currentField[i];
	for (i=0; i < fieldSize; i++) cmpField[i]		= finalField[i];
	for (i=0; i < fieldSize; i++) zones[i]			= theZones[i];
	
	minDiffInRelZone    		= 0;
	maxDiffInRelZone			= 0;
	useDoubleFeature			= false;
	useSingleReturn				= false;
	searchingSequences			= false;
	abortCalculation			= false;
	showProgressDepth			= callShowProgressUntilDepth;
	useMaxSearchDepth			= useMaxDepth;
	numSequences				= (unsigned int) useSequences.size();
	curState.lastSeq			= numSequences + 1;
	newFoundSeq.indicesMap		.resize(fieldSize);
	newFoundSeq.solPath.path	.resize(searchDepth+1);
	curState.blockedSeq			.resize(numSequences);
	indicesMaps					.resize(fieldSize * numSequences);
    idPossibilities     		.resize(numSequences * searchDepth);
	inverseSeq					.resize(numSequences);
	inverseIndicesMap			.resize(fieldSize);
	tmpField					.resize(fieldSize);

	// user defined blocked sequences?
	if (seqBlUser != NULL) {
		seqBlUser->copyTo(numSequences, &this->seqBlUser);
	}

	// clear list
	solPath.clear();

	// allow all sequences for the first move if initialBlockedMoves is NULL
	if (initialBlockedMoves != NULL) {
		curState.blockedSeq = *initialBlockedMoves;
	} else {
		fill(curState.blockedSeq.begin(), curState.blockedSeq.end(), PGS_BS_ALLOWED);
	}
	
	getMemForStates(oldStates, searchDepth, fieldSize, numSequences);

	// allow all sequences if curSeqBlock is NULL
	if (curSeqBlock != NULL)	{
		seqBlock = *curSeqBlock;
	} else {
		seqBlock.clear();
	}

	// extract indicesMaps out of sequences and copy sequences to own array
	for (numSequences = 0, itrSeq = useSequences.begin(); itrSeq != useSequences.end(); itrSeq++, numSequences++) {
		for (i=0; i < fieldSize; i++) {
			indicesMaps[numSequences * fieldSize + i] = (*itrSeq)->indicesMap[i];
		}
	}

	// find inverse sequences
	if (numSequences<1000) {
		for (i=0; i<numSequences; i++) {
			for (j=0; j<numSequences; j++) {
				for (h=0; h<fieldSize; h++) {
					if (indicesMaps[i * fieldSize + indicesMaps[j * fieldSize + h]] != h) break;
				}
				if (h == fieldSize) break;
			}
			inverseSeq[i] = j;
		}
	} else {
		for (i=0; i<numSequences; i++) {
			inverseSeq[i] = numSequences;
		}
	}

	// count relevant stones
	for (j=0, i=0; i<fieldSize; i++) { if (zones[i] == PGS_SS_REL) j++; }
		
	// count finished stones
	for (h=0, i=0; i<fieldSize; i++) { if (zones[i] == PGS_SS_REL && curState.field[i] == finalField[i]) h++; }

	// zone already finished?
	if (!useMaxDepth && j == h) {
	
		mySolPath.numSteps	= 0;
		mySolPath.path		.clear();
		solPath.push_back(mySolPath);
	
	// search solution
	} else {
		searchSolution(&solPath, searchDepth, returnShortestSolutionPath?maxNumSolutions:1);
	}

	// delete all solutions appart from the shortest one
	if (returnShortestSolutionPath && solPath.size() > 1) {
		lengthOfShortestSolution = UINT_MAX;
		
		// find shortest one
		for (itrSolPath=solPath.begin(); itrSolPath!=solPath.end(); itrSolPath++) {
			for (j=0, i=0; i<(*itrSolPath).numSteps; i++) {
				for (h=0, itrSqs1=useSequences.begin(); h<(*itrSolPath).path[i]; h++, itrSqs1++) {} 
				j +=  (*itrSqs1)->solPath.numSteps;
			}
			if (lengthOfShortestSolution > j) {
				itrShortest					= itrSolPath;
				lengthOfShortestSolution	= j;
			}
		}

		// delete others
		if (itrShortest != solPath.begin()) {
			itrSolPath = itrShortest;
			for (itrSolPathTmp=solPath.begin(); itrSolPathTmp!=itrSolPath; itrSolPathTmp++) (*itrSolPathTmp).path.clear();
			solPath.erase(solPath.begin(), itrSolPath);
		} 
		if (itrShortest != solPath.end()) {
			itrSolPath = itrShortest;
			itrSolPath++;
			for (itrSolPathTmp=itrSolPath; itrSolPathTmp!=solPath.end(); itrSolPathTmp++) (*itrSolPathTmp).path.clear();
			solPath.erase(itrSolPath, solPath.end());
		}
	}

	// free memory
	this->seqBlUser.~pgsSeqBlUserClass();
	sequenceStruct::clearList(foundSequences);
	deleteStatesFromMemory(oldStates);

	inverseIndicesMap		.clear();
	idPossibilities			.clear();
    indicesMaps				.clear();
	seqBlock				.clear();
	curState.blockedSeq		.clear();
	inverseSeq				.clear();
	relStones				.clear();
}
#pragma endregion

#pragma region pgsSeqBlUserClass
//-----------------------------------------------------------------------------
// Name: pgsSeqBlUserClass()
// Desc: 
//-----------------------------------------------------------------------------
pgsSeqBlUserClass::pgsSeqBlUserClass()
{
	setDuplicatesMap	= nullptr;
	setBlockedSequences = nullptr;
	pUserData			= nullptr;
}

//-----------------------------------------------------------------------------
// Name: ~pgsSeqBlUserClass()
// Desc: 
//-----------------------------------------------------------------------------
pgsSeqBlUserClass::~pgsSeqBlUserClass()
{
	setBlockedSequences = nullptr;
	pUserData			= nullptr;
	setDuplicatesMap	= nullptr;
	excludedMoves		.clear();
}

//-----------------------------------------------------------------------------
// Name: copyTo()
// Desc: 
//-----------------------------------------------------------------------------
void pgsSeqBlUserClass::copyTo(unsigned int numMoves, pgsSeqBlUserClass *dest)
{
	dest->setDuplicatesMap		= this->setDuplicatesMap;
	dest->setBlockedSequences	= this->setBlockedSequences;
	dest->pUserData				= this->pUserData;
	dest->excludedMoves			= this->excludedMoves;
}
#pragma endregion

#pragma region sequenceStruct
//-----------------------------------------------------------------------------
// Name: sequenceStruct::sequenceStruct()
// Desc: 
//-----------------------------------------------------------------------------
sequenceStruct::sequenceStruct()
{
}

//-----------------------------------------------------------------------------
// Name: sequenceStruct::sequenceStruct()
// Desc: 
//-----------------------------------------------------------------------------
sequenceStruct::sequenceStruct(const sequenceStruct& sourceSeqStruct)
{
	this->indicesMap		= sourceSeqStruct.indicesMap;
	this->solPath.path		= sourceSeqStruct.solPath.path;
	this->solPath.numSteps	= sourceSeqStruct.solPath.numSteps;
}

//-----------------------------------------------------------------------------
// Name: sequenceStruct::sequenceStruct()
// Desc: 
//-----------------------------------------------------------------------------
sequenceStruct::sequenceStruct(vectui& indicesMap, vectui& path)
{
	this->indicesMap		= indicesMap;
	this->solPath.path		= path;
	this->solPath.numSteps	= path.size();
}

//-----------------------------------------------------------------------------
// Name: sequenceStruct::sequenceStruct()
// Desc: 
//-----------------------------------------------------------------------------
sequenceStruct::sequenceStruct(unsigned int numFields, unsigned int* indicesMap, unsigned int numSteps, unsigned int* path)
{
	this->indicesMap		.assign(indicesMap, indicesMap + numFields);
	this->solPath.path		.assign(path,		path + numSteps);
	this->solPath.numSteps	= numSteps;	
}

//-----------------------------------------------------------------------------
// Name: sequenceStruct::~sequenceStruct()
// Desc: 
//-----------------------------------------------------------------------------
sequenceStruct::~sequenceStruct()
{
}

//-----------------------------------------------------------------------------
// Name: sequenceStruct::deleteArrays()
// Desc: 
//-----------------------------------------------------------------------------
void sequenceStruct::deleteArrays()
{
	indicesMap		 .clear();
	solPath.path	 .clear();
	solPath.numSteps = 0;
}

//-----------------------------------------------------------------------------
// Name: sequenceStruct::copy()
// Desc: 
//-----------------------------------------------------------------------------
void sequenceStruct::copy(sequenceStruct &from)
{
	this->indicesMap		= from.indicesMap;
	this->solPath.path		= from.solPath.path;
	this->solPath.numSteps	= from.solPath.numSteps;
}

//-----------------------------------------------------------------------------
// Name: clearList()
// Desc: 
//-----------------------------------------------------------------------------
void sequenceStruct::clearList(list<sequenceStruct*> &listToClear)
{
	for (auto& itr : listToClear) {
		itr->deleteArrays();
		delete itr;
	}
	listToClear.clear();
}
#pragma endregion