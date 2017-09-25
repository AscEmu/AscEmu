# Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>

if(NOT BUILD_MAPSCRIPTS )
  message("\tMapScripts was skipped because BUILD_MAPSCRIPTS is unchecked.")
  return()
endif()

project(MapScripts CXX)

include(Northrend/IceCrownCitadel/CMakeLists.txt)

set(SRC_BASIC
   Setup.cpp
   Setup.h
   ../Common/Instance_Base.cpp 
   ../Common/Instance_Base.h
   ../Common/Base.cpp
   ../Common/Base.h
   ../Common/AITimerCollection.cpp
   ../Common/AITimerCollection.h
   ../../world/Server/LazyTimer.cpp
   ../../world/Server/ServerState.cpp
)

set(sources
   ${SRC_BASIC}
   ${SRC_NORTHREND_ICC_FILES}
)

include_directories(
   ${OPENSSL_INCLUDE_DIR}
   ${PCRE_INCLUDE_DIR}
   ${CMAKE_SOURCE_DIR}/dep/recastnavigation/Detour/Include
   ${CMAKE_SOURCE_DIR}/src/collision
   ${CMAKE_SOURCE_DIR}/src/collision/Management
   ${CMAKE_SOURCE_DIR}/src/collision/Maps
   ${CMAKE_SOURCE_DIR}/src/collision/Models
   ${CMAKE_SOURCE_DIR}/dep/g3dlite/include
   ${CMAKE_SOURCE_DIR}/src/shared
   ${CMAKE_SOURCE_DIR}/src/world
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${ZLIB_INCLUDE_DIRS}
)

add_library(${PROJECT_NAME} SHARED ${sources})

target_link_libraries(${PROJECT_NAME} world shared)

if(USE_PCH AND PCHSupport_FOUND)
    ADD_NATIVE_PRECOMPILED_HEADER(${PROJECT_NAME} Setup.h ${CMAKE_CURRENT_SOURCE_DIR}/Setup.cpp)
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_SCRIPTLIB_PATH} LIBRARY DESTINATION ${ASCEMU_SCRIPTLIB_PATH})
unset(sources)
