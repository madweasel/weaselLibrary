/*********************************************************************
	wwguiElement.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/

#include "wildWeasel.h"

#pragma region guiElement
//-----------------------------------------------------------------------------
// Name: guiElement()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::guiElement::guiElement()
{
}

//-----------------------------------------------------------------------------
// Name: guiElement()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::guiElement2D::guiElement2D()
{
	matrixChain::insertMatrix(0, &targetRectMat3D,	&targetRectDirtyBit);	// matrix representing the targetRect	(setTargetRect())
	matrixChain::insertMatrix(1, &mat,				&dirtyBit);				// matrix of the gui element			(setPosition(), setScale(), setRotation())
}

//-----------------------------------------------------------------------------
// Name: guiElement()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::guiElement3D::guiElement3D()
{
	matrixChain::insertMatrix(0, &mat, &dirtyBit);
}

//-----------------------------------------------------------------------------
// Name: guiElement()
// Desc: 
//-----------------------------------------------------------------------------
wstring wildWeasel::guiElement3D::whyIsTextNotVisible()
{
	wstring ancestorReason = guiElement::whyAmINotVisible();
	if (ancestorReason != wwc::REASON_NOT_FOUND) return ancestorReason;

	// has text font been initialized
	if (!textFont) return L"Pointer 'textFont' is null, since function setFont() was not called properly.";

	return wwc::REASON_NOT_FOUND;
}

//-----------------------------------------------------------------------------
// Name: guiElement()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::guiElement::~guiElement()
{
	if (ww) {
		ww->unregisterGuiElement(this);
		// ... is already done by ~genericObject3D();
		// if (parent->threeD.initialized) {
		// 	parent->threeD.removeObjectToLoad(this);
		// 	parent->threeD.removeObjectToDraw(this);
		// }
	}
	if (cluster) {
	//	cluster->items.remove(this);
	// ... geht so nicht, wenn von cluster aus aufgerufen !
	}
	// guiMenue...
	// eventFollowerLists...
	if (animation) {
		if (ownerOfAnimation) {
			delete animation;
		} else {
			animation->elemWasDeleted();
		}
	}
}

//-----------------------------------------------------------------------------
// Name: whyAmINotVisible()
// Desc: 
//-----------------------------------------------------------------------------
wstring wildWeasel::guiElement::whyAmINotVisible()
{
	// Function create() must be called with parameter ww
	if (!ww) return L"Pointer 'ww' is null, since function create() was not called.";
	if (!initialized) return L"Variable 'initialized' is false, since function create() was not called.";

	// is visible state
	if (!isGuiElemStateVisible(status))	return L"Variable 'status' is neither DRAWED, GRAYED nor VISIBLE. Call setState() properly.";

	// check if color is equal to background
	// ... mainColor

	// is texture initialized
	//if (!mainTexture ) return L"Variable 'mainTexture' is null, since function setTexture() passed a pointer, which is still null.";
	if (mainTexture && !mainTexture->getTextureResource()) return L"Variable 'mainTexture->realTexture' is null, since texture passed in setTexture() was not loaded yet.";

	// was function performResourceUpload() calleld? 
	if (!uploaded) return L"Variable 'uploaded' is false, since function performResourceUpload() was not called after creation.";

	// am I part of the static lists?
	if ( isInListObjectsToLoad()) return L"This object is still in the list 'objectsToLoad', since function performResourceUpload() was not called after creation.";
	if (!isInListObjectsToDraw()) return L"This object is not in the list 'objectsToDraw', since function performResourceUpload() was not called after creation.";
	
	// check 'spritesToDraw'
	// ...

	// check 'descriptorHeaps'
	// ...

	return wwc::REASON_NOT_FOUND;
}


//-----------------------------------------------------------------------------
// Name: whyIsTextNotVisible()
// Desc: 
//-----------------------------------------------------------------------------
wstring wildWeasel::guiElement::whyIsTextNotVisible()
{
	// visible state?
	if (!isGuiElemStateVisible(textState)) return L"Variable 'textState' is neither DRAWED, GRAYED nor VISIBLE. Call setState() properly.";

	// same color as background?
	if (textColor.isSameAs(mainColor, 0.3f)) return L"Text color is nearly equal to the background color. Call setTextColor() properly.";

	// is scale to small or big?
	if (textScale.Length() <  0.1f) return L"Text scale is too small. Call setTextScale() properly.";
	if (textScale.Length() > 10.0f) return L"Text scale is too big. Call setTextScale() properly.";
	
	// is border too broad?
	// ... 'textBorder' cannot be checked without variable 'targetRect'
	
	return wwc::REASON_NOT_FOUND;
}

//-----------------------------------------------------------------------------
// Name: processInTime()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElement::setText(const WCHAR* newText)
{
	text.assign(newText);

	// ... work-around for chars, which does not exist in the font file. should be solved differently
	replace(text.begin(), text.end(), 8217,  39);	// ''' -> "'"	
	replace(text.begin(), text.end(), '\t', ' ');
};

//-----------------------------------------------------------------------------
// Name: setText()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElement::setText(const wstring& newText)
{
	setText(newText.c_str());
};

//-----------------------------------------------------------------------------
// Name: processInTime()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElement::processInTime(float totalSeconds, float elapsedSeconds)
{
	if (animation) {
		animation->processInTime(totalSeconds, elapsedSeconds);
	}
}

//-----------------------------------------------------------------------------
// Name: setAlignmentWithAnimation()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElement2D::setAlignmentWithAnimation(alignment& newAlignment, float durationInSeconds, bool doNotChangeStartAndFinalAlignment)
{
	// locals
	alignedRectAnimation*	myAnimation = nullptr;

	// delete current animation if there is any
	if (animation != nullptr) {
		if (!ownerOfAnimation) return;
		delete animation;
		animation = nullptr;
	}

	if (animation == nullptr) {

		// create animation
		myAnimation			= new alignedRectAnimation(ww, *this, *this, doNotChangeStartAndFinalAlignment);
		ownerOfAnimation	= true;
		animation			= myAnimation;
	}

	// start animation
	myAnimation->start(durationInSeconds, newAlignment);
}

//-----------------------------------------------------------------------------
// Name: blinkVisibility()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElement::blinkVisibility(float duration, unsigned int numberOfTimes, wildWeasel::guiElemState finalState)
{
	blinkAnimation* myAnimation = nullptr;

	// A) list<guiElemAnimation*> animations
	// B) several pointers, one for each type
	// C) delete & new
	// D) ???

	if (animation != nullptr) {
		if (!ownerOfAnimation) return;
		delete animation;
		animation			= nullptr;
	}

	if (animation == nullptr) {
		myAnimation			= new blinkAnimation(ww, *this);
		ownerOfAnimation	= true;
		animation			= myAnimation;
	}
	
	myAnimation->startBlinkVisibility(duration, numberOfTimes, finalState);
}

//-----------------------------------------------------------------------------
// Name: guiElement2D::calcFinalMatrix()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElement2D::calcFinalMatrix()
{
	// update target rect
	alignedRect::updateTargetRect(ww->alignmentRootFrame);

	// targetRect matrix (could be put at a place where targetRect is changed)
	targetRectMat3D._11	= (targetRect.right - targetRect.left);
	targetRectMat3D._22	= (targetRect.bottom- targetRect.top );
	targetRectMat3D._33	= 1;
	targetRectMat3D._41	= targetRect.left;
	targetRectMat3D._42	= targetRect.top;
	targetRectMat3D._43	= 0;
	targetRectDirtyBit	= true;

	// apply matrices and calc dimensions
	if (matrixChain::calcFinalMatrix()) {
		updateTransScaledRect(finalMatrix, targetRect);
	}
}

//-----------------------------------------------------------------------------
// Name: updateRotatedRect()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElement2D::updateTransScaledRect(matrix& mat, fRECT& rc)
{
	// locals
	vector2 translation2D, scale2D;

	// mat.Decompose(matScale, matRot, matTrans);	// ... maybe slow
	translation2D.x					= mat._41;													
	translation2D.y					= mat._42;													
	scale2D.x						= vector3(mat._11, mat._12, mat._13).Length();				
	scale2D.y						= vector3(mat._21, mat._22, mat._23).Length();				
	targetRectRotation				= -1 * atan2(mat._21 / scale2D.y, mat._11 / scale2D.x);
	targetRectRotMat2D.x			= cos(targetRectRotation);
	targetRectRotMat2D.y			= sin(targetRectRotation);
	transScaledTargetRect.left		= (translation2D.x);
	transScaledTargetRect.top		= (translation2D.y);
	transScaledTargetRect.right		= (translation2D.x + scale2D.x);
	transScaledTargetRect.bottom	= (translation2D.y + scale2D.y);

	// transScaledClippingRect			= 
	// ... a second matrix chain would be necessary
}

//-----------------------------------------------------------------------------
// Name: calcTextDimensions()
// Desc: CAUTION: scale must already have a starting value (e.g. (1,1)) !!!
//-----------------------------------------------------------------------------
void wildWeasel::guiElement2D::calcTextDimensions(vector2& fromPos, vector2& scale)
{								  
	// locals
	fRECT		textDrawBounds		= textFont->measureDrawBounds(text.c_str());
				scale				= textScale;

	// update matrix, just in case
	calcFinalMatrix();

	// calc rect for text inside targetRect so that text alignment is considered
	switch (alignHorizontal)
	{
	case alignmentHorizontal::LEFT:		fromPos.x = (transScaledTargetRect.left - scale.x * textDrawBounds.left) + textBorder.x;																																break;
	case alignmentHorizontal::CENTER:	fromPos.x = (transScaledTargetRect.left - scale.x * textDrawBounds.left)				+ 0.5f * ((transScaledTargetRect.right - transScaledTargetRect.left) - scale.x * (textDrawBounds.right - textDrawBounds.left));	break;
	case alignmentHorizontal::RIGHT:	fromPos.x = (transScaledTargetRect.left - scale.x * textDrawBounds.left) - textBorder.x	+        ((transScaledTargetRect.right - transScaledTargetRect.left) - scale.x * (textDrawBounds.right - textDrawBounds.left));	break;
	case alignmentHorizontal::BLOCK:	fromPos.x = (transScaledTargetRect.left - scale.x * textDrawBounds.left) + textBorder.x;																																break;
	}
	switch (alignVertical)
	{
	case alignmentVertical::TOP:		fromPos.y = (transScaledTargetRect.top - scale.y * textDrawBounds.top) + textBorder.y;																																	break;
	case alignmentVertical::CENTER:		fromPos.y = (transScaledTargetRect.top - scale.y * textDrawBounds.top)					+ 0.5f * ((transScaledTargetRect.bottom - transScaledTargetRect.top) - scale.y * (textDrawBounds.bottom - textDrawBounds.top));	break;
	case alignmentVertical::BOTTOM:		fromPos.y = (transScaledTargetRect.top - scale.y * textDrawBounds.top) - textBorder.y	+        ((transScaledTargetRect.bottom - transScaledTargetRect.top) - scale.y * (textDrawBounds.bottom - textDrawBounds.top));	break;
	case alignmentVertical::BELOW:		fromPos.y = (transScaledTargetRect.top - scale.y * textDrawBounds.top) + textBorder.y	+         (transScaledTargetRect.bottom - transScaledTargetRect.top);															break;
	case alignmentVertical::ABOVE:		fromPos.y = (transScaledTargetRect.top - scale.y * textDrawBounds.top) - textBorder.y	+        (															- scale.y * (textDrawBounds.bottom - textDrawBounds.top));	break;
	}

	// perform rotation, since text is shifted inside transScaledTargetRect
	vector2 P(fromPos.x - transScaledTargetRect.left, fromPos.y - transScaledTargetRect.top);
	rotateVectorAndTranslate(P, fromPos, false, false, true);
}

//-----------------------------------------------------------------------------
// Name: calcSpriteDimensions()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElement2D::calcSpriteDimensions(vector2& fromPos, vector2& toPos)
{
	fromPos.x	= transScaledTargetRect.left;
	fromPos.y	= transScaledTargetRect.top;
	toPos.x		= transScaledTargetRect.right;
	toPos.y		= transScaledTargetRect.bottom;
}

//-----------------------------------------------------------------------------
// Name: rotateVectorAndTranslate()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElement2D::rotateVectorAndTranslate(vector2 aVector, vector2& rotatedVector, bool invertRotation, bool translateBefore, bool translateAfter)
{
	if (translateBefore) {
		aVector.x -= transScaledTargetRect.left;
		aVector.y -= transScaledTargetRect.top;
	}

	if (!invertRotation) {
		rotatedVector.x = targetRectRotMat2D.x * aVector.x - targetRectRotMat2D.y * aVector.y;
		rotatedVector.y = targetRectRotMat2D.y * aVector.x + targetRectRotMat2D.x * aVector.y;
	} else {																				 
		rotatedVector.x = targetRectRotMat2D.x * aVector.x + targetRectRotMat2D.y * aVector.y;
		rotatedVector.y =-targetRectRotMat2D.y * aVector.x + targetRectRotMat2D.x * aVector.y;
	}

	if (translateAfter) {
		rotatedVector.x += transScaledTargetRect.left;
		rotatedVector.y += transScaledTargetRect.top;
	}
}
#pragma endregion

/*************************************************************************************************************************************/

#pragma region guiElemCluster
//-----------------------------------------------------------------------------
// Name: guiElemCluster()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::guiElemCluster::create(masterMind* ww)
{
	this->ww = ww;
	initialized	= true;
	return initialized;
}

//-----------------------------------------------------------------------------
// Name: guiElemCluster()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::guiElemCluster::~guiElemCluster()
{
	items.clear();
}

//-----------------------------------------------------------------------------
// Name: setState()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemCluster::setState(guiElemState newStatus)
{
	// Update Button States
	for (auto& curItem : items) {
		// BUG: guiElemEvFol::setState function is not called here 
		curItem->setState(newStatus);
	}
}

//-----------------------------------------------------------------------------
// Name: setTextStates()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemCluster::setTextStates(guiElemState newStatus)
{
	for (auto& curItem : items) {
		curItem->setTextState(newStatus);
	}
}

//-----------------------------------------------------------------------------
// Name: setTextAlignmentHorizontal()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemCluster::setTextAlignmentHorizontal(alignmentHorizontal newAlign)
{
	for (auto& curItem : items) {
		curItem->setTextAlignmentHorizontal(newAlign);
	}
}

//-----------------------------------------------------------------------------
// Name: setTextAlignmentVertical()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemCluster::setTextAlignmentVertical(alignmentVertical newAlign)
{
	for (auto& curItem : items) {
		curItem->setTextAlignmentVertical(newAlign);
	}
}

//-----------------------------------------------------------------------------
// Name: insertMatrix()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemCluster::insertMatrix(unsigned int position, matrix* additionalMatrix, bool* additionalDirtyBit)
{
	for (auto& curItem : items) {
		curItem->insertMatrix(position, additionalMatrix, additionalDirtyBit);
	}
}

//-----------------------------------------------------------------------------
// Name: removeMatrix()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemCluster::removeMatrix(matrix* matrixToRemove)
{
	for (auto& curItem : items) {
		curItem->removeMatrix(matrixToRemove);
	}
}

//-----------------------------------------------------------------------------
// Name: setTexts()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemCluster::setTexts(wstring& newText)
{
	// Update Button States
	for (auto& curItem : items) {
		curItem->setText(newText);
	}
}

//-----------------------------------------------------------------------------
// Name: setTexture()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemCluster::setTexture(texture* newTexture)
{
	// Update Button States
	for (auto& curItem : items) {
		curItem->setTexture(newTexture);
	}
}

//-----------------------------------------------------------------------------
// Name: setButtonColor()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemCluster::setButtonColor(color newColor)
{
	// Update Button States
	for (auto& curItem : items) {
		curItem->setColor(newColor);
	}
}

//-----------------------------------------------------------------------------
// Name: setTextSize()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemCluster::setTextBorder(float bx, float by)
{
	// Update Button States
	for (auto& curItem : items) {
		curItem->setTextBorder(bx, by);
	}
}

//-----------------------------------------------------------------------------
// Name: setTextSize()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemCluster::setTextSize(float sx, float sy)
{
	// Update Button States
	for (auto& curItem : items) {
		curItem->setTextSize(sx, sy);
	} 
}

//-----------------------------------------------------------------------------
// Name: setTextColor()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemCluster::setTextColor(color newColor)
{
	// Update Label States
	for (auto& curItem : items) {
		curItem->setTextColor(newColor);
	}
}

//-----------------------------------------------------------------------------
// Name: addItem()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemCluster::addItem(guiElement* newItem)
{
	// add cluster matrix to gui element
	newItem->cluster = this;

	// add gui element to list
	items.push_back(newItem);
}

//-----------------------------------------------------------------------------
// Name: addItem()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemCluster2D::addItem(guiElement2D* newItem)
{
	guiElemCluster::addItem(newItem);

	// add cluster matrix to gui element
	newItem->insertMatrix(2/*1*/, &this->mat, &this->dirtyBit);
}

//-----------------------------------------------------------------------------
// Name: addItem()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemCluster3D::addItem(guiElement3D* newItem)
{
	guiElemCluster::addItem(newItem);

	// add cluster matrix to gui element
	newItem->insertMatrix(1, &this->mat, &this->dirtyBit);
}

//-----------------------------------------------------------------------------
// Name: deleteLastItem()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemCluster::deleteLastItem()
{
	delete items.back();
	items.pop_back();
}

//-----------------------------------------------------------------------------
// Name: deleteAllItems()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemCluster::deleteAllItems()
{
	for (auto& curItem : items) {
		delete curItem;
		curItem = nullptr;
	}
	items.clear();
}

//-----------------------------------------------------------------------------
// Name: clearItemList()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemCluster::clearItemList()
{
	for (auto& curItem : items) {
		curItem->cluster = nullptr;
	}
	items.clear();
}

//-----------------------------------------------------------------------------
// Name: clearItemList()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemCluster2D::clearItemList()
{
	for (auto& curItem : items) {
		curItem->cluster = nullptr;
		if (typeid(curItem) == typeid(guiElement2D*)) {
			(static_cast<guiElement2D*>(curItem))->removeMatrix(&this->mat);
		}
	}
	items.clear();
}

//-----------------------------------------------------------------------------
// Name: clearItemList()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemCluster3D::clearItemList()
{
	for (auto& curItem : items) {
		curItem->cluster = nullptr;
		curItem->removeMatrix(&this->mat);
		// auto myGuiElem3D = curItem->getPointer<guiElement3D>();		// ... does not work when curItem is only inherits guiElement3D
		// if (myGuiElem3D != nullptr) {
		//		myGuiElem3D->removeMatrix(&this->mat);
		// }
	}
	items.clear();
}

//-----------------------------------------------------------------------------
// Name: setClippingRect()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemCluster2D::setClippingRect(fRECT* clipOnRect)
{
	// Update Button States
	for (auto& curItem : items) {
	//	if (typeid(*curItem) == typeid(guiElement2D)) {
			(static_cast<guiElement2D*>(curItem))->setClippingRect(clipOnRect);
	//	}
	}
}
#pragma endregion

/*************************************************************************************************************************************/

#pragma region guiScenario
//-----------------------------------------------------------------------------
// Name: setPointerToParent()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiScenario::setPointerToParent(masterMind *ww)
{
	parent = ww; 
};

//-----------------------------------------------------------------------------
// Name: isActive()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::guiScenario::isActive()
{
	return (parent ? parent->scenarioManager.activeScenario == this : false); 
};

//-----------------------------------------------------------------------------
// Name: setActiveScenario()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::scenarioManagerClass::setActiveScenario(guiScenario &newActiveScenario)
{
	if (activeScenario) activeScenario->deactivate();
	newActiveScenario.setPointerToParent(parent);
	newActiveScenario.activate();
	activeScenario = &newActiveScenario;
	return true;
}
#pragma endregion

/****************************************************************************************************************************/

#pragma region guiElemEvFol

// define and initialize static member variables
bool							wildWeasel::guiElemEvFol::um_uniqueButtonModeOn	= true;
bool							wildWeasel::guiElemEvFol::um_mouseEntered		= false;
wildWeasel::guiElemEvFol*	wildWeasel::guiElemEvFol::um_selectedButton		= nullptr;
wildWeasel::guiElemEvFol*	wildWeasel::guiElemEvFol::um_focusedButton		= nullptr;

//-----------------------------------------------------------------------------
// Name: guiElemEvFol()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::guiElemEvFol::guiElemEvFol(guiElemState* pStatus)
{
	// ... may be registration on event tracking could be done here
	this->pStatus	= pStatus;
}

//-----------------------------------------------------------------------------
// Name: guiElemEvFol()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::guiElemEvFol::~guiElemEvFol()
{
	if (this == um_focusedButton) {
		um_focusedButton	= nullptr;
	}
	if (this == um_selectedButton) {
		um_selectedButton	= nullptr;
		um_mouseEntered		= false;
	}
}

//-----------------------------------------------------------------------------
// Name: informMotherOnEvent()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemEvFol::informMotherOnEvent(eventFollower* mother)
{
	if (find(motherFollowers.begin(), motherFollowers.end(), mother) != motherFollowers.end()) return;
	motherFollowers.push_back(mother);
}

//-----------------------------------------------------------------------------
// Name: doNotInformMotherOnEvent()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemEvFol::doNotInformMotherOnEvent(eventFollower* mother)
{
	motherFollowers.erase(std::remove(motherFollowers.begin(), motherFollowers.end(), mother), motherFollowers.end());
}

//-----------------------------------------------------------------------------
// Name: focusThis()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemEvFol::focusThis()
{
	// old gui elem lost focus
	if (um_focusedButton != this && um_focusedButton != nullptr) {
		um_focusedButton->callEventFunc(eventFuncsLostFocus);
	}
	
	um_focusedButton		= this;
	
	// new gui elem got focus
	um_focusedButton->callEventFunc(eventFuncsGotFocus);
}

//-----------------------------------------------------------------------------
// Name: setState()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemEvFol::setState(guiElemState newStatus)
{
	// if element becomes inactive, it cannot be "mouse overred" any longer
	if (!isGuiElemStateActive(newStatus)) {
		isMouseOver				= false;
		mousePressedInRegion	= false;

		if (um_selectedButton == this) {

			// call virtual function, defined by sub-classes
			toolTipHide();
	
			// if already another button was entered, than call leave function first. otherwise the order of entering and leaving is disturbed.
			if ((!um_uniqueButtonModeOn) || (um_mouseEntered && um_selectedButton == this)) {
				this->callEventFunc(eventFuncsMouseLeftRegion);
				um_selectedButton	= nullptr;
				um_mouseEntered		= false;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Name: mouseMoved()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemEvFol::mouseMoved(int posX, int posY, const vector3& cursorPos, const vector3& cameraPos)
{
	// ignore if guiElemEvFol inactive
	if (*pStatus != guiElemState::DRAWED && *pStatus != guiElemState::INVISIBLE) return;
	
	// locals
	bool newMouseOver = isCursorOver(posX, posY, cameraPos, cursorPos, NULL);

	// call user function, when mouse was moved
	this->callEventFunc(eventFuncsMouseMove);

	// mouse was pressed in region
	if (newMouseOver && !isMouseOver && mousePressedInRegion) {
		isMouseOver = newMouseOver;
		// ... um_selectedButton->callEventFunc(eventFuncsLeftMouseButtonPressed);
	
	// region was entered
	} else if (newMouseOver && !isMouseOver) {

		// ... passt so noch nicht ganz !!! markierung findet beim verlassen von um_selectedButton nicht statt
		if (um_selectedButton != nullptr && um_uniqueButtonModeOn) return;

		isMouseOver = newMouseOver;

		if (um_selectedButton != nullptr) {
			um_selectedButton->toolTipHide();
		}

		// if already another guiElemEvFol was entered, than call leave function first. otherwise the order of entering and leaving is disturbed.
		if (um_uniqueButtonModeOn && um_mouseEntered && um_selectedButton != nullptr) {
			um_selectedButton->callEventFunc(eventFuncsMouseLeftRegion);
		}

		// call event functions
		toolTipShow();
		callEventFunc(eventFuncsMouseEnteredRegion);
		um_selectedButton	= this;
		um_mouseEntered		= true;

	// region was left
	} else if (!newMouseOver && isMouseOver) {
		isMouseOver = newMouseOver;

		// call virtual function, defined by sub-classes
		toolTipHide();
	
		// if already another button was entered, than call leave function first. otherwise the order of entering and leaving is disturbed.
		if ((!um_uniqueButtonModeOn) || (um_mouseEntered && um_selectedButton == this)) {
			this->callEventFunc(eventFuncsMouseLeftRegion);
			um_selectedButton	= nullptr;
			um_mouseEntered		= false;
		}
	}
}

//-----------------------------------------------------------------------------
// Name: leftMouseButtonPressed()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemEvFol::leftMouseButtonPressed(int xPos, int yPos)
{
	// ignore if button inactive
	if (*pStatus != guiElemState::DRAWED && *pStatus != guiElemState::INVISIBLE) return;

	if (isMouseOver) {
		mousePressedInRegion	= true;
		focusThis();
		this->callEventFunc(eventFuncsLeftMouseButtonPressed);
	}
}

//-----------------------------------------------------------------------------
// Name: leftMouseButtonReleased()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemEvFol::leftMouseButtonReleased(int xPos, int yPos)
{
	// ignore if guiElemEvFol inactive
	if (*pStatus != guiElemState::DRAWED && *pStatus != guiElemState::INVISIBLE) return;

	if (isMouseOver && mousePressedInRegion) {
		mousePressedInRegion = false;
		this->callEventFunc(eventFuncsLeftMouseButtonReleased);
	} else {
		mousePressedInRegion = false;
	}
}

//-----------------------------------------------------------------------------
// Name: rightMouseButtonPressed()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemEvFol::rightMouseButtonPressed(int xPos, int yPos)
{
	// ignore if guiElemEvFol inactive
	if (*pStatus != guiElemState::DRAWED && *pStatus != guiElemState::INVISIBLE) return;

	if (isMouseOver) {
		this->callEventFunc(eventFuncsRightMouseButtonPressed);
	}
}

//-----------------------------------------------------------------------------
// Name: rightMouseButtonReleased()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemEvFol::rightMouseButtonReleased(int xPos, int yPos)
{
	// ignore if button inactive
	if (*pStatus != guiElemState::DRAWED && *pStatus != guiElemState::INVISIBLE) return;

	if (isMouseOver) {
		this->callEventFunc(eventFuncsRightMouseButtonReleased);
	}
}

//-----------------------------------------------------------------------------
// Name: isCursorOver()
//-----------------------------------------------------------------------------
float wildWeasel::guiElemEvFol2D::calcArea(wildWeasel::vector2& A, wildWeasel::vector2& B, wildWeasel::vector2& C)
{
	return abs(A.x * (B.y - C.y) + B.x * (C.y -	A.y) + C.x * (A.y - B.y)) / 2;
}

//-----------------------------------------------------------------------------
// Name: isCursorOver()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::guiElemEvFol2D::isCursorOver(int posX, int posY, const vector3& cameraPosition, const vector3& cursorPos, float* distanceFromCamera)
{
	// parameters ok?
	if (*pStatus != guiElemState::DRAWED && *pStatus != guiElemState::INVISIBLE) return false;

    // difference vector from rotation center to mouse and rotate 
	vector2 P((float) posX, (float) posY), A;
	rotateVectorAndTranslate(P, A, true, true, true);

	// inside targetRectClipping ?
	if (targetRectClipping) {
		if (!(A.x > targetRectClipping->left && A.x < targetRectClipping->right && A.y > targetRectClipping->top && A.y < targetRectClipping->bottom)) {
			return false;
		}
	}

	// inside transScaledTargetRect ?
    return (A.x > transScaledTargetRect.left && A.x < transScaledTargetRect.right && A.y > transScaledTargetRect.top && A.y < transScaledTargetRect.bottom);
}

//-----------------------------------------------------------------------------
// Name: onMouseHoverAction()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemEvFol3D::onMouseHoverAction(guiElemEvFol * elem, void * pUser)
{
	color tmpColor = mainColor;
	setColor(hoverColor);
	hoverColor = tmpColor;
}

//-----------------------------------------------------------------------------
// Name: setHoverColor()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemEvFol3D::setHoverColor(color newHoverColor)
{
	// ... if hoverColor is already set then switch back to main color first
	// if (isMouseOver && ... ) setColor(hoverColor);

	hoverColor = newHoverColor;
	assignOnMouseEnteredRegion(bind(&wildWeasel::guiElemEvFol3D::onMouseHoverAction,	this, placeholders::_1, placeholders::_2), this);
	assignOnMouseLeftRegion	  (bind(&wildWeasel::guiElemEvFol3D::onMouseHoverAction,	this, placeholders::_1, placeholders::_2), this);
}

//-----------------------------------------------------------------------------
// Name: isCursorOver()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::guiElemEvFol3D::isCursorOver(int posX, int posY, const vector3& cameraPosition, const vector3& cursorPos, float* distanceFromCamera)
{
	// parameters ok?
	if (*pStatus != guiElemState::DRAWED && *pStatus != guiElemState::INVISIBLE) return false;

	matrix		mat;
	bool		isOver;
	float		dotProduct;

	getFinalMatrix(mat);

	isOver = rects.intersects(mat, cameraPosition, cursorPos, distanceFromCamera, &dotProduct);
	
	// when only one single plain/rect then consider, if looking on the backside
	if (isOver == true && dotProduct > 0 && rects.rects.size() == 1) {
		isOver = false;
	}

	return isOver;
}

//-----------------------------------------------------------------------------
// Name: whyAmINotVisible()
// Desc: 
//-----------------------------------------------------------------------------
wstring wildWeasel::guiElemEvFol3D::whyAmINotVisible()
{
	wstring ancestorReason = guiElement3D::whyAmINotVisible();
	if (ancestorReason != wwc::REASON_NOT_FOUND) return ancestorReason;

	// any vertices and rects present?
	if (!rects.vertices.size())				return L"Vector 'vertices' is empty, since function create() has not been called.";
	if (!rects.verticesTransformed.size())	return L"Vector 'verticesTransformed' is empty, since function create() has not been called.";
	if (!rects.rects.size())				return L"Vector 'rects' is empty, since function create() has not been called.";

	// are transformed rects inside the window?
	matrix matView, matProjection, matObj;
	camera* actCam = ww->getActiveCamera();

	actCam->makeViewMatrixFromCamera(matView);
	actCam->makeProjMatrixFromCamera(matProjection);
	getFinalMatrix(matObj);

	// BUG: Better would be to use the following variables. But they are currently inaccessible.
	// matObj			= finalMatrix
	// matView			= threeD.matView;
	// matProjection	= threeD.matProjection;

	for (auto& curVert : rects.vertices) {

		// is vertex behind the camera?
		vector3 objVec; 
		vector3::Transform(curVert, matObj, objVec);
		vector3 camVec = actCam->lookAt;
		camVec = camVec - actCam->position;
		camVec.Normalize();
		objVec = objVec - actCam->position;
		if (camVec.Dot(objVec) < 0) return L"Object is behind the camera.";

		// is object between Near and Far Plane?
		if (camVec.Dot(objVec) < actCam->zNear) return L"Object is closer than the near plane.";
		if (camVec.Dot(objVec) > actCam->zFar ) return L"Object is farer than the far plane.";

		// is object inside the camera view frstrum?
		vector3 curVertTransformed = curVert;
		vector3::Transform(curVertTransformed, matObj,			curVertTransformed);
		vector3::Transform(curVertTransformed, matView,			curVertTransformed);
		vector3::Transform(curVertTransformed, matProjection,	curVertTransformed);

		if (curVertTransformed.x < -1 || curVertTransformed.x > 1) return L"Transformed vertex is outside the viewing frustrum of the camera.";
		if (curVertTransformed.y < -1 || curVertTransformed.y > 1) return L"Transformed vertex is outside the viewing frustrum of the camera.";
	}

	return wwc::REASON_NOT_FOUND;
}

//-----------------------------------------------------------------------------
// Name: getSelectedGuiElem()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::guiElemEvFol* wildWeasel::guiElemEvFol::getSelectedGuiElem()
{
	return um_selectedButton;
}

//-----------------------------------------------------------------------------
// Name: getSelectedGuiElem()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::guiElemEvFol* wildWeasel::guiElemEvFol::setFocus(guiElemEvFol* newFocusedElement)
{
	guiElemEvFol* curFocusedElement	= um_focusedButton;
	newFocusedElement->focusThis();
	return curFocusedElement;
}

//-----------------------------------------------------------------------------
// Name: setUniqueButtonSelecionMode()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemEvFol::setUniqueButtonSelecionMode(bool onlyOneButtonSelectedAtAnyTime)
{
	this->um_uniqueButtonModeOn = onlyOneButtonSelectedAtAnyTime;
}

//-----------------------------------------------------------------------------
// Name: addOrRemoveEventFuncFromVector()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemEvFol::addOrRemoveEventFuncFromVector(eventFunction& eventFunc, vector<eventFunction>& eventFuncVector)
{
	// remove all functions with pUser, if userFunc is null
	if (eventFunc.func == nullptr) 	{

		vector<vector<eventFunction>::iterator> funcsToErase;

		// get all iterators, which shall be deleted
		for (auto itr = eventFuncVector.begin(); itr != eventFuncVector.end(); ++itr) {
			if (itr->pointer == eventFunc.pointer) {
				funcsToErase.push_back(itr);
			}
		}

		// remove iterators
		for (auto& itr : funcsToErase) {
			eventFuncVector.erase(itr);
		}

	// ... add function if not already in list. Problem is that function<> does not have a == operator.
	} else /*if (std::find(eventFuncVector.begin(), eventFuncVector.end(), eventFunc) != eventFuncVector.end()) */{
		eventFuncVector.push_back(eventFunc);
	}
}

//-----------------------------------------------------------------------------
// Name: callEventFunc()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemEvFol::callEventFunc(vector<eventFunction>& eventFuncVector)
{
	for (auto& curEventFunc : eventFuncVector) {
		if (curEventFunc.func) {
			curEventFunc.func(this, curEventFunc.pointer);
		}
	}
}

//-----------------------------------------------------------------------------
// Name: assignOnMouseMove()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemEvFol::assignOnMouseMove(function<void(guiElemEvFol*, void*)> userFunc, void* pUser)
{
	addOrRemoveEventFuncFromVector(eventFunction{userFunc, pUser}, eventFuncsMouseMove);
}

//-----------------------------------------------------------------------------
// Name: assignOnLeftMouseButtonPressed()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemEvFol::assignOnLeftMouseButtonPressed(function<void(guiElemEvFol*, void*)> userFunc, void* pUser)
{
	addOrRemoveEventFuncFromVector(eventFunction{userFunc, pUser}, eventFuncsLeftMouseButtonPressed);
}

//-----------------------------------------------------------------------------
// Name: assignOnLeftMouseButtonReleased()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemEvFol::assignOnLeftMouseButtonReleased(function<void(guiElemEvFol*, void*)> userFunc, void* pUser)
{
	addOrRemoveEventFuncFromVector(eventFunction{userFunc, pUser}, eventFuncsLeftMouseButtonReleased);
}

//-----------------------------------------------------------------------------
// Name: assignOnRightMouseButtonPressed()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemEvFol::assignOnRightMouseButtonPressed(function<void(guiElemEvFol*, void*)> userFunc, void* pUser)
{
	addOrRemoveEventFuncFromVector(eventFunction{userFunc, pUser}, eventFuncsRightMouseButtonPressed);
}

//-----------------------------------------------------------------------------
// Name: assignOnRightMouseButtonReleased()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemEvFol::assignOnRightMouseButtonReleased(function<void(guiElemEvFol*, void*)> userFunc, void* pUser)
{
	addOrRemoveEventFuncFromVector(eventFunction{userFunc, pUser}, eventFuncsRightMouseButtonReleased);
}

//-----------------------------------------------------------------------------
// Name: assignOnMouseEnteredRegion()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemEvFol::assignOnMouseEnteredRegion(function<void(guiElemEvFol*, void*)> userFunc, void* pUser)
{
	addOrRemoveEventFuncFromVector(eventFunction{userFunc, pUser}, eventFuncsMouseEnteredRegion);
}

//-----------------------------------------------------------------------------
// Name: assignOnMouseLeftRegion()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemEvFol::assignOnMouseLeftRegion(function<void(guiElemEvFol*, void*)> userFunc, void* pUser)
{
	addOrRemoveEventFuncFromVector(eventFunction{userFunc, pUser}, eventFuncsMouseLeftRegion);
}

//-----------------------------------------------------------------------------
// Name: assignOnGotFocus()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemEvFol::assignOnGotFocus(function<void(guiElemEvFol*, void*)> userFunc, void* pUser)
{
	addOrRemoveEventFuncFromVector(eventFunction{userFunc, pUser}, eventFuncsGotFocus);
}

//-----------------------------------------------------------------------------
// Name: assignOnLostFocus()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemEvFol::assignOnLostFocus(function<void(guiElemEvFol*, void*)> userFunc, void* pUser)
{
	addOrRemoveEventFuncFromVector(eventFunction{userFunc, pUser}, eventFuncsLostFocus);
}

//-----------------------------------------------------------------------------
// Name: toolTipGotFocus()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemEvFol::toolTipShow()
{
	if (toolTip == nullptr || strTooltip.size() == 0) return;
	toolTip->show(guiElemToolTip, strTooltip);
}

//-----------------------------------------------------------------------------
// Name: toolTipLostFocus()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemEvFol::toolTipHide()
{
	if (toolTip == nullptr || strTooltip.size() == 0) return;
	toolTip->hide(guiElemToolTip);
}

//-----------------------------------------------------------------------------
// Name: assignToolTip()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemEvFol::assignToolTip(const WCHAR* newString, toolTipClass* toolTip, guiElement* pGuiElem)
{
	this->strTooltip		.assign(newString);
	this->toolTip			= toolTip;
	this->guiElemToolTip	= pGuiElem;				// (guiElement*) this   <- does not work when converting to guiElem2D
}
#pragma endregion

/*****************************************************************************/

#pragma region toolTip
//-----------------------------------------------------------------------------
// Name: assignToolTip()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::toolTipClass::init(masterMind* ww, font2D* theFont)
{
	RECT rc = { 0, 0, 1, 1 };
	create(ww, wstring(L""), theFont);
	setTextSize(1.0f, 1.0f);
	setFont(theFont);
	setState(wildWeasel::guiElemState::HIDDEN);
	setTextState(wildWeasel::guiElemState::HIDDEN);
	setTextColor(wildWeasel::color::gray);
	setColor(wildWeasel::color::white);
	setPositioningMode(wildWeasel::matrixControl2D::matControlMode::posRotSca);
	setTextAlignmentHorizontal(wildWeasel::alignmentHorizontal::CENTER);
	setTextAlignmentVertical(wildWeasel::alignmentVertical::CENTER);
	setTargetRect(rc);
	setTexture(&ww->whiteDummyTexture);
}

//-----------------------------------------------------------------------------
// Name: show()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::toolTipClass::show(guiElement* elem, wstring& text)
{
	// set visibility
	setState(wildWeasel::guiElemState::VISIBLE);
	setTextState(wildWeasel::guiElemState::VISIBLE);
	setText(text);

	// position and size
	vector2 pos{0,0};
	vector2 size{1,1};
	fRECT	guiElemRect;
	float	guiElemRotation;
	fRECT	textDrawBounds;

	// size of tool tip
	textDrawBounds	= textFont->measureDrawBounds(text.c_str());
	size.x			= (textDrawBounds.right - textDrawBounds.left) * textScale.x;
	size.y			= (textDrawBounds.bottom - textDrawBounds.top) * textScale.y;

	// get dimension of considered gui element
	auto myElem2D = (guiElement2D*) elem; // elem->getPointer<guiElement2D>();		<- ... does not work this way. current work-around will crash if not elem not of type guiElement2D
	if (myElem2D != nullptr) {
		myElem2D->getTransScaledTarget(guiElemRect, guiElemRotation);
	} else {
		auto myElem3D = elem->getPointer<guiElement3D>();
		if (myElem3D != nullptr) {
			// ... guiElemRect, guiElemRotation
		}
	}
	
	// positioning
	calcPos(pos, size, guiElemRect, guiElemRotation);
	setPosition(&pos, false);
	setScale(&size, true);
}

//-----------------------------------------------------------------------------
// Name: hide()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::toolTipClass::hide(guiElement* elem)
{
	setState(wildWeasel::guiElemState::HIDDEN);
}

//-----------------------------------------------------------------------------
// Name: calcPos()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::toolTipClass::calcPos(vector2& toolTipPos, vector2& toolTipSize, fRECT& guiElemRect, float guiElemRotation)
{
	vector2 windowSize				= {(float) ww->getWindowSizeX(), (float) ww->getWindowSizeY()};
	float	distBetweenElemAndTip	= {10.0f};
	fRECT	spaceToWindowBorder		= {guiElemRect.left, guiElemRect.top, windowSize.x - guiElemRect.right, windowSize.y - guiElemRect.bottom};
	fRECT	spaceNeeded				= {toolTipSize.x + distBetweenElemAndTip, toolTipSize.y + distBetweenElemAndTip, toolTipSize.x + distBetweenElemAndTip, toolTipSize.y + distBetweenElemAndTip};

	if (spaceToWindowBorder.right > spaceNeeded.right) {
		toolTipPos.x	= guiElemRect.right + distBetweenElemAndTip;
		toolTipPos.y	= guiElemRect.centerY() - toolTipSize.y * 0.5f;
	} else if (spaceToWindowBorder.left > spaceNeeded.left) {
		toolTipPos.x	= guiElemRect.left - distBetweenElemAndTip - toolTipSize.x;
		toolTipPos.y	= guiElemRect.centerY() - toolTipSize.y * 0.5f;
	} else if (spaceToWindowBorder.top > spaceNeeded.top) {
		toolTipPos.x	= guiElemRect.centerX() - toolTipSize.x * 0.5f;
		toolTipPos.y	= guiElemRect.top -  distBetweenElemAndTip - toolTipSize.y;
	} else if (spaceToWindowBorder.bottom > spaceNeeded.bottom) {
		toolTipPos.x	= guiElemRect.centerX() - toolTipSize.x * 0.5f;
		toolTipPos.y	= guiElemRect.bottom + distBetweenElemAndTip;
	} else {
		toolTipPos.x	= 0;
		toolTipPos.y	= 0;
	}

	if (toolTipPos.x < distBetweenElemAndTip) toolTipPos.x = distBetweenElemAndTip; 
	if (toolTipPos.y < distBetweenElemAndTip) toolTipPos.y = distBetweenElemAndTip; 
	if (toolTipPos.x > windowSize.x - distBetweenElemAndTip - toolTipSize.x) toolTipPos.x = windowSize.x - distBetweenElemAndTip - toolTipSize.x; 
	if (toolTipPos.y > windowSize.y - distBetweenElemAndTip - toolTipSize.y) toolTipPos.y = windowSize.y - distBetweenElemAndTip - toolTipSize.y; 
}
#pragma endregion

/*****************************************************************************/

#pragma region cubicbutton
//-----------------------------------------------------------------------------
// Name: create()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::cubicButton::create(masterMind* ww)
{
	// params ok?
	if (!ww)			return false;
	if (initialized)	return false;

	// set reference to shared shape
	shape		= new ssf::shape::cube();
	this->ww	= ww;

	// polygons for mouse over detection
	float e = 0.5;

	vector3 ulf(+e,-e,+e);
	vector3 llf(-e,-e,+e);
	vector3 urf(+e,+e,+e);
	vector3 lrf(-e,+e,+e);
	vector3 ulb(+e,-e,-e);
	vector3 llb(-e,-e,-e);
	vector3 urb(+e,+e,-e);
	vector3 lrb(-e,+e,-e);

	rects.vertices.push_back(ulf);
	rects.vertices.push_back(llf);
	rects.vertices.push_back(urf);
	rects.vertices.push_back(lrf);
	rects.vertices.push_back(ulb);
	rects.vertices.push_back(llb);
	rects.vertices.push_back(urb);
	rects.vertices.push_back(lrb);
	
	rects.verticesTransformed.resize(rects.vertices.size());
	
	rects.rects.reserve(6);

	// ... die wahl der vertices ist manchmal falsch, da ein parallelogramm entsteht statt ein rechteck
	// ... hier sollte also noch eine sortierung folgen.
	rects.rects.push_back(rectIndizes(0, 1, 2, 3));
	rects.rects.push_back(rectIndizes(4, 5, 6, 7));
	rects.rects.push_back(rectIndizes(0, 2, 4, 6));
	rects.rects.push_back(rectIndizes(1, 3, 5, 7));
	rects.rects.push_back(rectIndizes(0, 1, 4, 5));
	rects.rects.push_back(rectIndizes(2, 3, 6, 7));
	rects.sortIndices();

	// remember to load the file later on together with the other ressources
	addObjectToLoad(ww);
	ww->registerGuiElement(this);
	addSpriteToDraw(0, nullptr);

	// track mouse moves and clicks
	eventFollower::followEvent(this, eventType::MOUSEMOVED);
	eventFollower::followEvent(this, eventType::LEFT_MOUSEBUTTON_PRESSED);
	eventFollower::followEvent(this, eventType::LEFT_MOUSEBUTTON_RELEASED);
	eventFollower::followEvent(this, eventType::RIGHT_MOUSEBUTTON_PRESSED);
	eventFollower::followEvent(this, eventType::RIGHT_MOUSEBUTTON_RELEASED);

	initialized = true;
	return initialized;
}
#pragma endregion

/*****************************************************************************/

#pragma region sphericalButton
//-----------------------------------------------------------------------------
// Name: create()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::sphericalButton::create(masterMind* ww)
{
	// params ok?
	if (!ww)			return false;
	if (initialized)	return false;

	// set reference to shared shape
	shape		= new ssf::shape::sphere();
	this->ww	= ww;

	// polygons for mouse over detection
	float e = 0.4f;

	vector3 ulf(+e,-e,+e);
	vector3 llf(-e,-e,+e);
	vector3 urf(+e,+e,+e);
	vector3 lrf(-e,+e,+e);
	vector3 ulb(+e,-e,-e);
	vector3 llb(-e,-e,-e);
	vector3 urb(+e,+e,-e);
	vector3 lrb(-e,+e,-e);

	rects.vertices.push_back(ulf);
	rects.vertices.push_back(llf);
	rects.vertices.push_back(urf);
	rects.vertices.push_back(lrf);
	rects.vertices.push_back(ulb);
	rects.vertices.push_back(llb);
	rects.vertices.push_back(urb);
	rects.vertices.push_back(lrb);
	
	rects.verticesTransformed.resize(rects.vertices.size());
	
	rects.rects.reserve(6);

	// ... die wahl der vertices ist manchmal falsch, da sonst ein parallelogramm entsteht statt ein rechteck
	// ... hier sollte also noch eine sortierung folgen.
	rects.rects.push_back(rectIndizes(0, 1, 2, 3));
	rects.rects.push_back(rectIndizes(4, 5, 6, 7));
	rects.rects.push_back(rectIndizes(0, 2, 4, 6));
	rects.rects.push_back(rectIndizes(1, 3, 5, 7));
	rects.rects.push_back(rectIndizes(0, 1, 4, 5));
	rects.rects.push_back(rectIndizes(2, 3, 6, 7));
	rects.sortIndices();

	// remember to load the file later on together with the other ressources
	addObjectToLoad(ww);
	ww->registerGuiElement(this);
	addSpriteToDraw(0, nullptr);

	// track mouse moves and clicks
	eventFollower::followEvent(this, eventType::MOUSEMOVED);
	eventFollower::followEvent(this, eventType::LEFT_MOUSEBUTTON_PRESSED);
	eventFollower::followEvent(this, eventType::LEFT_MOUSEBUTTON_RELEASED);
	eventFollower::followEvent(this, eventType::RIGHT_MOUSEBUTTON_PRESSED);
	eventFollower::followEvent(this, eventType::RIGHT_MOUSEBUTTON_RELEASED);

	initialized = true;
	return initialized;
}
#pragma endregion

/*****************************************************************************/

#pragma region buttonGeoPrim
//-----------------------------------------------------------------------------
// Name: update()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::buttonGeoPrim::update(matrix& matView, DX::StepTimer const& timer)
{
	if (!isGuiElemStateVisible(status)) return;

	matrix matWorld;
	getFinalMatrix(matWorld);

	shape->setWorld(matWorld);
	shape->setView(matView);
	shape->setColor(mainColor);
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::buttonGeoPrim::render(ssf::sharedVars& v)
{
	if (!isGuiElemStateVisible(status)) return;

	shape->draw();
}

//-----------------------------------------------------------------------------
// Name: buttonGeoPrim::createDeviceDependentResources()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::buttonGeoPrim::createDeviceDependentResources(ssf::sharedVars & v)
{
	if (shape) shape->createDeviceDependentResources(v);
}

//-----------------------------------------------------------------------------
// Name: createWindowSizeDependentResources()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::buttonGeoPrim::createWindowSizeDependentResources(matrix& matProjection)
{
	// BUG: When one process is in rendering and the other in performResourceUpload() than a crash occur
	if (mainTexture && shape) shape->setTexture(*mainTexture);
	shape->createWindowSizeDependentResources(matProjection);
}

//-----------------------------------------------------------------------------
// Name: onDeviceLost()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::buttonGeoPrim::onDeviceLost()
{
	shape->onDeviceLost();
}

//-----------------------------------------------------------------------------
// Name: renderSprites()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::buttonGeoPrim::renderSprites(ssf::sharedVars& v)
{
	if (!isGuiElemStateVisible(status)) return;
	
	// ... just a 2D substitute code
	if (textFont && isGuiElemStateVisible(textState)) {
		
		// variables
		float	lineSpacing	= 20;
		matrix	buttonMat;
		fRECT	clippingRect;

		// get matrix
		getFinalMatrix(buttonMat);
		
		// do not render text if only back face of button is visible
		if (showOnlyFrontSide && !ww->getActiveCamera()->isFaceVisible(buttonMat)) return;

		// clippingRect
		clippingRect.left	= 0;
		clippingRect.top	= 0;
		clippingRect.right	= (float) ww->getWindowSizeX();
		clippingRect.bottom	= (float) ww->getWindowSizeY();
		
		// render
		textFont->drawString(text.c_str(), buttonMat.Translation(), textColor, textScale, clippingRect, lineSpacing, false);
	}
}

//-----------------------------------------------------------------------------
// Name: whyAmINotVisible()
// Desc: 
//-----------------------------------------------------------------------------
wstring wildWeasel::buttonGeoPrim::whyAmINotVisible()
{
	wstring ancestorReason = guiElemEvFol3D::whyAmINotVisible();
	if (ancestorReason != wwc::REASON_NOT_FOUND) return ancestorReason;

	// ...
	// bool								useAlphaNoLightning	
	// dxGeoPrimitive*						gfx_geoPrim			
	// dxBasicEffect						gfx_basicEffect		
	// dxAlphaEffect						gfx_alphaEffect		
	return wwc::REASON_NOT_FOUND;
}

#pragma endregion

/*****************************************************************************/

#pragma region textLabel3D
//-----------------------------------------------------------------------------
// Name: plainButton()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::textLabel3D::create(masterMind* ww, wstring &theText, font3D* theFont)
{
	// params ok?
	if (!ww)			return false;
	if (initialized)	return false;
	
	// set reference to shared shape
	this->ww		= ww;

	// remember to load the file later on together with the other ressources
	addObjectToLoad(ww);
	ww->registerGuiElement(this);
	addSpriteToDraw(0, nullptr);

	setFont(theFont);
	setText(theText);

	initialized = true;
	return initialized;
}

//-----------------------------------------------------------------------------
// Name: renderSprites()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::textLabel3D::renderSprites(ssf::sharedVars& v)
{
	if (!isGuiElemStateVisible(status)) return;
	
	// ... just a 2D substitute code
	if (textFont && isGuiElemStateVisible(textState)) {
		
		// variables
		float	lineSpacing	= 20;
		matrix	buttonMat;
		fRECT	clippingRect;

		// get matrix
		getFinalMatrix(buttonMat);

		// do not render text if only back face of button is visible
		if (showOnlyFrontSide && !ww->getActiveCamera()->isFaceVisible(buttonMat)) return;

		// clippingRect
		clippingRect.left	= 0;
		clippingRect.top	= 0;
		clippingRect.right	= (float) ww->windowSize.x;
		clippingRect.bottom	= (float) ww->windowSize.y;
		
		// render
		textFont->drawString(text.c_str(), buttonMat.Translation(), textColor, textScale, clippingRect, lineSpacing, false);
	}
}

#pragma endregion

/*****************************************************************************/

#pragma region triad
//-----------------------------------------------------------------------------
// Name: create()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::triad::create(masterMind* ww, font3D* theFont)
{
	// params ok?
	if (!ww)			return false;
	if (initialized)	return false;

	// set reference to shared shape
	this->ww		= ww;

	axisX.create(ww, theFont, wstring(L"x"), defaultColors[0]);
	axisY.create(ww, theFont, wstring(L"y"), defaultColors[1]);
	axisZ.create(ww, theFont, wstring(L"z"), defaultColors[2]);

	axisX.matCone		*= matrix::CreateRotationZ(wwc::PI/2);
	axisX.matCylinder	*= matrix::CreateRotationZ(wwc::PI/2);
	axisX.matLabel		*= matrix::CreateRotationZ(wwc::PI/2);
	axisZ.matCone		*= matrix::CreateRotationX(wwc::PI/2);
	axisZ.matCylinder	*= matrix::CreateRotationX(wwc::PI/2);
	axisZ.matLabel		*= matrix::CreateRotationX(wwc::PI/2);

	axisX.label.setMatrix(&axisX.matLabel, false);
	axisY.label.setMatrix(&axisY.matLabel, false);
	axisZ.label.setMatrix(&axisZ.matLabel, false);

	axisX.label.insertMatrix(1, &matWorld, &dirtyBit);
	axisY.label.insertMatrix(1, &matWorld, &dirtyBit);
	axisZ.label.insertMatrix(1, &matWorld, &dirtyBit);
	
	// remember to load the file later on together with the other ressources
	addObjectToLoad(ww);
	ww->registerGuiElement(this);
	setFont(theFont);

	initialized = true;
	return initialized;
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::triad::render(ssf::sharedVars& v)
{
	if (!isGuiElemStateVisible(status)) return;

	axisX.gfx_geoPrimCylinder .draw();
	axisY.gfx_geoPrimCylinder .draw();
	axisZ.gfx_geoPrimCylinder .draw();
	axisX.gfx_geoPrimCone     .draw();
	axisY.gfx_geoPrimCone     .draw();
	axisZ.gfx_geoPrimCone     .draw();
}

//-----------------------------------------------------------------------------
// Name: update()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::triad::update(matrix& matView, DX::StepTimer const& timer)
{
	if (!isGuiElemStateVisible(status)) return;

	matrixChain::getFinalMatrix(matWorld);

	axisX.gfx_geoPrimCylinder	.setWorld(axisX.matCylinder * matWorld);
	axisX.gfx_geoPrimCylinder	.setView(matView);
	axisX.gfx_geoPrimCylinder	.setColor(axisX.axisColor);

	axisX.gfx_geoPrimCone		.setWorld(axisX.matCone * matWorld);
	axisX.gfx_geoPrimCone		.setView(matView);
	axisX.gfx_geoPrimCone		.setColor(axisX.axisColor);

	axisY.gfx_geoPrimCylinder	.setWorld(axisY.matCylinder * matWorld);
	axisY.gfx_geoPrimCylinder	.setView(matView);
	axisY.gfx_geoPrimCylinder	.setColor(axisY.axisColor);

	axisY.gfx_geoPrimCone		.setWorld(axisY.matCone * matWorld);
	axisY.gfx_geoPrimCone		.setView(matView);
	axisY.gfx_geoPrimCone		.setColor(axisY.axisColor);

	axisZ.gfx_geoPrimCylinder	.setWorld(axisZ.matCylinder * matWorld);
	axisZ.gfx_geoPrimCylinder	.setView(matView);
	axisZ.gfx_geoPrimCylinder	.setColor(axisZ.axisColor);

	axisZ.gfx_geoPrimCone		.setWorld(axisZ.matCone * matWorld);
	axisZ.gfx_geoPrimCone		.setView(matView);
	axisZ.gfx_geoPrimCone		.setColor(axisZ.axisColor);
}

//-----------------------------------------------------------------------------
// Name: createDeviceDependentResources()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::triad::createDeviceDependentResources(ssf::sharedVars& v)
{
	axisX.gfx_geoPrimCylinder .createDeviceDependentResources(v);
	axisY.gfx_geoPrimCylinder .createDeviceDependentResources(v);
	axisZ.gfx_geoPrimCylinder .createDeviceDependentResources(v);
	axisX.gfx_geoPrimCone     .createDeviceDependentResources(v);
	axisY.gfx_geoPrimCone     .createDeviceDependentResources(v);
	axisZ.gfx_geoPrimCone     .createDeviceDependentResources(v);
}

//-----------------------------------------------------------------------------
// Name: createWindowSizeDependentResources()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::triad::createWindowSizeDependentResources(matrix& matProjection)
{
	axisX.gfx_geoPrimCylinder .setProj(matProjection);
	axisY.gfx_geoPrimCylinder .setProj(matProjection);
	axisZ.gfx_geoPrimCylinder .setProj(matProjection);
	axisX.gfx_geoPrimCone     .setProj(matProjection);
	axisY.gfx_geoPrimCone     .setProj(matProjection);
	axisZ.gfx_geoPrimCone     .setProj(matProjection);
}

//-----------------------------------------------------------------------------
// Name: onDeviceLost()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::triad::onDeviceLost()
{
	axisX.gfx_geoPrimCylinder .onDeviceLost();
	axisY.gfx_geoPrimCylinder .onDeviceLost();
	axisZ.gfx_geoPrimCylinder .onDeviceLost();
	axisX.gfx_geoPrimCone     .onDeviceLost();
	axisY.gfx_geoPrimCone     .onDeviceLost();
	axisZ.gfx_geoPrimCone     .onDeviceLost();
}

//-----------------------------------------------------------------------------
// Name: create()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::triad::axis::create(masterMind* ww, font3D* theFont, wstring &theText, color initColor)
{
	axisColor			= initColor;

	float lengthCylinder = 1 - defaultConeLength;
	float lengthCone	 =     defaultConeLength;
	float radiusCylinder = defaultCylinderRadius;
	float radiusCone	 = defaultConeRadius;

	matCylinder = matrix::CreateScale(2*radiusCylinder, lengthCylinder,	2*radiusCylinder) * matrix::CreateTranslation(0,			lengthCylinder/2               , 0);
	matCone		= matrix::CreateScale(2*radiusCone    , lengthCone    ,	2*radiusCone    ) * matrix::CreateTranslation(0,			lengthCylinder   + lengthCone/2, 0);
	matLabel	= matrix::CreateScale(1		          , 1		      ,	1			    ) * matrix::CreateTranslation(2*radiusCone, lengthCylinder				   , 0);

	label.create(ww, theText, theFont);
	label.setFont(theFont);
	label.setState(guiElemState::DRAWED);
	label.setTextState(guiElemState::DRAWED);
	label.setColor(wildWeasel::color::white);
	label.setPositioningMode(matrixControl3D::matControlMode::matrix);
}

//-----------------------------------------------------------------------------
// Name: whyAmINotVisible()
// Desc: 
//-----------------------------------------------------------------------------
wstring wildWeasel::triad::whyAmINotVisible()
{
	wstring ancestorReason = guiElement3D::whyAmINotVisible();
	if (ancestorReason != wwc::REASON_NOT_FOUND) return ancestorReason;

	return wwc::REASON_NOT_FOUND;
}

#pragma endregion

/*****************************************************************************/

#pragma region plainButton
//-----------------------------------------------------------------------------
// Name: plainButton()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::plainButton::create(masterMind* ww, void buttonFunc(void* pUser), void* pUser, bool useAlphaNoLightning)
{
	return create(ww, buttonFunc, pUser, nullptr, 0, 0, useAlphaNoLightning);
}

//-----------------------------------------------------------------------------
// Name: plainButton()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::plainButton::create(masterMind* ww, void buttonFunc(void* pUser), void* pUser, guiElemCoord* cc, unsigned int posInsideCC, float depthInSpace, bool useAlphaNoLightning)
{
	// params ok?
	if (!ww)			return false;
	if (initialized)	return false;
	
	// set reference to shared shape
	shape		= new ssf::shape::rect();
	this->ww	= ww;

	// polygons for mouse over detection
	float	e = 0.5;
	vector3 ul(-e, +e, 0);
	vector3 ur(+e, +e, 0);
	vector3 ll(-e, -e, 0);
	vector3 lr(+e, -e, 0);
	rects.vertices.push_back(ul);
	rects.vertices.push_back(ur);
	rects.vertices.push_back(ll);
	rects.vertices.push_back(lr);
	rects.verticesTransformed.resize(rects.vertices.size());
	rects.rects.reserve(1);
	rects.rects.push_back(rectIndizes(0, 1, 2, 3));
	rects.sortIndices();

	// positioning
	if (cc != nullptr) {
		fRECT rc;
		ww->calcControlPos(posInsideCC, &rc, cc);	
		updateMatrixToAlignOnScreen(rc, depthInSpace);
	}

	// remember to load the file later on together with the other ressources
	addObjectToLoad(ww);
	ww->registerGuiElement(this);
	addSpriteToDraw(0, nullptr);

	// track mouse moves and clicks
	eventFollower::followEvent(this, eventType::MOUSEMOVED);
	eventFollower::followEvent(this, eventType::LEFT_MOUSEBUTTON_PRESSED);
	eventFollower::followEvent(this, eventType::LEFT_MOUSEBUTTON_RELEASED);
	eventFollower::followEvent(this, eventType::RIGHT_MOUSEBUTTON_PRESSED);
	eventFollower::followEvent(this, eventType::RIGHT_MOUSEBUTTON_RELEASED);
	
	this->useAlphaNoLightning = useAlphaNoLightning;
	windowSize	= &ww->windowSize;
	initialized = true;
	return initialized;
}

//-----------------------------------------------------------------------------
// Name: updateMatrixToAlignOnScreen()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::plainButton::updateMatrixToAlignOnScreen(fRECT& rc, float depthInSpace)
{
	// depends on windowSize, rc and depthInSpace
	float xCenter	= 0;
	float yCenter	= 0;
	float xScale	= 1;
	float yScale	= 1;

	setPosition	(xCenter, yCenter, depthInSpace, false, false);
	setRotation	(0,0,0, false, false);
	setScale	(xScale, yScale, 1, false, true);
}

//-----------------------------------------------------------------------------
// Name: plainButton()
// Desc: 
//-----------------------------------------------------------------------------
wstring wildWeasel::plainButton::whyAmINotVisible()
{
	wstring ancestorReason = buttonGeoPrim::whyAmINotVisible();
	if (ancestorReason != wwc::REASON_NOT_FOUND) return ancestorReason;

	return wwc::REASON_NOT_FOUND;
}
#pragma endregion

/*****************************************************************************/

#pragma region plainButton2D
//-----------------------------------------------------------------------------
// Name: plainButton()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::plainButton2D::create(masterMind* ww, buttonImageFiles &files, function<void(guiElemEvFol*, void*)> buttonFunc, void* pUser, alignment& newAlignment, unsigned int alignmentPos)
{
	bool result = create(ww, files, buttonFunc, pUser, 0);
	if (result) {
		setAlignment(newAlignment, alignmentPos);
		setState(wildWeasel::guiElemState::HIDDEN);
	}
	return result;
}

//-----------------------------------------------------------------------------
// Name: plainButton()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::plainButton2D::create(masterMind* ww, buttonImageFiles &files, function<void(guiElemEvFol*, void*)> buttonFunc, void* pUser, float depthInSpace)
{
	// params ok?
	if (!ww)			return false;
	if (initialized)	return false;
	
	// positioning
	targetRectPositionZ	= depthInSpace;

	// call back function
	assignOnLeftMouseButtonReleased(buttonFunc, pUser);
	this->ww			= ww;

	// remember to load the file later on together with the other ressources
	addObjectToLoad(ww);
	ww->registerGuiElement(this);
	addSpriteToDraw(targetRectPositionZ, nullptr);

	// load images
	animation.setParent(ww);
	animation.setDimensionPointers(&transScaledTargetRect, &targetRectRotation, &targetRectPositionZ, &mainColor, &targetRectClipping);
	animation.addFlipBook(&normal		, files.normal		);
	animation.addFlipBook(&mouseOver	, files.mouseOver	);
	animation.addFlipBook(&mouseLeave	, files.mouseLeave	);
	animation.addFlipBook(&pressed		, files.pressed		);
	animation.addFlipBook(&grayedOut	, files.grayedOut	);

	// start with normal state
	animation.setActiveFlipBook(normal);

	assignOnMouseEnteredRegion		(bind(&wildWeasel::plainButton2D::eventMouseEnteredRegion		,	this, placeholders::_1, placeholders::_2), this); 
	assignOnMouseLeftRegion			(bind(&wildWeasel::plainButton2D::eventMouseLeftRegion			,	this, placeholders::_1, placeholders::_2), this); 
	assignOnLeftMouseButtonPressed	(bind(&wildWeasel::plainButton2D::eventLeftMouseButtonPressed	,	this, placeholders::_1, placeholders::_2), this); 
	assignOnLeftMouseButtonReleased	(bind(&wildWeasel::plainButton2D::eventLeftMouseButtonReleased	,	this, placeholders::_1, placeholders::_2), this); 

	// track mouse moves and clicks
	eventFollower::followEvent(this, eventType::MOUSEMOVED);
	eventFollower::followEvent(this, eventType::LEFT_MOUSEBUTTON_PRESSED);
	eventFollower::followEvent(this, eventType::LEFT_MOUSEBUTTON_RELEASED);
	eventFollower::followEvent(this, eventType::RIGHT_MOUSEBUTTON_PRESSED);
	eventFollower::followEvent(this, eventType::RIGHT_MOUSEBUTTON_RELEASED);
	eventFollower::followEvent(this, eventType::WINDOWSIZE_CHANGED);
	
	initialized = true;
	return initialized;
}

//-----------------------------------------------------------------------------
// Name: renderSprites()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::plainButton2D::renderSprites(ssf::sharedVars& v)
{
	if (!initialized) return;

	// some other elements might be dependant on this element, so its position must be update although it might be invisible
	guiElement2D::calcFinalMatrix();

	if (!isGuiElemStateVisible(status)) return;

	animation.activeFlipBook->renderSprites(v);
	
	if (textFont) {
		vector2		position, scale, origin	= vector2(0,0);
		calcTextDimensions(position, scale);
		textFont->draw(text.c_str(), position, textColor, targetRectRotation, origin, scale, targetRectPositionZ, targetRectClipping, textFont->getLineSpacing(), true);
	}
}

//-----------------------------------------------------------------------------
// Name: setState()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::plainButton2D::setState(guiElemState newStatus)
{
	guiElemEvFol2D::setState(newStatus);	

	// update flipbook
	switch (status)
	{
	case guiElemState::GRAYED:		changeState(grayedOut);		break;
	case guiElemState::DRAWED:		changeState(normal);		break;
	case guiElemState::VISIBLE:		changeState(normal);		break;
	}
}

//-----------------------------------------------------------------------------
// Name: setImageFiles()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::plainButton2D::setImageFiles(buttonImageFiles &newFiles)
{
	normal		->loadGraphics(nullptr, newFiles.normal		);
	mouseOver	->loadGraphics(nullptr, newFiles.mouseOver	);
	mouseLeave	->loadGraphics(nullptr, newFiles.mouseLeave	);
	pressed		->loadGraphics(nullptr, newFiles.pressed	);
	grayedOut	->loadGraphics(nullptr, newFiles.grayedOut	);
}

//-----------------------------------------------------------------------------
// Name: changeState()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::plainButton2D::changeState(flipBook* newState) 
{
	if (newState == animation.activeFlipBook) return;
	animation.setActiveFlipBook(newState);
	animation.startAnimation();
}

//-----------------------------------------------------------------------------
// Name: eventMouseEnteredRegion()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::plainButton2D::eventMouseEnteredRegion(guiElemEvFol* item, void* pUser)
{
	changeState(mouseOver);
}

//-----------------------------------------------------------------------------
// Name: eventMouseLeftRegion()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::plainButton2D::eventMouseLeftRegion(guiElemEvFol* item, void* pUser)
{
	changeState(mouseLeave);
}

//-----------------------------------------------------------------------------
// Name: eventMousePressedRegion()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::plainButton2D::eventLeftMouseButtonPressed(guiElemEvFol* item, void* pUser)
{
	changeState(pressed);
}

//-----------------------------------------------------------------------------
// Name: eventMouseReleasedRegion()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::plainButton2D::eventLeftMouseButtonReleased(guiElemEvFol* item, void* pUser)
{
	changeState(mouseOver);
	animation.stopAnimation();
}

//-----------------------------------------------------------------------------
// Name: windowSizeChanged()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::plainButton2D::windowSizeChanged(int xSize, int ySize)
{
}
#pragma endregion

/*****************************************************************************/

#pragma region textLabel2D
//-----------------------------------------------------------------------------
// Name: create()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::textLabel2D::create(masterMind* ww, wstring &theText, font2D* theFont, float depthInSpace, alignment& newAlignment, unsigned int alignmentPos)
{
	bool result = create(ww, theText, theFont, depthInSpace);
	
	if (result) {
		setAlignment(newAlignment, alignmentPos);
	}

	return result;
}

//-----------------------------------------------------------------------------
// Name: create()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::textLabel2D::create(masterMind* ww, wstring &theText, font2D* theFont, float depthInSpace)
{
	// params ok?
	if (!ww)			return false;
	if (initialized)	return false;
	
	// positioning
	targetRectPositionZ	= depthInSpace;
	textFont			= theFont;

	setTextState(wildWeasel::guiElemState::DRAWED);
	setState(wildWeasel::guiElemState::DRAWED);
	setText(theText);

	// call back function
	this->ww		= ww;

	// remember to load the file later on together with the other ressources
	addObjectToLoad(ww);
	ww->registerGuiElement(this);
	addSpriteToDraw(targetRectPositionZ, nullptr);

	// track mouse moves and clicks
	eventFollower::followEvent(this, eventType::WINDOWSIZE_CHANGED);

	initialized = true;
	return initialized;
}

//-----------------------------------------------------------------------------
// Name: renderSprites()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::textLabel2D::renderSprites(ssf::sharedVars& v)
{
	if (!initialized) return;
	if (!textFont) return;
	if (!isGuiElemStateVisible(textState)) return;
	if (!isGuiElemStateVisible(status	)) return;

	// TODO: createDeviceDependentResources(v) should be called only once! But since flipBook is not a genericObject3D a special solution is necessary
	sprite.createDeviceDependentResources(v);

	// background texture
	if (mainTexture) {
		if (targetRectClipping == nullptr) {
			sprite.draw(*mainTexture, transScaledTargetRect, nullptr, mainColor, targetRectRotation, vector2(0,0), targetRectPositionZ);
		} else {
			//if (performClippingAndCheck(destRect, srcRect, myOrigin, clippingRect)) {
			//	sprite.Draw(*mainTexture, transScaledTargetRect, nullptr, mainColor, targetRectRotation, vector2(0,0), targetRectPositionZ);
			//}
		}
	}

	// ... code is not nice at all
	vector2		position, scale, origin	= vector2(0,0);
	calcTextDimensions(position, scale);
	textFont->draw(text.c_str(), position, textColor, targetRectRotation, origin, scale, targetRectPositionZ, targetRectClipping, textFont->getLineSpacing(), true);
}

//-----------------------------------------------------------------------------
// Name: windowSizeChanged()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::textLabel2D::windowSizeChanged(int xSize, int ySize)
{
}
#pragma endregion

/*************************************************************************************************************************************/

#pragma region scrollBar2D
//-----------------------------------------------------------------------------
// Name: create()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::scrollBar2D::create(masterMind* ww, float depthInSpace, buttonImageFiles &filesTriangle, buttonImageFiles &filesLane, buttonImageFiles &filesSledge)
{
	// params ok?
	if (!ww)				return false;
	if (isInitialized())	return false;
	
//	// positioning
//	zPosition = depthInSpace;

//	// call back function
//	this->parent		= ww;
//	this->windowSize	= &ww->windowSize;

//	// remember to load the file later on together with the other ressources
//	ww->threeD.addObjectToLoad(ww);
//	ww->registerGuiElement(this);

	// create buttons
	fromTriangle.create(ww, filesTriangle,	nullptr, nullptr, depthInSpace);
	fromLane	.create(ww, filesLane,		nullptr, nullptr, depthInSpace);
	sledge		.create(ww, filesSledge,	nullptr, nullptr, depthInSpace);
	toLane		.create(ww, filesLane,		nullptr, nullptr, depthInSpace);
	toTriangle	.create(ww, filesTriangle,	nullptr, nullptr, depthInSpace);

	// 2D dimensions
	RECT rc					= {0, 0, 1, 1};
	fromTriangle.setTargetRect(rc);
	fromLane	.setTargetRect(rc);
	sledge		.setTargetRect(rc);
	toLane		.setTargetRect(rc);
	toTriangle	.setTargetRect(rc);

	// 3d dimensions
	fromTriangle.setPositioningMode(wildWeasel::matrixControl2D::matControlMode::posRotSca);
	fromLane	.setPositioningMode(wildWeasel::matrixControl2D::matControlMode::posRotSca);
	sledge		.setPositioningMode(wildWeasel::matrixControl2D::matControlMode::posRotSca);
	toLane		.setPositioningMode(wildWeasel::matrixControl2D::matControlMode::posRotSca);
	toTriangle	.setPositioningMode(wildWeasel::matrixControl2D::matControlMode::posRotSca);
	fromTriangle.setRotation(0,			false);
	fromLane	.setRotation(0,			false);
	sledge		.setRotation(0,			false);
	toLane		.setRotation(wwc::PI,	false);
	toTriangle	.setRotation(wwc::PI,	false);

	// state
	fromTriangle.setState(wildWeasel::guiElemState::DRAWED);
	fromLane	.setState(wildWeasel::guiElemState::DRAWED);
	sledge		.setState(wildWeasel::guiElemState::DRAWED);
	toLane		.setState(wildWeasel::guiElemState::DRAWED);
	toTriangle	.setState(wildWeasel::guiElemState::DRAWED);

	// color
	fromTriangle.setColor(color::white);
	fromLane	.setColor(color::gray);
	sledge		.setColor(color::white);
	toLane		.setColor(color::gray);
	toTriangle	.setColor(color::white);
	
	// insert items into cluster list
	addItem(&fromTriangle);
	addItem(&fromLane	 );
	addItem(&sledge		 );
	addItem(&toLane		 );
	addItem(&toTriangle	 );

	// add event functions
	fromTriangle.assignOnLeftMouseButtonPressed(bind(&scrollBar2D::fromTrianglePressed	, this, placeholders::_1, placeholders::_2), this);
	fromLane	.assignOnLeftMouseButtonPressed(bind(&scrollBar2D::fromLanePressed		, this, placeholders::_1, placeholders::_2), this);
	sledge		.assignOnLeftMouseButtonPressed(bind(&scrollBar2D::sledgePressed		, this, placeholders::_1, placeholders::_2), this);
	sledge		.assignOnMouseMove			   (bind(&scrollBar2D::sledgeLaneMoved		, this, placeholders::_1, placeholders::_2), this);
	toLane		.assignOnLeftMouseButtonPressed(bind(&scrollBar2D::toLanePressed		, this, placeholders::_1, placeholders::_2), this);
	toTriangle	.assignOnLeftMouseButtonPressed(bind(&scrollBar2D::toTrianglePressed	, this, placeholders::_1, placeholders::_2), this);

	// get screen metrics
	HDC		screen					= GetDC(NULL);
	int		hSize					= GetDeviceCaps(screen, 4 /*=HORZSIZE*/);
	int		hRes					= GetDeviceCaps(screen, 8 /*=HORZRES */);
			pixelsPerMM				= (float) hRes / hSize;

	// quit
	alignAllItems();
	guiElemCluster2D::create(ww);
	return true;
}

//-----------------------------------------------------------------------------
// Name: getSledgePos()
// Desc: 
//-----------------------------------------------------------------------------
float wildWeasel::scrollBar2D::getSledgePos()
{
	return sledgePos;
}

//-----------------------------------------------------------------------------
// Name: getSledgeWidth()
// Desc: 
//-----------------------------------------------------------------------------
float wildWeasel::scrollBar2D::getSledgeWidth()
{
	return sledgeWidth;
}

//-----------------------------------------------------------------------------
// Name: create()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::scrollBar2D::setSledgePos(float fromPos)
{
	if (fromPos < 0) fromPos = 0;
	if (fromPos > 1 - sledgeWidth) fromPos = 1 - sledgeWidth;
	sledgePos = fromPos;

	alignAllItems();
	if (userFuncPosChanged) {
		userFuncPosChanged(this, pUser);
	}
}

//-----------------------------------------------------------------------------
// Name: setSledgeWidth()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::scrollBar2D::setSledgeWidth(float newSledgeWidth)
{
	// locals
	float	curLaneWidthInPixels	= scale.x * (1.0f - 2*triangleSize);
	float	minSledgeWidthInPixels	= minSledgeWidthInMM * pixelsPerMM;
	float	minSledgeWidthRelative	= minSledgeWidthInPixels / curLaneWidthInPixels;

	minSledgeWidthRelative = clamp(minSledgeWidthRelative, 0.0f, 1.0f);
	sledgeWidth = clamp(newSledgeWidth, minSledgeWidthRelative, 1.0f);
	alignAllItems();
}

//-----------------------------------------------------------------------------
// Name: getLaneStepSize()
// Desc: 
//-----------------------------------------------------------------------------
float wildWeasel::scrollBar2D::getLaneStepSize()
{
	return laneStepSize;
}

//-----------------------------------------------------------------------------
// Name: getTriangleStepSize()
// Desc: 
//-----------------------------------------------------------------------------
float wildWeasel::scrollBar2D::getTriangleStepSize()
{
	return triangleStepSize;
}

//-----------------------------------------------------------------------------
// Name: setTriangleStepSize()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::scrollBar2D::setTriangleStepSize(float newStepSize)
{
	triangleStepSize = newStepSize;
	if (triangleStepSize < minStepSize)		triangleStepSize = minStepSize;
	if (triangleStepSize > 1 - sledgeWidth) triangleStepSize = 1 - sledgeWidth;
}

//-----------------------------------------------------------------------------
// Name: setTriangleSize()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::scrollBar2D::setTriangleSize(float newSize)
{
	triangleSize = newSize;
	if (triangleSize < minTriangleSize)		triangleStepSize = minTriangleSize;
	if (triangleSize > maxTriangleSize)		triangleStepSize = maxTriangleSize;
}

//-----------------------------------------------------------------------------
// Name: setLaneStepSize()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::scrollBar2D::setLaneStepSize(float newStepSize)
{
	laneStepSize = newStepSize;
	if (laneStepSize < minStepSize)		laneStepSize = minStepSize;
	if (laneStepSize > 1 - sledgeWidth) laneStepSize = 1 - sledgeWidth;
}

//-----------------------------------------------------------------------------
// Name: assignOnSledgePosChange()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::scrollBar2D::assignOnSledgePosChange(function<void(scrollBar2D*, void*)> userFunc, void* pUser)
{
	this->userFuncPosChanged		= userFunc;
	this->pUser						= pUser;
}

//-----------------------------------------------------------------------------
// Name: doesGuiElemBelongToScrollbar()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::scrollBar2D::informMotherOnEvent(eventFollower* mother)
{
	fromTriangle	.informMotherOnEvent(mother);
	fromLane		.informMotherOnEvent(mother);
	sledge			.informMotherOnEvent(mother);
	toLane			.informMotherOnEvent(mother);
	toTriangle		.informMotherOnEvent(mother);
}

//-----------------------------------------------------------------------------
// Name: doesGuiElemBelongToScrollbar()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::scrollBar2D::doesGuiElemBelongToScrollbar(guiElemEvFol* elem)
{
	if (&fromTriangle	== elem) return true;
	if (&fromLane		== elem) return true;
	if (&sledge			== elem) return true;
	if (&toLane			== elem) return true;
	if (&toTriangle		== elem) return true;
	return false;
}

//-----------------------------------------------------------------------------
// Name: alignAllItems()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::scrollBar2D::alignAllItems()
{
	// ... update "scale"
	setTriangleSize(scale.y / scale.x);

	// locals
	float totalLaneSize		= 1.0f - 2*triangleSize;
	float fromLaneSize		= sledgePos * totalLaneSize;
	float sledgeSize		= sledgeWidth * totalLaneSize;
	float toLaneSize		= 1 - 2*triangleSize - fromLaneSize - sledgeSize;
	
	// position and size												+ because of rotation around upper left corner
	fromTriangle.setPosition(0,															0,	false);
	fromLane	.setPosition(triangleSize,												0,	false);
	sledge		.setPosition(triangleSize + fromLaneSize,								0,	false);
	toLane		.setPosition(triangleSize + fromLaneSize + sledgeSize	+ toLaneSize,	1,	false);
	toTriangle	.setPosition(triangleSize + totalLaneSize				+ triangleSize,	1,	false);
	fromTriangle.setScale	(triangleSize,	1, true);
	fromLane	.setScale	(fromLaneSize,	1, true);
	sledge		.setScale	(sledgeSize,	1, true);
	toLane		.setScale	(toLaneSize,	1, true);
	toTriangle	.setScale	(triangleSize,	1, true);
}

//-----------------------------------------------------------------------------
// Name: fromTrianglePressed()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::scrollBar2D::fromTrianglePressed(guiElemEvFol* elem, void* pUser)
{
	setSledgePos(sledgePos - triangleStepSize);
}

//-----------------------------------------------------------------------------
// Name: toTrianglePressed()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::scrollBar2D::toTrianglePressed(guiElemEvFol* elem, void* pUser)
{
	setSledgePos(sledgePos + triangleStepSize);
}

//-----------------------------------------------------------------------------
// Name: assignOnSledgePosChange()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::scrollBar2D::sledgeLaneMoved(guiElemEvFol* elem, void* pUser)
{
	if (sledge.isMousePressedInRegion()) {
		// locals
		vector2 mouseMovedVec((float) (ww->cursor2D.x - mousePressPos.x), (float) (ww->cursor2D.y - mousePressPos.y));
		vector2 laneVec((1 - 2*triangleSize) * scale.x, 0);			// lane size in screen coordinates
		vector2 R(cos(rotation), sin(rotation));					// rotation matrix for 'laneVecRot'
		vector2 laneVecRot(R.x  * laneVec.x, R.y * laneVec.x);		// 'laneVec' rotates by "rotation"
		float	stepSize = laneVecRot.Dot(mouseMovedVec) / laneVecRot.LengthSquared();
		setSledgePos(sledgePosWhenPressed + stepSize);
	}
}

//-----------------------------------------------------------------------------
// Name: sledgePressed()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::scrollBar2D::sledgePressed(guiElemEvFol* elem, void* pUser)
{
	mousePressPos.x	= ww->cursor2D.x;
	mousePressPos.y	= ww->cursor2D.y;
	sledgePosWhenPressed = sledgePos;
}

//-----------------------------------------------------------------------------
// Name: fromLanePressed()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::scrollBar2D::fromLanePressed(guiElemEvFol* elem, void* pUser)
{
	setSledgePos(sledgePos - laneStepSize);
}

//-----------------------------------------------------------------------------
// Name: toLanePressed()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::scrollBar2D::toLanePressed(guiElemEvFol* elem, void* pUser)
{
	setSledgePos(sledgePos + laneStepSize);
}

//-----------------------------------------------------------------------------
// Name: create()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::scrollBarDuo::createScrollBars(eventFollower* mother, buttonImageFiles &filesTriangle, buttonImageFiles &filesLane, buttonImageFiles &filesSledge, masterMind* ww, matrix& mat, bool& dirtyBit, float depthInSpace)
{
	// already initialized
	if (scrollBarsCreated) return false;

	// create scrollbars
	columnScrollBar = new scrollBar2D();
	rowScrollBar	= new scrollBar2D();
	if (!columnScrollBar->create(/*parent, zPosition*/ ww, depthInSpace, filesTriangle, filesLane, filesSledge))	return false;
	if (!rowScrollBar	->create(/*parent, zPosition*/ ww, depthInSpace, filesTriangle, filesLane, filesSledge))	return false;

	// register mother
	columnScrollBar	->informMotherOnEvent(mother);
	rowScrollBar	->informMotherOnEvent(mother);

	// attach the list view matrix to each sub item
	for (auto& curSubItem : columnScrollBar->items) {
		curSubItem->insertMatrix(100000, &mat, &dirtyBit);
	}

	// attach the list view matrix to each sub item
	for (auto& curSubItem : rowScrollBar->items) {
		curSubItem->insertMatrix(100000, &mat, &dirtyBit);
	}

	// sledge dimension
	columnScrollBar	->setSledgePos(0);
	rowScrollBar	->setSledgePos(0);
	updateSledgeWidth();

	// scroll bar events
	columnScrollBar	->assignOnSledgePosChange(bind(&scrollBarDuo::columnScrollBarMoved,	this, placeholders::_1, placeholders::_2), this);
	rowScrollBar	->assignOnSledgePosChange(bind(&scrollBarDuo::rowScrollBarMoved,	this, placeholders::_1, placeholders::_2), this);

	// quit
	scrollBarsCreated = true;
	return scrollBarsCreated;
}

//-----------------------------------------------------------------------------
// Name: updateScrollOffset()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::scrollBarDuo::updateScrollOffset()
{
	if (!scrollBarsCreated) return;
	scrollOffset.x	= (columnScrollBar->getSledgeWidth()<sledgeWidthThreshold) ? -1 * columnScrollBar->getSledgePos() / (1 - columnScrollBar->getSledgeWidth()) * (totalAreaSize.x - visibleAreaSize.x) : 0;
	scrollOffset.y	= (rowScrollBar   ->getSledgeWidth()<sledgeWidthThreshold) ? -1 * rowScrollBar   ->getSledgePos() / (1 - rowScrollBar   ->getSledgeWidth()) * (totalAreaSize.y - visibleAreaSize.y) : 0;
}

//-----------------------------------------------------------------------------
// Name: setVisibilityColumnScrollBar()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::scrollBarDuo::setVisibilityColumnScrollBar(bool isVisible)
{
	if (!scrollBarsCreated) return;
	columnScrollBarIsVisible = isVisible;
	updateSledgeWidth();
	columnScrollBar->setState(isVisible ? guiElemState::DRAWED : guiElemState::HIDDEN);
}

//-----------------------------------------------------------------------------
// Name: setVisibilityRowScrollBar()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::scrollBarDuo::setVisibilityRowScrollBar(bool isVisible)
{
	if (!scrollBarsCreated) return;
	rowScrollBarIsVisible = isVisible;
	updateSledgeWidth();
	rowScrollBar->setState(isVisible ? guiElemState::DRAWED : guiElemState::HIDDEN);
}

//-----------------------------------------------------------------------------
// Name: setColumnScrollBarHeight()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::scrollBarDuo::setColumnScrollBarHeight(unsigned int newHeight)
{
	columnScrollbarHeight = newHeight;
	updateSledgeWidth();
}

//-----------------------------------------------------------------------------
// Name: setRowScrollBarWidth()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::scrollBarDuo::setRowScrollBarWidth(unsigned int newWidth)
{
	rowScrollBarWidth = newWidth;
	updateSledgeWidth();
}

//-----------------------------------------------------------------------------
// Name: keyDown()
// Desc: scrolling with arrow keys
//-----------------------------------------------------------------------------
void wildWeasel::scrollBarDuo::keyDown(int keyCode)
{
	// focused? check not needed because keyDown is only called when focused			
	// if (!anySubItemFocused()) return;

	// process key press
	if (keyCode == VK_HOME) {
		if (columnScrollBar) columnScrollBar->setSledgePos(0);
		if (rowScrollBar   ) rowScrollBar   ->setSledgePos(0);
	} else if (keyCode == VK_LEFT) { 
		if (columnScrollBar) columnScrollBar->setSledgePos(columnScrollBar->getSledgePos() + (pixelsToScroll(direction::LEFT) / totalAreaSize.x));
	} else if (keyCode == VK_RIGHT) {
		if (columnScrollBar) columnScrollBar->setSledgePos(columnScrollBar->getSledgePos() + (pixelsToScroll(direction::RIGHT) / totalAreaSize.x));
	} else if (keyCode == VK_UP) {
		if (rowScrollBar) rowScrollBar->setSledgePos(rowScrollBar->getSledgePos() + (pixelsToScroll(direction::UP) / totalAreaSize.y));
	} else if (keyCode == VK_DOWN) {
		if (rowScrollBar) rowScrollBar->setSledgePos(rowScrollBar->getSledgePos() + (pixelsToScroll(direction::DOWN)  / totalAreaSize.y));
	} else if (keyCode == VK_NEXT) {
		if (rowScrollBar) rowScrollBar->setSledgePos(rowScrollBar->getSledgePos() + rowScrollBar->getLaneStepSize());
	} else if (keyCode == VK_PRIOR) {
		if (rowScrollBar) rowScrollBar->setSledgePos(rowScrollBar->getSledgePos() - rowScrollBar->getLaneStepSize());
	} else if (keyCode == VK_END) {
		if (columnScrollBar) columnScrollBar->setSledgePos(0);
		if (rowScrollBar   ) rowScrollBar   ->setSledgePos(1);
	}
}

//-----------------------------------------------------------------------------
// Name: ~scrollBarDuo()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::scrollBarDuo::~scrollBarDuo()
{
	if (!scrollBarsCreated) return;
	delete rowScrollBar;
	delete columnScrollBar;
}

#pragma endregion

/*************************************************************************************************************************************/

#pragma region checkBox2D

//-----------------------------------------------------------------------------
// Name: create()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::checkBox2D::create(masterMind* ww, buttonImageFiles &filesChecked, buttonImageFiles &filesUnhecked, alignment& newAlignment, unsigned int alignmentPos)
{
	bool result = create(ww, filesChecked, filesUnhecked, 0);
	if (result) {
		setAlignment(newAlignment, alignmentPos);
		setState(wildWeasel::guiElemState::HIDDEN);
	}
	return result;
}

//-----------------------------------------------------------------------------
// Name: create()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::checkBox2D::create(masterMind* ww, buttonImageFiles &filesChecked, buttonImageFiles &filesUnchecked, float depthInSpace)
{
	// params ok?
	if (!ww)			return false;
	if (initialized)	return false;

	// call back function
	this->ww					= ww;
	this->targetRectPositionZ	= depthInSpace;
	this->checkState			= false;
	this->alignVertical			= alignmentVertical::CENTER;
	this->alignHorizontal		= alignmentHorizontal::LEFT;

	// remember to load the file later on together with the other ressources
	addObjectToLoad(ww);
	ww->registerGuiElement(this);
	addSpriteToDraw(targetRectPositionZ, nullptr);

	// load images
	animation.setParent(ww);
	animation.setDimensionPointers(&transScaledTargetRect, &targetRectRotation, &targetRectPositionZ, &mainColor, &targetRectClipping);
	animation.addFlipBook(&normal_c		, filesChecked  .normal		);
	animation.addFlipBook(&mouseOver_c	, filesChecked  .mouseOver	);
	animation.addFlipBook(&mouseLeave_c	, filesChecked  .mouseLeave	);
	animation.addFlipBook(&pressed_c	, filesChecked  .pressed	);
	animation.addFlipBook(&grayedOut_c	, filesChecked  .grayedOut	);
	animation.addFlipBook(&normal_u		, filesUnchecked.normal		);
	animation.addFlipBook(&mouseOver_u	, filesUnchecked.mouseOver	);
	animation.addFlipBook(&mouseLeave_u	, filesUnchecked.mouseLeave	);
	animation.addFlipBook(&pressed_u	, filesUnchecked.pressed	);
	animation.addFlipBook(&grayedOut_u	, filesUnchecked.grayedOut	);

	// start with normal state
	animation.setActiveFlipBook(normal_u);

	assignOnMouseEnteredRegion		(bind(&wildWeasel::checkBox2D::eventMouseEnteredRegion		,	this, placeholders::_1, placeholders::_2), this); 
	assignOnMouseLeftRegion			(bind(&wildWeasel::checkBox2D::eventMouseLeftRegion			,	this, placeholders::_1, placeholders::_2), this); 
	assignOnLeftMouseButtonPressed	(bind(&wildWeasel::checkBox2D::eventLeftMouseButtonPressed	,	this, placeholders::_1, placeholders::_2), this); 
	assignOnLeftMouseButtonReleased	(bind(&wildWeasel::checkBox2D::eventLeftMouseButtonReleased	,	this, placeholders::_1, placeholders::_2), this); 

	// track mouse moves and clicks
	eventFollower::followEvent(this, eventType::MOUSEMOVED);
	eventFollower::followEvent(this, eventType::LEFT_MOUSEBUTTON_PRESSED);
	eventFollower::followEvent(this, eventType::LEFT_MOUSEBUTTON_RELEASED);
	eventFollower::followEvent(this, eventType::WINDOWSIZE_CHANGED);
	
	initialized = true;
	return initialized;
}

//-----------------------------------------------------------------------------
// Name: renderSprites()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::checkBox2D::renderSprites(ssf::sharedVars& v)
{
	if (!initialized) return;
	
	if (status == guiElemState::DRAWED || status == guiElemState::VISIBLE || status == guiElemState::GRAYED) {
		
		calcFinalMatrix();

		animation.activeFlipBook->renderSprites(v);
	
		if (textFont) {
			// ... totally crappy !!!
			vector2		position, scale, origin	= vector2(0,0);
			calcTextDimensions(position, scale);
			position.x += transScaledTargetRect.right - transScaledTargetRect.left + borderWidthForText; 
			textFont->draw(text.c_str(), position, textColor, targetRectRotation, origin, scale, targetRectPositionZ, targetRectClipping, textFont->getLineSpacing(), true);
		}
	}
}

//-----------------------------------------------------------------------------
// Name: setState()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::checkBox2D::setState(guiElemState newStatus)
{
	guiElemEvFol2D::setState(newStatus);		

	// update flipbook
	switch (status)
	{
	case guiElemState::GRAYED:		changeState(checkState ? grayedOut_c : grayedOut_u);	break;
	case guiElemState::DRAWED:		changeState(checkState ? normal_c	 : normal_u	);		break;
	case guiElemState::VISIBLE:		changeState(checkState ? normal_c	 : normal_u	);		break;
	}
}

//-----------------------------------------------------------------------------
// Name: changeState()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::checkBox2D::changeState(flipBook* newState) 
{
	if (newState == animation.activeFlipBook) return;
	animation.setActiveFlipBook(newState);
	animation.startAnimation();
}

//-----------------------------------------------------------------------------
// Name: eventMouseEnteredRegion()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::checkBox2D::eventMouseEnteredRegion(guiElemEvFol* item, void* pUser)
{
	changeState(checkState ? mouseOver_c : mouseOver_u);
}

//-----------------------------------------------------------------------------
// Name: eventMouseLeftRegion()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::checkBox2D::eventMouseLeftRegion(guiElemEvFol* item, void* pUser)
{
	changeState(checkState ? mouseLeave_c : mouseLeave_u);
}

//-----------------------------------------------------------------------------
// Name: eventMousePressedRegion()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::checkBox2D::eventLeftMouseButtonPressed(guiElemEvFol* item, void* pUser)
{
	changeState(checkState ? pressed_c : pressed_u);
}

//-----------------------------------------------------------------------------
// Name: eventMouseReleasedRegion()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::checkBox2D::eventLeftMouseButtonReleased(guiElemEvFol* item, void* pUser)
{
	checkState = (!checkState);
	changeState(checkState ? mouseOver_c : mouseOver_u);
	animation.stopAnimation();
}

//-----------------------------------------------------------------------------
// Name: windowSizeChanged()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::checkBox2D::windowSizeChanged(int xSize, int ySize)
{
}

//-----------------------------------------------------------------------------
// Name: create()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::checkBox2D::isChecked()
{
	return checkState;
}

//-----------------------------------------------------------------------------
// Name: create()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::checkBox2D::setCheckState(bool newCheckState)
{
	checkState = newCheckState;
	changeState(newCheckState ? normal_c : normal_u);
	animation.stopAnimation();
}
#pragma endregion

/*************************************************************************************************************************************/

#pragma region borderLine2D
//-----------------------------------------------------------------------------
// Name: borderLine2D()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::borderLine2D::create(masterMind* ww, texture& line, texture& corner, float depthInSpace)
{
	// params ok?
	if (!ww)			return false;
	if (initialized)	return false;
	
	// positioning
	targetRectPositionZ	= depthInSpace;

	// call back function
	this->ww			= ww;
	this->corner		= &corner;
	this->line			= &line;

	// remember to load the file later on together with the other ressources
	addObjectToLoad(ww);
	ww->registerGuiElement(this);
	addSpriteToDraw(targetRectPositionZ, nullptr);
	
	eventFollower::followEvent(this, eventType::WINDOWSIZE_CHANGED);

	initialized = true;
	return initialized;
}

//-----------------------------------------------------------------------------
// Name: renderSprites()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::borderLine2D::renderSprites(ssf::sharedVars& v)
{
	if (!initialized) return;
	if (!isGuiElemStateVisible(status)) return;

	// TODO: createDeviceDependentResources(v) should be called only once! But since flipBook is not a genericObject3D a special solution is necessary
	sprite.createDeviceDependentResources(v);
	
	calcFinalMatrix();

	// draw sprite
	if (corner && line) {
		vector2		textSizeLine		= line  ->getSize();
		vector2		textSizeCorner		= corner->getSize();
		vector2		myPos				= {transScaledTargetRect.left, transScaledTargetRect.top};
		vector2		myScale;
		vector2		origin;

		// corner upper left
		origin	.x	= 0;
		origin	.y	= 0;
		myScale	.x	= cornerSizeInPixels.x / textSizeCorner.x;
		myScale	.y	= cornerSizeInPixels.y / textSizeCorner.y;
		sprite.draw(*corner, myPos, nullptr, mainColor, targetRectRotation, origin,  myScale, targetRectPositionZ);
	
		// corner upper right
		origin	.x	= -1*(transScaledTargetRect.right	- cornerSizeInPixels.x - transScaledTargetRect.left)/myScale	.x;
		origin	.y	= 0;
		sprite.draw(*corner, myPos, nullptr, mainColor, targetRectRotation, origin,  myScale, targetRectPositionZ, false, true);
	
		// corner lower left
		origin	.x	= 0;
		origin	.y	= -1*(transScaledTargetRect.bottom	- cornerSizeInPixels.y - transScaledTargetRect.top)/myScale	.y;
		sprite.draw(*corner, myPos, nullptr, mainColor, targetRectRotation, origin,  myScale, targetRectPositionZ, true, false);
	
		// corner lower right
		origin	.x	= -1*(transScaledTargetRect.right	- cornerSizeInPixels.x - transScaledTargetRect.left)/myScale	.x;
		origin	.y	= -1*(transScaledTargetRect.bottom	- cornerSizeInPixels.y - transScaledTargetRect.top )/myScale	.y;
		sprite.draw(*corner, myPos, nullptr, mainColor, targetRectRotation, origin,  myScale, targetRectPositionZ, true, true);
		
		// upper line (left part)
		myScale	.x	= (titleOffset.x - gapBetweenLineAndTitleInPixels) / textSizeLine.x;
		myScale	.y	= borderSizeInPixels.y / textSizeLine.y;
		origin	.x	= -1*cornerSizeInPixels.x / myScale	.x;
		origin	.y	= 0;
		sprite.draw(*line, myPos, nullptr, mainColor, targetRectRotation, origin,  myScale, targetRectPositionZ);

		// upper line (right part)
		myScale	.x	= (transScaledTargetRect.right - transScaledTargetRect.left - 2 * cornerSizeInPixels.x - titleWidthInPixels - titleOffset.x - 2 * gapBetweenLineAndTitleInPixels) / textSizeLine.x;
		myScale	.y	= borderSizeInPixels.y / textSizeLine.y;
		origin	.x	= -1*(cornerSizeInPixels.x + titleOffset.x + titleWidthInPixels + 2 * gapBetweenLineAndTitleInPixels)/myScale	.x;
		origin	.y	= 0;
		sprite.draw(*line, myPos, nullptr, mainColor, targetRectRotation, origin,  myScale, targetRectPositionZ);

		// lower line
		myScale	.x	= (transScaledTargetRect.right - transScaledTargetRect.left - 2 * cornerSizeInPixels.x) / textSizeLine.x;
		myScale	.y	= borderSizeInPixels.y / textSizeLine.y;
		origin	.x	= -1*cornerSizeInPixels.x/myScale	.x;
		origin	.y	= -1*(transScaledTargetRect.bottom  - borderSizeInPixels.y - transScaledTargetRect.top)/myScale	.y;
		sprite.draw(*line, myPos, nullptr, mainColor, targetRectRotation, origin,  myScale, targetRectPositionZ);
	
		// left border
		myScale	.x	= (transScaledTargetRect.bottom - transScaledTargetRect.top - 2 * cornerSizeInPixels.y) / textSizeLine.x;
		myScale	.y	= borderSizeInPixels.x / textSizeLine.y;
		origin	.x	= -1*(cornerSizeInPixels.x)/myScale	.x;
		origin	.y	= -1*(-1*borderSizeInPixels.y)/myScale	.y;
		sprite.draw(*line, myPos, nullptr, mainColor, targetRectRotation + wwc::PI / 2, origin,  myScale, targetRectPositionZ);
	
		// right border
		origin	.x	= -1*(cornerSizeInPixels.x)/myScale	.x;
		origin	.y	= -1*(-transScaledTargetRect.right + transScaledTargetRect.left)/myScale	.y;
		sprite.draw(*line, myPos, nullptr, mainColor, targetRectRotation + wwc::PI / 2, origin,  myScale, targetRectPositionZ);
	}
	
	// draw title
	if (textFont) {
		vector2		position, scale, origin	= vector2(0,0);
		calcTextDimensions(position, scale);
		origin.x	-= cornerSizeInPixels.x + titleOffset.x;
		origin.y	-= titleOffset.y;
		textFont->draw(text.c_str(), position, textColor, targetRectRotation, origin, scale, targetRectPositionZ, targetRectClipping, textFont->getLineSpacing(), true);
	}
}

//-----------------------------------------------------------------------------
// Name: windowSizeChanged()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::borderLine2D::windowSizeChanged(int xSize, int ySize)
{
}

//-----------------------------------------------------------------------------
// Name: setAlignmentOnCluster()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::borderLine2D::setAlignmentOnCluster(guiElemCluster2D& cluster, fRECT distance)
{
	fRECT borderRect{(float) ww->getWindowSizeX(), (float) ww->getWindowSizeY(), 0, 0};

	for (auto curItem : cluster.items) {
		// BUG: does not work with getPointer(), but current work around can crash
		// auto curItem2D = curItem->getPointer<guiElement2D>();
		auto curItem2D = (guiElement2D*) curItem;
		if (curItem2D == nullptr) continue;
		auto curItemAlignment = curItem2D->getAlignment();
		// BUG: if function setAlignmentOnCluster() is called before ressourceUpload application will crash
		curItemAlignment->create(ww->alignmentRootFrame);
		fRECT curItemRect = curItemAlignment->getRect(curItem2D->getGridPosition());

		if (curItemRect.left 	- distance.left 	< borderRect.left	) borderRect.left	= curItemRect.left	 - distance.left 	;
		if (curItemRect.right	+ distance.right	> borderRect.right	) borderRect.right	= curItemRect.right	 + distance.right	;
		if (curItemRect.top		- distance.top		< borderRect.top	) borderRect.top	= curItemRect.top	 - distance.top		;
		if (curItemRect.bottom	+ distance.bottom	> borderRect.bottom	) borderRect.bottom	= curItemRect.bottom + distance.bottom	;
	}

	setTargetRect(borderRect.getRECT());
}
#pragma endregion

/*************************************************************************************************************************************/

#pragma region dropDown2D
//-----------------------------------------------------------------------------
// Name: dropDown2D()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::dropDown2D::create(masterMind* ww, buttonImageFiles &filesButton, texture& texArrow, float depthInSpace)
{
	// create button
	if (!plainButton2D::create(ww, filesButton, nullptr, nullptr, depthInSpace)) return false;

	this->alignHorizontal	= alignmentHorizontal::CENTER;
	this->alignVertical		= alignmentVertical  ::CENTER;
	this->textureArrow		= &texArrow;

	assignOnLostFocus				(bind(&wildWeasel::dropDown2D::eventLostFocus				,	this, placeholders::_1, placeholders::_2), this); 
	assignOnLeftMouseButtonPressed	(bind(&wildWeasel::dropDown2D::eventLeftMouseButtonPressed	,	this, placeholders::_1, placeholders::_2), this); 

	return initialized;
}

//-----------------------------------------------------------------------------
// Name: setTextColor()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::dropDown2D::setTextColor(color newColor)						
{
	guiElement::setTextColor(newColor);

	for (auto& curItem : items) {
		curItem->userSubItem->setTextColor(newColor);
	}
};


//-----------------------------------------------------------------------------
// Name: renderSprites()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::dropDown2D::renderSprites(ssf::sharedVars& v)
{
	if (!initialized) return;
	if (!isGuiElemStateVisible(status)) return;

	// draw button
	plainButton2D::renderSprites(v);

	// calcFinalMatrix() is already called by plainButton2D::renderSprites()

	// TODO: createDeviceDependentResources(v) should be called only once! But since flipBook is not a genericObject3D a special solution is necessary
	sprite.createDeviceDependentResources(v);

	// draw sprite
	if (textureArrow) {

		arrowSize.y			= transScaledTargetRect.height() * 0.5f;
		arrowSize.x			= arrowSize.y;

		distArrowToBorder.y	= (transScaledTargetRect.height() - arrowSize.y) * 0.5f;
		distArrowToBorder.x	= distArrowToBorder.y;

		vector2		textSize			= textureArrow  ->getSize();
		vector2		myPos				= {transScaledTargetRect.left, transScaledTargetRect.top};
		vector2		myScale				= { arrowSize.x / textSize.x, arrowSize.y / textSize.y };
		vector2		origin				= { -1 * (transScaledTargetRect.right - transScaledTargetRect.left - arrowSize.x - distArrowToBorder.x) / myScale.x, -1 * distArrowToBorder.y / myScale.y };

		sprite.draw(*textureArrow, myPos, nullptr, mainColor, targetRectRotation, origin,  myScale, targetRectPositionZ);
	}
}

//-----------------------------------------------------------------------------
// Name: keyDown()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::dropDown2D::keyDown(int keyCode)
{
	scrollBarDuo::keyDown(keyCode);
}

//-----------------------------------------------------------------------------
// Name: verticalWheelMoved()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::dropDown2D::verticalWheelMoved(int distance)
{
	scrollBarDuo::keyDown(distance < 0 ? VK_DOWN : VK_UP);
}

//-----------------------------------------------------------------------------
// Name: horizontalWheelMoved()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::dropDown2D::horizontalWheelMoved(int distance)
{
	scrollBarDuo::keyDown(distance < 0 ? VK_LEFT : VK_RIGHT);
}

//-----------------------------------------------------------------------------
// Name: pixelsToScroll()
// Desc: 
//-----------------------------------------------------------------------------
float wildWeasel::dropDown2D::pixelsToScroll(scrollBarDuo::direction dir)
{
	unsigned int itemHeightInPixels = (unsigned int) (transScaledTargetRect.bottom - transScaledTargetRect.top);
	unsigned int restingPixels		= (((unsigned int) abs(scrollOffset.y)) % itemHeightInPixels);

	switch (dir)
	{
	// case scrollBarDuo::direction::LEFT:
	// case scrollBarDuo::direction::RIGHT:
	case scrollBarDuo::direction::UP:
		return -1.0f * ((restingPixels == 0) ? itemHeightInPixels : restingPixels);
	case scrollBarDuo::direction::DOWN:
		return +1.0f * (itemHeightInPixels - restingPixels);
	}

	return 0;
}

//-----------------------------------------------------------------------------
// Name: eventLostFocus()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::dropDown2D::eventLostFocus(guiElemEvFol* elem, void* pUser)
{
	// do not close menu if focus lost on a gui element which belongs to the dropdown
	if (anySubItemSelected()) return;

	// hide menu
	closeMenu();
}

//-----------------------------------------------------------------------------
// Name: eventMousePressedRegion()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::dropDown2D::eventLeftMouseButtonPressed(guiElemEvFol* elem, void* pUser)
{
	plainButton2D::eventLeftMouseButtonPressed(elem, pUser);

	if (menuIsOpen) {
		closeMenu();
	} else {
		openMenu();
	}
}

//-----------------------------------------------------------------------------
// Name: alignAllItems()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::dropDown2D::alignAllItems()
{
	// locals
	vector2			curPos; 
	RECT			curRC			= {0, 0, 0, 0};

	// default size
	if (!itemsRectIsControlledByUser) {
		transScaledItemsRect.left	= transScaledTargetRect.left;
		transScaledItemsRect.top	= transScaledTargetRect.bottom;
		transScaledItemsRect.right	= transScaledTargetRect.right;
		transScaledItemsRect.bottom	= transScaledItemsRect.top + (transScaledTargetRect.bottom - transScaledTargetRect.top) * numVisibleRows;
	}

	// update clipping rect
	clippingArea.left	= transScaledItemsRect.left;
	clippingArea.top	= transScaledItemsRect.top ;
	clippingArea.right	= transScaledItemsRect.right  - (rowScrollBarIsVisible		? rowScrollBarWidth		: 0);
	clippingArea.bottom	= transScaledItemsRect.bottom - (columnScrollBarIsVisible	? columnScrollbarHeight : 0);

	// ... scrolling could be implemented as matrix
	updateScrollOffset();
	updateSledgeWidth();

	// items			  targetRect			    - scrolling
	curPos.x			= transScaledItemsRect.left + scrollOffset.x;
	curPos.y			= transScaledItemsRect.top  + scrollOffset.y;
	curRC.right			= (LONG) (clippingArea.right - clippingArea.left);
	curRC.bottom		= (LONG) ((clippingArea.bottom - clippingArea.top) / numVisibleRows);
	for (auto& curItem : items) {

		curItem->button.setPosition(&curPos, true);
		curItem->button.setTargetRect(curRC);

		if (curItem->userSubItem != nullptr) {
			curItem->userSubItem->setPosition(&curPos, true);
		}

		curPos.y += (transScaledItemsRect.bottom - transScaledItemsRect.top) / numVisibleRows;
	}

	// scrollbars
	if (scrollBarsCreated) {
		columnScrollBar	->setRotation	(0,         false);
		rowScrollBar	->setRotation	(wwc::PI/2, false);
		columnScrollBar ->setPosition	(transScaledItemsRect.left , transScaledItemsRect.bottom - columnScrollbarHeight, false);
		rowScrollBar	->setPosition	(transScaledItemsRect.right, transScaledItemsRect.top,							false);
		columnScrollBar	->setScale		(transScaledItemsRect.right  - transScaledItemsRect.left - (rowScrollBarIsVisible   ?rowScrollBarWidth    :0), (float) columnScrollbarHeight, true);
		rowScrollBar	->setScale		(transScaledItemsRect.bottom - transScaledItemsRect.top  - (columnScrollBarIsVisible?columnScrollbarHeight:0), (float) rowScrollBarWidth,     true);
		columnScrollBar	->alignAllItems();
		rowScrollBar	->alignAllItems();
	}
}


//-----------------------------------------------------------------------------
// Name: insertTextItems()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::dropDown2D::insertTextItems(buttonImageFiles& buttonFiles, int fromNumber, int toNumber, int stepSize)
{
	// locals
	unsigned int	itemIndex = 0;
	wstringstream	wss;
	
	for (int curNumber = fromNumber; curNumber < toNumber; curNumber += stepSize) {
		wss.str(L"");
		wss << curNumber;
		insertTextItem(itemIndex, wss.str(), buttonFiles);
		itemIndex++;
	}
}

//-----------------------------------------------------------------------------
// Name: insertTextItems()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::dropDown2D::insertTextItems(buttonImageFiles& buttonFiles, initializer_list<const wchar_t*> theTexts)
{
	// locals
	unsigned int	itemIndex = 0;
	wstring			wstr;

	for (auto& curText : theTexts) {
		wstr.assign(curText);
		insertTextItem(itemIndex, wstr, buttonFiles);
		itemIndex++;
	}
}

//-----------------------------------------------------------------------------
// Name: insertTextItems()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::dropDown2D::insertTextItems(buttonImageFiles& buttonFiles, vector<wstring>& theTexts)
{
	// locals
	unsigned int itemIndex = 0;

	for (auto& curText : theTexts) {
		insertTextItem(itemIndex, curText, buttonFiles);
		itemIndex++;
	}
}

//-----------------------------------------------------------------------------
// Name: insertTextItem()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::dropDown2D::insertTextItem(unsigned int itemIndex, wstring& text, buttonImageFiles& buttonFiles)
{
	// locals
	wildWeasel::guiElemCluster2D*	myCluster	= new wildWeasel::guiElemCluster2D();
	wildWeasel::plainButton2D*		myButton2D	= new wildWeasel::plainButton2D();
	RECT								rc			= { 0, 0, 0, 0 };

	rc.right	= (LONG) (targetRect.right - targetRect.left);
	rc.bottom	= (LONG) (targetRect.bottom - targetRect.top);

	myButton2D->create(ww, buttonFiles, nullptr, nullptr, 0);
	myButton2D->setTargetRect(rc);
	myButton2D->setTextSize(textScale.x, textScale.y);
	myButton2D->setText(text);
	myButton2D->setFont(textFont);
	myButton2D->setTextState(wildWeasel::guiElemState::HIDDEN);
	myButton2D->setTextColor(wildWeasel::color(0, 0, 0));
	myButton2D->setColor(wildWeasel::color::white);
	myButton2D->setPositioningMode(wildWeasel::matrixControl2D::matControlMode::posRotSca);
	myButton2D->setTextAlignmentHorizontal(wildWeasel::alignmentHorizontal::CENTER);
	myButton2D->setTextAlignmentVertical(wildWeasel::alignmentVertical::CENTER);

	myCluster->create(ww);
	myCluster->addItem(myButton2D);
	myCluster->setTextColor(textColor);
	myCluster->setState		(wildWeasel::guiElemState::HIDDEN);
	myCluster->setTextStates(wildWeasel::guiElemState::HIDDEN);

	this->insertItem(itemIndex, text, buttonFiles, myCluster);
}



//-----------------------------------------------------------------------------
// Name: insertItem()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::dropDown2D::insertItem(unsigned int itemIndex, wstring& text, buttonImageFiles& buttonFiles, guiElemCluster2D* newSubItem)
{
	// params ok?
	itemIndex = std::min(itemIndex, (unsigned int) items.size());

	// locals
	auto			curItem			= items.begin();
	dropDownItem*	newItem			= new dropDownItem();

	// create button
	// SPEED: Sorting Order: https://github.com/Microsoft/DirectXTK/wiki/SpriteBatch
	newItem->button.create(ww, buttonFiles, bind(&dropDown2D::itemWasSelected,  this, placeholders::_1), newItem, targetRectPositionZ);
	newItem->button.setText(text);
	newItem->button.setState(guiElemState::HIDDEN);
	newItem->button.setFont(textFont);
	newItem->button.setTextState(wildWeasel::guiElemState::DRAWED);
	newItem->button.setTextAlignmentHorizontal(wildWeasel::alignmentHorizontal::CENTER);
	newItem->button.setTextAlignmentVertical(wildWeasel::alignmentVertical::CENTER);
	newItem->button.setTextSize(getTextSize());
	newItem->button.informMotherOnEvent(this);

	// add cluster to list view item list
	newItem->userSubItem	= newSubItem;
	advance(curItem, itemIndex);
	items.insert(curItem, newItem);

	// clipping area
	newItem->button.setClippingRect(&clippingArea);
	newItem->userSubItem->setClippingRect(&clippingArea);

	// attach the list view matrix to each sub item
	newItem->button.insertMatrix(100000, &mat, &dirtyBit);
	if (newItem->userSubItem != nullptr) {
		for (auto& curSubItem : newItem->userSubItem->items) {
			curSubItem->insertMatrix(100000, &mat, &dirtyBit);
		}
	}

	// update scroll bar
	updateSledgeWidth();
}

//-----------------------------------------------------------------------------
// Name: deleteItem()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::dropDown2D::removeItem(unsigned int itemIndex,	bool alsoDeleteItem)
{
	// params ok?
	if (itemIndex >= (unsigned int) items.size()) itemIndex = (items.size() ? (unsigned int) items.size() - 1 : 0);

	// locals
	auto			curItem		= items.begin();
	
	advance(curItem, itemIndex);
	
	if (alsoDeleteItem && (*curItem)->userSubItem != nullptr) {
		(*curItem)->userSubItem->deleteAllItems();
	} else {
		(*curItem)->userSubItem->setClippingRect(nullptr);
		for (auto& curSubItem : (*curItem)->userSubItem->items) {
			curSubItem->removeMatrix(&mat);
		}
	}

	delete (*curItem);
	items.erase(curItem);

	// update scroll bar
	updateSledgeWidth();
}

//-----------------------------------------------------------------------------
// Name: assignOnItemChanged()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::dropDown2D::assignOnItemChanged(function<void(guiElement*, void*)> userFunc, void* pUser)
{
	this->userItemChanged	= userFunc;
	this->pUser				= pUser;
}

//-----------------------------------------------------------------------------
// Name: setState()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::dropDown2D::setState(guiElemState newStatus)
{
	guiElemEvFol2D::setState(newStatus);

	if (!isGuiElemStateVisible(status)) {
		closeMenu();
	}
}

//-----------------------------------------------------------------------------
// Name: openMenu()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::dropDown2D::openMenu()
{
	menuIsOpen = true;

	// show sub items
	for (auto& curItem : items) {
		curItem->button.setState(guiElemState::DRAWED);
		if (curItem->userSubItem != nullptr) {
			curItem->userSubItem->setState(guiElemState::VISIBLE);
		}
	}	
	// show scrollbars
	if (columnScrollBar	) columnScrollBar	->setState(columnScrollBarIsVisible	? guiElemState::DRAWED : guiElemState::HIDDEN);
	if (rowScrollBar	) rowScrollBar		->setState(rowScrollBarIsVisible	? guiElemState::DRAWED : guiElemState::HIDDEN);
	
	// arrange positions
	alignAllItems();
}

//-----------------------------------------------------------------------------
// Name: closeMenu()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::dropDown2D::closeMenu()
{
	menuIsOpen		= false;

	// hide all items, except the selected one
	for (auto& curItem : items) {
		curItem->button.setState(guiElemState::HIDDEN);
		if (curItem->userSubItem != nullptr) {
			curItem->userSubItem->setState(guiElemState::HIDDEN);
		}
	}
	
	// hide scrollbars
	if (columnScrollBar	) columnScrollBar	->setState(guiElemState::HIDDEN);
	if (rowScrollBar	) rowScrollBar		->setState(guiElemState::HIDDEN);
}

//-----------------------------------------------------------------------------
// Name: itemWasSelected()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::dropDown2D::itemWasSelected(void* pUser)
{
	selectedItem	= find(items.begin(), items.end(), (dropDownItem*) pUser);
	
	// hide menu
	closeMenu();

	// set text of main button to the one of the selected one
	setText((*selectedItem)->button.getText());

	// set position of user cluster on the main button
	if ((*selectedItem)->userSubItem != nullptr) {
		(*selectedItem)->userSubItem->setPosition(transScaledTargetRect.left, transScaledTargetRect.top, true);
		(*selectedItem)->userSubItem->setState(guiElemState::VISIBLE);
	}

	// call user function
	if (userItemChanged != nullptr) {
		userItemChanged(this, this->pUser);
	}
}

//-----------------------------------------------------------------------------
// Name: windowSizeChanged()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::dropDown2D::windowSizeChanged(int xSize, int ySize)
{
	plainButton2D::windowSizeChanged(xSize, ySize);
	alignAllItems();
	updateSledgeWidth();
}

//-----------------------------------------------------------------------------
// Name: createScrollBars()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::dropDown2D::createScrollBars(buttonImageFiles &filesTriangle, buttonImageFiles &filesLane, buttonImageFiles &filesSledge)
{
	// create
	scrollBarDuo::createScrollBars(this, filesTriangle, filesLane, filesSledge, ww, mat, dirtyBit, targetRectPositionZ);
	return true;
}

//-----------------------------------------------------------------------------
// Name: updateSledgeWidth()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::dropDown2D::updateSledgeWidth()
{
	// ... could be calculated incrementally by each function
	visibleAreaSize.x	 = (transScaledTargetRect.right - transScaledTargetRect.left) - (rowScrollBarIsVisible ? rowScrollBarWidth : 0);
	totalAreaSize.x		 = (transScaledTargetRect.right - transScaledTargetRect.left);

	// ... could be calculated incrementally by each function
	visibleAreaSize.y	= (transScaledTargetRect.bottom - transScaledTargetRect.top) * numVisibleRows - (columnScrollBarIsVisible ? columnScrollbarHeight : 0);
	totalAreaSize.y		= (transScaledTargetRect.bottom - transScaledTargetRect.top) * items.size();

	if (scrollBarsCreated) {
		columnScrollBar	->setSledgeWidth(visibleAreaSize.x / totalAreaSize.x);
		rowScrollBar	->setSledgeWidth(visibleAreaSize.y / totalAreaSize.y);
	}
}

//-----------------------------------------------------------------------------
// Name: columnScrollBarMoved()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::dropDown2D::columnScrollBarMoved(scrollBar2D* bar, void* pUser)
{
	alignAllItems();
}

//-----------------------------------------------------------------------------
// Name: rowScrollBarMoved()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::dropDown2D::rowScrollBarMoved(scrollBar2D* bar, void* pUser)
{
	alignAllItems();
}
#pragma endregion