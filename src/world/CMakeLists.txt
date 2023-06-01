# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

# set up our project name.
project(world CXX)

file(GLOB SRC_CHAT_FILES Chat/*.cpp Chat/*.h Chat/*.hpp)
file(GLOB SRC_COMMAND_FILES Chat/Commands/*.cpp Chat/Commands/*.h Chat/Commands/*.hpp)
file(GLOB SRC_DATA_FILES Data/*.cpp Data/*.h Data/*.hpp)

include(Game${ASCEMU_VERSION}/Storage/CMakeLists.txt) # SCH: coming soon

file(GLOB SRC_MACROS_FILES Macros/*.cpp Macros/*.h Macros/*.hpp)
file(GLOB SRC_MANAGEMENT_FILES Management/*.cpp Management/*.h Management/*.hpp)
file(GLOB SRC_MANAGEMENT_BATTLEGROUND_FILES Management/Battleground/*.cpp Management/Battleground/*.h Management/Battleground/*.hpp)
file(GLOB SRC_MANAGEMENT_GOSSIP_FILES Management/Gossip/*.cpp Management/Gossip/*.h Management/Gossip/*.hpp)
file(GLOB SRC_MANAGEMENT_GUILD_FILES Management/Guild/*.cpp Management/Guild/*.h Management/Guild/*.hpp)
file(GLOB SRC_MANAGEMENT_LFG_FILES Management/LFG/*.cpp Management/LFG/*.h Management/LFG/*.hpp)
file(GLOB SRC_MANAGEMENT_OBJECTUPDATES_FILES Management/ObjectUpdates/*.cpp Management/ObjectUpdates/*.h Management/ObjectUpdates/*.hpp)
file(GLOB SRC_MANAGEMENT_TICKETS_FILES Management/Tickets/*.cpp Management/Tickets/*.h Management/Tickets/*.hpp)
file(GLOB SRC_MAP_FILES Map/*.cpp Map/*.h Map/*.hpp)
file(GLOB SRC_MAP_AREA_FILES Map/Area/*.cpp Map/Area/*.h Map/Area/*.hpp)
file(GLOB SRC_MAP_CELLS_FILES Map/Cells/*.cpp Map/Cells/*.h Map/Cells/*.hpp)
file(GLOB SRC_MAP_MANAGEMENT_FILES Map/Management/*.cpp Map/Management/*.h Map/Management/*.hpp)
file(GLOB SRC_MAP_MAPS_FILES Map/Maps/*.cpp Map/Maps/*.h Map/Maps/*.hpp)
file(GLOB SRC_MOVEMENT_FILES Movement/*.cpp Movement/*.h Movement/*.hpp)
file(GLOB SRC_MOVEMENT_MOVEMENTGENERATOR_FILES Movement/Spline/*.cpp Movement/Spline/*.h Movement/Spline/*.hpp)
file(GLOB SRC_MOVEMENT_SPLINE_FILES Movement/MovementGenerators/*.cpp Movement/MovementGenerators/*.h Movement/MovementGenerators/*.hpp)
file(GLOB SRC_OBJECTS_FILES Objects/*.cpp Objects/*.h Objects/*.hpp)
file(GLOB SRC_UNITS_FILES Objects/Units/*.cpp Objects/Units/*.h Objects/Units/*.hpp)
file(GLOB SRC_UNITS_CREATURES_FILES Objects/Units/Creatures/*.cpp Objects/Units/Creatures/*.h Objects/Units/Creatures/*.hpp)
file(GLOB SRC_UNITS_SUMMONS_FILES Objects/Units/Creatures/Summons/*.cpp Objects/Units/Creatures/Summons/*.h Objects/Units/Creatures/Summons/*.hpp)
file(GLOB SRC_UNITS_PLAYERS_FILES Objects/Units/Players/*.cpp Objects/Units/Players/*.h Objects/Units/Players/*.hpp)
file(GLOB SRC_SERVER_FILES Server/*.cpp Server/*.h Server/*.hpp)
file(GLOB SRC_SERVER_CONSOLE_FILES Server/Console/*.cpp Server/Console/*.h Server/Console/*.hpp)
file(GLOB SRC_LOGON_COMMUNICATION_CLIENT_FILES Server/LogonCommClient/*.cpp Server/LogonCommClient/*.h Server/LogonCommClient/*.hpp)
file(GLOB SRC_PACKET_FILES Server/Packets/*.cpp Server/Packets/*.h Server/Packets/*.hpp)
file(GLOB SRC_PACKET_HANDLER_FILES Server/Packets/Handlers/*.cpp Server/Packets/Handlers/*.h Server/Packets/Handlers/*.hpp)
file(GLOB SRC_PACKET_MOVEMENT_FILES Server/Packets/Movement/*.cpp Server/Packets/Movement/*.h Server/Packets/Movement/*.hpp)
file(GLOB SRC_SERVER_SCRIPT_FILES Server/Script/*.cpp Server/Script/*.h Server/Script/*.hpp)
file(GLOB SRC_SERVER_WARDEN_FILES Server/Warden/*.cpp Server/Warden/*.h Server/Warden/*.hpp)
file(GLOB SRC_SPELL_FILES Spell/*.cpp Spell/*.h Spell/*.hpp)
file(GLOB SRC_SPELL_CUSTOM_FILES Spell/Customization/*.cpp Spell/Customization/*.h Spell/Customization/*.hpp)
file(GLOB SRC_SPELL_DEFINITIONS_FILES Spell/Definitions/*.cpp Spell/Definitions/*.h Spell/Definitions/*.hpp)
file(GLOB SRC_STORAGE_FILES Storage/*.cpp Storage/*.h Storage/*.hpp)
file(GLOB SRC_STORAGE_DBC_FILES Storage/DBC/*.cpp Storage/DBC/*.h Storage/DBC/*.hpp)

if ("${ASCEMU_VERSION}" STREQUAL "Mop" OR "${ASCEMU_VERSION}" STREQUAL "Cata")
    file(GLOB SRC_STORAGE_DB2_FILES Storage/DB2/*.cpp Storage/DB2/*.h Storage/DB2/*.hpp)
endif ()

if (WIN32)
    set(SRC_WORLD_PROJECT ${SRC_WORLD_PROJECT} ${CMAKE_CURRENT_SOURCE_DIR}/resources.rc)
endif ()

set(WORLD_SRCS
    ${WORLD_SRCS}
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
)

if ("${ASCEMU_VERSION}" STREQUAL "Mop" OR "${ASCEMU_VERSION}" STREQUAL "Cata")
    set(WORLD_SRCS
        ${WORLD_SRCS}
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

# source group.
source_group(world FILES ${SRC_WORLD_PROJECT})

source_group(Chat FILES ${SRC_CHAT_FILES})
source_group(Chat\\Commands FILES ${SRC_COMMAND_FILES})
source_group(Data FILES ${SRC_DATA_FILES})
source_group(Macros FILES ${SRC_MACROS_FILES})
source_group(Management FILES ${SRC_MANAGEMENT_FILES})
source_group(Management\\Battleground FILES ${SRC_MANAGEMENT_BATTLEGROUND_FILES})
source_group(Management\\Gossip FILES ${SRC_MANAGEMENT_GOSSIP_FILES})
source_group(Management\\Guild FILES ${SRC_MANAGEMENT_GUILD_FILES})
source_group(Management\\LFG FILES ${SRC_MANAGEMENT_LFG_FILES})
source_group(Management\\ObjectUpdates FILES ${SRC_MANAGEMENT_OBJECTUPDATES_FILES})
source_group(Management\\Tickets FILES ${SRC_MANAGEMENT_TICKETS_FILES})
source_group(Map FILES ${SRC_MAP_FILES})
source_group(Map\\Area FILES ${SRC_MAP_AREA_FILES})
source_group(Map\\Cells FILES ${SRC_MAP_CELLS_FILES})
source_group(Map\\Management FILES ${SRC_MAP_MANAGEMENT_FILES})
source_group(Map\\Maps FILES ${SRC_MAP_MAPS_FILES})
source_group(Movement FILES ${SRC_MOVEMENT_FILES})
source_group(Movement\\MovementGenerators FILES ${SRC_MOVEMENT_MOVEMENTGENERATOR_FILES})
source_group(Movement\\Spline FILES ${SRC_MOVEMENT_SPLINE_FILES})
source_group(Objects FILES ${SRC_OBJECTS_FILES})
source_group(Objects\\Units FILES ${SRC_UNITS_FILES})
source_group(Objects\\Units\\Creatures FILES ${SRC_UNITS_CREATURES_FILES})
source_group(Objects\\Units\\Creatures\\Summons FILES ${SRC_UNITS_SUMMONS_FILES})
source_group(Objects\\Units\\Players FILES ${SRC_UNITS_PLAYERS_FILES})
source_group(Server FILES ${SRC_SERVER_FILES})
source_group(Server\\Console FILES ${SRC_SERVER_CONSOLE_FILES})
source_group(Server\\LogonCommClient FILES ${SRC_LOGON_COMMUNICATION_CLIENT_FILES})
source_group(Server\\Packets FILES ${SRC_PACKET_FILES})
source_group(Server\\Packets\\Handlers FILES ${SRC_PACKET_HANDLER_FILES})
source_group(Server\\Packets\\Movement FILES ${SRC_PACKET_MOVEMENT_FILES})
source_group(Server\\Script FILES ${SRC_SERVER_SCRIPT_FILES})
source_group(Server\\Warden FILES ${SRC_SERVER_WARDEN_FILES})
source_group(Spell FILES ${SRC_SPELL_FILES})
source_group(Spell\\Customization FILES ${SRC_SPELL_CUSTOM_FILES})
source_group(Spell\\Definitions FILES ${SRC_SPELL_DEFINITIONS_FILES})
source_group(Storage FILES ${SRC_STORAGE_FILES})
source_group(Storage\\DBC FILES ${SRC_STORAGE_DBC_FILES})
source_group(Storage\\DB2 FILES ${SRC_STORAGE_DB2_FILES})

add_executable(${PROJECT_NAME} ${WORLD_SRCS})

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

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/WorldConf.h.in ${CMAKE_CURRENT_SOURCE_DIR}/WorldConf.h)

# include library dependencies.
target_link_libraries(${PROJECT_NAME}
  PUBLIC
    shared
    lualib
    collision
    g3dlite
    Detour
    Recast
    ${PCRE_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)

# generate precompiled header.
if (USE_PCH)
    message(STATUS "Generate PCH for ${PROJECT_NAME}")
    target_precompile_headers(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pchWorld.hpp)
endif ()
