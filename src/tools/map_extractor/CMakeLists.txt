# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

# set up our project name.
project(map_extractor CXX)

file(GLOB SRC_MAPS_EXTRACTOR *.cpp *.h)

set(MAPS_EXTRACTOR_SRCS
    ${SRC_MAPS_EXTRACTOR}
    ${CMAKE_SOURCE_DIR}/dep/libmpq/libmpq
)

include_directories(
    ${CMAKE_SOURCE_DIR}/dep/libmpq/libmpq
    ${CMAKE_SOURCE_DIR}/dep/dbcfile 
    ${CMAKE_SOURCE_DIR}/dep/loadlib
    ${CMAKE_SOURCE_DIR}/src/shared
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(${PROJECT_NAME} ${MAPS_EXTRACTOR_SRCS})

# include library dependencies.
target_link_libraries(${PROJECT_NAME}
  PUBLIC
    dbcfile
    libmpq
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH})
