/*********************************************************************\
	wwExamples.h
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/
#pragma once

#include "wildWeasel.h"
#include "wwListView.h"
#include "wwTreeView.h"
#include "wwEditField.h"
#include "wwRotCtrlCube.h"

namespace wildWeasel
{
	// a bunch of functions showing the functioning of gui elements
	class examples
	{
	public:
		static void							createAlignment						(masterMind* ww, font2D* theFont, texture &texLine, texture &texCorner, buttonImageFiles &filesVoid);
		static void							createListView2D					(masterMind* ww, font2D* theFont, buttonImageFiles &filesTriangle, buttonImageFiles &filesVoid, texture &texLine, texture &texCorner);
		static void							createTreeView2D					(masterMind* ww, font2D* theFont, buttonImageFiles &filesTriangle, buttonImageFiles &filesVoid, texture &texLine, texture &texCorner, buttonImageFiles &filesPlus, buttonImageFiles &filesMinus);
		static void							createScrollBar2D					(masterMind* ww,				  buttonImageFiles &filesTriangle, buttonImageFiles &filesVoid);
		static void							createEditField2D					(masterMind* ww, font2D* theFont, buttonImageFiles &filesTriangle, buttonImageFiles &filesVoid);
		static void							createPlainButton					(masterMind* ww, wildWeasel::texture* textureRect);
		static void							createrotationControlCube			(masterMind* ww, wildWeasel::texture* textureRect, font3D* theFont);
		static void							createPlainButton2D					(masterMind* ww, font2D* theFont,								   buttonImageFiles &filesVoid, texture &texLine, texture &texCorner);
		static void							createPlainButton2D_timerFunc		(void* pUser);
		static void							createCheckBox2D					(masterMind* ww, font2D* theFont, buttonImageFiles &filesChecked,  buttonImageFiles &filesUnchecked);
		static void							createBorderLine2D					(masterMind* ww, font2D* theFont, texture &texLine, texture &texCorner);
		static void							createDrowDown2D					(masterMind* ww, font2D* theFont, buttonImageFiles &filesButton, texture &texArrow, buttonImageFiles &filesTriangle, buttonImageFiles &filesVoid);
		static void							showOpenFileDialog					(masterMind* ww);
	};

} // namespace wildWeasel

