# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

file(GLOB sources_Management Management/*.cpp Management/*.h)
file(GLOB sources_Maps Maps/*.cpp Maps/*.h)
file(GLOB sources_Models Models/*.cpp Models/*.h)
file(GLOB sources_localdir *.cpp *.h)

set(collision_STAT_SRCS
  ${collision_STAT_SRCS}
  ${sources_Management}
  ${sources_Maps}
  ${sources_Models}
  ${sources_localdir}
)

include_directories(
  ${OPENSSL_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/dep/g3dlite/include
  ${CMAKE_SOURCE_DIR}/dep/recastnavigation/Detour/Include
  ${CMAKE_SOURCE_DIR}/dep/recastnavigation/Recast/Include
  ${CMAKE_SOURCE_DIR}/src/shared
  ${CMAKE_SOURCE_DIR}/src/world
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/Management
  ${CMAKE_CURRENT_SOURCE_DIR}/Maps
  ${CMAKE_CURRENT_SOURCE_DIR}/Models
  ${ZLIB_INCLUDE_DIRS}
)

add_library(collision STATIC ${collision_STAT_SRCS})
target_link_libraries(collision g3dlite shared)

