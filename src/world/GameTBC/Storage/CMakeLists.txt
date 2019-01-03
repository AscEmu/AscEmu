# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX GameTBC/Storage)

set(SRC_GAMEBC_STORAGE_FILES
    ${PATH_PREFIX}/DBCStores.cpp
    ${PATH_PREFIX}/DBCStores.h
    ${PATH_PREFIX}/DBCStructures.h
)

source_group(GameTBC\\Storage FILES ${SRC_GAMEBC_STORAGE_FILES})
unset(PATH_PREFIX)
