######################################################################
# CMakeLists.txt
# Copyright (c) Thomas Weber. All rights reserved.				
# Licensed under the MIT License.
# https://github.com/madweasel/madweasels-cpp
######################################################################

# Define source files
set(SOURCE_FILES
    cyclicArray.cpp
    threadManager.cpp
    strLib.cpp
    xml.cpp
    logger.cpp
)

# Define header files
set(HEADER_FILES
    cyclicArray.h
    threadManager.h
    strLib.h
    xml.h
    logger.h
)

# Add executable
add_library(weaselEssentialsLib ${SOURCE_FILES} ${HEADER_FILES})

# Include directories
target_include_directories(weaselEssentialsLib PRIVATE
    ${PATH_WEASEL_LIBRARY}
)

# Compiler options
target_compile_definitions(weaselEssentialsLib PRIVATE _CONSOLE X64)

# Linker options
target_link_libraries(weaselEssentialsLib PRIVATE xmllite.lib shlwapi.lib)
