# Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Tbc/TempestKeep/Botanica)

set(SRC_TBC_BOTANICA_SCRIPTS_FILES
    ${PATH_PREFIX}/Instance_Botanica.cpp
)

source_group(Tbc/TempestKeep/Botanica FILES ${SRC_TBC_BOTANICA_SCRIPTS_FILES})
unset(PATH_PREFIX)
