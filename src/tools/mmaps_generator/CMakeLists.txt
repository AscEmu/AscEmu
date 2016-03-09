# Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>

# set up project name
project(mmaps_generator CXX)

set(sources
   generator.cpp
   IntermediateValues.cpp
   MapBuilder.cpp
   TerrainBuilder.cpp
   VMapExtensions.cpp
)

set(headers
    Define.h
    IntermediateValues.h
    MapBuilder.h
    MMapCommon.h
    MoveMapSharedDefines.h
    TerrainBuilder.h
)

include_directories(
   ${CMAKE_SOURCE_DIR}/dep/recastnavigation/Detour/Include
   ${CMAKE_SOURCE_DIR}/dep/recastnavigation/Recast/Include
   ${CMAKE_SOURCE_DIR}/dep/g3dlite/include
   ${CMAKE_SOURCE_DIR}/dep/vmaplib/include
   ${PCRE_INCLUDE_DIR}
  # ${CMAKE_SOURCE_DIR}/src/shared
   ${CMAKE_SOURCE_DIR}/dep/loadlib
   ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(${PROJECT_NAME} ${sources} ${headers})
target_link_libraries(${PROJECT_NAME} shared g3dlite vmaplib Detour Recast ${PCRE_LIBRARIES})
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH})
