# Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>

# set up our project name
project(world CXX)

include(Game${ASCEMU_VERSION}/CMakeLists.txt)

include(Chat/CMakeLists.txt)
include(Chat/Commands/CMakeLists.txt)
include(Data/CMakeLists.txt)
include(Management/CMakeLists.txt)
include(Management/Battleground/CMakeLists.txt)
include(Management/Gossip/CMakeLists.txt)
include(Management/Guild/CMakeLists.txt)
include(Management/LFG/CMakeLists.txt)
include(Management/ObjectUpdates/CMakeLists.txt)
include(Management/Tickets/CMakeLists.txt)
include(Macros/CMakeLists.txt)
include(Map/CMakeLists.txt)
include(Map/Area/CMakeLists.txt)
include(Map/Cells/CMakeLists.txt)
include(Map/Management/CMakeLists.txt)
include(Map/Maps/CMakeLists.txt)
include(Movement/CMakeLists.txt)
include(Movement/Spline/CMakeLists.txt)
include(Movement/MovementGenerators/CMakeLists.txt)
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

if ("${ASCEMU_VERSION}" STREQUAL "Cata")
    include(Storage/DB2/CMakeLists.txt)
endif ()

if ("${ASCEMU_VERSION}" STREQUAL "Mop")
    include(Storage/DB2/CMakeLists.txt)
endif ()

include(Storage/DBC/CMakeLists.txt)
include(Objects/Units/CMakeLists.txt)
include(Objects/Units/Creatures/CMakeLists.txt)
include(Objects/Units/Players/CMakeLists.txt)
include(Objects/Units/Creatures/Summons/CMakeLists.txt)

if (WIN32)
    set(SRC_WORLD_PROJECT ${SRC_WORLD_PROJECT} ${CMAKE_CURRENT_SOURCE_DIR}/resources.rc)
endif ()

source_group(world FILES ${SRC_WORLD_PROJECT})

set(sources
    ${SRC_GAMECLASSIC_FILES}
    ${SRC_GAMECLASSIC_STORAGE_FILES}
   
    ${SRC_GAMEBC_FILES}
    ${SRC_GAMEBC_STORAGE_FILES}
   
    ${SRC_GAMELK_FILES}
    ${SRC_GAMELK_STORAGE_FILES}

    ${SRC_GAMECATA_FILES}
    ${SRC_GAMECATA_MANAGEMENT_FILES}
    ${SRC_GAMECATA_STORAGE_FILES}
   
    ${SRC_GAMEMOP_FILES}
    ${SRC_GAMEMOP_MANAGEMENT_FILES}
    ${SRC_GAMEMOP_STORAGE_FILES}

    ${SRC_AI_FILES}
    ${SRC_MACROS_FILES}
    ${SRC_CHAT_FILES}
    ${SRC_COMMAND_FILES}
    ${SRC_DATA_FILES}
    ${SRC_MANAGEMENT_FILES}
    ${SRC_MANAGEMENT_BATTLEGROUND_FILES}
    ${SRC_MANAGEMENT_GOSSIP_FILES}
    ${SRC_MANAGEMENT_GUILD_FILES}
    ${SRC_MANAGEMENT_LFG_FILES}
    ${SRC_MANAGEMENT_OBJECTUPDATES_FILES}
    ${SRC_MANAGEMENT_TICKETS_FILES}
    ${SRC_MAP_FILES}
    ${SRC_MAP_AREA_FILES}
    ${SRC_MAP_AREA_FILES}
    ${SRC_MAP_CELLS_FILES}
    ${SRC_MAP_MANAGEMENT_FILES}
    ${SRC_MAP_MAPS_FILES}
    ${SRC_MOVEMENT_FILES}
    ${SRC_MOVEMENT_SPLINE_FILES}
    ${SRC_MOVEMENT_MOVEMENTGENERATOR_FILES}
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

    # required for linker
    ../collision/Management/VMapFactory.cpp
    ../collision/Management/VMapManager2.cpp
    ../collision/Management/MMapFactory.cpp
    ../collision/Management/MMapManager.cpp
    ../collision/Models/ModelInstance.cpp
    ../collision/Models/WorldModel.cpp
    ../collision/Models/GameObjectModel.cpp
    ../collision/Maps/MapTree.cpp
    ../collision/Maps/TileAssembler.cpp
    ../collision/DynamicTree.cpp
    ../collision/BoundingIntervalHierarchy.cpp
)

if ("${ASCEMU_VERSION}" STREQUAL "Cata")
    set(sources
        ${sources}
        ${SRC_STORAGE_DB2_FILES}
    )
endif ()
if ("${ASCEMU_VERSION}" STREQUAL "Mop")
    set(sources
        ${sources}
        ${SRC_STORAGE_DB2_FILES}
    )
endif ()

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

if (WIN32 AND NOT USE_PCH)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/INCREMENTAL:NO")
endif ()

if (APPLE)
    target_link_libraries(${PROJECT_NAME} c++)
elseif (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" OR CMAKE_SYSTEM_NAME STREQUAL "kFreeBSD")
    target_link_libraries(${PROJECT_NAME} c++experimental)
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES ENABLE_EXPORTS TRUE)
add_dependencies(${PROJECT_NAME} shared g3dlite Detour Recast)
if (BUILD_ASCEMUSCRIPTS AND BUILD_LUAENGINE)
    add_dependencies(${PROJECT_NAME} lualib)
endif ()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/WorldConf.h.in ${CMAKE_CURRENT_SOURCE_DIR}/WorldConf.h)

target_link_libraries(${PROJECT_NAME} shared g3dlite Detour Recast ${PCRE_LIBRARIES})
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)

if (USE_PCH)
    message(STATUS "Generate PCH for ${PROJECT_NAME}")
	
    target_precompile_headers(${PROJECT_NAME}
    PRIVATE
    <string>
    <map>
    <vector>
    <array>
    <set>
    <cstdio>
    <cstdlib>
    <cstring>
    <CommonDefines.hpp>
    <Common.hpp>
    <WorldPacket.h>
    <Log.hpp>
    <Logging/Logger.hpp>
    <ByteBuffer.h>
    <Config/Config.h>
    <LocationVector.h>
    <Util/Strings.hpp>
    <Util.hpp>
    <Auth/BigNumber.h>
    <Auth/WowCrypt.hpp>
    <VMapManager2.h>
    <IVMapManager.h>
    <VMapFactory.h>
    <GameObjectModel.h>
    <LogonCommDefines.h>
    WorldConf.h
    Chat/ChatHandler.hpp
    Macros/ScriptMacros.hpp
    Management/Faction.h
    Management/ItemInterface.h
    Management/QuestLogEntry.hpp
    Map/Area/AreaStorage.hpp
    Map/Management/MapMgr.hpp
    Movement/Spline/Spline.h
    Movement/Spline/SplineImpl.h
    Movement/Spline/MoveSpline.h
    Movement/Spline/MoveSplineInit.h
    Objects/GameObject.h
    Server/Definitions.h
    Server/LogonCommClient/LogonCommHandler.h
    Server/LogonCommClient/LogonCommClient.h
    Server/Opcodes.hpp
    Server/Packets/SmsgMessageChat.h
    Server/Script/ScriptMgr.h
    Server/WorldSocket.h
    Spell/Definitions/SpellEffects.hpp
    Spell/Spell.h
    Spell/SpellAuras.h
    Storage/MySQLDataStore.hpp
    Objects/Units/Players/Player.h
    Objects/Units/Unit.h
    Management/ObjectMgr.h
)

endif ()
