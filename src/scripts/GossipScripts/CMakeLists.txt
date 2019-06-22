# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

if(NOT BUILD_GOSSIPSCRIPTS )
  message("\tGossipScripts was skipped because BUILD_GOSSIPSCRIPTS is unchecked.")
  return()
endif(NOT BUILD_GOSSIPSCRIPTS)

# set up our project name
project(GossipScripts CXX)

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
    # Required for linker
    ../../world/Server/ServerState.cpp
)

set(headers
   Setup.h
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
add_library(${PROJECT_NAME} SHARED ${sources} ${headers})

target_link_libraries(${PROJECT_NAME} world)

if(USE_PCH)
    gen_cxx_pch(${PROJECT_NAME} Setup.h ${CMAKE_CURRENT_SOURCE_DIR}/Setup.cpp)
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_SCRIPTLIB_PATH} LIBRARY DESTINATION ${ASCEMU_SCRIPTLIB_PATH})
unset(sources)
unset(headers)
