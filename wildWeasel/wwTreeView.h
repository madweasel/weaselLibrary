/*********************************************************************\
	wwTreeView.h												  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/

#pragma once

#include "wildWeasel.h"
#include "wwListView.h"

namespace wildWeasel
{

// 2D plain tree view
class treeView2D : public listView2D
{
public:
	class branch : public row																			// a branch (the rowIndex here is within the list 'subBranches')
	{
	friend class treeView2D;

	public:
		static const unsigned int		LAST_SUB_BRANCH							= UINT32_MAX;

	private:
		plainButton2D*					plusMinus								= nullptr;				// plus button if there any subrows, but row not expanded	
		bool							expanded								= false;				// the row is either collapsed (plus visible) or expanded (minus visible)
		list<branch*>					subBranches;													// rows which are branches of this row
		treeView2D*						treeView								= nullptr;
		unsigned int					level									= 0;
		void*							pUserExpand								= nullptr;
		void*							pUserCollapse							= nullptr;
		function<void(void*)>			userFuncExpand							= nullptr;
		function<void(void*)>			userFuncCollapse						= nullptr;

										branch									(treeView2D* treeView, unsigned int level, guiElement2D* theElem, unsigned int theHeight);
										~branch									();

		void							expandOrCollapse						(void* pUser);
		void							setVisibilityOfAllSubBranches			(bool visible, bool onlyOfFirstSubLevel);
		void							updatePos								(vector2& initPos, vector2& curPos);
		void							createPlusMinus							();
		inline bool						isRootBranch							();

		// Option A: make treeView2D a guiElement
		// void							renderSprites							(ssf::sharedVars& v);

		// Option B: use two lines-objects per row
		// list<plainTexture2D>			lines;

	public: 
		// adapted functions on treeView
		unsigned int					getNumSubBranches						();
		branch*							insertSubBranch							(unsigned int subBranchIndex, guiElement2D* item, unsigned int height);
		// branch*						insertSubBranch							(branch* afterBranch,		  guiElement2D* item, unsigned int height);
		branch*							insertSubBranchAndItems_plainButton2D	(unsigned int subBranchIndex,					  unsigned int height, buttonImageFiles &imageFiles, font2D* theFont, wstring text = wstring(L""), bool showExpandButtonAlthoughNoSubBranch = false);
		//branch*						insertSubBranchAndItems_plainButton2D	(branch* afterBranch,		  					  unsigned int height, buttonImageFiles &imageFiles, font2D* theFont);
		void							insertSubBranchesAndItems_plainButton2D	(unsigned int subBranchIndex,					  unsigned int height, buttonImageFiles &imageFiles, font2D* theFont, unsigned int numBranches);
		//void							insertSubBranchesAndItems_plainButton2D	(branch* afterBranch,		  					  unsigned int height, buttonImageFiles &imageFiles, font2D* theFont, unsigned int numBranches);
		void							removeSubBranch							(unsigned int subBranchIndex, bool alsoDeleteGuiElem);
		void							removeSubBranchAndItems					(unsigned int subBranchIndex, bool alsoDeleteGuiElem);
		void							removeAllSubBranches					(							  bool alsoDeleteGuiElem);
		void							removeAllSubBranchesAndItems			(							  bool alsoDeleteGuiElem);
		void							setRowHeight							(unsigned int  height);
		unsigned int					getRowHeight							();
		void							setFocusOnRow							();
		void							setRowColor								(color newColor);
		void							setText									(const wchar_t* theText);
		unsigned int					getRowIndex								();
		guiElement2D*					getGuiElemPointer						();
		branch*							getSubBranch							(unsigned int subBranchIndex);
		branch*							getLastSubBranch						();
		item*							getItem									(unsigned int columnIndex);
		void							assignOnExpand							(function<void(void*)>					userFuncExpand,		void* pUser);
		void							assignOnCollapse						(function<void(void*)>					userFuncCollapse,	void* pUser);
		// void							assignOnGotFocus						(function<void(guiElemEvFol*, void*)> userFunc, void* pUser);
		// void							assignOnLostFocus						(function<void(guiElemEvFol*, void*)> userFunc, void* pUser);
	};

	friend class branch;

private:
	buttonImageFiles*					filesPlus								= nullptr;				// textures for buttons and lines
	buttonImageFiles*					filesMinus								= nullptr;				// textures for buttons and lines
	texture*							texLine									= nullptr;				// textures for buttons and lines
	float								xOffsetPerLevel							= 20;					// the items of the first column are moved by this amount in pixels per branch level to the right
	float								plusMinusSize							= 12;					// size of the symbol in pixels
	float								linesThickness							= 3;					// thickness of the plus/minus connecting line in pixels
	branch								rootBranch								= {this, 0, nullptr, 0};// a dummy branch object, which is not part of the rowHeaders-list, but which points to the first branches

	void								updatePlusMinusPos						();						
	void								removeAllRows							(bool alsoDeleteGuiElem);	

public:
	
	// different functions from listView2D
	bool								create									(masterMind* ww, float depthInSpace, buttonImageFiles &filesPlus, buttonImageFiles &filesMinus, texture *texLine = nullptr);
	void								alignAllItems							();

	// new row functions
	branch*								getRootBranch							()	{ return &rootBranch; };

	// row functions are deleted, since class branch shall be used
	bool								create									(masterMind* ww, float depthInSpace = 0)																			= delete;
	listView2D::row*					insertRow								(unsigned int rowIndex, guiElement2D* item, unsigned int height)													= delete;
	listView2D::row*					insertRow								(row& beforeRow,		guiElement2D* item, unsigned int height)													= delete;
	void								insertRowsAndItems_plainButton2D		(unsigned int rowIndex, unsigned int numRows, unsigned int height, buttonImageFiles &imageFiles, font2D* theFont)	= delete;
	void								removeRow								(unsigned int rowIndex,		bool alsoDeleteRow)																		= delete;
	unsigned int						getRowIndex								(unsigned int itemIndex)																							= delete;
	void								setFocusOnRow							(unsigned int rowIndex)																								= delete;
	void								setRowColor								(unsigned int rowIndex, color newColor)																				= delete;
	guiElement2D*						getRowGuiElemPointer					(unsigned int rowIndex)																								= delete;
};

} // namespace wildWeasel

// BUG: If branch heights are different then alignAllItems messes them up
// BUG: slow when scrolling with cursor

