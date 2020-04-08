/*********************************************************************
	miniMaxWin.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/

#include "miniMaxWin.h"

#pragma region calculation mode

//-----------------------------------------------------------------------------
// Name: miniMaxWin constructor()
// Desc: 
//-----------------------------------------------------------------------------
miniMaxWinCalcDb::miniMaxWinCalcDb(	wildWeasel::masterMind *			ww,
									miniMax *							pMiniMax,
									wildWeasel::alignment&				amAreaCalculation,
									wildWeasel::font2D*					font,
									wildWeasel::texture*			    textureLine) : 
	ww					{ ww }, 
	pMiniMax			{ pMiniMax },
	amAreaCalculation	{ &amAreaCalculation },
	hFontOutputBox		{ font }
{
	// Tausender-Trennzeichen
	myLocale = locale("German_Switzerland"); 
	outputStringBuf.str("");
	outputStream	= new ostream(&outputStringBuf);
	outputStream->imbue(myLocale); 

	// calc position of gui elements
	resize(amAreaCalculation);
}

//-----------------------------------------------------------------------------
// Name: miniMaxWin destructor()
// Desc: 
//-----------------------------------------------------------------------------
miniMaxWinCalcDb::~miniMaxWinCalcDb()
{
	// request calculation threads to stop
	buttonFuncCalcCancel(nullptr);

	// wait for threads to finish since they still access the pMiniMax
	if (isCalculationOngoing()) {
		std::unique_lock<std::mutex> lk(threadMutex);
		threadConditionVariable.wait(lk);
	}

	SAFE_DELETE(outputStream);
}

//-----------------------------------------------------------------------------
// Name: createControls()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMaxWinCalcDb::createControls()
{
	// parameters ok?
	if (ww == nullptr) return false;
	if (hListViewLayer.isInitialized()) return false;
		
	/*** create controls for database calculation ******************************************************************************************************************/
	hButtonCalcContinue .create(ww, buttonImagesVoid, bind(&miniMaxWinCalcDb::buttonFuncCalcStartOrContinue	, this, placeholders::_1), this, 0); hButtonCalcContinue	.setText(L"Start\ncalculation"	);	hButtonCalcContinue	.setState(wildWeasel::guiElemState::HIDDEN);		hButtonCalcContinue		.setTextState(wildWeasel::guiElemState::DRAWED);
	hButtonCalcCancel	.create(ww, buttonImagesVoid, bind(&miniMaxWinCalcDb::buttonFuncCalcCancel			, this, placeholders::_1), this, 0); hButtonCalcCancel		.setText(L"Cancel\ncalculation"	);	hButtonCalcCancel	.setState(wildWeasel::guiElemState::HIDDEN);		hButtonCalcCancel		.setTextState(wildWeasel::guiElemState::DRAWED);
	hButtonCalcPause	.create(ww, buttonImagesVoid, bind(&miniMaxWinCalcDb::buttonFuncCalcPause			, this, placeholders::_1), this, 0); hButtonCalcPause		.setText(L"Pause\nCalculation"	);	hButtonCalcPause	.setState(wildWeasel::guiElemState::HIDDEN);		hButtonCalcPause		.setTextState(wildWeasel::guiElemState::DRAWED);
	hButtonCalcTestLayer.create(ww, buttonImagesVoid, bind(&miniMaxWinCalcDb::buttonFuncCalcTestLayer		, this, placeholders::_1), this, 0); hButtonCalcTestLayer	.setText(L"Test\nselected layer");	hButtonCalcTestLayer.setState(wildWeasel::guiElemState::HIDDEN);		hButtonCalcTestLayer	.setTextState(wildWeasel::guiElemState::DRAWED);
	hButtonCalcTestAll	.create(ww, buttonImagesVoid, bind(&miniMaxWinCalcDb::buttonFuncCalcTestAll			, this, placeholders::_1), this, 0); hButtonCalcTestAll		.setText(L"Test\nall layers"	);	hButtonCalcTestAll	.setState(wildWeasel::guiElemState::HIDDEN);		hButtonCalcTestAll		.setTextState(wildWeasel::guiElemState::DRAWED);

	hButtonCalcContinue .setFont(hFontOutputBox);	hButtonCalcContinue .setTextColor(wildWeasel::color::black);	hButtonCalcContinue	.setTextAlignmentHorizontal(wildWeasel::alignmentHorizontal::CENTER);	hButtonCalcContinue	.setTextAlignmentVertical(wildWeasel::alignmentVertical::CENTER);	hButtonCalcContinue	.setTextSize(0.5f, 0.5f);
	hButtonCalcCancel	.setFont(hFontOutputBox);	hButtonCalcCancel	.setTextColor(wildWeasel::color::black);	hButtonCalcCancel	.setTextAlignmentHorizontal(wildWeasel::alignmentHorizontal::CENTER);	hButtonCalcCancel	.setTextAlignmentVertical(wildWeasel::alignmentVertical::CENTER);	hButtonCalcCancel	.setTextSize(0.5f, 0.5f);
	hButtonCalcPause	.setFont(hFontOutputBox);	hButtonCalcPause	.setTextColor(wildWeasel::color::black);	hButtonCalcPause	.setTextAlignmentHorizontal(wildWeasel::alignmentHorizontal::CENTER);	hButtonCalcPause	.setTextAlignmentVertical(wildWeasel::alignmentVertical::CENTER);	hButtonCalcPause	.setTextSize(0.5f, 0.5f);
	hButtonCalcTestLayer.setFont(hFontOutputBox);	hButtonCalcTestLayer.setTextColor(wildWeasel::color::black);	hButtonCalcTestLayer.setTextAlignmentHorizontal(wildWeasel::alignmentHorizontal::CENTER);	hButtonCalcTestLayer.setTextAlignmentVertical(wildWeasel::alignmentVertical::CENTER);	hButtonCalcTestLayer.setTextSize(0.5f, 0.5f);
	hButtonCalcTestAll	.setFont(hFontOutputBox);	hButtonCalcTestAll	.setTextColor(wildWeasel::color::black);	hButtonCalcTestAll	.setTextAlignmentHorizontal(wildWeasel::alignmentHorizontal::CENTER);	hButtonCalcTestAll	.setTextAlignmentVertical(wildWeasel::alignmentVertical::CENTER);	hButtonCalcTestAll	.setTextSize(0.5f, 0.5f);

	amCalcButtons		.create(ww->alignmentRootFrame);
	amCalcStatusLabels	.create(ww->alignmentRootFrame);
	amListViewLayer		.create(ww->alignmentRootFrame);
	amListViewArray		.create(ww->alignmentRootFrame);
	amEditOutput		.create(ww->alignmentRootFrame);

	// output box
	hEditOutputBox.create						(ww, hFontOutputBox, 0);
	hEditOutputBox.setAlignment					(amEditOutput);
	hEditOutputBox.setText						(L"");
	hEditOutputBox.setPosition					(0, 0, false);
	hEditOutputBox.setScale						(1, 1, true );
	hEditOutputBox.createScrollBars				(buttonImagesArrow, buttonImagesVoid, buttonImagesVoid);
	hEditOutputBox.setVisibilityColumnScrollBar	(false);
	hEditOutputBox.setVisibilityRowScrollBar	(true);
	hEditOutputBox.setColumnScrollBarHeight		(30);
	hEditOutputBox.setRowScrollBarWidth			(20);
	hEditOutputBox.setBorderWidth				(5);
	hEditOutputBox.setTextSize					(0.5f, 0.5f);
	hEditOutputBox.alignAllItems				();
	hEditOutputBox.setState						(wildWeasel::guiElemState::HIDDEN);
	
	// set timer for regular output update
	timerUpdateOutput.setFunc(ww, updateOutputControls, this);

	// labels showing current calculation status 
	hLabelCalculationRunning	.create(ww, wstring(L""), hFontOutputBox, 0);		hLabelCalculationRunning	.setState(wildWeasel::guiElemState::HIDDEN);		hLabelCalculationRunning	.setColor(wildWeasel::color::white);			hLabelCalculationRunning	.setFont(hFontOutputBox);
	hLabelCalculatingLayer		.create(ww, wstring(L""), hFontOutputBox, 0);		hLabelCalculatingLayer		.setState(wildWeasel::guiElemState::HIDDEN);		hLabelCalculatingLayer		.setColor(wildWeasel::color::white);			hLabelCalculatingLayer		.setFont(hFontOutputBox);
	hLabelCalculationAction		.create(ww, wstring(L""), hFontOutputBox, 0);		hLabelCalculationAction		.setState(wildWeasel::guiElemState::HIDDEN);		hLabelCalculationAction		.setColor(wildWeasel::color::white);			hLabelCalculationAction		.setFont(hFontOutputBox);

	hLabelCalculationRunning	.setTextColor(wildWeasel::color::white);		hLabelCalculationRunning	.setTextState(wildWeasel::guiElemState::DRAWED);	hLabelCalculationRunning	.setTextAlignmentHorizontal(wildWeasel::alignmentHorizontal::LEFT);
	hLabelCalculatingLayer		.setTextColor(wildWeasel::color::white);		hLabelCalculatingLayer		.setTextState(wildWeasel::guiElemState::DRAWED);	hLabelCalculatingLayer		.setTextAlignmentHorizontal(wildWeasel::alignmentHorizontal::LEFT);
	hLabelCalculationAction		.setTextColor(wildWeasel::color::white);		hLabelCalculationAction		.setTextState(wildWeasel::guiElemState::DRAWED);	hLabelCalculationAction		.setTextAlignmentHorizontal(wildWeasel::alignmentHorizontal::LEFT);

	hLabelCalculationRunning	.setTextSize(0.6f, 0.6f);
	hLabelCalculatingLayer		.setTextSize(0.6f, 0.6f);
	hLabelCalculationAction		.setTextSize(0.6f, 0.6f);

	// create list view
	hListViewLayer.create(ww, 0.0f);
	hListViewLayer.setSelectionMode(wildWeasel::listView2D::selectionMode::ROW_WISE);
	hListViewLayer.setMarkerColor(wildWeasel::color::lightBlue);
	hListViewLayer.setColumnHeaderHeight(20);
	hListViewLayer.setVisibilityColumnHeader(true);
	hListViewLayer.createScrollBars(buttonImagesArrow, buttonImagesVoid, buttonImagesVoid);
	hListViewLayer.setColumnScrollBarHeight(20);
	hListViewLayer.setRowScrollBarWidth(20);
	hListViewLayer.setVisibilityColumnScrollBar(false);
	hListViewLayer.setVisibilityRowScrollBar(true);
	hListViewLayer.setPosition	(0, 0, true);
	hListViewLayer.setAlignment(amListViewLayer);
	hListViewLayer.setTextSize(0.6f, 0.6f);
	hListViewLayer.alignAllItems();
	hListViewLayer.setState(wildWeasel::guiElemState::HIDDEN);
	hListViewLayer.assignOnItemChanged(bind(&miniMaxWinCalcDb::lvSelectedLayerChanged, this, placeholders::_1, placeholders::_2, placeholders::_3, placeholders::_4), this);

	hListViewArray.create(ww, 0.0f);
	hListViewArray.setSelectionMode(wildWeasel::listView2D::selectionMode::ROW_WISE);
	hListViewArray.setMarkerColor(wildWeasel::color::lightBlue);
	hListViewArray.setColumnHeaderHeight(20);
	hListViewArray.setVisibilityColumnHeader(true);
	hListViewArray.createScrollBars(buttonImagesArrow, buttonImagesVoid, buttonImagesVoid);
	hListViewArray.setColumnScrollBarHeight(20);
	hListViewArray.setRowScrollBarWidth(20);
	hListViewArray.setVisibilityColumnScrollBar(false);
	hListViewArray.setVisibilityRowScrollBar(true);
	hListViewArray.setPosition	(0, 0, true);
	hListViewArray.setAlignment(amListViewArray);
	hListViewArray.setTextSize(0.6f, 0.6f);
	hListViewArray.alignAllItems();
	hListViewArray.setState(wildWeasel::guiElemState::HIDDEN);

    // Add columns to the list-view
	RECT rcCol;
	rcCol = {0, 0,  90, 20};	hListViewLayer	.insertColumn_plainButton2D(0, wstring(L"layer"				), hFontOutputBox,  30, -0.5, 0.5f, rcCol, buttonImagesVoid, 0);
	rcCol = {0, 0,  90, 20};	hListViewLayer	.insertColumn_plainButton2D(1, wstring(L"calculated"		), hFontOutputBox,  30, -0.5, 0.5f, rcCol, buttonImagesVoid, 0);
	rcCol = {0, 0,  90, 20};	hListViewLayer	.insertColumn_plainButton2D(2, wstring(L"won states"		), hFontOutputBox,  90, -0.5, 0.5f, rcCol, buttonImagesVoid, 0);
	rcCol = {0, 0,  90, 20};	hListViewLayer	.insertColumn_plainButton2D(3, wstring(L"lost states"		), hFontOutputBox,  90, -0.5, 0.5f, rcCol, buttonImagesVoid, 0);
	rcCol = {0, 0,  90, 20};	hListViewLayer	.insertColumn_plainButton2D(4, wstring(L"drawn states"		), hFontOutputBox,  90, -0.5, 0.5f, rcCol, buttonImagesVoid, 0);
	rcCol = {0, 0,  90, 20};	hListViewLayer	.insertColumn_plainButton2D(5, wstring(L"invalid states"	), hFontOutputBox,  90, -0.5, 0.5f, rcCol, buttonImagesVoid, 0);
	rcCol = {0, 0,  90, 20};	hListViewLayer	.insertColumn_plainButton2D(6, wstring(L"total num. states"	), hFontOutputBox,  90, -0.5, 0.5f, rcCol, buttonImagesVoid, 0);
	rcCol = {0, 0,  90, 20};	hListViewLayer	.insertColumn_plainButton2D(7, wstring(L"size in bytes"		), hFontOutputBox,  90, -0.5, 0.5f, rcCol, buttonImagesVoid, 0);
	rcCol = {0, 0,  90, 20};	hListViewLayer	.insertColumn_plainButton2D(8, wstring(L"partner layer"		), hFontOutputBox,  30, -0.5, 0.5f, rcCol, buttonImagesVoid, 0);
	rcCol = {0, 0,  90, 20};	hListViewLayer	.insertColumn_plainButton2D(9, wstring(L"details"			), hFontOutputBox,	90, -0.5, 0.5f, rcCol, buttonImagesVoid, 1);
	
	rcCol = {0, 0,  40, 20};	hListViewArray	.insertColumn_plainButton2D(0, wstring(L"layer"				), hFontOutputBox,  40,    0, 0.5f, rcCol, buttonImagesVoid, 0);
	rcCol = {0, 0, 120, 20};	hListViewArray	.insertColumn_plainButton2D(1, wstring(L"type"				), hFontOutputBox, 120,    0, 0.5f, rcCol, buttonImagesVoid, 0);
	rcCol = {0, 0, 120, 20};	hListViewArray	.insertColumn_plainButton2D(2, wstring(L"size in bytes"		), hFontOutputBox, 120,    0, 0.5f, rcCol, buttonImagesVoid, 0);
	rcCol = {0, 0, 120, 20};	hListViewArray	.insertColumn_plainButton2D(3, wstring(L"compression ratio"	), hFontOutputBox, 120,    0, 0.5f, rcCol, buttonImagesVoid, 1);
	
	// alignment
	hButtonCalcContinue			.setAlignment(amCalcButtons, 0);	
	hButtonCalcCancel			.setAlignment(amCalcButtons, 1);
	hButtonCalcPause			.setAlignment(amCalcButtons, 2);	
	hButtonCalcTestLayer		.setAlignment(amCalcButtons, 3);	
	hButtonCalcTestAll			.setAlignment(amCalcButtons, 4);	
	hLabelCalculationRunning	.setAlignment(amCalcStatusLabels, 0);
	hLabelCalculatingLayer		.setAlignment(amCalcStatusLabels, 1);
	hLabelCalculationAction		.setAlignment(amCalcStatusLabels, 2);

	return true;
}

//-----------------------------------------------------------------------------
// Name: showCalculationControls()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMaxWinCalcDb::showControls(bool visible)
{
	// locals
	auto	stateButton = visible ? wildWeasel::guiElemState::DRAWED  : wildWeasel::guiElemState::HIDDEN;
	auto	stateInfo   = visible ? wildWeasel::guiElemState::VISIBLE : wildWeasel::guiElemState::HIDDEN;

	// Abfrage ob berechnung abgebrochen werden  soll
	if (!visible && threadSolveIsRunning) {
		if (ww->showMessageBox(L"CANCEL", L"Do you really want to cancel the database calculation?", MB_YESNO) == IDNO) return false;
	}
	if (!visible && threadTestLayerIsRunning) {
		if (ww->showMessageBox(L"CANCEL", L"Do you really want to cancel the layer testing?", MB_YESNO) == IDNO) return false;
	}

	// initialize lists
	if (visible) {
		unsigned int	numLayers		= pMiniMax->getNumberOfLayers();
	
		// Add rows to layer list
		hListViewLayer.insertRowsAndItems_plainButton2D(0, numLayers, listViewRowHeight, buttonImagesVoid, hFontOutputBox);
	
		// update listview items
		for (unsigned int curLayer=0; curLayer<numLayers; curLayer++) {
			updateListItemLayer(curLayer);
		}
	} else {
		// delete items in listview
		hListViewLayer.removeAllItems(true);
		hListViewArray.removeAllItems(true);
		hListViewLayer.removeAllRows(true);
		hListViewArray.removeAllRows(true);
	}

	hListViewLayer				.setVisibilityRowScrollBar(true);
	hListViewArray				.setVisibilityRowScrollBar(true);

	hListViewLayer				.setState(stateButton);
	hListViewArray				.setState(stateButton);

	hListViewLayer				.alignAllItems();
	hListViewArray				.alignAllItems();

	hButtonCalcContinue			.setState(stateButton);
	hButtonCalcCancel			.setState(stateButton == wildWeasel::guiElemState::DRAWED ? wildWeasel::guiElemState::GRAYED : stateButton);		
	hButtonCalcPause			.setState(stateButton == wildWeasel::guiElemState::DRAWED ? wildWeasel::guiElemState::GRAYED : stateButton);
	hButtonCalcTestLayer		.setState(stateButton == wildWeasel::guiElemState::DRAWED ? wildWeasel::guiElemState::GRAYED : stateButton);
	hButtonCalcTestAll			.setState(stateButton);

	hLabelCalculationRunning	.setState(stateInfo  );
	hLabelCalculatingLayer		.setState(stateInfo  );
	hLabelCalculationAction		.setState(stateInfo  );

	hEditOutputBox				.setState(stateInfo  );

	showingCalculationControls	= visible;

	if (visible && amAreaCalculation) {
		resize(*amAreaCalculation);
	}

	if (visible) {
		timerUpdateOutput.start(250);
	} else {
		timerUpdateOutput.terminate();
	}

	return true;
}

//-----------------------------------------------------------------------------
// Name: resizeCalculationArea()
// Desc: 
//-----------------------------------------------------------------------------
void miniMaxWinCalcDb::resize(wildWeasel::alignment& amNewArea)
{
	amAreaCalculation	= &amNewArea;

	amListViewLayer		.setInsideAnotherRect(amNewArea);
	amListViewArray		.setInsideAnotherRect(amNewArea);
	amEditOutput		.setInsideAnotherRect(amNewArea);
	amCalcButtons		.setInsideAnotherRect(amNewArea);
	amCalcStatusLabels	.setInsideAnotherRect(amNewArea);

	// align list view array and edit output box at the lower edge of the list view layer
	amListViewArray		.top	.setRelation(wildWeasel::alignment::relDistance{amListViewLayer		.bottom,  defPixelDist});
	amEditOutput		.top	.setRelation(wildWeasel::alignment::relDistance{amListViewLayer		.bottom,  defPixelDist});
	amEditOutput		.bottom .setRelation(wildWeasel::alignment::relDistance{amCalcStatusLabels	.top,	 -defPixelDist});
	amListViewArray		.bottom .setRelation(wildWeasel::alignment::relDistance{amCalcStatusLabels	.top,	 -defPixelDist});
	amEditOutput		.left	.setRelation(wildWeasel::alignment::relDistance{amListViewArray		.right ,  defPixelDist});
	amCalcStatusLabels	.top	.setRelation(wildWeasel::alignment::relDistance{amCalcButtons		.top,	 -defPixelDist-labelHeight});

	hListViewLayer		.alignAllItems();
	hListViewArray		.alignAllItems();
	hEditOutputBox		.alignAllItems();
}		

//-----------------------------------------------------------------------------
// Name: isCalculationOngoing()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMaxWinCalcDb::isCalculationOngoing()
{
	return (threadSolveIsRunning || threadTestLayerIsRunning);
}

//-----------------------------------------------------------------------------
// Name: buttonFuncCalcContinue()
// Desc: 
//-----------------------------------------------------------------------------
void miniMaxWinCalcDb::buttonFuncCalcStartOrContinue(void* pUser)
{
	// clear output window
	hEditOutputBox	.setText(L"");
	hButtonCalcPause.setText(L"Pause");
	
	// turn "running" LED on
	hLabelCalculationRunning.setText(L"Calculation running");
	hButtonCalcContinue		.setState(wildWeasel::guiElemState::GRAYED);
	hButtonCalcCancel		.setState(wildWeasel::guiElemState::DRAWED);
	hButtonCalcPause		.setState(wildWeasel::guiElemState::DRAWED);
	hButtonCalcTestLayer	.setState(wildWeasel::guiElemState::GRAYED);
	hButtonCalcTestAll		.setState(wildWeasel::guiElemState::GRAYED);

	// create thread
	hThreadSolve = thread(&miniMaxWinCalcDb::threadSolve, this);
	hThreadSolve.detach();
}

//-----------------------------------------------------------------------------
// Name: buttonFuncCalcCancel()
// Desc: 
//-----------------------------------------------------------------------------
void miniMaxWinCalcDb::buttonFuncCalcCancel(void* pUser)
{
	pMiniMax->cancelDatabaseCalculation();
}

//-----------------------------------------------------------------------------
// Name: buttonFuncCalcPause()
// Desc: 
//-----------------------------------------------------------------------------
void miniMaxWinCalcDb::buttonFuncCalcPause(void* pUser)
{
	pMiniMax->pauseDatabaseCalculation();

	if (hButtonCalcPause.getText().at(0) == L'U') {
		hButtonCalcPause.setText(L"Pause");
		hLabelCalculationRunning.setText(L"Calculation running");
	} else {
		hButtonCalcPause.setText(L"Unpause");
		hLabelCalculationRunning.setText(L"Calculation paused");
	}
}

//-----------------------------------------------------------------------------
// Name: lvSelectedLayerChanged()
// Desc: 
//-----------------------------------------------------------------------------
void miniMaxWinCalcDb::lvSelectedLayerChanged(unsigned int row, unsigned int col, wildWeasel::guiElemEvFol* guiElem, void* pUser)
{
	if (!isCalculationOngoing()) {
		hButtonCalcTestLayer.setState(wildWeasel::guiElemState::DRAWED);
	}
}

//-----------------------------------------------------------------------------
// Name: buttonFuncCalcTestAll()
// Desc: 
//-----------------------------------------------------------------------------
void miniMaxWinCalcDb::buttonFuncCalcTestAll(void* pUser)
{
	layersToTest = {};
	for (unsigned int curLayer = 0; curLayer < pMiniMax->getNumberOfLayers(); curLayer++) {
		layersToTest.push(curLayer);
	}
	buttonFuncCalcTest();
}

//-----------------------------------------------------------------------------
// Name: buttonFuncCalcTestLayer()
// Desc: 
//-----------------------------------------------------------------------------
void miniMaxWinCalcDb::buttonFuncCalcTestLayer(void* pUser)
{
	unsigned int curLayer = hListViewLayer.getFocussedRowIndex();
	if (curLayer >= pMiniMax->getNumberOfLayers()) return;
	layersToTest = {};
	layersToTest.push(curLayer);
	buttonFuncCalcTest();
}

//-----------------------------------------------------------------------------
// Name: buttonFuncCalcTest()
// Desc: 
//-----------------------------------------------------------------------------
void miniMaxWinCalcDb::buttonFuncCalcTest()
{
	// clear output window
	hEditOutputBox	.setText(L"");
	hButtonCalcPause.setText(L"Pause");
	
	// turn "running" LED on
	hLabelCalculationRunning.setText(L"Testing layer...");
	hButtonCalcContinue		.setState(wildWeasel::guiElemState::GRAYED);
	hButtonCalcCancel		.setState(wildWeasel::guiElemState::DRAWED);
	hButtonCalcPause		.setState(wildWeasel::guiElemState::DRAWED);
	hButtonCalcTestLayer	.setState(wildWeasel::guiElemState::GRAYED);
	hButtonCalcTestAll		.setState(wildWeasel::guiElemState::GRAYED);

	// create thread
	hThreadTestLayer = thread(&miniMaxWinCalcDb::threadProcTestLayer, this);
	hThreadTestLayer.detach();
}

//-----------------------------------------------------------------------------
// Name: ThreadProcTestLayer()
// Desc: CALCULATION-thread
//-----------------------------------------------------------------------------
void miniMaxWinCalcDb::threadProcTestLayer()
{
	// locals
	miniMax *				pMiniMax				= getMinimaxPointer();
	
	// 
	threadTestLayerIsRunning = true;
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);

	// calculate
	pMiniMax->layerInDatabase			= true;
	pMiniMax->calcDatabase				= false;
	pMiniMax->threadManager.uncancelExecution();
	pMiniMax->arrayInfos.vectorArrays.resize(miniMax::arrayInfoStruct::numArrayTypes*pMiniMax->skvfHeader.numLayers, pMiniMax->arrayInfos.listArrays.end());
	pMiniMax->setOutputStream(outputStream, nullptr, nullptr);
	pMiniMax->prepareDatabaseCalculation();
	while (layersToTest.size()) {
		unsigned int curLayer = layersToTest.front();
		layersToTest.pop();
		if (pMiniMax->layerStats[curLayer].knotsInLayer == 0) continue;
		pMiniMax->testLayer(curLayer);
		pMiniMax->unloadAllLayers();
		pMiniMax->unloadAllPlyInfos();
		if (pMiniMax->wasDatabaseCalculationCancelled()) break;
	}
	pMiniMax->wrapUpDatabaseCalculation(true);
	
	// turn "running" LED off
	hLabelCalculationRunning	.setText(L"Layer testing stopped");
	hLabelCalculatingLayer		.setText(L"");
	hLabelCalculationAction		.setText(L"");
	hButtonCalcContinue			.setState(wildWeasel::guiElemState::DRAWED);
	hButtonCalcCancel			.setState(wildWeasel::guiElemState::GRAYED);
	hButtonCalcPause			.setState(wildWeasel::guiElemState::GRAYED);
	hButtonCalcTestLayer		.setState(wildWeasel::guiElemState::GRAYED);
	hButtonCalcTestAll			.setState(wildWeasel::guiElemState::DRAWED);

	// terminate thread
	threadTestLayerIsRunning = false;
	threadConditionVariable.notify_all();
}

//-----------------------------------------------------------------------------
// Name: ThreadProcSolve()
// Desc: CALCULATION-thread
//-----------------------------------------------------------------------------
void miniMaxWinCalcDb::threadSolve()
{
	// locals
	miniMax *				pMiniMax				= getMinimaxPointer();
    bool					onlyPrepareLayer		= false;
	unsigned int			maxDepthOfTree			= 100;			// ... gibt es einen gescheiteren wert?

	// 
	threadSolveIsRunning = true;
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
	
	// calculate
	pMiniMax->setOutputStream(outputStream, nullptr, nullptr);
	pMiniMax->calculateDatabase(maxDepthOfTree, onlyPrepareLayer);

	// turn "running" LED off
	hLabelCalculationRunning	.setText(L"Calculation stopped");
	hLabelCalculatingLayer		.setText(L"");
	hLabelCalculationAction		.setText(L"");
	hButtonCalcContinue			.setState(wildWeasel::guiElemState::DRAWED);
	hButtonCalcCancel			.setState(wildWeasel::guiElemState::GRAYED);
	hButtonCalcPause			.setState(wildWeasel::guiElemState::GRAYED);
	hButtonCalcTestLayer		.setState(wildWeasel::guiElemState::GRAYED);
	hButtonCalcTestAll			.setState(wildWeasel::guiElemState::DRAWED);

	// terminate thread
	threadSolveIsRunning = false;
	threadConditionVariable.notify_all();
}

//-----------------------------------------------------------------------------
// Name: updateOutputControls()
// Desc: called by MAIN-thread
//-----------------------------------------------------------------------------
void miniMaxWinCalcDb::updateOutputControls(void* pUser)
{
	// locals
	miniMaxWinCalcDb	*	pMiniMaxWin				= (miniMaxWinCalcDb*) pUser;
	vector<unsigned int>	curCalculatedLayers;
	wstringstream			newOutputTextW;
	int						curLayerId;

	// Enter critical section
	EnterCriticalSection(&pMiniMaxWin->pMiniMax->csOsPrint);

	// any new stuff in the output buffer?
	if (pMiniMaxWin->outputStringBuf.str().size()) {		// ... does not work properly - it always returns true

		// insert content of stream buffer into edit control
		pMiniMaxWin->hEditOutputBox.insertString(pMiniMaxWin->outputStringBuf);

		// update label with currently calculated layer
		pMiniMaxWin->pMiniMax->getCurrentCalculatedLayer(curCalculatedLayers);
		newOutputTextW.str(L""); newOutputTextW << L"current calculated layer" << (curCalculatedLayers.size() ? L"s" : L"") << L": ";
		if (curCalculatedLayers.size()) newOutputTextW << curCalculatedLayers[0];
		for (curLayerId=1; curLayerId<(int)curCalculatedLayers.size(); curLayerId++) {
			newOutputTextW << L", " << curCalculatedLayers[curLayerId];
		}
		pMiniMaxWin->hLabelCalculatingLayer.setText(newOutputTextW.str().c_str());

		// update label with currently performed action
		newOutputTextW.str(L"");
		newOutputTextW << L"current action: " << pMiniMaxWin->pMiniMax->getCurrentActionStr();
		pMiniMaxWin->hLabelCalculationAction.setText(newOutputTextW.str().c_str());
	}

	while (pMiniMaxWin->pMiniMax->anyFreshlyCalculatedLayer()) {
		pMiniMaxWin->updateListItemLayer(pMiniMaxWin->pMiniMax->getLastCalculatedLayer());
	};

	while (pMiniMaxWin->pMiniMax->anyArrawInfoToUpdate()) {
		pMiniMaxWin->updateListItemArray(pMiniMaxWin->pMiniMax->getArrayInfoForUpdate());
	};

	// leave critical section
	LeaveCriticalSection(&pMiniMaxWin->pMiniMax->csOsPrint);
}

//-----------------------------------------------------------------------------
// Name: updateListItemLayer()
// Desc: called by MAIN-thread in pMiniMax->csOsPrint critical-section
//-----------------------------------------------------------------------------
void miniMaxWinCalcDb::updateListItemLayer(unsigned int layerNumber)
{
	// locals
	wstringstream			wssTmp;
	wildWeasel::color	layercolor;

	// choose a color
	if (pMiniMax->isLayerInDatabase(layerNumber)) {
		if (pMiniMax->getNumberOfKnotsInLayer(layerNumber) > 0) {
			layercolor = wildWeasel::color::green;
		} else {
			layercolor = wildWeasel::color::lightBlue;
		}
	} else {
		layercolor = wildWeasel::color::lightGreen;
	}
	hListViewLayer.setRowColor(layerNumber, layercolor);

	wssTmp.imbue(myLocale);
	wssTmp.str(L""); wssTmp << layerNumber;																hListViewLayer.setItemText(hListViewLayer.getItemIndex(layerNumber, 0), 0, wssTmp.str().c_str());
	wssTmp.str(L""); wssTmp << (pMiniMax->isLayerInDatabase(layerNumber) ? L"Yes" : L"No");				hListViewLayer.setItemText(hListViewLayer.getItemIndex(layerNumber, 1), 0, wssTmp.str().c_str());
	wssTmp.str(L""); wssTmp << pMiniMax->getNumWonStates(layerNumber);									hListViewLayer.setItemText(hListViewLayer.getItemIndex(layerNumber, 2), 0, wssTmp.str().c_str());
	wssTmp.str(L""); wssTmp << pMiniMax->getNumLostStates(layerNumber);									hListViewLayer.setItemText(hListViewLayer.getItemIndex(layerNumber, 3), 0, wssTmp.str().c_str());
	wssTmp.str(L""); wssTmp << pMiniMax->getNumDrawnStates(layerNumber);								hListViewLayer.setItemText(hListViewLayer.getItemIndex(layerNumber, 4), 0, wssTmp.str().c_str());
	wssTmp.str(L""); wssTmp << pMiniMax->getNumInvalidStates(layerNumber);								hListViewLayer.setItemText(hListViewLayer.getItemIndex(layerNumber, 5), 0, wssTmp.str().c_str());
	wssTmp.str(L""); wssTmp << pMiniMax->getNumberOfKnotsInLayer(layerNumber);							hListViewLayer.setItemText(hListViewLayer.getItemIndex(layerNumber, 6), 0, wssTmp.str().c_str());
	wssTmp.str(L""); wssTmp << pMiniMax->getLayerSizeInBytes(layerNumber);								hListViewLayer.setItemText(hListViewLayer.getItemIndex(layerNumber, 7), 0, wssTmp.str().c_str());
	wssTmp.str(L""); wssTmp << pMiniMax->getPartnerLayer(layerNumber);									hListViewLayer.setItemText(hListViewLayer.getItemIndex(layerNumber, 8), 0, wssTmp.str().c_str());
	wssTmp.str(L""); wssTmp << wildWeasel::string2wstring(pMiniMax->getOutputInformation(layerNumber));	hListViewLayer.setItemText(hListViewLayer.getItemIndex(layerNumber, 9), 0, wssTmp.str().c_str());

	hListViewLayer.getItemGuiElemPointer(hListViewLayer.getItemIndex(layerNumber, 9))->setTextAlignmentHorizontal(wildWeasel::alignmentHorizontal::LEFT);
}

//-----------------------------------------------------------------------------
// Name: updateListItemArray()
// Desc: called by MAIN-thread in pMiniMax->csOsPrint critical-section
//-----------------------------------------------------------------------------
void miniMaxWinCalcDb::updateListItemArray(miniMax::arrayInfoChange infoChange)
{
	// locals
	wstringstream	wssTmp;

	// array was deleted
	if (infoChange.arrayInfo == nullptr) {
		hListViewArray.removeAllItemsInRow(infoChange.itemIndex, true);
		hListViewArray.removeRow(infoChange.itemIndex, true);
	// array was created
	} else {

		// add a new row in the list view if not enough rows existend
		if (infoChange.itemIndex >= hListViewArray.getNumRows())  {
			hListViewArray.insertRowsAndItems_plainButton2D(infoChange.itemIndex, 1, listViewRowHeight, buttonImagesVoid, hFontOutputBox);
			hListViewArray.alignAllItems();
		}

		// update array info
		wssTmp.imbue(myLocale); 
		wssTmp.str(L""); wssTmp << infoChange.arrayInfo->belongsToLayer;			hListViewArray.setItemText(hListViewArray.getItemIndex(infoChange.itemIndex, 0), 0, wssTmp.str().c_str());
		wssTmp.str(L""); wssTmp << infoChange.arrayInfo->sizeInBytes;				hListViewArray.setItemText(hListViewArray.getItemIndex(infoChange.itemIndex, 2), 0, wssTmp.str().c_str());
		wssTmp.str(L"");
		switch (infoChange.arrayInfo->type)
		{
		case miniMax::arrayInfoStruct::arrayType_invalid:				wssTmp << "invalid";					break;
		case miniMax::arrayInfoStruct::arrayType_countArray:			wssTmp << "count array";				break;
		case miniMax::arrayInfoStruct::arrayType_plyInfos:				wssTmp << "plyInfo";					break;
		case miniMax::arrayInfoStruct::arrayType_layerStats:			wssTmp << "short know value";			break;
		case miniMax::arrayInfoStruct::arrayType_knotAlreadyCalculated:	wssTmp << "knots already calculated";	break;
		default:														wssTmp << "unknown type";				break;
		}
		hListViewArray.setItemText(hListViewArray.getItemIndex(infoChange.itemIndex, 1), 0, wssTmp.str().c_str());
		wssTmp.str(L"");
		if (infoChange.arrayInfo->compressedSizeInBytes) {
			wssTmp << "-";
		} else {
			wssTmp << infoChange.arrayInfo->compressedSizeInBytes;
		}
		hListViewArray.setItemText(hListViewArray.getItemIndex(infoChange.itemIndex, 3), 0, wssTmp.str().c_str());
	}
}

#pragma endregion
