# Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Management/Guild)

set(SRC_MANAGEMENT_GUILD_FILES
    ${PATH_PREFIX}/GuildDefinitions.hpp
    ${PATH_PREFIX}/Guild.cpp
    ${PATH_PREFIX}/Guild.hpp
    ${PATH_PREFIX}/GuildBankTab.cpp
    ${PATH_PREFIX}/GuildBankTab.hpp
    ${PATH_PREFIX}/GuildBankEventLog.cpp
    ${PATH_PREFIX}/GuildBankEventLog.hpp
    ${PATH_PREFIX}/GuildBankRightsAndSlots.cpp
    ${PATH_PREFIX}/GuildBankRightsAndSlots.hpp
    ${PATH_PREFIX}/GuildEventLog.cpp
    ${PATH_PREFIX}/GuildEventLog.hpp
    ${PATH_PREFIX}/GuildEmblemInfo.cpp
    ${PATH_PREFIX}/GuildEmblemInfo.hpp
    ${PATH_PREFIX}/GuildFinderMgr.cpp
    ${PATH_PREFIX}/GuildFinderMgr.hpp
    ${PATH_PREFIX}/GuildLog.cpp
    ${PATH_PREFIX}/GuildLog.hpp
    ${PATH_PREFIX}/GuildLogHolder.cpp
    ${PATH_PREFIX}/GuildLogHolder.hpp
    ${PATH_PREFIX}/GuildMgr.cpp
    ${PATH_PREFIX}/GuildMgr.hpp
    ${PATH_PREFIX}/GuildNewsLog.cpp
    ${PATH_PREFIX}/GuildNewsLog.hpp
    ${PATH_PREFIX}/GuildRankInfo.cpp
    ${PATH_PREFIX}/GuildRankInfo.hpp
)

source_group(Management\\Guild FILES ${SRC_MANAGEMENT_GUILD_FILES})
unset(PATH_PREFIX)
