/*********************************************************************\
	wwEditField.h												  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/

#pragma once

#include "wildWeasel.h"

namespace wildWeasel
{

// 2D plain edit field
class editField2D : public guiElemEvFol2D, public scrollBarDuo
{
public:
	enum class							wordWrapMode						{ OFF, CHARACTER, WORD };
	enum class							textCursorMoveType					{ MOVE_RELATIVE, MOVE_ABSOLUTE };

private:

	// pre-definition
	class textSegmentChain;

	// structures
	class textSegment
	{
	public: 
		unsigned int					numChars							= 0;

		virtual void					changeWidth							(unsigned int newWidth)	{};
		virtual void					advanceOneSegment					(void* pUser)			{};
		virtual void					reverseOneSegment					(void* pUser)			{};
	};

	class pointerToTextSegment
	{
	private:
		textSegmentChain*				tsc									= nullptr;

	public:
		unsigned int					posWithInSegment					= 0;
		list<textSegment*>::iterator	itr;

		// pointer functions
		void							init								(textSegmentChain* tsc);
		void							moveRelative						(int numTextSegments, int newPosWithInSegment, void* pUser);
		void							moveRelative						(int numChars, void* pUser);
		void							moveAbsolute						(unsigned int newCharPos, void* pUser);

		// textSegement functions
		void							splitTextSegment					(unsigned int splitPosWithInSegment, textSegment* newTextSegment);
		void							mergeWithNextSegment				();
		void							changeTextSegmentWidth				(int changeWidth, unsigned int atPosWithInSegment);
		void							moveBorderLine						(int numChars);

		// list functions
		list<textSegment*>::iterator	begin()								{ return tsc->chain.begin(); };
		list<textSegment*>::iterator	end()								{ return tsc->chain.end();	 };
	};

	class textSegmentChain 
	{
	public:
		list<textSegment*>				chain;
		list<pointerToTextSegment*>		pointers;

		void							clear								();
	};

	class textFormatStruct : public textSegment
	{
	public:
		color							color								= color::black;
		vector2							size								= {1,1};
		bool							italic								= false;
		bool							bold								= false;
		bool							underlined							= false;
	};

	class textLineStruct : public textSegment
	{
	public:
		vector2							sizeInPixels						= vector2::Zero;

		struct segmentChangeVars
		{
			float						pixelsMovedY						= 0;
			int							charsMoved							= 0;
		};

		void							advanceOneSegment					(void* pUser)			override;
		void							reverseOneSegment					(void* pUser)			override;
	};

	class textMarkerStruct : public textSegment
	{
	public:
		color							color								= color::blue;
		bool							active								= false;
	};

	class textCursorStruct
	{
	public:
		timer							blinkTimer;
		unsigned int					blinkPeriod							= 500;
		float							widthInPixels						= 2;
		color							color								= color::black;
		bool							isVisible							= false;
		pointerToTextSegment			pFormat;
		pointerToTextSegment			pLine;
		pointerToTextSegment			pText;
		pointerToTextSegment			pMarker;
	};

	class visibleAreaStruct
	{
	public:
		float							firstPixelPosY						= 0;
		pointerToTextSegment			pFormat;
		pointerToTextSegment			pLine;
		pointerToTextSegment			pText;
		pointerToTextSegment			pMarker;

		unsigned int					calcNumVisibleLines					(float visibleHeight);
	};

	// variables
	textSegmentChain					formats;
	textSegmentChain					lines;
	textSegmentChain					markers;
	textSegmentChain					texts;
	textCursorStruct					textCursor;
	visibleAreaStruct					visibleArea;
	wordWrapMode						wrapMode							= wordWrapMode::OFF;
	float								borderWidth							= 3;
	bool								autoScrollOn						= true;
	ssf::sprite							sprite;

	function<void(guiElement*, void*)>	userValueEntered					= nullptr;
	void*								pUser								= nullptr;

	// functions
	static void							blinkTimerFunc						(void* pUser);
	float								getCharWidthInPixel					(WCHAR c);
	float								getPixelPosWithInLine				(pointerToTextSegment &p, unsigned int posCharInText);
	void								performWordWrap						();
	void								reset								();
	void								setTextCursorPosFinish				(bool jumpToNextAfterNewLine);
	float								getCurrentLineDistance				();
	void								scrollIfTextCursorOutside			();

	// event follower functions
	void								eventGotFocus						(guiElemEvFol* elem, void* pUser);
	void								eventLostFocus						(guiElemEvFol* elem, void* pUser);
	void								eventMouseEnteredRegion				(guiElemEvFol* elem, void* pUser);
	void								eventMouseLeftRegion				(guiElemEvFol* elem, void* pUser);
	void								keyDown								(int keyCode)  override;
	void								verticalWheelMoved					(int distance) override;
	void								horizontalWheelMoved				(int distance) override;
											
	// scroll bars						
	void								updateSledgeWidth					() override;
	void								columnScrollBarMoved				(scrollBar2D* bar, void* pUser) override;
	void								rowScrollBarMoved					(scrollBar2D* bar, void* pUser) override;
		
	// render functions
	void								renderSprites						(ssf::sharedVars& v);
	void								windowSizeChanged					(int xSize, int ySize);
											
public:									
	bool								create								(masterMind* ww, font2D* theFont, float depthInSpace);
	bool								createScrollBars					(buttonImageFiles &filesTriangle, buttonImageFiles &filesLane, buttonImageFiles &filesSledge);
	unsigned int						getTextCursorPos					();
	void								getTextCursorPos					(unsigned int& line, unsigned int& posWithInLine);
	void								setTextCursorPos					(textCursorMoveType moveType, int newPos, bool jumpToNextAfterNewLine);
	void								setTextCursorPos					(textCursorMoveType noveType, int newLine, int newPosWithInLine, bool jumpToNextAfterNewLine);
	void								insertChar							(WCHAR newChar);
	void								insertString						(const WCHAR* newString);
	void								insertString						(stringbuf& streamBuffer);
	void								removeChars							(unsigned int numChars);
	void								setWordWrapMode						(wordWrapMode newWrapMode);
	void								setText								(const WCHAR* newText);
	void								setText								(const wstring& newText);
	void								setBorderWidth						(float newWidth);
	void								setTextSize							(float sx, float sy);
	void								alignAllItems						();
	void								setState							(guiElemState newStatus) override;
	void								setAutoScroll						(bool autoScrollOn);
	void								assignOnValueEntered				(function<void(guiElement*, void*)> userFunc, void* pUser);
};

} // namespace wildWeasel

