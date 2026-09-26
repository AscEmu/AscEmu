# Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>

# all script libraries have SCRIPTLIB defined.
add_compile_options(-DSCRIPTLIB -DSKIP_ALLOCATOR_SHARING)

# Forever keeps the broad Classic-MoP script stack disabled, but GossipScripts
# is useful independently of the legacy logon topology and uses the common
# ScriptMgr/GossipScript interfaces from world. Build only that module.
if (ASCEMU_VERSION STREQUAL "Forever")
    set(BUILD_GOSSIPSCRIPTS ON CACHE BOOL
        "Build GossipScripts module for Forever" FORCE)

    message(STATUS "ASCEMU_VERSION=Forever: enabling GossipScripts only")
    add_subdirectory(GossipScripts)
    return()
endif ()

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
