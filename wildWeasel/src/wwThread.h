/*********************************************************************\
	wildWeasel.h												  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/

#pragma once

#include <list>
#include <functional>
#include <windows.h>

namespace wildWeasel
{
    // used namespaces
	using namespace std;							// standard library

    class masterMind;

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
}
