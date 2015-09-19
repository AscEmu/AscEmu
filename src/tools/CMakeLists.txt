#set tool path
SET(ASCEMU_TOOLS_PATH "tools" CACHE PATH "The directory where you want the tools installed." )
SET(BUILD_TOOLS_MAP_EXTRACTOR TRUE CACHE BOOL "Build DBC and Map extractors." )
SET(BUILD_TOOLS_VMAPS TRUE CACHE BOOL "Build VMAP extractor tools." )
SET(BUILD_TOOLS_MMAPS_GENERATOR FALSE CACHE BOOL "Build MMAPS generator tools." )
SET(BUILD_TOOLS_CREATUREDATA TRUE CACHE BOOL "Build creature data extractor tools" )

#check which tools are set
IF( BUILD_TOOLS_MAP_EXTRACTOR )
	add_subdirectory( map_extractor )
ENDIF( BUILD_TOOLS_MAP_EXTRACTOR )

IF( BUILD_TOOLS_CREATUREDATA )
	add_subdirectory( creature_data )
ENDIF( BUILD_TOOLS_CREATUREDATA )

IF( BUILD_TOOLS_VMAPS )
	add_subdirectory( vmap_tools )
ENDIF( BUILD_TOOLS_VMAPS )

IF( BUILD_TOOLS_MMAPS_GENERATOR )
	add_subdirectory( mmaps_generator )
ENDIF( BUILD_TOOLS_MMAPS_GENERATOR )
