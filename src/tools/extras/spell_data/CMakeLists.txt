# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

file(GLOB spell_data_sources *.cpp *.h)

set(spell_SRCS
  ${spell_data_sources}
)

if(NOT WIN32 AND NOT IS_64BIT)
    add_definitions(-D_FILE_OFFSET_BITS=64)
endif()

include_directories(
   ${LOADLIB_PATH}
   )

add_executable(spell_data ${spell_SRCS})
target_link_libraries(spell_data ${ZLIB_LIBRARIES})
install(TARGETS spell_data RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH})
