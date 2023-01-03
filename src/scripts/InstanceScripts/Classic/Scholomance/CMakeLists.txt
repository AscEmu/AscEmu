# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Classic/Scholomance)

set(SRC_CLASSIC_SCHOLO_SCRIPTS_FILES
    ${PATH_PREFIX}/Instance_Scholomance.cpp
)

source_group(Classic/Scholomance FILES ${SRC_CLASSIC_SCHOLO_SCRIPTS_FILES})
unset(PATH_PREFIX)
