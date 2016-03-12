# Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>

# set up our project name
project(vmaplib CXX)
set(sources
    BIH.cpp
    MapTree.cpp
    ModelInstance.cpp
    TileAssembler.cpp
    VMapFactory.cpp
    VMapManager2.cpp
    WorldModel.cpp)

include_directories(
    ${PCRE_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/dep/g3dlite/include
    ${CMAKE_SOURCE_DIR}/src/shared
)
link_directories(${DEPENDENCY_LIBS} ${EXTRA_LIBS_PATH})

add_library(${PROJECT_NAME} STATIC ${sources})
target_link_libraries(${PROJECT_NAME} shared g3dlite)
unset(sources)
