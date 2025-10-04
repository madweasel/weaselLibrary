/*********************************************************************\
	wwTreeView.h												  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/

#pragma once

#include "wildWeasel.h"

// The structure of a list view is in principle like a table with columns and rows:
//			column	column	column
//	row		item	item	item
//	row		item	item	item

namespace wildWeasel
{

// 2D plain list view
class listView2D : public guiElemCluster2D, public alignedRect, public scrollBarDuo, protected eventFollower	// ... should not be a cluster since "items" is not used. even worse, since variable items is named equally here. instead a more basic guiElem-Class would be fine
{
public: 
	enum class							posMode								{ ROW_WISE,  COLUMN_WISE };							// indexing of the items goes either along each column or along each row
	enum class							selectionMode						{ ROW_WISE,  COLUMN_WISE, ITEM_WISE, NONE };		// way the user can select: either a whole row/column or a single item
	
	class row;																													// pre-declaration
	class column;																												// ''
	class item;																													// ''

	class item																													// class for a single item in the list view
	{
	friend class listView2D;

	public:
		unsigned int					getRowIndex							();
		unsigned int					getColumnIndex						();
		unsigned int					getItemIndex						();
		guiElemCluster2D*				getGuiElemPointer					()													{ return subItems; };
		row*							getRow								()													{ return listView->getRow(getRowIndex()); };
		void							setFocusOnItem						();
		void							setText								(unsigned int subItemIndex, const WCHAR* theText);

	private:
		listView2D*						listView							= nullptr;											// list view owning the item
		list<item>::iterator			itr;																					// iterator pointing to 'this' in the item list owned by the list view
		guiElemCluster2D*				subItems							= nullptr;											// a cluster of gui elements for the graphical representation of the list view item
		color							colorWhenUnmarked					= color::white();
	};

	class autoSize
	{
	protected:
		unsigned int					curSize								= 0;												// width in pixels of the column
		unsigned int					minSize								= 0;												// size in pixels beeing the minimum size when auto setting the size
		unsigned int					sizeSetByUser						= 0;												// size in pixels set by the user, which is overruled by the auto width calculation if autoWidthFraction != 0
		float							autoSizeFraction					= 0.0f;												// fraction to which extend the column is automatically broadened/narrowed if the pixel width of the list view changes

										autoSize							()													{};
										autoSize							(unsigned int size, float fraction)				   : curSize{size}, minSize{size}, sizeSetByUser{size}, autoSizeFraction{fraction} {}
	};

	class column : public autoSize																								// class for a single column in the list view
	{
	friend class listView2D;

	public: 
		void							setAutoWidthFraction				(float fraction)									{ this->autoSizeFraction = fraction; }
		void							setWidth							(unsigned int width)								{ this->sizeSetByUser = width; listView->updateSledgeWidth(); }
		void							setMinWidth							(unsigned int width)								{ this->minSize = width; }
		unsigned int					getWidth							()													{ return curSize; }
		guiElement2D*					getGuiElemPointer					()													{ return elem; }
		unsigned int					getColumnIndex						();
		void							setFocusOnColumn					();

	protected:
										column								()													{};
										column								(listView2D* theList, guiElement2D* theElem, unsigned int theWidth, float autoWidthFraction = 0.0f) : listView{theList}, elem{theElem}, autoSize{theWidth, autoWidthFraction} {};

		listView2D*						listView							= nullptr;											// list view owning the column
		list<column>::iterator			itr;																					// iterator pointing to 'this' in the column list owned by the list view
		guiElement2D*					elem								= nullptr;											// a gui element for the graphical presentation of the column header
	};

	class row : public autoSize																									// class for a single row in the list view
	{
	friend class listView2D;

	public: 
		void							setAutoHeightFraction				(float fraction)									{ this->autoSizeFraction = fraction; };
		void							setHeight							(unsigned int height)								{ this->sizeSetByUser = height; listView->updateSledgeWidth(); };
		void							setMinHeight						(unsigned int height)								{ this->minSize = height; }
		unsigned int					getHeight							()													{ return curSize; };
		guiElement2D*					getRowGuiElemPointer				()													{ return elem; };
		unsigned int					getRowIndex							();
		void							setFocusOnRow						();
		void							assignOnGotFocus					(function<void(guiElemEvFol*, void*)> userFunc, void* pUser);
		void							assignOnLostFocus					(function<void(guiElemEvFol*, void*)> userFunc, void* pUser);

	protected:
										row									(listView2D* theList, guiElement2D* theElem, unsigned int theHeight, float autoHeightFraction = 0.0f) : listView{theList}, elem{theElem}, autoSize{theHeight, autoHeightFraction} {};

		listView2D*						listView							= nullptr;											// list view owning the row
		list<row*>::iterator			itr;																					// iterator pointing to 'this' in the row list owned by the list view
		guiElement2D*					elem								= nullptr;											// a gui element for the graphical presentation of the column header
		float							xOffsetInFirstColumn				= 0;												// since the list view is inherited by the tree view it is possible to apply an pixel offset to the items of the first column
		bool							visible								= true;												// since the list view is inherited by the tree view it is posible to hide certain rows
		void*							gotFocusUserPointer					= nullptr;											// pointer passed to 'user' when function gotFocusUserFunc() is called
		function<void(guiElemEvFol*, void*)>																					// a 'user' function called when an item of the row got the focus
										gotFocusUserFunc					= nullptr;											// ''
	};
	
protected:
	// variables
	bool								columnHeaderIsVisible				= false;											// hide/show the column gui elements
	bool								rowHeaderIsVisible					= false;											// hide/show the row gui elements
	unsigned int						columnHeaderHeight					= 0;												// height of the column header in whole (be aware: although each column has a gui element with an own height)
	unsigned int						rowHeaderWidth						= 0;												// ''
	vector2								defaultTextBorder					= vector2{3, 3};									// new items get this boder for the text positioning inside the gui element
	vector2								defaultTextSize						= vector2{1, 1};									// new items get this text size
	posMode								positioningMode						= posMode::COLUMN_WISE;								// indexing of the items goes either along each column or along each row
	list<item>							items;																					// all items ordered according to the chosen positioningMode
	list<column>						columnHeaders;																			// all column objects
	list<row*>							rowHeaders;																				// all row objects
	fRECT								clippingAreaItems;																		// area in which items are shown. outside this rect they are clipped.
	fRECT								clippingAreaColumnHeaders;																// ''
	fRECT								clippingAreaRowHeaders;																	// ''
	fRECT								transScaledTargetRect;																	// since listView2D is not a guiElem2D. transScaledTargetRect must be calculated manually.
	item*								markedItem							= nullptr;											// pointer to the item which is currently marked. only one item can be marked. depending on 'markerSelectionMode' it can represent the whole column/row
	color								markerColor							= color::red();										// color with which the marked item/row/column is highlighted
	selectionMode						markerSelectionMode					= selectionMode::NONE;								// way the user can mark/select: either a whole row/column or a single item
	void*								pUser								= nullptr;											// pointer to user data passed in the function userItemChanged()
	function<void(unsigned int, unsigned int, guiElemEvFol*, void*)>															// when a sub item of a list view item got the focus than this user function is called
										userItemChanged						= nullptr;											// ''
		
	// helper functions
	template<typename TAction> void		processItems						(unsigned int startIndex, unsigned int numItemsToProcess, unsigned int indexStep, TAction action);
	void								encolorMarkedItems					(bool setMarkerColor);
	void								subItemGotFocus						(guiElemEvFol* theGuiElem, void* pUser);
	void								columnScrollBarMoved				(scrollBar2D* bar, void* pUser);
	void								rowScrollBarMoved					(scrollBar2D* bar, void* pUser);
	void								updateSledgeWidth					();
	void								updateMarkedItem					(int numNewRows, int numNewColumns);
	void								releaseSubItem						(guiElemCluster2D* subItem);
	float								pixelsToScroll						(scrollBarDuo::direction dir) override;
	
	// alignment functions
	void								updateTransScaledRect				();
	void								updateClippingRects					();
	void								updateScrollBarPos					();
	void								updateColumnHeaderPos				();
	void								updateRowHeaderPos					();
	void								updateItemPos						();

	// user input functions
	void								windowSizeChanged					(int xSize, int ySize) override;
	void								keyDown								(int keyCode) override;
	void								verticalWheelMoved					(int distance) override;
	void								horizontalWheelMoved				(int distance) override;
											
public:									
											
	// initialization					
	bool								create								(masterMind* ww, float depthInSpace = 0);
	bool								createScrollBars					(buttonImageFiles &filesTriangle, buttonImageFiles &filesLane, buttonImageFiles &filesSledge);
											
	// setter							
	void								setColumnHeaderHeight				(unsigned int newHeight);
	void								setRowHeaderWidth					(unsigned int newWidth);
	void								setVisibilityColumnHeader			(bool isVisible);
	void								setVisibilityRowHeader				(bool isVisible);
	void								setItemTextAlignmentHorizontal		(alignmentHorizontal newAlign);
	void								setPositioningMode					(posMode newMode);
	void								setMarkerColor						(color newMarkerColor);
	void								setSelectionMode					(selectionMode newMode);
	void								setState							(guiElemState newStatus);
	void								setTextSize							(float sx, float sy);
	void								setTextBorder						(float bx, float by);

	// column based functions
	unsigned int						getNumColumns						();
	void								insertColumn						(unsigned int columnIndex,  guiElement2D* item, unsigned int width, float autoWidthFraction = 0.0f);
	void								insertColumn						(column& insertAfterColumn, guiElement2D* item, unsigned int width, float autoWidthFraction = 0.0f);
	void								insertColumn_plainButton2D			(unsigned int columnIndex, wstring const& text, font2D* theFont, unsigned int widthColumn, float angle, float textSize, RECT& rcButton, buttonImageFiles& imageFiles, float autoSizeFactor = 0.0f);
	void								removeColumn						(unsigned int columnIndex,	bool alsoDeleteColumn);
	void								removeColumn						(column& Column,			bool alsoDeleteColumn);
	void								removeAllColumns					(							bool alsoDeleteColumn);
	column*								getColumn							(unsigned int columnIndex);
	void								setFocusOnColumn					(unsigned int columnIndex);
	guiElement2D*						getColumnGuiElemPointer				(unsigned int columnIndex);
	void								setColumnWidth						(unsigned int columnIndex, unsigned int width);
	unsigned int						getColumnWidth						(unsigned int columnIndex);
	unsigned int						getColumnIndex						(unsigned int itemIndex);

	// row based functions
	unsigned int						getNumRows							();
	row*								insertRow							(unsigned int rowIndex, guiElement2D* item, unsigned int height, float autoHeightFraction = 0.0f);
	row*								insertRow							(row* insertAfterRow,	guiElement2D* item, unsigned int height, float autoHeightFraction = 0.0f);
	void								insertRowsAndItems_plainButton2D	(unsigned int rowIndex, unsigned int numRows, unsigned int height, buttonImageFiles &imageFiles, font2D* theFont);
	void								removeRow							(unsigned int rowIndex,		bool alsoDeleteRow);
	void								removeRow							(row& Row,					bool alsoDeleteRow);
	void								removeAllRows						(							bool alsoDeleteRow);
	row*								getRow								(unsigned int rowIndex);
	unsigned int						getFocussedRowIndex					();
	void								setFocusOnRow						(unsigned int rowIndex);
	void								setRowColor							(unsigned int rowIndex, color newColor);
	void								setRowTextColor						(unsigned int rowIndex, color newColor);
	guiElement2D*						getRowGuiElemPointer				(unsigned int rowIndex);
	void								setRowHeight						(unsigned int rowIndex, unsigned int  height);
	unsigned int						getRowHeight						(unsigned int rowIndex);
	unsigned int						getRowIndex							(unsigned int itemIndex);
		
	// item based functions
	unsigned int						getItemCount						();
	void								insertItem							(unsigned int itemIndex, guiElemCluster2D* newItem);
	void								removeAllItemsInRow					(unsigned int rowIndex,		bool alsoDeleteItem);
	void								removeAllItemsInColumn				(unsigned int columnIndex,	bool alsoDeleteItem);
	void								removeItem							(unsigned int itemIndex,	bool alsoDeleteItem);
	void								removeItem							(item&        itemToRemove,	bool alsoDeleteItem);
	void								removeAllItems						(							bool alsoDeleteItem);
	void								deleteAllItems						() = delete;
	item*								getItem								(unsigned int itemIndex);
	item*								getItem								(unsigned int rowIndex, unsigned int columnIndex);
	unsigned int						getItemIndex						(unsigned int rowIndex, unsigned int columnIndex);
	void								setFocusOnItem						(unsigned int itemIndex);
	void								setItemText							(unsigned int rowIndex, unsigned int columnIndex, unsigned int subItemIndex, const WCHAR* theText);
	void								setItemText							(unsigned int itemIndex,						  unsigned int subItemIndex, const WCHAR* theText);
	guiElemCluster2D*					getItemGuiElemPointer				(unsigned int itemIndex);

	// alignment						
	void								setElemVisibility					(guiElement&		elem,	 vector2& pos);					// TODO: should be implemented naturally when using guiElem2D::setPosition()
	void								setClusterVisibility				(guiElemCluster2D&	cluster, vector2& pos);					// ''
	virtual void						alignAllItems						();															// TODO: should be called automatically when dirty, and not be the user

	// TODO: not implemented yet (the problem is that sprites cannot be clipped diagonally)
	void								setRotation							(float z, bool updateMatrix) = delete;
		
	// events
	void								assignOnItemChanged					(function<void(unsigned int, unsigned int, guiElemEvFol*, void*)> userFunc, void* pUser);
};

} // namespace wildWeasel

