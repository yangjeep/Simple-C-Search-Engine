cmake_minimum_required(VERSION 3.14)
project(SearchEngine C)

# Download and configure Unity
include(FetchContent)
FetchContent_Declare(
    unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG v2.5.2
)
FetchContent_MakeAvailable(unity)

# Project structure
set(COMMON_SOURCES
    src/common/document.c
    src/common/indexer.c
)

# Create common library
add_library(common SHARED ${COMMON_SOURCES})
target_include_directories(common PUBLIC include include/common)

# Tests
enable_testing()
file(GLOB TEST_SOURCES "test/test_*.c")
foreach(TEST_SOURCE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    target_link_libraries(${TEST_NAME} PRIVATE common unity)
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach() 