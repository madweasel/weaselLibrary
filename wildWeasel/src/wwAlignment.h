/*********************************************************************\
	wwAlignment.h												  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/

#pragma once

#include "wildWeasel.h"

namespace wildWeasel 
{
	class alignment;
	class alignmentRoot;
	class alignedRect;

	class alignment
	{
	protected:
		class edge;
		class grid;

		class edgeRelation
		{
		friend class edge;
		friend class grid;
		friend class alignment;

		protected:
			edge*							followingEdge						= nullptr;
			vector<edge**>					leadingEdges;

			static edgeRelation*			newRelation							(const edgeRelation& source);
			virtual							~edgeRelation						();

			void							informFollowingEdge					();
			void							enrollInLeadingEdges				();
			void							setLeadingEdge						(edge* &edgeToSet, edge& newEdge);
			void							setFollowingEdge					(edge& newFollower);
			virtual void					setPosition							(float pixelPos) { };
			virtual float					calcPosition						()	{ return 0; };
			virtual void					fillLeadingEdges					() { };
		
		public:

			virtual float					getDistanceToRefEdge				()	{ return 0; };
		};

	public:
		class relDistance : public edgeRelation
		{
		private:
			float							distance							= 0;
			edge*							edgeReference						= nullptr;

			void							setPosition							(float pixelPos) override;
			float							calcPosition						() override;
			void							fillLeadingEdges					() override;

		public:
											relDistance							(edge& reference, float distance);
			float							getDistanceToRefEdge				() override;
			void							setDistance							(float newDistance);								// FOLLOWER-RELEVANT:
			void							setReferenceEdge					(edge& newReferenceEdge);							// FOLLOWER-RELEVANT:
			edge*							getEdgeReference					()	 { return edgeReference; };
		};

		class relFraction : public edgeRelation
		{
		private:
			float							fraction							= 0;
			edge*							edgeFrom							= nullptr;
			edge*							edgeTo								= nullptr;

			void							setPosition							(float pixelPos) override;
			float							calcPosition						() override;
			void							fillLeadingEdges					() override;

		public:
											relFraction							(edge& from, edge& to, float fraction);
			void							setFraction							(float newFraction);								// FOLLOWER-RELEVANT:
			void							setFromEdge							(edge& newFromEdge);								// FOLLOWER-RELEVANT:
			void							setToEdge							(edge& newToEdge);									// FOLLOWER-RELEVANT:
			float							getDistanceToRefEdge				() override;
		};
	
		class relFractionAdd : public edgeRelation
		{
		private:
			float							fraction							= 0;
			edge*							edgeFrom							= nullptr;
			edge*							edgeTo								= nullptr;
			edge*							edgeReference						= nullptr;

			void							setPosition							(float pixelPos) override;
			float							calcPosition						() override;
			void							fillLeadingEdges					() override;

		public:
											relFractionAdd						(edge& from, edge& to, edge& reference, float fraction);
			void							setFraction							(float newFraction);								// FOLLOWER-RELEVANT:		
			void							setReferenceEdge					(edge& newReferenceEdge);							// FOLLOWER-RELEVANT:
			void							setFromEdge							(edge& newFromEdge);								// FOLLOWER-RELEVANT:
			void							setToEdge							(edge& newToEdge);									// FOLLOWER-RELEVANT:
			float							getDistanceToRefEdge				() override;
		};

		enum class							posMode								{ ROW_WISE,  COLUMN_WISE };

	protected:
		class edge
		{
		friend class edgeRelation;
		friend class grid;
		
		protected:
			bool							edgeIsBeingDeleted					= false;
			float							position							= 0;												// pixel position
			edgeRelation*					leadingRelation						= nullptr;											// corresponding relationship, which determines the position
			vector<edgeRelation*>			followingRelations;																		// these relations are informed when the position of this edge changes

											edge								();
											edge								(const edge& copyFrom);
											~edge								();

			void							addFollower							(edgeRelation& newFollower);
			void							removeFollower						(edgeRelation& formerFollower);
			void							relationIsBeingDeleted				(edgeRelation& rel);

		public:
			float							getPosition							();													// return the position in pixels
			void							setPosition							(float pixelPos);									// FOLLOWER-RELEVANT: 
			void							move								(float distanceInPixels);							// FOLLOWER-RELEVANT: changes the relationship parameter so that the edge is move by this amount of pixels
			void							setRelation							(const edgeRelation& newEdgeRelation);				// FOLLOWER-RELEVANT:
			void							setRelationBasedOnDistance			(edge& reference, float distance);					// FOLLOWER-RELEVANT:
			template <typename T> T*		getRelation							()										{
																															if (leadingRelation != nullptr && typeid(T) == typeid(*leadingRelation)) {
																																return (dynamic_cast<T*>(leadingRelation));
																															} else {
																																return nullptr;
																															}
																														};											// FOLLOWER-RELEVANT:
			void							setInsideAnotherEdges				(edge& from, edge& to, edge& oldFrom, edge& oldTo);
		};

		class edgeX : public edge
		{

		};

		class edgeY : public edge
		{

		};

		class grid
		{
		friend class alignment;

		protected:
											~grid								();
		private:
			edge							distX;																			// distance between controls
			edge							distY;																			// ''
			unsigned int					periodicity							= 1;										// number of controls in one row/column
			posMode							mode								= posMode::ROW_WISE;						// mode
			alignmentHorizontal				alignmentX							= alignmentHorizontal::LEFT;
			alignmentVertical				alignmentY							= alignmentVertical::TOP;

		public:
											grid								(edgeRelation& dx, edgeRelation& dy, unsigned int per, posMode pm, alignmentHorizontal alignX = alignmentHorizontal::LEFT, alignmentVertical alignY = alignmentVertical::TOP);
			void							setRelation							(edgeRelation& dx, edgeRelation& dy, unsigned int per, posMode pm, alignmentHorizontal alignX = alignmentHorizontal::LEFT, alignmentVertical alignY = alignmentVertical::TOP);
			void							setInsideAnotherRect				(alignment& frameRect);
			vector2							getOffset							(unsigned int gridPosition, float itemWidth, float itemHeight);
			float							getGridDistX						() { return distX.leadingRelation->getDistanceToRefEdge();}
			float							getGridDistY						() { return distY.leadingRelation->getDistanceToRefEdge();}
		};

	private:
		class initializationInfo
		{
		public:
			alignmentTypeX					type_xPos							= alignmentTypeX::USER;
			alignmentTypeY					type_yPos							= alignmentTypeY::USER;
			alignmentTypeX					type_width							= alignmentTypeX::USER;
			alignmentTypeY					type_height							= alignmentTypeY::USER;
			float							xPos								= 0;
			float							yPos								= 0;
			float							width								= 0;
			float							height								= 0;
			alignmentTypeX					type_xDist							= alignmentTypeX::USER;
			alignmentTypeY					type_yDist							= alignmentTypeY::USER;
			float							xDist								= 0;
			float							yDist								= 0;
			unsigned int					periodicity							= 1;
			posMode							mode								= posMode::ROW_WISE;

											initializationInfo					(alignmentTypeX	type_xPos, float        xPos, alignmentTypeY type_yPos, float        yPos, alignmentTypeX type_width, float        width, alignmentTypeY type_height, float        height, alignmentTypeX type_xDist = alignmentTypeX::USER, float        xDist = 0, alignmentTypeY type_yDist = alignmentTypeY::USER, float		yDist = 0, unsigned int periodicity = 1, posMode mode = posMode::ROW_WISE);
		};

	private:
		initializationInfo*					initInfo							= nullptr;					// a datastruct containing the alignment infos as long as alignment::create() has not been called
		alignmentRoot*						root								= nullptr;					// pointer to the root frame alignment
		grid*								pGrid								= nullptr;					// repeated grid like positioning of rects
		alignment*							frameRect							= nullptr;					// position and sizing is always relative to a frame

	public:
		edgeX								left;															// the four edges of the aligned rect
		edgeY								top;															// ''
		edgeX								right;															// ''
		edgeY								bottom;															// ''

											alignment							();
											alignment							(const alignment& copyFrom);
											alignment							(alignmentTypeX	type_xPos, unsigned int	xPos, alignmentTypeY type_yPos,	unsigned int yPos, alignmentTypeX type_width, unsigned int width, alignmentTypeY type_height, unsigned int height, alignmentTypeX type_xDist = alignmentTypeX::USER, unsigned int xDist = 0, alignmentTypeY type_yDist = alignmentTypeY::USER, unsigned int yDist = 0, unsigned int periodicity = 1, posMode mode = posMode::ROW_WISE);
											alignment							(alignmentTypeX	type_xPos, float        xPos, alignmentTypeY type_yPos, float        yPos, alignmentTypeX type_width, float        width, alignmentTypeY type_height, float        height, alignmentTypeX type_xDist = alignmentTypeX::USER, float        xDist = 0, alignmentTypeY type_yDist = alignmentTypeY::USER, float		yDist = 0, unsigned int periodicity = 1, posMode mode = posMode::ROW_WISE);
											~alignment							();

		void								create								(alignmentRoot& root);
		inline bool							isCreated							();
		fRECT								getRect								(unsigned int gridPosition = 0);
		float								getWidth							();
		float								getHeight							();
		float								getGridDistX						() { return (pGrid ? pGrid->getGridDistX() : 0);}
		float								getGridDistY						() { return (pGrid ? pGrid->getGridDistY() : 0);}
		void								setPosition							(alignmentTypeX typeX, float xPos,  alignmentHorizontal alignX, alignmentTypeY typeY, float yPos, alignmentVertical alignY);
		void								setSize								(alignmentTypeX typeX, float width,								alignmentTypeY typeY, float height);
		void								setGrid								(alignmentTypeX typeX, float xDist, alignmentHorizontal alignX, alignmentTypeY typeY, float yDist, alignmentVertical alignY, unsigned int periodicity, posMode mode = posMode::ROW_WISE);
		void								setInsideAnotherRect				(alignment& frameRect);
	};

	// automatically updates a rect when the window size changes
	class alignmentRoot : public alignment, protected eventFollower
	{
	friend class masterMind;
	protected:
		POINT&								windowSize;

											alignmentRoot						(POINT& windowSize);
											~alignmentRoot						();

		void								windowSizeChanged					(int xSize, int ySize);								// FOLLOWER-RELEVANT:
	};

	// guiElement2D can inherit from this class, so that the user can easily control its position
	class alignedRect
	{
	public:
		void								setAlignment						(alignment& newAlignment, unsigned int gridPosition = 0);
		void								setTargetRect						(const RECT& newRect);
		void								setGridPosition						(unsigned int newPosition);
		fRECT								getTargetRect						() { return targetRect; };
		
		alignment*							getAlignment						() { return alignmentTargetRect; }
		unsigned int						getGridPosition						() { return alignmentGridPosition; };
											
	protected:								
		unsigned int						alignmentGridPosition;
		alignment*							alignmentTargetRect					= nullptr;
		fRECT								targetRect;

		void								updateTargetRect					(alignmentRoot& root);
	};
}


