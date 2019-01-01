/*********************************************************************
	zoneClass.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/
#ifndef zoneClass_H
#define zoneClass_H

#include <vector>
#include <windows.h>
#include "permutationGameSolver.h"

#define	CHECK_IO_OPERATION(h,a,b) { if (a != b) { CloseHandle(h); return false; } }

struct zoneStruct
{
	unsigned int		numRectangles					= 0;						// number of fields, which belonds to this zone
	unsigned int		overWriteDatabase				= 0;						// delete all calculations of a container file before adding the new calculated ones
	unsigned int		backTrackingDepth				= 0;						// when solving a specific problem, back tracking until this depth is performed, before using prior calculated sequences
	unsigned int		useDoubleFeature				= 0;						// use the double feature
	unsigned int		useSingleReturn					= 0;						// use the single return feature
	unsigned int		maxSequenceLength				= 0;						// maximum number of moves used to build up a sequence
	unsigned int		searchDepthUsingSequences		= 0;						// when solving a specific problem, back tracking until this search depth is used with prior calculated sequences
	vectui				excludedMove;												// Array size is [numMoves]. If not zero, then this move will not be used. 
};

class zoneClass
{
protected: 

	// Variables
	vectui				defBlockedMoves;											// [numMoves*numMoves]	- can be PGS_SB_BLOCKED, PGS_SB_NEUTRAL or PGS_SB_RELEASE
	vectui				defInitialBlockedMoves;										// [numMoves]			- can be PGS_BS_BLOCKED or PGS_BS_ALLOWED
	pgsSeqBlUserClass	seqBlUser;													// used when the blocked sequences are controlled by a user function

	// Functions
	void				copyZoneProp					(zoneStruct* from, zoneStruct *to, unsigned int numMoves);
	void				deleteZonePropArray				(vector<zoneStruct>& zoneProp, unsigned int numZones);
	void				makeNewZonePropArray			(vector<zoneStruct>& zoneProp, unsigned int numZones, unsigned int numMoves);
	void				errorOccured					();

public:

	// Variables
	unsigned int		numMoves						= 0;						// total number of possible moves that can be performed
	unsigned int		numFields						= 0;						// number of fields
	unsigned int		numZones						= 0;						// number of zones
	vectui				zoneId;														// [numFields]			- index corresponding to array zoneProp[] for each field
	vector<zoneStruct>  zoneProp;													// [numZones]			- array containing the zones

	// Constructor / destructor
						zoneClass						();
						~zoneClass						();

	// Functions
	unsigned int		calcMaxNumSequences				(unsigned int fromZone);
	void				calcPossToBranch				(unsigned int fromZone, vectui& possToBranch);
	unsigned int    	calcNumPossibleMoves			(unsigned int fromZone);
	void				setNewScenario					(unsigned int newNumMoves, unsigned int newNumFields);
	bool				moveZoneUp						(unsigned int zone);
	bool				moveZoneDown					(unsigned int zone);
	bool				exchangeZones					(unsigned int zone1, unsigned int zone2);
	bool				insertZone						(unsigned int zone);
	bool				deleteZone						(unsigned int zone);
	bool				deleteAllZones					();
	void				release							();
	bool				load							(const wstring& filename);
	bool				save							(const wstring& filename);
	void				copyTo							(zoneClass* dest);
	bool				setDefaultBlockedMoves			(vectui& initialBlockedMoves, vectui& seqBlock);
	bool				setDefaultBlockedMoves			(vectui& initialBlockedMoves, pgsSeqBlUserClass* seqBlUser);
	bool				translateForPGS					(unsigned int zone, vectui& theZones, vectui& seqBlock, vectui& initialBlockedMoves, pgsSeqBlUserClass* &seqBlUser);
	bool				translateForPGS		 			(unsigned int zone, vectui& theZones);
	bool				vanishDuplicates				(vectui& sourceMoveIndicesMap, vectui& sourceReverseMoves, vectui* targetMoveIndicesMap, vectui* targetReverseMoves, vectui* duplicatesMap, vectui* inverseDuplicatesMap);
};

#endif
