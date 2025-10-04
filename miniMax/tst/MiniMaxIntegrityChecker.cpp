/**************************************************************************************************************************
	MiniMaxDatabaseTest.cpp
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
***************************************************************************************************************************/
#include "gtest/gtest.h"
#include "weaselEssentials/src/threadManager.h"
#include "weaselEssentials/src/logger.h"
#include "miniMax/src/miniMax.h"
#include "miniMax/src/database/database.h"
#include "miniMax/src/integrity/integrityChecker.h"
#include "miniMax/src/typeDef.h"
#include "miniMax/tst/MiniMaxGameStub.h"

#include <filesystem>

#pragma region gameInterface
class MiniMaxIntegrityChecker_gameInterface : public MiniMaxTestGameFixture {
protected:
	MiniMaxIntegrityChecker_gameInterface(): MiniMaxTestGameFixture("integrityChecker") {}
};

TEST_F(MiniMaxIntegrityChecker_gameInterface, gameStub) 
{
	miniMax::integrity::checker c{log, tm, db, game};

    for (auto& knot : game.graph.knots) {
        EXPECT_TRUE(c.testSetSituationAndGetStateNum(knot.layerNumber));
        EXPECT_TRUE(c.testMoveAndUndo(knot.layerNumber));
        EXPECT_TRUE(c.testGetPredecessors(knot.layerNumber));
        EXPECT_TRUE(c.testGetPossibilities(knot.layerNumber));
    }
}

TEST_F(MiniMaxIntegrityChecker_gameInterface, database) 
{
    GTEST_SKIP() << "Skipping 'all-in' test";

    miniMax::miniMax mm{&game, 10};

    // calculate database
    mm.openDatabase(tmpFileDirectory);
    mm.setNumThreads(3);
    mm.calculateDatabase();

    // test each state
    for (unsigned int layerNumber = 0; layerNumber < game.getNumberOfLayers(); layerNumber++) {
        for (unsigned int stateNumber = 0; stateNumber < game.getNumberOfKnotsInLayer(layerNumber); stateNumber++) {
            EXPECT_TRUE(mm.checker.testState(layerNumber, stateNumber));
        }
        EXPECT_TRUE(mm.checker.testLayer(layerNumber));
        EXPECT_TRUE(mm.checker.testIfSymStatesHaveSameValue(layerNumber));
    }
}

#pragma endregion
