######################################################################
# CMakeLists.txt
# Copyright (c) Thomas Weber. All rights reserved.				
# Licensed under the MIT License.
# https://github.com/madweasel/madweasels-cpp
######################################################################

# Define source files
set(SOURCE_FILES
    MiniMaxTest.cpp
    MiniMaxDatabaseTest.cpp
    MiniMaxAlphaBetaTest.cpp
    MiniMaxRetroAnalysisTest.cpp
    MiniMaxStatisticsTest.cpp
    MiniMaxGameStub.cpp
    MiniMaxIntegrityChecker.cpp
)

# Define header files
set(HEADER_FILES
)

# Add executable
add_executable(MiniMaxTest ${SOURCE_FILES} ${HEADER_FILES})

# Include directories
target_include_directories(MiniMaxTest PRIVATE
    ${PATH_WEASEL_LIBRARY}
)

# Set compilation options for different build configurations
target_compile_definitions(MiniMaxTest PRIVATE $<$<CONFIG:Debug>:_DEBUG;_CONSOLE;GTEST_HAS_STD_TUPLE_;GTEST_HAS_TR1_TUPLE=0>)
target_compile_definitions(MiniMaxTest PRIVATE $<$<CONFIG:Release>:NDEBUG;_CONSOLE;GTEST_HAS_STD_TUPLE_;GTEST_HAS_TR1_TUPLE=0>)

# Linker options
target_link_libraries(MiniMaxTest PRIVATE Cabinet.lib Shlwapi.lib)

# Add Google Test
target_link_libraries(MiniMaxTest PRIVATE GTest::gtest GTest::gtest_main miniMaxLib)

# Add tests
add_test(MiniMaxTests               MiniMaxTest --gtest_filter=MiniMaxMain*)
add_test(MiniMaxDatabaseTest        MiniMaxTest --gtest_filter=MiniMaxDatabase*)
add_test(MiniMaxAlphaBetaTest       MiniMaxTest --gtest_filter=MiniMaxAlphaBeta*)
add_test(MiniMaxRetroAnalysisTest   MiniMaxTest --gtest_filter=MiniMaxRetroAnalysis*)
add_test(MiniMaxStatisticsTest      MiniMaxTest --gtest_filter=MiniMaxStatistics*)
add_test(MiniMaxIntegrityChecker    MiniMaxTest --gtest_filter=MiniMaxIntegrityChecker*)
