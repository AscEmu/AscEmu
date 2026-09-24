# Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>

# The selected client profile controls the complete server topology.
# Legacy clients use logonserver; Forever uses Battle.net and the modern
# version layer. Do not expose a second BUILD_BATTLENET switch to users,
# because combinations such as Forever + legacy logonserver are invalid.
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
    Forever
)

set(AE_USES_BATTLENET OFF)
set(AE_IS_FOREVER OFF)

if (ASCEMU_VERSION STREQUAL "Forever")
    set(AE_USES_BATTLENET ON)
    set(AE_IS_FOREVER ON)
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

# Modern profiles intentionally do not build the legacy script stack. Force the
# cache value too so the generated build configuration is unambiguous in IDEs.
if (AE_IS_FOREVER)
    set(BUILD_ASCEMUSCRIPTS OFF CACHE BOOL
        "Build legacy AscEmu script modules" FORCE)
endif ()

add_subdirectory(shared)

if (AE_USES_BATTLENET)
    message(STATUS "ASCEMU_VERSION=${ASCEMU_VERSION}: Battle.net topology enabled")
    message(STATUS "  - logonserver: disabled")
    message(STATUS "  - bnetserver: enabled")
    message(STATUS "  - legacy scripts: disabled")

    # Stage 1 already owns the topology. The version directory is added as soon
    # as the Forever implementation is present (Stage 2) without another CMake
    # topology change.
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/version/CMakeLists.txt")
        add_subdirectory(version)
    endif ()

    add_subdirectory(battlenet)
else ()
    message(STATUS "ASCEMU_VERSION=${ASCEMU_VERSION}: legacy logonserver topology enabled")
    add_subdirectory(logonserver)
endif ()

add_subdirectory(world)
add_subdirectory(collision)

if (BUILD_TOOLS)
    add_subdirectory(tools)
endif ()

if (BUILD_EXTRAS)
    add_subdirectory(tools/extras)
endif ()
