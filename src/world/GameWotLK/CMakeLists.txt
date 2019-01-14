# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

include(GameWotLK/Network/CMakeLists.txt)
include(GameWotLK/Storage/CMakeLists.txt)

set(PATH_PREFIX GameWotLK)

set(SRC_GAMELK_FILES
    ${PATH_PREFIX}/UpdateFields.h
)

source_group(GameWotLK FILES ${SRC_GAMELK_FILES})
unset(PATH_PREFIX)
