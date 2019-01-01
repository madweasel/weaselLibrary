/*********************************************************************
	miniMaxWinInspectDb.cpp
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/

#include "miniMaxWin.h"

#pragma region inspection mode

const float miniMaxWinInspectDb::treeViewItemInfo::textSize = 0.5f;

//-----------------------------------------------------------------------------
// Name: miniMaxWin constructor()
// Desc: 
//-----------------------------------------------------------------------------
miniMaxWinInspectDb::miniMaxWinInspectDb(	wildWeasel::masterMind *			ww,
											miniMax *							pMiniMax,
											wildWeasel::alignment &				amAreaInspectDb,
											wildWeasel::font2D*					font,
											wildWeasel::texture*			    textureLine,
											miniMaxGuiField&					pGuiField)
{
	this->ww								= ww;
	this->pMiniMax							= pMiniMax;
	this->textureLine						= textureLine;
	this->hFontOutputBox					= font;
	this->pGuiField							= &pGuiField;

	// calc position of gui elements
	resize(amAreaInspectDb);
}

//-----------------------------------------------------------------------------
// Name: miniMaxWin destructor()
// Desc: 
//-----------------------------------------------------------------------------
miniMaxWinInspectDb::~miniMaxWinInspectDb()
{
	for (auto& curTreeViewItem : treeViewItems) {
		delete curTreeViewItem;
	}
	treeViewItems.clear();
	pTreeViewInspect.getRootBranch()->removeAllSubBranchesAndItems(true);
	pTreeViewInspect.removeAllItems(true);
	pTreeViewInspect.removeAllColumns(true);
}

//-----------------------------------------------------------------------------
// Name: createControls()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMaxWinInspectDb::createControls()
{
	// parameters ok?
	if (ww == nullptr) return false;
	if (pTreeViewInspect.isInitialized()) return false;

	// create tree view for database inspection
	pTreeViewInspect.create(ww, 0, buttonImagesPlus, buttonImagesMinus, textureLine);
	pTreeViewInspect.setAlignment(amListInspectDb);
	pTreeViewInspect.setSelectionMode(wildWeasel::listView2D::selectionMode::ROW_WISE);
	pTreeViewInspect.setMarkerColor(wildWeasel::color::lightBlue);
	pTreeViewInspect.setVisibilityColumnHeader(false);
	pTreeViewInspect.setVisibilityRowHeader(false);
	pTreeViewInspect.insertColumn(0, nullptr, treeViewItemInfo::colWidthInPixels);
	pTreeViewInspect.createScrollBars(buttonImagesArrow, buttonImagesVoid, buttonImagesVoid);
	pTreeViewInspect.setColumnScrollBarHeight(scrollBarWidth);
	pTreeViewInspect.setRowScrollBarWidth	(scrollBarWidth);
	pTreeViewInspect.setVisibilityColumnScrollBar(true);
	pTreeViewInspect.setVisibilityRowScrollBar   (true);
	pTreeViewInspect.setPosition(0, 0, true);
	pTreeViewInspect.setScale	(1, 1, true);
	pTreeViewInspect.setState(wildWeasel::guiElemState::HIDDEN);
	pTreeViewInspect.setTextBorder(3, 3);
	pTreeViewInspect.setTextSize(treeViewItemInfo::textSize, treeViewItemInfo::textSize);
	pTreeViewInspect.alignAllItems();

	return true;
}

//-----------------------------------------------------------------------------
// Name: showInspectionControls()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMaxWinInspectDb::showControls(bool visible)
{
	// locals
	wildWeasel::guiElemState	nCmdShow = visible ? wildWeasel::guiElemState::DRAWED : wildWeasel::guiElemState::HIDDEN;
	unsigned int					curLayer;

	pTreeViewInspect.setState(nCmdShow);
	pGuiField->setVisibility(curShowedLayer > 0 ? visible : false);
	
	curShowedLayer				= 0;
	curShowedState				= 0;
	showingInspectionControls	= visible;

	if (showingInspectionControls) {
		pGuiField->setAlignment(amFieldInspectDb);
	}

	// add layers
	if (visible) {

		// do not create tree view items twice
		if (treeViewItems.size() == 0) {
			treeViewItems.resize(pMiniMax->getNumberOfLayers());
			for (curLayer = 0; curLayer < pMiniMax->getNumberOfLayers(); curLayer++) {
				if (!pMiniMax->getNumberOfKnotsInLayer(curLayer)) continue;
				treeViewItems[curLayer] = new treeViewItemLayer{&pTreeViewInspect, pTreeViewInspect.getRootBranch(), &buttonImagesListItem, hFontOutputBox, curLayer, pMiniMax, pGuiField, &curShowedLayer, &curShowedState};
			}
		}
		pTreeViewInspect.alignAllItems();
	
	} else {

		// delete tree view items
		for (auto& curTreeViewItem : treeViewItems) {
			delete curTreeViewItem;
		}
		treeViewItems.clear();
		pTreeViewInspect.getRootBranch()->removeAllSubBranchesAndItems(true);
		pTreeViewInspect.removeAllItems(true);
	}

	return true;
}

//-----------------------------------------------------------------------------
// Name: resize()
// Desc: 
//-----------------------------------------------------------------------------
void miniMaxWinInspectDb::resize(wildWeasel::alignment& amNewArea)
{
	amAreaInspectDb	= &amNewArea;

	amListInspectDb .create(ww->alignmentRootFrame);
	amFieldInspectDb.create(ww->alignmentRootFrame);

	amListInspectDb .setInsideAnotherRect(amNewArea);
	amFieldInspectDb.setInsideAnotherRect(amNewArea);
	
	if (showingInspectionControls) {
		pGuiField->setAlignment(amFieldInspectDb);
	}
}

#pragma endregion

#pragma region treeView

#pragma region treeViewItemInfo
//-----------------------------------------------------------------------------
// Name: treeViewItemInfo()
// Desc: 
//-----------------------------------------------------------------------------
miniMaxWinInspectDb::treeViewItemInfo::treeViewItemInfo(treeViewItemInfo *parent) : 
	type				{ treeViewItemInfo::classType::base	} ,
	layerNumber			{ parent->layerNumber				} ,
	pTreeViewInspect	{ parent->pTreeViewInspect			} ,
	pMiniMax			{ parent->pMiniMax					} ,
	pGuiField			{ parent->pGuiField					} ,
	pCurShowedLayer		{ parent->pCurShowedLayer			} ,
	pCurShowedState		{ parent->pCurShowedState			} ,
	numberOfKnotsInLayer{ parent->numberOfKnotsInLayer		} ,
	pListImages			{ parent->pListImages				} ,
	pFont				{ parent->pFont						} ,
	parent				{ parent							}
{
}

//-----------------------------------------------------------------------------
// Name: ~treeViewItemInfo()
// Desc: 
//-----------------------------------------------------------------------------
miniMaxWinInspectDb::treeViewItemInfo::~treeViewItemInfo()
{
	collapse();
}

//-----------------------------------------------------------------------------
// Name: writeKnotValueIntoString()
// Desc: 
//-----------------------------------------------------------------------------
void miniMaxWinInspectDb::treeViewItemInfo::writeKnotValueIntoString(wstringstream &wssTmp, unsigned int stateNumber)
{
	// locals
	float						floatValue;
	miniMax::twoBit				shortKnotValue;
	miniMax::plyInfoVarType		plyInfo;

	wssTmp.str(L"");
	pMiniMax->layerInDatabase = pMiniMax->isLayerInDatabase(layerNumber);
	if (pMiniMax->layerInDatabase) {
		pMiniMax->readKnotValueFromDatabase(layerNumber, stateNumber, shortKnotValue);
		pMiniMax->readPlyInfoFromDatabase(layerNumber, stateNumber, plyInfo);
		wssTmp << "short value: " << ((shortKnotValue == SKV_VALUE_GAME_WON) ? "WON" : ((shortKnotValue == SKV_VALUE_GAME_LOST) ? "LOST" : ((shortKnotValue == SKV_VALUE_GAME_DRAWN) ? "DRAW" : "INVALID")));
		wssTmp << ", ply info: " << ((int) plyInfo) << ((plyInfo == PLYINFO_VALUE_DRAWN) ? " (DRAWN)" : ((plyInfo == PLYINFO_VALUE_UNCALCULATED) ? " (UNCALC)" : ((plyInfo == PLYINFO_VALUE_INVALID) ? " (INVALID)" : "")));
	} else {
		pMiniMax->setOpponentLevel(0, false);
		if (pMiniMax->setSituation(0, layerNumber, stateNumber)) {
			pGuiField->setState(layerNumber, stateNumber);
			pMiniMax->getValueOfSituation(0, floatValue, shortKnotValue);
			wssTmp << "short value: " << ((shortKnotValue == SKV_VALUE_GAME_WON) ? "WON" : ((shortKnotValue == SKV_VALUE_GAME_LOST) ? "LOST" : ((shortKnotValue == SKV_VALUE_GAME_DRAWN) ? "UNCALCULATED" : "INVALID")));
		} else {
			wssTmp << "short value: " << "INVALID";
		}
	}
}

//-----------------------------------------------------------------------------
// Name: collapse()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMaxWinInspectDb::treeViewItemInfo::collapse()
{
	// delete all child treeViewItemInfo elements
	for (auto& curChild : children) {
		delete curChild;
	}

	children.clear();

	pBranch->removeAllSubBranchesAndItems(true);
	
	return true;
}
#pragma endregion

#pragma region treeViewItemLayer
//-----------------------------------------------------------------------------
// Name: treeViewItemInfo()
// Desc: 
//-----------------------------------------------------------------------------
miniMaxWinInspectDb::treeViewItemLayer::treeViewItemLayer(wildWeasel::treeView2D* pTreeViewInspect, wildWeasel::treeView2D::branch* branch, wildWeasel::buttonImageFiles* pListImages, wildWeasel::font2D* pFont, unsigned int layerNumber, miniMax* pMiniMax, miniMaxGuiField* pGuiField, unsigned int* pCurShowedLayer, miniMax::stateNumberVarType* pCurShowedState)
{
	// locals
	wstringstream				wssTmp;			
	
	wssTmp						<< L"layer: " << layerNumber;
	this->pTreeViewInspect		= pTreeViewInspect;
	this->pListImages			= pListImages;
	this->pFont					= pFont;
	this->layerNumber			= layerNumber;
	this->pGuiField				= pGuiField;
	this->pMiniMax				= pMiniMax;
	this->numberOfKnotsInLayer	= pMiniMax->getNumberOfKnotsInLayer(layerNumber);
	this->pCurShowedLayer		= pCurShowedLayer;
	this->pCurShowedState		= pCurShowedState;
	this->pBranch				= branch->insertSubBranchAndItems_plainButton2D(wildWeasel::treeView2D::branch::LAST_SUB_BRANCH, treeViewItemInfo::rowHeightInPixels, *pListImages, pFont, wssTmp.str(), true);
	this->type					= classType::layer;

	this->pBranch->assignOnExpand  (bind(&treeViewItemLayer::expand,   this), nullptr);
	this->pBranch->assignOnCollapse(bind(&treeViewItemLayer::collapse, this), nullptr);
}

//-----------------------------------------------------------------------------
// Name: expand()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMaxWinInspectDb::treeViewItemLayer::expand()
{
	// locals
	wstringstream				wssTmp;
	unsigned int				numMissingDigits	= (unsigned int) log10(numberOfKnotsInLayer) + 1;
	unsigned long long			stepSize			= (unsigned long long) pow(10, numMissingDigits);
	unsigned int				stateNumberPrefix	= 0;
	
	// number of won/lost/invalid/drawn states
	wssTmp.imbue(locale("German_Switzerland"));
	wssTmp.str(L"");		wssTmp << L"total number of knots    : "	<< pMiniMax->getNumberOfKnotsInLayer(layerNumber);								pBranch->insertSubBranchAndItems_plainButton2D(0, treeViewItemInfo::rowHeightInPixels, *pListImages, pFont, wssTmp.str());	 
	wssTmp.str(L"");		wssTmp << L"additional information   : "	<< wildWeasel::string2wstring(pMiniMax->getOutputInformation(layerNumber));	pBranch->insertSubBranchAndItems_plainButton2D(1, treeViewItemInfo::rowHeightInPixels, *pListImages, pFont, wssTmp.str());	 
	wssTmp.str(L"");		wssTmp << L"number of won states     : "	<< pMiniMax->getNumWonStates(layerNumber);										pBranch->insertSubBranchAndItems_plainButton2D(2, treeViewItemInfo::rowHeightInPixels, *pListImages, pFont, wssTmp.str());	 
	wssTmp.str(L"");		wssTmp << L"number of lost states      : "	<< pMiniMax->getNumLostStates(layerNumber);										pBranch->insertSubBranchAndItems_plainButton2D(3, treeViewItemInfo::rowHeightInPixels, *pListImages, pFont, wssTmp.str());	 
	wssTmp.str(L"");		wssTmp << L"number of drawn states  : "		<< pMiniMax->getNumDrawnStates(layerNumber);									pBranch->insertSubBranchAndItems_plainButton2D(4, treeViewItemInfo::rowHeightInPixels, *pListImages, pFont, wssTmp.str());	 
	wssTmp.str(L"");		wssTmp << L"number of invalid states : "	<< pMiniMax->getNumInvalidStates(layerNumber);									pBranch->insertSubBranchAndItems_plainButton2D(5, treeViewItemInfo::rowHeightInPixels, *pListImages, pFont, wssTmp.str());	 

	// state numbers
	treeViewItemUncomplState::expand_static(*this, stateNumberPrefix, numMissingDigits, stepSize);

	return true;
};
#pragma endregion

#pragma region treeViewItemUncomplState
//-----------------------------------------------------------------------------
// Name: treeViewItemUncomplState()
// Desc: 
//-----------------------------------------------------------------------------
miniMaxWinInspectDb::treeViewItemUncomplState::treeViewItemUncomplState(miniMaxWinInspectDb::treeViewItemInfo* parent, unsigned int stateNumberPrefix, unsigned int curStateDigit, unsigned long long newStepSize, unsigned int newNumMissingDigits) : treeViewItemInfo{parent}
{
	wstringstream			wssTmp;
	unsigned int			prefixDigits		= (unsigned int) (stateNumberPrefix / (10*newStepSize));

	wssTmp.str(L"");			
	wssTmp << L"state: ";
	if (prefixDigits) wssTmp << prefixDigits;
	wssTmp << curStateDigit << setw(newNumMissingDigits) << setfill(L'x') << L"";

	this->type					= classType::uncompleteState;
	this->stateNumberPrefix		= (unsigned int) (stateNumberPrefix + curStateDigit * newStepSize);
	this->numMissingDigits		= newNumMissingDigits;
	this->stepSize				= newStepSize;
	this->pBranch				= parent->pBranch->insertSubBranchAndItems_plainButton2D(wildWeasel::treeView2D::branch::LAST_SUB_BRANCH, treeViewItemInfo::rowHeightInPixels, *pListImages, pFont, wssTmp.str(), true);

	pBranch->assignOnExpand  (bind(&treeViewItemUncomplState::expand,   this), nullptr);
	pBranch->assignOnCollapse(bind(&treeViewItemUncomplState::collapse, this), nullptr);
}

//-----------------------------------------------------------------------------
// Name: expand()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMaxWinInspectDb::treeViewItemUncomplState::expand()
{
	// call static function
	return treeViewItemUncomplState::expand_static(*this, stateNumberPrefix, numMissingDigits, stepSize);
}

//-----------------------------------------------------------------------------
// Name: expand()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMaxWinInspectDb::treeViewItemUncomplState::expand_static(treeViewItemInfo& tvii, unsigned int& stateNumberPrefix, unsigned int& numMissingDigits, unsigned long long& stepSize)
{
	// locals
	unsigned int				curStateDigit;
	
	// expand last prefix number with only one digit missing
	if (numMissingDigits == 1) {
		unsigned int	maxDigit		= (tvii.numberOfKnotsInLayer - stateNumberPrefix - 1) < 9 ? tvii.numberOfKnotsInLayer - stateNumberPrefix - 1 : 9;
		
		for (curStateDigit=0; curStateDigit<=maxDigit; curStateDigit++) {
			tvii.children.push_back(new treeViewItemState{&tvii, stateNumberPrefix + curStateDigit});
		}

	// expand prefix number
	} else {
		unsigned long long	newStepSize			= stepSize / 10;
		unsigned int		newNumMissingDigits	= numMissingDigits - 1;
		unsigned int		maxDigit			= (unsigned int) (((tvii.numberOfKnotsInLayer - stateNumberPrefix - 1) / newStepSize < 9) ? (tvii.numberOfKnotsInLayer - stateNumberPrefix - 1) / newStepSize : 9);

		for (curStateDigit=0; curStateDigit<=maxDigit; curStateDigit++) {
			tvii.children.push_back(new treeViewItemUncomplState{&tvii, stateNumberPrefix, curStateDigit, newStepSize, newNumMissingDigits});
		}
	}

	return true;
}
#pragma endregion

#pragma region treeViewItemState

//-----------------------------------------------------------------------------
// Name: treeViewItemState()
// Desc: 
//-----------------------------------------------------------------------------
miniMaxWinInspectDb::treeViewItemState::treeViewItemState(treeViewItemInfo *parent, unsigned int stateNumber) : treeViewItemState{parent, parent->layerNumber, stateNumber}
{
}

//-----------------------------------------------------------------------------
// Name: treeViewItemState()
// Desc: 
//-----------------------------------------------------------------------------
miniMaxWinInspectDb::treeViewItemState::treeViewItemState(treeViewItemInfo *parent, unsigned int layerNumber, unsigned int stateNumber) : treeViewItemInfo{parent}
{
	wstringstream			wssTmp;

	wssTmp.str(L"");		
	if (parent->layerNumber != layerNumber) wssTmp << L"layer: " << layerNumber << L", ";
	wssTmp << L"state: " << stateNumber;

	this->type				= classType::state;
	this->stateNumber		= stateNumber;
	this->layerNumber		= layerNumber;
	this->pBranch			= parent->pBranch->insertSubBranchAndItems_plainButton2D(wildWeasel::treeView2D::branch::LAST_SUB_BRANCH, treeViewItemInfo::rowHeightInPixels, *pListImages, pFont, wssTmp.str(), true);
			
	pBranch->assignOnExpand		(bind(&treeViewItemState::expand,	this), nullptr);
	pBranch->assignOnCollapse	(bind(&treeViewItemState::collapse,	this), nullptr);
	pBranch->assignOnGotFocus	(bind(&treeViewItemState::select,	this), nullptr);
	pBranch->assignOnLostFocus	(bind(&treeViewItemState::deselect,	this), nullptr);
}

//-----------------------------------------------------------------------------
// Name: selected()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMaxWinInspectDb::treeViewItemState::select()
{
	// don't show current selected state if its invalid
	// ... state is invalid
	if (false) {
		*pCurShowedLayer	= 0;
		*pCurShowedState	= 0;
	// show current selected state
	} else {
		*pCurShowedLayer	= layerNumber;
		*pCurShowedState	= stateNumber;
	}

	pGuiField->setState(*pCurShowedLayer, *pCurShowedState);
	pGuiField->setVisibility(true);

	return true;
}

//-----------------------------------------------------------------------------
// Name: selected()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMaxWinInspectDb::treeViewItemState::deselect()
{
	// don't show any state
	*pCurShowedLayer	= 0;
	*pCurShowedState	= 0;

	pGuiField->setVisibility(false);

	return true;
}

//-----------------------------------------------------------------------------
// Name: expand()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMaxWinInspectDb::treeViewItemState::expand()
{
	// locals
	wstringstream		wssTmp;
	unsigned int  *		symStateNumbers		= NULL;
	unsigned int		numSymmetricStates	= 0;

	writeKnotValueIntoString(wssTmp, stateNumber);			
	pBranch->insertSubBranchAndItems_plainButton2D(0, treeViewItemInfo::rowHeightInPixels, *pListImages, pFont, wssTmp.str());	 

	pMiniMax->setOpponentLevel(0, false);
	if (pMiniMax->setSituation(0, layerNumber, stateNumber)) {
		pGuiField->setState(layerNumber, stateNumber);
		children.push_back(new treeViewItemSym {this, stateNumber});
		children.push_back(new treeViewItemSuc {this, stateNumber});
		children.push_back(new treeViewItemPrec{this, stateNumber});
	}

	return true;
}
#pragma endregion

#pragma region treeViewItemSym
//-----------------------------------------------------------------------------
// Name: treeViewItemSym()
// Desc: 
//-----------------------------------------------------------------------------
miniMaxWinInspectDb::treeViewItemSym::treeViewItemSym(treeViewItemInfo *parent, unsigned int stateNumber) : treeViewItemInfo{parent}
{
	wstringstream			wssTmp;

	wssTmp.str(L"");		wssTmp << L"symmetric states:";

	this->stateNumber		= stateNumber;
	this->type				= classType::symmetricStates;
	this->pBranch			= parent->pBranch->insertSubBranchAndItems_plainButton2D(wildWeasel::treeView2D::branch::LAST_SUB_BRANCH, treeViewItemInfo::rowHeightInPixels, *pListImages, pFont, wssTmp.str(), true);
			
	pBranch->assignOnExpand		(bind(&treeViewItemSym::expand,		this), nullptr);
	pBranch->assignOnCollapse	(bind(&treeViewItemSym::collapse,	this), nullptr);
	pBranch->assignOnGotFocus	(bind(&treeViewItemSym::select,		this), nullptr);
	pBranch->assignOnLostFocus	(bind(&treeViewItemSym::deselect,	this), nullptr);
}

//-----------------------------------------------------------------------------
// Name: expand()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMaxWinInspectDb::treeViewItemSym::expand()
{
	// locals
	wstringstream	wssTmp;

	// show current state
	pMiniMax->setOpponentLevel(0, false);
	pMiniMax->setSituation(0, layerNumber, stateNumber);
	pGuiField->setState(layerNumber, stateNumber);

	// get symmetric states
	unsigned int	numSymmetricStates		= 0;
	unsigned int*	symStateNumbers			= nullptr;
	pMiniMax->getSymStateNumWithDoubles(0, &numSymmetricStates, &symStateNumbers);

	// remove duplicates
	vector<unsigned int> vecSymStateNumbers{symStateNumbers, symStateNumbers + numSymmetricStates};
	sort( vecSymStateNumbers.begin(), vecSymStateNumbers.end() );
	vecSymStateNumbers.erase( unique( vecSymStateNumbers.begin(), vecSymStateNumbers.end() ), vecSymStateNumbers.end() );
	
	// print number of symmetric states
	wssTmp.str(L"");	wssTmp << L"number of symmetric states: " << vecSymStateNumbers.size();	 pBranch->insertSubBranchAndItems_plainButton2D(0, treeViewItemInfo::rowHeightInPixels, *pListImages, pFont, wssTmp.str());	 

	// add each symmetric state to list
	for (auto& curSymmetricState : vecSymStateNumbers) {
		//if (curSymmetricState != stateNumber) {
			if (curSymmetricState >= pMiniMax->layerStats[layerNumber].knotsInLayer) {
				// ... pTreeViewInspect->parent->showMessageBox();
				MessageBox(NULL, L"getSymStateNumWithDoubles() returns an invalid state number.", L"ERROR", MB_OK);
			} else {
				children.push_back(new treeViewItemState{this, curSymmetricState});
			}
		//}
	}

	return true;
}
#pragma endregion

#pragma region treeViewItemSuc
//-----------------------------------------------------------------------------
// Name: treeViewItemState()
// Desc: 
//-----------------------------------------------------------------------------
miniMaxWinInspectDb::treeViewItemSuc::treeViewItemSuc(treeViewItemInfo *parent, unsigned int stateNumber) : treeViewItemInfo{parent}
{
	wstringstream			wssTmp;

	wssTmp.str(L"");		wssTmp << L"succeding states:";

	this->stateNumber		= stateNumber;
	this->type				= classType::succedingStates;
	this->pBranch			= parent->pBranch->insertSubBranchAndItems_plainButton2D(wildWeasel::treeView2D::branch::LAST_SUB_BRANCH, treeViewItemInfo::rowHeightInPixels, *pListImages, pFont, wssTmp.str(), true);
			
	pBranch->assignOnExpand		(bind(&treeViewItemSuc::expand,		this), nullptr);
	pBranch->assignOnCollapse	(bind(&treeViewItemSuc::collapse,	this), nullptr);
	pBranch->assignOnGotFocus	(bind(&treeViewItemSuc::select,		this), nullptr);
	pBranch->assignOnLostFocus	(bind(&treeViewItemSuc::deselect,	this), nullptr);
}

//-----------------------------------------------------------------------------
// Name: expand()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMaxWinInspectDb::treeViewItemSuc::expand()
{
	// locals
	unsigned int			threadNo			= 0;
	unsigned int			curSuccedingState;
	unsigned int			curSuccedingLayer;
	wstringstream			wssTmp;
	void *					pBackup;
	void *					pPossibilities;
	bool					isOpponentLevel;
	unsigned int			numPossibilities;
	unsigned int			curPossibility;
	unsigned int *			idPossibility;

	// set current state
	pMiniMax->setOpponentLevel(threadNo, false);
	pMiniMax->setSituation(threadNo, layerNumber, stateNumber);
	pGuiField->setState(layerNumber, stateNumber);
	idPossibility = pMiniMax->getPossibilities(threadNo, &numPossibilities, &isOpponentLevel, &pPossibilities);

	// write number of succeding states
	wssTmp.str(L"");	wssTmp << L"number of succeding states: " << numPossibilities;		pBranch->insertSubBranchAndItems_plainButton2D(0, treeViewItemInfo::rowHeightInPixels, *pListImages, pFont, wssTmp.str());	 
	
	// add each move possibility to list
	for (curPossibility=0; curPossibility<numPossibilities; curPossibility++) {

		// do move
		pMiniMax->move(threadNo, idPossibility[curPossibility], isOpponentLevel, &pBackup, pPossibilities);
		pMiniMax->getLayerAndStateNumber(threadNo, curSuccedingLayer, curSuccedingState);

		// add item in list
		if (curSuccedingState >= pMiniMax->layerStats[curSuccedingLayer].knotsInLayer) {
			// ... pTreeViewInspect->parent->showMessageBox();
			MessageBox(NULL, L"getLayerAndStateNumber() returns an invalid state number.", L"ERROR", MB_OK);
		} else {
			children.push_back(new treeViewItemState{this, curSuccedingLayer, curSuccedingState});
		}

		// undo move
		pMiniMax->undo(threadNo, idPossibility[curPossibility], isOpponentLevel, pBackup, pPossibilities);
	}

	// clean up
	pMiniMax->deletePossibilities(threadNo, pPossibilities);
	return true;
}
#pragma endregion

#pragma region treeViewItemPrec
//-----------------------------------------------------------------------------
// Name: treeViewItemState()
// Desc: 
//-----------------------------------------------------------------------------
miniMaxWinInspectDb::treeViewItemPrec::treeViewItemPrec(treeViewItemInfo *parent, unsigned int stateNumber) : treeViewItemInfo{parent}
{
	wstringstream			wssTmp;

	wssTmp.str(L"");		wssTmp << L"preceeding states:";

	this->stateNumber		= stateNumber;
	this->type				= classType::precedingStates;
	this->pBranch			= parent->pBranch->insertSubBranchAndItems_plainButton2D(wildWeasel::treeView2D::branch::LAST_SUB_BRANCH, treeViewItemInfo::rowHeightInPixels, *pListImages, pFont, wssTmp.str(), true);
			
	pBranch->assignOnExpand		(bind(&treeViewItemPrec::expand,	this), nullptr);
	pBranch->assignOnCollapse	(bind(&treeViewItemPrec::collapse,	this), nullptr);
	pBranch->assignOnGotFocus	(bind(&treeViewItemPrec::select,	this), nullptr);
	pBranch->assignOnLostFocus	(bind(&treeViewItemPrec::deselect,	this), nullptr);
}
	
//-----------------------------------------------------------------------------
// Name: expand()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMaxWinInspectDb::treeViewItemPrec::expand()
{
	// locals
	unsigned int					threadNo			= 0;
	unsigned int					curPreceedingState;
	unsigned int					curPreceedingLayer;
	wstringstream					wssTmp;
	miniMax::retroAnalysisPredVars 	predVars[MAX_NUM_PREDECESSORS];
	unsigned int					amountOfPred;
	unsigned int					curPred;

	// set current state
	pMiniMax->setOpponentLevel(threadNo, false);
	pMiniMax->setSituation(threadNo, layerNumber, stateNumber);
	pGuiField->setState(layerNumber, stateNumber);
	pMiniMax->getPredecessors(threadNo, &amountOfPred, predVars);

	// write number of succeding states
	wssTmp.str(L"");	wssTmp << L"number of preceeding states: " << amountOfPred;		pBranch->insertSubBranchAndItems_plainButton2D(0, treeViewItemInfo::rowHeightInPixels, *pListImages, pFont, wssTmp.str());	 
	
	// add each move possibility to list
	for (curPred=0; curPred<amountOfPred; curPred++) {

		curPreceedingLayer = predVars[curPred].predLayerNumbers;
		curPreceedingState = predVars[curPred].predStateNumbers;

		// add item in list
		if (curPreceedingState >= pMiniMax->layerStats[curPreceedingLayer].knotsInLayer) {
			// ... pTreeViewInspect->parent->showMessageBox();
			MessageBox(NULL, L"getPredecessors() returns an invalid state number.", L"ERROR", MB_OK);
		} else {
			children.push_back(new treeViewItemState{this, curPreceedingLayer, curPreceedingState });
		}
	}

	// clean up
	return true;
}
#pragma endregion

#pragma endregion

/*** To do's ************************************************************************************************************
ERLEDIGT:
- Symmetric states haben "häufig" die selben nummern! Ist möglicherweise kein Bug, sondern ein Feature da nämlich symmetrische Zustände in den Bereichen A&B jeweils die selbe Nummer bekommen.
  Dies also einfach verständlicher in der Anzeige machen. Womöglich muss Symmetrieoperation bei StateItem gespeichert werden. => Duplikate werden nun entfernt und die Liste sortiert.
- Auflistung der Preceding states implementieren
- treeViewItems müssen beim collapsen gelöscht werden oder wenigstens bei showControls(false). Beim Expanden vorher prüfen, ob die Elemente bereits existieren.

BUGS:
- manchmal absturz beim schließen

FEATURES:
- Wenn state nicht in databse dann tree item text in rot einfärben
- Database pfad muss von ini-Datei eingelesen werden. Hierfür xml-reader benutzen.
*************************************************************************************************************************/
