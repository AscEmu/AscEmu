# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

# set up project name
project(mmaps_generator CXX)
file(GLOB source *.cpp *.h)

set(source
   ${source}
)

include_directories(
   ${CMAKE_SOURCE_DIR}/dep/recastnavigation/Detour/Include
   ${CMAKE_SOURCE_DIR}/dep/recastnavigation/Recast/Include
   ${CMAKE_SOURCE_DIR}/dep/g3dlite/include
   ${PCRE_INCLUDE_DIR}
   ${CMAKE_SOURCE_DIR}/src/shared
   ${CMAKE_SOURCE_DIR}/src/collision
   ${CMAKE_SOURCE_DIR}/src/collision/Management
   ${CMAKE_SOURCE_DIR}/src/collision/Maps
   ${CMAKE_SOURCE_DIR}/src/collision/Models
   ${CMAKE_SOURCE_DIR}/dep/loadlib
   ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(${PROJECT_NAME} ${source})
target_link_libraries(${PROJECT_NAME} shared g3dlite collision Detour Recast ${PCRE_LIBRARIES})
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH})
