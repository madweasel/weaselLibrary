/*********************************************************************
	wwMasterMind.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/

#include "wwMasterMind.h"		

#pragma region constructor
//-----------------------------------------------------------------------------
// Name: wildWeasel()
// Desc: wildWeasel class constructor
//-----------------------------------------------------------------------------
wildWeasel::masterMind::masterMind(HINSTANCE hInstance, WNDPROC WndProc, UINT iTitle, UINT iWindowClass, UINT iIcon, LPWSTR iCursor, UINT iMenu, UINT iIconSmall, UINT iAccelerator) : 
	gfxDevice(this), sfxDevice(this), mouseDevice(this), keyboardDevice(this), mainLoadingScreen(this), scenarioManager(this), cursor2D(this), alignmentRootFrame(this->windowSize), graphicManager{this}, screenInfo{this}
{
	//
	#if (NTDDI_VERSION >= NTDDI_WINBLUE)
	SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);
	#endif

	// init vars
	szTitle		 [0]				= '\0';
	szWindowClass[0]				= '\0';
	hInst							= hInstance;
	hAccelTable						= LoadAccelerators(hInstance, MAKEINTRESOURCE(iAccelerator));
	userWndProc						= WndProc;
	
	// Initialize global strings
	LoadString(hInstance, iTitle,		szTitle,		MAX_PATH);
	LoadString(hInstance, iWindowClass, szWindowClass,	MAX_PATH);

	// RegisterClass()
	WNDCLASSEX wcex;
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= this->processMessageStatic;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(iIcon)); 
	wcex.hCursor		= NULL; // LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= 0;
	wcex.lpszMenuName	= MAKEINTRESOURCE(iMenu);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(hInstance,  MAKEINTRESOURCE(iIconSmall));
	RegisterClassEx(&wcex);

	// initialize rand()
	srand(GetTickCount());

	// Tausender-Trennzeichen
	myLocale = locale("German_Switzerland"); 
	
	// So common dialogs can be used
	initCommonControls();

	// Create WIC factory (Needs CoInitialize(NULL), but it is already called in initCommonControls())
	CoCreateInstance(CLSID_WICImagingFactory1,  NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicFactory));

	// set up default camera
	camera::defaultCamera.create(this);
}

//-----------------------------------------------------------------------------
// Name: ~masterMind()
// Desc: masterMind class destructor
//-----------------------------------------------------------------------------
wildWeasel::masterMind::~masterMind()
{
	// wait until resource upload has finished
	// while (mainLoadingScreen.isActive());
	// graphicManager.resourceUploadMutex.lock();

	for (auto& curElement : guiElements) {
		curElement->ww	= nullptr;
//		curElement->cluster = nulltr;
	}

	sfxDevice		.destroy();
	gfxDevice		.destroy();
	mouseDevice		.destroy();
	keyboardDevice	.destroy();
	
	// wicFactory
	// wicDecoder
	
	CoUninitialize();
}
#pragma endregion

#pragma region setter
//-----------------------------------------------------------------------------
// Name: setBackground()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::masterMind::setBackground(wstring &filename)
{
	if (!backgroundTexture.loadFile(this, filename)) return false;
	gfxDevice.setBackground(&backgroundTexture);
	return true;
}

//-----------------------------------------------------------------------------
// Name: setBackground()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::masterMind::setBackground(color newColor)
{
	backgroundColor = newColor;
	return true;
}

//-----------------------------------------------------------------------------
// Name: setDummyWhiteTexture()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::masterMind::setDummyWhiteTexture(wstring &filePath)
{
	return whiteDummyTexture.loadFile(this, filePath, false);
}

//-----------------------------------------------------------------------------
// Name: setTexturesPath()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::masterMind::setTexturesPath(wstring &newPath)
{
	// locals
	wstring possiblePath = newPath;

	if (checkPathAndModify(possiblePath, newPath)) {
		texturesPath = possiblePath;
		return true;
	} else {
		return false;
	}

}
#pragma endregion

#pragma region setter
//-----------------------------------------------------------------------------
// Name: setDummyWhiteTexture()
// Desc: 
//-----------------------------------------------------------------------------
wstring wildWeasel::masterMind::getTexturesPath()
{
	return texturesPath;
}
#pragma endregion

#pragma region main loop
//-----------------------------------------------------------------------------
// Name: goIntoMainLoop()
// Desc: calls GetMessage(), TranslateAccelerator(), TranslateMessage(), DispatchMessage()
//-----------------------------------------------------------------------------
bool wildWeasel::masterMind::goIntoMainLoop()
{
 	// locals
	MSG		msg	= { 0 };

	while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
			
			if (!mainLoadingScreen.isActive()) {
				graphicManager.uploadIfNecessary();
			}

			keyboardDevice.process();
			mouseDevice.process();

			stepTimer.Tick([&]() {
				updateAllObjects(stepTimer);
			});
			
			threadEvent::process();
			processGuiElements();
			sfxDevice.process();
			gfxDevice.process();
        }
    }

	return TRUE;
}

//-----------------------------------------------------------------------------
// Name: processMessageStatic()
// Desc: Processes messages for the main window.
//-----------------------------------------------------------------------------
LRESULT CALLBACK wildWeasel::masterMind::processMessageStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_NCCREATE) {
		SetWindowLongPtr(hWnd, (-21) /*GWL_USERDATA*/, (LONG_PTR) ((CREATESTRUCT*)lParam)->lpCreateParams);
	} else {
		masterMind*	ww = (masterMind*) GetWindowLongPtr(hWnd, (-21) /*GWL_USERDATA*/);
		if (ww) {
			return ww->processMessageMember(hWnd, message, wParam, lParam);
		}
	} 
	return DefWindowProc(hWnd, message, wParam, lParam);
}

//-----------------------------------------------------------------------------
// Name: processMessageMember()
// Desc: Processes messages for the main window.
//-----------------------------------------------------------------------------
LRESULT CALLBACK wildWeasel::masterMind::processMessageMember(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// locals
	guiElement*		curGuiElem		= NULL;
	NMBCHOTITEM*	pnmbchotitem	= (NMBCHOTITEM*) lParam;
	NMCUSTOMDRAW*	lpnmCD			= (NMCUSTOMDRAW*)lParam;
	int				xPos			= GET_X_LPARAM(lParam); 
	int				yPos			= GET_Y_LPARAM(lParam); 

	// static locals
    static bool		s_in_sizemove	= false;
    static bool		s_in_suspend	= false;
    static bool		s_minimized		= false;
    static bool		s_fullscreen	= false;

	// call owner windows message handler
	if (userWndProc && !mainLoadingScreen.isActive()) userWndProc(hWnd, message, wParam, lParam);
	
	// debugPrintWindowMessage(Message);

	switch (message)
	{
	case WM_MOUSEWHEEL:
		processMouseVerticalWheel((int) GET_WHEEL_DELTA_WPARAM(wParam));
		break;

	case WM_MOUSEHWHEEL:
		processMouseHorizontalWheel((int) GET_WHEEL_DELTA_WPARAM(wParam));		
		break;

	case WM_LBUTTONDOWN:
		processLeftButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;

	case WM_RBUTTONDOWN:
        processRightButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        break;

	case WM_LBUTTONUP:
		processLeftButtonUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;

	case WM_RBUTTONUP:
		processRightButtonUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;	
	
	case WM_MOUSEMOVE:
		processMouseMove((int) GET_X_LPARAM(lParam), (int) GET_Y_LPARAM(lParam), (unsigned int) wParam);
		break;

	case WM_KEYDOWN:
		processKeyDown((unsigned int) wParam);
		break;

	case WM_CREATE:
        break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) 
		{
		// Custem draw: WORKS FINE: Shall catch the mouse hover and leave events 
		case BCN_HOTITEMCHANGE:
			break;

		 // Custem draw: WORKS FINE:
		 case NM_CUSTOMDRAW:
			break;
		}
		break;

	 case WM_DEVICECHANGE:
        switch (wParam)
        {
        case DBT_DEVICEARRIVAL:
			{
				auto pDev = reinterpret_cast<PDEV_BROADCAST_HDR>(lParam);
				if (pDev) {
					if (pDev->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) {
						//auto pInter = reinterpret_cast<const PDEV_BROADCAST_DEVICEINTERFACE>(pDev);
						//if (pInter->dbcc_classguid == KSCATEGORY_AUDIO) {
						// 	if (sfxDevice.initialized) sfxDevice.onNewAudioDevice();
						// }
					}
				}
			}
 			break;
        case DBT_DEVICEREMOVECOMPLETE:
			{
				auto pDev = reinterpret_cast<PDEV_BROADCAST_HDR>(lParam);
				if (pDev) {
					if (pDev->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) {
						// auto pInter = reinterpret_cast<const PDEV_BROADCAST_DEVICEINTERFACE>(pDev);
						// if (pInter->dbcc_classguid == KSCATEGORY_AUDIO) {
						// 	if (sfxDevice.initialized) sfxDevice.onNewAudioDevice();
						// }
					}
				}
				break;
			}
			break;
		}
		break;

	case WM_GETMINMAXINFO:
	{
		LPMINMAXINFO lpMMI = reinterpret_cast<MINMAXINFO*>(lParam);
		lpMMI->ptMinTrackSize.x = minWindowSize.x;
		lpMMI->ptMinTrackSize.y = minWindowSize.y;
		if (maxWindowSize.x > minWindowSize.x && maxWindowSize.y > minWindowSize.y) {
			lpMMI->ptMaxTrackSize.x = maxWindowSize.x;
			lpMMI->ptMaxTrackSize.y = maxWindowSize.y;
		}
		break;
	}
    case WM_MOVE:
        onWindowMoved();
        break;

    case WM_SIZE:
		// calc windowSize and camera stuff
		{
			windowSize.x						= LOWORD(lParam);
			windowSize.y						= HIWORD(lParam);
			windowSizeChanged(windowSize.x, windowSize.y);
		}
		if (wParam == SIZE_MINIMIZED) {
            if (!s_minimized) {
                s_minimized = true;
                if (!s_in_suspend) onSuspending();
                s_in_suspend = true;
            }
        } else if (s_minimized) {
            s_minimized = false;
            if (s_in_suspend) onResuming();
            s_in_suspend = false;
        } else if (!s_in_sizemove) {
            onWindowSizeChanged(windowSize.x, windowSize.y);
        }
        break;

    case WM_ENTERSIZEMOVE:
        s_in_sizemove = true;
        break;

    case WM_EXITSIZEMOVE:
        s_in_sizemove = false;
        RECT rc;
        GetClientRect(hWnd, &rc);
        onWindowSizeChanged(rc.right - rc.left, rc.bottom - rc.top);
        break;

    case WM_ACTIVATEAPP:
            if (wParam) {
				onActivated();
            } else {
                onDeactivated();
            }
        break;

    case WM_POWERBROADCAST:
        switch (wParam)
        {
        case PBT_APMQUERYSUSPEND:
            if (!s_in_suspend) onSuspending();
            s_in_suspend = true;
            return true;

        case PBT_APMRESUMESUSPEND:
            if (!s_minimized) {
                if (s_in_suspend) onResuming();
                s_in_suspend = false;
            }
            return true;
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

	case WM_CLOSE:
		PostQuitMessage(0);
		break;

    case WM_SYSKEYDOWN:
        if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000) {
            // Implements the classic ALT+ENTER fullscreen toggle
            if (s_fullscreen) {
                SetWindowLongPtr(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
                SetWindowLongPtr(hWnd, GWL_EXSTYLE, 0);
                int width = 800;
                int height = 600;
                ShowWindow(hWnd, SW_SHOWNORMAL);
                SetWindowPos(hWnd, HWND_TOP, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
            } else {
                SetWindowLongPtr(hWnd, GWL_STYLE, 0);
                SetWindowLongPtr(hWnd, GWL_EXSTYLE, WS_EX_TOPMOST);
                SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
                ShowWindow(hWnd, SW_SHOWMAXIMIZED);
            }
            s_fullscreen = !s_fullscreen;
        }
        break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

//-----------------------------------------------------------------------------
// Name: updateAllObjects()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::masterMind::updateAllObjects(tickCounter const& timer)
{
	// view & world matrix
	activeCamera->makeViewMatrixFromCamera(matView);

	// process all objects
	for (auto& curObject : genericObject3D::objectsToDraw) {
		if ( mainLoadingScreen.isActive() && curObject != &mainLoadingScreen) continue;
		if (!mainLoadingScreen.isActive() && curObject == &mainLoadingScreen) continue;
		curObject->update(matView, timer);
	}
}
#pragma endregion

#pragma region called by ssf::graphicDevice
//-----------------------------------------------------------------------------
// Name: renderShapes()
// Desc: called by sfx::graphicDevice
//-----------------------------------------------------------------------------
void wildWeasel::masterMind::renderShapes()
{
	// process all objects
	for (auto& curObject : genericObject3D::objectsToDraw) {
		if ( mainLoadingScreen.isActive() && curObject != &mainLoadingScreen) continue;
		if (!mainLoadingScreen.isActive() && curObject == &mainLoadingScreen) continue;
		curObject->render(gfxDevice.sharedVars);
	}
}

//-----------------------------------------------------------------------------
// Name: renderSprites()
// Desc: called by sfx::graphicDevice
//-----------------------------------------------------------------------------
void wildWeasel::masterMind::renderSprites()
{
	// process all objects
	for (auto& curObject : genericObject3D::spritesToDraw) {
		if ( mainLoadingScreen.isActive() && curObject.object != &mainLoadingScreen) continue;
		if (!mainLoadingScreen.isActive() && curObject.object == &mainLoadingScreen) continue;
		if (!curObject.object->uploaded)											 continue;
		curObject.object->renderSprites(gfxDevice.sharedVars);
	}
}

//-----------------------------------------------------------------------------
// Name: createDeviceDependentResources()
// Desc: called by sfx::graphicDevice
//-----------------------------------------------------------------------------
void wildWeasel::masterMind::createDeviceDependentResources()
{
	// process all objects
	for (auto& curObject : genericObject3D::objectsToDraw) {
		curObject->createDeviceDependentResources(gfxDevice.sharedVars);
	}
}

//-----------------------------------------------------------------------------
// Name: createWindowSizeDependentResources()
// Desc: called by sfx::graphicDevice
//-----------------------------------------------------------------------------
void wildWeasel::masterMind::createWindowSizeDependentResources()
{
    // This sample makes use of a right-handed coordinate system using row-major matrices.
    getActiveCamera()->makeProjMatrixFromCamera(matProjection);

	// process all objects
	for (auto& curObject : genericObject3D::objectsToDraw) {
		curObject->createWindowSizeDependentResources(matProjection);
	}

	gfxDevice.setBackground(&backgroundTexture);
}

//-----------------------------------------------------------------------------
// Name: onDeviceLost()
// Desc: called by sfx::graphicDevice
//-----------------------------------------------------------------------------
void wildWeasel::masterMind::onDeviceLost()
{
	// process all objects
	for (auto& curObject : genericObject3D::objectsToDraw) {
		curObject->onDeviceLost();
	}
}

//-----------------------------------------------------------------------------
// Name: onDeviceRestored()
// Desc: called by sfx::graphicDevice
//-----------------------------------------------------------------------------
void wildWeasel::masterMind::onDeviceRestored()
{
}
#pragma endregion

#pragma region events
//-----------------------------------------------------------------------------
// Name: onActivated()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::masterMind::onActivated()
{
	sfxDevice		.onActivated();
	gfxDevice		.onActivated();
	mouseDevice		.onActivated();
	keyboardDevice	.onActivated();
}

//-----------------------------------------------------------------------------
// Name: onDeactivated()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::masterMind::onDeactivated()
{
	sfxDevice		.onDeactivated();
	gfxDevice		.onDeactivated();
	mouseDevice		.onDeactivated();
	keyboardDevice	.onDeactivated();
}

//-----------------------------------------------------------------------------
// Name: onSuspending()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::masterMind::onSuspending()
{
	sfxDevice		.onSuspending();
	gfxDevice		.onSuspending();
	mouseDevice		.onSuspending();
	keyboardDevice	.onSuspending();
}

//-----------------------------------------------------------------------------
// Name: onResuming()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::masterMind::onResuming()
{
	stepTimer.ResetElapsedTime();
	sfxDevice		.onResuming();
	gfxDevice		.onResuming();
	mouseDevice		.onResuming();
	keyboardDevice	.onResuming();
}

//-----------------------------------------------------------------------------
// Name: onWindowMoved()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::masterMind::onWindowMoved()
{
	sfxDevice		.onWindowMoved();
	gfxDevice		.onWindowMoved();
	mouseDevice		.onWindowMoved();
	keyboardDevice	.onWindowMoved();
}

//-----------------------------------------------------------------------------
// Name: onWindowSizeChanged()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::masterMind::onWindowSizeChanged(int width, int height)
{
	sfxDevice		.onWindowSizeChanged(width, height);
	gfxDevice		.onWindowSizeChanged(width, height);
	mouseDevice		.onWindowSizeChanged(width, height);
	keyboardDevice	.onWindowSizeChanged(width, height);
}
#pragma endregion

#pragma region folder & file functions

//-----------------------------------------------------------------------------
// Name: makeFileTypeFilter()
// Desc: 
//-----------------------------------------------------------------------------
wstring wildWeasel::masterMind::makeFileTypeFilter(const vector<pair<wstring, wstring>>& fileTypeFilter)
{
	wstring strFileTypeFilter;

	// make file type filter string
	for (auto curFilter : fileTypeFilter) {
		strFileTypeFilter += curFilter.first;
		strFileTypeFilter += L'\0';
		strFileTypeFilter += curFilter.second;
		strFileTypeFilter += L'\0';
	}
	strFileTypeFilter += L'\0';

	return strFileTypeFilter;
}

//-----------------------------------------------------------------------------
// Name: isFolderPathValid()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::masterMind::isFolderPathValid(const wstring& folderPath)
{
	// ... What is the difference to "return PathIsDirectory(folderPath.c_str());" ?

	DWORD fileAttrib;

	fileAttrib = GetFileAttributes(folderPath.c_str());	

	if (fileAttrib != INVALID_FILE_ATTRIBUTES && fileAttrib & FILE_ATTRIBUTE_DIRECTORY) {
		return true;
	} else {
		return false;
	}
}

//-----------------------------------------------------------------------------
// Name: browseForFolder()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::masterMind::browseForFolder(wstring& folderPath, const wstring& windowTitle)
{
	// locals
	BROWSEINFO				brInfo;
	TCHAR					displayName[MAX_PATH];
	TCHAR					newFolder[MAX_PATH];
	PIDLIST_ABSOLUTE		idListFolder;
	
	ZeroMemory(&brInfo, sizeof(brInfo));
	brInfo.hwndOwner		= hWnd;
	brInfo.pidlRoot			= NULL;
	brInfo.pszDisplayName	= displayName;
	brInfo.lpszTitle		= windowTitle.c_str();
	brInfo.ulFlags			= BIF_USENEWUI;
	brInfo.lpfn				= NULL;
	brInfo.lParam			= 0;
	brInfo.iImage			= 0;
			
	// show dialog, so that user can select a folder
	idListFolder = SHBrowseForFolder(&brInfo);

	if (idListFolder != NULL) {
		SHGetTargetFolderPath(idListFolder, newFolder);
		folderPath.assign(newFolder);
		return true;
	} else {
		return false;
	}
}

//-----------------------------------------------------------------------------
// Name: getOpenFileName()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::masterMind::getOpenFileName(wstring& filePath, const wstring& strFileTypeFilter, const wstring& initialDirectory, const wstring& dialogTitle, const wstring& defaultExtension, bool fileMustExist)
{
	wstring			path;
	vector<wstring> files;
	bool			result = getOpenFileName(path, files, strFileTypeFilter, initialDirectory, dialogTitle, defaultExtension, fileMustExist, false);

	filePath.clear();
	if (result) {
		filePath = path + files.front();
	}

	return result;
}

//-----------------------------------------------------------------------------
// Name: getOpenFileName()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::masterMind::getOpenFileName(wstring& path, vector<wstring>& files, const wstring& strFileTypeFilter, const wstring& initialDirectory, const wstring& dialogTitle, const wstring& defaultExtension, bool fileMustExist, bool multiSelect)
{
	// locals
	OPENFILENAME	ofn;
	vector<wchar_t>	vecPathName(512);
	
	path .clear();
	files.clear();

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize			= sizeof(OPENFILENAME);
	ofn.hwndOwner			= getHwnd();
	ofn.hInstance			= getHinst();
	ofn.lpstrFilter			= strFileTypeFilter.c_str();
	ofn.nFilterIndex		= 1;
	ofn.lpstrFile			= &vecPathName[0];
	ofn.nMaxFile			= (DWORD) (vecPathName.size()-1);
	ofn.lpstrInitialDir		= initialDirectory.c_str();
	ofn.lpstrTitle			= dialogTitle.c_str();
	ofn.Flags				= OFN_EXPLORER | (fileMustExist ? OFN_FILEMUSTEXIST : 0) | (multiSelect ? OFN_ALLOWMULTISELECT : 0);
	ofn.lpstrDefExt			= defaultExtension.c_str();

	// open dialog
	if (GetOpenFileName(&ofn)) {
		if (multiSelect) {
			auto	pNullPos = find(vecPathName.begin(), vecPathName.end(), L'\0');
			path.assign(vecPathName.begin(), pNullPos);
			while (pNullPos < vecPathName.end() && *(pNullPos+1) != L'\0') {
				pNullPos++;
				files.push_back(wstring(&(*pNullPos)));
				pNullPos = find(pNullPos, vecPathName.end(), L'\0');
			}
		} else {
			files.push_back(wstring(&vecPathName[0]));
		} 
		return true;
	} else {
		return false;
	}
}

//-----------------------------------------------------------------------------
// Name: getOpenFileName()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::masterMind::getSaveFileName(wstring& filePath, const wstring& fileTypeFilter, const wstring& initialDirectory, const wstring& dialogTitle, const wstring& defaultExtension, bool dontAddToRecent)
{
	// locals
	OPENFILENAME	ofn;
	vector<wchar_t>	vecPathName(512);
	
	filePath .clear();

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize			= sizeof(OPENFILENAME);
	ofn.hwndOwner			= getHwnd();
	ofn.hInstance			= getHinst();
	ofn.lpstrFilter			= fileTypeFilter.c_str();
	ofn.nFilterIndex		= 1;
	ofn.lpstrFile			= &vecPathName[0];
	ofn.nMaxFile			= (DWORD) (vecPathName.size()-1);
	ofn.lpstrInitialDir		= initialDirectory.c_str();
	ofn.lpstrTitle			= dialogTitle.c_str();
	ofn.Flags				= OFN_EXPLORER | (dontAddToRecent ? OFN_DONTADDTORECENT : 0);
	ofn.lpstrDefExt			= defaultExtension.c_str();

	// open dialog
	if (GetSaveFileName(&ofn)) {
		// replace wrong extension
		if (ofn.Flags & OFN_EXTENSIONDIFFERENT) {
			copy(defaultExtension.begin(), defaultExtension.end(), vecPathName.begin() + ofn.nFileExtension);
		}
		filePath.assign(&vecPathName[0]);
		return true;
	} else {
		return false;
	}
}

//-----------------------------------------------------------------------------
// Name: checkPathAndModify()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::masterMind::checkPathAndModify(wstring& strPath, wstring const& fallBackPath, bool useExeDirAsFallBack)
{
	// locals
	WCHAR strTmp[MAX_PATH];

	// current path ok?
	if (strPath.length() == 0 || PathIsDirectory(strPath.c_str()) == FALSE) {

		// if not try fallback path
		GetModuleFileName(NULL, strTmp, MAX_PATH);
		PathRemoveFileSpec(strTmp);
		strPath.assign(strTmp);
		strPath.append(L"\\");
		strPath.append(fallBackPath);
	
		// if still not ok try exe directory
		if (PathIsDirectory(strPath.c_str()) == FALSE) {
			if (useExeDirAsFallBack) {
				strPath.assign(strTmp);
			} else {
				CreateDirectory(strPath.c_str(), NULL);
			}
		}
	}
	return true;
}

//-----------------------------------------------------------------------------
// Name: makeRelativePathToExe()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::masterMind::makeRelativePathToExe(wstring& fullPath, wstring& relativePath)
{
	// locals
	WCHAR strExeDir  [MAX_PATH];
	WCHAR strRelative[MAX_PATH];

	GetModuleFileName(NULL, strExeDir, MAX_PATH);
	PathRemoveFileSpec(strExeDir);
	
	// is already relative or a different drive?
	if (fullPath.at(0) == '\\' || fullPath.at(0) == '.' || fullPath.at(0) != strExeDir[0])	{
		relativePath = fullPath;
	// make relative
	} else {
		PathRelativePathTo(strRelative, strExeDir, FILE_ATTRIBUTE_DIRECTORY, fullPath.c_str(), FILE_ATTRIBUTE_DIRECTORY);
		relativePath.assign(strRelative);
	}

	return true;
}

//-----------------------------------------------------------------------------
// Name: SHGetUIObjectFrFullPIDL()
// Desc: Retrieves the UIObject interface for the specified full PIDL
//-----------------------------------------------------------------------------
HRESULT wildWeasel::masterMind::SHGetUIObjectFrFullPIDL(LPCITEMIDLIST pidl, HWND hwnd, REFIID riid, void **ppv)
{    
	LPCITEMIDLIST pidlChild;
	IShellFolder* psf;
	*ppv = NULL;
	HRESULT hr = SHBindToParent(pidl, IID_IShellFolder, (LPVOID*)&psf, &pidlChild);
	if (SUCCEEDED(hr))    {
		hr = psf->GetUIObjectOf(hwnd, 1, &pidlChild, riid, NULL, ppv);        
		psf->Release();    
	}    
	return hr;
}

//-----------------------------------------------------------------------------
// Name: SHILClone()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT wildWeasel::masterMind::SHILClone(LPCITEMIDLIST pidl, LPITEMIDLIST *ppidl)
{    
	DWORD cbTotal = 0;
	if (pidl)    {
		LPCITEMIDLIST pidl_temp = pidl;
		cbTotal += pidl_temp->mkid.cb;
		while (pidl_temp->mkid.cb)         {
			cbTotal += pidl_temp->mkid.cb;            
			pidl_temp = ILNext(pidl_temp);
		}    
	}        
	*ppidl = (LPITEMIDLIST)CoTaskMemAlloc(cbTotal);
	if (*ppidl)        CopyMemory(*ppidl, pidl, cbTotal);
	return  *ppidl ? S_OK: E_OUTOFMEMORY;
}

//-----------------------------------------------------------------------------
// Name: SHGetTargetFolderIDList()
// Desc: Get the target PIDL for a folder PIDL. This also deals with cases of a folder  
//		 shortcut or an alias to a real folder.
//-----------------------------------------------------------------------------
HRESULT wildWeasel::masterMind::SHGetTargetFolderIDList(LPCITEMIDLIST pidlFolder, LPITEMIDLIST *ppidl)	
{    
	IShellLink *psl;
	*ppidl = NULL;
	HRESULT hr = SHGetUIObjectFrFullPIDL(pidlFolder, NULL, IID_IShellLink, (LPVOID*)&psl);
	if (SUCCEEDED(hr))    {
		hr = psl->GetIDList(ppidl);        
		psl->Release();
	}        
	// It's not a folder shortcut so get the PIDL normally.    
	if (FAILED(hr))        hr = SHILClone(pidlFolder, ppidl);
	return hr;
}

//-----------------------------------------------------------------------------
// Name: SHGetTargetFolderPath()
// Desc: Get the target folder for a folder PIDL. This deals with cases where a folder
//		 is an alias to a real folder, folder shortcuts, the My Documents folder, etc.		 
//-----------------------------------------------------------------------------
HRESULT wildWeasel::masterMind::SHGetTargetFolderPath(LPCITEMIDLIST pidlFolder, LPWSTR pszPath)
{
	LPITEMIDLIST pidlTarget;
	*pszPath = 0;
	HRESULT hr = SHGetTargetFolderIDList(pidlFolder, &pidlTarget);
	if (SUCCEEDED(hr))    {
		// Make sure it is a path
		SHGetPathFromIDListW(pidlTarget, pszPath);
		CoTaskMemFree(pidlTarget);    
	}       
	return *pszPath ? S_OK : E_FAIL;
}
#pragma endregion

#pragma region noname
//-----------------------------------------------------------------------------
// Name: createWindow()
// Desc: calls CreateWindow(), ShowWindow(), UpdateWindow(), CoInitialize() and InitCommonControlsEx()
//-----------------------------------------------------------------------------
bool wildWeasel::masterMind::createWindow(int width, int height, int nCmdShow, DWORD dwStyle, void drawUserStuff(HDC hdc))
{
	// yet, only one single window can be created
	if (hWnd) return FALSE;

	// assign function pointer
	this->drawUserStuff	= drawUserStuff;

	// CreateWindow()
	hWnd = CreateWindow(szWindowClass, szTitle, dwStyle != 0 ? dwStyle : WS_OVERLAPPED | WS_SIZEBOX | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, CW_USEDEFAULT, 0, width, height, NULL, NULL, hInst, this);

	// Actually the parameters width and height shall be equal to the variable 'windowsSize'. Both are equal to the visible area according to the windows function 'GetClientRect()'. 
	// The window itself is created with the size 'GetWindowRect()', which contains the window borders. Thus it must be resized here.
	RECT rcClient, rcWind;
	POINT ptDiff;
	GetClientRect(hWnd, &rcClient);
	GetWindowRect(hWnd, &rcWind);
	ptDiff.x = (rcWind.right - rcWind.left) - rcClient.right;
	ptDiff.y = (rcWind.bottom - rcWind.top) - rcClient.bottom;
	MoveWindow(hWnd,rcWind.left, rcWind.top, width + ptDiff.x, height + ptDiff.y, TRUE);
	
	if (!hWnd) {
	   return false;
	}

	hMenu = GetMenu(hWnd);

	if (!hMenu) {
	   return false;
	}

	// set default cursor
	cursor2D.setCursor(cursorType::ARROW);

	// init devices
	if (!gfxDevice.init())			{	return false;	}
	if (!sfxDevice.init())			{	return false;	}
	if (!mouseDevice.init())		{	return false;	}
	if (!keyboardDevice.init())		{	return false;	}
	
	// loading screen
	mainLoadingScreen.init(new ssf::shape::cube(), windowSize);
	mainLoadingScreen.addObjectToLoad(this);
	mainLoadingScreen.addSpriteToDraw(0, nullptr);

	return true;
}

//-----------------------------------------------------------------------------
// Name: calcControlPos()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::masterMind::calcControlPos(unsigned int index, RECT *rc, guiElemCoord *cc)
{
	rc->left	= cc->xPos + (index % cc->periodicity) * (cc->xDist + cc->width );
	rc->top		= cc->yPos + (index / cc->periodicity) * (cc->yDist + cc->height);
	rc->right	= cc->width;
	rc->bottom	= cc->height;
}

//-----------------------------------------------------------------------------
// Name: calcControlPos()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::masterMind::calcControlPos(unsigned int index, fRECT *rc, guiElemCoord *cc)
{
	rc->left	= (float) cc->xPos + (index % cc->periodicity) * (cc->xDist + cc->width );
	rc->top		= (float) cc->yPos + (index / cc->periodicity) * (cc->yDist + cc->height);
	rc->right	= (float) cc->width;
	rc->bottom	= (float) cc->height;
}

#pragma endregion

#pragma region commonDialogs
//-----------------------------------------------------------------------------
// Name: initCommonControls()
// Desc: creates main window
//-----------------------------------------------------------------------------
bool wildWeasel::masterMind::initCommonControls()
{
	// Single Thread (does not work together with initialize(RO_INIT_MULTITHREADED))
	if (CoInitialize(NULL) != S_OK) {
		return false;
	}

	// Common Controls
	INITCOMMONCONTROLSEX icc = { 0 };
	icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icc);

	return true;
}

//-----------------------------------------------------------------------------
// Name: letUserSelectColor()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::masterMind::letUserSelectColor(color &curColor)
{
	// locals
	CHOOSECOLOR cc;                 // common dialog box structure 
	COLORREF	acrCustClr[16];		// array of custom colors 
	COLORREF	rgbCurrent	= curColor.getCOLORREF();

	// Initialize CHOOSECOLOR 
	ZeroMemory(&cc, sizeof(cc));
	cc.lStructSize	= sizeof(cc);
	cc.hwndOwner	= hWnd;
	cc.lpCustColors = (LPDWORD) acrCustClr;
	cc.rgbResult	= rgbCurrent;
	cc.Flags		= CC_FULLOPEN | CC_RGBINIT;
	 
	if (ChooseColor(&cc)==TRUE) {
		rgbCurrent	= cc.rgbResult; 
		curColor	= rgbCurrent;
		return true;
	} else {
		return false;
	}
}

//-----------------------------------------------------------------------------
// Name: showMessageBox()
// Desc: 
//-----------------------------------------------------------------------------
int wildWeasel::masterMind::showMessageBox(const wchar_t* strTitle, const wchar_t* strText, unsigned int type)
{
	return MessageBox(hWnd, strText, strTitle, type);
}

//-----------------------------------------------------------------------------
// Name: openBrowser()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::masterMind::openBrowser(const wchar_t* strURL)
{
	ShellExecute(0, 0, strURL, 0, 0 , SW_SHOW);
}

//-----------------------------------------------------------------------------
// Name: exitProgram()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::masterMind::exitProgram()
{
	PostQuitMessage(0);
}

#pragma endregion

#pragma region debugging
//-----------------------------------------------------------------------------
// Name: debugPrintWindowMessages()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::masterMind::debugPrintWindowMessage(UINT message)
{
	// locals
	switch (message)
	{
	case 0x0000: OutputDebugStringW(L"WM_NULL                         \n");		break;
	case 0x0001: OutputDebugStringW(L"WM_CREATE                       \n");		break;
	case 0x0002: OutputDebugStringW(L"WM_DESTROY                      \n");		break;
	case 0x0003: OutputDebugStringW(L"WM_MOVE                         \n");		break;
	case 0x0005: OutputDebugStringW(L"WM_SIZE                         \n");		break;
	case 0x0006: OutputDebugStringW(L"WM_ACTIVATE                     \n");		break;
	case 0x0007: OutputDebugStringW(L"WM_SETFOCUS                     \n");		break;
	case 0x0008: OutputDebugStringW(L"WM_KILLFOCUS                    \n");		break;
	case 0x000A: OutputDebugStringW(L"WM_ENABLE                       \n");		break;
	case 0x000B: OutputDebugStringW(L"WM_SETREDRAW                    \n");		break;
	case 0x000C: OutputDebugStringW(L"WM_SETTEXT                      \n");		break;
	case 0x000D: OutputDebugStringW(L"WM_GETTEXT                      \n");		break;
	case 0x000E: OutputDebugStringW(L"WM_GETTEXTLENGTH                \n");		break;
	case 0x000F: OutputDebugStringW(L"WM_PAINT                        \n");		break;
	case 0x0010: OutputDebugStringW(L"WM_CLOSE                        \n");		break;
	case 0x0011: OutputDebugStringW(L"WM_QUERYENDSESSION              \n");		break;
	case 0x0013: OutputDebugStringW(L"WM_QUERYOPEN                    \n");		break;
	case 0x0016: OutputDebugStringW(L"WM_ENDSESSION                   \n");		break;
	case 0x0012: OutputDebugStringW(L"WM_QUIT                         \n");		break;
	case 0x0014: OutputDebugStringW(L"WM_ERASEBKGND                   \n");		break;
	case 0x0015: OutputDebugStringW(L"WM_SYSCOLORCHANGE               \n");		break;
	case 0x0018: OutputDebugStringW(L"WM_SHOWWINDOW                   \n");		break;
	case 0x001A: OutputDebugStringW(L"WM_WININICHANGE                 \n");		break;
	case 0x001B: OutputDebugStringW(L"WM_DEVMODECHANGE                \n");		break;
	case 0x001C: OutputDebugStringW(L"WM_ACTIVATEAPP                  \n");		break;
	case 0x001D: OutputDebugStringW(L"WM_FONTCHANGE                   \n");		break;
	case 0x001E: OutputDebugStringW(L"WM_TIMECHANGE                   \n");		break;
	case 0x001F: OutputDebugStringW(L"WM_CANCELMODE                   \n");		break;
	case 0x0020: OutputDebugStringW(L"WM_SETCURSOR                    \n");		break;
	case 0x0021: OutputDebugStringW(L"WM_MOUSEACTIVATE                \n");		break;
	case 0x0022: OutputDebugStringW(L"WM_CHILDACTIVATE                \n");		break;
	case 0x0023: OutputDebugStringW(L"WM_QUEUESYNC                    \n");		break;
	case 0x0024: OutputDebugStringW(L"WM_GETMINMAXINFO                \n");		break;
	case 0x0026: OutputDebugStringW(L"WM_PAINTICON                    \n");		break;
	case 0x0027: OutputDebugStringW(L"WM_ICONERASEBKGND               \n");		break;
	case 0x0028: OutputDebugStringW(L"WM_NEXTDLGCTL                   \n");		break;
	case 0x002A: OutputDebugStringW(L"WM_SPOOLERSTATUS                \n");		break;
	case 0x002B: OutputDebugStringW(L"WM_DRAWITEM                     \n");		break;
	case 0x002C: OutputDebugStringW(L"WM_MEASUREITEM                  \n");		break;
	case 0x002D: OutputDebugStringW(L"WM_DELETEITEM                   \n");		break;
	case 0x002E: OutputDebugStringW(L"WM_VKEYTOITEM                   \n");		break;
	case 0x002F: OutputDebugStringW(L"WM_CHARTOITEM                   \n");		break;
	case 0x0030: OutputDebugStringW(L"WM_SETFONT                      \n");		break;
	case 0x0031: OutputDebugStringW(L"WM_GETFONT                      \n");		break;
	case 0x0032: OutputDebugStringW(L"WM_SETHOTKEY                    \n");		break;
	case 0x0033: OutputDebugStringW(L"WM_GETHOTKEY                    \n");		break;
	case 0x0037: OutputDebugStringW(L"WM_QUERYDRAGICON                \n");		break;
	case 0x0039: OutputDebugStringW(L"WM_COMPAREITEM                  \n");		break;
	case 0x003D: OutputDebugStringW(L"WM_GETOBJECT                    \n");		break;
	case 0x0041: OutputDebugStringW(L"WM_COMPACTING                   \n");		break;
	case 0x0044: OutputDebugStringW(L"WM_COMMNOTIFY                   \n");		break;
	case 0x0046: OutputDebugStringW(L"WM_WINDOWPOSCHANGING            \n");		break;
	case 0x0047: OutputDebugStringW(L"WM_WINDOWPOSCHANGED             \n");		break;
	case 0x0048: OutputDebugStringW(L"WM_POWER                        \n");		break;
	case 0x004A: OutputDebugStringW(L"WM_COPYDATA                     \n");		break;
	case 0x004B: OutputDebugStringW(L"WM_CANCELJOURNAL                \n");		break;
	case 0x004E: OutputDebugStringW(L"WM_NOTIFY                       \n");		break;
	case 0x0050: OutputDebugStringW(L"WM_INPUTLANGCHANGEREQUEST       \n");		break;
	case 0x0051: OutputDebugStringW(L"WM_INPUTLANGCHANGE              \n");		break;
	case 0x0052: OutputDebugStringW(L"WM_TCARD                        \n");		break;
	case 0x0053: OutputDebugStringW(L"WM_HELP                         \n");		break;
	case 0x0054: OutputDebugStringW(L"WM_USERCHANGED                  \n");		break;
	case 0x0055: OutputDebugStringW(L"WM_NOTIFYFORMAT                 \n");		break;
	case 0x007B: OutputDebugStringW(L"WM_CONTEXTMENU                  \n");		break;
	case 0x007C: OutputDebugStringW(L"WM_STYLECHANGING                \n");		break;
	case 0x007D: OutputDebugStringW(L"WM_STYLECHANGED                 \n");		break;
	case 0x007E: OutputDebugStringW(L"WM_DISPLAYCHANGE                \n");		break;
	case 0x007F: OutputDebugStringW(L"WM_GETICON                      \n");		break;
	case 0x0080: OutputDebugStringW(L"WM_SETICON                      \n");		break;
	case 0x0081: OutputDebugStringW(L"WM_NCCREATE                     \n");		break;
	case 0x0082: OutputDebugStringW(L"WM_NCDESTROY                    \n");		break;
	case 0x0083: OutputDebugStringW(L"WM_NCCALCSIZE                   \n");		break;
	case 0x0084: OutputDebugStringW(L"WM_NCHITTEST                    \n");		break;
	case 0x0085: OutputDebugStringW(L"WM_NCPAINT                      \n");		break;
	case 0x0086: OutputDebugStringW(L"WM_NCACTIVATE                   \n");		break;
	case 0x0087: OutputDebugStringW(L"WM_GETDLGCODE                   \n");		break;
	case 0x0088: OutputDebugStringW(L"WM_SYNCPAINT                    \n");		break;
	case 0x00A0: OutputDebugStringW(L"WM_NCMOUSEMOVE                  \n");		break;
	case 0x00A1: OutputDebugStringW(L"WM_NCLBUTTONDOWN                \n");		break;
	case 0x00A2: OutputDebugStringW(L"WM_NCLBUTTONUP                  \n");		break;
	case 0x00A3: OutputDebugStringW(L"WM_NCLBUTTONDBLCLK              \n");		break;
	case 0x00A4: OutputDebugStringW(L"WM_NCRBUTTONDOWN                \n");		break;
	case 0x00A5: OutputDebugStringW(L"WM_NCRBUTTONUP                  \n");		break;
	case 0x00A6: OutputDebugStringW(L"WM_NCRBUTTONDBLCLK              \n");		break;
	case 0x00A7: OutputDebugStringW(L"WM_NCMBUTTONDOWN                \n");		break;
	case 0x00A8: OutputDebugStringW(L"WM_NCMBUTTONUP                  \n");		break;
	case 0x00A9: OutputDebugStringW(L"WM_NCMBUTTONDBLCLK              \n");		break;
	case 0x00AB: OutputDebugStringW(L"WM_NCXBUTTONDOWN                \n");		break;
	case 0x00AC: OutputDebugStringW(L"WM_NCXBUTTONUP                  \n");		break;
	case 0x00AD: OutputDebugStringW(L"WM_NCXBUTTONDBLCLK              \n");		break;
	case 0x00FE: OutputDebugStringW(L"WM_INPUT_DEVICE_CHANGE          \n");		break;
	case 0x00FF: OutputDebugStringW(L"WM_INPUT                        \n");		break;
	case 0x0100: OutputDebugStringW(L"WM_KEYDOWN                      \n");		break;
	case 0x0101: OutputDebugStringW(L"WM_KEYUP                        \n");		break;
	case 0x0102: OutputDebugStringW(L"WM_CHAR                         \n");		break;
	case 0x0103: OutputDebugStringW(L"WM_DEADCHAR                     \n");		break;
	case 0x0104: OutputDebugStringW(L"WM_SYSKEYDOWN                   \n");		break;
	case 0x0105: OutputDebugStringW(L"WM_SYSKEYUP                     \n");		break;
	case 0x0106: OutputDebugStringW(L"WM_SYSCHAR                      \n");		break;
	case 0x0107: OutputDebugStringW(L"WM_SYSDEADCHAR                  \n");		break;
	case 0x0109: OutputDebugStringW(L"WM_UNICHAR                      \n");		break;
	case 0xFFFF: OutputDebugStringW(L"UNICODE_NOCHAR                  \n");		break;
	case 0x0108: OutputDebugStringW(L"WM_KEYLAST                      \n");		break;
	case 0x010D: OutputDebugStringW(L"WM_IME_STARTCOMPOSITION         \n");		break;
	case 0x010E: OutputDebugStringW(L"WM_IME_ENDCOMPOSITION           \n");		break;
	case 0x010F: OutputDebugStringW(L"WM_IME_COMPOSITION              \n");		break;
	case 0x0110: OutputDebugStringW(L"WM_INITDIALOG                   \n");		break;
	case 0x0111: OutputDebugStringW(L"WM_COMMAND                      \n");		break;
	case 0x0112: OutputDebugStringW(L"WM_SYSCOMMAND                   \n");		break;
	case 0x0113: OutputDebugStringW(L"WM_TIMER                        \n");		break;
	case 0x0114: OutputDebugStringW(L"WM_HSCROLL                      \n");		break;
	case 0x0115: OutputDebugStringW(L"WM_VSCROLL                      \n");		break;
	case 0x0116: OutputDebugStringW(L"WM_INITMENU                     \n");		break;
	case 0x0117: OutputDebugStringW(L"WM_INITMENUPOPUP                \n");		break;
	case 0x0119: OutputDebugStringW(L"WM_GESTURE                      \n");		break;
	case 0x011A: OutputDebugStringW(L"WM_GESTURENOTIFY                \n");		break;
	case 0x011F: OutputDebugStringW(L"WM_MENUSELECT                   \n");		break;
	case 0x0120: OutputDebugStringW(L"WM_MENUCHAR                     \n");		break;
	case 0x0121: OutputDebugStringW(L"WM_ENTERIDLE                    \n");		break;
	case 0x0122: OutputDebugStringW(L"WM_MENURBUTTONUP                \n");		break;
	case 0x0123: OutputDebugStringW(L"WM_MENUDRAG                     \n");		break;
	case 0x0124: OutputDebugStringW(L"WM_MENUGETOBJECT                \n");		break;
	case 0x0125: OutputDebugStringW(L"WM_UNINITMENUPOPUP              \n");		break;
	case 0x0126: OutputDebugStringW(L"WM_MENUCOMMAND                  \n");		break;
	case 0x0127: OutputDebugStringW(L"WM_CHANGEUISTATE                \n");		break;
	case 0x0128: OutputDebugStringW(L"WM_UPDATEUISTATE                \n");		break;
	case 0x0129: OutputDebugStringW(L"WM_QUERYUISTATE                 \n");		break;
	case 0x0132: OutputDebugStringW(L"WM_CTLCOLORMSGBOX               \n");		break;
	case 0x0133: OutputDebugStringW(L"WM_CTLCOLOREDIT                 \n");		break;
	case 0x0134: OutputDebugStringW(L"WM_CTLCOLORLISTBOX              \n");		break;
	case 0x0135: OutputDebugStringW(L"WM_CTLCOLORBTN                  \n");		break;
	case 0x0136: OutputDebugStringW(L"WM_CTLCOLORDLG                  \n");		break;
	case 0x0137: OutputDebugStringW(L"WM_CTLCOLORSCROLLBAR            \n");		break;
	case 0x0138: OutputDebugStringW(L"WM_CTLCOLORSTATIC               \n");		break;
	case 0x01E1: OutputDebugStringW(L"MN_GETHMENU                     \n");		break;
	case 0x0200: OutputDebugStringW(L"WM_MOUSEMOVE                    \n");		break;
	case 0x0201: OutputDebugStringW(L"WM_LBUTTONDOWN                  \n");		break;
	case 0x0202: OutputDebugStringW(L"WM_LBUTTONUP                    \n");		break;
	case 0x0203: OutputDebugStringW(L"WM_LBUTTONDBLCLK                \n");		break;
	case 0x0204: OutputDebugStringW(L"WM_RBUTTONDOWN                  \n");		break;
	case 0x0205: OutputDebugStringW(L"WM_RBUTTONUP                    \n");		break;
	case 0x0206: OutputDebugStringW(L"WM_RBUTTONDBLCLK                \n");		break;
	case 0x0207: OutputDebugStringW(L"WM_MBUTTONDOWN                  \n");		break;
	case 0x0208: OutputDebugStringW(L"WM_MBUTTONUP                    \n");		break;
	case 0x0209: OutputDebugStringW(L"WM_MBUTTONDBLCLK                \n");		break;
	case 0x020A: OutputDebugStringW(L"WM_MOUSEWHEEL                   \n");		break;
	case 0x020B: OutputDebugStringW(L"WM_XBUTTONDOWN                  \n");		break;
	case 0x020C: OutputDebugStringW(L"WM_XBUTTONUP                    \n");		break;
	case 0x020D: OutputDebugStringW(L"WM_XBUTTONDBLCLK                \n");		break;
	case 0x020E: OutputDebugStringW(L"WM_MOUSEHWHEEL                  \n");		break;
	case 0x0210: OutputDebugStringW(L"WM_PARENTNOTIFY                 \n");		break;
	case 0x0211: OutputDebugStringW(L"WM_ENTERMENULOOP                \n");		break;
	case 0x0212: OutputDebugStringW(L"WM_EXITMENULOOP                 \n");		break;
	case 0x0213: OutputDebugStringW(L"WM_NEXTMENU                     \n");		break;
	case 0x0214: OutputDebugStringW(L"WM_SIZING                       \n");		break;
	case 0x0215: OutputDebugStringW(L"WM_CAPTURECHANGED               \n");		break;
	case 0x0216: OutputDebugStringW(L"WM_MOVING                       \n");		break;
	default:	 OutputDebugStringW(L"Unknown window message!         \n");		break;
	}
}

//-----------------------------------------------------------------------------
// Name: wildWeasel::whyIsGuiElemNotVisible()
// Desc: 
//-----------------------------------------------------------------------------
wstring wildWeasel::masterMind::whyIsGuiElemNotVisible(const wstring& textOfGuiElement)
{
	auto myElem = getGuiElementByName(textOfGuiElement);

	if (myElem) {

		if (auto myElem2 = myElem->getPointer<plainButton			>()) return myElem2->whyAmINotVisible();
		if (auto myElem2 = myElem->getPointer<cubicButton			>()) return myElem2->whyAmINotVisible();
		if (auto myElem2 = myElem->getPointer<guiElement			>()) return myElem2->whyAmINotVisible();
		if (auto myElem2 = myElem->getPointer<guiElement2D			>()) return myElem2->whyAmINotVisible();
		if (auto myElem2 = myElem->getPointer<guiElement3D			>()) return myElem2->whyAmINotVisible();
		if (auto myElem2 = myElem->getPointer<textLabel2D			>()) return myElem2->whyAmINotVisible();
		if (auto myElem2 = myElem->getPointer<guiElemEvFol2D		>()) return myElem2->whyAmINotVisible();
		if (auto myElem2 = myElem->getPointer<guiElemEvFol3D		>()) return myElem2->whyAmINotVisible();
		if (auto myElem2 = myElem->getPointer<buttonGeoPrim			>()) return myElem2->whyAmINotVisible();
		if (auto myElem2 = myElem->getPointer<plainButton2D			>()) return myElem2->whyAmINotVisible();
		if (auto myElem2 = myElem->getPointer<checkBox2D			>()) return myElem2->whyAmINotVisible();
		if (auto myElem2 = myElem->getPointer<borderLine2D			>()) return myElem2->whyAmINotVisible();
		if (auto myElem2 = myElem->getPointer<dropDown2D			>()) return myElem2->whyAmINotVisible();
		if (auto myElem2 = myElem->getPointer<triad					>()) return myElem2->whyAmINotVisible();
		if (auto myElem2 = myElem->getPointer<lineButton			>()) return myElem2->whyAmINotVisible();
		// if (auto myElem2 = myElem->getPointer<toolTipClass		>()) return myElem2->whyAmINotVisible();
		
		// Cluster
		// if (auto myElem2 = myElem->getPointer<scrollBar2D		>()) return myElem2->whyAmINotVisible();
		// if (auto myElem2 = myElem->getPointer<scrollBarDuo		>()) return myElem2->whyAmINotVisible();
		// if (auto myElem2 = myElem->getPointer<treeView2D			>()) return myElem2->whyAmINotVisible();
		// if (auto myElem2 = myElem->getPointer<editField2D		>()) return myElem2->whyAmINotVisible();
		// if (auto myElem2 = myElem->getPointer<listView2D			>()) return myElem2->whyAmINotVisible();
		// if (auto myElem2 = myElem->getPointer<rotationControlCube>()) return myElem2->whyAmINotVisible();
		// TODO: Implement function whyAmINotVisible() for Cluster classes

		return L"Gui element is of unknown type.";
	} else {
		return L"No gui element with this name found.";
	}
}

#pragma endregion

#pragma region camera
//-----------------------------------------------------------------------------
// Name: setActiveCamera()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::masterMind::setActiveCamera(camera& newActiveCamera)
{
	activeCamera = &newActiveCamera;
}

//-----------------------------------------------------------------------------
// Name: getActiveCamera()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::camera* wildWeasel::masterMind::getActiveCamera()
{
	return activeCamera;
}
#pragma endregion

#pragma region parent functions 
//-----------------------------------------------------------------------------
// Name: registerGuiElement()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::masterMind::registerGuiElement(guiElement* newElem)
{
	// locals
	guiElements.push_back(newElem);
	return true;
}

//-----------------------------------------------------------------------------
// Name: unregisterGuiElement()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::masterMind::unregisterGuiElement(guiElement* newElem)
{
	// locals
	guiElements.remove(newElem);
	return true;
}

//-----------------------------------------------------------------------------
// Name: unregisterGuiElement()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::guiElement* wildWeasel::masterMind::getGuiElementByName(const wstring& text)
{
	for (auto& curElement : guiElements) {
		if (text == curElement->getText()) return curElement;
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
// Name: processGuiElements()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::masterMind::processGuiElements()
{
	// this function could also be located in Device3D::Update(DX::StepTimer const& timer)
	if (mainLoadingScreen.isActive()) return;

	float elapsedSeconds	= (float) stepTimer.GetElapsedSeconds();
	float totalSeconds		= (float) stepTimer.GetTotalSeconds();

	for (auto& curElement : guiElements) {
		curElement->processInTime(totalSeconds, elapsedSeconds);
		if (curElement->userTimeProgress) {
			curElement->userTimeProgress((float) stepTimer.GetElapsedSeconds());
		}
	}
}

//-----------------------------------------------------------------------------
// Name: windowSizeChanged()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::masterMind::windowSizeChanged(int xSize, int ySize)
{
	// do nothing if window is minimized
	if (xSize + ySize == 0) return; 

	activeCamera->windowSizeChanged(xSize, ySize);

	for (auto& curElement : eventFollower::list_windowSizeChanged) {
		curElement->windowSizeChanged(xSize, ySize);
	}
}

//-----------------------------------------------------------------------------
// Name: processKeyDown()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::masterMind::processKeyDown(int keyCode)
{
	for (auto& curElement : eventFollower::list_keyDown) {
		if (curElement == guiElemEvFol::um_focusedButton) continue;
		curElement->keyDown(keyCode);
	}

	if (guiElemEvFol::um_focusedButton != nullptr) {
		guiElemEvFol::um_focusedButton->keyDown(keyCode);

		for (auto& curElement : guiElemEvFol::um_focusedButton->motherFollowers) {
			curElement->keyDown(keyCode);
		}
	}
}

//-----------------------------------------------------------------------------
// Name: processMouseVerticalWheel()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::masterMind::processMouseVerticalWheel(int distance)
{
	for (auto& curElement : eventFollower::list_verticalWheelMoved) {
		if (curElement == guiElemEvFol::um_focusedButton) continue;
		curElement->verticalWheelMoved(distance);
	}

	if (guiElemEvFol::um_focusedButton != nullptr) {
		guiElemEvFol::um_focusedButton->verticalWheelMoved(distance);

		for (auto& curElement : guiElemEvFol::um_focusedButton->motherFollowers) {
			curElement->verticalWheelMoved(distance);
		}
	}
}

//-----------------------------------------------------------------------------
// Name: processMouseHorizontalWheel()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::masterMind::processMouseHorizontalWheel(int distance)
{
	for (auto& curElement : eventFollower::list_horizontalWheelMoved) {
		if (curElement == guiElemEvFol::um_focusedButton) continue;
		curElement->horizontalWheelMoved(distance);
	}

	if (guiElemEvFol::um_focusedButton != nullptr) {
		guiElemEvFol::um_focusedButton->horizontalWheelMoved(distance);

		for (auto& curElement : guiElemEvFol::um_focusedButton->motherFollowers) {
			curElement->horizontalWheelMoved(distance);
		}
	}
}

//-----------------------------------------------------------------------------
// Name: processLeftButtonDown()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::masterMind::processLeftButtonDown(int xPos, int yPos)
{
	// ... when element is removed from list, while looop ist processed: BANG !
	// ... only process selected (mouseover) items like done in processKeyDown()
	for (auto& curElement : eventFollower::list_leftMouseButtonPressed) {
		curElement->leftMouseButtonPressed(xPos, yPos);
	}
}

//-----------------------------------------------------------------------------
// Name: processLeftButtonUp()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::masterMind::processLeftButtonUp(int xPos, int yPos)
{
	// ... only process selected (mouseover) items like done in processKeyDown()
	for (auto& curElement : eventFollower::list_leftMouseButtonReleased) {
		curElement->leftMouseButtonReleased(xPos, yPos);
	}
}

//-----------------------------------------------------------------------------
// Name: processRightButtonDown()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::masterMind::processRightButtonDown(int xPos, int yPos)
{
	// ... doing so has the advantage of allowing the list 'followingObjects.rightMouseButtonPressed' to be modified while looping through all elements. the drawback is this expensive copying.
	vector<eventFollower*> vecRightMouseButtonPressed(eventFollower::list_rightMouseButtonPressed.begin(), eventFollower::list_rightMouseButtonPressed.end());

	// ... only process selected (mouseover) items like done in processKeyDown()
	for (auto& curElement : vecRightMouseButtonPressed) {
	 	curElement->rightMouseButtonPressed(xPos, yPos);
	}
}

//-----------------------------------------------------------------------------
// Name: processRightButtonUp()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::masterMind::processRightButtonUp(int xPos, int yPos)
{
	// ... only process selected (mouseover) items like done in processKeyDown()
	for (auto& curElement : eventFollower::list_rightMouseButtonReleased) {
		curElement->rightMouseButtonReleased(xPos, yPos);
	}
}

//-----------------------------------------------------------------------------
// Name: processMouseMove()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::masterMind::processMouseMove(int xPos, int yPos, unsigned int wParam)
{
	// calc new position of 3D cursor
	cursor3D.calcCursorPos(cursor3D.position, *activeCamera, xPos, yPos);
	cursor3D.recalcMatrix();

	// update 2D cursor
	cursor2D.x = xPos;
	cursor2D.y = yPos;

	for (auto& curElement : eventFollower::list_mouseMoved) {
		curElement->mouseMoved(xPos, yPos, cursor3D.position, activeCamera->position);
	}
}
#pragma endregion

/*****************************************************************************/