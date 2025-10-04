/*********************************************************************\
	threadManager.h												  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/
#ifndef THREADMANAGER_H
#define THREADMANAGER_H

// standard library & win32 api
#include <windows.h>
#include <cstdio>
#include <iostream>
#include <vector>
#include <memory>
#include <barrier> 

using namespace std;														// use standard library namespace

/*** Constants ******************************************************/
#define TM_SCHEDULE_USER_DEFINED					0						// user defined scheduling
#define TM_SCHEDULE_STATIC							1						// each thread gets the same number of iterations
#define TM_SCHEDULE_DYNAMIC							2						// not implemented yet
#define TM_SCHEDULE_GUIDED							3						// not implemented yet
#define TM_SCHEDULE_RUNTIME							4						// not implemented yet
#define TM_SCHEDULE_NUM_TYPES						5						// number of scheduling types

#define TM_RETURN_VALUE_OK							0						// return values of the execution functions
#define TM_RETURN_VALUE_TERMINATE_ALL_THREADS		1
#define TM_RETURN_VALUE_EXECUTION_CANCELLED			2
#define TM_RETURN_VALUE_INVALID_PARAM				3
#define TM_RETURN_VALUE_UNEXPECTED_ERROR			4

/*** Classes *********************************************************/

// In principle, the class threadManagerClass is a wrapper for the win32 api functions for creating and managing threads.
// The class provides a simple interface for executing a function in parallel on multiple threads.
// It is possible to execute a function in parallel on multiple threads, or to execute a loop in parallel on multiple threads.
// The loop is divided into chunks and each thread gets a chunk to work on.
// The class also provides a barrier function, which can be used to synchronize the threads.
// The class also provides a function to pause and cancel the execution of the threads.
// The class also provides a function to set a callback function which is called every x-milliseconds during execution between two iterations.
class threadManagerClass
{
private:

	// structures
	struct forLoopStruct														// structure used in threadForLoop()
	{
		unsigned int		scheduleType			= TM_SCHEDULE_USER_DEFINED;	// type of scheduling, for load balancing
		int64_t				increment				= 1;						// step size of the loop
		int64_t				initialValue			= 0;						// initial value of the loop
		int64_t				finalValue				= 0;						// final value of the loop
		void *				pParameter				= nullptr;					// pointer to the user defined structure
		DWORD				(*threadProc)(void* pParameter, int64_t index)	= nullptr;	// pointer to the user function to be executed by the threads
		threadManagerClass *threadManager;										// pointer to the threadManagerClass object
	};
	
	struct threadItem
	{
		HANDLE				hThread;											// the thread handle given by the system
		DWORD				threadId;											// the thread id given by the system
		unsigned int		threadNo;											// the thread number from 0 to numThreads-1

							threadItem();
							~threadItem();
	};

	// Variables
	unsigned int			numThreads						= 0;				// number of threads
	bool					terminateAllThreads				= false;			// true when cancelExecution() was called
	bool					executionPaused					= false;			// true if thread execution is currently paused
	bool					executionCancelled				= false;			// true when cancelExecution() was called
	vector<threadItem>		threads;											// array of size 'numThreads' containing the thread handles, thread ids
	std::barrier<>*			pBarrier						= nullptr;			// pointer to a barrier object
	bool 					anyThreadOnLastIteration		= false;			// true if any thread is on the last iteration

	// functions
	static DWORD WINAPI		threadForLoop					(LPVOID lpParameter);
	bool					resizeArrays					();

public:

	class threadVarsArrayItem
	{
	public:
		unsigned int										curThreadNo;

		virtual void										destroyElement	  () {};	// destroy the element
		virtual void										reduce			  () {};	// merge the results of the threads
	};

	template <class varType> class threadVarsArray
	{
	public:
		unsigned int		numberOfThreads					= 0;			// number of threads
		vector<varType>		item;											// array of size 'numberOfThreads' containing the threadVarsArrayItem objects

		threadVarsArray(unsigned int numberOfThreads, varType const&  master)
		{
			this->numberOfThreads	= numberOfThreads;
			this->item = std::vector<varType>(numberOfThreads, master);

			for (unsigned int threadCounter=0; threadCounter<numberOfThreads; threadCounter++) {
				item[threadCounter].curThreadNo		= threadCounter;
			}
		};

		~threadVarsArray()
		{
			for (unsigned int threadCounter=0; threadCounter<numberOfThreads; threadCounter++) {
				item[threadCounter].destroyElement();
			}
		};

		void *getPointerToArray()
		{
			return (void*) item.data();
		};

		unsigned int getSizeOfArray()
		{
			return sizeof(varType);
		};

		void reduce()
		{
			for (unsigned int threadCounter=0; threadCounter<numberOfThreads; threadCounter++) {
				item[threadCounter].reduce();
			}
		};	
	};

    // Constructor / destructor
    threadManagerClass();
    ~threadManagerClass();

	// Functions
	unsigned int			getThreadNumber					();									// returns a number from 0 to 'numThreads'-1
	unsigned int			getNumThreads					();									// returns the total number of threads
	
	bool					setNumThreads					(unsigned int newNumThreads);		// tries to set the number of threads. Returns false if any thread is running.
	void					waitForOtherThreads				();									// waits for all threads to reach this point
	void	 				waitForAllThreadsToTerminate	();									// waits for all threads to terminate
	void					pauseExecution					();									// un-/suspend all threads
	void					cancelExecution					();									// terminateAllThreads = true
	bool					wasExecutionCancelled			();									// tells if the execution was cancelled
	void					reset							();									// resets to the initial state
	void					setCallBackFunction				(void userFunction(void* pUser), void* pUser, DWORD milliseconds);		// a user function which is called every x-milliseconds during execution between two iterations
	bool					anyThreadRunning				();									// returns true if any thread is running
	
	// execute
	unsigned int 			executeInParallel				(DWORD threadProc(void* pParameter			 	 ), void *pParameter, unsigned int parameterStructSize);
	unsigned int			executeParallelLoop				(DWORD threadProc(void* pParameter, int64_t index), void *pParameter, unsigned int parameterStructSize, unsigned int scheduleType, int64_t initialValue, int64_t finalValue, int64_t increment);
};

#endif
