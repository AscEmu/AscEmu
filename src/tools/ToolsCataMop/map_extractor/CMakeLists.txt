# Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(EXECUTABLE_NAME map_extractor)

file(GLOB SRC_MAPS_EXTRACTOR *.cpp *.h)

# source group.
ascemu_source_group(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(${EXECUTABLE_NAME} ${SRC_MAPS_EXTRACTOR})

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/dep/StormLib/src
    ${CMAKE_SOURCE_DIR}/src/shared
    ${CMAKE_CURRENT_SOURCE_DIR}/loadlib
    ${CMAKE_SOURCE_DIR}/dep/g3dlite/include
)

add_dependencies(${EXECUTABLE_NAME} storm)

# include library dependencies.
target_link_libraries(${EXECUTABLE_NAME}
  PUBLIC
    storm
)

install(TARGETS ${EXECUTABLE_NAME} RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH})
