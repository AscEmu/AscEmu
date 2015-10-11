# Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>

if(NOT BUILD_MISCSCRIPTS )
  message("\tMiscScripts was skipped because BUILD_MISCSCRIPTS is unchecked.")
  return()
endif(NOT BUILD_MISCSCRIPTS)

# set up our project name
project(MiscScripts)

set(prefix ${SCRIPTS_PATH}/${PROJECT_NAME})

#sources -contains source files for our library.
set(sources
   City_Dalaran.cpp
   Creatures.cpp
   GameObjects.cpp
   GameObjectTeleportTable.cpp
   RandomScripts.cpp
   Setup.cpp
   ../Common/Base.cpp 
   )

set(headers
   Setup.h
   ../Common/EasyFunctions.h
   ../Common/Instance_Base.h
   ../Common/Base.h
   )

foreach(src IN ITEMS ${sources})
   set(SRCS ${SRCS} ${prefix}/${src})
endforeach(src)
foreach(src IN ITEMS ${headers})
   set(HEADERS ${HEADERS} ${prefix}/${src})
endforeach(src)

include_directories(${include_dirs} ${DEPENDENCY_ROOT}/zlib)
add_library(${PROJECT_NAME} SHARED ${SRCS} ${HEADERS})
add_dependencies(${PROJECT_NAME} ${script_dependencies})
target_link_libraries(${PROJECT_NAME} ${script_linkages})

include(PCHSupport)
if(CMAKE_GENERATOR MATCHES Visual*)
   ADD_NATIVE_PRECOMPILED_HEADER(${PROJECT_NAME} Setup.h ${prefix}/Setup.cpp)
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_SCRIPTLIB_PATH} LIBRARY DESTINATION ${ASCEMU_SCRIPTLIB_PATH})
