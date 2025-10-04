######################################################################
# CMakeLists.txt
# Copyright (c) Thomas Weber. All rights reserved.				
# Licensed under the MIT License.
# https://github.com/madweasel/madweasels-cpp
######################################################################

# Define source files
set(SOURCE_FILES
    alphaBeta/commonThreadVars.cpp
    alphaBeta/alphaBeta.cpp
    alphaBeta/knotStruct.cpp
    database/database.cpp
    database/databaseFile.cpp
    database/databaseStats.cpp
    database/dbCompTrans.cpp
    database/databaseTypes.cpp
    miniMax.cpp
    typeDef.cpp
    retroAnalysis/retroAnalysis.cpp
    retroAnalysis/stateQueue.cpp
    retroAnalysis/successorCountArray.cpp
    statistics/statistics.cpp
    integrity/integrityChecker.cpp
    gui/winCalcDb.cpp
    gui/winInspectDb.cpp
)

# Define header files
set(HEADER_FILES
    alphaBeta/commonThreadVars.h
    alphaBeta/alphaBeta.h
    alphaBeta/knotStruct.h
    database/database.h
    database/databaseFile.h
    database/databaseStats.h
    database/dbCompTrans.h
    database/databaseTypes.h
    miniMax.h
    typeDef.h
    retroAnalysis/retroAnalysis.h
    retroAnalysis/stateQueue.h
    retroAnalysis/successorCountArray.h
    statistics/statistics.h
    integrity/integrityChecker.h
    gui/winCalcDb.h
    gui/winInspectDb.h
)

# Add executable
add_library(miniMaxLib ${SOURCE_FILES} ${HEADER_FILES})

# Include directories
target_include_directories(miniMaxLib PRIVATE
    ${PATH_WEASEL_LIBRARY}
)

# Compiler options
target_compile_definitions(miniMaxLib PRIVATE _CONSOLE X64)

# Linker options
target_link_libraries(miniMaxLib PRIVATE Microsoft::DirectXTK Cabinet.lib Shlwapi.lib weaselEssentialsLib CompressorLib)
