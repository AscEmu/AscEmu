# Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>

#check if scripts are selected
if(NOT ${BUILD_ASCEMUSCRIPTS})
  message(WARNING, "BUILD_ASCEMUSCRIPTS option is set to false. Script libraries will not be built.")
  return()
endif()

#common include directories that script libraries might include.
set(include_dirs ${GLOBAL_INCLUDE_DIRS})

#All script libraries have SCRIPTLIB defined.
add_definitions(-DSCRIPTLIB -DSKIP_ALLOCATOR_SHARING)

# add our modules
add_subdirectory(Battlegrounds)
add_subdirectory(SpellHandlers)
add_subdirectory(EventScripts)
add_subdirectory(ExtraScripts)
add_subdirectory(GossipScripts)
add_subdirectory(InstanceScripts)
add_subdirectory(LuaEngine)
add_subdirectory(MiscScripts)
add_subdirectory(QuestScripts)
