# Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX GameCata)

set(SRC_GAMECATA_FILES
    ${PATH_PREFIX}/Handlers/CharacterHandler.cpp
    ${PATH_PREFIX}/Handlers/MiscellaneousHandler.cpp
    ${PATH_PREFIX}/Network/Opcodes.cpp
    ${PATH_PREFIX}/Network/Opcodes.h
)

source_group(GameCata FILES ${SRC_GAMECATA_FILES})
unset(PATH_PREFIX)
