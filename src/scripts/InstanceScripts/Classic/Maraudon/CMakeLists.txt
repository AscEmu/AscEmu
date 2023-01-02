# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Classic/Maraudon)

set(SRC_CLASSIC_MARAUDON_SCRIPTS_FILES
    ${PATH_PREFIX}/Instance_Maraudon.cpp
)

source_group(Classic/Maraudon FILES ${SRC_CLASSIC_MARAUDON_SCRIPTS_FILES})
unset(PATH_PREFIX)
