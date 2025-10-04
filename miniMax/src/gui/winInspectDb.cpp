/*********************************************************************
	miniMaxWinInspectDb.cpp
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/

#include "winInspectDb.h"

namespace miniMax {

#pragma region inspection mode

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
	treeViewInspect.getRootBranch()->removeAllSubBranchesAndItems(true);
	treeViewInspect.removeAllItems(true);
	treeViewInspect.removeAllColumns(true);
}

//-----------------------------------------------------------------------------
// Name: createControls()
// Desc: 
//-----------------------------------------------------------------------------
bool miniMaxWinInspectDb::createControls()
{
	// parameters ok?
	if (ww == nullptr) return false;
	if (treeViewInspect.isInitialized()) return false;

	// create tree view for database inspection
	treeViewInspect.create(ww, 0, buttonImagesPlus, buttonImagesMinus, textureLine);
	treeViewInspect.setAlignment(amListInspectDb);
	treeViewInspect.setSelectionMode(wildWeasel::listView2D::selectionMode::ROW_WISE);
	treeViewInspect.setMarkerColor(wildWeasel::color::lightBlue());
	treeViewInspect.setVisibilityColumnHeader(false);
	treeViewInspect.setVisibilityRowHeader(false);
	treeViewInspect.insertColumn(0, nullptr, treeViewItemInfo::colWidthInPixels);
	treeViewInspect.createScrollBars(buttonImagesArrow, buttonImagesVoid, buttonImagesVoid);
	treeViewInspect.setColumnScrollBarHeight(scrollBarWidth);
	treeViewInspect.setRowScrollBarWidth	(scrollBarWidth);
	treeViewInspect.setVisibilityColumnScrollBar(true);
	treeViewInspect.setVisibilityRowScrollBar   (true);
	treeViewInspect.setPosition(0, 0, true);
	treeViewInspect.setScale	(1, 1, true);
	treeViewInspect.setState(wildWeasel::guiElemState::HIDDEN);
	treeViewInspect.setTextBorder(3, 3);
	treeViewInspect.setTextSize(treeViewItemInfo::textSize, treeViewItemInfo::textSize);
	treeViewInspect.alignAllItems();

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
	unsigned int				curLayer;

	treeViewInspect.setState(nCmdShow);
	pGuiField->setVisibility(curShowedLayer > 0 ? visible : false);
	
	curShowedLayer				= 0;
	curShowedState				= 0;
	curShownSymOp				= SO_DO_NOTHING;
	curPlayer					= 0;
	showingInspectionControls	= visible;

	if (showingInspectionControls) {
		pGuiField->setAlignment(amFieldInspectDb);
	}

	// add layers
	if (visible) {

		// do not create tree view items twice
		if (treeViewItems.size() == 0) {
			treeViewItems.resize(pMiniMax->game->getNumberOfLayers());
			for (curLayer = 0; curLayer < pMiniMax->game->getNumberOfLayers(); curLayer++) {
				if (!pMiniMax->game->getNumberOfKnotsInLayer(curLayer)) continue;
				treeViewItems[curLayer] = new treeViewItemLayer{&treeViewInspect, treeViewInspect.getRootBranch(), &buttonImagesListItem, hFontOutputBox, curLayer, pMiniMax, pGuiField, &curShowedLayer, &curShowedState, &curShownSymOp, &curPlayer};
			}
		}
		treeViewInspect.alignAllItems();
	
	} else {

		// delete tree view items
		for (auto& curTreeViewItem : treeViewItems) {
			delete curTreeViewItem;
		}
		treeViewItems.clear();
		treeViewInspect.getRootBranch()->removeAllSubBranchesAndItems(true);
		treeViewInspect.removeAllItems(true);
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
	treeViewInspect		{ parent->treeViewInspect			} ,
	pMiniMax			{ parent->pMiniMax					} ,
	pGuiField			{ parent->pGuiField					} ,
	pCurShowedLayer		{ parent->pCurShowedLayer			} ,
	pCurShowedState		{ parent->pCurShowedState			} ,
	pCurShownSymOp		{ parent->pCurShownSymOp			} ,
	pCurPlayer			{ parent->pCurPlayer				} ,
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
void miniMaxWinInspectDb::treeViewItemInfo::writeKnotValueIntoString(wstringstream &wssTmp, unsigned int stateNumber, unsigned char symOp, unsigned int curPlayer)
{
	// locals
	float				floatValue;
	twoBit				shortKnotValue;
	plyInfoVarType		plyInfo;

	wssTmp.str(L"");
	if (pMiniMax->db.isLayerCompleteAndInFile(layerNumber)) {
		pMiniMax->db.readKnotValueFromDatabase(layerNumber, stateNumber, shortKnotValue);
		pMiniMax->db.readPlyInfoFromDatabase(layerNumber, stateNumber, plyInfo);
		wssTmp << "short value: " << ((shortKnotValue == SKV_VALUE_GAME_WON) ? "WON" : ((shortKnotValue == SKV_VALUE_GAME_LOST) ? "LOST" : ((shortKnotValue == SKV_VALUE_GAME_DRAWN) ? "DRAW" : "INVALID")));
		wssTmp << ", ply info: " << ((int) plyInfo) << ((plyInfo == PLYINFO_VALUE_DRAWN) ? " (DRAWN)" : ((plyInfo == PLYINFO_VALUE_UNCALCULATED) ? " (UNCALC)" : ((plyInfo == PLYINFO_VALUE_INVALID) ? " (INVALID)" : "")));
	} else {
		if (pMiniMax->game->setSituation(0, layerNumber, stateNumber)) {
			pGuiField->setState(layerNumber, stateNumber, symOp, curPlayer);
			pMiniMax->game->getValueOfSituation(0, floatValue, shortKnotValue);
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
miniMaxWinInspectDb::treeViewItemLayer::treeViewItemLayer(
	wildWeasel::treeView2D* treeViewInspect, 
	wildWeasel::treeView2D::branch* branch, 
	wildWeasel::buttonImageFiles* pListImages, 
	wildWeasel::font2D* pFont, 
	unsigned int layerNumber, 
	miniMax* pMiniMax, 
	miniMaxGuiField* pGuiField, 
	unsigned int* pCurShowedLayer, 
	stateNumberVarType* pCurShowedState, 
	unsigned char* pCurShownSymOp,
	unsigned int* curPlayer)
{
	// locals
	wstringstream				wssTmp;			
	
	wssTmp						<< L"layer: " << layerNumber;
	this->treeViewInspect		= treeViewInspect;
	this->pListImages			= pListImages;
	this->pFont					= pFont;
	this->layerNumber			= layerNumber;
	this->pGuiField				= pGuiField;
	this->pMiniMax				= pMiniMax;
	this->numberOfKnotsInLayer	= pMiniMax->game->getNumberOfKnotsInLayer(layerNumber);
	this->pCurShowedLayer		= pCurShowedLayer;
	this->pCurShowedState		= pCurShowedState;
	this->pCurShownSymOp		= pCurShownSymOp;
	this->pCurPlayer			= curPlayer;
	this->pBranch				= branch->insertSubBranchAndItems_plainButton2D(wildWeasel::treeView2D::branch::LAST_SUB_BRANCH, treeViewItemInfo::rowHeightInPixels, *pListImages, pFont, wssTmp.str(), true);
	this->type					= classType::layer;

	this->pBranch->assignOnExpand  (bind(&treeViewItemLayer::expand,   this), nullptr);
	this->pBranch->assignOnCollapse(bind(&treeViewItemLayer::collapse, this), nullptr);
	if (pMiniMax->db.isLayerCompleteAndInFile(layerNumber)) {
		this->pBranch->setRowColor(wildWeasel::color::green());
	} else {
		this->pBranch->setRowColor(wildWeasel::color::red());
	}
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
	wssTmp.str(L"");		wssTmp << L"total number of knots    : "	<< pMiniMax->game->getNumberOfKnotsInLayer(layerNumber);							pBranch->insertSubBranchAndItems_plainButton2D(0, treeViewItemInfo::rowHeightInPixels, *pListImages, pFont, wssTmp.str());	 
	wssTmp.str(L"");		wssTmp << L"additional information   : "	<< pMiniMax->game->getOutputInformation(layerNumber);								pBranch->insertSubBranchAndItems_plainButton2D(1, treeViewItemInfo::rowHeightInPixels, *pListImages, pFont, wssTmp.str());	 
	wssTmp.str(L"");		wssTmp << L"number of won states     : "	<< pMiniMax->db.getNumWonStates(layerNumber);										pBranch->insertSubBranchAndItems_plainButton2D(2, treeViewItemInfo::rowHeightInPixels, *pListImages, pFont, wssTmp.str());	 
	wssTmp.str(L"");		wssTmp << L"number of lost states      : "	<< pMiniMax->db.getNumLostStates(layerNumber);										pBranch->insertSubBranchAndItems_plainButton2D(3, treeViewItemInfo::rowHeightInPixels, *pListImages, pFont, wssTmp.str());	 
	wssTmp.str(L"");		wssTmp << L"number of drawn states  : "		<< pMiniMax->db.getNumDrawnStates(layerNumber);										pBranch->insertSubBranchAndItems_plainButton2D(4, treeViewItemInfo::rowHeightInPixels, *pListImages, pFont, wssTmp.str());	 
	wssTmp.str(L"");		wssTmp << L"number of invalid states : "	<< pMiniMax->db.getNumInvalidStates(layerNumber);									pBranch->insertSubBranchAndItems_plainButton2D(5, treeViewItemInfo::rowHeightInPixels, *pListImages, pFont, wssTmp.str());	 

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
	// Calculate the number of digits in the prefix before the current state digit.
	// This helps format the state number display with missing digits represented by 'x'.
	unsigned int			prefixDigits		= (unsigned int) (stateNumberPrefix / (10*newStepSize));
	wstringstream			wssTmp;

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
			tvii.children.push_back(new treeViewItemState{&tvii, stateNumberPrefix + curStateDigit, SO_DO_NOTHING, PLAYER_ONE});
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
miniMaxWinInspectDb::treeViewItemState::treeViewItemState(treeViewItemInfo *parent, unsigned int stateNumber, unsigned char symOp, unsigned int curPlayer) : treeViewItemState{parent, parent->layerNumber, stateNumber, symOp, curPlayer}
{
}

//-----------------------------------------------------------------------------
// Name: treeViewItemState()
// Desc: 
//-----------------------------------------------------------------------------
miniMaxWinInspectDb::treeViewItemState::treeViewItemState(treeViewItemInfo *parent, unsigned int layerNumber, unsigned int stateNumber, unsigned char symOp, unsigned int curPlayer) : treeViewItemInfo{parent}
{
	wstringstream			wssTmp;

	wssTmp.str(L"");		
	if (parent->layerNumber != layerNumber) wssTmp << L"layer: " << layerNumber << L", ";
	wssTmp << L"state: " << stateNumber;
	wssTmp << L", symOp: " << symOp;
	wssTmp << L", player: " << curPlayer;

	this->type				= classType::state;
	this->stateNumber		= stateNumber;
	this->layerNumber		= layerNumber;
	this->symOp				= symOp;
	this->curPlayer			= curPlayer;
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
		*pCurShownSymOp		= SO_DO_NOTHING;
		*pCurPlayer			= 0;
	// show current selected state
	} else {
		*pCurShowedLayer	= layerNumber;
		*pCurShowedState	= stateNumber;
		*pCurShownSymOp		= symOp;
		*pCurPlayer			= curPlayer;
	}

	pGuiField->setVisibility(true);
	pGuiField->setState(*pCurShowedLayer, *pCurShowedState, *pCurShownSymOp, *pCurPlayer);

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
	*pCurShownSymOp		= SO_DO_NOTHING;
	*pCurPlayer			= 0;

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
	unsigned int  *		symStateNumbers		= nullptr;
	unsigned int		numSymmetricStates	= 0;

	writeKnotValueIntoString(wssTmp, stateNumber, symOp, curPlayer);			
	pBranch->insertSubBranchAndItems_plainButton2D(0, treeViewItemInfo::rowHeightInPixels, *pListImages, pFont, wssTmp.str());	 

	if (pMiniMax->game->setSituation(0, layerNumber, stateNumber)) {
		pGuiField->setState(layerNumber, stateNumber, symOp, curPlayer);
		children.push_back(new treeViewItemSym {this, stateNumber, symOp, curPlayer});
		children.push_back(new treeViewItemSuc {this, stateNumber, symOp, curPlayer});
		children.push_back(new treeViewItemPrec{this, stateNumber, symOp, curPlayer});
	}

	return true;
}
#pragma endregion

#pragma region treeViewItemSym
//-----------------------------------------------------------------------------
// Name: treeViewItemSym()
// Desc: 
//-----------------------------------------------------------------------------
miniMaxWinInspectDb::treeViewItemSym::treeViewItemSym(treeViewItemInfo *parent, unsigned int stateNumber, unsigned char symOp, unsigned int curPlayer) : treeViewItemInfo{parent}
{
	wstringstream			wssTmp;

	wssTmp.str(L"");		wssTmp << L"symmetric states:";

	this->stateNumber		= stateNumber;
	this->symOp				= symOp;
	this->curPlayer			= curPlayer;
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
	pMiniMax->game->setSituation(0, layerNumber, stateNumber);
	pGuiField->setState(layerNumber, stateNumber, symOp, curPlayer);

	// get symmetric states
	vector<stateAdressStruct> symStates;
	pMiniMax->game->getSymStateNumWithDuplicates(0, symStates);

	// remove duplicates
	sort( symStates.begin(), symStates.end() );
	symStates.erase( unique( symStates.begin(), symStates.end() ), symStates.end() );
	
	// print number of symmetric states
	wssTmp.str(L"");	wssTmp << L"number of symmetric states: " << symStates.size();	 pBranch->insertSubBranchAndItems_plainButton2D(0, treeViewItemInfo::rowHeightInPixels, *pListImages, pFont, wssTmp.str());	 

	// add each symmetric state to list
	for (auto& curSymmetricState : symStates) {
		//if (curSymmetricState != stateNumber) {
			if (curSymmetricState.stateNumber >= pMiniMax->db.getNumberOfKnots(layerNumber)) {
				// ... treeViewInspect->parent->showMessageBox();
				MessageBox(NULL, L"getSymStateNumWithDuplicates() returns an invalid state number.", L"ERROR", MB_OK);
			} else {
				children.push_back(new treeViewItemState{this, curSymmetricState.stateNumber, SO_DO_NOTHING, PLAYER_ONE});
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
miniMaxWinInspectDb::treeViewItemSuc::treeViewItemSuc(treeViewItemInfo *parent, unsigned int stateNumber, unsigned char symOp, unsigned int curPlayer) : treeViewItemInfo{parent}
{
	wstringstream			wssTmp;

	wssTmp.str(L"");		wssTmp << L"succeding states:";

	this->stateNumber		= stateNumber;
	this->symOp				= symOp;
	this->curPlayer			= curPlayer;
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
	unsigned int			curSuccedingSymOp;
	unsigned int 			curSuccedingPlayer;
	wstringstream			wssTmp;
	void *					pBackup;
	void *					pPossibilities;
	bool					playerToMoveChanged;
	unsigned int			curPossibility;
	vector<unsigned int>	possibilityIds;

	// set current state
	pMiniMax->game->setSituation(threadNo, layerNumber, stateNumber);
	pGuiField->setState(layerNumber, stateNumber, symOp, curPlayer);
	pMiniMax->game->getPossibilities(threadNo, possibilityIds);

	// write number of succeding states
	wssTmp.str(L"");	wssTmp << L"number of succeding states: " << possibilityIds.size();		pBranch->insertSubBranchAndItems_plainButton2D(0, treeViewItemInfo::rowHeightInPixels, *pListImages, pFont, wssTmp.str());	 
	
	// add each move possibility to list
	for (curPossibility=0; curPossibility<possibilityIds.size(); curPossibility++) {

		// do move
		pMiniMax->game->move(threadNo, possibilityIds[curPossibility], playerToMoveChanged, pBackup);
		pMiniMax->game->getLayerAndStateNumber(threadNo, curSuccedingLayer, curSuccedingState, curSuccedingSymOp);
		curSuccedingPlayer = playerToMoveChanged ? (curPlayer + 1) % 2: curPlayer;

		// add item in list
		if (curSuccedingState >= pMiniMax->db.getNumberOfKnots(curSuccedingLayer)) {
			// ... treeViewInspect->parent->showMessageBox();
			MessageBox(NULL, L"getLayerAndStateNumber() returns an invalid state number.", L"ERROR", MB_OK);
		} else {
			children.push_back(new treeViewItemState{this, curSuccedingLayer, curSuccedingState, (unsigned char) curSuccedingSymOp, curSuccedingPlayer});
		}

		// undo move
		pMiniMax->game->undo(threadNo, possibilityIds[curPossibility], playerToMoveChanged, pBackup);
	}

	// clean up
	return true;
}
#pragma endregion

#pragma region treeViewItemPrec
//-----------------------------------------------------------------------------
// Name: treeViewItemState()
// Desc: 
//-----------------------------------------------------------------------------
miniMaxWinInspectDb::treeViewItemPrec::treeViewItemPrec(treeViewItemInfo *parent, unsigned int stateNumber, unsigned char symOp, unsigned int curPlayer) : treeViewItemInfo{parent}
{
	wstringstream			wssTmp;

	wssTmp.str(L"");		wssTmp << L"preceeding states:";

	this->stateNumber		= stateNumber;
	this->symOp				= symOp;
	this->curPlayer			= curPlayer;
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
	unsigned char					curPreceedingSymOp;
	unsigned int					curPreceedingPlayer;
	wstringstream					wssTmp;
	vector<retroAnalysis::predVars> predVars;
	unsigned int					curPred;

	// set current state
	pMiniMax->game->setSituation(threadNo, layerNumber, stateNumber);
	pGuiField->setState(layerNumber, stateNumber, symOp, curPlayer);
	pMiniMax->game->getPredecessors(threadNo, predVars);

	// write number of succeding states
	wssTmp.str(L"");	wssTmp << L"number of preceeding states: " << predVars.size();		pBranch->insertSubBranchAndItems_plainButton2D(0, treeViewItemInfo::rowHeightInPixels, *pListImages, pFont, wssTmp.str());	 
	
	// add each move possibility to list
	for (curPred=0; curPred<predVars.size(); curPred++) {

		curPreceedingLayer = predVars[curPred].predLayerNumber;
		curPreceedingState = predVars[curPred].predStateNumber;
		curPreceedingSymOp = predVars[curPred].predSymOperation;
		curPreceedingPlayer = predVars[curPred].playerToMoveChanged ? (curPlayer + 1) % 2: curPlayer;

		// add item in list
		if (curPreceedingState >= pMiniMax->db.getNumberOfKnots(curPreceedingLayer)) {
			// ... treeViewInspect->parent->showMessageBox();
			MessageBox(NULL, L"getPredecessors() returns an invalid state number.", L"ERROR", MB_OK);
		} else {
			children.push_back(new treeViewItemState{this, curPreceedingLayer, curPreceedingState, curPreceedingSymOp, curPreceedingPlayer });
		}
	}

	// clean up
	return true;
}
#pragma endregion

#pragma endregion

} // namespace miniMax
