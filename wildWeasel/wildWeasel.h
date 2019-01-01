/*********************************************************************\
	wildWeasel.h												  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/

#pragma once

// precompiled header
#include "..\\DirectXTK11\\pch.h"

// own libraries
#include "strLib.h"
#include "xml.h"

/*** Constants ******************************************************/
namespace wwc
{
	const float								PI 									= 3.1415926f;
	static const wchar_t*					REASON_NOT_FOUND					= L"Reason not found.";
}

namespace wildWeasel
{
	// used namespaces
	using namespace std;							// standard library

	// TODO: Currently a stepTimer implementation from DirectX Toolkit is used, but it should be replaced by an own one in future.
	using vector2							= DirectX::SimpleMath::Vector2;
	using vector3							= DirectX::SimpleMath::Vector3;
	using matrix							= DirectX::SimpleMath::Matrix;
	using quaternion						= DirectX::SimpleMath::Quaternion;

	// TODO: Currently a stepTimer implementation from DirectX Toolkit is used, but it should be replaced by an own one in future.
	using tickCounter						= DX::StepTimer;
	
	// pre definition of classes
	class masterMind;
	class realTextureItem;
	namespace ssf {
		class sharedVars;
		class textureResource;
		class text2D;
		namespace shape {
			class genericShape;
			class cylinder;
			class cube;
			class cone;
			class rect;
			class custom;
		}
		class sprite;
	}
	
	// enumeration constants
	enum class								errorCode							{ NOT_AN_ERROR, NOT_INITIALIZED, POINTER_IS_NULL, TOO_MUCH_POINTS, INVALID_PARAMETER_VAL };		// function return values
	enum class								guiElemState						{ DRAWED, HIDDEN, GRAYED, UNUSED, VISIBLE, INVISIBLE };											// DRAWED=active&visible, HIDDEN=deactivated&invisible, GRAYED=deactivated&grayed, UNUSED=not initialized, VISIBLE=deactivated&visible, INVISIBLE=active&invisible
	enum class								mouseButtonId						{ LEFT, RIGHT, MIDDLE };																		// 
	enum class								alignmentTypeX						{ BORDER_LEFT,   BORDER_RIGHT, FRACTION, FRACTION_WIDTH,  PIXEL_WIDTH,  USER };					// USER means that setTargetRect()-functions are used instead of setAlignment(). FRACTION_WIDTH and PIXEL_WIDTH is only used for right and bottom. For left and top they are equal to FRACTION and BORDER_LEFT.
	enum class								alignmentTypeY						{ BORDER_BOTTOM, BORDER_TOP,   FRACTION, FRACTION_HEIGHT, PIXEL_HEIGHT, USER };					//
	enum class								alignmentHorizontal					{ LEFT, CENTER, RIGHT, BLOCK };																	// for any sort of horizontal alignment
	enum class								alignmentVertical					{ TOP,  CENTER, BOTTOM, BELOW, ABOVE};															// for any sort of vertical   alignment
	enum class								cursorType							{ ARROW,  CROSS, HAND, HELP, IBEAM, NO, SIZEALL, SIZENESW, SIZENS, SIZENWSE, SIZEWE, UPARROW, WAIT };	//
	enum class								axisId								{ X = 0, Y = 1, Z = 2};																			// vector indices for the 3 axes

	// RECT with floats						
	class fRECT								
	{										
	public:									
		float								left;
		float								top;
		float								right;
		float								bottom;
											
											fRECT()														{ this->left = 0;		this->top = 0;		this->right = 0;		this->bottom = 0;		  }
											fRECT(float _left, float _top, float _right, float _bottom) { this->left = _left;	this->top = _top;	this->right = _right;	this->bottom = _bottom;   }
											fRECT(const fRECT& rc)										{ this->left = rc.left; this->top = rc.top; this->right = rc.right; this->bottom = rc.bottom; }
											fRECT(const  RECT& rc)										{ this->left = (float) rc.left; this->top = (float) rc.top; this->right = (float) rc.right; this->bottom = (float) rc.bottom; }
    										
		const RECT							getRECT								();
		inline float						height								()	{return bottom - top; };
		inline float						width								()	{return right  - left;};
		inline float						centerX								()	{return (right + left) * 0.5f; };
		inline float						centerY								()	{return (bottom + top) * 0.5f; };
											
	    fRECT&								operator=							(const  RECT& rc) { left = (float) rc.left; top = (float) rc.top; right = (float) rc.right; bottom = (float) rc.bottom; return *this; }
		fRECT&								operator=							(const fRECT& rc) { left =		   rc.left; top =         rc.top; right =         rc.right; bottom =         rc.bottom; return *this; }
	};

	// color with 4 floats
	class color
	{
	public: 
		float								r, g, b, a;
											
											
											color								()													{r = 0;			g = 0;			b = 0;			a = 1;  };
											color								(float _r, float _g, float _b, float _a)			{r = _r;		g = _g;			b = _b;			a = _a; };
											color								(BYTE _r, BYTE _g, BYTE _b)							{r = _r/255.0f; g = _g/255.0f;	b = _b/255.0f;	a = 1;  };
											
	    color&								operator=							(const COLORREF& c) { b = ((c & 0x00ff0000) >> 16) / 255.0f; g = ((c & 0x0000ff00) >> 8) / 255.0f; r = ((c & 0x000000ff) >> 0) / 255.0f; a = 1; return *this; }
		color&								operator=							(const color&    c) { r = c.r; g = c.g; b = c.b; a = c.a; return *this; }
		bool								operator==							(const color&    c) { return (r == c.r && g == c.g && b == c.b && a == c.a); }
											
		const COLORREF						getCOLORREF							();
		const color							rainbow								(float f);
		void								limitBetweenZeroAndOne				();
		bool								isSameAs							(const color&   c, float totalAllowedDelta);
											
		// Constants						
		static const color					green;
		static const color					lightGreen;
		static const color					red;
		static const color					blue;
		static const color					white;
		static const color					black;
		static const color					gray;
		static const color					lightBlue;
		static const color					darkGray;
	};

	// call back user function based on passed time
	// TODO: Currently a system specific implementation. Maybe a way using C++ time functions is possible.
	class timer
	{
	private:
		UINT_PTR							timerId								= 0;
		masterMind*							ww									= nullptr;
		void*								pUser								= nullptr;
		function<void(void*)>				timerFunc							= nullptr;							// user defined functions called on timer				
											
		static VOID CALLBACK				TimerProc							(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
											
	public:									
											timer								();
											~timer								();
											
		bool								isActive							();
		void								setFunc								(masterMind* ww, void timerFunc(void* pUser), void* pUser);
		void								start								(														   unsigned int milliseconds);
		void								start								(masterMind* ww, void timerFunc(void* pUser), void* pUser, unsigned int milliseconds);
		void								terminate							();
	};

	// call back user function based on triggered event
	// TODO: Currently a system specific implementation. Maybe a way using C++ thread functions is possible.
	class threadEvent
	{
	friend class masterMind;

	private:
		static list<threadEvent*>			allEvents;
		static void							process								();
											
		HANDLE								handle								= nullptr;
		masterMind*							ww									= nullptr;
		function<void(void*)>				eventFunc							= nullptr;							// user defined functions called on timer				
		void*								pUser								= nullptr;
											
	public:									
											~threadEvent						();
											
		void								create								(masterMind* ww);
		void								create								(masterMind* ww, bool manualReset, bool initialState);
		void								set									();
		void								reset								();
		bool								wait								(unsigned int timeOutInMilliseconds);
		void								setFunc								(function<void(void*)> eventFunc, void* pUser, bool callInMainLoop);
	};

	// classes can inherit this class, which provides functions beeing called when a certain event occurs
	class eventFollower
	{
	friend class masterMind;

		// eventFollower::lists								only contain elements, which shall be called in each case, no matter if focussed or not
		// guiElemEvFol::um_focusedButton					is called for each event
		// guiElemEvFol::um_focusedButton->motherFollowers	is called for each event and contains elements which wants to be called if a sub-element has focus

	private:
		static								list<eventFollower*>				list_keyDown;
		static								list<eventFollower*>				list_mouseMoved;
		static								list<eventFollower*>				list_verticalWheelMoved;
		static								list<eventFollower*>				list_horizontalWheelMoved;
		static								list<eventFollower*>				list_leftMouseButtonPressed;
		static								list<eventFollower*>				list_leftMouseButtonReleased;
		static								list<eventFollower*>				list_rightMouseButtonPressed;
		static								list<eventFollower*>				list_rightMouseButtonReleased;
		static								list<eventFollower*>				list_windowSizeChanged;
											
	public:									
		enum class							eventType							{ KEYDOWN, MOUSEMOVED, LEFT_MOUSEBUTTON_PRESSED, LEFT_MOUSEBUTTON_RELEASED, RIGHT_MOUSEBUTTON_PRESSED, RIGHT_MOUSEBUTTON_RELEASED, VERTICAL_WHEEL_MOVED, HORIZONTAL_WHEEL_MOVED, WINDOWSIZE_CHANGED};
											
		virtual								~eventFollower						();
											
		virtual void						keyDown								(int keyCode)			{};
		virtual void						mouseMoved							(int posX, int posY, const vector3& cursorPos, const vector3& cameraPos) {};
		virtual void						verticalWheelMoved					(int distance)			{};
		virtual void						horizontalWheelMoved				(int distance)			{};
		virtual void						leftMouseButtonPressed				(int xPos, int yPos)	{};
		virtual void						leftMouseButtonReleased				(int xPos, int yPos)	{};
		virtual void						rightMouseButtonPressed				(int xPos, int yPos)	{};
		virtual void						rightMouseButtonReleased			(int xPos, int yPos)	{};
		virtual void						windowSizeChanged					(int xSize, int ySize)	{};
											
		void								followEvent							(eventFollower* newFollower, eventType type);
		void								forgetEvent							(eventFollower* newFollower, eventType type);

		bool								anySubItemFocused					();
		bool								anySubItemSelected					();
	};

	// three indices of a triangle
	class triangleIndizes
	{
	public:
		size_t								i1, i2, i3;
	
											triangleIndizes						(size_t n1, size_t n2, size_t n3)	{ i1 = n1; i2 = n2; i3 = n3; }
	};

	// four indices of a rect
	class rectIndizes
	{
	public:
		size_t	i1, i2, i3, i4;

											rectIndizes							(size_t n1, size_t n2, size_t n3, size_t n4)	{ i1 = n1; i2 = n2; i3 = n3; i4 = n4; }
	};

	// a container of indexed rects in 3D space
	class indexedRectContainer
	{
	public:
		vector<vector3>						vertices;
		vector<vector3>						verticesTransformed;
		vector<rectIndizes>					rects;								// i1 is the vertex in the center
											
		void								sortIndices							()	{};
		bool								intersects							(const matrix& mat, const vector3& from, const vector3& to, float* distance, float* dotProduct);
		bool								intersectsInverse					(const matrix& mat, const vector3& from, const vector3& to, float* distance, float* dotProduct);
	};

	// class for loading, saving or storing in memory of bitmap images
	class bitmap
	{
	public:
		BITMAPFILEHEADER					bmfh; 
		BITMAPV4HEADER						bmih;
		BYTE*								pData								= nullptr;
		BYTE*								pBitmap								= nullptr;
											
											~bitmap								();
		size_t								getHeaderSizeInBytes				();
		size_t								getDataSizeInBytes					();
		size_t								getTotalSizeInBytes					();
		bool								writeToFile							(wstring &filename);
		bool								loadFromFile						(IWICImagingFactory* wicFactory, wstring &filename);
		bool								removeNastyPowerpointBorder			();
	};
	
	// a group of gui elements and clusters resulting in a whole menu
	class guiScenario
	{
	private:
		masterMind *						parent								= nullptr;
											
	public:									
		void								setPointerToParent					(masterMind *ww);
		bool								isActive							();
											
		virtual void						init								()	{};
		virtual void						activate							()	{};
		virtual void						deactivate							()	{};
		virtual void						release								()	{};
											
		virtual void						mouseMove							()	{};
	};

	// manages the guiScenario objects
	class scenarioManagerClass
	{
	friend class guiScenario;
	friend class masterMind;

	protected:
		guiScenario*						activeScenario						= nullptr;
		masterMind *						parent								= nullptr;
											
											scenarioManagerClass				(masterMind* ww)		{ parent = ww; };
											
	public:									
		bool								setActiveScenario					(guiScenario &newActiveScenario);
	};

	// this class contains general virtual functions every 3D object must have
	class genericObject3D
	{
	friend class masterMind;
	friend class resourceManager;

	private:
		struct gfx3D_spriteObject
		{
			genericObject3D*				object								= nullptr;		// 
			float							zPosition							= 0;			// 
			realTextureItem*				texture								= nullptr;		// ... the problem is that a genericObject3D::renderSprites() function can call draw functions for several textures !!!	 So either renderSprites() needs to be splitted for each texture or a spriteBatch-instance is used for each texture 

											gfx3D_spriteObject					(genericObject3D* newObject, float z, realTextureItem* tex)	{ object = newObject; zPosition = z; texture = tex; };
		};

		static list<genericObject3D*>		objectsToDraw;
		static list<genericObject3D*>		objectsToLoad;
		static list<gfx3D_spriteObject>		spritesToDraw;

	protected:
		bool								uploaded							= false;		// gets true if the performRessourceUpload() has been called
											
		virtual								~genericObject3D					();
											
		virtual void						update								(matrix& matView, tickCounter const& timer)	{};
		virtual void						render								(ssf::sharedVars& v)	{};
		virtual void						createDeviceDependentResources		(ssf::sharedVars& v)	{};
		virtual void						createWindowSizeDependentResources	(matrix& matProjection)	{};
		virtual void						onDeviceLost						()	{};
		virtual void						renderSprites						(ssf::sharedVars& v)	{};

		void								addSpriteToDraw						(float z, realTextureItem* tex);
		void								removeSpriteToDraw					();
		void								addObjectToLoad						(masterMind* ww);
		void								removeObjectToLoad					();
		void								addObjectToDraw						();
		void								removeObjectToDraw					();
		bool								isInListObjectsToDraw				();
		bool								isInListObjectsToLoad				();
	};

	// helper functions						
	void									determineRectangularBorder			(RECT &border, RGBQUAD* pQuad, UINT width, UINT height, BYTE alphaThreshold, RGBQUAD borderColor);	
	bool									performClippingAndCheck				(fRECT& destRect, fRECT& srcRect, const vector2& origin, const fRECT& clippingRect);
	bool									performClippingAndCheck				(fRECT& destRect, fRECT& srcRect, const vector2& origin, const fRECT& clippingRect, float rotation);
	bool									moveEdge							(float& v, float& p1, float& p2, const float& clipPositive, const float& clipNegative, float& f);
	bool									isGuiElemStateVisible				(guiElemState& state);
	bool									isGuiElemStateActive				(guiElemState& state);
	wstring									string2wstring						(const string& s);
	
} // namespace wildWeasel

// default functionality of wildWeasel
#include "wwSsf.h"
#include "ww3D.h"
#include "wwAlignment.h"
#include "wwGuiElement.h"
#include "wwMasterMind.h"
#include "wwAnimation.h"

// these header must be inlcuded manually/explicit by the user
// #include "wwRotCtrlCube.h"
// #include "wwListView.h"
// #include "wwTreeView.h"
// #include "wwEditField.h"
// #include "wwExamples.h"


