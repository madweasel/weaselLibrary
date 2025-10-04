/*********************************************************************
	wwAnimation.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/

#include "wwAnimation.h"

/*************************************************************************************************************************************/

#pragma region animationRoot

//-----------------------------------------------------------------------------
// Name: animationRoot()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::animationRoot::animationRoot(masterMind* ww) 
{
	this->wwAni = ww;
	timerAni.setFunc(ww, timerFuncAni, this);
}

//-----------------------------------------------------------------------------
// Name: isAnimationRunning()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::animationRoot::isAnimationRunning() 
{
	return running; 
}

//-----------------------------------------------------------------------------
// Name: startAnimation()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::animationRoot::startAnimation(float duration) 
{
	// stop if started already
	stopAnimation();

	this->startTime		= (float) wwAni->stepTimer.GetTotalSeconds();
	this->lastCallTime	= startTime;
	this->duration		= duration;
	this->running		= true;

	timerAni.start(wwAni, timerFuncAni, this, (unsigned int) (1000 / updatesPerSecond));
}

//-----------------------------------------------------------------------------
// Name: stopAnimation()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::animationRoot::stopAnimation() 
{
	running = false; 
	timerAni.terminate(); 
}

//-----------------------------------------------------------------------------
// Name: timerFuncAni()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::animationRoot::timerFuncAni(void* pUser)
{
	auto	mai						= (animationRoot*) pUser;
	float	curTime					= (float) mai->wwAni->stepTimer.GetTotalSeconds();
	float	elapsedTimeSinceStart	= curTime - mai->startTime;
	float	elapsedTimeSinceLastCall= curTime - mai->lastCallTime;
	float	fraction				= elapsedTimeSinceStart / mai->duration;
	mai->lastCallTime				= curTime;

	mai->progressInTime(elapsedTimeSinceStart, elapsedTimeSinceLastCall, fraction);
}
#pragma endregion

/*************************************************************************************************************************************/

#pragma region guiElemAnimation
//-----------------------------------------------------------------------------
// Name: guiElemAnimation()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::guiElemAnimation::guiElemAnimation(masterMind* ww, guiElement& theElem) :
	animationRoot	{ ww }, 
	elem			{ &theElem	}
{
	theElem.animation = this;
}

//-----------------------------------------------------------------------------
// Name: processInTime()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemAnimation::processInTime(float totalSeconds, float elapsedSeconds)
{
	// nothing to do
}

//-----------------------------------------------------------------------------
// Name: processInTime()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::guiElemAnimation::elemWasDeleted()
{
	elem		= nullptr;
	wwAni		= nullptr;
	running		= false;
}
#pragma endregion

/*************************************************************************************************************************************/

#pragma region alignedRectAnimation
// //-----------------------------------------------------------------------------
// // Name: guiElemAnimation()
// // Desc: 
// //-----------------------------------------------------------------------------
// wildWeasel::alignedRectAnimation::alignedRectAnimation(guiElement2D& theElem, bool doNotChangeStartAndFinalAlignment) : 
// 	guiElemAnimation{theElem.ww, theElem}, 
// 	rect { &theElem}, 
// 	doNotChangeStartAndFinalAlignment { doNotChangeStartAndFinalAlignment }
// {
// }

//-----------------------------------------------------------------------------
// Name: guiElemAnimation()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::alignedRectAnimation::alignedRectAnimation(masterMind* ww, guiElement& theElem, alignedRect& theRect, bool doNotChangeStartAndFinalAlignment) : 
	guiElemAnimation{ww, theElem}, 
	rect { &theRect}, 
	doNotChangeStartAndFinalAlignment { doNotChangeStartAndFinalAlignment }
{
}

//-----------------------------------------------------------------------------
// Name: alignedRectAnimation()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::alignedRectAnimation::~alignedRectAnimation()
{
	stop(true);
}

//-----------------------------------------------------------------------------
// Name: processInTime()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignedRectAnimation::start(float duration, alignment& targetAlignment)
{
	stop(false);

	// locals
	alignment* rectAlignment	= rect->getAlignment();

	// if this gui element is controlled by an alignment class, then either use it or make a copy of it
	if (rectAlignment) {
		if (doNotChangeStartAndFinalAlignment) {
			movedAlignment	= new alignment{*rectAlignment};
			startAlignment	= rectAlignment;
			gridPosition	= rect->getGridPosition();
			rect->setAlignment(*movedAlignment);
		} else {
			movedAlignment	= rectAlignment;
			startAlignment	= new alignment{*rectAlignment};
		}
	// if this gui element is directly controlled by the target rect then make a proper alignment instance
	} else {
		// ... not supported yet
		return;
	}
	this->targetAlignment	= &targetAlignment;

	animationRoot::startAnimation(duration);
}

//-----------------------------------------------------------------------------
// Name: processInTime()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignedRectAnimation::stop(bool setDirectlyToTarget)
{
	animationRoot::stopAnimation();

	if (setDirectlyToTarget) {
		rect->setAlignment(*targetAlignment);
	}

	if (doNotChangeStartAndFinalAlignment) {
		if (movedAlignment) { delete movedAlignment; movedAlignment = nullptr; }
	} else {
		if (startAlignment) { delete startAlignment; startAlignment = nullptr; }
	}
}

//-----------------------------------------------------------------------------
// Name: processInTime()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignedRectAnimation::processInTime(float totalSeconds, float elapsedSeconds)
{
	if (!running) return;

	float fractionDone = (totalSeconds - startTime) / duration;

	if (fractionDone >= 1) {
		stop(true);
	} else {
		fRECT			startRect		= startAlignment ->getRect(gridPosition);
		fRECT			targetRect		= targetAlignment->getRect(gridPosition);

		movedAlignment->left  .setPosition((targetRect.left   - startRect.left  ) * fractionDone + startRect.left  );
		movedAlignment->top   .setPosition((targetRect.top    - startRect.top   ) * fractionDone + startRect.top   );
		movedAlignment->right .setPosition((targetRect.right  - startRect.right ) * fractionDone + startRect.right );
		movedAlignment->bottom.setPosition((targetRect.bottom - startRect.bottom) * fractionDone + startRect.bottom);
	}
}
#pragma endregion

/*************************************************************************************************************************************/

#pragma region blinkAnimation
//-----------------------------------------------------------------------------
// Name: blinkAnimation()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::blinkAnimation::blinkAnimation(masterMind* ww, guiElement& theElem) : guiElemAnimation{ww, theElem}
{
}

//-----------------------------------------------------------------------------
// Name: blinkAnimation()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::blinkAnimation::~blinkAnimation()
{
	stop();
}

//-----------------------------------------------------------------------------
// Name: processInTime()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::blinkAnimation::startBlinkVisibility(float duration, unsigned int numberOfTimes, wildWeasel::guiElemState finalState)
{
	this->type				= blinkType::VISIBILITY;
	this->finalState		= finalState;
	this->numberOfTimes		= numberOfTimes;
	animationRoot::startAnimation(duration);
}

//-----------------------------------------------------------------------------
// Name: processInTime()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::blinkAnimation::startBlinkColor(float duration, unsigned int numberOfTimes, wildWeasel::guiElemState finalState, wildWeasel::color blinkColor)
{
	this->type				= blinkType::COLOR;
	this->blinkColor		= blinkColor;
	this->startColor		= elem->mainColor;
	this->finalState		= finalState;
	this->numberOfTimes		= numberOfTimes;
	animationRoot::startAnimation(duration);
}

//-----------------------------------------------------------------------------
// Name: processInTime()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::blinkAnimation::stop()
{
	switch (type) 
	{
	case blinkType::COLOR:			elem->setColor(startColor);		break;
	case blinkType::VISIBILITY:										break;
	case blinkType::SIZE:			/*...*/							break;
	}

	elem->setState(finalState);

	animationRoot::stopAnimation();
}

//-----------------------------------------------------------------------------
// Name: processInTime()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::blinkAnimation::processInTime(float totalSeconds, float elapsedSeconds)
{
	if (!running) return;

	float				fractionDone = (totalSeconds - startTime) / duration;

	if (fractionDone >= 1) {
		stop();
	} else {
		switch (type) 
		{
		case blinkType::COLOR:
			// ... elem->setColor(startColor);
			break;
		case blinkType::VISIBILITY:		
			wildWeasel::guiElemState curState;
			if (((int) (fractionDone * numberOfTimes * 2)) % 2 == 0) {
				curState = wildWeasel::guiElemState::HIDDEN;
			} else {
				curState = wildWeasel::guiElemState::VISIBLE;
			}
			elem->setState(curState);
			break;
		case blinkType::SIZE:			
			/*...*/			
			break;
		}
	}
}
#pragma endregion

/*************************************************************************************************************************************/

#pragma region clusterAnimation
//-----------------------------------------------------------------------------
// Name: startAnimation()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::clusterAnimation::startAnimation(float duration)
{
	// show rotation animation
	if (!running) {
		size_t i = 0;
		aniItems.resize(pItems->size());
		for (auto& curItem : (*pItems)) {
			aniItems[i].dirtyBit	= false;
			aniItems[i].mat			= matrix::Identity;
			aniItems[i].elem		= curItem;
		//	if (typeid(curItem) == typeid(guiElement3D*)) {	// ... does not work
				(static_cast<guiElement3D*>(curItem))->insertMatrix(-1, &aniItems[i].mat, &aniItems[i].dirtyBit);
		//	}
			i++;
		}
		animationRoot::startAnimation(duration);
		return true;
	} else {
		return false;
	}
}

//-----------------------------------------------------------------------------
// Name: clusterAnimation()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::clusterAnimation::clusterAnimation(masterMind* ww, list<guiElement*>& theItems) : animationRoot {ww}
{
	this->pItems	= &theItems;
}

//-----------------------------------------------------------------------------
// Name: stopAnimation()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::clusterAnimation::stopAnimation()
{
	for (auto& curItem : aniItems) {
	//	if (typeid(curItem.elem) == typeid(guiElement3D*)) {	// ... does not work
			(static_cast<guiElement3D*>(curItem.elem))->removeMatrix(&curItem.mat);
	//	}
	}
	animationRoot::stopAnimation();
}

//-----------------------------------------------------------------------------
// Name: explode()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::clusterAniExplosion::explode()
{
	// do not abort running explosion
	if (running) return;

	exploded			= false;

	clusterAnimation::startAnimation(4 * countDown);
	vecCurrentVelocity.resize(aniItems.size());
}

//-----------------------------------------------------------------------------
// Name: clusterAniExplosion()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::clusterAniExplosion::clusterAniExplosion(masterMind* ww, list<guiElement*>& aniItems) : clusterAnimation(ww, aniItems)
{
}

//-----------------------------------------------------------------------------
// Name: progressInTime()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::clusterAniExplosion::progressInTime(float elapsedTimeSinceStart, float elapsedTimeSinceLastCall, float fraction)
{
	// locals
	unsigned int			secondsPassed	= (unsigned int) elapsedTimeSinceStart;
	wildWeasel::vector3	position;
	size_t					i;

	// count down
	if (secondsPassed < countDown) {
		wstringstream wss;
		for (auto& curItem : aniItems)  {
			wss.str(L""); wss << countDown - secondsPassed;
			curItem.elem->setText(wss.str());
			curItem.elem->setTextState(wildWeasel::guiElemState::VISIBLE);
		}

	// explosion
	} else if (secondsPassed < duration) {

		// initiate explosion?
		if (!exploded) {
			uniform_real_distribution<float> distribution(-1,1);
			exploded = true;
			i = 0;
			for (auto& curItem : aniItems) {
			//	if (typeid(curItem.elem) == typeid(guiElement3D*)) {	// ... does not work
					(static_cast<guiElement3D*>(curItem.elem))->getPosition(position);
			//	}
				vecCurrentVelocity[i] = (explosiveStrength + distribution(wwAni->randomEngine) * explosiveStrengthVariation) * position + explosiveDirection;
				curItem.mat = wildWeasel::matrix::Identity;
				curItem.elem->setTextState(wildWeasel::guiElemState::HIDDEN);
				i++;
			}
		}

		// move buttons
		i = 0; 
		for (auto& curItem : aniItems) {
			vecCurrentVelocity[i]	+= vecGravity * elapsedTimeSinceLastCall;
			curItem.dirtyBit		 = true;
			curItem.mat				+= wildWeasel::matrix::CreateTranslation(vecCurrentVelocity[i] * elapsedTimeSinceLastCall) - matrix::Identity;
			i++;
		}

	// quit
	} else {
		stopAnimation();
	}
}
#pragma endregion

/*************************************************************************************************************************************/

#pragma region matrixControlAnimation
//-----------------------------------------------------------------------------
// Name: matrixControlAnimation()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::matrixControlAnimation::matrixControlAnimation(masterMind* ww) : 
	animationRoot {ww}
{
}

//-----------------------------------------------------------------------------
// Name: matrixControlAnimation()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::matrixControlAnimation::~matrixControlAnimation()
{
	stopAnimation();
}

//-----------------------------------------------------------------------------
// Name: append()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::matrixControlAnimation::append(matrixControl3D* newMat)
{
	append(&newMat->mat, &newMat->dirtyBit);
}

//-----------------------------------------------------------------------------
// Name: append()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::matrixControlAnimation::append(matrix* newMat, bool* newDirtyBit)
{
	matrixDirty newMatDirty{newMat, newDirtyBit};
	matrices.push_back(newMatDirty);
}

//-----------------------------------------------------------------------------
// Name: remove()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::matrixControlAnimation::remove(matrixControl3D* newMat)
{
	remove(&newMat->mat, &newMat->dirtyBit);
}

//-----------------------------------------------------------------------------
// Name: remove()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::matrixControlAnimation::remove(matrix* newMat, bool* newDirtyBit)
{
	matrixDirty newMatDirty{newMat, newDirtyBit};
	matrices.erase(find(matrices.begin(), matrices.end(), newMatDirty));
}

//-----------------------------------------------------------------------------
// Name: start()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::matrixControlAnimation::startAnimation(float duration, matrix& origin, matrix& target)
{
	float epsilon = 0.001f;

	// show rotation animation
	if (!running) {
		matStart			= origin;
		matTarget			= target;
		useRotationalAxis	= (abs(matStart.Determinant() - 1) < epsilon && abs(matTarget.Determinant() - 1) < epsilon);

		if (useRotationalAxis) {
			matrix		matStartInv = matStart.Transpose();
			matrix		deltaMatrix	= matTarget * matStartInv;

			// // Option A:
			// rotAngle					= -acos((deltaMatrix._11 + deltaMatrix._22 + deltaMatrix._33 - 1) / 2);
			// rotAxis					= { deltaMatrix._32 - deltaMatrix._23, deltaMatrix._13 - deltaMatrix._31, deltaMatrix._21 - deltaMatrix._12};
			// rotAxis.Normalize();
			// if (abs(rotAxis.x) < epsilon && abs(rotAxis.y) < epsilon && abs(rotAxis.z) < epsilon) {
			// 	useRotationalAxis	= false;
			// }

			// Option B: 
			quaternion q			= quaternion::CreateFromRotationMatrix(deltaMatrix);
			rotAngle				= (q.w > 0 ? 2*acos(q.w) : -2*acos(-q.w));
			rotAxis					= vector3{q.x, q.y, q.z};
			rotAxis.Normalize();
		}

		animationRoot::startAnimation(duration);
	}
}

//-----------------------------------------------------------------------------
// Name: progressInTime()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::matrixControlAnimation::progressInTime(float elapsedTimeSinceStart, float elapsedTimeSinceLastCall, float fraction)
{
	matrix	curMat;
	
	if (useRotationalAxis) {
		curMat	= matrix::CreateFromAxisAngle(rotAxis, rotAngle * fraction) * matStart;
	} else {
		curMat	= (matTarget - matStart) * fraction + matStart;
	}
	
	if (fraction > 1) {
		curMat = matTarget;
		stopAnimation();
	}

	for (auto& curMatrixDirty : matrices) {
		*(curMatrixDirty.dirtyBit ) = true;
		*(curMatrixDirty.mat)		= curMat;
	}
}
#pragma endregion

/*************************************************************************************************************************************/

