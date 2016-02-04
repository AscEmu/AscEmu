# Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>

if(NOT BUILD_GOSSIPSCRIPTS )
  message("\tGossipScripts was skipped because BUILD_GOSSIPSCRIPTS is unchecked.")
  return()
endif(NOT BUILD_GOSSIPSCRIPTS)

# set up our project name
project(GossipScripts)

#sources -contains source files for our library.
set(sources
   Gossip_Dalaran.cpp
   Gossip_Innkeepers.cpp
   Gossip_Guard.cpp
   Gossip_Trainer.cpp
   Gossip_Mulgore.cpp
   Gossip_Shattrath.cpp
   Gossip_Tanaris.cpp
   Gossip_Stormwind.cpp
   Gossip_Moonglade.cpp
   Gossip_Theramore.cpp
   Gossip_Teldrassil.cpp
   Gossip_BoreanTundra.cpp
   Gossip_WyrmrestTemple.cpp
   Gossip_XpEliminator.cpp
   Setup.cpp
)

set(headers
   Setup.h
)

include_directories(
   ${OPENSSL_INCLUDE_DIR}
   ${PCRE_INCLUDE_DIR}
   ${CMAKE_SOURCE_DIR}/dep/recastnavigation/Detour/Include
   ${CMAKE_SOURCE_DIR}/dep/vmaplib/include
   ${CMAKE_SOURCE_DIR}/dep/g3dlite/include
   ${CMAKE_SOURCE_DIR}/src/shared
   ${CMAKE_SOURCE_DIR}/src/world
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${ZLIB_INCLUDE_DIRS}
)
add_library(${PROJECT_NAME} SHARED ${sources} ${headers})

# special case for project linking
# on some operating systems linking is a bit strange
# OS X can't link executable, MSVC can't link world library... weird
if(WIN32)
    add_dependencies(${PROJECT_NAME} worldserver)
    target_link_libraries(${PROJECT_NAME} worldserver)
elseif(APPLE)
    add_dependencies(${PROJECT_NAME} world)
    target_link_libraries(${PROJECT_NAME} world)
endif()
if(USE_PCH AND PCHSupport_FOUND)
    ADD_NATIVE_PRECOMPILED_HEADER(${PROJECT_NAME} Setup.h ${CMAKE_CURRENT_SOURCE_DIR}/Setup.cpp)
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_SCRIPTLIB_PATH} LIBRARY DESTINATION ${ASCEMU_SCRIPTLIB_PATH})
unset(sources)
unset(headers)
