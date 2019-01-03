# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX GameWotLK/Network)

set(SRC_GAMELK_NETWORK_FILES
    ${PATH_PREFIX}/Opcodes.cpp
    ${PATH_PREFIX}/Opcodes.h
    ${PATH_PREFIX}/WorldPacketHandlers.cpp
)

source_group(GameWotLK\\Network FILES ${SRC_GAMELK_NETWORK_FILES})
unset(PATH_PREFIX)
