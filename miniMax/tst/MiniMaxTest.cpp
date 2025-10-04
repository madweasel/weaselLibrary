/**************************************************************************************************************************
	miniMaxTest.cpp
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
***************************************************************************************************************************/
#include "gtest/gtest.h"
#include "miniMax/src/miniMax.h"
#include "miniMax/src/typeDef.h"
#include "miniMax/tst/MiniMaxGameStub.h"

#include <numeric>

namespace miniMax
{

#pragma region gameInterface
class MiniMaxMainTest : public MiniMaxTestGameFixture {
protected:
	MiniMaxMainTest(): MiniMaxTestGameFixture("miniMax") {
		db.closeDatabase();
	}
};

TEST_F(MiniMaxMainTest, calculateDatabase) 
{
	// locals
	miniMax mm{&game, 4};
	unsigned int choice = 11233;
	stateInfo infoAboutChoices;

	EXPECT_TRUE(mm.setOutputStream(std::wcout));						// set output stream
	EXPECT_TRUE(mm.getBestChoice(choice, infoAboutChoices));			// no state set yet, but still a move suggestion should be available
	EXPECT_EQ(choice, 0);												// best choice should be 0 by default
	EXPECT_TRUE(mm.setNumThreads(numThreads));							// set number of threads
	EXPECT_FALSE(mm.isCurrentStateInDatabase(0));						// state should not be in database yet
	EXPECT_TRUE(mm.openDatabase(tmpFileDirectory));						// reopen database
	EXPECT_FALSE(mm.isCurrentStateInDatabase(0));						// state should not be in database yet
	EXPECT_TRUE(mm.calculateDatabase());								// calculate database now
	EXPECT_FALSE(mm.wasDatabaseCalculationCancelled());					// calculation should was not cancelled
	std::vector<unsigned int> layerNumbers(game.getNumberOfLayers());	// vector with layer numbers
	std::iota(std::begin(layerNumbers), std::end(layerNumbers), 0);		// fill with 0, 1, 2, ...
	db.closeDatabase();													// close database
	db.openDatabase(tmpFileDirectory);									// reopen database
	game.checkWithDatabase(db, layerNumbers);							// check if database is correct
	EXPECT_TRUE(db.isComplete());										// database should be complete
	for (auto& layer : layerNumbers) {			
		EXPECT_TRUE(db.isLayerCompleteAndInFile(layer));				// all layers should be complete
	}
	EXPECT_TRUE(mm.anyFreshlyCalculatedLayer());						// there should be a freshly calculated layer
	EXPECT_EQ(mm.getLastCalculatedLayer(), 0);							// there is only one layer
	EXPECT_FALSE(mm.anyFreshlyCalculatedLayer());						// there should be no freshly calculated layer
	EXPECT_EQ(mm.getLastCalculatedLayer(), game.getNumberOfLayers());	// there is no freshly calculated layer, so return invalid layer number
	game.setSituation(0, 0, 2);											// set drawn state
	EXPECT_TRUE(mm.isCurrentStateInDatabase(0));						// check if state is in database
	mm.closeDatabase();													// close database
	EXPECT_FALSE(mm.isCurrentStateInDatabase(0));						// database currently closed
	EXPECT_TRUE(mm.openDatabase(tmpFileDirectory));						// reopen database
	EXPECT_TRUE(mm.isCurrentStateInDatabase(0));						// check if state is in database
	EXPECT_TRUE(mm.getBestChoice(choice, infoAboutChoices));			// a movce suggestion should be available now
	EXPECT_EQ(0, choice);												// best choice should be 0, since it is the only one
	EXPECT_TRUE(mm.calculateDatabase());								// expect true, although the database is already complete
	EXPECT_EQ(mm.getNumThreads(), numThreads);							// number of threads should be the same
	EXPECT_FALSE(mm.anyFreshlyCalculatedLayer());						// there should be no freshly calculated layer
}

#pragma endregion

} // namespace miniMax
