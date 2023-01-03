# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Config)

set(SRC_CONFIG_FILES
    ${PATH_PREFIX}/Config.cpp
    ${PATH_PREFIX}/Config.h
)

source_group(Config FILES ${SRC_CONFIG_FILES})
unset(PATH_PREFIX)
