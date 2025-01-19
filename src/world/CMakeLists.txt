# Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>

# set up our project name.
set(EXECUTABLE_NAME world)

file(GLOB SRC_CHAT_FILES Chat/*.cpp Chat/*.h Chat/*.hpp)
file(GLOB SRC_COMMAND_FILES Chat/Commands/*.cpp Chat/Commands/*.h Chat/Commands/*.hpp)
file(GLOB SRC_COMMAND_ACCOUNT_FILES Chat/Commands/Account/*.cpp Chat/Commands/Account/*.hpp)
file(GLOB SRC_COMMAND_ACHIEVEMENT_FILES Chat/Commands/Achievement/*.cpp Chat/Commands/Achievement/*.hpp)
file(GLOB SRC_DATA_FILES Data/*.cpp Data/*.h Data/*.hpp)
file(GLOB SRC_MACROS_FILES Macros/*.cpp Macros/*.h Macros/*.hpp)
file(GLOB SRC_MANAGEMENT_FILES Management/*.cpp Management/*.h Management/*.hpp)
file(GLOB SRC_MANAGEMENT_BATTLEGROUND_FILES Management/Battleground/*.cpp Management/Battleground/*.h Management/Battleground/*.hpp)
file(GLOB SRC_MANAGEMENT_GOSSIP_FILES Management/Gossip/*.cpp Management/Gossip/*.h Management/Gossip/*.hpp)
file(GLOB SRC_MANAGEMENT_GUILD_FILES Management/Guild/*.cpp Management/Guild/*.h Management/Guild/*.hpp)
file(GLOB SRC_MANAGEMENT_LFG_FILES Management/LFG/*.cpp Management/LFG/*.h Management/LFG/*.hpp)
file(GLOB SRC_MANAGEMENT_LOOT_FILES Management/Loot/*.cpp Management/Loot/*.h Management/Loot/*.hpp)
file(GLOB SRC_MANAGEMENT_OBJECTUPDATES_FILES Management/ObjectUpdates/*.cpp Management/ObjectUpdates/*.h Management/ObjectUpdates/*.hpp)
file(GLOB SRC_MANAGEMENT_TICKETS_FILES Management/Tickets/*.cpp Management/Tickets/*.h Management/Tickets/*.hpp)
file(GLOB SRC_MAP_FILES Map/*.cpp Map/*.h Map/*.hpp)
file(GLOB SRC_MAP_AREA_FILES Map/Area/*.cpp Map/Area/*.h Map/Area/*.hpp)
file(GLOB SRC_MAP_CELLS_FILES Map/Cells/*.cpp Map/Cells/*.h Map/Cells/*.hpp)
file(GLOB SRC_MAP_MANAGEMENT_FILES Map/Management/*.cpp Map/Management/*.h Map/Management/*.hpp)
file(GLOB SRC_MAP_MAPS_FILES Map/Maps/*.cpp Map/Maps/*.h Map/Maps/*.hpp)
file(GLOB SRC_MOVEMENT_FILES Movement/*.cpp Movement/*.h Movement/*.hpp)
file(GLOB SRC_MOVEMENT_MOVEMENTGENERATOR_FILES Movement/MovementGenerators/*.cpp Movement/MovementGenerators/*.h Movement/MovementGenerators/*.hpp)
file(GLOB SRC_MOVEMENT_SPLINE_FILES Movement/Spline/*.cpp Movement/Spline/*.h Movement/Spline/*.hpp)
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
file(GLOB SRC_STORAGE_WDB_FILES Storage/WDB/*.cpp Storage/WDB/*.h Storage/WDB/*.hpp)
file(GLOB SRC_LOCAL_DIR *.cpp *.h *.hpp)

if (WIN32)
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/resources.rc")
endif ()

# source group.
ascemu_source_group(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(${EXECUTABLE_NAME}
    ${SRC_CHAT_FILES}
    ${SRC_COMMAND_FILES}
    ${SRC_COMMAND_ACCOUNT_FILES}
    ${SRC_COMMAND_ACHIEVEMENT_FILES}
    ${SRC_DATA_FILES}
    ${SRC_MACROS_FILES}
    ${SRC_MANAGEMENT_FILES}
    ${SRC_MANAGEMENT_BATTLEGROUND_FILES}
    ${SRC_MANAGEMENT_GOSSIP_FILES}
    ${SRC_MANAGEMENT_GUILD_FILES}
    ${SRC_MANAGEMENT_LFG_FILES}
    ${SRC_MANAGEMENT_LOOT_FILES}
    ${SRC_MANAGEMENT_OBJECTUPDATES_FILES}
    ${SRC_MANAGEMENT_TICKETS_FILES}
    ${SRC_MAP_FILES}
    ${SRC_MAP_AREA_FILES}
    ${SRC_MAP_CELLS_FILES}
    ${SRC_MAP_MANAGEMENT_FILES}
    ${SRC_MAP_MAPS_FILES}
    ${SRC_MOVEMENT_FILES}
    ${SRC_MOVEMENT_MOVEMENTGENERATOR_FILES}
    ${SRC_MOVEMENT_SPLINE_FILES}
    ${SRC_OBJECTS_FILES}
    ${SRC_UNITS_FILES}
    ${SRC_UNITS_CREATURES_FILES}
    ${SRC_UNITS_SUMMONS_FILES}
    ${SRC_UNITS_PLAYERS_FILES}
    ${SRC_SERVER_FILES}
    ${SRC_SERVER_CONSOLE_FILES}   
    ${SRC_LOGON_COMMUNICATION_CLIENT_FILES}
    ${SRC_PACKET_FILES}
    ${SRC_PACKET_HANDLER_FILES}
    ${SRC_PACKET_MOVEMENT_FILES}
    ${SRC_SERVER_SCRIPT_FILES}
    ${SRC_SERVER_WARDEN_FILES}
    ${SRC_SPELL_FILES}
    ${SRC_SPELL_CUSTOM_FILES}
    ${SRC_SPELL_DEFINITIONS_FILES}
    ${SRC_STORAGE_FILES}
    ${SRC_STORAGE_WDB_FILES}
    ${SRC_LOCAL_DIR}
    ${APP_ICON_RESOURCE_WINDOWS}
)

include_directories(
    ${OPENSSL_INCLUDE_DIR}
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

if (WIN32 AND NOT AE_USE_PCH)
    set_target_properties(${EXECUTABLE_NAME} PROPERTIES LINK_FLAGS "/INCREMENTAL:NO")
endif ()

if (APPLE)
    LIST(APPEND WORLD_LINK_EXTRA_LIBS c++)
elseif (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" OR CMAKE_SYSTEM_NAME STREQUAL "kFreeBSD")
    LIST(APPEND WORLD_LINK_EXTRA_LIBS c++experimental)
endif ()

if (BUILD_LUAENGINE)
    LIST(APPEND WORLD_LINK_EXTRA_LIBS lualib)
endif()

set_target_properties(${EXECUTABLE_NAME} PROPERTIES ENABLE_EXPORTS TRUE)

add_dependencies(${EXECUTABLE_NAME} shared g3dlite Detour Recast)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/WorldConf.h.in ${CMAKE_CURRENT_SOURCE_DIR}/WorldConf.h)

# include library dependencies.
target_link_libraries(${EXECUTABLE_NAME}
  PUBLIC
    ${WORLD_LINK_EXTRA_LIBS}
    shared
    collision
    g3dlite
    Detour
    Recast
)

unset(WORLD_LINK_EXTRA_LIBS)

# generate precompiled header.
if (AE_USE_PCH)
    message(STATUS "Generate PCH for ${EXECUTABLE_NAME}")
    target_precompile_headers(${EXECUTABLE_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pchWorld.hpp)
endif ()

install(TARGETS ${EXECUTABLE_NAME} RUNTIME DESTINATION .)
