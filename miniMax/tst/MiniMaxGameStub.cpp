/*********************************************************************
	MiniMaxGameStub.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/

#include "MiniMaxGameStub.h"
#include <algorithm>
#include <sstream>

using miniMax::twoBit;
using miniMax::plyInfoVarType;
using miniMax::stateAdressStruct;
using miniMax::stateNumberVarType;
using miniMax::gameInterface;
using miniMax::database::database;
using miniMax::retroAnalysis::predVars;
using miniMax::PLYINFO_VALUE_INVALID;
using miniMax::PLYINFO_EXP_VALUE;
using miniMax::SKV_VALUE_INVALID;
using miniMax::SKV_VALUE_GAME_LOST;
using miniMax::SKV_VALUE_GAME_DRAWN;
using miniMax::SKV_VALUE_GAME_WON;

#pragma region testGraph
testGraph::testGraph()
{
    assert(!anyDuplicateKnot());
}

bool testGraph::anyDuplicateKnot()
{
    for (auto& knot : knots) {
        for (auto& knot2 : knots) {
            if (&knot != &knot2 && knot.layerNumber == knot2.layerNumber && knot.stateNumber == knot2.stateNumber) {
                return true;
            }
        }
    }
    return false;
}

testGraph::testKnot& testGraph::getKnot(unsigned int layerNumber, unsigned int stateNumber)
{
    for (auto& knot : knots) {
        if (knot.layerNumber == layerNumber && knot.stateNumber == stateNumber) {
            return knot;
        }
    }
    return knots[0];
}

bool testGraph::doesKnotExist(unsigned int layerNumber, unsigned int stateNumber)
{
    for (auto& knot : knots) {
        if (knot.layerNumber == layerNumber && knot.stateNumber == stateNumber) {
            return true;
        }
    }
    return false;
}

unsigned int testGraph::getNumExpectedStates(unsigned int layerNumber, twoBit expValue)
{
    unsigned int num = 0;
    for (auto& knot : knots) {
        if (knot.layerNumber == layerNumber && knot.expValue == expValue) {
            num++;
        }
    }
    return num;
}

#pragma endregion

#pragma region testGame
testGame::testGame()
{
    setNumberOfThreads(numThreads);
}

void testGame::checkWithDatabase(database& db, const vector<unsigned int>& layersCalculated)
{
    // locals
	twoBit actKnotValue, expKnotValue;
	plyInfoVarType actPlyInfo, expPlyInfo;

	// check database
	// EXPECT_FALSE(db.isComplete());
	EXPECT_EQ(db.getNumLayers(), getNumberOfLayers());
	for (auto& layer : layersCalculated) {
		// EXPECT_TRUE(db.isLayerCompleteAndInFile(layer));
		EXPECT_EQ(db.getNumberOfKnots(layer), 		graph.knots.size());
		EXPECT_EQ(db.getNumberOfKnots(layer), 		getNumberOfKnotsInLayer(layer));
		EXPECT_EQ(db.getNumDrawnStates(layer), 		graph.getNumExpectedStates(layer, graph.PLY_DRAW));
		EXPECT_EQ(db.getNumLostStates(layer), 		graph.getNumExpectedStates(layer, graph.SKV_LOST));
		EXPECT_EQ(db.getNumWonStates(layer), 		graph.getNumExpectedStates(layer, graph.SKV_WON));
		EXPECT_EQ(db.getNumInvalidStates(layer), 	graph.getNumExpectedStates(layer, graph.SKV_INV));
	}

	// check values of states
	for (auto& knot : graph.knots) {
		db.readKnotValueFromDatabase(knot.layerNumber, knot.stateNumber, actKnotValue);
		db.readPlyInfoFromDatabase  (knot.layerNumber, knot.stateNumber, actPlyInfo);
		EXPECT_EQ(actKnotValue, knot.expValue);
		EXPECT_EQ(actPlyInfo, 	knot.expPlyInfo);
	}
}

void testGame::setNumberOfThreads(unsigned int numThreads)
{ 
    this->numThreads = numThreads; 
    states.resize(numThreads);
    movementBackups.resize(numThreads);
}

void testGame::prepareCalculation()
{
}

bool testGame::shallRetroAnalysisBeUsed(unsigned int layerNum)
{
    return true;
}

void testGame::getPossibilities(unsigned int threadNo, vector<unsigned int>& possibilityIds)
{
    // if thread number is invalid, return empty vector
    if (threadNo >= states.size()) return;

    auto& knot = graph.getKnot(states[threadNo].layerNumber, states[threadNo].stateNumber);
    for (unsigned int possibilityId = 0; possibilityId < knot.successors.size(); possibilityId++) {
        possibilityIds.push_back(possibilityId);
    }
}

unsigned int testGame::getMaxNumPossibilities()
{
    unsigned int maxNumPossibilities = 0;
    for (auto& knot : graph.knots) {
        if (knot.successors.size() > maxNumPossibilities) {
            maxNumPossibilities = knot.successors.size();
        }
    }
    return maxNumPossibilities;
}

unsigned int testGame::getNumberOfLayers() 
{
    // find the highest layer number
    unsigned int maxLayer = 0;
    for (auto& knot : graph.knots) {
        if (knot.layerNumber > maxLayer) {
            maxLayer = knot.layerNumber;
        }
    }
    return maxLayer + 1;
};

unsigned int testGame::getMaxNumPlies()
{
    unsigned int maxNumPlies = 0;
    for (auto& knot : graph.knots) {
        if (knot.expPlyInfo > maxNumPlies && knot.expPlyInfo < PLYINFO_EXP_VALUE ) {
            maxNumPlies = knot.expPlyInfo;
        }
    }
    return maxNumPlies;
}

unsigned int testGame::getNumberOfKnotsInLayer(unsigned int layerNum)	
{
    // if layer number is invalid, return 0
    if (layerNum >= getNumberOfLayers()) return 0;

    // count the number of knots in the layer
    unsigned int numKnots = 0;
    for (auto& knot : graph.knots) {
        if (knot.layerNumber == layerNum) {
            numKnots++;
        }
    }
    return numKnots;
};

void testGame::getSuccLayers(unsigned int layerNum, vector<unsigned int>& succLayers)
{
    // if layer number is invalid, return empty vector
    if (layerNum >= getNumberOfLayers()) return;

    // find the successors of the layer
    for (auto& knot : graph.knots) {
        if (knot.layerNumber == layerNum) {
            for (auto& successor : knot.successors) {
                // add the successor layer to the list, if it is not already in the list
                if (find(succLayers.begin(), succLayers.end(), successor.layerNumber) == succLayers.end()) {
                    succLayers.push_back(successor.layerNumber);
                }
            }
        }
    }
};

gameInterface::uint_1d testGame::getPartnerLayers(unsigned int layerNum) 
{ 
    if (layerNum >= partnerLayers.size()) return gameInterface::uint_1d{};
    return partnerLayers[layerNum];
};

void testGame::getValueOfSituation(unsigned int threadNo, float& floatValue, twoBit& curStateValue) 
{
    // if thread number is invalid, return invalid
    if (threadNo >= states.size()) {
        curStateValue = SKV_VALUE_INVALID;
        return;
    }

    // find the knot
    if (!graph.doesKnotExist(states[threadNo].layerNumber, states[threadNo].stateNumber)) {
        curStateValue = SKV_VALUE_INVALID;
        return;
    }
    curStateValue = graph.getKnot(states[threadNo].layerNumber, states[threadNo].stateNumber).value; 
}

void testGame::getLayerAndStateNumber(unsigned int threadNo, unsigned int& layerNum, unsigned int& stateNumber, unsigned int& symOp)
{
    // if thread number is invalid, return invalid
    if (threadNo >= states.size()) {
        layerNum = 0;
        stateNumber = 0;
        symOp = 0;
        return;
    }

    if (graph.doesKnotExist(states[threadNo].layerNumber, states[threadNo].stateNumber)) {
        layerNum    = states[threadNo].layerNumber;
        stateNumber = states[threadNo].stateNumber;
        symOp       = 0;
        return;
    } else {
        layerNum    = 0;
        stateNumber = 0;
        symOp       = 0;
    }
}

unsigned int testGame::getLayerNumber(unsigned int threadNo) 
{
    return states[threadNo].layerNumber;
};

void testGame::getSymStateNumWithDuplicates(unsigned int threadNo, vector<miniMax::stateAdressStruct>& symStates)
{
    if (!graph.doesKnotExist(states[threadNo].layerNumber, states[threadNo].stateNumber)) return;
    auto& knot = graph.getKnot(states[threadNo].layerNumber, states[threadNo].stateNumber);
    for (auto& symState : knot.symmetricStates) {
        symStates.push_back(stateAdressStruct{symState.stateNumber, symState.layerNumber});
    }
}

void testGame::getPredecessors(unsigned int threadNo, vector<predVars>& pv)
{
    // if thread number is invalid, return false
    if (threadNo >= states.size()) return;

    if (graph.doesKnotExist(states[threadNo].layerNumber, states[threadNo].stateNumber)) {
        auto& knotPredecessors = graph.getKnot(states[threadNo].layerNumber, states[threadNo].stateNumber).predecessors;
        pv.resize(knotPredecessors.size());
        for (unsigned int i = 0; i < knotPredecessors.size(); i++) {
            pv[i].predLayerNumber       = knotPredecessors[i].layerNumber;
            pv[i].predStateNumber       = knotPredecessors[i].stateNumber;
            pv[i].predSymOperation      = 0;
            pv[i].playerToMoveChanged   = knotPredecessors[i].playerToMoveHasChanged;
        }
    }
}

bool testGame::isStateIntegrityOk(unsigned int threadNo)
{
    return true;
}

void testGame::applySymOp(unsigned int threadNo, unsigned char symmetryOperationNumber, bool doInverseOperation, bool playerToMoveChanged)
{
    // TODO: implement
}

bool testGame::lostIfUnableToMove(unsigned int threadNo)
{
    return true;
}

bool testGame::setSituation(unsigned int threadNo, unsigned int layerNumber, stateNumberVarType stateNumber) 
{
    // if thread number is invalid, return false
    if (threadNo >= states.size()) return false;

    // remember state for this thread
    states[threadNo].layerNumber = layerNumber;
    states[threadNo].stateNumber = stateNumber;

    // if state is invalid, return false
    if (graph.getKnot(layerNumber, stateNumber).value == SKV_VALUE_INVALID) {
        return false;
    }
    return true;
}

void testGame::move(unsigned int threadNo, unsigned int idPossibility, bool& playerToMoveChanged, void* &pBackup)
{
    // if thread number is invalid, return false
    if (threadNo >= states.size()) return;
    testGraph::ts& curState = states[threadNo];

    // find the knot
    auto& knot = graph.getKnot(curState.layerNumber, curState.stateNumber);

    // if possibility is valid, move to the successor and make a backup
    if (idPossibility < knot.successors.size()) {
        pBackup                     = &movementBackups[threadNo];
        movementBackups[threadNo]   = testGraph::ts{curState.layerNumber, curState.stateNumber, knot.successors[idPossibility].playerToMoveHasChanged};
        curState.layerNumber        = knot.successors[idPossibility].layerNumber;
        curState.stateNumber        = knot.successors[idPossibility].stateNumber;
        playerToMoveChanged         = knot.successors[idPossibility].playerToMoveHasChanged;
    }
}

void testGame::undo(unsigned int threadNo, unsigned int idPossibility, bool& playerToMoveChanged, void*  pBackup)
{
    // if thread number is invalid, return false
    if (threadNo >= states.size()) return;

    // if backup is invalid, return false
    if (pBackup == nullptr) return;
    testGraph::ts* backupState = (testGraph::ts*) pBackup;

    // restore the state
    states[threadNo].layerNumber    = backupState->layerNumber;
    states[threadNo].stateNumber    = backupState->stateNumber;
    playerToMoveChanged             = backupState->playerToMoveHasChanged;
}

wstring testGame::getOutputInformation(unsigned int layerNum) 
{
    wstringstream wss;
    wss << L"8,9";
    return wss.str();
}

void testGame::printMoveInformation(unsigned int threadNo, unsigned int idPossibility)
{
}

void testGame::printField(unsigned int threadNo, twoBit value, unsigned int indentSpaces)
{
}
#pragma endregion

#pragma region MiniMaxTestGameFixture

MiniMaxTestGameFixture::MiniMaxTestGameFixture(const string folderName)
    : tmpFileDirectory((std::filesystem::temp_directory_path() / "wildWeasel" / folderName).c_str())
{
}

void MiniMaxTestGameFixture::SetUp()
{
    // create folder
    if (std::filesystem::exists(tmpFileDirectory)) {
        std::filesystem::remove_all(tmpFileDirectory);
    }
    std::filesystem::create_directories(tmpFileDirectory);

    // create database
    db.openDatabase(tmpFileDirectory);

    // create thread manager
    tm.setNumThreads(numThreads);
    game.setNumberOfThreads(numThreads);
}

void MiniMaxTestGameFixture::TearDown() 
{
    db.closeDatabase();
}

#pragma endregion 
