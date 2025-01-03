# Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(LIBRARY_NAME Detour)

file(GLOB SRC_DETOUR Source/*.cpp Include/*.h)

set(DETOUR_SRCS
    ${SRC_DETOUR}
)

# source group.
ascemu_source_group(${CMAKE_CURRENT_SOURCE_DIR})

add_library(${LIBRARY_NAME} STATIC ${DETOUR_SRCS})

target_include_directories(${LIBRARY_NAME} PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/Include
    ${ZLIB_INCLUDE_DIRS}
)

# include library dependencies.
target_link_libraries(${LIBRARY_NAME}
  PUBLIC
    ${ZLIB_LIBRARIES}
)

unset(SRC_DETOUR)
