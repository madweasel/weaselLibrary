######################################################################
# CMakeLists.txt
# Copyright (c) Thomas Weber. All rights reserved.				
# Licensed under the MIT License.
# https://github.com/madweasel/madweasels-cpp
######################################################################

# Define source files
set(SOURCE_FILES
    pgsTest.cpp
)

# Define header files
set(HEADER_FILES
)

# Add executable
add_executable(pgsTest ${SOURCE_FILES} ${HEADER_FILES})

# Include directories
target_include_directories(pgsTest PRIVATE
    ${PATH_WEASEL_LIBRARY}/wildWeasel/src
    ${PATH_WEASEL_LIBRARY}/permutationGameSolver/src
    ${PATH_WEASEL_LIBRARY}
)

# Set compilation options for different build configurations
target_compile_definitions(pgsTest PRIVATE $<$<CONFIG:Debug>:_DEBUG;_CONSOLE;GTEST_HAS_STD_TUPLE_;GTEST_HAS_TR1_TUPLE=0>)
target_compile_definitions(pgsTest PRIVATE $<$<CONFIG:Release>:NDEBUG;_CONSOLE;GTEST_HAS_STD_TUPLE_;GTEST_HAS_TR1_TUPLE=0>)

# Add libs
target_link_libraries(pgsTest PRIVATE GTest::gtest GTest::gtest_main CompressorLib pgsLib)

add_test(pgsTests pgsTest)
