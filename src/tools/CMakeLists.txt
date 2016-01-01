# Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>

#set tool path
set(ASCEMU_TOOLS_PATH "tools" CACHE PATH "The directory where you want the tools installed.")
set(BUILD_TOOLS_MAP_EXTRACTOR FALSE CACHE BOOL "Build DBC and Map extractors.")
set(BUILD_TOOLS_VMAPS FALSE CACHE BOOL "Build VMAP extractor tools.")
set(BUILD_TOOLS_MMAPS_GENERATOR FALSE CACHE BOOL "Build MMAPS generator tools.")

#check which tools are set
if(BUILD_TOOLS_MAP_EXTRACTOR)
	add_subdirectory(map_extractor)
endif()

if(BUILD_TOOLS_VMAPS)
	add_subdirectory(vmap_tools)
endif()

if(BUILD_TOOLS_MMAPS_GENERATOR)
	add_subdirectory(mmaps_generator)
endif()
