# Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>

# Forever protocol implementation selected by ASCEMU_VERSION.
if (NOT ASCEMU_VERSION STREQUAL "Forever")
    message(FATAL_ERROR "src/version is currently available only for ASCEMU_VERSION=Forever")
endif ()

file(GLOB_RECURSE FOREVER_VERSION_SRCS CONFIGURE_DEPENDS
    Forever/*.cpp
    Forever/*.h
    Forever/*.hpp
    Forever/*.inc
)

add_library(version_client STATIC ${FOREVER_VERSION_SRCS})
target_compile_definitions(version_client PUBLIC
    AE_HAS_WORLD_V2_PROFILE=1
    AE_WORLD_PROFILE_FOREVER=1
)

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
unset(FOREVER_VERSION_SRCS)
