# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

file(GLOB Recast_SRCS Source/*.cpp Include/*.h)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/Include
   ${ZLIB_INCLUDE_DIRS}
)

add_library(Recast STATIC ${Recast_SRCS})
target_link_libraries(Recast ${ZLIB_LIBRARIES})

unset(Recast_SRCS)