/**************************************************************************************************************************
	MiniMaxDatabaseTest.cpp
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
***************************************************************************************************************************/
#define _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <thread>
#include <chrono>
#include <vector>

#include "miniMax/src/database/database.h"
#include "miniMax/src/database/databaseStats.h"
#include "miniMax/src/database/databaseFile.h"
#include "weaselEssentials/src/logger.h"
#include "miniMax/src/typeDef.h"

using ::testing::ElementsAre;
using namespace std;

#pragma region speedometerTest
TEST(MiniMaxDatabase, speedometer_singleThread) 
{
	int callCount = 0;
	auto speedoPrint_1 = [&](wstring& name, float operationsPerSec) {
		EXPECT_STREQ(name.c_str(), L"test");
		EXPECT_GT(operationsPerSec, 9.0f);
		EXPECT_LT(operationsPerSec, 11.0f);
		callCount++;
	};

	miniMax::database::speedometer speedometer(L"test", 10, speedoPrint_1);
	for (int i = 0; i < 21; i++) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		speedometer.measureIops();
	}

	EXPECT_EQ(callCount, 2);
}

TEST(MiniMaxDatabase, speedometer_multiThread) 
{
	int callCount 			= 0;
	const int numThreads 	= 4;
	const int numCalls 		= 21;
	const int sleepTime 	= 100;
	const int printFreq 	= 10;
	auto speedoPrint_1 = [&](wstring& name, float operationsPerSec) {
		float duration 					= numCalls * sleepTime / 1000.0f;
		float expectedOperationsPerSec 	= numThreads * numCalls / duration;
		EXPECT_STREQ(name.c_str(), L"test");
		EXPECT_GT(operationsPerSec, expectedOperationsPerSec - 10);
		EXPECT_LT(operationsPerSec, expectedOperationsPerSec + 10);
		callCount++;
	};

	// start threads calling measureIops each numCalls times
	miniMax::database::speedometer speedometer(L"test", printFreq, speedoPrint_1);
	std::vector<std::thread> threads;
	for (int t = 0; t < numThreads; ++t) {
		threads.emplace_back([&speedometer, sleepTime]() {
			for (int i = 0; i < numCalls; i++) {
				std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
				speedometer.measureIops();
			}
		});
	}
	for (auto& thread : threads) {
		thread.join();
	}

	EXPECT_EQ(callCount, 8);

}
#pragma endregion

#pragma region arrayInfoContainerTest
class MiniMaxDatabase_arrayInfoContainerTest : public ::testing::Test {

protected:
	logger log{logger::logLevel::none, logger::logType::none, L""};
	miniMax::database::arrayInfoContainer aic{log};

	void SetUp() override {
		aic.init(2);
	}
};

TEST_F(MiniMaxDatabase_arrayInfoContainerTest, addRemoveArray) 
{
	EXPECT_FALSE(aic.addArray(0, miniMax::database::arrayInfoStruct::arrayType::knotAlreadyCalculated, 0, 0));		// invalid size
	EXPECT_TRUE(aic.addArray(0, miniMax::database::arrayInfoStruct::arrayType::knotAlreadyCalculated, 1, 0));		// compressed size 0 is valid
	EXPECT_FALSE(aic.addArray(0, miniMax::database::arrayInfoStruct::arrayType::knotAlreadyCalculated, 0, 1));		// invalid size
	EXPECT_FALSE(aic.addArray(0, miniMax::database::arrayInfoStruct::arrayType::size, 5, 5));						// invalid type
	EXPECT_FALSE(aic.addArray(2, miniMax::database::arrayInfoStruct::arrayType::knotAlreadyCalculated, 1, 1));		// invalid layer
	EXPECT_FALSE(aic.addArray(0, miniMax::database::arrayInfoStruct::arrayType::knotAlreadyCalculated, 1, 1));		// already exists
	EXPECT_TRUE(aic.addArray(1, miniMax::database::arrayInfoStruct::arrayType::countArray, 2, 2));					// valid
	EXPECT_TRUE(aic.addArray(0, miniMax::database::arrayInfoStruct::arrayType::plyInfos, 3, 3));					// valid
	EXPECT_TRUE(aic.addArray(0, miniMax::database::arrayInfoStruct::arrayType::layerStats, 4, 4));					// valid
	EXPECT_FALSE(aic.addArray(0, miniMax::database::arrayInfoStruct::arrayType::layerStats, 4, 4));					// already exists
	EXPECT_FALSE(aic.removeArray(0, miniMax::database::arrayInfoStruct::arrayType::countArray, 2, 2));				// does not exist
	EXPECT_FALSE(aic.removeArray(0, miniMax::database::arrayInfoStruct::arrayType::knotAlreadyCalculated, 1, 1));	// wrong compressed size
	EXPECT_TRUE(aic.removeArray(0, miniMax::database::arrayInfoStruct::arrayType::knotAlreadyCalculated, 1, 0));	// valid
	EXPECT_TRUE(aic.removeArray(1, miniMax::database::arrayInfoStruct::arrayType::countArray, 2, 2));				// valid
	EXPECT_TRUE(aic.removeArray(0, miniMax::database::arrayInfoStruct::arrayType::plyInfos, 3, 3));					// valid
	EXPECT_TRUE(aic.addArray(0, miniMax::database::arrayInfoStruct::arrayType::plyInfos, 3, 3));					// add array again
}

TEST_F(MiniMaxDatabase_arrayInfoContainerTest, anyArrayInfoToUpdate) 
{
	EXPECT_FALSE(aic.anyArrayInfoToUpdate());
	EXPECT_TRUE(aic.addArray(0, miniMax::database::arrayInfoStruct::arrayType::knotAlreadyCalculated, 1, 1));
	EXPECT_TRUE(aic.anyArrayInfoToUpdate());
}

TEST_F(MiniMaxDatabase_arrayInfoContainerTest, getArrayInfoForUpdate) 
{
	auto aic0 = aic.getArrayInfoForUpdate();																		// invalid, no array info to update
	EXPECT_EQ(aic0.arrayInfo, nullptr);																				// thus, nullptr
	EXPECT_EQ(aic0.itemIndex, 0xffffffff);																			// invalid index
	EXPECT_TRUE(aic.addArray(1, miniMax::database::arrayInfoStruct::arrayType::knotAlreadyCalculated, 2, 1));		// add any array
	EXPECT_TRUE(aic.anyArrayInfoToUpdate());																		// there is an array to update
	auto aic1 = aic.getArrayInfoForUpdate();																		// get the array info
	EXPECT_FALSE(aic.anyArrayInfoToUpdate());																		// no more array to update
	EXPECT_EQ(aic1.arrayInfo->type, miniMax::database::arrayInfoStruct::arrayType::knotAlreadyCalculated);			// correct type
	EXPECT_EQ(aic1.arrayInfo->sizeInBytes, 2);
	EXPECT_EQ(aic1.arrayInfo->compressedSizeInBytes, 1);
	EXPECT_EQ(aic1.arrayInfo->belongsToLayer, 1);
	EXPECT_EQ(aic1.itemIndex, 0);

	EXPECT_TRUE(aic.removeArray(1, miniMax::database::arrayInfoStruct::arrayType::knotAlreadyCalculated, 2, 1));	// remove array again
	EXPECT_TRUE(aic.anyArrayInfoToUpdate());																		// there is an array to update
	auto aic2 = aic.getArrayInfoForUpdate();																		// get the array info
	EXPECT_EQ(aic2.arrayInfo, nullptr);																				// array was removed
	EXPECT_EQ(aic2.itemIndex, 0);																					// index is still valid
	EXPECT_FALSE(aic.anyArrayInfoToUpdate());																		// no more array to update
}

TEST_F(MiniMaxDatabase_arrayInfoContainerTest, getMemoryUsed) 
{
	EXPECT_EQ(aic.getMemoryUsed(), 0);	EXPECT_TRUE(aic.addArray(0, miniMax::database::arrayInfoStruct::arrayType::knotAlreadyCalculated, 1, 1));		// valid
	EXPECT_EQ(aic.getMemoryUsed(), 1);	EXPECT_TRUE(aic.addArray(0, miniMax::database::arrayInfoStruct::arrayType::countArray, 2, 2));					// valid
	EXPECT_EQ(aic.getMemoryUsed(), 3);	EXPECT_TRUE(aic.addArray(0, miniMax::database::arrayInfoStruct::arrayType::plyInfos, 3, 3));					// valid
	EXPECT_EQ(aic.getMemoryUsed(), 6);	EXPECT_FALSE(aic.addArray(0, miniMax::database::arrayInfoStruct::arrayType::plyInfos, 3, 3));					// already exists
	EXPECT_EQ(aic.getMemoryUsed(), 6);	EXPECT_TRUE(aic.addArray(0, miniMax::database::arrayInfoStruct::arrayType::layerStats, 4, 4));					// valid
	EXPECT_EQ(aic.getMemoryUsed(), 10);	EXPECT_TRUE(aic.removeArray(0, miniMax::database::arrayInfoStruct::arrayType::knotAlreadyCalculated, 1, 1));	// valid
	EXPECT_EQ(aic.getMemoryUsed(), 9);	EXPECT_TRUE(aic.removeArray(0, miniMax::database::arrayInfoStruct::arrayType::countArray, 2, 2));				// valid
	EXPECT_EQ(aic.getMemoryUsed(), 7);	EXPECT_TRUE(aic.removeArray(0, miniMax::database::arrayInfoStruct::arrayType::plyInfos, 3, 3));					// valid
	EXPECT_EQ(aic.getMemoryUsed(), 4);	EXPECT_TRUE(aic.removeArray(0, miniMax::database::arrayInfoStruct::arrayType::layerStats, 4, 4));				// valid
	EXPECT_EQ(aic.getMemoryUsed(), 0);	EXPECT_FALSE(aic.removeArray(0, miniMax::database::arrayInfoStruct::arrayType::countArray, 2, 2));				// does not exist
	EXPECT_EQ(aic.getMemoryUsed(), 0);
}

TEST_F(MiniMaxDatabase_arrayInfoContainerTest, threadSafety) 
{
	const int numThreads = 4;
	const int numCalls = 1000;

	auto addRemoveArray = [this]() {
		for (int i = 0; i < numCalls; i++) {
			// add and remove arrays again
			aic.addArray(0, miniMax::database::arrayInfoStruct::arrayType::knotAlreadyCalculated, 1, 1);
			aic.getArrayInfoForUpdate();
			aic.addArray(1, miniMax::database::arrayInfoStruct::arrayType::countArray, 2, 2);
			aic.addArray(0, miniMax::database::arrayInfoStruct::arrayType::plyInfos, 3, 3);
			aic.anyArrayInfoToUpdate();
			aic.getArrayInfoForUpdate();
			aic.addArray(0, miniMax::database::arrayInfoStruct::arrayType::layerStats, 4, 4);
			aic.removeArray(0, miniMax::database::arrayInfoStruct::arrayType::knotAlreadyCalculated, 1, 1);
			aic.getMemoryUsed();
			aic.removeArray(1, miniMax::database::arrayInfoStruct::arrayType::countArray, 2, 2);
			aic.removeArray(0, miniMax::database::arrayInfoStruct::arrayType::plyInfos, 3, 3);
			aic.removeArray(0, miniMax::database::arrayInfoStruct::arrayType::layerStats, 4, 4);
			aic.anyArrayInfoToUpdate();
			aic.getArrayInfoForUpdate();
			aic.getArrayInfoForUpdate();
			aic.getArrayInfoForUpdate();
			aic.getArrayInfoForUpdate();
			aic.getArrayInfoForUpdate();
			aic.getArrayInfoForUpdate();		// call it 10 times, as there are 10 add/remove calls
		}
	};

	EXPECT_EQ(aic.getMemoryUsed(), 0);
	std::vector<std::thread> threads;
	for (int t = 0; t < numThreads; ++t) {
		threads.emplace_back(addRemoveArray);
	}
	for (auto& thread : threads) {
		thread.join();
	}
	EXPECT_EQ(aic.getMemoryUsed(), 0);
	EXPECT_FALSE(aic.anyArrayInfoToUpdate());
}
#pragma endregion

#pragma region gameInterfaceStub
namespace miniMax {
class gameInterfaceStub : public gameInterface {
public:
	gameInterfaceStub() : gameInterface() {}
	~gameInterfaceStub() {}

	unsigned int	getNumberOfLayers				()															{ return 2;											};
	unsigned int	getNumberOfKnotsInLayer			(unsigned int layerNum)										{ return layerNum == 0 ? 100 : 200;					};
	void            getSuccLayers               	(unsigned int layerNum, vector<unsigned int>& succLayers)	{ succLayers.assign({1,0});							};
	uint_1d			getPartnerLayers				(unsigned int layerNum)										{ return layerNum == 0 ? uint_1d{1} : uint_1d{0};	};
};
}	// namespace miniMax
#pragma endregion

#pragma region GenericFileTest
class MiniMaxDatabase_genericFileTest : public ::testing::Test {

public:
	static const unsigned int numKnotsInLayer0 = 100;
	static const unsigned int numKnotsInLayer1 = 200;
	static const unsigned int numSkvBytes0 		= (numKnotsInLayer0 + 3) / 4;
	static const unsigned int numSkvBytes1 		= (numKnotsInLayer1 + 3) / 4;

protected:
	logger log{logger::logLevel::none, logger::logType::none, L""};
	miniMax::gameInterfaceStub game{};
	miniMax::database::databaseStatsStruct dbStats{};
	vector<miniMax::database::layerStatsStruct> layerStats{};

	void SetUp() override {
		dbStats.numLayers = 2;
		dbStats.completed = false;

		layerStats.resize(2);
		
		layerStats[0].completedAndInFile = false;
		layerStats[0].partnerLayer 		= 1;
		layerStats[0].knotsInLayer 		= numKnotsInLayer0;
		layerStats[0].numWonStates 		= 10;
		layerStats[0].numLostStates 	= 5;
		layerStats[0].numDrawnStates 	= numKnotsInLayer0 - 15;
		layerStats[0].numInvalidStates 	= 0;
		layerStats[0].succLayers 		= {1, 0};
		layerStats[0].partnerLayers 	= {1};
		layerStats[0].skv 				= vector<miniMax::twoBit>(numSkvBytes0, miniMax::SKV_VALUE_INVALID);
		layerStats[0].plyInfo 			= vector<miniMax::plyInfoVarType>(numKnotsInLayer0, miniMax::PLYINFO_VALUE_INVALID);

		layerStats[1].completedAndInFile = true;
		layerStats[1].partnerLayer 		= 0;
		layerStats[1].knotsInLayer 		= numKnotsInLayer1;
		layerStats[1].numWonStates 		= 20;
		layerStats[1].numLostStates 	= 10;
		layerStats[1].numDrawnStates 	= numKnotsInLayer1 - 30;
		layerStats[1].numInvalidStates 	= 0;
		layerStats[1].succLayers 		= {1, 0};
		layerStats[1].partnerLayers		 = {0};
		layerStats[1].skv 				= vector<miniMax::twoBit>(numSkvBytes1, miniMax::SKV_VALUE_INVALID);
		layerStats[1].plyInfo 			= vector<miniMax::plyInfoVarType>(numKnotsInLayer1, miniMax::PLYINFO_VALUE_INVALID);
	}

	void TearDown() override {
	}
};

void fillSkvAndPlyInfoWithRandomData(vector<miniMax::twoBit>& skv, vector<miniMax::plyInfoVarType>& plyInfo, unsigned int numKnotsInLayer0) 
{
	for (unsigned int i = 0; i < numKnotsInLayer0; i++) {
		skv[i / 4] = (miniMax::twoBit)(rand() % 4);
		plyInfo[i] = (miniMax::plyInfoVarType)(rand() % 4);
	}
}

void runGenericFileTest(
	miniMax::database::genericFile& gf, 
	miniMax::gameInterface& game, 
	miniMax::database::databaseStatsStruct& dbStats, 
	vector<miniMax::database::layerStatsStruct>& layerStats)
{
	// locals
	miniMax::twoBit dbByte, expected_dbByte;
	vector<miniMax::twoBit> skv, expected_skv;
	vector<miniMax::plyInfoVarType> plyInfo, expected_plyInfo;
	miniMax::plyInfoVarType plyInfoVar, expected_plyInfoVar;
	miniMax::database::databaseStatsStruct expected_dbStats = dbStats;
	vector<miniMax::database::layerStatsStruct> expected_layerStats = layerStats;
	const wstring 		tmpFileDirectory 	= (std::filesystem::temp_directory_path() / "wildWeasel" / "genericFile").c_str();
	const unsigned int 	numKnotsInLayer0	= MiniMaxDatabase_genericFileTest::numKnotsInLayer0;
	const unsigned int 	numKnotsInLayer1	= MiniMaxDatabase_genericFileTest::numKnotsInLayer1;
	const unsigned int 	numSkvBytes0 		= MiniMaxDatabase_genericFileTest::numSkvBytes0;

	// recreate the database folder
	if (std::filesystem::exists(tmpFileDirectory)) {
		std::filesystem::remove_all(tmpFileDirectory);
	}
	std::filesystem::create_directories(tmpFileDirectory);

	// Negative tests
	ASSERT_FALSE(gf.isOpen());											// database has not been opened yet
	ASSERT_FALSE(gf.saveHeader(dbStats, layerStats));					// fails, as the database is not open
	ASSERT_FALSE(gf.loadHeader(dbStats, layerStats));					// fails, as the database is not open
	ASSERT_EQ(dbStats, expected_dbStats);								// dbStats should be unchanged
	ASSERT_EQ(layerStats, expected_layerStats);							// layerStats should be unchanged
	ASSERT_FALSE(gf.readSkv(0, skv));									// fails, as the database is not open
	ASSERT_EQ(skv.size(), 0);											// skv should be unchanged
	ASSERT_FALSE(gf.readSkv(0, dbByte, 0));
	ASSERT_FALSE(gf.writeSkv(0, vector<miniMax::twoBit>()));
	ASSERT_FALSE(gf.readPlyInfo(0, plyInfo));
	ASSERT_EQ(plyInfo.size(), 0);										// plyInfo should be unchanged
	ASSERT_FALSE(gf.readPlyInfo(0, plyInfoVar, 0));
	ASSERT_FALSE(gf.writePlyInfo(0, vector<miniMax::plyInfoVarType>()));
	ASSERT_FALSE(gf.openDatabase(L"z:\\doesNotExist"));					// fails, as the folder does not exist

	// Positive tests
	skv.resize(numSkvBytes0);											// skv must already have the correct size, before reading
	plyInfo.resize(numKnotsInLayer0);									// plyInfo must already have the correct size, before reading
	gf.removeFile(tmpFileDirectory);									// remove the file if it exists

	// fill skv and plyInfo with random data
	fillSkvAndPlyInfoWithRandomData(skv, plyInfo, numKnotsInLayer0);
	expected_skv 		= skv;
	expected_plyInfo 	= plyInfo;

	// simple open and close
	ASSERT_TRUE(gf.openDatabase(tmpFileDirectory));						// create a new database
	ASSERT_TRUE(gf.isOpen());											// database is open
	ASSERT_TRUE(gf.loadHeader(dbStats, layerStats));					// load the header, create an empty header
	ASSERT_TRUE(gf.saveHeader(dbStats, layerStats));					// save the header, although it is empty
	// ASSERT_EQ(dbStats, empty_dbStats);								// dbStats should be unchanged
	// ASSERT_EQ(layerStats, empty_layerStats);							// layerStats should be unchanged
	ASSERT_TRUE(gf.saveHeader(expected_dbStats, expected_layerStats));	// save the header, but now with sample data
	gf.closeDatabase();													// close the database
	ASSERT_FALSE(gf.isOpen());											// database is closed

	// open and close with data
	ASSERT_TRUE(gf.openDatabase(tmpFileDirectory));						// open the database again
	ASSERT_TRUE(gf.isOpen());											// database is open
	ASSERT_TRUE(gf.loadHeader(dbStats, layerStats));					// load the header
	ASSERT_TRUE(gf.saveHeader(dbStats, layerStats));					// save the header
	expected_layerStats[0].skv.clear();									// directly after loading the header, this array is empty
	expected_layerStats[0].plyInfo.clear();								// directly after loading the header, this array is empty
	expected_layerStats[1].skv.clear();									// directly after loading the header, this array is empty
	expected_layerStats[1].plyInfo.clear();								// directly after loading the header, this array is empty
	ASSERT_EQ(dbStats, expected_dbStats);								// dbStats should now contain the sample data
	ASSERT_EQ(layerStats, expected_layerStats);							// layerStats should now contain the sample data
	ASSERT_FALSE(gf.readSkv(0, skv));									// fails, as the layer is not in the file
	ASSERT_EQ(skv, expected_skv);										// leave skv unchanged
	ASSERT_FALSE(gf.readSkv(0, dbByte, 0));								// fails, as the layer is not in the file
	ASSERT_FALSE(gf.readPlyInfo(0, plyInfo));							// fails, as the layer is not in the file
	ASSERT_EQ(plyInfo, expected_plyInfo);								// leave plyInfo unchanged
	ASSERT_FALSE(gf.readPlyInfo(0, plyInfoVar, 0));						// fails, as the layer is not in the file
	ASSERT_TRUE(gf.writeSkv(0, skv));									// write the skv
	ASSERT_TRUE(gf.writePlyInfo(0, plyInfo));							// write the plyInfo
	fillSkvAndPlyInfoWithRandomData(skv, plyInfo, numKnotsInLayer0);	// scramble the data
	ASSERT_TRUE(gf.readSkv(0, skv));									// read the skv
	ASSERT_EQ(skv, expected_skv);										// compare the two skv
	ASSERT_TRUE(gf.readPlyInfo(0, plyInfo));							// read the plyInfo
	ASSERT_EQ(plyInfo, expected_plyInfo);								// compare the two plyInfo
	ASSERT_TRUE(gf.readSkv(0, dbByte, 0));								// read a single byte from first knot in layer
	ASSERT_EQ(skv[0], dbByte);											// compare the two bytes
	ASSERT_TRUE(gf.readSkv(0, dbByte, numKnotsInLayer0-1));				// read a single byte from last knot in layer
	ASSERT_EQ(skv[(numKnotsInLayer0-1) / 4], dbByte);					// compare the two bytes
	ASSERT_TRUE(gf.readPlyInfo(0, plyInfoVar, 0));						// read a single plyInfo
	ASSERT_EQ(plyInfo[0], plyInfoVar);									// compare the two plyInfo
	ASSERT_TRUE(gf.readPlyInfo(0, plyInfoVar, numKnotsInLayer0-1));		// read a single plyInfo
	ASSERT_EQ(plyInfo[numKnotsInLayer0-1], plyInfoVar);					// compare the two plyInfo
	layerStats[0].completedAndInFile = true;							// mark layer as completed
	ASSERT_TRUE(gf.saveHeader(dbStats, layerStats));					// save the header, now with data
	gf.closeDatabase();													// close the database

	// open database again, check data again, and check layerStats
	ASSERT_TRUE(gf.openDatabase(tmpFileDirectory));						// open the database again
	ASSERT_FALSE(gf.readSkv(0, skv));									// fails, since header is not loaded
	ASSERT_FALSE(gf.readPlyInfo(0, plyInfo));							// fails, since header is not loaded
	dbStats = miniMax::database::databaseStatsStruct{};					// reset dbStats, before loading
	layerStats = vector<miniMax::database::layerStatsStruct>(2);		// reset layerStats, before loading
	ASSERT_TRUE(gf.loadHeader(dbStats, layerStats));					// load the header
	expected_layerStats[0].completedAndInFile = true;					// mark layer as completed
	EXPECT_EQ(layerStats, expected_layerStats);							// check loaded layerStats
	ASSERT_EQ(dbStats, expected_dbStats);								// check loaded dbStats
	fillSkvAndPlyInfoWithRandomData(skv, plyInfo, numKnotsInLayer0);	// scramble the data, before reading
	ASSERT_TRUE(gf.readSkv(0, skv));									// read the skv
	ASSERT_EQ(skv, expected_skv);										// compare the two skv
	ASSERT_TRUE(gf.readPlyInfo(0, plyInfo));							// read the plyInfo
	ASSERT_EQ(plyInfo, expected_plyInfo);								// compare the two plyInfo
	ASSERT_EQ(game.getNumberOfLayers(), dbStats.numLayers);				// check number of layers
	ASSERT_EQ(game.getNumberOfKnotsInLayer(0), numKnotsInLayer0);		// check number of knots in layer 0
	ASSERT_EQ(game.getNumberOfKnotsInLayer(1), numKnotsInLayer1);		// check number of knots in layer 1
	ASSERT_THAT(game.getPartnerLayers(0), ElementsAre(1));				// check partner layer of layer 0
	ASSERT_THAT(game.getPartnerLayers(1), ElementsAre(0));				// check partner layer of layer 1
	gf.closeDatabase();													// close the database	
}

TEST_F(MiniMaxDatabase_genericFileTest, uncompFile) {
	miniMax::database::genericFile* gf = new miniMax::database::uncompFile(&game, log);
	runGenericFileTest(*gf, game, dbStats, layerStats);
	delete gf;
}

TEST_F(MiniMaxDatabase_genericFileTest, compFile) {
	miniMax::database::genericFile* gf = new miniMax::database::compFile(&game, log);
	runGenericFileTest(*gf, game, dbStats, layerStats);
	delete gf;
}

#pragma endregion

#pragma region databaseTest
namespace miniMax {
class MiniMaxDatabase_databaseTest : public ::testing::Test {
protected:
	twoBit 				dbByte 				= SKV_VALUE_GAME_LOST;
	twoBit				expected_dbByte 	= SKV_VALUE_INVALID;
	plyInfoVarType 		plyInfoVar 			= PLYINFO_VALUE_UNCALCULATED;
	plyInfoVarType 		expected_plyInfoVar = PLYINFO_VALUE_INVALID;
	const wstring 		tmpFileDirectory 	= (std::filesystem::temp_directory_path() / "wildWeasel" / "database").c_str();
	logger 				log					{logger::logLevel::none, logger::logType::console, L""};
	gameInterfaceStub 	game				{};
	database::database 	db					{game, log};

	void SetUp() override {
		if (std::filesystem::exists(tmpFileDirectory)) {
			std::filesystem::remove_all(tmpFileDirectory);
		}
		std::filesystem::create_directories(tmpFileDirectory);
	}

	void TearDown() override {
	}
};

TEST_F(MiniMaxDatabase_databaseTest, negativeTests)
{
	// negative tests
	db.unload();
	db.showLayerStats(0);
	EXPECT_FALSE(db.closeDatabase());								// fails, as the database is not open
	EXPECT_FALSE(db.removeDatabaseFiles());							// fails, as the database is not open
	EXPECT_FALSE(db.isOpen());										// database has not been opened yet
	EXPECT_FALSE(db.saveLayerToFile(0));							// fails, as the database is not open
	EXPECT_EQ(db.getNumWonStates(0),		0);						// fails, as the database is not open
	EXPECT_EQ(db.getNumLostStates(0),		0);						// fails, as the database is not open
	EXPECT_EQ(db.getNumDrawnStates(0),		0);						// fails, as the database is not open
	EXPECT_EQ(db.getNumInvalidStates(0),	0);						// fails, as the database is not open
	EXPECT_FALSE(db.openDatabase(L"z:\\doesNotExist"));				// fails, as the folder does not exist
	EXPECT_FALSE(db.saveHeader());									// fails, as the database is not open
	EXPECT_FALSE(db.saveLayerToFile(0));							// fails, as the database is not open
	EXPECT_EQ(db.getNumWonStates(0),		0);						// fails, as the database is not open
	EXPECT_EQ(db.getNumLostStates(0),		0);						// fails, as the database is not open
	EXPECT_EQ(db.getNumDrawnStates(0),		0);						// fails, as the database is not open
	EXPECT_EQ(db.getNumInvalidStates(0),	0);						// fails, as the database is not open
	EXPECT_FALSE(db.updateLayerStats(0));							// fails, as the database is not open
	EXPECT_FALSE(db.setAsComplete());								// fails, as the database is not open
	EXPECT_FALSE(db.isComplete());									// fails, as the database is not open
	EXPECT_FALSE(db.isLayerCompleteAndInFile(0));					// fails, as the database is not open
	EXPECT_EQ(db.getNumberOfKnots(0), 0);							// none, as the database is not open
	EXPECT_EQ(db.getNumLayers(), 0);								// none, as the database is not open
	EXPECT_EQ(db.getPartnerLayers(0).size(), 0);					// none, as the database is not open
	EXPECT_EQ(db.getMemoryUsed(), 0);								// none, as the database is not open
	EXPECT_EQ(db.getLayerSizeInBytes(0), 0);						// none, as the database is not open
	EXPECT_EQ(db.getSuccLayers(0).size(), 0);						// none, as the database is not open
	EXPECT_FALSE(db.readKnotValueFromDatabase(0, 0, dbByte));		// fails, as the database is not open
	EXPECT_EQ(dbByte, SKV_VALUE_INVALID);							// expect invalid value
	EXPECT_FALSE(db.writeKnotValueInDatabase(0, 0, dbByte));			// fails, as the database is not open
	EXPECT_FALSE(db.readPlyInfoFromDatabase(0, 0, plyInfoVar));		// fails, as the database is not open
	EXPECT_EQ(plyInfoVar, PLYINFO_VALUE_INVALID);					// expect invalid value
	EXPECT_FALSE(db.writePlyInfoInDatabase(0, 0, plyInfoVar));		// fails, as the database is not open

	EXPECT_TRUE(db.openDatabase(tmpFileDirectory));					// create a new database
	EXPECT_FALSE(db.openDatabase(tmpFileDirectory));				// fail to open the database, since it is already open
	EXPECT_FALSE(db.readKnotValueFromDatabase(2, 0, dbByte));		// fails, since layer 2 does not exist
	EXPECT_EQ(dbByte, SKV_VALUE_INVALID);							// compare the two values	
	EXPECT_FALSE(db.readPlyInfoFromDatabase(2, 0, plyInfoVar));		// fails, since layer 2 does not exist
	EXPECT_EQ(plyInfoVar, PLYINFO_VALUE_INVALID);					// expect invalid value
	EXPECT_FALSE(db.writeKnotValueInDatabase(2, 0, dbByte));			// fails, since layer 2 does not exist
	EXPECT_FALSE(db.writePlyInfoInDatabase(2, 0, plyInfoVar));		// fails, since layer 2 does not exist
	EXPECT_FALSE(db.saveLayerToFile(2));							// fails, since layer 2 does not exist
	EXPECT_FALSE(db.updateLayerStats(2));							// fails, since layer 2 does not exist
	EXPECT_FALSE(db.isLayerCompleteAndInFile(2));					// fails, since layer 2 does not exist
	EXPECT_FALSE(db.writeKnotValueInDatabase(0, 100, dbByte));		// fails, since knot 100 does not exist
	EXPECT_FALSE(db.writePlyInfoInDatabase(0, 100, plyInfoVar));		// fails, since knot 100 does not exist
	EXPECT_FALSE(db.readKnotValueFromDatabase(0, 100, dbByte));		// fails, since knot 100 does not exist
	EXPECT_FALSE(db.readPlyInfoFromDatabase(0, 100, plyInfoVar));	// fails, since knot 100 does not exist
	EXPECT_FALSE(db.writeKnotValueInDatabase(0, 0, (twoBit)4));		// fails, since value is invalid
	EXPECT_FALSE(db.writePlyInfoInDatabase(0, 0, 0xfffe));			// fails, since value is invalid
	EXPECT_EQ(db.getNumWonStates(2),		0);						// fails, since layer 2 does not exist
	EXPECT_EQ(db.getNumLostStates(2),		0);						// fails, since layer 2 does not exist
	EXPECT_EQ(db.getNumDrawnStates(2),		0);						// fails, since layer 2 does not exist
	EXPECT_EQ(db.getNumInvalidStates(2),	0);						// fails, since layer 2 does not exist
	EXPECT_EQ(db.getNumberOfKnots(2), 0);							// none, since layer 2 does not exist
	EXPECT_EQ(db.getLayerSizeInBytes(2), 0);						// none, since layer 2 does not exist
	EXPECT_EQ(db.getSuccLayers(2).size(), 0);						// none, since layer 2 does not exist
	EXPECT_FALSE(db.isLayerCompleteAndInFile(2));					// fails, since layer 2 does not exist
	db.showLayerStats(2);											// check if it crashes, since layer 2 does not exist
	EXPECT_EQ(db.arrayInfos.anyArrayInfoToUpdate(), false);			// no array info to update
	EXPECT_EQ(db.arrayInfos.getMemoryUsed(), 0);					// no memory used
	EXPECT_EQ(db.arrayInfos.getArrayInfoForUpdate().arrayInfo, nullptr);	// no array info to update
																	// forget to close the database
}

TEST_F(MiniMaxDatabase_databaseTest, positiveTests)
{
	// positive tests
	EXPECT_TRUE(db.openDatabase(tmpFileDirectory));					// create a new database
	EXPECT_TRUE(std::filesystem::exists(tmpFileDirectory + L"\\shortKnotValue.dat"));	// check if the file exists
	EXPECT_TRUE(std::filesystem::exists(tmpFileDirectory + L"\\plyInfo.dat"));			// check if the file exists
	EXPECT_TRUE(db.isOpen());										// database is open
	EXPECT_TRUE(db.saveHeader());									// save the header, although it is empty
	EXPECT_FALSE(db.saveLayerToFile(0));							// fail to save the layer, since it is empty
	EXPECT_EQ(db.getNumWonStates(0),		0);						// 0, since nothing stored yet
	EXPECT_EQ(db.getNumLostStates(0),		0);						// 0, since nothing stored yet
	EXPECT_EQ(db.getNumDrawnStates(0),		0);						// 0, since nothing stored yet
	EXPECT_EQ(db.getNumInvalidStates(0),	0);						// 0, since nothing stored yet
	EXPECT_TRUE(db.updateLayerStats(0));							// update the layer stats is also possible, when the layer is not saved yet
	EXPECT_FALSE(db.setAsComplete());								// fail, since layer has not been save to file yet
	EXPECT_FALSE(db.isComplete());									// check if the database is complete
	EXPECT_FALSE(db.isLayerCompleteAndInFile(0));					// expect false, since nothing stored yet
	EXPECT_EQ(db.getNumberOfKnots(0), 100);							// check the number of knots in layer 0. value comes from gameInterfaceStub
	EXPECT_EQ(db.getNumLayers(), 2);								// check the number of layers. value comes from gameInterfaceStub.
	EXPECT_THAT(db.getPartnerLayers(0), ElementsAre(1));			// check the partner layer of layer 0
	EXPECT_EQ(db.getMemoryUsed(), 25);								// check the memory used
	EXPECT_EQ(db.getLayerSizeInBytes(0), 225);						// = (100+3)/4 + 100*2 bytes
	EXPECT_EQ(db.getSuccLayers(0).size(), 2);						// check the successor layers

	// read from empty database
	EXPECT_TRUE(db.readKnotValueFromDatabase(0, 0, dbByte));		// read a knot value from the database. invalid value, since nothing stored yet
	EXPECT_EQ(dbByte, SKV_VALUE_INVALID);							// compare the two values
	EXPECT_EQ(db.getMemoryUsed(), 25);								// now, the skv info of layer 0 should be in memory
	EXPECT_TRUE(db.readPlyInfoFromDatabase(0, 0, plyInfoVar));		// read a ply value from the database. invalid value, since nothing stored yet
	EXPECT_EQ(plyInfoVar, PLYINFO_VALUE_UNCALCULATED);				// it's not an an invalid state, but uncalculated
	EXPECT_EQ(db.getMemoryUsed(), 225);								// now, the skv and plyinfo of layer 0 should be in memory

	// save and read knot value
	EXPECT_TRUE(db.writeKnotValueInDatabase(0, 99, SKV_VALUE_GAME_WON));	// save a knot value in the database
	EXPECT_TRUE(db.writeKnotValueInDatabase(0, 0, SKV_VALUE_GAME_LOST));	// save a knot value in the database	
	EXPECT_TRUE(db.writeKnotValueInDatabase(0, 3, SKV_VALUE_GAME_LOST));	// save a knot value in the database	
	EXPECT_TRUE(db.writeKnotValueInDatabase(0, 5, SKV_VALUE_GAME_DRAWN));// save a knot value in the database	
	EXPECT_TRUE(db.writeKnotValueInDatabase(0, 6, SKV_VALUE_GAME_DRAWN));// save a knot value in the database	
	EXPECT_TRUE(db.writeKnotValueInDatabase(0, 7, SKV_VALUE_GAME_DRAWN));// save a knot value in the database	
	EXPECT_TRUE(db.writePlyInfoInDatabase(0, 99, 22));				// save a ply value in the database
	EXPECT_EQ(db.getMemoryUsed(), 225);								// now, the skv info of layer 0 should be in memory
	EXPECT_TRUE(db.readKnotValueFromDatabase(0, 99, dbByte));		// read a knot value from the database. invalid value, since nothing stored yet
	EXPECT_EQ(dbByte, SKV_VALUE_GAME_WON);							// compare the two values
	EXPECT_TRUE(db.readPlyInfoFromDatabase(0, 99, plyInfoVar));		// read a ply value from the database. invalid value, since nothing stored yet
	EXPECT_EQ(plyInfoVar, 22);										// expect invalid value
	EXPECT_TRUE(db.saveLayerToFile(0));								// save the layer
	EXPECT_TRUE(db.isLayerCompleteAndInFile(0));					// expect true, since layer 0 is stored
	EXPECT_FALSE(db.isLayerCompleteAndInFile(1));					// expect false, since layer 1 is not saved to file
	EXPECT_TRUE(db.updateLayerStats(0));							// update the layer stats
	EXPECT_EQ(db.getNumWonStates(0),		1);						// check the number of won states
	EXPECT_EQ(db.getNumLostStates(0),		2);						// check the number of lost states
	EXPECT_EQ(db.getNumDrawnStates(0),		3);						// check the number of drawn states
	EXPECT_EQ(db.getNumInvalidStates(0), 100-6);					// check the number of invalid states
	db.showLayerStats(0);											// now, the layer stats should be shown with some values
	db.unload();													// unload the data from memory
	EXPECT_TRUE(db.saveHeader());									// save the header, with some data in it. but after unloading the data from memory.
	EXPECT_EQ(db.getMemoryUsed(), 0);								// check the memory used, nothing is stored yet. only header stuff so far.
	EXPECT_TRUE(db.closeDatabase());								// close the database

	// open database again, check data again, and check layerStats
	EXPECT_TRUE(db.openDatabase(tmpFileDirectory));					// open the database again
	EXPECT_FALSE(db.isComplete());									// was set to true before closing the database
	EXPECT_TRUE(db.isLayerCompleteAndInFile(0));					// expect true, since layer is stored
	EXPECT_FALSE(db.isLayerCompleteAndInFile(1));					// expect false, since layer was not stored
	EXPECT_EQ(db.getNumWonStates(0),		1);						// read from file
	EXPECT_EQ(db.getNumLostStates(0),		2);						// read from file
	EXPECT_EQ(db.getNumDrawnStates(0),		3);						// read from file
	EXPECT_EQ(db.getNumInvalidStates(0), 100-6);					// read from file
	EXPECT_EQ(db.getNumWonStates(1),		0);						// nothing stored yet
	EXPECT_EQ(db.getNumLostStates(1),		0);						// nothing stored yet	
	EXPECT_EQ(db.getNumDrawnStates(1),		0);						// nothing stored yet	
	EXPECT_EQ(db.getNumInvalidStates(1),  	0);						// nothing stored yet	
	EXPECT_EQ(db.getNumberOfKnots(0), 100);							// check the number of knots in layer 0. value comes from gameInterfaceStub
	EXPECT_EQ(db.getNumberOfKnots(1), 200);							// check the number of knots in layer 1. value comes from gameInterfaceStub
	EXPECT_EQ(db.getLayerSizeInBytes(0), 225);						// = (100+3)/4 + 100*2 bytes
	EXPECT_EQ(db.getLayerSizeInBytes(1), 450);						// = (200+3)/4 + 200*2 bytes
	EXPECT_EQ(db.getSuccLayers(0).size(), 2);						// check the successor layers
	EXPECT_EQ(db.getSuccLayers(1).size(), 2);						// check the successor layers
	EXPECT_THAT(db.getPartnerLayers(0), ElementsAre(1));			// check the partner layer of layer 0
	EXPECT_THAT(db.getPartnerLayers(1), ElementsAre(0));			// check the partner layer of layer 1
	EXPECT_EQ(db.getMemoryUsed(), 0);								// check the memory used, nothing is stored yet. only header stuff so far.
	EXPECT_TRUE(db.readKnotValueFromDatabase(0, 3, dbByte));		// read a knot value from the database. invalid value, since nothing stored yet
	EXPECT_EQ(dbByte, SKV_VALUE_GAME_LOST);							// compare the two values
	EXPECT_TRUE(db.readPlyInfoFromDatabase(0, 99, plyInfoVar));		// read a ply value from the database. invalid value, since nothing stored yet
	EXPECT_EQ(plyInfoVar, 22);										// expect invalid value
	EXPECT_TRUE(db.readKnotValueFromDatabase(1, 3, dbByte));		// read a knot value from the database. invalid value, since nothing stored yet
	EXPECT_EQ(dbByte, SKV_VALUE_INVALID);							// expect invalid value, since layer 1 is not stored so far
	EXPECT_TRUE(db.readPlyInfoFromDatabase(1, 99, plyInfoVar));		// read a ply value from the database. invalid value, since nothing stored yet
	EXPECT_EQ(plyInfoVar, PLYINFO_VALUE_UNCALCULATED);				// layer 1 is not calculated so for
	db.unload();													// unload the data from memory
	EXPECT_EQ(db.getMemoryUsed(), 0);								// check the memory used, nothing is stored yet. only header stuff so far.
	EXPECT_TRUE(db.writePlyInfoInDatabase(1, 77, 33));				// save a ply value in the database
	EXPECT_EQ(db.getMemoryUsed(), 400);								// now, the ply info of layer 1 should be in memory
	EXPECT_TRUE(db.readKnotValueFromDatabase(0, 3, dbByte));		// read a knot value from the database. invalid value, since nothing stored yet
	EXPECT_EQ(dbByte, SKV_VALUE_GAME_LOST);							// compare the two values
	EXPECT_TRUE(db.readPlyInfoFromDatabase(0, 99, plyInfoVar));		// read a ply value from the database. invalid value, since nothing stored yet
	EXPECT_EQ(plyInfoVar, 22);										// expect invalid value
	EXPECT_FALSE(db.saveLayerToFile(1));							// fail to save layer 1, since skv is still empty
	EXPECT_TRUE(db.writeKnotValueInDatabase(1, 6, SKV_VALUE_GAME_DRAWN));	// save a ply value in the database
	EXPECT_TRUE(db.saveLayerToFile(1));								// save the layer
	EXPECT_TRUE(db.updateLayerStats(1));							// update the layer stats
	EXPECT_TRUE(db.setAsComplete());								// now both layers are complete
	EXPECT_TRUE(db.isComplete());									// check if the database is complete
	db.showLayerStats(0);											// now, the layer stats should be shown with some values
	db.showLayerStats(1);											// now, the layer stats should be shown with some values
	EXPECT_TRUE(db.closeDatabase());								// close the database

	// delete the database files
	EXPECT_TRUE(db.openDatabase(tmpFileDirectory));					// open the database again
	EXPECT_TRUE(db.removeDatabaseFiles());							// remove the database files
	EXPECT_FALSE(std::filesystem::exists(tmpFileDirectory + L"\\shortKnotValue.dat"));	// check if the file exists
	EXPECT_FALSE(std::filesystem::exists(tmpFileDirectory + L"\\plyInfo.dat"));			// check if the file exists
	EXPECT_TRUE(db.closeDatabase());								// close the database

	// open database again. but now the database is empty again.
	EXPECT_TRUE(db.openDatabase(tmpFileDirectory));					// open the database again
	EXPECT_FALSE(db.isComplete());									// should be false, since the database was removed
	EXPECT_FALSE(db.isLayerCompleteAndInFile(0));					// expect false, since layer was not stored
	EXPECT_FALSE(db.isLayerCompleteAndInFile(1));					// expect false, since layer was not stored
	EXPECT_EQ(db.getNumWonStates(0),		0);						// nothing stored yet
	EXPECT_EQ(db.getNumLostStates(0),		0);						// nothing stored yet
	EXPECT_EQ(db.getNumDrawnStates(0),		0);						// nothing stored yet
	EXPECT_EQ(db.getNumInvalidStates(0),    0);						// nothing stored yet
	EXPECT_TRUE(db.readKnotValueFromDatabase(0, 3, dbByte));		// read a knot value from the database. invalid value, since nothing stored yet
	EXPECT_EQ(dbByte, SKV_VALUE_INVALID);							// expect invalid value
	EXPECT_TRUE(db.readPlyInfoFromDatabase(1, 77, plyInfoVar));		// read a ply value from the database. invalid value, since nothing stored yet
	EXPECT_EQ(plyInfoVar, PLYINFO_VALUE_UNCALCULATED);				// layer 1 is not calculated so for
	EXPECT_TRUE(db.closeDatabase());								// close the database
}

TEST_F(MiniMaxDatabase_databaseTest, saveToFileTest)
{
	// create a new database, store some data in it, and save it to file
	EXPECT_TRUE(db.openDatabase(tmpFileDirectory));							// create a new database
	EXPECT_TRUE(db.writeKnotValueInDatabase(0, 99, SKV_VALUE_GAME_WON));	// save a knot value in the database
	EXPECT_TRUE(db.writeKnotValueInDatabase(0, 0, SKV_VALUE_GAME_LOST));	// save a knot value in the database
	EXPECT_TRUE(db.writeKnotValueInDatabase(0, 3, SKV_VALUE_GAME_LOST));	// save a knot value in the database
	EXPECT_TRUE(db.writeKnotValueInDatabase(0, 5, SKV_VALUE_GAME_DRAWN));	// save a knot value in the database
	EXPECT_TRUE(db.writeKnotValueInDatabase(0, 6, SKV_VALUE_GAME_DRAWN));	// save a knot value in the database
	EXPECT_TRUE(db.writeKnotValueInDatabase(0, 7, SKV_VALUE_GAME_DRAWN));	// save a knot value in the database
	EXPECT_TRUE(db.writePlyInfoInDatabase(0, 99, 22));						// save a ply value in the database
	EXPECT_TRUE(db.writePlyInfoInDatabase(1, 99, 26));						// save a ply value in the database
	EXPECT_TRUE(db.saveLayerToFile(0));										// save the layer
	EXPECT_FALSE(db.saveLayerToFile(1));									// fail, since skv is missing in layer 1
	EXPECT_TRUE(db.updateLayerStats(0));									// update the layer stats for layer 0
	EXPECT_TRUE(db.updateLayerStats(1));									// update the layer stats for layer 1
	EXPECT_TRUE(db.saveHeader());											// save the header

	// continue to store some data in it, but do not save it to file this time and close the database
	EXPECT_FALSE(db.writeKnotValueInDatabase(0, 99, SKV_VALUE_GAME_LOST));	// fail, since layer already stored
	EXPECT_FALSE(db.writeKnotValueInDatabase(0, 1, SKV_VALUE_GAME_LOST));	// fail, since layer already stored
	EXPECT_TRUE(db.writePlyInfoInDatabase(1, 199, 44));						// ply info of layer 1 is still writable
	EXPECT_TRUE(db.writeKnotValueInDatabase(1, 7, SKV_VALUE_GAME_WON));		// save a knot value in the database
	EXPECT_TRUE(db.saveLayerToFile(1));										// now, layer 1 is stored
	EXPECT_TRUE(db.updateLayerStats(1));									// now, layer 1 is complete
	EXPECT_TRUE(db.setAsComplete());										// set the database as complete
																			// forget to save the header
	EXPECT_TRUE(db.closeDatabase());										// close the database

	// check if the data is still there, and if is equal to the state after saving the layer to file
	EXPECT_TRUE(db.openDatabase(tmpFileDirectory));							// open the database again
	EXPECT_FALSE(db.isComplete());											// was set to true before closing the database
	EXPECT_TRUE(db.isLayerCompleteAndInFile(0));							// expect true, since layer is stored
	EXPECT_FALSE(db.isLayerCompleteAndInFile(1));							// expect false, since layer was not stored
	EXPECT_EQ(db.getNumWonStates(0),		1);								// read from file
	EXPECT_EQ(db.getNumLostStates(0),		2);								// read from file
	EXPECT_EQ(db.getNumDrawnStates(0),		3);								// read from file
	EXPECT_EQ(db.getNumInvalidStates(0), 100-6);							// read from file
	EXPECT_EQ(db.getNumWonStates(1),		0);								// nothing stored yet
	EXPECT_EQ(db.getNumLostStates(1),		0);								// nothing stored yet
	EXPECT_EQ(db.getNumDrawnStates(1),		0);								// nothing stored yet
	EXPECT_EQ(db.getNumInvalidStates(1),  200);								// default value
	EXPECT_TRUE(db.readKnotValueFromDatabase(0, 99, dbByte));				// read a knot value from the database. invalid value, since nothing stored yet
	EXPECT_EQ(dbByte, SKV_VALUE_GAME_WON);									// expect invalid value
	EXPECT_TRUE(db.readPlyInfoFromDatabase(1, 199, plyInfoVar));			// read a ply value from the database. invalid value, since nothing stored yet
	EXPECT_EQ(plyInfoVar, PLYINFO_VALUE_UNCALCULATED);						// expect uncalulated state
	EXPECT_TRUE(db.readKnotValueFromDatabase(0, 3, dbByte));				// read a knot value from the database. invalid value, since nothing stored yet
	EXPECT_EQ(dbByte, SKV_VALUE_GAME_LOST);									// compare the two values
	db.closeDatabase();														// close the database
}

} // namespace miniMax

#pragma endregion
