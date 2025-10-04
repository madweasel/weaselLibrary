/*********************************************************************
	debugHelper.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/
#ifndef debugHelper_H
#define debugHelper_H

#include <list>
#include <Windows.h>

using namespace std;

#define DHC_DEFAULT_CHAR_ARRAY_SIZE				  100
#define DHC_WAIT_TIME_FOR_THREAD_TERMINATION	60000	// milliseconds
#define	DHC_UPDATE_RATE							  200	// milliseconds
#define	DHC_TIMER_ID_SECOND_HAS_PASSED			    1

#define DHC_FLOAT									0
#define DHC_DOUBLE									1
#define	DHC_UINT									2
#define	DHC_INT										3
#define DHC_STRING									4
#define	DHC_CHAR									5

class debugHelper
{
protected: 

	struct dbArray
	{
		unsigned int				posX;
		unsigned int				posY;
		unsigned int				width;
		unsigned int				size;
		unsigned int				type;
		unsigned int				id;
		unsigned int				numDigits;
		char						*name;
		void						*pArray;
		HWND						*hEditNumbers;
		HWND						hTextTitle;
	};

	// Constants
	unsigned int					charWidth;
	unsigned int					charHeight;
	unsigned int					updateCounter;

	// Variables
	HFONT							hFont ;
	HINSTANCE						hInst;
	HWND							hWndParent;
	HANDLE							hThreadDebugHelper;
	HWND							hTextCounter;
	HWND							hWndDebugHelper;
	DWORD							threadId;
	RECT							rcWnd;
	list<dbArray*>					arrayList;
	
	HANDLE							hEventTerminateThread,		hEventThreadTerminated,		hEventThreadReady;
	HANDLE							hEventCreateArray,			hEventDeleteArray;
		
	// Functions

	// Static Functions
	static void						createControlsForArray		(debugHelper *dh, dbArray *dba);
	static void						deleteControlsOfArray		(debugHelper *dh, dbArray *dba);
	static LRESULT CALLBACK			DebugHelperWndProc			(HWND, UINT, WPARAM, LPARAM);
	static DWORD WINAPI				ThreadProcDebugHelper		(LPVOID lpParameter);

public:

	// Constructor / destructor
									debugHelper					(HWND hWndParent, HINSTANCE hInst, RECT *rcWindow);
									~debugHelper				();

	// Functions
	void							updateDebugHelper			();
	void							addArray					(unsigned int id, void* pArray, unsigned int varType, unsigned int posX, unsigned int posY, unsigned int width, unsigned int size, char name[], unsigned int numDigits);
	void							deleteArray					(void* pArray);
	bool							setPointer					(unsigned int id, void* pArray);
	bool							setName						(unsigned int id, char *newName);

	// getter
	unsigned int					getNumArrays				() { return (unsigned int) arrayList.size(); };
};

// TODO: should make use of templates
// TODO: updating should be timer based
// TODO: should be object, not id based
// TODO: printing the array/template should be user customizable by a callback function
// TODO: one window per array

#endif