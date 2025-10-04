/*********************************************************************\
	permutationGameSolver.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/
#ifndef PERMGAMESOLVER_H
#define PERMGAMESOLVER_H


#include <Windows.h>
#include <iostream>
#include <vector>
#include <intrin.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <codecvt>
#include "backTracking.h"

using namespace std;

#define	PGS_SS_FIX				0							// the square contains already the final stone
#define	PGS_SS_VAR				1							// the square contains a stone, which still can be changed
#define	PGS_SS_REL				2							// the square is part of the next zone with final stones

#define PGS_SB_BLOCKED			1							// Used with array seqBlock[] in order to determine the possible moves
#define PGS_SB_RELEASE			2							// after each move. 
#define PGS_SB_NEUTRAL			3							

#define	PGS_BS_BLOCKED			1							// Used with array curState.blockedSeq[]. Indicates that the move is not allowed.
#define	PGS_BS_ALLOWED			2							// Indicates that the move is allowed.

#define	PGS_STR_LENGTH			128							// default size for a char array

class sequenceStruct
{
public:
								sequenceStruct				(const sequenceStruct &sourceSeqStruct);
								sequenceStruct				(vectui& indicesMap, vectui& path);
								sequenceStruct				(unsigned int numFields, unsigned int* indicesMap, unsigned int numSteps, unsigned int* path);
								sequenceStruct				();
								~sequenceStruct				();

	backTracking::solutionPath	solPath;					// Moves describing a sequence.
	vectui						indicesMap;					// Array size is [numFields]. Represents the effect of this sequence.

	bool						loadFromFile				(istream& is, unsigned int numFields);
	bool						saveToFile					(ostream& os);
	bool						addIfNotDuplicate			(vectui& theZone, vector<sequenceStruct>& sequences, unsigned int& posInFile, unsigned int* posInVector);
	void						deleteArrays				();
	void						copy						(sequenceStruct &from);
	bool						equalTo						(vectui& theZone, sequenceStruct& anotherSeq);
	static void					clearList					(list<sequenceStruct*> &listToClear);
};

struct sequenceCalculation
{
	SYSTEMTIME					dateOfCalculation;			// timepoint at which the calculation has been started
	DWORD						calculationDuration;		// duration of the calculation
	char						cpuType[0x40];				// description of the computer system on which the calculation was done
	bool						useSingleReturn;			// calculation was done with useSingleReturn
	bool						useDoubleFeature;			// calculation was done with useDoubleFeature
	unsigned int				maxSearchDepth;				// the maximum search depth
	vectui						initialBlockedMoves;		// array size is [numMoves]
	vectui						seqBlock;					// array size is [numMoves*numMoves]
	unsigned int				numSequencesInFile;			// number of sequences found during this calculation
	vectui						sequenceIndexInFile;		// indices of seqContainer::sequences

	bool						loadFromFile				(istream& is, unsigned int numFields, unsigned int numMoves);
	bool						saveToFile					(ostream&  os);
};

struct seqContainer
{
	unsigned int				numFields;					// number of fields
	unsigned int				numMoves;					// number of possible moves
	unsigned int				numCalculations;			// number of performed calculations
	unsigned int				numSequences;				// number of found sequences in container
	LONG						offsetSequences;			// relative offset where the array sequences[] starts in file
	LONG						offsetCalculations;			// relative offset where the array calculations[] starts in file
	uint64_t			totalCalculationTime;		// total calculation time of all sequences in milliseconds
	vector<sequenceStruct>		sequences;					// the calculated sequences
	vector<sequenceCalculation>	calculations;				// the performed calculations

	bool						loadFromFile				(istream& is, bool loadSequences, bool loadCalculations);
	bool						saveToFile					(ostream& os);
	bool						addSeqContainer				(vectui& theZone, seqContainer& newSeqCont);
	bool						removeCalculation			(unsigned int calcIndex);
};

struct zoneContainer
{
	unsigned int				id = 0x32423;				// unique id
	unsigned int				numZones;					// number of zones, to which sequences are stored in the file
	unsigned int				numFields;					// number of fields
	vector<vectui>				zones;						// [numZones] each item is of size [numFields] and contains all stored zones in the file
	vectui						offsetSeqContainers;		// [numZones] absolut offset of each sequences container in the file
	vector<seqContainer>		seqContainers;				// [numZones]

	wstring						getHashStr					(const vectui& theZones);
	unsigned int				getSeqContIndex				(const vectui& theZones);
	void						makeFileNameFromZones		(const vectui& theZones, wstring& filename, wstring& dbDirectory, wstring& fileType);
	bool						convertToNewestType			(const vectui& theZones, fstream& fs, bool writeMode, wstring& filename);
	bool						openFile					(const vectui& theZones, fstream& fs, bool writeMode, wstring& dbDirectory, wstring& fileType);
	bool						loadFromFile				(istream& is, const vectui& theZones, bool loadSeqContainers);
	bool						setFilePointer				(istream& is, const vectui& theZones);
	bool						saveToFile					(ostream& os);
	bool						addSeqContainer				(const vectui& theZones, seqContainer& seqCont);
	bool						removeCalculation			(const vectui& theZones, unsigned int calcIndex);
};

struct sequenceStatistics
{
	vectui						zones;						//
	unsigned int				numSequences;				//
};

class pgsSeqBlUserClass										// contains information about sequences, which are blocked and thus not used by the solving algorithmn
{
public:
	// Constructur/Destructor
								pgsSeqBlUserClass			();
								~pgsSeqBlUserClass			();

	// Varibles
	void						(*setBlockedSequences)		(unsigned int moveId, vectui& curField, vectui& curBlockedSeq, bool searchingSequences, pgsSeqBlUserClass *me);
	void						(*setDuplicatesMap)			(unsigned int numFields, unsigned int numMoveIds, unsigned int numUniqueMoveIds, unsigned int *newDuplicatesMap, pgsSeqBlUserClass *me);
	void *						pUserData;					// pointer to user data, passed when calling setBlockedSequences()
	vectui						excludedMoves;				// [numSequences] These moves are not allowed at all.

	// Functions
	void						copyTo						(unsigned int numMoves, pgsSeqBlUserClass *dest);
};

class permutationGameSolver : public backTracking
{
protected: 
		
	struct stateStruct										// describes a state
	{
		vectui					blockedSeq;					// Array of size numSequences representing the allowed and forbidden sequences
		vectui					field;						// Array representing the current state of the game, while backtracking
		unsigned int			lastSeq;					// The last performed sequence
	};

	// Variables
	void					*	parentClassPointer;			// pointer which can be used from the user-class
	wstring						dbFileType;					// name of the file ending, e.g. "dbs"
	wstring						dbDirectory;				// Name of the directory for the sequencedatabase.
	bool						useDoubleFeature;			// True if "Double Feature" ist desired.
	bool						useSingleReturn;			// True if "Single Return Feature" is desired.
	bool						useMaxSearchDepth;			// a state is only found when the current search depth is equal to the maximum search depth
	bool						searchingSequences;			// true when function searchSequences was called
	bool						removeSeqDuplicates;		// true when only unique solutions are desired
	unsigned int				numRelStones;				// Number of relevant stones
	vectui						relStones;					// [fieldSize] Indices of relevant stones
	vectui						inverseSeq;					// [numSequences] numSequences when there is no inverse sequence, otherwise the index of the inverse sequence
	vectui						cmpField;					// [fieldSize] the curState.field is compared with this in the targetReached()-function
	vectui						initialField;				// [fieldSize] Array representing the initial state of the game
	vectui						finalField;					// [fieldSize] Array defining the final state of the game
	vectui						zones;						// [fieldSize] Array defining the state of each square: FIX, VAR, REL
	unsigned int				fieldSize;					// size of the field
	unsigned int				minDiffInRelZone;			// minimum number of differences between cmpField and curField in relevant zone
	unsigned int				maxDiffInRelZone;			// maximum  ''
	unsigned int				numSequences;				// number of sequences/moves used for backtracking
	unsigned int				maxNumSolutions				= UINT_MAX;
	vectui						indicesMaps;				// [fieldSize*numSequences] array containing the results of the sequences
	vectui						seqBlock;					// [numSequences*numSequences] array defining which sequences are blocked and which allowed. If NULL than it the value is supposed to be equal PGS_SB_RELEASE.
    vectui						idPossibilities;   			// [...] Array containing the possibilityID in getPossibilities()
	vectui						tmpField;					// [fieldSize] temporary array
	vectui						inverseIndicesMap;			// [fieldSize] temporary array, indices map having the reverse effect
	unsigned int				showProgressDepth;			// call the function showProgress() until this search depth

	vector<bool>				srMoveWasClosed;			// srMoveWasClosed[curDepth] is true when this move was a closing one
	vectui						srIsOpenMove;				// [numSequences] For single return: Times a move is "opened".
	unsigned int				srNumMovesOpen;				// For single return: Number of open moves.

	unsigned int				dfNumForwardMoves;			// For double Feature: Amount of resting "Forwardmoves"
	vectui						dfList;						// [fieldSize] For double Feature: A field, where each square contains a list with the inverse moves.
	vectui						dfCurIndex;					// [fieldSize] For double Feature: Current index for each square of the field.
	vector<bool>				dfMoveIsForward;			// dfMoveIsForward[curDepth * numSequences + move] is true when it is a "Forwardmove"

	stateStruct					curState;					// struct describing the state during backtracking
	vector<stateStruct>			oldStates;					// the states of each node while backtracking
	list<sequenceStruct*>		foundSequences;				// list containing all dound sequences
	sequenceStruct				newFoundSeq;				// used during calculation, so that "new" don't have to be used
	pgsSeqBlUserClass			seqBlUser;					// class containing the calling function and a pointer to user data for a manual control of the array seqBlock[] after each call of move()

	// Functions
	void						getCPU						(char* cpuName);
	void						getMemForStates				(vector<stateStruct>& states, unsigned int searchDepth, unsigned int fieldSize, unsigned int numSequences);
	bool						putInListIfItIsNotDuplicate (list<sequenceStruct*> &listSeq, sequenceStruct *newSeq, unsigned int* posInList);
	void						deleteStatesFromMemory      (vector<stateStruct>& states);
	unsigned int				numDifferencesInIndicesMap  (vectui& firstIndicesMap, vectui& secondIndicesMap);
	void						getZoneOfIndicesMap			(vectui& outZone, vectui& theIndicesMap, vectui& theZone);
	bool						doesIndicesMapFitToZone     (vectui& theIndicesMap, vectui& theZone);
	unsigned int				searchValue                 (unsigned int value, unsigned int position, unsigned int arraySize, vectui& theArray);
	unsigned int				power                       (unsigned int base, unsigned int exponent);

	// Virtual Functions
	bool						targetReached				();
	void						setBeginningSituation		();
	unsigned int           *	deletePossibilities         (void *pPossibilities);
	unsigned int		   *	getPossibilities            (unsigned int *numPossibilities,  void **pPossibilities);
	void						undo                        (unsigned int idPossibility, void  *pBackup,  void  *pPossibilities);
	void						move                        (unsigned int idPossibility, void **pBackup,  void  *pPossibilities);

	// Virtual Functions
	virtual	bool				showProgress				(unsigned int solutionsFound, unsigned int currentDepth, unsigned int idPossibility) { return true; };

public:
   // Constructor / destructor
								permutationGameSolver		();
								~permutationGameSolver		();	

	// Functions
	void						setParentClassPointer		(void *pointer) { parentClassPointer = pointer; };
	bool						loadSeqContainer			(vectui& theZones, seqContainer &seqCont, bool loadSequences);
	bool						loadSequences				(const vectui& theZones, list<sequenceStruct*> &sequences);
	bool						saveSequences				(vectui& theZones, list<sequenceStruct*> &sequences, unsigned int numMoves, unsigned int maxSearchDepth, bool useSingleReturn, bool useDoubleFeature, vectui& initialBlockedMoves, vectui& seqBlock, DWORD calculationDurationInMilliseconds, bool overWriteDatabase);
	bool						removeCalculationFromFile	(const vectui& theZones, unsigned int calcIndex);
	void						setFieldSize	            (unsigned int theFieldSize, wstring& databaseDirectory, wstring& databaseFileType);
	void						setInitialState	            (const vectui& theInitialField);
	void						setFinalState	            (const vectui& theFinalField  );
	void						setMaxFoundSolutions		(const unsigned int maxNumSolutions);
	list<sequenceStruct*>		searchSequences	            (													vectui& theZones, vectui* initialBlockedMoves, vectui* curSeqBlock, pgsSeqBlUserClass *seqBlUser, unsigned int callShowProgressUntilDepth, unsigned int minChangesInRelevantZone, unsigned int maxChangesInRelevantZone, unsigned int maxNumMoves, unsigned int numMoves, vectui& indicesMapsOfMoves, vectui* inverseMoves, bool removeSequenceDuplicates, bool useMaxDepth, bool useTheDoubleFeature, bool useTheSingleReturnFeature, bool overWriteDatabase, bool saveSequencesToFile);
	void						finishZone		            (list<solutionPath>	&solPath, vectui& currentField, vectui& theZones, vectui* initialBlockedMoves, vectui* curSeqBlock, pgsSeqBlUserClass *seqBlUser, unsigned int callShowProgressUntilDepth, unsigned int searchDepth, list<sequenceStruct*> &useSequences, bool useMaxDepth, bool returnShortestSolutionPath);
	list<sequenceStatistics>	getSequenceTypes            (list<sequenceStruct*> *theSequences, vectui& theZone);
	unsigned int			*	countIncidences				(list<sequenceStatistics> *theSeqStatisList);
	bool						sequencesSufficientForZone	(list<sequenceStruct*> *theSequences, vectui& theZone);
};

#endif
