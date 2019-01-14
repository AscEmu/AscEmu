# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX GameCata/Management)

set(SRC_GAMECATA_MANAGEMENT_FILES
    ${PATH_PREFIX}/GuildFinderMgr.cpp
    ${PATH_PREFIX}/GuildFinderMgr.h
    ${PATH_PREFIX}/QuestMgr.Cata.cpp
)

source_group(GameCata\\Management FILES ${SRC_GAMECATA_MANAGEMENT_FILES})
unset(PATH_PREFIX)
