# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

# set up our project name.
project(Recast CXX)

file(GLOB SRC_RECAST Source/*.cpp Include/*.h)

set(RECAST_SRCS
    ${SRC_RECAST}
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/Include
    ${ZLIB_INCLUDE_DIRS}
)

add_library(${PROJECT_NAME} STATIC ${RECAST_SRCS})

# include library dependencies.
target_link_libraries(${PROJECT_NAME}
  PUBLIC
    ${ZLIB_LIBRARIES}
)

unset(RECAST_SRCS)