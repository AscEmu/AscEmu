# Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>

#set tool path
set(ASCEMU_TOOLS_PATH "tools" CACHE PATH "The directory where you want the tools installed.")
set(BUILD_TOOLS_MAP_EXTRACTOR TRUE CACHE BOOL "Build DBC and Map extractors.")
set(BUILD_TOOLS_VMAPS TRUE CACHE BOOL "Build VMAP extractor tools.")
set(BUILD_TOOLS_MMAPS_GENERATOR FALSE CACHE BOOL "Build MMAPS generator tools.")
set(BUILD_TOOLS_CREATUREDATA TRUE CACHE BOOL "Build creature data extractor tools")

#check which tools are set
if(BUILD_TOOLS_MAP_EXTRACTOR)
	add_subdirectory(map_extractor)
endif(BUILD_TOOLS_MAP_EXTRACTOR)

if(BUILD_TOOLS_CREATUREDATA)
	add_subdirectory(creature_data)
endif(BUILD_TOOLS_CREATUREDATA)

if(BUILD_TOOLS_VMAPS)
	add_subdirectory(vmap_tools)
endif(BUILD_TOOLS_VMAPS)

if(BUILD_TOOLS_MMAPS_GENERATOR)
	add_subdirectory(mmaps_generator)
endif(BUILD_TOOLS_MMAPS_GENERATOR)
