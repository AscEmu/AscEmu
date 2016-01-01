# Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>

if(NOT BUILD_LUAENGINE)
  message("\tLuaEngine was skipped because BUILD_LUAENGINE is unchecked.")
  return()
endif(NOT BUILD_LUAENGINE)

# set up our project name
project(LuaEngine)

set(prefix ${SCRIPTS_PATH}/${PROJECT_NAME})

#SRCS - contains the source files that compose this project.
set(SRCS
   ${prefix}/LUAEngine.cpp 
   )

set(HEADERS
   ${prefix}/AuraFunctions.h
   ${prefix}/FunctionTables.h
   ${prefix}/GameobjectFunctions.h
   ${prefix}/GlobalFunctions.h
   ${prefix}/ItemFunctions.h
   ${prefix}/LUAEngine.h
   ${prefix}/LUAFunctions.h
   ${prefix}/LuaSqlApi.h
   ${prefix}/PacketFunctions.h
   ${prefix}/SpellFunctions.h
   ${prefix}/TaxiFunctions.h
   ${prefix}/UnitFunctions.h
   )

include_directories(${include_dirs} ${prefix} ${DEPENDENCY_ROOT}/zlib)
IF(IS_LINUX)
	add_definitions(-Wno-deprecated)
ENDIF()

add_library(${PROJECT_NAME} SHARED ${SRCS} ${HEADERS})
add_dependencies(${PROJECT_NAME} ${script_dependencies} lualib)
target_link_libraries(${PROJECT_NAME} ${script_linkages} lualib)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_SCRIPTLIB_PATH} LIBRARY DESTINATION ${ASCEMU_SCRIPTLIB_PATH})
