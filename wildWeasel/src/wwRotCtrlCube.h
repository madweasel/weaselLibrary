/*********************************************************************\
	wwRotCtrlCube.h												  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/

#pragma once

#include "wildWeasel.h"

//	----------------------------
//	| area |	area	| area |
//  ----------------------------
//  |      |			|      |
//  | area |	area	| area |
//  |      |			|      |
//  ----------------------------
//	| area |	area	| area |
//  ----------------------------

namespace wildWeasel
{

// 3D cube made of plain buttons to rotate objects via a matrix
class rotationControlCube : public guiElemCluster3D, public alignedRect, protected eventFollower
{
private:
	static const int					numFaces							= 6;								// number of faces of a cube
	static const int					numAreas							= 9;								// each face have 9 areas
	static const int					numConnectedAreas					= 2;								// number of areas at the edges (2) and corners (3) of the cube which belong together
	static const int					maxRollStates						= 4;								// the object is rolled stepwise by 90/120/180 deg n-times around the view axis
	const float							edgeLength							= 1.0f;								// edge length of the control cube in space coordinates
	const float							edgeFraction						= 0.2f;								// fraction of an cube edge which is part of a corner area
	const float							textFactor							= 0.1f;								// factor by which the text is scaled compared to the plain
	float								rotationDuration					= 0.5f;								// duration in seconds of a rotation animation
	float								homeButtonSize						= 0.15f;							// size of the homebutton
				
	enum idFace							{ idFrontFace, idBackFace, idRightFace, idLeftFace, idTopFace, idBottomFace, idInvalidFace };
	enum idFaceSubArea					{ idCenterArea, idUpperArea, idLowerArea, idRightArea, idLeftArea, idUpperLeftArea, idUpperRightArea, idLowerLeftArea, idLowerRightArea, idInvalidArea };
	enum id90DegRotation				{ idLeft, idRight, idTop, idBottom, idNum90DegRotations};

	struct faceAreaIndex
	{
		idFace							face;																	// index representing the face
		idFaceSubArea					area;																	// index representing the area

										faceAreaIndex						()									{ face = idInvalidFace; area = idInvalidArea; }
										faceAreaIndex						(idFace face, idFaceSubArea area)	{ this->face = face; this->area = area; }
		// bool							operator==							(const faceAreaIndex& f)			{ return (this->face == f.face && this->area == f.area); }
	    bool 							operator==							(const faceAreaIndex& f) const 		{ return (this->face == f.face && this->area == f.area); }
		bool							isValid								()									{ return (face<numFaces && area<numAreas); }
	};

	struct faceAreaRollIndex
	{
		faceAreaIndex					fai;
		unsigned int					rollState							= 0;

										faceAreaRollIndex					()									{ }
										faceAreaRollIndex					(faceAreaIndex fai, unsigned int rollState)	{ this->rollState = rollState; this->fai = fai; }
	};

	struct areaClass
	{
		plainButton						button;																	// gui element representing the area
		faceAreaIndex					fai;																	// identifier for the corresponding area
		faceAreaIndex					connectedAreas[numConnectedAreas];										// edge and corner areas are connect to aras of the neighboured face
		matrix							stateMatrices[maxRollStates];											// matrix associated with the area
	};

	plainButton							buttonRollLeft;															// cycles through the 4 rollStates
	plainButton							buttonRollRight;														// ''
	plainButton							buttonLeft;																// rotates the cube by 90�
	plainButton							buttonTop;																// ''
	plainButton							buttonRight;															// ''
	plainButton							buttonBottom;															// ''
	plainButton							buttonHome;																// rotates the cube back, so that the front area is the current one
	areaClass							areas[numFaces][numAreas];												// arrays with all areas
	faceAreaIndex						faceAreaState;															// orientation state - current area belonging to the orientation state of the cube
	faceAreaRollIndex					stateAfter90DegRot[numFaces][maxRollStates][idNum90DegRotations];		// target state after a 90� rotation, when a button buttonLeft, buttonRight, etc. was clicked
	unsigned int						numRollStates[numAreas];												// number of rollstates of each area (center=4, edge=2, corner=3)
	unsigned int						rollState							= 0;								// orientation state - the object is rolled stepwise by 90/120/180 deg around the view axis n-times
	color								colorNormalCenter					= color::gray();					// for inner center areas
	color								colorNormalAround					= color::darkGray();				// for outer areas
	color								colorSelected						= color::red();						// for mouse over
	matrix								matRotToCameraForCube				= matrix::Identity;					// rotational matrix used so that the front face of the cube points to the camera
	matrix								matRotToCameraForObject				= matrix::Identity;					// rotational matrix used so that the object is rotated correctly: M' = A*M*A^-1
	matrix								matRotToCameraForObjectInv			= matrix::Identity;					// rotational matrix used so that the object is rotated correctly
	matrix								controlledMatrix					= matrix::Identity;					// current rotation matrix 
	bool								controlledDirtyBit					= false;							// current bit, telling if matrix has been modified since last getFinalMatrix() call
	bool								dummyDirtyBit						= true;								// 
	masterMind*							ww									= nullptr;							// 
	guiElement3D*						controlledElem						= nullptr;							// linked gui element
	guiElemCluster3D*					controlledCluster					= nullptr;							// linked cluster
	matrixControlAnimation*				matAnimation						= nullptr;							// helper object to perform the animations
	font3D*								theFont								= nullptr;
	texture								texNinetyDegButton;
	texture								texHomeButton;
	texture								texRollLeft;
	texture								texRollRight;

	void								hoverArea							(guiElemEvFol* item, void* pUser);
	void								leftArea							(guiElemEvFol* item, void* pUser);
	void								leftClickOnArea						(guiElemEvFol* item, void* pUser);
	void								leftClickOnHomeButton				(guiElemEvFol* item, void* pUser);
	void								leftClickOnRollButton				(guiElemEvFol* item, void* pUser);
	void								leftClickOn90DegButton				(guiElemEvFol* item, void* pUser);
	void								setDoubleConn						(idFace face1, idFaceSubArea area1, idFace face2, idFaceSubArea area2);
	void								setTrippleConn						(idFace face1, idFaceSubArea area1, idFace face2, idFaceSubArea area2, idFace face3, idFaceSubArea area3);
	void								makeStateMatrix						(idFace face, idFaceSubArea area, bool firstRotAroundX, float yaw, float pitch, int numRollSteps, float rollStepSize);
	void								makeFace							(wstring text, idFace face, font3D* theFont);
	void								makeArea							(wstring text, idFace face, idFaceSubArea area, float scaleX, float scaleY, float scaleZ, float transX, float transY, float transZ, matrix& matrixRotation, font3D* theFont);
	void								createButton						(plainButton& myButton, vector3 const & position, vector3 const & rotation, vector3 const & scale, texture& tex, function<void(guiElemEvFol*, void*)> userFunc);
	void								rotate								(faceAreaIndex newFaceAreaState, unsigned int newRollState);
	void								calcStateAfter90DegRot				();
	faceAreaRollIndex					findStateMatrix						(matrix const& toMatrix);
	bool								compareMatrices						(matrix const& a, matrix& b);
	float								getRotationAngle					(matrix& matRotFrom, matrix& matRotTo);

public:
										~rotationControlCube				();

	bool								create								(masterMind* ww, font3D* theFont);
	bool								linkToObject						(guiElement3D* elem, unsigned int matrixPosition);
	bool								linkToObject						(guiElemCluster3D* cluster, unsigned int matrixPosition);
	void								unlinkObject						();
	void								setColor							(color newColorNormalCenter, color newColorNormalAround, color newColorSelected);
	void								setPosition							(float x, float y, float z, bool relativeToView, bool updateMatrix);
};

} // namespace wildWeasel

