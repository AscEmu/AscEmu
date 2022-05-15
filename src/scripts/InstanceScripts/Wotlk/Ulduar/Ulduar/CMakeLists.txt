# Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Wotlk/Ulduar/Ulduar)

set(SRC_WOTLK_ULDUAR_SCRIPTS_FILES
    ${PATH_PREFIX}/Raid_Ulduar.cpp
)

source_group(Wotlk/Ulduar/Ulduar FILES ${SRC_WOTLK_ULDUAR_SCRIPTS_FILES})
unset(PATH_PREFIX)
