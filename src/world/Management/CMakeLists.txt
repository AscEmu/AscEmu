# Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Management)

set(SRC_MANAGEMENT_FILES
    ${PATH_PREFIX}/AchievementMgr.cpp
    ${PATH_PREFIX}/AchievementMgr.h
    ${PATH_PREFIX}/AddonMgr.cpp
    ${PATH_PREFIX}/AddonMgr.h
    ${PATH_PREFIX}/Arenas.cpp
    ${PATH_PREFIX}/Arenas.h
    ${PATH_PREFIX}/ArenaTeam.cpp
    ${PATH_PREFIX}/ArenaTeam.h
    ${PATH_PREFIX}/AuctionHouse.cpp
    ${PATH_PREFIX}/AuctionHouse.h
    ${PATH_PREFIX}/AuctionMgr.cpp
    ${PATH_PREFIX}/AuctionMgr.h
    ${PATH_PREFIX}/CalendarMgr.cpp
    ${PATH_PREFIX}/CalendarMgr.h
    ${PATH_PREFIX}/Charter.cpp
    ${PATH_PREFIX}/Charter.hpp
    ${PATH_PREFIX}/EquipmentSetMgr.cpp
    ${PATH_PREFIX}/EquipmentSetMgr.h
    ${PATH_PREFIX}/Faction.cpp
    ${PATH_PREFIX}/Faction.h
    ${PATH_PREFIX}/G3DPosition.hpp
    ${PATH_PREFIX}/GameEvent.cpp
    ${PATH_PREFIX}/GameEvent.h
    ${PATH_PREFIX}/GameEventMgr.cpp
    ${PATH_PREFIX}/GameEventMgr.h
    ${PATH_PREFIX}/Group.cpp
    ${PATH_PREFIX}/Group.h
    ${PATH_PREFIX}/HonorHandler.cpp
    ${PATH_PREFIX}/HonorHandler.h
    ${PATH_PREFIX}/ItemInterface.cpp
    ${PATH_PREFIX}/ItemInterface.h
    ${PATH_PREFIX}/ItemProperties.hpp
    ${PATH_PREFIX}/LootMgr.cpp
    ${PATH_PREFIX}/LootMgr.h
    ${PATH_PREFIX}/MailMgr.cpp
    ${PATH_PREFIX}/MailMgr.h
    ${PATH_PREFIX}/ObjectMgr.cpp
    ${PATH_PREFIX}/ObjectMgr.h
    ${PATH_PREFIX}/Quest.cpp
    ${PATH_PREFIX}/Quest.h
    ${PATH_PREFIX}/QuestDefines.hpp
    ${PATH_PREFIX}/QuestLogEntry.cpp
    ${PATH_PREFIX}/QuestLogEntry.hpp
    ${PATH_PREFIX}/QuestMgr.cpp
    ${PATH_PREFIX}/QuestMgr.h
    ${PATH_PREFIX}/Skill.hpp
    ${PATH_PREFIX}/SkillNameMgr.h
    ${PATH_PREFIX}/TaxiMgr.cpp
    ${PATH_PREFIX}/TaxiMgr.h
    ${PATH_PREFIX}/TransporterHandler.cpp
    ${PATH_PREFIX}/TransporterHandler.h
    ${PATH_PREFIX}/WeatherMgr.cpp
    ${PATH_PREFIX}/WeatherMgr.h
    ${PATH_PREFIX}/WordFilter.cpp
    ${PATH_PREFIX}/WordFilter.h
    ${PATH_PREFIX}/WorldStates.h
    ${PATH_PREFIX}/WorldStatesHandler.cpp
    ${PATH_PREFIX}/WorldStatesHandler.h
)

source_group(Management FILES ${SRC_MANAGEMENT_FILES})
unset(PATH_PREFIX)
