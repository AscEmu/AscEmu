# Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>

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
