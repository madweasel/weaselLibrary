/*********************************************************************
	threadManager.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/

#include "threadManager.h"

//-----------------------------------------------------------------------------
// Name: threadItem()
// Desc: threadItem class constructor
//-----------------------------------------------------------------------------
threadManagerClass::threadItem::threadItem()
{
	threadNo 	= 0;
	hThread		= NULL;
	threadId	= 0;
}

//-----------------------------------------------------------------------------
// Name: ~threadItem()
// Desc: threadItem class destructor
//-----------------------------------------------------------------------------
threadManagerClass::threadItem::~threadItem()
{
	if (hThread && hThread != INVALID_HANDLE_VALUE) {
		CloseHandle(hThread);
	}
	hThread = NULL;
	threadId = 0;
}

//-----------------------------------------------------------------------------
// Name: threadManagerClass()
// Desc: threadManagerClass class constructor
//-----------------------------------------------------------------------------
threadManagerClass::threadManagerClass()
{
	SYSTEM_INFO		m_si			= {0};
	GetSystemInfo(&m_si);
	numThreads						= m_si.dwNumberOfProcessors;
	resizeArrays();
	reset();
}

//-----------------------------------------------------------------------------
// Name: ~threadManagerClass()
// Desc: threadManagerClass class destructor
//-----------------------------------------------------------------------------
threadManagerClass::~threadManagerClass()
{
	cancelExecution();
	waitForAllThreadsToTerminate();
}

//-----------------------------------------------------------------------------
// Name: resizeArrays()
// Desc: Resizes the arrays. Returns false if any thread is running.
//-----------------------------------------------------------------------------
bool threadManagerClass::resizeArrays()
{
	// cancel if any thread running
	if (anyThreadRunning()) {
		return false;
	}
	threads.resize(numThreads);
	if (pBarrier) {
		delete pBarrier;
		pBarrier = nullptr;
	}
	pBarrier = new std::barrier(numThreads);
	for (unsigned int i=0; i<numThreads; i++) {
		threads[i].threadNo = i;
	}
	return true;
}

//-----------------------------------------------------------------------------
// Name: reset()
// Desc: Resets to the initial state.
//-----------------------------------------------------------------------------
void threadManagerClass::reset()
{
	cancelExecution();
	waitForAllThreadsToTerminate();
	terminateAllThreads				= false;
	executionPaused					= false;
	executionCancelled				= false;
	anyThreadOnLastIteration		= false;
}

//-----------------------------------------------------------------------------
// Name: waitForAllThreadsToTerminate()
// Desc: Waits for all threads to terminate.
//-----------------------------------------------------------------------------
void threadManagerClass::waitForAllThreadsToTerminate()
{
	// checks
	if (numThreads == 0) return;
	if (!anyThreadRunning()) return;

	// map member variable of vector items to linear array
	vector<HANDLE> hThreads;
	for (auto& thread : threads) {
		hThreads.push_back(thread.hThread);
	}

	// wait for every thread to end
	WaitForMultipleObjects(numThreads, hThreads.data(), TRUE, INFINITE);

	// Close all thread handles upon completion.
	for (auto& thread : threads) {
		if (thread.hThread) {
			CloseHandle(thread.hThread);
		}
		thread.hThread		= NULL;
		thread.threadId		= 0;
	}
}

//-----------------------------------------------------------------------------
// Name: waitForOtherThreads()
// Desc: Waits for all other threads to reach the barrier.
//
// This function is used to synchronize multiple threads. Each thread calls this function
// and waits until all threads have reached the barrier. Once all threads have reached the
// barrier, they are all released to continue execution.
//
// The function uses a critical section to ensure that the increment of the counter
// `numThreadsPassedBarrier` is thread-safe. When the last thread reaches the barrier,
// it resets the counter and signals an event to release all waiting threads.
//
// This function assumes that `numThreads` is the total number of threads that need to reach the barrier.
//-----------------------------------------------------------------------------
void threadManagerClass::waitForOtherThreads()
{
	if (anyThreadOnLastIteration) return;	// no need to wait if the last iteration is reached
	pBarrier->arrive_and_wait(); 
}

//-----------------------------------------------------------------------------
// Name: getNumThreads()
// Desc: Returns the number of threads
//-----------------------------------------------------------------------------
unsigned int threadManagerClass::getNumThreads()
{
	return numThreads;
}

//-----------------------------------------------------------------------------
// Name: anyThreadRunning()
// Desc: Returns true if any thread is running.
//-----------------------------------------------------------------------------
bool threadManagerClass::anyThreadRunning()
{
	for (auto& thread : threads) {
		if (thread.hThread) {
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
// Name: setNumThreads()
// Desc: Tries to set the number of threads. Returns false if any thread is running.
//-----------------------------------------------------------------------------
bool threadManagerClass::setNumThreads(unsigned int newNumThreads)
{
	if (newNumThreads == 0) return false;
	if (newNumThreads == numThreads) return true;

	// cancel if any thread running
	if (anyThreadRunning()) {
		return false;
	}
	numThreads = newNumThreads;
	if (!resizeArrays()) {
		return false;
	}
	reset();
	return true;
}

//-----------------------------------------------------------------------------
// Name: pauseExecution()
// Desc: Suspends all threads. Call this function again to resume execution. 
//-----------------------------------------------------------------------------
void threadManagerClass::pauseExecution()
{
	for (auto& thread : threads) {
		
		// unsuspend all threads
		if (!executionPaused) {
			SuspendThread(thread.hThread);
		// suspend all threads
		} else {
			ResumeThread(thread.hThread);
		}
	}
	executionPaused = (!executionPaused);
}

//-----------------------------------------------------------------------------
// Name: cancelExecution()
// Desc: Stops executeParallelLoop() before the next iteration.
//	     When executeInParallel() was called, user has to handle cancellation by himself.
//-----------------------------------------------------------------------------
void threadManagerClass::cancelExecution()
{
	terminateAllThreads  = true;
	executionCancelled = true;
	if (executionPaused) {
		pauseExecution();
	}
}

//-----------------------------------------------------------------------------
// Name: wasExecutionCancelled()
// Desc: Tells if the execution was cancelled.
//-----------------------------------------------------------------------------
bool threadManagerClass::wasExecutionCancelled()
{
	return executionCancelled;
}

//-----------------------------------------------------------------------------
// Name: getThreadId()
// Desc: Returns a number from 0 to 'numThreads'-1. Returns numThreads if the function fails.
//-----------------------------------------------------------------------------
unsigned int threadManagerClass::getThreadNumber()
{
	// locals
	DWORD			curThreadId = GetCurrentThreadId();

	for (auto& thread : threads) {
		if (curThreadId == thread.threadId) {
			return thread.threadNo;
		}
	}
	return numThreads;
}

//-----------------------------------------------------------------------------
// Name: executeInParallel()
// Desc: The user defined function threadProc is called numThreads times in parallel.
//		 pParameter is an array of size numThreads. Each element having parameterStructSize bytes.
//		 If parameterStructSize is 0, then a const pParameter is used in every thread.
//-----------------------------------------------------------------------------
unsigned int threadManagerClass::executeInParallel(DWORD threadProc(void* pParameter), void *pParameter, unsigned int parameterStructSize)
{
	// locals
	unsigned int	curThreadNo;
	SIZE_T			dwStackSize		= 0;

	// parameters ok?
	if (pParameter == NULL)			return TM_RETURN_VALUE_INVALID_PARAM;
	if (numThreads == 0)			return TM_RETURN_VALUE_INVALID_PARAM;
	if (threadProc == NULL)			return TM_RETURN_VALUE_INVALID_PARAM;
	if (executionCancelled)			return TM_RETURN_VALUE_EXECUTION_CANCELLED;
	if (anyThreadRunning())			return TM_RETURN_VALUE_UNEXPECTED_ERROR;

	// globals
	terminateAllThreads			= false;
	anyThreadOnLastIteration 	= false;

	// create threads
	for (curThreadNo=0; curThreadNo<numThreads; curThreadNo++) {
		
		void* pUser 					= (void*) (((char *) pParameter) + curThreadNo * parameterStructSize);
		threads[curThreadNo].hThread 	= CreateThread(NULL, dwStackSize, (LPTHREAD_START_ROUTINE) threadProc, pUser, CREATE_SUSPENDED, &threads[curThreadNo].threadId);
		SetThreadPriority(threads[curThreadNo].hThread, THREAD_PRIORITY_BELOW_NORMAL);
		
		if (threads[curThreadNo].hThread == NULL) {
			for (curThreadNo; curThreadNo>0; curThreadNo--) {
				CloseHandle(threads[curThreadNo-1].hThread);
				threads[curThreadNo-1].hThread = NULL;
			}
			return TM_RETURN_VALUE_UNEXPECTED_ERROR;
		}
	}

	// start threads
	for (auto& thread : threads) {
		if (!executionPaused) ResumeThread(thread.hThread);
	}

	// wait for every thread to end
	waitForAllThreadsToTerminate();

	// everything ok
	if (executionCancelled) {
		return TM_RETURN_VALUE_EXECUTION_CANCELLED;
	} else {
		return TM_RETURN_VALUE_OK;
	}
}

//-----------------------------------------------------------------------------
// Name: executeInParallel()
// Desc: Runs a loop in parallel. The loop is divided into chunks and each thread gets a chunk to work on.
// pParameter  - an array of size numThreads containing the user defined structures
// finalValue  - this value is part of the iteration, meaning that index ranges from initialValue to finalValue including both border values
//-----------------------------------------------------------------------------
unsigned int threadManagerClass::executeParallelLoop(	DWORD 			threadProc(void* pParameter, int64_t index), 
														void *			pParameter, 
														unsigned int	parameterStructSize, 
														unsigned int	scheduleType, 
														int64_t			initialValue, 
														int64_t			finalValue, 
														int64_t			increment)
{
	// parameters ok?
	if (numThreads		== 0)							return TM_RETURN_VALUE_INVALID_PARAM;
	if (threadProc		== NULL)						return TM_RETURN_VALUE_INVALID_PARAM;
	if (executionCancelled == true)						return TM_RETURN_VALUE_EXECUTION_CANCELLED;
	if (pParameter   == NULL)							return TM_RETURN_VALUE_INVALID_PARAM;
	if (scheduleType >= TM_SCHEDULE_NUM_TYPES)			return TM_RETURN_VALUE_INVALID_PARAM;
	if (increment	 == 0)								return TM_RETURN_VALUE_INVALID_PARAM;
	if (abs(finalValue-initialValue)+1 < abs(increment))return TM_RETURN_VALUE_INVALID_PARAM;

	// locals
	unsigned int	curThreadNo;														// the threads are enumerated from 0 to numThreads-1
	int64_t			numIterations		= (finalValue - initialValue) / increment + 1;	// total number of iterations
	int64_t			chunkSize			= 0;											// number of iterations per chunk
	SIZE_T			dwStackSize			= 0;											// initital stack size of each thread. 0 means default size ~1MB
	std::vector<forLoopStruct> forLoopParameters(numThreads); 							// array of size numThreads containing the parameters for the threads

	// globals
	terminateAllThreads			= false;
	anyThreadOnLastIteration 	= false;

	// create threads
	for (curThreadNo=0; curThreadNo<numThreads; curThreadNo++) {

		forLoopParameters[curThreadNo].pParameter			= (pParameter!=NULL ? (void*) (((char *) pParameter) + curThreadNo * parameterStructSize) : NULL);
		forLoopParameters[curThreadNo].threadManager		= this;
		forLoopParameters[curThreadNo].threadProc			= threadProc;
		forLoopParameters[curThreadNo].increment			= increment;
		forLoopParameters[curThreadNo].scheduleType			= scheduleType;
		
		switch (scheduleType)
		{
		case TM_SCHEDULE_STATIC: 
			chunkSize										= numIterations / numThreads;								// number of iterations per thread
			chunkSize									   += (curThreadNo < numIterations % numThreads ? 1 : 0);		// add one more iteration to the first threads to balance the rest
			if (curThreadNo==0) { 
				forLoopParameters[curThreadNo].initialValue	= initialValue;
			} else {
				forLoopParameters[curThreadNo].initialValue	= forLoopParameters[curThreadNo-1].finalValue + increment;
			}
			forLoopParameters[curThreadNo].finalValue		= forLoopParameters[curThreadNo].initialValue + (chunkSize-1) * increment;
			break;
		case TM_SCHEDULE_DYNAMIC:
			return TM_RETURN_VALUE_INVALID_PARAM;
			break;
		case TM_SCHEDULE_GUIDED:
			return TM_RETURN_VALUE_INVALID_PARAM;
			break;
		case TM_SCHEDULE_RUNTIME:
			return TM_RETURN_VALUE_INVALID_PARAM;
			break;
		}

		// create suspended thread
		threads[curThreadNo].hThread = CreateThread(NULL, dwStackSize, threadForLoop, (LPVOID) (&forLoopParameters[curThreadNo]), CREATE_SUSPENDED, &threads[curThreadNo].threadId);
		SetThreadPriority(threads[curThreadNo].hThread, THREAD_PRIORITY_BELOW_NORMAL);
		if (threads[curThreadNo].hThread == NULL) {
			for (curThreadNo; curThreadNo>0; curThreadNo--) {
				CloseHandle(threads[curThreadNo-1].hThread);
				threads[curThreadNo-1].hThread = NULL;
			}
			return TM_RETURN_VALUE_UNEXPECTED_ERROR;
		}
	}

	// start threads, but don't resume if in pause mode
	for (curThreadNo=0; curThreadNo<numThreads; curThreadNo++) {
		if (!executionPaused) ResumeThread(threads[curThreadNo].hThread);
	}

	// wait for every thread to end
	waitForAllThreadsToTerminate();

	// everything ok
	if (executionCancelled) {
		return TM_RETURN_VALUE_EXECUTION_CANCELLED;
	} else {
		return TM_RETURN_VALUE_OK;
	}
}

//-----------------------------------------------------------------------------
// Name: threadForLoop()
// Desc: 
//-----------------------------------------------------------------------------
DWORD WINAPI threadManagerClass::threadForLoop(LPVOID lpParameter)
{
	// locals
	forLoopStruct *		forLoopParameters		= (forLoopStruct *) lpParameter;
	int64_t				index;

	switch (forLoopParameters->scheduleType)
	{
	case TM_SCHEDULE_STATIC: 
		// loop through the iterations 
		for (index=forLoopParameters->initialValue; (forLoopParameters->increment < 0) ? index >= forLoopParameters->finalValue : index <= forLoopParameters->finalValue; index += forLoopParameters->increment) {
			// check if this is the last iteration
			if (index == forLoopParameters->finalValue) {
				forLoopParameters->threadManager->anyThreadOnLastIteration = true;
			}
			// call the user function
			switch (forLoopParameters->threadProc(forLoopParameters->pParameter, index))
			{
			case TM_RETURN_VALUE_OK:
				break;
			case TM_RETURN_VALUE_TERMINATE_ALL_THREADS:
				forLoopParameters->threadManager->terminateAllThreads = true;
				break;
			default:
				break;
			}
			// check if the execution was cancelled
			if (forLoopParameters->threadManager->terminateAllThreads) break;
		}
		break;
	case TM_SCHEDULE_DYNAMIC:
		return TM_RETURN_VALUE_INVALID_PARAM;
		break;
	case TM_SCHEDULE_GUIDED:
		return TM_RETURN_VALUE_INVALID_PARAM;
		break;
	case TM_SCHEDULE_RUNTIME:
		return TM_RETURN_VALUE_INVALID_PARAM;
		break;
	}

	return TM_RETURN_VALUE_OK;
}
