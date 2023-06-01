# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

# set up our project name.
project(creature_data CXX)

file(GLOB SRC_CREATURE_DATA *.cpp *.h)

set(CREATURE_DATA_SRCS
    ${SRC_CREATURE_DATA}
)

if (NOT WIN32 AND NOT IS_64BIT)
    add_definitions(-D_FILE_OFFSET_BITS=64)
endif ()

include_directories(
    ${DEPENDENCY_ROOT}/libmpq/libmpq
    ${DEPENDENCY_ROOT}/libmpq/win
    ${DEPENDENCY_ROOT}/dbcfile
    ${LOADLIB_PATH}
)

add_executable(${PROJECT_NAME} ${CREATURE_DATA_SRCS})

# include library dependencies.
target_link_libraries(${PROJECT_NAME}
  PUBLIC
    dbcfile
    ${ZLIB_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH})
