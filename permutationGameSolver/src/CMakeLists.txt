######################################################################
# CMakeLists.txt
# Copyright (c) Thomas Weber. All rights reserved.				
# Licensed under the MIT License.
# https://github.com/madweasel/madweasels-cpp
######################################################################

# Define source files
set(SOURCE_FILES
    backTracking.cpp
    permutationGameSolver.cpp
    pgsThread.cpp
    zoneClass.cpp
    ${PATH_WEASEL_LIBRARY}/wildWeasel/src/wwThread.cpp
)

# Define header files
set(HEADER_FILES
    backTracking.h
    permutationGameSolver.h
    pgsThread.h
    zoneClass.h
    ${PATH_WEASEL_LIBRARY}/wildWeasel/src/wwThread.h
)

# Add executable
add_library(pgsLib ${SOURCE_FILES} ${HEADER_FILES})

# Include directories
target_include_directories(pgsLib PRIVATE
    ${PATH_WEASEL_LIBRARY}
)

# Compiler options
target_compile_definitions(pgsLib PRIVATE _CONSOLE X64)

# Linker options
target_link_libraries(pgsLib PRIVATE Cabinet.lib Shlwapi.lib weaselEssentialsLib CompressorLib)
