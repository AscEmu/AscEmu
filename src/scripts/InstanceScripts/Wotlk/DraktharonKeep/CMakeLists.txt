# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Wotlk/DraktharonKeep)

set(SRC_WOTLK_DRAKTHARON_SCRIPTS_FILES
    ${PATH_PREFIX}/Instance_DrakTharonKeep.cpp
)

source_group(Wotlk/DraktharonKeep FILES ${SRC_WOTLK_DRAKTHARON_SCRIPTS_FILES})
unset(PATH_PREFIX)
