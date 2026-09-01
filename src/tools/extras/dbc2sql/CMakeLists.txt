# Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(EXECUTABLE_NAME dbc2sql)

file(GLOB SRC_DBC2SQL *.cpp *.hpp)

# source group.
ascemu_source_group(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(${EXECUTABLE_NAME} ${SRC_DBC2SQL})

# Only needs WDBFormat.hpp's per-version format-string table
# (dbcFieldDefines) - it is header-only and dependency-free (just
# <string>/<map>), so no link against the world/shared libraries is needed.
include_directories(
    ${CMAKE_SOURCE_DIR}/src/world/Storage/WDB
)

install(TARGETS ${EXECUTABLE_NAME} RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH})
