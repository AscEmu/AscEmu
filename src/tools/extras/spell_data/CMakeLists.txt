# Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>

# set up our project name.
project(spell_data CXX)

file(GLOB SRC_SPELL_DATA *.cpp *.h)

set(SPELL_DATA_SRCS
    ${SRC_SPELL_DATA}
)

if (NOT WIN32 AND NOT IS_64BIT)
    add_definitions(-D_FILE_OFFSET_BITS=64)
endif ()

include_directories(
    ${LOADLIB_PATH}
)

add_executable(${PROJECT_NAME} ${SPELL_DATA_SRCS})

# include library dependencies.
target_link_libraries(${PROJECT_NAME}
  PUBLIC
    ${ZLIB_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH})
