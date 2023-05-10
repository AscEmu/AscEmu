# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

file(GLOB CREATURE_DATA_SOURCES *.cpp *.h)

set(SRC_CREATURE_DATA
    ${CREATURE_DATA_SOURCES}
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

add_executable(creature_data ${SRC_CREATURE_DATA})
target_link_libraries(creature_data dbcfile ${ZLIB_LIBRARIES})
install(TARGETS creature_data RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH})
