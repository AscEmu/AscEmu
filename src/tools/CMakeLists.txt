# Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>

# check which tools are set
if (BUILD_TOOLS)
    # Built once, unconditionally: no per-client-version code path
    # (vmap4_assembler) or only runtime-detected tuning (mmaps_generator) -
    # see dep/mpqlib/ClientVersion.hpp.
    add_subdirectory(vmap_tools/vmap4_assembler)
    add_subdirectory(mmaps_generator)

    if ("${ASCEMU_VERSION}" STREQUAL "WotLK" OR "${ASCEMU_VERSION}" STREQUAL "TBC" OR "${ASCEMU_VERSION}" STREQUAL "Classic")
        add_subdirectory(map_extractor)
        add_subdirectory(vmap_tools)
    else ()
        add_subdirectory(ToolsCataMop/connection)
        add_subdirectory(ToolsCataMop/map_extractor)
        add_subdirectory(ToolsCataMop/vmap_tools)
    endif ()
endif ()

if (WIN32)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/ae_tools.bat DESTINATION ${ASCEMU_TOOLS_PATH})
else ()
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/ae_tools.sh DESTINATION ${ASCEMU_TOOLS_PATH})
endif ()
