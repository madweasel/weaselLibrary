/*********************************************************************
	winInspectDb.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/
#ifndef MINIMAXWIN_INSPECT_DB_H
#define MINIMAXWIN_INSPECT_DB_H

// Windows Header Files:
#include "wildWeasel/src/wildWeasel.h"
#include "wildWeasel/src/wwTreeView.h"
#include "wildWeasel/src/wwListView.h"
#include "wildWeasel/src/wwEditField.h"
#include "../miniMax.h"
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace miniMax
{

class miniMaxGuiField
{
public:
	virtual void								setAlignment						(wildWeasel::alignment& newAlignment) {};
	virtual void								setVisibility						(bool visible) {};
	virtual void								setState							(unsigned int curShowedLayer, stateNumberVarType curShowedState, unsigned char curShownSymOp, unsigned int curPlayer) {};
};

/*------------------------------------------------------------------------------------

|	-------------------------------------		---------------------------------	|
|	|									|		|								|	|
|	|									|		|								|	|
|	|		treeViewInspect				|		|		miniMaxGuiField			|	|
|	|									|		|								|	|
|	|									|		|								|	|
|	|									|		|								|	|
|	|									|		|								|	|
|	|									|		|								|	|
|	|									|		|								|	|
|	|									|		|								|	|
|	|									|		|								|	|
|	|									|		|								|	|
|	|									|		|								|	|
|	|									|		|								|	|
|	|									|		|								|	|
|	|									|		|								|	|
|	|									|		|								|	|
|	|									|		|								|	|
|	-------------------------------------		---------------------------------	|
|																					|
-----------------------------------------------------------------------------------*/

/**
 * @brief The miniMaxWinInspectDb class provides a GUI interface for inspecting the miniMax database.
 *
 * This class manages the controls and visualization for inspecting the internal state and layers of the miniMax AI database.
 * It integrates with the wildWeasel GUI framework to display tree views and related controls, allowing users to explore
 * states, layers, and symmetry operations. The class is responsible for creating, showing, and resizing the inspection controls,
 * and interacts with miniMax and miniMaxGuiField for data access and display.
 */
class miniMaxWinInspectDb
{
protected:
	static const unsigned int 					PLAYER_ONE							= 1;							// player one
	static const unsigned int 					PLAYER_TWO							= 0;							// player two
	static const unsigned char					SO_DO_NOTHING						= 3;							// maximum number of children in each layer

	struct treeViewItemInfo
	{
		enum class								classType							{ base, layer, uncompleteState, state, symmetricStates, precedingStates, succedingStates };
		static const unsigned char              NUM_SYMMETRIC_OPERATIONS			= 16;							// number of symmetric operations
		static const unsigned int				rowHeightInPixels					= 30;
		static const unsigned int				colWidthInPixels					= 400;
		static constexpr float					textSize							= 0.5f;

		classType								type								= classType::base;
		unsigned int							layerNumber							= 0;
		unsigned int							numberOfKnotsInLayer				= 0;
		miniMax *								pMiniMax							= nullptr;
		miniMaxGuiField*						pGuiField							= nullptr;
		wildWeasel::treeView2D::branch*			pBranch								= nullptr;
		wildWeasel::treeView2D*					treeViewInspect						= nullptr;
		treeViewItemInfo*						parent								= nullptr;
		vector<treeViewItemInfo*>				children;
		unsigned int *							pCurShowedLayer						= nullptr;						// current showed layer
		stateNumberVarType*						pCurShowedState						= nullptr;						// current showed state
		unsigned char *							pCurShownSymOp						= nullptr;						// current shown symmetry operation
		unsigned int *							pCurPlayer							= nullptr;						// current player
		wildWeasel::buttonImageFiles*			pListImages							= nullptr;	
		wildWeasel::font2D*						pFont								= nullptr;

		virtual bool							deselect							() {return true;};
		virtual bool							select								() {return true;};
		virtual bool							expand								() {return true;};

		bool									collapse							();
		void									writeKnotValueIntoString			(wstringstream &wssTmp, unsigned int stateNumber, unsigned char symOp, unsigned int curPlayer);		// may use use setSituation()
												
												treeViewItemInfo					() {};
												treeViewItemInfo					(treeViewItemInfo *parent);
												~treeViewItemInfo					();
	};

	struct treeViewItemLayer : treeViewItemInfo
	{
												treeViewItemLayer					(wildWeasel::treeView2D* treeViewInspect, wildWeasel::treeView2D::branch* branch, wildWeasel::buttonImageFiles* pListImages, wildWeasel::font2D* pFont, unsigned int layerNumber, miniMax* pMiniMax, miniMaxGuiField* guiField, unsigned int* pCurShowedLayer, stateNumberVarType* pCurShowedState, unsigned char* pCurShownSymOp, unsigned int* curPlayer);
		bool									expand								();
	};

	struct treeViewItemUncomplState : treeViewItemInfo
	{
		unsigned int							stateNumberPrefix					= 0;							// state number is for example 12000 when missingDigits is equal 3
		unsigned int							numMissingDigits					= 0;							// number of digits missing, since only 10 tree view items are expanded in each level
		unsigned long long						stepSize							= 0;							// 

												treeViewItemUncomplState			(miniMaxWinInspectDb::treeViewItemInfo* parent, unsigned int stateNumberPrefix, unsigned int curStateDigit, unsigned long long newStepSize, unsigned int newNumMissingDigits);
		bool									expand								();
		static bool								expand_static						(treeViewItemInfo& tvii, unsigned int& stateNumberPrefix, unsigned int& numMissingDigits, unsigned long long& stepSize);
	};

	struct treeViewItemValues
	{
		unsigned int							stateNumber							= 0;
		unsigned char 							symOp								= 0;
		unsigned int 							curPlayer							= 0;
	};

	struct treeViewItemState : treeViewItemValues, treeViewItemInfo
	{
		bool									expand								();
		bool									select								();
		bool									deselect							();
												treeViewItemState					(treeViewItemInfo *parent, unsigned int layerNumber, unsigned int stateNumber, unsigned char symOp, unsigned int curPlayer);
												treeViewItemState					(treeViewItemInfo *parent,							 unsigned int stateNumber, unsigned char symOp, unsigned int curPlayer);
	};

	struct treeViewItemSym : treeViewItemValues, treeViewItemInfo
	{
		bool									expand								();
												treeViewItemSym						(treeViewItemInfo *parent, unsigned int stateNumber, unsigned char symOp, unsigned int curPlayer);
	};

	struct treeViewItemSuc : treeViewItemValues, treeViewItemInfo
	{
		bool									expand								();
												treeViewItemSuc						(treeViewItemInfo *parent, unsigned int stateNumber, unsigned char symOp, unsigned int curPlayer);
	};

	struct treeViewItemPrec : treeViewItemValues, treeViewItemInfo
	{
		bool									expand								();
												treeViewItemPrec					(treeViewItemInfo *parent, unsigned int stateNumber, unsigned char symOp, unsigned int curPlayer);
	};

	// General Variables
	wildWeasel::masterMind *					ww									= nullptr;					// handle of calling window
	miniMax *									pMiniMax							= nullptr;					// pointer to perfect AI class granting the access to the database
	miniMaxGuiField*							pGuiField							= nullptr;
	bool										showingInspectionControls			= false;
	unsigned int								curShowedLayer						= 0;						// current showed layer
	stateNumberVarType							curShowedState						= 0;						// current showed state
	unsigned char								curShownSymOp						= 0;						// current shown symmetry operation
	unsigned int								curPlayer							= 0;						// current player
	vector<treeViewItemLayer*>					treeViewItems;
	wildWeasel::treeView2D						treeViewInspect;
	wildWeasel::alignment*						amAreaInspectDb						= nullptr;
	wildWeasel::alignment						amListInspectDb						= { wildWeasel::alignmentTypeX::FRACTION, 0.05f, wildWeasel::alignmentTypeY::FRACTION, 0.05f, wildWeasel::alignmentTypeX::FRACTION, 0.40f, wildWeasel::alignmentTypeY::FRACTION, 0.90f };
	wildWeasel::alignment						amFieldInspectDb					= { wildWeasel::alignmentTypeX::FRACTION, 0.55f, wildWeasel::alignmentTypeY::FRACTION, 0.05f, wildWeasel::alignmentTypeX::FRACTION, 0.40f, wildWeasel::alignmentTypeY::FRACTION, 0.90f };
	wildWeasel::buttonImageFiles				buttonImagesArrow					= { L"button_Arrow__normal.png",     1, 0, L"button_Arrow__mouseOver.png",    10, 100, L"button_Arrow__mouseLeave.png",    10, 100, L"button_Arrow__pressed.png",    10, 100, L"button_Arrow__grayedOut.png",    1, 0};
	wildWeasel::buttonImageFiles				buttonImagesPlus					= { L"button_Plus___normal.png",     1, 0, L"button_Plus___mouseOver.png",    10, 100, L"button_Plus___mouseLeave.png",    10, 100, L"button_Plus___pressed.png",    10, 100, L"button_Plus___grayedOut.png",    1, 0};
	wildWeasel::buttonImageFiles				buttonImagesMinus					= { L"button_Minus__normal.png",     1, 0, L"button_Minus__mouseOver.png",    10, 100, L"button_Minus__mouseLeave.png",    10, 100, L"button_Minus__pressed.png",    10, 100, L"button_Minus__grayedOut.png",    1, 0};
	wildWeasel::buttonImageFiles				buttonImagesVoid					= { L"button_Void___normal.png",     1, 0, L"button_Void___mouseOver.png",    10, 100, L"button_Void___mouseLeave.png",    10, 100, L"button_Void___pressed.png",    10, 100, L"button_Void___grayedOut.png",    1, 0};
	wildWeasel::buttonImageFiles				buttonImagesListItem				= { L"button_List___normal.png",     1, 0, L"button_List___mouseOver.png",    10, 100, L"button_List___mouseLeave.png",    10, 100, L"button_List___pressed.png",    10, 100, L"button_List___grayedOut.png",    1, 0};
	wildWeasel::texture*						textureLine							= nullptr;
	wildWeasel::font2D*							hFontOutputBox						= nullptr;
	const unsigned int							scrollBarWidth						= 20;

public:
	
	// Constructor / destructor
												miniMaxWinInspectDb					(wildWeasel::masterMind* ww, miniMax* pMiniMax, wildWeasel::alignment& amInspectDb, wildWeasel::font2D* font, wildWeasel::texture* textureLine, miniMaxGuiField& guiField);
												~miniMaxWinInspectDb				();

	// Generals Functions
	bool										createControls						();
	bool										showControls						(bool visible);
	void										resize								(wildWeasel::alignment &rcNewArea);
};

} // namespace miniMax

#endif