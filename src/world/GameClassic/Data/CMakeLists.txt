# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX GameClassic/Data)

set(SRC_GAMECLASSIC_DATA_FILES
    ${PATH_PREFIX}/MovementInfoClassic.h
)

source_group(GameClassic\\Data FILES ${SRC_GAMECLASSIC_DATA_FILES})
unset(PATH_PREFIX)
