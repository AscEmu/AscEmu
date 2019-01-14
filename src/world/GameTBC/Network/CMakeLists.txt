# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX GameTBC/Network)

set(SRC_GAMEBC_NETWORK_FILES
    ${PATH_PREFIX}/Opcodes.cpp
    ${PATH_PREFIX}/Opcodes.h
    ${PATH_PREFIX}/WorldPacketHandlers.cpp
)

source_group(GameTBC\\Network FILES ${SRC_GAMEBC_NETWORK_FILES})
unset(PATH_PREFIX)
