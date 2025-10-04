//--------------------------------------------------------------------------------------
//
// pch.h
// Header for standard system include files.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
//
// https://github.com/walbourn/directxtk-samples
//--------------------------------------------------------------------------------------

#pragma once

#include <WinSDKVer.h>
#define _WIN32_WINNT 0x0A00
#include <SDKDDKVer.h>

// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN		
#define _CRT_SECURE_NO_DEPRECATE

// Use the C++ standard templated min/max
#define NOMINMAX

// DirectX apps don't need GDI   -> yes they do !
//#define NODRAWTEXT
//#define NOGDI
//#define NOBITMAP

// Include <mcx.h> if you need this
#define NOMCX

// Include <winsvc.h> if you need this
#define NOSERVICE

// WinHelp is deprecated
#define NOHELP

#include <windows.h>
#include <wrl/client.h>
#include <wrl/event.h>
#include <shellapi.h>

#include <d3d12.h>
#include <dxgi1_4.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

#include "d3dx12.h"

#include <stdio.h>
#include <pix.h>

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

#include "Audio.h"
#include "CommonStates.h"
#include "DirectXHelpers.h"
#include "DDSTextureLoader.h"
#include "DescriptorHeap.h"
#include "Effects.h"
#include "GamePad.h"
#include "GeometricPrimitive.h"
#include "GraphicsMemory.h"
#include "Keyboard.h"
#include "Model.h"
#include "Mouse.h"
#include "PrimitiveBatch.h"
#include "ResourceUploadBatch.h"
#include "RenderTargetState.h"
#include "SimpleMath.h"
#include "SpriteBatch.h"
#include "SpriteFont.h" 
#include "VertexTypes.h"
#include <WICTextureLoader.h>

// standard library & win32 api
#include <windows.h>
#include <wincodec.h>
#include <windowsx.h>
#include <Wingdi.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <commdlg.h>
#include <Commctrl.h>
#include <Dbt.h>
#include <ShellScalingApi.h>

// std
#include <algorithm>
#include <exception>
#include <memory>
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>
#include <list>
#include <thread>
#include <unordered_set>
#include <functional>
#include <iomanip>
#include <random>
#include <numeric>

// boost (c++17)
// ... #include <boost/filesystem.hpp>

// DirectX Toolkit
#include "..\\DirectXTK\\DeviceResources.h"
#include "..\\DirectXTK\\StepTimer.h"

namespace DX
{
    // Helper class for COM exceptions
    class com_exception : public std::exception
    {
    public:
        com_exception(HRESULT hr) : result(hr) {}

        virtual const char* what() const override
        {
            static char s_str[64] = { 0 };
            sprintf_s(s_str, "Failure with HRESULT of %08X", result);
            return s_str;
        }

    private:
        HRESULT result;
    };

    // Helper utility converts D3D API failures into exceptions.
    inline void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            throw com_exception(hr);
        }
    }
}