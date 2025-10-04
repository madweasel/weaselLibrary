/**************************************************************************************************************************
	threadManagerTest.cpp
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
***************************************************************************************************************************/
#include "gtest/gtest.h"
#include <vector>
#include <thread>
#include <random>
#include <mutex>

#include "threadManager.h"

// test fixture
class ThreadManagerTest : public ::testing::Test {
protected:
	threadManagerClass tm;

	void SetUp() override {
		tm.setNumThreads(2);
	}

	void TearDown() override {
	}
};

// strcuture used by threads to access parent data
struct defaultThreadVars : public threadManagerClass::threadVarsArrayItem
{
	threadManagerClass *	pTm;			// pointer to the threadManagerClass object
	int*					pValues;		// pointer to an array, where all threads write their results to
	std::mutex*				pMutex;			// pointer to a mutex for synchronizing access to pValues

						defaultThreadVars	(defaultThreadVars const& master) : pTm(master.pTm), pValues(master.pValues), pMutex(master.pMutex) {};
						defaultThreadVars	(threadManagerClass *pTm, int* pValues, std::mutex* pMutex) : pTm(pTm), pValues(pValues), pMutex(pMutex) {};
	void				reduce				()									{  }; 
};

// control thread, coordinating the other threads
void threadControl(void* pParameter) {
	threadManagerClass* tm = (threadManagerClass *) pParameter;
	EXPECT_EQ(tm->anyThreadRunning(), false);		// no thread should be running yet
	Sleep(500);										// wait for the threads to start					
	EXPECT_EQ(tm->anyThreadRunning(), true);		// check if the threads are running
	EXPECT_EQ(tm->setNumThreads(2), true);			// try to set the number of threads while the threads are running
	EXPECT_EQ(tm->setNumThreads(4), false);			// try to set the number of threads while the threads are running
	EXPECT_EQ(tm->getThreadNumber(), 2);			// an extern thread has no thread number, thus the function should return the number of threads
	tm->pauseExecution();							// pause the execution
	Sleep(100);										// wait for the threads to pause
	tm->pauseExecution();							// resume the execution
	EXPECT_EQ(tm->anyThreadRunning(), true);		// check if the threads are running
	Sleep(100);										// wait again
	tm->cancelExecution();							// cancel the execution
	EXPECT_EQ(tm->wasExecutionCancelled(), true);	// check if the execution was cancelled
	tm->waitForAllThreadsToTerminate();				// wait for the threads to terminate
	EXPECT_EQ(tm->anyThreadRunning(), false);		// now no thread should be running
}

DWORD threadProc_1(void* pParameter) {
	return 0;
}

DWORD threadProc_2(void* pParameter, int64_t index) {
	defaultThreadVars *			tVars		= (defaultThreadVars *) pParameter;
	threadManagerClass*			tm			= tVars->pTm;
	Sleep(100);
	if (index > 2 && tm->getThreadNumber() == 1) {
		tm->cancelExecution();
	}
	return 0;
}

DWORD threadProc_3(void* pParameter, int64_t index) {
	Sleep(100);
	return 0;
}

// Some threads reaches the barrier and waits for the other threads to reach the barrier
// The distance of the value of each thread is at maximum 1 since every thread is bound by the barrier
DWORD threadProc_4(void* pParameter, int64_t index) {
	defaultThreadVars *	tVars			= (defaultThreadVars *) pParameter;
	threadManagerClass*	tm				= tVars->pTm;
	unsigned int 		numThreads 		= tm->getNumThreads();
	unsigned int 		threadNumber 	= tm->getThreadNumber();
	
	// simulate a random calculation time
	DWORD msToWait = rand() % 50;
	if (msToWait > 20) {
		Sleep(msToWait);
	}
	{
		std::lock_guard<std::mutex> lock(*tVars->pMutex);
		tVars->pValues[threadNumber]++;

		// check if the distance to the other threads is at maximum 1
		for (unsigned int i=0; i<numThreads; i++) {
			EXPECT_LT(abs(tVars->pValues[threadNumber] - tVars->pValues[i]), 2);
		}
	}

	EXPECT_TRUE(tm->anyThreadRunning());
	EXPECT_FALSE(tm->wasExecutionCancelled());

	// wait for the other threads to reach the barrier
	tm->waitForOtherThreads();
	return 0;
}

TEST(ThreadManager, negativeTest) {
	SYSTEM_INFO		m_si		= {0};
	GetSystemInfo(&m_si);
	threadManagerClass tm;
	EXPECT_EQ(tm.getThreadNumber(), tm.getNumThreads());			// an extern thread has no thread number, thus the function should return the number of threads
	EXPECT_EQ(tm.getNumThreads(), m_si.dwNumberOfProcessors);		// the number of threads should be the number of processors
	EXPECT_EQ(tm.setNumThreads(0), false);							// the number of threads must be greater than 0
	EXPECT_EQ(tm.setNumThreads(4), true);							// set the number of threads
	EXPECT_EQ(tm.getNumThreads(), 4);								// check if the number of threads is set correctly
	EXPECT_EQ(tm.getThreadNumber(), 4);								// an extern thread has no thread number, thus the function should return the number of threads
	EXPECT_EQ(tm.wasExecutionCancelled(), false);					// the execution was not cancelled yet
	EXPECT_EQ(tm.anyThreadRunning(), false);						// no thread should be running yet

	auto vData = std::vector<int>(4);
	void* pParameter = static_cast<void*>(vData.data());
	unsigned int parameterStructSize = sizeof(vData[0]);

	EXPECT_EQ(tm.executeInParallel(NULL, NULL, 0), TM_RETURN_VALUE_INVALID_PARAM);								// invalid parameters
	EXPECT_EQ(tm.executeInParallel(NULL, pParameter, parameterStructSize), TM_RETURN_VALUE_INVALID_PARAM);		// no threadProc
	EXPECT_EQ(tm.executeInParallel(threadProc_1, NULL, parameterStructSize), TM_RETURN_VALUE_INVALID_PARAM);	// no pParameter
	EXPECT_EQ(tm.executeInParallel(threadProc_1, pParameter, 0), TM_RETURN_VALUE_OK);							// no parameterStructSize, but this is ok
}

TEST_F(ThreadManagerTest, positiveTest) {
	auto vData = std::vector<int>(2);

	EXPECT_EQ(tm.getNumThreads(), 2);					// check if the number of threads is set correctly
	EXPECT_EQ(tm.anyThreadRunning(), false);			// no thread should be running yet
	EXPECT_EQ(tm.executeInParallel(						// run two threads in parallel
		[](void* pParameter) -> DWORD {
			return 0;
		}, 
		static_cast<void*>(vData.data()), 
		sizeof(vData[0])), 
		TM_RETURN_VALUE_OK);
	EXPECT_EQ(tm.anyThreadRunning(), false);			// no thread should be running anymore
	EXPECT_EQ(tm.wasExecutionCancelled(), false);		// the execution was not cancelled
	tm.cancelExecution();								// cancel the execution
	EXPECT_EQ(tm.wasExecutionCancelled(), true);		// the execution was cancelled, even if the threads were not running
	EXPECT_EQ(tm.anyThreadRunning(), false);			// still no thread should be running 
}

TEST_F(ThreadManagerTest, cancelExecution) {
	threadManagerClass::threadVarsArray<defaultThreadVars> tva(tm.getNumThreads(), defaultThreadVars(&tm, NULL, NULL));

	// run two threads in parallel, one thread cancels the execution
	EXPECT_EQ(tm.executeParallelLoop(
		threadProc_2, 
		tva.getPointerToArray(), 
		tva.getSizeOfArray(),
		TM_SCHEDULE_STATIC,
		0,
		10000,
		1), TM_RETURN_VALUE_EXECUTION_CANCELLED);
	EXPECT_EQ(tm.anyThreadRunning(), false);
	EXPECT_EQ(tm.wasExecutionCancelled(), true);

	// create a control thread with std::thread which cancel the execution
	tm.reset();
	EXPECT_EQ(tm.anyThreadRunning(), false);
	EXPECT_EQ(tm.wasExecutionCancelled(), false);
	std::thread t1(threadControl, &tm);
	Sleep(100);										// wait for the control thread to start
	EXPECT_EQ(tm.executeParallelLoop(
		threadProc_3,
		tva.getPointerToArray(), 
		tva.getSizeOfArray(),
		TM_SCHEDULE_STATIC,
		0,
		10000,
		1), TM_RETURN_VALUE_EXECUTION_CANCELLED);
	EXPECT_EQ(tm.anyThreadRunning(), false);
	EXPECT_EQ(tm.wasExecutionCancelled(), true);
	t1.join();
}

TEST_F(ThreadManagerTest, waitForOthers) {
	const unsigned int numThreads = 2;
	tm.setNumThreads(numThreads);

	// create a global array where each thread writes its result to
	std::mutex valuesMutex;
	std::vector<int> values(numThreads, 0); 
	threadManagerClass::threadVarsArray<defaultThreadVars> tva(tm.getNumThreads(), defaultThreadVars(&tm, values.data(), &valuesMutex));

	// run threads in parallel, but the shall sync each iteration
	EXPECT_EQ(tm.executeParallelLoop(
		threadProc_4, 
		tva.getPointerToArray(), 
		tva.getSizeOfArray(),
		TM_SCHEDULE_STATIC,
		0,
		100*numThreads,
		1), TM_RETURN_VALUE_OK);
	EXPECT_EQ(tm.anyThreadRunning(), false);
	EXPECT_EQ(tm.wasExecutionCancelled(), false);
}
