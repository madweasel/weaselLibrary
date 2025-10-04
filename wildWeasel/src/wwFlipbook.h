/*********************************************************************\
	wwFlipbook.h
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/

#pragma once

#include "wildWeasel.h"

namespace wildWeasel
{
	// internal pre-definition
	class commonFlipBookVars;
	class flipBookShelf;
	class flipBook;
	class animationFilename;
	class buttonImageFiles;

	// filename and additional information for initialization of flipbooks
	class animationFilename
	{
	public:
		texture								tex;
		wstring								filename;																// filename of a .bmp, .jpg, .png. the path is set by setTexturesPath()
		unsigned int						numberOfImages						= 0;								// number of images beneath each other. thereby the height of each image is defined (= totalHeight / num).
		unsigned int						timeInMilliseconds					= 0;								// time in milliseconds for the whole animation
											
		animationFilename					(animationFilename &af)									{filename.assign(af.filename);	numberOfImages = af.numberOfImages; timeInMilliseconds = af.timeInMilliseconds; };
		animationFilename					(const wchar_t* str, unsigned int n, unsigned int t)	{filename.assign(str);			numberOfImages = n;					timeInMilliseconds = t; };
	};

	// strcuture containing all the necessary files for a button
	class buttonImageFiles
	{
	public:
		animationFilename					normal, mouseOver, mouseLeave, pressed, grayedOut;						// the five states of a button
		
		buttonImageFiles					(animationFilename &n, animationFilename &o, animationFilename &l, animationFilename &p, animationFilename &g) : normal(n), mouseOver(o), mouseLeave(l), pressed(p), grayedOut(g) { };
		buttonImageFiles					(	const wchar_t* strN, unsigned int nN, unsigned int tN,
												const wchar_t* strO, unsigned int nO, unsigned int tO,
												const wchar_t* strL, unsigned int nL, unsigned int tL,
												const wchar_t* strP, unsigned int nP, unsigned int tP,
												const wchar_t* strG, unsigned int nG, unsigned int tG) : 
												normal		(strN, nN, tN), 
												mouseOver	(strO, nO, tO), 
												mouseLeave	(strL, nL, tL), 
												pressed		(strP, nP, tP), 
												grayedOut	(strG, nG, tG) { };
	};

	// shared variables by several flipbooks of a single 2D button
	class commonFlipBookVars
	{
	public:
		masterMind *						parent								= nullptr;							// pointer to parent
		fRECT**								clippingRect						= nullptr;							// only a pointer to a rect, since it shall be controlled by the owner
		fRECT*								targetRect							= nullptr;							// only a pointer to a rect, since it shall be controlled by the owner
		float*								rotationZ							= nullptr;
		float*								positionZ							= nullptr;
		color*								mainColor							= nullptr;
		DWORD								lastTickCount						= 0;								// time point at which the bitmap was rendered the last time
		UINT_PTR							timerId								= 0;								// return value of setTimer for reference of the timer
		float								restingTimes						= 0;								// number of images still to show until end of animation cycle
		bool								repeat								= false;							// constantly repeat the animation
	};

	// a single 2D animation
	class flipBook
	{
	public:
		ssf::sprite							sprite;
		texture*							images;
		commonFlipBookVars*					vars;
		unsigned int						numberOfImages						= 0;								// number of images beneath each other. thereby the height of each image is defined (= totalHeight / num).
		unsigned int						timeInMilliseconds					= 0;								// time in milliseconds for the whole animation
		float								invNumberOfImages					= 0;								// should speed up a little since not a division but a multiplication can be used
		float								invTimeInMilliseconds				= 0;								// ''

		void								renderSprites						(ssf::sharedVars& v);
		void								triggerRedraw						();
		void								loadGraphics						(commonFlipBookVars* vars, animationFilename &filename);
	};

	// a group of 2D animation containing several flipbooks
	class flipBookShelf
	{			
	public:
		flipBook*							activeFlipBook						= NULL;								// pointer to the current active bitmap
		list<flipBook*>						flipBooks;																// 
		commonFlipBookVars					vars;																	//

											~flipBookShelf						();
		BOOL								setParent							(masterMind *parent);
		BOOL								setDimensionPointers				(fRECT* newRect, float* rotationZ, float* positionZ, color* mainColor, fRECT** clipRect);
		BOOL								addFlipBook							(flipBook **newFlipBook, animationFilename &filename);
		BOOL								setActiveFlipBook					(flipBook *newFlipBook);
		BOOL								startAnimation						();
		BOOL								stopAnimation						();
	
		static VOID CALLBACK				TimerProc							(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
	};

} // namespace wildWeasel

