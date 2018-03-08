# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX GameCata/Data)

set(SRC_GAMECATA_DATA_FILES
    ${PATH_PREFIX}/MovementInfoCata.h
)

source_group(GameCata\\Data FILES ${SRC_GAMECATA_DATA_FILES})
unset(PATH_PREFIX)
