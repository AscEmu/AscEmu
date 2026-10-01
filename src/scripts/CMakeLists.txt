# Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>

# Forever uses a dedicated modern protocol/data layer. The Classic-MoP script
# libraries (including Battlegrounds and SpellHandlers) must not be compiled
# into a Forever build.
if (ASCEMU_VERSION STREQUAL "Forever")
    message(STATUS "ASCEMU_VERSION=Forever: all legacy script modules are disabled")
    return()
endif ()

# all script libraries have SCRIPTLIB defined.
add_compile_options(-DSCRIPTLIB -DSKIP_ALLOCATOR_SHARING)

# add our modules.
add_subdirectory(Battlegrounds)
add_subdirectory(SpellHandlers)

# check if scripts are selected.
if (NOT ${BUILD_ASCEMUSCRIPTS})
    message(WARNING, "BUILD_ASCEMUSCRIPTS option is set to false. Script libraries will not be built.")
    return()
endif ()

add_subdirectory(EventScripts)
add_subdirectory(GossipScripts)
add_subdirectory(InstanceScripts)
add_subdirectory(LuaEngine)
add_subdirectory(MiscScripts)
add_subdirectory(QuestScripts)
#add_subdirectory(WorldPvPScripts)
