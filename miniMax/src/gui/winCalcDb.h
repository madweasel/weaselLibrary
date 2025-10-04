/*********************************************************************
	winCalcDb.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/
#ifndef MINIMAXWIN_CALCDB_H
#define MINIMAXWIN_CALCDB_H

// Windows Header Files:
#include "wildWeasel/src/wildWeasel.h"
#include "wildWeasel/src/wwTreeView.h"
#include "wildWeasel/src/wwListView.h"
#include "wildWeasel/src/wwEditField.h"
#include "../miniMax.h"
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace miniMax
{

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
	miniMax *									pMiniMax							= nullptr;					// pointer to perfect AI class granting the access to the database
	wostream *									outputStream						= nullptr;					// pointer to a stream for the console output of the calculation done by the class miniMax
	wstringbuf									outputStringBuf;												// buffer linked to the stream, for reading out of the stream into the buffer
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
	void										updateListItemArray					(database::arrayInfoChange infoChange);
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
};

} // namespace miniMax

#endif