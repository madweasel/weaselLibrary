/*********************************************************************
	pgsWin.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/
#ifndef PGSWIN_H
#define PGSWIN_H

// Exclude rarely-used stuff from Windows headers
#include "wildWeasel\\wildWeasel.h"
#include "wildWeasel\\wwListView.h"
#include "permutationGameSolver.h"
#include "pgsThread.h"
#include "zoneClass.h"
#include "strLib.h"

/***********************************************************

RubikCubeWin
    |
pgsWin
	|
	|		/ pgsThreadFindSequences
pgsThread --- pgsThreadConcatenateSequences
	|		\ pgsThreadSolve
	|
permutationGameSolver
	|
backTracking

***********************************************************/

class pgsWin
{
public:
	// parameter passed by the calling function
	struct userParams
	{
		struct userFunctions
		{
			void												(*showZoneControlsSubfunc)				(pgsWin* pgsW, bool visible);
			void												(*setDefaultBlockedMoves)				(zoneClass *zones);
			void												(*setCBoxesBlockedMovesUser)			(pgsWin* pgsW, vectui& blockedMoves, unsigned int checkedValue, bool enableCheckBoxes);
			void												(*updateLabelsAndButtonsOfZoneState)	(pgsWin* pgsW);
			void												(*setRandomState)						();
			void												(*showMoveBegin)						(unsigned int moveId);
			void												(*showMoveEnd)							();
			void												(*zoneDoMoveAndShow)					(unsigned int moveId, bool justPrepare);
			void												(*resetZoneState)						(pgsWin* pgsW);
			void												(*calcMaxNumSequences)					(pgsWin* pgsW, unsigned int &maxNum, unsigned int zoneId);
		} functions;

		struct userTimers
		{
			wildWeasel::timer*								timerPlay;																		// Timer for showing the next step of the solution
		} timers;

		struct userAlignment
		{
			wildWeasel::alignment *							alignmentZoneButtons;
			wildWeasel::alignment *							alignmentShowSeqButtons;
			wildWeasel::alignment *							alignmentZoneEditFields;
			wildWeasel::alignment *							alignmentZoneEditLabels;
			wildWeasel::alignment *							alignmentSolveButtons;
			wildWeasel::alignment *							alignmentSolveLabels;
			wildWeasel::alignment *							alignmentBlockedMoves;
			wildWeasel::alignment *							alignmentBlockedBorder;
			wildWeasel::alignment *							alignmentListViewZone;
			wildWeasel::alignment *							alignmentListViewSeq;
			wildWeasel::alignment *							alignmentListViewCal;
			wildWeasel::alignment *							alignmentListViewActiveCal;
		} alignments;

		struct userTextures
		{
			wildWeasel::texture *							textureCorner;
			wildWeasel::texture *							textureLine;
			wildWeasel::texture *							textureTriangle;
		} textures;

		struct userButtonImages
		{
			wildWeasel::buttonImageFiles *					buttonImagesZoneLoad;			
			wildWeasel::buttonImageFiles *					buttonImagesZoneSave;			
			wildWeasel::buttonImageFiles *					buttonImagesZoneQuit;			
			wildWeasel::buttonImageFiles *					buttonImagesZoneInsert;			
			wildWeasel::buttonImageFiles *					buttonImagesZoneDelete;			
			wildWeasel::buttonImageFiles *					buttonImagesZoneMoveUp;			
			wildWeasel::buttonImageFiles *					buttonImagesZoneMoveDown;		
			wildWeasel::buttonImageFiles *					buttonImagesZoneFindSequences;	
			wildWeasel::buttonImageFiles *					buttonImagesZonePlay;			
			wildWeasel::buttonImageFiles *					buttonImagesZoneForward;			
			wildWeasel::buttonImageFiles *					buttonImagesZoneBackward;		
			wildWeasel::buttonImageFiles *					buttonImagesSolvePause;			
			wildWeasel::buttonImageFiles *					buttonImagesSolveFaster;			
			wildWeasel::buttonImageFiles *					buttonImagesSolveSlower;			
			wildWeasel::buttonImageFiles *					buttonImagesSolveShowStatistics;	
			wildWeasel::buttonImageFiles *					buttonImagesSolveStopCalc;		
			wildWeasel::buttonImageFiles *					buttonImagesZoneCalc;	
			wildWeasel::buttonImageFiles *					buttonImagesListBackground;
			wildWeasel::buttonImageFiles *					buttonImagesVoid;
			wildWeasel::buttonImageFiles *					buttonImagesTriangle;
			wildWeasel::buttonImageFiles *					buttonImagesChecked;
			wildWeasel::buttonImageFiles *					buttonImagesUnchecked;
		} buttonImages;

		float													listViewTextSize;
		unsigned int											listRowHeight;
		wildWeasel *										ww;
		wstring													strDatabaseDir;																	// path to database directory
		wstring													strDatabaseFileType;															// file type ending used for database files, e.g. "dbs"
		bool													cbBlockedMovesByUser;															// check boxes for blocked moves are controlled by user
		wildWeasel::font2D *								hFont2D;
		unsigned int											maxFoundSolutions;
	};

private: 

#pragma region common used stuff
	// General Variables
	userParams													uP										;									// parameter passed by the calling function
	bool														showingZones							= false;							// true if zone controls are shown
	DWORD														solutionAnimationSleepDuration			= 500;								// interval between each showed step in milliseconds
	bool														autoForward								= false;							// the solutions steps are shown automatically 

	class pgsWinThread
	{
	public:
		wildWeasel::threadEvent								eventUpdateOnThreadVars					;									// if the calculating thread found the solution for the next zone, than he sets this event, which calls a function to update the gui
		wildWeasel::threadEvent								eventThreadFinished						;									//
		wildWeasel::timer									timerUpdateResult						;									// the information in the list view about the current calculation shall be updated regularly
		userParams*												uP										= nullptr;
		mutex													sharedVarsMutex							;
		mutex													pauseMutex								;
	};
#pragma endregion 

#pragma region zones and sequences
	// variables for find sequence calculation thread
	class listViewActCalItem : public pgsWinThread
	{
	private:
		wildWeasel::plainButton2D*							pauseButton								= nullptr;
		wildWeasel::plainButton2D*							stopButton								= nullptr;
		wildWeasel::threadEvent									eventRemoveRow							;
		pgsThread*												pThread									= nullptr;

		static void												updateListViewCalculations				(void* pUser);
		void													pauseFindSeqCalculation					(wildWeasel::guiElemEvFol* elem, void* pUser);
	public:
		wildWeasel::listView2D*								pListView								= nullptr;
		unsigned int											rowIndex								= 0;								// row index within the list view to which this calculation belongs to

		void													create									(pgsThread* pgsT, pgsWin* pgsW, wildWeasel *ww, wildWeasel::listView2D* pList, float listViewTextSize, unsigned int listRowHeight, wildWeasel::buttonImageFiles& buttonImages, wildWeasel::font2D* font);
		void													stopFindSeqCalculation					(wildWeasel::guiElemEvFol* elem, void* pUser);
	};
	void														finishedFindingSeq						(void* pUser);						// the calculation has been stopped or has been finished but is still shown in the list view
	void														quitFindingSeq							(void* pUser);						// delete row in list view
	list<listViewActCalItem*>									findSeqThreads;																// list containing all running calculations, which are presented in list view gui element 'hListViewActCal'

	class pgsWinThreadFindSeq : public pgsThreadFindSequences, public listViewActCalItem
	{
	public:
																pgsWinThreadFindSeq						(pgsWin* pgsW, wildWeasel *ww, zoneClass* curZones, unsigned int selZone, wstring& databaseDir, wstring& databaseFileType, vector<unsigned int>& moveIndicesMap, vector<unsigned int>& reverseMoves, wildWeasel::listView2D* pList, float listViewTextSize, unsigned int listRowHeight, wildWeasel::buttonImageFiles& buttonImages, wildWeasel::font2D* font, unsigned int maxFoundSolutions);
																~pgsWinThreadFindSeq					();
	};

	// variables for sequence concatenation thread 
	class pgsWinThreadConcatenateSequences : public pgsThreadConcatenateSequences, public listViewActCalItem
	{
	public:
		wildWeasel::listView2D*								hListViewZone							= nullptr;			
		wildWeasel::timer									timerUpdateResultConc					;									// the information in the list view about the current calculation shall be updated regularly
		unsigned int											columnIndex								= -1;
		
		static void												updateResult							(void* pUser);
		void													threadHasFinished						();
	} concThread;

	// Zones Variables
	vector<wildWeasel::checkBox2D>							hCheckBoxBlockedMoves;														// gui element handles
	wildWeasel::plainButton2D								hButtonZoneLoad;						
	wildWeasel::plainButton2D								hButtonZoneSave;
	wildWeasel::plainButton2D								hButtonZoneQuit;
	wildWeasel::plainButton2D								hButtonZoneInsert;
	wildWeasel::plainButton2D								hButtonZoneDelete;
	wildWeasel::plainButton2D								hButtonZoneMoveUp;
	wildWeasel::plainButton2D								hButtonZoneMoveDown;
	wildWeasel::plainButton2D								hButtonZoneTest;
	wildWeasel::plainButton2D								hButtonZoneFindSequences;
	wildWeasel::plainButton2D*								hButtonZoneSeqConcatenate;
	wildWeasel::plainButton2D								hButtonFindSeqQuit;
	wildWeasel::plainButton2D								hButtonZonePlay;
	wildWeasel::plainButton2D								hButtonZoneForward;
	wildWeasel::plainButton2D								hButtonZoneBackward;
	wildWeasel::textLabel2D									hTextZoneCurStep;
	wildWeasel::textLabel2D									hTextBlockedMoves;
	wildWeasel::textLabel2D									hTextOverwriteDatabase;
	wildWeasel::textLabel2D									hTextBackTrackingDepth;
	wildWeasel::textLabel2D									hTextDoubleFeature;
	wildWeasel::textLabel2D									hTextUseSingleReturn;
	wildWeasel::textLabel2D									hTextMaxSeqLength;
	wildWeasel::textLabel2D									hTextSearchDepthUsingSeq;
	wildWeasel::checkBox2D									hEditOverwriteDatabase;		
	wildWeasel::checkBox2D									hEditDoubleFeature;			
	wildWeasel::checkBox2D									hEditUseSingleReturn;		
	wildWeasel::dropDown2D									hEditMaxSeqLength;			
	wildWeasel::dropDown2D									hEditBackTrackingDepth;		
	wildWeasel::dropDown2D									hEditSearchDepthUsingSeq;	
	wildWeasel::listView2D									hListViewZone;
	wildWeasel::listView2D									hListViewCal;
	wildWeasel::listView2D									hListViewSeq;
	wildWeasel::listView2D									hListViewActCal;
	wildWeasel::borderLine2D								hBorderLineZone;
	wildWeasel::borderLine2D								hBorderLineCal;
	wildWeasel::borderLine2D								hBorderLineSeq;
	wildWeasel::borderLine2D								hBorderLineActCal;
	wildWeasel::borderLine2D								hBorderLineBlocked;
	wildWeasel::alignment									alignmentBorderZone						= {wildWeasel::alignmentTypeX::BORDER_LEFT, +10.0f, wildWeasel::alignmentTypeY::BORDER_TOP, -100.0f, wildWeasel::alignmentTypeX::BORDER_RIGHT, -20.0f, wildWeasel::alignmentTypeY::BORDER_BOTTOM,  -10.0f};
	wildWeasel::alignment									alignmentBorderCal						= {wildWeasel::alignmentTypeX::BORDER_LEFT, +10.0f, wildWeasel::alignmentTypeY::BORDER_TOP, +100.0f, wildWeasel::alignmentTypeX::BORDER_RIGHT, -20.0f, wildWeasel::alignmentTypeY::BORDER_BOTTOM, +110.0f};		
	wildWeasel::alignment									alignmentBorderSeq						= {wildWeasel::alignmentTypeX::BORDER_LEFT, +10.0f, wildWeasel::alignmentTypeY::BORDER_TOP,  +20.0f, wildWeasel::alignmentTypeX::BORDER_RIGHT, -20.0f, wildWeasel::alignmentTypeY::BORDER_BOTTOM,  +30.0f};		
	wildWeasel::alignment									alignmentBorderActiveCal				= {wildWeasel::alignmentTypeX::BORDER_LEFT, +10.0f, wildWeasel::alignmentTypeY::BORDER_TOP, -100.0f, wildWeasel::alignmentTypeX::BORDER_RIGHT, -20.0f, wildWeasel::alignmentTypeY::BORDER_BOTTOM, +110.0f};	
	wildWeasel::alignment									alignmentBorderBlockedBorder			= {wildWeasel::alignmentTypeX::BORDER_LEFT, +10.0f, wildWeasel::alignmentTypeY::BORDER_TOP,  -10.0f, wildWeasel::alignmentTypeX::BORDER_RIGHT, -20.0f, wildWeasel::alignmentTypeY::BORDER_BOTTOM,  +10.0f};		
	wildWeasel::threadEvent									eventUpdateAll							;
	unsigned int												curShowedStep							= 0;
	unsigned int												selZone									= 0;								// current selected zone
	unsigned int												selCalculation							= 0;								// current selected calculation
	unsigned int												selSequence								= 0;								// current selected sequence
	unsigned int												numUniqueMoves							= 0;								// number of unique moves, calculated by curZones->vanishDuplicates()
	vectui														reverseUniqueMoves						;									// [numUniqueMoves] reverse moves of the unique moves
	vectui														duplicatesMapBlockedMoves				;									// [numUniqueMoves] map duplicated moves calculated by curZones->vanishDuplicates()
	vectui														invDuplicatesMapBlockedMoves			;									// [curZones->numZones] inverse map of  duplicatesMapBlockedMoves[]
	vectui														curExcludedMoves						;									// [curZones->numMoves] for the current selected calculation/zone these moves are blocked/excluded
	vectui														theZonesSeqContainer					;									// [curZones->numZones*curZones->numFields] pgsZone of each seqContainer
	vectui														moveIndicesMap							;									//
	vectui														reverseMoves							;									//
	wstring														extFilter								;									// file extension filter for open zone file dialog
	wstring														defExtension							;									// default file extension for open zone file dialog
	bool														showingSeqMove							= false;							// showing a move of a calculated sequence
	vector<seqContainer>										seqContainers							;									// sequences
	permutationGameSolver *										solver									= nullptr;							// used for loading sequences
	
	// Zone Functions
	void														buttonFuncFindSequences					();
	void														buttonFuncZonePlay						();
	void														buttonFuncZoneBackward					();
	void														buttonFuncTestZone						();
	void														buttonFuncDeleteZone					();
	void														buttonFuncInsertZone					();
	void														buttonFuncLoadZoneFile					();
	void														buttonFuncMoveZoneUp					();
	void														buttonFuncMoveZoneDown					();	
	void														buttonFuncSaveZoneFile					();
	void														setSelItemInListViewZone				(unsigned int item);
	void														insertRowInListViewZone					(unsigned int afterRowIndex, unsigned int numRows);
	void														updateListViewZoneItems					();
	void														loadSequenceContainerFiles				(bool redimSeqContainersArray);
	void														unloadSequenceContainerFiles			();
	void														loadSequenceContainerFile				(unsigned int zoneNumber);
	void														unloadSequenceContainerFile				(unsigned int zoneNumber);
	void														buttonFuncSeqConcatenate				(wildWeasel::guiElemEvFol* elem,    void* pUser);
	void														buttonDeleteCalculation					(wildWeasel::guiElemEvFol* elem,    void* pUser);
	void														updateAllLists							(void* pUser);
	void														buttonBlockedMovesChanged				(unsigned int uniqueMoveId);
	void														calcCurBlockedMovesFromBlockSeq			();
	void														setCBoxesShowingBlockedMoves			(vectui& blockedMoves, unsigned int checkedValue, bool enableCheckBoxes);
	void														processListViewItemChanged_preFunc		();
	void														processListViewItemChanged_Zone			(unsigned int rowIndex, unsigned int columnIndex, wildWeasel::guiElemEvFol* guiElem, void* pUser);
	void														processListViewItemChanged_Seq			(unsigned int rowIndex, unsigned int columnIndex, wildWeasel::guiElemEvFol* guiElem, void* pUser);
	void														processListViewItemChanged_Cal			(unsigned int rowIndex, unsigned int columnIndex, wildWeasel::guiElemEvFol* guiElem, void* pUser);
	void														processListViewItemChanged_ActCal		(unsigned int rowIndex, unsigned int columnIndex, wildWeasel::guiElemEvFol* guiElem, void* pUser);
	bool														calcCurSeq								(unsigned int &curSeq);
#pragma endregion 	

#pragma region solve mode

	// variables for solve thread 
	class pgsWinThreadSolve : public pgsThreadSolve, public pgsWinThread
	{
	public:
		unsigned int											curShowedZone							= 0;								// current showed step belongs to zone
		unsigned int											curStepWithInZone						= 0;								// current step within zone
		unsigned int											curShowedStep							= 0;								// current showed step of the solution
		unsigned int											numStepOfCurZone						= 0;								// current showed zone consists of this amount of steps
		unsigned int  											numRandomStateAlreadySolved				= 0;								// number of random states already solved
		unsigned int 											numRandomStatesTried					= 0;								// number of random states tried to solve
		bool													solvingRandomStates						= false;							// true if solving random states
		wildWeasel *										ww										= nullptr;							// contains all the winapi GUII stuff
		vector<unsigned int>									timesUnsolved;																// [curZones->numZones] number of times a zone could not be solved while solving random states
		wildWeasel::textLabel2D								hTextStepWithinZone;														// handles of solve controls
		wildWeasel::textLabel2D								hTextStepInTotal;
		wildWeasel::textLabel2D								hTextShowingZone;
		wildWeasel::textLabel2D								hTextAnimationSpeed;
		wildWeasel::textLabel2D								hTextZonesSolved;
		wildWeasel::textLabel2D								hTextTopBranch;
		wildWeasel::textLabel2D								hTextRndStatesSolved;		
		wildWeasel::plainButton2D							hButtonSolveStopCalc;									
		wildWeasel::plainButton2D							hButtonSolvePlay;
		wildWeasel::plainButton2D							hButtonSolvePause;
		wildWeasel::plainButton2D							hButtonSolveForward;
		wildWeasel::plainButton2D							hButtonSolveBackward;
		wildWeasel::plainButton2D							hButtonSolveShowStatistics;
		wildWeasel::plainButton2D							hButtonSolveFaster;
		wildWeasel::plainButton2D							hButtonSolveSlower;
		wildWeasel::threadEvent									hEventSolveNextRandomState				;									// start solving algorithmn for next random state

		void													(*doMove)								(unsigned int moveId);				// user defined function called during solving, so that move is shown in gui
		void													(*setInitialState)						();
		bool													(*doesAllFieldsBelongToZone)			();
		void													(*setRandomState)						();
		void													(*doMoveAndShow)						(unsigned int moveId);

		void													updateResult							(void* pUser);
		void													threadHasFinished						();
	} solveThread;	

	// Solve Functions
	void														buttonFuncSolvePlay						();
	void														buttonFuncSolvePause					();
	void														buttonFuncSolveBackward					();
	void														buttonFuncSolveStopCalc					();
	void														buttonFuncSolveShowStats				();
	void														buttonFuncSolveFaster					();
	void														buttonFuncSolveSlower					();	
	void														solveNextRandomState					(void* pUser);
#pragma endregion 

public:

	// Variables
	zoneClass			*										curZones;																	// ... should be private 

	// Constructor / destructor
																pgsWin									(userParams &paramStruct);
																~pgsWin									();
						
	// Generals Functions
	bool														createControls							();
	void														terminateAllCalculationsImmediatelly	();

	// Zones Functions
	void														setFieldSize							(unsigned int theFieldSize, wstring& databaseDirectory, wstring& databaseFileType);
	void														loadDefaultZoneFile						(wstring &defZoneFileName, wstring &extensionFilter, wstring &defaultExtension, unsigned int numFields, unsigned int numMoves, vector<unsigned int>& moveIndicesMap, vector<unsigned int>& reverseMoves);
	void														setControlOfBlockedMoves				(bool controlBlockedMovesByUser)	{ uP.cbBlockedMovesByUser = controlBlockedMovesByUser; };
	void														showZoneControls						(bool visible);
	void														updateSingleZone						(unsigned int zone);
	void														deleteAllZones							();
	void														buttonFuncZoneForward					();

	// Solve Functions
	bool														showSolveControls						(bool visible);
	bool														solveState								(/*const*/ vector<unsigned int>* stateToSolve, const vector<unsigned int>& initialState, const vector<unsigned int>& finalState, vector<unsigned int>& reverseMoves, void doMoveAndShow(unsigned int moveId), void doMove(unsigned int moveId), void setInitialState(), bool doesAllFieldsBelongToZone(), void setRandomState ());
	void														buttonFuncSolveForward					();
	void														setSolvingRandomStates					(bool active);
	
	// Getter
	unsigned int 												getSelZone								() { return selZone;					};
	unsigned int 												getSelCalculation						() { return selCalculation;				};
	unsigned int 												getSelSequence							() { return selSequence;				};
	bool														isShowingZones							() { return showingZones;				};
	bool														isShowingSeqMove						() { return showingSeqMove;				};
};

#endif