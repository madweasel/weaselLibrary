/*********************************************************************
	wwRotCtrlCube.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/

#include "wwRotCtrlCube.h"

/*************************************************************************************************************************************/

#pragma region rotationControlCube

//-----------------------------------------------------------------------------
// Name: create()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::rotationControlCube::create(masterMind* ww, font3D* theFont)
{
	// params ok?
	if (!ww)			return false;
	if (initialized)	return false;

	guiElemCluster3D::create(ww);
	
	// numRollStates[]
	numRollStates[idCenterArea    ]	= 4;
	numRollStates[idUpperArea	  ]	= 4;
	numRollStates[idLowerArea	  ]	= 4;
	numRollStates[idRightArea	  ]	= 4;
	numRollStates[idLeftArea	  ]	= 4;
	numRollStates[idLowerLeftArea ]	= 3;
	numRollStates[idLowerRightArea]	= 3;
	numRollStates[idUpperLeftArea ]	= 3;
	numRollStates[idUpperRightArea]	= 3;
	
	// connectedAreas[][]
	setDoubleConn (idRightFace, idUpperArea,		idTopFace,		idRightArea);
	setDoubleConn (idRightFace, idLowerArea,		idBottomFace,	idRightArea);
	setDoubleConn (idLeftFace,  idUpperArea,		idTopFace,		idLeftArea);
	setDoubleConn (idLeftFace,  idLowerArea,		idBottomFace,	idLeftArea);

	setDoubleConn (idFrontFace, idUpperArea,		idTopFace,		idLowerArea);
	setDoubleConn (idFrontFace, idLowerArea,		idBottomFace,	idUpperArea);
	setDoubleConn (idFrontFace, idRightArea,		idRightFace,	idLeftArea);
	setDoubleConn (idFrontFace, idLeftArea,			idLeftFace,		idRightArea);
	setTrippleConn(idFrontFace, idUpperLeftArea	,	idTopFace,		idLowerLeftArea,	idLeftFace,		idUpperRightArea);
	setTrippleConn(idFrontFace, idLowerLeftArea	,	idLeftFace,		idLowerRightArea,	idBottomFace,	idUpperLeftArea);
	setTrippleConn(idFrontFace, idUpperRightArea,	idRightFace,	idUpperLeftArea,	idTopFace,		idLowerRightArea);
	setTrippleConn(idFrontFace, idLowerRightArea,	idBottomFace,	idUpperRightArea,	idRightFace,	idLowerLeftArea);

	setDoubleConn (idBackFace, idUpperArea,			idTopFace,		idUpperArea);
	setDoubleConn (idBackFace, idLowerArea,			idBottomFace,	idLowerArea);
	setDoubleConn (idBackFace, idRightArea,			idLeftFace,		idLeftArea);
	setDoubleConn (idBackFace, idLeftArea,			idRightFace,	idRightArea);
	setTrippleConn(idBackFace, idUpperLeftArea,		idRightFace,	idUpperRightArea,	idTopFace,		idUpperRightArea);
	setTrippleConn(idBackFace, idLowerLeftArea,		idBottomFace,	idLowerRightArea,	idRightFace,	idLowerRightArea);
	setTrippleConn(idBackFace, idUpperRightArea,	idTopFace,		idUpperLeftArea,	idLeftFace,		idUpperLeftArea);
	setTrippleConn(idBackFace, idLowerRightArea,	idLeftFace,		idLowerLeftArea,	idBottomFace,	idLowerLeftArea);

	// create plain buttons with text
	makeFace(L"front",	idFrontFace , theFont);
	makeFace(L"right",	idRightFace , theFont);
	makeFace(L"back",	idBackFace  , theFont);
	makeFace(L"left",	idLeftFace  , theFont);
	makeFace(L"top",	idTopFace   , theFont);
	makeFace(L"bottom", idBottomFace, theFont);

	texHomeButton		.loadFile(ww, L"buttonHome.png",		true);
	texNinetyDegButton	.loadFile(ww, L"buttonTriangle.png",	true);
	texRollLeft			.loadFile(ww, L"buttonRollLeft.png",	true);
	texRollRight		.loadFile(ww, L"buttonRollRight.png",	true);
	ww->graphicManager.performResourceUpload();

	// create home button
	createButton(buttonHome,		vector3(0.6f * edgeLength, 0.5f * edgeLength, -edgeLength/2), vector3(0,0,0), vector3(homeButtonSize), texHomeButton, bind(&wildWeasel::rotationControlCube::leftClickOnHomeButton,	this, placeholders::_1, placeholders::_2));

	// create roll buttons
	createButton(buttonRollLeft,	vector3(-0.7f * edgeLength, 0.45f * edgeLength, -edgeLength/2), vector3(0,       0, 0.25*wwc::PI), vector3(homeButtonSize), texRollLeft,  bind(&wildWeasel::rotationControlCube::leftClickOnRollButton,	this, placeholders::_1, placeholders::_2));
	createButton(buttonRollRight,	vector3(-0.6f * edgeLength, 0.65f * edgeLength, -edgeLength/2), vector3(0,       0, 0.25*wwc::PI), vector3(homeButtonSize), texRollRight, bind(&wildWeasel::rotationControlCube::leftClickOnRollButton,	this, placeholders::_1, placeholders::_2));

	// create 90 degree roation buttons
	createButton(buttonLeft,		vector3(-edgeLength*0.6f, 0, -edgeLength/2), vector3(0,0,-wwc::PI/2), vector3(homeButtonSize), texNinetyDegButton, bind(&wildWeasel::rotationControlCube::leftClickOn90DegButton,	this, placeholders::_1, placeholders::_2));
	createButton(buttonTop,			vector3(0, +edgeLength*0.6f, -edgeLength/2), vector3(0,0,+wwc::PI/1), vector3(homeButtonSize), texNinetyDegButton, bind(&wildWeasel::rotationControlCube::leftClickOn90DegButton,	this, placeholders::_1, placeholders::_2));
	createButton(buttonRight,		vector3(+edgeLength*0.6f, 0, -edgeLength/2), vector3(0,0,+wwc::PI/2), vector3(homeButtonSize), texNinetyDegButton, bind(&wildWeasel::rotationControlCube::leftClickOn90DegButton,	this, placeholders::_1, placeholders::_2));
	createButton(buttonBottom,		vector3(0, -edgeLength*0.6f, -edgeLength/2), vector3(0,0,         0), vector3(homeButtonSize), texNinetyDegButton, bind(&wildWeasel::rotationControlCube::leftClickOn90DegButton,	this, placeholders::_1, placeholders::_2));

	// rotate cube, so that the front face points to the camera
	this->insertMatrix(1, &matRotToCameraForCube, &dummyDirtyBit);	// ... controlledDirtyBit sollte noch was anderes sein

	// init helper for the matrix rotation animation
	matAnimation = new matrixControlAnimation(ww);
	matAnimation->append(&controlledMatrix, &controlledDirtyBit);	
	this->insertMatrix(1, &controlledMatrix, &controlledDirtyBit);
	this->faceAreaState		= faceAreaIndex(idFrontFace, idCenterArea);

	buttonHome		.removeMatrix(&controlledMatrix);
	buttonRollLeft	.removeMatrix(&controlledMatrix);
	buttonRollRight	.removeMatrix(&controlledMatrix);
	buttonLeft		.removeMatrix(&controlledMatrix);
	buttonTop		.removeMatrix(&controlledMatrix);
	buttonRight		.removeMatrix(&controlledMatrix);
	buttonBottom	.removeMatrix(&controlledMatrix);

	// centre (rotation order is pitch (x-axis), yaw (y-axis), roll (z-axis))
	makeStateMatrix(idFrontFace , idCenterArea		, true,      0,       0, 4, wwc::PI/2);
	makeStateMatrix(idRightFace , idCenterArea		, true, 1* wwc::PI/2,       0, 4, wwc::PI/2);
	makeStateMatrix(idBackFace  , idCenterArea		, true, 2* wwc::PI/2,       0, 4, wwc::PI/2);
	makeStateMatrix(idLeftFace  , idCenterArea		, true, 3* wwc::PI/2,       0, 4, wwc::PI/2);
	makeStateMatrix(idTopFace   , idCenterArea		, true,      0,   - wwc::PI/2, 4, wwc::PI/2);
	makeStateMatrix(idBottomFace, idCenterArea		, true,      0,   + wwc::PI/2, 4, wwc::PI/2);
	
	// edges
	makeStateMatrix(idFrontFace, idUpperArea		, true,       0, -1* wwc::PI/4, 4, wwc::PI/2);
	makeStateMatrix(idFrontFace, idLowerArea		, true,       0, +1* wwc::PI/4, 4, wwc::PI/2);
	makeStateMatrix(idFrontFace, idLeftArea			, true, -1* wwc::PI/4,       0, 4, wwc::PI/2);
	makeStateMatrix(idFrontFace, idRightArea		, true, +1* wwc::PI/4,       0, 4, wwc::PI/2);
	
	makeStateMatrix(idRightFace, idUpperArea		, false,     wwc::PI/2, -1* wwc::PI/4, 4, wwc::PI/2);
	makeStateMatrix(idRightFace, idLowerArea		, false,     wwc::PI/2,  1* wwc::PI/4, 4, wwc::PI/2);
	makeStateMatrix(idLeftFace,  idUpperArea		, false, -1* wwc::PI/2, -1* wwc::PI/4, 4, wwc::PI/2);
	makeStateMatrix(idLeftFace,  idLowerArea		, false, -1* wwc::PI/2,  1* wwc::PI/4, 4, wwc::PI/2);

	makeStateMatrix(idBackFace,	 idUpperArea		, true,       0,  5* wwc::PI/4, 4, wwc::PI/2);
	makeStateMatrix(idBackFace,	 idLowerArea		, true,       0,  3* wwc::PI/4, 4, wwc::PI/2);
	makeStateMatrix(idBackFace,	 idLeftArea			, true,  3* wwc::PI/4,       0, 4, wwc::PI/2);
	makeStateMatrix(idBackFace,	 idRightArea		, true,  5* wwc::PI/4,       0, 4, wwc::PI/2);
	
	// corners
	makeStateMatrix(idFrontFace, idUpperLeftArea	, true, -1* wwc::PI/4, -1*wwc::PI/4, 3, 2*wwc::PI/3);
	makeStateMatrix(idFrontFace, idLowerLeftArea	, true, -1* wwc::PI/4, +1*wwc::PI/4, 3, 2*wwc::PI/3);
	makeStateMatrix(idFrontFace, idUpperRightArea	, true, +1* wwc::PI/4, -1*wwc::PI/4, 3, 2*wwc::PI/3);
	makeStateMatrix(idFrontFace, idLowerRightArea	, true, +1* wwc::PI/4, +1*wwc::PI/4, 3, 2*wwc::PI/3);

	makeStateMatrix(idBackFace,	idUpperLeftArea		, true,  3*wwc::PI/4, +1*wwc::PI/4, 3, 2*wwc::PI/3);
	makeStateMatrix(idBackFace,	idLowerLeftArea		, true,  3*wwc::PI/4, -1*wwc::PI/4, 3, 2*wwc::PI/3);
	makeStateMatrix(idBackFace,	idUpperRightArea	, true,  5*wwc::PI/4, +1*wwc::PI/4, 3, 2*wwc::PI/3);
	makeStateMatrix(idBackFace,	idLowerRightArea	, true,  5*wwc::PI/4, -1*wwc::PI/4, 3, 2*wwc::PI/3);

	// calc target states for the buttons buttonLeft, buttonRight, etc.
	calcStateAfter90DegRot();

	// track mouse moves and clicks
	eventFollower::followEvent(this, eventType::WINDOWSIZE_CHANGED);

	this->theFont	= theFont; 
	this->ww		= ww;

	return initialized;
}

//-----------------------------------------------------------------------------
// Name: createButton()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::rotationControlCube::createButton(plainButton& myButton, vector3 const & position, vector3 const & rotation, vector3 const & scale, texture& tex, function<void(guiElemEvFol*, void*)> userFunc)
{
	myButton.create(ww, nullptr, nullptr, true);
	myButton.setPositioningMode(matrixControl3D::matControlMode::posRotSca);
	myButton.setPosition(position, false, false);
	myButton.setRotation(rotation, false, false);
	myButton.setScale(scale, false, true);
	myButton.setState(guiElemState::DRAWED);
	myButton.setColor(color::white());
	myButton.setTexture(&tex);
	myButton.assignOnLeftMouseButtonPressed(userFunc, &myButton);
	myButton.setHoverColor(colorSelected);
	this->addItem(&myButton);
}

//-----------------------------------------------------------------------------
// Name: linkToObject()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::rotationControlCube::linkToObject(guiElement3D* elem, unsigned int matrixPosition)
{
	if (!elem) return false;
	if (controlledCluster) unlinkObject();
	controlledElem = elem;
	elem->insertMatrix(matrixPosition, &matRotToCameraForObject,	&dummyDirtyBit);
	elem->insertMatrix(matrixPosition, &controlledMatrix,			&controlledDirtyBit);
	elem->insertMatrix(matrixPosition, &matRotToCameraForObjectInv,	&dummyDirtyBit);
	return true;
}

//-----------------------------------------------------------------------------
// Name: linkToObject()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::rotationControlCube::linkToObject(guiElemCluster3D* cluster, unsigned int matrixPosition)
{
	if (!cluster) return false;
	if (controlledElem) unlinkObject();
	controlledCluster = cluster;
	cluster->insertMatrix(matrixPosition, &matRotToCameraForObject,		&dummyDirtyBit);
	cluster->insertMatrix(matrixPosition, &controlledMatrix,			&controlledDirtyBit);
	cluster->insertMatrix(matrixPosition, &matRotToCameraForObjectInv,	&dummyDirtyBit);
	return true;
}

//-----------------------------------------------------------------------------
// Name: unlinkObject()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::rotationControlCube::unlinkObject()
{
	if (controlledElem)		{
		controlledElem->removeMatrix(&matRotToCameraForObject);
		controlledElem->removeMatrix(&controlledMatrix);
		controlledElem->removeMatrix(&matRotToCameraForObjectInv);
		controlledElem = nullptr;
	}
	if (controlledCluster)	{
		controlledCluster->removeMatrix(&matRotToCameraForObject);
		controlledCluster->removeMatrix(&controlledMatrix);
		controlledCluster->removeMatrix(&matRotToCameraForObjectInv);
		controlledCluster = nullptr;
	}
}

//-----------------------------------------------------------------------------
// Name: makeArea()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::rotationControlCube::makeArea(wstring text, idFace face, idFaceSubArea area, float scaleX, float scaleY, float scaleZ, float transX, float transY, float transZ, matrix& matrixRotation, font3D* theFont)
{
	// parameters ok?
	if (face >= numFaces) return;
	if (area >= numAreas) return;
	
	// plain matrix
	matrix matScale = matrix::CreateScale(scaleX, scaleY, scaleZ);				// scale
	matrix matTrans = matrix::CreateTranslation(transX, transY, transZ);		// translate
	matrix matArea = (matScale * matTrans) * matrixRotation;					// multiply

	areas[face][area].fai = faceAreaIndex{face, area};
	
	for (size_t roll=0; roll<maxRollStates; roll++) {
		areas[face][area].stateMatrices[roll] = matrix::Identity;
	}

	auto& myButton = areas[face][area].button;
	myButton.create(ww, nullptr, nullptr, false);
	myButton.setMatrix(&matArea, false);
	myButton.setPositioningMode(matrixControl3D::matControlMode::matrix);
	myButton.setState(guiElemState::DRAWED);
	myButton.setColor(area==idCenterArea ? colorNormalCenter : colorNormalAround);
	myButton.setText(text);
	myButton.setTextState(text.size() ? guiElemState::DRAWED : guiElemState::HIDDEN);
	myButton.setTextAlignmentHorizontal(wildWeasel::alignmentHorizontal::CENTER);
	myButton.setTextAlignmentVertical(wildWeasel::alignmentVertical::CENTER);
	myButton.setTextColor(color::white());
	myButton.setFont(theFont);
	myButton.setShowTextOnlyForFrontSide(true);
	myButton.assignOnLeftMouseButtonPressed	(bind(&wildWeasel::rotationControlCube::leftClickOnArea,	this, placeholders::_1, placeholders::_2), (void*) &areas[face][area]);
	myButton.assignOnMouseEnteredRegion		(bind(&wildWeasel::rotationControlCube::hoverArea,			this, placeholders::_1, placeholders::_2), (void*) &areas[face][area]);
	myButton.assignOnMouseLeftRegion		(bind(&wildWeasel::rotationControlCube::leftArea,			this, placeholders::_1, placeholders::_2), (void*) &areas[face][area]);
	this->addItem(&myButton);
}

//-----------------------------------------------------------------------------
// Name: makeFace()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::rotationControlCube::makeFace(wstring text, idFace face, font3D* theFont)
{
	// locals
	matrix matrixRotation;

	// calc matrixRotation
	switch (face) 
	{
	case idFrontFace :	matrixRotation = matrix::CreateRotationY(		   0);	break;
	case idRightFace :	matrixRotation = matrix::CreateRotationY( -wwc::PI/2);	break;
	case idBackFace  :	matrixRotation = matrix::CreateRotationY( +wwc::PI/1);	break;
	case idLeftFace  :	matrixRotation = matrix::CreateRotationY( +wwc::PI/2);	break;
	case idTopFace   :	matrixRotation = matrix::CreateRotationX( +wwc::PI/2);	break;
	case idBottomFace:	matrixRotation = matrix::CreateRotationX( -wwc::PI/2);	break;	
	default:			return;
	}

	makeArea(text, face, idCenterArea,		1-2*edgeFraction, 1-2*edgeFraction, 1, 				      0, 		  	        0, -0.5f, matrixRotation, theFont);
	makeArea(L"",  face, idUpperArea,		1-2*edgeFraction, 0+1*edgeFraction, 1, 				      0,  0.5f-edgeFraction/2, -0.5f, matrixRotation, theFont);
	makeArea(L"",  face, idLowerArea,		1-2*edgeFraction, 0+1*edgeFraction, 1, 				      0, -0.5f+edgeFraction/2, -0.5f, matrixRotation, theFont);
	makeArea(L"",  face, idRightArea,		0+1*edgeFraction, 1-2*edgeFraction, 1,  0.5f-edgeFraction/2, 				    0, -0.5f, matrixRotation, theFont);
	makeArea(L"",  face, idLeftArea,		0+1*edgeFraction, 1-2*edgeFraction, 1, -0.5f+edgeFraction/2, 				    0, -0.5f, matrixRotation, theFont);
	makeArea(L"",  face, idUpperRightArea,	0+1*edgeFraction, 0+1*edgeFraction, 1,  0.5f-edgeFraction/2,  0.5f-edgeFraction/2, -0.5f, matrixRotation, theFont);
	makeArea(L"",  face, idUpperLeftArea,	0+1*edgeFraction, 0+1*edgeFraction, 1, -0.5f+edgeFraction/2,  0.5f-edgeFraction/2, -0.5f, matrixRotation, theFont);
	makeArea(L"",  face, idLowerRightArea,	0+1*edgeFraction, 0+1*edgeFraction, 1,  0.5f-edgeFraction/2, -0.5f+edgeFraction/2, -0.5f, matrixRotation, theFont);
	makeArea(L"",  face, idLowerLeftArea,	0+1*edgeFraction, 0+1*edgeFraction, 1, -0.5f+edgeFraction/2, -0.5f+edgeFraction/2, -0.5f, matrixRotation, theFont);
}

//-----------------------------------------------------------------------------
// Name: setDoubleConn()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::rotationControlCube::setDoubleConn(idFace face1, idFaceSubArea area1, idFace face2, idFaceSubArea area2)
{
	areas[face1][area1].connectedAreas[0] = faceAreaIndex(face2,			area2);
	areas[face2][area2].connectedAreas[0] = faceAreaIndex(face1,			area1);
	areas[face1][area1].connectedAreas[1] = faceAreaIndex(idInvalidFace,	idInvalidArea);
	areas[face2][area2].connectedAreas[1] = faceAreaIndex(idInvalidFace,	idInvalidArea);
}

//-----------------------------------------------------------------------------
// Name: setTrippleConn()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::rotationControlCube::setTrippleConn(idFace face1, idFaceSubArea area1, idFace face2, idFaceSubArea area2, idFace face3, idFaceSubArea area3)
{
	areas[face1][area1].connectedAreas[0] = faceAreaIndex(face2,		area2);
	areas[face2][area2].connectedAreas[0] = faceAreaIndex(face3,		area3);
	areas[face3][area3].connectedAreas[0] = faceAreaIndex(face1,		area1);
	areas[face1][area1].connectedAreas[1] = faceAreaIndex(face3,		area3);
	areas[face2][area2].connectedAreas[1] = faceAreaIndex(face1,		area1);
	areas[face3][area3].connectedAreas[1] = faceAreaIndex(face2,		area2);
}

//-----------------------------------------------------------------------------
// Name: makeStateMatrix()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::rotationControlCube::makeStateMatrix(idFace face, idFaceSubArea area, bool firstRotAroundX, float yaw, float pitch, int numRollSteps, float rollStepSize)
{
	// locals
	faceAreaIndex	partner;
	matrix			rotX, rotY, rotZ, rotA;
	int				roll;

	// prepare rotation around axis
	rotX = matrix::CreateRotationX(pitch);
	rotY = matrix::CreateRotationY(yaw);

	if (firstRotAroundX) {
		rotA = rotX * rotY;
	} else {
		rotA = rotY * rotX;
	}

	for (roll=0; roll<numRollSteps; roll++) {
		rotZ = matrix::CreateRotationZ(roll*rollStepSize);
		areas[face][area].stateMatrices[roll] = rotA * rotZ;
		partner = areas[face][area].connectedAreas[0];	 if (partner.isValid()) { areas[partner.face][partner.area].stateMatrices[roll] = areas[face][area].stateMatrices[roll]; }
		partner = areas[face][area].connectedAreas[1];	 if (partner.isValid()) { areas[partner.face][partner.area].stateMatrices[roll] = areas[face][area].stateMatrices[roll]; }
	}
}

//-----------------------------------------------------------------------------
// Name: compareMatrices()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::rotationControlCube::compareMatrices(matrix const & a, matrix& b)
{
	const float maxDelta = 1.0e-6f;

	for (size_t x=0; x<4; x++) {
		for (size_t y=0; y<4; y++) {
			if (abs(a.m[x][y] - b.m[x][y]) > maxDelta) 
				return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// Name: findStateMatrix()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::rotationControlCube::faceAreaRollIndex wildWeasel::rotationControlCube::findStateMatrix(matrix const& toMatrix)
{
	unsigned int toFace, toRollState;

	for (toFace=0; toFace<numFaces; toFace++) {
		for (toRollState=0; toRollState<maxRollStates; toRollState++) {
			if (compareMatrices(toMatrix, areas[toFace][idCenterArea].stateMatrices[toRollState])) {
				return faceAreaRollIndex{faceAreaIndex{(idFace) toFace, idCenterArea}, toRollState};
			}
		}
	}
	return faceAreaRollIndex{};
}

//-----------------------------------------------------------------------------
// Name: calcStateAfter90DegRot()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::rotationControlCube::calcStateAfter90DegRot()
{
	matrix fromMatrix;
	unsigned int fromFace, fromRollState;

	for (fromFace=0; fromFace<numFaces; fromFace++) {
		for (fromRollState=0; fromRollState<maxRollStates; fromRollState++) {
			fromMatrix = areas[fromFace][idCenterArea].stateMatrices[fromRollState];
			
			stateAfter90DegRot[fromFace][fromRollState][idLeft		] = findStateMatrix(fromMatrix * matrix::CreateRotationY(-wwc::PI/2));
			stateAfter90DegRot[fromFace][fromRollState][idRight		] = findStateMatrix(fromMatrix * matrix::CreateRotationY(+wwc::PI/2));
			stateAfter90DegRot[fromFace][fromRollState][idTop		] = findStateMatrix(fromMatrix * matrix::CreateRotationX(-wwc::PI/2));
			stateAfter90DegRot[fromFace][fromRollState][idBottom	] = findStateMatrix(fromMatrix * matrix::CreateRotationX(+wwc::PI/2));
		}
	}
}

//-----------------------------------------------------------------------------
// Name: ~rotationControlCube()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::rotationControlCube::~rotationControlCube()
{
	unlinkObject();
}

//-----------------------------------------------------------------------------
// Name: getRotationAngle()
// Desc: 
//-----------------------------------------------------------------------------
float wildWeasel::rotationControlCube::getRotationAngle(matrix& matRotFrom, matrix& matRotTo)
{
	// Problem:  deltaMatrix*matRotFrom*x = matRotTo*x
	// 			 deltaMatrix*matRotFrom   = matRotTo
	// Solution: deltaMatrix = matRotTo*matRotFrom^(-1) = matRotTo*matRotFrom^(T)
	//			 angle		 = acos((Spur(deltaMatrix)+2-3)/2)
	matrix matRotFromInverse	= matRotFrom.Transpose();
	matrix deltaMatrix			= matRotTo * matRotFromInverse;
	return acos((deltaMatrix._11 + deltaMatrix._22 + deltaMatrix._33 - 1) / 2);
}

//-----------------------------------------------------------------------------
// Name: setPosition()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::rotationControlCube::setPosition(float x, float y, float z, bool relativeToView, bool updateMatrix)
{
	// locals
	vector3 cubePos		= vector3(x, y, z);
	camera& cam			= *(ww->getActiveCamera());
	vector3 vx, vy, vz;

	// set normalized triad vectors as rotation matrix
	// so that cube front face is rotated to the camera
	vz = cubePos - cam.position;
	vx = (cam.worldUp).Cross(vz);
	vy = vz.Cross(vx);
	matRotToCameraForCube = matrixCreateFromTriad(vx, vy, vz);

	// set normalized triad vectors as rotation matrix
	// so that object front face is rotated to the camera
	vz = cam.lookAt - cam.position;
	vx = (cam.worldUp).Cross(vz);
	vy = vz.Cross(vx);
	matRotToCameraForObject		= matrixCreateFromTriad(vx, vy, vz);
	matRotToCameraForObjectInv	= matRotToCameraForObject.Invert();

	// set position of cube
	matrixControl3D::setPosition(cubePos, relativeToView, updateMatrix);

	//// triad for debugging
	//triad*				myTriad1		= new triad();
	//matrixControl3D*		myMatrix1		= new matrixControl3D();
	//myTriad1->create(ww, theFont);
	//myTriad1->insertMatrix(1, &myMatrix1->mat, &dummyDirtyBit);
	//myMatrix1->setPosition(30,-45,20,false, true);
	//myMatrix1->setScale(10,10,10,false, true);
	//myTriad1->setMatrix(&matRotToCameraForCube, false);
	//myTriad1->setPositioningMode(matrixControl3D::matControlMode::matrix);
	//myTriad1->setState(guiElemState::DRAWED);
}

//-----------------------------------------------------------------------------
// Name: setColor()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::rotationControlCube::setColor(color newColorNormalCenter, color newColorNormalAround, color newColorSelected)
{
	this->colorNormalCenter	= newColorNormalCenter;
	this->colorNormalAround	= newColorNormalAround;
	this->colorSelected		= newColorSelected;
}

//-----------------------------------------------------------------------------
// Name: hoverArea()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::rotationControlCube::hoverArea(wildWeasel::guiElemEvFol* item, void* pUser)
{
	auto myArea = (areaClass*) pUser;
	myArea->button.setColor(colorSelected);
	for (size_t curConnection = 0; curConnection < numConnectedAreas; curConnection++) {
		if (myArea->connectedAreas[curConnection].isValid()) {
			areas[myArea->connectedAreas[curConnection].face][myArea->connectedAreas[curConnection].area].button.setColor(colorSelected);
		}
	}
}

//-----------------------------------------------------------------------------
// Name: leftArea()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::rotationControlCube::leftArea(wildWeasel::guiElemEvFol* item, void* pUser)
{
	auto myArea = (areaClass*) pUser;
	myArea->button.setColor(myArea->fai.area==idCenterArea ? colorNormalCenter : colorNormalAround);
	for (size_t curConnection = 0; curConnection < numConnectedAreas; curConnection++) {
		if (myArea->connectedAreas[curConnection].isValid()) {
			areas[myArea->connectedAreas[curConnection].face][myArea->connectedAreas[curConnection].area].button.setColor(myArea->fai.area==idCenterArea ? colorNormalCenter : colorNormalAround);
		}
	}
}

//-----------------------------------------------------------------------------
// Name: rotate()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::rotationControlCube::rotate(faceAreaIndex newFaceAreaState, unsigned int newRollState)
{
	// do not rotate if new state is already the current one
	if (newFaceAreaState == faceAreaState && newRollState == rollState) return;

	// start animation
	matAnimation->startAnimation(rotationDuration, areas[faceAreaState.face][faceAreaState.area].stateMatrices[rollState], areas[newFaceAreaState.face][newFaceAreaState.area].stateMatrices[newRollState]);

	// remember new state
	faceAreaState	= newFaceAreaState;
	rollState		= newRollState;

	// the 90� buttons shall only be visible when the center area is shown
	guiElemState newState = (faceAreaState.area == idCenterArea ? guiElemState::DRAWED : guiElemState::HIDDEN);
	buttonLeft		.setState(newState);
	buttonTop		.setState(newState);
	buttonRight		.setState(newState);
	buttonBottom	.setState(newState);
}

//-----------------------------------------------------------------------------
// Name: leftClickOnHomeButton()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::rotationControlCube::leftClickOnHomeButton(wildWeasel::guiElemEvFol* item, void* pUser)
{
	rotate(faceAreaIndex(idFrontFace, idCenterArea), 0);
}

//-----------------------------------------------------------------------------
// Name: leftClickOn90DegButton()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::rotationControlCube::leftClickOn90DegButton(wildWeasel::guiElemEvFol* item, void* pUser)
{
	id90DegRotation id90DegRot; 
	auto myButton = (plainButton*) pUser;

	if (myButton == &buttonLeft		) {	id90DegRot = idLeft		; }
	if (myButton == &buttonTop		) {	id90DegRot = idTop		; }
	if (myButton == &buttonRight	) {	id90DegRot = idRight	; }
	if (myButton == &buttonBottom	) {	id90DegRot = idBottom	; }

	rotate(	stateAfter90DegRot[faceAreaState.face][rollState][id90DegRot].fai, 
			stateAfter90DegRot[faceAreaState.face][rollState][id90DegRot].rollState);
}

//-----------------------------------------------------------------------------
// Name: leftClickOnRollButton()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::rotationControlCube::leftClickOnRollButton(wildWeasel::guiElemEvFol* item, void* pUser)
{
	auto myButton = (plainButton*) pUser;
	unsigned int newRollState;
	
	if (myButton == &buttonRollLeft) {
		newRollState = (rollState + 1) % numRollStates[faceAreaState.area];
	} else if (myButton == &buttonRollRight) {
		newRollState = (rollState > 0 ? rollState - 1 : numRollStates[faceAreaState.area] - 1);
	}

	// rotate to chosen matrix
	rotate(faceAreaState, newRollState);
}

//-----------------------------------------------------------------------------
// Name: leftClickOnArea()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::rotationControlCube::leftClickOnArea(wildWeasel::guiElemEvFol* item, void* pUser)
{
	// locals
	auto myArea = (areaClass*) pUser;

	// skip, if no animation is already running
	if (matAnimation->isAnimationRunning()) return;

	// skip, if current view is already selected
	if (myArea->fai == faceAreaState || myArea->connectedAreas[0] == faceAreaState || myArea->connectedAreas[1] == faceAreaState) return;

	// preparation rotation
	unsigned int	newRollState		= 0;
	faceAreaIndex	newFaceAreaState	= myArea->fai;
	float			curRotAngle;
	float			minRotAngle			= 360.0f;

	// select best roll state
	for (unsigned int roll=0; roll<numRollStates[newFaceAreaState.area]; roll++) {

		curRotAngle = getRotationAngle(areas[faceAreaState.face][faceAreaState.area].stateMatrices[rollState], areas[newFaceAreaState.face][newFaceAreaState.area].stateMatrices[roll]);

		// get minimum rotation angle
		if (curRotAngle < minRotAngle) {
			minRotAngle		= curRotAngle;
			newRollState	= roll;
		}
	}

	// rotate to chosen matrix
	rotate(newFaceAreaState, newRollState);
}

#pragma endregion
