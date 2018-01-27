# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX GameTBC/Handlers)

set(SRC_GAMEBC_HANDLER_FILES
    ${PATH_PREFIX}/CharacterHandler.cpp
)

source_group(GameTBC\\Handlers FILES ${SRC_GAMEBC_HANDLER_FILES})
unset(PATH_PREFIX)
