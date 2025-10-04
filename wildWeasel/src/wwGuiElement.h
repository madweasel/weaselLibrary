/*********************************************************************\
	wwGuiElement.h
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/

#pragma once

#include "wildWeasel.h"
#include "wwFlipbook.h"

namespace wildWeasel
{
	// internal
	class guiElement;
	class guiElement2D;
	class guiElement3D;
	class guiElemCluster;
	class guiElemCluster2D;
	class guiElemCluster3D;
	class guiElemEvFol;
	class guiElemEvFol2D;
	class guiElemEvFol3D;
	
	// 2D elements
	class guiElemCoord;
	class toolTipClass;
	class scrollBar2D;
	class scrollBarDuo;
	class checkBox2D;
	class borderLine2D;
	class dropDown2D;
	class plainButton2D;
	class textLabel2D;

	// 3D elements
	class buttonGeoPrim;
	class plainButton;
	class cubicButton;
	class sphericalButton;
	class triad;
	class lineButton;
	class textLabel3D;

	// external
	class guiElemAnimation;
	//class alignedRect;
	class alignment;

	// a two-dimensional sprite has the same functionality as a 2D text label
	using sprite2D							= textLabel2D;

	// struct for initialization of of 2D gui elements. it contains coordinates and so
	class guiElemCoord
	{
	public:
		unsigned int						xPos,  yPos;																	// upper left corner
		unsigned int						width, height;																	// dimensions
		unsigned int						xDist, yDist;																	// distance between controls
		unsigned int						periodicity;																	// number of controls in one row
	};

	// a general gui object
	class guiElement : protected genericObject3D, public matrixChain
	{
	friend class masterMind;
	friend class guiElemCluster;
	friend class guiElemCluster2D;
	friend class guiElemCluster3D;
	friend class guiElemAnimation;
	friend class blinkAnimation;

	protected:
		masterMind*							ww									= nullptr;
		bool								ownerOfAnimation					= false;
		guiElemAnimation*					animation							= nullptr;
		bool								initialized							= false;								// can only be set to true by create function of final sub class
		guiElemState						status								= guiElemState::UNUSED;					// default state
		guiElemCluster*						cluster								= nullptr;								// pointer to the cluster if this is the case
		color								mainColor							= color::white();						// color
		texture*							mainTexture							= nullptr;								// texture
		wstring								text;																		// text shown on the button
		vector2								textScale							= vector2(1,1);							// 
		vector2								textBorder							= vector2(0.0f, 0.0f);					// space in pixels between text and targetRect
		alignmentHorizontal					alignHorizontal						= alignmentHorizontal::LEFT;			//
		alignmentVertical					alignVertical						= alignmentVertical::TOP;				// 
		guiElemState						textState							= guiElemState::UNUSED;					// default text state
		color								textColor							= color::white();						// default text color
											
		function<void(float)>				userTimeProgress					= nullptr;								// ???
											
											guiElement							();
		virtual								~guiElement							();

		virtual	void						processInTime						(float totalSeconds, float elapsedSeconds);
											
	public:									
		void *								userPointer							= nullptr;								// a pointer which is used by the owner exclusively
											
		wstring								whyAmINotVisible					();
		wstring								whyIsTextNotVisible					();
		void								getText								(wstring& curText)						{ curText = text; };
		const wstring&						getText								()										{ return text; };
		void								setText								(const WCHAR* newText)					;
		void								setText								(const wstring& newText)				;
		void								setTextColor						(color newColor)						{ textColor = newColor; };
		void								setTextState						(guiElemState newStatus)				{ textState = newStatus; };
		void								setTextSize							(float sx, float sy)					{ textScale.x = sx; textScale.y = sy; };
		void								setTextSize							(vector2 s)								{ textScale = s; };
		const vector2&						getTextSize							()										{ return textScale; };
		void								setTextBorder						(float bx, float by)					{ textBorder.x = bx; textBorder.y = by; };
		void								setTextBorder						(vector2 b)								{ textBorder = b; };
		const vector2&						getTextBorder						()										{ return textBorder; };
		void								setColor							(color newColor)						{ mainColor = newColor; };
		void								setTexture							(texture* newTexture)					{ mainTexture = newTexture; };
		void								setTextAlignmentHorizontal			(alignmentHorizontal newAlign)			{ alignHorizontal = newAlign; };
		void								setTextAlignmentVertical			(alignmentVertical newAlign)			{ alignVertical = newAlign; };
		virtual void						setState							(guiElemState newStatus)				{ status = newStatus; };
		guiElemCluster*						getCluster							()										{ return cluster; };
		guiElemState						getState							()										{ return status; };
		void								assignOnTimeProgress				(function<void(float)> userFunc)		{ userTimeProgress = userFunc; };
		template <typename T> T*			getPointer							()										{
																															if (typeid(T) == typeid(*this)) {
																																return (dynamic_cast<T*>(this));
																															} else {
																																return nullptr;
																															}
																														};

		// animations
		void								blinkVisibility						(float duration, unsigned int numberOfTimes, guiElemState finalState);
	};

	// a general gui object
	class guiElement2D : public guiElement, public alignedRect, public matrixControl2D
	{
	protected:
		// used 2D font from DirectX Toolkit
		font2D *							textFont							= nullptr;								// used font

		// since a matrixChain is inherited by guiElement the alignedRect must be transformed in a special 2D way
		vector2								targetRectRotMat2D;															// used for function rotateVectorAndTranslate()
		matrix								targetRectMat3D;															// member targetRect from class alignedRect is considered as additional matrix in the matrix chain of a guiElement2D
		float								targetRectRotation					= 0;
		float								targetRectPositionZ					= 0;
		bool								targetRectDirtyBit					= false;
		fRECT*								targetRectClipping					= nullptr;
		fRECT								transScaledTargetRect;														// the transformed rect is translated and scaled, but not rotated.	thereby, scaling does not apply on the translation.
		fRECT								transScaledClippingRect;
											
		void								updateTransScaledRect				(matrix& mat, fRECT& alignedRect);
		void								rotateVectorAndTranslate			(vector2  aVector, vector2& rotatedVector, bool invertRotation, bool translateBefore, bool translateAfter);
		void								calcSpriteDimensions				(vector2& fromPos, vector2& toPos);
		void								calcFinalMatrix						();
		void								calcTextDimensions					(vector2& fromPos, vector2& scale);
											
	public:									
											guiElement2D						();
		void								setFont								(font2D* newFont)						{ textFont = newFont; };
		void								setClippingRect						(fRECT* clipOnRect)						{ targetRectClipping = clipOnRect; };
		void								getTransScaledTarget				(fRECT& theRect, float& rotation)		{ theRect = transScaledTargetRect; rotation = targetRectRotation; };
		void								setAlignmentWithAnimation			(alignment& newAlignment, float durationInSeconds, bool doNotChangeStartAndFinalAlignment);
	};

	// a general gui object
	class guiElement3D : public guiElement, public matrixControl3D
	{
	protected:
		font3D *							textFont							= nullptr;								// used font
		bool								showOnlyFrontSide					= false;								// true if only one side, which is facing the camera, is beeing rendered

	public:									
											guiElement3D						();
		void								setFont								(font3D* newFont)					{ textFont = newFont; };
		void								setShowTextOnlyForFrontSide			(bool showOnlyFrontSide)			{ this->showOnlyFrontSide = showOnlyFrontSide; };
		wstring								whyIsTextNotVisible					();
	};

	// 2D text label
	class textLabel2D : public guiElement2D, protected eventFollower
	{
	public:
		bool								create								(masterMind* ww, wstring &theText, font2D* theFont, float depthInSpace, alignment& newAlignment, unsigned int alignmentPos);
		bool								create								(masterMind* ww, wstring const &theText, font2D* theFont, float depthInSpace = 0);

	private:
		// variables
		ssf::sprite							sprite;

		// render functions
		void								windowSizeChanged					(int xSize, int ySize);
		void								renderSprites						(ssf::sharedVars& v);
	};

	// 3d text label
	class textLabel3D : public guiElement3D
	{
	public:
		bool								create								(masterMind* ww, wstring const &theText, font3D* theFont);

	private:
		void								renderSprites						(ssf::sharedVars& v);
	};

	// class for showing tooltip boxes
	class toolTipClass : protected textLabel2D
	{
	private:
		void								calcPos								(vector2& toolTipPos, vector2& toolTipSize, fRECT& guiElemRect, float guiElemRotation);

	public:
		void								init								(masterMind* ww, font2D* theFont);
		void								show								(guiElement* elem, wstring& text);
		void								hide								(guiElement* elem);
	};

	//  a parent class for buttons (not defining if plain, cubic, etc.)
	class guiElemEvFol : protected eventFollower
	{
	friend class masterMind;
	friend class eventFollower;
	
	private: 
		wstring								strTooltip;																// string shown to user when he overs over
		toolTipClass*						toolTip								= nullptr;							// tooltip class is informed about hovering
		guiElement*							guiElemToolTip						= nullptr;							// tooltip class is informed about hovering
		vector<eventFollower*>				motherFollowers;														// list of gui elements, which are also informed about an event
		
	protected:
		struct eventFunction
		{
			function<void(guiElemEvFol*, void*)>	func						= nullptr;							// this function will be called in case of an event			
			void*									pointer						= nullptr;							// thereby this user pointer is passed
		};
		
		// general variables
		guiElemState*						pStatus								= nullptr;							// events are not checked if the gui element is inactive
		bool								isMouseOver							= false;							// true, if the cursor is inside the gui element region
		bool								mousePressedInRegion				= false;							// true, if the mouse button was pressed, but not released

		// variables to make sure that only one button is selected by the user at any time (um = unique mode)
		static bool							um_mouseEntered;														// mouse entered a any button region
		static guiElemEvFol*				um_selectedButton;														// selected button by mouse over
		static bool							um_uniqueButtonModeOn;													// true, if mouse can only enter the region of a single gui element. otherwise multiple regions can be entered at once
		static guiElemEvFol*				um_focusedButton;														// gui element which has the current focus
	
		// user functions called when registered event (done in the corresponding assign-function) occurs
		vector<eventFunction>				eventFuncsGotFocus					;									// list of functions to be called in case of an event
		vector<eventFunction>				eventFuncsLostFocus					;
		vector<eventFunction>				eventFuncsLeftMouseButtonPressed	;
		vector<eventFunction>				eventFuncsLeftMouseButtonReleased	;
		vector<eventFunction>				eventFuncsRightMouseButtonPressed	;
		vector<eventFunction>				eventFuncsRightMouseButtonReleased	;
		vector<eventFunction>				eventFuncsMouseEnteredRegion		;
		vector<eventFunction>				eventFuncsMouseLeftRegion			;
		vector<eventFunction>				eventFuncsMouseMove					;

		// constructors/destructors
											guiElemEvFol						(guiElemState* pStatus);
		virtual								~guiElemEvFol						();

		// event follower functions
		void								mouseMoved							(int posX, int posY, const vector3& cursorPos, const vector3& cameraPos);
		void								leftMouseButtonPressed				(int xPos, int yPos);
		void								leftMouseButtonReleased				(int xPos, int yPos);
		void								rightMouseButtonPressed				(int xPos, int yPos);
		void								rightMouseButtonReleased			(int xPos, int yPos);

		void								focusThis							();
		void								addOrRemoveEventFuncFromVector		(eventFunction const& eventFunc, vector<eventFunction>& eventFuncVector);
		void								callEventFunc						(vector<eventFunction>& eventFuncVector);

		// implemented either by guiElemEvFol2D or by guiElemEvFol3D
		virtual bool						isCursorOver						(int posX, int posY, const vector3& cameraPosition, const vector3& cursorPos, float* distanceFromCamera) { return false; };
		virtual void						setState							(guiElemState newStatus);

		void								toolTipShow							();
		void								toolTipHide							();
		
	public:
		// general functions
		void								setUniqueButtonSelecionMode			(bool onlyOneButtonSelectedAtAnyTime);
		bool								isMousePressedInRegion				()	{ return mousePressedInRegion; };
		bool								isMouseOverRegion					()	{ return isMouseOver; };
		bool								hasFocus							()	{ return (um_focusedButton == this); };
		void								informMotherOnEvent					(eventFollower* mother);
		void								doNotInformMotherOnEvent			(eventFollower* mother);
		static guiElemEvFol*				setFocus							(guiElemEvFol* newFocusedElement);
		static guiElemEvFol*				getSelectedGuiElem					();

		// with these functions user defined functions are called on the corresponding event
		void								assignOnGotFocus					(function<void(guiElemEvFol*, void*)> userFunc, void* pUser);
		void								assignOnLostFocus					(function<void(guiElemEvFol*, void*)> userFunc, void* pUser);
		void								assignOnLeftMouseButtonPressed		(function<void(guiElemEvFol*, void*)> userFunc, void* pUser);
		void								assignOnLeftMouseButtonReleased		(function<void(guiElemEvFol*, void*)> userFunc, void* pUser);
		void								assignOnRightMouseButtonPressed		(function<void(guiElemEvFol*, void*)> userFunc, void* pUser);
		void								assignOnRightMouseButtonReleased	(function<void(guiElemEvFol*, void*)> userFunc, void* pUser);
		void								assignOnMouseEnteredRegion			(function<void(guiElemEvFol*, void*)> userFunc, void* pUser);
		void								assignOnMouseLeftRegion				(function<void(guiElemEvFol*, void*)> userFunc, void* pUser);
		void								assignOnMouseMove					(function<void(guiElemEvFol*, void*)> userFunc, void* pUser);
		void								assignToolTip						(const WCHAR* newString, toolTipClass* toolTip, guiElement* pGuiElem);
	};

	//  a parent class for buttons (plain, listView, treeView, etc.)
	class guiElemEvFol2D : public guiElemEvFol, public guiElement2D
	{
	protected:
											guiElemEvFol2D()					: guiElemEvFol(&status)		{};
		float								calcArea							(vector2& A, vector2& B, vector2& C);

	public:
		bool								isCursorOver						(int posX, int posY, const vector3& cameraPosition, const vector3& cursorPos, float* distanceFromCamera);
		virtual void						setState							(guiElemState newStatus)	{ guiElemEvFol::setState(newStatus); guiElement2D::setState(newStatus); };
	};

	//  a parent class for buttons (plain, cubic, etc.)
	class guiElemEvFol3D : public guiElemEvFol, public guiElement3D
	{
	protected:
		indexedRectContainer				rects;
		color								hoverColor							= color::red();

											guiElemEvFol3D()					: guiElemEvFol(&status)		{};
		void								onMouseHoverAction					(guiElemEvFol* elem, void* pUser);
	public:
		
		bool								isCursorOver						(int posX, int posY, const vector3& cameraPosition, const vector3& cursorPos, float* distanceFromCamera);
		virtual void						setState							(guiElemState newStatus)	{ guiElemEvFol::setState(newStatus); guiElement3D::setState(newStatus); };
		void								setHoverColor						(color newHoverColor);
		wstring								whyAmINotVisible					();
	};

	// a group of gui elements sharing a common matrix for positioning and orientation
	class guiElemCluster
	{
	protected:
		// variables
		masterMind*							ww									= nullptr;									// pointer to parent
		bool								initialized							= false;

	public:
		// variables
		list<guiElement*>					items;																		// list containing all items belonging to this cluster of gui elements

		// constructor/destructor
		virtual								~guiElemCluster						();

		// functions
		bool								create								(masterMind* ww);
		void								setState							(guiElemState newStatus);
		void								setTextStates						(guiElemState newStatus);
		void								setTextSize							(float sx, float sy);
		void								setTextBorder						(float bx, float by);
		void								setTextColor						(color newColor);
		void								setTexts							(wstring& newText);
		void								setTexture							(texture* newTexture);
		void								setTextAlignmentHorizontal			(alignmentHorizontal newAlign);
		void								setTextAlignmentVertical			(alignmentVertical newAlign);
		void								insertMatrix						(unsigned int position, matrix* additionalMatrix, bool* additionalDirtyBit);
		void								removeMatrix						(matrix* matrixToRemove);
		void								setButtonColor						(color newColor);
		void								addItem								(guiElement* newItem);
		void								deleteLastItem						();
		void								deleteAllItems						();
		void								clearItemList						();
		bool								isInitialized						() { return initialized; };
		template <typename T> vector<T*>	getContainer						() {
																						vector<T*> myItems;
																						myItems.reserve(items.size());

																						for (auto& curItem : items) {
																							if (typeid(T) == typeid(*curItem)) {
																								myItems.push_back((dynamic_cast<T*>(curItem)));
																							}
																						}

																						return myItems;
																					};

	};

	// a group of gui elements sharing a common matrix for positioning and orientation
	class guiElemCluster2D : public guiElemCluster, public matrixControl2D
	{
	public:
		void								addItem								(guiElement2D* newItem);
		void								clearItemList						();
		void								setClippingRect						(fRECT* clipOnRect);
		// void								setBorderLine						(borderLine2D* theLine);
	};

	// a group of gui elements sharing a common matrix for positioning and orientation
	class guiElemCluster3D : public guiElemCluster, public matrixControl3D
	{
	public:
		void								addItem								(guiElement3D* newItem);
		void								clearItemList						();
		void								setPositionByCluster				(guiElemCluster3D* newPosition)			{ matrixControl3D::setPosition(newPosition->position	, false, true);	};
		void								setScaleByCluster					(guiElemCluster3D* newScale   )			{ matrixControl3D::setScale   (newScale   ->scale		, false, true);	};
		void								setRotationByCluster				(guiElemCluster3D* newRotation)			{ matrixControl3D::setRotation(newRotation->rotation	, false, true);	};
		void								setViewByCluster					(guiElemCluster3D* newView)				{ matrixControl3D::setPointerToViewMatrix	(newView->matView);			};
	};

	// a cubic 3D button with n surfaces
	class buttonGeoPrim : public guiElemEvFol3D
	{
	protected:
		bool								useAlphaNoLightning					= false;
		ssf::shape::genericShape*			shape								= nullptr;

		void								update								(matrix& matView, DX::StepTimer const& timer);
		void								render								(ssf::sharedVars& v);
		void								createDeviceDependentResources		(ssf::sharedVars& v);
		void								createWindowSizeDependentResources	(matrix& matProjection);
		void								onDeviceLost						();
		void								renderSprites						(ssf::sharedVars& v);
	public:
		wstring								whyAmINotVisible					();
	};

	// 2D plain button
	class plainButton2D : public guiElemEvFol2D
	{
	public:
		bool								create								(masterMind* ww, buttonImageFiles &files, function<void(guiElemEvFol*, void*)> buttonFunc, void* pUser, alignment& newAlignment, unsigned int alignmentPos);
		bool								create								(masterMind* ww, buttonImageFiles &files, function<void(guiElemEvFol*, void*)> buttonFunc, void* pUser = nullptr, float depthInSpace = 0);
		void								setState							(guiElemState newStatus)			override;
		void								setImageFiles						(buttonImageFiles &newFiles);

	protected:
		// variables
		flipBook							*normal, *mouseOver, *mouseLeave, *pressed, *grayedOut;					// the five states of a button
		flipBookShelf						animation;

		// event functions
		void								eventMouseEnteredRegion				(guiElemEvFol* elem, void* pUser);
		void								eventMouseLeftRegion				(guiElemEvFol* elem, void* pUser);
		void								eventLeftMouseButtonPressed			(guiElemEvFol* elem, void* pUser);
		void								eventLeftMouseButtonReleased		(guiElemEvFol* elem, void* pUser);
		void								changeState							(flipBook* newState);				// changes activeAnimation and restarts the timer
		void								windowSizeChanged					(int xSize, int ySize);

		// render functions
		void								renderSprites						(ssf::sharedVars& v);
	};

	// 2D scrollbar (Implemented as cluster and not as gui element, since gui element got a bunch of ballast.)
	// triangle - lane - sledge - lane - triangle
	class scrollBar2D : public guiElemCluster2D
	{
	private:
		float								pixelsPerMM							=-1.0f;								// is calculated in function create()
		float								minSledgeWidthInMM					= 2.0f;								// the sledge has a minimum width of 2 mm
		const float							minStepSize							= 0.00001f;							// lane and triangle step size must not be smaller than this fraction of 1
		const float							minTriangleSize						= 0.01f;							// lane and triangle step size must not be smaller than this fraction of 1
		const float							maxTriangleSize						= 0.30f;							// lane and triangle step size must not be greater than this fraction of 1
		float								sledgePos							= 0;								// range from 0 to 1-sledgeWidth;
		float								sledgeWidth							= 1;								// ranges from >0 to 1
		float								laneStepSize						= 0.1f;								// sledge is moved by this amount if lane     is clicked
		float								triangleStepSize					= 0.1f;								// sledge is moved by this amount if triangle is clicked
		float								triangleSize						= 0.1f;								// actual size of a triangle button relative to the total scrollbar length
		POINT								mousePressPos;
		float								sledgePosWhenPressed				= 0;								// set to sledgePos when sledgePressed() is called

		plainButton2D						fromTriangle;
		plainButton2D						fromLane;
		plainButton2D						sledge;
		plainButton2D						toLane;
		plainButton2D						toTriangle;

		void*								pUser								= nullptr;
		function<void(scrollBar2D*, void*)>	userFuncPosChanged					= nullptr;

		void								fromTrianglePressed					(guiElemEvFol* elem, void* pUser);
		void								toTrianglePressed					(guiElemEvFol* elem, void* pUser);
		void								sledgePressed						(guiElemEvFol* elem, void* pUser);
		void								sledgeLaneMoved						(guiElemEvFol* elem, void* pUser);
		void								fromLanePressed						(guiElemEvFol* elem, void* pUser);
		void								toLanePressed						(guiElemEvFol* elem, void* pUser);

	public:
		bool								create								(masterMind* ww, float depthInSpace, buttonImageFiles &filesTriangle, buttonImageFiles &filesLane, buttonImageFiles &filesSledge);
		float								getSledgePos						();
		float								getSledgeWidth						();
		float								getTriangleStepSize					();
		float								getLaneStepSize						();
		void								setSledgePos						(float fromPos);
		void								setSledgeWidth						(float sledgeWidth);
		void								setTriangleStepSize					(float newStepSize);
		void								setTriangleSize						(float newSize);
		void								setLaneStepSize						(float newStepSize);
		void								assignOnSledgePosChange				(function<void(scrollBar2D*, void*)> userFunc, void* pUser);
		bool								doesGuiElemBelongToScrollbar		(guiElemEvFol* elem);
		void								informMotherOnEvent					(eventFollower* mother);
		void								alignAllItems						();
	};

	// for gui elements with two scroll bars
	class scrollBarDuo
	{
	public: 
		enum class							direction							{ LEFT, RIGHT, UP, DOWN };	
		enum class 							scrollPosition						{ TOP, BOTTOM, LEFT, RIGHT, NONE };	// used to determine the position of the scroll bar	

	protected:
		const float							sledgeWidthThreshold				= 0.999f;								// above this threshold the sledge width is considered to be one
		bool								scrollBarsCreated					= false;
		bool								columnScrollBarIsVisible			= false;
		bool								rowScrollBarIsVisible				= false;
		unsigned int						columnScrollbarHeight				= 0;
		unsigned int						rowScrollBarWidth					= 0;
		scrollBar2D*						columnScrollBar						= nullptr;
		scrollBar2D*						rowScrollBar						= nullptr;
		vector2								visibleAreaSize;															// visible area size in pixel
		vector2								totalAreaSize;																// area size in pixel	
		vector2								scrollOffset;																// amount of pixels the elements have to be shifted

		virtual void						updateSledgeWidth					() {};
		virtual void						columnScrollBarMoved				(scrollBar2D* bar, void* pUser) {};
		virtual void						rowScrollBarMoved					(scrollBar2D* bar, void* pUser) {};
		virtual float						pixelsToScroll						(direction dir) { return 0; };

		bool								createScrollBars					(eventFollower* mother, buttonImageFiles &filesTriangle, buttonImageFiles &filesLane, buttonImageFiles &filesSledge, masterMind* ww, matrix& mat, bool& dirtyBit, float depthInSpace);
		void								keyDown								(int keyCode);

	public:
		virtual								~scrollBarDuo						();
		virtual bool						createScrollBars					(buttonImageFiles &filesTriangle, buttonImageFiles &filesLane, buttonImageFiles &filesSledge) { return false; };

		void								updateScrollOffset					();
		void								setVisibilityColumnScrollBar		(bool isVisible);
		void								setVisibilityRowScrollBar			(bool isVisible);
		void								setColumnScrollBarHeight			(unsigned int newHeight);
		void								setRowScrollBarWidth				(unsigned int newWidth);
		void 								scrollTo							(scrollPosition pos);
	};
	
	// 2D checkbox
	class checkBox2D : public guiElemEvFol2D
	{
	private:
		// variables
		flipBook							*normal_c, *mouseOver_c, *mouseLeave_c, *pressed_c, *grayedOut_c;					// checked
		flipBook							*normal_u, *mouseOver_u, *mouseLeave_u, *pressed_u, *grayedOut_u;					// unchecked
		flipBookShelf						animation;
		bool								checkState							= false;
		float								borderWidthForText					= 5;
											
		// event functions					
		void								eventMouseEnteredRegion				(guiElemEvFol* elem, void* pUser);
		void								eventMouseLeftRegion				(guiElemEvFol* elem, void* pUser);
		void								eventLeftMouseButtonPressed			(guiElemEvFol* elem, void* pUser);
		void								eventLeftMouseButtonReleased		(guiElemEvFol* elem, void* pUser);
		void								changeState							(flipBook* newState);				// changes activeAnimation and restarts the timer
		void								windowSizeChanged					(int xSize, int ySize);
											
		// render functions					
		void								renderSprites						(ssf::sharedVars& v);
											
	public:									
		bool								create								(masterMind* ww, buttonImageFiles &filesChecked, buttonImageFiles &filesUnhecked, alignment& newAlignment, unsigned int alignmentPos);
		bool								create								(masterMind* ww, buttonImageFiles &filesChecked, buttonImageFiles &filesUnhecked, float depthInSpace = 0);
		bool								isChecked							();
		void								setCheckState						(bool newCheckState);
		void								setState							(guiElemState newStatus)			override;
	};

	// 2D border line
	class borderLine2D : public guiElement2D, protected eventFollower
	{
	private:
		// variables
		texture*							line;
		texture*							corner;
		ssf::sprite							sprite;
		vector2								borderSizeInPixels					= {  3,  3 };
		vector2								cornerSizeInPixels					= { 15, 15 };
		vector2								titleOffset							= {  8, -10 };
		float								titleWidthInPixels					= 150;
		float								gapBetweenLineAndTitleInPixels		= 5;

		// render functions
		void								windowSizeChanged					(int xSize, int ySize);
		void								renderSprites						(ssf::sharedVars& v);	

	public:
		bool								create								(masterMind* ww, texture& line, texture& corner, float depthInSpace);

		void								setTextOffset						(float ox, float oy)		{ titleOffset.x = ox; titleOffset.y = oy; };
		void								setGapWidthBetweenTextAndLine		(float gapWidth)			{ gapBetweenLineAndTitleInPixels = gapWidth; };
		void								setTextWidth						(float textWidth)			{ titleWidthInPixels = textWidth; };
		void								setAlignmentOnCluster				(guiElemCluster2D& cluster, fRECT distance);
	};

	// 2D drop down menu
	class dropDown2D : public plainButton2D, public scrollBarDuo
	{
	public:									
		bool								create								(masterMind* ww, buttonImageFiles &filesButton, texture& texArrow, float depthInSpace = 0);
		bool								createScrollBars					(buttonImageFiles &filesTriangle, buttonImageFiles &filesLane, buttonImageFiles &filesSledge);
		void								setState							(guiElemState newStatus);
		void								insertTextItems						(buttonImageFiles& buttonFiles, int fromNumber, int toNumber, int stepSize);
		void								insertTextItems						(buttonImageFiles& buttonFiles, initializer_list<const wchar_t*> theTexts);
		void								insertTextItems						(buttonImageFiles& buttonFiles, vector<wstring>& theTexts);
		void								insertTextItem						(unsigned int itemIndex, wstring const& text, buttonImageFiles& buttonFiles);
		void								insertItem							(unsigned int itemIndex, wstring const& text, buttonImageFiles& buttonFiles, guiElemCluster2D* newSubItem = nullptr);
		void								removeItem							(unsigned int itemIndex, bool alsoDeleteItem);
		void								enableBorderLine					(texture& line, texture& corner);
		const wstring&						getSelectedItemText					()	{ return (*selectedItem)->userSubItem->items.front()->getText(); };
		const guiElemCluster2D*				getSelectedItemCluster				()	{ return (*selectedItem)->userSubItem; };
		void								setTextColor						(color newColor);

		void								assignOnItemChanged					(function<void(guiElement*, void*)> userFunc, void* pUser);

	private:

		struct dropDownItem
		{
			guiElemCluster2D*				userSubItem							= nullptr;
			plainButton2D					button;	
		};

		void*								pUser								= nullptr;
		function<void(guiElement*, void*)>	userItemChanged						= nullptr;

		list<dropDownItem*>::iterator		selectedItem;
		list<dropDownItem*>					items;
		texture*							textureArrow						= nullptr;
		borderLine2D*						border								= nullptr;
		ssf::sprite							sprite;
		vector2								distArrowToBorder					= { 0, 0 };
		vector2								arrowSize							= { 0, 0 };
		unsigned int						numVisibleRows						= 5;
		bool								menuIsOpen							= false;
		bool								itemsRectIsControlledByUser			= false;
		fRECT								transScaledItemsRect;														// the transformed rect is translated and scaled, but not rotated.	thereby, scaling does not apply on the translation.
		fRECT								clippingArea;

		void								openMenu							();
		void								closeMenu							();
		void								eventLostFocus						(guiElemEvFol* elem, void* pUser);
		void								eventLeftMouseButtonPressed			(guiElemEvFol* elem, void* pUser);
		void								alignAllItems						();
		void								itemWasSelected						(void* pUser);
		void								columnScrollBarMoved				(scrollBar2D* bar, void* pUser) override;
		void								rowScrollBarMoved					(scrollBar2D* bar, void* pUser) override;
		void								updateSledgeWidth					() override;
		float								pixelsToScroll						(scrollBarDuo::direction dir) override;

		// render functions					
		void								renderSprites						(ssf::sharedVars& v) override;
		void								windowSizeChanged					(int xSize, int ySize) override;
		void								keyDown								(int keyCode) override;
		void								verticalWheelMoved					(int distance) override;
		void								horizontalWheelMoved				(int distance) override;
	};

	// 3D flat button with 2 polygons
	class plainButton : public buttonGeoPrim
	{
	private:
		void								updateMatrixToAlignOnScreen			(fRECT& rc, float depthInSpace);
		POINT*								windowSize							= nullptr;
											
	public:									
		bool								create								(masterMind* ww, void buttonFunc(void* pUser), void* pUser, guiElemCoord* cc, unsigned int posInsideCC, float depthInSpace, bool useAlphaNoLightning);
		bool								create								(masterMind* ww, void buttonFunc(void* pUser), void* pUser, bool useAlphaNoLightning);
		wstring								whyAmINotVisible					();
	};

	// 3D cubic button with 6 surfaces
	class cubicButton : public buttonGeoPrim
	{
	public:
		bool								create								(masterMind* ww);
	};

	// 3D spherical button with n surfaces
	class sphericalButton : public buttonGeoPrim
	{
	public:
		bool								create								(masterMind* ww);
	};

	// 3D triad showing the x-y-z-axis
	class triad : public guiElement3D
	{
	public:
		bool								create								(masterMind* ww, font3D* theFont);
		void								linkWithMatrix						(matrix& mat);
		wstring								whyAmINotVisible					();
	private:
		struct axis
		{
			ssf::shape::cylinder			gfx_geoPrimCylinder;
			ssf::shape::cone				gfx_geoPrimCone;
			matrix							matCylinder;
			matrix							matCone;
			matrix							matLabel;
			textLabel3D						label;
			color							axisColor;
			float							defaultConeLength					= 0.2f;								// as fraction of one. the length of the axis is defined to be one.
			float							defaultConeRadius					= 0.2f;
			float							defaultCylinderRadius				= 0.1f;

			void							create								(masterMind* ww, font3D* theFont, wstring const &theText, color initColor);
		};

		color								defaultColors[3]					= {color::red(), color::green(), color::blue()};
		axis								axisX, axisY, axisZ;
		matrix*								linkedMatrix						= nullptr;
		matrix								matWorld;

		void								update								(matrix& matView, DX::StepTimer const& timer);
		void								render								(ssf::sharedVars& v);
		void								createDeviceDependentResources		(ssf::sharedVars& v);
		void								createWindowSizeDependentResources	(matrix& matProjection);
		void								onDeviceLost						();
	};

	// 3D single line as button
	class lineButton : public guiElemEvFol3D
	{
		// ...
	private:
		float								distanceThreshold;

	public:
		void								setDistThreshold		(float newDistanceThreshold);
		unsigned int						getNumLines				();
	};
	
} // namespace wildWeasel

