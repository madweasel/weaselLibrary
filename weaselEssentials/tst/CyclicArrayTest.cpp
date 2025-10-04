/**************************************************************************************************************************
	CyclicArrayTest.cpp
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
***************************************************************************************************************************/
#include "gtest/gtest.h"
#include <filesystem>
#include <vector>
#include <algorithm>

#include "cyclicArray.h"

class CyclicArrayTest : public ::testing::Test {
protected:
	logger log{logger::logLevel::none, logger::logType::console, L""};
	static const int blockSizeInBytes	= 100;				// size of a block in bytes
	static const int numberOfBlocks		= 20;				// number of blocks
	static const int fileSize			= 2000;				// size of the file in bytes
	const std::wstring fileName  = (std::filesystem::temp_directory_path() / "temp_test_file_CyclicArrayTest.txt").c_str();
	const std::wstring fileName2 = (std::filesystem::temp_directory_path() / "temp_test_file_CyclicArrayTest2.txt").c_str();
	cyclicArray* ca;
	std::vector<unsigned char> writeData;
	std::vector<unsigned char> readData;
	
	void SetUp() override {
		// fill the write and read data with random values
		writeData.resize(fileSize);
		readData .resize(fileSize);
		std::generate(writeData.begin(), writeData.end(), []() { return static_cast<unsigned char>(rand() % 256); });
		std::generate(readData.begin(),  readData.end(),  []() { return static_cast<unsigned char>(rand() % 256); });
		// delete the file if it exists
		std::filesystem::remove(fileName);
		std::filesystem::remove(fileName2);
		// create the buffered file
		ca = new cyclicArray(blockSizeInBytes, numberOfBlocks, fileName, log);
	}

	void TearDown() override {
		delete ca;
	}
};

TEST_F(CyclicArrayTest, AddAndTakeBytes) {
	ASSERT_FALSE(ca->bytesAvailable());										// no bytes available						
	ASSERT_EQ(ca->writeableBytes(), blockSizeInBytes * numberOfBlocks);		// writeable bytes should be the size of the cyclic array
	ASSERT_TRUE(ca->addBytes(writeData.size(), writeData.data()));			// add all bytes
	ASSERT_EQ(ca->writeableBytes(), 0);										// writeable bytes should be 0, since the cyclic array is full
	ASSERT_TRUE(ca->bytesAvailable());										// bytes should be available now
	ASSERT_TRUE(ca->takeBytes(readData.size(), readData.data()));			// take all bytes
	ASSERT_FALSE(ca->bytesAvailable());										// no bytes available anymore
	ASSERT_EQ(ca->writeableBytes(), blockSizeInBytes * numberOfBlocks);		// writeable bytes should be the size of the cyclic array
	ASSERT_EQ(writeData, readData);											// compare the data	
}

TEST_F(CyclicArrayTest, LoadFile) {
	uint64_t numBytesLoaded;
	ASSERT_TRUE(ca->addBytes(writeData.size(), writeData.data()));			// add all bytes
	ASSERT_TRUE(ca->saveFile(fileName2));									// save the file
	ASSERT_TRUE(ca->loadFile(fileName2, numBytesLoaded));					// load the file
	ASSERT_EQ(numBytesLoaded, writeData.size());							// check the number of bytes loaded
	ASSERT_EQ(ca->writeableBytes(), 0);										// writeable bytes should be 0, since the cyclic array is still full
	ASSERT_TRUE(ca->takeBytes(readData.size(), readData.data()));			// take all bytes
	ASSERT_EQ(writeData, readData);											// compare the data
	std::filesystem::remove(fileName2);										// delete the file
}

TEST_F(CyclicArrayTest, SaveFile) {
	uint64_t numBytesLoaded;													
	ASSERT_TRUE(ca->addBytes(writeData.size(), writeData.data()));			// add all bytes
	ASSERT_TRUE(ca->saveFile(fileName2));									// save the file
	cyclicArray ca2(blockSizeInBytes, numberOfBlocks, fileName, log);		// create a new cyclic array
	ASSERT_TRUE(ca2.loadFile(fileName2, numBytesLoaded));					// load the file, with the new cyclic array
	ASSERT_EQ(numBytesLoaded, writeData.size());							// check the number of bytes loaded	
	ASSERT_TRUE(ca2.takeBytes(readData.size(), readData.data()));			// take all bytes
	ASSERT_EQ(writeData, readData);											// compare the data
	std::filesystem::remove(fileName2);										// delete the file
}

// Test using only one block
TEST(CyclicArray, OneBlock) {
	logger log{logger::logLevel::none, logger::logType::console, L""};
	const std::wstring fileName  = (std::filesystem::temp_directory_path() / "temp_test_file_CyclicArrayTest.txt").c_str();
	const std::vector<unsigned char> writeData = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	const std::vector<unsigned int> blockSizeInBytes = {1, 2, 10};
	const std::vector<unsigned int> numberOfBlocks = {10, 5, 1}; 
	
	for (unsigned int i = 0; i < blockSizeInBytes.size(); i++) {
		std::vector<unsigned char> readData = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		std::filesystem::remove(fileName);										// delete the file if it exists
		cyclicArray ca(blockSizeInBytes[i], numberOfBlocks[i], fileName, log);	// create a new cyclic array with only one block
		ASSERT_TRUE(ca.addBytes(3, &writeData[0]));								// add 3 bytes
		ASSERT_EQ(ca.writeableBytes(), 7);										// writeable bytes should be 7
		ASSERT_EQ(ca.bytesAvailable(), 3);										// bytes should be available now
		ASSERT_TRUE(ca.addBytes(7, &writeData[3]));								// add 7 bytes
		ASSERT_EQ(ca.bytesAvailable(), 10);										// bytes should be available now
		ASSERT_EQ(ca.writeableBytes(), 0);										// writeable bytes should be 0, since the cyclic array is full
		ASSERT_TRUE(ca.takeBytes(6, &readData[0]));								// take 6 bytes
		ASSERT_EQ(ca.bytesAvailable(), 4);										// bytes should be available now
		ASSERT_EQ(ca.writeableBytes(), 6);										// writeable bytes should be 0, since the cyclic array is full
		ASSERT_TRUE(ca.takeBytes(4, &readData[6]));								// take 4 bytes
		ASSERT_EQ(ca.bytesAvailable(), 0);										// bytes should be available now
		ASSERT_EQ(ca.writeableBytes(), 10);										// writeable bytes should be 0, since the cyclic array is full
		ASSERT_EQ(writeData, readData);											// compare the data
	}
}
