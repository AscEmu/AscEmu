# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX GameCata/Network)

set(SRC_GAMECATA_NETWORK_FILES
    ${PATH_PREFIX}/Opcodes.cpp
    ${PATH_PREFIX}/Opcodes.h
    ${PATH_PREFIX}/WorldPacketHandlers.cpp
)

source_group(GameCata\\Network FILES ${SRC_GAMECATA_NETWORK_FILES})
unset(PATH_PREFIX)
