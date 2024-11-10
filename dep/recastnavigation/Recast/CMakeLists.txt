# Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(EXECUTABLE_NAME Recast)

file(GLOB SRC_RECAST Source/*.cpp Include/*.h)

set(RECAST_SRCS
    ${SRC_RECAST}
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/Include
    ${ZLIB_INCLUDE_DIRS}
)

add_library(${EXECUTABLE_NAME} STATIC ${RECAST_SRCS})

# include library dependencies.
target_link_libraries(${EXECUTABLE_NAME}
  PUBLIC
    ${ZLIB_LIBRARIES}
)

unset(RECAST_SRCS)