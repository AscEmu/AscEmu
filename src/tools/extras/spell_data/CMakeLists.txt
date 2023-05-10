# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

file(GLOB SPELL_DATA_SOURCES *.cpp *.h)

set(SRC_SPELL_DATA
    ${SPELL_DATA_SOURCES}
)

if (NOT WIN32 AND NOT IS_64BIT)
    add_definitions(-D_FILE_OFFSET_BITS=64)
endif ()

include_directories(
    ${LOADLIB_PATH}
)

add_executable(spell_data ${SRC_SPELL_DATA})
target_link_libraries(spell_data ${ZLIB_LIBRARIES})
install(TARGETS spell_data RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH})
