cmake_minimum_required(VERSION 3.10)
project(votable_test)

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
set(LINK_LIBS ${LINK_LIBS} pugixml fmt tbb)
add_executable(votable_test main.cpp Table.cc Table.h Columns.h Columns.cc TableView.cc TableView.h)
target_link_libraries(votable_test ${LINK_LIBS})
