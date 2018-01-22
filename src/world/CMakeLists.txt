# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

# set up our project name
project(world CXX)

include(Game${ASCEMU_VERSION}/CMakeLists.txt)

include(Chat/CMakeLists.txt)
include(Chat/Commands/CMakeLists.txt)
include(Management/CMakeLists.txt)
include(Management/Battleground/CMakeLists.txt)
include(Management/Gossip/CMakeLists.txt)
include(Management/Guild/CMakeLists.txt)
include(Management/LFG/CMakeLists.txt)
include(Map/CMakeLists.txt)
include(Map/Area/CMakeLists.txt)
include(Movement/CMakeLists.txt)
include(Movement/Spline/CMakeLists.txt)
include(Objects/CMakeLists.txt)
include(Server/CMakeLists.txt)
include(Server/Console/CMakeLists.txt)
include(Server/LogonCommClient/CMakeLists.txt)
include(Server/Packets/CMakeLists.txt)
include(Server/Packets/Handlers/CMakeLists.txt)
include(Server/Packets/Movement/CMakeLists.txt)
include(Server/Script/CMakeLists.txt)
include(Server/Warden/CMakeLists.txt)
include(Spell/CMakeLists.txt)
include(Spell/Customization/CMakeLists.txt)
include(Spell/Definitions/CMakeLists.txt)
include(Storage/CMakeLists.txt)
if("${ASCEMU_VERSION}" STREQUAL "Cata")
   include(Storage/DB2/CMakeLists.txt)
endif()
include(Storage/DBC/CMakeLists.txt)
include(Units/CMakeLists.txt)
include(Units/Creatures/CMakeLists.txt)
include(Units/Players/CMakeLists.txt)
include(Units/Summons/CMakeLists.txt)

set(SRC_WORLD_PROJECT
   ${CMAKE_CURRENT_SOURCE_DIR}/StdAfx.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/StdAfx.h
)

if(WIN32)
   set(SRC_WORLD_PROJECT ${SRC_WORLD_PROJECT} ${CMAKE_CURRENT_SOURCE_DIR}/resources.rc)
endif()

source_group(world FILES ${SRC_WORLD_PROJECT})

set(sources
   ${SRC_GAMECLASSIC_FILES}
   
   ${SRC_GAMEBC_FILES}
   ${SRC_GAMEBC_HANDLER_FILES}
   ${SRC_GAMEBC_NETWORK_FILES}
   ${SRC_GAMEBC_NETWORK_PACKETS_FILES}
   ${SRC_GAMEBC_STORAGE_FILES}
   
   ${SRC_GAMELK_FILES}
   ${SRC_GAMELK_HANDLER_FILES}
   ${SRC_GAMELK_NETWORK_FILES}
   ${SRC_GAMELK_STORAGE_FILES}

   ${SRC_GAMECATA_FILES}
   ${SRC_GAMECATA_HANDLER_FILES}
   ${SRC_GAMECATA_MANAGEMENT_FILES}
   ${SRC_GAMECATA_MOVEMENT_FILES}
   ${SRC_GAMECATA_NETWORK_FILES}
   ${SRC_GAMECATA_STORAGE_FILES}
   ${SRC_GAMECATA_UNITS_FILES}
   ${SRC_GAMECATA_UNITS_PLAYERS_FILES}
   ${SRC_GAMECATA_SPELL_FILES}

   ${SRC_CHAT_FILES}
   ${SRC_COMMAND_FILES}
   ${SRC_MANAGEMENT_FILES}
   ${SRC_MANAGEMENT_BATTLEGROUND_FILES}
   ${SRC_MANAGEMENT_GOSSIP_FILES}
   ${SRC_MANAGEMENT_GUILD_FILES}
   ${SRC_MANAGEMENT_LFG_FILES}
   ${SRC_MAP_FILES}
   ${SRC_MAP_AREA_FILES}
   ${SRC_MOVEMENT_FILES}
   ${SRC_MOVEMENT_SPLINE_FILES}
   ${SRC_OBJECTS_FILES}
   ${SRC_PACKET_FILES}
   ${SRC_PACKET_HANDLER_FILES}
   ${SRC_PACKET_MOVEMENT_FILES}
   ${SRC_SERVER_FILES}
   ${SRC_SERVER_CONSOLE_FILES}
   ${SRC_SERVER_SCRIPT_FILES}
   ${SRC_SERVER_WARDEN_FILES}
   ${SRC_LOGON_COMMUNICATION_CLIENT_FILES}
   ${SRC_SPELL_FILES}
   ${SRC_SPELL_CUSTOM_FILES}
   ${SRC_SPELL_DEFINITIONS_FILES}
   ${SRC_STORAGE_FILES}
   ${SRC_STORAGE_DBC_FILES}
   ${SRC_UNITS_FILES}
   ${SRC_UNITS_CREATURES_FILES}
   ${SRC_UNITS_PLAYERS_FILES}
   ${SRC_UNITS_SUMMONS_FILES}
   ${SRC_WORLD_PROJECT}
   
   # Required for linker
   ../collision/Management/VMapFactory.cpp
   ../collision/Management/VMapManager2.cpp
   ../collision/Management/MMapFactory.cpp
   ../collision/Management/MMapManager.cpp
   ../collision/Models/ModelInstance.cpp
   ../collision/Models/WorldModel.cpp
   ../collision/Models/GameObjectModel.cpp
   ../collision/Maps/MapTree.cpp
   ../collision/Maps/TileAssembler.cpp
   ../collision/BoundingIntervalHierarchy.cpp
)

if("${ASCEMU_VERSION}" STREQUAL "Cata")
   set(sources
      ${sources}
      ${SRC_STORAGE_DB2_FILES})
endif()

include_directories(
   ${OPENSSL_INCLUDE_DIR}
   ${PCRE_INCLUDE_DIR}
   ${CMAKE_SOURCE_DIR}/dep/recastnavigation/Detour/Include
   ${CMAKE_SOURCE_DIR}/dep/recastnavigation/Recast/Include
   ${CMAKE_SOURCE_DIR}/src/collision
   ${CMAKE_SOURCE_DIR}/src/collision/Management
   ${CMAKE_SOURCE_DIR}/src/collision/Maps
   ${CMAKE_SOURCE_DIR}/src/collision/Models
   ${CMAKE_SOURCE_DIR}/dep/g3dlite/include
   ${CMAKE_SOURCE_DIR}/dep/lualib/src
   ${CMAKE_SOURCE_DIR}/src/shared
   ${CMAKE_SOURCE_DIR}/src
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${ZLIB_INCLUDE_DIRS}
)

add_executable(${PROJECT_NAME} ${sources})

if(USE_PCH AND PCHSupport_FOUND)
    ADD_NATIVE_PRECOMPILED_HEADER(${PROJECT_NAME} StdAfx.h ${CMAKE_CURRENT_SOURCE_DIR}/StdAfx.cpp)
endif()

if(WIN32 AND NOT USE_PCH)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/INCREMENTAL:NO")
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES ENABLE_EXPORTS TRUE)
add_dependencies(${PROJECT_NAME} shared g3dlite Detour Recast lualib)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/WorldConf.h.in ${CMAKE_CURRENT_SOURCE_DIR}/WorldConf.h)

target_link_libraries(${PROJECT_NAME} shared g3dlite Detour Recast ${PCRE_LIBRARIES})
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)
