# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Wotlk/Ulduar/HallsOfLightning)

set(SRC_WOTLK_HALLSOFLIGHTNING_SCRIPTS_FILES
    ${PATH_PREFIX}/Instance_HallsOfLightning.cpp
)

source_group(Wotlk/Ulduar/HallsOfLightning FILES ${SRC_WOTLK_HALLSOFLIGHTNING_SCRIPTS_FILES})
unset(PATH_PREFIX)
