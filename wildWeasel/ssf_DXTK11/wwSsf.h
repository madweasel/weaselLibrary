/*********************************************************************\
	wwSsf.h (Implementation: DirectX Tool Kit 11)
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/

#pragma once

#include "..\\wildWeasel.h"

namespace wildWeasel
{
	// external classes
	class genericObject3D;
	class texture;

namespace ssf
{
	// used namespaces
	using namespace DirectX::SimpleMath;			// for DXTK
	using Microsoft::WRL::ComPtr;					// for DXTK
	
	// redefinition for shorter type names
    using dxInputLayout						= Microsoft::WRL::ComPtr<ID3D11InputLayout>;
	using dxTexture							= Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>;
	using dxDevice							= ID3D11Device*;
	using dxContext							= ID3D11DeviceContext1*;
	using dxDeviceResources					= unique_ptr<DX::DeviceResources>;
	using dxSpriteFont						= unique_ptr<DirectX::SpriteFont> ;
	using dxSpriteBatch						= unique_ptr<DirectX::SpriteBatch>;
	using dxGeoPrimitive					= unique_ptr<DirectX::GeometricPrimitive>;
	using dxBasicEffect						= unique_ptr<DirectX::BasicEffect>;
	using dxAlphaEffect						= unique_ptr<DirectX::AlphaTestEffect>;
	using dxBatchPosCol						= unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>;
	using dxCommonStates					= unique_ptr<DirectX::CommonStates>;
	using dxKeyboard						= unique_ptr<DirectX::Keyboard>;
	using dxMouse							= unique_ptr<DirectX::Mouse>;
	using dxKeyTracker						= DirectX::Keyboard::KeyboardStateTracker;
	using dxColor							= DirectX::XMVECTORF32;
	// using dxAudioEngine						= unique_ptr<DirectX::AudioEngine>;

	// internal classes
	class textureResource;
	class genericDevice;
	class graphicDevice;
	class mouseDevice;
	class keyboardDevice;
	class soundDevice;
	class sound;
	class text2D;
	class text3D;
	class line3D;
	class sprite;
	class sharedShapes;
	class sharedVars;

	// shared objects used by different shapes
	class sharedShapes
	{
	public:
		dxGeoPrimitive						geoPrim_rect;
		dxGeoPrimitive						geoPrim_teapot;
		dxGeoPrimitive						geoPrim_cube;
		dxGeoPrimitive						geoPrim_cylinder;
		dxGeoPrimitive						geoPrim_cone;
		dxGeoPrimitive						geoPrim_sphere;

		void								createDeviceDependentResources		(sharedVars& v);
		void								onDeviceLost						();
	};

	// this structure is passed to a bunch of graphic functions
	class sharedVars {
	public:
		dxCommonStates						commonStates;
		dxDeviceResources					deviceResources;
		dxDevice							device;
		dxSpriteBatch						spriteBatch;
		dxContext							context;
		sharedShapes						shapes;
		std::mutex							renderMutex;
	};

	class system
	{
	protected:
		masterMind*							ww									= nullptr;						// vlt. eher zeiger auf gebrauchte variablen als "generalschlüssel"
		bool								initialized							= false;

											system								(masterMind* ww) : ww{ww}, initialized{false} {};
		virtual								~system								() { destroy(); };
											
	public:
		bool								init								() { initialized = true;  return true; };
		void								destroy								() { initialized = false; };
		void								process								() { };
		bool								isInitialized						() { return initialized; };
	};

	class genericDevice
	{
	protected:
		masterMind*							ww									= nullptr;						// vlt. eher zeiger auf gebrauchte variablen als "generalschlüssel"
		bool								initialized							= false;

											genericDevice						(masterMind* ww) : ww{ww}, initialized{false} {};
		virtual								~genericDevice						() { destroy(); };
											
	public:
		virtual bool						init								() { initialized = true;  return true; };
		virtual void						destroy								() { initialized = false; };
		virtual void						process								() {};
		bool								isInitialized						() {return initialized; };

		virtual void						onActivated							() {};
		virtual void						onDeactivated						() {};
		virtual void						onSuspending						() {};
		virtual void						onResuming							() {};
		virtual void						onWindowMoved						() {};
		virtual void						onWindowSizeChanged					(int width, int height) {};
	};

	class graphicDevice : public DX::IDeviceNotify, public genericDevice
	{
	friend class camera;

	private:
		textureResource*					backgroundTexture					= nullptr;
		
		// IDeviceNotify
		virtual void						OnDeviceLost						() override;
		virtual void						OnDeviceRestored					() override;

		// Rendering
		void								Clear								();
		void								Render								();

	public:
		sharedVars							sharedVars;
				
		// Initialization and management
											graphicDevice						(masterMind* ww) : genericDevice{ww}  {};

		bool								init								() override;
		void								destroy								() override;
		void								process								() override;

		void								setBackground						(texture* backgroundTexture);
		void								CreateDeviceDependentResources		();
		void								CreateWindowSizeDependentResources	();

		// Events
		void								onActivated							() override;
		void								onDeactivated						() override;
		void								onSuspending						() override;
		void								onResuming							() override;
		void								onWindowMoved						() override;
		void								onWindowSizeChanged					(int width, int height) override;
	};

	class mouseDevice : public genericDevice
	{
	public:
											mouseDevice							(masterMind* ww) : genericDevice{ww}  {};

		bool								init								() override;
		void								destroy								() override;
		void								process								() override;

	private:
		dxMouse								mouse;
	};

	class keyboardDevice : public genericDevice
	{
	public:
											keyboardDevice						(masterMind* ww) : genericDevice{ww} {};

		bool								init								() override;
		void								destroy								() override;
		void								process								() override;

		void								onResuming							() override;

	private:
		dxKeyTracker						keyboardButtons;
		dxKeyboard							keyboard;
	};

	class audioDevice : public genericDevice
	{
	public:
											audioDevice							(masterMind* ww) : genericDevice{ww}  {};

		bool								init								() override;
		void								destroy								() override;
		void								process								() override;

		void								onSuspending						() override;
		void								onResuming							() override;
		void								onNewAudioDevice					();
		
	private:
		//HDEVNOTIFY						hNewAudio							= nullptr;
		//dxAudioEngine						audioEngine;
		//uint32_t							audioEvent;
		//float								audioTimerAcc;
		//bool								retryDefault;
	};

	class sound
	{
	public: 
		void								load								(wstring& filename);
		void								play								();
		void								stop								();
		void								setRelativeVolume					();
	};

	// for displaying 2D texts
	class text2D : public genericObject3D
	{
	protected:
		wstring								strFileName;
		dxSpriteFont						gfx_font;
		dxSpriteBatch*						gfx_sprites;
		vector<DirectX::SpriteFont::Glyph>	glyphs								{WCHAR_MAX};
		vector<vector2>						charSize							{WCHAR_MAX};
											
	public:									
		void								draw								(wchar_t const* text, vector2 const& position, color color, float rotation = 0.0f, vector2 const& origin = vector2{0,0}, vector2 const& scale = vector2{1,1}, float layerDepth = 1, fRECT* clippingRect = nullptr, const float lineSpacing = -1.0f, bool applyOffsetOnFirstCharacter = true);
		fRECT								measureDrawBounds					(wchar_t const* text, vector2 const& position = vector2{0,0});
		float								getLineSpacing						();

		void								createDeviceDependentResources		(sharedVars& v);
		vector2 const &						getCharSize							(WCHAR c);
	};

	// TODO: Implement text3D, which is not provided by DirectX TK
	class text3D
	{

	};

	//
	class sprite : public genericObject3D
	{
	private:
		sharedVars*							v									= nullptr;

	public: 
		void								draw								(texture& tex, fRECT& destinationRectangle, fRECT* sourceRectangle, color col = color::white, float rotation = 0, vector2 const& origin = vector2{0,0},								  float layerDepth = 0, bool flipVertically = false, bool flipHorizontally = false);
		void								draw								(texture& tex, vector2 const& position,		fRECT* sourceRectangle, color col = color::white, float rotation = 0, vector2 const& origin = vector2{0,0}, vector2 scale = vector2{1,1}, float layerDepth = 0, bool flipVertically = false, bool flipHorizontally = false);
		void								createDeviceDependentResources		(sharedVars & v);
	};

	//
	namespace shape
	{
		class genericShape : public genericObject3D
		{
		protected:
			sharedVars*						v									= nullptr;
			dxGeoPrimitive*					geoPrim								= nullptr;
			dxBasicEffect					basicEffect;
			dxInputLayout					inputLayout;

		public: 
			void							draw								();
			void							createDeviceDependentResources		(sharedVars & v);
			void							createWindowSizeDependentResources	(matrix & matProjection);
			void							onDeviceLost						();
			void							setTexture							(texture& tex);
			void							setWorld							(matrix const& matWorld);
			void							setView								(matrix const& matView);
			void							setProj								(matrix const& matProjection);
			void							setColor							(color const& col);
		};

		class rect : public genericShape
		{
		public:
			void							createDeviceDependentResources		(sharedVars & v) {geoPrim = &(v.shapes.geoPrim_rect);		genericShape::createDeviceDependentResources(v); };
		};

		class cube : public genericShape
		{
		public:
			void							createDeviceDependentResources		(sharedVars & v) {geoPrim = &(v.shapes.geoPrim_cube);		genericShape::createDeviceDependentResources(v); };
		};

		class cylinder : public genericShape
		{
		public:
			void							createDeviceDependentResources		(sharedVars & v) {geoPrim = &(v.shapes.geoPrim_cylinder);	genericShape::createDeviceDependentResources(v); };
		};

		class cone : public genericShape
		{
		public:
			void							createDeviceDependentResources		(sharedVars & v) {geoPrim = &(v.shapes.geoPrim_cone);		genericShape::createDeviceDependentResources(v); };
		};

		class sphere : public genericShape
		{
		public:
			void							createDeviceDependentResources		(sharedVars & v) {geoPrim = &(v.shapes.geoPrim_sphere);		genericShape::createDeviceDependentResources(v); };
		};

		class teapot : public genericShape
		{
		public:
			void							createDeviceDependentResources		(sharedVars & v) {geoPrim = &(v.shapes.geoPrim_teapot);		genericShape::createDeviceDependentResources(v); };
		};

		// TODO: To be implemented
		class custom : public genericShape
		{
		public:
			void							createDeviceDependentResources		(sharedVars & v) {genericShape::createDeviceDependentResources(v); geoPrim = nullptr; };
		};
	}

	//
	class line3D : public genericObject3D
	{
	public:
		void								createDeviceDependentResources		(sharedVars & v);
		void								createWindowSizeDependentResources	(matrix & matProjection);
		void								onDeviceLost						();
		void								update								(matrix & matView, tickCounter const & timer);
		void								drawGrid							(vector3 const& xAxis, vector3 const& yAxis, vector3 const& origin, size_t xdivs, size_t ydivs, color const& color);
	
	private:
		sharedVars*							v									= nullptr;
		dxBatchPosCol						batch;
		dxBasicEffect						lineEffect;
		dxInputLayout						inputLayout;
		
		void XM_CALLCONV					DrawGrid							(sharedVars & v, DirectX::FXMVECTOR xAxis, DirectX::FXMVECTOR yAxis, DirectX::FXMVECTOR origin, size_t xdivs, size_t ydivs, DirectX::GXMVECTOR color);
	};
	
	// 
	class textureResource
	{
	friend class graphicDevice;
	friend class shape::genericShape;
	friend class sprite;
	friend class text2D;

	private:
		dxTexture							dxTexRes;

		POINT								getSize_POINT						();
		DirectX::XMUINT2					getSize_XMUINT2						();
		DirectX::XMFLOAT2					getSize_XMFLOAT2					();
		RECT								getSize_RECT						();
		static DirectX::XMUINT2				GetTextureSize						(ID3D11ShaderResourceView * texture);
		
	public: 
		void								load								(sharedVars& v, bitmap& bm);
		void								load								(sharedVars& v, wstring& filename);
		void								onDeviceLost						();
		vector2								getSize_vector2						();
	};

	// helper functions
	dxColor									getDxColor							(color const& col);
	
} // namespace ssf 

} // namespace wildWeasel

