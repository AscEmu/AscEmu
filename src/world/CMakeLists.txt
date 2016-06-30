# Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>

# set up our project name
project(world CXX)

include(Chat/CMakeLists.txt)
include(Chat/Commands/CMakeLists.txt)
include(Management/CMakeLists.txt)
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
include(Spell/CMakeLists.txt)
include(Spell/Customization/CMakeLists.txt)
include(Storage/CMakeLists.txt)
include(Storage/DBC/CMakeLists.txt)
include(Units/CMakeLists.txt)
include(Units/Creatures/CMakeLists.txt)
include(Units/Players/CMakeLists.txt)
include(Units/Summons/CMakeLists.txt)

set(sources
   ${SRC_CHAT_FILES}
   ${SRC_COMMAND_FILES}
   ${SRC_MANAGEMENT_FILES}
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
   ${SRC_LOGON_COMMUNICATION_CLIENT_FILES}
   ${SRC_SPELL_FILES}
   ${SRC_SPELL_CUSTOM_FILES}
   ${SRC_STORAGE_FILES}
   ${SRC_STORAGE_DBC_FILES}
   ${SRC_UNITS_FILES}
   ${SRC_UNITS_CREATURES_FILES}
   ${SRC_UNITS_PLAYERS_FILES}
   ${SRC_UNITS_SUMMONS_FILES}
   
	Faction.cpp
	ObjectMgr.cpp 
	PlayerCache.cpp
	PlayerPacketWrapper.cpp
	Quest.cpp 
    QuestLogEntry.cpp
	QuestMgr.cpp 
	ScriptMgr.cpp 
	SpeedDetector.cpp
	StdAfx.cpp
	Stats.cpp 
	WeatherMgr.cpp 
	WUtil.cpp 
	DeathKnight.cpp
)

if(WIN32)
   set(sources ${sources} resources.rc)
endif()

set(headers
	Definitions.h
	Faction.h
	IUpdatable.h
	ObjectMgr.h
	PlayerCache.h
	Quest.h
    QuestDefines.hpp
    QuestLogEntry.hpp
	QuestMgr.h
    RecastIncludes.hpp
	ScriptMgr.h
	ScriptSetup.h
	Skill.h
	SkillNameMgr.h
	SpeedDetector.h
	Stats.h
	StdAfx.h
	UpdateFields.h
	UpdateMask.h
	WeatherMgr.h
    WorldStrings.h
	WUtil.h
)

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
   ${CMAKE_SOURCE_DIR}/src/shared
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${ZLIB_INCLUDE_DIRS}
)

add_executable(${PROJECT_NAME} ${sources} ${headers})

if(USE_PCH AND PCHSupport_FOUND)
    ADD_NATIVE_PRECOMPILED_HEADER(${PROJECT_NAME} StdAfx.h ${CMAKE_CURRENT_SOURCE_DIR}/StdAfx.cpp)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES ENABLE_EXPORTS TRUE)
add_dependencies(${PROJECT_NAME} shared g3dlite collision Detour Recast)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/WorldConf.h.in ${CMAKE_CURRENT_SOURCE_DIR}/WorldConf.h)

target_link_libraries(${PROJECT_NAME} shared g3dlite collision Detour Recast ${PCRE_LIBRARIES})
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)
