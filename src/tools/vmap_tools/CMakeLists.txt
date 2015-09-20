# Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>

add_subdirectory(vmap_extractor)
add_subdirectory(vmap_assembler)

if(WIN32)
	install(FILES ${ROOT_PATH}/src/tools/vmap_tools/vmaps.bat DESTINATION ${ASCEMU_TOOLS_PATH})
else(WIN32)
	install(FILES ${ROOT_PATH}/src/tools/vmap_tools/vmaps.sh DESTINATION ${ASCEMU_TOOLS_PATH})
endif(WIN32)
