# Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(LIBRARY_NAME collision)

file(GLOB SRC_MANAGEMENT Management/*.cpp Management/*.h)
file(GLOB SRC_MAPS Maps/*.cpp Maps/*.h)
file(GLOB SRC_MODELS Models/*.cpp Models/*.h)
file(GLOB SRC_LOCAL_DIR *.cpp *.h)

set(COLLISION_SRCS
    ${SRC_MANAGEMENT}
    ${SRC_MAPS}
    ${SRC_MODELS}
    ${SRC_LOCAL_DIR}
)

# source group.
ascemu_source_group(${CMAKE_CURRENT_SOURCE_DIR})

add_library(${LIBRARY_NAME} STATIC ${COLLISION_SRCS})

target_include_directories(${LIBRARY_NAME} PUBLIC 
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

# include library dependencies.
target_link_libraries(${LIBRARY_NAME}
  PUBLIC
    g3dlite
    shared
)

# generate precompiled header.
if (AE_USE_PCH)
    message(STATUS "Generate PCH for ${LIBRARY_NAME}")
    target_precompile_headers(${LIBRARY_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pchCollision.hpp)
endif ()
