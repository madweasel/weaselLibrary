/*********************************************************************
	wwExamples.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/

#include "wwExamples.h"

/*************************************************************************************************************************************/

//-----------------------------------------------------------------------------
// Name: createAlignment()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::examples::createAlignment(masterMind* ww, font2D* theFont, texture &texLine, texture &texCorner, buttonImageFiles &filesVoid)
{
	// alignment
	wildWeasel::alignment*					myAlign1	= new wildWeasel::alignment{};
	wildWeasel::alignment*					myAlign2	= new wildWeasel::alignment{wildWeasel::alignmentTypeX::BORDER_LEFT, 50.0f, wildWeasel::alignmentTypeY::FRACTION, 0.05f, wildWeasel::alignmentTypeX::PIXEL_WIDTH, 100, wildWeasel::alignmentTypeY::FRACTION, 0.4f};
	wildWeasel::plainButton2D*				myButton2D	= new wildWeasel::plainButton2D();
	wildWeasel::borderLine2D*				myBorder	= new wildWeasel::borderLine2D();

	myAlign1->create(ww->alignmentRootFrame);
	myAlign2->create(ww->alignmentRootFrame);

	myAlign1->setSize    (wildWeasel::alignmentTypeX::PIXEL_WIDTH, 200,												wildWeasel::alignmentTypeY::FRACTION, 0.4f);
	myAlign1->setPosition(wildWeasel::alignmentTypeX::BORDER_LEFT, 350, wildWeasel::alignmentHorizontal::RIGHT, wildWeasel::alignmentTypeY::FRACTION, 0.5f, wildWeasel::alignmentVertical::CENTER);

	myAlign2->setInsideAnotherRect(*myAlign1);
	
	// Plan A: call a certain function
	myAlign2->left.setRelationBasedOnDistance(ww->alignmentRootFrame.left, 20);
	
	// Plan B: pass reference to edgeRelation, which is copied
	wildWeasel::alignment::relDistance		myRelation {ww->alignmentRootFrame.left, 120};
	myAlign2->right.setRelation(myRelation);
	myAlign2->right.getRelation<wildWeasel::alignment::relDistance>()->setDistance(110);

	myButton2D->create(ww, filesVoid, nullptr, nullptr, 0.2f);
	myButton2D->setTextSize(1.2f, 1.2f);
	myButton2D->setText(L"Text");
	myButton2D->setFont(theFont);
	myButton2D->setTextState(wildWeasel::guiElemState::DRAWED);
	myButton2D->setState(wildWeasel::guiElemState::DRAWED);
	myButton2D->setTextColor(wildWeasel::color(255,100,0));
	myButton2D->setColor(wildWeasel::color::white());
	myButton2D->setAlignment(*myAlign2);
	
	myBorder->create(ww, texLine, texCorner, 0);
	myBorder->setFont(theFont);
	myBorder->setState(guiElemState::DRAWED);
	myBorder->setText(L"");
	myBorder->setTextWidth(0);
	myBorder->setGapWidthBetweenTextAndLine(0);
	myBorder->setAlignment(*myAlign1);
}

//-----------------------------------------------------------------------------
// Name: createTreeView2D()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::examples::createTreeView2D(masterMind* ww, font2D* theFont, buttonImageFiles &filesTriangle, buttonImageFiles &filesVoid, texture &texLine, texture &texCorner, buttonImageFiles &filesPlus, buttonImageFiles &filesMinus)
{
	// locals
	treeView2D::branch*	myRow		= nullptr;
	treeView2D*			myTreeView	= new wildWeasel::treeView2D();
	RECT				rc			= {50, 80, 300, 520};
	wstringstream		wss;

	myTreeView->create(ww, 0.0f, filesPlus, filesMinus, &texLine);
	myTreeView->setTargetRect(rc);
	
	for (unsigned int i=0; i<6; i++) {
		for (unsigned int j=0; j<4; j++) {
			wildWeasel::guiElemCluster2D*	myCluster	= new wildWeasel::guiElemCluster2D();
			wildWeasel::plainButton2D*		myButton2D	= new wildWeasel::plainButton2D();
			RECT								rc			= {0, 0, 100, 40};
	
			myButton2D->create(ww, filesVoid, nullptr, nullptr, 0);
			myButton2D->setTargetRect(rc);
			myButton2D->setTextSize(1.2f, 1.2f);
			myButton2D->setText(L"Text");
			myButton2D->setFont(theFont);
			myButton2D->setTextState(wildWeasel::guiElemState::DRAWED);
			myButton2D->setTextColor(wildWeasel::color(255,100,0));
			myButton2D->setColor(wildWeasel::color::white());
			myButton2D->setPositioningMode(wildWeasel::matrixControl2D::matControlMode::posRotSca);
			myButton2D->setTextAlignmentHorizontal(wildWeasel::alignmentHorizontal::RIGHT);
			myButton2D->setTextAlignmentVertical(wildWeasel::alignmentVertical::CENTER);
			myButton2D->setPosition (0, 0, true);
			myButton2D->setRotation (0, true);
			myButton2D->setScale	(1, 1, true);
	
			myCluster->create(ww);
			myCluster->addItem(myButton2D);
			myCluster->setState			(wildWeasel::guiElemState::DRAWED);
			myCluster->setTextStates	(wildWeasel::guiElemState::DRAWED);
			//myCluster->setPosition	(50, 100, true);
			//myCluster->setRotation	(0.1f, true);
			//myCluster->setScale		(1.2f, 0.8f, true);

			// main item
			if (j==0) {
				wss.str(L""); wss << i;
			// sub item
			} else {
				wss.str(L""); wss << i << L"." << j;
			}
			myTreeView->insertItem (i*4+j, myCluster);
			myTreeView->setItemText(i*4+j, 0, wss.str().c_str());

			// main row
			if (j==0) {
				myRow =	myTreeView->getRootBranch();
				myRow =	myRow->insertSubBranch(i, nullptr, 40);
			// sub row
			} else {
						myRow->insertSubBranch(j, nullptr, 40);
			}
		}
	}
	myTreeView->setSelectionMode(wildWeasel::listView2D::selectionMode::ROW_WISE);
	myTreeView->setMarkerColor(wildWeasel::color::blue());
	myTreeView->insertColumn(0, nullptr, 100);
	myTreeView->insertColumn(1, nullptr, 100);
	myTreeView->setColumnHeaderHeight(20);
	myTreeView->setVisibilityColumnHeader(false);
	myTreeView->createScrollBars(filesTriangle, filesVoid, filesVoid);
	myTreeView->setColumnScrollBarHeight(30);
	myTreeView->setRowScrollBarWidth	(30);
	myTreeView->setVisibilityColumnScrollBar(true);
	myTreeView->setVisibilityRowScrollBar   (true);
//	myTreeView->removeItem(*myTreeView->getRootBranch()->getSubBranch(4)->getSubBranch(2)->getItem(0), true);
//	myTreeView->getRootBranch()->getSubBranch(4)->removeSubBranch(2, true);
	myTreeView->getRootBranch()->removeSubBranchAndItems(3, true);
	myTreeView->setPosition	(0, 0, true);
	//myTreeView->setRotation	(0.0f, true);
	myTreeView->setScale	(1, 1, true);
	myTreeView->setFocusOnItem(3);
	myTreeView->setState(wildWeasel::guiElemState::DRAWED);
	myTreeView->alignAllItems();
}

//-----------------------------------------------------------------------------
// Name: createListView2D()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::examples::createListView2D(masterMind* ww, font2D* theFont, buttonImageFiles &filesTriangle, buttonImageFiles &filesVoid, texture &texLine, texture &texCorner)
{
	/* a single button in the list
	listView2D*		myListView	= new listView2D();
	RECT			rc			= {50, 100, 250, 200};

	myListView->create(ww, 0.0f);
	myListView->setTargetRect(rc);

	wildWeasel::guiElemCluster2D*	myCluster	= new wildWeasel::guiElemCluster2D();
	wildWeasel::plainButton2D*		myButton2D	= new wildWeasel::plainButton2D();
	RECT								rc2			= {0, 0, 100, 40};
	
	myButton2D->create(ww, filesVoid, nullptr, nullptr, 0);
	myButton2D->setTargetRect(rc2);
	myButton2D->setTextSize(1.2f, 1.2f);
	myButton2D->setText(L"Intel");
	myButton2D->setFont(theFont);
	myButton2D->setTextState(wildWeasel::guiElemState::DRAWED);
	myButton2D->setTextColor(wildWeasel::color(255,100,0));
	myButton2D->setColor(wildWeasel::color::white());
	myButton2D->setPositioningMode(wildWeasel::matrixControl2D::matControlMode::posRotSca);
	myButton2D->setTextAlignmentHorizontal(wildWeasel::alignmentHorizontal::RIGHT);
	myButton2D->setTextAlignmentVertical(wildWeasel::alignmentVertical::CENTER);
	myButton2D->setPosition (0, 0, true);
	//myButton2D->setScale	(0.5f, 0.8f, true);
	
	myCluster->create(ww);
	myCluster->addItem(myButton2D);
	myCluster->setState			(wildWeasel::guiElemState::DRAWED);
	myCluster->setTextStates	(wildWeasel::guiElemState::DRAWED);

	myListView->insertItem(0, myCluster);
	myListView->insertRow(0, nullptr, 40);

	myListView->setSelectionMode(wildWeasel::listView2D::selectionMode::ROW_WISE);
	myListView->setMarkerColor(wildWeasel::color::blue());
	myListView->insertColumn(0, nullptr, 300);
	myListView->setState(wildWeasel::guiElemState::DRAWED);
	myListView->createScrollBars(filesTriangle, filesVoid, filesVoid);
	myListView->setColumnScrollBarHeight(30);
	myListView->setVisibilityColumnScrollBar(true);
	//myListView->setPosition	(50, 100, true);
	//myListView->setScale	(1.2f, 0.8f, true);
	myListView->setFocusOnItem(0);
	myListView->alignAllItems();*/

	// locals
	listView2D*		myListView	= new listView2D();
	RECT			rc			= {50, 80, 200, 420};
	wstringstream	wss;

	myListView->create(ww, 0.0f);
	myListView->setTargetRect(rc);
	//myListView.setAlignment(alignmentSetStateButtons.calcAlignment(0));

	for (unsigned int i=0; i<6; i++) {
		wildWeasel::guiElemCluster2D*	myCluster	= new wildWeasel::guiElemCluster2D();
		wildWeasel::plainButton2D*		myButton2D	= new wildWeasel::plainButton2D();
		wildWeasel::textLabel2D*		myText2D	= new wildWeasel::textLabel2D();
		RECT								rc			= {0, 0, 100, 40};
	
		myButton2D->create(ww, filesVoid, nullptr, nullptr, 0);
		myButton2D->setTargetRect(rc);
		myButton2D->setTextSize(1.2f, 1.2f);
		myButton2D->setText(L"Text");
		myButton2D->setFont(theFont);
		myButton2D->setTextState(wildWeasel::guiElemState::DRAWED);
		myButton2D->setTextColor(wildWeasel::color(255,100,0));
		myButton2D->setColor(wildWeasel::color::white());
		myButton2D->setPositioningMode(wildWeasel::matrixControl2D::matControlMode::posRotSca);
		myButton2D->setTextAlignmentHorizontal(wildWeasel::alignmentHorizontal::RIGHT);
		myButton2D->setTextAlignmentVertical(wildWeasel::alignmentVertical::CENTER);
		myButton2D->setPosition (10, 20, true);
		//myButton2D->setRotation (0.1, true);
		myButton2D->setScale	(0.5f, 0.8f, true);
	
		myText2D->create(ww, wstring(L"abc"), theFont, 1);
		myText2D->setTextColor(wildWeasel::color(0,100,200));
		myText2D->setTextState(wildWeasel::guiElemState::DRAWED);
		myText2D->setTargetRect(rc);
		myText2D->setRotation (0, true);
		myText2D->setTextAlignmentHorizontal(wildWeasel::alignmentHorizontal::LEFT);
	
		myCluster->create(ww);
	//	myCluster->addItem(myText2D);
		myCluster->addItem(myButton2D);
		myCluster->setState			(wildWeasel::guiElemState::DRAWED);
		myCluster->setTextStates	(wildWeasel::guiElemState::DRAWED);

		//myCluster->setPosition	(50, 100, true);
		//myCluster->setRotation	(0.1f, true);
		//myCluster->setScale		(1.2f, 0.8f, true);

		wss.str(L""); wss << i;
		if (i==1) {
			myButton2D->setText(L"afoj2");
		}
		myListView->insertItem(i, myCluster);
		myListView->setItemText(i, 0, wss.str().c_str());
		myListView->insertRow(i, nullptr, 40);
	}
	myListView->setSelectionMode(wildWeasel::listView2D::selectionMode::ROW_WISE);
	myListView->setMarkerColor(wildWeasel::color::blue());
//	myListView->insertColumn(0, nullptr, 100);
//	myListView->insertColumn(1, nullptr, 100);
	RECT rcCol = {-20, 60, 80, 80};
	myListView->insertColumn_plainButton2D(0, wstring(L"column 0"), theFont, 100, -0.5f, 0.5f, rcCol, filesVoid);
	rcCol = {0, 0, 100, 20};
	myListView->insertColumn_plainButton2D(1, wstring(L"column 1"), theFont, 100, +0.5f, 0.5f, rcCol, filesVoid);
	myListView->setColumnHeaderHeight(80);
	myListView->setVisibilityColumnHeader(true);
	myListView->setState(wildWeasel::guiElemState::DRAWED);
	myListView->createScrollBars(filesTriangle, filesVoid, filesVoid);
	myListView->setColumnScrollBarHeight(30);
	myListView->setRowScrollBarWidth(20);
	myListView->setVisibilityColumnScrollBar(true);
	myListView->setVisibilityRowScrollBar(true);
	myListView->removeRow(3, true);
	//myListView->setPosition	(50, 100, true);
	myListView->setPosition	(0, 0, true);
	//myListView->setRotation	(0.1f, true);
	//myListView->setScale		(1.2f, 0.8f, true);
	myListView->setFocusOnItem(3);
	myListView->alignAllItems();

	// clipping border of column headers
	wildWeasel::borderLine2D*		myBorder	= new wildWeasel::borderLine2D();
	fRECT*								rcClip		= new fRECT{50, 80, 200, 160};
	myBorder->create(ww, texLine, texCorner, 0);
	myBorder->setFont(theFont);
	myBorder->setState(guiElemState::DRAWED);
	myBorder->setText(L"");
	myBorder->setTargetRect(rcClip->getRECT());
	myBorder->setPosition(0, 0, false);
	myBorder->setRotation(0, true);
	myBorder->setTextWidth(0);
	myBorder->setGapWidthBetweenTextAndLine(0);
}

//-----------------------------------------------------------------------------
// Name: createScrollBar2D()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::examples::createScrollBar2D(masterMind* ww, buttonImageFiles &filesTriangle, buttonImageFiles &filesVoid)
{
	// locals
	scrollBar2D*	myScrollBar	= new scrollBar2D();
	
	myScrollBar->create(ww, 0, filesTriangle, filesVoid, filesVoid);
	myScrollBar->setSledgeWidth(0.3f);
	myScrollBar->setSledgePos(0.4f);
	myScrollBar->setState(wildWeasel::guiElemState::DRAWED);
	myScrollBar->setPosition(300, 300, false);
	myScrollBar->setScale(300, 40, true);
	myScrollBar->setRotation(0.3f, true);
	myScrollBar->alignAllItems();
	myScrollBar->setSledgeWidth(0.0003f);		// since the scale of the scrollbar is now given, the minimum sledge size is limited to 2 mm screen size
	//myScrollBar->assignOnSledgePosChange(...);
}

//-----------------------------------------------------------------------------
// Name: createEditField2D()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::examples::createEditField2D(masterMind* ww, font2D* theFont, buttonImageFiles &filesTriangle, buttonImageFiles &filesVoid)
{
	// locals
	editField2D*	myEditField = new wildWeasel::editField2D();
	RECT			rcEdit		= {450, 20, 650, 420};

	myEditField->create(ww, theFont, 0);
	myEditField->setTargetRect(rcEdit);
	myEditField->setText(L"Edit Field\nLine 2\nLine 3 with some words\n");
	myEditField->setState(wildWeasel::guiElemState::DRAWED);
	myEditField->setPosition	(50, 100, true);
	myEditField->setRotation	(0.2f, true);
	myEditField->setScale		(1.2f, 0.8f, true);
	myEditField->createScrollBars(filesTriangle, filesVoid, filesVoid);
	myEditField->setVisibilityColumnScrollBar(true);
	myEditField->setVisibilityRowScrollBar(true);
	myEditField->setColumnScrollBarHeight(30);
	myEditField->setRowScrollBarWidth(20);
	myEditField->setBorderWidth(5);
	myEditField->alignAllItems();
}

//-----------------------------------------------------------------------------
// Name: createPlainButton()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::examples::createPlainButton(masterMind* ww, wildWeasel::texture* textureRect)
{
	plainButton*		myButton	= new plainButton();

	myButton->create(ww, nullptr, nullptr, false);
	myButton->setState(guiElemState::DRAWED);
	myButton->setScale(50, 10, 1, false, true);
	myButton->setColor(wildWeasel::color::white());
	myButton->setRotation(0,0,0,false, true);
	myButton->setPosition(-50,-30,0,false, true);
	myButton->setTexture(textureRect);
	myButton->setText(L"I am a unique plain button!");
}

//-----------------------------------------------------------------------------
// Name: createrotationControlCube()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::examples::createrotationControlCube(masterMind* ww, wildWeasel::texture* textureRect, font3D* theFont)
{
	cubicButton*			myButton	= new cubicButton();
	matrixControl3D*		myMatrix	= new matrixControl3D();
	matrixControl3D*		myMatrix2	= new matrixControl3D();
	guiElemCluster3D*		myCluster	= new guiElemCluster3D();
	rotationControlCube*	myCtrl		= new rotationControlCube();
	triad*					myTriad		= new triad();

	myButton->create(ww);
	myButton->setState(guiElemState::DRAWED);
	myButton->setScale(50, 10, 20, false, true);
	myButton->setColor(wildWeasel::color::white());
	myButton->setRotation(1,1,1,false, true);		
	myButton->insertMatrix(1, &myMatrix->mat, &myMatrix->dirtyBit);	// Rotation ans translation are separated, since rotation by rotationControlCube shall be between. 
	myMatrix->setPosition(0,40,0,false, true);
	myButton->setTexture(textureRect);
	myButton->setText(L"I am a unique cubic button!");

	myTriad->create(ww, theFont);
	myTriad->insertMatrix(1, &myMatrix2->mat, &myMatrix2->dirtyBit);
	myMatrix2->setPosition(-50,-30,20,false, true);
	myTriad->setScale(10,10,10,false, true);
	myTriad->setState(guiElemState::DRAWED);

	myCluster->create(ww);
	myCluster->addItem(myTriad);
	myCluster->addItem(myButton);

	myCtrl->create(ww, theFont);
	//myCtrl->linkToObject(myButton, 1);
	//myCtrl->linkToObject(myTriad, 1);
	myCtrl->linkToObject(myCluster, 1);
	myCtrl->setState(guiElemState::DRAWED);
	myCtrl->setPosition(50,-30,20,false, false);
	myCtrl->setScale(20,20,20,false, true);
}

//-----------------------------------------------------------------------------
// Name: createPlainButton2D()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::examples::createPlainButton2D(masterMind* ww, font2D* theFont, buttonImageFiles &filesVoid, texture &texLine, texture &texCorner)
{
	wildWeasel::plainButton2D*		myButton2D	= new wildWeasel::plainButton2D();
	wildWeasel::plainButton2D*		myButton2E	= new wildWeasel::plainButton2D();
	wildWeasel::plainButton2D*		myButton2F	= new wildWeasel::plainButton2D();
	wildWeasel::plainButton2D*		myButton2G	= new wildWeasel::plainButton2D();
	wildWeasel::borderLine2D*		myBorder	= new wildWeasel::borderLine2D();
	wildWeasel::timer*				myTimer		= new wildWeasel::timer();
	RECT								rc			= {20, 10, 280, 40};
	fRECT*								rcClip		= new fRECT{150, 30, 230, 320};
	wildWeasel::alignment*			amButton2G	= new wildWeasel::alignment{ wildWeasel::alignmentTypeX::FRACTION, 0.033f, wildWeasel::alignmentTypeY::FRACTION  , 0.92f, wildWeasel::alignmentTypeX::PIXEL_WIDTH, 120, wildWeasel::alignmentTypeY::PIXEL_HEIGHT, 30, wildWeasel::alignmentTypeX::FRACTION,   0.05f, wildWeasel::alignmentTypeY::FRACTION,   0.02f, 5};
	wildWeasel::alignment*			amArea		= new wildWeasel::alignment{ wildWeasel::alignmentTypeX::FRACTION, 0.033f, wildWeasel::alignmentTypeY::FRACTION  , 0.92f, wildWeasel::alignmentTypeX::FRACTION, 0.8f, wildWeasel::alignmentTypeY::FRACTION, 0.8f};

	myButton2D->create(ww, filesVoid, nullptr, nullptr, 0.2f);
	myButton2D->setTargetRect(rc);
	myButton2D->setTextSize(1.2f, 1.2f);
	myButton2D->setText(L"Text");
	myButton2D->setFont(theFont);
	myButton2D->setTextState(wildWeasel::guiElemState::DRAWED);
	myButton2D->setState(wildWeasel::guiElemState::DRAWED);
	myButton2D->setTextColor(wildWeasel::color(255,100,0));
	myButton2D->setColor(wildWeasel::color::white());
	myButton2D->setPositioningMode(wildWeasel::matrixControl2D::matControlMode::posRotSca);
	myButton2D->setTextAlignmentHorizontal(wildWeasel::alignmentHorizontal::RIGHT);
	myButton2D->setTextAlignmentVertical(wildWeasel::alignmentVertical::CENTER);
	myButton2D->setPosition (130, 120, true);
	myButton2D->setRotation (-0.5f, true);
	myButton2D->setScale	(0.5f, 0.8f, true);
	myButton2D->setClippingRect(rcClip);

	myButton2E->create(ww, filesVoid, nullptr, nullptr, 0.1f);
	myButton2E->setTargetRect(rc);
	myButton2E->setTextSize(1.2f, 1.2f);
	myButton2E->setText(L"Text");
	myButton2E->setFont(theFont);
	myButton2E->setTextState(wildWeasel::guiElemState::DRAWED);
	myButton2E->setState(wildWeasel::guiElemState::DRAWED);
	myButton2E->setTextColor(wildWeasel::color(255,100,0));
	myButton2E->setColor(wildWeasel::color::red());
	myButton2E->setPositioningMode(wildWeasel::matrixControl2D::matControlMode::posRotSca);
	myButton2E->setTextAlignmentHorizontal(wildWeasel::alignmentHorizontal::RIGHT);
	myButton2E->setTextAlignmentVertical(wildWeasel::alignmentVertical::CENTER);
	myButton2E->setPosition (130, 120, true);
	myButton2E->setRotation (-0.5f, true);
	myButton2E->setScale	(0.5f, 0.8f, true);

	myButton2F->create(ww, filesVoid, nullptr, nullptr, 0.1f);
	myButton2F->setTargetRect(rc);
	myButton2F->setTextSize(1.2f, 1.2f);
	myButton2F->setText(L"Text");
	myButton2F->setFont(theFont);
	myButton2F->setTextState(wildWeasel::guiElemState::DRAWED);
	myButton2F->setState(wildWeasel::guiElemState::DRAWED);
	myButton2F->setTextColor(wildWeasel::color(255,100,0));
	myButton2F->setColor(wildWeasel::color::lightBlue());
	myButton2F->setPositioningMode(wildWeasel::matrixControl2D::matControlMode::posRotSca);
	myButton2F->setTextAlignmentHorizontal(wildWeasel::alignmentHorizontal::RIGHT);
	myButton2F->setTextAlignmentVertical(wildWeasel::alignmentVertical::CENTER);
	myButton2F->setPosition (130, 120, true);
	myButton2F->setRotation (0.0f, true);
	myButton2F->setScale	(0.5f, 0.8f, true);
	
	myButton2G->create(ww, filesVoid, nullptr, nullptr, 0.1f);
	myButton2G->setText(L"Aligned Button");
	myButton2G->setFont(theFont);
	myButton2G->setTextState(wildWeasel::guiElemState::DRAWED);
	myButton2G->setState(wildWeasel::guiElemState::DRAWED);
	myButton2G->setTextColor(wildWeasel::color(0,0,0));
	myButton2G->setColor(wildWeasel::color::white());
	myButton2G->setTextAlignmentHorizontal(wildWeasel::alignmentHorizontal::CENTER);
	myButton2G->setTextAlignmentVertical(wildWeasel::alignmentVertical::CENTER);
	myButton2G->setAlignment(*amButton2G, 1);

	amButton2G->setInsideAnotherRect(*amArea);

	myBorder->create(ww, texLine, texCorner, 0);
	myBorder->setFont(theFont);
	myBorder->setState(guiElemState::DRAWED);
	myBorder->setText(L"");
	myBorder->setTargetRect(rcClip->getRECT());
	myBorder->setPosition(0, 0, false);
	myBorder->setRotation(0, true);
	myBorder->setTextWidth(0);
	myBorder->setGapWidthBetweenTextAndLine(0);

	myTimer->setFunc(ww, createPlainButton2D_timerFunc, nullptr);
	myTimer->start(100);
}

//-----------------------------------------------------------------------------
// Name: createCheckBox2D()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::examples::createPlainButton2D_timerFunc(void* pUser)
{
	//curRot += 0.01f;
	//myButton2D->setRotation(curRot, true);
	//myButton2E->setRotation(curRot, true);
}

//-----------------------------------------------------------------------------
// Name: createCheckBox2D()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::examples::createCheckBox2D(masterMind* ww, font2D* theFont, buttonImageFiles &filesChecked, buttonImageFiles &filesUnchecked)
{
	checkBox2D*		myCheckBox	= new checkBox2D();
	RECT			rc			= {450, 100, 480, 130};

	myCheckBox->create(ww, filesChecked, filesUnchecked, 0);
	myCheckBox->setFont(theFont);
	myCheckBox->setState(guiElemState::DRAWED);
	myCheckBox->setText(L"Checkbox");
	myCheckBox->setTargetRect(rc);
}

//-----------------------------------------------------------------------------
// Name: createBorderLine2D()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::examples::createBorderLine2D(masterMind* ww, font2D* theFont, texture &texLine, texture &texCorner)
{
	borderLine2D*	myBorderLine	= new borderLine2D();
	RECT			rc				= {0, 0, 330, 180};

	myBorderLine->create(ww, texLine, texCorner, 0);
	myBorderLine->setFont(theFont);
	myBorderLine->setState(guiElemState::DRAWED);
	myBorderLine->setText(L"Border Line");
	myBorderLine->setTargetRect(rc);
	myBorderLine->setPosition(350, 50, false);
	myBorderLine->setRotation(1, true);
}

//-----------------------------------------------------------------------------
// Name: createDrowDown2D()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::examples::createDrowDown2D(masterMind* ww, font2D* theFont, buttonImageFiles &filesButton, texture &texArrow, buttonImageFiles &filesTriangle, buttonImageFiles &filesVoid)
{
	dropDown2D*		myDropDown	= new dropDown2D();
	RECT			rc			= {450, 150, 650, 190};
	wstringstream	wss;

	myDropDown->create(ww, filesButton, texArrow, 0);
	myDropDown->setFont(theFont);
	myDropDown->setState(guiElemState::DRAWED);
	myDropDown->setText(L"Drop Down Menu");
	myDropDown->setTextSize(0.7f, 0.7f);
	myDropDown->setTargetRect(rc);
	myDropDown->createScrollBars(filesTriangle, filesVoid, filesVoid);
	myDropDown->setVisibilityRowScrollBar(true);
	myDropDown->setRowScrollBarWidth(20);

	for (unsigned int i=0; i<6; i++) {
		wildWeasel::guiElemCluster2D*	myCluster	= new wildWeasel::guiElemCluster2D();
		wildWeasel::plainButton2D*		myButton2D	= new wildWeasel::plainButton2D();
		wildWeasel::textLabel2D*		myText2D	= new wildWeasel::textLabel2D();
		RECT								rc			= {0, 0, 100, 40};
	
		myButton2D->create(ww, filesButton, nullptr, nullptr, 0);
		myButton2D->setTargetRect(rc);
		myButton2D->setTextSize(1.2f, 1.2f);
		myButton2D->setText(L"T");
		myButton2D->setFont(theFont);
		myButton2D->setTextState(wildWeasel::guiElemState::DRAWED);
		myButton2D->setTextColor(wildWeasel::color(255,100,0));
		myButton2D->setColor(wildWeasel::color::white());
		myButton2D->setPositioningMode(wildWeasel::matrixControl2D::matControlMode::posRotSca);
		myButton2D->setTextAlignmentHorizontal(wildWeasel::alignmentHorizontal::RIGHT);
		myButton2D->setTextAlignmentVertical(wildWeasel::alignmentVertical::CENTER);
		myButton2D->setPosition (10, 0, true);
		myButton2D->setScale	(0.5f, 0.8f, true);
	
		myText2D->create(ww, wstring(L"abc"), theFont, 0);
		myText2D->setTextColor(wildWeasel::color(0,100,200));
		myText2D->setTextState(wildWeasel::guiElemState::DRAWED);
		myText2D->setTargetRect(rc);
		myText2D->setRotation (0, true);
		myText2D->setTextAlignmentHorizontal(wildWeasel::alignmentHorizontal::LEFT);
		myText2D->setPosition (120, 0, true);
	
		myCluster->create(ww);
		myCluster->addItem(myText2D);
		myCluster->addItem(myButton2D);
		myCluster->setState			(wildWeasel::guiElemState::DRAWED);
		myCluster->setTextStates	(wildWeasel::guiElemState::DRAWED);

		wss.str(L""); wss << i;
		myDropDown->insertItem(i, wss.str(), filesButton, myCluster);
	}
}

//-----------------------------------------------------------------------------
// Name: createDrowDown2D()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::examples::showOpenFileDialog(masterMind* ww)
{
	wstring			filePath;
	wstring			path;
	vector<wstring> files;

	ww->getOpenFileName(path, files, ww->makeFileTypeFilter({{L"All files (*.*)", L"*.*"}, {L"Executable (*.exe)", L"*.exe"}, {L"Text (*.txt)", L"*.txt"}}), L"C:\\", L"My special dialog title", L"txt", true, true);
	ww->getSaveFileName(filePath);
}