#/*********************************************************************\
	xml.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/
#ifndef XMLCLASS_H
#define XMLCLASS_H

#include <windows.h>
#include <fstream>
#include <cstring>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdio>
#include <list>
#include <algorithm>
#include <vector>
#include <Shlwapi.h>		// needs library file: Shlwapi.lib
#include <XmlLite.h>		// needs library file: XmlLite.lib

using namespace std;

//-----------------------------------------------------------------------------
// This class is a simple xml-parser and xml-writer. 
// - It is based on the XmlLite library from Microsoft. 
// - The class is able to read and write xml-files. 
// - The class is able to handle unicode strings. 
// - The class is able to handle attributes and subnodes. 
// - The class is able to handle the root node. 
// - The class is able to handle empty elements. 
//-----------------------------------------------------------------------------
class xmlClass;

class xmlAttribute
{
friend class xmlClass;
friend class xmlNode;

	string				strNameA;
	wstring				strNameW;
	string				strValueA;
	wstring				strValueW;

public:
						xmlAttribute			(const WCHAR *attributeName, const WCHAR *attributeValue);
						xmlAttribute			(const WCHAR *attributeName, double attributeValue);
						~xmlAttribute			();
	string				valueA					();
	wstring				valueW					();
	bool				exists					();
	bool				valueEqual				(const char  *cStr);
	bool				valueEqual				(const WCHAR *cStr);
};

class xmlNode
{
friend class xmlClass;
private:
	list<xmlNode*>		subNodes;
	list<xmlAttribute*>	attributes;
	string				strValueA;
	wstring				strValueW;
	string				strNodeNameA;
	wstring				strNodeNameW;
	xmlClass *			parentClass				= nullptr;
	xmlNode *			parentNode				= nullptr;
	
public:
						xmlNode					(const WCHAR *nodeName, xmlClass *parentClass, xmlNode *parentNode);
						~xmlNode				();
	xmlNode	*			node					(unsigned int position);
	xmlNode	*			node					(const char  *cStr, unsigned int position);
	xmlNode	*			node					(const char  *cStr);
	xmlNode	*			node					(const WCHAR *cStr, unsigned int position);
	xmlNode	*			node					(const WCHAR *cStr);
	xmlAttribute *		attribute				(const char  *cStr);
	xmlAttribute *		attribute				(const WCHAR *cStr);
	string				valueA					();
	wstring				valueW					();
	string				nameA					();
	wstring				nameW					();
	bool				exists					();
	bool				valueEqual				(const char  *cStr);
	bool				valueEqual				(const WCHAR *cStr);
	bool				nodeNameEqual			(const char  *cStr);
	bool				nodeNameEqual			(const WCHAR *cStr);
	unsigned int		getNumSubNodes			();
	unsigned int		getNumSubNodes			(const char *subNodeName);
	void				addAttribute			(xmlAttribute *newAttribute);
	void				addSubNode				(xmlNode* subNode);
	void				addSubNodeWithAttribute	(const WCHAR *cSubNodeName, const WCHAR *cAttributeName, const WCHAR* valueW);
	void				addSubNodeWithAttribute	(const WCHAR *cSubNodeName, const WCHAR *cAttributeName, const int	  value);
	void				setValue				(const WCHAR *cStr);
};

class xmlClass
{
friend class xmlNode;
private:
	xmlAttribute		dummyAttribute;
	xmlNode				dummyNode;
	xmlNode				rootNode;
	IXmlReader		*	pReader					= nullptr;
	IXmlWriter		*	pWriter					= nullptr;
	
	bool				loadFileSubFunction		(xmlNode * curNode);
	bool				writeFileSubFunction	(xmlNode * curNode);

public:
						xmlClass				();
						xmlClass				(wstring const& sourceFile, xmlNode* &theRootNode);
						~xmlClass				();

	bool				loadFile				(wstring const& sourceFile);
	bool				writeFile				(wstring const& destFile);
	xmlNode	*			getRootNode				();
};

#endif
