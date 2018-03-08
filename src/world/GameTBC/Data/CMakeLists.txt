# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX GameTBC/Data)

set(SRC_GAMEBC_DATA_FILES
    ${PATH_PREFIX}/MovementInfoTBC.h
)

source_group(GameTBC\\Data FILES ${SRC_GAMEBC_DATA_FILES})
unset(PATH_PREFIX)
