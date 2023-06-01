# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

# set up our project name.
project(map_extractor CXX)

file(GLOB SRC_MAPS_EXTRACTOR *.cpp *.h)

set(MAPS_EXTRACTOR_SRCS
    ${SRC_MAPS_EXTRACTOR}
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/dep/StormLib/src
    ${CMAKE_SOURCE_DIR}/src/shared
    ${CMAKE_CURRENT_SOURCE_DIR}/loadlib
    ${CMAKE_SOURCE_DIR}/dep/g3dlite/include
)

add_executable(${PROJECT_NAME} ${MAPS_EXTRACTOR_SRCS})

add_dependencies(${PROJECT_NAME} storm)

# include library dependencies.
target_link_libraries(${PROJECT_NAME}
  PUBLIC
    storm
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH})
