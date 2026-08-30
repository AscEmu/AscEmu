# Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>

# check which tools are set
if (BUILD_TOOLS)
    # Built once, unconditionally: no per-client-version code path
    # (vmap4_assembler), only runtime-detected tuning (mmaps_generator), or
    # fully runtime client-version detection and branching (map_extractor,
    # vmap_tools/vmap4_extractor, connection) - see dep/mpqlib/ClientVersion.hpp.
    add_subdirectory(vmap_tools/vmap4_assembler)
    add_subdirectory(vmap_tools)
    add_subdirectory(mmaps_generator)
    add_subdirectory(map_extractor)
    add_subdirectory(connection)
endif ()

if (WIN32)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/ae_tools.bat DESTINATION ${ASCEMU_TOOLS_PATH})
else ()
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/ae_tools.sh DESTINATION ${ASCEMU_TOOLS_PATH})
endif ()
