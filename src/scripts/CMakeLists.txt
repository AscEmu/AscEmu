# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

#All script libraries have SCRIPTLIB defined.
add_definitions(-DSCRIPTLIB -DSKIP_ALLOCATOR_SHARING)

# add our modules
add_subdirectory(Battlegrounds)
add_subdirectory(SpellHandlers)

#check if scripts are selected
if(NOT ${BUILD_ASCEMUSCRIPTS})
  message(WARNING, "BUILD_ASCEMUSCRIPTS option is set to false. Script libraries will not be built.")
  return()
endif()

add_subdirectory(EventScripts)
add_subdirectory(ExtraScripts)
add_subdirectory(GossipScripts)
add_subdirectory(InstanceScripts)
add_subdirectory(LuaEngine)
add_subdirectory(MapScripts)
add_subdirectory(MiscScripts)
add_subdirectory(QuestScripts)
