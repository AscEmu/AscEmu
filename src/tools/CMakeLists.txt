# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

#check which tools are set
if (BUILD_TOOLS)
   add_subdirectory(Tools${ASCEMU_VERSION}/map_extractor)
   add_subdirectory(Tools${ASCEMU_VERSION}/vmap_tools)
   add_subdirectory(Tools${ASCEMU_VERSION}/mmaps_generator)
endif()
