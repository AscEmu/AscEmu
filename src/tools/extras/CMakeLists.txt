# Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>

# All three extras tools now detect what they need themselves at runtime
# (mpqlib::detectClientVersion() for creature_data/dbc2sql; spellfailure_data
# just reads whatever wow.exe it's pointed at, no version awareness needed at
# all) - none of them are restricted to a particular ASCEMU_VERSION build.
set(BUILD_EXTRAS_CREATUREDATA FALSE CACHE BOOL "Build creature data extractor")
set(BUILD_EXTRAS_DBC2SQL FALSE CACHE BOOL "Build generic DBC/DB2 to SQL converter")
set(BUILD_EXTRAS_SPELLFAILUREDATA FALSE CACHE BOOL "Build spell failure enum extractor")

if (BUILD_EXTRAS_CREATUREDATA)
    add_subdirectory(creature_data)
endif ()

if (BUILD_EXTRAS_DBC2SQL)
    add_subdirectory(dbc2sql)
endif ()

if (BUILD_EXTRAS_SPELLFAILUREDATA)
    add_subdirectory(spellfailure_data)
endif ()
