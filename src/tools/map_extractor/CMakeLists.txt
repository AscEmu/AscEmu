# Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(EXECUTABLE_NAME map_extractor)

file(GLOB SRC_MAPS_EXTRACTOR *.cpp *.h)

add_executable(${EXECUTABLE_NAME} ${SRC_MAPS_EXTRACTOR})

include_directories(
    ${CMAKE_SOURCE_DIR}/dep/libmpq/libmpq
    ${CMAKE_SOURCE_DIR}/dep/dbcfile 
    ${CMAKE_SOURCE_DIR}/dep/loadlib
    ${CMAKE_SOURCE_DIR}/src/shared
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# include library dependencies.
target_link_libraries(${EXECUTABLE_NAME}
  PUBLIC
    dbcfile
    libmpq
)

install(TARGETS ${EXECUTABLE_NAME} RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH})
