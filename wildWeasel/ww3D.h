/*********************************************************************\
	ww3D.h
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/

#pragma once

#include "wildWeasel.h"

namespace wildWeasel
{
	// pre-definition
	class genericObject3D;
	class realTextureItem;
	class texture;
	class matrix2;
	class polygon;
	class font2D;
	class font3D;
	class matrixDirty;
	class matrixChain;
	class matrixControl2D;
	class matrixControl3D;
	class camera;
	class screenInformation;
	class cursorClass2D;
	class cursorClass3D;
	class loadingScreen;

	// this class represents a unique real existing texture holding a pointer to 'ssf::textureResource'
	class realTextureItem : private genericObject3D
	{
	private:
		static list<realTextureItem*>		allTextures;
			
		masterMind*							ww									= nullptr;		// pointer to masterMind
		bool								removeBorder						= false;		// if the nasty PowerPoint border shall be removed
		bool								generateMipmaps						= false;		// if mipmaps shall be generated automatically
		wstring								strFileName;										// based on this string duplicate textures are identified
		ssf::textureResource*				ssfTexture							= nullptr;		// thats the actual implementation with system specific function
		int									linkedToThis						= 0;			// the number of textures objects pointing to this realTextureItem. 
											
											~realTextureItem					();
		void								createDeviceDependentResources		(ssf::sharedVars& v);
		void								onDeviceLost						();
											
	public:									
		static realTextureItem*				loadFile							(masterMind* ww, const wstring& strFileName, bool removeInvisibleBorder = false);
		static realTextureItem*				getRealTexture						(const wstring& strFileName);
		static realTextureItem*				addLink								(realTextureItem* pItem);
		static void							removeLink							(realTextureItem* pItem);
		fRECT								getSize_fRECT						();
		vector2								getSize_vector2						();
		ssf::textureResource*				getTextureResource					() { return uploaded ? ssfTexture : nullptr; };
		wstring&							getFileName							() { return strFileName; }
	};

	// virtual texture class seen by the user. it actually points only a realTextureItem.
	class texture
	{
	private:
		realTextureItem*					realTexture							= nullptr;		// pointer to the actual texture, so that duplicates can be mapped to only one object instance
											
	public:									
											~texture							();
		bool								loadFile							(masterMind* ww, const wstring& strFileName, bool removeInvisibleBorder = false);
		vector2								getSize								() { return realTexture->getSize_vector2(); };
		ssf::textureResource*				getTextureResource					() { return realTexture ? realTexture->getTextureResource() : nullptr; };
	};

	// a two-dimensional matrix for operations with vector2
	class matrix2
	{
	public:
		float								m11, m12;
		float								m21, m22;

											matrix2								(float rotation) {m11 = cos(rotation); m21 = sin(rotation); m12 = -m21; m22 = m11; }

	    vector2								operator*							(const vector2& v) const { return vector2(m11*v.x + m12*v.y, m21*v.x + m22*v.y); }
	};

	// 3 vectors representing one polygon
	class polygon
	{
	public:
		vector3								v1, v2, v3;
											polygon								()	{};
											polygon								(vector3& v1, vector3& v2, vector3& v3)	{ this->v1 = v1; this->v2 = v2; this->v3 = v3; };
	};
											
	// information about a font for displaying 2D texts
	class font2D : public ssf::text2D
	{
	friend class guiElement;

	public:									
		bool								loadFontFile						(masterMind* ww, wstring& strFileName);
	};

	// information about a font for displaying 2D texts
	// TODO: currently only a dummy 2D text is used. real 3d text 
	class font3D : private genericObject3D
	{
	friend class guiElement;

	private:
		wstring								strFileName;
		masterMind*							parent;
		font2D								substitute2D;

	public:									
		void								drawString							(wchar_t const* text, vector3 const& position, color theColor, vector2 const& scale, fRECT& clippingRect, const float lineSpacing, bool applyOffsetOnFirstCharacter);
		bool								loadFontFile						(masterMind* ww, wstring& strFileName);
	};

	// each matrix can be set as dirty marking that the whole chain must be recalculated
	class matrixDirty
	{
	public:
		matrix*								mat									= NULL;
		bool*								dirtyBit							= NULL;

		bool								operator==							(const matrixDirty& m) { return (mat == m.mat && dirtyBit == m.dirtyBit); }
	};

	// each vertex is transformed by several matrices. here the final matrix is only recalculated when at least one matrix factor is marked as dirty
	class matrixChain
	{
	friend class guiElement2D;

	private:
		vector<matrixDirty>					matrices;
		matrix								finalMatrix;
											
	protected:								
		bool								calcFinalMatrix						();
											
	public:									
		void								getFinalMatrix						(matrix& mat);
		void								insertMatrix						(unsigned int position, matrix* additionalMatrix, bool* additionalDirtyBit);
		void								removeMatrix						(matrix* matrixToRemove);
	};

	// class for easily modifing the position and orientation of all gui elements
	class matrixControl3D
	{
	friend class matrixControlAnimation;

	public:
		enum class							matControlMode						{ matrix, posRotSca };
											
		void								setDirty							();
		void 								setPositioningMode					(matControlMode newMode);
		void								setPointerToViewMatrix				(matrix* newView);
	
		// mode==matControlMode::matrix
		void								setMatrix							(matrix* newMat, bool relative);

		// mode==matControlMode::posRotSca
		void								setPosition							(float x, float y, float z, bool relativeToView, bool updateMatrix);
		void								setScale							(float x, float y, float z, bool relativeToView, bool updateMatrix);
		void								setRotation							(float x, float y, float z, bool relativeToView, bool updateMatrix);
		void								getPosition							(vector3& position);
		void								getScale							(vector3& scale   );
		void								getRotation							(vector3& rotation);
		void								setPosition							(vector3* newPosition, bool relativeToView, bool updateMatrix);
		void								setScale							(vector3* newScale   , bool relativeToView, bool updateMatrix);
		void								setRotation							(vector3* newRotation, bool relativeToView, bool updateMatrix);
		void								setScaleRotPos						(float sx, float sy, float sz, float rx, float ry, float rz, float tx, float ty, float tz);
		void								setScale							(float xyz, bool relativeToView, bool updateMatrix)	{setScale(xyz, xyz, xyz, relativeToView, updateMatrix); };
											
	protected:								
		matControlMode 						mode								= matControlMode::matrix;
		matrix*								matView								= NULL;									// view matrix, this one is important so that the buttons can be moved relative to the view
		vector3								position							= vector3(0, 0, 0);						// position of whole object in space
		vector3								scale								= vector3(1, 1, 1);						// scale of whole object in space
		vector3								rotation							= vector3(0, 0, 0);						// position of whole object in space
	public:
		matrix								mat;																		// controlled matrix
		bool								dirtyBit							= true;									// could be unified in matrixDirty object
											
		void								recalcMatrix						();
	};

	// class for easily modifing the position and orientation of all gui elements
	class matrixControl2D
	{
	public:
		enum class							matControlMode						{ matrix, posRotSca };
											
		void								setDirty							();
		void 								setPositioningMode					(matControlMode newMode);
	
		// mode==matControlMode::matrix
		void								setMatrix							(matrix* newMat, bool relative);

		// mode==matControlMode::posRotSca
		void								setPosition							(float x, float y, bool updateMatrix);
		void								setScale							(float x, float y, bool updateMatrix);
		void								setRotation							(float z,		   bool updateMatrix);
		void								getPosition							(vector2&	position);
		void								getScale							(vector2&	scale   );
		void								getRotation							(float&		rotation);
		void								setPosition							(vector2* newPosition, bool updateMatrix);
		void								setScale							(vector2* newScale   , bool updateMatrix);
		void								setScaleRotPos						(float sx, float sy, float rz, float tx, float ty);
		void								setScale							(float xy, bool updateMatrix)			{ setScale(xy, xy, updateMatrix); };
											
	protected:								
		matControlMode 						mode								= matControlMode::posRotSca;
		vector2								translation							= vector2(0, 0);						// position of whole object in space
		vector2								scale								= vector2(1, 1);						// scale of whole object in space
		float								rotation							= 0;									// position of whole object in space
		matrix								mat;																		// controlled matrix
		bool								dirtyBit							= true;									// could be unified in matrixDirty object
											
		void								recalcMatrix						();
	};

	// object that describes a camera (position, lookAt, etc.)
	class camera
	{
	friend class masterMind;

	private:

		const float							minFieldOfViewX						= wwc::PI/10.0f;
		const float							maxFieldOfViewX						= wwc::PI/1.5f;
		masterMind*							ww									= nullptr;

		void								windowSizeChanged					(int x, int y);

	public:
		static camera						defaultCamera;																// should be private, but friend class not working properly for masterMind

		vector3								position							= vector3(0,0,0);						// position in space of the camera
		vector3								lookAt								= vector3(0,0,-10);						// 
		vector3								moveRight							= vector3(1,0,0);						// 
		vector3								moveForward							= vector3(0,0,-1);						// 
		vector3								worldUp								= vector3(0,1,0);						// forward, right, up
		vector3								perspectiveTranslation				= vector3(0,0,0);						// ???
		bool								worldUpIsY							= true;									// TRUE means that WorldUp is (0,1,0)
		float								fieldOfViewX						= wwc::PI/2;							// field of view angle in y-direction
		float								fieldOfViewY						= 0;									// calculated from fieldOfViewX and aspectRatio
		float								zNear								= 0.01f;								// objects nearer than this distance to the camera are invisible
		float								zFar								= 1000.0f;								// objects farer  than this distance to the camera are invisible
		float								aspectRatio							= 0;									// ratio width/height

		void								create								(masterMind* ww);
		static void							makeCameraFromViewMatrix			(matrix& mat, camera& cam);
		void								makeProjMatrixFromCamera			(matrix& mat);
		void								makeViewMatrixFromCamera			(matrix& mat);
		void								rotate								(float Yaw, float Pitch, float Roll);
		void								setFieldOfViewX						(float newFieldOfViewX);
		void								setPosition							(vector3 newPosition);
		void								setLookAt							(vector3 newLookAt);
		void								setAsActiveCamera					();
		bool								isFaceVisible						(matrix& matObject);
		vector3								getPosition							() { return position; };
	};

	// frames per seconds
	class screenInformation : public genericObject3D
	{
	friend class masterMind;

	public:
		void								showFramesPerSecond					(bool show, font2D* textFont);

		void								renderSprites						(ssf::sharedVars& v);

	private:
											screenInformation					(masterMind* ww);

		masterMind*							ww									= nullptr;
		bool								showFPS								= false;
		font2D*								font;
	};	

	// a background and a rotating shape/sprite and a percentage counter 
	class loadingScreen : public genericObject3D
	{
	public:
											loadingScreen						(masterMind* ww);

		void								show								(void loadFunc(void* pUser), void* pUser);
		bool								setTextures							(texture* texBackground, texture* texSymbol, font2D* textFont);
		void								setCompletionFraction				(float fraction);
		void								progress							();
		inline bool							isActive							() { return active; };
		void								getMeasuredFractions				(vector<float>& frac);
		void								setMeasuredFractions				(vector<float>& frac);

		void								init								(ssf::shape::genericShape* rotatingShape, POINT& windowSize);
	
	private:
		masterMind*							ww									= nullptr;			// parent
		bool								active								= true;				// default is on
		float								rotationAngle						= 0;				// in rad
		float								rotationSpeed						= 1;				// in rad per seconds
		float								completionFraction					= 0;				// 1 means loading is complete
		bool								measuringFractions					= true;				// when function progress() is called than it indicates if the fractions has already been measured or not
		bool								progressFinished					= false;			// ... work around for strange crash with fractionItr++;
		vector<float>						fractions;												// each time function progress() is called one float is taken and passed to setCompletionFraction()
		vector<float>::iterator				fractionItr;											// current considered float in 'fractions'
		wstring								text;													// text showed within the loadings screen
		POINT*								windowSize							= nullptr;			// 
		font2D*								font								= nullptr;			// 
		texture*							background							= nullptr;			// 
		texture*							rotatingTexture						= nullptr;			// 
		ssf::sprite*						rotatingSprite						= nullptr;			// 
		ssf::shape::genericShape*			rotatingShape						= nullptr;			//

		void								update								(matrix& matView, tickCounter const& timer);
		void								render								(ssf::sharedVars& v);
		void								createDeviceDependentResources		(ssf::sharedVars& v);
		void								createWindowSizeDependentResources	(matrix& matProjection);
		void								onDeviceLost						();
		void								renderSprites						(ssf::sharedVars& sprite);
	};
	
	// class describing the 3D cursor 
	class cursorClass3D : public genericObject3D, public matrixControl3D
	{
	friend class masterMind;

	public:
		enum class							cursorControlMode					{ controlByOwner, automaticControl };
											
	protected:								
		void								update								(matrix& matView, tickCounter const& timer);
		void								render								(ssf::sharedVars& v);
		void								createDeviceDependentResources		(ssf::sharedVars& v);
		void								createWindowSizeDependentResources	(matrix& matProjection);
		void								onDeviceLost						();
											
	private:								
		cursorControlMode					controlMode							= cursorControlMode::automaticControl;
		POINT*								windowSize							= nullptr;
		color								mainColor							= color::white;		// white
		float								speed								= 1.0f;				// in space units per ???
		float								distanceToCamera					= 20;				// in space units 
		bool								isVisible							= true;				// default visibility
		bool								initialized							= false;			// initialization is triggered by owner
											
		ssf::shape::cylinder				gfx_cylinder;											// geometric effect for rendering a box  
		ssf::shape::cone					gfx_cone;												// geometric effect for rendering a box  

	public:									
		void								calcCursorPos						(vector3& cursorPosInSpace, camera& theCamera, LONG xPos, LONG yPos);
		void								setControlMode						(cursorControlMode newMode);
		void								setCursorSpeed						(float newSpeed);
		void								setDistanceToCamera					(float newDistance);
		void								setCursorVisibility					(bool visible);
		void								init								(masterMind* ww);
	};										
											
	// 
	class cursorClass2D						
	{										
		masterMind*							ww;
											
	public:									
		long								x, y;
											
											cursorClass2D						(masterMind* ww);
		void								setCursor							(cursorType newType);
	};

	// a grid in space for measuring the size of objects in space
	class grid3D : public genericObject3D
	{
	private:
		ssf::line3D							lines;

	public:
		void								create								(masterMind* ww);
		void								draw3Grids							();
		void								update								(matrix& matView, tickCounter const& timer)	{ lines.update(matView, timer);};
		void								createWindowSizeDependentResources	(matrix& matProjection)						{ lines.createWindowSizeDependentResources(matProjection); };
		void								createDeviceDependentResources		(ssf::sharedVars& v)						{ lines.createDeviceDependentResources(v); };
		void								onDeviceLost						()											{ lines.onDeviceLost(); };
	};

	// 
	class resourceManager
	{
	friend class masterMind;
	
	public:
		bool								performResourceUpload				();
		void								setAutomaticResourceUpload			(bool automaticUpload);
		void								setResourceUploadDirty				();
		void								uploadIfNecessary					();

	private:
											resourceManager						(masterMind* ww)	{this->ww = ww;};

		masterMind*							ww									= nullptr;
		bool								autoResourceUpload					= true;
		bool								resourceUploadIsDirty				= false;
		bool								performingRessourceUpload			= false;
		std::mutex							resourceUploadMutex;
	};

	// helper functions
	matrix									matrixCreateFromTriad				(vector3& vx, vector3& vy, vector3& vz);
	void									matrixScaleRotateTranslate			(matrix* m, float sx, float sy, float sz, float rx, float ry, float rz, float tx, float ty, float tz);
	void									solveSystemOfLinearEquations		(vector3 e, vector3 a, vector3 b, vector3 c, float* x, float* y, float* z);

} // namespace wildWeasel

