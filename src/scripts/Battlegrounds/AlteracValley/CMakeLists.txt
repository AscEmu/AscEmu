# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX AlteracValley)

set(SRC_AlteracValley
    ${PATH_PREFIX}/AlteracValley.cpp
    ${PATH_PREFIX}/AlteracValley.h
    ${PATH_PREFIX}/AlteracValleyDefinitions.h
)

source_group(AlteracValley FILES ${SRC_AlteracValley})
unset(PATH_PREFIX)
