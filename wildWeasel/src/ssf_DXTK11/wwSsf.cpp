/*********************************************************************
	wwSSf.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/

#include "../wildWeasel.h"

/*******************************************************************************************/

#pragma region graphic device

#pragma region Initialization
//-----------------------------------------------------------------------------
// Name: init()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::ssf::graphicDevice::init()
{
	// ready?
	if (initialized) return false;

	// Indicates if the DirectXMath Library supports the current platform.
	if (!DirectX::XMVerifyCPUSupport()) return false;

	// ???
	sharedVars.deviceResources = std::make_unique<DX::DeviceResources>();
    sharedVars.deviceResources->RegisterDeviceNotify(this);
    sharedVars.deviceResources->SetWindow(ww->getHwnd(), ww->getWindowSizeX(), ww->getWindowSizeY());
    sharedVars.deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();
    sharedVars.deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

	initialized = true;
	return true;
}

//-----------------------------------------------------------------------------
// Name: destroy()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::graphicDevice::destroy()
{
	initialized = false;
}
#pragma endregion

#pragma region Frame Update
//-----------------------------------------------------------------------------
// Name: process()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::graphicDevice::process()
{
	// ready?
	if (!initialized) return;

    Render();
}
#pragma endregion

#pragma region Frame Render
//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Draws the scene.
//-----------------------------------------------------------------------------
void wildWeasel::ssf::graphicDevice::Render()
{
    // Don't try to render anything before the first Update.
    if (ww->stepTimer.GetFrameCount() == 0) {
        return;
    }

	// do not allow any resource upload now
	sharedVars.renderMutex.lock();

    // Prepare the command list to render a new frame.
	Clear();
    sharedVars.deviceResources->PIXBeginEvent(L"Render");
    auto context = sharedVars.deviceResources->GetD3DDeviceContext();

	// Draw background
	if (backgroundTexture) {
		RECT backgroundRect = { 0, 0, ww->getWindowSizeX(), ww->getWindowSizeY() };
	    sharedVars.deviceResources->PIXBeginEvent(L"Draw background");
		sharedVars.spriteBatch->Begin();
		sharedVars.spriteBatch->Draw(backgroundTexture->dxTexRes.Get(), backgroundRect, nullptr, DirectX::Colors::White, 0, DirectX::XMFLOAT2(0,0), DirectX::SpriteEffects_None, 1.0f);
		sharedVars.spriteBatch->End();
		sharedVars.deviceResources->PIXEndEvent();
	}

	ww->renderShapes();

	// Draw 2D sprites
	sharedVars.deviceResources->PIXBeginEvent(L"Draw sprite");
	sharedVars.spriteBatch->Begin(DirectX::SpriteSortMode_Deferred, sharedVars.commonStates->NonPremultiplied());

	ww->renderSprites();

	// End sprite drawing
	sharedVars.spriteBatch->End();
    sharedVars.deviceResources->PIXEndEvent();

    // Show the new frame.
    sharedVars.deviceResources->Present();

	// release lock
	sharedVars.renderMutex.unlock();
}

//-----------------------------------------------------------------------------
// Name: Clear()
// Desc: Helper method to clear the back buffers.
//-----------------------------------------------------------------------------
void wildWeasel::ssf::graphicDevice::Clear()
{
	sharedVars.deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context		= sharedVars.deviceResources->GetD3DDeviceContext();
    auto renderTarget	= sharedVars.deviceResources->GetRenderTargetView();
    auto depthStencil	= sharedVars.deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, DirectX::Colors::CornflowerBlue);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto viewport = sharedVars.deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

	sharedVars.deviceResources->PIXEndEvent();
}
#pragma endregion

#pragma region Message Handlers
//-----------------------------------------------------------------------------
// Name: destroy()
// Desc: These are the resources that depend on the device.
//-----------------------------------------------------------------------------
void wildWeasel::ssf::graphicDevice::CreateDeviceDependentResources()
{
	// common stuff
	sharedVars.context		= sharedVars.deviceResources->GetD3DDeviceContext();
	sharedVars.device		= sharedVars.deviceResources->GetD3DDevice();
    sharedVars.commonStates	= std::make_unique<DirectX::CommonStates>(sharedVars.device);
	sharedVars.spriteBatch	= std::make_unique<DirectX::SpriteBatch>(sharedVars.context);
	sharedVars.shapes.createDeviceDependentResources(sharedVars);
	
	ww->createDeviceDependentResources();
}

//-----------------------------------------------------------------------------
// Name: destroy()
// Desc: Allocate all memory resources that change on a window SizeChanged event.
//-----------------------------------------------------------------------------
void wildWeasel::ssf::graphicDevice::CreateWindowSizeDependentResources()
{
	// SpriteBatch for 2D sprites
	auto viewport = sharedVars.deviceResources->GetScreenViewport();
	sharedVars.spriteBatch->SetViewport(viewport);

	ww->createWindowSizeDependentResources();
}

//-----------------------------------------------------------------------------
// Name: OnDeviceLost()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::graphicDevice::OnDeviceLost()
{
	sharedVars.spriteBatch.reset();
    sharedVars.commonStates.reset();
	sharedVars.shapes.onDeviceLost();

	ww->onDeviceLost();
}

//-----------------------------------------------------------------------------
// Name: OnDeviceRestored()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::graphicDevice::OnDeviceRestored()
{
    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();
}
//-----------------------------------------------------------------------------
// Name: OnActivated()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::graphicDevice::onActivated()
{
	if (!initialized) return;
}

//-----------------------------------------------------------------------------
// Name: OnDeactivated()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::graphicDevice::onDeactivated()
{
	if (!initialized) return;
}

//-----------------------------------------------------------------------------
// Name: OnSuspending()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::graphicDevice::onSuspending()
{
	if (!initialized) return;
}

//-----------------------------------------------------------------------------
// Name: OnResuming()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::graphicDevice::onResuming()
{
	if (!initialized) return;
}

//-----------------------------------------------------------------------------
// Name: OnResuming()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::graphicDevice::onWindowMoved()
{
	if (!initialized) return;
    auto r = sharedVars.deviceResources->GetOutputSize();
    sharedVars.deviceResources->WindowSizeChanged(r.right, r.bottom);
}

//-----------------------------------------------------------------------------
// Name: OnWindowSizeChanged()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::graphicDevice::onWindowSizeChanged(int width, int height)
{
	if (!initialized) return;
    if (!sharedVars.deviceResources->WindowSizeChanged(width, height)) return;
    CreateWindowSizeDependentResources();
}

//-----------------------------------------------------------------------------
// Name: OnWindowSizeChanged()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::graphicDevice::setBackground(texture* backgroundTexture)
{
	this->backgroundTexture = (backgroundTexture ? backgroundTexture->getTextureResource() : nullptr); 
};
#pragma endregion

#pragma endregion

/*******************************************************************************************/

#pragma region audio device

#pragma region Initialization
//-----------------------------------------------------------------------------
// Name: init()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::ssf::audioDevice::init()
{
	/* Ask for notification of new audio devices
    DEV_BROADCAST_DEVICEINTERFACE filter = { 0 };
    filter.dbcc_size = sizeof(filter);
    filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    filter.dbcc_classguid = KSCATEGORY_AUDIO;
    hNewAudio = RegisterDeviceNotification(ww->hWnd, &filter, DEVICE_NOTIFY_WINDOW_HANDLE);

    // Create DirectXTK for Audio objects
    DirectX::AUDIO_ENGINE_FLAGS eflags = DirectX::AudioEngine_Default;
	#ifdef _DEBUG
		eflags = eflags | DirectX::AudioEngine_Debug;
	#endif
	// BUG: When audio device does not work properly in Windows then this line crashes!
    audioEngine		= std::make_unique<DirectX::AudioEngine>(eflags);
    audioEvent		= 0;
    audioTimerAcc	= 10.f;
    retryDefault	= false;*/

    //    if (!hNewAudio) {
    //        // Ask for notification of new audio devices
    //        DEV_BROADCAST_DEVICEINTERFACE filter = { 0 };
    //        filter.dbcc_size = sizeof(filter);
    //        filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    //        filter.dbcc_classguid = KSCATEGORY_AUDIO;
    //        hNewAudio = RegisterDeviceNotification(hWnd, &filter, DEVICE_NOTIFY_WINDOW_HANDLE);
    //    }

	return true;
}

//-----------------------------------------------------------------------------
// Name: destroy()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::audioDevice::destroy()
{
	/*if (hNewAudio) {
        UnregisterDeviceNotification(hNewAudio);
        hNewAudio = nullptr;
    } 
	
	if (audioEngine) {
        audioEngine->Suspend();
    }*/

}
#pragma endregion

#pragma region events
//-----------------------------------------------------------------------------
// Name: process()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::audioDevice::process()
{
	if (!initialized) return;

    /* Only update audio engine once per frame
    if (!audioEngine->IsCriticalError() && audioEngine->Update()) {
        // Setup a retry in 1 second
        audioTimerAcc = 1.f;
        retryDefault = true;
    }*/
}

//-----------------------------------------------------------------------------
// Name: OnSuspending()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::audioDevice::onSuspending()
{
	if (!initialized) return;
    //audioEngine->Suspend();
}

//-----------------------------------------------------------------------------
// Name: OnResuming()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::audioDevice::onResuming()
{
    //audioEngine->Resume();
}

//-----------------------------------------------------------------------------
// Name: NewAudioDevice()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::audioDevice::onNewAudioDevice()
{
    /*if (audioEngine && !audioEngine->IsAudioDevicePresent())
    {
        // Setup a retry in 1 second
        audioTimerAcc = 1.f;
        retryDefault = true;
    }*/
}

/*** Audio & Sound ******************************************************

	// vars
	unique_ptr<DirectX::WaveBank>										m_waveBank;
	unique_ptr<DirectX::SoundEffect>									m_soundEffect;
	unique_ptr<DirectX::SoundEffectInstance>							m_effect1;
	unique_ptr<DirectX::SoundEffectInstance>							m_effect2;

	// load audio files
	m_waveBank		= std::make_unique<DirectX::WaveBank>(audioEngine.get(), L"assets\\adpcmdroid.xwb");
	m_soundEffect	= std::make_unique<DirectX::SoundEffect>(audioEngine.get(), L"assets\\MusicMono_adpcm.wav");
	m_effect1		= m_soundEffect->CreateInstance();
	m_effect2		= m_waveBank->CreateInstance(10);
	m_effect1->Play(true);
	m_effect2->Play();

	// update()
	audioTimerAcc -= (float)timer.GetElapsedSeconds();
	if (audioTimerAcc < 0) {
	    if (retryDefault) {
	        retryDefault = false;
	        if (audioEngine->Reset()) {
	            // Restart looping audio
	            m_effect1->Play(true);
	        }
	    } else {
	        audioTimerAcc = 4.f;
	        m_waveBank->Play(audioEvent++);
	
	        if (audioEvent >= 11)
	            audioEvent = 0;
	    }
	}

*************************************************************************/

#pragma endregion
#pragma endregion

/*******************************************************************************************/

#pragma region mouse device

//-----------------------------------------------------------------------------
// Name: init()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::ssf::mouseDevice::init()
{
	mouse		= std::make_unique<DirectX::Mouse>();
    mouse->SetWindow(ww->getHwnd());

	return true;
}

//-----------------------------------------------------------------------------
// Name: destroy()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::mouseDevice::destroy()
{
}

//-----------------------------------------------------------------------------
// Name: process()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::mouseDevice::process()
{
    auto mouseState = mouse->GetState();
}
#pragma endregion

/*******************************************************************************************/

#pragma region keyboard device

#pragma region Initialization

//-----------------------------------------------------------------------------
// Name: init()
// Desc: 
//-----------------------------------------------------------------------------
bool wildWeasel::ssf::keyboardDevice::init()
{
	keyboard	= std::make_unique<DirectX::Keyboard>();

	return true;
}

//-----------------------------------------------------------------------------
// Name: destroy()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::keyboardDevice::destroy()
{
}

//-----------------------------------------------------------------------------
// Name: process()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::keyboardDevice::process()
{
	// process keyboard input
	auto kb = keyboard->GetState();
	keyboardButtons.Update(kb);
	// if (kb.Escape) {
	// 	PostQuitMessage(0);
	// }
}

//-----------------------------------------------------------------------------
// Name: OnResuming()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::keyboardDevice::onResuming()
{
    keyboardButtons.Reset();
}
#pragma endregion

#pragma endregion

/*******************************************************************************************/

#pragma region text2D
//-----------------------------------------------------------------------------
// Name: measureDrawBounds()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::fRECT wildWeasel::ssf::text2D::measureDrawBounds(wchar_t const * text, vector2 const & position)
{
	return fRECT{gfx_font->MeasureDrawBounds(text, DirectX::XMFLOAT2{position.x, position.y})};
}

//-----------------------------------------------------------------------------
// Name: getLineSpacing()
// Desc: 
//-----------------------------------------------------------------------------
float wildWeasel::ssf::text2D::getLineSpacing()
{
	return gfx_font->GetLineSpacing();
}

//-----------------------------------------------------------------------------
// Name: createDeviceDependentResources()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::text2D::createDeviceDependentResources(sharedVars& v)
{
	gfx_font = std::make_unique<DirectX::SpriteFont>(v.device, strFileName.c_str());
	gfx_sprites	= &v.spriteBatch;

	// ... work-around since FindGlyph() is slow
	for (wchar_t curChar = 0; curChar < WCHAR_MAX; curChar++) {
		if (gfx_font->ContainsCharacter(curChar)) {
			glyphs[curChar] = *gfx_font->FindGlyph(curChar);
		}
	}

	// make char size table
	DirectX::SpriteFont::Glyph const* pGlyph;
	for (WCHAR curChar = 0; curChar < WCHAR_MAX; curChar++) {
		if (curChar != '\n' && glyphs[curChar].Character) {
			pGlyph = &glyphs[curChar];
			charSize[curChar].x	 = (pGlyph->Subrect.right - pGlyph->Subrect.left + pGlyph->XAdvance);
			if (pGlyph->XOffset > 0) {
				charSize[curChar].x	 += pGlyph->XOffset;
			}
			charSize[curChar].y	 = (float) pGlyph->Subrect.bottom - pGlyph->Subrect.top;
		} else {
			charSize[curChar].x	 = 0;
			charSize[curChar].y	 = 0;
		}
	}
}

//-----------------------------------------------------------------------------
// Name: createDeviceDependentResources()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::vector2 const& wildWeasel::ssf::text2D::getCharSize(WCHAR c)
{
	return charSize[c];
}

//-----------------------------------------------------------------------------
// Name: customDrawString()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::text2D::draw(wchar_t const* text, vector2 const& position, color color, float rotation, vector2 const& origin, vector2 const& scale, float layerDepth, fRECT* clippingRect, const float lineSpacing, bool applyOffsetOnFirstCharacter)
{
	// locals
	ID3D11ShaderResourceView*			pTex		= nullptr;
	vector2								myOrigin	= origin;
	DirectX::XMFLOAT2					curPos		= {0, 0};
	DirectX::SpriteFont::Glyph const*	pGlyph		= nullptr;
	unsigned int						posChar		= 0;
	WCHAR								curChar		= text[posChar];
	float								myLineSpace = (lineSpacing < 0.0f ? getLineSpacing() : lineSpacing);
	float								lineSpaces	= 0;
	fRECT								destRect;
	fRECT								srcRect;
	fRECT								clipRect;

	gfx_font->GetSpriteSheet(&pTex);
	
	// process each char in 'text'
	while (curChar != '\0') {

		switch (curChar)
        {
        case '\r':
            // Skip carriage returns.
            break;

        case '\n':
            // New line.
            curPos.x = 0;
            lineSpaces += myLineSpace;
            break;

        default:
			pGlyph = &glyphs[curChar];

			if (applyOffsetOnFirstCharacter || posChar > 0) {
				curPos.x += pGlyph->XOffset * scale.x;
			}

			if (curPos.x < 0) {
				curPos.x = 0;
			}

			float advance = (pGlyph->Subrect.right - pGlyph->Subrect.left + pGlyph->XAdvance) * scale.x;

			if ( !iswspace(curChar) || ( ( pGlyph->Subrect.right - pGlyph->Subrect.left ) > 1 ) || ( ( pGlyph->Subrect.bottom - pGlyph->Subrect.top ) > 1 ) ) {

				// TODO: Calculation should be rewritten from the scratch
				curPos.y			= (pGlyph->YOffset + (applyOffsetOnFirstCharacter ? 0 : -5) + lineSpaces) * scale.y;
				myOrigin			= origin - curPos;
				srcRect				= pGlyph->Subrect;

				destRect.left		= position.x;
				destRect.top		= position.y;
				destRect.right		= destRect.left + scale.x * (pGlyph->Subrect.right - pGlyph->Subrect.left);
				destRect.bottom		= destRect.top  + scale.y * (pGlyph->Subrect.bottom - pGlyph->Subrect.top);

				// render
				if (!clippingRect || (clippingRect && performClippingAndCheck(destRect, srcRect, myOrigin, *clippingRect/*, rotation*/))) {

					// 
					vector2 aVector, myPos;
					aVector.x			= cos(rotation) * myOrigin.x - sin(rotation) * myOrigin.y;
					aVector.y			= sin(rotation) * myOrigin.x + cos(rotation) * myOrigin.y;
					myPos.x				= destRect.left;
					myPos.y				= destRect.top;
					myPos				-= aVector;
					myOrigin.x			= 0;
					myOrigin.y			= 0;
					RECT tempRect 		= srcRect.getRECT();
					(*gfx_sprites)->Draw(pTex, myPos, &tempRect, getDxColor(color), rotation, myOrigin, scale, DirectX::SpriteEffects_None, layerDepth);
				}
			}

			curPos.x += advance;
		}

		// proceed to next char in 'text'
		posChar++;
		curChar	= text[posChar];
	}
}
#pragma endregion

/*****************************************************************************************/

#pragma region textureResource
//-----------------------------------------------------------------------------
// Name: load()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::textureResource::load(sharedVars& v, bitmap & bm)
{
	DX::ThrowIfFailed(DirectX::CreateWICTextureFromMemory(v.device, (uint8_t*) bm.pBitmap, bm.getTotalSizeInBytes(), nullptr, dxTexRes.ReleaseAndGetAddressOf()));
}

//-----------------------------------------------------------------------------
// Name: load()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::textureResource::load(sharedVars& v, wstring & strFileName)
{
	if (strFileName.find(L".dds", strFileName.size() - 4) != string::npos) {
		DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(v.device, strFileName.c_str(), nullptr, dxTexRes.ReleaseAndGetAddressOf()));
	} else {
		DX::ThrowIfFailed(DirectX::CreateWICTextureFromFile(v.device, strFileName.c_str(), nullptr, dxTexRes.ReleaseAndGetAddressOf()));
	} 
}

//-----------------------------------------------------------------------------
// Name: onDeviceLost()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::textureResource::onDeviceLost()
{
	dxTexRes.Reset();
}

//-----------------------------------------------------------------------------
// Name: getSize()
// Desc: 
//-----------------------------------------------------------------------------
POINT wildWeasel::ssf::textureResource::getSize_POINT()
{
	DirectX::XMUINT2	mySize		= getSize_XMUINT2();
	POINT myPoint = { (LONG) mySize.x, (LONG) mySize.y };
	return myPoint;
}

//-----------------------------------------------------------------------------
// Name: getSize()
// Desc: 
//-----------------------------------------------------------------------------
DirectX::XMUINT2 wildWeasel::ssf::textureResource::getSize_XMUINT2()
{
	if (true/*uploaded*/) {	// uploaded is a member of genericObject3D
		return GetTextureSize(dxTexRes.Get());
	} else {
		return DirectX::XMUINT2(0,0);
	}
}

//-----------------------------------------------------------------------------
// Name: getSize()
// Desc: 
//-----------------------------------------------------------------------------
DirectX::XMFLOAT2 wildWeasel::ssf::textureResource::getSize_XMFLOAT2()
{
	if (true/*uploaded*/) {
		DirectX::XMUINT2 s = GetTextureSize(dxTexRes.Get());
		return DirectX::XMFLOAT2((float) s.x, (float) s.y);
	} else {
		return DirectX::XMFLOAT2(0,0);
	}
}

//-----------------------------------------------------------------------------
// Name: getSize()
// Desc: 
//-----------------------------------------------------------------------------
RECT wildWeasel::ssf::textureResource::getSize_RECT()
{
	DirectX::XMUINT2	mySize		= getSize_XMUINT2();
	RECT myRect = { 0, 0, (LONG) mySize.x, (LONG) mySize.y };
	return myRect;
}

//-----------------------------------------------------------------------------
// Name: getSize()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::vector2 wildWeasel::ssf::textureResource::getSize_vector2()
{
	DirectX::XMUINT2	mySize		= getSize_XMUINT2();
	vector2 mVec2 = { (float) mySize.x, (float) mySize.y };
	return mVec2;
}

//-----------------------------------------------------------------------------
// Name: GetTextureSize()
// Desc: Helper looks up the size of the specified texture.
//-----------------------------------------------------------------------------
DirectX::XMUINT2 wildWeasel::ssf::textureResource::GetTextureSize(ID3D11ShaderResourceView* texture)
{
    // Convert resource view to underlying resource.
    ComPtr<ID3D11Resource> resource;

    texture->GetResource(&resource);
    
    // Cast to texture.
    ComPtr<ID3D11Texture2D> texture2D;
    
    if (FAILED(resource.As(&texture2D))) {
        throw std::logic_error("SpriteBatch can only draw Texture2D resources");
    }

    // Query the texture size.
    D3D11_TEXTURE2D_DESC desc;

    texture2D->GetDesc(&desc);

    return DirectX::XMUINT2(desc.Width, desc.Height);
}

#pragma endregion

/*****************************************************************************************/

#pragma region helper functions

//-----------------------------------------------------------------------------
// Name: getDxColor()
// Desc: 
//-----------------------------------------------------------------------------
wildWeasel::ssf::dxColor wildWeasel::ssf::getDxColor(color const& col)
{
	DirectX::XMVECTORF32 dxtkColor = {col.r, col.g, col.b, col.a};
	return dxtkColor; 
}

#pragma endregion

/*****************************************************************************************/

#pragma region shapes
//-----------------------------------------------------------------------------
// Name: createDeviceDependentResources()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::sharedShapes::createDeviceDependentResources(sharedVars& v)
{
	// locals
	vector<DirectX::VertexPositionNormalTexture>	verticesRect(4);
    vector<uint16_t>								indicesRect(6);
	float											e					= 0.5f;

	vector3 ul(-e, +e, 0);
	vector3 ur(+e, +e, 0);
	vector3 ll(-e, -e, 0);
	vector3 lr(+e, -e, 0);
	vector3 n(0,0,1);

	verticesRect[0] = DirectX::VertexPositionNormalTexture(ul, n, DirectX::XMFLOAT2(0,0));
	verticesRect[1] = DirectX::VertexPositionNormalTexture(ur, n, DirectX::XMFLOAT2(1,0));
	verticesRect[2] = DirectX::VertexPositionNormalTexture(ll, n, DirectX::XMFLOAT2(0,1));
	verticesRect[3] = DirectX::VertexPositionNormalTexture(lr, n, DirectX::XMFLOAT2(1,1));

	indicesRect[0] = 0;
	indicesRect[1] = 1;
	indicesRect[2] = 2;
	indicesRect[3] = 3;
	indicesRect[4] = 1;
	indicesRect[5] = 2;

	geoPrim_cube	 = DirectX::GeometricPrimitive::CreateCube		(v.context, 1.f);
	geoPrim_teapot	 = DirectX::GeometricPrimitive::CreateTeapot	(v.context, 1.f,		8);
	geoPrim_cylinder = DirectX::GeometricPrimitive::CreateCylinder	(v.context, 1.f, 1.f,	8);
	geoPrim_cone	 = DirectX::GeometricPrimitive::CreateCone		(v.context, 1.f, 1.f,	8);
	geoPrim_sphere	 = DirectX::GeometricPrimitive::CreateSphere	(v.context, 1.f,		8);
	geoPrim_rect	 = DirectX::GeometricPrimitive::CreateCustom	(v.context, verticesRect, indicesRect);
}

//-----------------------------------------------------------------------------
// Name: onDeviceLost()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::sharedShapes::onDeviceLost()
{
	geoPrim_teapot	 .reset();
	geoPrim_cube	 .reset();
	geoPrim_cylinder .reset();
	geoPrim_cone	 .reset();
	geoPrim_sphere	 .reset();
	geoPrim_rect	 .reset();
}

//-----------------------------------------------------------------------------
// Name: draw()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::shape::genericShape::draw()
{
	if (!v) return;
	v->context->OMSetBlendState(v->commonStates->Opaque(), nullptr, 0xFFFFFFFF);
	v->context->OMSetDepthStencilState(v->commonStates->DepthDefault(), 0);
	v->context->RSSetState(v->commonStates->CullNone());
	(*geoPrim)->Draw(basicEffect.get(), inputLayout.Get(), false, false, nullptr);
}

//-----------------------------------------------------------------------------
// Name: createDeviceDependentResources()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::shape::genericShape::createDeviceDependentResources(sharedVars& v)
{
	//if (this->v) return;
	this->v = &v;
	basicEffect = std::make_unique<DirectX::BasicEffect>(v.device);
	basicEffect->SetPerPixelLighting(true);
	basicEffect->SetLightingEnabled(true);
	if (false) {
		// basicEffect->EnableDefaultLighting();
	} else {
		const unsigned int numDirectionalLights = 1;
		static const DirectX::XMVECTORF32 defaultDirections[numDirectionalLights] =
		{
			{ -0.20f, -0.20f, -0.80f },
		};

		static const DirectX::XMVECTORF32 defaultDiffuse[numDirectionalLights] =
		{
			{ 0.40f, 0.46f, 0.40f },
		};

		static const DirectX::XMVECTORF32 defaultSpecular[numDirectionalLights] =
		{
			{ 0.60f, 0.46f, 0.30f },
		};

		static const DirectX::XMVECTORF32 defaultAmbient = { 0.5f, 0.5f, 0.5f };

		basicEffect->SetAmbientLightColor(defaultAmbient);
		basicEffect->SetSpecularPower(50.0f);

		for (int i = 0; i < numDirectionalLights; i++) {
			basicEffect->SetLightEnabled(i, true);
			basicEffect->SetLightDirection(i, defaultDirections[i]);
			basicEffect->SetLightDiffuseColor(i, defaultDiffuse[i]);
			basicEffect->SetLightSpecularColor(i, defaultSpecular[i]);
		}
	}
	//basicEffect->SetTextureEnabled(false);	// ... when no texture set, than object will be black
	(*geoPrim)->CreateInputLayout(basicEffect.get(), inputLayout.ReleaseAndGetAddressOf());
}

//-----------------------------------------------------------------------------
// Name: createWindowSizeDependentResources()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::shape::genericShape::createWindowSizeDependentResources(matrix& matProjection)
{
	if (!v) return;
	basicEffect->SetProjection(matProjection);
}

//-----------------------------------------------------------------------------
// Name: onDeviceLost()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::shape::genericShape::onDeviceLost()
{
	if (!v) return;
	basicEffect.reset();
	inputLayout.Reset();
}

//-----------------------------------------------------------------------------
// Name: setTexture()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::shape::genericShape::setTexture(texture& tex)
{
	if (!v) return;

	// do not allow any rendering now
	v->renderMutex.lock();

	basicEffect->SetTexture(tex.getTextureResource()->dxTexRes.Get());
	basicEffect->SetTextureEnabled(true);
	(*geoPrim)->CreateInputLayout(basicEffect.get(), inputLayout.ReleaseAndGetAddressOf());

	// release lock
	v->renderMutex.unlock();
}

//-----------------------------------------------------------------------------
// Name: setWorld()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::shape::genericShape::setWorld(matrix const& matWorld)
{
	if (!v) return;
	basicEffect->SetWorld(matWorld);
}

//-----------------------------------------------------------------------------
// Name: setColor()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::shape::genericShape::setColor(color const& col)
{
	if (!v) return;
	basicEffect->SetColorAndAlpha(getDxColor(col));
}

//-----------------------------------------------------------------------------
// Name: setView()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::shape::genericShape::setView(matrix const& matView)
{
	if (!v) return;
	basicEffect->SetView(matView);
}

//-----------------------------------------------------------------------------
// Name: setView()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::shape::genericShape::setProj(matrix const& matProjection)
{
	if (!v) return;
	basicEffect->SetProjection(matProjection);
}
#pragma endregion

/*****************************************************************************************/

#pragma region line3D
//-----------------------------------------------------------------------------
// Name: loadingScreen::createDeviceDependentResources()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::line3D::createDeviceDependentResources(sharedVars& v)
{
	this->v = &v;

	// PrimitiveBatch (grid) for dynamic mesh
	batch = std::make_unique<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>(v.context);

	// line effect
	//... D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE
	lineEffect = std::make_unique<DirectX::BasicEffect>(v.device);
    lineEffect->SetVertexColorEnabled(true);

	void const* shaderByteCode;
	size_t		byteCodeLength;

	lineEffect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

	DX::ThrowIfFailed(
		v.device->CreateInputLayout(DirectX::VertexPositionColor::InputElements, DirectX::VertexPositionColor::InputElementCount, shaderByteCode, byteCodeLength, inputLayout.ReleaseAndGetAddressOf())
	);
}

//-----------------------------------------------------------------------------
// Name: createWindowSizeDependentResources()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::line3D::createWindowSizeDependentResources(matrix & matProjection)
{
	lineEffect ->SetProjection(matProjection);
}

//-----------------------------------------------------------------------------
// Name: onDeviceLost()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::line3D::onDeviceLost()
{
	batch.reset();
	lineEffect.reset();
	inputLayout.Reset();
}

//-----------------------------------------------------------------------------
// Name: update()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::line3D::update(matrix & matView, tickCounter const & timer)
{
	lineEffect->SetView(matView);
	lineEffect->SetWorld(Matrix::Identity);
}

//-----------------------------------------------------------------------------
// Name: DrawGrid()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::line3D::drawGrid(vector3 const& xAxis, vector3 const& yAxis, vector3 const& origin, size_t xdivs, size_t ydivs, color const& color)
{
	if (!v) return;
	DrawGrid(*v, DirectX::FXMVECTOR{xAxis.x, xAxis.y, xAxis.z}, DirectX::FXMVECTOR{yAxis.x, yAxis.y, yAxis.z}, DirectX::FXMVECTOR{origin.x, origin.y, origin.z}, xdivs, ydivs, DirectX::GXMVECTOR{color.a, color.r, color.g, color.b});
}

//-----------------------------------------------------------------------------
// Name: DrawGrid()
// Desc: 
//-----------------------------------------------------------------------------
void XM_CALLCONV wildWeasel::ssf::line3D::DrawGrid(sharedVars& v, DirectX::FXMVECTOR xAxis, DirectX::FXMVECTOR yAxis, DirectX::FXMVECTOR origin, size_t xdivs, size_t ydivs, DirectX::GXMVECTOR color)
{
    v.context->OMSetBlendState(v.commonStates->Opaque(), nullptr, 0xFFFFFFFF);
    v.context->OMSetDepthStencilState(v.commonStates->DepthNone(), 0);
    v.context->RSSetState(v.commonStates->CullCounterClockwise());
    lineEffect->Apply(v.context);
    v.context->IASetInputLayout(inputLayout.Get());
    batch->Begin();

    xdivs = std::max<size_t>(1, xdivs);
    ydivs = std::max<size_t>(1, ydivs);

    for (size_t i = 0; i <= xdivs; ++i)
    {
        float fPercent = float(i) / float(xdivs);
        fPercent = (fPercent * 2.0f) - 1.0f;
        DirectX::XMVECTOR vScale = DirectX::XMVectorScale(xAxis, fPercent);
        vScale = DirectX::XMVectorAdd(vScale, origin);

        DirectX::VertexPositionColor v1(DirectX::XMVectorSubtract(vScale, yAxis), color);
        DirectX::VertexPositionColor v2(DirectX::XMVectorAdd(vScale, yAxis), color);
        batch->DrawLine(v1, v2);
    }

    for (size_t i = 0; i <= ydivs; i++)
    {
        float fPercent = float(i) / float(ydivs);
        fPercent = (fPercent * 2.0f) - 1.0f;
        DirectX::XMVECTOR vScale = DirectX::XMVectorScale(yAxis, fPercent);
        vScale = DirectX::XMVectorAdd(vScale, origin);

        DirectX::VertexPositionColor v1(DirectX::XMVectorSubtract(vScale, xAxis), color);
        DirectX::VertexPositionColor v2(DirectX::XMVectorAdd(vScale, xAxis), color);
        batch->DrawLine(v1, v2);
    }
    batch->End();
}
#pragma endregion

/*****************************************************************************************/

#pragma region sprite
//-----------------------------------------------------------------------------
// Name: createDeviceDependentResources()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::sprite::createDeviceDependentResources(sharedVars & v)
{
	this->v = &v;
}

//-----------------------------------------------------------------------------
// Name: draw()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::sprite::draw(texture& tex, fRECT& destinationRectangle, fRECT* sourceRectangle, color col, float rotation, vector2 const& origin, float layerDepth, bool flipVertically, bool flipHorizontally)
{
	DirectX::SpriteEffects se;

	if (flipVertically && flipHorizontally) {	se = DirectX::SpriteEffects::SpriteEffects_FlipBoth;
	} else if (flipVertically  ) {				se = DirectX::SpriteEffects::SpriteEffects_FlipVertically;
	} else if (flipHorizontally) {				se = DirectX::SpriteEffects::SpriteEffects_FlipHorizontally;
	} else {									se = DirectX::SpriteEffects::SpriteEffects_None;
	}

	if (!v)
		return;

	RECT tempRect = sourceRectangle ? sourceRectangle->getRECT() : RECT{};
	const RECT* pTempRect = sourceRectangle ? &tempRect : nullptr;
	v->spriteBatch->Draw(
		tex.getTextureResource() ? tex.getTextureResource()->dxTexRes.Get() : nullptr, 
		destinationRectangle.getRECT(), pTempRect, 
		getDxColor(col), 
		rotation, 
		DirectX::XMFLOAT2{origin.x, origin.y}, 
		se, 
		layerDepth);
}

//-----------------------------------------------------------------------------
// Name: draw()
// Desc: 
//-----------------------------------------------------------------------------
void wildWeasel::ssf::sprite::draw(texture& tex, vector2 const& position, fRECT* sourceRectangle, color col, float rotation, vector2 const& origin, vector2 scale, float layerDepth, bool flipVertically, bool flipHorizontally)
{
	DirectX::SpriteEffects se;

	if (flipVertically && flipHorizontally) {	se = DirectX::SpriteEffects::SpriteEffects_FlipBoth;
	} else if (flipVertically  ) {				se = DirectX::SpriteEffects::SpriteEffects_FlipVertically;
	} else if (flipHorizontally) {				se = DirectX::SpriteEffects::SpriteEffects_FlipHorizontally;
	} else {									se = DirectX::SpriteEffects::SpriteEffects_None;
	}

	if (!v)
		return;

	RECT tempRect = sourceRectangle ? sourceRectangle->getRECT() : RECT{};
	const RECT* pTempRect = sourceRectangle ? &tempRect : nullptr;
	v->spriteBatch->Draw(
		tex.getTextureResource() ? tex.getTextureResource()->dxTexRes.Get() : nullptr, 
		DirectX::XMFLOAT2{position.x, position.y}, pTempRect, 
		getDxColor(col), 
		rotation, 
		DirectX::XMFLOAT2{origin.x, origin.y}, 
		DirectX::XMFLOAT2{scale.x, scale.y}, 
		se, 
		layerDepth);
}
#pragma endregion

