# Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX GameWotLK)

set(SRC_GAMELK_FILES
    ${PATH_PREFIX}/Handlers/CharacterHandler.cpp
    ${PATH_PREFIX}/Network/Opcodes.cpp
    ${PATH_PREFIX}/Network/Opcodes.h
    ${PATH_PREFIX}/UpdateFields.h
)

source_group(GameWotLK FILES ${SRC_GAMELK_FILES})
unset(PATH_PREFIX)
