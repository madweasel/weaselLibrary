/*********************************************************************
	wwAlignment.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/

#include "wildWeasel.h"		// when "wwAlignment.h" is included than it is included at last and the times it is listed in the "wildWeasel.h" it is skipped.

#pragma region relationships
#pragma region edgeRelation

//-----------------------------------------------------------------------------
// Name: edgeRelation::newRelation()
// Desc: returns a pointer to specific sub-class of a relation. so the deletion of the passed relation can be controlled by the owner an the used relation for an edge is controlled by the edges
//-----------------------------------------------------------------------------
wildWeasel::alignment::edgeRelation* wildWeasel::alignment::edgeRelation::newRelation(const edgeRelation& source)
{
	        if (typeid(source) == typeid(relDistance   )) {	return new relDistance		{dynamic_cast<const relDistance&>	(source)};
	} else 	if (typeid(source) == typeid(relFraction   )) {	return new relFraction		{dynamic_cast<const relFraction&>	(source)};
	} else 	if (typeid(source) == typeid(relFractionAdd)) {	return new relFractionAdd	{dynamic_cast<const relFractionAdd&>(source)};
	} else {												return nullptr; }
}

//-----------------------------------------------------------------------------
// Name: edgeRelation::~edgeRelation()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::alignment::edgeRelation::~edgeRelation()
{
	// remove this relation from all leading edges
	for (auto& curSourceEdge : leadingEdges) {
		if (*curSourceEdge) (*curSourceEdge)->removeFollower(*this);
	}

	// inform following edge, that its relation is going to be deleted
	if (followingEdge) {
		followingEdge->relationIsBeingDeleted(*this);
	}
}

//-----------------------------------------------------------------------------
// Name: edgeRelation::informDetermingEdge()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignment::edgeRelation::informFollowingEdge()
{
	if (followingEdge) {
		followingEdge->setPosition(calcPosition());
	}
}

//-----------------------------------------------------------------------------
// Name: edgeRelation::enrollInLeadingEdges()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignment::edgeRelation::enrollInLeadingEdges()
{
	// inform base class edgeRelation about leading edges
	if (leadingEdges.size() == 0) {
		fillLeadingEdges();
	}

	// enroll
	for (auto& curSourceEdge : leadingEdges) {
		if (*curSourceEdge) (*curSourceEdge)->addFollower(*this);
	}
}

//-----------------------------------------------------------------------------
// Name: edgeRelation::informDetermingEdge()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignment::edgeRelation::setFollowingEdge(edge& newFollower)
{
	if (followingEdge == &newFollower) return;
	followingEdge = &newFollower;
}

//-----------------------------------------------------------------------------
// Name: edgeRelation::setEdge()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignment::edgeRelation::setLeadingEdge(edge* &edgeToSet, edge& newEdge)
{
	if (&newEdge == edgeToSet) return;
	if (edgeToSet) {
		edgeToSet->removeFollower(*this);
	}
	edgeToSet = &newEdge;
	edgeToSet->addFollower(*this);
	informFollowingEdge();
}
#pragma endregion
#pragma region relFraction
//-----------------------------------------------------------------------------
// Name: relFraction::relFraction()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::alignment::relFraction::relFraction(edge& from, edge& to, float fraction) :
	edgeFrom	{&from		},
	edgeTo		{&to		},
	fraction	{fraction	}
{
}

//-----------------------------------------------------------------------------
// Name: relFraction::fillLeadingEdges()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignment::relFraction::fillLeadingEdges()
{
	leadingEdges.push_back(&edgeFrom);
	leadingEdges.push_back(&edgeTo  );
}

//-----------------------------------------------------------------------------
// Name: relFraction::calcPosition()
// Desc: 
//-----------------------------------------------------------------------------
float wildWeasel::alignment::relFraction::calcPosition()
{
	return (edgeTo->getPosition() - edgeFrom->getPosition()) * fraction + edgeFrom->getPosition();
}

//-----------------------------------------------------------------------------
// Name: relFraction::setPosition()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignment::relFraction::setPosition(float pixelPos) 
{
	setFraction((pixelPos - edgeFrom->getPosition()) / (edgeTo->getPosition() - edgeFrom->getPosition()));
}

//-----------------------------------------------------------------------------
// Name: relFraction::setFraction()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignment::relFraction::setFraction(float newFraction)
{
	if (abs(fraction - newFraction) < 0.001f) return;
	fraction = newFraction;
	informFollowingEdge();
}

//-----------------------------------------------------------------------------
// Name: relFraction::setFromEdge()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignment::relFraction::setFromEdge(edge& newFromEdge)
{
	setLeadingEdge(edgeFrom, newFromEdge);
}

//-----------------------------------------------------------------------------
// Name: relFraction::setToEdge()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignment::relFraction::setToEdge(edge& newToEdge)
{
	setLeadingEdge(edgeTo, newToEdge);
}

//-----------------------------------------------------------------------------
// Name: relFraction::getDistanceToRefEdge()
// Desc: 
//-----------------------------------------------------------------------------
float wildWeasel::alignment::relFraction::getDistanceToRefEdge()
{
	return fraction * (edgeTo->getPosition() - edgeFrom->getPosition());
}
#pragma endregion
#pragma region relDistance
//-----------------------------------------------------------------------------
// Name: relDistance::relDistance()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::alignment::relDistance::relDistance(edge& reference, float distance) :
	edgeReference	{ &reference} ,
	distance		{ distance	}
{
}

//-----------------------------------------------------------------------------
// Name: relDistance::fillLeadingEdges()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignment::relDistance::fillLeadingEdges()
{
	leadingEdges.push_back(&edgeReference);
}

//-----------------------------------------------------------------------------
// Name: relDistance::getPosition()
// Desc: 
//-----------------------------------------------------------------------------
float wildWeasel::alignment::relDistance::calcPosition()
{
	return edgeReference->getPosition() + distance;
}

//-----------------------------------------------------------------------------
// Name: relDistance::setDistance()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignment::relDistance::setDistance(float newDistance)
{
	if (abs(distance - newDistance) < 0.001f) return;
	distance = newDistance;
	informFollowingEdge();
}

//-----------------------------------------------------------------------------
// Name: relDistance::setPosition()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignment::relDistance::setPosition(float pixelPos) 
{
	setDistance(pixelPos - edgeReference->getPosition());
}

//-----------------------------------------------------------------------------
// Name: relDistance::setReferenceEdge()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignment::relDistance::setReferenceEdge(edge& newReferenceEdge)
{
	setLeadingEdge(edgeReference, newReferenceEdge);
}

//-----------------------------------------------------------------------------
// Name: relDistance::getDistanceToRefEdge()
// Desc: 
//-----------------------------------------------------------------------------
float wildWeasel::alignment::relDistance::getDistanceToRefEdge()
{
	return distance;
}
#pragma endregion
#pragma region relFractionAdd
//-----------------------------------------------------------------------------
// Name: relFractionAdd::relFractionAdd()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::alignment::relFractionAdd::relFractionAdd(edge& from, edge& to, edge& reference, float fraction) : 
	edgeFrom		{ &from		} ,
	edgeTo			{ &to		} ,
	edgeReference	{ &reference} ,
	fraction		{ fraction	} 
{
}

//-----------------------------------------------------------------------------
// Name: relFractionAdd::fillLeadingEdges()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignment::relFractionAdd::fillLeadingEdges()
{
	leadingEdges.push_back(&edgeFrom		);
	leadingEdges.push_back(&edgeTo			);
	leadingEdges.push_back(&edgeReference	);
}

//-----------------------------------------------------------------------------
// Name: relFractionAdd::getPosition()
// Desc: 
//-----------------------------------------------------------------------------
float wildWeasel::alignment::relFractionAdd::calcPosition()
{
	return edgeReference->getPosition() + (edgeTo->getPosition() - edgeFrom->getPosition()) * fraction;
}

//-----------------------------------------------------------------------------
// Name: relDistance::setPosition()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignment::relFractionAdd::setPosition(float pixelPos) 
{
	setFraction((pixelPos - edgeReference->getPosition()) / (edgeTo->getPosition() - edgeFrom->getPosition()));
}

//-----------------------------------------------------------------------------
// Name: relFractionAdd::setFraction()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignment::relFractionAdd::setFraction(float newFraction)
{
	if (abs(fraction - newFraction) < 0.001f) return;
	fraction = newFraction;
	informFollowingEdge();
}

//-----------------------------------------------------------------------------
// Name: relFractionAdd::setFromEdge()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignment::relFractionAdd::setReferenceEdge(edge& newReferenceEdge)
{
	setLeadingEdge(edgeReference, newReferenceEdge);
}

//-----------------------------------------------------------------------------
// Name: relFractionAdd::setFromEdge()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignment::relFractionAdd::setFromEdge(edge& newFromEdge)
{
	setLeadingEdge(edgeFrom, newFromEdge);
}

//-----------------------------------------------------------------------------
// Name: relFractionAdd::setToEdge()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignment::relFractionAdd::setToEdge(edge& newToEdge)
{
	setLeadingEdge(edgeTo, newToEdge);
}

//-----------------------------------------------------------------------------
// Name: relFractionAdd::getDistanceToRefEdge()
// Desc: 
//-----------------------------------------------------------------------------
float wildWeasel::alignment::relFractionAdd::getDistanceToRefEdge()
{
	return fraction * (edgeTo->getPosition() - edgeFrom->getPosition());
}
#pragma endregion
#pragma endregion

#pragma region edge
//-----------------------------------------------------------------------------
// Name: edge::addFollower()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignment::edge::addFollower(edgeRelation& newFollower)
{
	// do not change followingRelations array, when it is deleted anyway
	if (edgeIsBeingDeleted) return;

	// don't add followingRelations twice
	auto position = find(followingRelations.begin(), followingRelations.end(), &newFollower);

	if (position != followingRelations.end()) {
		while (true);
	}

	followingRelations.push_back(&newFollower);
}

//-----------------------------------------------------------------------------
// Name: edge::removeFollower()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignment::edge::removeFollower(edgeRelation& formerFollower)
{
	// do not change followingRelations array, when it is deleted anyway
	if (edgeIsBeingDeleted) return;
	if (followingRelations.size() == 0) return;

	auto position = find(followingRelations.begin(), followingRelations.end(), &formerFollower);

	if (position != followingRelations.end()) {
		followingRelations.erase(position);
	} else {
		// BUG: Should never occur, but it does!!!
		printf("WARNING: edge::removeFollower: follower not found in followingRelations!\n");
	}
}

//-----------------------------------------------------------------------------
// Name: edge::relationIsBeingDeleted()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignment::edge::relationIsBeingDeleted(edgeRelation& rel)
{
	if (&rel != leadingRelation) {
		while(true);
	}

	leadingRelation = nullptr;
}

//-----------------------------------------------------------------------------
// Name: edge::edge()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::alignment::edge::edge()
{
}

//-----------------------------------------------------------------------------
// Name: edge::edge()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::alignment::edge::edge(const edge& copyFrom) :
	edgeIsBeingDeleted					{ copyFrom.edgeIsBeingDeleted } ,
	position							{ copyFrom.position }
{
	if (copyFrom.leadingRelation) {
		setRelation(*copyFrom.leadingRelation);
	}
}

//-----------------------------------------------------------------------------
// Name: edge::~edge()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::alignment::edge::~edge()
{
	// do not change followingRelations array any more, since it is deleted anyway
	edgeIsBeingDeleted = true;

	// relations having contact to edges must be deleted
	if (leadingRelation) {
		delete leadingRelation;
		leadingRelation = nullptr;
	}

	// relations having contact to edges must be deleted
	for (auto& curFollowingRelation : followingRelations) {
		if (curFollowingRelation) {
			delete curFollowingRelation;
		}
	}
}

//-----------------------------------------------------------------------------
// Name: edge::move()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignment::edge::move(float distanceInPixels)
{
	setPosition(position + distanceInPixels);
}

//-----------------------------------------------------------------------------
// Name: edge::setRelation()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignment::edge::setRelation(const edgeRelation& newEdgeRelation)
{
	if (leadingRelation) delete leadingRelation;

	leadingRelation = edgeRelation::newRelation(newEdgeRelation);

	float newPosition;

	if (leadingRelation)	{
		newPosition	= leadingRelation->calcPosition();
		leadingRelation->setFollowingEdge(*this);
		leadingRelation->enrollInLeadingEdges();
	} else {
		newPosition	= 0;
	}

	setPosition(newPosition);
}

//-----------------------------------------------------------------------------
// Name: edge::setRelationBasedOnDistance()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignment::edge::setRelationBasedOnDistance(edge& reference, float distance)
{
	setRelation(wildWeasel::alignment::relDistance{reference, distance});
}

//-----------------------------------------------------------------------------
// Name: alignment::setInsideAnotherEdges()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignment::edge::setInsideAnotherEdges(edge& from, edge& to, edge& oldFrom, edge& oldTo)
{
	if (auto myRelation = getRelation<relDistance>()) {
		if (myRelation->getEdgeReference() == &oldFrom) { 
			myRelation->setReferenceEdge(from);
		} else if (myRelation->getEdgeReference() == &oldTo) {
			myRelation->setReferenceEdge(to);
		} else {
			// do nothing, since the edge is than not dependant on the old frame, but on other edges
		}
	}  else if (auto myRelation = getRelation<relFraction>()) {
		myRelation->setFromEdge	(from);
		myRelation->setToEdge	(to);
	}  else if (auto myRelation = getRelation<relFractionAdd>()) {
		myRelation->setFromEdge	(from);
		myRelation->setToEdge	(to);
	} 
}

//-----------------------------------------------------------------------------
// Name: edge::getPosition()
// Desc: 
//-----------------------------------------------------------------------------
float wildWeasel::alignment::edge::getPosition()
{
	return position;
}

//-----------------------------------------------------------------------------
// Name: edge::setPosition()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignment::edge::setPosition(float pixelPos)
{
	// leave if difference is too small
	if (abs(pixelPos - position) < 0.001f) return;
	if (isnan(pixelPos)) return;

	// set position
	position	= pixelPos;

	// inform followingRelations	
	for (auto& curFollowingRelation : followingRelations) {
		if (curFollowingRelation) {
			curFollowingRelation->informFollowingEdge();
		}
	}

	// update relation ship
	if (leadingRelation) {
		leadingRelation->setPosition(position);
	}
}
#pragma endregion

#pragma region grid
//-----------------------------------------------------------------------------
// Name: grid::grid()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::alignment::grid::grid(edgeRelation& dx, edgeRelation& dy, unsigned int per, posMode pm, alignmentHorizontal alignX, alignmentVertical alignY) :
	periodicity	{per},
	mode		{pm },
	alignmentX	{alignX},
	alignmentY	{alignY}
{
	distX.setRelation(dx);
	distY.setRelation(dy);
}

//-----------------------------------------------------------------------------
// Name: grid::setRelation()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignment::grid::setRelation(edgeRelation & dx, edgeRelation & dy, unsigned int per, posMode pm, alignmentHorizontal alignX, alignmentVertical alignY)
{
	periodicity = per;
	mode		= pm;
	alignmentX	= alignX;
	alignmentY	= alignY;
	distX.setRelation(dx);
	distY.setRelation(dy);
}

//-----------------------------------------------------------------------------
// Name: grid::getOffset()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::vector2 wildWeasel::alignment::grid::getOffset(unsigned int gridPosition, float itemWidth, float itemHeight)
{
	// locals
	vector2 distInPixels;
	vector2 offset;
	unsigned int gridPosX;
	unsigned int gridPosY;

	switch (mode)
	{
	case posMode::ROW_WISE: 
		gridPosX	= gridPosition % periodicity;
		gridPosY	= gridPosition / periodicity;
		break;
	case posMode::COLUMN_WISE: 
		gridPosX	= gridPosition / periodicity;
		gridPosY	= gridPosition % periodicity;
		break;
	}

	distInPixels.x = distX.leadingRelation->getDistanceToRefEdge();
	distInPixels.y = distY.leadingRelation->getDistanceToRefEdge();

	offset.x = gridPosX * (itemWidth  + distInPixels.x);
	offset.y = gridPosY * (itemHeight + distInPixels.y);

	switch (alignmentX) 
	{
	case alignmentHorizontal::LEFT:																								break;
	case alignmentHorizontal::CENTER: 	offset.x += -0.5f * (periodicity * itemWidth + (periodicity-1) * distInPixels.x);		break;
	case alignmentHorizontal::RIGHT: 	offset.x += -1.0f * (periodicity * itemWidth + (periodicity-1) * distInPixels.x);		break;
	}

	switch (alignmentY) 
	{
	case alignmentVertical::TOP:																								break;
	case alignmentVertical::CENTER: 	offset.y += -0.5f * (periodicity * itemHeight + (periodicity-1) * distInPixels.y);		break;
	case alignmentVertical::BOTTOM: 	offset.y += -1.0f * (periodicity * itemHeight + (periodicity-1) * distInPixels.y);		break;
	}

	return offset;
}

//-----------------------------------------------------------------------------
// Name: grid::setInsideAnotherRect()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignment::grid::setInsideAnotherRect(alignment& frameRect)
{
	if (auto leadingRelation = distX.getRelation<relFraction>()) {
		leadingRelation->setFromEdge	(frameRect.left );
		leadingRelation->setToEdge		(frameRect.right);
	} else if (auto leadingRelation = distX.getRelation<relFractionAdd>()) {
		leadingRelation->setFromEdge	(frameRect.left );
		leadingRelation->setToEdge		(frameRect.right);
	}

	if (auto leadingRelation = distY.getRelation<relFraction>()) {
		leadingRelation->setFromEdge	(frameRect.top);
		leadingRelation->setToEdge		(frameRect.bottom);
	} else if (auto leadingRelation = distY.getRelation<relFractionAdd>()) {
		leadingRelation->setFromEdge	(frameRect.top);
		leadingRelation->setToEdge		(frameRect.bottom);
	}
}

//-----------------------------------------------------------------------------
// Name: grid::~grid()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::alignment::grid::~grid()
{
}
#pragma endregion

#pragma region alignment
//-----------------------------------------------------------------------------
// Name: alignment::alignment()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::alignment::alignment()
{

}

//-----------------------------------------------------------------------------
// Name: alignment::alignment()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::alignment::alignment(const alignment& copyFrom) :
	root		{ copyFrom.root			} ,
	frameRect	{ copyFrom.frameRect	} ,
	left		{ copyFrom.left			} ,
	right		{ copyFrom.right		} ,
	top			{ copyFrom.top			} ,
	bottom		{ copyFrom.bottom		}
{
	if (copyFrom.initInfo) {
		initInfo = new initializationInfo{*copyFrom.initInfo};
	}

	if (copyFrom.pGrid) {
		pGrid = new grid{*copyFrom.pGrid};
	}
}

//-----------------------------------------------------------------------------
// Name: alignment::alignment()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::alignment::alignment(alignmentTypeX	type_xPos, unsigned int	xPos, alignmentTypeY type_yPos, unsigned int yPos, alignmentTypeX type_width, unsigned int width, alignmentTypeY type_height, unsigned int height, alignmentTypeX type_xDist, unsigned int xDist, alignmentTypeY type_yDist, unsigned int yDist, unsigned int periodicity, posMode mode) :
	alignment(type_xPos, (float) xPos, type_yPos, (float) yPos, type_width, (float) width, type_height, (float) height, type_xDist, (float) xDist, type_yDist, (float) yDist, periodicity, mode)
{
}

//-----------------------------------------------------------------------------
// Name: alignment::alignment()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::alignment::alignment(alignmentTypeX	type_xPos, float        xPos, alignmentTypeY type_yPos, float        yPos, alignmentTypeX type_width, float        width, alignmentTypeY type_height, float        height, alignmentTypeX type_xDist, float        xDist, alignmentTypeY type_yDist, float		yDist, unsigned int periodicity, posMode mode)
{
	if (initInfo) {
		delete initInfo;
	}
	initInfo = new initializationInfo{type_xPos, xPos, type_yPos, yPos, type_width, width, type_height, height, type_xDist, xDist, type_yDist, yDist, periodicity, mode};
}

//-----------------------------------------------------------------------------
// Name: alignment::~alignment()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::alignment::~alignment()
{
	if (pGrid	) { delete pGrid;		pGrid	 = nullptr;	}
	if (initInfo) { delete initInfo;	initInfo = nullptr; }
	root		= nullptr;
	frameRect	= nullptr;
}

//-----------------------------------------------------------------------------
// Name: alignment::create()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignment::create(alignmentRoot& root)
{
	this->root		= &root;
	this->frameRect	= this->root;

	if (initInfo) {
		setPosition(initInfo->type_xPos, initInfo->xPos, alignmentHorizontal::LEFT, initInfo->type_yPos, initInfo->yPos, alignmentVertical::TOP);
		setSize(initInfo->type_width, initInfo->width, initInfo->type_height, initInfo->height);
		setGrid(initInfo->type_xDist, initInfo->xDist, alignmentHorizontal::LEFT, initInfo->type_yDist, initInfo->yDist, alignmentVertical::TOP, initInfo->periodicity, initInfo->mode);
		delete initInfo;
		initInfo = nullptr;
	}
}

//-----------------------------------------------------------------------------
// Name: alignment::isCreated()
// Desc: 
//-----------------------------------------------------------------------------
inline bool wildWeasel::alignment::isCreated()
{
	return (root && !initInfo);
}

//-----------------------------------------------------------------------------
// Name: alignment::getRect()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::fRECT wildWeasel::alignment::getRect(unsigned int gridPosition)
{
	if (!isCreated()) {
		// TODO: Is there a way to inform the user about this error? Maybe a static cout stream or so?
		// wildWeasel::printError("ERROR: wildWeasel:alignment was NOT created prior get function!");
		return fRECT{0,0,0,0};
	}

	if (pGrid) {
		vector2 offset{pGrid->getOffset(gridPosition, getWidth(), getHeight())};
		return fRECT{left .getPosition() + offset.x, top   .getPosition() + offset.y, 
			         right.getPosition() + offset.x, bottom.getPosition() + offset.y};
	} else {
		return fRECT{left.getPosition(), top.getPosition(), right.getPosition(), bottom.getPosition()};
	}
}

//-----------------------------------------------------------------------------
// Name: alignment::getWidth()
// Desc: 
//-----------------------------------------------------------------------------
float wildWeasel::alignment::getWidth()
{
	//	if (!isCreated()) {	/* wildWeasel::printError("ERROR: wildWeasel:alignment was NOT created prior get function!");*/	return 0;	}
	return right.getPosition() - left.getPosition();
}

//-----------------------------------------------------------------------------
// Name: alignment::getHeight()
// Desc: 
//-----------------------------------------------------------------------------
float wildWeasel::alignment::getHeight()
{
	//	if (!isCreated()) {	/* wildWeasel::printError("ERROR: wildWeasel:alignment was NOT created prior get function!");*/ return 0;	}
	return bottom.getPosition() - top.getPosition();
}

//-----------------------------------------------------------------------------
// Name: alignment::setInsideAnotherRect()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignment::setInsideAnotherRect(alignment& frameRect)
{
	// is alignment already initialized ?
	if (!isCreated()) {	/* wildWeasel::printError("ERROR: wildWeasel:alignment was NOT created prior get function!");*/	return;	}

	// align edges to new frame
	left	.setInsideAnotherEdges(frameRect.left,	frameRect.right , this->frameRect->left, this->frameRect->right );
	right	.setInsideAnotherEdges(frameRect.left,	frameRect.right , this->frameRect->left, this->frameRect->right );
	top		.setInsideAnotherEdges(frameRect.top,	frameRect.bottom, this->frameRect->top , this->frameRect->bottom);
	bottom	.setInsideAnotherEdges(frameRect.top,	frameRect.bottom, this->frameRect->top , this->frameRect->bottom);

	// remember passed frame
	this->frameRect = &frameRect;

	// adapt grid onto new frame
	if (pGrid) {
		pGrid->setInsideAnotherRect(frameRect);
	}
}

//-----------------------------------------------------------------------------
// Name: alignment::setPosition()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignment::setPosition(alignmentTypeX typeX, float xPos,  alignmentHorizontal alignX, alignmentTypeY typeY, float yPos, alignmentVertical alignY)
{
	// initialized?
	if (root == nullptr || frameRect == nullptr) {	/*wildWeasel::printError("ERROR: wildWeasel:alignment::setPosition was NOT created prior get function!");*/	return;	}

	// offset
	vector2 offset;

	// alignX, alignY
	switch (alignX)
	{
	case alignmentHorizontal::BLOCK:		offset.x = 0;					break;
	case alignmentHorizontal::LEFT:			offset.x = 0;					break;
	case alignmentHorizontal::RIGHT:		offset.x = -1.0f * getWidth();	break;
	case alignmentHorizontal::CENTER:		offset.x = -0.5f * getWidth();	break;
	}
	
	switch (alignY)
	{
	case alignmentVertical::ABOVE:			offset.y = 0;					break;
	case alignmentVertical::BELOW:			offset.y = 0;					break;
	case alignmentVertical::TOP:			offset.y = 0;					break;
	case alignmentVertical::BOTTOM:			offset.y = -1.0f * getHeight();	break;
	case alignmentVertical::CENTER:			offset.y = -0.5f * getHeight();	break;
	}

	// edges
	switch (typeX)
	{
	case alignmentTypeX::BORDER_LEFT:		left	.setRelation(relDistance		{frameRect->left,					xPos + offset.x});								break;
	case alignmentTypeX::BORDER_RIGHT:		left	.setRelation(relDistance		{frameRect->right,					xPos + offset.x});								break;
	case alignmentTypeX::FRACTION:			left	.setRelation(relFraction		{frameRect->left, frameRect->right,	xPos + offset.x / frameRect->getWidth()});		break;
	case alignmentTypeX::FRACTION_WIDTH:	left	.setRelation(relFraction		{frameRect->left, frameRect->right,	xPos + offset.x / frameRect->getWidth()});		break;
	case alignmentTypeX::PIXEL_WIDTH:		left	.setRelation(relDistance		{frameRect->left,					xPos + offset.x});								break;
	case alignmentTypeX::USER:				left	.setRelation(relDistance		{frameRect->left,					xPos + offset.x});								break;
	}

	switch (typeY) 
	{
	case alignmentTypeY::BORDER_TOP:		top		.setRelation(relDistance		{frameRect->top,					yPos + offset.y});								break;
	case alignmentTypeY::BORDER_BOTTOM:		top		.setRelation(relDistance		{frameRect->bottom,					yPos + offset.y});								break;
	case alignmentTypeY::FRACTION:			top		.setRelation(relFraction		{frameRect->top, frameRect->bottom,	yPos + offset.y / frameRect->getHeight()});		break;
	case alignmentTypeY::FRACTION_HEIGHT:	top		.setRelation(relFraction		{frameRect->top, frameRect->bottom,	yPos + offset.y / frameRect->getHeight()});		break;
	case alignmentTypeY::PIXEL_HEIGHT:		top		.setRelation(relDistance		{frameRect->top,					yPos + offset.y});								break;
	case alignmentTypeY::USER:				top		.setRelation(relDistance		{frameRect->top,					yPos + offset.y});								break;
	}
}

//-----------------------------------------------------------------------------
// Name: alignment::setSize()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignment::setSize(alignmentTypeX typeX, float width, alignmentTypeY typeY, float height)
{
	// initialized?
	if (root == nullptr || frameRect == nullptr) {	/*wildWeasel::printError("ERROR: wildWeasel:alignment::setPosition was NOT created prior get function!");*/	return;	}

	// offset
	// ... alignmentHorizontal alignX and alignmentVertical alignY should be considered here! but, this means that the horizontal and vertical alignment must be stored in the alignment class.

	switch (typeX) 
	{
	case alignmentTypeX::BORDER_LEFT:		right	.setRelation(relDistance		{frameRect->left,							width});				break;
	case alignmentTypeX::BORDER_RIGHT:		right	.setRelation(relDistance		{frameRect->right,							width});				break;
	case alignmentTypeX::FRACTION:			right	.setRelation(relFractionAdd		{frameRect->left, frameRect->right, left,	width});				break;
	case alignmentTypeX::FRACTION_WIDTH:	right	.setRelation(relFractionAdd		{frameRect->left, frameRect->right, left,	width});				break;
	case alignmentTypeX::PIXEL_WIDTH:		right	.setRelation(relDistance		{this->left,								width});				break;
	case alignmentTypeX::USER:				right	.setRelation(relDistance		{this->left,								width});				break;
	}

	switch (typeY) 
	{
	case alignmentTypeY::BORDER_TOP:		bottom	.setRelation(relDistance		{frameRect->top,							height});				break;
	case alignmentTypeY::BORDER_BOTTOM:		bottom	.setRelation(relDistance		{frameRect->bottom,							height});				break;
	case alignmentTypeY::FRACTION:			bottom	.setRelation(relFractionAdd		{frameRect->top, frameRect->bottom, top,	height});				break;
	case alignmentTypeY::FRACTION_HEIGHT:	bottom	.setRelation(relFractionAdd		{frameRect->top, frameRect->bottom, top,	height});				break;
	case alignmentTypeY::PIXEL_HEIGHT:		bottom	.setRelation(relDistance		{this->top,									height});				break;
	case alignmentTypeY::USER:				bottom	.setRelation(relDistance		{this->top,									height});				break;
	}
}

//-----------------------------------------------------------------------------
// Name: alignment::setGrid()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignment::setGrid(alignmentTypeX typeX, float xDist, alignmentHorizontal alignX, alignmentTypeY typeY, float yDist, alignmentVertical alignY, unsigned int periodicity, posMode mode)
{
	// initialized?
	if (root == nullptr || frameRect == nullptr) {	/*wildWeasel::printError("ERROR: wildWeasel:alignment::setPosition was NOT created prior get function!");*/	return;	}
	if (abs(xDist) < 0.00001f && abs(yDist) < 0.00001f) return;

	// relations
	edgeRelation* relationDistX;
	edgeRelation* relationDistY;

	switch (typeX) 
	{
	case alignmentTypeX::BORDER_LEFT:		relationDistX = new relDistance{right, xDist};											break;
	case alignmentTypeX::BORDER_RIGHT:		relationDistX = new relDistance{right, xDist};											break;
	case alignmentTypeX::FRACTION:			relationDistX = new relFractionAdd{frameRect->left, frameRect->right, right, xDist};	break;
	case alignmentTypeX::FRACTION_WIDTH:	relationDistX = new relFractionAdd{frameRect->left, frameRect->right, right, xDist};	break;
	case alignmentTypeX::PIXEL_WIDTH:		relationDistX = new relDistance	{right, xDist};											break;
	case alignmentTypeX::USER:				relationDistX = new relDistance	{right, xDist};											break;
	}

	switch (typeY) 
	{
	case alignmentTypeY::BORDER_TOP:		relationDistY = new relDistance{bottom,	yDist};											break;
	case alignmentTypeY::BORDER_BOTTOM:		relationDistY = new relDistance{bottom,	yDist};											break;
	case alignmentTypeY::FRACTION:			relationDistY = new relFractionAdd{frameRect->top, frameRect->bottom, bottom, yDist};	break;
	case alignmentTypeY::FRACTION_HEIGHT:	relationDistY = new relFractionAdd{frameRect->top, frameRect->bottom, bottom, yDist};	break;
	case alignmentTypeY::PIXEL_HEIGHT:		relationDistY = new relDistance{bottom, yDist};											break;
	case alignmentTypeY::USER:				relationDistY = new relDistance{bottom,	yDist};											break;
	}

	if (pGrid) {
		pGrid->setRelation(*relationDistX, *relationDistY, periodicity, mode, alignX, alignY);
	} else {
		pGrid = new grid{*relationDistX, *relationDistY, periodicity, mode, alignX, alignY};
	}
	
	delete relationDistX;
	delete relationDistY;
}
#pragma endregion

#pragma region alignmentRoot
//-----------------------------------------------------------------------------
// Name: alignmentRoot::alignmentRoot()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::alignmentRoot::alignmentRoot(POINT& windowSize) : windowSize{windowSize}
{
	eventFollower::followEvent(this, eventType::WINDOWSIZE_CHANGED);
	windowSizeChanged(windowSize.x, windowSize.y);
}

//-----------------------------------------------------------------------------
// Name: alignmentRoot::alignmentRoot()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::alignmentRoot::~alignmentRoot()
{
	eventFollower::forgetEvent(this, eventType::WINDOWSIZE_CHANGED);
}

//-----------------------------------------------------------------------------
// Name: alignmentRoot::alignmentRoot()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignmentRoot::windowSizeChanged(int xSize, int ySize)
{
	left.setPosition(0);
	top.setPosition(0);
	right.setPosition((float) xSize);
	bottom.setPosition((float) ySize);
}
#pragma endregion

#pragma region initializationInfo
//-----------------------------------------------------------------------------
// Name: initializationInfo::initializationInfo()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::alignment::initializationInfo::initializationInfo(alignmentTypeX type_xPos, float xPos, alignmentTypeY type_yPos, float yPos, alignmentTypeX type_width, float width, alignmentTypeY type_height, float height, alignmentTypeX type_xDist, float xDist, alignmentTypeY type_yDist, float yDist, unsigned int periodicity, posMode mode) :
	type_xPos		{ type_xPos		},
	type_yPos		{ type_yPos		},
	type_width		{ type_width	},
	type_height		{ type_height	},
	xPos			{ xPos			},
	yPos			{ yPos			},
	width			{ width			},
	height			{ height		},
	type_xDist		{ type_xDist	},
	type_yDist		{ type_yDist	},
	xDist			{ xDist			},
	yDist			{ yDist			},
	periodicity		{ periodicity	},
	mode			{ mode			}
{
}
#pragma endregion

#pragma region alignedRect
//-----------------------------------------------------------------------------
// Name: alignedRect::alignedRect()
// Desc: Assigns a certain alignment to a gui element.
// Info: The passed 'newAlignment' argument must not be deleted after the call, since the pointer to it is used.
//-----------------------------------------------------------------------------
void wildWeasel::alignedRect::setAlignment(alignment& newAlignment, unsigned int gridPosition)
{
	alignmentTargetRect		= &newAlignment;
	alignmentGridPosition	= gridPosition;
	// updateTargetRect();		// removed since parameter 'root' is not know here
}

//-----------------------------------------------------------------------------
// Name: alignedRect::setTargetRect()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignedRect::setTargetRect(const RECT& newRect)
{
	alignmentTargetRect = nullptr;
	targetRect			= newRect;	
}

//-----------------------------------------------------------------------------
// Name: alignedRect::setGridPosition()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignedRect::setGridPosition(unsigned int newPosition)
{
	alignmentGridPosition = newPosition;
}

//-----------------------------------------------------------------------------
// Name: alignedRect::updateTargetRect()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::alignedRect::updateTargetRect(alignmentRoot& root)
{
	if (alignmentTargetRect) {

		// create alignment if not already done by the user
		if (!alignmentTargetRect->isCreated()) {
			alignmentTargetRect->create(root);
		}

		// get positions of the edges and put them into the 'targetRect'
		targetRect = alignmentTargetRect->getRect(alignmentGridPosition);
	} else {
		// do nothing
	}
}
#pragma endregion
