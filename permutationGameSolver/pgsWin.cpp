/*********************************************************************
	pgsWin.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/

#include "pgsWin.h"

#pragma region initialization
//-----------------------------------------------------------------------------
// Name: pgsWin constructor()
// Desc: 
//-----------------------------------------------------------------------------
pgsWin::pgsWin(userParams &p)
{
	// set vars
	this->uP			= p;
	this->curZones		= new zoneClass();
	this->solver		= new permutationGameSolver();

	// solver
	solver->setParentClassPointer(this);

	// create events
	solveThread .hEventSolveNextRandomState	.create(uP.ww, false, false);	solveThread.hEventSolveNextRandomState	.setFunc(bind(&pgsWin::solveNextRandomState,								this, placeholders::_1), this, true);
	solveThread	.eventUpdateOnThreadVars	.create(uP.ww, false, false);	solveThread.eventUpdateOnThreadVars		.setFunc(bind(&pgsWin::pgsWinThreadSolve::updateResult,						&solveThread, placeholders::_1), &solveThread, true);
	solveThread	.eventThreadFinished		.create(uP.ww, false, false);	solveThread.eventThreadFinished			.setFunc(bind(&pgsWin::pgsWinThreadSolve::threadHasFinished,				&solveThread), this, true);
	concThread	.eventThreadFinished		.create(uP.ww, false, false);	concThread .eventThreadFinished			.setFunc(bind(&pgsWin::pgsWinThreadConcatenateSequences::threadHasFinished, &concThread ), this, true);
				 eventUpdateAll				.create(uP.ww, false, false);			    eventUpdateAll				.setFunc(bind(&pgsWin::updateAllLists, this, placeholders::_1),							   this, true);
}

//-----------------------------------------------------------------------------
// Name: pgsWin destructor()
// Desc: 
//-----------------------------------------------------------------------------
pgsWin::~pgsWin()
{
	// terminate threads
	terminateAllCalculationsImmediatelly();

	delete solver;
	delete curZones;
}

//-----------------------------------------------------------------------------
// Name: pgsWin destructor()
// Desc: 
//-----------------------------------------------------------------------------
bool pgsWin::createControls()
{
	// parameters ok?
	if (uP.ww == nullptr) return false;

	// create buttons
	hButtonZoneLoad							.create(uP.ww, *uP.buttonImages.buttonImagesZoneLoad			, bind(&pgsWin::buttonFuncLoadZoneFile,		this		), this, *uP.alignments.alignmentZoneButtons, 4);
	hButtonZoneSave							.create(uP.ww, *uP.buttonImages.buttonImagesZoneSave			, bind(&pgsWin::buttonFuncSaveZoneFile,		this		), this, *uP.alignments.alignmentZoneButtons, 0);
	hButtonZoneQuit							.create(uP.ww, *uP.buttonImages.buttonImagesZoneQuit			, bind(&pgsWin::showZoneControls,			this, false	), this, *uP.alignments.alignmentZoneButtons, 3);
	hButtonZoneInsert						.create(uP.ww, *uP.buttonImages.buttonImagesZoneInsert			, bind(&pgsWin::buttonFuncInsertZone,		this		), this, *uP.alignments.alignmentZoneButtons, 5);
	hButtonZoneDelete						.create(uP.ww, *uP.buttonImages.buttonImagesZoneDelete			, bind(&pgsWin::buttonFuncDeleteZone,		this		), this, *uP.alignments.alignmentZoneButtons, 1);
	hButtonZoneMoveUp						.create(uP.ww, *uP.buttonImages.buttonImagesZoneMoveUp			, bind(&pgsWin::buttonFuncMoveZoneUp,		this		), this, *uP.alignments.alignmentZoneButtons, 6);
	hButtonZoneMoveDown						.create(uP.ww, *uP.buttonImages.buttonImagesZoneMoveDown		, bind(&pgsWin::buttonFuncMoveZoneDown,		this		), this, *uP.alignments.alignmentZoneButtons, 2);
	hButtonZoneFindSequences				.create(uP.ww, *uP.buttonImages.buttonImagesZoneFindSequences	, bind(&pgsWin::buttonFuncFindSequences,	this		), this, *uP.alignments.alignmentZoneButtons, 7);
	
	hButtonZonePlay							.create(uP.ww, *uP.buttonImages.buttonImagesZonePlay			, bind(&pgsWin::buttonFuncZonePlay,			this		), this, *uP.alignments.alignmentShowSeqButtons, 0);
	hButtonZoneForward						.create(uP.ww, *uP.buttonImages.buttonImagesZoneForward			, bind(&pgsWin::buttonFuncZoneForward,		this		), this, *uP.alignments.alignmentShowSeqButtons, 1);
	hButtonZoneBackward						.create(uP.ww, *uP.buttonImages.buttonImagesZoneBackward		, bind(&pgsWin::buttonFuncZoneBackward,		this		), this, *uP.alignments.alignmentShowSeqButtons, 2);
	
	solveThread.hButtonSolvePlay			.create(uP.ww, *uP.buttonImages.buttonImagesZonePlay			, bind(&pgsWin::buttonFuncSolvePlay,		this		), this, *uP.alignments.alignmentSolveButtons, 0);
	solveThread.hButtonSolvePause			.create(uP.ww, *uP.buttonImages.buttonImagesSolvePause			, bind(&pgsWin::buttonFuncSolvePause,		this		), this, *uP.alignments.alignmentSolveButtons, 1);
	solveThread.hButtonSolveForward			.create(uP.ww, *uP.buttonImages.buttonImagesZoneForward			, bind(&pgsWin::buttonFuncSolveForward,		this		), this, *uP.alignments.alignmentSolveButtons, 2);
	solveThread.hButtonSolveBackward		.create(uP.ww, *uP.buttonImages.buttonImagesZoneBackward		, bind(&pgsWin::buttonFuncSolveBackward,	this		), this, *uP.alignments.alignmentSolveButtons, 3);
	solveThread.hButtonSolveFaster			.create(uP.ww, *uP.buttonImages.buttonImagesSolveFaster			, bind(&pgsWin::buttonFuncSolveFaster,		this		), this, *uP.alignments.alignmentSolveButtons, 4);
	solveThread.hButtonSolveSlower			.create(uP.ww, *uP.buttonImages.buttonImagesSolveSlower			, bind(&pgsWin::buttonFuncSolveSlower,		this		), this, *uP.alignments.alignmentSolveButtons, 5);
	solveThread.hButtonSolveShowStatistics	.create(uP.ww, *uP.buttonImages.buttonImagesSolveShowStatistics	, bind(&pgsWin::buttonFuncSolveShowStats,	this		), this, *uP.alignments.alignmentSolveButtons, 6);
	solveThread.hButtonSolveStopCalc		.create(uP.ww, *uP.buttonImages.buttonImagesSolveStopCalc		, bind(&pgsWin::buttonFuncSolveStopCalc,	this		), this, *uP.alignments.alignmentSolveButtons, 7);
	
	// check boxes
	float	textSizeEdit = 0.7f;
	hEditOverwriteDatabase	.create(uP.ww, *uP.buttonImages.buttonImagesChecked, *uP.buttonImages.buttonImagesUnchecked, 0);	hEditOverwriteDatabase	.setAlignment(*uP.alignments.alignmentZoneEditFields, 1);		
	hEditDoubleFeature		.create(uP.ww, *uP.buttonImages.buttonImagesChecked, *uP.buttonImages.buttonImagesUnchecked, 0);	hEditDoubleFeature		.setAlignment(*uP.alignments.alignmentZoneEditFields, 3);		
	hEditUseSingleReturn	.create(uP.ww, *uP.buttonImages.buttonImagesChecked, *uP.buttonImages.buttonImagesUnchecked, 0);	hEditUseSingleReturn	.setAlignment(*uP.alignments.alignmentZoneEditFields, 5);

	// drop downs
	hEditMaxSeqLength		.create(uP.ww, *uP.buttonImages.buttonImagesVoid, *uP.textures.textureTriangle, 0.5f);	hEditMaxSeqLength		.setAlignment(*uP.alignments.alignmentZoneEditFields, 2);
	hEditBackTrackingDepth	.create(uP.ww, *uP.buttonImages.buttonImagesVoid, *uP.textures.textureTriangle, 0.4f);	hEditBackTrackingDepth	.setAlignment(*uP.alignments.alignmentZoneEditFields, 0);
	hEditSearchDepthUsingSeq.create(uP.ww, *uP.buttonImages.buttonImagesVoid, *uP.textures.textureTriangle, 0.6f);	hEditSearchDepthUsingSeq.setAlignment(*uP.alignments.alignmentZoneEditFields, 4);
	
	hEditMaxSeqLength		.setTextAlignmentHorizontal(wildWeasel::alignmentHorizontal::LEFT);
	hEditBackTrackingDepth	.setTextAlignmentHorizontal(wildWeasel::alignmentHorizontal::LEFT);
	hEditSearchDepthUsingSeq.setTextAlignmentHorizontal(wildWeasel::alignmentHorizontal::LEFT);

	hEditMaxSeqLength		.setTextSize(textSizeEdit, textSizeEdit);												hEditMaxSeqLength		.setFont(uP.hFont2D);
	hEditBackTrackingDepth	.setTextSize(textSizeEdit, textSizeEdit);												hEditBackTrackingDepth	.setFont(uP.hFont2D);
	hEditSearchDepthUsingSeq.setTextSize(textSizeEdit, textSizeEdit);												hEditSearchDepthUsingSeq.setFont(uP.hFont2D);
	
	hEditMaxSeqLength		.createScrollBars(*uP.buttonImages.buttonImagesTriangle, *uP.buttonImages.buttonImagesVoid, *uP.buttonImages.buttonImagesVoid);	
	hEditBackTrackingDepth	.createScrollBars(*uP.buttonImages.buttonImagesTriangle, *uP.buttonImages.buttonImagesVoid, *uP.buttonImages.buttonImagesVoid);	
	hEditSearchDepthUsingSeq.createScrollBars(*uP.buttonImages.buttonImagesTriangle, *uP.buttonImages.buttonImagesVoid, *uP.buttonImages.buttonImagesVoid);	

	hEditMaxSeqLength		.setVisibilityRowScrollBar(true);														hEditMaxSeqLength		.setRowScrollBarWidth(15);												
	hEditBackTrackingDepth	.setVisibilityRowScrollBar(true);														hEditBackTrackingDepth	.setRowScrollBarWidth(15);												
	hEditSearchDepthUsingSeq.setVisibilityRowScrollBar(true);														hEditSearchDepthUsingSeq.setRowScrollBarWidth(15);												
	
	hEditMaxSeqLength		.insertTextItems(*uP.buttonImages.buttonImagesVoid, 1, 14, 1);							hEditMaxSeqLength		.setTextColor(wildWeasel::color::black);
	hEditBackTrackingDepth	.insertTextItems(*uP.buttonImages.buttonImagesVoid, 1, 14, 1);							hEditBackTrackingDepth	.setTextColor(wildWeasel::color::black);
	hEditSearchDepthUsingSeq.insertTextItems(*uP.buttonImages.buttonImagesVoid, 1,  6, 1);							hEditSearchDepthUsingSeq.setTextColor(wildWeasel::color::black);

	// create labels
	float	textSizeLabel	= 0.7f;
	hTextBackTrackingDepth		.create(uP.ww, wstring(L"backtracking depth:"		), uP.hFont2D, 0, *uP.alignments.alignmentZoneEditLabels, 0);		hTextBackTrackingDepth		.setTextSize(textSizeLabel, textSizeLabel);
	hTextOverwriteDatabase		.create(uP.ww, wstring(L"overwrite database:"		), uP.hFont2D, 0, *uP.alignments.alignmentZoneEditLabels, 1);		hTextOverwriteDatabase		.setTextSize(textSizeLabel, textSizeLabel);
	hTextMaxSeqLength			.create(uP.ww, wstring(L"max sequence length:"		), uP.hFont2D, 0, *uP.alignments.alignmentZoneEditLabels, 2);		hTextMaxSeqLength			.setTextSize(textSizeLabel, textSizeLabel);
	hTextDoubleFeature			.create(uP.ww, wstring(L"use double feature:"		), uP.hFont2D, 0, *uP.alignments.alignmentZoneEditLabels, 3);		hTextDoubleFeature			.setTextSize(textSizeLabel, textSizeLabel);
	hTextSearchDepthUsingSeq	.create(uP.ww, wstring(L"search depth using seq:"	), uP.hFont2D, 0, *uP.alignments.alignmentZoneEditLabels, 4);		hTextSearchDepthUsingSeq	.setTextSize(textSizeLabel, textSizeLabel);
	hTextUseSingleReturn		.create(uP.ww, wstring(L"use single return:"		), uP.hFont2D, 0, *uP.alignments.alignmentZoneEditLabels, 5);		hTextUseSingleReturn		.setTextSize(textSizeLabel, textSizeLabel);

	solveThread.hTextStepWithinZone	.create(uP.ww, wstring(L""						), uP.hFont2D, 0, *uP.alignments.alignmentSolveLabels, 0);
	solveThread.hTextStepInTotal	.create(uP.ww, wstring(L""						), uP.hFont2D, 0, *uP.alignments.alignmentSolveLabels, 1);
	solveThread.hTextShowingZone	.create(uP.ww, wstring(L""						), uP.hFont2D, 0, *uP.alignments.alignmentSolveLabels, 2);
	solveThread.hTextAnimationSpeed	.create(uP.ww, wstring(L""						), uP.hFont2D, 0, *uP.alignments.alignmentSolveLabels, 3);
	solveThread.hTextZonesSolved	.create(uP.ww, wstring(L""						), uP.hFont2D, 0, *uP.alignments.alignmentSolveLabels, 4);
	solveThread.hTextTopBranch		.create(uP.ww, wstring(L""						), uP.hFont2D, 0, *uP.alignments.alignmentSolveLabels, 5);
	solveThread.hTextRndStatesSolved.create(uP.ww, wstring(L""						), uP.hFont2D, 0, *uP.alignments.alignmentSolveLabels, 3);
	
	hTextZoneCurStep			.create(uP.ww, wstring(L""							), uP.hFont2D, 0, *uP.alignments.alignmentShowSeqButtons, 5);
	hTextBlockedMoves			.create(uP.ww, wstring(L"Blocked moves:"			), uP.hFont2D, 0, *uP.alignments.alignmentShowSeqButtons, 0);

	// create list view
    hListViewZone	.create(uP.ww, 0);	hListViewZone	.setAlignment(*uP.alignments.alignmentListViewZone		, 0);	hListViewZone	.setTextSize(0.6f, 0.6f);
	hListViewSeq	.create(uP.ww, 0);	hListViewSeq	.setAlignment(*uP.alignments.alignmentListViewSeq		, 0);	hListViewSeq	.setTextSize(0.6f, 0.6f);
	hListViewCal	.create(uP.ww, 0);	hListViewCal	.setAlignment(*uP.alignments.alignmentListViewCal		, 0);	hListViewCal	.setTextSize(0.6f, 0.6f);
    hListViewActCal	.create(uP.ww, 0);	hListViewActCal	.setAlignment(*uP.alignments.alignmentListViewActiveCal	, 0);	hListViewActCal	.setTextSize(0.6f, 0.6f);

    // Add columns to the list-view
	RECT	rc			= {0, 0, 130, 25};
	float	angle		= -0.24f * wwc::PI;
	float	textSize	= 0.5f; 

	hListViewZone.insertColumn_plainButton2D( 0, wstring(L"zone"						), uP.hFont2D,  30, angle, textSize, rc, *uP.buttonImages.buttonImagesVoid);
	hListViewZone.insertColumn_plainButton2D( 1, wstring(L"numRectangles"				), uP.hFont2D,  30, angle, textSize, rc, *uP.buttonImages.buttonImagesVoid);
	hListViewZone.insertColumn_plainButton2D( 2, wstring(L"num done calculations"		), uP.hFont2D,  30, angle, textSize, rc, *uP.buttonImages.buttonImagesVoid);
	hListViewZone.insertColumn_plainButton2D( 3, wstring(L"total calculation time"		), uP.hFont2D,  55, angle, textSize, rc, *uP.buttonImages.buttonImagesVoid);
	hListViewZone.insertColumn_plainButton2D( 4, wstring(L"num found sequences"			), uP.hFont2D,  55, angle, textSize, rc, *uP.buttonImages.buttonImagesVoid);
	hListViewZone.insertColumn_plainButton2D( 5, wstring(L"sequence concatenation"		), uP.hFont2D,  55, angle, textSize, rc, *uP.buttonImages.buttonImagesVoid);	concThread.columnIndex = 5;
	hListViewZone.insertColumn_plainButton2D( 6, wstring(L"max. num sequences"			), uP.hFont2D,  70, angle, textSize, rc, *uP.buttonImages.buttonImagesVoid);
																	   									  	  	 
	hListViewCal.insertColumn_plainButton2D(0, wstring(L"no."							), uP.hFont2D,  30, angle, textSize, rc, *uP.buttonImages.buttonImagesVoid);
	hListViewCal.insertColumn_plainButton2D(1, wstring(L"search depth"					), uP.hFont2D,  30, angle, textSize, rc, *uP.buttonImages.buttonImagesVoid);
	hListViewCal.insertColumn_plainButton2D(2, wstring(L"double feature"				), uP.hFont2D,  30, angle, textSize, rc, *uP.buttonImages.buttonImagesVoid);
	hListViewCal.insertColumn_plainButton2D(3, wstring(L"single return"					), uP.hFont2D,  30, angle, textSize, rc, *uP.buttonImages.buttonImagesVoid);
	hListViewCal.insertColumn_plainButton2D(4, wstring(L"num sequences"					), uP.hFont2D,  55, angle, textSize, rc, *uP.buttonImages.buttonImagesVoid);
	hListViewCal.insertColumn_plainButton2D(5, wstring(L"calculation duration"			), uP.hFont2D,  55, angle, textSize, rc, *uP.buttonImages.buttonImagesVoid);
	hListViewCal.insertColumn_plainButton2D(6, wstring(L"calculation date"				), uP.hFont2D,  90, angle, textSize, rc, *uP.buttonImages.buttonImagesVoid);
	hListViewCal.insertColumn_plainButton2D(7, wstring(L"cpu type"						), uP.hFont2D, 400, angle, textSize, rc, *uP.buttonImages.buttonImagesVoid);
	hListViewCal.insertColumn_plainButton2D(8, wstring(L"delete"						), uP.hFont2D,  55, angle, textSize, rc, *uP.buttonImages.buttonImagesVoid);
																	   										  
	hListViewActCal.insertColumn_plainButton2D( 0, wstring(L"zone number"				), uP.hFont2D,  30, angle, textSize, rc, *uP.buttonImages.buttonImagesVoid);
	hListViewActCal.insertColumn_plainButton2D( 1, wstring(L"max search depth"			), uP.hFont2D,  30, angle, textSize, rc, *uP.buttonImages.buttonImagesVoid);
	hListViewActCal.insertColumn_plainButton2D( 2, wstring(L"num possible moves"		), uP.hFont2D,  30, angle, textSize, rc, *uP.buttonImages.buttonImagesVoid);
	hListViewActCal.insertColumn_plainButton2D( 3, wstring(L"solutions found"			), uP.hFont2D,  30, angle, textSize, rc, *uP.buttonImages.buttonImagesVoid);
	hListViewActCal.insertColumn_plainButton2D( 4, wstring(L"num rectangles"			), uP.hFont2D,  30, angle, textSize, rc, *uP.buttonImages.buttonImagesVoid);
	hListViewActCal.insertColumn_plainButton2D( 5, wstring(L"use double feature"		), uP.hFont2D,  30, angle, textSize, rc, *uP.buttonImages.buttonImagesVoid);
	hListViewActCal.insertColumn_plainButton2D( 6, wstring(L"use single return "		), uP.hFont2D,  30, angle, textSize, rc, *uP.buttonImages.buttonImagesVoid);
	hListViewActCal.insertColumn_plainButton2D( 7, wstring(L"time elapsed"				), uP.hFont2D,  70, angle, textSize, rc, *uP.buttonImages.buttonImagesVoid);
	hListViewActCal.insertColumn_plainButton2D( 8, wstring(L"top branch"				), uP.hFont2D,  40, angle, textSize, rc, *uP.buttonImages.buttonImagesVoid);
	hListViewActCal.insertColumn_plainButton2D( 9, wstring(L"2nd branch"				), uP.hFont2D,  40, angle, textSize, rc, *uP.buttonImages.buttonImagesVoid);
	hListViewActCal.insertColumn_plainButton2D(10, wstring(L"3rd branch"				), uP.hFont2D,  40, angle, textSize, rc, *uP.buttonImages.buttonImagesVoid);
	hListViewActCal.insertColumn_plainButton2D(11, wstring(L"Pause"						), uP.hFont2D,  60, angle, textSize, rc, *uP.buttonImages.buttonImagesVoid);	rc.right = 100;
	hListViewActCal.insertColumn_plainButton2D(12, wstring(L"Stop"						), uP.hFont2D,  60, angle, textSize, rc, *uP.buttonImages.buttonImagesVoid);

	rc.right	=  50; rc.bottom = 20; hListViewSeq.insertColumn_plainButton2D(0, wstring(L"no."			), uP.hFont2D,  50,     0, textSize, rc, *uP.buttonImages.buttonImagesVoid);
	rc.right	=  50; rc.bottom = 20; hListViewSeq.insertColumn_plainButton2D(1, wstring(L"length"			), uP.hFont2D,  50,     0, textSize, rc, *uP.buttonImages.buttonImagesVoid);
	rc.right	= 275; rc.bottom = 20; hListViewSeq.insertColumn_plainButton2D(2, wstring(L"unique moves"	), uP.hFont2D, 275,     0, textSize, rc, *uP.buttonImages.buttonImagesVoid);
	
	hListViewZone	.assignOnItemChanged(bind(&pgsWin::processListViewItemChanged_Zone,		this, placeholders::_1, placeholders::_2, placeholders::_3, placeholders::_4), this);
	hListViewSeq	.assignOnItemChanged(bind(&pgsWin::processListViewItemChanged_Seq,		this, placeholders::_1, placeholders::_2, placeholders::_3, placeholders::_4), this);
	hListViewCal	.assignOnItemChanged(bind(&pgsWin::processListViewItemChanged_Cal,		this, placeholders::_1, placeholders::_2, placeholders::_3, placeholders::_4), this);
	hListViewActCal	.assignOnItemChanged(bind(&pgsWin::processListViewItemChanged_ActCal,	this, placeholders::_1, placeholders::_2, placeholders::_3, placeholders::_4), this);
	
	hListViewZone	.setColumnHeaderHeight(20);								hListViewZone	.setColumnScrollBarHeight(20);			hListViewZone	.setRowScrollBarWidth(20);
	hListViewSeq	.setColumnHeaderHeight(20);								hListViewSeq	.setColumnScrollBarHeight(20);			hListViewSeq	.setRowScrollBarWidth(20);
	hListViewCal	.setColumnHeaderHeight(20);								hListViewCal	.setColumnScrollBarHeight(20);			hListViewCal	.setRowScrollBarWidth(20);
	hListViewActCal	.setColumnHeaderHeight(20);								hListViewActCal	.setColumnScrollBarHeight(20);			hListViewActCal	.setRowScrollBarWidth(20);

	hListViewZone	.createScrollBars(*uP.buttonImages.buttonImagesTriangle, *uP.buttonImages.buttonImagesVoid, *uP.buttonImages.buttonImagesVoid);
	hListViewSeq	.createScrollBars(*uP.buttonImages.buttonImagesTriangle, *uP.buttonImages.buttonImagesVoid, *uP.buttonImages.buttonImagesVoid);
	hListViewCal	.createScrollBars(*uP.buttonImages.buttonImagesTriangle, *uP.buttonImages.buttonImagesVoid, *uP.buttonImages.buttonImagesVoid);
	hListViewActCal	.createScrollBars(*uP.buttonImages.buttonImagesTriangle, *uP.buttonImages.buttonImagesVoid, *uP.buttonImages.buttonImagesVoid);

	hListViewZone	.setVisibilityColumnScrollBar(false);					hListViewZone	.setVisibilityRowScrollBar(true);		hListViewZone	.setVisibilityColumnHeader(true);
	hListViewSeq	.setVisibilityColumnScrollBar(false);					hListViewSeq	.setVisibilityRowScrollBar(true);		hListViewSeq	.setVisibilityColumnHeader(true);
	hListViewCal	.setVisibilityColumnScrollBar(true);					hListViewCal	.setVisibilityRowScrollBar(true);		hListViewCal	.setVisibilityColumnHeader(true);
	hListViewActCal	.setVisibilityColumnScrollBar(false);					hListViewActCal	.setVisibilityRowScrollBar(true);		hListViewActCal	.setVisibilityColumnHeader(true);

	hListViewZone	.setMarkerColor(wildWeasel::color::lightBlue);		hListViewZone	.setSelectionMode(wildWeasel::listView2D::selectionMode::ROW_WISE);
	hListViewSeq	.setMarkerColor(wildWeasel::color::lightBlue);		hListViewSeq	.setSelectionMode(wildWeasel::listView2D::selectionMode::ROW_WISE);
	hListViewCal	.setMarkerColor(wildWeasel::color::lightBlue);		hListViewCal	.setSelectionMode(wildWeasel::listView2D::selectionMode::ROW_WISE);
	hListViewActCal	.setMarkerColor(wildWeasel::color::lightBlue);		hListViewActCal	.setSelectionMode(wildWeasel::listView2D::selectionMode::ROW_WISE);

	hListViewZone	.setPosition(0, 0, true);								hListViewZone	.alignAllItems();
	hListViewSeq	.setPosition(0, 0, true);								hListViewSeq	.alignAllItems();
	hListViewCal	.setPosition(0, 0, true);								hListViewCal	.alignAllItems();
	hListViewActCal	.setPosition(0, 0, true);								hListViewActCal	.alignAllItems();

	// assign function on column header button (for sequence concatenation calculation)
	hButtonZoneSeqConcatenate = hListViewZone	.getColumnGuiElemPointer(concThread.columnIndex)->getPointer<wildWeasel::plainButton2D>();
	hButtonZoneSeqConcatenate->assignOnLeftMouseButtonReleased(	bind(&pgsWin::buttonFuncSeqConcatenate, this, placeholders::_1, placeholders::_2), (void*) -1);

	// create border lines
	hBorderLineZone		.create(uP.ww, *uP.textures.textureLine, *uP.textures.textureCorner, 0); 	hBorderLineZone		.setText(L"Zones");											hBorderLineZone		.setFont(uP.hFont2D);	
	hBorderLineCal		.create(uP.ww, *uP.textures.textureLine, *uP.textures.textureCorner, 0); 	hBorderLineCal		.setText(L"Done Calculations");								hBorderLineCal		.setFont(uP.hFont2D);	
	hBorderLineSeq		.create(uP.ww, *uP.textures.textureLine, *uP.textures.textureCorner, 0); 	hBorderLineSeq		.setText(L"Sequences");										hBorderLineSeq		.setFont(uP.hFont2D);	
	hBorderLineActCal	.create(uP.ww, *uP.textures.textureLine, *uP.textures.textureCorner, 0); 	hBorderLineActCal	.setText(L"Active calculations");							hBorderLineActCal	.setFont(uP.hFont2D);	
	hBorderLineBlocked	.create(uP.ww, *uP.textures.textureLine, *uP.textures.textureCorner, 0); 	hBorderLineBlocked	.setText(L"Used settings in new calculation");				hBorderLineBlocked	.setFont(uP.hFont2D);	

	hBorderLineZone		.setTextOffset(35, -10); hBorderLineZone	.setGapWidthBetweenTextAndLine(30);	hBorderLineZone		.setTextWidth(100);
	hBorderLineCal		.setTextOffset(35, -10); hBorderLineCal		.setGapWidthBetweenTextAndLine(30);	hBorderLineCal		.setTextWidth(100);
	hBorderLineSeq		.setTextOffset(35, -10); hBorderLineSeq		.setGapWidthBetweenTextAndLine(30);	hBorderLineSeq		.setTextWidth(100);
	hBorderLineActCal	.setTextOffset(35, -10); hBorderLineActCal	.setGapWidthBetweenTextAndLine(30);	hBorderLineActCal	.setTextWidth(100);
	hBorderLineActCal	.setTextOffset(35, -10); hBorderLineBlocked	.setGapWidthBetweenTextAndLine(30);	hBorderLineBlocked	.setTextWidth(220);

	alignmentBorderZone				.setInsideAnotherRect(*uP.alignments.alignmentListViewZone		);
	alignmentBorderCal				.setInsideAnotherRect(*uP.alignments.alignmentListViewCal		);		
	alignmentBorderSeq				.setInsideAnotherRect(*uP.alignments.alignmentListViewSeq		);		
	alignmentBorderActiveCal		.setInsideAnotherRect(*uP.alignments.alignmentListViewActiveCal	);	
	alignmentBorderBlockedBorder	.setInsideAnotherRect(*uP.alignments.alignmentBlockedBorder		);		

	hBorderLineZone		.setTextSize(textSize, textSize); hBorderLineZone	.setAlignment(*uP.alignments.alignmentListViewZone		);
	hBorderLineCal		.setTextSize(textSize, textSize); hBorderLineCal	.setAlignment(*uP.alignments.alignmentListViewCal		);
	hBorderLineSeq		.setTextSize(textSize, textSize); hBorderLineSeq	.setAlignment(*uP.alignments.alignmentListViewSeq		);
	hBorderLineActCal	.setTextSize(textSize, textSize); hBorderLineActCal	.setAlignment(*uP.alignments.alignmentListViewActiveCal	);
	hBorderLineBlocked	.setTextSize(textSize, textSize); hBorderLineBlocked.setAlignment(*uP.alignments.alignmentBlockedBorder		);

	// assign ... processMouseOverCheckBox
	//for (unsigned int i = 0; i < hCheckBoxBlockedMoves.size(); hCheckBoxBlockedMoves++) {
	//	hCheckBoxBlockedMoves[i].assignOnMouseEnteredRegion(bind(...), this);
	//}

	// message for another control
//	unsigned int uniqueMoveId;
//	} else if (!cbBlockedMovesByUser && hCheckBoxBlockedMoves.size() != 0) {
//		for (uniqueMoveId=0; uniqueMoveId<numUniqueMoves; uniqueMoveId++) {
//			if (hControl == hCheckBoxBlockedMoves[uniqueMoveId])	  { buttonBlockedMovesChanged(uniqueMoveId);	return true;}}
//		return false;

	return true;
}
#pragma endregion

/************************************************************************************************************************************\
  set zones mode
\************************************************************************************************************************************/

#pragma region setZonesMode
//-----------------------------------------------------------------------------
// Name: buttonFuncMoveZoneDown()
// Desc: Function called when the button 'move down' is pressed.
//-----------------------------------------------------------------------------
void pgsWin::buttonFuncMoveZoneDown()
{
	// not first item?
	if (selZone==curZones->numZones-1) return;
	
	// move zone
	curZones->moveZoneDown(selZone);

	// reload seq cont
	loadSequenceContainerFiles(false);

	selZone++;

	// update list view items
	updateListViewZoneItems();

	// update edit fields
	setSelItemInListViewZone(selZone);
}

//-----------------------------------------------------------------------------
// Name: buttonFuncMoveZoneUp()
// Desc: Function called when the button 'move up' is pressed.
//-----------------------------------------------------------------------------
void pgsWin::buttonFuncMoveZoneUp()
{
	// not first item?
	if (selZone==0) return;

	// move zone
	curZones->moveZoneUp(selZone);

	// reload seq cont
	loadSequenceContainerFiles(false);

	selZone--;

	// update list view items
	updateListViewZoneItems();

	// update edit fields
	setSelItemInListViewZone(selZone);
}

//-----------------------------------------------------------------------------
// Name: setFieldSize()
// Desc: 
//-----------------------------------------------------------------------------
void pgsWin::setFieldSize(unsigned int theFieldSize, wstring& databaseDirectory, wstring& databaseFileType)
{
	uP.strDatabaseFileType = databaseFileType;
	solver->setFieldSize(theFieldSize, databaseDirectory, databaseFileType);
}

//-----------------------------------------------------------------------------
// Name: loadDefaultZoneFile()
// Desc: 
//-----------------------------------------------------------------------------
void pgsWin::loadDefaultZoneFile(wstring &defZoneFileName, wstring &extensionFilter, wstring &defaultExtension, unsigned int numFields, unsigned int numMoves, vector<unsigned int>& moveIndicesMap, vector<unsigned int>& reverseMoves)
{
	// locals
    LVITEM   lv  = { 0 };

	// parameters ok?
	if (uP.strDatabaseDir	  .size() == 0) return;
	if (uP.strDatabaseFileType.size() == 0) return;

	// delete all zones if necessary
	deleteAllZones();

	// load default zone file
	if (!curZones->load(defZoneFileName.c_str())) {
		curZones->setNewScenario(numMoves, numFields);
	}
	if (uP.functions.setDefaultBlockedMoves) uP.functions.setDefaultBlockedMoves(curZones);

	this->selZone			= 0;
	this->selCalculation	= 0;
	this->selSequence		= 0;
	this->extFilter			= extensionFilter;
	this->defExtension		= defaultExtension;
	this->moveIndicesMap	= moveIndicesMap;
	this->reverseMoves		= reverseMoves;

	solver->setFieldSize(numFields, uP.strDatabaseDir, uP.strDatabaseFileType);

	// blocked moves check boxes controlled by user?
//	DWORD	 i;
//	if (hCheckBoxBlockedMoves		!=NULL) {
//		for (i=0; i<numUniqueMoves; i++) {
//			DestroyWindow(hCheckBoxBlockedMoves[i]);
//		}
//		delete [] hCheckBoxBlockedMoves;
//		hCheckBoxBlockedMoves = NULL;
//	}

	// calculate duplicatesMapBlockedMoves, reverseUniqueMoves and numUniqueMoves
	{
		zoneClass	 myZones;					
		curZones->copyTo(&myZones);
		myZones.vanishDuplicates(moveIndicesMap, reverseMoves, NULL, &reverseUniqueMoves, &duplicatesMapBlockedMoves, &invDuplicatesMapBlockedMoves);
		numUniqueMoves = myZones.numMoves;
	}

	// auto mode
//	RECT	 rc;
//	if (!uP.cbBlockedMovesByUser) {
//		// recreate check boxes for blocked moves
//		hCheckBoxBlockedMoves = new HWND[numUniqueMoves];
//		for (i=0; i<numUniqueMoves; i++) {
//			ww->calcControlPos(i, &rc, &ccBlockedMoves);	hCheckBoxBlockedMoves[i]	= CreateWindowW(L"BUTTON", L"",							BS_PUSHBUTTON | WS_CHILD | SS_CENTER | BS_AUTOCHECKBOX, rc.left, rc.top, rc.right, rc.bottom, hWnd, NULL, hInst, NULL);
//		}
//	}

	// Load Sequence container files
	loadSequenceContainerFiles(true);

	// delete items in listview and insert some
	hListViewZone.removeAllItems(true);
	hListViewZone.removeAllRows(true);
	insertRowInListViewZone(0, curZones->numZones);

	// update text of items
	updateListViewZoneItems();
}

//-----------------------------------------------------------------------------
// Name: buttonFuncLoadZoneFile()
// Desc: Function called when the button 'load' is pressed.
//-----------------------------------------------------------------------------
void pgsWin::buttonFuncLoadZoneFile()
{
	if (extFilter.length() == 0) return;

	// open dialog
	wstring pathName;

	if (uP.ww->getOpenFileName(pathName, extFilter)) {

		if (!curZones->load(pathName)) {
			uP.ww->showMessageBox(L"Error", L"Could not load zones of the selected file.", MB_OK | MB_ICONSTOP | MB_APPLMODAL);
		} else {

			// call curZones->setDefaultBlockedMoves()
			if (uP.functions.setDefaultBlockedMoves) uP.functions.setDefaultBlockedMoves(curZones);

			// Load Sequence container files
			loadSequenceContainerFiles(true);

			// delete items in listview
			hListViewZone.removeAllItems(true);
			hListViewZone.removeAllRows(true);
			insertRowInListViewZone(0, curZones->numZones);

			// currently selected zone
			selZone = 0;

			// update text of items
			updateListViewZoneItems();

			// update edit fields
			setSelItemInListViewZone(selZone);
		}
	}
}

//-----------------------------------------------------------------------------
// Name: unloadSequenceContainerFile()
// Desc: The content of the class 'seqContainers[zoneNumber]' is deleted.
//-----------------------------------------------------------------------------
void pgsWin::unloadSequenceContainerFile(unsigned int zoneNumber)
{
	// locals
	unsigned int j;

	if (seqContainers.size() <= zoneNumber) return;

	if (seqContainers[zoneNumber].sequences.size()) {
		for (j=0; j<seqContainers[zoneNumber].numSequences; j++) {
			seqContainers[zoneNumber].sequences[j].deleteArrays();
		}
		//delete [] seqContainers[zoneNumber].sequences;
	}
	if (seqContainers[zoneNumber].calculations.size()) {
		for (j=0; j<seqContainers[zoneNumber].numCalculations; j++) {
			seqContainers[zoneNumber].calculations[j].initialBlockedMoves	.clear();
			seqContainers[zoneNumber].calculations[j].sequenceIndexInFile	.clear();
			seqContainers[zoneNumber].calculations[j].seqBlock				.clear();
		}
		//delete [] seqContainers[zoneNumber].calculations;
	}
}

//-----------------------------------------------------------------------------
// Name: unloadSequenceContainerFiles()
// Desc: The entire content of the class-array 'seqContainers' is deleted.
//-----------------------------------------------------------------------------
void pgsWin::unloadSequenceContainerFiles()
{
	for (unsigned int i=0; i<seqContainers.size(); i++) {
		unloadSequenceContainerFile(i);
	}
	seqContainers			.clear(); 
	theZonesSeqContainer	.clear();
}

//-----------------------------------------------------------------------------
// Name: loadSequenceContainerFiles()
// Desc: First the entire content of the class-array 'seqContainers' is deleted.
//       Then a file containing the calculated sequences is loaded from drive
//       for each zone.
//-----------------------------------------------------------------------------
void pgsWin::loadSequenceContainerFiles(bool redimSeqContainersArray)
{
	// locals
	unsigned int	zoneNumber;

	// allocate mem for sequence containers
	if (redimSeqContainersArray) {
		unloadSequenceContainerFiles();
		seqContainers			.resize(curZones->numZones);
		theZonesSeqContainer	.resize(curZones->numZones*curZones->numFields, 0);
		for (zoneNumber=0; zoneNumber<curZones->numZones; zoneNumber++) {
			seqContainers[zoneNumber].calculations			.clear();
			seqContainers[zoneNumber].sequences				.clear();
			seqContainers[zoneNumber].numSequences			= 0;
			seqContainers[zoneNumber].numCalculations		= 0;
			seqContainers[zoneNumber].numFields				= curZones->numFields;
			seqContainers[zoneNumber].numMoves				= curZones->numMoves;
			seqContainers[zoneNumber].totalCalculationTime	= 0;
			seqContainers[zoneNumber].offsetSequences		= sizeof(seqContainer);
			seqContainers[zoneNumber].offsetCalculations	= sizeof(seqContainer);
		}
	}

	// process each zone
	for (zoneNumber=0; zoneNumber<curZones->numZones; zoneNumber++) {
		loadSequenceContainerFile(zoneNumber);
	}
}

//-----------------------------------------------------------------------------
// Name: loadSequenceContainerFiles()
// Desc: First the content of the class 'seqContainers[zoneNumber]' is deleted.
//       Then a single file containing the calculated sequences is loaded from 
//       drive for the desired zone.
//-----------------------------------------------------------------------------
void pgsWin::loadSequenceContainerFile(unsigned int zoneNumber)
{
	// locals
	vectui			pgsZone;

	// get zone
	curZones->translateForPGS(zoneNumber, pgsZone);

	// is it necessary to relaod the sequence container?
	if (equal(pgsZone.begin(), pgsZone.end(), theZonesSeqContainer.begin() + zoneNumber * curZones->numFields)) {
		return;
	} else {
		copy (pgsZone.begin(), pgsZone.end(), theZonesSeqContainer.begin() + zoneNumber * curZones->numFields);
		unloadSequenceContainerFile(zoneNumber);
	}

	// load sequence container
	if (!solver->loadSeqContainer(pgsZone, seqContainers[zoneNumber], true)) {
		seqContainers[zoneNumber].numFields				= curZones->numFields;
		seqContainers[zoneNumber].numMoves				= curZones->numMoves;
		seqContainers[zoneNumber].totalCalculationTime	= 0;
		seqContainers[zoneNumber].offsetSequences		= sizeof(seqContainer);
		seqContainers[zoneNumber].offsetCalculations	= sizeof(seqContainer);
		seqContainers[zoneNumber].numSequences			= 0;
		seqContainers[zoneNumber].numCalculations		= 0;
		seqContainers[zoneNumber].sequences				.clear();
		seqContainers[zoneNumber].calculations			.clear();
	}
}

//-----------------------------------------------------------------------------
// Name: buttonFuncSaveZoneFile()
// Desc: Function called when the button 'save' is pressed.
//-----------------------------------------------------------------------------
void pgsWin::buttonFuncSaveZoneFile()
{
	if (extFilter.length() == 0) return;

	wstring pathName;
	if (uP.ww->getSaveFileName(pathName, extFilter)) {
		if (!curZones->save(pathName)) {
			uP.ww->showMessageBox(L"Error", L"Could not save zones into the selected file.", MB_OK | MB_ICONSTOP | MB_APPLMODAL);
		}
	}
}

//-----------------------------------------------------------------------------
// Name: buttonFuncDeleteZone()
// Desc: Function called when the button 'delete' is pressed.
//-----------------------------------------------------------------------------
void pgsWin::buttonFuncDeleteZone()
{
	// delete zone
	curZones->deleteZone(selZone);

	// reload seq cont
	loadSequenceContainerFiles(true);

	// delete item in listview
	hListViewZone.removeAllItemsInRow(selZone, true);
	hListViewZone.removeRow(selZone, true);
	hListViewZone.alignAllItems();

	// update list view items
	updateListViewZoneItems();

	// update zone state
	uP.functions.updateLabelsAndButtonsOfZoneState(this);

	// update edit fields
	setSelItemInListViewZone(selZone);
}

//-----------------------------------------------------------------------------
// Name: buttonFuncInsertZone()
// Desc: Function called when the button 'insert' is pressed.
//-----------------------------------------------------------------------------
void pgsWin::buttonFuncInsertZone()
{
	// insert zone after current selected one
	if (curZones->numZones > selZone) selZone++;

	// delete zone
	curZones->insertZone(selZone);

	// reload seq cont
	loadSequenceContainerFiles(true);

	// delete item in listview
	insertRowInListViewZone(selZone, 1);

	// update listview items
	updateListViewZoneItems();

	// update edit fields
	setSelItemInListViewZone(selZone);
}

//-----------------------------------------------------------------------------
// Name: terminateAllCalculationsImmediatelly()
// Desc: 
//-----------------------------------------------------------------------------
void pgsWin::terminateAllCalculationsImmediatelly()
{
	solveThread.stop();
	concThread .stop();
	for (auto& curCalculation : findSeqThreads) {
		if (curCalculation != &concThread) {
			delete curCalculation;
		}
	}
}

//-----------------------------------------------------------------------------
// Name: deleteAllZones()
// Desc: All items of both lists are removed and each zone of the 
//       class 'curZones' is deleted. Then setSelItemInListViewZone() is called.
//-----------------------------------------------------------------------------
void pgsWin::deleteAllZones()
{
	// delete items in listview
	hListViewZone	.removeAllItems(true);
	hListViewSeq	.removeAllItems(true);
	hListViewCal	.removeAllItems(true);
	hListViewActCal .removeAllItems(true);

	// stop all calculations
	terminateAllCalculationsImmediatelly();

	// delete sequence container
	unloadSequenceContainerFiles();

	// delete zones
	curZones->deleteAllZones();

	// update edit fields
	selZone			= 0;
}

//-----------------------------------------------------------------------------
// Name: buttonDeleteCalculation()
// Desc: 
//-----------------------------------------------------------------------------
void pgsWin::buttonDeleteCalculation(wildWeasel::guiElemEvFol* elem, void* pUser)
{
	// check if user really wants to do this
	if (uP.ww->showMessageBox(L"Sure?", L"Do you really want to remove this calculation?", MB_OKCANCEL) != IDOK) return;

	// locals
	vectui			pgsZone;

	// get zone
	curZones->translateForPGS(selZone, pgsZone);

	// load file, remove calculation and save
	solver->removeCalculationFromFile(pgsZone, selCalculation);

	// update gui
	eventUpdateAll.set();
}

//-----------------------------------------------------------------------------
// Name: processListViewItemChanged_Zone()
// Desc: 
//-----------------------------------------------------------------------------
void pgsWin::processListViewItemChanged_Zone(unsigned int rowIndex, unsigned int columnIndex, wildWeasel::guiElemEvFol* guiElem, void* pUser)
{
	// stop any animation
	processListViewItemChanged_preFunc();
	
	// locals
	wstringstream	wss;
					selZone			= rowIndex;

	// quit if invalid zone was selected
	if (selZone >= curZones->numZones) return;

	// update values
	hEditOverwriteDatabase	.setCheckState(curZones->numZones > 0 && curZones->zoneProp[selZone].overWriteDatabase > 0);
	hEditDoubleFeature		.setCheckState(curZones->numZones > 0 && curZones->zoneProp[selZone].useDoubleFeature  > 0);
	hEditUseSingleReturn	.setCheckState(curZones->numZones > 0 && curZones->zoneProp[selZone].useSingleReturn   > 0);

	wss.str(L""); wss << ((curZones->numZones > 0) ? curZones->zoneProp[selZone].backTrackingDepth			: 0);	hEditBackTrackingDepth	.setText(wss.str()); 
	wss.str(L""); wss << ((curZones->numZones > 0) ? curZones->zoneProp[selZone].maxSequenceLength			: 0);	hEditMaxSeqLength		.setText(wss.str()); 
	wss.str(L""); wss << ((curZones->numZones > 0) ? curZones->zoneProp[selZone].searchDepthUsingSequences	: 0);	hEditSearchDepthUsingSeq.setText(wss.str()); 

	// update check boxes
	setCBoxesShowingBlockedMoves(curZones->zoneProp[selZone].excludedMove, 0, true);

	// clear calculations and sequence list
	hListViewCal.removeAllItems(true);
	hListViewSeq.removeAllItems(true);
	hListViewCal.removeAllRows (true);
	hListViewSeq.removeAllRows (true);

	// do not allow use of play buttons (since no specific sequence is selected)
	hTextZoneCurStep    .setState(wildWeasel::guiElemState::GRAYED);
	hButtonZonePlay		.setState(wildWeasel::guiElemState::GRAYED);
	hButtonZoneForward  .setState(wildWeasel::guiElemState::GRAYED);
	hButtonZoneBackward .setState(wildWeasel::guiElemState::GRAYED);
	hTextZoneCurStep	.setText(L"");

	// Add rows
	if (seqContainers.size() > selZone && seqContainers[selZone].numCalculations > 0) { 
		hListViewCal.insertRowsAndItems_plainButton2D(0, seqContainers[selZone].numCalculations, uP.listRowHeight, *uP.buttonImages.buttonImagesListBackground, uP.hFont2D);

		for (unsigned int i=0; i<seqContainers[selZone].numCalculations; i++) {
			wss.str(L""); wss << i;																			hListViewCal.setItemText(i, 0, 0, wss.str().c_str());
			wss.str(L""); wss << seqContainers[selZone].calculations[i].maxSearchDepth;						hListViewCal.setItemText(i, 1, 0, wss.str().c_str());
			wss.str(L""); wss << seqContainers[selZone].calculations[i].useDoubleFeature;					hListViewCal.setItemText(i, 2, 0, wss.str().c_str());
			wss.str(L""); wss << seqContainers[selZone].calculations[i].useSingleReturn;					hListViewCal.setItemText(i, 3, 0, wss.str().c_str());
			wss.str(L""); wss << seqContainers[selZone].calculations[i].numSequencesInFile;					hListViewCal.setItemText(i, 4, 0, wss.str().c_str());
			wss.str(L""); wss << seqContainers[selZone].calculations[i].calculationDuration / 60000;		hListViewCal.setItemText(i, 5, 0, wss.str().c_str());
			wss.str(L""); wss << seqContainers[selZone].calculations[i].dateOfCalculation.wDay		<< L"."
							  << seqContainers[selZone].calculations[i].dateOfCalculation.wMonth	<< L"."
							  << seqContainers[selZone].calculations[i].dateOfCalculation.wYear;			hListViewCal.setItemText(i, 6, 0, wss.str().c_str());
			wss.str(L""); wss << seqContainers[selZone].calculations[i].cpuType;							hListViewCal.setItemText(i, 7, 0, wss.str().c_str());
			wss.str(L""); wss << L"delete";																	hListViewCal.setItemText(i, 8, 0, wss.str().c_str());
			
			// assign delete calculation function on button
			auto buttonDelete = hListViewCal.getItem(i, 8)->getGuiElemPointer()->items.front()->getPointer<wildWeasel::plainButton2D>();
			// wildWeasel::plainButton2D* buttonDelete = (wildWeasel::plainButton2D*) hListViewCal.getItemPointer(i, 8)->items.front();
			buttonDelete->assignOnLeftMouseButtonReleased(bind(&pgsWin::buttonDeleteCalculation, this, placeholders::_1, placeholders::_2), this);
	}}
	hListViewCal.setTextSize(uP.listViewTextSize, uP.listViewTextSize);
	hListViewCal.setState(wildWeasel::guiElemState::DRAWED);
	hListViewCal.alignAllItems();
//	if (!uP.ww->mainLoadingScreen.isActive()) uP.ww->performResourceUpload();

	// update zone cube
	uP.functions.updateLabelsAndButtonsOfZoneState(this);
}

//-----------------------------------------------------------------------------
// Name: processListViewItemChanged_Seq()
// Desc: 
//-----------------------------------------------------------------------------
void pgsWin::processListViewItemChanged_Seq(unsigned int rowIndex, unsigned int columnIndex, wildWeasel::guiElemEvFol* guiElem, void* pUser)
{
	// stop any animation
	processListViewItemChanged_preFunc();

	// locals
	selSequence	= rowIndex;
	unsigned int	curSeq;
	wstringstream	ss;

	if (calcCurSeq(curSeq)) {
		hTextZoneCurStep	.setState(wildWeasel::guiElemState::DRAWED);
		hButtonZonePlay		.setState(wildWeasel::guiElemState::DRAWED);
		hButtonZoneForward	.setState(wildWeasel::guiElemState::DRAWED);
		hButtonZoneBackward	.setState(wildWeasel::guiElemState::GRAYED);
		ss.str(L""); ss << L"step: " << solveThread.curShowedStep << L"/" << seqContainers[selZone].sequences[curSeq].solPath.numSteps;					
		hTextZoneCurStep.setText(ss.str());
		showingSeqMove = true;
		uP.functions.zoneDoMoveAndShow(0, true);
	}
}

//-----------------------------------------------------------------------------
// Name: processListViewItemChanged_ActCal()
// Desc: 
//-----------------------------------------------------------------------------
void pgsWin::processListViewItemChanged_ActCal(unsigned int rowIndex, unsigned int columnIndex, wildWeasel::guiElemEvFol* guiElem, void* pUser)
{
	// nothing to do
}

//-----------------------------------------------------------------------------
// Name: processListViewItemChanged_Cal()
// Desc: 
//-----------------------------------------------------------------------------
void pgsWin::processListViewItemChanged_Cal(unsigned int rowIndex, unsigned int columnIndex, wildWeasel::guiElemEvFol* guiElem, void* pUser)
{
	// locals
	unsigned int	i, curStep, curSeq;
	wstringstream	wss;

	// stop any animation
	processListViewItemChanged_preFunc();

	// let checkboxes show the used moves
	selCalculation = rowIndex;
	calcCurBlockedMovesFromBlockSeq();
	setCBoxesShowingBlockedMoves(curExcludedMoves, 0, false);

	// acitvate buttons for showing a sequence
	hTextZoneCurStep    .setState(wildWeasel::guiElemState::DRAWED);
	hButtonZonePlay		.setState(wildWeasel::guiElemState::DRAWED);
	hButtonZoneForward  .setState(wildWeasel::guiElemState::DRAWED);
	hButtonZoneBackward .setState(wildWeasel::guiElemState::GRAYED);
	hTextZoneCurStep.setText(L"");

	// Update rows in sequence list
	hListViewSeq.removeAllRows (true);
	hListViewSeq.removeAllItems(true);
	if (seqContainers.size() > selZone && selZone < curZones->numZones && selCalculation < seqContainers[selZone].numCalculations && seqContainers[selZone].calculations[selCalculation].numSequencesInFile > 0) { 
		hListViewSeq.insertRowsAndItems_plainButton2D(0, seqContainers[selZone].calculations[selCalculation].numSequencesInFile, uP.listRowHeight, *uP.buttonImages.buttonImagesListBackground, uP.hFont2D);

		for (i=0; i<seqContainers[selZone].calculations[selCalculation].numSequencesInFile; i++) {
			curSeq = seqContainers[selZone].calculations[selCalculation].sequenceIndexInFile[i];
			wss.str(L""); wss << i;																			hListViewSeq.setItemText(i, 0, 0, wss.str().c_str());
			wss.str(L""); wss << seqContainers[selZone].sequences[curSeq].solPath.numSteps;					hListViewSeq.setItemText(i, 1, 0, wss.str().c_str());
			wss.str(L""); wss << seqContainers[selZone].sequences[curSeq].solPath.path[0];
			for (curStep=1; curStep<seqContainers[selZone].sequences[curSeq].solPath.numSteps; curStep++) {
				wss << "," << seqContainers[selZone].sequences[curSeq].solPath.path[curStep];
			}																								hListViewSeq.setItemText(i, 2, 0, wss.str().c_str());
		}
	}
	hListViewSeq.setTextSize(uP.listViewTextSize, uP.listViewTextSize);
	hListViewSeq.setState(wildWeasel::guiElemState::DRAWED);
	hListViewSeq.alignAllItems();
//	if (!uP.ww->mainLoadingScreen.isActive()) uP.ww->performResourceUpload();
}

//-----------------------------------------------------------------------------
// Name: processListViewItemChanged()
// Desc: Either the passed handle corresponds to 'hListViewCal', then only the 
//       checkboxes are updated to the selected calculation. Or the passed value
//       is 'hListViewZone', then the text fields, the checkboxes and the entire list
//       'hListViewCal' is rebuild according to 'seqContainers[selZone]'.
//       Finally the function updateLabelsAndButtonsOfZoneState() is called.
//-----------------------------------------------------------------------------
void pgsWin::processListViewItemChanged_preFunc()
{
	showingSeqMove					= false;
	solveThread.curShowedStep		= 0;
	autoForward						= false;
	hButtonZonePlay.setText(L"Play");
	uP.timers.timerPlay->terminate();
	uP.functions.resetZoneState(this);
}

//-----------------------------------------------------------------------------
// Name: updateAllLists()
// Desc: 
//-----------------------------------------------------------------------------
void pgsWin::updateAllLists(void* pUser)
{
	// reload seq cont
	loadSequenceContainerFiles(true);

	// update list view items
	updateListViewZoneItems();

	// update zone state
	uP.functions.updateLabelsAndButtonsOfZoneState(this);

	// update edit fields
	setSelItemInListViewZone(selZone);
}

//-----------------------------------------------------------------------------
// Name: updateSingleZone()
// Desc: Call the functions for a selected zone:
//		 - loadSequenceContainerFile()
//		 - updateListViewZoneItems()
//		 - setSelItemInListViewZone()
//-----------------------------------------------------------------------------
void pgsWin::updateSingleZone(unsigned int zone)
{
	loadSequenceContainerFiles(false);
	updateListViewZoneItems();
	setSelItemInListViewZone(zone);
}

//-----------------------------------------------------------------------------
// Name: setSelItemInListViewZone()
// Desc: Prepare the parameters and calls processListViewItemChanged().
//-----------------------------------------------------------------------------
void pgsWin::setSelItemInListViewZone(unsigned int row)
{
	hListViewZone.setFocusOnRow(row);
}

//-----------------------------------------------------------------------------
// Name: insertRowInListViewZone()
// Desc: 
//-----------------------------------------------------------------------------
void pgsWin::insertRowInListViewZone(unsigned int afterRowIndex, unsigned int numRows)
{
	hListViewZone.insertRowsAndItems_plainButton2D(afterRowIndex, numRows, uP.listRowHeight, *uP.buttonImages.buttonImagesListBackground, uP.hFont2D);
	hListViewZone.setItemTextAlignmentHorizontal(wildWeasel::alignmentHorizontal::RIGHT);
	hListViewZone.setTextSize(uP.listViewTextSize, uP.listViewTextSize);
	hListViewZone.alignAllItems();

	// assign function for calculation of sequence concatenation
	for (unsigned int curRow=afterRowIndex; curRow<afterRowIndex+numRows; curRow++) {
		auto curListItem = hListViewZone.getItem(curRow, concThread.columnIndex)->getGuiElemPointer()->items.front()->getPointer<wildWeasel::plainButton2D>();
		curListItem->assignOnLeftMouseButtonReleased(bind(&pgsWin::buttonFuncSeqConcatenate, this, placeholders::_1, placeholders::_2), (void*) curRow);
	}
}

//-----------------------------------------------------------------------------
// Name: updateListViewZoneItems()
// Desc: Updates the items of the list 'hListViewZone' according to the classes 
//       'curZones' and 'seqContainers' for each zone. 
//       Enables the buttons if the number of zones is >0.
//-----------------------------------------------------------------------------
void pgsWin::updateListViewZoneItems()
{
	// locals
	unsigned int i, maxNumSeq;
	WCHAR			strTmp[100];

	// update listview items
	for (i=0; i<curZones->numZones; i++) {

		// calc maximum number of possible sequences
		if (uP.functions.calcMaxNumSequences != nullptr) {
			uP.functions.calcMaxNumSequences(this, maxNumSeq, i);
		} else {
			maxNumSeq = curZones->calcMaxNumSequences(i);
		}

		wsprintf(strTmp, L"%d", i);													hListViewZone.setItemText(i, 0, 0, strTmp);
		wsprintf(strTmp, L"%d", curZones->zoneProp[i].numRectangles);				hListViewZone.setItemText(i, 1, 0, strTmp);
		wsprintf(strTmp, L"%d", seqContainers[i].numCalculations);					hListViewZone.setItemText(i, 2, 0, strTmp);
		wsprintf(strTmp, L"%d", seqContainers[i].totalCalculationTime / 60000);		hListViewZone.setItemText(i, 3, 0, strTmp);
		wsprintf(strTmp, L"%d", seqContainers[i].numSequences);						hListViewZone.setItemText(i, 4, 0, strTmp);
		wsprintf(strTmp, L"");														hListViewZone.setItemText(i, 5, 0, strTmp);
		wsprintf(strTmp, L"%d", maxNumSeq);											hListViewZone.setItemText(i, 6, 0, strTmp);
	}

	// terminate thread
	concThread.stop();

	if (showingZones) {
		hButtonZoneMoveUp			.setState(curZones->numZones!=0 ? wildWeasel::guiElemState::DRAWED : wildWeasel::guiElemState::GRAYED);
		hButtonZoneMoveDown			.setState(curZones->numZones!=0 ? wildWeasel::guiElemState::DRAWED : wildWeasel::guiElemState::GRAYED);
		hButtonZoneFindSequences	.setState(curZones->numZones!=0 ? wildWeasel::guiElemState::DRAWED : wildWeasel::guiElemState::GRAYED);
		hButtonZoneSeqConcatenate  ->setState(curZones->numZones!=0 ? wildWeasel::guiElemState::DRAWED : wildWeasel::guiElemState::GRAYED);
		hButtonZoneDelete			.setState(curZones->numZones!=0 ? wildWeasel::guiElemState::DRAWED : wildWeasel::guiElemState::GRAYED);
		hButtonZoneSave				.setState(curZones->numZones!=0 ? wildWeasel::guiElemState::DRAWED : wildWeasel::guiElemState::GRAYED);
	} else {
		hListViewZone.setState(wildWeasel::guiElemState::HIDDEN);
	}
}

#pragma region sequence concatenation
//-----------------------------------------------------------------------------
// Name: buttonFuncSeqConcatenate()
// Desc: Function called when the button 'calc' is pressed.
//-----------------------------------------------------------------------------
void pgsWin::buttonFuncSeqConcatenate(wildWeasel::guiElemEvFol* elem, void* pUser)
{
	// locals
	unsigned int zoneToCalculate;

	// stop calculation
	if (concThread.isRunning()) {
		concThread.stop();
	}
	
	// check database directory
	if (!PathIsDirectory(uP.strDatabaseDir.c_str())) {
		uP.ww->showMessageBox(L"ERROR", L"Selected database directory doesn't exist.\nPlease chose a vaid directory first.", MB_OK);
		return;
	}

	// is there any zone?
	if (curZones->numZones == 0) {
		uP.ww->showMessageBox(L"ERROR", L"There is no single zone existend.", MB_OK);
		return;
	}

	// let it call an update every half second automatically
	concThread.timerUpdateResultConc.terminate();
	concThread.timerUpdateResultConc.start(uP.ww, pgsWinThreadConcatenateSequences::updateResult, &concThread, 500);
	concThread.hListViewZone				= &hListViewZone;
	concThread.uP							= &uP;

	// calc all if column header was pressed
	if (elem == hButtonZoneSeqConcatenate) {
		zoneToCalculate = curZones->numZones;
		for (unsigned int curZoneId=0; curZoneId < curZones->numZones; curZoneId++) {
			concThread.hListViewZone->setItemText(curZoneId, concThread.columnIndex, 0, L"");
		}
	// otherwise only the one of the selected zone
	} else {
		zoneToCalculate = (unsigned int) pUser;		// ... there should be a better way
		concThread.hListViewZone->setItemText(zoneToCalculate, concThread.columnIndex, 0, L"");
	}

	// show in hListViewActCal and replace old row if any already exists
	quitFindingSeq((listViewActCalItem*) &concThread);
	concThread.create(&concThread, this, uP.ww, &hListViewActCal, uP.listViewTextSize, uP.listRowHeight, *uP.buttonImages.buttonImagesListBackground, uP.hFont2D);
	findSeqThreads.push_back(&concThread);

	// start thread
	concThread.start(&concThread.sharedVarsMutex, &concThread.pauseMutex, nullptr, &concThread.eventThreadFinished, curZones, zoneToCalculate, uP.strDatabaseDir, uP.strDatabaseFileType, moveIndicesMap, reverseMoves);
}

//-----------------------------------------------------------------------------
// Name: updateResult()
// Desc: 
//-----------------------------------------------------------------------------
void pgsWin::pgsWinThreadConcatenateSequences::updateResult(void* pUser)
{
	// locals
	auto			pgsTs		= (pgsWinThreadConcatenateSequences*) pUser;
	wstringstream	wss;
	unsigned int	curZoneId;

	// lock variables
	auto sv = pgsTs->lockThreadVariables();
	if (!sv) return;

	// update gui
	for (curZoneId = 0; curZoneId < sv->solutionsThroughConcatenation.size(); curZoneId++) {
		if (sv->solutionsThroughConcatenation[curZoneId] == 0) continue;
		wss.str(L""); wss << sv->solutionsThroughConcatenation[curZoneId];
		pgsTs->hListViewZone->setItemText(curZoneId, pgsTs->columnIndex, 1, wss.str().c_str());
	}

	// unlock
	pgsTs->unlockThreadVariables();
}

//-----------------------------------------------------------------------------
// Name: threadHasFinished()
// Desc: 
//-----------------------------------------------------------------------------
void pgsWin::pgsWinThreadConcatenateSequences::threadHasFinished()
{
	// callback when thread has finished
}
#pragma endregion

#pragma region find sequences
//-----------------------------------------------------------------------------
// Name: buttonFuncFindSequences()
// Desc: Function called when the button 'find sequences' is pressed.
//-----------------------------------------------------------------------------
void pgsWin::buttonFuncFindSequences()
{
	// param ok?
	if (!moveIndicesMap.size()) return;
	if (!reverseMoves.size())	return;

	// check database directory
	if (!PathIsDirectory(uP.strDatabaseDir.c_str())) {
		uP.ww->showMessageBox(L"ERROR", L"Selected database directory doesn't exist.\nPlease chose a vaid directory first.", MB_OK);
		return;
	}

	// is there any zone?
	if (curZones->numZones == 0) {
		uP.ww->showMessageBox(L"ERROR", L"There is no single zone existend.", MB_OK);
		return;
	}

	// create new "findSeqCalculation" object
	pgsWinThreadFindSeq*  newItem	= new pgsWinThreadFindSeq(this, uP.ww, curZones, selZone, uP.strDatabaseDir, uP.strDatabaseFileType, moveIndicesMap, reverseMoves, &hListViewActCal, uP.listViewTextSize, uP.listRowHeight, *uP.buttonImages.buttonImagesListBackground, uP.hFont2D, uP.maxFoundSolutions);
	findSeqThreads.push_back(newItem);
}

//-----------------------------------------------------------------------------
// Name: pgsWinThreadFindSeq()
// Desc: 
//-----------------------------------------------------------------------------
pgsWin::pgsWinThreadFindSeq::pgsWinThreadFindSeq(pgsWin* pgsW, wildWeasel *ww, zoneClass* curZones, unsigned int selZone, wstring& databaseDir, wstring& databaseFileType, vector<unsigned int>& moveIndicesMap, vector<unsigned int>& reverseMoves, wildWeasel::listView2D* pList, float listViewTextSize, unsigned int listRowHeight, wildWeasel::buttonImageFiles& buttonImages, wildWeasel::font2D* font, unsigned int maxFoundSolutions)
{
	// create new "find sequence class" object
	listViewActCalItem		::create(this, pgsW, ww, pList, listViewTextSize, listRowHeight, buttonImages, font);
	pgsThreadFindSequences	::start	(&sharedVarsMutex, &pauseMutex, nullptr, &eventThreadFinished, curZones, selZone, databaseDir, databaseFileType, moveIndicesMap, reverseMoves, maxFoundSolutions);
}

//-----------------------------------------------------------------------------
// Name: pgsWinThreadFindSeq()
// Desc: 
//-----------------------------------------------------------------------------
pgsWin::pgsWinThreadFindSeq::~pgsWinThreadFindSeq()
{
}

//-----------------------------------------------------------------------------
// Name: create()
// Desc: 
//-----------------------------------------------------------------------------
void pgsWin::listViewActCalItem::create(
	pgsThread*					pgsT, 
	pgsWin*						pgsW,
	wildWeasel *			ww,
	wildWeasel::listView2D*	pList,
	float						listViewTextSize,
	unsigned int				listRowHeight,
	wildWeasel::buttonImageFiles& buttonImages,
	wildWeasel::font2D*		font)
{
	// create event notifying that the thread has finished
	eventThreadFinished	.create(ww);	eventThreadFinished	.setFunc(bind(&pgsWin::finishedFindingSeq, pgsW, placeholders::_1), this, true);
	eventRemoveRow		.create(ww);	eventRemoveRow		.setFunc(bind(&pgsWin::quitFindingSeq,     pgsW, placeholders::_1), this, true);

	// add new list view item
	pThread		= pgsT;
	pListView	= pList;
	pListView->insertRowsAndItems_plainButton2D(pListView->getNumRows(), 1, listRowHeight, buttonImages, font);

	// assign button functionsto pause & quit button
	rowIndex	= pListView->getNumRows() - 1;
	pauseButton = (wildWeasel::plainButton2D*) pListView->getItem(rowIndex, 11)->getGuiElemPointer()->items.front();
	stopButton  = (wildWeasel::plainButton2D*) pListView->getItem(rowIndex, 12)->getGuiElemPointer()->items.front();
	pauseButton->assignOnLeftMouseButtonPressed(bind(&listViewActCalItem::pauseFindSeqCalculation, this, placeholders::_1, placeholders::_2), this);
	stopButton ->assignOnLeftMouseButtonPressed(bind(&listViewActCalItem::stopFindSeqCalculation,  this, placeholders::_1, placeholders::_2), this);
	pauseButton->setText(L"Pause");
	stopButton ->setText(L"Stop");

	// finish list view
	pListView->setSelectionMode(wildWeasel::listView2D::selectionMode::NONE);
	pListView->setTextSize(listViewTextSize, listViewTextSize);
	pListView->setState(wildWeasel::guiElemState::DRAWED);
	pListView->alignAllItems();
//	if (!ww->mainLoadingScreen.isActive()) ww->performResourceUpload();

	// let it call an update every half second automatically
	timerUpdateResult.start(ww, listViewActCalItem::updateListViewCalculations, this, 500);
}

//-----------------------------------------------------------------------------
// Name: updateListViewCalculations()
// Desc: 
//-----------------------------------------------------------------------------
void pgsWin::listViewActCalItem::updateListViewCalculations(void* pUser)
{
	// locals
	wstringstream				wss;
	listViewActCalItem*			calcItem	= (listViewActCalItem*) pUser;

	// lock solution variables
	auto						sv		= calcItem->pThread->lockThreadVariables();
	if (!sv) return;

	// update gui
	wss.str(L""); wss << sv->zoneNumber;											calcItem->pListView->setItemText(calcItem->rowIndex,  0, 0, wss.str().c_str());
	wss.str(L""); wss << sv->maxSearchDepth;										calcItem->pListView->setItemText(calcItem->rowIndex,  1, 0, wss.str().c_str());
	wss.str(L""); wss << sv->numPossibleMoves;										calcItem->pListView->setItemText(calcItem->rowIndex,  2, 0, wss.str().c_str());
	wss.str(L""); wss << sv->numSolutionsFound;										calcItem->pListView->setItemText(calcItem->rowIndex,  3, 0, wss.str().c_str());
	wss.str(L""); wss << sv->numRectangles;											calcItem->pListView->setItemText(calcItem->rowIndex,  4, 0, wss.str().c_str());
	wss.str(L""); wss << sv->useDoubleFeature;										calcItem->pListView->setItemText(calcItem->rowIndex,  5, 0, wss.str().c_str());
	wss.str(L""); wss << sv->useSingleReturn;										calcItem->pListView->setItemText(calcItem->rowIndex,  6, 0, wss.str().c_str());
	wss.str(L""); wss << sv->timeElapsed;											calcItem->pListView->setItemText(calcItem->rowIndex,  7, 0, wss.str().c_str());
	wss.str(L""); wss << sv->curBranch[0] + 1 /*<< L"/" << sv->numPossibleMoves*/;	calcItem->pListView->setItemText(calcItem->rowIndex,  8, 0, wss.str().c_str());
	wss.str(L""); wss << sv->curBranch[1] + 1 /*<< L"/" << sv->numPossibleMoves*/;	calcItem->pListView->setItemText(calcItem->rowIndex,  9, 0, wss.str().c_str());
	wss.str(L""); wss << sv->curBranch[2] + 1 /*<< L"/" << sv->numPossibleMoves*/;	calcItem->pListView->setItemText(calcItem->rowIndex, 10, 0, wss.str().c_str());

	calcItem->pThread->unlockThreadVariables();
}

//-----------------------------------------------------------------------------
// Name: pauseFindSeqCalculation()
// Desc: 
//-----------------------------------------------------------------------------
void pgsWin::listViewActCalItem::pauseFindSeqCalculation(wildWeasel::guiElemEvFol* elem, void* pUser)
{
	// locals
	listViewActCalItem*	calcItem = (listViewActCalItem*) pUser;

	// pause
	if (!calcItem->pThread->isPaused()) {
		calcItem->pListView->setRowColor(calcItem->rowIndex, wildWeasel::color::gray);
		calcItem->pauseButton->setText(L"Resume");

	// resume
	} else {
		calcItem->pListView->setRowColor(calcItem->rowIndex, wildWeasel::color::white);
		calcItem->pauseButton->setText(L"Pause");
	}

	calcItem->pThread->togglePauseMode();
}

//-----------------------------------------------------------------------------
// Name: stopFindSeqCalculation()
// Desc: 
//-----------------------------------------------------------------------------
void pgsWin::listViewActCalItem::stopFindSeqCalculation(wildWeasel::guiElemEvFol* elem, void* pUser)
{
	// locals
	listViewActCalItem*	calcItem = (listViewActCalItem*) pUser;
	wstring curText;

	// stop
	calcItem->stopButton->getText(curText);
	if (curText == L"Stop") {
		// button was not pressed by user, so the calculation has finished
		if (elem == nullptr) {
			calcItem->pListView->setRowColor(calcItem->rowIndex, wildWeasel::color::green);
		// calculation was aborted by user
		} else {
			calcItem->pListView->setRowColor(calcItem->rowIndex, wildWeasel::color::red);
		}
		calcItem->pauseButton->assignOnLeftMouseButtonPressed(nullptr, this);
		calcItem->pauseButton->setText(L"");
		calcItem->stopButton->setText(L"Quit");
		calcItem->pThread->stop();
	// quit
	} else {

		// delete list view item
		calcItem->stopButton->setState(wildWeasel::guiElemState::GRAYED);
		calcItem->timerUpdateResult.terminate();

		// inforn parent class that object can be deleted
		calcItem->eventRemoveRow.set();
	}
}

//-----------------------------------------------------------------------------
// Name: finishedFindingSeq()
// Desc: 
//-----------------------------------------------------------------------------
void pgsWin::finishedFindingSeq(void* pUser)
{
	// locals
	listViewActCalItem*	calcItem = (listViewActCalItem*) pUser;

	// remove line in list view 
	calcItem->stopFindSeqCalculation(nullptr, calcItem);

	// reload sequence files. the new calculated sequence should be stored in the files.
	if (calcItem != &concThread) {
		loadSequenceContainerFiles(true);
		updateListViewZoneItems();
		setSelItemInListViewZone(selZone);
	} else {
		pgsWinThreadConcatenateSequences::updateResult(&concThread);
	}
}

//-----------------------------------------------------------------------------
// Name: quitFindingSeq()
// Desc: 
//-----------------------------------------------------------------------------
void pgsWin::quitFindingSeq(void* pUser)
{
	// locals
	listViewActCalItem*	calcItem	= (listViewActCalItem*) pUser;
	auto				calcItemItr	= find(findSeqThreads.begin(), findSeqThreads.end(), calcItem);

	// skip if passed calculation is not in list
	if (calcItemItr == findSeqThreads.end()) return;

	// update rowIndex of all subsequent calculations
	for (calcItemItr++; calcItemItr != findSeqThreads.end(); calcItemItr++) {
		(*calcItemItr)->rowIndex--;
	}
	calcItem->pListView->removeAllItemsInRow(calcItem->rowIndex, true);
	calcItem->pListView->removeRow(calcItem->rowIndex, true);
	calcItem->pListView->alignAllItems();
	findSeqThreads.remove(calcItem);

	// do not delete concThread, since it is a member of pgsWin
	if (calcItem != &concThread) {
		delete calcItem;
	}
}
#pragma endregion

//-----------------------------------------------------------------------------
// Name: showZoneControls()
// Desc: 
//-----------------------------------------------------------------------------
void pgsWin::showZoneControls(bool visible)
{
	// locals
	wildWeasel::guiElemState	nCmdShow = visible ? wildWeasel::guiElemState::DRAWED : wildWeasel::guiElemState::HIDDEN;

	// update state
	showingZones		= visible;
	showingSeqMove		= false;

	// call external sub function
	uP.functions.showZoneControlsSubfunc(this, visible);

	// terminate thread
	concThread.stop();

	// render rect
	if (visible) {
		loadSequenceContainerFiles(true);

		// select first row
		if (curZones->numZones > 0) {
			selZone		= 0;
			updateListViewZoneItems();
			setSelItemInListViewZone(selZone);
		}
	} else if (!visible) {
		unloadSequenceContainerFiles();
	}

	hBorderLineZone				.setState(nCmdShow);
	hBorderLineCal				.setState(nCmdShow);
	hBorderLineSeq				.setState(nCmdShow);
	hBorderLineActCal			.setState(nCmdShow);
	hBorderLineBlocked			.setState(nCmdShow);

	hListViewZone				.setState(nCmdShow);
	hListViewSeq				.setState(nCmdShow);
	hListViewCal				.setState(nCmdShow);
	hListViewActCal				.setState(nCmdShow);
								
	hButtonZonePlay				.setState(visible ? wildWeasel::guiElemState::GRAYED : wildWeasel::guiElemState::HIDDEN);
	hButtonZoneForward			.setState(visible ? wildWeasel::guiElemState::GRAYED : wildWeasel::guiElemState::HIDDEN);
	hButtonZoneBackward			.setState(visible ? wildWeasel::guiElemState::GRAYED : wildWeasel::guiElemState::HIDDEN);
	hTextZoneCurStep			.setState(visible ? wildWeasel::guiElemState::GRAYED : wildWeasel::guiElemState::HIDDEN);
								
	hButtonZoneLoad				.setState(nCmdShow);
	hButtonZoneSave				.setState(nCmdShow);
	hButtonZoneQuit				.setState(nCmdShow);
	hButtonZoneInsert			.setState(nCmdShow);
	hButtonZoneDelete			.setState(nCmdShow);
	hButtonZoneMoveUp			.setState(nCmdShow);
	hButtonZoneMoveDown			.setState(nCmdShow);
	hButtonZoneFindSequences	.setState(nCmdShow);
	
	hButtonZoneMoveUp			.setState(visible ? (curZones->numZones!=0 ? wildWeasel::guiElemState::DRAWED : wildWeasel::guiElemState::GRAYED) : wildWeasel::guiElemState::HIDDEN);
	hButtonZoneMoveDown			.setState(visible ? (curZones->numZones!=0 ? wildWeasel::guiElemState::DRAWED : wildWeasel::guiElemState::GRAYED) : wildWeasel::guiElemState::HIDDEN);
	hButtonZoneFindSequences	.setState(visible ? (curZones->numZones!=0 ? wildWeasel::guiElemState::DRAWED : wildWeasel::guiElemState::GRAYED) : wildWeasel::guiElemState::HIDDEN);
	hButtonZoneSeqConcatenate  ->setState(visible ? (curZones->numZones!=0 ? wildWeasel::guiElemState::DRAWED : wildWeasel::guiElemState::GRAYED) : wildWeasel::guiElemState::HIDDEN);
	hButtonZoneDelete			.setState(visible ? (curZones->numZones!=0 ? wildWeasel::guiElemState::DRAWED : wildWeasel::guiElemState::GRAYED) : wildWeasel::guiElemState::HIDDEN);
	hButtonZoneSave				.setState(visible ? (curZones->numZones!=0 ? wildWeasel::guiElemState::DRAWED : wildWeasel::guiElemState::GRAYED) : wildWeasel::guiElemState::HIDDEN);

	hEditOverwriteDatabase		.setState(nCmdShow);		hTextOverwriteDatabase		.setTextState(nCmdShow);
	hEditBackTrackingDepth		.setState(nCmdShow);		hTextBackTrackingDepth		.setTextState(nCmdShow);
	hEditDoubleFeature			.setState(nCmdShow);		hTextDoubleFeature			.setTextState(nCmdShow);
	hEditUseSingleReturn		.setState(nCmdShow);		hTextUseSingleReturn		.setTextState(nCmdShow);
	hEditMaxSeqLength			.setState(nCmdShow);		hTextMaxSeqLength			.setTextState(nCmdShow);
	hEditSearchDepthUsingSeq	.setState(nCmdShow);		hTextSearchDepthUsingSeq	.setTextState(nCmdShow);
	
//	// blocked moves controls
//	unsigned int	uniqueMoveId;
//	if (!uP.cbBlockedMovesByUser && hCheckBoxBlockedMoves!=NULL) {
//		for (uniqueMoveId=0; uniqueMoveId<numUniqueMoves; uniqueMoveId++) {
//			ShowWindow(hCheckBoxBlockedMoves[uniqueMoveId],	nCmdShow);	
//		}
//	}
	hTextBlockedMoves			.setTextState((uP.cbBlockedMovesByUser || hCheckBoxBlockedMoves.size() == 0) ? wildWeasel::guiElemState::HIDDEN : nCmdShow);
}

//-----------------------------------------------------------------------------
// Name: buttonBlockedMovesChanged()
// Desc: Called when a checkbox is being pressed by the user.
//       Update the array 'curZones->zoneProp[selZone].excludedMove[numMoves]'.
//-----------------------------------------------------------------------------
void pgsWin::buttonBlockedMovesChanged(unsigned int uniqueMoveId)
{
	// parameters ok?
	if (duplicatesMapBlockedMoves.empty()) return;
	if (curZones						== NULL) return;
//	if (hCheckBoxBlockedMoves			== NULL) return;
	
	// locals
//	unsigned int newBst	= SendMessage(hCheckBoxBlockedMoves[uniqueMoveId], BM_GETCHECK, 0, 0);
//	unsigned int newVal = ((newBst == BST_CHECKED) ? 0 : 1);
//
//	if (curZones->numZones > 0 && selZone < curZones->numZones) {
//		// when calculation is selected than undo change
//		if (selCalculation < seqContainers[selZone].numCalculations && (seqContainers[selZone].calculations[selCalculation].numSequencesInFile == 0 || ListView_GetItemCount(hListViewSeq) > 0)) {
//			newBst = (newBst == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
//			SendMessage(hCheckBoxBlockedMoves[uniqueMoveId], BM_SETCHECK, newBst, 0);
//		// a zone is selected, so update curZones->zoneProp[selZone].excludedMove[]
//		} else {
//			curZones->zoneProp[selZone].excludedMove[duplicatesMapBlockedMoves[uniqueMoveId]] = newVal;
//			SetFocus(hListViewZone);
//		}
//	}
}

//-----------------------------------------------------------------------------
// Name: calcCurBlockedMovesFromBlockSeq()
// Desc: calc 'blockedMoves[numMoves]' for setCBoxesShowingBlockedMoves()
//       from seqBlock[numUniqueMoves*numUniqueMoves]
// seqBlock[A*numMoves+B]: When move A is performed then B is not allowed when the value is PGS_SB_BLOCKED,
//						   is allowed when the value is PGS_SB_RELEASE and not changed when the value is PGS_SB_NEUTRAL.
//-----------------------------------------------------------------------------
void pgsWin::calcCurBlockedMovesFromBlockSeq()
{
	// locals
	unsigned int curMove, curUniqueMove, curStepWithinSeq, curSeqOfCal, curSeq;

	// redim array
	curExcludedMoves.resize(curZones->numMoves, 1);

	// process each sequnce calculated by this calculation
	for (curSeqOfCal=0; curSeqOfCal<seqContainers[selZone].calculations[selCalculation].numSequencesInFile; curSeqOfCal++) {
		curSeq = seqContainers[selZone].calculations[selCalculation].sequenceIndexInFile[curSeqOfCal];

		// process each step of sequence
		for (curStepWithinSeq=0; curStepWithinSeq<seqContainers[selZone].sequences[curSeq].solPath.numSteps; curStepWithinSeq++) {
			curUniqueMove = seqContainers[selZone].sequences[curSeq].solPath.path[curStepWithinSeq];

			// Mark this unique move as used/not excluded/not blocked
			for (curMove=0; curMove<curZones->numMoves; curMove++) {
				if (curUniqueMove == invDuplicatesMapBlockedMoves[curMove]) {
					curExcludedMoves[curMove] = 0;
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Name: setCBoxesShowingBlockedMoves()
// Desc: Sets the value of every blocked move checkbox corresponding 
//       to the passed array 'blockedMoves[numMoves]'.
//-----------------------------------------------------------------------------
void pgsWin::setCBoxesShowingBlockedMoves(vectui& blockedMoves, unsigned int checkedValue, bool enableCheckBoxes)
{
	// locals
//	unsigned int uniqueMoveId;

	// user controlled?
	if (uP.cbBlockedMovesByUser) {
		uP.functions.setCBoxesBlockedMovesUser(this, blockedMoves, checkedValue, enableCheckBoxes);

	// automode
	} else {
		// parameters ok?
//		if (hCheckBoxBlockedMoves			== NULL) return;
//		if (duplicatesMapBlockedMoves		== NULL) return;

//		for (uniqueMoveId=0; uniqueMoveId<numUniqueMoves; uniqueMoveId++) {
//			if (curZones->numZones > 0) {
//				SendMessage(hCheckBoxBlockedMoves[uniqueMoveId], BM_SETCHECK, (blockedMoves[duplicatesMapBlockedMoves[uniqueMoveId]]==checkedValue)?BST_CHECKED:BST_UNCHECKED, 0);
//			} else {
//				SendMessage(hCheckBoxBlockedMoves[uniqueMoveId], BM_SETCHECK, BST_UNCHECKED, 0);
//			}
//			EnableWindow(hCheckBoxBlockedMoves[uniqueMoveId], enableCheckBoxes);
//		}
	}
}

//-----------------------------------------------------------------------------
// Name: calcCurSeq()
// Desc: return 'curSeq' based on the selected zone, selected calculation and selected sequence.
//-----------------------------------------------------------------------------
bool pgsWin::calcCurSeq(unsigned int &curSeq)
{
	// locals
	unsigned int mySeq;

	if (duplicatesMapBlockedMoves.empty())															return false;
	if (curZones		== NULL)																	return false;
	if (seqContainers	.empty())																	return false;
	if (selZone			>= curZones->numZones)														return false;
	if (selCalculation	>= seqContainers[selZone].numCalculations)									return false;
	if (selSequence		>= seqContainers[selZone].calculations[selCalculation].numSequencesInFile)	return false;

	mySeq = seqContainers[selZone].calculations[selCalculation].sequenceIndexInFile[selSequence];

	if (mySeq			>= seqContainers[selZone].numSequences)										return false;

	curSeq = mySeq;

	return true;
}

//-----------------------------------------------------------------------------
// Name: buttonFuncZonePlay()
// Desc: let the zone state demonstrate the current selected sequence
//-----------------------------------------------------------------------------
void pgsWin::buttonFuncZonePlay()
{
	// locals
	wstring			str;
	wstringstream	ss;
	unsigned int	curSeq;

	if (!calcCurSeq(curSeq)) return;
	hButtonZonePlay.getText(str);
	
	if (autoForward) {
		hButtonZonePlay.setText(L"Play");
		uP.timers.timerPlay->terminate();
		autoForward = false;
	} else {
		if (str.compare(L"Restart") == 0) {
			curShowedStep	 = 0;
			showingSeqMove	 = true;
			ss.str(L""); ss << L"step: " << curShowedStep << L"/" << seqContainers[selZone].sequences[curSeq].solPath.numSteps;
			hTextZoneCurStep.setText(ss.str());
			hButtonZonePlay.setText(L"Play");
			hButtonZoneForward.setState(wildWeasel::guiElemState::DRAWED);
			hButtonZoneBackward.setState(wildWeasel::guiElemState::GRAYED);
			uP.functions.zoneDoMoveAndShow(0, true);
		} else {
			hButtonZonePlay.setText(L"Pause");
			uP.timers.timerPlay->start(solutionAnimationSleepDuration);
			autoForward = true;
		}
	}
}

//-----------------------------------------------------------------------------
// Name: buttonFuncTestZone()
// Desc: let the zone state demonstrate the current selected sequence (but just one step)
//-----------------------------------------------------------------------------
void pgsWin::buttonFuncZoneForward()
{
	// locals
	wstringstream ss;
	unsigned int curSeq, curUniqueMoveId, curMoveId;

	if (!calcCurSeq(curSeq)) return;

	if (curShowedStep >= seqContainers[selZone].sequences[curSeq].solPath.numSteps - 1) {
		hButtonZonePlay.setText(L"Restart");
		uP.timers.timerPlay->terminate();
		autoForward = false;
		hButtonZoneForward.setState(wildWeasel::guiElemState::GRAYED);
	} 

	hButtonZoneBackward.setState(wildWeasel::guiElemState::DRAWED);
	curUniqueMoveId	= seqContainers[selZone].sequences[curSeq].solPath.path[curShowedStep];
	curMoveId		= duplicatesMapBlockedMoves[curUniqueMoveId];
	uP.functions.zoneDoMoveAndShow(curMoveId, false);
	curShowedStep++;
	ss.str(L""); ss << L"step: " << curShowedStep << L"/" << seqContainers[selZone].sequences[curSeq].solPath.numSteps;					
	hTextZoneCurStep.setText(ss.str());
}

//-----------------------------------------------------------------------------
// Name: buttonFuncTestZone()
// Desc: 
//-----------------------------------------------------------------------------
void pgsWin::buttonFuncZoneBackward()
{
	// locals
	wstringstream ss;
	unsigned int curSeq, curUniqueMoveId, curMoveId;

	if (!calcCurSeq(curSeq)) return;

	// if there a further step ?
	if (0 == curShowedStep) return;
	hButtonZoneForward.setState(wildWeasel::guiElemState::DRAWED);
	hButtonZonePlay.setText(L"Play");
	uP.timers.timerPlay->terminate();
	autoForward = false;
	curShowedStep--;

	if (curShowedStep == 0) hButtonZoneBackward.setState(wildWeasel::guiElemState::GRAYED);

	// current move
	curUniqueMoveId	= seqContainers[selZone].sequences[curSeq].solPath.path[curShowedStep];
	curMoveId		= duplicatesMapBlockedMoves[reverseUniqueMoves[curUniqueMoveId]];

	ss.str(L""); ss << L"step: " << curShowedStep << L"/" << seqContainers[selZone].sequences[curSeq].solPath.numSteps;					
	hTextZoneCurStep.setText(ss.str());

	uP.functions.zoneDoMoveAndShow(curMoveId, false);
}

//-----------------------------------------------------------------------------
// Name: buttonFuncTestZone()
// Desc: 
//-----------------------------------------------------------------------------
void pgsWin::buttonFuncTestZone()
{
}
#pragma endregion

/************************************************************************************************************************************\
  solving mode
\************************************************************************************************************************************/

#pragma region solvingMode
//-----------------------------------------------------------------------------
// Name: showSolveControls()
// Desc: 
//-----------------------------------------------------------------------------
bool pgsWin::showSolveControls(bool visible)
{
	// locals
	wildWeasel::guiElemState	nCmdShow = visible ? wildWeasel::guiElemState::DRAWED : wildWeasel::guiElemState::HIDDEN;

	solveThread.hTextStepWithinZone				.setState(nCmdShow);
	solveThread.hTextStepInTotal				.setState(nCmdShow);
	solveThread.hTextShowingZone				.setState(nCmdShow);
	solveThread.hTextZonesSolved				.setState(nCmdShow);
	solveThread.hTextTopBranch					.setState(nCmdShow);
	solveThread.hButtonSolvePlay				.setState(nCmdShow);
	solveThread.hButtonSolvePause				.setState(nCmdShow);
	solveThread.hButtonSolveForward				.setState(nCmdShow);
	solveThread.hButtonSolveBackward			.setState(nCmdShow);
	solveThread.hButtonSolveStopCalc			.setState(nCmdShow);
	solveThread.hButtonSolveShowStatistics		.setState(solveThread.solvingRandomStates?nCmdShow:wildWeasel::guiElemState::HIDDEN);
	solveThread.hButtonSolveFaster				.setState(nCmdShow);
	solveThread.hButtonSolveSlower				.setState(nCmdShow);

	if (solveThread.solvingRandomStates) {
		solveThread.hButtonSolvePlay			.setState(visible ? wildWeasel::guiElemState::GRAYED : wildWeasel::guiElemState::HIDDEN);
		solveThread.hButtonSolvePause			.setState(visible ? wildWeasel::guiElemState::GRAYED : wildWeasel::guiElemState::HIDDEN);
		solveThread.hButtonSolveForward   		.setState(visible ? wildWeasel::guiElemState::GRAYED : wildWeasel::guiElemState::HIDDEN);
		solveThread.hButtonSolveBackward		.setState(visible ? wildWeasel::guiElemState::GRAYED : wildWeasel::guiElemState::HIDDEN);
		solveThread.hButtonSolveFaster    		.setState(visible ? wildWeasel::guiElemState::GRAYED : wildWeasel::guiElemState::HIDDEN);
		solveThread.hButtonSolveSlower			.setState(visible ? wildWeasel::guiElemState::GRAYED : wildWeasel::guiElemState::HIDDEN);
		solveThread.hTextRndStatesSolved 		.setState(visible ? nCmdShow							 : wildWeasel::guiElemState::HIDDEN);
		solveThread.hTextAnimationSpeed 		.setState(visible ? wildWeasel::guiElemState::HIDDEN : wildWeasel::guiElemState::HIDDEN);
	} else {
		solveThread.hTextAnimationSpeed			.setState(nCmdShow);
		solveThread.hTextRndStatesSolved 		.setState(wildWeasel::guiElemState::HIDDEN);
	}

	// stop thread and timer
	if (visible == false) {
		uP.timers.timerPlay->terminate();
		solveThread.stop();
	}
	return true;
}

//-----------------------------------------------------------------------------
// Name: solveState()
// Desc: 
//-----------------------------------------------------------------------------
bool pgsWin::solveState(vector<unsigned int>* stateToSolve, 
						const vector<unsigned int>& initialState,
						const vector<unsigned int>& finalState,
						vector<unsigned int>& reverseMoves, 
						void doMoveAndShow	(unsigned int moveId), 
						void doMove			(unsigned int moveId), 
						void setInitialState(),
						bool doesAllFieldsBelongToZone(),
						void setRandomState ())
{
	// locals
	wstringstream	ss;
	unsigned int	i;

	// check if initial und final states are set correct
	if (initialState.empty()) {
		uP.ww->showMessageBox(L"ERROR", L"Set initial state first!", MB_OK);
		return false;
	}
	if (finalState.empty()) {
		uP.ww->showMessageBox(L"ERROR", L"Set final state first!", MB_OK);
		return false;
	}

	// check database directory
	if (!PathIsDirectory(uP.strDatabaseDir.c_str())) {
		uP.ww->showMessageBox(L"ERROR", L"Selected database directory doesn't exist.\nPlease chose a vaid directory first.", MB_OK);
		return false;
	}
	
	// check if sequences are correct
	solver->setFieldSize(curZones->numFields, uP.strDatabaseDir, uP.strDatabaseFileType);
	solver->setInitialState(initialState);
	solver->setFinalState  (finalState);

	loadSequenceContainerFiles(true);
	if (curZones->numZones==0) {
		uP.ww->showMessageBox(L"ERROR", L"No zones have been defined so far.\nPlease define zones first.", MB_OK);
		unloadSequenceContainerFiles();
		return false;
	}
	if (!doesAllFieldsBelongToZone()) {
		uP.ww->showMessageBox(L"ERROR", L"Not all fields does belong to a zone.\nPlease define sufficient zones first.", MB_OK);
		unloadSequenceContainerFiles();
		return false;
	}
	for (i=0; i<curZones->numZones; i++) {
		if (seqContainers[i].numSequences == 0) {
			uP.ww->showMessageBox(L"ERROR", L"Some sequence files does not exist or does not contain sequences.\nPlease calculate sequences first.", MB_OK);
			unloadSequenceContainerFiles();
			return false;
		}
	}
	unloadSequenceContainerFiles();

	// init solver
	solveThread.doMove						= doMove;
	solveThread.doMoveAndShow				= doMoveAndShow;
	solveThread.setInitialState				= setInitialState;
	solveThread.doesAllFieldsBelongToZone	= doesAllFieldsBelongToZone;
	solveThread.setRandomState				= setRandomState;
	solveThread.curShowedStep				= 0;
	solveThread.curShowedZone				= 0;
	solveThread.curStepWithInZone			= 0;
	solveThread.numStepOfCurZone			= 0;
	solveThread.ww							= uP.ww;
	solveThread.uP							= &uP;

	// set initial state in gui
	setInitialState();

	// solving random states?
	if (solveThread.solvingRandomStates) {
		setRandomState();
	}

	// show controls
	solveThread.hButtonSolveStopCalc.setImageFiles(*uP.buttonImages.buttonImagesSolveStopCalc);
	solveThread.hButtonSolveStopCalc.setText(L"Stop Calculation");
	buttonFuncSolvePause();

	// create thread
	solveThread.start(&solveThread.sharedVarsMutex, &solveThread.pauseMutex, &solveThread.eventUpdateOnThreadVars, &solveThread.eventThreadFinished, curZones, 0, uP.strDatabaseDir, uP.strDatabaseFileType, moveIndicesMap, reverseMoves, stateToSolve, initialState, finalState, doMove);

	// update gui
	auto sv = solveThread.lockThreadVariables();
	ss.str(L""); ss << L"step within zone: "	 << solveThread.curStepWithInZone+1			<< L"/" << solveThread.numStepOfCurZone;		solveThread.hTextStepWithinZone	.setText(ss.str());
	ss.str(L""); ss << L"showing zone: "		 << solveThread.curShowedZone				<< L"/" << curZones->numZones;					solveThread.hTextShowingZone	.setText(ss.str());
	ss.str(L""); ss << L"steps per sec: "		 << setprecision(2) << 1024.0f / ((float) solutionAnimationSleepDuration);					solveThread.hTextAnimationSpeed	.setText(ss.str());
	ss.str(L""); ss << L"states solved: "		 << solveThread.numRandomStateAlreadySolved << L"/" << solveThread.numRandomStatesTried;	solveThread.hTextRndStatesSolved.setText(ss.str());
	ss.str(L""); ss << L"zones solved: "		 << sv->numZonesSolved						<< L"/" << curZones->numZones;					solveThread.hTextZonesSolved	.setText(ss.str());
	ss.str(L""); ss << L"top branch: "			 << sv->curBranch[0]						<< L"/" << sv->numPossibleMoves;				solveThread.hTextTopBranch		.setText(ss.str());
	ss.str(L""); ss << L"step in total: "		 << solveThread.curShowedStep				<< L"/" << sv->totalNumSteps;					solveThread.hTextStepInTotal	.setText(ss.str());
	solveThread.unlockThreadVariables();

	return true;
}

//-----------------------------------------------------------------------------
// Name: updateResult()
// Desc: 
//-----------------------------------------------------------------------------
void pgsWin::pgsWinThreadSolve::updateResult(void* pUser)
{
	// locals
	auto pgsTs = (pgsWinThreadSolve*) pUser;
	wstringstream ss;

	// lock variables
	auto sv = pgsTs->lockThreadVariables();
	if (!sv) return;

	// update edit fields
	ss.str(L""); ss << L"step in total: "		 << pgsTs->curShowedStep	<< L"/" << sv->totalNumSteps;			pgsTs->hTextStepInTotal	.setText(ss.str());
	ss.str(L""); ss << L"zones solved: "		 << sv->numZonesSolved		<< L"/" << sv->totalNumZones;			pgsTs->hTextZonesSolved	.setText(ss.str());
	ss.str(L""); ss << L"top branch: "			 << sv->curBranch[0] + 1	<< L"/" << sv->numPossibleMoves;		pgsTs->hTextTopBranch	.setText(ss.str());

	// unlock
	pgsTs->unlockThreadVariables();
}

//-----------------------------------------------------------------------------
// Name: threadHasFinished()
// Desc: 
//-----------------------------------------------------------------------------
void pgsWin::pgsWinThreadSolve::threadHasFinished()
{
	auto sv = lockThreadVariables();

	if (sv) {
		// a solution was found
		if (sv->numSolutionsFound) {
			if (!solvingRandomStates) {
				hButtonSolveStopCalc.setText(L"Quit");
				hButtonSolveStopCalc.setImageFiles(*uP->buttonImages.buttonImagesZoneQuit);
			} else {
				if (!tc->threadShallBeStopped) hEventSolveNextRandomState.set();
				numRandomStateAlreadySolved++;
				numRandomStatesTried++;
			}
		// no solution was found
		} else {
			if (!solvingRandomStates) {
				ww->showMessageBox(L"No solution found!", L"ERROR", MB_OK);
			} else {
				if (!tc->threadShallBeStopped) hEventSolveNextRandomState.set();
				timesUnsolved[sv->zoneNumber]++; 
				numRandomStateAlreadySolved--;
			}
		}
	}
	unlockThreadVariables();
	hTextTopBranch.setState(wildWeasel::guiElemState::HIDDEN);
	hButtonSolveStopCalc.setText(L"Quit");
	hButtonSolveStopCalc.setImageFiles(*uP->buttonImages.buttonImagesZoneQuit);
}

//-----------------------------------------------------------------------------
// Name: buttonFuncSolvePlay()
// Desc: 
//-----------------------------------------------------------------------------
void pgsWin::buttonFuncSolvePlay()
{
	solveThread.hButtonSolvePlay .setState(wildWeasel::guiElemState::GRAYED);
	solveThread.hButtonSolvePause.setState(wildWeasel::guiElemState::DRAWED);
	uP.timers.timerPlay->start(solutionAnimationSleepDuration);
	autoForward = true;
}

//-----------------------------------------------------------------------------
// Name: buttonFuncSolvePause()
// Desc: 
//-----------------------------------------------------------------------------
void pgsWin::buttonFuncSolvePause()
{
	solveThread.hButtonSolvePlay .setState(solveThread.solvingRandomStates ? wildWeasel::guiElemState::GRAYED : wildWeasel::guiElemState::DRAWED);
	solveThread.hButtonSolvePause.setState(wildWeasel::guiElemState::GRAYED);
	uP.timers.timerPlay->terminate();
	autoForward = false;
}

//-----------------------------------------------------------------------------
// Name: buttonFuncSolveForward()
// Desc: 
//-----------------------------------------------------------------------------
void pgsWin::buttonFuncSolveForward()
{
	// locals
	wstringstream								ss;
	unsigned int								moveId;
	auto										sv		= solveThread.lockThreadVariables();
	list<backTracking::solutionPath>::iterator	spItr	= sv->solPathItr;

	// since subsequent back- and forward moves are vanished, one have to wait until a zone is completly finished
	if (sv->numZonesSolved <= solveThread.curShowedZone) {
		solveThread.unlockThreadVariables();
		return;
	}

	// is there a further step ?
	if (sv->totalNumSteps == solveThread.curShowedStep) {
		solveThread.unlockThreadVariables();
		return;
	}
	solveThread.curShowedStep++;

	// numStepOfCurZone initialized?
	solveThread.numStepOfCurZone = (*sv->solPathItr).numSteps;
	while (solveThread.numStepOfCurZone == 0) {
		sv->solPathItr++;
		solveThread.curShowedZone++;
		solveThread.numStepOfCurZone = (*sv->solPathItr).numSteps;
	}

	// current move
	moveId = (*sv->solPathItr).path[solveThread.curStepWithInZone];

	// is current showed step already the last one of the current showed zone ?
	if (solveThread.curStepWithInZone == solveThread.numStepOfCurZone - 1 && sv->totalNumSteps > solveThread.curShowedStep) {
		do {
			sv->solPathItr++;
			solveThread.curShowedZone++;
			if (sv->solPathItr == sv->listSolPath.end()) {
				buttonFuncSolvePause();
				sv->solPathItr	= spItr;
				solveThread.curShowedZone		= solveThread.curShowedZone;
				solveThread.curShowedStep--;
				solveThread.unlockThreadVariables();
				return;
			}
		} while ((*sv->solPathItr).numSteps == 0);
		solveThread.curStepWithInZone				= 0;
		solveThread.numStepOfCurZone				= (*sv->solPathItr).numSteps;
	} else {
		solveThread.curStepWithInZone++;
	}

	// update edit fields
	ss.str(L""); ss << L"step within zone: "	 << solveThread.curStepWithInZone+1			<< L"/" << solveThread.numStepOfCurZone;	solveThread.hTextStepWithinZone .setText(ss.str());
	ss.str(L""); ss << L"step in total: "		 << solveThread.curShowedStep				<< L"/" << sv->totalNumSteps;				solveThread.hTextStepInTotal	.setText(ss.str());
	ss.str(L""); ss << L"showing zone: "		 << solveThread.curShowedZone				<< L"/" << curZones->numZones;				solveThread.hTextShowingZone	.setText(ss.str());

	// do move
	solveThread.doMoveAndShow(moveId);
	solveThread.unlockThreadVariables();
}

//-----------------------------------------------------------------------------
// Name: buttonFuncSolveBackward()
// Desc: 
//-----------------------------------------------------------------------------
void pgsWin::buttonFuncSolveBackward()
{
	// locals
	wstringstream ss;
	unsigned int moveId;
	auto							sv = solveThread.lockThreadVariables();

	// is there a further step ?
	if (0 == solveThread.curShowedStep) {
		solveThread.unlockThreadVariables();
		return;
	}
	buttonFuncSolvePause();
	solveThread.curShowedStep--;

	// is current showed step already the last one of the current showed zone ?
	if (solveThread.curStepWithInZone == 0) {
		do {
			sv->solPathItr--;
			solveThread.curShowedZone--;
			if (sv->solPathItr == sv->listSolPath.begin()) {
				break;
			}
		} while ((*sv->solPathItr).numSteps == 0);
		solveThread.curStepWithInZone				= (*sv->solPathItr).numSteps - 1;
		solveThread.numStepOfCurZone				= (*sv->solPathItr).numSteps;
	} else {
		solveThread.curStepWithInZone--;
	}

	// current move
	moveId = reverseMoves[(*sv->solPathItr).path[solveThread.curStepWithInZone]];

	// update edit fields
	ss.str(L""); ss << L"step within zone: "	 << solveThread.curStepWithInZone+1			<< L"/" << solveThread.numStepOfCurZone;	solveThread.hTextStepWithinZone	.setText(ss.str());
	ss.str(L""); ss << L"step in total: "		 << solveThread.curShowedStep				<< L"/" << sv->totalNumSteps;				solveThread.hTextStepInTotal	.setText(ss.str());
	ss.str(L""); ss << L"showing zone: "		 << solveThread.curShowedZone				<< L"/" << curZones->numZones;				solveThread.hTextShowingZone	.setText(ss.str());

	// do move
	solveThread.doMoveAndShow(moveId);
	solveThread.unlockThreadVariables();
}

//-----------------------------------------------------------------------------
// Name: buttonFuncSolveShowStats()
// Desc: 
//-----------------------------------------------------------------------------
void pgsWin::buttonFuncSolveShowStats()
{
	// locals
	wstringstream wss;
	unsigned int i;

	// add number of times each zone wasn't solved
	if (solveThread.numRandomStatesTried - solveThread.numRandomStateAlreadySolved == 0) {
		wss << "All random states could be solved so far.\n";
	} else {
		wss << "Amount of times zone could not be solved:\n";
		for (i=0; i<curZones->numZones; i++) {
			if (solveThread.timesUnsolved[i]) {
				wss << "Zone " << setfill(L'0') << setw(2) << i << ": " << solveThread.timesUnsolved[i] << "\n";
			}
		}
	}

	// add solving ratio
	wss << "\nSolving ratio: " << (int) ((float)solveThread.numRandomStateAlreadySolved / solveThread.numRandomStatesTried * 100.0f) << "%\n";

	// show messagebox
	uP.ww->showMessageBox(L"Statistics", wss.str().c_str(), MB_OK);
}

//-----------------------------------------------------------------------------
// Name: buttonFuncSolveStopCalc()
// Desc: 
//-----------------------------------------------------------------------------
void pgsWin::buttonFuncSolveStopCalc()
{
	// calcution is not running any more, so just quit
	if (!solveThread.isRunning()) {
		uP.timers.timerPlay->terminate();
		showSolveControls(false);
	// calculation is still running
	} else {
		solveThread.stop();
		solveThread.hButtonSolveStopCalc.setText(L"Quit");
		solveThread.hButtonSolveStopCalc.setImageFiles(*uP.buttonImages.buttonImagesZoneQuit);
	}
}

//-----------------------------------------------------------------------------
// Name: buttonFuncSolveFaster()
// Desc: 
//-----------------------------------------------------------------------------
void pgsWin::buttonFuncSolveFaster()
{
	// locals
	wstringstream ss;

	if (solutionAnimationSleepDuration <= 256) return;
	solutionAnimationSleepDuration /= 2;
	if (autoForward) {
		uP.timers.timerPlay->terminate();
		uP.timers.timerPlay->start(solutionAnimationSleepDuration);
	}
	ss.str(L""); ss << L"steps per sec: "		 << setprecision(2) << 1024.0f / ((float) solutionAnimationSleepDuration);	
	solveThread.hTextAnimationSpeed.setText(ss.str());
}

//-----------------------------------------------------------------------------
// Name: buttonFuncSolveSlower()
// Desc: 
//-----------------------------------------------------------------------------
void pgsWin::buttonFuncSolveSlower()
{	
	// locals
	wstringstream ss;

	if (solutionAnimationSleepDuration >= 16384) return;
	solutionAnimationSleepDuration *= 2;
	if (autoForward) {
		uP.timers.timerPlay->terminate();
		uP.timers.timerPlay->start(solutionAnimationSleepDuration);
	}
	ss.str(L""); ss << L"steps per sec: "		 << setprecision(2) << 1024.0f / ((float) solutionAnimationSleepDuration);	
	solveThread.hTextAnimationSpeed.setText(ss.str());
}

//-----------------------------------------------------------------------------
// Name: solveNextRandomState()
// Desc: 
//-----------------------------------------------------------------------------
void pgsWin::solveNextRandomState(void* pUser)
{
	// leave if "stop calculation" button was already pressed
	if (!solveThread.solvingRandomStates) return;

	// set random state
	solveState(solveThread.getStateToSolve(), solveThread.getInitialState(), solveThread.getFinalState(), reverseMoves, solveThread.doMoveAndShow, solveThread.doMove, solveThread.setInitialState, solveThread.doesAllFieldsBelongToZone, solveThread.setRandomState);
}

//-----------------------------------------------------------------------------
// Name: setSolvingRandomStates()
// Desc: 
//-----------------------------------------------------------------------------
void pgsWin::setSolvingRandomStates(bool active) 
{
	solveThread.solvingRandomStates			= active;
	solveThread.numRandomStateAlreadySolved	= 0;
	solveThread.numRandomStatesTried		= 0;
	solveThread.timesUnsolved.resize(curZones->numZones, 0);
}
#pragma endregion
