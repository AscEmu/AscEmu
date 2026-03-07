# Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(EXECUTABLE_NAME connection_patcher)

file(GLOB SRC_CONNECTION *.cpp *.hpp)

# source group.
ascemu_source_group(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(${EXECUTABLE_NAME} ${SRC_CONNECTION})

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS ${EXECUTABLE_NAME} RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH})
