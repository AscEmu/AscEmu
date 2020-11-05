# Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>

#check which tools are set
if(BUILD_TOOLS)
   if("${ASCEMU_VERSION}" STREQUAL "WotLK" OR "${ASCEMU_VERSION}" STREQUAL "TBC" OR "${ASCEMU_VERSION}" STREQUAL "Classic")
      add_subdirectory(map_extractor)
	  add_subdirectory(vmap_tools)
      add_subdirectory(mmaps_generator)
   else()
      add_subdirectory(Tools${ASCEMU_VERSION}/map_extractor)
	  add_subdirectory(Tools${ASCEMU_VERSION}/vmap_tools)
      add_subdirectory(Tools${ASCEMU_VERSION}/mmaps_generator)
   endif()
endif()
