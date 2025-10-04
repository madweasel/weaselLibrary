/*********************************************************************
	debugHelper.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/

#include "debugHelper.h"

//-----------------------------------------------------------------------------
// Name: debugHelper constructor()
// Desc: 
//-----------------------------------------------------------------------------
debugHelper::debugHelper(HWND hWndParent, HINSTANCE hInst, RECT *rcWindow)
{
	// locals
	unsigned int		defCharWidth	= 8;
	unsigned int		defCharHeight	= 14;

	// copy parameters
	this->hWndParent	= hWndParent;
	this->hInst			= hInst;
	rcWnd				= *rcWindow;
	charWidth			= defCharWidth;
	charHeight			= defCharHeight;
	hFont				= CreateFontA(charHeight, charWidth, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, "Courier New");

	// create events
	hEventTerminateThread   = CreateEvent(NULL, true,  false, NULL);
	hEventThreadTerminated  = CreateEvent(NULL, false, false, NULL);
	hEventCreateArray		= CreateEvent(NULL, true,  false, NULL);
	hEventDeleteArray		= CreateEvent(NULL, true,  false, NULL);
	hEventThreadReady		= CreateEvent(NULL, true,  false, NULL);

	// create thread
	hThreadDebugHelper = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) ThreadProcDebugHelper, (void*) this, 0, &threadId);
	SetThreadPriority(hThreadDebugHelper, THREAD_PRIORITY_ABOVE_NORMAL);
}

//-----------------------------------------------------------------------------
// Name: debugHelper destructor()
// Desc: 
//-----------------------------------------------------------------------------
debugHelper::~debugHelper()
{
	// locals
	list<dbArray*>::iterator	itr, nextItr;

	// terminate thread
	if (hWndDebugHelper != NULL) {
		PostMessage(hWndDebugHelper, WM_COMMAND, 0, (LPARAM) hEventTerminateThread);
	}

	if (WaitForSingleObject(hEventThreadTerminated, DHC_WAIT_TIME_FOR_THREAD_TERMINATION) != WAIT_OBJECT_0) {
		TerminateThread(hThreadDebugHelper, 0);	
		hThreadDebugHelper = NULL;
	}

	//find item in list
	for (itr=arrayList.begin(); itr!=arrayList.end(); itr++) {
		nextItr = itr;
		nextItr++;
		deleteArray((*itr)->pArray);
		if (nextItr==arrayList.end()) break;
		itr = nextItr;
	}

	// handles
	DeleteObject(hFont);
	CloseHandle(hEventTerminateThread   );
	CloseHandle(hEventThreadTerminated  );
	CloseHandle(hEventCreateArray		);
	CloseHandle(hEventDeleteArray		);
	CloseHandle(hEventThreadReady		);
}

//-----------------------------------------------------------------------------
// Name: DebugHelperWndProc()
// Desc: Processes messages for the window.
//-----------------------------------------------------------------------------
LRESULT CALLBACK debugHelper::DebugHelperWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// locals
	HWND		 hControl;
	debugHelper *dh;

	// get member class
	dh = (debugHelper*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (dh == NULL) return DefWindowProc(hWnd, message, wParam, lParam);

	switch (message)
	{
	case WM_TIMER:
		switch (wParam)
		{
			case DHC_TIMER_ID_SECOND_HAS_PASSED:
				dh->updateDebugHelper();
				break;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_COMMAND:
		hControl = (HWND) lParam;
		if (hControl == dh->hEventTerminateThread) {	
			DestroyWindow(dh->hWndDebugHelper);
			PostQuitMessage(0);
		} else if (hControl == dh->hEventCreateArray) {
			createControlsForArray(dh, (dbArray*) wParam);
		} else if (hControl == dh->hEventDeleteArray) {
			deleteControlsOfArray(dh, (dbArray*) wParam);
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

//-----------------------------------------------------------------------------
// Name: ThreadProcDebugHelper()
// Desc: 
//-----------------------------------------------------------------------------
DWORD WINAPI debugHelper::ThreadProcDebugHelper(LPVOID lpParameter)
{
	// locals
	WNDCLASSEX	wcex;
	MSG msg;

	// get member class
	debugHelper		 *dh		= (debugHelper*) lpParameter;

	// register window class
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_DROPSHADOW;
	wcex.lpfnWndProc	= debugHelper::DebugHelperWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= sizeof(debugHelper*);
	wcex.hInstance		= dh->hInst;
	wcex.hIcon			= LoadIcon(dh->hInst, IDI_INFORMATION);
	wcex.hCursor		= LoadCursor(dh->hInst, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_BTNFACE+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= L"wndClassDebugHelper";
	wcex.hIconSm		= LoadIcon(dh->hInst, IDI_INFORMATION);
	RegisterClassEx(&wcex);

	// create window
	dh->hWndDebugHelper = CreateWindow(L"wndClassDebugHelper", L"Debug Helper", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_BORDER | WS_MINIMIZEBOX, 
							       dh->rcWnd.left, dh->rcWnd.top, dh->rcWnd.right, dh->rcWnd.bottom, NULL, NULL, dh->hInst, NULL);

	// connect this window handle with this member class
	SetWindowLongPtr(dh->hWndDebugHelper, GWLP_USERDATA, (LONG_PTR) dh);

	// updateCounter
	dh->hTextCounter	= CreateWindowA("STATIC", "", WS_VISIBLE | WS_CHILD | ES_LEFT, 0, 0, 5*dh->charWidth, dh->charHeight, dh->hWndDebugHelper, NULL, dh->hInst, NULL);
	PostMessage(dh->hTextCounter, WM_SETFONT, (WPARAM) dh->hFont, (LPARAM) TRUE);
	dh->updateCounter	= 0;

	// thread is ready
	SetEvent(dh->hEventThreadReady);

	// show window
	ShowWindow(dh->hWndDebugHelper, SW_SHOW);

	// set timer, so that progress is updated each second
	SetTimer(dh->hWndDebugHelper, DHC_TIMER_ID_SECOND_HAS_PASSED, DHC_UPDATE_RATE, NULL);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	SetTimer(dh->hWndDebugHelper, DHC_TIMER_ID_SECOND_HAS_PASSED, 0, NULL);
	dh->hThreadDebugHelper = NULL;
	ResetEvent(dh->hEventTerminateThread);
	SetEvent(dh->hEventThreadTerminated);

	// terminate thread
	return 0;
}

//-----------------------------------------------------------------------------
// Name: createControlsForArray()
// Desc: 
//-----------------------------------------------------------------------------
void debugHelper::createControlsForArray(debugHelper *dh, dbArray *dba)
{
	// locals
	char			str[DHC_DEFAULT_CHAR_ARRAY_SIZE];
	unsigned int	i;
	RECT			rc;

	rc.left				= dba->posX * dh->charWidth;
	rc.top				= dba->posY * dh->charHeight; 
	rc.right			= dh->charWidth * (unsigned int) strlen(dba->name);
	rc.bottom			= dh->charHeight;

	dba->hEditNumbers	= new HWND[dba->size];
	dba->hTextTitle		= CreateWindowA("STATIC", dba->name, WS_VISIBLE | WS_CHILD | ES_LEFT, rc.left, rc.top, rc.right, rc.bottom, dh->hWndDebugHelper, NULL, dh->hInst, NULL);
	
	PostMessage(dba->hTextTitle, WM_SETFONT, (WPARAM) dh->hFont, (LPARAM) TRUE);

	// create buttons and labels
	for (i=0; i<dba->size; i++) {
		rc.left				= (dba->posX     + (i % dba->width) * (dba->numDigits + 1)) * dh->charWidth;
		rc.top				= (dba->posY + 1 + i / dba->width) * dh->charHeight; 
		rc.right			= dh->charWidth * (dba->numDigits + 1);
		rc.bottom			= dh->charHeight;
		switch (dba->type)
		{
			case DHC_UINT:	sprintf(str, "%d",  ((unsigned int*) dba->pArray)[i]);	break;
		}
		dba->hEditNumbers[i]	= CreateWindowA("STATIC", str, WS_VISIBLE | WS_CHILD | ES_RIGHT, rc.left, rc.top, rc.right, rc.bottom, dh->hWndDebugHelper, NULL, dh->hInst, NULL);
		PostMessage(dba->hEditNumbers[i], WM_SETFONT, (WPARAM) dh->hFont, (LPARAM) TRUE);
	}

	ResetEvent(dh->hEventCreateArray);
}

//-----------------------------------------------------------------------------
// Name: deleteControlsOfArray()
// Desc: 
//-----------------------------------------------------------------------------
void debugHelper::deleteControlsOfArray(debugHelper *dh, dbArray *dba)
{
	// locals
	unsigned int i;

	// delete arrays
	for (i=0; i<dba->size; i++) {
		DestroyWindow(dba->hEditNumbers[i]);
	}
	DestroyWindow(dba->hTextTitle);
	delete [] dba->hEditNumbers;	 dba->hEditNumbers	= nullptr;
	delete [] dba->name;			 dba->name			= nullptr;
	delete dba;						 dba				= nullptr;

	ResetEvent(dh->hEventDeleteArray);
}

//-----------------------------------------------------------------------------
// Name: addArray()
// Desc: 
//-----------------------------------------------------------------------------
void debugHelper::addArray(unsigned int id, void* pArray, unsigned int varType, unsigned int posX, unsigned int posY, unsigned int width, unsigned int size, char name[], unsigned int numDigits)
{
	// locals
	dbArray	*		dba			= new dbArray;

	dba->posX			= posX;
	dba->posY			= posY;
	dba->width			= width;
	dba->size			= size;
	dba->numDigits		= numDigits;
	dba->hTextTitle		= NULL;
	dba->hEditNumbers	= NULL;
	dba->id				= id;
	dba->type			= varType;
	dba->pArray			= (void*) pArray;
	dba->name			= new char[strlen(name)+1];

	strcpy(dba->name, name);
	arrayList.push_back(dba);

	WaitForSingleObject(hEventThreadReady, INFINITE);
	PostMessage(hWndDebugHelper, WM_COMMAND, (WPARAM) dba, (LPARAM) hEventCreateArray);
}

//-----------------------------------------------------------------------------
// Name: deleteArray()
// Desc: 
//-----------------------------------------------------------------------------
void debugHelper::deleteArray(void* pArray)
{
	// locals
	list<dbArray*>::iterator	itr;
	dbArray	*		dba;

	//find item in list
	for (itr=arrayList.begin(); itr!=arrayList.end(); itr++) {
		dba = (*itr);
		if (dba->pArray == pArray) break;
	}
	arrayList.erase(itr);

	InvalidateRect(hWndDebugHelper, NULL, TRUE);
	WaitForSingleObject(hEventThreadReady, INFINITE);
	PostMessage(hWndDebugHelper, WM_COMMAND, (WPARAM) dba, (LPARAM) hEventDeleteArray);
}

//-----------------------------------------------------------------------------
// Name: setName()
// Desc: 
//-----------------------------------------------------------------------------
bool debugHelper::setName(unsigned int id, char *newName)
{
	// parameters ok?
	if (newName == NULL) return false;
		
	// locals
	dbArray	*		dba;
	list<dbArray*>::iterator	itr;

	// process each array
	for (itr=arrayList.begin(); itr!=arrayList.end(); itr++) {
		
		dba = (*itr);

		// and each item
		if (dba->id == id) {
			if (dba->name != NULL) delete [] dba->name;
			dba->name = new char[strlen(newName) + 1];
			strcpy(dba->name, newName);
			SetWindowTextA(dba->hTextTitle, dba->name);
			SetWindowPos(dba->hTextTitle, NULL, 0, 0, charWidth * (unsigned int) strlen(dba->name), charHeight, SWP_NOMOVE | SWP_NOZORDER);
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
// Name: setPointer()
// Desc: 
//-----------------------------------------------------------------------------
bool debugHelper::setPointer(unsigned int id, void* pArray)
{
	// parameters ok?
	if (pArray == NULL) return false;
		
	// locals
	dbArray	*		dba;
	list<dbArray*>::iterator	itr;

	// process each array
	for (itr=arrayList.begin(); itr!=arrayList.end(); itr++) {
		
		dba = (*itr);

		// and each item
		if (dba->id == id) {
			dba->pArray = pArray;
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
// Name: updateDebugHelper()
// Desc: 
//-----------------------------------------------------------------------------
void debugHelper::updateDebugHelper()
{
	// locals
	dbArray	*		dba;
	char			str[DHC_DEFAULT_CHAR_ARRAY_SIZE];
	unsigned int	i;
	list<dbArray*>::iterator	itr;

	// update counter
	sprintf(str, "%d", updateCounter++);
	SetWindowTextA(hTextCounter, str);

	// process each array
	for (itr=arrayList.begin(); itr!=arrayList.end(); itr++) {
		
		dba = (*itr);

		// and each item
		for (i=0; i<dba->size; i++) {
			switch (dba->type)
			{
				case DHC_UINT:	sprintf(str, "%d",  ((unsigned int*) dba->pArray)[i]);	break;
			}
			if (dba->hEditNumbers != NULL) {
				SetWindowTextA(dba->hEditNumbers[i], str);
				SetWindowTextA(dba->hTextTitle, dba->name);
			}
		}
	}
}
