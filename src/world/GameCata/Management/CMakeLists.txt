# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX GameCata/Management)

set(SRC_GAMECATA_MANAGEMENT_FILES
    ${PATH_PREFIX}/Guild.cpp
    ${PATH_PREFIX}/Guild.h
    ${PATH_PREFIX}/GuildBankEventLog.cpp
    ${PATH_PREFIX}/GuildBankEventLog.h
    ${PATH_PREFIX}/GuildBankRightsAndSlots.cpp
    ${PATH_PREFIX}/GuildBankRightsAndSlots.h
    ${PATH_PREFIX}/GuildBankTab.cpp
    ${PATH_PREFIX}/GuildBankTab.h
    ${PATH_PREFIX}/GuildEmblemInfo.cpp
    ${PATH_PREFIX}/GuildEmblemInfo.h
    ${PATH_PREFIX}/GuildEventLog.cpp
    ${PATH_PREFIX}/GuildEventLog.h
    ${PATH_PREFIX}/GuildFinderMgr.cpp
    ${PATH_PREFIX}/GuildFinderMgr.h
    ${PATH_PREFIX}/GuildLog.cpp
    ${PATH_PREFIX}/GuildLog.h
    ${PATH_PREFIX}/GuildLogHolder.cpp
    ${PATH_PREFIX}/GuildLogHolder.h
    ${PATH_PREFIX}/GuildMgr.cpp
    ${PATH_PREFIX}/GuildMgr.h
    ${PATH_PREFIX}/GuildNewsLog.cpp
    ${PATH_PREFIX}/GuildNewsLog.h
    ${PATH_PREFIX}/GuildRankInfo.cpp
    ${PATH_PREFIX}/GuildRankInfo.h
    ${PATH_PREFIX}/QuestMgr.Cata.cpp
)

source_group(GameCata\\Management FILES ${SRC_GAMECATA_MANAGEMENT_FILES})
unset(PATH_PREFIX)
