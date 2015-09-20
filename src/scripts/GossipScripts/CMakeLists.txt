# Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>

if(NOT BUILD_GOSSIPSCRIPTS )
  message("\tGossipScripts was skipped because BUILD_GOSSIPSCRIPTS is unchecked.")
  return()
endif(NOT BUILD_GOSSIPSCRIPTS)

# set up our project name
project(GossipScripts)

set(prefix ${SCRIPTS_PATH}/GossipScripts)

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

foreach(src IN ITEMS ${sources})
   set(SRCS ${SRCS} ${prefix}/${src})
endforeach(src)
foreach(src IN ITEMS ${headers})
   set(HEADERS ${HEADERS} ${prefix}/${src})
endforeach(src)

include_directories(${include_dirs})
add_library(${PROJECT_NAME} SHARED ${SRCS} ${HEADERS})
add_dependencies(${PROJECT_NAME} ${script_dependencies})
target_link_libraries(${PROJECT_NAME} ${script_linkages})

include(PCHSupport)
if(CMAKE_GENERATOR MATCHES Visual*)
   ADD_NATIVE_PRECOMPILED_HEADER(${PROJECT_NAME} Setup.h ${prefix}/Setup.cpp)
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_SCRIPTLIB_PATH} LIBRARY DESTINATION ${ASCEMU_SCRIPTLIB_PATH})
