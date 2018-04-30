# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX GameWotLK/Data)

set(SRC_GAMEWOTLK_DATA_FILES
    ${PATH_PREFIX}/MovementInfoWotLK.h
)

source_group(GameWotLK\\Data FILES ${SRC_GAMEWOTLK_DATA_FILES})
unset(PATH_PREFIX)
