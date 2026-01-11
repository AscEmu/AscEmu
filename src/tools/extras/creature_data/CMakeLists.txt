# Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(EXECUTABLE_NAME creature_data)

file(GLOB SRC_CREATURE_DATA *.cpp *.h)

# source group.
ascemu_source_group(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(${EXECUTABLE_NAME} ${SRC_CREATURE_DATA})

if (NOT WIN32 AND NOT IS_64BIT)
    add_compile_options(-D_FILE_OFFSET_BITS=64)
endif ()

include_directories(
    ${DEPENDENCY_ROOT}/libmpq/libmpq
    ${DEPENDENCY_ROOT}/libmpq/win
    ${DEPENDENCY_ROOT}/dbcfile
    ${LOADLIB_PATH}
)

# include library dependencies.
target_link_libraries(${EXECUTABLE_NAME}
  PUBLIC
    dbcfile
    ${ZLIB_LIBRARIES}
)

install(TARGETS ${EXECUTABLE_NAME} RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH})
