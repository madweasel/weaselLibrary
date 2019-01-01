/*********************************************************************
	wwguiElement.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/

#include "wwListView.h"

/*************************************************************************************************************************************/

#pragma region listView2D
//-----------------------------------------------------------------------------
// Name: create()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::listView2D::create(masterMind* ww, float depthInSpace)
{
	// params ok?
	if (!ww)			return false;
	if (initialized)	return false;
	
//	// positioning
//	zPosition = depthInSpace;
//
//	// call back function
//	this->parent		= ww;
//	this->windowSize	= &ww->windowSize;

//	// remember to load the file later on together with the other ressources
//	ww->threeD.addObjectToLoad(this);
//	ww->registerGuiElement(this);
	guiElemCluster2D::create(ww);

	// track mouse moves and clicks
	eventFollower::followEvent(this, eventType::WINDOWSIZE_CHANGED);
	
	initialized = true;
	return initialized;
}

//-----------------------------------------------------------------------------
// Name: setPositioningMode()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::setPositioningMode(posMode newMode)
{
	positioningMode = newMode;
}

//-----------------------------------------------------------------------------
// Name: setPositioningMode()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::setMarkerColor(color newMarkerColor)
{
	markerColor = newMarkerColor;
}

//-----------------------------------------------------------------------------
// Name: setSelectionMode()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::setSelectionMode(selectionMode newMode)
{
	markerSelectionMode = newMode;
}

//-----------------------------------------------------------------------------
// Name: setTextSize()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::setTextSize(float sx, float sy)
{
	for (auto& curItem : items) {
		if (curItem.subItems) curItem.subItems->setTextSize(sx, sy);
	}
	// for (auto& curCol: columnHeaders) {
	// 	if (curCol.elem) curCol.elem->setTextSize(sx, sy);
	// }
	// for (auto& curRow : rowHeaders) {
	// 	if (curRow.elem) curRow.elem->setTextSize(sx, sy);
	// }

	defaultTextSize = vector2{sx, sy};
}

//-----------------------------------------------------------------------------
// Name: setTextBorder()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::setTextBorder(float bx, float by)
{
	for (auto& curItem : items) {
		if (curItem.subItems) curItem.subItems->setTextBorder(bx, by);
	}
	// for (auto& curCol: columnHeaders) {
	// 	if (curCol.elem) curCol.elem->setTextSize(sx, sy);
	// }
	// for (auto& curRow : rowHeaders) {
	// 	if (curRow.elem) curRow.elem->setTextSize(sx, sy);
	// }

	defaultTextBorder = vector2{bx, by};
}

//-----------------------------------------------------------------------------
// Name: setSelectionMode()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::setState(guiElemState newStatus)
{
	// locals
	bool visible = isGuiElemStateVisible(newStatus);

	for (auto& curItem : items) {
		if (curItem.subItems) curItem.subItems->setState(newStatus);
	}
	for (auto& curCol: columnHeaders) {
		if (curCol.elem) curCol.elem->setState(columnHeaderIsVisible && visible ? guiElemState::VISIBLE : guiElemState::HIDDEN);
	}
	for (auto& curRow : rowHeaders) {
		if (curRow->elem) curRow->elem->setState(rowHeaderIsVisible && visible ? guiElemState::VISIBLE : guiElemState::HIDDEN);
	}
	if (columnScrollBar	) columnScrollBar	->setState(columnScrollBarIsVisible && visible ? guiElemState::DRAWED : guiElemState::HIDDEN);
	if (rowScrollBar	) rowScrollBar		->setState(rowScrollBarIsVisible	&& visible ? guiElemState::DRAWED : guiElemState::HIDDEN);

	// COMMENT: setVisibilityColumnHeader(), ... must not be used since variable 'columnHeaderIsVisible', ... shall not be changed.
}

//-----------------------------------------------------------------------------
// Name: windowSizeChanged()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::windowSizeChanged(int xSize, int ySize)
{
	alignAllItems();
	updateSledgeWidth();
}

//-----------------------------------------------------------------------------
// Name: keyDown()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::keyDown(int keyCode)
{
	scrollBarDuo::keyDown(keyCode);
}

//-----------------------------------------------------------------------------
// Name: verticalWheelMoved()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::verticalWheelMoved(int distance)
{
	scrollBarDuo::keyDown(distance < 0 ? VK_DOWN : VK_UP);
}

//-----------------------------------------------------------------------------
// Name: horizontalWheelMoved()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::horizontalWheelMoved(int distance)
{
	scrollBarDuo::keyDown(distance < 0 ? VK_LEFT : VK_RIGHT);
}

//-----------------------------------------------------------------------------
// Name: pixelsToScroll()
// Desc: 
//-----------------------------------------------------------------------------
float wildWeasel::listView2D::pixelsToScroll(scrollBarDuo::direction dir)
{
	float curPos = 0;

	switch (dir)
	{
	case scrollBarDuo::direction::LEFT:
		for (auto& curColHeaderItem : columnHeaders) {
			curPos += curColHeaderItem.curSize;
			if (curPos > abs(scrollOffset.x) - 0.01f) {
				return curPos - curColHeaderItem.curSize + scrollOffset.x;
			}
		}
	case scrollBarDuo::direction::RIGHT:
		for (auto& curColHeaderItem : columnHeaders) {
			curPos += curColHeaderItem.curSize;
			if (curPos > abs(scrollOffset.x) + 0.01f) {
				return (curPos + scrollOffset.x);
			}
		}
	case scrollBarDuo::direction::UP:
		for (auto& curRowHeaderItem : rowHeaders) {
			curPos += curRowHeaderItem->curSize;
			if (curPos > abs(scrollOffset.y) - 0.01f) {
				return curPos - curRowHeaderItem->curSize + scrollOffset.y;
			}
		}
	case scrollBarDuo::direction::DOWN:
		for (auto& curRowHeaderItem : rowHeaders) {
			curPos += curRowHeaderItem->curSize;
			if (curPos > abs(scrollOffset.y) + 0.01f) {
				return (curPos + scrollOffset.y);
			}
		}
	}

	return 0;
}

//-----------------------------------------------------------------------------
// Name: createScrollBars()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::listView2D::createScrollBars(buttonImageFiles &filesTriangle, buttonImageFiles &filesLane, buttonImageFiles &filesSledge)
{
	// create
	scrollBarDuo::createScrollBars(this, filesTriangle, filesLane, filesSledge, ww, mat, dirtyBit, 0);
	return true;
}

//-----------------------------------------------------------------------------
// Name: updateSledgeWidth()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::updateSledgeWidth()
{
	// ... could be calculated incrementally by each function
	visibleAreaSize.x	= (transScaledTargetRect.right - transScaledTargetRect.left) - (float) ((rowHeaderIsVisible?rowHeaderWidth:0) + (rowScrollBarIsVisible?rowScrollBarWidth:0));
	totalAreaSize.x		= 0;
	for (auto& curCol : columnHeaders) {
		totalAreaSize.x += curCol.curSize;
	}
	
	// ... could be calculated incrementally by each function
	visibleAreaSize.y	= (transScaledTargetRect.bottom - transScaledTargetRect.top) - (float) ((columnHeaderIsVisible?columnHeaderHeight:0) + (columnScrollBarIsVisible?columnScrollbarHeight:0));
	totalAreaSize.y		= 0;
	for (auto& curRow : rowHeaders) {
		if (curRow->visible) {
			totalAreaSize.y += curRow->curSize;
		}
	}

	if (scrollBarsCreated) {
		columnScrollBar	->setSledgeWidth(visibleAreaSize.x / totalAreaSize.x);
		rowScrollBar	->setSledgeWidth(visibleAreaSize.y / totalAreaSize.y);
	}
}

//-----------------------------------------------------------------------------
// Name: columnScrollBarMoved()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::columnScrollBarMoved(scrollBar2D* bar, void* pUser)
{
	alignAllItems();
}

//-----------------------------------------------------------------------------
// Name: rowScrollBarMoved()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::rowScrollBarMoved(scrollBar2D* bar, void* pUser)
{
	alignAllItems();
}

//-----------------------------------------------------------------------------
// Name: setColumnHeaderHeight()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::setColumnHeaderHeight(unsigned int newHeight)
{
	columnHeaderHeight = newHeight;
	updateSledgeWidth();
}

//-----------------------------------------------------------------------------
// Name: setRowHeaderWidth()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::setRowHeaderWidth(unsigned int newWidth)
{
	rowHeaderWidth = newWidth;
	updateSledgeWidth();
}

//-----------------------------------------------------------------------------
// Name: setVisibilityColumnHeader()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::setVisibilityColumnHeader(bool isVisible)
{
	// calc useful header height
	if (columnHeaderHeight == 0) {
	//	columnHeaderHeight = ...
	}

	// set visibility of each header item
	for (auto& curColHeader : columnHeaders) {
		if (curColHeader.elem != nullptr) {
			curColHeader.elem->setState(isVisible ? guiElemState::VISIBLE : guiElemState::HIDDEN);
		}
	}

	updateSledgeWidth();
	columnHeaderIsVisible = isVisible;
}

//-----------------------------------------------------------------------------
// Name: setVisibilityRowHeader()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::setVisibilityRowHeader(bool isVisible)
{
	// calc useful header height
	if (rowHeaderWidth == 0) {
	//	rowHeaderWidth = ...
	}

	// set visibility of each header item
	for (auto& curRowHeader : rowHeaders) {
		if (curRowHeader->elem != nullptr) {
			curRowHeader->elem->setState(isVisible ? guiElemState::VISIBLE : guiElemState::HIDDEN);
		}
	}

	updateSledgeWidth();
	rowHeaderIsVisible = isVisible;
}

//-----------------------------------------------------------------------------
// Name: getNumColumns()
// Desc: 
//-----------------------------------------------------------------------------
unsigned int wildWeasel::listView2D::getNumColumns()
{
	return columnHeaders.size();
}

//-----------------------------------------------------------------------------
// Name: insertColumn()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::insertColumn(unsigned int columnIndex, guiElement2D* item, unsigned int width, float autoWidthFraction)
{
	// locals
	column	myCHI			{this, item, width, autoWidthFraction};
	auto	curHeaderItem	{columnHeaders.begin()};

	if (item) {
		auto myEvFol = item->getPointer<guiElemEvFol>();
		if (myEvFol) {
			myEvFol->informMotherOnEvent(this);
		}
	}

	if (myCHI.elem) {
		myCHI.elem->setClippingRect(&clippingAreaColumnHeaders);
	}

	advance(curHeaderItem, columnIndex);
	updateMarkedItem(0, 1);
	myCHI.itr = columnHeaders.insert(curHeaderItem, myCHI);
	(*myCHI.itr).itr = myCHI.itr;
	updateSledgeWidth();
}

//-----------------------------------------------------------------------------
// Name: insertColumn_plainButton2D()
// Desc: 
// autoWidthFraction: A value (>=0) indicating how much the column width shall be changed, when the size of the list view is changed. A value equal zero means static column width. The fractions along all coumns are normalized and the width changes distributed accordingly.
//-----------------------------------------------------------------------------			
void wildWeasel::listView2D::insertColumn_plainButton2D(unsigned int columnIndex, wstring& text, font2D* theFont, unsigned int widthColumn, float angle, float textSize, RECT& rcButton, buttonImageFiles& imageFiles, float autoWidthFraction)
{
	// locals
	wildWeasel::plainButton2D*	newColumn	= new wildWeasel::plainButton2D();

	newColumn->create(ww, imageFiles, nullptr, nullptr, 0);
	newColumn->setTextSize(textSize, textSize);
	newColumn->setTextState(wildWeasel::guiElemState::DRAWED);
	newColumn->setTextColor(wildWeasel::color::black);
	newColumn->setColor(wildWeasel::color::white);
	newColumn->setPositioningMode(wildWeasel::matrixControl2D::matControlMode::posRotSca);
	newColumn->setTextAlignmentHorizontal(wildWeasel::alignmentHorizontal::LEFT);
	newColumn->setTextAlignmentVertical(wildWeasel::alignmentVertical::CENTER);
	newColumn->setFont(theFont); 
	newColumn->setText(text);	
	newColumn->setTextSize(defaultTextSize);
	newColumn->setTextBorder(defaultTextBorder);
	newColumn->setTargetRect(rcButton);	
	newColumn->setPosition(0, 0, false);
	newColumn->setRotation(angle, true);	

	this->insertColumn(columnIndex, newColumn, widthColumn, autoWidthFraction);
}

//-----------------------------------------------------------------------------
// Name: setColumnWidth()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::setColumnWidth(unsigned int columnIndex, unsigned int width)
{
	// locals
	auto			curHeaderItem	= columnHeaders.begin();
	advance(curHeaderItem, columnIndex);
	curHeaderItem->sizeSetByUser	= width;
	curHeaderItem->curSize			= width;
	curHeaderItem->minSize			= width;
	updateSledgeWidth();
}
			
//-----------------------------------------------------------------------------
// Name: getColumnWidth()
// Desc: 
//-----------------------------------------------------------------------------
unsigned int wildWeasel::listView2D::getColumnWidth(unsigned int columnIndex)
{
	return getColumn(columnIndex)->curSize;
}

//-----------------------------------------------------------------------------
// Name: getColumnGuiElemPointer()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::guiElement2D* wildWeasel::listView2D::getColumnGuiElemPointer(unsigned int columnIndex)
{
	// locals
	auto curHeaderItem	= columnHeaders.begin();
	advance(curHeaderItem, columnIndex);
	return curHeaderItem->elem;
}

//-----------------------------------------------------------------------------
// Name: removeColumn()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::removeColumn(unsigned int columnIndex,	bool alsoDeleteColumn)
{
	unsigned int	counter	= 0;

	for (auto& curHeaderItem = columnHeaders.begin(); curHeaderItem != columnHeaders.end(); curHeaderItem++, counter++) {
		if (counter == columnIndex) {
			if (alsoDeleteColumn && curHeaderItem->elem != nullptr) delete curHeaderItem->elem;
			updateMarkedItem(0, -1);
			columnHeaders.erase(curHeaderItem);
			break;
		}
	}
	updateSledgeWidth();
}

//-----------------------------------------------------------------------------
// Name: removeAllColumns()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::removeAllColumns(bool alsoDeleteColumn)
{
	if (alsoDeleteColumn) {
		for (auto& curHeaderItem : columnHeaders) {
			if (curHeaderItem.elem != nullptr) delete curHeaderItem.elem;
		}
	}
	columnHeaders.clear();
	markedItem = (items.size() ? &items.front() : nullptr);
	updateSledgeWidth();
}


//-----------------------------------------------------------------------------
// Name: getNumRows()
// Desc: 
//-----------------------------------------------------------------------------
unsigned int wildWeasel::listView2D::getNumRows()
{
	return rowHeaders.size();
}

//-----------------------------------------------------------------------------
// Name: insertRow()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::listView2D::row* wildWeasel::listView2D::insertRow(row* insertAfterRow, guiElement2D* item, unsigned int height, float autoHeightFraction)
{
	if (item) {
		if (auto myEvFol = item->getPointer<guiElemEvFol>()) {
			myEvFol->informMotherOnEvent(this);
		}

		item->setClippingRect(&clippingAreaRowHeaders);
	}

	list<row*>::iterator insertBeforeItr;
	if (insertAfterRow) {
		insertBeforeItr = insertAfterRow->itr;
		insertBeforeItr++;
	} else {
		insertBeforeItr = rowHeaders.begin();
	}

	updateMarkedItem(1, 0);
	auto ret = rowHeaders.insert(insertBeforeItr, new row(this, item, height, autoHeightFraction));
	(*ret)->itr = ret;
	updateSledgeWidth();
	return *ret;
}

//-----------------------------------------------------------------------------
// Name: insertRow()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::listView2D::row* wildWeasel::listView2D::insertRow(unsigned int rowIndex, guiElement2D* item, unsigned int height, float autoHeightFraction)
{
	row* insertAfteRow = nullptr;

	if (rowHeaders.size() && rowIndex) {
		auto itr = rowHeaders.begin();
		advance(itr, min((size_t) rowIndex, rowHeaders.size()) - 1);
		insertAfteRow = (*itr);
	}

	return insertRow(insertAfteRow, item, height, autoHeightFraction);
}

//-----------------------------------------------------------------------------
// Name: insertRowsAndItems_plainButton2D()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::insertRowsAndItems_plainButton2D(unsigned int afterRowIndex, unsigned int numRows, unsigned int height, buttonImageFiles &imageFiles, font2D* theFont)
{
	// locals
	unsigned int curRow, curCol;
	
	// Add rows to zone list
	for (curRow=afterRowIndex; curRow<afterRowIndex+numRows; curRow++) {
		insertRow(curRow, nullptr, height);

		for (curCol=0; curCol<getNumColumns(); curCol++) {
			
			wildWeasel::guiElemCluster2D*	newItem		= new wildWeasel::guiElemCluster2D();
			wildWeasel::plainButton2D*		myButton2D	= new wildWeasel::plainButton2D();
			RECT								rc;

			rc.left		= 0;
			rc.top		= 0;
			rc.bottom	= getRow   (curRow)->sizeSetByUser;
			rc.right	= getColumn(curCol)->sizeSetByUser;

			myButton2D->create(ww, imageFiles, nullptr, nullptr, 0);
			myButton2D->setTargetRect(rc);
			myButton2D->setTextSize(1.0f, 1.0f);	// ... if listView2D would be a guiElemClass then textSize could be used!!!!
			myButton2D->setText(L"");
			myButton2D->setTextSize(defaultTextSize);
			myButton2D->setTextBorder(defaultTextBorder);
			myButton2D->setFont(theFont);
			myButton2D->setTextState(wildWeasel::guiElemState::DRAWED);
			myButton2D->setTextColor(wildWeasel::color::black);
			myButton2D->setColor(wildWeasel::color::white);
			myButton2D->setPositioningMode(wildWeasel::matrixControl2D::matControlMode::posRotSca);
			myButton2D->setTextAlignmentHorizontal(wildWeasel::alignmentHorizontal::CENTER);
			myButton2D->setTextAlignmentVertical(wildWeasel::alignmentVertical::CENTER);
			myButton2D->setPosition(0, 0, true);

			newItem->create(ww);
			newItem->addItem(myButton2D);
			newItem->setState		(wildWeasel::guiElemState::DRAWED);
			newItem->setTextStates	(wildWeasel::guiElemState::DRAWED);
			insertItem(getItemIndex(curRow, curCol), newItem);
		}
	}
}

//-----------------------------------------------------------------------------
// Name: insertRow()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::setItemTextAlignmentHorizontal(alignmentHorizontal newAlign)
{
	for (auto& curItem : items) {
		for (auto& subItem : curItem.subItems->items) {
			subItem->setTextAlignmentHorizontal(wildWeasel::alignmentHorizontal::CENTER);
		}
	}
}

//-----------------------------------------------------------------------------
// Name: insertRow()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::setRowHeight(unsigned int rowIndex, unsigned int height)
{
	auto myRow = getRow(rowIndex);
	myRow->sizeSetByUser	= height;
	myRow->curSize			= height;
	myRow->minSize			= height;
	updateSledgeWidth();
}

//-----------------------------------------------------------------------------
// Name: getRowHeight()
// Desc: 
//-----------------------------------------------------------------------------
unsigned int wildWeasel::listView2D::getRowHeight(unsigned int rowIndex)
{
	return getRow(rowIndex)->curSize;
}

//-----------------------------------------------------------------------------
// Name: getRowGuiElemPointer()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::guiElement2D* wildWeasel::listView2D::getRowGuiElemPointer(unsigned int rowIndex)
{
	return getRow(rowIndex)->elem;
}

//-----------------------------------------------------------------------------
// Name: getRow()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::listView2D::row* wildWeasel::listView2D::getRow(unsigned int rowIndex)
{
	if (rowIndex >= rowHeaders.size()) return nullptr;
	auto	curHeaderItem	= rowHeaders.begin();
	advance(curHeaderItem, rowIndex);
	return *curHeaderItem;
}

//-----------------------------------------------------------------------------
// Name: getColumn()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::listView2D::column* wildWeasel::listView2D::getColumn(unsigned int columnIndex)
{
	if (columnIndex >= columnHeaders.size()) return nullptr;
	auto	curHeaderItem	= columnHeaders.begin();
	advance(curHeaderItem, columnIndex);
	return &(*curHeaderItem);
}

//-----------------------------------------------------------------------------
// Name: deleteRow()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::removeRow(unsigned int rowIndex, bool alsoDeleterow)
{
	row* myRow = getRow(rowIndex);
	if (alsoDeleterow && myRow->elem != nullptr) delete myRow->elem;
	updateMarkedItem(-1, 0);
	rowHeaders.erase(myRow->itr);
	delete myRow;
	updateSledgeWidth();
}

//-----------------------------------------------------------------------------
// Name: deleteAllRows()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::removeAllRows(bool alsoDeleterow)
{
	for (auto& curHeaderItem : rowHeaders) {
		if (alsoDeleterow) {
			 if (curHeaderItem->elem != nullptr) delete curHeaderItem->elem;
		}
		 delete curHeaderItem;
	}
	rowHeaders.clear();
	markedItem	= nullptr;
	updateSledgeWidth();
}

//-----------------------------------------------------------------------------
// Name: getItemCount()
// Desc: 
//-----------------------------------------------------------------------------
unsigned int wildWeasel::listView2D::getItemCount()
{
	return items.size();
}


//-----------------------------------------------------------------------------
// Name: setItemText()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::setItemText(unsigned int rowIndex, unsigned int columnIndex, unsigned int subItemIndex, const WCHAR* theText)
{
	setItemText(getItemIndex(rowIndex, columnIndex), subItemIndex, theText);
}
	
//-----------------------------------------------------------------------------
// Name: setItemText()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::setItemText(unsigned int itemIndex, unsigned int subItemIndex, const WCHAR* theText)
{
	// locals
	guiElemCluster* myCluster;
	unsigned int counter	= 0;

	for (auto& curItem = items.begin(); curItem != items.end(); curItem++, counter++) {
		if (counter == itemIndex) {
			myCluster = (*curItem).subItems;
			if (myCluster == nullptr) {
				break;
			}
			for (auto& curSubItem = myCluster->items.begin(); curSubItem != myCluster->items.end(); curSubItem++, counter++) {
				if (counter == itemIndex) {
					if (*curSubItem != nullptr) {
						(*curSubItem)->setText(theText);
					}
					break;
				}
			}
			break;
		}
	}
}

//-----------------------------------------------------------------------------
// Name: insertItem()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::insertItem(unsigned int itemIndex, guiElemCluster2D* newItem)
{
	// locals
	item myLVI;
	auto curItem			= items.begin();

	if (itemIndex > items.size()) itemIndex = items.size();

	// add cluster to list view item list
	myLVI.listView = this;
	myLVI.subItems = newItem;
	myLVI.subItems->setClippingRect(&clippingAreaItems);
	advance(curItem, itemIndex);
	myLVI.itr = items.insert(curItem, myLVI);
	(*myLVI.itr).itr = myLVI.itr;

	if (items.size() == 1) 	this->markedItem	= &(items.front());
	
	// attach the list view matrix to each sub item
	for (auto& curSubItem : newItem->items) {
		curSubItem->insertMatrix(100000, &mat, &dirtyBit);

		// if (curSubItem->typeId & guiElemTypes::eventFollower) {
		if (typeid(*curSubItem) == typeid(plainButton2D/*guiElemEvFol2D*/)) {	// <-- ... very very dirty !!!
			guiElemEvFol2D* myEvFol = (static_cast<guiElemEvFol2D*>(curSubItem));
			myEvFol->assignOnGotFocus (bind(&listView2D::subItemGotFocus,  this, placeholders::_1, placeholders::_2), this);
			myEvFol->informMotherOnEvent(this);
		}
	}
	updateSledgeWidth();
}

//-----------------------------------------------------------------------------
// Name: removeItem()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::removeItem(item& itemToRemove,	bool alsoDeleteItem)
{
	if (alsoDeleteItem) {
		itemToRemove.subItems->deleteAllItems();
		delete itemToRemove.subItems;
	} else {
		releaseSubItem(itemToRemove.subItems);
	}
	items.erase(itemToRemove.itr);
	if (items.empty()) markedItem = nullptr;
	if (markedItem == &itemToRemove) markedItem = nullptr;
	updateSledgeWidth();
}

//-----------------------------------------------------------------------------
// Name: removeItem()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::removeItem(unsigned int itemIndex,	bool alsoDeleteItem)
{
	unsigned int counter	= 0;

	for (auto& curItem = items.begin(); curItem != items.end(); curItem++, counter++) {
		if (counter == itemIndex) {
			removeItem(*curItem, alsoDeleteItem);
			return;
		}
	}
}

//-----------------------------------------------------------------------------
// Name: removeAllItemsInRow()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::removeAllItemsInRow(unsigned int rowIndex,	bool alsoDeleteItem)
{
	// locals
	auto			curItem		= items.begin();					// current considered item
	auto			delItem		= items.begin();					// item, which shall be removed
	unsigned int	delta		= getItemIndex(rowIndex, 0);		// number of items to move forward in list
	unsigned int	itemIndex	= delta;							// index of current considered item
	unsigned int	columnIndex	= 0;								// index of current considered column

	// go to first item of the row to remove
	if (itemIndex >= items.size()) return;
	advance(curItem, delta);

	// remove the corresponding item of each column
	for (columnIndex = 0; columnIndex < getNumColumns(); columnIndex++) {

		// delete all sub items of the current considered item
		if (alsoDeleteItem && curItem->subItems) {
			curItem->subItems->deleteAllItems();
			delete curItem->subItems;
		} else {
			releaseSubItem(curItem->subItems);
		}

		// remember item, which shall be removed
		delItem = curItem;

		// avance to next item, which shall be removed
		if (columnIndex + 1 < getNumColumns()) {
			if (positioningMode == posMode::ROW_WISE) {
				delta		= 1;
			} else if(positioningMode == posMode::COLUMN_WISE) {
				delta		= getNumRows();
			}
// ... this does not work for 'hListViewArray'
//			itemIndex  += delta;
//			if (itemIndex >= items.size()) {
//				columnIndex = getNumColumns();
//			} else {
				advance(curItem, delta);
//			}
		}

		// unmark item, if it is deleted
		if (&(*delItem) == markedItem) {
			markedItem = nullptr;
		}

		// remove item
		items.erase(delItem);
	}

	// update sledge and marked item
	if (items.empty()) markedItem = nullptr;
	updateSledgeWidth();
}

//-----------------------------------------------------------------------------
// Name: removeAllItemsInColumn()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::removeAllItemsInColumn(unsigned int columnIndex,	bool alsoDeleteItem)
{
	unsigned int	rowIndex	= 0;
	auto			curItem		= items.begin();
	auto			delItem		= items.begin();

	advance(curItem, getItemIndex(0, columnIndex));

	for (rowIndex = 0; rowIndex < getNumRows(); rowIndex++) {

		if (alsoDeleteItem) {
			curItem->subItems->deleteAllItems();
			delete curItem->subItems;
		} else {
			releaseSubItem(curItem->subItems);
		}
		delItem = curItem;

		if (rowIndex + 1 < getNumRows()) {
			if (positioningMode == posMode::ROW_WISE) {
				advance(curItem, getNumColumns());
			} else if(positioningMode == posMode::COLUMN_WISE) {
				advance(curItem, 1);
			}
		}

		items.erase(delItem);
	}
	if (items.empty()) markedItem = nullptr;
	updateSledgeWidth();
}

//-----------------------------------------------------------------------------
// Name: releaseSubItem()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::releaseSubItem(guiElemCluster2D* subItem)
{
	subItem->setClippingRect(nullptr);
	for (auto& curSubItem : subItem->items) {
		curSubItem->removeMatrix(&mat);
		auto myEvFol = curSubItem->getPointer<guiElemEvFol2D>();
		if (myEvFol) {
			myEvFol->assignOnGotFocus (nullptr, nullptr);
			myEvFol->assignOnLostFocus(nullptr, nullptr);
			myEvFol->doNotInformMotherOnEvent(this);
		}
	}
}

//-----------------------------------------------------------------------------
// Name: deleteAllItems()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::removeAllItems(bool alsoDeleteItem)
{
	for (auto& curItem : items) {
		if (alsoDeleteItem) {
			curItem.subItems->deleteAllItems();
			delete curItem.subItems;
		} else {
			releaseSubItem(curItem.subItems);
		}
	}
	items.clear();
	markedItem = nullptr;
	updateSledgeWidth();
}

//-----------------------------------------------------------------------------
// Name: assignOnItemChanged()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::assignOnItemChanged(function<void(unsigned int, unsigned int, guiElemEvFol*, void*)> userFunc, void* pUser)
{
	this->userItemChanged	= userFunc;
	this->pUser				= pUser;

	// .. is this really necessary? because the function insertItem() already assigns to each item
	// for (auto& curItem : items) {
	// 	for (auto& curSubItem : curItem.subItems->items) {
	// 		auto myEvFol = curSubItem->getPointer<guiElemEvFol2D>();
	// 		if (myEvFol) {
	// 			myEvFol->assignOnGotFocus (bind(&listView2D::subItemGotFocus,  this, placeholders::_1, placeholders::_2), this);
	// 		}
	// 	}
	// }
}

//-----------------------------------------------------------------------------
// Name: subItemGotFocus()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::subItemGotFocus(guiElemEvFol* theGuiElem, void* pUser)
{
	unsigned int	counter		= 0;
	bool			itemFound	= false;

	// ... slow
	for (auto& curItem : items) {
		for (auto& curSubItem : curItem.subItems->items) {
			if ((guiElemEvFol2D*) curSubItem == theGuiElem) {		// <-- ... very very dirty !!! since guiElemEvFol2D inherits from guiELem2D and from guiElemEvFol the pointers curSubItem and theGuiElem differ!
	
				// unmark currently marked items
				encolorMarkedItems(false);

				markedItem	= &curItem;
				
				// update color of items
				encolorMarkedItems(true);

				itemFound	= true;
				break;
			}
		}
		if (itemFound) break;
		counter++;
	}

	// call user function 
	if (userItemChanged != nullptr) {
		userItemChanged(markedItem->getRowIndex(), markedItem->getColumnIndex(), theGuiElem, pUser);
	}

	// call user function specific to row
	auto markedRow = markedItem->getRow();
	if (markedRow->gotFocusUserFunc) {
		markedRow->gotFocusUserFunc(theGuiElem, markedRow->gotFocusUserPointer);
	}

	// call user function specific to column
	// ...
}

//-----------------------------------------------------------------------------
// Name: updateMarkedItem()
// Desc: 'markedItem' is an item index based on the number of column and rows. if their amount changes than 'markedItem' needs to updated.
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::updateMarkedItem(int numNewRows, int numNewColumns)
{
	if (markedItem == nullptr) return;
	unsigned int	markedItemIndex = markedItem->getItemIndex();
	int				delta			= ((positioningMode == posMode::COLUMN_WISE) ? numNewRows * getColumnIndex(markedItemIndex) + numNewColumns : numNewColumns * getRowIndex(markedItemIndex)) + numNewRows;
	
	if (delta && markedItemIndex + delta >= 0 && markedItemIndex + delta < items.size()) {
		auto myItr = markedItem->itr;
		advance(myItr, delta);
		markedItem = &(*myItr);
	}
}

//-----------------------------------------------------------------------------
// Name: getColumnIndex()
// Desc: 
//-----------------------------------------------------------------------------
unsigned int wildWeasel::listView2D::getColumnIndex(unsigned int itemIndex)
{
	// determine focused row and column
	if (columnHeaders.size() > 0 && positioningMode == posMode::ROW_WISE) {
		return itemIndex % columnHeaders.size();
	} else if (rowHeaders.size() > 0 && positioningMode == posMode::COLUMN_WISE) {
		return itemIndex / rowHeaders.size();
	}
	return 0;
}

//-----------------------------------------------------------------------------
// Name: getColumnIndex()
// Desc: 
//-----------------------------------------------------------------------------
unsigned int wildWeasel::listView2D::item::getColumnIndex()
{
	return listView->getColumnIndex(getItemIndex());
}

//-----------------------------------------------------------------------------
// Name: getRowIndex()
// Desc: 
//-----------------------------------------------------------------------------
unsigned int wildWeasel::listView2D::getRowIndex(unsigned int itemIndex)
{
	// determine focused row and column
	if (columnHeaders.size() > 0 && positioningMode == posMode::ROW_WISE) {
		return itemIndex / columnHeaders.size();
	} else if (rowHeaders.size() > 0 && positioningMode == posMode::COLUMN_WISE) {
		return itemIndex % rowHeaders.size();
	}
	return 0;
}

//-----------------------------------------------------------------------------
// Name: getColumnIndex()
// Desc: 
//-----------------------------------------------------------------------------
unsigned int wildWeasel::listView2D::column::getColumnIndex()
{
	return distance(listView->columnHeaders.begin(), itr);
}


//-----------------------------------------------------------------------------
// Name: getRowIndex()
// Desc: 
//-----------------------------------------------------------------------------
unsigned int wildWeasel::listView2D::row::getRowIndex()
{
	return distance(listView->rowHeaders.begin(), itr);
}

//-----------------------------------------------------------------------------
// Name: getRowIndex()
// Desc: 
//-----------------------------------------------------------------------------
unsigned int wildWeasel::listView2D::item::getRowIndex()
{
	return listView->getRowIndex(distance(listView->items.begin(), itr));
}

//-----------------------------------------------------------------------------
// Name: getItemGuiElemPointer()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::guiElemCluster2D* wildWeasel::listView2D::getItemGuiElemPointer(unsigned int itemIndex)
{
	// locals
	auto		 curItem	= items.begin();
	advance(curItem, itemIndex);
	return curItem->subItems;
}

//-----------------------------------------------------------------------------
// Name: getItemIndex()
// Desc: 
//-----------------------------------------------------------------------------
unsigned int wildWeasel::listView2D::getItemIndex(unsigned int rowIndex, unsigned int columnIndex)
{
	// determine focused row and column
	if (positioningMode == posMode::ROW_WISE) {
		return rowIndex * columnHeaders.size() + columnIndex;
	} else if (positioningMode == posMode::COLUMN_WISE) {
		return columnIndex * rowHeaders.size() + rowIndex;
	}
	return 0;
}

//-----------------------------------------------------------------------------
// Name: getItemIndex()
// Desc: 
//-----------------------------------------------------------------------------
unsigned int wildWeasel::listView2D::item::getItemIndex()
{
	return distance(listView->items.begin(), itr);
}

//-----------------------------------------------------------------------------
// Name: getItem()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::listView2D::item* wildWeasel::listView2D::getItem(unsigned int rowIndex, unsigned int columnIndex)
{
	return getItem(getItemIndex(rowIndex, columnIndex));
}

//-----------------------------------------------------------------------------
// Name: getItem()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::listView2D::item* wildWeasel::listView2D::getItem(unsigned int itemIndex)
{
	if (itemIndex >= items.size()) return nullptr;
	auto myItemItr = items.begin();
	advance(myItemItr, itemIndex);
	return &(*myItemItr);
}

//-----------------------------------------------------------------------------
// Name: encolorMarkedItems()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::encolorMarkedItems(bool setMarkerColor)
{
	if (markedItem == nullptr) return;

	// locals
	unsigned int	startIndex = 0;
	unsigned int	numItemsToMark = 0;
	unsigned int	indexStep = 0;
	unsigned int	rowIndex	= markedItem->getRowIndex();
	unsigned int	columnIndex = markedItem->getColumnIndex();

	if (markerSelectionMode == selectionMode::ROW_WISE && positioningMode == posMode::ROW_WISE) {
		startIndex = rowIndex * columnHeaders.size();
		numItemsToMark = columnHeaders.size();
		indexStep = 1;
	}
	else if (markerSelectionMode == selectionMode::ROW_WISE && positioningMode == posMode::COLUMN_WISE) {
		startIndex = rowIndex;
		numItemsToMark = columnHeaders.size();
		indexStep = rowHeaders.size();
	}
	else if (markerSelectionMode == selectionMode::COLUMN_WISE && positioningMode == posMode::ROW_WISE) {
		startIndex = columnIndex;
		numItemsToMark = rowHeaders.size();
		indexStep = columnHeaders.size();
	}
	else if (markerSelectionMode == selectionMode::COLUMN_WISE && positioningMode == posMode::COLUMN_WISE) {
		startIndex = columnIndex * rowHeaders.size();
		numItemsToMark = rowHeaders.size();
		indexStep = 1;
	}
	else if (markerSelectionMode == selectionMode::ITEM_WISE && positioningMode == posMode::ROW_WISE) {
		startIndex = rowIndex * columnHeaders.size() + columnIndex;
		numItemsToMark = 1;
		indexStep = 0;
	}
	else if (markerSelectionMode == selectionMode::ITEM_WISE && positioningMode == posMode::COLUMN_WISE) {
		startIndex = columnIndex * rowHeaders.size() + rowIndex;
		numItemsToMark = 1;
		indexStep = 0;
	}
	
	// encolor each item in the marked row, either to the marker color or revert it to the unmarked color
	processItems(startIndex, numItemsToMark, indexStep, [&](list<item>::iterator& curItem) {
		if (setMarkerColor) {
			curItem->subItems->setButtonColor(markerColor);
		} else {
			curItem->subItems->setButtonColor(curItem->colorWhenUnmarked);
		}
	});
}

//-----------------------------------------------------------------------------
// Name: processItems()
// Desc: 
//-----------------------------------------------------------------------------
template<typename TAction> void wildWeasel::listView2D::processItems(unsigned int startIndex, unsigned int numItemsToProcess, unsigned int indexStep, TAction action)
{
	// locals
	unsigned int	counter = 0;
	auto			curItem = items.begin();

	if (numItemsToProcess) {
		advance(curItem, startIndex);
		for (counter = 0; counter < numItemsToProcess; counter++) {
			action(curItem);
			if (startIndex + (counter+1) * indexStep < items.size()) {
				advance(curItem, indexStep);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Name: setRowColor()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::setRowColor(unsigned int rowIndex, color newColor)
{
	// locals
	unsigned int	startIndex		= 0;
	unsigned int	numItemsToMark	= getNumColumns();
	unsigned int	indexStep		= 0;

	if (positioningMode == posMode::COLUMN_WISE) {
		startIndex	= rowIndex;
		indexStep	= getNumRows();
	} else if (positioningMode == posMode::ROW_WISE) {
		startIndex	= getNumColumns() * rowIndex;
		indexStep	= 1;
	}

	// remember and set new color for each item in row
	processItems(startIndex, numItemsToMark, indexStep, [&](list<item>::iterator& curItem) {
		curItem->colorWhenUnmarked = newColor;
		curItem->subItems->setButtonColor(curItem->colorWhenUnmarked);
	});
}

//-----------------------------------------------------------------------------
// Name: getFocussedRowIndex()
// Desc: 
//-----------------------------------------------------------------------------
unsigned int wildWeasel::listView2D::getFocussedRowIndex()
{
	if (markerSelectionMode == selectionMode::ROW_WISE && markedItem != nullptr) {
		return markedItem->getRowIndex();
	} else {
		return getNumRows();
	}
}

//-----------------------------------------------------------------------------
// Name: setFocusOnRow()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::setFocusOnRow(unsigned int rowIndex)
{
	if (markerSelectionMode == selectionMode::ROW_WISE) {
		setFocusOnItem(getItemIndex(rowIndex, 0));
	}
}

//-----------------------------------------------------------------------------
// Name: setFocusOnColumn()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::column::setFocusOnColumn()
{
	if (listView->markerSelectionMode == selectionMode::COLUMN_WISE) {
		listView->getItem(0, getColumnIndex())->setFocusOnItem();
	}
}
//-----------------------------------------------------------------------------
// Name: setFocusOnRow()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::row::setFocusOnRow()
{
	if (listView->markerSelectionMode == selectionMode::ROW_WISE) {
		listView->getItem(getRowIndex(), 0)->setFocusOnItem();
	}
}

//-----------------------------------------------------------------------------
// Name: assignOnGotFocus()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::row::assignOnGotFocus(function<void(guiElemEvFol*, void*)> userFunc, void* pUser)
{
	this->gotFocusUserFunc		= userFunc;
	this->gotFocusUserPointer	= pUser;
}

//-----------------------------------------------------------------------------
// Name: assignOnLostFocus()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::row::assignOnLostFocus(function<void(guiElemEvFol*, void*)> userFunc, void* pUser)
{
	// this->lostFocusUserFunc		= userFunc;
	// this->lostFocusUserPointer	= pUser;
}

//-----------------------------------------------------------------------------
// Name: setFocusOnColumn()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::setFocusOnColumn(unsigned int columnIndex)
{
	if (markerSelectionMode == selectionMode::COLUMN_WISE) {
		setFocusOnItem(getItemIndex(0, columnIndex));
	}
}

//-----------------------------------------------------------------------------
// Name: setFocusOnItem()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::item::setFocusOnItem()
{
	for (auto& curSubItem : subItems->items) {
		auto myButton = curSubItem->getPointer<plainButton2D>();
		if (myButton) {
			wildWeasel::guiElemEvFol::setFocus(myButton);
			break;
		}
	}
}

//-----------------------------------------------------------------------------
// Name: setText()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::item::setText(unsigned int subItemIndex, const wchar_t* theText)
{
	auto curGuiElem = subItems->items.begin();
	if (subItemIndex < subItems->items.size()) {
		advance(curGuiElem, subItemIndex);
		(*curGuiElem)->setText(theText);
	}
}

//-----------------------------------------------------------------------------
// Name: setFocusOnItem()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::setFocusOnItem(unsigned int itemIndex)
{
	// locals
	auto curItem = items.begin();
	if (itemIndex < items.size()) {
		advance(curItem, itemIndex);
		curItem->setFocusOnItem();
	}
}

//-----------------------------------------------------------------------------
// Name: alignAllItems()
// Desc: Cluster-Matrix of items is overwritten here with function call curItem->subItems->setPosition(&curPos, true);
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::setElemVisibility(guiElement& elem, vector2& pos)
{
	// locals
	vector2			elemSize		= vector2((float) ww->getWindowSizeX(), (float) ww->getWindowSizeY());	// ... dirty solution - real element size not accessible since guiElement and not guiElement2D passed as parameter
	guiElemState	curState		= elem.getState();
	guiElemState	newState;
	bool			isVisibleInList = (pos.x + elemSize.x >= transScaledTargetRect.left && pos.x <= transScaledTargetRect.right && pos.y + elemSize.y >= transScaledTargetRect.top && pos.y <= transScaledTargetRect.bottom);
		
	// every state must have its counterpart
	switch (curState)
	{
	case guiElemState::DRAWED	:	newState = isVisibleInList ? guiElemState::DRAWED	 : guiElemState::INVISIBLE;		break;	// DRAWED <-> INVISIBLE
	case guiElemState::INVISIBLE:	newState = isVisibleInList ? guiElemState::DRAWED	 : guiElemState::INVISIBLE;		break;	
	case guiElemState::GRAYED	:	newState = isVisibleInList ? guiElemState::GRAYED	 : guiElemState::UNUSED;		break;	// GRAYED <-> UNUSED
	case guiElemState::UNUSED	:	newState = isVisibleInList ? guiElemState::GRAYED	 : guiElemState::UNUSED;		break;	
	case guiElemState::HIDDEN	:	newState = isVisibleInList ? guiElemState::VISIBLE	 : guiElemState::HIDDEN;		break;	// HIDDEN <-> VISIBLE
	case guiElemState::VISIBLE	:	newState = isVisibleInList ? guiElemState::VISIBLE	 : guiElemState::HIDDEN;		break;	
	}

	// set new state
	// ... state chain necessary !!!
	// elem.setState(newState);
}

//-----------------------------------------------------------------------------
// Name: alignAllItems()
// Desc: Cluster-Matrix of items is overwritten here with function call curItem->subItems->setPosition(&curPos, true);
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::setClusterVisibility(guiElemCluster2D& cluster, vector2& pos)
{
	for (auto& curSubItem : cluster.items) {
		if (curSubItem != nullptr) {
			setElemVisibility(*curSubItem, pos);
		}
	}
}

//-----------------------------------------------------------------------------
// Name: updateTransScaledRect()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::updateTransScaledRect()
{
	// ... since listView2D is not a guiElem2D. transScaledTargetRect must be calculated manually.
	vector2 translation2D, scale2D;
	matrix targetRectMat3D;
	matrix myMat;

	targetRectMat3D._11	= (targetRect.right - targetRect.left);
	targetRectMat3D._22	= (targetRect.bottom- targetRect.top );
	targetRectMat3D._33	= 1;
	targetRectMat3D._41	= targetRect.left;
	targetRectMat3D._42	= targetRect.top;
	targetRectMat3D._43	= 0;

	myMat = targetRectMat3D * mat;

	translation2D.x					= myMat._41;													
	translation2D.y					= myMat._42;													
	scale2D.x						= vector3(myMat._11, myMat._12, myMat._13).Length();				
	scale2D.y						= vector3(myMat._21, myMat._22, myMat._23).Length();				
	transScaledTargetRect.left		= (translation2D.x);
	transScaledTargetRect.top		= (translation2D.y);
	transScaledTargetRect.right		= (translation2D.x + scale2D.x);
	transScaledTargetRect.bottom	= (translation2D.y + scale2D.y);
}

//-----------------------------------------------------------------------------
// Name: updateClippingRects()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::updateClippingRects()
{
	clippingAreaItems			.left		= (rowHeaderIsVisible?rowHeaderWidth:0)			+ transScaledTargetRect.left;
	clippingAreaItems			.top		= (columnHeaderIsVisible?columnHeaderHeight:0)  + transScaledTargetRect.top ;
	clippingAreaItems			.right		= transScaledTargetRect.right  - (rowScrollBarIsVisible?rowScrollBarWidth:0);
	clippingAreaItems			.bottom		= transScaledTargetRect.bottom - (columnScrollBarIsVisible?columnScrollbarHeight:0);

	// update clipping rect for headers
	clippingAreaColumnHeaders	.left		= (rowHeaderIsVisible?rowHeaderWidth:0)			+ transScaledTargetRect.left;
	clippingAreaColumnHeaders	.top		=												  transScaledTargetRect.top			- 100;		// ... dirty work-around for rotated column headers
	clippingAreaColumnHeaders	.right		= transScaledTargetRect.right  /*- (rowScrollBarIsVisible?rowScrollBarWidth:0)*/;
	clippingAreaColumnHeaders	.bottom		= transScaledTargetRect.top + (columnHeaderIsVisible?columnHeaderHeight:0);

	clippingAreaRowHeaders		.left		=												  transScaledTargetRect.left;
	clippingAreaRowHeaders		.top		= (columnHeaderIsVisible?columnHeaderHeight:0)  + transScaledTargetRect.top ;
	clippingAreaRowHeaders		.right		= transScaledTargetRect.left  + (rowHeaderIsVisible?rowHeaderWidth:0);
	clippingAreaRowHeaders		.bottom		= transScaledTargetRect.bottom /*- (columnScrollBarIsVisible?columnScrollbarHeight:0)*/;
}

//-----------------------------------------------------------------------------
// Name: updateScrollBarPos()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::updateScrollBarPos()
{
	if (scrollBarsCreated) {
		updateSledgeWidth();

		columnScrollBar	->setRotation	(0, false);
		columnScrollBar ->setPosition	(transScaledTargetRect.left, transScaledTargetRect.bottom - columnScrollbarHeight, false);
		columnScrollBar	->setScale		(transScaledTargetRect.right - transScaledTargetRect.left - (rowHeaderIsVisible?rowHeaderWidth:0) - (rowScrollBarIsVisible?rowScrollBarWidth:0), (float) columnScrollbarHeight, true);
		rowScrollBar	->setRotation	(wwc::PI/2, false);
		rowScrollBar	->setPosition	(transScaledTargetRect.right, transScaledTargetRect.top   + (columnHeaderIsVisible?columnHeaderHeight:0), false);
		rowScrollBar	->setScale		(transScaledTargetRect.bottom - transScaledTargetRect.top - (columnHeaderIsVisible?columnHeaderHeight:0) - (columnScrollBarIsVisible?columnScrollbarHeight:0), (float) rowScrollBarWidth, true);
		columnScrollBar	->alignAllItems();
		rowScrollBar	->alignAllItems();
	}
}

//-----------------------------------------------------------------------------
// Name: updateHeaderPos()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::updateRowHeaderPos()
{
	// row header
	vector2			curPos	= vector2(transScaledTargetRect.left, transScaledTargetRect.top + scrollOffset.y);

	// TODO: implement auto width

	// set position and visibility of each row gui element
	for (auto& curRowHeaderItem : rowHeaders) {
		if (curRowHeaderItem->elem != nullptr) {
			curRowHeaderItem->elem->setPosition(&curPos, true);
			curRowHeaderItem->elem->setState(curRowHeaderItem->visible ? guiElemState::DRAWED	: guiElemState::HIDDEN);
			// ... setElemVisibility(*curRowHeaderItem->elem, curPos);
		}
		if (curRowHeaderItem->visible) {
			curPos.y += curRowHeaderItem->curSize;
		}
	}
}

//-----------------------------------------------------------------------------
// Name: updateHeaderPos()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::updateColumnHeaderPos()
{
	// column header
	vector2			curPos				= vector2(transScaledTargetRect.left + scrollOffset.x, transScaledTargetRect.top);
	unsigned int	targetWidth			= ((unsigned int) transScaledTargetRect.width()) - (rowScrollBarIsVisible?rowScrollBarWidth:0) - (rowHeaderIsVisible?rowHeaderWidth:0);		// should be the same as clippingAreaColumnHeaders.width()
	unsigned int	currentWidth		= 0;
	unsigned int	widthOfFixedCols	= 0;
	unsigned int	widthToDistribute = 0;
	bool			autoWidthOn			= false;
	const float		minFloat			= 0.000001f;

	// if no space to show the elements then spare to position and size them
	if (targetWidth < minFloat) return;

	// auto width
	for (auto& curColHeaderItem : columnHeaders) {
	
		// is there at least one column with auto width?
		if (curColHeaderItem.autoSizeFraction> minFloat) {
			autoWidthOn = true;
			widthOfFixedCols += curColHeaderItem.minSize;
		} else {
			widthOfFixedCols += curColHeaderItem.curSize;
		}

		// get current total width of all columns
		currentWidth += curColHeaderItem.curSize;
	}
	
	if (autoWidthOn) {

		if (targetWidth > widthOfFixedCols) {
			widthToDistribute = targetWidth - widthOfFixedCols;
		} else {
			widthToDistribute = 0;
		}

		// automatically adjust the column widths, if the current total width differ from the visible area
		if (currentWidth != targetWidth) {

			// process each column
			for (auto& curColHeaderItem : columnHeaders) {

				// only adjust columns which have a positive 'autoWidthFraction'
				if (curColHeaderItem.autoSizeFraction > minFloat) {
					curColHeaderItem.curSize = (unsigned int) (widthToDistribute * curColHeaderItem.autoSizeFraction) + curColHeaderItem.minSize;
					
					// adjust scale of the gui elem if it is not rotated
					if (curColHeaderItem.elem != nullptr) {
						float elemRotation;
						curColHeaderItem.elem->getRotation(elemRotation);
						if (abs(elemRotation) < minFloat) {
							vector2 elemScale;
							curColHeaderItem.elem->getScale(elemScale);
							curColHeaderItem.elem->setScale((float) curColHeaderItem.curSize / curColHeaderItem.sizeSetByUser, elemScale.y, false);
						}
					}
				}
			}
		}
	}

	// set position and visibility of each column gui element
	for (auto& curColHeaderItem : columnHeaders) {
		if (curColHeaderItem.elem != nullptr) {
			curColHeaderItem.elem->setPosition(&curPos, true);
			setElemVisibility(*curColHeaderItem.elem, curPos);
		}
		curPos.x += curColHeaderItem.curSize;
	}
}

//-----------------------------------------------------------------------------
// Name: updateItemPos()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::updateItemPos()
{
	// locals
	unsigned int	numColumns			= getNumColumns();
	unsigned int	numRows				= getNumRows();
	auto			curRowHeaderItem	= rowHeaders.begin();
	auto			curColHeaderItem	= columnHeaders.begin();
	unsigned int	counter				= 0;
	vector2			initialItemPos		= vector2((rowHeaderIsVisible?rowHeaderWidth:0)			+ transScaledTargetRect.left + scrollOffset.x,
												  (columnHeaderIsVisible?columnHeaderHeight:0)  + transScaledTargetRect.top  + scrollOffset.y);
	vector2			curPos				= initialItemPos;

	if (curRowHeaderItem == rowHeaders.end()) {
		return;
	} else {
		curPos.x		   += (*curRowHeaderItem)->xOffsetInFirstColumn;
	}

	for (auto& curItem = items.begin(); curItem != items.end(); curItem++, counter++) {

		if (curItem->subItems != nullptr) {
			// TODO: implement scale according to auto height
			curItem->subItems->setScale((float) curColHeaderItem->curSize / curColHeaderItem->sizeSetByUser, (float) (*curRowHeaderItem)->curSize / (*curRowHeaderItem)->sizeSetByUser, false);
			curItem->subItems->setPosition(&curPos, true);
			// curItem->subItems->setState((*curRowHeaderItem)->visible ? guiElemState::DRAWED	: guiElemState::HIDDEN);	// ... where does this line come from? it makes no sense?
			// ... setClusterVisibility(*curItem->subItems, curPos);
		}

		if (positioningMode == posMode::ROW_WISE) {
			unsigned int    nextColumn = ((counter+1)%numColumns);
			curPos.x += curColHeaderItem->curSize;
			curColHeaderItem++;

			if (nextColumn == 0) {
				curRowHeaderItem++;
				curColHeaderItem	= columnHeaders.begin();
				curPos.x = initialItemPos.x + (*curRowHeaderItem)->xOffsetInFirstColumn;
				if ((*curRowHeaderItem)->visible) {
					curPos.y += (float) (*curRowHeaderItem)->curSize;
				}
			} else if (nextColumn == 1) {
				curPos.x -= (*curRowHeaderItem)->xOffsetInFirstColumn;
			}

		} else if (positioningMode == posMode::COLUMN_WISE) {
			curRowHeaderItem++;
			
			if (curRowHeaderItem != rowHeaders.end()) {
				if ((*curRowHeaderItem)->visible) {
					curPos.y += (*curRowHeaderItem)->curSize;
				}
				if (counter < numRows) {
					curPos.x = initialItemPos.x + (*curRowHeaderItem)->xOffsetInFirstColumn;
				} else if (counter == numRows - 1) {
					curPos.x = initialItemPos.x;
				}
			}

			if (((counter+1)%numRows) == 0) {
				curPos.y = initialItemPos.y;
				curPos.x += (float) curColHeaderItem->curSize;
				curColHeaderItem++;
				if (curColHeaderItem == columnHeaders.end()) break;
				curRowHeaderItem	= rowHeaders.begin();
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Name: alignAllItems()
// Desc: Cluster-Matrix of items is overwritten here with function call curItem->subItems->setPosition(&curPos, true);
//-----------------------------------------------------------------------------
void wildWeasel::listView2D::alignAllItems()
{
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

	// items
	updateItemPos();

	// scrollbars
	updateScrollBarPos();
}
#pragma endregion
