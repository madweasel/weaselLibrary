######################################################################
# CMakeLists.txt
# Copyright (c) Thomas Weber. All rights reserved.				
# Licensed under the MIT License.
# https://github.com/madweasel/madweasels-cpp
######################################################################

# Define source files
set(SOURCE_FILES
    CyclicArrayTest.cpp
    strLibTest.cpp
    threadManagerTest.cpp
    xmlTest.cpp
)

# Define header files
set(HEADER_FILES
)

# Add executable
add_executable(GenericTest ${SOURCE_FILES} ${HEADER_FILES})

# Include directories
target_include_directories(GenericTest PRIVATE
    ${PATH_WEASEL_LIBRARY}/weaselEssentials/src
)

# Set compilation options for different build configurations
target_compile_definitions(GenericTest PRIVATE $<$<CONFIG:Debug>:_DEBUG;_CONSOLE;GTEST_HAS_STD_TUPLE_;GTEST_HAS_TR1_TUPLE=0>)
target_compile_definitions(GenericTest PRIVATE $<$<CONFIG:Release>:NDEBUG;_CONSOLE;GTEST_HAS_STD_TUPLE_;GTEST_HAS_TR1_TUPLE=0>)

# Linker options
target_link_libraries(GenericTest PRIVATE xmllite.lib shlwapi.lib)

# Add Google Test
target_link_libraries(GenericTest PRIVATE GTest::gtest GTest::gtest_main weaselEssentialsLib)

add_test(CyclicArrayTests   GenericTest --gtest_filter=CyclicArray*)
add_test(StrLibTests        GenericTest --gtest_filter=strLib*)
add_test(ThreadManagerTests GenericTest --gtest_filter=ThreadManager*)
add_test(XmlTest            GenericTest --gtest_filter=xmlTest*)
