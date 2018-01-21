# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

#set our variables
set(BUILD_EXTRAS_CREATUREDATA FALSE CACHE BOOL "Build creature data extractor")
set(BUILD_EXTRAS_SPELLDATA FALSE CACHE BOOL "Build spell data extractor")
set(BUILD_EXTRAS_SPELLFAILUREDATA FALSE CACHE BOOL "Build spell data extractor")
set(BUILD_EXTRAS_TAXIPATHDATA FALSE CACHE BOOL "Build taxi path data extractor")

if(BUILD_EXTRAS_CREATUREDATA)
    add_subdirectory(creature_data)
endif()

if(BUILD_EXTRAS_SPELLDATA)
    add_subdirectory(spell_data)
endif()

if(BUILD_EXTRAS_SPELLFAILUREDATA)
    add_subdirectory(spellfailure_data)
endif()

if(BUILD_EXTRAS_TAXIPATHDATA)
    add_subdirectory(taxi_path_data)
endif()
