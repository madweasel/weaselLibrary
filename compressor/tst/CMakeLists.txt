######################################################################
# CMakeLists.txt
# Copyright (c) Thomas Weber. All rights reserved.				
# Licensed under the MIT License.
# https://github.com/madweasel/madweasels-cpp
######################################################################

# Define source files
set(SOURCE_FILES
    compressorTest.cpp
)

# Define header files
set(HEADER_FILES
)

# Add executable
add_executable(CompressorTest ${SOURCE_FILES} ${HEADER_FILES})

# Include directories
target_include_directories(CompressorTest PRIVATE
    ${PATH_WEASEL_LIBRARY}/compressor/src
)

# Compiler options
target_compile_definitions(CompressorTest PRIVATE _CONSOLE X64 GTEST_HAS_STD_TUPLE_ GTEST_HAS_TR1_TUPLE=0)

# Linker options
target_link_libraries(CompressorTest PRIVATE Cabinet.lib)

# Add Google Test
target_link_libraries(CompressorTest PRIVATE GTest::gtest GTest::gtest_main CompressorLib)

add_test(CompressorTests CompressorTest)
