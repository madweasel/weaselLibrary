######################################################################
# CMakeLists.txt
# Copyright (c) Thomas Weber. All rights reserved.				
# Licensed under the MIT License.
# https://github.com/madweasel/madweasels-cpp
######################################################################

# Define source files
set(SOURCE_FILES
    wildWeasel.cpp
    ww3D.cpp
    wwAlignment.cpp
    wwAnimation.cpp
    wwEditField.cpp
    wwExamples.cpp
    wwFlipBook.cpp
    wwGuiElement.cpp
    wwListView.cpp
    wwRotCtrlCube.cpp
    wwTreeView.cpp
    wwThread.cpp
    wwMasterMind.cpp
    ssf_DXTK11/wwSsf.cpp
    ${PATH_WEASEL_LIBRARY}/DirectXTK11/DeviceResources.cpp
    ${PATH_WEASEL_LIBRARY}/DirectXTK11/pch.cpp
)

# Define header files
set(HEADER_FILES
    wildWeasel.h
    ww3D.h
    wwAlignment.h
    wwAnimation.h
    wwEditField.h
    wwExamples.h
    wwFlipBook.h
    wwGuiElement.h
    wwListView.h
    wwRotCtrlCube.h
    wwTreeView.h
    wwThread.h
    wwMasterMind.h
    ssf_DXTK11/wwSsf.h
)

# Add executable
add_library(wildWeaselLib ${SOURCE_FILES} ${HEADER_FILES})

# Include directories
target_include_directories(wildWeaselLib PRIVATE
    ${PATH_WEASEL_LIBRARY}
)

# Compiler options
target_compile_definitions(wildWeaselLib PRIVATE _CONSOLE X64)

# Linker options
target_link_libraries(wildWeaselLib PRIVATE Microsoft::DirectXTK Cabinet.lib Shlwapi.lib weaselEssentialsLib)
