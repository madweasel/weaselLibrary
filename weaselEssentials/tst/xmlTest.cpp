/**************************************************************************************************************************
	xmlTest.cpp
	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
***************************************************************************************************************************/
#include "gtest/gtest.h"
#include <filesystem>

#include "xml.h"

const char exampleXml[] = 
	"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\
	<dataroot>\n\
		<rootNode>\n\
			<subNode1 attribute1=\"value1\" attribute2=\"value2\">value1</subNode1>\n\
			<subNode2 attribute1=\"value1\" attribute2=\"value2\">value2</subNode2>\n\
			<subNode2 attribute2=\"value2\" attribute4=\"value4\">value4</subNode2>\n\
			<subNode3 attribute1=\"value1\" attribute2=\"value2\">value3</subNode3>\n\
		</rootNode>\n\
	</dataroot>";

class xmlTest : public ::testing::Test {
protected:
	xmlClass xml;
	const std::wstring exampleFileName  = (std::filesystem::temp_directory_path() / "temp_test.xml").c_str();
	const std::wstring outputFileName   = (std::filesystem::temp_directory_path() / "temp_test_output.xml").c_str();

	void createExampleFile() {
		wofstream file;
		file.open(exampleFileName.c_str());
		file << exampleXml;
		file.close();
	}

	void SetUp() override {
		createExampleFile();
	}

	void TearDown() override {
		std::filesystem::remove(exampleFileName.c_str());
	}
};

TEST_F(xmlTest, LoadFile) {
	xmlNode* rootNode = nullptr;
	xmlClass xml(exampleFileName.c_str(), rootNode);
	ASSERT_NE(rootNode, nullptr);
	ASSERT_TRUE(rootNode->exists());
	ASSERT_EQ(rootNode->nameW(), L"rootNode");
	ASSERT_EQ(rootNode->nameA(), "rootNode");
}

TEST_F(xmlTest, WriteFile) {
	xmlNode* rootNode = nullptr;
	xmlClass xml(exampleFileName.c_str(), rootNode);
	ASSERT_TRUE(xml.writeFile(outputFileName.c_str()));

	xmlNode* newRootNode = nullptr;
	xmlClass newXml(outputFileName.c_str(), newRootNode);
	ASSERT_NE(newRootNode, nullptr);
	ASSERT_TRUE(newRootNode->exists());
	ASSERT_EQ(newRootNode->nameW(), L"rootNode");
	std::filesystem::remove(outputFileName.c_str());
}

TEST_F(xmlTest, NodeAttributes) {
	xmlNode* rootNode = nullptr;
	xmlClass xml(exampleFileName.c_str(), rootNode);
	ASSERT_NE(rootNode, nullptr);

	xmlNode* subNode1 = rootNode->node(L"subNode1");
	ASSERT_NE(subNode1, nullptr);
	ASSERT_TRUE(subNode1->exists());

	xmlAttribute* attr1 = subNode1->attribute(L"attribute1");
	ASSERT_NE(attr1, nullptr);
	ASSERT_TRUE(attr1->exists());
	ASSERT_EQ(attr1->valueW(), L"value1");
	ASSERT_EQ(attr1->valueA(), "value1");

	xmlAttribute* attr2 = subNode1->attribute(L"attribute2");
	ASSERT_NE(attr2, nullptr);
	ASSERT_TRUE(attr2->exists());
	ASSERT_EQ(attr2->valueW(), L"value2");
}

TEST_F(xmlTest, NodeValues) {
	xmlNode* rootNode = nullptr;
	xmlClass xml(exampleFileName.c_str(), rootNode);
	ASSERT_NE(rootNode, nullptr);

	xmlNode* subNode1 = rootNode->node(L"subNode1");
	ASSERT_NE(subNode1, nullptr);
	ASSERT_TRUE(subNode1->exists());
	ASSERT_EQ(subNode1->valueW(), L"value1");
	ASSERT_EQ(subNode1->valueA(), "value1");

	xmlNode* subNode2 = rootNode->node(L"subNode2");
	ASSERT_NE(subNode2, nullptr);
	ASSERT_TRUE(subNode2->exists());
	ASSERT_EQ(subNode2->valueW(), L"value2");

	xmlNode* subNode3 = rootNode->node(L"subNode3");
	ASSERT_NE(subNode3, nullptr);
	ASSERT_TRUE(subNode3->exists());
	ASSERT_EQ(subNode3->valueW(), L"value3");
}

TEST_F(xmlTest, AddSubNodeWithAttribute) {
	xmlNode* rootNode = nullptr;
	xmlClass xml(exampleFileName.c_str(), rootNode);
	ASSERT_NE(rootNode, nullptr);

	rootNode->addSubNodeWithAttribute(L"newSubNode", L"newAttribute", L"newValue");
	xmlNode* newSubNode = rootNode->node(L"newSubNode");
	ASSERT_NE(newSubNode, nullptr);
	ASSERT_TRUE(newSubNode->exists());

	xmlAttribute* newAttr = newSubNode->attribute(L"newAttribute");
	ASSERT_NE(newAttr, nullptr);
	ASSERT_TRUE(newAttr->exists());
	ASSERT_EQ(newAttr->valueW(), L"newValue");
}

TEST_F(xmlTest, AddSubNodeWithAttributeInt) {
	xmlNode* rootNode = nullptr;
	xmlClass xml(exampleFileName.c_str(), rootNode);
	ASSERT_NE(rootNode, nullptr);

	rootNode->addSubNodeWithAttribute(L"newSubNode", L"newAttribute", 123);
	xmlNode* newSubNode = rootNode->node(L"newSubNode");
	ASSERT_NE(newSubNode, nullptr);
	ASSERT_TRUE(newSubNode->exists());

	xmlAttribute* newAttr = newSubNode->attribute(L"newAttribute");
	ASSERT_NE(newAttr, nullptr);
	ASSERT_TRUE(newAttr->exists());
	ASSERT_EQ(newAttr->valueW(), L"123");
	ASSERT_EQ(newAttr->valueA(), "123");
}

TEST_F(xmlTest, nodeNameEqual) {
	xmlNode* rootNode = nullptr;
	xmlClass xml(exampleFileName.c_str(), rootNode);
	ASSERT_NE(rootNode, nullptr);

	ASSERT_TRUE(rootNode->nodeNameEqual("rootNode"));
	ASSERT_TRUE(rootNode->nodeNameEqual(L"rootNode"));
	ASSERT_FALSE(rootNode->nodeNameEqual("dummyNode"));
	ASSERT_FALSE(rootNode->nodeNameEqual(L"dummyNode"));

	ASSERT_TRUE(rootNode->node(L"subNode1")->nodeNameEqual("subNode1"));
	ASSERT_TRUE(rootNode->node(L"subNode1")->nodeNameEqual(L"subNode1"));
	ASSERT_FALSE(rootNode->node(L"subNode1")->nodeNameEqual("dummyNode"));
	ASSERT_FALSE(rootNode->node(L"subNode1")->nodeNameEqual(L"dummyNode"));

	ASSERT_TRUE(rootNode->getNumSubNodes() == 4);
	ASSERT_TRUE(rootNode->getNumSubNodes("subNode1") == 1);
}

TEST_F(xmlTest, valueEqual) {
	xmlNode* rootNode = nullptr;
	xmlClass xml(exampleFileName.c_str(), rootNode);
	ASSERT_NE(rootNode, nullptr);

	ASSERT_TRUE(rootNode->valueEqual(""));
	ASSERT_TRUE(rootNode->valueEqual(L""));
	ASSERT_FALSE(rootNode->valueEqual("rootNode"));
	ASSERT_FALSE(rootNode->valueEqual(L"rootNode"));

	xmlNode* subNode1 = rootNode->node(L"subNode1");
	xmlAttribute* attr1 = subNode1->attribute(L"attribute1");
	ASSERT_TRUE(attr1->valueEqual("value1"));
	ASSERT_TRUE(attr1->valueEqual(L"value1"));
	ASSERT_FALSE(attr1->valueEqual("value2"));
	ASSERT_FALSE(attr1->valueEqual(L"value2"));
}

TEST_F(xmlTest, attributeFunc) {
	xmlNode* rootNode = nullptr;
	xmlClass xml(exampleFileName.c_str(), rootNode);
	ASSERT_NE(rootNode, nullptr);

	xmlAttribute* attr1 = rootNode->attribute("subNode1");
	ASSERT_NE(attr1, nullptr);
	ASSERT_FALSE(attr1->exists());

	xmlAttribute* attr2 = rootNode->node("subNode2")->attribute("attribute1");
	ASSERT_NE(attr2, nullptr);
	ASSERT_TRUE(attr2->exists());
	ASSERT_EQ(attr2->valueW(), L"value1");
}

TEST_F(xmlTest, nodePosition) {
	xmlNode* rootNode = nullptr;
	xmlClass xml(exampleFileName.c_str(), rootNode);
	ASSERT_NE(rootNode, nullptr);

	xmlNode* subNode1 = rootNode->node("subNode1", 0);
	ASSERT_NE(subNode1, nullptr);
	ASSERT_TRUE(subNode1->exists());
	ASSERT_EQ(subNode1->valueW(), L"value1");

	xmlNode* subNode2 = rootNode->node("subNode2", 1);
	ASSERT_NE(subNode2, nullptr);
	ASSERT_TRUE(subNode2->exists());
	ASSERT_EQ(subNode2->valueW(), L"value4");

	xmlNode* subNode3 = rootNode->node(1);
	ASSERT_NE(subNode3, nullptr);
	ASSERT_TRUE(subNode3->exists());
	ASSERT_EQ(subNode3->valueA(), "value2");
}
