# Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX GameTBC)

set(SRC_GAMEBC_FILES
    ${PATH_PREFIX}/Handlers/CharacterHandler.cpp
    ${PATH_PREFIX}/Network/Opcodes.cpp
    ${PATH_PREFIX}/Network/Opcodes.h
    ${PATH_PREFIX}/Network/WorldPacketHandlers.cpp
    ${PATH_PREFIX}/Storage/DBCStores.cpp
    ${PATH_PREFIX}/Storage/DBCStores.h
    ${PATH_PREFIX}/Storage/DBCStructures.h
    ${PATH_PREFIX}/UpdateFields.h
)

source_group(GameTBC FILES ${SRC_GAMEBC_FILES})
unset(PATH_PREFIX)
