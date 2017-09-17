# Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX GameWotLK/Handlers)

set(SRC_GAMELK_HANDLER_FILES
    ${PATH_PREFIX}/CharacterHandler.cpp
    ${PATH_PREFIX}/GMTicketHandler.cpp
)

source_group(GameWotLK\\Handlers FILES ${SRC_GAMELK_HANDLER_FILES})
unset(PATH_PREFIX)
