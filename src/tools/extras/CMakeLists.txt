# Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>

if ("${ASCEMU_VERSION}" STREQUAL "WotLK" OR "${ASCEMU_VERSION}" STREQUAL "TBC" OR "${ASCEMU_VERSION}" STREQUAL "Classic")
    # set our variables
    set(BUILD_EXTRAS_CREATUREDATA FALSE CACHE BOOL "Build creature data extractor")
    set(BUILD_EXTRAS_SPELLFAILUREDATA FALSE CACHE BOOL "Build spell data extractor")

    if (BUILD_EXTRAS_CREATUREDATA)
        add_subdirectory(creature_data)
    endif ()

    if (BUILD_EXTRAS_SPELLFAILUREDATA)
        add_subdirectory(spellfailure_data)
    endif ()
endif ()

# dbc2sql works for any client version (it detects the right format itself,
# see its own CMakeLists.txt/source), so it isn't restricted to the
# WotLK/TBC/Classic-only block above like the other extras tools are.
# It supersedes the old spell_data/taxi_path_data tools (both were
# hardcoded, WotLK-only, single-DBC exporters).
set(BUILD_EXTRAS_DBC2SQL FALSE CACHE BOOL "Build generic DBC/DB2 to SQL converter")

if (BUILD_EXTRAS_DBC2SQL)
    add_subdirectory(dbc2sql)
endif ()
