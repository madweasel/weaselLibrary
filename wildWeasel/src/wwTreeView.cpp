/*********************************************************************
	wwTreeView.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/

#include "wwTreeView.h"

/*************************************************************************************************************************************/

#pragma region treeView2D
//-----------------------------------------------------------------------------
// Name: create()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::treeView2D::create(masterMind* ww, float depthInSpace, buttonImageFiles &filesPlus, buttonImageFiles &filesMinus, texture *texLine)
{
	this->filesPlus						= &filesPlus;
	this->filesMinus					= &filesMinus;
	this->texLine						=  texLine;
	this->rootBranch.itr				= rowHeaders.begin();

	return listView2D::create(ww, depthInSpace);
}

//-----------------------------------------------------------------------------
// Name: row()
// Desc: ... using a pointer for rowHeaderItr is a nasty work-aorund for the rootBranch-object in treeView2D
//-----------------------------------------------------------------------------
wildWeasel::treeView2D::branch::branch(wildWeasel::treeView2D* treeView, unsigned int level, guiElement2D* elem, unsigned int height) : row{treeView, elem, height}
{
	this->level					= level;
	this->treeView				= treeView;
	this->xOffsetInFirstColumn	= level * treeView->xOffsetPerLevel;
	this->visible				= (level == 1);
}

//-----------------------------------------------------------------------------
// Name: ~row()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::treeView2D::branch::~branch()
{
	if (plusMinus) delete plusMinus;
	plusMinus = nullptr;
	//for (auto& curSubRow : subBranches) {
	//	delete curSubRow;
	//}
}

//-----------------------------------------------------------------------------
// Name: getItem()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::treeView2D::item* wildWeasel::treeView2D::branch::getItem(unsigned int columnIndex)
{
	return treeView->getItem(getRowIndex(), columnIndex);
}

//-----------------------------------------------------------------------------
// Name: assignOnExpand()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::treeView2D::branch::assignOnExpand(function<void(void*)> userFuncExpand, void * pUser)
{
	this->userFuncExpand = userFuncExpand;
	this->pUserExpand	 = pUser;
}

//-----------------------------------------------------------------------------
// Name: assignOnCollapse()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::treeView2D::branch::assignOnCollapse(function<void(void*)> userFuncCollapse, void * pUser)
{
	this->userFuncCollapse	= userFuncCollapse;
	this->pUserCollapse		= pUser;
}

//-----------------------------------------------------------------------------
// Name: getRowIndex()
// Desc: 
//-----------------------------------------------------------------------------
unsigned int wildWeasel::treeView2D::branch::getRowIndex()
{
	if (isRootBranch()) {
		return 0;
	} else {
		return distance(treeView->rowHeaders.begin(), itr);
	}
}

//-----------------------------------------------------------------------------
// Name: getNumRows()
// Desc: 
//-----------------------------------------------------------------------------
unsigned int wildWeasel::treeView2D::branch::getNumSubBranches()
{
	return subBranches.size();
}

//-----------------------------------------------------------------------------
// Name: getLastSubRow()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::treeView2D::branch* wildWeasel::treeView2D::branch::getLastSubBranch()
{
	if (subBranches.empty()) {
		return this;
	} else {
		return subBranches.back()->getLastSubBranch();
	}
}

//-----------------------------------------------------------------------------
// Name: isRootBranch()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::treeView2D::branch::isRootBranch()
{
	return treeView->rootBranch.itr == itr;
}

//-----------------------------------------------------------------------------
// Name: createPlusMinus()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::treeView2D::branch::createPlusMinus()
{
	RECT rc{0,0,1,1};
	plusMinus	= new plainButton2D();

	plusMinus->create(treeView->ww, *treeView->filesPlus, bind(&treeView2D::branch::expandOrCollapse, this, placeholders::_1), this, 0);
	plusMinus->setTargetRect(rc);
	plusMinus->setColor(wildWeasel::color::white());
	plusMinus->setPositioningMode(wildWeasel::matrixControl2D::matControlMode::posRotSca);
	plusMinus->setState(wildWeasel::guiElemState::DRAWED);
	plusMinus->setPosition (0, 0, false);
	plusMinus->setScale	(treeView->plusMinusSize, treeView->plusMinusSize, true);
	plusMinus->setClippingRect(&treeView->clippingAreaItems);
	plusMinus->informMotherOnEvent(treeView);
}

//-----------------------------------------------------------------------------
// Name: insertRow()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::treeView2D::branch* wildWeasel::treeView2D::branch::insertSubBranch(unsigned int subBranchIndex, guiElement2D* item, unsigned int height)
{
	// row	<- itr
	// 0	<- rootBranch.subBranches[0].rowHeaderItr				& rootBranch.rowHeaderItr
	// 1	<- rootBranch.subBranches[0].subBranches[0].rowHeaderItr
	// 2	<- rootBranch.subBranches[0].subBranches[1].rowHeaderItr
	// 3	<- rootBranch.subBranches[0].subBranches[2].rowHeaderItr
	// 4	<- rootBranch.subBranches[1].rowHeaderItr
	// 5	<- rootBranch.subBranches[1].subBranches[0].rowHeaderItr
	// 6	<- rootBranch.subBranches[1].subBranches[1].rowHeaderItr

	// locals
	auto					mySubRowItr		= subBranches.begin();
	list<row*>::iterator	myRowHeaderIter; 

	// create expand button if not existend yet
	if (subBranches.empty()) {
		
		if (!plusMinus && !isRootBranch()) {
			createPlusMinus();
		}

		expanded		= false;

		myRowHeaderIter	= getLastSubBranch()->itr;

	// some sub rows are already present
	} else {
		advance(mySubRowItr, min(subBranches.size(), (size_t) subBranchIndex));
		mySubRowItr--;
		myRowHeaderIter	= (*mySubRowItr)->getLastSubBranch()->itr;
		mySubRowItr++;
	}

	if (treeView->rowHeaders.size()) {
		myRowHeaderIter++;
	}

	if (item) {
		if (auto myEvFol = item->getPointer<guiElemEvFol>()) {
			myEvFol->informMotherOnEvent(treeView);
		}

		item->setClippingRect(&treeView->clippingAreaRowHeaders);
	}

	treeView->updateMarkedItem(1, 0);

	// add new row
	auto newBranch		= new branch(treeView, level + 1, item, height);
	newBranch->itr		= treeView->rowHeaders.insert(myRowHeaderIter, newBranch);
	subBranches.insert(mySubRowItr, newBranch);
	treeView->updateSledgeWidth();

	return newBranch;
}

//-----------------------------------------------------------------------------
// Name: insertSubBranchAndItems_plainButton2D()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::treeView2D::branch* wildWeasel::treeView2D::branch::insertSubBranchAndItems_plainButton2D(unsigned int subBranchIndex, unsigned int height, buttonImageFiles &imageFiles, font2D* theFont, wstring text, bool showExpandButtonAlthoughNoSubBranch)
{
	// locals
	unsigned int	curSubBranchIndex, curCol;
	branch*			newBranch;
	
	// add branch and items
	curSubBranchIndex = std::min(subBranchIndex, (unsigned int) subBranches.size());
	{
		newBranch	= insertSubBranch(subBranchIndex, nullptr, height);

		if (!newBranch->plusMinus && !newBranch->isRootBranch() && showExpandButtonAlthoughNoSubBranch) {
			newBranch->createPlusMinus();
		}

		for (curCol=0; curCol<treeView->getNumColumns(); curCol++) {
			
			wildWeasel::guiElemCluster2D*	newItem		= new wildWeasel::guiElemCluster2D();
			wildWeasel::plainButton2D*		myButton2D	= new wildWeasel::plainButton2D();
			RECT								rc;

			rc.left		= 0;
			rc.top		= 0;
			rc.bottom	= height;
			rc.right	= treeView->getColumnWidth(curCol);

			// button
			myButton2D->create(treeView->ww, imageFiles, nullptr, nullptr, 0);
			myButton2D->setTargetRect(rc);
			myButton2D->setTextSize(1.0f, 1.0f);	// ... if listView2D would be a guiElemClass then textSize could be used!!!!
			myButton2D->setText(text);
			myButton2D->setTextSize(treeView->defaultTextSize);
			myButton2D->setTextBorder(treeView->defaultTextBorder);
			myButton2D->setFont(theFont);
			myButton2D->setTextState(wildWeasel::guiElemState::DRAWED);
			myButton2D->setTextColor(wildWeasel::color::black());
			myButton2D->setColor(wildWeasel::color::white());
			myButton2D->setPositioningMode(wildWeasel::matrixControl2D::matControlMode::posRotSca);
			myButton2D->setTextAlignmentHorizontal(wildWeasel::alignmentHorizontal::LEFT);
			myButton2D->setTextAlignmentVertical(wildWeasel::alignmentVertical::CENTER);
			myButton2D->setPosition(0, 0, true);

			// cluster
			newItem->create(treeView->ww);
			newItem->addItem(myButton2D);
			newItem->setState		(wildWeasel::guiElemState::DRAWED);
			newItem->setTextStates	(wildWeasel::guiElemState::DRAWED);

			// item
			treeView->insertItem(treeView->getItemIndex(newBranch->getRowIndex(), curCol), newItem);
		}
	}

	return newBranch;
}

//-----------------------------------------------------------------------------
// Name: expandOrCollapse()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::treeView2D::branch::expandOrCollapse(void* pUser)
{
	// collapse
	if (expanded) {
		plusMinus->setImageFiles(*treeView->filesPlus);
		if (userFuncCollapse) {
			userFuncCollapse(pUserCollapse);
		}
	// expand
	} else {
		plusMinus->setImageFiles(*treeView->filesMinus);
		if (userFuncExpand) {
			userFuncExpand(pUserExpand);
		}
	}

	expanded = !expanded;

	setVisibilityOfAllSubBranches(expanded, true);

	treeView->alignAllItems();
}

//-----------------------------------------------------------------------------
// Name: setVisibilityOfAllSubBranches()
// Desc: Sets 'rowHeaderItr->visible' of all subBranches
//-----------------------------------------------------------------------------
void wildWeasel::treeView2D::branch::setVisibilityOfAllSubBranches(bool visible, bool onlyOfFirstSubLevel)
{
	for (auto& curSubBranch : subBranches) {
		curSubBranch->visible = visible;
		if (!onlyOfFirstSubLevel || visible == false)  {
			curSubBranch->setVisibilityOfAllSubBranches(visible, onlyOfFirstSubLevel);
		}
	}
}

//-----------------------------------------------------------------------------
// Name: setRowHeight()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::treeView2D::branch::setRowHeight(unsigned int height)
{
	height = height;
	treeView->updateSledgeWidth();
}

//-----------------------------------------------------------------------------
// Name: getRowHeight()
// Desc: 
//-----------------------------------------------------------------------------
unsigned int wildWeasel::treeView2D::branch::getRowHeight()
{
	return curSize;
}

//-----------------------------------------------------------------------------
// Name: removeSubBranchAndItems()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::treeView2D::branch::removeSubBranchAndItems(unsigned int subBranchIndex, bool alsoDeleteGuiElem)
{
	if (subBranchIndex >= subBranches.size()) return;
	auto curSubBranch = getSubBranch(subBranchIndex);
	curSubBranch->removeAllSubBranchesAndItems(alsoDeleteGuiElem);
	treeView->removeAllItemsInRow(curSubBranch->getRowIndex(), alsoDeleteGuiElem);	// ... should be optimized, since list is passed several times
	removeSubBranch(subBranchIndex, alsoDeleteGuiElem);
}

//-----------------------------------------------------------------------------
// Name: removeSubBranch()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::treeView2D::branch::removeSubBranch(unsigned int subBranchIndex, bool alsoDeleteGuiElem)
{
	// set itr to corresponding sub row
	if (subBranchIndex >= subBranches.size()) return;
	auto mySubRowItr = subBranches.begin();
	advance(mySubRowItr, subBranchIndex);

	// remove all sub rows of the considered sub row
	(*mySubRowItr)->removeAllSubBranches(alsoDeleteGuiElem);

	// remove row header of considered sub row
	if (alsoDeleteGuiElem && (*mySubRowItr)->elem != nullptr) delete (*mySubRowItr)->elem;
	treeView->rowHeaders.erase((*mySubRowItr)->itr);

	// remove subRow from list
	delete *mySubRowItr;
	subBranches.erase(mySubRowItr);

	// delete plus minus button if not needed any more
	if (subBranches.empty()) {
		// if (plusMinus) delete plusMinus;
		// plusMinus = nullptr;
	}

	treeView->updateSledgeWidth();
}

//-----------------------------------------------------------------------------
// Name: removeAllSubBranches()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::treeView2D::branch::removeAllSubBranches(bool alsoDeleteGuiElem)
{
	// delete each sub row and remove the corresding row header
	for (auto& curSubRow : subBranches) {
		curSubRow->removeAllSubBranches(alsoDeleteGuiElem);
		if (alsoDeleteGuiElem && curSubRow->elem != nullptr) delete curSubRow->elem;
		treeView->rowHeaders.erase(curSubRow->itr);
		delete curSubRow;
	}
	subBranches.clear();

	// since no subrow left, delete plus minus button
	// if (plusMinus) delete plusMinus;
	// plusMinus = nullptr;
}

//-----------------------------------------------------------------------------
// Name: removeAllSubBranchesAndItems()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::treeView2D::branch::removeAllSubBranchesAndItems(bool alsoDeleteGuiElem)
{
	// delete each sub row and remove the corresding row header
	for (auto& curSubRow : subBranches) {
		curSubRow->removeAllSubBranchesAndItems(alsoDeleteGuiElem);
		treeView->removeAllItemsInRow(curSubRow->getRowIndex(), alsoDeleteGuiElem);		// ... should be optimized, since list is passed several times
		if (alsoDeleteGuiElem && curSubRow->elem != nullptr) delete curSubRow->elem;
		treeView->rowHeaders.erase(curSubRow->itr);
		delete curSubRow;
	}
	subBranches.clear();
}

//-----------------------------------------------------------------------------
// Name: removeAllRows()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::treeView2D::removeAllRows(bool alsoDeleteGuiElem)
{
	listView2D::removeAllRows(alsoDeleteGuiElem);
}

//-----------------------------------------------------------------------------
// Name: setFocusOnRow()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::treeView2D::branch::setFocusOnRow()
{
	row::setFocusOnRow();
}

//-----------------------------------------------------------------------------
// Name: setRowColor()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::treeView2D::branch::setRowColor(color newColor)
{
	treeView->listView2D::setRowColor(getRowIndex(), newColor);
}

//-----------------------------------------------------------------------------
// Name: setText()
// Desc: Sets the text of the first guiElem in the first cluster.
//-----------------------------------------------------------------------------
void wildWeasel::treeView2D::branch::setText(const wchar_t * theText)
{
	getItem(0)->setText(0, theText);
}

//-----------------------------------------------------------------------------
// Name: getGiElemPointer()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::guiElement2D* wildWeasel::treeView2D::branch::getGuiElemPointer()
{
	return elem;
}

//-----------------------------------------------------------------------------
// Name: getRow()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::treeView2D::branch* wildWeasel::treeView2D::branch::getSubBranch(unsigned int rowIndex)
{
	if (rowIndex >= subBranches.size()) return nullptr;
	auto myItr = subBranches.begin();
	advance(myItr, rowIndex);
	return (*myItr);
}

//-----------------------------------------------------------------------------
// Name: updatePlusMinusPos()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::treeView2D::updatePlusMinusPos()
{
	// row header
	vector2			initPos				= vector2(transScaledTargetRect.left + 0.5f * (xOffsetPerLevel - plusMinusSize), transScaledTargetRect.top + scrollOffset.y);
	vector2			curPos				= initPos;
	auto			curRowHeaderItem	= rowHeaders.begin();
	
	getRootBranch()->updatePos(initPos, curPos);
}

//-----------------------------------------------------------------------------
// Name: updatePos()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::treeView2D::branch::updatePos(vector2& initPos, vector2& curPos)
{
	if (treeView) {
		curPos.x = initPos.x + (level - 1) * treeView->xOffsetPerLevel;
	}
	
	if (plusMinus) {
		float yBorder = 0.5f * (curSize - treeView->plusMinusSize);
		curPos.y += yBorder;
		plusMinus->setPosition(&curPos, true);
		curPos.y -= yBorder;
		// ... treeView->setElemVisibility(*minus, curPos);
	}

	if (visible) {
		curPos.y += curSize;
	}

	for (auto& curSubRow : subBranches) {
		curSubRow->updatePos(initPos, curPos);
	}
}

//-----------------------------------------------------------------------------
// Name: alignAllItems()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::treeView2D::alignAllItems()
{
	// locals
	unsigned int	counter				= 0;
	unsigned int	numColumns			= getNumColumns();
	unsigned int	numRows				= getNumRows();
	auto			curRowHeaderItem	= rowHeaders.begin();
	auto			curColHeaderItem	= columnHeaders.begin();
	vector2			initialItemPos;

	// update target rect
	alignedRect::updateTargetRect(ww->alignmentRootFrame);

	// ... scrolling could be implemented as matrix
	updateScrollOffset();

	// ... since listView2D is not a guiElem2D. transScaledTargetRect must be calculated manually.
	updateTransScaledRect();

	// matrix transformation of listView2D must be applied
	// ...

	// update clipping rect for items
	updateClippingRects();

	// row & column header
	updateColumnHeaderPos();
	updateRowHeaderPos	 ();

	// plus & minus and branch lines
	updatePlusMinusPos();

	// items
	updateItemPos();

	// scrollbars
	updateScrollBarPos();
}

#pragma endregion
