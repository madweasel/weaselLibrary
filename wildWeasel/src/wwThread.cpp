/*********************************************************************
	wildWeasel.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/

#include "wwThread.h"

std::list<wildWeasel::threadEvent*>	wildWeasel::threadEvent::allEvents;

//-----------------------------------------------------------------------------
// Name: event::process()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::threadEvent::process()
{
	// locals
	auto		nextItr = allEvents.begin();
	threadEvent*	curEvent;

	// call function eventFunc() of each event, but use the subsequent pointer to iterator through the loop
	// doing so allows the function eventFunc() to remove itself from the list 'allEvents'
	while (nextItr != allEvents.end()) {
		curEvent = (*nextItr);
		nextItr++;
		if (curEvent->eventFunc != nullptr && curEvent->handle != nullptr && WaitForSingleObject(curEvent->handle, 0) == WAIT_OBJECT_0) {
			curEvent->eventFunc(curEvent->pUser);
		}
	}
}

//-----------------------------------------------------------------------------
// Name: event::create()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::threadEvent::create(masterMind* ww)
{
	handle = CreateEvent(NULL, false, false, NULL);
}

//-----------------------------------------------------------------------------
// Name: event::create()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::threadEvent::create(masterMind* ww, bool manualReset, bool initialState)
{
	handle = CreateEvent(NULL, manualReset, initialState, NULL);
}

//-----------------------------------------------------------------------------
// Name: event::set()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::threadEvent::~threadEvent()
{
	CloseHandle(handle);
	allEvents.remove(this);
}

//-----------------------------------------------------------------------------
// Name: event::set()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::threadEvent::set()
{
	SetEvent(handle);
}

//-----------------------------------------------------------------------------
// Name: event::reset()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::threadEvent::reset()
{
	ResetEvent(handle);
}

//-----------------------------------------------------------------------------
// Name: event::reset()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::threadEvent::wait(unsigned int timeOutInMilliseconds)
{
	if (WaitForSingleObject(handle, timeOutInMilliseconds) == WAIT_OBJECT_0) {
		return true;
	} else {
		return false;
	}
}

//-----------------------------------------------------------------------------
// Name: event::setFunc()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::threadEvent::setFunc(function<void(void*)> eventFunc, void* pUser, bool callInMainLoop)
{
	this->eventFunc	= eventFunc;
	this->pUser		= pUser;

	if (callInMainLoop) {
		if (find(allEvents.begin(), allEvents.end(), this) == allEvents.end()) {
			allEvents.push_back(this);
		}
	} else {
		allEvents.remove(this);
	}
}
