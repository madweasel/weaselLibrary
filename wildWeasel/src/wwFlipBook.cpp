/*********************************************************************
	wwFlipBook.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/

#include "wildWeasel.h"

#pragma region flipBookShelf
//-----------------------------------------------------------------------------
// Name: flipBookShelf::setParent()
// Desc: 
//-----------------------------------------------------------------------------
BOOL wildWeasel::flipBookShelf::setParent(wildWeasel::masterMind *parent)
{
	this->vars.parent			= parent;
	return TRUE;
}

//-----------------------------------------------------------------------------
// Name: flipBookShelf::makeFlipBook()
// Desc: 
//-----------------------------------------------------------------------------
BOOL wildWeasel::flipBookShelf::setDimensionPointers(fRECT* newRect, float* rotationZ, float* positionZ, color* mainColor, fRECT** clipRect)
{
	this->vars.targetRect	= newRect;
	this->vars.rotationZ	= rotationZ;
	this->vars.positionZ	= positionZ;
	this->vars.mainColor	= mainColor;
	this->vars.clippingRect	= clipRect;
	return TRUE;
}

//-----------------------------------------------------------------------------
// Name: flipBookShelf::makeFlipBook()
// Desc: 
//-----------------------------------------------------------------------------
BOOL wildWeasel::flipBookShelf::addFlipBook(flipBook **newFlipBook, animationFilename &filename)
{
	flipBook* myNewFlipBook = new flipBook();

	myNewFlipBook->loadGraphics(&vars, filename);

	flipBooks.push_back(myNewFlipBook);
	*newFlipBook	= myNewFlipBook;
	activeFlipBook	= myNewFlipBook;

	return TRUE;
}

//-----------------------------------------------------------------------------
// Name: flipBookShelf::setActiveFlipBook()
// Desc: 
//-----------------------------------------------------------------------------
BOOL wildWeasel::flipBookShelf::setActiveFlipBook(flipBook *newFlipBook)
{
	activeFlipBook		= newFlipBook;
	vars.restingTimes	= 0;
	return TRUE;
}

//-----------------------------------------------------------------------------
// Name: flipBookShelf::startAnimation()
// Desc: 
//-----------------------------------------------------------------------------
BOOL wildWeasel::flipBookShelf::startAnimation()
{
	vars.lastTickCount	= GetTickCount();
	vars.timerId		= SetTimer(vars.parent->getHwnd(), (UINT_PTR) this, activeFlipBook->timeInMilliseconds / activeFlipBook->numberOfImages, (TIMERPROC) TimerProc);
	vars.restingTimes	= (float) (activeFlipBook->numberOfImages - (unsigned int) vars.restingTimes - 1);
	return TRUE;
}

//-----------------------------------------------------------------------------
// Name: flipBookShelf::stopAnimation()
// Desc: 
//-----------------------------------------------------------------------------
BOOL wildWeasel::flipBookShelf::stopAnimation()
{
	vars.restingTimes = 0;
	return TRUE;
}

//-----------------------------------------------------------------------------
// Name: TimerProc()
// Desc: 
//-----------------------------------------------------------------------------
VOID CALLBACK wildWeasel::flipBookShelf::TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	// this pointer is stored in timer id
	wildWeasel::flipBookShelf* me = (wildWeasel::flipBookShelf*) idEvent;
	me->activeFlipBook->triggerRedraw();
}

//-----------------------------------------------------------------------------
// Name: flipBookShelf::wmPaint()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::flipBookShelf::~flipBookShelf()
{
	for (auto itr = flipBooks.begin(); itr != flipBooks.end(); itr++) {
		delete (*itr);
		*itr = nullptr;
	}
}
#pragma endregion

#pragma region flipBook
//-----------------------------------------------------------------------------
// Name: flipBook::loadGraphics()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::flipBook::loadGraphics(commonFlipBookVars* vars, animationFilename &filename)
{
	if (vars != nullptr) {
		this->vars = vars;
	}

	this->invTimeInMilliseconds = 1.0f / filename.timeInMilliseconds;
	this->invNumberOfImages		= 1.0f / filename.numberOfImages;

 	this->numberOfImages		= filename.numberOfImages;
 	this->timeInMilliseconds	= filename.timeInMilliseconds;
	if (filename.tex.getTextureResource() == nullptr) {
		filename.tex.loadFile(this->vars->parent, filename.filename, true);
	}
	this->images = &filename.tex;
}

//-----------------------------------------------------------------------------
// Name: triggerRedraw()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::flipBook::triggerRedraw()
{
	// initiate redraw
	// ...
}

//-----------------------------------------------------------------------------
// Name: triggerRedraw()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::flipBook::renderSprites(ssf::sharedVars& v)
{
	// locals
	texture* curTexture = images;

	// TODO: createDeviceDependentResources(v) should be called only once! But since flipBook is not a genericObject3D a special solution is necessary
	sprite.createDeviceDependentResources(v);
	
	// parameters ok?
	if ((unsigned int) vars->restingTimes > numberOfImages) return;
	if (vars->restingTimes < 0) vars->restingTimes = 0;
	
	// get dimensions of source image
	DWORD		curTickCound	= GetTickCount();
	vector2		texSize			= curTexture->getSize();
	float		srcImgHeight	= (float) texSize.y * invNumberOfImages;
	float		srcImgWidth		= (float) texSize.x;
	float		srcImgOffsetY	= (float) srcImgHeight * (numberOfImages - (unsigned int) vars->restingTimes - 1);
	fRECT		sourceRect		= { 0, srcImgOffsetY, srcImgWidth, (srcImgOffsetY + srcImgHeight) };
	fRECT		destRect		= *vars->targetRect;
	vector2		origin			= {0,0};

	// draw sprite
	if (curTexture && curTexture->getTextureResource() && vars->mainColor) {
		if (vars->clippingRect == nullptr || *vars->clippingRect == nullptr || performClippingAndCheck(destRect, sourceRect, origin, **vars->clippingRect, *vars->rotationZ)) {
			sprite.draw(*curTexture, destRect, &sourceRect, *vars->mainColor, *vars->rotationZ, origin, *vars->positionZ);
		}
	}
			
	// progress or kill timer
	vars->restingTimes -= (curTickCound - vars->lastTickCount) * invTimeInMilliseconds * numberOfImages;
	vars->lastTickCount = curTickCound;
	if (vars->restingTimes < 0) {
		KillTimer(vars->parent->getHwnd(), vars->timerId);
		vars->restingTimes = 0;
	}
}
#pragma endregion
