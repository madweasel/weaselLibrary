/***************************************************************************************************************************
	compressorTest.cpp
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
***************************************************************************************************************************/

#define _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING
#include "gtest/gtest.h"
#include <vector>
#include <string>

#include "compLib_winCompApi.h"

class CompressorTest : public ::testing::Test {
public:
	static const int fileSize;					// size of the file in bytes
	static const int blockSize;					// size of the buffer in bytes, which must be smaller than the file size

protected:
	compressor::winCompApi		comp;					// compressor
	compressor::file*			pFile 		= nullptr;	// compressed file
	const std::wstring 			tmpFolder 	= (std::filesystem::temp_directory_path() / "compressor").c_str();
	const std::wstring 			fileName 	= (std::filesystem::temp_directory_path() / "temp_test_file.dat").c_str();
	std::vector<unsigned char> 	writeData;				// random data to write
	std::vector<unsigned char> 	readData;				// random data to read

	void SetUp() override {
		// fill the write and read data with random values
		writeData.resize(fileSize);
		readData .resize(fileSize);
		std::generate(writeData.begin(), writeData.end(), []() { return static_cast<unsigned char>(rand() % 256); });
		std::generate(readData.begin(),  readData.end(),  []() { return static_cast<unsigned char>(rand() % 256); });
		// delete the file if it exists
		if (std::filesystem::exists(fileName)) {
			std::filesystem::remove(fileName);
		}
		if (std::filesystem::exists(tmpFolder)) {
			std::filesystem::remove_all((std::filesystem::temp_directory_path() / "compressor").c_str());
		}
		// create the compressed file
		pFile = new compressor::file{comp};
		pFile->setBlockSize(blockSize);
	}

	void TearDown() override {
		if (pFile) delete pFile;
	}
};

const int CompressorTest::fileSize 	= 69;
const int CompressorTest::blockSize = 20;

TEST_F(CompressorTest, NegativeTest)
{
	compressor::file& file = *pFile;
	EXPECT_FALSE(file.close());															// close file which is not open
	EXPECT_FALSE(file.isOpen());														// file is not open
	EXPECT_EQ(file.getKeys().size(), 0);												// no keys in the file
	EXPECT_FALSE(file.doesKeyExist(L"key"));											// key does not exist
	EXPECT_FALSE(file.read(L"key", 1000, 10000, nullptr));								// try to read data from non existing key
	EXPECT_FALSE(file.write(L"123456 Bytes", 0, 123456, nullptr));						// write nullptr
	EXPECT_EQ(file.getSizeOfUncompressedSection(L""), 0);								// size of the uncompressed section
	EXPECT_EQ(file.getSizeOfCompressedSection(L"key"), 0);								// size of the compressed section, and non existing key
	EXPECT_FALSE(file.flush());															// flush file which is not open
	EXPECT_TRUE(file.open(fileName, false));											// open file for writing				
	EXPECT_FALSE(file.write(L"writeData", 0, fileSize, nullptr));						// write data
	EXPECT_FALSE(file.read(L"writeData", 0, fileSize, nullptr));						// try to read data from non existing key
																						// forget to close the file on purpose. This should happen automatically.
}

TEST_F(CompressorTest, PositiveTest)
{
	std::vector<char> abactData(3, 0);
	compressor::file& file = *pFile;

	EXPECT_TRUE(file.open(fileName, false));											// open file for writing				
	EXPECT_TRUE(file.isOpen());															// file is open
	EXPECT_TRUE(file.write(L"abc", 1, 3, "abc"));										// write some string data
	EXPECT_TRUE(file.doesKeyExist(L"abc"));												// key exists
	EXPECT_FALSE(file.read(L"writeData", 0, fileSize, readData.data()));				// try to read data from non existing key
	EXPECT_TRUE(file.write(L"writeData", 0, fileSize, writeData.data()));				// write data
	EXPECT_TRUE(file.setBlockSize(CompressorTest::blockSize));							// setting block size after writing data to empty compressed file is still ok
	EXPECT_EQ(file.getKeys().size(), 2);												// two keys in the file
	EXPECT_EQ(file.getSizeOfUncompressedSection(L"writeData"), fileSize);				// size of the uncompressed section, which is still not flushed, thus only a temporary file
	EXPECT_LE(file.getSizeOfCompressedSection(L"writeData"), 0);						// size of the compressed section is 0 bytes since the file is not flushed yet
	EXPECT_TRUE(file.read(L"writeData", 10, fileSize-10, &readData[10]));				// read fraction of data
	std::copy(writeData.begin(), writeData.begin() + 10, readData.begin());				// copy rest of fraction
	EXPECT_TRUE(std::equal(writeData.begin(), writeData.end(), readData.begin()));		// compare data
	EXPECT_TRUE(file.flush());															// flush data to disk
	EXPECT_EQ(file.getSizeOfUncompressedSection(L"writeData"), fileSize);				// size of the uncompressed section
	EXPECT_LE(file.getSizeOfCompressedSection(L"writeData"), 3*fileSize);				// size of the compressed section
	EXPECT_FALSE(file.setBlockSize(132));												// setting block size after flushing is nok
	EXPECT_TRUE(file.write(L"abc", 2, 1, "d"));											// overwrite the 2nd byte of the string "abc" with "d"
	EXPECT_GT(std::filesystem::file_size(fileName), 0);									// file size is greater than 0
	EXPECT_TRUE(file.read(L"abc", 1, 3, abactData.data()));								// read some string data
	EXPECT_TRUE(std::equal(abactData.begin(), abactData.end(), "adc"));					// compare data
	{																					// check the key names
		auto keys = file.getKeys();
		EXPECT_EQ(keys.size(), 2);
		EXPECT_EQ(keys[0], L"abc");
		EXPECT_EQ(keys[1], L"writeData");
	}
	EXPECT_TRUE(file.close());															// close file
	EXPECT_FALSE(file.isOpen());														// file is not open anymore

	// reopen file again and check if the data is still there
	EXPECT_TRUE(file.open(fileName, true));												// open file for reading
	EXPECT_TRUE(file.isOpen());															// file is open
	EXPECT_FALSE(file.write(L"writeData", 10, fileSize-10, writeData.data()));			// fail to write data to read only file
	EXPECT_FALSE(file.flush());															// fail to flush file which is read only
	EXPECT_FALSE(file.setBlockSize(132));												// fail to setting block size after flushing is nok
	EXPECT_EQ(file.getKeys().size(), 2);												// two keys in the file
	EXPECT_EQ(file.getSizeOfUncompressedSection(L"writeData"), fileSize);				// size of the uncompressed section
	EXPECT_LE(file.getSizeOfCompressedSection(L"writeData"), 3*fileSize);				// size of the compressed section
	EXPECT_TRUE(file.read(L"writeData", 0, fileSize, readData.data()));					// read data
	EXPECT_TRUE(std::equal(writeData.begin(), writeData.end(), readData.begin()));		// compare data
	EXPECT_TRUE(file.doesKeyExist(L"abc"));												// key exists
	EXPECT_TRUE(file.read(L"abc", 1, 3, abactData.data()));								// read some string data
	EXPECT_TRUE(std::equal(abactData.begin(), abactData.end(), "adc"));					// compare data
	EXPECT_TRUE(file.close());															// close file
	EXPECT_FALSE(file.isOpen());														// file is not open anymore
}

// initialize the actual data with the expected data, but without the data from startPos to (startPos + numBytes)
void actDataInit(std::vector<char>& actData, std::vector<char>& expData, long long startPos, long long numBytes)
{
	// zero the actual data
	actData.assign(123456, 0);
	// copy data from the expected data to the actual  data until startPos
	copy(expData.begin(), expData.begin() + startPos, actData.begin());
	// copy data from the expected data to the actual  data starting from (startPos + numBytes) until end
	copy(expData.begin() + startPos + numBytes, expData.end(), actData.begin() + startPos + numBytes);
}

TEST_F(CompressorTest, TestName) 
{
	compressor::file& 			file 			= *pFile;			// compressed file
	unsigned int				myBlockSize		= 10000;			// size of the block in bytes, which must be smaller than the file size
	std::vector<char>			expData			(123456, 0);		// actual data read from the file
	std::vector<char>			actData			(123456, 0);		// expected data for the comparison

	// create random data
	srand(0);
	std::generate(expData.begin(), expData.end(), []() { return static_cast<char>(rand() % 256); });

	// delete test file
	remove("compTestFile.dat");

	// open empty file
	ASSERT_EQ(file.open(std::wstring(L"compTestFile.dat"), false),			true);			// open file for writing
	ASSERT_EQ(file.getSizeOfUncompressedSection(L""),						0);				// size of the uncompressed section is 0 bytes
	ASSERT_EQ(file.getSizeOfCompressedSection(L""),							0);				// size of the compressed section is 0 bytes
	ASSERT_EQ(file.read(L"key", 1000, 10000, nullptr),						false);			// try to read data from non existing key
	ASSERT_EQ(file.read(L"key", 1000, 10000, &expData[0]),					false);			// try to read data from non existing key
	ASSERT_EQ(file.setBlockSize(123),										true);			// setting block size is ok
	ASSERT_EQ(file.setBlockSize(myBlockSize),								true);			// setting block size is ok
	ASSERT_EQ(file.write(L"123456 Bytes", 	0, 123456,	&expData[0]),		true);			// write some data
	ASSERT_EQ(file.setBlockSize(456),										true);			// setting block size is ok, since the file is not flushed yet
	ASSERT_EQ(file.write(L"123456 Bytes", 	0, 123456,	&expData[0]),		true);			// write some data again
	ASSERT_EQ(file.write(L"10001 Byte", 	0, 10001,	&expData[0]),		true);			// write some data
	ASSERT_EQ(file.write(L"9999 Byte", 		0, 9999,	&expData[0]),		true);			// write some data
	ASSERT_EQ(file.write(L"1 Byte", 		0, 1,		&expData[0]),		true);			// write some data
	ASSERT_EQ(file.write(L"0 Bytes", 		0, 0,		&expData[0]),		false);			// fail to write no data
	// read all five sections, which were written to the file before
	actDataInit(actData, expData, 0     , 123456);	ASSERT_EQ(file.read(L"123456 Bytes"	, 0     , 123456,	&actData[0]),			true);		ASSERT_EQ(equal(expData.begin(), expData.end(), actData.begin()),			true);		
	actDataInit(actData, expData, 0     , 10001 );	ASSERT_EQ(file.read(L"10001 Byte"	, 0     , 10001 ,	&actData[0]),			true);		ASSERT_EQ(equal(expData.begin(), expData.end(), actData.begin()),			true);		
	actDataInit(actData, expData, 0     , 9999  );	ASSERT_EQ(file.read(L"9999 Byte"	, 0     , 9999  ,	&actData[0]),			true);		ASSERT_EQ(equal(expData.begin(), expData.end(), actData.begin()),			true);		
	actDataInit(actData, expData, 0     , 1     );	ASSERT_EQ(file.read(L"1 Byte"		, 0     , 1     ,	&actData[0]),			true);		ASSERT_EQ(equal(expData.begin(), expData.end(), actData.begin()),			true);		
	actDataInit(actData, expData, 0     , 0     );	ASSERT_EQ(file.read(L"0 Bytes"		, 0     , 0     ,	&actData[0]),			false);		ASSERT_EQ(equal(expData.begin(), expData.end(), actData.begin()),			true);		
	// read at different positions
	actDataInit(actData, expData, 123454, 2     );	ASSERT_EQ(file.read(L"123456 Bytes"	, 123454, 2     ,	&actData[123454]),		true);		ASSERT_EQ(equal(expData.begin(), expData.end(), actData.begin()),			true);		
	actDataInit(actData, expData, 100000, 23456 );	ASSERT_EQ(file.read(L"123456 Bytes"	, 100000, 23456 ,	&actData[100000]),		true);		ASSERT_EQ(equal(expData.begin(), expData.end(), actData.begin()),			true);		
	actDataInit(actData, expData, 0     , 123456);	ASSERT_EQ(file.read(L"123456 Bytes"	, 0     , 123456,	&actData[0]),			true);		ASSERT_EQ(equal(expData.begin(), expData.end(), actData.begin()),			true);		
	actDataInit(actData, expData, 10000 , 1     );	ASSERT_EQ(file.read(L"10001 Byte"	, 10000 , 1     ,	&actData[10000]),		true);		ASSERT_EQ(equal(expData.begin(), expData.end(), actData.begin()),			true);		
	actDataInit(actData, expData, 10000 , 0     );	ASSERT_EQ(file.read(L"9999 Byte"	, 10000 , 1     ,	&actData[0]),			false);		ASSERT_EQ(equal(expData.begin(), expData.end(), actData.begin()),			true);		
	actDataInit(actData, expData, 1     , 0     );	ASSERT_EQ(file.read(L"1 Byte"		, 1     , 1     ,	&actData[0]),			false);		ASSERT_EQ(equal(expData.begin(), expData.end(), actData.begin()),			true);		
	actDataInit(actData, expData, 1     , 0     );	ASSERT_EQ(file.read(L"0 Bytes"		, 1     , 1     ,	&actData[0]),			false);		ASSERT_EQ(equal(expData.begin(), expData.end(), actData.begin()),			true);		
	ASSERT_EQ(file.close(),													true);			// close file
	
	// file is closed and should not be usable any more
	ASSERT_EQ(file.close(),													false);			// fail to close file which is not open
	ASSERT_EQ(file.setBlockSize(123),										true);			// setting block size is ok, since the file is not open
	ASSERT_EQ(file.getSizeOfUncompressedSection(L"123456 Bytes"),			0);				// size of the uncompressed section is 0 bytes, since the file is not open
	ASSERT_EQ(file.getSizeOfCompressedSection(L"123456 Bytes"),				0);				// size of the compressed section is 0 bytes, since the file is not open
	// fail to read closed file. thereby the actData must not be changed
	actDataInit(actData, expData, 0     , 0);		ASSERT_EQ(file.read(L"123456 Bytes"	, 0     , 123456,	&actData[0]),			false);		ASSERT_EQ(equal(expData.begin(), expData.end(), actData.begin()),			true);		
	ASSERT_EQ(file.write(L"123 Bytes", 0, 123,	&expData[0]),				false);			// fail to write data to non existing file

	// open created test file again in read only mode
	ASSERT_EQ(file.open(std::wstring(L"compTestFile.dat"), true),			true);			// open file for reading	
	ASSERT_EQ(file.setBlockSize(456),										false);			// setting block size is nok, since the file is already open
	ASSERT_EQ(file.write(L"0 Bytes", 	0, 0,	&expData[0]),				false);			// fail to write no data
	ASSERT_EQ(file.write(L"30 Bytes", 	0, 30,	&expData[0]),				false);			//fail to write data, since the file is read only
	// fail to read data from non existing key
	actDataInit(actData, expData, 0     , 0     );	ASSERT_EQ(file.read(L"30 Bytes"		, 0     , 3     ,	&actData[0]),			false);		ASSERT_EQ(equal(expData.begin(), expData.end(), actData.begin()),			true);		
	// successfully read data from existing key
	actDataInit(actData, expData, 0     , 123456);	ASSERT_EQ(file.read(L"123456 Bytes"	, 0     , 123456,	&actData[0]),			true);		ASSERT_EQ(equal(expData.begin(), expData.end(), actData.begin()),			true);		
	ASSERT_EQ(file.close(),													true);			// close file

	// open created test file again in write mode
	ASSERT_EQ(file.open(std::wstring(L"compTestFile.dat"), true),			true);			// open file for reading
	ASSERT_EQ(file.open(std::wstring(L"compTestFile.dat"), false),			false);			// fail to open file for writing, since it is already open
	ASSERT_EQ(file.close(),													true);			// close file
	ASSERT_EQ(file.open(std::wstring(L"compTestFile.dat"), false),			true);			// open file for writing
	ASSERT_EQ(file.setBlockSize(456),										false);			// setting block size is nok, since the file already contains data
	ASSERT_EQ(file.write(L"0 Bytes", 0, 0,		&expData[0]),				false);			// fail to write no data
	ASSERT_EQ(file.write(L"30 Bytes", 0, 30,	&expData[0]),				true);			// write some data
	ASSERT_EQ(file.getSizeOfUncompressedSection(L""),						0);				// size of the uncompressed section is 0 bytes, when no key is given
	ASSERT_EQ(file.getSizeOfUncompressedSection(L"30 Bytes"),				30);			// size of the uncompressed section is 30 bytes
	ASSERT_EQ(file.getSizeOfUncompressedSection(L"123454 Bytes"),			0);				// size of the uncompressed section is 0 bytes, when the key does not exist
	ASSERT_EQ(file.getSizeOfUncompressedSection(L"123456 Bytes"),			123456);		// size of the uncompressed section is 123456 bytes
	// read all five sections, which were written to the file before
	actDataInit(actData, expData, 0     , 123456);	ASSERT_EQ(file.read(L"123456 Bytes"	, 0     , 123456,	&actData[0]),			true);		ASSERT_EQ(equal(expData.begin(), expData.end(), actData.begin()),			true);		
	actDataInit(actData, expData, 0     , 10001 );	ASSERT_EQ(file.read(L"10001 Byte"	, 0     , 10001 ,	&actData[0]),			true);		ASSERT_EQ(equal(expData.begin(), expData.end(), actData.begin()),			true);		
	actDataInit(actData, expData, 0     , 9999  );	ASSERT_EQ(file.read(L"9999 Byte"	, 0     , 9999  ,	&actData[0]),			true);		ASSERT_EQ(equal(expData.begin(), expData.end(), actData.begin()),			true);		
	actDataInit(actData, expData, 0     , 1     );	ASSERT_EQ(file.read(L"1 Byte"		, 0     , 1     ,	&actData[0]),			true);		ASSERT_EQ(equal(expData.begin(), expData.end(), actData.begin()),			true);		
	actDataInit(actData, expData, 0     , 0     );	ASSERT_EQ(file.read(L"0 Bytes"		, 0     , 0     ,	&actData[0]),			false);		ASSERT_EQ(equal(expData.begin(), expData.end(), actData.begin()),			true);		
	// read at different positions
	actDataInit(actData, expData, 123454, 2     );	ASSERT_EQ(file.read(L"123456 Bytes"	, 123454, 2     ,	&actData[123454]),		true);		ASSERT_EQ(equal(expData.begin(), expData.end(), actData.begin()),			true);		
	actDataInit(actData, expData, 100000, 23456 );	ASSERT_EQ(file.read(L"123456 Bytes"	, 100000, 23456 ,	&actData[100000]),		true);		ASSERT_EQ(equal(expData.begin(), expData.end(), actData.begin()),			true);
	actDataInit(actData, expData, 0     , 123456);	ASSERT_EQ(file.read(L"123456 Bytes"	, 0     , 123456,	&actData[0]),			true);		ASSERT_EQ(equal(expData.begin(), expData.end(), actData.begin()),			true);		
	actDataInit(actData, expData, 10000 , 1     );	ASSERT_EQ(file.read(L"10001 Byte"	, 10000 , 1     ,	&actData[10000]),		true);		ASSERT_EQ(equal(expData.begin(), expData.end(), actData.begin()),			true);		
	actDataInit(actData, expData, 10000 , 0     );	ASSERT_EQ(file.read(L"9999 Byte"	, 10000 , 1     ,	&actData[0]),			false);		ASSERT_EQ(equal(expData.begin(), expData.end(), actData.begin()),			true);		
	actDataInit(actData, expData, 1     , 0     );	ASSERT_EQ(file.read(L"1 Byte"		, 1     , 1     ,	&actData[0]),			false);		ASSERT_EQ(equal(expData.begin(), expData.end(), actData.begin()),			true);		
	actDataInit(actData, expData, 1     , 0     );	ASSERT_EQ(file.read(L"0 Bytes"		, 1     , 1     ,	&actData[0]),			false);		ASSERT_EQ(equal(expData.begin(), expData.end(), actData.begin()),			true);		
	actDataInit(actData, expData, 0     , 30    );	ASSERT_EQ(file.read(L"30 Bytes"		, 0     , 30    ,	&actData[0]),			true);		ASSERT_EQ(equal(expData.begin(), expData.end(), actData.begin()),			true);		
	ASSERT_EQ(file.close(),													true);			// close file

	// open created test file again in read only mode
	ASSERT_EQ(file.open(std::wstring(L"compTestFile.dat"), true),			true);			// open file for reading
	actDataInit(actData, expData, 0     , 30    );	ASSERT_EQ(file.read(L"30 Bytes"		, 0     , 30    ,	&actData[0]),			true);		ASSERT_EQ(equal(expData.begin(), expData.end(), actData.begin()),			true);		
	ASSERT_EQ(file.write(L"30 Bytes", 0, 30,		&expData[0]),			false);			// fail to write data to read only file	
	actDataInit(actData, expData, 0     , 30    );	ASSERT_EQ(file.read(L"30 Bytes"		, 0     , 30    ,	&actData[0]),			true);		ASSERT_EQ(equal(expData.begin(), expData.end(), actData.begin()),			true);		
	ASSERT_EQ(file.close(),													true);			// close file
	
	// delete test file
	remove("compTestFile.dat");
}

TEST(sectionInfoTests, WriteData)
{
	compressor::file::sectionInfo 		sectionSingleThread;
	compressor::file::sectionInfo 		sectionMultiThread;
	std::fstream 						fsSingleThread(std::filesystem::temp_directory_path() / "singleThread.dat", std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);
	std::fstream 						fsMultiThread (std::filesystem::temp_directory_path() / "multiThread.dat",  std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);
	compressor::file::footerStruct 		footer;
	compressor::winCompApi	 			comp;
	compressor::file::tmpFile 			tmpFile(L"dummyKeyName");
	size_t 								testedBlockSize 	= 1000;
	size_t 								dataSize       		= 1234567;

	// create some random data
	srand(0);
	std::vector<char> data(dataSize, 0);
	for (auto& byte : data) {
		byte = rand() % 256;
	}
	// write the random data to the tmpFile
	tmpFile.write(0, data.size(), data.data());

	// setup the section info
	sectionSingleThread.offsetInFile      	= 0;
	sectionSingleThread.uncompressedSize  	= data.size();
	sectionSingleThread.compressedSize    	= 0; // will be set after compression
	sectionSingleThread.numBlocks         	= static_cast<unsigned int>((data.size() + testedBlockSize - 1) / testedBlockSize);
	sectionSingleThread.sectionId         	= 0;
	sectionSingleThread.keyLengthInBytes  	= static_cast<unsigned int>(std::wstring(L"dummyKeyName").size() * sizeof(wchar_t));
	sectionSingleThread.keyName           	= L"dummyKeyName";
	sectionSingleThread.blocks.resize(sectionSingleThread.numBlocks);
	sectionMultiThread	 					= sectionSingleThread;

	// setup the footer
	footer.typeId							= 0x7d67;
	footer.versionId						= 1;
	footer.numSections						= 1;
	footer.sectionsOffsetInFile				= 0;
	footer.footerOffsetInFile				= 0;
	footer.fileInfoOffsetInFile				= 0;
	footer.blockSizeInBytes					= testedBlockSize;	// set a block size smaller than the data size to enforce multi threading
	footer.usedLib							= compressor::generalLib::libId::undefined;
	footer.dictionary.clear();
	footer.sections.clear();

	// setup the uncompressed data
	ASSERT_EQ(fsSingleThread.is_open(), true);
	ASSERT_EQ(fsMultiThread .is_open(), true);

	// Test writing data with forceSingleThreading = true
	ASSERT_EQ(sectionSingleThread.writeData(fsSingleThread, footer, comp, tmpFile, true), true);

	// Test writing data with forceSingleThreading = false
	ASSERT_EQ(sectionMultiThread.writeData(fsMultiThread, footer, comp, tmpFile, false), true);

	// Now read back the data and compare
	fsSingleThread.seekg(0, std::ios::beg);
	fsMultiThread .seekg(0, std::ios::beg);
	std::vector<char> dataSingleThread((std::istreambuf_iterator<char>(fsSingleThread)), std::istreambuf_iterator<char>());
	std::vector<char> dataMultiThread ((std::istreambuf_iterator<char>(fsMultiThread)),  std::istreambuf_iterator<char>());
	ASSERT_EQ(dataSingleThread, dataMultiThread);
	fsSingleThread.close();
	fsMultiThread .close();
	remove(std::filesystem::temp_directory_path() / "singleThread.dat");
	remove(std::filesystem::temp_directory_path() / "multiThread.dat");
}