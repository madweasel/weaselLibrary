//--------------------------------------------------------------------------------------
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
#define _WIN32_WINNT 0x0601
#include <SDKDDKVer.h>

// Use the C++ standard templated min/max
#define NOMINMAX

// DirectX apps don't need GDI
#define NODRAWTEXT
//#define NOGDI
#define NOBITMAP

// Include <mcx.h> if you need this
#define NOMCX

// Include <winsvc.h> if you need this
#define NOSERVICE

// WinHelp is deprecated
#define NOHELP

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <wrl/client.h>

// DirectX 11
#include <d3d11_1.h>
#if defined(NTDDI_WIN10_RS2)
#include <dxgi1_6.h>
#else
#include <dxgi1_5.h>
#endif
#include <DirectXMath.h>
#include <DirectXColors.h>

// DirectXTK 11
// #include "Audio.h"
#include "CommonStates.h"
#include "DirectXHelpers.h"
#include "DDSTextureLoader.h"
#include "Effects.h"
#include "GamePad.h"
#include "GeometricPrimitive.h"
#include "Keyboard.h"
#include "Model.h"
#include "Mouse.h"
#include "PrimitiveBatch.h"
#include "SimpleMath.h"
#include "SpriteBatch.h"
#include "SpriteFont.h" 
#include "VertexTypes.h"
#include <WICTextureLoader.h>

// win32 api
#include <windows.h>
#include <wincodec.h>
#include <windowsx.h>
#include <wingdi.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <commdlg.h>
#include <Commctrl.h>
#include <Dbt.h>
#include <ShellScalingApi.h>
#include <shellapi.h>

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
#include <mutex> 
#include <stdio.h>

// DirectX TK Samples
#include "DeviceResources.h"
#include "StepTimer.h"


#ifdef _DEBUG
#include <dxgidebug.h>
#endif

namespace DX
{
    // Helper class for COM exceptions
    class com_exception : public std::exception
    {
    public:
        com_exception(HRESULT hr) : result(hr) {}

        virtual const char* what() const override
        {
            static char s_str[64] = {};
            sprintf_s(s_str, "Failure with HRESULT of %08X", static_cast<unsigned int>(result));
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