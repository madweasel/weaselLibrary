######################################################################
# CMakeLists.txt
# Copyright (c) Thomas Weber. All rights reserved.				
# Licensed under the MIT License.
# https://github.com/madweasel/madweasels-cpp
######################################################################
cmake_minimum_required (VERSION 3.15)

# check if the Developer Command Prompt for Visual Studio (x64) is used
if(NOT DEFINED ENV{VSCMD_VER} OR NOT DEFINED ENV{VSCMD_ARG_TGT_ARCH} OR NOT "$ENV{VSCMD_ARG_TGT_ARCH}" STREQUAL "x64")
    message(FATAL_ERROR "Please run VS Code inside the 'x64 Native Tools Command Prompt for VS 2022'!\n\
        1) Press 'WINDOWS-KEY' and type 'x64 Native Tools Command Prompt for VS 2022'.\n\
        2) Type 'code' and press 'ENTER'.")
endif()

# extract compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable Hot Reload for MSVC compilers if supported.
if(MSVC)
  set(CMAKE_CXX_HOT_RELOAD ON)
  if (POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
  endif()
endif()

project ("WeaselLibrary")

# paths
if(NOT DEFINED PATH_WEASEL_LIBRARY)
  set(PATH_WEASEL_LIBRARY "${CMAKE_SOURCE_DIR}")
endif()

# Print paths
message("CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")

# Specify the compiler and settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Unicode
add_definitions(-DUNICODE -D_UNICODE)

# Set 64-bit architecture
set(CMAKE_GENERATOR_PLATFORM x64)

# Set compiler flags for different build types
if(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "/MDd /ZI")
    set(CMAKE_CXX_FLAGS_RELEASE "/MD /Zi /O2")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/MD /ZI /O2")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "/MD")
else()
    set(CMAKE_CXX_FLAGS "--coverage")
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os")
endif()

# Specify the generator
set(CMAKE_GENERATOR "Ninja")

# Add packages
enable_testing()
find_package(GTest CONFIG REQUIRED)
find_package(directxtk CONFIG REQUIRED)

# Include libs
add_subdirectory("compressor/src")
add_subdirectory("miniMax/src")
add_subdirectory("weaselEssentials/src")
add_subdirectory("permutationGameSolver/src")

if (MSVC)
    add_subdirectory("wildWeasel/src")    # gcc fails to compile this library
endif()

# Include tests
add_subdirectory("compressor/tst")
add_subdirectory("miniMax/tst")
add_subdirectory("permutationGameSolver/tst")
add_subdirectory("weaselEssentials/tst")
