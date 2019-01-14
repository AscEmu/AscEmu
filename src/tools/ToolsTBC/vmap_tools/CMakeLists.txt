# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

add_subdirectory(vmap4_extractor)
add_subdirectory(vmap4_assembler)

if(WIN32)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/vmaps.bat DESTINATION ${ASCEMU_TOOLS_PATH})
else()
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/vmaps.sh DESTINATION ${ASCEMU_TOOLS_PATH})
endif()
