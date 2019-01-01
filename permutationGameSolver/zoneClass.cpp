/*********************************************************************
	zoneClass.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/

#include "zoneClass.h"

//-----------------------------------------------------------------------------
// Name: zoneClass()
// Desc: Constructor
//-----------------------------------------------------------------------------
zoneClass::zoneClass()
{
}

//-----------------------------------------------------------------------------
// Name: ~zoneClass()
// Desc: Destructor
//-----------------------------------------------------------------------------
zoneClass::~zoneClass()
{
	release();
}

//-----------------------------------------------------------------------------
// Name: release()
// Desc: 
//-----------------------------------------------------------------------------
void zoneClass::release()
{
	deleteZonePropArray(zoneProp, numZones);
	zoneId					.clear();
	defBlockedMoves 		.clear();
	defInitialBlockedMoves	.clear();
	seqBlUser				.~pgsSeqBlUserClass();
}

//-----------------------------------------------------------------------------
// Name: setNewScenario()
// Desc: 
//-----------------------------------------------------------------------------
void zoneClass::setNewScenario(unsigned int newNumMoves, unsigned int newNumFields)
{
	release();
	numZones  = 0;
	numMoves  = newNumMoves;
	numFields = newNumFields;
	zoneId    .resize(newNumFields, 0);
}

//-----------------------------------------------------------------------------
// Name: copyZoneProp()
// Desc: 
//-----------------------------------------------------------------------------
void zoneClass::copyZoneProp(zoneStruct* from, zoneStruct *to, unsigned int numMoves)
{
	to->numRectangles				= from->numRectangles				;
	to->overWriteDatabase			= from->overWriteDatabase			;
	to->backTrackingDepth			= from->backTrackingDepth			;
	to->useDoubleFeature			= from->useDoubleFeature			;
	to->useSingleReturn				= from->useSingleReturn				;
	to->maxSequenceLength			= from->maxSequenceLength			;
	to->searchDepthUsingSequences	= from->searchDepthUsingSequences	;
	to->excludedMove				= from->excludedMove				;
}

//-----------------------------------------------------------------------------
// Name: errorOccured()
// Desc: 
//-----------------------------------------------------------------------------
void zoneClass::errorOccured()
{
	return;
}

//-----------------------------------------------------------------------------
// Name: deleteZonePropArray()
// Desc: 
//-----------------------------------------------------------------------------
void zoneClass::deleteZonePropArray(vector<zoneStruct>& zoneProp, unsigned int numZones)
{
	// locals
	unsigned int i;

	for (i=0; i<numZones; i++) {
		zoneProp[i].excludedMove.clear();
	}
	zoneProp.clear();
}

//-----------------------------------------------------------------------------
// Name: makeNewZonePropArray()
// Desc: 
//-----------------------------------------------------------------------------
void zoneClass::makeNewZonePropArray(vector<zoneStruct>& zoneProp, unsigned int numZones, unsigned int numMoves)
{
	// locals
	unsigned int i;

	zoneProp.resize(numZones);
	for (i=0; i<numZones; i++) {
		zoneProp[i].excludedMove.resize(numMoves);
	}
}

//-----------------------------------------------------------------------------
// Name: copyTo()
// Desc: 
//-----------------------------------------------------------------------------
void zoneClass::copyTo(zoneClass* dest)
{
	// locals
	unsigned int i;

	dest->setNewScenario(numMoves, numFields);
	dest->makeNewZonePropArray(dest->zoneProp, numZones, numMoves);
	for (i=0; i<numZones;  i++) copyZoneProp(&zoneProp[i], &dest->zoneProp[i], numMoves);
	dest->zoneId					= this->zoneId;
	dest->defInitialBlockedMoves	= this->defInitialBlockedMoves;
	dest->defBlockedMoves			= this->defBlockedMoves;
	this->seqBlUser					.copyTo(numMoves, &dest->seqBlUser);
	dest->numZones					= numZones;
}

//-----------------------------------------------------------------------------
// Name: insertZone()
// Desc: 
//-----------------------------------------------------------------------------
bool zoneClass::insertZone(unsigned int zone)
{
	// locals
	unsigned int		i;
	vector<zoneStruct>	zonePropTmp;
	makeNewZonePropArray(zonePropTmp, numZones+1, numMoves);

	// parameter ok?
	if (zone > numZones && (numZones > 0 || zone > 0)) return false;

	// copy following zones
	for (i=0;    i<zone;     i++) copyZoneProp(&zoneProp[i], &zonePropTmp[  i], numMoves);
	for (i=zone; i<numZones; i++) copyZoneProp(&zoneProp[i], &zonePropTmp[i+1], numMoves);

	// zero properties of new zone
	zonePropTmp[zone].numRectangles					= 0;
	zonePropTmp[zone].overWriteDatabase				= 0;
	zonePropTmp[zone].backTrackingDepth				= 0;
	zonePropTmp[zone].useDoubleFeature				= 0;
	zonePropTmp[zone].useSingleReturn				= 0;
	zonePropTmp[zone].maxSequenceLength				= 0;
	zonePropTmp[zone].searchDepthUsingSequences		= 0;
	for (i=0; i<numMoves; i++) {
		zonePropTmp[zone].excludedMove[i]			= 0;
	}

	// adjust zone id's
	for (i=0; i<numFields; i++) {
		if (zoneId[i] >= zone) {
			zoneId[i]++;
		}
	}

	// copy from tmp to ordinary array
	deleteZonePropArray(zoneProp, numZones);
	numZones++;
	makeNewZonePropArray(zoneProp, numZones, numMoves);
	for (i=0; i<numZones; i++) copyZoneProp(&zonePropTmp[i], &zoneProp[i], numMoves);
	deleteZonePropArray(zonePropTmp, numZones);

	return true;
}

//-----------------------------------------------------------------------------
// Name: moveZoneUp()
// Desc: 
//-----------------------------------------------------------------------------
bool zoneClass::moveZoneUp(unsigned int zone)
{
	if (zone >= numZones) return false;
	if (zone == 0       ) return false;
	
	return exchangeZones(zone, zone-1);
}

//-----------------------------------------------------------------------------
// Name: moveZoneDown()
// Desc: 
//-----------------------------------------------------------------------------
bool zoneClass::moveZoneDown(unsigned int zone)
{
	if (zone >= numZones  ) return false;
	if (zone == numZones-1) return false;
	
	return exchangeZones(zone, zone+1);
}

//-----------------------------------------------------------------------------
// Name: calcPossToBranch()
// Desc: 
//-----------------------------------------------------------------------------
void zoneClass::calcPossToBranch(unsigned int fromZone, vectui& possToBranch)
{
	// locals
	unsigned int i, numPossibleMoves;
	possToBranch.resize(numMoves, numMoves);

	if (fromZone>=numZones) return;

	for (i=0, numPossibleMoves=0; i<numMoves; i++) {
		if (zoneProp[fromZone].excludedMove[i] == 0) {
			possToBranch[i] = numPossibleMoves;
			numPossibleMoves++;
		} else {
			possToBranch[i] = numMoves;
		}
	}
}

//-----------------------------------------------------------------------------
// Name: moveZoneDown()
// Desc: 
//-----------------------------------------------------------------------------
unsigned int zoneClass::calcNumPossibleMoves(unsigned int fromZone)
{
	// locals
	unsigned int i, numPossibleMoves;

	if (fromZone>=numZones) return 0;

	for (i=0, numPossibleMoves=0; i<numMoves; i++) {
		if (zoneProp[fromZone].excludedMove[i] == 0) numPossibleMoves++;
	}
	return numPossibleMoves;
}

//-----------------------------------------------------------------------------
// Name: calcMaxNumSequences()
// Desc: 
//-----------------------------------------------------------------------------
unsigned int zoneClass::calcMaxNumSequences(unsigned int fromZone)
{
	// locals
	unsigned int curRectangle;
	unsigned int result				= 1;
	unsigned int numRelAndVarFields = 0;

	// calc number of relevant and variable fields for the current zone
	for (curRectangle=0; curRectangle<numFields; curRectangle++) {
			 if (fromZone     > zoneId[curRectangle]) 	{ }
		else if (fromZone    == zoneId[curRectangle]) 	numRelAndVarFields++;
		else if (numZones	  > zoneId[curRectangle])   numRelAndVarFields++;
		else 											{ }
	}

	// calc the number of permutations
	for (curRectangle=0; curRectangle<zoneProp[fromZone].numRectangles; curRectangle++) {
		if (result * numRelAndVarFields - curRectangle > 100000000) return 0;
		result *= numRelAndVarFields - curRectangle;
	}

	// -1, since one situation is the starting one
	return result - 1;
}

//-----------------------------------------------------------------------------
// Name: exchangeZones()
// Desc: 
//-----------------------------------------------------------------------------
bool zoneClass::exchangeZones(unsigned int zone1, unsigned int zone2)
{
	// locals
	unsigned int i;
	vector<zoneStruct>	zonePropTmp;
	makeNewZonePropArray(zonePropTmp, 1, numMoves);

	// parameter ok?
	if (zone1 >= numZones) return false;
	if (zone2 >= numZones) return false;

	// copy following zones
	copyZoneProp(&zoneProp[zone1], &zonePropTmp[ 0], numMoves);
	copyZoneProp(&zoneProp[zone2], &zoneProp[zone1], numMoves);
	copyZoneProp(&zonePropTmp[ 0], &zoneProp[zone2], numMoves);

	// adjust zone id's
	for (i=0; i<numFields; i++) {
		if (zoneId[i] == zone1) {
			zoneId[i] = zone2;
		} else if (zoneId[i] == zone2) {
			zoneId[i] = zone1;
		}
	}

	deleteZonePropArray(zonePropTmp, 1);

	return true;
}


//-----------------------------------------------------------------------------
// Name: deleteZone()
// Desc: 
//-----------------------------------------------------------------------------
bool zoneClass::deleteAllZones()
{
	// delete zones
	while (numZones) {
		if (!deleteZone(numZones - 1)) return false;
	}
	return true;
}

//-----------------------------------------------------------------------------
// Name: deleteZone()
// Desc: 
//-----------------------------------------------------------------------------
bool zoneClass::deleteZone(unsigned int zone)
{
	// locals
	unsigned int i;

	// parameter ok?
	if (zone >= numZones) return false;

	// copy following zones
	for (i=zone; i<numZones-1; i++) copyZoneProp(&zoneProp[i+1], &zoneProp[i], numMoves);

	// adjust zone id's
	for (i=0; i<numFields; i++) {
		if (zoneId[i] > zone) {
			zoneId[i]--;
		} else if (zoneId[i] == zone) {
			zoneId[i] = numZones-1;
		}
	}

	numZones--;

	return true;
}

//-----------------------------------------------------------------------------
// Name: setDefaultBlockedMoves()
// Desc: 
//-----------------------------------------------------------------------------
bool zoneClass::setDefaultBlockedMoves(vectui& initialBlockedMoves, vectui& seqBlock)
{
	if (seqBlock			.size() != numMoves*numMoves) return false;
	if (initialBlockedMoves	.size() != numMoves			) return false;

	this->seqBlUser					.~pgsSeqBlUserClass();
	this->defBlockedMoves			= seqBlock;
	this->defInitialBlockedMoves	= initialBlockedMoves;

	return true;
}

//-----------------------------------------------------------------------------
// Name: setDefaultBlockedMoves()
// Desc: Caution: seqBlUser.excludedMoves will be ignored in function translateForPGS() - zoneProp[zone].excludedMove[] is used instead!
//-----------------------------------------------------------------------------
bool zoneClass::setDefaultBlockedMoves(vectui& initialBlockedMoves, pgsSeqBlUserClass* seqBlUser)
{
	// parameters ok?
	if (initialBlockedMoves	.size()		!= numMoves			) return false;
	if (seqBlUser						== NULL				) return false;
	if (seqBlUser->setBlockedSequences	== NULL				) return false;

	this->defInitialBlockedMoves		= initialBlockedMoves;
	this->defBlockedMoves				.clear();
	seqBlUser->copyTo(numMoves, &this->seqBlUser);

	return true;
}

//-----------------------------------------------------------------------------
// Name: translateZonesForPGS()
// Desc: 
//-----------------------------------------------------------------------------
bool zoneClass::translateForPGS(unsigned int zone, vectui& theZone)
{
	// locals
	unsigned int j;

	// make array
	theZone.resize(numFields);

	// extract zones
	for (j=0; j<numFields; j++) {
			 if (zone     > zoneId[j]) 	theZone[j] = PGS_SS_FIX;
		else if (zone    == zoneId[j]) 	theZone[j] = PGS_SS_REL;
		else if (numZones > zoneId[j])  theZone[j] = PGS_SS_VAR;
		else 							theZone[j] = PGS_SS_FIX;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Name: translateForPGS()
// Desc: 
//   Example for zone = 2 and numZones = 5:
//	 zoneId[]		theZones[]
//   1 2 4			PGS_SS_FIX	PGS_SS_REL	PGS_SS_VAR
//   0 9 2	  --->	PGS_SS_FIX	PGS_SS_FIX	PGS_SS_REL
//   1 3 3			PGS_SS_FIX	PGS_SS_VAR	PGS_SS_VAR
//-----------------------------------------------------------------------------
bool zoneClass::translateForPGS(unsigned int 			zone,					// in:  zone number
								vectui&					theZones,				// out: [numFields] 
								vectui&					seqBlock,				// out: [numMoves * numMoves]
								vectui&					initialBlockedMoves,	// out: [numMoves]
								pgsSeqBlUserClass	* &	seqBlUser)				// out: [1]
{
	// parameters ok?
	if (zone				>= numZones)	return false;
//	if (theZones			!= NULL)		return false;
//	if (initialBlockedMoves != NULL)		return false;
//	if (seqBlock			!= NULL)		return false;
//	if (seqBlUser			!= NULL)		return false;

	// locals
	unsigned int j, k;
	
	// make arrays
	initialBlockedMoves 			.resize(numMoves);

	// calc array "theZones"
	translateForPGS(zone, theZones);

	// user defined sequence block mechanism
	if (this->seqBlUser.setBlockedSequences != NULL) {
		seqBlock					.clear();
		seqBlUser					= new pgsSeqBlUserClass();
		this->seqBlUser.copyTo(numMoves, seqBlUser);
		seqBlUser->excludedMoves	= zoneProp[zone].excludedMove;

	// predefined automatic mechanism
	} else {

		seqBlUser					= NULL;
		seqBlock					.resize(numMoves * numMoves);
			
		// calc sequenceBlock
		for(j=0; j<numMoves; j++) { for(k=0; k<numMoves; k++) {
			     if (zoneProp[zone].excludedMove[j] != 0) 						seqBlock[j * numMoves + k] = PGS_SB_BLOCKED;
			else if (defBlockedMoves.empty())									seqBlock[j * numMoves + k] = PGS_SB_RELEASE;
			else {
				     if (defBlockedMoves[j * numMoves + k] == PGS_SB_BLOCKED) 	seqBlock[j * numMoves + k] = PGS_SB_BLOCKED;
				else if	(defBlockedMoves[j * numMoves + k] == PGS_SB_NEUTRAL)	seqBlock[j * numMoves + k] = PGS_SB_NEUTRAL;
				else															seqBlock[j * numMoves + k] = PGS_SB_RELEASE;
			}
		}}
	}

	// calc initialBlockedMoves
	for(j=0; j<numMoves; j++) {
		     if (zoneProp[zone].excludedMove[j] != 0)			initialBlockedMoves[j] = PGS_BS_BLOCKED;
		else if (defInitialBlockedMoves.empty())				initialBlockedMoves[j] = PGS_BS_ALLOWED;
		else { 
			if (defInitialBlockedMoves[j] == PGS_BS_BLOCKED)	initialBlockedMoves[j] = PGS_BS_BLOCKED;
			else												initialBlockedMoves[j] = PGS_BS_ALLOWED;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// Name: vanishDuplicates()
// Desc: Searches the passed sourceMoveIndicesMap[] for duplicates and deletes them.
//       numMoves is beeing changed to the number of unique moves without duplicates.
//-----------------------------------------------------------------------------
bool zoneClass::vanishDuplicates(vector<unsigned int>& sourceMoveIndicesMap,	// in:  array of size [numMoves (before function call)]
								 vector<unsigned int>& sourceReverseMoves,		// in:  array of size [numMoves (before function call)]
								 vector<unsigned int>* targetMoveIndicesMap,	// out: array of size [numMoves (after function call)]
								 vector<unsigned int>* targetReverseMoves,		// out: array of size [numMoves (after function call)]
								 vector<unsigned int>* duplicatesMap,			// out: array of size [numMoves (after function call)] - contains the index in the corresponding sourceMoveIndicesMap[] array
								 vector<unsigned int>* inverseDuplicatesMap)	// out: array of size [numMoves (before function call)]
{
	// locals
	unsigned int	uniqueMoveId, moveId, field, numberOfUniqueMoves, curZone, j, k;
	vectui			duplicatesMapTmp		(numMoves);
	vectui			inverseDuplicatesMapTmp	(numMoves);
	vectui			tmpExcludedMove, tmpDefSeqBlock, tmpDefInitialBlockedMoves, tmpTargetMoveIndicesMap;

	// parameters ok?
	if (!sourceMoveIndicesMap.size()) return false;

	// detect duplicates
	for (moveId=0, numberOfUniqueMoves=0; moveId<numMoves; moveId++) {

		// is it trivial map?
		for (field=0; field<numFields; field++) {
			if (sourceMoveIndicesMap[moveId*numFields+field] != field) {
				break;
			}
		}
		if (field==numFields) {
			continue;	
		}

		// does the current move differ from each unique move in the list?
		for (uniqueMoveId=0; uniqueMoveId<numberOfUniqueMoves; uniqueMoveId++) {
			for (field=0; field<numFields; field++) {
				if (sourceMoveIndicesMap[moveId*numFields+field] != sourceMoveIndicesMap[duplicatesMapTmp[uniqueMoveId]*numFields+field]) {
					break;
				}
			}
			// are moves equal?
			if (field==numFields) {
				inverseDuplicatesMapTmp[moveId] = uniqueMoveId;
				break;
			}
		}
		// yes, the current move does differ so add it as unique move
		if (uniqueMoveId==numberOfUniqueMoves) {
			duplicatesMapTmp[numberOfUniqueMoves] = moveId;
			inverseDuplicatesMapTmp[moveId] = numberOfUniqueMoves;
			numberOfUniqueMoves++;
		}
	}

	// copy moveIndicesMap and reverseMoves
	tmpTargetMoveIndicesMap.resize(numberOfUniqueMoves*numFields);
	if (targetReverseMoves != NULL) {
		targetReverseMoves->resize(numberOfUniqueMoves);
	}
	for (uniqueMoveId=0; uniqueMoveId<numberOfUniqueMoves; uniqueMoveId++) {
		for (field=0; field<numFields; field++) {
			tmpTargetMoveIndicesMap[uniqueMoveId*numFields+field] = sourceMoveIndicesMap[duplicatesMapTmp[uniqueMoveId]*numFields+field];
		}
	}

	// calc targetReverseMoves[]
	if (targetReverseMoves != NULL) {

		// process each unique move
		for (uniqueMoveId=0; uniqueMoveId<numberOfUniqueMoves; uniqueMoveId++) {

			// find reverse move
			for (j=0; j<numberOfUniqueMoves; j++) {

				for (field=0; field<numFields; field++) {
					if (tmpTargetMoveIndicesMap[uniqueMoveId*numFields+tmpTargetMoveIndicesMap[j*numFields+field]] != field) break;
				}

				if (field==numFields) break;
			}

			// save found reverse move
			if (j < numberOfUniqueMoves) {
				(*targetReverseMoves)[uniqueMoveId] = j;
			// no reverse move found
			} else {
				return false;
			}
		}

	}

	// adjust defBlockedMoves
	if (defBlockedMoves.size()) {
		tmpDefSeqBlock.resize(numberOfUniqueMoves*numberOfUniqueMoves);
		for (j=0; j<numberOfUniqueMoves; j++) {
			for (k=0; k<numberOfUniqueMoves; k++) {
				tmpDefSeqBlock[j * numberOfUniqueMoves + k] = defBlockedMoves[duplicatesMapTmp[j] * numMoves + duplicatesMapTmp[k]];
			}
		}
		defBlockedMoves = tmpDefSeqBlock;
	} 

	// adjust defInitialBlockedMoves
	if (defInitialBlockedMoves.size()) {
		tmpDefInitialBlockedMoves.resize(numberOfUniqueMoves);
		for (uniqueMoveId=0; uniqueMoveId<numberOfUniqueMoves; uniqueMoveId++) {
			tmpDefInitialBlockedMoves[uniqueMoveId] = defInitialBlockedMoves[duplicatesMapTmp[uniqueMoveId]];
		}
		defInitialBlockedMoves = tmpDefInitialBlockedMoves;
	}

	// adjust excludedMove[]
	for (curZone=0; curZone<numZones; curZone++) {
		if (zoneProp[curZone].excludedMove.size()) {
			tmpExcludedMove.resize(numberOfUniqueMoves);
			for (uniqueMoveId=0; uniqueMoveId<numberOfUniqueMoves; uniqueMoveId++) {
				tmpExcludedMove[uniqueMoveId] = zoneProp[curZone].excludedMove[duplicatesMapTmp[uniqueMoveId]];
			}
			zoneProp[curZone].excludedMove = tmpExcludedMove;
		}
	}

	// since move indexing has changed, other classes must be informed
	if (seqBlUser.setDuplicatesMap) seqBlUser.setDuplicatesMap(numFields, numMoves, numberOfUniqueMoves, &duplicatesMapTmp[0], &seqBlUser);

	// set new number of moves and return
	numMoves		= numberOfUniqueMoves;
	if (targetMoveIndicesMap != nullptr) { *targetMoveIndicesMap	= tmpTargetMoveIndicesMap;	}
	if (duplicatesMap		 != nullptr) { *duplicatesMap			= duplicatesMapTmp;			}
	if (inverseDuplicatesMap != nullptr) { *inverseDuplicatesMap	= inverseDuplicatesMapTmp;	}
	return true;
}

//-----------------------------------------------------------------------------
// Name: load()
// Desc: 
//-----------------------------------------------------------------------------
bool zoneClass::load(const wstring& filename)
{
	// locals
	HANDLE			hFile			= NULL;
	DWORD			bytesRead		= 0;
	DWORD			i;

	// parameters ok?
	if (filename.empty()) return false;

	// open file
	hFile = CreateFileW(filename.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	// handle valid ?
	if (hFile == INVALID_HANDLE_VALUE) return false;

	// delete everything
	release();

	// read vars
	ReadFile(hFile, &numMoves,  sizeof(numMoves),  &bytesRead, NULL);													CHECK_IO_OPERATION(hFile, bytesRead, sizeof(numMoves))
	ReadFile(hFile, &numFields, sizeof(numFields), &bytesRead, NULL);													CHECK_IO_OPERATION(hFile, bytesRead, sizeof(numFields))
	ReadFile(hFile, &numZones,  sizeof(numZones),  &bytesRead, NULL);													CHECK_IO_OPERATION(hFile, bytesRead, sizeof(numZones))

	// allocate memory
	zoneProp	.resize(numZones );
	zoneId		.resize(numFields);

	// read zone id's
	ReadFile(hFile, &zoneId[0], sizeof(unsigned int) * numFields, &bytesRead, NULL);									CHECK_IO_OPERATION(hFile, bytesRead, sizeof(unsigned int) * numFields)

	// read properties
	for (i=0; i<numZones; i++) {
		ReadFile(hFile, &zoneProp[i].numRectangles				, sizeof(unsigned int), &bytesRead, NULL);				CHECK_IO_OPERATION(hFile, bytesRead, sizeof(unsigned int))
		ReadFile(hFile, &zoneProp[i].overWriteDatabase			, sizeof(unsigned int), &bytesRead, NULL);				CHECK_IO_OPERATION(hFile, bytesRead, sizeof(unsigned int))
		ReadFile(hFile, &zoneProp[i].backTrackingDepth			, sizeof(unsigned int), &bytesRead, NULL);				CHECK_IO_OPERATION(hFile, bytesRead, sizeof(unsigned int))
		ReadFile(hFile, &zoneProp[i].useDoubleFeature			, sizeof(unsigned int), &bytesRead, NULL);				CHECK_IO_OPERATION(hFile, bytesRead, sizeof(unsigned int))
		ReadFile(hFile, &zoneProp[i].useSingleReturn			, sizeof(unsigned int), &bytesRead, NULL);				CHECK_IO_OPERATION(hFile, bytesRead, sizeof(unsigned int))
		ReadFile(hFile, &zoneProp[i].maxSequenceLength			, sizeof(unsigned int), &bytesRead, NULL);				CHECK_IO_OPERATION(hFile, bytesRead, sizeof(unsigned int))
		ReadFile(hFile, &zoneProp[i].searchDepthUsingSequences	, sizeof(unsigned int), &bytesRead, NULL);				CHECK_IO_OPERATION(hFile, bytesRead, sizeof(unsigned int))
		SetFilePointer(hFile, 4, 0, FILE_CURRENT);				// work-around for old file format, so that old 32 bit pointer to excludedMove is skipped
	}

	// read blocked moves
	for (i=0; i<numZones; i++) {
		zoneProp[i].excludedMove.resize(numMoves);
		ReadFile(hFile, &zoneProp[i].excludedMove[0],			sizeof(unsigned int) * numMoves, &bytesRead, NULL);		CHECK_IO_OPERATION(hFile, bytesRead, sizeof(unsigned int) * numMoves)
	}

	// close file
	CloseHandle(hFile);

	return true;
}

//-----------------------------------------------------------------------------
// Name: save()
// Desc: 
//-----------------------------------------------------------------------------
bool zoneClass::save(const wstring& filename)
{
	// locals
	HANDLE	hFile		= NULL;
	DWORD	bytesWritten= 0;
	DWORD	i;

	// parameters ok?
	if (filename .empty())	return false;
	if (zoneId   .empty())	return false;
	if (zoneProp .empty())	return false;
	for (i=0; i<numZones; i++) if (zoneProp[i].excludedMove .empty()) return false;
			
	// open file
	hFile = CreateFileW(filename.c_str(), GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	// handle valid ?
	if (hFile == INVALID_HANDLE_VALUE) return false;

	// save vars
	WriteFile(hFile, &numMoves,  sizeof(numMoves),	&bytesWritten, NULL);												CHECK_IO_OPERATION(hFile, bytesWritten, sizeof(numMoves))
	WriteFile(hFile, &numFields, sizeof(numFields), &bytesWritten, NULL);												CHECK_IO_OPERATION(hFile, bytesWritten, sizeof(numFields))
	WriteFile(hFile, &numZones,  sizeof(numZones),	&bytesWritten, NULL);												CHECK_IO_OPERATION(hFile, bytesWritten, sizeof(numZones))

	// save zone id's
	WriteFile(hFile, &zoneId[0], sizeof(unsigned int) * numFields, &bytesWritten, NULL);								CHECK_IO_OPERATION(hFile, bytesWritten, sizeof(unsigned int) * numFields)

	// save properties
	for (i=0; i<numZones; i++) {
		WriteFile(hFile, &zoneProp[i].numRectangles				, sizeof(unsigned int), &bytesWritten, NULL);			CHECK_IO_OPERATION(hFile, bytesWritten, sizeof(unsigned int))
		WriteFile(hFile, &zoneProp[i].overWriteDatabase			, sizeof(unsigned int), &bytesWritten, NULL);			CHECK_IO_OPERATION(hFile, bytesWritten, sizeof(unsigned int))
		WriteFile(hFile, &zoneProp[i].backTrackingDepth			, sizeof(unsigned int), &bytesWritten, NULL);			CHECK_IO_OPERATION(hFile, bytesWritten, sizeof(unsigned int))
		WriteFile(hFile, &zoneProp[i].useDoubleFeature			, sizeof(unsigned int), &bytesWritten, NULL);			CHECK_IO_OPERATION(hFile, bytesWritten, sizeof(unsigned int))
		WriteFile(hFile, &zoneProp[i].useSingleReturn			, sizeof(unsigned int), &bytesWritten, NULL);			CHECK_IO_OPERATION(hFile, bytesWritten, sizeof(unsigned int))
		WriteFile(hFile, &zoneProp[i].maxSequenceLength			, sizeof(unsigned int), &bytesWritten, NULL);			CHECK_IO_OPERATION(hFile, bytesWritten, sizeof(unsigned int))
		WriteFile(hFile, &zoneProp[i].searchDepthUsingSequences	, sizeof(unsigned int), &bytesWritten, NULL);			CHECK_IO_OPERATION(hFile, bytesWritten, sizeof(unsigned int))
		SetFilePointer(hFile, 4, 0, FILE_CURRENT);				// work-around for old file format, so that old 32 bit pointer to excludedMove is skipped
	}

	// save blocked moves
	for (i=0; i<numZones; i++) {
		WriteFile(hFile, &zoneProp[i].excludedMove[0],			 sizeof(unsigned int) * numMoves, &bytesWritten, NULL);	CHECK_IO_OPERATION(hFile, bytesWritten, sizeof(unsigned int) * numMoves)
	}

	// close file
	CloseHandle(hFile);

	return true;
}
