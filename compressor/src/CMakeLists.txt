######################################################################
# CMakeLists.txt
# Copyright (c) Thomas Weber. All rights reserved.				
# Licensed under the MIT License.
# https://github.com/madweasel/madweasels-cpp
######################################################################

# Define source files
set(SOURCE_FILES
    compLib_winCompApi.cpp
    compressor.cpp
)

# Define header files
set(HEADER_FILES
    compLib_winCompApi.h
    compressor.h
)

# Add executable
add_library(CompressorLib ${SOURCE_FILES} ${HEADER_FILES})

# Include directories
target_include_directories(CompressorLib PRIVATE
    ${PATH_WEASEL_LIBRARY}
)

# Compiler options
target_compile_definitions(CompressorLib PRIVATE _CONSOLE X64)

# Linker options
target_link_libraries(CompressorLib PRIVATE Cabinet.lib)
