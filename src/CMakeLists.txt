# Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>

# The selected client profile controls the server topology. The logonserver
# serves every client version (classic realm list and Battle.net login); the
# Battle.net profiles add the modern version layer of the world transport.
# Do not expose a second BUILD_BATTLENET switch to users, the profile is the
# single source of truth.
if (NOT DEFINED ASCEMU_VERSION OR ASCEMU_VERSION STREQUAL "")
    set(ASCEMU_VERSION "WotLK" CACHE STRING "AscEmu client version")
else ()
    set(ASCEMU_VERSION "${ASCEMU_VERSION}" CACHE STRING "AscEmu client version")
endif ()

set_property(CACHE ASCEMU_VERSION PROPERTY STRINGS
    Classic
    TBC
    WotLK
    Cata
    Mop
    WoD
    Legion
    BfA
    Shadowlands
    Dragonflight
    TheWarWithin
    Midnight
    Forever
)

set(AE_USES_BATTLENET OFF)
set(AE_IS_FOREVER OFF)

# WoD (6.2.4), Legion (7.3.5), BfA (8.3.7), Shadowlands (9.2.7), Dragonflight (10.2.7), The War Within (11.2.7) and Midnight (12.1.5) clients log in through Battle.net as well,
# all with the protobuf login over TLS. They keep the classic world server and scripts;
# their world transport comes from src/version.
if (ASCEMU_VERSION STREQUAL "Forever")
    set(AE_USES_BATTLENET ON)
    set(AE_IS_FOREVER ON)
elseif (ASCEMU_VERSION STREQUAL "WoD" OR ASCEMU_VERSION STREQUAL "Legion" OR ASCEMU_VERSION STREQUAL "BfA" OR ASCEMU_VERSION STREQUAL "Shadowlands" OR ASCEMU_VERSION STREQUAL "Dragonflight" OR ASCEMU_VERSION STREQUAL "TheWarWithin" OR ASCEMU_VERSION STREQUAL "Midnight")
    set(AE_USES_BATTLENET ON)
endif ()

# Compatibility value for CMake files that still test BUILD_BATTLENET. Keep it
# internal so the selected client profile remains the single source of truth.
set(BUILD_BATTLENET ${AE_USES_BATTLENET} CACHE INTERNAL
    "Battle.net is selected automatically by ASCEMU_VERSION" FORCE)

# Human-readable profile names must not be emitted as C/C++ macros. Forever is
# also a C++ namespace in the version layer, so use a dedicated numeric token.
if (AE_IS_FOREVER)
    set(ASCEMU_VERSION_TOKEN "AE_PROFILE_FOREVER")
else ()
    set(ASCEMU_VERSION_TOKEN "${ASCEMU_VERSION}")
endif ()

# Forever keeps the legacy script stack disabled, but the standalone
# GossipScripts module is enabled explicitly in src/scripts/CMakeLists.txt.
# Do not enable BUILD_ASCEMUSCRIPTS here, otherwise all legacy modules would
# be pulled into the modern profile.
if (AE_IS_FOREVER)
    set(BUILD_ASCEMUSCRIPTS OFF CACHE BOOL
        "Build legacy AscEmu script modules" FORCE)
endif ()

add_subdirectory(shared)

# one logonserver for every client version: the classic realm list and the Battle.net login
add_subdirectory(logonserver)

if (AE_USES_BATTLENET)
    message(STATUS "ASCEMU_VERSION=${ASCEMU_VERSION}: Battle.net login profile")
    if (AE_IS_FOREVER)
        message(STATUS "  - legacy script stack: disabled")
    endif ()

    # The version directory provides the world transport of the selected
    # profile when it has one.
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/version/CMakeLists.txt")
        add_subdirectory(version)
    endif ()
else ()
    message(STATUS "ASCEMU_VERSION=${ASCEMU_VERSION}: classic login profile")
endif ()

add_subdirectory(world)
add_subdirectory(collision)

if (BUILD_TOOLS)
    add_subdirectory(tools)
endif ()

if (BUILD_EXTRAS)
    add_subdirectory(tools/extras)
endif ()
