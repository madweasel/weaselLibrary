/*********************************************************************
	ww3D.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/

#include "wildWeasel.h"

#pragma region genericObject3D
list<wildWeasel::genericObject3D*					>	wildWeasel::genericObject3D::objectsToLoad;
list<wildWeasel::genericObject3D*					>	wildWeasel::genericObject3D::objectsToDraw;
list<wildWeasel::genericObject3D::gfx3D_spriteObject>	wildWeasel::genericObject3D::spritesToDraw;

//-----------------------------------------------------------------------------
// Name: ~genericObject3D()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::genericObject3D::~genericObject3D()
{
	objectsToLoad.remove(this);
	objectsToDraw.remove(this);
	removeSpriteToDraw();
}

//-----------------------------------------------------------------------------
// Name: isInListObjectsToDraw()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::genericObject3D::isInListObjectsToDraw()
{
	return (objectsToDraw.end() != find(objectsToDraw.begin(), objectsToDraw.end(), this));
}

//-----------------------------------------------------------------------------
// Name: isInListObjectsToLoad()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::genericObject3D::isInListObjectsToLoad()
{
	return (objectsToLoad.end() != find(objectsToLoad.begin(), objectsToLoad.end(), this));
}

//-----------------------------------------------------------------------------
// Name: addObjectToLoad()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::genericObject3D::addObjectToLoad(masterMind* ww)
{
	objectsToLoad.push_back(this);
	ww->graphicManager.setResourceUploadDirty();
}

//-----------------------------------------------------------------------------
// Name: removeObjectToLoad()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::genericObject3D::removeObjectToLoad()
{
	objectsToLoad.remove(this);
}

//-----------------------------------------------------------------------------
// Name: addObjectToDraw()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::genericObject3D::addObjectToDraw()
{
	objectsToDraw.push_back(this);
}

//-----------------------------------------------------------------------------
// Name: removeObjectToDraw()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::genericObject3D::removeObjectToDraw()
{
	objectsToDraw.remove(this);
}

//-----------------------------------------------------------------------------
// Name: addSpriteToDraw()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::genericObject3D::addSpriteToDraw(float z, realTextureItem* tex)
{
	const float deltaZ		= 0.001f;
	auto		curSprite	= spritesToDraw.begin();

	for (; curSprite != spritesToDraw.end(); curSprite++) {
		if (curSprite->zPosition > z + deltaZ || (curSprite->texture == tex && (curSprite->zPosition - deltaZ < z && z < curSprite->zPosition + deltaZ))) {
			break;
		}
	}

	spritesToDraw.insert(curSprite, gfx3D_spriteObject(this, z, tex));
}

//-----------------------------------------------------------------------------
// Name: removeSpriteToDraw()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::genericObject3D::removeSpriteToDraw()
{
	for (auto curSprite = spritesToDraw.begin(); curSprite != spritesToDraw.end(); curSprite++) {
		if ((*curSprite).object == this) {
			spritesToDraw.erase(curSprite);
			break;
		}
	}
}
#pragma endregion

/*******************************************************************************************/

#pragma region useFullFunctions
//-----------------------------------------------------------------------------
// Name: isGuiElemStateVisible()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::isGuiElemStateVisible(guiElemState& state)
{
	switch (state) 
	{
	case guiElemState::DRAWED:		return true;
	case guiElemState::GRAYED:		return true;
	case guiElemState::HIDDEN:		return false;
	case guiElemState::INVISIBLE:	return false;
	case guiElemState::UNUSED:		return false;
	case guiElemState::VISIBLE:		return true;
	default:						return false;
	}
}

//-----------------------------------------------------------------------------
// Name: isGuiElemStateActive()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::isGuiElemStateActive(guiElemState& state)
{
	switch (state) 
	{
	case guiElemState::DRAWED	:	return true;
	case guiElemState::GRAYED	:	return false;
	case guiElemState::HIDDEN	:	return false;
	case guiElemState::INVISIBLE:	return true;
	case guiElemState::UNUSED	:	return false;
	case guiElemState::VISIBLE	:	return false;
	default:						return false;
	}
}

//-----------------------------------------------------------------------------
// Name: string2wstring()
// Desc: 
//-----------------------------------------------------------------------------
wstring wildWeasel::string2wstring(const string& str)
{
	if (str.size()) {
		return {str.begin(), str.end()};
	} else {
		return wstring{L""};
	}
}

//-----------------------------------------------------------------------------
// Name: registerGuiElement()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::matrixScaleRotateTranslate(matrix* m, float sx, float sy, float sz, float rx, float ry, float rz, float tx, float ty, float tz)
{
	*m  = matrix::CreateScale		(sx, sy, sz);
	*m *= matrix::CreateRotationX	(rx);				// ... CreateFromYawPitchRoll
	*m *= matrix::CreateRotationY	(ry);			
	*m *= matrix::CreateRotationZ	(rz);			
	*m *= matrix::CreateTranslation	(tx, ty, tz);
}

//-----------------------------------------------------------------------------
// Name: matrixCreateFromTriad()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::matrix wildWeasel::matrixCreateFromTriad(vector3& vx, vector3& vy, vector3& vz)
{
	// normalize vectors for the triad
	vx.Normalize();
	vy.Normalize();
	vz.Normalize();

	// set normalized triad vectors as rotation matrix
	return matrix{	vx.x, vx.y, vx.z, 0, 
					vy.x, vy.y, vy.z, 0, 
					vz.x, vz.y, vz.z, 0, 
					0,    0,    0,    1};
}

//-----------------------------------------------------------------------------
// Name: solveSystemOfLinearEquations()
// Desc: Löst ein LGS mit 3 Gleichungen und 3 Unbekannten mithilfe der Cramerschen Regel
//-----------------------------------------------------------------------------
void wildWeasel::solveSystemOfLinearEquations(vector3 e, vector3 a, vector3 b, vector3 c, float* x, float* y, float* z)
{
	float Dx, Dy, Dz, D;

	D = a.x*b.y*c.z - a.x*b.z*c.y + a.y*b.z*c.x - a.y*b.x*c.z + a.z*b.x*c.y - a.z*b.y*c.x;
	Dx= e.x*b.y*c.z - e.x*b.z*c.y + e.y*b.z*c.x - e.y*b.x*c.z + e.z*b.x*c.y - e.z*b.y*c.x;
	Dy= a.x*e.y*c.z - a.x*e.z*c.y + a.y*e.z*c.x - a.y*e.x*c.z + a.z*e.x*c.y - a.z*e.y*c.x;
	Dz= a.x*b.y*e.z - a.x*b.z*e.y + a.y*b.z*e.x - a.y*b.x*e.z + a.z*b.x*e.y - a.z*b.y*e.x;

	*x = Dx / D;
	*y = Dy / D;
	*z = Dz / D;
}

//-----------------------------------------------------------------------------
// Name: determineRectangularBorder()
// Desc: Calculates the thickness of a surrounding border based on pixels having an alpha value smaller than 'alphaThreshold'. 
//-----------------------------------------------------------------------------
void wildWeasel::determineRectangularBorder(RECT &border, RGBQUAD* pQuad, UINT width, UINT height, BYTE alphaThreshold, RGBQUAD borderColor)
{
	// locals
	UINT	x, y, i;

	// top 
	for (y=0; y<height; y++) { for (x=0; x<width; x++) {	i = y*width + x;
	 	if (pQuad[i].rgbReserved > alphaThreshold || (pQuad[i].rgbRed == borderColor.rgbRed && pQuad[i].rgbGreen == borderColor.rgbGreen && pQuad[i].rgbBlue == borderColor.rgbBlue)) {
			border.top		= y; 
			x				= width;
			y				= height;
	}}}

	//  bottom 
	for (y=height-1; y>0; y--) { for (x=0; x<width; x++) {	i = y*width + x;
	 	if (pQuad[i].rgbReserved > alphaThreshold || (pQuad[i].rgbRed == borderColor.rgbRed && pQuad[i].rgbGreen == borderColor.rgbGreen && pQuad[i].rgbBlue == borderColor.rgbBlue)) {
			border.bottom	= height - y - 1;
			x				= width;
			y				= 1;
	}}}

	// left
	for (x=0; x<width; x++) {	for (y=0; y<height; y++) { i = y*width + x;
	 	if (pQuad[i].rgbReserved > alphaThreshold || (pQuad[i].rgbRed == borderColor.rgbRed && pQuad[i].rgbGreen == borderColor.rgbGreen && pQuad[i].rgbBlue == borderColor.rgbBlue)) {
			border.left		= x; 
			x				= width;
			y				= height;
	}}}

	// right 
	for (x=width-1; x>0; x--) {	for (y=0; y<height; y++) { i = y*width + x;
	 	if (pQuad[i].rgbReserved > alphaThreshold || (pQuad[i].rgbRed == borderColor.rgbRed && pQuad[i].rgbGreen == borderColor.rgbGreen && pQuad[i].rgbBlue == borderColor.rgbBlue)) {
			border.right	= width - x - 1; 
			x				= 1;
			y				= height;
	}}}

	/* remove pink border
	for (x=0; x<width; x++) {	for (y=0; y<height; y++) {
		i = y*width + x;
		int t = 100;
		if (pQuad[i].rgbRed >= borderColor.rgbRed - t && pQuad[i].rgbGreen <= borderColor.rgbGreen + t && pQuad[i].rgbBlue >= borderColor.rgbBlue - t) {
			pQuad[i].rgbRed = 0;
			pQuad[i].rgbGreen = 255;
			pQuad[i].rgbBlue = 0;
			pQuad[i].rgbReserved = 0;
		}
	}}*/
}

//-----------------------------------------------------------------------------
// Name: intersectsInverse()
// Desc: Uses the inverse matrix to solve the system of linear equations
//-----------------------------------------------------------------------------
bool wildWeasel::indexedRectContainer::intersectsInverse(const matrix& mat, const vector3& from, const vector3& to, float* distance, float* dotProduct)
{
//	// locals
//	float		i, j, k;
//	size_t		curRect;
//	vector3		RS = to - from;							// Richtungsvektor Strahl von Beobachter durch MouseCursor
//				RS.Normalize();
//	vector3		OB, RB1, RB2;							// Ortsvektor Button, Richtungsvektor Button, etc.
//	size_t		numRect		= rects   .size();
//	size_t		numVertices	= vertices.size();
//	matrix		matInv;
//
//	// first calc the inverse matrix without translation
//	OB		= mat.Translation();
//	mat._41	= mat._42 = mat._43 = 0;
//	matInv	= mat.Invert();
//	Vector3::Transform(from - OB, matInv, RB1);
//	Vector3::Transform(RS,		  matInv, RB2);
//
//	for (curRect=0; curRect<numRect; curRect++) {
//
//		// Prinziell kann über die inverse matrix von mat, die berechnung von polygonsTransformed gespart werden. Allerdings wäre zu prüfen ob sich das lohnt.
//		solveSystemOfLinearEquations(RB1 - vertices[rects[curRect].i1], RB2, vertices[rects[curRect].i2]-vertices[rects[curRect].i1], vertices[rects[curRect].i3]-vertices[rects[curRect].i1], &i, &j, &k);
//			
//		// does ray pass through the current polygon?
//		if ((j > 0) && (k > 0) && (j < 1) && (k < 1)) {
//			if (distance != NULL) (*distance) = abs(i);
//			return true;
//		}
//	}
	
	return false;
}

//-----------------------------------------------------------------------------
// Name: intersects()
// Desc: 
//		Parameterdarstellung der Ebene:		X = OB + j*RB1 + k*RB2
//		Parameterdarstellung der Gerade:	Y = from + i*(to - from)	= from + i*RS
//		Gleichsetzen:						X = Y
//											from - OB = -i*RS + j*RB1 + k*RB2
//-----------------------------------------------------------------------------
bool wildWeasel::indexedRectContainer::intersects(const matrix& mat, const vector3& from, const vector3& to, float* distance, float* dotProduct)
{
	// locals
	float		i, j, k;
	size_t		curRect, curVertex;
	vector3		RS = to - from;							// Richtungsvektor Strahl von Beobachter durch MouseCursor
				RS.Normalize();
	vector3		OB, RB1, RB2;							// Ortsvektor Button, Richtungsvektor Button, etc.
	size_t		numRect		= rects   .size();
	size_t		numVertices	= vertices.size();
	
	// first the polygons must be transformed according to the current matrix
	// ... dirtyBit hinzufügen
	for (curVertex=0; curVertex<numVertices; curVertex++) {
		vector3::Transform(vertices[curVertex], mat, verticesTransformed[curVertex]);
	}

	for (curRect=0; curRect<numRect; curRect++) {
		OB  = verticesTransformed[rects[curRect].i1];
		RB1 = verticesTransformed[rects[curRect].i2] - OB;
		RB2 = verticesTransformed[rects[curRect].i3] - OB;
		solveSystemOfLinearEquations(from - OB, RS, RB1, RB2, &i, &j, &k);
			
		// does ray pass through the current polygon?
		if ((j > 0) && (k > 0) && (j < 1) && (k < 1)) {
			if (distance != NULL) {
				(*distance) = -i;	// abs(i);
			}
			if (dotProduct != nullptr) {
				(*dotProduct) = (RB1.Cross(RB2)).Dot(RS);
			}
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
// Name: performClippingAndCheck()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::performClippingAndCheck(fRECT& destRect, fRECT& srcRect, const vector2& origin, const fRECT& clippingRect)
{
	// locals
	vector2 scale(((float) srcRect.right - srcRect.left) / (destRect.right - destRect.left), 
				  ((float) srcRect.bottom - srcRect.top) / (destRect.bottom - destRect.top));

	if (destRect.left		- origin.x < clippingRect.left   ) {
		srcRect.left	   += (clippingRect.left   + origin.x - destRect.left) * scale.x;
		destRect.left		= clippingRect.left   + origin.x ;
	}
	if (destRect.top		- origin.y < clippingRect.top    ) {
		srcRect.top		   += (clippingRect.top   + origin.y - destRect.top) * scale.y;
		destRect.top		= clippingRect.top  + origin.y ;
	}
	if (destRect.right		- origin.x > clippingRect.right  ) {
		srcRect.right	   += (clippingRect.right   + origin.x - destRect.right) * scale.x;
		destRect.right		= clippingRect.right   + origin.x ;
	}
	if (destRect.bottom		- origin.y > clippingRect.bottom ) {
		srcRect.bottom	   += (clippingRect.bottom   + origin.y - destRect.bottom) * scale.y;
		destRect.bottom		= clippingRect.bottom   + origin.y ;
	}

	return destRect.left < destRect.right && destRect.top < destRect.bottom;
}

//-----------------------------------------------------------------------------
// Name: performClippingAndCheck()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::performClippingAndCheck(fRECT& destRect, fRECT& srcRect, const vector2& origin, const fRECT& clippingRect, float rotation)
{
	// ... dirty bit benutzen um rechenzeit zu sparen	(geht nicht, da variablen nur temporär)
	// ... rotMat vlt. übergeben anstatt rotation um cos() und sin() nicht jedesmal berechnen zu müssen. (geht nicht da manchmal nur rotationswinkel vorhanden)

	// destRect, srcRect -> a,b,c,d -> a', b', c', d' -> destRect', srcRect'
	// a---b
	// |   |
	// c---d
	vector2 srcSize (srcRect .right - srcRect .left, srcRect .bottom - srcRect .top);
	vector2 destSize(destRect.right - destRect.left, destRect.bottom - destRect.top);
	matrix2 rotMat2D(rotation);
	vector2 a, b, c, d;
	vector2 v_ab, v_ba, v_ac, v_ca;
	float	f_ab(0), f_ba(0), f_ac(0), f_ca(0);

	// destRect, srcRect -> a,b,c,d
	a		= vector2(destRect.left,  destRect.top);			// unrotated and unshifted (by origin)
	b		= vector2(destRect.right, destRect.top);
	c		= vector2(destRect.left,  destRect.bottom);
	d		= vector2(destRect.right, destRect.bottom);

	b		= rotMat2D * (b - a - origin) + a;					// rotated around origin
	c		= rotMat2D * (c - a - origin) + a;
	d		= rotMat2D * (d - a - origin) + a;					// alternatively: b + c - a;
	a		= rotMat2D * (a - a - origin) + a;

	v_ab	= b - a;											// vectors representing the directions alongs the edges
	v_ba	= a - b;
	v_ac	= c - a;
	v_ca	= a - c;

	// a,b,c,d -> a', b', c', d' 
	if (abs(v_ab.x) > abs(v_ab.y)) {

		// move edges of destRect onto the borders of the clippingRect
		if (moveEdge(v_ab.x, a.x, c.x, clippingRect.left, clippingRect.right, f_ab)) { f_ab = clamp(f_ab, 0.0f, 1-f_ba);	a += v_ab * f_ab;	c += v_ab * f_ab; }
		if (moveEdge(v_ba.x, b.x, d.x, clippingRect.left, clippingRect.right, f_ba)) { f_ba = clamp(f_ba, 0.0f, 1-f_ab);	b += v_ba * f_ba;	d += v_ba * f_ba; }	
		if (moveEdge(v_ac.y, a.y, b.y, clippingRect.top, clippingRect.bottom, f_ac)) { f_ac = clamp(f_ac, 0.0f, 1-f_ca);	a += v_ac * f_ac;	b += v_ac * f_ac; }
		if (moveEdge(v_ca.y, c.y, d.y, clippingRect.top, clippingRect.bottom, f_ca)) { f_ca = clamp(f_ca, 0.0f, 1-f_ac);	c += v_ca * f_ca;	d += v_ca * f_ca; }

	} else {

		// move edges of destRect onto the borders of the clippingRect
		if (moveEdge(v_ab.y, a.y, c.y, clippingRect.top, clippingRect.bottom, f_ab)) { f_ab = clamp(f_ab, 0.0f, 1-f_ba);	a += v_ab * f_ab;	c += v_ab * f_ab; }
		if (moveEdge(v_ba.y, b.y, d.y, clippingRect.top, clippingRect.bottom, f_ba)) { f_ba = clamp(f_ba, 0.0f, 1-f_ab);	b += v_ba * f_ba;	d += v_ba * f_ba; }
		if (moveEdge(v_ac.x, a.x, b.x, clippingRect.left, clippingRect.right, f_ac)) { f_ac = clamp(f_ac, 0.0f, 1-f_ca);	a += v_ac * f_ac;	b += v_ac * f_ac; }
		if (moveEdge(v_ca.x, c.x, d.x, clippingRect.left, clippingRect.right, f_ca)) { f_ca = clamp(f_ca, 0.0f, 1-f_ac);	c += v_ca * f_ca;	d += v_ca * f_ca; }
	}

	// a', b', c', d' -> destRect', srcRect'
	srcRect.left	+= srcSize.x * f_ab;
	srcRect.right	-= srcSize.x * f_ba;
	srcRect.top		+= srcSize.y * f_ac;
	srcRect.bottom	-= srcSize.y * f_ca;
															 
	vector2 rotOrig	= rotMat2D * origin;
	destRect.left	= a.x + rotOrig.x;
	destRect.top	= a.y + rotOrig.y;
	destRect.right	= destRect.left + (b-a).Length();
	destRect.bottom	= destRect.top  + (c-a).Length();
	
	// if (f_ab == f_ba || f_ac == f_ca) return false;
	return true;
}

//-----------------------------------------------------------------------------
// Name: moveEdge()
// Desc: Helper function for performClippingAndCheck()
//-----------------------------------------------------------------------------
bool wildWeasel::moveEdge(float& v, float& p1, float& p2, const float& clipPositive, const float& clipNegative, float& f)
{
	// move corners of left edge of destRect onto the left border of the clippingRect
	if (v > 0) {
		if (p1 < p2) {
			if (p1 < clipPositive) {
				f	= (clipPositive - p1) / v;
				return true;
			}
		} else {
			if (p2 < clipPositive) {
				f	= (clipPositive - p2) / v;
				return true;
			}
		}
	// move corners of right edge of destRect onto the left border of the clippingRect
	} else {
		if (p1 > p2) {
			if (p1 > clipNegative) {
				f	= (clipNegative - p1) / v;
				return true;
			}
		} else {
			if (p2 > clipNegative) {
				f	= (clipNegative - p2) / v;
				return true;
			}
		}
	}
	return false;
}
#pragma endregion

/*******************************************************************************************/

#pragma region matrixChain
//-----------------------------------------------------------------------------
// Name: calcFinalMatrix()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::matrixChain::calcFinalMatrix()
{
	// locals
	size_t			i				= 0;
	size_t			maxNumMatrices	= matrices.size();
	matrixDirty *	curMatrix		= NULL;
	
	// VECTOR IMPLEMENTATION 
	// check all elements if there is any dirty matrix
	for (i=0; i<maxNumMatrices; i++) {

		curMatrix = &matrices[i];
		// ... Does not work when matrix is used by several elements at once.
		// -> Each guiElement must have exactly one dirtyBit. Not one for each matrix!
		if (true /*curMatrix != nullptr && *curMatrix->dirtyBit == true*/) {		

			// then recalc final matrix
			curMatrix = &matrices[0];
			*curMatrix->dirtyBit = false;
			finalMatrix = *curMatrix->mat;
			for (i=1; i<maxNumMatrices; i++) {
				curMatrix				= &matrices[i];
				*curMatrix->dirtyBit	= false;
				finalMatrix			   *= *curMatrix->mat;
			}

			// quit
			return true;
		}
	}

	return false;

	/* LIST IMPLEMENTATION
	// check all elements if there is any dirty matrix
	for (auto curMatrix : matrices) {

		if (curMatrix.dirtyBit) {

			// then recalc final matrix
			curMatrix.dirtyBit	= false;
			finalMatrix			= *curMatrix.mat;
			i					= 0;
			for (auto curMatrix : matrices) {
				i++; if (i==1) continue;
				curMatrix.dirtyBit = false;
				finalMatrix		  *= *curMatrix.mat;
			}

			// quit
			return;
		}
	}*/
}

//-----------------------------------------------------------------------------
// Name: getFinalMatrix()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::matrixChain::getFinalMatrix(matrix& mat)
{
	calcFinalMatrix();
	mat = finalMatrix;
}

//-----------------------------------------------------------------------------
// Name: insertMatrix()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::matrixChain::removeMatrix(matrix* matrixToRemove)
{
	// VECTOR IMPLEMENTATION 
	for (vector<matrixDirty>::iterator curMatrix = matrices.begin(); curMatrix != matrices.end(); curMatrix++) {
		if (matrixToRemove == (*curMatrix).mat) {
			matrices.erase(curMatrix);
			break;
		}
	}

	/* LIST IMPLEMENTATION
	for (list<matrixDirty>::iterator curMatrix = matrices.begin(); curMatrix != matrices.end(); curMatrix++) {
		if (matrixToRemove == (*curMatrix).mat) {
			matrices.erase(curMatrix);
			break;
		}
	}*/
}

//-----------------------------------------------------------------------------
// Name: insertMatrix()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::matrixChain::insertMatrix(unsigned int position, matrix* additionalMatrix, bool* additionalDirtyBit)
{
	// locals
	auto			itr				= matrices.begin();
	matrixDirty		newDirtyMatrix;
	unsigned int	i;

	newDirtyMatrix.dirtyBit	= additionalDirtyBit;
	newDirtyMatrix.mat		= additionalMatrix;

	for (i=0; i<position; i++) {
		if (i >= matrices.size()) break;
		itr++;
	}

	matrices.insert(itr, newDirtyMatrix);
}
#pragma endregion

/*******************************************************************************************/

#pragma region matrixControl2D
//-----------------------------------------------------------------------------
// Name: updateMatrix()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::matrixControl2D::recalcMatrix()
{
	matrixScaleRotateTranslate(&mat, scale.x, scale.y, 0, 0, 0, rotation, translation.x, translation.y, 0);
	dirtyBit = true;
}

//-----------------------------------------------------------------------------
// Name: setScaleRotPos()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::matrixControl2D::setScaleRotPos(float sx, float sy, float rz, float tx, float ty)
{
	matrixScaleRotateTranslate(&mat, sx, sy, 0, 0, 0, rz, tx, ty, 0);
	dirtyBit = true;
}

//-----------------------------------------------------------------------------
// Name: setScale()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::matrixControl2D::setScale(vector2 *newScale, bool updateMatrix)
{
	scale = *newScale;
	if (updateMatrix) recalcMatrix();
}

//-----------------------------------------------------------------------------
// Name: setScale()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::matrixControl2D::setScale(float x, float y, bool updateMatrix)
{
	scale.x = x;
	scale.y = y;
	if (updateMatrix) recalcMatrix();
}

//-----------------------------------------------------------------------------
// Name: setRotation()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::matrixControl2D::setRotation(float z, bool updateMatrix)
{
	rotation = z;
	if (updateMatrix) recalcMatrix();
}

//-----------------------------------------------------------------------------
// Name: setPosition()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::matrixControl2D::setPosition(vector2 *newPosition, bool updateMatrix)
{
	translation = *newPosition;
	if (updateMatrix) recalcMatrix();
}

//-----------------------------------------------------------------------------
// Name: setPosition()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::matrixControl2D::getPosition(vector2& position)
{
	if (mode == matControlMode::posRotSca) {
		position = this->translation;	
	} else if (mode == matControlMode::matrix) {
		vector3 vec(mat.Translation());
		position.x = vec.x;
		position.y = vec.y;
	}
}

//-----------------------------------------------------------------------------
// Name: getScale()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::matrixControl2D::getScale(vector2& scale)
{
	quaternion	rot;
	vector3		trans;
	vector3		scale3D;

	if (mode == matControlMode::posRotSca) {
		scale    = this->scale;
	} else if (mode == matControlMode::matrix) {
		mat.Decompose(scale3D, rot, trans);
		scale.x = scale3D.x;
		scale.y = scale3D.y;
	}
}

//-----------------------------------------------------------------------------
// Name: getRotation()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::matrixControl2D::getRotation(float& rotation)						
{ 
	vector3		scale;
	quaternion	rot;
	vector3		trans;

	if (mode == matControlMode::posRotSca) {
		rotation = this->rotation;	
	} else if (mode == matControlMode::matrix) {
		mat.Decompose(scale, rot, trans);
		// rotation = rot;
		// ...
	}
}

//-----------------------------------------------------------------------------
// Name: setPosition()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::matrixControl2D::setPosition(float x, float y, bool updateMatrix)
{
	translation.x	= x;
	translation.y	= y;
	if (updateMatrix) recalcMatrix();
}

//-----------------------------------------------------------------------------
// Name: setDirty()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::matrixControl2D::setDirty()
{
	dirtyBit	= true;
}

//-----------------------------------------------------------------------------
// Name: setMode()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::matrixControl2D::setPositioningMode(matControlMode newMode)
{
	mode		= newMode;
	dirtyBit	= true;

	if (mode==matControlMode::posRotSca) {
		recalcMatrix();
	}
}

//-----------------------------------------------------------------------------
// Name: setPosition()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::matrixControl2D::setMatrix(matrix* newMat, bool relative)
{
	if (relative) {
		mat *= *newMat;
	}
	else {
		mat = *newMat;
	}
	dirtyBit = true;
}
#pragma endregion

#pragma region matrixControl3D
//-----------------------------------------------------------------------------
// Name: updateMatrix()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::matrixControl3D::recalcMatrix()
{
	matrixScaleRotateTranslate(&mat, scale.x, scale.y, scale.z, rotation.x, rotation.y, rotation.z, position.x, position.y, position.z);
	dirtyBit = true;
}

//-----------------------------------------------------------------------------
// Name: setScaleRotPos()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::matrixControl3D::setScaleRotPos(float sx, float sy, float sz, float rx, float ry, float rz, float tx, float ty, float tz)
{
	matrixScaleRotateTranslate(&mat, sx, sy, sz, rx, ry, rz, tx, ty, tz);
	dirtyBit = true;
}

//-----------------------------------------------------------------------------
// Name: setScale()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::matrixControl3D::setScale(vector3 *newScale, bool relativeToView, bool updateMatrix)
{
	scale = *newScale;
	if (updateMatrix) recalcMatrix();
}

//-----------------------------------------------------------------------------
// Name: setScale()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::matrixControl3D::setScale(float x, float y, float z, bool relativeToView, bool updateMatrix)
{
	scale.x = x;
	scale.y = y;
	scale.z = z;
	if (updateMatrix) recalcMatrix();
}

//-----------------------------------------------------------------------------
// Name: setRotation()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::matrixControl3D::setRotation(float x, float y, float z, bool relativeToView, bool updateMatrix)
{
	rotation.x = x;
	rotation.y = y;
	rotation.z = z;
	if (updateMatrix) recalcMatrix();
}

//-----------------------------------------------------------------------------
// Name: setRotation()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::matrixControl3D::setRotation(vector3 *newRotation, bool relativeToView, bool updateMatrix)
{
	rotation = *newRotation;
	if (updateMatrix) recalcMatrix();
}

//-----------------------------------------------------------------------------
// Name: setPosition()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::matrixControl3D::setPosition(vector3 *newPosition, bool relativeToView, bool updateMatrix)
{
	position = *newPosition;
	if (updateMatrix) recalcMatrix();
}

//-----------------------------------------------------------------------------
// Name: setPosition()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::matrixControl3D::getPosition(vector3& position)
{
	if (mode == matControlMode::posRotSca) {
		position = this->position;	
	} else if (mode == matControlMode::matrix) {
		position = mat.Translation();
	}
}

//-----------------------------------------------------------------------------
// Name: getScale()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::matrixControl3D::getScale(vector3& scale)
{
	quaternion	rot;
	vector3		trans;

	if (mode == matControlMode::posRotSca) {
		scale    = this->scale;
	} else if (mode == matControlMode::matrix) {
		mat.Decompose(scale, rot, trans);
	}
}

//-----------------------------------------------------------------------------
// Name: getRotation()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::matrixControl3D::getRotation(vector3& rotation)						
{ 
	vector3		scale;
	quaternion	rot;
	vector3		trans;

	if (mode == matControlMode::posRotSca) {
		rotation = this->rotation;	
	} else if (mode == matControlMode::matrix) {
		mat.Decompose(scale, rot, trans);
		// rotation = rot;
		// ...
	}
}

//-----------------------------------------------------------------------------
// Name: setPosition()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::matrixControl3D::setPosition(float x, float y, float z, bool relativeToView, bool updateMatrix)
{
	position.x	= x;
	position.y	= y;
	position.z	= z;
	if (updateMatrix) recalcMatrix();
}

//-----------------------------------------------------------------------------
// Name: setDirty()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::matrixControl3D::setDirty()
{
	dirtyBit	= true;
}

//-----------------------------------------------------------------------------
// Name: setMode()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::matrixControl3D::setPositioningMode(matControlMode newMode)
{
	mode		= newMode;
	dirtyBit	= true;

	if (mode==matControlMode::posRotSca) {
		recalcMatrix();
	}
}

//-----------------------------------------------------------------------------
// Name: setView()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::matrixControl3D::setPointerToViewMatrix(matrix* newView)
{
	this->matView = newView;
}

//-----------------------------------------------------------------------------
// Name: setPosition()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::matrixControl3D::setMatrix(matrix* newMat, bool relative)
{
	if (relative) {
		mat *= *newMat;
	}
	else {
		mat = *newMat;
	}
	dirtyBit = true;
}
#pragma endregion

/*******************************************************************************************/

#pragma region texture

list<wildWeasel::realTextureItem*>	wildWeasel::realTextureItem::allTextures;

//-----------------------------------------------------------------------------
// Name: loadFile()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::realTextureItem* wildWeasel::realTextureItem::loadFile(masterMind* ww, const wstring& strFileName, bool removeInvisibleBorder)
{
	// look for existing texture
	realTextureItem* pAnotherTexture = realTextureItem::getRealTexture(strFileName);
	if (pAnotherTexture) {
		return realTextureItem::addLink(pAnotherTexture);
	}

	// does file exist?
	wstring fullFilePath;
	fullFilePath.assign(ww->texturesPath);
	fullFilePath.append(L"\\");
	fullFilePath.append(strFileName);
	if (!PathFileExists(fullFilePath.c_str())) {
		fullFilePath = L"Program stopped, since the following file could not be found:\n" + fullFilePath;
		ww->showMessageBox(L"ERROR", fullFilePath.c_str(), MB_OK);
		ww->exitProgram();
		while(true);
		return nullptr;
	}

	// locals
	realTextureItem* newItem = new realTextureItem();

	// add to global list
	allTextures.push_back(newItem);

	// make filename
	newItem->strFileName		= fullFilePath;
	newItem->removeBorder		= removeInvisibleBorder;
	newItem->ww					= ww;
	newItem->ssfTexture			= new ssf::textureResource();
	newItem->linkedToThis		= 0;

	// remember to load the file later on together with the other ressources
	newItem->addObjectToLoad(ww);

	return realTextureItem::addLink(newItem);
}

//-----------------------------------------------------------------------------
// Name: getRealTexture()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::realTextureItem* wildWeasel::realTextureItem::getRealTexture(const wstring& strFileName)
{
	// was texture already requested before?
	// ... search could be improved in speed
	for (auto& existingItem : allTextures) {
		if (strFileName == existingItem->strFileName.substr(existingItem->ww->texturesPath.size() + 1, MAX_PATH)) {
			return existingItem;
		}
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
// Name: loadFile()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::texture::loadFile(masterMind* ww, const wstring& strFileName, bool removeInvisibleBorder)
{
	// point to wildWeasel must be passed
	if (!ww) return false;

	// does this texture already point to a real texture item?
	if (realTexture) {

		// is the filename the same?
		wstring fullFilePath;
		fullFilePath.assign(ww->texturesPath);
		fullFilePath.append(L"\\");
		fullFilePath.append(strFileName);
		if (fullFilePath == realTexture->getFileName()) {
			return true;
		// if not, forget old texture link
		} else {
			realTextureItem::removeLink(realTexture);
		}
	}

	// look for existing texture and load if necessary
	realTexture = realTextureItem::loadFile(ww, strFileName, removeInvisibleBorder);

	return true;
}

//-----------------------------------------------------------------------------
// Name: createDeviceDependentResources()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::realTextureItem::createDeviceDependentResources(ssf::sharedVars& v)
{
	// do not load if file was deleted
	if (strFileName.size() == 0) return;

	if (!removeBorder) {
		ssfTexture->load(v, strFileName);
	} else {
		// remove invisible border around image
		wildWeasel::bitmap	myBitmap;
	
		// load image from file and decode
		myBitmap.loadFromFile(ww->wicFactory, strFileName);

		// remove nasty powerpoint border if alpha information is available
		myBitmap.removeNastyPowerpointBorder();

		// load texture from memory
		ssfTexture->load(v, myBitmap);
	}
}

//-----------------------------------------------------------------------------
// Name: onDeviceLost()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::realTextureItem::onDeviceLost()
{
	ssfTexture->onDeviceLost();
}

//-----------------------------------------------------------------------------
// Name: addLink()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::realTextureItem* wildWeasel::realTextureItem::addLink(wildWeasel::realTextureItem* pItem)
{
	pItem->linkedToThis++;
	return pItem;
}

//-----------------------------------------------------------------------------
// Name: removeLink()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::realTextureItem::removeLink(wildWeasel::realTextureItem* pItem)
{
	if (pItem == nullptr) return;

	pItem->linkedToThis--;
	
	if (pItem->linkedToThis == 0) {
		delete pItem;
	}
}

//-----------------------------------------------------------------------------
// Name: ~texture()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::texture::~texture()
{
	realTextureItem::removeLink(realTexture);
}

//-----------------------------------------------------------------------------
// Name: ~realTextureItem()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::realTextureItem::~realTextureItem()
{
	allTextures.remove(this);
	delete ssfTexture;

	ssfTexture				= nullptr;
	strFileName				= L"";
	ww						= nullptr;
	linkedToThis			= 0;
	uploaded				= false;
}

//-----------------------------------------------------------------------------
// Name: getSize_fRECT()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::fRECT wildWeasel::realTextureItem::getSize_fRECT()
{
	auto s = ssfTexture->getSize_vector2();
	return fRECT{0,0,s.x, s.y};
}

//-----------------------------------------------------------------------------
// Name: getSize_vector2()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::vector2 wildWeasel::realTextureItem::getSize_vector2()
{
	return ssfTexture->getSize_vector2();
}

#pragma endregion

/*****************************************************************************************/

#pragma region font2D
//-----------------------------------------------------------------------------
// Name: loadFontFile()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::font2D::loadFontFile(masterMind* ww, wstring& strFileName)
{
	// make filename
	this->strFileName.assign(ww->getTexturesPath());
	this->strFileName.append(L"\\");
	this->strFileName.append(strFileName);

	// does file exist?
	if (!PathFileExists(this->strFileName.c_str())) return false;

	// remember to load the file later on together with the other ressources
	addObjectToLoad(ww);
	return true;
}
#pragma endregion

/*****************************************************************************************/

#pragma region font3D
//-----------------------------------------------------------------------------
// Name: loadFontFile()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::font3D::loadFontFile(masterMind* ww, wstring& strFileName)
{
	substitute2D.loadFontFile(ww, strFileName);
	parent	= ww;
	return true;
}

//-----------------------------------------------------------------------------
// Name: font::drawString()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::font3D::drawString(wchar_t const* text, vector3 const& position, color theColor, vector2 const& scale, fRECT& clippingRect, const float lineSpacing, bool applyOffsetOnFirstCharacter)
{
	// locals
	vector2		scale2D;
	vector2		pos2D;
	vector2		origin;
	float		rotation;
	float		layerDepth;
	vector3		posTransformed;
	matrix		matTransform		= parent->getMatView() * parent->getMatProj();
	fRECT		textDrawBounds		= substitute2D.measureDrawBounds(text, vector2{0, 0});

	// consider view and projection matrix
	vector3::Transform(position, matTransform, posTransformed);

	// turn up-side down
	posTransformed.y *= -1;
	
	// calc pixel coordinates
	pos2D.x		= (posTransformed.x + 1.0f) * 0.5f * parent->getWindowSizeX() - 0.5f * (textDrawBounds.right  - textDrawBounds.left) * scale.x; 
	pos2D.y		= (posTransformed.y + 1.0f) * 0.5f * parent->getWindowSizeY() - 1.0f * (textDrawBounds.bottom - textDrawBounds.top ) * scale.y;
	scale2D		= scale;	
	origin.x	= 0;
	origin.y	= 0;
	rotation	= 0;
	layerDepth	= 0;

	// render
	substitute2D.draw(text, pos2D, theColor, rotation, origin, scale2D, layerDepth, &clippingRect, lineSpacing, applyOffsetOnFirstCharacter);
}
#pragma endregion

/*****************************************************************************************/

#pragma region loadingScreen
//-----------------------------------------------------------------------------
// Name: loadingScreen constructor()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::loadingScreen::loadingScreen(masterMind* ww) 
{
	this->ww = ww;
}

//-----------------------------------------------------------------------------
// Name: show()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::loadingScreen::show(void loadFunc(void* pUser), void* pUser)
{
	// ready?
	if (!ww->gfxDevice.isInitialized()) return;

	// show window
	active = true;

	// load ressources needed for loading screen
	ww->graphicManager.performResourceUpload();

	// show window
	ShowWindow(ww->getHwnd(), SW_SHOWNORMAL);
	UpdateWindow(ww->getHwnd());	

	// threading (when thread has finished mainLoadingScreen.active should be set to false. this is done by the owner calling setLoadingCompletionFraction(1);)
	thread (loadFunc, pUser).detach();
}

//-----------------------------------------------------------------------------
// Name: getMeasuredFractions()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::loadingScreen::getMeasuredFractions(vector<float>& frac)
{
	// normalize
	if (fractions.back() >= 1) {
		float startingOffset	= fractions.front();
		float maxValue			= fractions.back () - startingOffset;
		for (auto& curFrac : fractions) {
			curFrac -= startingOffset;
			curFrac /= maxValue;
		}
	}

	frac				= move(fractions);
	measuringFractions	= true;
	fractionItr			= fractions.begin();
}

//-----------------------------------------------------------------------------
// Name: isActive()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::loadingScreen::setMeasuredFractions(vector<float>& frac)
{
	fractions			= move(frac);
	measuringFractions	= false;
	fractionItr			= fractions.begin();
}

//-----------------------------------------------------------------------------
// Name: setLoadingScreenTextures()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::loadingScreen::setTextures(texture* texBackground, texture* texSymbol, font2D* textFont)
{
	if (texSymbol) rotatingSprite = new ssf::sprite();
	background		= texBackground;
	rotatingTexture	= texSymbol;
	font			= textFont;
	return true;
}

//-----------------------------------------------------------------------------
// Name: progress()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::loadingScreen::progress()
{
	// measure if not done yet
	if (measuringFractions) {
		fractions.push_back((float) chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now().time_since_epoch()).count());
	} else if (!progressFinished) {
		if (fractionItr != fractions.end()) {
			setCompletionFraction(*fractionItr);
			fractionItr++;
		} else {
			progressFinished = true;
		}
	}
}


//-----------------------------------------------------------------------------
// Name: setCompletionFraction()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::loadingScreen::setCompletionFraction(float fraction)
{
	wstringstream wss;
	wss << setprecision(0) << fraction*100 << L" %";
	text				= wss.str();
	completionFraction	= fraction;

	// is loading complete?
	if (fraction >= 1) {
		active = false;
		progressFinished = true;
	}
}

//-----------------------------------------------------------------------------
// Name: loadingScreen::init()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::loadingScreen::init(ssf::shape::genericShape* rotatingShape, POINT& windowSize)
{
	this->rotatingShape		= rotatingShape;
	this->windowSize		= &windowSize;
	this->text				= L"0 %";
	this->progressFinished	= false;
}

//-----------------------------------------------------------------------------
// Name: loadingScreen::createDeviceDependentResources()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::loadingScreen::createDeviceDependentResources(ssf::sharedVars & v)
{
	if (rotatingShape ) rotatingShape ->createDeviceDependentResources(v);
	if (rotatingSprite) rotatingSprite->createDeviceDependentResources(v);
}

//-----------------------------------------------------------------------------
// Name: loadingScreen::createWindowSizeDependentResources()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::loadingScreen::createWindowSizeDependentResources(matrix& matProjection)
{
	//if (rotatingShape && rotatingTexture->getTextureResource()) rotatingShape->setTexture(*rotatingTexture);
	if (rotatingShape ) rotatingShape ->createWindowSizeDependentResources(matProjection);
	//if (rotatingSprite) rotatingSprite->createWindowSizeDependentResources(matProjection);
}

//-----------------------------------------------------------------------------
// Name: loadingScreen::onDeviceLost()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::loadingScreen::onDeviceLost()
{
	if (rotatingShape ) rotatingShape ->onDeviceLost();
	//if (rotatingSprite) rotatingSprite->onDeviceLost();
}

//-----------------------------------------------------------------------------
// Name: loadingScreen::update()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::loadingScreen::update(matrix& matView, tickCounter const& timer)
{
	// do nothing if not active
	if (!active) return;

	// update matrices for rotation
	rotationAngle = rotationSpeed * (float) timer.GetTotalSeconds();
	
	matrix matCube;
	matCube  = matrix::CreateScale(30);
	matCube *= matrix::CreateRotationX(0.5f);
	matCube *= matrix::CreateRotationY(0.3f);
	matCube *= matrix::CreateRotationZ(rotationAngle);
	matCube *= matrix::CreateTranslation(0, 0, -100);
	
	if (rotatingShape) {
		rotatingShape->setColor(color{255, 183, 72});
		rotatingShape->setWorld(matCube);
		rotatingShape->setView(matrix::Identity);
	}
	if (rotatingSprite) {
	//	rotatingSprite->update(matView, timer);
	}
}

//-----------------------------------------------------------------------------
// Name: loadingScreen::render()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::loadingScreen::render(ssf::sharedVars& v)
{
	// do nothing if not active
	if (!active) return;

	if (rotatingShape) {
		rotatingShape->draw();
	}
}

//-----------------------------------------------------------------------------
// Name: loadingScreen::renderSprites()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::loadingScreen::renderSprites(ssf::sharedVars& v)
{
	// do nothing if not active
	if (!active) return;
   
	// locals
	fRECT					symbolRect;
	vector2					textPos;
	textPos.x				= windowSize->x * 0.8f;	// upper left edge
	textPos.y				= windowSize->y * 0.8f;
	symbolRect.left			= windowSize->x * 0.5f;	// upper left edge
	symbolRect.right		= windowSize->x * 0.7f;
	symbolRect.top			= windowSize->y * 0.5f;
	symbolRect.bottom		= windowSize->y * 0.7f;

	// write completion fraction
	if (font) {
		font->draw(text.c_str(), textPos, color::white);
	}

	// rotating symbol
	if (rotatingSprite && rotatingTexture) {
		auto texSize = rotatingTexture->getSize() / 2.0f;
		// rotatingSprite->draw(*rotatingTexture, symbolRect, nullptr, color::white, rotationAngle, texSize, 0.1f);
	}
}
#pragma endregion

/*****************************************************************************************/

#pragma region camera

wildWeasel::camera			wildWeasel::camera::defaultCamera;

//-----------------------------------------------------------------------------
// Name: create()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::camera::create(masterMind* ww)
{
	this->ww = ww;
}

//-----------------------------------------------------------------------------
// Name: windowSizeChanged()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::camera::windowSizeChanged(int x, int y)
{
	aspectRatio		= (float) x / y;
	fieldOfViewY	= 2*atan(tan(fieldOfViewX/2) / aspectRatio);
}

//-----------------------------------------------------------------------------
// Name: rotate()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::camera::rotate(float Yaw, float Pitch, float Roll)
{
// 	// Achtung hat BUG bei extremen Yaw Werten.
// 	matrix  mat;
// 	vector3 vecLookingDirection = LookAt - Position;
// 
// 	if (WorldUpIsY) {
// 		WorldUp.x		= 0.0f;
// 		WorldUp.y		= 1.0f;
// 		WorldUp.z		= 0.0f;
// 	} else {
// 		// WorldUp um LookAt drehen
// 		D3DXMatrixRotationAxis(&mat, &vecLookingDirection, Roll);
// 		D3DXVec3TransformCoord(&WorldUp, &WorldUp, &mat);
// 	}
// 
// 	// vecLookingDirection um WorldUp drehen
// 	D3DXMatrixRotationAxis(&mat, &WorldUp, Yaw);
// 	D3DXVec3TransformCoord(&vecLookingDirection, &vecLookingDirection, &mat);
// 	
// 	// MoveRight = vecLookingDirection x WorldUp
// 	D3DXVec3Cross(&MoveRight, &vecLookingDirection, &WorldUp);
// 
// 	// LookAt um MoveRight drehen
// 	D3DXMatrixRotationAxis(&mat, &MoveRight, Pitch);
// 	D3DXVec3TransformCoord(&vecLookingDirection, &vecLookingDirection, &mat);
// 	
// 	// Neues LookAt berechnen
// 	LookAt = vecLookingDirection + Position;
}

//-----------------------------------------------------------------------------
// Name: makeCameraFromViewMatrix()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::camera::makeCameraFromViewMatrix(matrix& mat, camera& cam)
{
//	matrix mat;
//	D3DXMatrixInverse(&mat, NULL, mat);
//	D3DXVec3TransformCoord(&WorldUp,   &vector3(0,1,0), &mat);
//	D3DXVec3TransformCoord(&LookAt,	  &vector3(0,0,1), &mat);
//	D3DXVec3TransformCoord(&MoveRight, &vector3(1,0,0), &mat);
//	D3DXVec3TransformCoord(&Position,  &vector3(0,0,0), &mat);
//	WorldUpIsY	= false;
}

//-----------------------------------------------------------------------------
// Name: makeViewMatrixFromCamera()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::camera::makeViewMatrixFromCamera(matrix& mat)
{
	mat = matrix::CreateLookAt(position, lookAt, worldUp);
}

//-----------------------------------------------------------------------------
// Name: makeProjMatrixFromCamera()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::camera::makeProjMatrixFromCamera(matrix& mat)
{
	mat = matrix::CreatePerspectiveFieldOfView(fieldOfViewY, aspectRatio, zNear, zFar);
}

//-----------------------------------------------------------------------------
// Name: setPosition()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::camera::setPosition(vector3 newPosition)
{
	position	= newPosition;
	
	moveForward	= lookAt - position;				
	moveForward.Normalize();
	moveRight	= moveForward.Cross(worldUp);		
	worldUp		= moveRight	 .Cross(moveForward);	
}

//-----------------------------------------------------------------------------
// Name: setLookAt()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::camera::setLookAt(vector3 newLookAt)
{
	lookAt		= newLookAt;

	moveForward	= lookAt - position;				
	moveForward.Normalize();
	moveRight	= moveForward.Cross(worldUp);		
	worldUp		= moveRight	 .Cross(moveForward);	
}

//-----------------------------------------------------------------------------
// Name: setAsActiveCamera()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::camera::setAsActiveCamera()
{
	if (ww == nullptr) return;
	ww->setActiveCamera(*this);
	windowSizeChanged(ww->getWindowSizeX(), ww->getWindowSizeY());
	setFieldOfViewX(fieldOfViewX);
}

//-----------------------------------------------------------------------------
// Name: setFieldOfViewX()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::camera::setFieldOfViewX(float newFieldOfViewX)
{
	if (ww == nullptr) return;
	fieldOfViewX = clamp(newFieldOfViewX, minFieldOfViewX, maxFieldOfViewX);
	ww->gfxDevice.CreateWindowSizeDependentResources();
}

//-----------------------------------------------------------------------------
// Name: isFaceVisible()
// Desc: checks whether the vector{0,0,1} is  pointing to the camera or not
//-----------------------------------------------------------------------------
bool wildWeasel::camera::isFaceVisible(matrix& matObject)
{
	vector3 objPos		= matObject.Translation();
	vector3 vecZ		= vector3::Transform({0,0,1}, matObject) - objPos;
	vector3	vecObjToCam	= position - objPos;
	return vecObjToCam.Dot(vecZ) < 0;
}

#pragma endregion

/*****************************************************************************************/

#pragma region screenInformation
//-----------------------------------------------------------------------------
// Name: screenInformation()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::screenInformation::screenInformation(masterMind* ww)
{
	this->ww = ww;
}

//-----------------------------------------------------------------------------
// Name: showFramesperSecond()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::screenInformation::showFramesPerSecond(bool show, font2D* textFont)
{
	font	= textFont;
	showFPS	= show;
}

//-----------------------------------------------------------------------------
// Name: renderSprites()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::screenInformation::renderSprites(ssf::sharedVars& v)
{
	// show frames per second
	if (showFPS) {
		wstringstream wss;		wss.str(L"");
		wss << ww->stepTimer.GetFramesPerSecond() << L" fps";
		font->draw(wss.str().c_str(), vector2(100, 10),color::white);
	}
}

#pragma endregion

/*****************************************************************************************/

#pragma region cursorStruct

//-----------------------------------------------------------------------------
// Name: cursorClass2D()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::cursorClass2D::cursorClass2D(masterMind* ww)
{
	this->ww = ww;
}

//-----------------------------------------------------------------------------
// Name: setCursor()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::cursorClass2D::setCursor(cursorType newType)
{
	auto hWnd = ww->getHwnd();

	// SetCursor() does not work, since normal arrow is not restored when entering main window from outside
	switch (newType) 
	{
	case cursorType::ARROW		: SetClassLongPtr(hWnd, GCLP_HCURSOR, (LONG_PTR) LoadCursor(NULL, IDC_ARROW		)); break;
	case cursorType::CROSS		: SetClassLongPtr(hWnd, GCLP_HCURSOR, (LONG_PTR) LoadCursor(NULL, IDC_CROSS		)); break;
	case cursorType::HAND		: SetClassLongPtr(hWnd, GCLP_HCURSOR, (LONG_PTR) LoadCursor(NULL, IDC_HAND		)); break;
	case cursorType::HELP		: SetClassLongPtr(hWnd, GCLP_HCURSOR, (LONG_PTR) LoadCursor(NULL, IDC_HELP		)); break;
	case cursorType::IBEAM		: SetClassLongPtr(hWnd, GCLP_HCURSOR, (LONG_PTR) LoadCursor(NULL, IDC_IBEAM		)); break;
	case cursorType::NO			: SetClassLongPtr(hWnd, GCLP_HCURSOR, (LONG_PTR) LoadCursor(NULL, IDC_NO		)); break;
	case cursorType::SIZEALL	: SetClassLongPtr(hWnd, GCLP_HCURSOR, (LONG_PTR) LoadCursor(NULL, IDC_SIZEALL	)); break;
	case cursorType::SIZENESW	: SetClassLongPtr(hWnd, GCLP_HCURSOR, (LONG_PTR) LoadCursor(NULL, IDC_SIZENESW	)); break;
	case cursorType::SIZENS		: SetClassLongPtr(hWnd, GCLP_HCURSOR, (LONG_PTR) LoadCursor(NULL, IDC_SIZENS	)); break;
	case cursorType::SIZENWSE	: SetClassLongPtr(hWnd, GCLP_HCURSOR, (LONG_PTR) LoadCursor(NULL, IDC_SIZENWSE	)); break;
	case cursorType::SIZEWE		: SetClassLongPtr(hWnd, GCLP_HCURSOR, (LONG_PTR) LoadCursor(NULL, IDC_SIZEWE	)); break;
	case cursorType::UPARROW	: SetClassLongPtr(hWnd, GCLP_HCURSOR, (LONG_PTR) LoadCursor(NULL, IDC_UPARROW	)); break;
	case cursorType::WAIT		: SetClassLongPtr(hWnd, GCLP_HCURSOR, (LONG_PTR) LoadCursor(NULL, IDC_WAIT		)); break;
	}
}

//-----------------------------------------------------------------------------
// Name: calcCursorPos()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::cursorClass3D::calcCursorPos(vector3& cursorPosInSpace, camera& theCamera, LONG xPos, LONG yPos)
{
	// params ok?
	if (!windowSize) return;

	// this calculation assumes a flat rectangle area in front of the camera on which the cursor is moved
	// another areas on which the cursor moves could be considered
	// theCamera->moveRight
	vector2 posInsideArea	= {(float) (xPos) / (windowSize->x), (float) (yPos) / (windowSize->y)};
	vector2 areaSize		= {2 * distanceToCamera * tan(theCamera.fieldOfViewX/2), 2 * distanceToCamera * tan(theCamera.fieldOfViewY/2)};
	vector3 camLookDir		= {theCamera.lookAt - theCamera.position}; 
			camLookDir.Normalize();
	vector3 areaX			= {theCamera.moveRight * areaSize.x * +1};
	vector3 areaY			= {theCamera.worldUp   * areaSize.y * -1};	// -1 since xPos=0, yPos=0 is the top left corner in the window/area
	vector3 areaCenter		= {theCamera.position + (camLookDir * distanceToCamera)};
	vector3 areaOrigin		= areaCenter - areaX * 0.5f - areaY * 0.5f;
	cursorPosInSpace		= areaX * posInsideArea.x + areaY * posInsideArea.y + areaOrigin;

	// float cursorPlaneZ			= theCamera->position.z - distanceToCamera;
	// float xScale				= tan(theCamera->fieldOfViewX/2);
	// float yScale				= xScale / theCamera->aspectRatio;
	// float widthOfCursorPlane	= 2 * abs(distanceToCamera) * xScale;
	// float minCursorPosInSpaceX	=  widthOfCursorPlane/2 * ((distanceToCamera)>0 ? -1:  1);
	// float maxCursorPosInSpaceX	=  widthOfCursorPlane/2 * ((distanceToCamera)>0 ?  1: -1);
	// float minCursorPosInSpaceY	=  widthOfCursorPlane/2/theCamera->aspectRatio;
	// float maxCursorPosInSpaceY	= -widthOfCursorPlane/2/theCamera->aspectRatio;
	// cursorPosInSpace->x			= ((float) (xPos) / (windowSize->x)) * (maxCursorPosInSpaceX - minCursorPosInSpaceX) + minCursorPosInSpaceX + cursorPlaneZ * theCamera->perspectiveTranslation.x * xScale;
	// cursorPosInSpace->y			= ((float) (yPos) / (windowSize->y)) * (maxCursorPosInSpaceY - minCursorPosInSpaceY) + minCursorPosInSpaceY + cursorPlaneZ * theCamera->perspectiveTranslation.y * yScale;
	// cursorPosInSpace->z			= cursorPlaneZ;
}

/*-----------------------------------------------------------------------------
// Name: isCursorOverButton()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::guiElemEvFol3D* wildWeasel::isCursorOverButton()
{
	// ready?
	if (!cursor3D.initialized)	return nullptr;

	// locals
	bool			isOverButton;
	float			distanceFromCamera;
	float			bestDist = 0;
	guiElemEvFol3D*		resElem = NULL;

	// check all relevant gui elements
	for (auto curElem : guiElements) {
		if ((curElem->status == guiElemState::DRAWED) || (curElem->status == guiElemState::INVISIBLE)) {
			isOverButton = curElem->isCursorOver(cursor2D.x, cursor2D.y, cameraPosition, cursor3D.position, &distanceFromCamera);
			if (isOverButton && ((resElem == NULL) || (abs(distanceFromCamera) < bestDist)))	{ 
				resElem  = curElem; 
				bestDist = distanceFromCamera;
			}
		}
	}

	// return result
	return resElem;
}*/

/*-----------------------------------------------------------------------------
// Name: setCursorPosition()
// Desc: When no view matrix is passed, then the global coordinate system is used.
//		 Otherwise vec(0,0,10) would be ten units in front of the camera position.
//-----------------------------------------------------------------------------
void wildWeasel::cursorClass3D::setCursorPosition(bool relative, vector3 vec)
{
	// ready?
	if (!matView)		return;
	if (!initialized)	return;

	// Locals
	vector3 tmpVec;
	matrix  mat		= matView->Invert();

	// transform point by matrix
	vector3::Transform(vec, mat, tmpVec);

	// Drehe Mauscursor
	matrixScaleRotateTranslate(&cursorMatrix, size, size, size, 0, 0, PI/4, 0, 0, 0);
	cursorMatrix *= mat;

	// Bewege Cursor
	if (relative)	pos += (tmpVec *= speed);
	else			pos  = tmpVec;

	cursorMatrix._41 = pos.x;
	cursorMatrix._42 = pos.y;
	cursorMatrix._43 = pos.z;
}*/

//-----------------------------------------------------------------------------
// Name: SetCursorSize()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::cursorClass3D::setCursorVisibility(bool visible)
{
	isVisible	= visible;
}

//-----------------------------------------------------------------------------
// Name: SetCursorSpeed()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::cursorClass3D::setCursorSpeed(float newSpeed)
{
	speed	= newSpeed;
}

//-----------------------------------------------------------------------------
// Name: SetCursorSpeed()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::cursorClass3D::setDistanceToCamera(float newDistance)
{
	distanceToCamera = newDistance;
}

//-----------------------------------------------------------------------------
// Name: SetCursorSpeed()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::cursorClass3D::setControlMode(cursorControlMode newMode)
{
	controlMode	= newMode;
}

//-----------------------------------------------------------------------------
// Name: initCursor()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::cursorClass3D::init(masterMind* ww)
{
	if (initialized) return;

	windowSize			= &ww->windowSize;
	matView				= &ww->matView;
	rotation			= vector3(0, 0, wwc::PI/4);

	addObjectToLoad(ww);

	// Set Initial Position and Size
	initialized = true;
}

//-----------------------------------------------------------------------------
// Name: update()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::cursorClass3D::update(matrix& matView, tickCounter const& timer)
{
	gfx_cone	.setView(matView);
	gfx_cylinder.setView(matView);
	gfx_cone	.setColor(mainColor);
	gfx_cylinder.setColor(mainColor);
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::cursorClass3D::render(ssf::sharedVars& v)
{
	if (!initialized || !isVisible) return;

	matrix matWorld;
	
	matWorld  = matrix::CreateTranslation(0,-0.5f,0);
	matWorld *= mat;
	gfx_cone.setWorld(matWorld);
	gfx_cone.draw();

	matWorld  = matrix::CreateScale(0.3f, 1.0f, 0.3f);
	matWorld *= matrix::CreateTranslation(0,-1.5f,0);
	matWorld *= mat;
	gfx_cylinder.setWorld(matWorld);
	gfx_cylinder.draw();
}

//-----------------------------------------------------------------------------
// Name: createDeviceDependentResources()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::cursorClass3D::createDeviceDependentResources(ssf::sharedVars& v)
{
	gfx_cone    .createDeviceDependentResources(v);
	gfx_cylinder.createDeviceDependentResources(v);
}

//-----------------------------------------------------------------------------
// Name: createWindowSizeDependentResources()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::cursorClass3D::createWindowSizeDependentResources(matrix& matProjection)
{
	gfx_cone	.setProj(matProjection);
	gfx_cylinder.setProj(matProjection);
}

//-----------------------------------------------------------------------------
// Name: onDeviceLost()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::cursorClass3D::onDeviceLost()
{
	gfx_cone	.onDeviceLost();
	gfx_cylinder.onDeviceLost();
}
#pragma endregion

/*****************************************************************************************/

#pragma region bitmap

// Source code in the region is derived from http://www.antillia.com/sol9.2.0/classes/WICBitmapFileReader.tmp.html

/******************************************************************************
 *
 * Copyright (c) 2015 Antillia.com TOSHIYUKI ARAI. ALL RIGHTS RESERVED.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions, and the following disclaimer.
 *  
 * 2. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR 
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/

// get the dxgi format equivilent of a wic format
DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID)
{
         if (wicFormatGUID == GUID_WICPixelFormat128bppRGBAFloat	) return DXGI_FORMAT_R32G32B32A32_FLOAT;
    else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBAHalf		) return DXGI_FORMAT_R16G16B16A16_FLOAT;
    else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBA			) return DXGI_FORMAT_R16G16B16A16_UNORM;
    else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBA			) return DXGI_FORMAT_R8G8B8A8_UNORM;
    else if (wicFormatGUID == GUID_WICPixelFormat32bppBGRA			) return DXGI_FORMAT_B8G8R8A8_UNORM;
    else if (wicFormatGUID == GUID_WICPixelFormat32bppBGR			) return DXGI_FORMAT_B8G8R8X8_UNORM;
    else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBA1010102XR	) return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;
    else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBA1010102	) return DXGI_FORMAT_R10G10B10A2_UNORM;
    else if (wicFormatGUID == GUID_WICPixelFormat16bppBGRA5551		) return DXGI_FORMAT_B5G5R5A1_UNORM;
    else if (wicFormatGUID == GUID_WICPixelFormat16bppBGR565		) return DXGI_FORMAT_B5G6R5_UNORM;
    else if (wicFormatGUID == GUID_WICPixelFormat32bppGrayFloat		) return DXGI_FORMAT_R32_FLOAT;
    else if (wicFormatGUID == GUID_WICPixelFormat16bppGrayHalf		) return DXGI_FORMAT_R16_FLOAT;
    else if (wicFormatGUID == GUID_WICPixelFormat16bppGray			) return DXGI_FORMAT_R16_UNORM;
    else if (wicFormatGUID == GUID_WICPixelFormat8bppGray			) return DXGI_FORMAT_R8_UNORM;
    else if (wicFormatGUID == GUID_WICPixelFormat8bppAlpha			) return DXGI_FORMAT_A8_UNORM;
    else															  return DXGI_FORMAT_UNKNOWN;
}

// get the number of bits per pixel for a dxgi format
int GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat)
{
         if (dxgiFormat == DXGI_FORMAT_R32G32B32A32_FLOAT)			return 128;
    else if (dxgiFormat == DXGI_FORMAT_R16G16B16A16_FLOAT)			return 64;
    else if (dxgiFormat == DXGI_FORMAT_R16G16B16A16_UNORM)			return 64;
    else if (dxgiFormat == DXGI_FORMAT_R8G8B8A8_UNORM)				return 32;
    else if (dxgiFormat == DXGI_FORMAT_B8G8R8A8_UNORM)				return 32;
    else if (dxgiFormat == DXGI_FORMAT_B8G8R8X8_UNORM)				return 32;
    else if (dxgiFormat == DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM)	return 32;
    else if (dxgiFormat == DXGI_FORMAT_R10G10B10A2_UNORM)			return 32;
    else if (dxgiFormat == DXGI_FORMAT_B5G5R5A1_UNORM)				return 16;
    else if (dxgiFormat == DXGI_FORMAT_B5G6R5_UNORM)				return 16;
    else if (dxgiFormat == DXGI_FORMAT_R32_FLOAT)					return 32;
    else if (dxgiFormat == DXGI_FORMAT_R16_FLOAT)					return 16;
    else if (dxgiFormat == DXGI_FORMAT_R16_UNORM)					return 16;
    else if (dxgiFormat == DXGI_FORMAT_R8_UNORM)					return 8;
    else if (dxgiFormat == DXGI_FORMAT_A8_UNORM)					return 8;
	else															return 0;
}

// get a dxgi compatible wic format from another wic format
WICPixelFormatGUID GetConvertToWICFormat(WICPixelFormatGUID& wicFormatGUID)
{
         if (wicFormatGUID == GUID_WICPixelFormatBlackWhite				) return GUID_WICPixelFormat8bppGray;
    else if (wicFormatGUID == GUID_WICPixelFormat1bppIndexed			) return GUID_WICPixelFormat32bppRGBA;
    else if (wicFormatGUID == GUID_WICPixelFormat2bppIndexed			) return GUID_WICPixelFormat32bppRGBA;
    else if (wicFormatGUID == GUID_WICPixelFormat4bppIndexed			) return GUID_WICPixelFormat32bppRGBA;
    else if (wicFormatGUID == GUID_WICPixelFormat8bppIndexed			) return GUID_WICPixelFormat32bppRGBA;
    else if (wicFormatGUID == GUID_WICPixelFormat2bppGray				) return GUID_WICPixelFormat8bppGray;
    else if (wicFormatGUID == GUID_WICPixelFormat4bppGray				) return GUID_WICPixelFormat8bppGray;
    else if (wicFormatGUID == GUID_WICPixelFormat16bppGrayFixedPoint	) return GUID_WICPixelFormat16bppGrayHalf;
    else if (wicFormatGUID == GUID_WICPixelFormat32bppGrayFixedPoint	) return GUID_WICPixelFormat32bppGrayFloat;
    else if (wicFormatGUID == GUID_WICPixelFormat16bppBGR555			) return GUID_WICPixelFormat16bppBGRA5551;
    else if (wicFormatGUID == GUID_WICPixelFormat32bppBGR101010			) return GUID_WICPixelFormat32bppRGBA1010102;
    else if (wicFormatGUID == GUID_WICPixelFormat24bppBGR				) return GUID_WICPixelFormat32bppRGBA;
    else if (wicFormatGUID == GUID_WICPixelFormat24bppRGB				) return GUID_WICPixelFormat32bppRGBA;
    else if (wicFormatGUID == GUID_WICPixelFormat32bppPBGRA				) return GUID_WICPixelFormat32bppRGBA;
    else if (wicFormatGUID == GUID_WICPixelFormat32bppPRGBA				) return GUID_WICPixelFormat32bppRGBA;
    else if (wicFormatGUID == GUID_WICPixelFormat48bppRGB				) return GUID_WICPixelFormat64bppRGBA;
    else if (wicFormatGUID == GUID_WICPixelFormat48bppBGR				) return GUID_WICPixelFormat64bppRGBA;
    else if (wicFormatGUID == GUID_WICPixelFormat64bppBGRA				) return GUID_WICPixelFormat64bppRGBA;
    else if (wicFormatGUID == GUID_WICPixelFormat64bppPRGBA				) return GUID_WICPixelFormat64bppRGBA;
    else if (wicFormatGUID == GUID_WICPixelFormat64bppPBGRA				) return GUID_WICPixelFormat64bppRGBA;
    else if (wicFormatGUID == GUID_WICPixelFormat48bppRGBFixedPoint		) return GUID_WICPixelFormat64bppRGBAHalf;
    else if (wicFormatGUID == GUID_WICPixelFormat48bppBGRFixedPoint		) return GUID_WICPixelFormat64bppRGBAHalf;
    else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBAFixedPoint	) return GUID_WICPixelFormat64bppRGBAHalf;
    else if (wicFormatGUID == GUID_WICPixelFormat64bppBGRAFixedPoint	) return GUID_WICPixelFormat64bppRGBAHalf;
    else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBFixedPoint		) return GUID_WICPixelFormat64bppRGBAHalf;
    else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBHalf			) return GUID_WICPixelFormat64bppRGBAHalf;
    else if (wicFormatGUID == GUID_WICPixelFormat48bppRGBHalf			) return GUID_WICPixelFormat64bppRGBAHalf;
    else if (wicFormatGUID == GUID_WICPixelFormat128bppPRGBAFloat		) return GUID_WICPixelFormat128bppRGBAFloat;
    else if (wicFormatGUID == GUID_WICPixelFormat128bppRGBFloat			) return GUID_WICPixelFormat128bppRGBAFloat;
    else if (wicFormatGUID == GUID_WICPixelFormat128bppRGBAFixedPoint	) return GUID_WICPixelFormat128bppRGBAFloat;
    else if (wicFormatGUID == GUID_WICPixelFormat128bppRGBFixedPoint	) return GUID_WICPixelFormat128bppRGBAFloat;
    else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBE				) return GUID_WICPixelFormat128bppRGBAFloat;
    else if (wicFormatGUID == GUID_WICPixelFormat32bppCMYK				) return GUID_WICPixelFormat32bppRGBA;
    else if (wicFormatGUID == GUID_WICPixelFormat64bppCMYK				) return GUID_WICPixelFormat64bppRGBA;
    else if (wicFormatGUID == GUID_WICPixelFormat40bppCMYKAlpha			) return GUID_WICPixelFormat64bppRGBA;
    else if (wicFormatGUID == GUID_WICPixelFormat80bppCMYKAlpha			) return GUID_WICPixelFormat64bppRGBA;
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8) || defined(_WIN7_PLATFORM_UPDATE)
    else if (wicFormatGUID == GUID_WICPixelFormat32bppRGB				) return GUID_WICPixelFormat32bppRGBA;
    else if (wicFormatGUID == GUID_WICPixelFormat64bppRGB				) return GUID_WICPixelFormat64bppRGBA;
    else if (wicFormatGUID == GUID_WICPixelFormat64bppPRGBAHalf			) return GUID_WICPixelFormat64bppRGBAHalf;
#endif
    else																  return GUID_WICPixelFormatDontCare;
}

// End of Source code region derived from http://www.antillia.com/sol9.2.0/classes/WICBitmapFileReader.tmp.html

//-----------------------------------------------------------------------------
// Name: getTotalSizeInBytes()
// Desc: 
//-----------------------------------------------------------------------------
size_t wildWeasel::bitmap::getTotalSizeInBytes()
{
	return 	sizeof(bmih) + 	sizeof(bmfh) + bmih.bV4BitCount / 8 * bmih.bV4Width * abs(bmih.bV4Height);
}

//-----------------------------------------------------------------------------
// Name: getHeaderSizeInBytes()
// Desc: 
//-----------------------------------------------------------------------------
size_t wildWeasel::bitmap::getHeaderSizeInBytes()
{
	return 	sizeof(bmih) + 	sizeof(bmfh);
}

//-----------------------------------------------------------------------------
// Name: getDataSizeInBytes()
// Desc: 
//-----------------------------------------------------------------------------
size_t wildWeasel::bitmap::getDataSizeInBytes()
{
	return 	bmih.bV4BitCount / 8 * bmih.bV4Width * abs(bmih.bV4Height);
}

//-----------------------------------------------------------------------------
// Name: bitmap()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::bitmap::~bitmap()
{
	if (pBitmap) delete pBitmap;
	pBitmap = nullptr;
}

//-----------------------------------------------------------------------------
// Name: loadOneImageFromFile()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::bitmap::writeToFile(wstring &filename)
{
	// locals
	HANDLE	hFile;
	DWORD	bytesWritten;

	// Datei erstellen
	hFile = CreateFile(filename.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	
	// Erfolgreich geöffnet ?
	if (hFile == INVALID_HANDLE_VALUE)	return false;

	// write
	WriteFile(hFile, pBitmap, (DWORD) getTotalSizeInBytes(), &bytesWritten, NULL);

	// 
	CloseHandle(hFile);
	return 	true;
}

//-----------------------------------------------------------------------------
// Name: loadOneImageFromFile()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::bitmap::loadFromFile(IWICImagingFactory* wicFactory, wstring &filename)
{
	// --------------------------
	// Possible Libraries
	// --------------------------
	// - OpenCV:	cv::imread();
	// - DirectX:	D3DXCreateTextureFromFileEx();
	// - WIC:		Windows Imaging Component
	// - GDI+:		Old fashioned

	// locals
	IWICBitmapDecoder *		wicDecoder			= NULL; 
	IWICBitmapFrameDecode *	wicFrame 			= NULL;	
	IWICFormatConverter *	wicConverter		= NULL;
	WICPixelFormatGUID		wicPixelFormat;
	HRESULT					hr;
	UINT					width, height;
	UINT					nStride, nImage, bytesPerPixel;
	bool					imageConverted		= false;

	// does file exist?
	if (!PathFileExists(filename.c_str())) {
		return false;
	}

	// Create a decoder
	hr = wicFactory->CreateDecoderFromFilename(filename.c_str(), NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &wicDecoder);
	
	// Retrieve the first frame of the image from the decoder
	if (SUCCEEDED(hr)) {
		hr = wicDecoder->GetFrame(0, &wicFrame );
	} else {
		return NULL;
	}

	// copy image properties
	wicFrame ->GetSize(&width, &height);
	wicFrame ->GetPixelFormat(&wicPixelFormat);

	// convert wic pixel format to dxgi pixel format
    DXGI_FORMAT dxgiFormat = GetDXGIFormatFromWICFormat(wicPixelFormat);

    // if the format of the image is not a supported dxgi format, try to convert it
    if (dxgiFormat == DXGI_FORMAT_UNKNOWN)
    {
        // get a dxgi compatible wic format from the current image format
        WICPixelFormatGUID convertToPixelFormat = GetConvertToWICFormat(wicPixelFormat);

        // return if no dxgi compatible format was found
        if (convertToPixelFormat == GUID_WICPixelFormatDontCare) return 0;

        // set the dxgi format
        dxgiFormat = GetDXGIFormatFromWICFormat(convertToPixelFormat);
            
        // create the format converter
        hr = wicFactory->CreateFormatConverter(&wicConverter);
        if (FAILED(hr)) return 0;

        // make sure we can convert to the dxgi compatible format
        BOOL canConvert = FALSE;
        hr = wicConverter->CanConvert(wicPixelFormat, convertToPixelFormat, &canConvert);
        if (FAILED(hr) || !canConvert) return 0;

        // do the conversion (wicConverter will contain the converted image)
        hr = wicConverter->Initialize(wicFrame, convertToPixelFormat, WICBitmapDitherTypeNone, NULL, 0.f, WICBitmapPaletteTypeCustom);
        if (FAILED(hr)) return 0;

        // this is so we know to get the image data from the wicConverter (otherwise we will get from wicFrame)
        imageConverted = true;
    }

	// Create Info Header
	bmih.bV4Size			= sizeof(bmih);
	bmih.bV4Width			= width;
	bmih.bV4Height			= -1 * (LONG) height;
	bmih.bV4Planes			= 1;
	bmih.bV4BitCount		= GetDXGIFormatBitsPerPixel(dxgiFormat); // number of bits per pixel
	bmih.bV4V4Compression	= BI_BITFIELDS;
	bmih.bV4SizeImage		= bmih.bV4BitCount / 8 * bmih.bV4Width * abs(bmih.bV4Height);
	bmih.bV4XPelsPerMeter	= 1000;
	bmih.bV4YPelsPerMeter	= 1000;
	bmih.bV4ClrUsed			= 0;
	bmih.bV4ClrImportant	= 0;
	bmih.bV4AlphaMask		= 0xff000000;
	bmih.bV4RedMask			= 0x00ff0000;
	bmih.bV4GreenMask		= 0x0000ff00;
	bmih.bV4BlueMask		= 0x000000ff;
	bmih.bV4CSType			= 0;

	// Create File Header
	bmfh.bfType				= 0x4D42;
	bmfh.bfSize				= (DWORD) getTotalSizeInBytes();
	bmfh.bfReserved1		= 0;
	bmfh.bfReserved2		= 0;
	bmfh.bfOffBits			= sizeof(bmfh) + sizeof(bmih);

	// create bitmap in memory & Copy the pixels to the DIB section
	bytesPerPixel	= bmih.bV4BitCount / 8;						// byters per pixel
	nStride			= bytesPerPixel * width;					// bytes per line
	nImage			= width * height * bytesPerPixel;			// total number of bytes

	if (pBitmap) delete pBitmap;
	pBitmap			= new BYTE[getTotalSizeInBytes()];
	pData			= pBitmap + getHeaderSizeInBytes();

	// fill bitmap array with header
	memcpy(pBitmap,					&bmfh, sizeof(bmfh));
	memcpy(pBitmap + sizeof(bmfh),  &bmih, sizeof(bmih));

	// copy (decoded) raw image data into the newly allocated memory (imageData)
    if (imageConverted) {
        // if image format needed to be converted, the wic converter will contain the converted image
        hr = wicConverter ->CopyPixels(NULL, nStride, nImage, reinterpret_cast<BYTE*>(pData));		// ->CopyPixels(0, bytesPerRow, imageSize, *imageData);
		wicConverter->Release();
        if (FAILED(hr)) return 0;
    } else {
        // no need to convert, just copy data from the wic frame
		hr = wicFrame->CopyPixels(NULL, nStride, nImage, reinterpret_cast<BYTE*>(pData));			// ->CopyPixels(0, bytesPerRow, imageSize, *imageData);
        if (FAILED(hr)) return 0;
    }

	// close image
	wicFrame->Release();
	wicDecoder->Release();

	return true;
}

//-----------------------------------------------------------------------------
// Name: removeNastyPowerpointBorder()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::bitmap::removeNastyPowerpointBorder()
{
	// works only for bitmaps with alpha information
	if (bmih.bV4BitCount != 32) return false;

	// locals
	int					x, y, i, j;
	RECT				border;
	bitmap				bmpWithoutBorder;
	COLORREF*			pDataWithoutBorder;
	COLORREF*			pDataWithBorder			= (COLORREF*) pData;
	RGBQUAD				borderColor				= RGBQUAD{255,0,255,0};
	RGBQUAD*			curPixel				= nullptr;
	int					boderColorThreshold		= 100;
	LONG				distanceToBorder		= 5;

	// determine borders (RGBQUAD != COLORREF)
	wildWeasel::determineRectangularBorder(border, (RGBQUAD*) pDataWithBorder, bmih.bV4Width, abs(bmih.bV4Height), 10, borderColor);

	// create smaller bitmap
	bmpWithoutBorder.bmfh				= bmfh;
	bmpWithoutBorder.bmih				= bmih;
	bmpWithoutBorder.bmih.bV4Width	   -=  border.left   + border.right;
	bmpWithoutBorder.bmih.bV4Height	   -= (border.bottom + border.top) * (bmih.bV4Height>0 ? 1 : -1);
	bmpWithoutBorder.bmih.bV4SizeImage	= (DWORD) bmpWithoutBorder.getDataSizeInBytes();
	bmpWithoutBorder.bmfh.bfSize		= (DWORD) bmpWithoutBorder.getTotalSizeInBytes();
	
	bmpWithoutBorder.pBitmap			= new BYTE[bmpWithoutBorder.getTotalSizeInBytes()];
	bmpWithoutBorder.pData				= bmpWithoutBorder.pBitmap + bmpWithoutBorder.getHeaderSizeInBytes();
	memcpy(bmpWithoutBorder.pBitmap,				 &bmpWithoutBorder.bmfh, sizeof(bmpWithoutBorder.bmfh));
	memcpy(bmpWithoutBorder.pBitmap + sizeof(bmfh),  &bmpWithoutBorder.bmih, sizeof(bmpWithoutBorder.bmih));

	pDataWithoutBorder	= (COLORREF*) bmpWithoutBorder.pData;

	// copy relevant pixels
	for (i=0, j=0, y=0; y<abs(bmih.bV4Height); y++) {
		for (x=0; x<bmih.bV4Width; x++, i++) {
			if (x>=border.left && x<bmih.bV4Width-border.right && y>=border.top && y<abs(bmih.bV4Height)-border.bottom) {
				pDataWithoutBorder[j] = pDataWithBorder[i];
				curPixel = (RGBQUAD*) &pDataWithoutBorder[j];
				j++;

				// make border invible
				// ... assumes borderColor				= RGBQUAD{255,0,255,0}
				if (x - border.left < distanceToBorder || bmih.bV4Width - border.right - x < distanceToBorder ||  y - border.top < distanceToBorder || abs(bmih.bV4Height) - border.bottom - y < distanceToBorder) {
					if (curPixel->rgbRed >= borderColor.rgbRed - boderColorThreshold && curPixel->rgbGreen <= borderColor.rgbGreen + boderColorThreshold && curPixel->rgbBlue >= borderColor.rgbBlue - boderColorThreshold) {
						curPixel->rgbReserved = 0;
					}
				}
			}
		}
	}

	// delete old bitmap and take new one
	delete pBitmap;
	bmfh	= bmpWithoutBorder.bmfh;
	bmih	= bmpWithoutBorder.bmih;
	pData	= bmpWithoutBorder.pData;
	pBitmap = bmpWithoutBorder.pBitmap;

	// prevent calling the destructor
	bmpWithoutBorder.pBitmap	= nullptr;

	return true;
}
#pragma endregion

/*****************************************************************************************/

#pragma region grid3D
//-----------------------------------------------------------------------------
// Name: create()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::grid3D::create(masterMind* ww)
{
		addObjectToLoad(ww);
}

//-----------------------------------------------------------------------------
// Name: draw3Grids()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::grid3D::draw3Grids()
{
	// Draw procedurally generated dynamic grid
	size_t	numSteps	= 10;
	float	stepSize	= 10.0f;
	const vector3 xaxis  = { stepSize*numSteps, 0.f, 0.f };
	const vector3 yaxis  = { 0.f, stepSize*numSteps, 0.f };
	const vector3 zaxis  = { 0.f, 0.f, stepSize*numSteps };
	const vector3 origin = { 0.f, 0.f, 0.0f };
	lines.drawGrid(xaxis, yaxis, origin, 2*numSteps, 2*numSteps, color::red);
	lines.drawGrid(yaxis, zaxis, origin, 2*numSteps, 2*numSteps, color::green);
	lines.drawGrid(zaxis, xaxis, origin, 2*numSteps, 2*numSteps, color::blue);
}
#pragma endregion

/*****************************************************************************************/

#pragma region resourceManager

//-----------------------------------------------------------------------------
// Name: setResourceUploadDirty()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::resourceManager::setResourceUploadDirty()
{
	resourceUploadIsDirty = true;
}

//-----------------------------------------------------------------------------
// Name: setAutomaticResourceUpload()
// Desc: Option for the user to decide wether the resource upload shlal occur automatically before the next frame render.
//-----------------------------------------------------------------------------
void wildWeasel::resourceManager::setAutomaticResourceUpload(bool automaticUpload)
{
	autoResourceUpload = automaticUpload;
}

//-----------------------------------------------------------------------------
// Name: destroy()
// Desc: These are the resources that depend on the device.
//-----------------------------------------------------------------------------
bool wildWeasel::resourceManager::performResourceUpload()
{
	// ready?
	if (!ww->gfxDevice.isInitialized()) return false;

	resourceUploadMutex.lock();
	performingRessourceUpload = true;
	   
	// locals
	auto originalObjectsToLoad	= genericObject3D::objectsToLoad;

	// process all objects
	for (auto& curObject : originalObjectsToLoad) {
		if (curObject == nullptr) continue;
		curObject->createDeviceDependentResources(ww->gfxDevice.sharedVars);
		curObject->uploaded = true;
		curObject->removeObjectToLoad();
		curObject->addObjectToDraw();
		ww->mainLoadingScreen.progress();
	}

	// process all objects again and perform CreateWindowSizeDependentResources
	for (auto& curObject : genericObject3D::objectsToDraw) {
		curObject->createWindowSizeDependentResources(ww->matProjection);
		ww->mainLoadingScreen.progress();
	}

	performingRessourceUpload	= false;
	resourceUploadIsDirty		= false;
	resourceUploadMutex.unlock();
	return true;
}

//-----------------------------------------------------------------------------
// Name: uploadIfNecessary()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::resourceManager::uploadIfNecessary()
{
	// perform resource upload if necessary
	if (resourceUploadIsDirty && autoResourceUpload && !performingRessourceUpload) {
		performResourceUpload();
	}
}

#pragma endregion

/*****************************************************************************************/
