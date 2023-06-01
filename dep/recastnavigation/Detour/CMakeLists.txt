# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

# set up our project name.
project(Detour CXX)

file(GLOB SRC_DETOUR Source/*.cpp Include/*.h)

set(DETOUR_SRCS
    ${SRC_DETOUR}
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/Include
    ${ZLIB_INCLUDE_DIRS}
)

add_library(${PROJECT_NAME} STATIC ${DETOUR_SRCS})

# include library dependencies.
target_link_libraries(${PROJECT_NAME}
  PUBLIC
    ${ZLIB_LIBRARIES}
)
