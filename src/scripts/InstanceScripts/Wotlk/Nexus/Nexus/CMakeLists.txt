# Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Wotlk/Nexus/Nexus)

set(SRC_WOTLK_NEXUS_SCRIPTS_FILES
    ${PATH_PREFIX}/Instance_Nexus.cpp
)

source_group(Wotlk/Nexus/Nexus FILES ${SRC_WOTLK_NEXUS_SCRIPTS_FILES})
unset(PATH_PREFIX)
