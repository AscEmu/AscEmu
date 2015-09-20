# Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>

# set up project name
project(mmaps_generator CXX)

set(prefix ${ROOT_PATH}/src/tools/mmaps_generator)
set(sources
   generator.cpp
   IntermediateValues.cpp
   MapBuilder.cpp
   TerrainBuilder.cpp
   VMapExtensions.cpp
   )

set(headers
   IntermediateValues.h
   MoveMapSharedDefines.h
   MapBuilder.h
   TerrainBuilder.h
   CompilerDefs.h
   Define.h
   )

foreach(src IN ITEMS ${sources})
   set(SRCS ${SRCS} ${prefix}/${src})
endforeach(src)
foreach(src IN ITEMS ${headers})
   set(HEADERS ${HEADERS} ${prefix}/${src})
endforeach(src)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${ROOT_PATH}/src/shared
   ${DEPENDENCY_ROOT}/pcre/include
   ${DEPENDENCY_ROOT}/g3dlite/include
   ${DEPENDENCY_ROOT}/vmaplib/include
   ${DEPENDENCY_ROOT}/loadlib
   ${RECASTNAVIGATION}/detour/include
   ${RECASTNAVIGATION}/recast/include
   ${DEPENDENCY_LIBS}
   )
  
add_executable(${PROJECT_NAME} ${SRCS} ${HEADERS})
target_link_libraries(${PROJECT_NAME} shared g3dlite vmaplib loadlib detour recast ${PCRE_LIBRARIES})
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH})
