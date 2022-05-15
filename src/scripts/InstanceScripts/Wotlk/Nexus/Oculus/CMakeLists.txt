# Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Wotlk/Nexus/Oculus)

set(SRC_WOTLK_OCULUS_SCRIPTS_FILES
    ${PATH_PREFIX}/Instance_TheOculus.cpp
)

source_group(Wotlk/Nexus/Oculus FILES ${SRC_WOTLK_OCULUS_SCRIPTS_FILES})
unset(PATH_PREFIX)
