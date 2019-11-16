# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX IsleOfConquest)

set(SRC_IsleOfConquest
    ${PATH_PREFIX}/IsleOfConquest.cpp
    ${PATH_PREFIX}/IsleOfConquest.h
)

source_group(IsleOfConquest FILES ${SRC_IsleOfConquest})
unset(PATH_PREFIX)
