cmake_minimum_required(VERSION 3.10)
project(simple_votable)

# if you want to build pugixml in compact mode, manually add include and link dirs.
# Compact mode uses a LOT less memory than the default mode, but is slightly slower.
# https://pugixml.org/docs/manual.html#dom.memory.compact
# (Debian package libpuglixml-dev is built in normal mode)
#include_directories(/home/angus/repos/pugixml/src)
#link_directories(/home/angus/repos/pugixml/build)
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    set(LINK_LIBS ${LINK_LIBS} PUBLIC OpenMP::OpenMP_CXX)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_PARALLEL")

set(CMAKE_CXX_STANDARD 17)
link_directories(/usr/local/lib)
set(LINK_LIBS ${LINK_LIBS} pugixml fmt tbb cfitsio)

set(SRC_FILES src/Table.cc src/Columns.cc src/TableView.cc)

add_executable(simple_votable src/main.cpp ${SRC_FILES})
target_link_libraries(simple_votable ${LINK_LIBS})

# Tests
option(test "Build all tests." OFF)
if (test)
    enable_testing()
    find_package(GTest)
    include_directories(${GTEST_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR} src)

    add_executable(test_votable test/TestVOTable.cc ${SRC_FILES})
    target_link_libraries(test_votable ${LINK_LIBS} gtest gtest_main)

    add_executable(test_fits test/TestFITS.cc ${SRC_FILES})
    target_link_libraries(test_fits ${LINK_LIBS} gtest gtest_main)

    add_test(NAME TestVOTable COMMAND test_votable)
    add_test(NAME TestFITS COMMAND test_fits)
endif (test)