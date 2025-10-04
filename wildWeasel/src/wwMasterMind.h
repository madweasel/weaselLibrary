/*********************************************************************\
	wwMasterMind.h												  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/

#pragma once

#include "wildWeasel.h"

namespace wildWeasel 
{
	class masterMind
	{
	friend class ssf::graphicDevice;
	friend class guiElement;
	friend class realTextureItem;
	friend class texture;
	friend class loadingScreen;
	friend class cursorClass3D;
	friend class editField2D;
	friend class toolTipClass;
	friend class cubicButton;
	friend class sphericalButton;
	friend class textLabel3D;
	friend class triad;
	friend class plainButton;
	friend class plainButton2D;
	friend class textLabel2D;
	friend class checkBox2D;
	friend class borderLine2D;
	friend class camera;
	friend class resourceManager;
	friend class screenInformation;

	private:

		// winapi variables
		IWICImagingFactory *					wicFactory							= NULL;								// for reading .jpg and .png files
		HACCEL									hAccelTable							= NULL;								// table containt short-cuts
		HMENU									hMenu								= NULL;								// handle of the main menu
		HWND									hWnd								= NULL;								// handle of the main window
		HINSTANCE								hInst								= NULL;								// current instance
		TCHAR									szTitle		 [MAX_PATH];												// The title bar text
		TCHAR									szWindowClass[MAX_PATH];												// the main window class name
		WNDPROC									userWndProc							= nullptr;							// called by processMessageMember()
		void									(*drawUserStuff)(HDC hdc)			= nullptr;							// called by drawEverything()
											
		// rendering variables					
		BYTE									alphaThreshold						= 100;								// pixels with an alpha value smaller than this threshold will be completly transparent
		color									transparentColor					= color(1, 0, 1, 0);				// used as default value for DrawTranspBitmap()
		POINT									windowSize							= {  0,  0 };						// size in pixels of the window (Actually it is the visible area according to the windows function 'GetClientRect()'. The window itself is created with the size 'GetWindowRect()', which contains the window borders.)
		POINT									minWindowSize						= {  0,  0 };
		POINT									maxWindowSize						= { -1, -1 };
											
		// gui elements							
		list<guiElement*>						guiElements;															// container of all registered buttons
											
		// 3D stuff	
		camera*									activeCamera						= &camera::defaultCamera;
		wstring									texturesPath;															// default path under which images are searched and loaded. can be relative to the exe-file.
		texture									whiteDummyTexture;
		texture									backgroundTexture;
		color									backgroundColor						= color::black();
		ssf::graphicDevice						gfxDevice;
		ssf::audioDevice						sfxDevice;
		ssf::mouseDevice						mouseDevice;
		ssf::keyboardDevice						keyboardDevice;
		matrix									matView;
		matrix									matProjection;

		// winapi functions						
		bool									initCommonControls					();
		static LRESULT CALLBACK					processMessageStatic				(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
			   LRESULT CALLBACK					processMessageMember				(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
											
		// gui element functions				
		bool									registerGuiElement					(guiElement* newElem);				//
		bool									unregisterGuiElement				(guiElement* newElem);				//
		void									processGuiElements					();

		// main loop functions
		void									updateAllObjects					(tickCounter const& timer);

		// called by ssf::graphicDevice
		void									createDeviceDependentResources		();
		void									createWindowSizeDependentResources	();
		void									onDeviceLost						();
		void									onDeviceRestored					();									
		void									renderShapes						();
		void									renderSprites						();

		// events
		void									onActivated							();
		void									onDeactivated						();
		void									onSuspending						();
		void									onResuming							();
		void									onWindowMoved						();
		void									onWindowSizeChanged					(int width, int height);
		void									windowSizeChanged					(int xSize, int ySize);

		// user input							
		void									processKeyDown						(int keyCode);
		void									processMouseVerticalWheel			(int distance);
		void									processMouseHorizontalWheel			(int distance);
		void									processLeftButtonDown				(int xPos, int yPos);
		void									processLeftButtonUp					(int xPos, int yPos);
		void									processRightButtonDown				(int xPos, int yPos);
		void									processRightButtonUp				(int xPos, int yPos);
		void									processMouseMove					(int xPos, int yPos, unsigned int wParam);

	public:
											
		// gui stuff							
		cursorClass2D							cursor2D;
		cursorClass3D							cursor3D;
		scenarioManagerClass					scenarioManager;
		alignmentRoot							alignmentRootFrame;
		loadingScreen							mainLoadingScreen;
		resourceManager							graphicManager;
		screenInformation						screenInfo;
		tickCounter								stepTimer;

		// Constructor / destructor
												masterMind							(HINSTANCE hInstance, WNDPROC WndProc, UINT iTitle, UINT iWindowClass, UINT iIcon, LPWSTR iCursor, UINT iMenu, UINT iIconSmall, UINT iAccelerator);
												~masterMind							();

		// miscellaneous
		default_random_engine					randomEngine;															// for calling 
		locale									myLocale;																// format stringstreams like 1'000.000

		// core functions
		bool									goIntoMainLoop						();
		void									exitProgram							();
		inline int								getWindowSizeX						() { return windowSize.x; };
		inline int								getWindowSizeY						() { return windowSize.y; };
		inline void								setMinimumWindowSize				(int width, int height)	{ minWindowSize.x = width; minWindowSize.y = height; };
		inline void								setMaximumWindowSize				(int width, int height)	{ maxWindowSize.x = width; maxWindowSize.y = height; };
		static void								debugPrintWindowMessage				(UINT message);
		static void								calcControlPos						(unsigned int index,  RECT *rc, guiElemCoord *cc);
		static void								calcControlPos						(unsigned int index, fRECT *rc, guiElemCoord *cc);

		// winapi functions (to be removed one day)
		bool									createWindow						(int width, int height, int nCmdShow, DWORD dwStyle, void drawUserStuff(HDC hdc));
		inline HWND								getHwnd								() { return hWnd;};
		inline HINSTANCE						getHinst							() { return hInst;};
		inline HMENU							getHMenu							() { return hMenu;};

		// control creation & common dialogs
		bool									letUserSelectColor					(color &curColor);
		int										showMessageBox						(const wchar_t* strTitle, const wchar_t* strText, unsigned int type);
		void									openBrowser							(const wchar_t* strURL);

		// getter
		wstring									getTexturesPath						();
		inline matrix const&					getMatView							() { return matView;		};
		inline matrix const&					getMatProj							() { return matProjection;	};

		// images functions
		bool									setTexturesPath						(wstring &newPath);
		bool									setBackground						(wstring &filename);
		bool									setBackground						(color newColor);
		bool									setDummyWhiteTexture				(wstring &filePath);

		// folder & file functions
		bool									getOpenFileName						(wstring& filePath,						const wstring& fileTypeFilter = {L"All files\0*.*"}, const wstring& initialDirectory = L"", const wstring& dialogTitle = L"", const wstring& defaultExtension = L"", bool fileMustExist = true);
		bool									getOpenFileName						(wstring& path, vector<wstring>& files, const wstring& fileTypeFilter = {L"All files\0*.*"}, const wstring& initialDirectory = L"", const wstring& dialogTitle = L"", const wstring& defaultExtension = L"", bool fileMustExist = true, bool multiSelect = false);
		bool									getSaveFileName						(wstring& filePath,						const wstring& fileTypeFilter = {L"All files\0*.*"}, const wstring& initialDirectory = L"", const wstring& dialogTitle = L"", const wstring& defaultExtension = L"", bool dontAddToRecent = true);
		bool									browseForFolder						(wstring& folderPath, const wstring& windowTitle);
		bool									isFolderPathValid					(const wstring& folderPath);
		static wstring							makeFileTypeFilter					(const vector<pair<wstring,wstring>>& fileTypeFilter);
		static bool								checkPathAndModify					(wstring& strPath, wstring const& fallBackPath, bool useExeDirAsFallBack = true);
		static bool								makeRelativePathToExe				(wstring& fullPath, wstring& relativePath);
		static HRESULT							SHGetTargetFolderPath				(LPCITEMIDLIST pidlFolder, LPWSTR pszPath);
		static HRESULT 							SHGetTargetFolderIDList				(LPCITEMIDLIST pidlFolder, LPITEMIDLIST *ppidl);
		static HRESULT 							SHILClone							(LPCITEMIDLIST pidl, LPITEMIDLIST *ppidl);
		static HRESULT 							SHGetUIObjectFrFullPIDL				(LPCITEMIDLIST pidl, HWND hwnd, REFIID riid, void **ppv);

		// 3D drawing stuff
		void									setActiveCamera						(camera& newActiveCamera);
		camera*									getActiveCamera						();

		// debugging
		guiElement*								getGuiElementByName					(const wstring& text);
		wstring									whyIsGuiElemNotVisible				(const wstring& textOfGuiElement);
	};
}

