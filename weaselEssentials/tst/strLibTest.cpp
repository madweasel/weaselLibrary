/**************************************************************************************************************************
	strLibTest.cpp
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
***************************************************************************************************************************/
#include "gtest/gtest.h"
#include <vector>
#include <Windows.h>
#include <filesystem>

#include "strLib.h"

TEST(strLibTest, mystring) {
	EXPECT_STREQ(mystring("Hello").c_strA(), "Hello");
	EXPECT_STREQ(mystring(L"Hello").c_strW(), L"Hello");
	EXPECT_STREQ(mystring(L"Hello").c_strA(), "Hello");
	EXPECT_STREQ(mystring(L"Hello").c_strW(), L"Hello");
	EXPECT_STREQ(mystring().c_strA(), nullptr);
	EXPECT_STREQ(mystring().c_strW(), nullptr);
	EXPECT_STREQ(mystring(L"World").assign("Hello").c_strA(), "Hello");
	EXPECT_STREQ(mystring().assign(L"Hello").c_strW(), L"Hello");
}

TEST(strLibTest, readAsciiData) {
	const std::wstring filename = (std::filesystem::temp_directory_path() / "strLibTest.txt").c_str();
	vector<double> data(10);

	// invalid parameters
	EXPECT_FALSE(readAsciiData(nullptr, data.data(), 10, '.', ','));
	EXPECT_FALSE(readAsciiData(INVALID_HANDLE_VALUE, data.data(), 10, '.', ','));
	
	// test with a file
	HANDLE hFile = CreateFile(filename.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	EXPECT_TRUE(hFile != nullptr);
	DWORD dwBytesWritten;
	WriteFile(hFile, "1.2,3.4,5.6,7.8,9.0", 20, &dwBytesWritten, nullptr);
	CloseHandle(hFile);

	hFile = CreateFile(filename.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	EXPECT_TRUE(readAsciiData(hFile, data.data(), 5, '.', ','));
	EXPECT_DOUBLE_EQ(data[0], 1.2);
	EXPECT_DOUBLE_EQ(data[1], 3.4);
	EXPECT_DOUBLE_EQ(data[2], 5.6);
	EXPECT_DOUBLE_EQ(data[3], 7.8);
	EXPECT_DOUBLE_EQ(data[4], 9.0);
	CloseHandle(hFile);
}
