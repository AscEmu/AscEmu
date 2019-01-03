# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX GameCata/Movement)

set(SRC_GAMECATA_MOVEMENT_FILES
    ${PATH_PREFIX}/MovementDefines.h
    ${PATH_PREFIX}/MovementStructures.h
)

source_group(GameCata\\Movement FILES ${SRC_GAMECATA_MOVEMENT_FILES})
unset(PATH_PREFIX)
