# Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(EXECUTABLE_NAME Detour)

file(GLOB SRC_DETOUR Source/*.cpp Include/*.h)

set(DETOUR_SRCS
    ${SRC_DETOUR}
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/Include
    ${ZLIB_INCLUDE_DIRS}
)

add_library(${EXECUTABLE_NAME} STATIC ${DETOUR_SRCS})

# include library dependencies.
target_link_libraries(${EXECUTABLE_NAME}
  PUBLIC
    ${ZLIB_LIBRARIES}
)

unset(SRC_DETOUR)
