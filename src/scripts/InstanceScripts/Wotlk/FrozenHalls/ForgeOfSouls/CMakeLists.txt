# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Wotlk/FrozenHalls/ForgeOfSouls)

set(SRC_WOTLK_FORGEOFSOULS_SCRIPTS_FILES
    ${PATH_PREFIX}/Instance_ForgeOfSouls.cpp
)

source_group(Wotlk/FrozenHalls/ForgeOfSouls FILES ${SRC_WOTLK_FORGEOFSOULS_SCRIPTS_FILES})
unset(PATH_PREFIX)
