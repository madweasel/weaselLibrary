/**************************************************************************************************************************
	MiniMaxDatabaseTest.cpp
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
***************************************************************************************************************************/
#include "gtest/gtest.h"
#include "weaselEssentials/src/threadManager.h"
#include "weaselEssentials/src/logger.h"
#include "miniMax/src/database/database.h"
#include "miniMax/src/retroAnalysis/stateQueue.h"
#include "miniMax/src/retroAnalysis/successorCountArray.h"
#include "miniMax/src/retroAnalysis/retroAnalysis.h"
#include "miniMax/src/typeDef.h"
#include "miniMax/tst/MiniMaxGameStub.h"

#include <filesystem>
#include <numeric>

using namespace miniMax;

#pragma region stateQueue
class MiniMaxRetroAnalysis_stateQueue_Test : public ::testing::Test {

protected:
	logger 						log					{logger::logLevel::none, logger::logType::none, L""};
	const wstring 				tmpFileDirectory 	= (std::filesystem::temp_directory_path() / "wildWeasel" / "stateQueue").c_str();
	const unsigned int 			threadNo 			= 3;
	retroAnalysis::stateQueue 	sq					{log, tmpFileDirectory, threadNo};

	virtual void SetUp() override {
		if (std::filesystem::exists(tmpFileDirectory)) {
			std::filesystem::remove_all(tmpFileDirectory);
		}
		std::filesystem::create_directories(tmpFileDirectory);
	}
	virtual void TearDown() override {}
};

TEST_F(MiniMaxRetroAnalysis_stateQueue_Test, stateQueue) 
{
	stateAdressStruct actState, expState;
	
	actState = stateAdressStruct{11, 22};
	EXPECT_FALSE(sq.pop_front(actState, 0));						// empty queue
	expState = stateAdressStruct{11, 22};				
	EXPECT_EQ(actState, expState);									// nothing changed
	EXPECT_EQ(sq.size(0), 0);										// empty queue
	EXPECT_EQ(sq.getNumStatesToProcess(), 0);						// empty queue
	EXPECT_EQ(sq.getMaxPlyInfoValue(), 0);							// empty queue
	EXPECT_FALSE(sq.push_back(actState, 0, 0));						// empty queue
	EXPECT_FALSE(sq.push_back(stateAdressStruct{33, 44}, 0, 99));	// fail, since not resized
	EXPECT_TRUE(sq.resize(0, 99));									// resize to 100 states for ply 0
	EXPECT_TRUE(sq.push_back(stateAdressStruct{33, 44}, 0, 99));	// add one state with ply 0
	EXPECT_EQ(sq.size(0), 1);										// one state in layer 0
	EXPECT_EQ(sq.getNumStatesToProcess(), 1);						// one state in total
	EXPECT_EQ(sq.getMaxPlyInfoValue(), 0);							// max ply is still 0
	EXPECT_TRUE(sq.pop_front(actState, 0));							// remove one state
	expState = stateAdressStruct{33, 44};				
	EXPECT_EQ(actState, expState);									// state is correct
	EXPECT_EQ(sq.size(0), 0);										// empty queue
	EXPECT_EQ(sq.getNumStatesToProcess(), 0);						// empty queue
	EXPECT_EQ(sq.getMaxPlyInfoValue(), 0);							// empty queue
	EXPECT_FALSE(sq.pop_front(actState, 0));						// empty queue
	EXPECT_FALSE(sq.resize(PLYINFO_EXP_VALUE, 99));				// resize to 100 states for ply 0
	EXPECT_TRUE(sq.resize(1, 99));
	EXPECT_TRUE(sq.resize(3, 99));
	EXPECT_TRUE(sq.push_back(stateAdressStruct{55, 66}, 1, 99));	// add one state with ply 1
	EXPECT_TRUE(sq.push_back(stateAdressStruct{77, 88}, 1, 99));	// add one state with ply 1
	EXPECT_FALSE(sq.push_back(stateAdressStruct{99, 00}, 3, 99));	// fail to push back, because the cyclic array is too small
	EXPECT_TRUE(sq.push_back(stateAdressStruct{98, 00}, 3, 99));	// add one state with ply 3
	EXPECT_EQ(sq.size(1), 2);										// two states with ply 1
	EXPECT_EQ(sq.size(3), 1);										// one state with ply 3
	EXPECT_EQ(sq.getNumStatesToProcess(), 3);						// three states in total
	EXPECT_EQ(sq.getMaxPlyInfoValue(), 3);							// max ply is 3
	EXPECT_TRUE(sq.pop_front(actState, 1));							// remove one state with ply 1
	expState = stateAdressStruct{55, 66};
	EXPECT_EQ(actState, expState);									// state is correct
	EXPECT_EQ(sq.size(1), 1);										// one state with ply 1
	EXPECT_EQ(sq.size(3), 1);										// one state with ply 3
	EXPECT_EQ(sq.getNumStatesToProcess(), 2);						// two states in total
	EXPECT_EQ(sq.getMaxPlyInfoValue(), 3);							// max ply is 3
	EXPECT_TRUE(sq.resize(8, 9));
	EXPECT_TRUE(sq.push_back(stateAdressStruct{11, 22}, 1, 99));	// add one state with ply 1
	EXPECT_FALSE(sq.push_back(stateAdressStruct{11, 22}, 8, 9));	// fail to push back, because the cyclic array is too small
	EXPECT_EQ(sq.size(1), 2);										// two states with ply 1
	EXPECT_EQ(sq.size(3), 1);										// one state with ply 3
	EXPECT_EQ(sq.getNumStatesToProcess(), 3);						// three states in total
	EXPECT_EQ(sq.getMaxPlyInfoValue(), 3);							// max ply is 3
	EXPECT_TRUE(sq.pop_front(actState, 1));							// remove one state with ply 1
	expState = stateAdressStruct{77, 88};
	EXPECT_EQ(actState, expState);									// state is correct
	EXPECT_EQ(sq.size(1), 1);										// one state with ply 1
	EXPECT_EQ(sq.size(3), 1);										// one state with ply 3
	EXPECT_EQ(sq.getNumStatesToProcess(), 2);						// two states in total
	EXPECT_EQ(sq.getMaxPlyInfoValue(), 3);							// max ply is 3
	EXPECT_TRUE(sq.pop_front(actState, 1));							// remove one state with ply 1
	expState = stateAdressStruct{11, 22};
	EXPECT_EQ(actState, expState);									// state is correct
	EXPECT_EQ(sq.size(1), 0);										// empty queue
	EXPECT_EQ(sq.size(3), 1);										// one state with ply 3
	EXPECT_EQ(sq.getNumStatesToProcess(), 1);						// one state in total
	EXPECT_EQ(sq.getMaxPlyInfoValue(), 3);							// max ply is 3
	EXPECT_TRUE(sq.pop_front(actState, 3));							// remove one state with ply 3
	expState = stateAdressStruct{98, 00};
	EXPECT_EQ(actState, expState);									// state is correct
	EXPECT_EQ(sq.size(1), 0);										// empty queue
	EXPECT_EQ(sq.size(3), 0);										// empty queue
	EXPECT_EQ(sq.getNumStatesToProcess(), 0);						// empty queue
	EXPECT_EQ(sq.getMaxPlyInfoValue(), 3);							// max ply is 3
	EXPECT_FALSE(sq.pop_front(actState, 3));						// empty queue
	EXPECT_FALSE(sq.pop_front(actState, 1));						// empty queue
	EXPECT_FALSE(sq.pop_front(actState, 0));						// empty queue
	EXPECT_EQ(sq.size(1), 0);										// empty queue
	EXPECT_EQ(sq.size(3), 0);										// empty queue
	EXPECT_EQ(sq.getNumStatesToProcess(), 0);						// empty queue
	EXPECT_EQ(sq.getMaxPlyInfoValue(), 3);							// max ply is 3
}
#pragma endregion

#pragma region successorCountArray
class MiniMaxRetroAnalysis_successorCountArray : public MiniMaxTestGameFixture {
protected:
	MiniMaxRetroAnalysis_successorCountArray(): MiniMaxTestGameFixture("successorCountArray") {}
};

TEST_F(MiniMaxRetroAnalysis_successorCountArray, successorCountArray) 
{
	retroAnalysis::successorCountArray sca(log, db, 0);

	EXPECT_EQ(sca.getLayerNumber(), 0);
	EXPECT_EQ(sca.increaseCounter(0), 1);
	EXPECT_EQ(sca.increaseCounter(0), 2);
	EXPECT_EQ(sca.increaseCounter(0), 3);
	EXPECT_EQ(sca.decreaseCounter(0), 2);
	EXPECT_EQ(sca.decreaseCounter(0), 1);
	EXPECT_EQ(sca.decreaseCounter(0), 0);
	EXPECT_EQ(sca.decreaseCounter(0), COUNT_ARRAY_MAX_VALUE);
}
#pragma endregion

#pragma region successorCountManager
class MiniMaxRetroAnalysis_successorCountManager : public MiniMaxTestGameFixture {
protected:
	MiniMaxRetroAnalysis_successorCountManager(): MiniMaxTestGameFixture("successorCountManager") {}
};

TEST_F(MiniMaxRetroAnalysis_successorCountManager, successorCountArray) 
{
	// locals
	vector<retroAnalysis::stateQueue> statesToProcess;
	retroAnalysis::successorCountManager scm(log, tm, db, game, statesToProcess);
	std::vector<unsigned int> layerNumbers(game.getNumberOfLayers());
	std::iota(std::begin(layerNumbers), std::end(layerNumbers), 0);		// fill with 0, 1, 2, ...

	if (statesToProcess.size() != tm.getNumThreads()) {
		statesToProcess.clear();
		statesToProcess.reserve(tm.getNumThreads());
		for (unsigned int threadNo=0; threadNo<tm.getNumThreads(); threadNo++) {
			statesToProcess.push_back(retroAnalysis::stateQueue(log, db.getFileDirectory(), threadNo));
			for (auto& layer : layerNumbers) {
				for (unsigned int plyNumber = 0; plyNumber < game.getMaxNumPlies(); plyNumber++) {
					statesToProcess.back().resize(plyNumber, db.getNumberOfKnots(layer));
				}
			}
		}
	}

	// Init database according to game graph
	for (auto& knot : game.graph.knots) {
		db.writeKnotValueInDatabase(knot.layerNumber, knot.stateNumber, knot.value);
		db.writePlyInfoInDatabase(knot.layerNumber, knot.stateNumber, knot.expPlyInfo);
	}

	// run two passes. 1st is to calculate and 2nd to read from file
	for (unsigned int pass = 0; pass < 2; pass++) {

		// init
		EXPECT_TRUE(scm.init(layerNumbers));
		EXPECT_TRUE(scm.isReady());
		
		// check counters
		for (auto& layerNumber : layerNumbers) {
			for (unsigned int stateNumber = 0; stateNumber < db.getNumberOfKnots(layerNumber); stateNumber++) {
				countArrayVarType expectedCounter = game.graph.getKnot(layerNumber, stateNumber).successors.size();
				if (!expectedCounter) {
					expectedCounter = COUNT_ARRAY_MAX_VALUE;
				} else {
					expectedCounter--;
				}
				EXPECT_EQ(scm.getAndDecreaseCounter(layerNumber, stateNumber), expectedCounter);
			}
		}
	}
}

#pragma endregion

#pragma region solver
class MiniMaxRetroAnalysis_solver : public MiniMaxTestGameFixture {
protected:
	MiniMaxRetroAnalysis_solver(): MiniMaxTestGameFixture("retroAnalysisSolver") {}
	retroAnalysis::solver	solver{log, tm, db, game};
};

TEST_F(MiniMaxRetroAnalysis_solver, positiveTests)
{
	vector<unsigned int> layersToCalculate = {0};

	EXPECT_TRUE(solver.calcKnotValuesByRetroAnalysis(layersToCalculate));

	game.checkWithDatabase(db, layersToCalculate);
	
	EXPECT_FALSE(db.isComplete());
	for (auto& layer : layersToCalculate) {
		EXPECT_FALSE(db.isLayerCompleteAndInFile(layer));
	}
}
#pragma endregion
