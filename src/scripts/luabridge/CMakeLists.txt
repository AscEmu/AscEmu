# Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>

if(NOT BUILD_LUABRIDGE)
  message("\tluabridge was skipped because BUILD_LUABRIDGE is unchecked.")
  return()
endif(NOT BUILD_LUABRIDGE)

if(BUILD_LUAENGINE)
	message(ERROR, "\tBUILD_LUAENGINE is enabled but the LuaEngine cannot run together with luabridge.")
	return()
endif(BUILD_LUAENGINE)

# set up our project name
project(luabridge)

set(prefix ${SCRIPTS_PATH}/${PROJECT_NAME})

#SRCS - contains the source files that compose this project.
set(SRCS
  ${prefix}/interpreter/LUAEngine.cpp 
  ${prefix}/luabridge_src/luabridge.cpp 
  ${prefix}/object_functions/aura_functions.cpp
  ${prefix}/object_functions/gameobject_functions.cpp
  ${prefix}/object_functions/global_functions.cpp 
  ${prefix}/object_functions/instance_functions.cpp 
  ${prefix}/object_functions/item_functions.cpp 
  ${prefix}/object_functions/object_functions.cpp 
  ${prefix}/object_functions/packet_functions.cpp 
  ${prefix}/object_functions/player_functions.cpp 
  ${prefix}/object_functions/quest_functions.cpp 
  ${prefix}/object_functions/register_functions.cpp 
  ${prefix}/object_functions/spell_functions.cpp 
  ${prefix}/object_functions/sql_functions.cpp 
  ${prefix}/object_functions/taxi_functions.cpp 
  ${prefix}/object_functions/unit_functions.cpp 
  ${prefix}/object_functions/creature_functions.cpp
  ${prefix}/object_functions/lua_hooks.cpp
  )
  
set(HEADERS
  ${prefix}/interpreter/class_decls.h
  ${prefix}/interpreter/lua_defs.h
  ${prefix}/interpreter/lua_instance.h
  ${prefix}/interpreter/LUAEngine.h
  ${prefix}/luabridge_src/luabridge.hpp
  ${prefix}/object_functions/GlobalFunctions.h
  ${prefix}/object_functions/lua_creature.h
  ${prefix}/object_functions/lua_gameobject.h
  )

include_directories(${include_dirs} ${prefix}/interpreter)
include(PCHSupport)
add_library(${PROJECT_NAME} SHARED ${SRCS} ${HEADERS})
add_dependencies(${PROJECT_NAME} ${script_dependencies} lualib)
target_link_libraries(${PROJECT_NAME} ${script_linkages} lualib)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_SCRIPTLIB_PATH} LIBRARY DESTINATION ${ASCEMU_SCRIPTLIB_PATH})

if(CMAKE_GENERATOR MATCHES Visual*)
   ADD_NATIVE_PRECOMPILED_HEADER( ${PROJECT_NAME} LUAEngine.h ${prefix}/interpreter/LUAEngine.cpp)
endif()
