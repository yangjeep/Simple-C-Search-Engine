cmake_minimum_required(VERSION 3.14)
project(SearchEngine C)


# Find LMDB package
find_package(PkgConfig REQUIRED)
pkg_check_modules(LMDB REQUIRED lmdb)

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
    src/common/gtrie.c
)

# Create common library
add_library(common SHARED ${COMMON_SOURCES})
target_include_directories(common PUBLIC 
    include
    ${LMDB_INCLUDE_DIRS}
)
target_link_libraries(common PUBLIC ${LMDB_LIBRARIES})

# Comment out indexer executable
#add_executable(indexer 
#    src/indexer/indexer.c
#)
#target_include_directories(indexer PUBLIC include)

# Comment out server executable
#add_executable(server 
#    src/server/server.c
#)
#target_include_directories(server PUBLIC include)

# Tests
enable_testing()
file(GLOB TEST_SOURCES "test/test_*.c")
foreach(TEST_SOURCE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    target_link_libraries(${TEST_NAME} PRIVATE 
        common 
        unity
        ${LMDB_LIBRARIES}
    )
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()

