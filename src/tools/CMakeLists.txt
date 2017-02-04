# Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>

#check which tools are set
if(BUILD_TOOLS)
    add_subdirectory(map_extractor)
    add_subdirectory(vmap_tools)
    add_subdirectory(mmaps_generator)
endif()
