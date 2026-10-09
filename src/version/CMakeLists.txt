# Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>

# World transport of the client profile selected by ASCEMU_VERSION. Every profile
# keeps its own directory; profiles without one stay on the classic WorldSocket path.
# WoD and Legion share the RC4 transport in Shared/, BfA uses the AES transport there; the values of a
# profile come from <profile>/World/WorldProfile.hpp.
if (ASCEMU_VERSION STREQUAL "Forever")
    set(VERSION_CLIENT_DIRS Forever)
    set(VERSION_CLIENT_DEFINITIONS
        AE_HAS_WORLD_V2_PROFILE=1
        AE_WORLD_PROFILE_FOREVER=1
    )
elseif (ASCEMU_VERSION STREQUAL "WoD")
    set(VERSION_CLIENT_DIRS WoD Shared)
    set(VERSION_CLIENT_DEFINITIONS
        AE_WORLD_PROFILE_WOD=1
    )
elseif (ASCEMU_VERSION STREQUAL "Legion")
    set(VERSION_CLIENT_DIRS Legion Shared)
    set(VERSION_CLIENT_DEFINITIONS
        AE_WORLD_PROFILE_LEGION=1
    )
elseif (ASCEMU_VERSION STREQUAL "BfA")
    set(VERSION_CLIENT_DIRS BfA Shared)
    set(VERSION_CLIENT_DEFINITIONS
        AE_WORLD_PROFILE_BFA=1
    )
else ()
    return()
endif ()

list(GET VERSION_CLIENT_DIRS 0 VERSION_CLIENT_PROFILE_DIR)
if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${VERSION_CLIENT_PROFILE_DIR}")
    unset(VERSION_CLIENT_PROFILE_DIR)
    unset(VERSION_CLIENT_DIRS)
    unset(VERSION_CLIENT_DEFINITIONS)
    return()
endif ()

set(VERSION_CLIENT_SRCS)
foreach (VERSION_CLIENT_DIR ${VERSION_CLIENT_DIRS})
    file(GLOB_RECURSE VERSION_CLIENT_DIR_SRCS CONFIGURE_DEPENDS
        ${VERSION_CLIENT_DIR}/*.cpp
        ${VERSION_CLIENT_DIR}/*.h
        ${VERSION_CLIENT_DIR}/*.hpp
        ${VERSION_CLIENT_DIR}/*.inc
    )
    list(APPEND VERSION_CLIENT_SRCS ${VERSION_CLIENT_DIR_SRCS})
endforeach ()
unset(VERSION_CLIENT_DIR_SRCS)

add_library(version_client STATIC ${VERSION_CLIENT_SRCS})
target_compile_definitions(version_client PUBLIC ${VERSION_CLIENT_DEFINITIONS})

target_include_directories(version_client PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/shared
    ${CMAKE_SOURCE_DIR}/src/world
)

target_include_directories(version_client SYSTEM PRIVATE
    ${CMAKE_SOURCE_DIR}/dep/g3dlite/include
)

if (TARGET zlibstatic)
    set(VERSION_CLIENT_ZLIB_TARGET zlibstatic)
elseif (TARGET zlib)
    set(VERSION_CLIENT_ZLIB_TARGET zlib)
elseif (TARGET ZLIB::ZLIB)
    set(VERSION_CLIENT_ZLIB_TARGET ZLIB::ZLIB)
else ()
    message(FATAL_ERROR "version_client requires a zlib target")
endif ()

target_link_libraries(version_client
    PUBLIC
        shared
    PRIVATE
        ${VERSION_CLIENT_ZLIB_TARGET}
        g3dlite
)

unset(VERSION_CLIENT_ZLIB_TARGET)
ascemu_source_group(${CMAKE_CURRENT_SOURCE_DIR})
unset(VERSION_CLIENT_SRCS)
unset(VERSION_CLIENT_PROFILE_DIR)
unset(VERSION_CLIENT_DIRS)
unset(VERSION_CLIENT_DEFINITIONS)
