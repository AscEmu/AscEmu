# Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Wotlk/Naxxramas)

set(SRC_WOTLK_NAXXRAMAS_SCRIPTS_FILES
    ${PATH_PREFIX}/Raid_Naxxramas.cpp
)

source_group(Wotlk/Naxxramas FILES ${SRC_WOTLK_NAXXRAMAS_SCRIPTS_FILES})
unset(PATH_PREFIX)
