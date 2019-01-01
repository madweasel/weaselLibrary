/*********************************************************************
	miniMaxWin.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/
#ifndef MINIMAXWIN_H
#define MINIMAXWIN_H

// Windows Header Files:
#include "wildWeasel\\wildWeasel.h"
#include "wildWeasel\\wwTreeView.h"
#include "wildWeasel\\wwListView.h"
#include "wildWeasel\\wwEditField.h"
#include "miniMax\\miniMax.h"
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

class miniMaxGuiField
{
public:
	virtual void								setAlignment						(wildWeasel::alignment& newAlignment) {};
	virtual void								setVisibility						(bool visible) {};
	virtual void								setState							(unsigned int curShowedLayer, miniMax::stateNumberVarType curShowedState) {};
};

/*------------------------------------------------------------------------------------

|	-------------------------------------		---------------------------------	|
|	|									|		|								|	|
|	|									|		|								|	|
|	|		pTreeViewInspect			|		|		miniMaxGuiField			|	|
|	|									|		|								|	|
|	|									|		|								|	|
|	|									|		|								|	|
|	|									|		|								|	|
|	|									|		|								|	|
|	|									|		|								|	|
|	|									|		|								|	|
|	|									|		|								|	|
|	|									|		|								|	|
|	|									|		|								|	|
|	|									|		|								|	|
|	|									|		|								|	|
|	|									|		|								|	|
|	|									|		|								|	|
|	|									|		|								|	|
|	-------------------------------------		---------------------------------	|
|																					|
-----------------------------------------------------------------------------------*/

class miniMaxWinInspectDb
{
protected:

	struct treeViewItemInfo
	{
		enum class								classType							{ base, layer, uncompleteState, state, symmetricStates, precedingStates, succedingStates };

		static const unsigned int				rowHeightInPixels					= 30;
		static const unsigned int				colWidthInPixels					= 400;
		static const float						textSize							;

		classType								type								= classType::base;
		unsigned int							layerNumber							= 0;
		unsigned int							numberOfKnotsInLayer				= 0;
		miniMax *								pMiniMax							= nullptr;
		miniMaxGuiField*						pGuiField							= nullptr;
		wildWeasel::treeView2D::branch*			pBranch								= nullptr;
		wildWeasel::treeView2D*					pTreeViewInspect					= nullptr;
		treeViewItemInfo*						parent								= nullptr;
		vector<treeViewItemInfo*>				children;
		unsigned int *							pCurShowedLayer						= nullptr;						// current showed layer
		miniMax::stateNumberVarType*			pCurShowedState						= nullptr;						// current showed state
		wildWeasel::buttonImageFiles*			pListImages							= nullptr;	
		wildWeasel::font2D*						pFont								= nullptr;

		virtual bool							deselect							() {return true;};
		virtual bool							select								() {return true;};
		virtual bool							expand								() {return true;};

		bool									collapse							();
		void									writeKnotValueIntoString			(wstringstream &wssTmp, unsigned int stateNumber);		// may use use setSituation()
												
												treeViewItemInfo					() {};
												treeViewItemInfo					(treeViewItemInfo *parent);
												~treeViewItemInfo					();
	};

	struct treeViewItemLayer : treeViewItemInfo
	{
												treeViewItemLayer					(wildWeasel::treeView2D* pTreeViewInspect, wildWeasel::treeView2D::branch* branch, wildWeasel::buttonImageFiles* pListImages, wildWeasel::font2D* pFont, unsigned int layerNumber, miniMax* pMiniMax, miniMaxGuiField* guiField, unsigned int* pCurShowedLayer, miniMax::stateNumberVarType* pCurShowedState);
		bool									expand								();
	};

	struct treeViewItemUncomplState : treeViewItemInfo
	{
		unsigned int							stateNumberPrefix					= 0;							// state number is for example 12000 when missingDigits is equal 3
		unsigned int							numMissingDigits					= 0;							// number of digits missing, since only 10 tree view items are expanded in each level
		unsigned long long						stepSize							= 0;							// 

												treeViewItemUncomplState			(miniMaxWinInspectDb::treeViewItemInfo* parent, unsigned int stateNumberPrefix, unsigned int curStateDigit, unsigned long long newStepSize, unsigned int newNumMissingDigits);
		bool									expand								();
		static bool								expand_static						(treeViewItemInfo& tvii, unsigned int& stateNumberPrefix, unsigned int& numMissingDigits, unsigned long long& stepSize);
	};

	struct treeViewItemState : treeViewItemInfo
	{
		unsigned int							stateNumber							= 0;
		
		bool									expand								();
		bool									select								();
		bool									deselect							();
												treeViewItemState					(treeViewItemInfo *parent, unsigned int layerNumber, unsigned int stateNumber);
												treeViewItemState					(treeViewItemInfo *parent,							 unsigned int stateNumber);
	};

	struct treeViewItemSym : treeViewItemInfo
	{
		unsigned int							stateNumber							= 0;						

		bool									expand								();
												treeViewItemSym						(treeViewItemInfo *parent, unsigned int stateNumber);
	};

	struct treeViewItemSuc : treeViewItemInfo
	{
		unsigned int							stateNumber							= 0;

		bool									expand								();
												treeViewItemSuc						(treeViewItemInfo *parent, unsigned int stateNumber);
	};

	struct treeViewItemPrec : treeViewItemInfo
	{
		unsigned int							stateNumber							= 0;

		bool									expand								();
												treeViewItemPrec					(treeViewItemInfo *parent, unsigned int stateNumber);
	};

	// General Variables
	wildWeasel::masterMind *					ww									= nullptr;					// handle of calling window
	miniMax *									pMiniMax							= nullptr;					// pointer to perfect KI class granting the access to the database
	miniMaxGuiField*							pGuiField							= nullptr;
	bool										showingInspectionControls			= false;
	unsigned int								curShowedLayer						= 0;						// current showed layer
	miniMax::stateNumberVarType					curShowedState						= 0;						// current showed state
	vector<treeViewItemLayer*>					treeViewItems;
	wildWeasel::treeView2D						pTreeViewInspect;
	wildWeasel::alignment*						amAreaInspectDb;
	wildWeasel::alignment						amListInspectDb						= { wildWeasel::alignmentTypeX::FRACTION, 0.05f, wildWeasel::alignmentTypeY::FRACTION, 0.05f, wildWeasel::alignmentTypeX::FRACTION, 0.40f, wildWeasel::alignmentTypeY::FRACTION, 0.90f };
	wildWeasel::alignment						amFieldInspectDb					= { wildWeasel::alignmentTypeX::FRACTION, 0.55f, wildWeasel::alignmentTypeY::FRACTION, 0.05f, wildWeasel::alignmentTypeX::FRACTION, 0.40f, wildWeasel::alignmentTypeY::FRACTION, 0.90f };
	wildWeasel::buttonImageFiles				buttonImagesArrow					= { L"button_Arrow__normal.png",     1, 0, L"button_Arrow__mouseOver.png",    10, 100, L"button_Arrow__mouseLeave.png",    10, 100, L"button_Arrow__pressed.png",    10, 100, L"button_Arrow__grayedOut.png",    1, 0};
	wildWeasel::buttonImageFiles				buttonImagesPlus					= { L"button_Plus___normal.png",     1, 0, L"button_Plus___mouseOver.png",    10, 100, L"button_Plus___mouseLeave.png",    10, 100, L"button_Plus___pressed.png",    10, 100, L"button_Plus___grayedOut.png",    1, 0};
	wildWeasel::buttonImageFiles				buttonImagesMinus					= { L"button_Minus__normal.png",     1, 0, L"button_Minus__mouseOver.png",    10, 100, L"button_Minus__mouseLeave.png",    10, 100, L"button_Minus__pressed.png",    10, 100, L"button_Minus__grayedOut.png",    1, 0};
	wildWeasel::buttonImageFiles				buttonImagesVoid					= { L"button_Void___normal.png",     1, 0, L"button_Void___mouseOver.png",    10, 100, L"button_Void___mouseLeave.png",    10, 100, L"button_Void___pressed.png",    10, 100, L"button_Void___grayedOut.png",    1, 0};
	wildWeasel::buttonImageFiles				buttonImagesListItem				= { L"button_List___normal.png",     1, 0, L"button_List___mouseOver.png",    10, 100, L"button_List___mouseLeave.png",    10, 100, L"button_List___pressed.png",    10, 100, L"button_List___grayedOut.png",    1, 0};
	wildWeasel::texture*						textureLine							= nullptr;
	wildWeasel::font2D*							hFontOutputBox						= nullptr;
	const unsigned int							scrollBarWidth						= 20;

public:
	
	// Constructor / destructor
												miniMaxWinInspectDb					(wildWeasel::masterMind* ww, miniMax* pMiniMax, wildWeasel::alignment& amInspectDb, wildWeasel::font2D* font, wildWeasel::texture* textureLine, miniMaxGuiField& guiField);
												~miniMaxWinInspectDb				();

	// Generals Functions
	bool										createControls						();
	bool										showControls						(bool visible);
	void										resize								(wildWeasel::alignment &rcNewArea);
};

/*------------------------------------------------------------------------------------
|	-----------------------------------------------------------------------------	|
|	|																			|	|
|	|																			|	|
|	|		hListViewLayer														|	|
|	|																			|	|
|	|																			|	|
|	|																			|	|
|	|																			|	|
|	-----------------------------------------------------------------------------	|
|																					|
|	-------------------------------------		---------------------------------	|
|	|									|		|								|	|
|	|									|		|		hEditOutputBox			|	|
|	|		hListViewArray				|		|								|	|
|	|									|		|								|	|
|	|									|		|								|	|
|	|									|		|								|	|
|	|									|		|								|	|
|	-------------------------------------		---------------------------------	|
|																					|
|	hLabelCalculationRunning	hLabelCalculatingLayer	hLabelCalculationAction		|
|																					|
|	-------------------  -----------------  ----------------  ---------------		|
|	hButtonCalcContinue	 hButtonCalcCancel  hButtonCalcPause  hButtonCalcTest		|
|	-------------------  -----------------  ----------------  ---------------		|
-----------------------------------------------------------------------------------*/

class miniMaxWinCalcDb
{
protected:

	// Calculation variables
	wildWeasel::masterMind *					ww									= nullptr;					// pointer to engine
	miniMax *									pMiniMax							= nullptr;					// pointer to perfect KI class granting the access to the database
	ostream *									outputStream						= nullptr;					// pointer to a stream for the console output of the calculation done by the class miniMax
	stringbuf									outputStringBuf;												// buffer linked to the stream, for reading out of the stream into the buffer
	locale										myLocale;														// for formatting the output
	queue<unsigned int>							layersToTest;													// layer numbers to be tested
	thread										hThreadSolve;
	thread										hThreadTestLayer;
	bool										showingCalculationControls			= false;
	bool										threadSolveIsRunning				= false;
	bool										threadTestLayerIsRunning			= false;
	condition_variable							threadConditionVariable;
	mutex										threadMutex;

	// positions, metrics, sizes, dimensions
	unsigned int								listViewRowHeight					= 20;						// height in pixel of a single row
	const float									defPixelDist						= 15;						// 
	const float									labelHeight							= 30;						// 
	const float									buttonHeight						= 30;						// 

	// gui elements
	wildWeasel::plainButton2D					hButtonCalcContinue;											// handles of solve controls
	wildWeasel::plainButton2D					hButtonCalcCancel;	
	wildWeasel::plainButton2D					hButtonCalcPause;	
	wildWeasel::plainButton2D					hButtonCalcTestLayer;
	wildWeasel::plainButton2D					hButtonCalcTestAll;
	wildWeasel::listView2D						hListViewLayer;
	wildWeasel::listView2D						hListViewArray;
	wildWeasel::editField2D						hEditOutputBox;
	wildWeasel::textLabel2D						hLabelCalculationRunning;
	wildWeasel::textLabel2D						hLabelCalculatingLayer;
	wildWeasel::textLabel2D						hLabelCalculationAction;
	wildWeasel::timer							timerUpdateOutput;												// a timer which calls regularly the function updateOutputControls()
	wildWeasel::font2D*							hFontOutputBox						= nullptr;					// used font in the controls
	wildWeasel::alignment*						amAreaCalculation					= nullptr;					// parent alignment area provided by the caller
	wildWeasel::alignment						amListViewLayer						= { wildWeasel::alignmentTypeX::BORDER_LEFT, defPixelDist,	wildWeasel::alignmentTypeY::BORDER_TOP		, 45.0f						, wildWeasel::alignmentTypeX::BORDER_RIGHT, -defPixelDist,	wildWeasel::alignmentTypeY::FRACTION	, 0.40f };					
	wildWeasel::alignment						amListViewArray						= { wildWeasel::alignmentTypeX::BORDER_LEFT, defPixelDist,	wildWeasel::alignmentTypeY::USER			, 0.00f						, wildWeasel::alignmentTypeX::FRACTION    , 0.45f,			wildWeasel::alignmentTypeY::USER		, 0.00f};
	wildWeasel::alignment						amEditOutput						= { wildWeasel::alignmentTypeX::USER       , 0           ,	wildWeasel::alignmentTypeY::USER			, 0.00f						, wildWeasel::alignmentTypeX::BORDER_RIGHT, -defPixelDist,	wildWeasel::alignmentTypeY::USER		, 0.00f};
	wildWeasel::alignment						amCalcStatusLabels					= { wildWeasel::alignmentTypeX::BORDER_LEFT, defPixelDist,	wildWeasel::alignmentTypeY::USER			, 0.00f						, wildWeasel::alignmentTypeX::PIXEL_WIDTH , 200,			wildWeasel::alignmentTypeY::PIXEL_HEIGHT, labelHeight	, wildWeasel::alignmentTypeX::PIXEL_WIDTH,   defPixelDist, wildWeasel::alignmentTypeY::PIXEL_HEIGHT,   defPixelDist, 5}; // position and dimensions of controls
	wildWeasel::alignment						amCalcButtons						= { wildWeasel::alignmentTypeX::BORDER_LEFT, defPixelDist,	wildWeasel::alignmentTypeY::BORDER_BOTTOM,-buttonHeight-defPixelDist	, wildWeasel::alignmentTypeX::PIXEL_WIDTH , 120,			wildWeasel::alignmentTypeY::PIXEL_HEIGHT, buttonHeight	, wildWeasel::alignmentTypeX::PIXEL_WIDTH,   defPixelDist, wildWeasel::alignmentTypeY::PIXEL_HEIGHT,   defPixelDist, 5}; // position and dimensions of controls
	wildWeasel::buttonImageFiles				buttonImagesArrow					= { L"button_Arrow__normal.png",     1, 0, L"button_Arrow__mouseOver.png",    10, 100, L"button_Arrow__mouseLeave.png",    10, 100, L"button_Arrow__pressed.png",    10, 100, L"button_Arrow__grayedOut.png",    1, 0};
	wildWeasel::buttonImageFiles				buttonImagesVoid					= { L"button_Void___normal.png",     1, 0, L"button_Void___mouseOver.png",    10, 100, L"button_Void___mouseLeave.png",    10, 100, L"button_Void___pressed.png",    10, 100, L"button_Void___grayedOut.png",    1, 0};

	// Calculation Functions
	void										buttonFuncCalcStartOrContinue		(void* pUser);
	void										buttonFuncCalcCancel				(void* pUser);
	void										buttonFuncCalcPause					(void* pUser);
	void										buttonFuncCalcTest					();
	void										buttonFuncCalcTestAll				(void* pUser);
	void										buttonFuncCalcTestLayer				(void* pUser);
	void										lvSelectedLayerChanged				(unsigned int row, unsigned int col, wildWeasel::guiElemEvFol* guiElem, void* pUser);
	static void									updateOutputControls				(void* pUser);
	void										updateListItemLayer					(unsigned int layerNumber);
	void										updateListItemArray					(miniMax::arrayInfoChange infoChange);
	void										threadSolve							();
	void										threadProcTestLayer					();

public:

	// Constructor / destructor
												miniMaxWinCalcDb					(wildWeasel::masterMind* ww, miniMax* pMiniMax, wildWeasel::alignment& amCalculation, wildWeasel::font2D* font, wildWeasel::texture* textureLine);
												~miniMaxWinCalcDb					();

	// Generals Functions
	bool										createControls						();
	void										resize								(wildWeasel::alignment &amNewArea);
	bool										showControls						(bool visible);
	bool										isCalculationOngoing				();
	miniMax *									getMinimaxPointer 					() { return pMiniMax;					};
	CRITICAL_SECTION *							getCriticalSectionOutput			() { return &pMiniMax->csOsPrint;		};
};

#endif