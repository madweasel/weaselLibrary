# Introduction
This C++ library are used by the applications in other repositories (Traga 2, Muehle). 

## compressor
This is a generic wrapper for external compression algorithms.

## DirectXTK11
Modified header files from the [DirectX Tool Kit Samples](https://github.com/walbourn/directxtk-samples) adapted for the wildWeasel library.

## minimax
Implementation of the minimax, alpha-beta and retro analysis algorithmn used by 'Muehle'. It also contains GUI functions for the creation and inspection of the database.

## permutationGameSolver
Own algorithmn to solve a permutation games. It also contains GUI functions for the creation and inspection of the database.

## wildWeasel
Library providing simple GUI elements and standard system functionality (mouse and keybord input, sounds, message box, etc.). 
It's main purpose is to have a common interface for all applications of this repository without using a specific API.

# Software dependencies

## Libraries
The following EXTERNAL libraries are employed:
- [googletest](https://github.com/google/googletest)
- [DirectX 11 Toolkit](https://github.com/Microsoft/DirectXTK)
- [vcpkg](https://github.com/microsoft/vcpkg)
- shlwapi.lib, comctl32.lib, DXGI.lib, D3D11.lib, XmlLite.lib
- Win32 API
- C++ STL

## Environment
System requirements:
- Windows 10/11
- DirectX 11 (since the DirectXTK 11 is used)
- Visual Studio Community 2022
- Msys MinGW 64-bit 
- CMake
- vcpkg
- Git 

Setup:
- See https://code.visualstudio.com/docs/cpp/config-mingw
- See https://learn.microsoft.com/en-us/vcpkg/users/platforms/mingw
- pacman -S --needed     base-devel mingw-w64-ucrt-x86_64-toolchain
- pacman -S --needed git base-devel mingw-w64-x86_64-toolchain

# Latest releases
January 1, 2019 - First release.
January 1, 2025 - Second release.

# Contribute
Feel free to fork and send pull requests.

Contact: [karaizy@mad-weasel.de](mailto:karaizy@mad-weasel.de).

# License
Copyright (c) Thomas Weber. All rights reserved.

Licensed under the [MIT](LICENSE) License.
