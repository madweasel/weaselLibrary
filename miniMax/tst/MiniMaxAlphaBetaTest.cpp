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
#include "miniMax/src/alphaBeta/knotStruct.h"
#include "miniMax/src/alphaBeta/alphaBeta.h"
#include "miniMax/tst/MiniMaxGameStub.h"

#include <filesystem>

using namespace miniMax;

#pragma region stateInfo
TEST(MiniMaxAlphaBeta_stateInfo, updateBestAmountOfPlies)
{
	// locals
	stateInfo info;

	// test invalid state
	info.shortValue = SKV_VALUE_INVALID;
	info.updateBestAmountOfPlies();
	EXPECT_EQ(info.bestAmountOfPlies, PLYINFO_VALUE_INVALID);

	// test won state
	info.shortValue = SKV_VALUE_GAME_WON;
	info.choices.resize(3);
	info.choices[0].plyInfo = 1;	info.choices[0].shortValue = SKV_VALUE_GAME_WON;	// win in 1 move
	info.choices[1].plyInfo = 2;	info.choices[1].shortValue = SKV_VALUE_GAME_WON;	// win in 2 moves
	info.choices[2].plyInfo = 3;	info.choices[2].shortValue = SKV_VALUE_GAME_LOST;	// do not consider lost state
	info.updateBestAmountOfPlies();
	EXPECT_EQ(info.bestAmountOfPlies, 2);	// minimum 1+1 move to win

	// test lost state
	info.shortValue = SKV_VALUE_GAME_LOST;
	info.choices[0].plyInfo = 1;	info.choices[0].shortValue = SKV_VALUE_GAME_LOST;	// lost in 1 move
	info.choices[1].plyInfo = 2;	info.choices[1].shortValue = SKV_VALUE_GAME_LOST;	// lost in 2 moves
	info.choices[2].plyInfo = 3;	info.choices[2].shortValue = SKV_VALUE_GAME_LOST;	// lost in 3 moves
	info.updateBestAmountOfPlies();
	EXPECT_EQ(info.bestAmountOfPlies, 4);	// at maximum 3+1 moves to lose

	// test drawn state
	info.shortValue = SKV_VALUE_GAME_DRAWN;
	info.choices[0].plyInfo = 1;	info.choices[0].shortValue = SKV_VALUE_GAME_DRAWN;	info.choices[0].freqValuesSubMoves[SKV_VALUE_GAME_WON] = 1;
	info.choices[1].plyInfo = 2;	info.choices[1].shortValue = SKV_VALUE_GAME_DRAWN;	info.choices[1].freqValuesSubMoves[SKV_VALUE_GAME_WON] = 2;
	info.choices[2].plyInfo = 3;	info.choices[2].shortValue = SKV_VALUE_GAME_DRAWN;	info.choices[2].freqValuesSubMoves[SKV_VALUE_GAME_WON] = 3;
	info.updateBestAmountOfPlies();
	EXPECT_EQ(info.bestAmountOfPlies, 3);	// at maximum 3 winning possibilities on choice [2]
}
#pragma endregion

#pragma region knotStruct
class MiniMaxAlphaBeta_KnotStruct : public testing::Test {
protected:
	unsigned int 					maxNumPossibilities 	= 10;		// maximum number of possibilities/branches
	alphaBeta::knotStruct			knot;								// class being tested
	vector<alphaBeta::knotStruct> 	branchArray;						// array for storing branches

	void SetUp() override 
	{
		branchArray.resize(maxNumPossibilities);
		knot.initForCalculation(branchArray.data());
	}

	void TearDown() override 
	{
	}

	void testCalcKnotValue(
		unsigned int numPossibilities, 
		const vector<twoBit> values, 
		const vector<float> floatValues,
		const vector<bool> playerToMoveChanged, 
		twoBit expectedKnotValue,
		float expectedFloatValue)
	{
		knot.numPossibilities 	= numPossibilities;
		for (unsigned int i=0; i<numPossibilities; i++) {
			knot.branches[i].shortValue = values[i];
			knot.branches[i].floatValue = floatValues[i];
			knot.branches[i].playerToMoveChanged = playerToMoveChanged[i];
		}
		EXPECT_TRUE(knot.calcKnotValue());
		EXPECT_EQ(knot.shortValue, expectedKnotValue);
		EXPECT_EQ(knot.floatValue, expectedFloatValue);
	}

	void testCalcPlyInfo(
		unsigned int numPossibilities, 
		const vector<twoBit> values, 
		const vector<plyInfoVarType> plyInfos,
		const vector<bool> playerToMoveChanged, 
		plyInfoVarType expectedPlyInfo)
	{
		knot.numPossibilities 	= numPossibilities;
		for (unsigned int i=0; i<numPossibilities; i++) {
			knot.branches[i].shortValue = values[i];
			knot.branches[i].plyInfo = plyInfos[i];
			knot.branches[i].playerToMoveChanged = playerToMoveChanged[i];
		}
		EXPECT_TRUE(knot.calcKnotValue());
		EXPECT_TRUE(knot.calcPlyInfo());
		EXPECT_EQ(knot.plyInfo, expectedPlyInfo);
	}

	void testGetBestBranchesBasedOnSkvValue(
		unsigned int numPossibilities, 
		const twoBit shortValue,
		const vector<twoBit> values, 
		const vector<plyInfoVarType> plyInfos,
		const vector<bool> playerToMoveChanged, 
		const vector<unsigned int> expectedBestBranches)
	{
		vector<unsigned int> bestBranches;
		knot.numPossibilities 	= numPossibilities;
		knot.shortValue 		= shortValue;
		for (unsigned int i=0; i<numPossibilities; i++) {
			knot.branches[i].shortValue = values[i];
			knot.branches[i].plyInfo = plyInfos[i];
			knot.branches[i].playerToMoveChanged = playerToMoveChanged[i];
		}
		EXPECT_TRUE(knot.calcKnotValue());
		EXPECT_TRUE(knot.calcPlyInfo());
		EXPECT_TRUE(knot.getBestBranchesBasedOnSkvValue(bestBranches));
		EXPECT_EQ(bestBranches, expectedBestBranches);
	}

	void testGetBestBranchesBasedOnFloatValue(
		unsigned int numPossibilities, 
		float floatValue,
		const vector<float> branchValues,
		const vector<bool> playerToMoveChanged, 
		const vector<unsigned int> expectedBestBranches)
	{
		vector<unsigned int> bestBranches;
		knot.numPossibilities 	= numPossibilities;
		knot.floatValue 		= floatValue;
		for (unsigned int i=0; i<numPossibilities; i++) {
			knot.branches[i].shortValue 			= SKV_VALUE_GAME_DRAWN;
			knot.branches[i].floatValue 			= branchValues[i];
			knot.branches[i].playerToMoveChanged 	= playerToMoveChanged[i];
		}
		EXPECT_TRUE(knot.calcKnotValue());
		EXPECT_TRUE(knot.getBestBranchesBasedOnFloatValue(bestBranches));
		EXPECT_EQ(bestBranches, expectedBestBranches);
	}

	void testCanCutOff(
		unsigned int branchId,
		bool playerToMoveChanged,
		float& alpha,
		float& beta, 
		bool expectedReturnValue,
		float expectedAlpha,
		float expectedBeta)
	{
		knot.branches[branchId].playerToMoveChanged = playerToMoveChanged;
		EXPECT_EQ(knot.canCutOff(branchId, alpha, beta), expectedReturnValue);
		EXPECT_EQ(alpha, 	expectedAlpha);
		EXPECT_EQ(beta, 	expectedBeta);
	}
};

TEST_F(MiniMaxAlphaBeta_KnotStruct, calcKnotValue)
{
	EXPECT_FALSE(knot.calcKnotValue());

	testCalcKnotValue(1, {SKV_VALUE_GAME_WON}, 												{FPKV_MAX_VALUE}, 						{false}, 				SKV_VALUE_GAME_WON, 		FPKV_MAX_VALUE);
	testCalcKnotValue(1, {SKV_VALUE_GAME_LOST}, 											{FPKV_MIN_VALUE}, 						{false}, 				SKV_VALUE_GAME_LOST, 		FPKV_MIN_VALUE);
	testCalcKnotValue(1, {SKV_VALUE_GAME_DRAWN}, 											{0.0f}, 								{true}, 				SKV_VALUE_GAME_DRAWN, 		0.0f);
	testCalcKnotValue(1, {SKV_VALUE_INVALID}, 												{0.0f}, 								{true}, 				SKV_VALUE_INVALID, 			FPKV_INV_VALUE);
	testCalcKnotValue(1, {SKV_VALUE_GAME_LOST}, 											{FPKV_MIN_VALUE}, 						{false}, 				SKV_VALUE_GAME_LOST, 		FPKV_MIN_VALUE);
	testCalcKnotValue(1, {SKV_VALUE_GAME_LOST}, 											{FPKV_MIN_VALUE}, 						{true}, 				SKV_VALUE_GAME_WON, 		FPKV_MAX_VALUE);
	testCalcKnotValue(1, {SKV_VALUE_GAME_DRAWN}, 											{0.0f}, 								{true}, 				SKV_VALUE_GAME_DRAWN, 		0.0f);
	testCalcKnotValue(1, {SKV_VALUE_INVALID}, 												{0.0f}, 								{false}, 				SKV_VALUE_INVALID, 			FPKV_INV_VALUE);
	testCalcKnotValue(2, {SKV_VALUE_GAME_WON, SKV_VALUE_GAME_WON}, 							{FPKV_MAX_VALUE, FPKV_MAX_VALUE}, 		{true, true}, 			SKV_VALUE_GAME_LOST, 		FPKV_MIN_VALUE);
	testCalcKnotValue(2, {SKV_VALUE_GAME_WON, SKV_VALUE_GAME_LOST}, 						{FPKV_MAX_VALUE, FPKV_MIN_VALUE}, 		{true, true}, 			SKV_VALUE_GAME_WON, 		FPKV_MAX_VALUE);
	testCalcKnotValue(2, {SKV_VALUE_GAME_WON, SKV_VALUE_GAME_LOST}, 						{FPKV_MAX_VALUE, FPKV_MIN_VALUE}, 		{true, false}, 			SKV_VALUE_GAME_LOST, 		FPKV_MIN_VALUE);
	testCalcKnotValue(2, {SKV_VALUE_GAME_WON, SKV_VALUE_GAME_LOST}, 						{FPKV_MAX_VALUE, FPKV_MIN_VALUE}, 		{false, false}, 		SKV_VALUE_GAME_WON, 		FPKV_MAX_VALUE);
	testCalcKnotValue(2, {SKV_VALUE_GAME_WON, SKV_VALUE_GAME_LOST}, 						{FPKV_MAX_VALUE, FPKV_MIN_VALUE}, 		{false, true}, 			SKV_VALUE_GAME_WON, 		FPKV_MAX_VALUE);
	testCalcKnotValue(3, {SKV_VALUE_GAME_LOST, SKV_VALUE_GAME_DRAWN, SKV_VALUE_GAME_WON}, 	{FPKV_MIN_VALUE, 0.0f, FPKV_MAX_VALUE}, {true, true, true}, 	SKV_VALUE_GAME_WON, 		FPKV_MAX_VALUE);
	testCalcKnotValue(3, {SKV_VALUE_GAME_LOST, SKV_VALUE_INVALID, SKV_VALUE_GAME_WON}, 		{FPKV_MIN_VALUE, 0.0f, FPKV_MAX_VALUE}, {true, true, true}, 	SKV_VALUE_GAME_WON, 		FPKV_MAX_VALUE);
	testCalcKnotValue(3, {SKV_VALUE_GAME_LOST, SKV_VALUE_GAME_LOST, SKV_VALUE_GAME_DRAWN}, 	{FPKV_MIN_VALUE, FPKV_MIN_VALUE, 0.0f}, {true, true, true}, 	SKV_VALUE_GAME_WON, 		FPKV_MAX_VALUE);
	testCalcKnotValue(3, {SKV_VALUE_GAME_WON, SKV_VALUE_GAME_WON, SKV_VALUE_GAME_DRAWN}, 	{FPKV_MAX_VALUE, FPKV_MAX_VALUE, 0.0f}, {true, true, false}, 	SKV_VALUE_GAME_DRAWN, 		0.0f);
	testCalcKnotValue(3, {SKV_VALUE_GAME_WON, SKV_VALUE_GAME_WON, SKV_VALUE_GAME_DRAWN}, 	{FPKV_MAX_VALUE, FPKV_MAX_VALUE, 0.0f}, {true, false, false}, 	SKV_VALUE_GAME_WON, 		FPKV_MAX_VALUE);
	testCalcKnotValue(3, {SKV_VALUE_GAME_WON, SKV_VALUE_GAME_WON, SKV_VALUE_GAME_DRAWN}, 	{FPKV_MAX_VALUE, FPKV_MAX_VALUE, 0.0f}, {false, false, false}, 	SKV_VALUE_GAME_WON, 		FPKV_MAX_VALUE);
}

TEST_F(MiniMaxAlphaBeta_KnotStruct, calcPlyInfo)
{
	EXPECT_FALSE(knot.calcPlyInfo());

	//              n  values  										plyInfos  	playerToMoveChanged  	expectedPlyInfo
	testCalcPlyInfo(1, {SKV_VALUE_GAME_WON}, 						{0}, 		{false}, 				1);
	testCalcPlyInfo(1, {SKV_VALUE_GAME_LOST}, 						{0}, 		{false}, 				1);
	testCalcPlyInfo(1, {SKV_VALUE_GAME_DRAWN}, 						{0}, 		{true}, 				PLYINFO_VALUE_DRAWN);
	testCalcPlyInfo(1, {SKV_VALUE_INVALID}, 						{0}, 		{true}, 				PLYINFO_VALUE_INVALID);

	testCalcPlyInfo(2, {SKV_VALUE_GAME_WON, SKV_VALUE_GAME_LOST}, 	{0, 2}, 	{false, true}, 			1);
	testCalcPlyInfo(2, {SKV_VALUE_GAME_WON, SKV_VALUE_GAME_LOST}, 	{0, 2}, 	{true, true}, 			3);
	testCalcPlyInfo(2, {SKV_VALUE_GAME_WON, SKV_VALUE_GAME_LOST}, 	{0, 2}, 	{true, false}, 			3);
	testCalcPlyInfo(2, {SKV_VALUE_GAME_WON, SKV_VALUE_GAME_LOST}, 	{0, 2}, 	{false, false}, 		1);
}

TEST_F(MiniMaxAlphaBeta_KnotStruct, increaseFreqValuesSubMoves)
{
	// test invalid state
	EXPECT_FALSE(knot.increaseFreqValuesSubMoves(0));

	// test valid state
	knot.numPossibilities = 5;
	knot.branches[0].shortValue = SKV_VALUE_GAME_WON;
	knot.branches[1].shortValue = SKV_VALUE_GAME_LOST;
	knot.branches[2].shortValue = SKV_VALUE_GAME_DRAWN;
	knot.branches[3].shortValue = SKV_VALUE_INVALID;
	knot.branches[4].shortValue = SKV_VALUE_GAME_WON;
	EXPECT_TRUE (knot.increaseFreqValuesSubMoves(0));
	EXPECT_TRUE (knot.increaseFreqValuesSubMoves(1));
	EXPECT_TRUE (knot.increaseFreqValuesSubMoves(2));
	EXPECT_TRUE (knot.increaseFreqValuesSubMoves(3));
	EXPECT_EQ(knot.freqValuesSubMoves[SKV_VALUE_GAME_WON], 	 1);
	EXPECT_EQ(knot.freqValuesSubMoves[SKV_VALUE_GAME_LOST],  1);
	EXPECT_EQ(knot.freqValuesSubMoves[SKV_VALUE_GAME_DRAWN], 1);
	EXPECT_EQ(knot.freqValuesSubMoves[SKV_VALUE_INVALID], 	 1);
}

TEST_F(MiniMaxAlphaBeta_KnotStruct, getInfoAboutChoices)
{
	// locals
	stateInfo info;

	knot.shortValue 				= SKV_VALUE_GAME_WON;
	knot.numPossibilities 			= 3;
	knot.plyInfo					= 11;
	knot.possibilityIds 			= {20, 21, 22};
	knot.branches[0].shortValue 	= SKV_VALUE_GAME_WON;
	knot.branches[1].shortValue 	= SKV_VALUE_GAME_LOST;
	knot.branches[2].shortValue 	= SKV_VALUE_GAME_DRAWN;
	knot.branches[0].plyInfo 		= 1;
	knot.branches[1].plyInfo 		= 11;
	knot.branches[2].plyInfo 		= 2;
	knot.branches[0].freqValuesSubMoves[SKV_VALUE_INVALID] 		= 1;
	knot.branches[0].freqValuesSubMoves[SKV_VALUE_GAME_LOST] 	= 2;
	knot.branches[0].freqValuesSubMoves[SKV_VALUE_GAME_DRAWN] 	= 3;
	knot.branches[0].freqValuesSubMoves[SKV_VALUE_GAME_WON] 	= 4;
	knot.branches[1].freqValuesSubMoves[SKV_VALUE_INVALID] 		= 5;
	knot.branches[1].freqValuesSubMoves[SKV_VALUE_GAME_LOST] 	= 6;
	knot.branches[1].freqValuesSubMoves[SKV_VALUE_GAME_DRAWN] 	= 7;
	knot.branches[1].freqValuesSubMoves[SKV_VALUE_GAME_WON] 	= 8;
	knot.branches[2].freqValuesSubMoves[SKV_VALUE_INVALID] 		= 9;
	knot.branches[2].freqValuesSubMoves[SKV_VALUE_GAME_LOST] 	= 10;
	knot.branches[2].freqValuesSubMoves[SKV_VALUE_GAME_DRAWN] 	= 11;
	knot.branches[2].freqValuesSubMoves[SKV_VALUE_GAME_WON] 	= 12;

	EXPECT_TRUE(knot.getInfoAboutChoices(info));

	EXPECT_EQ(info.plyInfo, 					11);							// plyInfo of the knot
	EXPECT_EQ(info.shortValue, 					SKV_VALUE_GAME_WON);			// short value of the knot	
	EXPECT_EQ(info.choices.size(), 				3);								// number of choices
	EXPECT_EQ(info.choices[0].possibilityId, 	20);							// possibility id
	EXPECT_EQ(info.choices[0].shortValue, 		SKV_VALUE_GAME_LOST);			// changed perspective
	EXPECT_EQ(info.choices[0].plyInfo, 			1);								// plyInfo of the choice
	EXPECT_EQ(info.choices[0].freqValuesSubMoves[SKV_VALUE_INVALID], 1);
	EXPECT_EQ(info.choices[0].freqValuesSubMoves[SKV_VALUE_GAME_LOST], 2);
	EXPECT_EQ(info.choices[0].freqValuesSubMoves[SKV_VALUE_GAME_DRAWN], 3);
	EXPECT_EQ(info.choices[0].freqValuesSubMoves[SKV_VALUE_GAME_WON], 4);
	EXPECT_EQ(info.choices[1].possibilityId, 21);	
	EXPECT_EQ(info.choices[1].shortValue, SKV_VALUE_GAME_WON);					// changed perspective
	EXPECT_EQ(info.choices[1].plyInfo, 11);
	EXPECT_EQ(info.choices[1].freqValuesSubMoves[SKV_VALUE_INVALID], 5);
	EXPECT_EQ(info.choices[1].freqValuesSubMoves[SKV_VALUE_GAME_LOST], 6);
	EXPECT_EQ(info.choices[1].freqValuesSubMoves[SKV_VALUE_GAME_DRAWN], 7);
	EXPECT_EQ(info.choices[1].freqValuesSubMoves[SKV_VALUE_GAME_WON], 8);
	EXPECT_EQ(info.choices[2].possibilityId, 22);
	EXPECT_EQ(info.choices[2].shortValue, SKV_VALUE_GAME_DRAWN);				// changed perspective, but drawn anyway
	EXPECT_EQ(info.choices[2].plyInfo, 2);
	EXPECT_EQ(info.choices[2].freqValuesSubMoves[SKV_VALUE_INVALID], 9);
	EXPECT_EQ(info.choices[2].freqValuesSubMoves[SKV_VALUE_GAME_LOST], 10);
	EXPECT_EQ(info.choices[2].freqValuesSubMoves[SKV_VALUE_GAME_DRAWN], 11);
	EXPECT_EQ(info.choices[2].freqValuesSubMoves[SKV_VALUE_GAME_WON], 12);
}

TEST_F(MiniMaxAlphaBeta_KnotStruct, getBestBranchesBasedOnSkvValue)
{
	// invalid state, since possibilities are not set
	vector<unsigned int> bestBranches;
	EXPECT_FALSE(knot.getBestBranchesBasedOnSkvValue(bestBranches));

	// test valid state				// n, shortValue, 			branch-values, 				plyInfos, 	playerToMoveChanged, 	expectedBestBranches
	testGetBestBranchesBasedOnSkvValue(1, SKV_VALUE_GAME_WON, 	{SKV_VALUE_GAME_WON}, 		{0}, 		{false}, 				{0});	// only one possibility
	testGetBestBranchesBasedOnSkvValue(1, SKV_VALUE_GAME_LOST, 	{SKV_VALUE_GAME_LOST}, 		{0}, 		{false},  				{0});	// only one possibility
	testGetBestBranchesBasedOnSkvValue(1, SKV_VALUE_GAME_DRAWN,	{SKV_VALUE_GAME_DRAWN}, 	{0}, 		{true}, 				{0});	// only one possibility
	testGetBestBranchesBasedOnSkvValue(1, SKV_VALUE_INVALID, 	{SKV_VALUE_INVALID}, 		{0}, 		{true}, 				{});	// invalid state

	testGetBestBranchesBasedOnSkvValue(2, SKV_VALUE_GAME_WON, 	{SKV_VALUE_GAME_WON, SKV_VALUE_GAME_LOST}, {2, 0}, {false, true}, {1});	// both possibilities lead to a win,  but the 2nd has a lower  plyInfo
	testGetBestBranchesBasedOnSkvValue(2, SKV_VALUE_GAME_LOST,	{SKV_VALUE_GAME_WON, SKV_VALUE_GAME_LOST}, {2, 0}, {true, false}, {0});	// both possibilities lead to a lost, but the 1st has a higher plyInfo
	testGetBestBranchesBasedOnSkvValue(2, SKV_VALUE_GAME_WON, 	{SKV_VALUE_GAME_WON, SKV_VALUE_GAME_LOST}, {2, 0}, {true, true},  {1});	// only the 2nd   possibility leads to lost state for the opponent
	testGetBestBranchesBasedOnSkvValue(2, SKV_VALUE_GAME_WON, 	{SKV_VALUE_GAME_WON, SKV_VALUE_GAME_LOST}, {2, 0}, {false, false},{0});	// only the 1st   possibility leads to win  state for the current player

	testGetBestBranchesBasedOnSkvValue(3, SKV_VALUE_GAME_WON, 	{SKV_VALUE_GAME_WON, SKV_VALUE_GAME_LOST, SKV_VALUE_GAME_DRAWN}, {2, 0, 1}, {false, true, true}, {1});	// possibilities 0, 1 lead to win,  but 1 has a lower plyInfo
	testGetBestBranchesBasedOnSkvValue(3, SKV_VALUE_GAME_WON, 	{SKV_VALUE_GAME_WON, SKV_VALUE_GAME_LOST, SKV_VALUE_GAME_DRAWN}, {2, 0, 1}, {true, false, true}, {2});	// possibilities 0, 1 lead to lost, so 2 is the best choice

	testGetBestBranchesBasedOnSkvValue(3, SKV_VALUE_GAME_DRAWN, {SKV_VALUE_GAME_DRAWN, SKV_VALUE_GAME_DRAWN, SKV_VALUE_GAME_DRAWN}, {PLYINFO_VALUE_DRAWN, PLYINFO_VALUE_DRAWN, PLYINFO_VALUE_DRAWN}, {true, true, true}, {0, 1, 2});	// all possibilities are drawn
}

TEST_F(MiniMaxAlphaBeta_KnotStruct, getBestBranchesBasedOnFloatValue)
{
	// invalid state, since possibilities are not set
	vector<unsigned int> bestBranches;
	EXPECT_FALSE(knot.getBestBranchesBasedOnFloatValue(bestBranches));		

	// test valid state
	testGetBestBranchesBasedOnFloatValue(1, FPKV_MAX_VALUE, {FPKV_MAX_VALUE}, {false},	{0});	// only one possibility
	testGetBestBranchesBasedOnFloatValue(1, FPKV_MIN_VALUE, {FPKV_MAX_VALUE}, {true}, 	{0});
	testGetBestBranchesBasedOnFloatValue(1, FPKV_INV_VALUE, {FPKV_INV_VALUE}, {true}, 	{});	// invalid state
	testGetBestBranchesBasedOnFloatValue(1, FPKV_INV_VALUE, {FPKV_INV_VALUE}, {false}, 	{});	// invalid state
	testGetBestBranchesBasedOnFloatValue(1, 0, 				{0}, 			  {false}, 	{0});
	testGetBestBranchesBasedOnFloatValue(1, 0, 				{0}, 			  {true}, 	{0});

	testGetBestBranchesBasedOnFloatValue(2, FPKV_MAX_VALUE, {FPKV_MAX_VALUE, FPKV_MIN_VALUE}, {false, true}, 	{0, 1});	// both possibilities lead to a win
	testGetBestBranchesBasedOnFloatValue(2, FPKV_MIN_VALUE, {FPKV_MAX_VALUE, FPKV_MIN_VALUE}, {true,  false}, 	{0, 1});	// both possibilities lead to a loss
	testGetBestBranchesBasedOnFloatValue(2, FPKV_MAX_VALUE, {FPKV_MAX_VALUE, FPKV_MIN_VALUE}, {true,  true}, 	{1});		// only the 2nd   possibility leads to lost state for the opponent
	testGetBestBranchesBasedOnFloatValue(2, FPKV_MAX_VALUE, {FPKV_MAX_VALUE, FPKV_MIN_VALUE}, {false, false}, 	{0});		// only the 1st   possibility leads to win  state for the current player
	testGetBestBranchesBasedOnFloatValue(2, FPKV_INV_VALUE, {FPKV_INV_VALUE, FPKV_INV_VALUE}, {true,  true}, 	{});		// both possibilities are invalid, so no choice
	testGetBestBranchesBasedOnFloatValue(2, 2, 				{-2, 1}, 						  {true,  true}, 	{0});		// only the first possibility leads to bad state for the opponent
	testGetBestBranchesBasedOnFloatValue(2, 2, 				{1, -2}, 						  {true,  true}, 	{1});		// only the 2nd   possibility leads to bad state for the opponent
	testGetBestBranchesBasedOnFloatValue(2, 2, 				{1, 1}, 						  {true,  true}, 	{0, 1});	// both possibilities lead to good state for the opponent, so both are valid
}

TEST_F(MiniMaxAlphaBeta_KnotStruct, canCutOff)
{
	// locals
	float alpha, beta;

	// TODO: implement tests
}

#pragma endregion

#pragma region solver
class MiniMaxAlphaBeta_solver : public MiniMaxTestGameFixture {
protected:
	alphaBeta::solver		solver{log, tm, db, game};

	MiniMaxAlphaBeta_solver(): MiniMaxTestGameFixture("alphaBeta") {}
};

TEST_F(MiniMaxAlphaBeta_solver, databaseCalculation)
{
	std::vector<unsigned int>layersToCalculate = {0};

	EXPECT_TRUE(solver.calcKnotValuesByAlphaBeta(layersToCalculate));

	game.checkWithDatabase(db, layersToCalculate);

	EXPECT_FALSE(db.isComplete());
	for (auto& layer : layersToCalculate) {
		EXPECT_FALSE(db.isLayerCompleteAndInFile(layer));
	}	
}

TEST_F(MiniMaxAlphaBeta_solver, depthSearch)
{
	// locals
	unsigned int 	choice;
	stateInfo 		infoAboutChoices;

	// prepare solver
	solver.setSearchDepth(3);

	// test invalid state
	EXPECT_FALSE(game.setSituation(0, 0, 0));
	EXPECT_TRUE(solver.getBestChoice(choice, infoAboutChoices));
	EXPECT_EQ(choice, 								0);
	EXPECT_EQ(infoAboutChoices.choices.size(), 		0);
	EXPECT_EQ(infoAboutChoices.bestAmountOfPlies, 	PLYINFO_VALUE_INVALID);
	EXPECT_EQ(infoAboutChoices.plyInfo, 			PLYINFO_VALUE_INVALID);
	EXPECT_EQ(infoAboutChoices.shortValue, 			SKV_VALUE_INVALID);

	// test won state
	EXPECT_TRUE(game.setSituation(0, 0, 1));
	EXPECT_TRUE(solver.getBestChoice(choice, infoAboutChoices));
	EXPECT_EQ(choice, 								0);
	EXPECT_EQ(infoAboutChoices.choices.size(), 		0);
	EXPECT_EQ(infoAboutChoices.bestAmountOfPlies, 	PLYINFO_VALUE_INVALID);
	EXPECT_EQ(infoAboutChoices.plyInfo, 			0);
	EXPECT_EQ(infoAboutChoices.shortValue, 			SKV_VALUE_GAME_WON);

	// test lost state
	EXPECT_TRUE(game.setSituation(0, 0, 2));
	EXPECT_TRUE(solver.getBestChoice(choice, infoAboutChoices));
	EXPECT_EQ(choice, 										0);
	EXPECT_EQ(infoAboutChoices.choices.size(), 				1);
	EXPECT_EQ(infoAboutChoices.bestAmountOfPlies,			1);
	EXPECT_EQ(infoAboutChoices.plyInfo, 					1);
	EXPECT_EQ(infoAboutChoices.shortValue, 					SKV_VALUE_GAME_LOST);
	EXPECT_EQ(infoAboutChoices.choices[0].possibilityId, 	0);
	EXPECT_EQ(infoAboutChoices.choices[0].shortValue, 		SKV_VALUE_GAME_LOST);
	EXPECT_EQ(infoAboutChoices.choices[0].plyInfo, 			0);
	EXPECT_EQ(infoAboutChoices.choices[0].freqValuesSubMoves[SKV_VALUE_INVALID], 	0);
	EXPECT_EQ(infoAboutChoices.choices[0].freqValuesSubMoves[SKV_VALUE_GAME_LOST], 	0);
	EXPECT_EQ(infoAboutChoices.choices[0].freqValuesSubMoves[SKV_VALUE_GAME_DRAWN], 0);
	EXPECT_EQ(infoAboutChoices.choices[0].freqValuesSubMoves[SKV_VALUE_GAME_WON], 	1);
}
#pragma endregion

