/*********************************************************************
	wwguiElement.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/

#include "wwEditField.h"

/*************************************************************************************************************************************/

#pragma region editField2D

/* data model
text				|											|
lines				|	|			| |							|
formats				|			  | |							|
markers				|	off	|on|		off		| on |	off		|
textCursor.pText	|--------------------->|
textCursor.pLine	|                 |--->|
textCursor.pFormat	|               |----->|
textCursor.pMarker	|          |---------->|
visibleArea.pText	|---------------------------------->|
visibleArea.pLine	|                 |---------------->|
visibleArea.pFormat	|               |------------------>|
visibleArea.pMarker	|                                |->|
*/

//-----------------------------------------------------------------------------
// Name: create()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::editField2D::create(masterMind* ww, font2D* theFont, float depthInSpace)
{
	// params ok?
	if (!ww)			return false;
	if (initialized)	return false;
	
	// positioning
	targetRectPositionZ	= depthInSpace;
	textFont			= theFont;

	setState(wildWeasel::guiElemState::DRAWED);
	setColor(wildWeasel::color::white);
	setTextColor(wildWeasel::color::black);
	setTextAlignmentHorizontal(alignmentHorizontal::LEFT);
	setTextAlignmentVertical  (alignmentVertical::TOP);

	// call back function
	this->ww			= ww;
	this->mainTexture	= &ww->backgroundTexture;

	// remember to load the file later on together with the other ressources
	addObjectToLoad(ww);
	ww->registerGuiElement(this);
	addSpriteToDraw(targetRectPositionZ, nullptr);

	// track mouse moves and clicks
	eventFollower::followEvent(this, eventType::WINDOWSIZE_CHANGED);
	eventFollower::followEvent(this, eventType::MOUSEMOVED);
	eventFollower::followEvent(this, eventType::LEFT_MOUSEBUTTON_PRESSED);
	eventFollower::followEvent(this, eventType::LEFT_MOUSEBUTTON_RELEASED);
	eventFollower::followEvent(this, eventType::KEYDOWN);

	assignOnGotFocus			(bind(&wildWeasel::editField2D::eventGotFocus			,	this, placeholders::_1, placeholders::_2), this); 
	assignOnLostFocus			(bind(&wildWeasel::editField2D::eventLostFocus			,	this, placeholders::_1, placeholders::_2), this); 
	assignOnMouseEnteredRegion	(bind(&wildWeasel::editField2D::eventMouseEnteredRegion	,	this, placeholders::_1, placeholders::_2), this); 
	assignOnMouseLeftRegion		(bind(&wildWeasel::editField2D::eventMouseLeftRegion	,	this, placeholders::_1, placeholders::_2), this); 

	// initialize the rest
	reset();
	textCursor.blinkTimer.setFunc(ww, blinkTimerFunc, this);

	// quit
	initialized = true;
	return initialized;
}

//-----------------------------------------------------------------------------
// Name: reset()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::reset()
{
	// clear everything
	text	.clear();
	texts	.clear();
	markers	.clear();
	lines	.clear();
	formats	.clear();

	// add a text segments
	textLineStruct	*	dummyLine	= new textLineStruct();
	textFormatStruct*	dummyFormat = new textFormatStruct();
	textMarkerStruct*	dummyMarker = new textMarkerStruct();
	textSegment		*	dummyText	= new textSegment();

	// consider current format
	dummyFormat->size	= textScale;
	dummyFormat->color	= textColor;

	lines	.chain.push_back(dummyLine);
	formats	.chain.push_back(dummyFormat);
	markers	.chain.push_back(dummyMarker);
	texts	.chain.push_back(dummyText);

	// set visible area
	visibleArea.pLine	.init(&lines	);
	visibleArea.pFormat	.init(&formats	);
	visibleArea.pMarker	.init(&markers	);
	visibleArea.pText	.init(&texts	);
	visibleArea.firstPixelPosY			= 0;

	// set text cursor
	textCursor.isVisible				= false;
	textCursor.pLine	.init(&lines	);
	textCursor.pFormat	.init(&formats	);
	textCursor.pMarker	.init(&markers	);
	textCursor.pText	.init(&texts	);
}

//-----------------------------------------------------------------------------
// Name: keyDown()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::keyDown(int keyCode)
{
	// focused?
	if (!hasFocus()) return;

	/* scrolling by arrow keys (not used because done by cursor pos)
	if ((GetKeyState(VK_MENU) & 0xff00)) {
		if (scrollBarDuo::keyDown(keyCode)) {
			return;
		}
	}*/

	// ... maybe switch should be used here
	if (keyCode >= 0x30 && keyCode <= 0x5A) {
		WCHAR newChar = MapVirtualKey(keyCode, MAPVK_VK_TO_CHAR);
		if (!(GetKeyState(VK_SHIFT) & 0xff00)) {
			newChar = towlower(newChar);
		}
		insertChar(newChar);
	} else if (keyCode == VK_RETURN) {
		if (true /*multiLineMode*/) {
			insertChar('\n');
		} else {
			if (userValueEntered != nullptr) {
				userValueEntered(this, pUser);
			}
		}
	} else if (keyCode == VK_SPACE) {
		insertChar(' ');
	} else if (keyCode == VK_BACK) {
		if (textCursor.pText.posWithInSegment > 0) {
			setTextCursorPos(textCursorMoveType::MOVE_RELATIVE, -1, false); 
			removeChars(1);
		}
	} else if (keyCode == VK_DELETE) {
		removeChars(1);
	} else if (keyCode == VK_HOME) {
		if ((GetKeyState(VK_CONTROL) & 0xff00)) {
			setTextCursorPos(textCursorMoveType::MOVE_ABSOLUTE,  0, false); 
			columnScrollBar	->setSledgePos(0);
			rowScrollBar	->setSledgePos(0);
		} else {
			setTextCursorPos(textCursorMoveType::MOVE_RELATIVE,  0, 0, false);
		}
	} else if (keyCode == VK_LEFT) { 
		setTextCursorPos(textCursorMoveType::MOVE_RELATIVE, -1, false); 
	} else if (keyCode == VK_RIGHT) {
		setTextCursorPos(textCursorMoveType::MOVE_RELATIVE,  1, true); 
	} else if (keyCode == VK_UP) {
		setTextCursorPos(textCursorMoveType::MOVE_RELATIVE, -1, textCursor.pLine.posWithInSegment, false); 
	} else if (keyCode == VK_DOWN) {
		setTextCursorPos(textCursorMoveType::MOVE_RELATIVE,  1, textCursor.pLine.posWithInSegment, false); 
	} else if (keyCode == VK_NEXT) {
		setTextCursorPos(textCursorMoveType::MOVE_RELATIVE,  1 * visibleArea.calcNumVisibleLines(transScaledTargetRect.bottom - transScaledTargetRect.top), textCursor.pLine.posWithInSegment, false); 
	} else if (keyCode == VK_PRIOR) {
		setTextCursorPos(textCursorMoveType::MOVE_RELATIVE, -1 * visibleArea.calcNumVisibleLines(transScaledTargetRect.bottom - transScaledTargetRect.top), textCursor.pLine.posWithInSegment, false); 
	} else if (keyCode == VK_END) {
		if ((GetKeyState(VK_CONTROL) & 0xff00)) {
			setTextCursorPos(textCursorMoveType::MOVE_ABSOLUTE, (int) text.size(), false);
			columnScrollBar	->setSledgePos(1);
			rowScrollBar	->setSledgePos(1);
		} else {
			setTextCursorPos(textCursorMoveType::MOVE_RELATIVE,  (*textCursor.pLine.itr)->numChars - textCursor.pLine.posWithInSegment, false);
		}
	} else if (keyCode == VK_TAB) {
		insertString(L"    ");
	} else if (keyCode == VK_MULTIPLY) {
		insertChar('*');
	} else if (keyCode == VK_ADD) {
		insertChar('+');
	} else if (keyCode == VK_DIVIDE) {
		insertChar('/');
	} else if (keyCode == VK_SUBTRACT) {
		insertChar('-');
	} else if (keyCode == VK_ESCAPE) {
		guiElemEvFol::setFocus(nullptr);
	}
}


//-----------------------------------------------------------------------------
// Name: verticalWheelMoved()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::verticalWheelMoved(int distance)
{
	editField2D::keyDown(distance < 0 ? VK_DOWN : VK_UP);
}

//-----------------------------------------------------------------------------
// Name: horizontalWheelMoved()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::horizontalWheelMoved(int distance)
{
	editField2D::keyDown(distance < 0 ? VK_LEFT : VK_RIGHT);
}

//-----------------------------------------------------------------------------
// Name: getTextCursorPos()
// Desc: 
//-----------------------------------------------------------------------------
unsigned int wildWeasel::editField2D::getTextCursorPos()
{
	return textCursor.pText.posWithInSegment;
}

//-----------------------------------------------------------------------------
// Name: getTextCursorPos()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::getTextCursorPos(unsigned int& line, unsigned int& posWithInLine)
{
	line			= (unsigned int) distance(lines.chain.begin(), textCursor.pLine.itr);
	posWithInLine	= textCursor.pLine.posWithInSegment;
}

//-----------------------------------------------------------------------------
// Name: getTextCursorPos()
// Desc: 
//-----------------------------------------------------------------------------
unsigned int wildWeasel::editField2D::visibleAreaStruct::calcNumVisibleLines(float visibleHeight)
{
	// locals
	float			linesHeight		= 0;
	unsigned int	numVisibleLines = 0;
	auto			curSegment		= pLine.itr;
	textLineStruct* curLine;

	while (curSegment != pLine.end() && linesHeight < visibleHeight) {
		curLine	= (textLineStruct*) (*curSegment);
		linesHeight	+= curLine->sizeInPixels.y;
		numVisibleLines++;
		curSegment++;
	}
	return numVisibleLines;
}

//-----------------------------------------------------------------------------
// Name: setTextCursorPos()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::setTextCursorPos(textCursorMoveType moveType, int newPos, bool jumpToNextAfterNewLine)
{
	// locals
	textLineStruct::segmentChangeVars lineVars;
	
	if (moveType == textCursorMoveType::MOVE_ABSOLUTE) {
		if (newPos < (int) 0)			newPos = 0;
		if (newPos > (int) text.size())	newPos = (int) text.size();

		textCursor.pLine	.moveAbsolute(newPos, &lineVars);
		textCursor.pText	.moveAbsolute(newPos, nullptr);
		textCursor.pFormat	.moveAbsolute(newPos, nullptr);
		textCursor.pMarker	.moveAbsolute(newPos, nullptr);

	} else if (moveType == textCursorMoveType::MOVE_RELATIVE) {
		if (newPos + (int) textCursor.pText.posWithInSegment < 0)					newPos = (int) textCursor.pText.posWithInSegment * -1;
		if (newPos + (int) textCursor.pText.posWithInSegment > (int) text.size())	newPos = (int) text.size() - (int) textCursor.pText.posWithInSegment;
		if (newPos == 0) return;

		textCursor.pLine	.moveRelative(newPos, &lineVars);
		textCursor.pText	.moveRelative(newPos, nullptr);
		textCursor.pFormat	.moveRelative(newPos, nullptr);
		textCursor.pMarker	.moveRelative(newPos, nullptr);
	}
	
	setTextCursorPosFinish(jumpToNextAfterNewLine);
}

//-----------------------------------------------------------------------------
// Name: setTextCursorPos()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::setTextCursorPos(textCursorMoveType moveType, int newLine, int newPosWithInLine, bool jumpToNextAfterNewLine)
{
	// locals
	textLineStruct::segmentChangeVars lineVars;

	if (moveType == textCursorMoveType::MOVE_ABSOLUTE) {
		if (newLine < 0)						newLine = 0;
		if (newLine > (int) lines.chain.size())	newLine = (int) lines.chain.size();

		// set to beginning
		lineVars.charsMoved	= 0;
		textCursor.pLine	.itr = textCursor.pLine		.begin();
		textCursor.pLine	.posWithInSegment	= 0;
		textCursor.pLine	.moveRelative(newLine, newPosWithInLine, &lineVars);
		lineVars.charsMoved	+= textCursor.pLine.posWithInSegment;

		textCursor.pText	.moveAbsolute(lineVars.charsMoved, nullptr);
		textCursor.pFormat	.moveAbsolute(lineVars.charsMoved, nullptr);
		textCursor.pMarker	.moveAbsolute(lineVars.charsMoved, nullptr);

	} else if (moveType == textCursorMoveType::MOVE_RELATIVE) {
		lineVars.charsMoved	= -1 * ((int) textCursor.pLine.posWithInSegment);
		textCursor.pLine	.moveRelative(newLine, newPosWithInLine, &lineVars);
		lineVars.charsMoved	+= textCursor.pLine.posWithInSegment;

		textCursor.pText	.moveRelative(lineVars.charsMoved, nullptr);
		textCursor.pFormat	.moveRelative(lineVars.charsMoved, nullptr);
		textCursor.pMarker	.moveRelative(lineVars.charsMoved, nullptr);
	}
	
	setTextCursorPosFinish(jumpToNextAfterNewLine);
}

//-----------------------------------------------------------------------------
// Name: setTextCursorPos()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::setTextCursorPosFinish(bool jumpToNextAfterNewLine)
{
	textCursor.blinkTimer.start(textCursor.blinkPeriod);
	textCursor.isVisible			= true;

	// if preceeding char is a newline then move cursor further on
	if (textCursor.pText.posWithInSegment > 0 && textCursor.pLine.posWithInSegment > 0 && text.at(textCursor.pText.posWithInSegment-1) == '\n') {
		if (jumpToNextAfterNewLine)  {
			if (textCursor.pLine.itr != textCursor.pLine.end()) {
				textCursor.pLine.itr++;
				textCursor.pLine.posWithInSegment = 0;
			}
		} else {
			setTextCursorPos(textCursorMoveType::MOVE_RELATIVE, jumpToNextAfterNewLine ? 1 : -1, jumpToNextAfterNewLine);
		}
	}

	scrollIfTextCursorOutside();
}

//-----------------------------------------------------------------------------
// Name: scrollIfTextCursorOutside()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::scrollIfTextCursorOutside()
{
	if (!autoScrollOn) return; 

	// if textcursor is outside the visible area then scroll by calling setSledgePos()
	vector2							posInSideEditField(scrollOffset.x, visibleArea.firstPixelPosY + scrollOffset.y);
	list<textSegment*>::iterator	lineItr	= visibleArea.pLine.itr;
	textLineStruct*					curLine = (textLineStruct*)   (*lineItr);

	for (; lineItr!=lines.chain.end(); lineItr++) {
		curLine					= (textLineStruct*) (*lineItr);

		// text cursor
		if (textCursor.isVisible == true && textCursor.pLine.itr == lineItr) {
			vector2 P(scrollOffset.x + getPixelPosWithInLine(textCursor.pLine, textCursor.pText.posWithInSegment), posInSideEditField.y);
			if (P.x < 0) {
				columnScrollBar->setSledgePos((P.x - scrollOffset.x - visibleAreaSize.x * 0.5f) / totalAreaSize.x);
			} else if (P.x > visibleAreaSize.x - rowScrollBarWidth) {
				columnScrollBar->setSledgePos((P.x - scrollOffset.x - visibleAreaSize.x * 0.5f) / totalAreaSize.x);
			}
			if (P.y < 0) {
				rowScrollBar->setSledgePos((P.y - scrollOffset.y - visibleAreaSize.y * 0.5f) / totalAreaSize.y);
			} else if (P.y > visibleAreaSize.y - columnScrollbarHeight - (*curLine).sizeInPixels.y) {
				rowScrollBar->setSledgePos((P.y - scrollOffset.y - visibleAreaSize.y * 0.5f) / totalAreaSize.y);
			}
		}

		// stop if bottom of edit field rect reached
		posInSideEditField.y += (*curLine).sizeInPixels.y;
		if (posInSideEditField.y > targetRect.bottom - targetRect.top + visibleArea.firstPixelPosY) {
			break;
		}
	}
}

//-----------------------------------------------------------------------------
// Name: insertChar()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::insertChar(WCHAR newChar)
{
	WCHAR newString[2];
	newString[0] = newChar;
	newString[1] = '\0';
	insertString(newString);
}

//-----------------------------------------------------------------------------
// Name: insertString()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::insertString(stringbuf& streamBuffer)
{
	// locals
	char					newChar;
	wstringstream			newOutputTextW;

	// any input?
	if (!streamBuffer.str().size()) return;

	// convert to unicode
	while ((newChar = streamBuffer.sbumpc()) != EOF) {
		newOutputTextW << newChar;
	}
	
	if (!newOutputTextW.str().size()) return;

	insertString(newOutputTextW.str().c_str());

	wildWeasel::scrollBarDuo::keyDown(VK_END);
}

//-----------------------------------------------------------------------------
// Name: insertString()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::insertString(const WCHAR* newString)
{
	// parameters ok?
	if (newString == nullptr) return;

	// locals
	WCHAR			newChar;
	float			additionalPixels;
	unsigned int	posWithInNewString;
	textLineStruct* myLine;
	textLineStruct* newLine;
	
	/* is text marked?
	if (marker.size()) {
		for (auto& curMaker : marker) {
			removeChars(calcPosChar(curMaker.line), curMaker.line, curMaker.posWithInLine, curMaker.numChars);
		}
		marker.clear();
	}*/

	// must format be considered?
	// ...

	// insert char into text string
	text.insert(textCursor.pText.posWithInSegment, newString);

	// adjust text cursor and lines and formats
	posWithInNewString	= 0;
	newChar				= newString[0];

	while (newChar != '\0') {

		// change text segment width
		textCursor.pLine	.changeTextSegmentWidth(1, textCursor.pLine		.posWithInSegment);
		textCursor.pFormat	.changeTextSegmentWidth(1, textCursor.pFormat	.posWithInSegment);
		textCursor.pMarker	.changeTextSegmentWidth(1, textCursor.pMarker	.posWithInSegment);
		textCursor.pText	.changeTextSegmentWidth(1, textCursor.pText		.posWithInSegment);

		textCursor.pLine	.moveRelative(1, nullptr);
		textCursor.pFormat	.moveRelative(1, nullptr);
		textCursor.pMarker	.moveRelative(1, nullptr);
		textCursor.pText	.moveRelative(1, nullptr);

		// line break
		if (newChar == '\n') {
			newLine							= new textLineStruct();
			myLine							= (textLineStruct*) (*textCursor.pLine.itr);
			float xPixelPos					= getPixelPosWithInLine(textCursor.pLine, textCursor.pText.posWithInSegment);
			textCursor.pLine				.splitTextSegment(textCursor.pLine	.posWithInSegment, newLine);
			
			newLine->sizeInPixels.x			= myLine->sizeInPixels.x - xPixelPos;
			newLine->sizeInPixels.y			= myLine->sizeInPixels.y;					// ... not correct, when a big string is missing !!!
			myLine->sizeInPixels.x			= xPixelPos;
			myLine->sizeInPixels.y			= myLine->sizeInPixels.y;					// ... update if a big char is vanished

		// add a common character
		} else {
			additionalPixels				= getCharWidthInPixel(newChar);
			myLine							= (textLineStruct*) (*textCursor.pLine.itr);
			myLine->sizeInPixels.x			+= additionalPixels;
			
			float curLineHeight				= getCurrentLineDistance();
			if (myLine->sizeInPixels.y < curLineHeight) {
				myLine->sizeInPixels.y = curLineHeight;
			}
		}

		// go to next char
		posWithInNewString++;
		newChar = newString[posWithInNewString];
	}

	scrollIfTextCursorOutside();
	updateSledgeWidth();
}

//-----------------------------------------------------------------------------
// Name: getCurrentLineDistance()
// Desc: 
//-----------------------------------------------------------------------------
float wildWeasel::editField2D::getCurrentLineDistance()
{
	//textFont->gfx_font->SetLineSpacing(getCharHeightInPixel('A'));
	//textFont->gfx_font->GetLineSpacing();
	return ((textFormatStruct*) *textCursor.pFormat.itr)->size.y * textFont->getCharSize('A').y * 1.3f;
}

//-----------------------------------------------------------------------------
// Name: removeChars()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::removeChars(unsigned int numChars)
{
	// parameters ok?
	if (textCursor.pText.posWithInSegment + numChars > (*textCursor.pText.itr)->numChars) numChars = (*textCursor.pText.itr)->numChars - textCursor.pText.posWithInSegment;
	if (numChars == 0) return;

	// locals
	WCHAR			oldChar;
	unsigned int	fromPos;
	float			decreasePixels;
	unsigned int	posWithInOldString;
	textLineStruct* myLine;

	// adjust text cursor and lines and formats
	posWithInOldString	= 0;
	fromPos				= textCursor.pText.posWithInSegment;
	oldChar				= text.at(fromPos + posWithInOldString);

	while (posWithInOldString < numChars) {

		// last position within any text segment?
		if (textCursor.pLine	.posWithInSegment == (*textCursor.pLine		.itr)->numChars)	{ textCursor.pLine		.mergeWithNextSegment(); }
		if (textCursor.pFormat	.posWithInSegment == (*textCursor.pFormat	.itr)->numChars)	{ textCursor.pFormat	.mergeWithNextSegment(); }
		if (textCursor.pMarker	.posWithInSegment == (*textCursor.pMarker	.itr)->numChars)	{ textCursor.pMarker	.mergeWithNextSegment(); }
		if (textCursor.pText	.posWithInSegment == (*textCursor.pText		.itr)->numChars)	{ textCursor.pText		.mergeWithNextSegment(); }

		// line break
		if (oldChar == '\n') {
			myLine					= (textLineStruct*) (*textCursor.pLine.itr);

			textCursor.pLine.itr++;
			float nextLineWidth		= ((textLineStruct*) (*textCursor.pLine.itr))->sizeInPixels.x;
			textCursor.pLine.itr--;

			textCursor.pLine		.changeTextSegmentWidth(-1, textCursor.pLine	.posWithInSegment);
			textCursor.pFormat		.changeTextSegmentWidth(-1, textCursor.pFormat	.posWithInSegment);
			textCursor.pMarker		.changeTextSegmentWidth(-1, textCursor.pMarker	.posWithInSegment);
			textCursor.pText		.changeTextSegmentWidth(-1, textCursor.pText	.posWithInSegment);

			textCursor.pLine		.mergeWithNextSegment();

			myLine->sizeInPixels.x += nextLineWidth;
			myLine->sizeInPixels.y	= myLine->sizeInPixels.y;					// ... update if a big char is vanished

		// add a common character
		} else {
			decreasePixels			= getCharWidthInPixel(oldChar);

			textCursor.pLine		.changeTextSegmentWidth(-1, textCursor.pLine	.posWithInSegment);
			textCursor.pFormat		.changeTextSegmentWidth(-1, textCursor.pFormat	.posWithInSegment);
			textCursor.pMarker		.changeTextSegmentWidth(-1, textCursor.pMarker	.posWithInSegment);
			textCursor.pText		.changeTextSegmentWidth(-1, textCursor.pText	.posWithInSegment);

			myLine					 = (textLineStruct*) (*textCursor.pLine.itr);
			myLine->sizeInPixels.x	-= decreasePixels;
			myLine->sizeInPixels.y	= myLine->sizeInPixels.y;					// ... update if a big char is vanished
		}

		// go to next char
		posWithInOldString++;
		if (fromPos + posWithInOldString >= text.size()) break;
		oldChar = text.at(fromPos + posWithInOldString);
	}

	// update text
	text.erase(fromPos, numChars);
	scrollIfTextCursorOutside();
	updateSledgeWidth();
}

//-----------------------------------------------------------------------------
// Name: setWordWrapMode()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::setWordWrapMode(wordWrapMode newWrapMode)
{
	wrapMode = newWrapMode;
	performWordWrap();
}

//-----------------------------------------------------------------------------
// Name: setBorderWidth()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::setBorderWidth(float newWidth)
{
	borderWidth = newWidth;
	performWordWrap();
}

//-----------------------------------------------------------------------------
// Name: setTextSize()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::setTextSize(float sx, float sy)					
{
	// default of editfield
	textScale.x = sx; 
	textScale.y = sy;

	// current format
	textFormatStruct* curFormat;
	curFormat = (textFormatStruct*) (*textCursor.pFormat.itr);
	curFormat->size.x = sx;
	curFormat->size.y = sy;
}

//-----------------------------------------------------------------------------
// Name: setAutoScroll()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::setAutoScroll(bool autoScrollOn)
{
	this->autoScrollOn = autoScrollOn;
}

//-----------------------------------------------------------------------------
// Name: setState()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::setState(guiElemState newStatus)
{
	// locals
	bool visible = isGuiElemStateVisible(newStatus);

	status = newStatus;

	if (columnScrollBar	) columnScrollBar	->setState(columnScrollBarIsVisible && visible ? guiElemState::DRAWED : guiElemState::HIDDEN);
	if (rowScrollBar	) rowScrollBar		->setState(rowScrollBarIsVisible	&& visible ? guiElemState::DRAWED : guiElemState::HIDDEN);

}

//-----------------------------------------------------------------------------
// Name: performWordWrap()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::performWordWrap()
{
	switch (wrapMode)
	{
	case wordWrapMode::OFF:			
		break;
	case wordWrapMode::CHARACTER:	
		// ...
		break;
	case wordWrapMode::WORD:		
		// ...
		break;
	} 

	updateSledgeWidth();
}

//-----------------------------------------------------------------------------
// Name: eventMouseEnteredRegion()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::eventMouseEnteredRegion(guiElemEvFol* elem, void* pUser)
{
	ww->cursor2D.setCursor(cursorType::IBEAM);
}

//-----------------------------------------------------------------------------
// Name: eventMouseLeftRegion()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::eventMouseLeftRegion(guiElemEvFol* elem, void* pUser)
{
	ww->cursor2D.setCursor(cursorType::ARROW);
}

//-----------------------------------------------------------------------------
// Name: eventGotFocus()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::eventGotFocus(guiElemEvFol* elem, void* pUser)
{
	// locals
	vector2	cursorWithInTotalArea;
	vector2	P((float) ww->cursor2D.x, (float) ww->cursor2D.y);
	rotateVectorAndTranslate(P, cursorWithInTotalArea, true, true, false);
	cursorWithInTotalArea -= scrollOffset;

	// get selected line
	textLineStruct* curLine		= nullptr;
	int				lineCounter = 0;
	float			curLinePosY = visibleArea.firstPixelPosY;
	for (auto/*& <-- here no reference, since visibleArea.pLine.itr must not be changed! */ curSegment = visibleArea.pLine.itr; curSegment != lines.chain.end(); curSegment++, lineCounter++) {
		curLine = (textLineStruct*) (*curSegment);
		if (cursorWithInTotalArea.y >= curLinePosY && cursorWithInTotalArea.y <= curLinePosY + curLine->sizeInPixels.y) {
			break;
		}
		curLinePosY += curLine->sizeInPixels.y;
	}

	// get character position
	unsigned int	posWithInLine	= 0;
	size_t			charPos			= visibleArea.pText.posWithInSegment;
	float			curLinePosX		= 0;
	for (posWithInLine = 0; posWithInLine < curLine->numChars; posWithInLine++, charPos++) {
		if (cursorWithInTotalArea.x >= curLinePosX && cursorWithInTotalArea.x <= curLinePosX + getCharWidthInPixel(text.at(charPos))) {
			break;
		}
		curLinePosX += getCharWidthInPixel(text.at(charPos));
	}

	// set cursor (... muss noch optimiert werden, da die linien liste unnötig durchlaufen wird !!!)
	if (visibleArea.pText.posWithInSegment > textCursor.pText.posWithInSegment) {
		lineCounter += (int) distance(textCursor.pLine.itr, visibleArea.pLine.itr);
	} else {
		lineCounter -= (int) distance(visibleArea.pLine.itr, textCursor.pLine.itr);
	}
	setTextCursorPos(textCursorMoveType::MOVE_RELATIVE, lineCounter, posWithInLine, false);
}

//-----------------------------------------------------------------------------
// Name: eventLostFocus()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::eventLostFocus(guiElemEvFol* elem, void* pUser)
{
	textCursor.blinkTimer.terminate();
	textCursor.isVisible	= false;

	if (userValueEntered != nullptr) {
		userValueEntered(this, pUser);
	}
}

//-----------------------------------------------------------------------------
// Name: windowSizeChanged()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::windowSizeChanged(int xSize, int ySize)
{
	alignAllItems();
}

//-----------------------------------------------------------------------------
// Name: windowSizeChanged()
// Desc: 
//-----------------------------------------------------------------------------
float wildWeasel::editField2D::getPixelPosWithInLine(pointerToTextSegment &p, unsigned int posCharInText)
{
	float			curPixelPos			= 0;				
	unsigned int	curPosWithInLine	= 0;
	unsigned int	curCharPos			= posCharInText - p.posWithInSegment;	

	for (curPosWithInLine = 0; curPosWithInLine < p.posWithInSegment; curPosWithInLine++, curCharPos++) {
		curPixelPos += getCharWidthInPixel(text.at(curCharPos));
	}

	return curPixelPos;
}

//-----------------------------------------------------------------------------
// Name: windowSizeChanged()
// Desc: 
//-----------------------------------------------------------------------------
float wildWeasel::editField2D::getCharWidthInPixel(WCHAR c)
{
	return textFont->getCharSize(c).x;
}

//-----------------------------------------------------------------------------
// Name: create()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::editField2D::createScrollBars(buttonImageFiles &filesTriangle, buttonImageFiles &filesLane, buttonImageFiles &filesSledge)
{
	// create
	scrollBarDuo::createScrollBars(this, filesTriangle, filesLane, filesSledge, ww, mat, dirtyBit, targetRectPositionZ);
	return true;
}

//-----------------------------------------------------------------------------
// Name: updateSledgeWidth()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::updateSledgeWidth()
{
	// locals
	textLineStruct* curLine;

	// update 'transScaledTargetRect'
	calcFinalMatrix();

	// ... might be wrong, since border could be considered twice when matrix/transScaledTargetRect is not re-calculated
	transScaledTargetRect.left		+= borderWidth;
	transScaledTargetRect.top		+= borderWidth;
	transScaledTargetRect.right		-= borderWidth;
	transScaledTargetRect.bottom	-= borderWidth;

	// ... could be calculated incrementally by each function
	visibleAreaSize.x	= (transScaledTargetRect.right - transScaledTargetRect.left);
	visibleAreaSize.y	= (transScaledTargetRect.bottom - transScaledTargetRect.top);
	totalAreaSize.x		= 0;
	totalAreaSize.y		= 0;
	for (auto& curSegment : lines.chain) {
		curLine = (textLineStruct*) curSegment;
		if (totalAreaSize.x < curLine->sizeInPixels.x) {
			totalAreaSize.x = curLine->sizeInPixels.x;
		}
		totalAreaSize.y += curLine->sizeInPixels.y;
	}
	totalAreaSize.x		+= (float) (rowScrollBarIsVisible?rowScrollBarWidth:0);
	totalAreaSize.y		+= (float) (columnScrollBarIsVisible?columnScrollbarHeight:0);

	if (scrollBarsCreated) {
		columnScrollBar	->setSledgeWidth(visibleAreaSize.x / totalAreaSize.x);
		rowScrollBar	->setSledgeWidth(visibleAreaSize.y / totalAreaSize.y);
		columnScrollBar	->alignAllItems();
		rowScrollBar	->alignAllItems();
	}
}

//-----------------------------------------------------------------------------
// Name: columnScrollBarMoved()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::columnScrollBarMoved(scrollBar2D* bar, void* pUser)
{
	alignAllItems();
}

//-----------------------------------------------------------------------------
// Name: rowScrollBarMoved()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::rowScrollBarMoved(scrollBar2D* bar, void* pUser)
{
	alignAllItems();
}

//-----------------------------------------------------------------------------
// Name: alignAllItems()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::alignAllItems()
{
	alignedRect::updateTargetRect(ww->alignmentRootFrame);

	// update 'transScaledTargetRect'
	// ... calcFinalMatrix();

	// scrollbars
	if (scrollBarsCreated) {
		columnScrollBar	->setRotation	(0, false);
		columnScrollBar	->setPosition	(targetRect.left, targetRect.bottom - columnScrollbarHeight, false);
		columnScrollBar	->setScale		(targetRect.right - targetRect.left - (rowScrollBarIsVisible?rowScrollBarWidth:0), (float) columnScrollbarHeight, true);
		rowScrollBar	->setRotation	(wwc::PI/2, false);
		rowScrollBar	->setPosition	(targetRect.right, targetRect.top, false);	
		rowScrollBar	->setScale		(targetRect.bottom - targetRect.top - (columnScrollBarIsVisible?columnScrollbarHeight:0), (float) rowScrollBarWidth, true);
		columnScrollBar	->alignAllItems();
		rowScrollBar	->alignAllItems();
		updateSledgeWidth();
	}
}

//-----------------------------------------------------------------------------
// Name: assignOnValueEntered()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::assignOnValueEntered(function<void(guiElement*, void*)> userFunc, void* pUser)
{
	this->userValueEntered	= userFunc;
	this->pUser				= pUser;
}
	
//-----------------------------------------------------------------------------
// Name: blinkTimerFunc()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::blinkTimerFunc(void* pUser)
{
	editField2D* myField = (editField2D*) pUser;
	myField->textCursor.isVisible	= (!myField->textCursor.isVisible);
}

//-----------------------------------------------------------------------------
// Name: setText()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::setText(const WCHAR* newText)
{
	reset();
	insertString(newText);
}
		
//-----------------------------------------------------------------------------
// Name: setText()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::setText(const wstring& newText)
{
	setText(newText.c_str());
}

//-----------------------------------------------------------------------------
// Name: renderSprites()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::renderSprites(ssf::sharedVars& v)
{
	// parameters ok?
	if (!initialized) return;
	if (!textFont) return;
	if (status != guiElemState::DRAWED && status != guiElemState::VISIBLE && status != guiElemState::GRAYED) return;

	// update 'transScaledTargetRect'
	calcFinalMatrix();

	// locals
	texture*	curTexture		= &ww->whiteDummyTexture;
	fRECT		sourceRect		= { 0, 0, 1, 1 };

	// reduce transScaledTargetRect by scrollbars
	if (rowScrollBarIsVisible) {
		transScaledTargetRect.right -= (rowScrollBarWidth * scale.x);
	}
	if (columnScrollBarIsVisible) {
		transScaledTargetRect.bottom -= (columnScrollbarHeight * scale.y);
	}

	// background
	if (curTexture) {
		sprite.draw(*curTexture, transScaledTargetRect, &sourceRect, mainColor, targetRectRotation, vector2(0,0), targetRectPositionZ);
	}
	
	// after having drawn the background the border should be considered
	transScaledTargetRect.left		+= borderWidth;
	transScaledTargetRect.top		+= borderWidth;
	transScaledTargetRect.right		-= borderWidth;
	transScaledTargetRect.bottom	-= borderWidth;

	// text
	if (text.size()) {
		size_t							fromChar, curCharInLine, numChars, numRestingCharsFormat;
		vector2							finalPosition, posInSideEditField, origin = vector2(0,0);
		list<textSegment*>::iterator	formatItr;
		list<textSegment*>::iterator	lineItr;
		textFormatStruct*				curFormat;
		textLineStruct*					curLine;

		// consider scrolling
		updateScrollOffset();

		// render each line
		posInSideEditField.x	= scrollOffset.x;
		posInSideEditField.y	= visibleArea.firstPixelPosY + scrollOffset.y;
		fromChar				= visibleArea.pText.posWithInSegment;
		lineItr					= visibleArea.pLine.itr;
		formatItr				= visibleArea.pFormat.itr;
		curFormat				= (textFormatStruct*) (*formatItr);
		curLine					= (textLineStruct*)   (*lineItr);
		numRestingCharsFormat	= curFormat->numChars - visibleArea.pFormat.posWithInSegment;
		for (; lineItr!=lines.chain.end(); lineItr++) {
			curLine					= (textLineStruct*) (*lineItr);
			numChars				= curLine->numChars;
			posInSideEditField.x	= scrollOffset.x;
			curCharInLine			= 0;

			// text cursor
			if (curTexture, textCursor.isVisible == true && textCursor.pLine.itr == lineItr) {
				vector2 P(scrollOffset.x + getPixelPosWithInLine(textCursor.pLine, textCursor.pText.posWithInSegment) - 1.2f * textCursor.widthInPixels, posInSideEditField.y);
				vector2 textCursorLeftTop;
				vector2 textCursorScale(textCursor.widthInPixels, curLine->sizeInPixels.y + 5);	// ... + 5 due to some strange border around the text
				rotateVectorAndTranslate(P, textCursorLeftTop, false, false, true);
				sprite.draw(*curTexture, textCursorLeftTop, &sourceRect, textCursor.color, targetRectRotation, vector2(0,0), textCursorScale, targetRectPositionZ + 0.1f);
			}

			// skip if no chars to render
			if (numChars == 0) continue;

			// render each different format
			do {
				// if toChar exceeds the current format range than stop at format range end
				if (curLine->numChars - curCharInLine > numRestingCharsFormat) {
					numChars = numRestingCharsFormat;
				}

				// render
				finalPosition.x	= transScaledTargetRect.left;
				finalPosition.y	= transScaledTargetRect.top;
				origin			= -posInSideEditField;
				textFont->draw(text.substr(fromChar, numChars).c_str(), finalPosition, curFormat->color, targetRectRotation, origin, curFormat->size, targetRectPositionZ, &transScaledTargetRect, curLine->sizeInPixels.y, false);
								
				// update vars
				curCharInLine			+= numChars;

				// go to next format, if necessary
				if (curCharInLine < curLine->numChars) {
					for (auto/*&*/ curCharPos = fromChar; curCharPos < fromChar + numChars; curCharPos++) {
						posInSideEditField.x   += getCharWidthInPixel(text.at(curCharPos));
					}
					formatItr++;
					numChars				= curLine->numChars - curCharInLine;
					curFormat				= (textFormatStruct*) (*formatItr);
					numRestingCharsFormat	= curFormat->numChars;
				} else {
					numRestingCharsFormat	-= numChars;
				}

				// update vars
				fromChar				+= numChars;

			} while (curCharInLine < (*curLine).numChars);

			// stop if bottom of edit field rect reached
			posInSideEditField.y += (*curLine).sizeInPixels.y;
			if (posInSideEditField.y > targetRect.bottom - targetRect.top + visibleArea.firstPixelPosY) {
				break;
			}
		}
	}

	// marker
	// for (auto& curMarker : markers.chain) {
	// 	// ...
	// }
}

//-----------------------------------------------------------------------------
// Name: renderSprites()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::pointerToTextSegment::init(textSegmentChain* tsc)
{
	this->tsc				= tsc;
	this->itr				= tsc->chain.begin();
	this->posWithInSegment	= 0;
	this->tsc->pointers.push_back(this);
}

//-----------------------------------------------------------------------------
// Name: moveRelative()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::pointerToTextSegment::moveRelative(int numTextSegments, int newPosWithInSegment, void* pUser)
{
	int lineCounter = 0;

	if (numTextSegments != 0) {

		while (itr != tsc->chain.end()) {

			if (numTextSegments == lineCounter) {
				break;
			}

			if (numTextSegments > 0) {
				(*itr)->advanceOneSegment(pUser);
				lineCounter++;
				itr++;
			} else {
				if (itr == tsc->chain.begin()) break;
				itr--;
				lineCounter--;
				(*itr)->reverseOneSegment(pUser);	
			}
		}

		if (itr == tsc->chain.end()) {
			itr--;
			lineCounter--;
			(*itr)->reverseOneSegment(pUser);	
		}
	}
	
	if (newPosWithInSegment < (int) 0) newPosWithInSegment = (int) 0;
	if (newPosWithInSegment > (int) (*itr)->numChars) newPosWithInSegment = (int) (*itr)->numChars;
	this->posWithInSegment	= newPosWithInSegment;
}

//-----------------------------------------------------------------------------
// Name: moveRelative()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::pointerToTextSegment::moveRelative(int numChars, void* pUser)
{
	int charCounter = 0;

	if (numChars == 0) return;

	while (itr != tsc->chain.end()) {

		if (charCounter == numChars) {
			break;
		}
		
		if (numChars > 0 && posWithInSegment == (*itr)->numChars) {
			(*itr)->advanceOneSegment(pUser);
			posWithInSegment = 0;
			itr++;
		} else if (numChars < 0 && posWithInSegment == 0) {
			if (itr == tsc->chain.begin()) break;
			itr--;
			posWithInSegment = (*itr)->numChars;
			(*itr)->reverseOneSegment(pUser);	
		}

		if (numChars > 0) {
			charCounter++;
			posWithInSegment++;
		} else {
			charCounter--;
			posWithInSegment--;
		}
	}

	if (itr == tsc->chain.end()) {
		itr--;
		posWithInSegment = (*itr)->numChars;
		(*itr)->reverseOneSegment(pUser);	
	}
}

//-----------------------------------------------------------------------------
// Name: moveAbsolute()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::pointerToTextSegment::moveAbsolute(unsigned int newCharPos, void* pUser)
{
	itr					= tsc->chain.begin();
	posWithInSegment	= 0;
	moveRelative(newCharPos, pUser);
}

//-----------------------------------------------------------------------------
// Name: splitTextSegment()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::pointerToTextSegment::splitTextSegment(unsigned int splitPosWithInSegment, textSegment* newTextSegment)
{
	// parameters ok?
	if (itr == tsc->chain.end()) return;

	// locals
	list<textSegment*>::iterator preItr = itr;
	list<textSegment*>::iterator sucItr = itr;

	// new iterator
	sucItr++;
	tsc->chain.insert(sucItr, newTextSegment);
	sucItr--;

	// adjust text segment width of both elements
	(*sucItr)->numChars	= (*preItr)->numChars - splitPosWithInSegment;
	(*preItr)->numChars	= splitPosWithInSegment;

	// adjust pointers if necessary
	for (auto& curPointer : tsc->pointers) {
		if (curPointer->itr == itr && curPointer->posWithInSegment >= splitPosWithInSegment) {
			curPointer->itr = sucItr;
			curPointer->posWithInSegment -= splitPosWithInSegment;
		}
	}
}

//-----------------------------------------------------------------------------
// Name: mergeWithNextSegment()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::pointerToTextSegment::mergeWithNextSegment()
{
	// parameters ok?
	if (itr == tsc->chain.end()) return;

	// locals
	list<textSegment*>::iterator myItr = itr;
	myItr++;

	// adjust pointers if necessary
	for (auto& curPointer : tsc->pointers) {
		if (curPointer->itr == myItr) {
			curPointer->itr = itr;
			curPointer->posWithInSegment += (*itr  )->numChars;
		}
	}

	// adjust text segment width of both elements
	(*itr  )->numChars	+= (*myItr)->numChars;
	tsc->chain.erase(myItr);
}

//-----------------------------------------------------------------------------
// Name: changeTextSegmentWidth()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::pointerToTextSegment::changeTextSegmentWidth(int changeWidth, unsigned int atPosWithInSegment)
{
	// locals
	unsigned int absChangeWidth = abs(changeWidth);

	// adjust pointers if necessary
	for (auto& curPointer : tsc->pointers) {
		if (curPointer->itr == itr && curPointer->posWithInSegment > atPosWithInSegment) {
			if (changeWidth > 0) {
				curPointer->posWithInSegment += absChangeWidth;
			} else if (curPointer->posWithInSegment >= absChangeWidth) {
				curPointer->posWithInSegment -= absChangeWidth;
			} else {
				curPointer->posWithInSegment = 0;
			}
		}
	}

	// change text segment size
	if (changeWidth > 0) {
		(*itr  )->numChars	+= absChangeWidth;
	} else if ((*itr  )->numChars >= absChangeWidth) {
		(*itr  )->numChars	-= absChangeWidth;
	} else {
		(*itr  )->numChars	= 0;
	}
}

//-----------------------------------------------------------------------------
// Name: moveBorderLine()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::pointerToTextSegment::moveBorderLine(int numChars)
{
	// parameters ok?
	if (itr == tsc->chain.end()) return;

	// locals
	list<textSegment*>::iterator myItr = itr;
	unsigned int absNumChars = abs(numChars);
	myItr++;

	// adjust pointers if necessary
	for (auto& curPointer : tsc->pointers) {
		if (curPointer->itr == itr && numChars < 0 && absNumChars > (*itr  )->numChars - curPointer->posWithInSegment) {
			if ((*itr  )->numChars >= absNumChars) {
				curPointer->posWithInSegment = (*itr  )->numChars - absNumChars;
			} else {
				curPointer->posWithInSegment = 0;
			}
		} else if (curPointer->itr == myItr && numChars > 0 && absNumChars > curPointer->posWithInSegment) {
				curPointer->posWithInSegment += absNumChars;
		}
	}

	// change text segment size
	if (numChars > 0) {
		(*itr  )->numChars	+= absNumChars;
		if ((*myItr)->numChars >= absNumChars) {
			(*myItr)->numChars	-= absNumChars;
		} else {
			(*myItr)->numChars	= 0;
		}
	} else if ((*itr  )->numChars >= absNumChars) {
		(*itr  )->numChars	-= absNumChars;
		(*myItr)->numChars	+= absNumChars;
	} else {
		(*itr  )->numChars	= 0;
		(*myItr)->numChars	+= absNumChars;
	}
}

//-----------------------------------------------------------------------------
// Name: clear()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::textSegmentChain::clear()
{
	for (auto& myTS : chain) {
		delete myTS;
	}
	chain.clear();
	pointers.clear();
}

//-----------------------------------------------------------------------------
// Name: advanceOneSegment()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::textLineStruct::advanceOneSegment(void* pUser)
{
	if (pUser == nullptr) return;
	segmentChangeVars* lineVars = (segmentChangeVars*) pUser;
	lineVars->pixelsMovedY	+= sizeInPixels.y;
	lineVars->charsMoved	+= numChars;
}

//-----------------------------------------------------------------------------
// Name: reverseOneSegment()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::editField2D::textLineStruct::reverseOneSegment(void* pUser)
{
	if (pUser == nullptr) return;
	segmentChangeVars* lineVars = (segmentChangeVars*) pUser;
	lineVars->pixelsMovedY	-= sizeInPixels.y;
	lineVars->charsMoved	-= numChars;
}
#pragma endregion

/*************************************************************************************************************************************/