/*********************************************************************
	xml.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/

#include "xml.h"

#pragma region xmlClass

//-----------------------------------------------------------------------------
// Name: xmlClass()
// Desc: class constructor
//-----------------------------------------------------------------------------
xmlClass::xmlClass() : dummyNode(L"dummyNode", this, NULL), rootNode(L"rootNode", this, NULL), dummyAttribute(L"dummyAttribute", L"dummyValue")
{
	dummyNode.parentNode = &dummyNode;
	CreateXmlReader(__uuidof(IXmlReader), (void**) &pReader, NULL);
	CreateXmlWriter(__uuidof(IXmlWriter), (void**) &pWriter, NULL);
	pReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit);
	pWriter->SetProperty(XmlWriterProperty_Indent, TRUE);
}

//-----------------------------------------------------------------------------
// Name: xmlClass()
// Desc: class constructor
//-----------------------------------------------------------------------------
xmlClass::xmlClass(wstring const& sourceFile, xmlNode* &theRootNode) : xmlClass()
{
	if (!loadFile(sourceFile)) return;
	theRootNode	= getRootNode()->node("dataroot")->node("rootNode");
}

//-----------------------------------------------------------------------------
// Name: xmlClass()
// Desc: class deconstructor
//-----------------------------------------------------------------------------
xmlClass::~xmlClass()
{
}

//-----------------------------------------------------------------------------
// Name: writeFile()
// Desc: Writes the xml-tree to a file. 
//-----------------------------------------------------------------------------
bool xmlClass::writeFile(wstring const& destFile)
{
	// locals
	IStream  *		pFileStream;
	bool			returnValue;

	// open file
	SHCreateStreamOnFile(destFile.c_str(), STGM_WRITE | STGM_CREATE, &pFileStream);
	if (pFileStream==NULL) { 
		cout << "ERROR: xmlClass::writeFile(): Could not open file stream. Probably wrong filepath." << endl;
		return false;
	}

	pWriter->SetOutput(pFileStream);
	pWriter->WriteStartDocument(XmlStandalone_Yes);
	pWriter->WriteStartElement(NULL, L"dataroot", NULL);
	pWriter->WriteAttributeString(NULL, L"xmlns:dt", NULL, L"urn:schemas-microsoft-com:datatypes");
	pWriter->WriteWhitespace(L"\n");

	// process xml-tree
	returnValue = writeFileSubFunction(&rootNode);
	
	// flush
	pWriter->WriteEndDocument();
	pWriter->Flush();

	// close stream
	pFileStream->Release();
	pWriter->Release(); 

	return returnValue;
}

//-----------------------------------------------------------------------------
// Name: writeFileSubFunction()
// Desc: Helper function for writeFile(). 
//-----------------------------------------------------------------------------
bool xmlClass::writeFileSubFunction(xmlNode * curNode)
{
	// locals
	list<xmlNode*>::iterator		itrSubNodes;
	list<xmlAttribute*>::iterator	itrAttributes;

	// node name
	pWriter->WriteStartElement(NULL, curNode->strNodeNameW.c_str(), NULL);

	// attributes
	for (itrAttributes=curNode->attributes.begin(); itrAttributes!=curNode->attributes.end(); itrAttributes++) {
		pWriter->WriteAttributeString(NULL, (*itrAttributes)->strNameW.c_str(), NULL, (*itrAttributes)->strValueW.c_str());
	}

	// node value
	pWriter->WriteString(curNode->strValueW.c_str());

	// sub nodes
	for (itrSubNodes=curNode->subNodes.begin(); itrSubNodes!=curNode->subNodes.end(); itrSubNodes++) {
		writeFileSubFunction(*itrSubNodes);
	}

	// end element
	pWriter->WriteFullEndElement();
	pWriter->WriteWhitespace(L"\n");

	return true;
}

//-----------------------------------------------------------------------------
// Name: loadFile()
// Desc: Loads an xml-file into the xml-tree.
//-----------------------------------------------------------------------------
bool xmlClass::loadFile(wstring const& sourceFile)
{
	// locals
	IStream  *		pFileStream;
	bool			returnValue;

	// open file
	SHCreateStreamOnFile(sourceFile.c_str(), STGM_READ, &pFileStream);
	if (pFileStream == NULL) { 
		cout << "ERROR: xmlClass::loadFile(): Could not open file stream. Probably wrong filepath." << endl;
		return false;
	}
	pReader->SetInput(pFileStream);

	// process xml-tree
	returnValue = loadFileSubFunction(&rootNode);

	// close stream
	pFileStream->Release();
	pReader->Release();

	return returnValue;
}

//-----------------------------------------------------------------------------
// Name: loadFileSubFunction()
// Desc: Helper function for loadFile().
//-----------------------------------------------------------------------------
bool xmlClass::loadFileSubFunction(xmlNode * curNode)
{
	// locals
	XmlNodeType		nodeType;
	xmlNode *		subNode;
	xmlAttribute *	newAttribute;
	PCWSTR			pLocalName;
	PCWSTR			pValue;
	UINT			attributeCount;
	UINT			curAttribute; 
	BOOL			isEmptyElement;

	// process until end of file
	while (!pReader->IsEOF()) {
		
		// read
		pReader->Read(&nodeType);

		switch (nodeType) 
		{
		case XmlNodeType_XmlDeclaration:
			break;
		case XmlNodeType_Element:

			// element without closing tag?
			isEmptyElement = pReader->IsEmptyElement();

			// add node
			pReader->GetLocalName(&pLocalName, NULL);
			subNode = new xmlNode(pLocalName, this, curNode);
			curNode->addSubNode(subNode);

			// add attributes
			pReader->GetAttributeCount(&attributeCount); 
			if (attributeCount) {
				pReader->MoveToFirstAttribute();
				for (curAttribute=0; curAttribute<attributeCount; curAttribute++, pReader->MoveToNextAttribute()) {
					pReader->GetLocalName(&pLocalName, NULL);
					pReader->GetValue(&pValue, NULL);
					newAttribute = new xmlAttribute(pLocalName, pValue); 
					subNode->addAttribute(newAttribute);
				}
			}
			
			// go deeper if element is not empty
			if (!isEmptyElement) {
				loadFileSubFunction(subNode);
			} 
			break;

		case XmlNodeType_EndElement:
			return true;
		case XmlNodeType_Text:
			pReader->GetValue(&pLocalName, NULL);
			curNode->setValue(pLocalName);
			break;
		case XmlNodeType_Whitespace:
			break;
		case XmlNodeType_ProcessingInstruction:
			break;
		case XmlNodeType_Comment:
			break;
		case XmlNodeType_DocumentType:
			break;
		case XmlNodeType_CDATA:
			break;
		case XmlNodeType_Attribute:
			break;
		case XmlNodeType_None:
			break; // return true;
		}
	};

	return true;
}

//-----------------------------------------------------------------------------
// Name: getRootNode()
// Desc: Returns a pointer on the root node.
//-----------------------------------------------------------------------------
xmlNode	* xmlClass::getRootNode()
{
	return &rootNode;
}

#pragma endregion

#pragma region xmlNode

//-----------------------------------------------------------------------------
// Name: setValue()
// Desc: Sets the value of a node.
//-----------------------------------------------------------------------------
void xmlNode::setValue(const WCHAR *cStr)
{
	size_t length = wcslen(cStr) + 1;
	char *cTmp = new char[length];
	wcstombs(cTmp, cStr, length);
	strValueW.assign(cStr);
	strValueA.assign(cTmp);
	delete cTmp;
}

//-----------------------------------------------------------------------------
// Name: xmlNode()
// Desc: class constructor
//-----------------------------------------------------------------------------
xmlNode::xmlNode(const WCHAR *nodeName, xmlClass *parentClass, xmlNode *parentNode)
{
	size_t length = wcslen(nodeName) + 1;
	char *cTmp = new char[length];
	wcstombs(cTmp, nodeName, length);
	this->strNodeNameW.assign(nodeName);
	this->strNodeNameA.assign(cTmp);
	this->parentClass	= parentClass;
	this->parentNode	= parentNode;
	delete cTmp;
}

//-----------------------------------------------------------------------------
// Name: xmlNode()
// Desc: class constructor
//-----------------------------------------------------------------------------
xmlNode::~xmlNode()
{
	list<xmlNode*>     ::iterator itrNode;
	list<xmlAttribute*>::iterator itrAttr;

	for (itrNode=subNodes.begin(); itrNode!=subNodes.end(); itrNode++) {
		delete *itrNode;
	}
	for (itrAttr=attributes.begin();itrAttr!=attributes.end(); itrAttr++) {
		delete *itrAttr;
	}
	attributes.clear();
	subNodes.clear();
}

//-----------------------------------------------------------------------------
// Name: addSubNode()
// Desc: Adds a subnode to the node. 
//-----------------------------------------------------------------------------
void xmlNode::addSubNode(xmlNode* subNode)
{
	subNodes.push_back(subNode);
}

//-----------------------------------------------------------------------------
// Name: addSubNodeWithAttribute()
// Desc: Adds a subnode to the node. 
//-----------------------------------------------------------------------------
void xmlNode::addSubNodeWithAttribute(const WCHAR *cSubNodeName, const WCHAR *cAttributeName, const WCHAR* valueW)
{
	xmlNode* myNode	= node(cSubNodeName);
	if (myNode->nameW().compare(L"dummyNode") == 0) {
		myNode = new xmlNode(cSubNodeName, parentClass, parentNode);
		myNode->addAttribute(new xmlAttribute(cAttributeName, valueW));
		addSubNode(myNode);
	} else {
		myNode->addAttribute(new xmlAttribute(cAttributeName, valueW));
	}
}

//-----------------------------------------------------------------------------
// Name: addSubNodeWithAttribute()
// Desc: Adds a subnode to the node. 
//-----------------------------------------------------------------------------
void xmlNode::addSubNodeWithAttribute(const WCHAR *cSubNodeName, const WCHAR *cAttributeName, const int value)
{
	xmlNode* myNode = node(cSubNodeName);
	if (myNode->nameW().compare(L"dummyNode") == 0) {
		myNode = new xmlNode(cSubNodeName, parentClass, parentNode);
		myNode->addAttribute(new xmlAttribute(cAttributeName, value));
		addSubNode(myNode);
	} else {
		myNode->addAttribute(new xmlAttribute(cAttributeName, value));
	}
}

//-----------------------------------------------------------------------------
// Name: addAttribute
// Desc: Adds an attribute to the node. 
//-----------------------------------------------------------------------------
void xmlNode::addAttribute(xmlAttribute *newAttribute)
{
	attributes.push_back(newAttribute);
}

//-----------------------------------------------------------------------------
// Name: node()
// Desc: Returns the position-th subnode named as 'cStr'.
//-----------------------------------------------------------------------------
xmlNode* xmlNode::node(unsigned int position)
{
	// locals
	list<xmlNode*>::iterator itr;
	unsigned int posCounter;

	for (itr=subNodes.begin(), posCounter=0; itr!=subNodes.end(); itr++) {
		if (position==posCounter) {
			break;
		} else {
			posCounter++;
		}
	}
	return (itr!=subNodes.end() ? *itr : &parentClass->dummyNode);
}

//-----------------------------------------------------------------------------
// Name: node()
// Desc: Returns the position-th subnode named as 'cStr'.
//-----------------------------------------------------------------------------
xmlNode* xmlNode::node(const char *cStr, unsigned int position)
{
	// locals
	list<xmlNode*>::iterator itr;
	unsigned int posCounter;

	for (itr=subNodes.begin(), posCounter=0; itr!=subNodes.end(); itr++) {
		if ((*itr)->strNodeNameA.compare(cStr)==0) {
			if (position==posCounter) {
				break;
			} else {
				posCounter++;
			}
		}
	}
	return (itr!=subNodes.end() ? *itr : &parentClass->dummyNode);
}

//-----------------------------------------------------------------------------
// Name: node()
// Desc: Returns the position-th subnode named as 'cStr'.
//-----------------------------------------------------------------------------
xmlNode* xmlNode::node(const WCHAR *cStr, unsigned int position)
{
	// locals
	list<xmlNode*>::iterator itr;
	unsigned int posCounter;

	for (itr=subNodes.begin(), posCounter=0; itr!=subNodes.end(); itr++) {
		if ((*itr)->strNodeNameW.compare(cStr)==0) {
			if (position==posCounter) {
				break;
			} else {
				posCounter++;
			}
		}
	}
	return (itr!=subNodes.end() ? *itr : &parentClass->dummyNode);
}

//-----------------------------------------------------------------------------
// Name: attribute()
// Desc: Returns the first attribute named as 'cStr'.
//-----------------------------------------------------------------------------
xmlAttribute* xmlNode::attribute(const char *cStr)
{
	// locals
	list<xmlAttribute*>::iterator itr;

	for (itr=attributes.begin(); itr!=attributes.end(); itr++) {
		if ((*itr)->strNameA.compare(cStr)==0) {
			break;
		}
	}
	return (itr!=attributes.end() ? *itr : &parentClass->dummyAttribute);
}

//-----------------------------------------------------------------------------
// Name: attribute()
// Desc: Returns the first attribute named as 'cStr'.
//-----------------------------------------------------------------------------
xmlAttribute* xmlNode::attribute(const WCHAR *cStr)
{
	// locals
	list<xmlAttribute*>::iterator itr;

	for (itr=attributes.begin(); itr!=attributes.end(); itr++) {
		if ((*itr)->strNameW.compare(cStr)==0) {
			break;
		}
	}
	return (itr!=attributes.end() ? *itr : &parentClass->dummyAttribute);
}

//-----------------------------------------------------------------------------
// Name: node()
// Desc: Returns the first subnode named as 'cStr'.
//-----------------------------------------------------------------------------
xmlNode* xmlNode::node(const char *cStr)
{
	return node(cStr, 0);
}

//-----------------------------------------------------------------------------
// Name: node()
// Desc: Returns the first subnode named as 'cStr'.
//-----------------------------------------------------------------------------
xmlNode* xmlNode::node(const WCHAR *cStr)
{
	return node(cStr, 0);
}

//-----------------------------------------------------------------------------
// Name: value()
// Desc: Returns the value of the node.
//-----------------------------------------------------------------------------
string xmlNode::valueA()
{
	return strValueA;
}

//-----------------------------------------------------------------------------
// Name: value()
// Desc: Returns the value of the node.
//-----------------------------------------------------------------------------
wstring xmlNode::valueW()
{
	return strValueW;
}

//-----------------------------------------------------------------------------
// Name: nameA()
// Desc: Returns the name of the node.
//-----------------------------------------------------------------------------
string xmlNode::nameA()
{
	return strNodeNameA;
}

//-----------------------------------------------------------------------------
// Name: nameW()
// Desc: Returns the name of the node.
//-----------------------------------------------------------------------------
wstring xmlNode::nameW()
{
	return strNodeNameW;
}

//-----------------------------------------------------------------------------
// Name: valueEqual()
// Desc: Returns true if this is not the dummynode.
//-----------------------------------------------------------------------------
bool xmlNode::exists()
{
	return (strNodeNameW.compare(L"dummyNode") != 0);
}

//-----------------------------------------------------------------------------
// Name: valueEqual()
// Desc: Returns true if the value of the node is equal 'cStr'.
//-----------------------------------------------------------------------------
bool xmlNode::nodeNameEqual(const char *cStr)
{
	string valA = strNodeNameA;
	string valB(cStr);
	transform(valA.begin(), valA.end(), valA.begin(),::tolower);
	transform(valB.begin(), valB.end(), valB.begin(),::tolower);
	return (valA.compare(valB) == 0);
}

//-----------------------------------------------------------------------------
// Name: valueEqual()
// Desc: Returns true if the value of the node is equal 'cStr'.
//-----------------------------------------------------------------------------
bool xmlNode::nodeNameEqual(const WCHAR *cStr)
{
	wstring valA = strNodeNameW;
	wstring valB(cStr);
	transform(valA.begin(), valA.end(), valA.begin(),::tolower);
	transform(valB.begin(), valB.end(), valB.begin(),::tolower);
	return (valA.compare(valB) == 0);
}

//-----------------------------------------------------------------------------
// Name: valueEqual()
// Desc: Returns true if the value of the node is equal 'cStr'.
//-----------------------------------------------------------------------------
bool xmlNode::valueEqual(const char *cStr)
{
	string valA = strValueA;
	string valB(cStr);
	transform(valA.begin(), valA.end(), valA.begin(),::tolower);
	transform(valB.begin(), valB.end(), valB.begin(),::tolower);
	return (valA.compare(valB) == 0);
}

//-----------------------------------------------------------------------------
// Name: valueEqual()
// Desc: Returns true if the value of the node is equal 'cStr'.
//-----------------------------------------------------------------------------
bool xmlNode::valueEqual(const WCHAR *cStr)
{
	wstring valA = strValueW;
	wstring valB(cStr);
	transform(valA.begin(), valA.end(), valA.begin(),::tolower);
	transform(valB.begin(), valB.end(), valB.begin(),::tolower);
	return (valA.compare(valB) == 0);
}

//-----------------------------------------------------------------------------
// Name: getNumSubNodes()
// Desc: Returns the number of subnodes. 
//-----------------------------------------------------------------------------
unsigned int xmlNode::getNumSubNodes()
{
	return (unsigned int) subNodes.size();
}

//-----------------------------------------------------------------------------
// Name: getNumSubNodes()
// Desc: Returns the number of subnodes with the passed string as name.
//-----------------------------------------------------------------------------
unsigned int xmlNode::getNumSubNodes(const char *subNodeName)
{
	// locals
	list<xmlNode*>::iterator itr;
	unsigned int numSubNodes;

	for (itr=subNodes.begin(), numSubNodes=0; itr!=subNodes.end(); itr++) {
		if ((*itr)->strNodeNameA.compare(subNodeName)==0) {
			numSubNodes++;
		}
	}

	return numSubNodes;
}

#pragma endregion

#pragma region xmlAttribute

//-----------------------------------------------------------------------------
// Name: xmlAttribute()
// Desc: Constructor 
//-----------------------------------------------------------------------------
xmlAttribute::xmlAttribute(const WCHAR *attributeName, double attributeValue)
{
	wstringstream wss;

	wss.str(L"");
	wss << attributeValue;
	this->strValueW.assign(wss.str());

	size_t lengthName, lengthValue;
	lengthName  = wcslen(attributeName)  + 1;
	lengthValue = this->strValueW.size() + 1;
	char *cTmp  = new char[max(lengthName,lengthValue)];

	wcstombs(cTmp, attributeName,  lengthName);
	this->strNameA.assign(cTmp);
	this->strNameW.assign(attributeName);

	wcstombs(cTmp, this->strValueW.c_str(), lengthValue);
	this->strValueA.assign(cTmp);
	//this->strValueW.assign(attributeValue);

	delete cTmp;
}

//-----------------------------------------------------------------------------
// Name: xmlAttribute()
// Desc: Constructor
//-----------------------------------------------------------------------------
xmlAttribute::xmlAttribute(const WCHAR *attributeName, const WCHAR *attributeValue)
{
	size_t lengthName, lengthValue;
	lengthName  = wcslen(attributeName)  + 1;
	lengthValue = wcslen(attributeValue) + 1;
	char *cTmp  = new char[max(lengthName,lengthValue)];

	wcstombs(cTmp, attributeName,  lengthName);
	this->strNameA.assign(cTmp);
	this->strNameW.assign(attributeName);

	wcstombs(cTmp, attributeValue, lengthValue);
	this->strValueA.assign(cTmp);
	this->strValueW.assign(attributeValue);

	delete cTmp;
}

//-----------------------------------------------------------------------------
// Name: xmlAttribute()
// Desc: Destructor 
//-----------------------------------------------------------------------------
xmlAttribute::~xmlAttribute()
{
}

//-----------------------------------------------------------------------------
// Name: valueA()
// Desc: Returns the value of the node.
//-----------------------------------------------------------------------------
string xmlAttribute::valueA()
{
	return strValueA;
}

//-----------------------------------------------------------------------------
// Name: valueW()
// Desc: Returns the value of the node.
//-----------------------------------------------------------------------------
wstring xmlAttribute::valueW()
{
	return strValueW;
}

//-----------------------------------------------------------------------------
// Name: exists()
// Desc: Returns the value of the node.
//-----------------------------------------------------------------------------
bool xmlAttribute::exists()
{
	return (strNameW.compare(L"dummyAttribute") != 0);
}

//-----------------------------------------------------------------------------
// Name: valueEqual()
// Desc: Returns true if the value of the node is equal 'cStr'.
//-----------------------------------------------------------------------------
bool xmlAttribute::valueEqual(const char *cStr)
{
	string valA = strValueA;
	string valB(cStr);
	transform(valA.begin(), valA.end(), valA.begin(),::tolower);
	transform(valB.begin(), valB.end(), valB.begin(),::tolower);
	return (valA.compare(valB) == 0);
}

//-----------------------------------------------------------------------------
// Name: valueEqual()
// Desc: Returns true if the value of the node is equal 'cStr'.
//-----------------------------------------------------------------------------
bool xmlAttribute::valueEqual(const WCHAR *cStr)
{
	wstring valA = strValueW;
	wstring valB(cStr);
	transform(valA.begin(), valA.end(), valA.begin(),::tolower);
	transform(valB.begin(), valB.end(), valB.begin(),::tolower);
	return (valA.compare(valB) == 0);
}

#pragma endregion
