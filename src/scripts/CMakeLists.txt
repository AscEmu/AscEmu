# Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>

#check if scripts are selected
if(NOT BUILD_ASCEMUSCRIPTS)
  message(WARNING, "BUILD_ASCEMUSCRIPTS option is set to false. Script libraries will not be built.")
  return()
endif()

message("Configuring scripts :")
set(SCRIPTS_PATH ${ROOT_PATH}/src/scripts)

#common include directories that script libraries might include.
set(include_dirs ${GLOBAL_INCLUDE_DIRS})

#set up script dependencies and linkages
set(script_dependencies shared world)
set(script_linkages world)

#All script libraries have SCRIPTLIB defined.
add_definitions(-DSCRIPTLIB -DSKIP_ALLOCATOR_SHARING)
set(BUILD_EVENTSCRIPTS TRUE CACHE BOOL "Build ascEventScripts.")
set(BUILD_INSTANCESCRIPTS TRUE CACHE BOOL "Build ascInstanceScripts.")
set(BUILD_EXTRASCRIPTS TRUE CACHE BOOL "Build ascExtraScripts.")
set(BUILD_GOSSIPSCRIPTS TRUE CACHE BOOL "Build ascGossipScripts.")
set(BUILD_QUESTSCRIPTS TRUE CACHE BOOL "Build ascQuestScripts.")
set(BUILD_MISCSCRIPTS TRUE CACHE BOOL "Build ascMiscScripts." )
set(BUILD_LUAENGINE TRUE CACHE BOOL "Build LuaEngine.")
set(BUILD_LUABRIDGE FALSE CACHE BOOL "Build LuaBridge.")

# add our modules
add_subdirectory(Battlegrounds)
add_subdirectory(SpellHandlers)
add_subdirectory(EventScripts)
add_subdirectory(ExtraScripts)
add_subdirectory(GossipScripts)
add_subdirectory(InstanceScripts)
add_subdirectory(luabridge)
add_subdirectory(LuaEngine)
add_subdirectory(MiscScripts)
add_subdirectory(QuestScripts)
