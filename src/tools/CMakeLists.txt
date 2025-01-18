# Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>

# check which tools are set
if (BUILD_TOOLS)
    if ("${ASCEMU_VERSION}" STREQUAL "WotLK" OR "${ASCEMU_VERSION}" STREQUAL "TBC" OR "${ASCEMU_VERSION}" STREQUAL "Classic")
        add_subdirectory(map_extractor)
        add_subdirectory(vmap_tools)
        add_subdirectory(mmaps_generator)
    else ()
        add_subdirectory(ToolsCataMop/map_extractor)
        add_subdirectory(ToolsCataMop/vmap_tools)
        add_subdirectory(ToolsCataMop/mmaps_generator)
    endif ()
endif ()

if (WIN32)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/ae_tools.bat DESTINATION ${ASCEMU_TOOLS_PATH})
else ()
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/ae_tools.sh DESTINATION ${ASCEMU_TOOLS_PATH})
endif ()
