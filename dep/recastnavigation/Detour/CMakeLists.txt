# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

file(GLOB Detour_SRCS Source/*.cpp Include/*.h)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/Include
   ${ZLIB_INCLUDE_DIRS}
)

add_library(Detour STATIC ${Detour_SRCS})
target_link_libraries(Detour ${ZLIB_LIBRARIES})

unset(Detour_SRCS)