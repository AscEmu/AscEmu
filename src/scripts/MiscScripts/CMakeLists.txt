# Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>

if(NOT BUILD_MISCSCRIPTS)
  message("\tMiscScripts was skipped because BUILD_MISCSCRIPTS is unchecked.")
  return()
endif()

# set up our project name
project(MiscScripts)

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
   ../Common/Base.h
   ../Common/EasyFunctions.h
   ../Common/Instance_Base.h
)

include_directories(
   ${OPENSSL_INCLUDE_DIR}
   ${PCRE_INCLUDE_DIR}
   ${CMAKE_SOURCE_DIR}/dep/recastnavigation/Detour/Include
   ${CMAKE_SOURCE_DIR}/dep/vmaplib/include
   ${CMAKE_SOURCE_DIR}/dep/g3dlite/include
   ${CMAKE_SOURCE_DIR}/src/shared
   ${CMAKE_SOURCE_DIR}/src/world
   ${ZLIB_INCLUDE_DIRS}
)
add_library(${PROJECT_NAME} SHARED ${sources} ${headers})
#add_dependencies(${PROJECT_NAME} ${script_dependencies})
target_link_libraries(${PROJECT_NAME} world)

if(MSVC)
    include(PCHSupport)
    ADD_NATIVE_PRECOMPILED_HEADER(${PROJECT_NAME} Setup.h Setup.cpp)
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_SCRIPTLIB_PATH} LIBRARY DESTINATION ${ASCEMU_SCRIPTLIB_PATH})
unset(sources)
unset(headers)