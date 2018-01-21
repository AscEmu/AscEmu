# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

include(GameTBC/Handlers/CMakeLists.txt)
include(GameTBC/Network/CMakeLists.txt)
include(GameTBC/Storage/CMakeLists.txt)

set(PATH_PREFIX GameTBC)

set(SRC_GAMEBC_FILES
    ${PATH_PREFIX}/UpdateFields.h
)

source_group(GameTBC FILES ${SRC_GAMEBC_FILES})
unset(PATH_PREFIX)
