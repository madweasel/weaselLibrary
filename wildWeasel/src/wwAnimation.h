/*********************************************************************\
	wwAnimation.h												  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/

#pragma once

#include "wildWeasel.h"

namespace wildWeasel
{
	// internal pre-definition
	class animationRoot;
	class matrixControlAnimation;
	class guiElemAnimation;
	class alignedRectAnimation;
	class blinkAnimation;
	class clusterAnimation;
	class clusterAniExplosion;

	// external pre-definition
	
	// a baseline class for all animations
	class animationRoot
	{
	protected:
		masterMind*							wwAni								= nullptr;								// parent class
		timer								timerAni;																	// timer object, which regurlarly calls the function timerFuncAni()
		float								startTime							= 0;									// time point at which the whole animation started
		float								lastCallTime						= 0;									// time point at which the last update was done
		float								duration							= 0;									// total duration of the animation in seconds
		float								updatesPerSecond					= 25;									// times per second the finishing animation is updated
		bool								running								= false;								// state of the animation

											animationRoot						(masterMind* ww);
		void								startAnimation						(float duration);
		void								stopAnimation						();
		virtual void						progressInTime						(float elapsedTimeSinceStart, float elapsedTimeSinceLastCall, float fraction) {};
		static void							timerFuncAni						(void* pUser);

	public:
		bool								isAnimationRunning					();
	};

	// class for easy animation of matrixControl3D objects
	class matrixControlAnimation : public animationRoot
	{
	public:
											matrixControlAnimation				(masterMind* ww);
											~matrixControlAnimation				();
		void								append								(matrixControl3D* newMat);
		void								append								(matrix* newMat, bool* newDirtyBit);
		void								remove								(matrixControl3D* newMat);
		void								remove								(matrix* newMat, bool* newDirtyBit);
		void								startAnimation						(float duration, matrix& origin, matrix& target);

	private:
		vector<matrixDirty>					matrices;																	// matrices and dirty bits to be updated by the animation
		matrix								matTarget;																	// target matrix for all associated matrixes
		matrix								matStart;																	// starting matrix
		bool								useRotationalAxis;
		vector3								rotAxis;
		float								rotAngle;

		void								progressInTime						(float elapsedTimeSinceStart, float elapsedTimeSinceLastCall, float fraction);
	};

	class guiElemAnimation : public animationRoot
	{
	public:
		virtual void						processInTime						(float totalSeconds, float elapsedSeconds);
		void								elemWasDeleted						();

	protected:
		guiElement*							elem								= nullptr;								// considered gui element

											guiElemAnimation					(masterMind* ww, guiElement& theElem);
	};

	class alignedRectAnimation : public guiElemAnimation
	{
	private:
		alignedRect*						rect								= nullptr;								// considered alignRect to be animated
		alignment*							startAlignment						= nullptr;								// animation starting point
		alignment*							targetAlignment						= nullptr;								// animation target
		alignment*							movedAlignment						= nullptr;								// modified alignment during the animation
		bool								doNotChangeStartAndFinalAlignment	= false;								// 
		unsigned int						gridPosition						= 0;									// 

		void								processInTime						(float totalSeconds, float elapsedSeconds);

	public:
											alignedRectAnimation				(masterMind* ww, guiElement& theElem, alignedRect& theRect, bool doNotChangeStartAndFinalAlignment);
											~alignedRectAnimation				();
		void								start								(float duration, alignment& targetAlignment);
		void								stop								(bool setDirectlyToTarget);
	};

	class blinkAnimation : public guiElemAnimation
	{
	private:
		enum class							blinkType							{ COLOR, VISIBILITY, SIZE };			// either the color, the visibility or the size is switched between two states

		blinkType							type								= blinkType::VISIBILITY;				// default and chosen blink type
		unsigned int						numberOfTimes						= 0;									// amount of blinks
		wildWeasel::guiElemState		finalState;																	// final state to be set after the animation
		wildWeasel::color				blinkColor;																	// toggle between blinkColor and startColor
		wildWeasel::color				startColor;																	// toggle between blinkColor and startColor

		void								processInTime						(float totalSeconds, float elapsedSeconds);

	public:
											blinkAnimation						(masterMind* ww, guiElement& theElem);
											~blinkAnimation						();
		void								startBlinkVisibility				(float duration, unsigned int numberOfTimes, wildWeasel::guiElemState finalState);
		void								startBlinkColor						(float duration, unsigned int numberOfTimes, wildWeasel::guiElemState finalState, wildWeasel::color	blinkColor);
		void								stop								();
	};
	
	class clusterAnimation : public animationRoot
	{
	protected:
		struct aniItemStruct
		{
			guiElement*						elem;																		// corresponding gui element
			matrix							mat;																		// matrix for the gui element, which is added into its matrix chain
			bool							dirtyBit;																	// set to true when the matrix has been changed
		};
		vector<aniItemStruct>				aniItems;																	// all animated items of the cluster
		list<guiElement*>*					pItems								= nullptr;								// since there is no access to the cluster::item object a copy is necessary
		
											clusterAnimation					(masterMind* ww, list<guiElement*>& theItems);
		bool								startAnimation						(float duration);
	
	public:
		void								stopAnimation						();
	};

	// explosion
	class clusterAniExplosion : public clusterAnimation
	{
	private:
		float								countDown							= 5;									// time in seconds of the finishing animation count down
		bool								exploded							= false;								// true, if the explosion succeeded and the single rectangles are flying through the air
		vector3								vecGravity							= vector3(0,-11.0f,0);					// gravity acting on the rectangles
		vector3								explosiveDirection					= vector3(0, 0.75f,0);					// initial velocity for all rectangles upon explosion
		float								explosiveStrength					= 1.5f;									// a random velocity is added on each rectangle upon the explosion
		float								explosiveStrengthVariation			= 2.0f;									// random variation of the added velocity
		vector<vector3>						vecCurrentVelocity;															// current velocity of each single rectangle

		void								progressInTime						(float elapsedTimeSinceStart, float elapsedTimeSinceLastCall, float fraction);

	public:
											clusterAniExplosion					(masterMind* ww, list<guiElement*>& aniItems);
		void								explode								();
	};

} // namespace wildWeasel

