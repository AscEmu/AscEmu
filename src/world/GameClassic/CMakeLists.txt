# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

include(GameClassic/Network/CMakeLists.txt)
include(GameClassic/Storage/CMakeLists.txt)

set(PATH_PREFIX GameClassic)

set(SRC_GAMECLASSIC_FILES
    ${PATH_PREFIX}/UpdateFields.h
)

source_group(GameClassic FILES ${SRC_GAMECLASSIC_FILES})
unset(PATH_PREFIX)
