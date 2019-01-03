# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX GameClassic/Network)

set(SRC_GAMECLASSIC_NETWORK_FILES
    ${PATH_PREFIX}/Opcodes.cpp
    ${PATH_PREFIX}/Opcodes.h
    ${PATH_PREFIX}/WorldPacketHandlers.cpp
)

source_group(GameClassic\\Network FILES ${SRC_GAMECLASSIC_NETWORK_FILES})
unset(PATH_PREFIX)
