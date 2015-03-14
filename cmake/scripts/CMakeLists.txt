#child CMakeLists use this variable to access the source file directories. 
#e.g ${SCRIPTS_PATH}/LuaEngine to access the source files for LuaEngine.

if( NOT BUILD_ASCEMUSCRIPTS )
  message( WARNING, "BUILD_ASCEMUSCRIPTS option is set to false. Script libraries will not be built." )
  return()
endif(NOT BUILD_ASCEMUSCRIPTS)
message("Configuring scripts :")
SET( SCRIPTS_PATH ${ROOT_PATH}/src/scripts )
#common include directories that script libraries might include.
SET( include_dirs ${GLOBAL_INCLUDE_DIRS} )
SET( dependancies shared world )
SET( linkages world)

#All script libraries have SCRIPTLIB defined.
add_definitions(-DSCRIPTLIB -DSKIP_ALLOCATOR_SHARING)
SET( BUILD_EVENTSCRIPTS TRUE CACHE BOOL "Build ascEventScripts.")
SET( BUILD_INSTANCESCRIPTS TRUE CACHE BOOL "Build ascInstanceScripts.")
SET( BUILD_EXTRASCRIPTS TRUE CACHE BOOL "Build ascExtraScripts.")
SET( BUILD_GOSSIPSCRIPTS TRUE CACHE BOOL "Build ascGossipScripts.")
SET( BUILD_QUESTSCRIPTS TRUE CACHE BOOL "Build ascQuestScripts.")
SET( BUILD_MISCSCRIPTS TRUE CACHE BOOL "Build ascMiscScripts." )
SET( BUILD_SERVERSTATUSPLUGIN TRUE CACHE BOOL "Build ServerStatusPlugin.")
SET( BUILD_LUAENGINE TRUE CACHE BOOL "Build Ascemu Lua Engine ( ALE ).")
SET( BUILD_LUABRIDGE FALSE CACHE BOOL "Build luabridge.")


add_subdirectory( eventscripts)
add_subdirectory( instancescripts )
add_subdirectory( extrascripts )
add_subdirectory( gossipscripts )
add_subdirectory( questscripts )
add_subdirectory( miscscripts )
add_subdirectory( serverstatusplugin )
add_subdirectory( lualib ) #Used by luabridge and luaengine modules.
add_subdirectory( luabridge ) 
add_subdirectory( luaengine )

unset(SCRIPTS_PATH)
unset(include_dirs)